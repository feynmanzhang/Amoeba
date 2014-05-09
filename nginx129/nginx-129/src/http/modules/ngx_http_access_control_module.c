
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <math.h>

#define NGX_MAX_TPS_MS 100000*1000 //zll,max request limited in 100000r/s
#define NGX_MAX_BURST_MULTIPLE 100 //zll,max request burst = burst * tps,it is different from "limit req moudle".

typedef struct {
    u_char                       color;
    u_char                       dummy;
    u_short                      len;
    ngx_queue_t                  queue;
    ngx_msec_t                   last;
    /* integer value, 1 corresponds to 0.001 r/s */这
    ngx_uint_t                   excess;
    ngx_uint_t                   count;
    u_char                       data[1];
} ngx_http_access_control_node_t;


typedef struct {
    ngx_rbtree_t                  rbtree;
    ngx_rbtree_node_t             sentinel;
    ngx_queue_t                   queue;
} ngx_http_access_control_shctx_t;


typedef struct {
    ngx_http_access_control_shctx_t  *sh;
    ngx_slab_pool_t             *shpool;
    /* integer value, 1 corresponds to 0.001 r/s */
//    ngx_uint_t                   rate;
    ngx_int_t                    index;
    ngx_str_t                    var;
    ngx_http_access_control_node_t   *node;
//    ngx_uint_t                   keylimit;///zll keylimit:0
    ngx_array_t*                 keyarray;
    ngx_array_t*                 urisidarray;
    ngx_array_t*                 hashkeyarray;
//    ngx_hash_t*                  keyhash;
//    ngx_pool_t*         		 keyhashpool;
} ngx_http_access_control_ctx_t;


typedef struct {
    ngx_shm_zone_t              *shm_zone;
    /* integer value, 1 corresponds to 0.001 r/s */
    ngx_uint_t                   burst;
    ngx_uint_t                   nodelay; /* unsigned  nodelay:1 */
} ngx_http_access_control_limit_t;


typedef struct {
    ngx_array_t                  limits;
    ngx_uint_t                   limit_log_level;
    ngx_uint_t                   delay_log_level;
    ngx_uint_t                   status_code;
} ngx_http_access_control_conf_t;


static void ngx_http_access_control_delay(ngx_http_request_t *r);
static ngx_int_t ngx_http_access_control_lookup(ngx_http_access_control_limit_t *limit,
    ngx_uint_t hash, u_char *data, size_t len, ngx_uint_t *ep,
    ngx_uint_t account, size_t maxtps);
static ngx_msec_t ngx_http_access_control_account(ngx_http_access_control_limit_t *limits,
    ngx_uint_t n, ngx_uint_t *ep, ngx_http_access_control_limit_t **limit,size_t maxtps);
static void ngx_http_access_control_expire(ngx_http_access_control_ctx_t *ctx,ngx_uint_t n,size_t maxtps);

static void *ngx_http_access_control_create_conf(ngx_conf_t *cf);
static char *ngx_http_access_control_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);
//static char *ngx_http_access_control_zone(ngx_conf_t *cf, ngx_command_t *cmd,
//    void *conf);
static char *ngx_http_access_control(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static ngx_int_t ngx_http_access_control_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_limit_init_keypool(ngx_conf_t *cf,ngx_http_access_control_ctx_t  *ctx);
static void checkerr(OCIError *errhp, sword status,ngx_log_t *log);
static void cleanOCIEnv(ngx_cycle_t *cycle);
static char *
ngx_http_access_control_dbconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
void ngx_http_access_control_body_handler(ngx_http_request_t *r)
{}

static ngx_conf_enum_t  ngx_http_access_control_log_levels[] = {
//	{ngx_string("debug"),NGX_LOG_DEBUG},
    { ngx_string("info"), NGX_LOG_INFO },
    { ngx_string("notice"), NGX_LOG_NOTICE },
    { ngx_string("warn"), NGX_LOG_WARN },
    { ngx_string("error"), NGX_LOG_ERR },
    { ngx_null_string, 0 }
};


static ngx_conf_num_bounds_t  ngx_http_access_control_status_bounds = {
    ngx_conf_check_num_bounds, 400, 599
};


static ngx_command_t  ngx_http_access_control_commands[] = {

//    { ngx_string("access_control_zone"),
//      NGX_HTTP_MAIN_CONF|NGX_CONF_1MORE,
//      ngx_http_access_control_zone,
//      0,
//      0,
//      NULL },
    { ngx_string("accesscontrol_dbconf"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_1MORE,
      ngx_http_access_control_dbconf,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("access_control"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,//NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE123,
      ngx_http_access_control,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("access_control_log_level"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,//NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_access_control_conf_t, limit_log_level),
      &ngx_http_access_control_log_levels },

    { ngx_string("access_control_status"),
      NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,//NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_access_control_conf_t, status_code),
      &ngx_http_access_control_status_bounds },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_access_control_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_access_control_init,           /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_access_control_create_conf,        /* create location configuration */
    ngx_http_access_control_merge_conf          /* merge location configuration */
};


ngx_module_t  ngx_http_access_control_module = {
    NGX_MODULE_V1,
    &ngx_http_access_control_module_ctx,        /* module context */
    ngx_http_access_control_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    cleanOCIEnv,                           /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_access_control_handler(ngx_http_request_t *r)
{
    uint32_t                     hash;
    ngx_int_t                    rc,rc2,fm;
    ngx_uint_t                   n, excess;
    ngx_msec_t                   delay;
    ngx_http_access_control_ctx_t    *ctx;
    ngx_http_access_control_conf_t   *lrcf;
    ngx_http_access_control_limit_t  *limit, *limits;
    size_t                       maxtps = 0, access = 0, i = 0;//,count =0;
    ngx_str_t                    tokenkey,uri,hashkey,sid;
    u_char						*keystrstart,*keystrend,*sidstrstart,*sidstrend,*uristrstart,*uristrend;


    if (r->main->access_control_set) {
        return NGX_DECLINED;
    }

    lrcf = ngx_http_get_module_loc_conf(r, ngx_http_access_control_module);
    limits = lrcf->limits.elts;

    excess = 0;

    rc = NGX_DECLINED;

#if (NGX_SUPPRESS_WARN)
    limit = NULL;
#endif

    for (n = 0; n < lrcf->limits.nelts; n++) {

        limit = &limits[n];

        ctx = limit->shm_zone->data;

        if(r->uri.len <= 1)
        	return NGX_HTTP_BAD_REQUEST;

            if(r->method == NGX_HTTP_POST){


			rc2 = ngx_http_read_client_request_body(r,ngx_http_access_control_body_handler);
//			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"rc2:\"%d\"",rc2);
		    if (rc2 >= NGX_HTTP_SPECIAL_RESPONSE) {
		        return rc2;
		    }
		    ngx_http_finalize_request(r,rc2);

        	if(r->request_body ==  NULL)
        		return NGX_HTTP_INTERNAL_SERVER_ERROR;

        	if(r->request_body->bufs == NULL || r->request_body->bufs->buf == NULL)
        		return NGX_HTTP_BAD_REQUEST;

        	if((uristrend = ngx_strlchr(r->uri_start,r->uri_end,'?')) != NULL){
				uri.data = r->uri.data;
				uristrstart = r->uri_start;
				uri.len = uristrend - uristrstart;
			}else{
				uri.data = r->uri.data;
				uri.len = r->uri.len;
			}

//			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"uri:\"%V\"",&uri);

			//catch the 'sid' parameter
			if((sidstrstart = ngx_strcasestrn(r->request_body->bufs->buf->pos,"sid=",3)) != NULL)
			{
				sid.data = sidstrstart+4;

				if((sidstrend = ngx_strlchr(sidstrstart,r->request_body->bufs->buf->last,'&'))== NULL)
					sid.len = r->request_body->bufs->buf->last - sidstrstart - 4;
				else
					sid.len = sidstrend - sidstrstart - 4;

//				 ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"sid:\"%V\"",&sid);
			}
			else if((sidstrstart = ngx_strcasestrn(r->request_body->bufs->buf->pos,"&sid=",4)) != NULL)
			{
				sid.data = sidstrstart+5;

				if((sidstrend = ngx_strlchr(sidstrstart+1,r->request_body->bufs->buf->last,'&'))== NULL)
					sid.len = r->request_body->bufs->buf->last - sidstrstart - 5;
				else
					sid.len = sidstrend - sidstrstart - 5;

//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"sid:\"%V\"",&sid);
			}
			else
				return NGX_HTTP_BAD_REQUEST;

			//store "request_sid" var  to log
			r->request_sid.data = sid.data;
			r->request_sid.len = sid.len;



			access = 0;
			for(i =0; i < ctx->urisidarray->nelts; i++){
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"urisidarray:\"%V\"",&((ngx_str_t*)ctx->urisidarray->elts)[i]);
				if( ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data, uri.data,uri.len) == 0
						&&  ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data + uri.len, sid.data,sid.len)== 0){
					access++;
					break;
				}
			}

			if(access == 0)
				return NGX_HTTP_BAD_REQUEST;

			//catch the 'key' parameter
			if((keystrstart = ngx_strcasestrn(r->request_body->bufs->buf->pos,"&key=",4)) != NULL)
			{
				tokenkey.data = keystrstart+5;

				if((keystrend = ngx_strlchr(keystrstart+1,r->request_body->bufs->buf->last,'&'))== NULL)
					tokenkey.len = r->request_body->bufs->buf->last - keystrstart - 5;
				else
					tokenkey.len = keystrend - keystrstart - 5;

//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"key:\"%V\"",&tokenkey);
			}
			else if((keystrstart = ngx_strcasestrn(r->request_body->bufs->buf->pos,"key=",3)) != NULL)
			{
				tokenkey.data = keystrstart+4;

				if((keystrend = ngx_strlchr(keystrstart,r->request_body->bufs->buf->last,'&'))== NULL)
					tokenkey.len = r->request_body->bufs->buf->last - keystrstart - 4;
				else
					tokenkey.len = keystrend - keystrstart - 4;
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"key:\"%V\"",&tokenkey);
			}
			else
				return NGX_HTTP_UNAUTHORIZED;

			//store "request_key" var  to log
			r->request_key.data = tokenkey.data;
			r->request_key.len = tokenkey.len;


			access =0;
			for(i =0; i < ctx->keyarray->nelts; i++){
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"tokenkeyarr:\"%V\"",&((ngx_str_t*)ctx->keyarray->elts)[i]);
				if( ngx_strncmp(((ngx_str_t*)ctx->keyarray->elts)[i].data, tokenkey.data,((ngx_str_t*)ctx->keyarray->elts)[i].len) == 0 ){
					if( ngx_strncmp(((ngx_str_t*)ctx->keyarray->elts)[i].data, tokenkey.data,tokenkey.len) == 0 )
                    {
                        access++;
                        break;
                    }
				}
			}

			if(access == 0)
				return NGX_HTTP_UNAUTHORIZED;

        }
        else if(r->method == NGX_HTTP_GET) {
        	if((uristrend = ngx_strlchr(r->uri_start,r->uri_end,'?')) == NULL)
        	return NGX_HTTP_NOT_FOUND;
			uri.data = r->uri.data;
			uristrstart = r->uri_start;
			uri.len = uristrend - uristrstart;
//			ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"uri:\"%V\"",&uri);

			//catch the 'sid' parameter
			if((sidstrstart = ngx_strcasestrn(r->uri.data,"sid=",3)) != NULL)
			{
				sid.data = sidstrstart+4;

				if((sidstrend = ngx_strlchr(sidstrstart,r->uri_end,'&'))== NULL)
					sid.len = r->uri_end - sidstrstart - 4;
				else
					sid.len = sidstrend - sidstrstart - 4;

//				 ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"sid:\"%V\"",&sid);
			}
			else if((sidstrstart = ngx_strcasestrn(r->uri.data,"&sid=",4)) != NULL)
			{
				sid.data = sidstrstart+5;

				if((sidstrend = ngx_strlchr(sidstrstart+1,r->uri_end,'&'))== NULL)
					sid.len = r->uri_end - sidstrstart - 5;
				else
					sid.len = sidstrend - sidstrstart - 5;

//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"sid:\"%V\"",&sid);
			}
			else
				return NGX_HTTP_BAD_REQUEST;

			//store "request_sid" var  to log
			r->request_sid.data = sid.data;
			r->request_sid.len = sid.len;



			access = 0;
			for(i =0; i < ctx->urisidarray->nelts; i++){
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"urisidarray:\"%V\"",&((ngx_str_t*)ctx->urisidarray->elts)[i]);
				if( ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data, uri.data,uri.len) == 0
						&&  ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data + uri.len, sid.data,sid.len)== 0){
					access++;
					break;
				}
			}

			if(access == 0)
				return NGX_HTTP_BAD_REQUEST;

			//catch the 'key' parameter
			if((keystrstart = ngx_strcasestrn(r->uri.data,"&key=",4)) != NULL)
			{
				tokenkey.data = keystrstart+5;

				if((keystrend = ngx_strlchr(keystrstart+1,r->uri_end,'&'))== NULL)
					tokenkey.len = r->uri_end - keystrstart - 5;
				else
					tokenkey.len = keystrend - keystrstart - 5;

//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"key:\"%V\"",&tokenkey);
			}
			else if((keystrstart = ngx_strcasestrn(r->uri.data,"key=",3)) != NULL)
			{
				tokenkey.data = keystrstart+4;

				if((keystrend = ngx_strlchr(keystrstart,r->uri_end,'&'))== NULL)
					tokenkey.len = r->uri_end - keystrstart - 4;
				else
					tokenkey.len = keystrend - keystrstart - 4;
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"key:\"%V\"",&tokenkey);
			}
			else
				return NGX_HTTP_UNAUTHORIZED;

			//store "request_key" var  to log
			r->request_key.data = tokenkey.data;
			r->request_key.len = tokenkey.len;

			access =0;
			for(i =0; i < ctx->keyarray->nelts; i++){
//				ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"tokenkeyarr:\"%V\"",&((ngx_str_t*)ctx->keyarray->elts)[i]);
				if( ngx_strncmp(((ngx_str_t*)ctx->keyarray->elts)[i].data, tokenkey.data,((ngx_str_t*)ctx->keyarray->elts)[i].len) == 0 ){
                    if( ngx_strncmp(((ngx_str_t*)ctx->keyarray->elts)[i].data, tokenkey.data,tokenkey.len) == 0 )
                    {
                        access++;
                        break;
                    }
				}
			}

			if(access == 0)
				return NGX_HTTP_UNAUTHORIZED;
        }
        else
        	return NGX_HTTP_NOT_ALLOWED;

		//tokenkey and uri(sid) is available,but if could not find,the connection is limited.
		hashkey.len = tokenkey.len + sid.len;
		hashkey.data = (text *) ngx_pcalloc(r->pool, hashkey.len);
//		sprintf((char*)hashkey.data, "%s%s", (char*)tokenkey.data, (char*)sid.data); sprinf sometimes make bug!

		ngx_memcpy(ngx_cpymem(hashkey.data,tokenkey.data,tokenkey.len),sid.data,sid.len);
//		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"hashkey:\"%V\"",&hashkey);


     	fm = 0;
     	hash = 0;
        for(i =0; i < ctx->hashkeyarray->nelts; i++){
//            ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"hashkeyarr:\"%V\"",&((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].key);
            if( ngx_strncmp(((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].key.data, hashkey.data,((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].key.len) == 0 ){
        		if( ngx_strncmp(((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].key.data, hashkey.data,hashkey.len) == 0 )
		        {
		            fm = (ngx_int_t)((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].value;
		            hash = ((ngx_hash_key_t*)ctx->hashkeyarray->elts)[i].key_hash;
		            break;
		        }
            }
        }

///	    hash = ngx_hash_key_lc(hashkey.data, hashkey.len);
///       ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"%%hash(key):\"%d\"",hash);
///       fm = (size_t)ngx_hash_find(ctx->keyhash, hash, hashkey.data, hashkey.len);
        if(fm <= 0)
        	return NGX_HTTP_FORBIDDEN;
        else if(fm <= 3600)
        	maxtps = 1 * 1000;
        else
        {
        	maxtps = ((size_t)floor(fm/3600) +1)*1000;
        	maxtps = maxtps < NGX_MAX_TPS_MS ? maxtps : NGX_MAX_TPS_MS;
//        	if(maxtps < 0)//overflow!
 //       		maxtps = NGX_MAX_TPS_MS;
//        	limit->burst *= (size_t)floor(fm/3600);
        }

//        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"maxtps:\"%d\"",maxtps);
//        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,"limit->burst:\"%d\"",limit->burst);

        ngx_shmtx_lock(&ctx->shpool->mutex);

        rc = ngx_http_access_control_lookup(limit, hash, hashkey.data, hashkey.len, &excess,(n == lrcf->limits.nelts - 1),maxtps);

        ngx_shmtx_unlock(&ctx->shpool->mutex);

        ngx_log_debug4(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "limit_req[%ui]: %i %ui.%03ui",
                       n, rc, excess / 1000, excess % 1000);

        if (rc != NGX_AGAIN) {
            break;
        }
    }

    if (rc == NGX_DECLINED) {
        return NGX_DECLINED;
    }

    r->main->access_control_set = 1;

    if (rc == NGX_BUSY || rc == NGX_ERROR) {

        if (rc == NGX_BUSY) {
            ngx_log_error(lrcf->limit_log_level, r->connection->log, 0,
                          "limiting requests, excess: %ui.%03ui by zone \"%V\"",
                          excess / 1000, excess % 1000,
                          &limit->shm_zone->shm.name);
        }

        while (n--) {
            ctx = limits[n].shm_zone->data;

            if (ctx->node == NULL) {
                continue;
            }

            ngx_shmtx_lock(&ctx->shpool->mutex);

            ctx->node->count--;

            ngx_shmtx_unlock(&ctx->shpool->mutex);

            ctx->node = NULL;
        }

        return lrcf->status_code;
    }

    /* rc == NGX_AGAIN || rc == NGX_OK */

    if (rc == NGX_AGAIN) {
        excess = 0;
    }

    delay = ngx_http_access_control_account(limits, n, &excess, &limit, maxtps);

    if (!delay) {
        return NGX_DECLINED;
    }

    ngx_log_error(lrcf->delay_log_level, r->connection->log, 0,
                  "delaying request, excess: %ui.%03ui, by zone \"%V\"",
                  excess / 1000, excess % 1000, &limit->shm_zone->shm.name);

    if (ngx_handle_read_event(r->connection->read, 0) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    r->read_event_handler = ngx_http_test_reading;
    r->write_event_handler = ngx_http_access_control_delay;
    ngx_add_timer(r->connection->write, delay);

    return NGX_AGAIN;
}


static void
ngx_http_access_control_delay(ngx_http_request_t *r)
{
    ngx_event_t  *wev;

    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "access_control delay");

    wev = r->connection->write;

    if (!wev->timedout) {

        if (ngx_handle_write_event(wev, 0) != NGX_OK) {
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        }

        return;
    }

    wev->timedout = 0;

    if (ngx_handle_read_event(r->connection->read, 0) != NGX_OK) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    r->read_event_handler = ngx_http_block_reading;
    r->write_event_handler = ngx_http_core_run_phases;

    ngx_http_core_run_phases(r);
}


static void
ngx_http_access_control_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t          **p;
    ngx_http_access_control_node_t   *lrn, *lrnt;

    for ( ;; ) {

        if (node->key < temp->key) {

            p = &temp->left;

        } else if (node->key > temp->key) {

            p = &temp->right;

        } else { /* node->key == temp->key */

            lrn = (ngx_http_access_control_node_t *) &node->color;
            lrnt = (ngx_http_access_control_node_t *) &temp->color;

            p = (ngx_memn2cmp(lrn->data, lrnt->data, lrn->len, lrnt->len) < 0)
                ? &temp->left : &temp->right;
        }

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}


static ngx_int_t
ngx_http_access_control_lookup(ngx_http_access_control_limit_t *limit, ngx_uint_t hash,
    u_char *data, size_t len, ngx_uint_t *ep, ngx_uint_t account,size_t maxtps)
{
    size_t                      size;
    ngx_int_t                   rc, excess;
    ngx_time_t                 *tp;
    ngx_msec_t                  now;
    ngx_msec_int_t              ms;
    ngx_rbtree_node_t          *node, *sentinel;
    ngx_http_access_control_ctx_t   *ctx;
    ngx_http_access_control_node_t  *lr;

    tp = ngx_timeofday();
    now = (ngx_msec_t) (tp->sec * 1000 + tp->msec);

    ctx = limit->shm_zone->data;

    node = ctx->sh->rbtree.root;
    sentinel = ctx->sh->rbtree.sentinel;

    while (node != sentinel) {

        if (hash < node->key) {
            node = node->left;
            continue;
        }

        if (hash > node->key) {
            node = node->right;
            continue;
        }

        /* hash == node->key */

        lr = (ngx_http_access_control_node_t *) &node->color;

        rc = ngx_memn2cmp(data, lr->data, len, (size_t) lr->len);

        if (rc == 0) {
            ngx_queue_remove(&lr->queue);
            ngx_queue_insert_head(&ctx->sh->queue, &lr->queue);

            ms = (ngx_msec_int_t) (now - lr->last);

            excess = lr->excess - maxtps* ngx_abs(ms) / 1000 + 1000;//zll excess = lr->excess - ctx->rate * ngx_abs(ms) / 1000 + 1000;

            if (excess < 0) {
                excess = 0;
            }

            *ep = excess;

            if ((ngx_uint_t) excess > limit->burst * maxtps) {//zll,if ((ngx_uint_t) excess > limit->burst) {
                return NGX_BUSY;
            }

            if (account) {
                lr->excess = excess;
                lr->last = now;
                return NGX_OK;
            }

            lr->count++;

            ctx->node = lr;

            return NGX_AGAIN;
        }

        node = (rc < 0) ? node->left : node->right;
    }

    *ep = 0;

    size = offsetof(ngx_rbtree_node_t, color)
           + offsetof(ngx_http_access_control_node_t, data)
           + len;

    ngx_http_access_control_expire(ctx, 1, maxtps);

    node = ngx_slab_alloc_locked(ctx->shpool, size);

    if (node == NULL) {
        ngx_http_access_control_expire(ctx, 0, maxtps);

        node = ngx_slab_alloc_locked(ctx->shpool, size);
        if (node == NULL) {
            return NGX_ERROR;
        }
    }

    node->key = hash;

    lr = (ngx_http_access_control_node_t *) &node->color;

    lr->len = (u_char) len;
    lr->excess = 0;

    ngx_memcpy(lr->data, data, len);

    ngx_rbtree_insert(&ctx->sh->rbtree, node);

    ngx_queue_insert_head(&ctx->sh->queue, &lr->queue);

    if (account) {
        lr->last = now;
        lr->count = 0;
        return NGX_OK;
    }

    lr->last = 0;
    lr->count = 1;

    ctx->node = lr;

    return NGX_AGAIN;
}


static ngx_msec_t
ngx_http_access_control_account(ngx_http_access_control_limit_t *limits, ngx_uint_t n,
    ngx_uint_t *ep, ngx_http_access_control_limit_t **limit,size_t maxtps)
{
    ngx_int_t                   excess;
    ngx_time_t                 *tp;
    ngx_msec_t                  now, delay, max_delay;
    ngx_msec_int_t              ms;
    ngx_http_access_control_ctx_t   *ctx;
    ngx_http_access_control_node_t  *lr;

    excess = *ep;

    if (excess == 0 || (*limit)->nodelay) {
        max_delay = 0;

    } else {
        ctx = (*limit)->shm_zone->data;
        max_delay = excess * 1000 / maxtps;//max_delay = excess * 1000 / ctx->rate;
    }

    while (n--) {
        ctx = limits[n].shm_zone->data;
        lr = ctx->node;

        if (lr == NULL) {
            continue;
        }

        ngx_shmtx_lock(&ctx->shpool->mutex);

        tp = ngx_timeofday();

        now = (ngx_msec_t) (tp->sec * 1000 + tp->msec);
        ms = (ngx_msec_int_t) (now - lr->last);

        excess = lr->excess - maxtps * ngx_abs(ms) / 1000 + 1000;//excess = lr->excess - ctx->rate * ngx_abs(ms) / 1000 + 1000;

        if (excess < 0) {
            excess = 0;
        }

        lr->last = now;
        lr->excess = excess;
        lr->count--;

        ngx_shmtx_unlock(&ctx->shpool->mutex);

        ctx->node = NULL;

        if (limits[n].nodelay) {
            continue;
        }

        delay = excess * 1000 / maxtps;//delay = excess * 1000 / ctx->rate;

        if (delay > max_delay) {
            max_delay = delay;
            *ep = excess;
            *limit = &limits[n];
        }
    }

    return max_delay;
}


static void
ngx_http_access_control_expire(ngx_http_access_control_ctx_t *ctx, ngx_uint_t n,size_t maxtps)
{
    ngx_int_t                   excess;
    ngx_time_t                 *tp;
    ngx_msec_t                  now;
    ngx_queue_t                *q;
    ngx_msec_int_t              ms;
    ngx_rbtree_node_t          *node;
    ngx_http_access_control_node_t  *lr;

    tp = ngx_timeofday();

    now = (ngx_msec_t) (tp->sec * 1000 + tp->msec);

    /*
     * n == 1 deletes one or two zero rate entries
     * n == 0 deletes oldest entry by force
     *        and one or two zero rate entries
     */

    while (n < 3) {

        if (ngx_queue_empty(&ctx->sh->queue)) {
            return;
        }

        q = ngx_queue_last(&ctx->sh->queue);

        lr = ngx_queue_data(q, ngx_http_access_control_node_t, queue);

        if (lr->count) {

            /*
             * There is not much sense in looking further,
             * because we bump nodes on the lookup stage.
             */

            return;
        }

        if (n++ != 0) {

            ms = (ngx_msec_int_t) (now - lr->last);
            ms = ngx_abs(ms);

            if (ms < 60000) {
                return;
            }

            excess = lr->excess - maxtps * ms / 1000;//excess = lr->excess - ctx->rate * ms / 1000;

            if (excess > 0) {
                return;
            }
        }

        ngx_queue_remove(q);

        node = (ngx_rbtree_node_t *)
                   ((u_char *) lr - offsetof(ngx_rbtree_node_t, color));

        ngx_rbtree_delete(&ctx->sh->rbtree, node);

        ngx_slab_free_locked(ctx->shpool, node);
    }
}


static ngx_int_t
ngx_http_access_control_init_zone(ngx_shm_zone_t *shm_zone, void *data)
{
    ngx_http_access_control_ctx_t  *octx = data;

    size_t                     len;
    ngx_http_access_control_ctx_t  *ctx;

    ctx = shm_zone->data;

    if (octx) {
        if (ngx_strcmp(ctx->var.data, octx->var.data) != 0) {
            ngx_log_error(NGX_LOG_EMERG, shm_zone->shm.log, 0,
                          "access_control \"%V\" uses the \"%V\" variable "
                          "while previously it used the \"%V\" variable",
                          &shm_zone->shm.name, &ctx->var, &octx->var);
            return NGX_ERROR;
        }
        ctx->sh = octx->sh;
        ctx->shpool = octx->shpool;

        return NGX_OK;
    }

    ctx->shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    if (shm_zone->shm.exists) {
        ctx->sh = ctx->shpool->data;

        return NGX_OK;
    }

    ctx->sh = ngx_slab_alloc(ctx->shpool, sizeof(ngx_http_access_control_shctx_t));
    if (ctx->sh == NULL) {
        return NGX_ERROR;
    }

    ctx->shpool->data = ctx->sh;

    ngx_rbtree_init(&ctx->sh->rbtree, &ctx->sh->sentinel,
                    ngx_http_access_control_rbtree_insert_value);

    ngx_queue_init(&ctx->sh->queue);

    len = sizeof(" in access_control zone \"\"") + shm_zone->shm.name.len;

    ctx->shpool->log_ctx = ngx_slab_alloc(ctx->shpool, len);
    if (ctx->shpool->log_ctx == NULL) {
        return NGX_ERROR;
    }

    ngx_sprintf(ctx->shpool->log_ctx, " in limit_req zone \"%V\"%Z",
                &shm_zone->shm.name);

//    ngx_log_error(NGX_LOG_EMERG, shm_zone->shm.log, 0,"555");

    return NGX_OK;
}


static void *
ngx_http_access_control_create_conf(ngx_conf_t *cf)
{
    ngx_http_access_control_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_access_control_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->limits.elts = NULL;
     */

    conf->limit_log_level = NGX_CONF_UNSET_UINT;
    conf->status_code = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_http_access_control_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_access_control_conf_t *prev = parent;
    ngx_http_access_control_conf_t *conf = child;

    if (conf->limits.elts == NULL) {
        conf->limits = prev->limits;
    }

    ngx_conf_merge_uint_value(conf->limit_log_level, prev->limit_log_level,
                              NGX_LOG_ERR);

    conf->delay_log_level = (conf->limit_log_level == NGX_LOG_INFO) ?
                                NGX_LOG_INFO : conf->limit_log_level + 1;

    ngx_conf_merge_uint_value(conf->status_code, prev->status_code,
                              NGX_HTTP_SERVICE_UNAVAILABLE);

    return NGX_CONF_OK;
}


//static char *
//ngx_http_access_control_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
//{
//    u_char                    *p;
////    size_t                     len;
//    ssize_t                    size;
//    ngx_str_t                 *value, name, s;
//    ngx_int_t                  rate, scale;
//    ngx_uint_t                 i;
//    ngx_shm_zone_t            *shm_zone;
//    ngx_http_access_control_ctx_t  *ctx;
//
//    value = cf->args->elts;
//
//    ctx = NULL;
//    size = 0;
//    rate = 1;
//    scale = 1;
//    name.len = 0;
//
//    for (i = 1; i < cf->args->nelts; i++) {
//
//        if (ngx_strncmp(value[i].data, "zone=", 5) == 0) {
//
//            name.data = value[i].data + 5;
//
//            p = (u_char *) ngx_strchr(name.data, ':');
//
//            if (p == NULL) {
//                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                                   "invalid zone size \"%V\"", &value[i]);
//                return NGX_CONF_ERROR;
//            }
//
//            name.len = p - name.data;
//
//            s.data = p + 1;
//            s.len = value[i].data + value[i].len - s.data;
//
//            size = ngx_parse_size(&s);
//
//            if (size == NGX_ERROR) {
//                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                                   "invalid zone size \"%V\"", &value[i]);
//                return NGX_CONF_ERROR;
//            }
//
//            if (size < (ssize_t) (8 * ngx_pagesize)) {
//                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"zone \"%V\" is too small", &value[i]);
//                return NGX_CONF_ERROR;
//            }
//
//            continue;
//        }
//
//        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                           "invalid parameter \"%V\"", &value[i]);
//        return NGX_CONF_ERROR;
//    }
//
//    ctx = ngx_pcalloc(cf->pool, sizeof(ngx_http_access_control_ctx_t));
//    if (ctx == NULL) {
//        return NGX_CONF_ERROR;
//    }
//
//	if(ngx_http_limit_init_keypool(cf,ctx)!= NGX_OK){
//		 return NGX_CONF_ERROR;
//	}
//
//    if (name.len == 0) {
//        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                           "\"%V\" must have \"zone\" parameter",
//                           &cmd->name);
//        return NGX_CONF_ERROR;
//    }
//
//    if (ctx == NULL) {
//        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                           "no variable is defined for %V \"%V\"",
//                           &cmd->name, &name);
//        return NGX_CONF_ERROR;
//    }
//
////    ctx->rate = rate * 1000 / scale;
//
//    shm_zone = ngx_shared_memory_add(cf, &name, size, &ngx_http_access_control_module);
//
//
//    if (shm_zone == NULL) {
//        return NGX_CONF_ERROR;
//    }
//
//    if (shm_zone->data) {
//        ctx = shm_zone->data;
//
//        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
//                           "%V \"%V\" is already bound to variable \"%V\"",
//                           &cmd->name, &name, &ctx->var);
//        return NGX_CONF_ERROR;
//    }
//
//    shm_zone->init = ngx_http_access_control_init_zone;
//    shm_zone->data = ctx;
//
//    return NGX_CONF_OK;
//}

//void ngx_http_access_control_body_handler(ngx_http_request_t *r)
//{
////	r->main->read_request_set = 1;
////	ngx_http_access_control_handler(r);
//	return;
//}

static ngx_str_t  ngx_http_ctx_var = ngx_string("key");

static char *
ngx_http_access_control(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_access_control_conf_t  *lrcf = conf;

    u_char                    *p;
    ssize_t                    size;
    ngx_int_t                    burst;
    ngx_str_t                   *value, s,name;
    ngx_uint_t                   i, nodelay;
    ngx_shm_zone_t              *shm_zone;
    ngx_http_access_control_limit_t  *limit, *limits;
    ngx_http_access_control_ctx_t  *ctx;


    ctx = ngx_pcalloc(cf->pool, sizeof(ngx_http_access_control_ctx_t));
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }

	ctx->var = ngx_http_ctx_var;

//    shm_zone = ngx_shared_memory_add(cf, "ACZONE", 10*1024*1024, &ngx_http_access_control_module);//shared_memory size is default 10m.
//    if (shm_zone == NULL) {
//        return NGX_CONF_ERROR;
//    }
//    shm_zone->init = ngx_http_access_control_init_zone;
//    shm_zone->data = ctx;

    if(ngx_http_limit_init_keypool(cf,ctx)!= NGX_OK){
         return NGX_CONF_ERROR;
    }


    value = cf->args->elts;

    shm_zone = NULL;
    burst = 0;
    nodelay = 0;

    for (i = 1; i < cf->args->nelts; i++) {

        if (ngx_strncmp(value[i].data, "aczone=", 7) == 0) {
        	name.data = value[i].data + 7;
            p = (u_char *) ngx_strchr(name.data, ':');
            if (p == NULL) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid zone size \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }
            name.len = p - name.data;

            s.data = p + 1;
            s.len = value[i].data + value[i].len - s.data;

            size = ngx_parse_size(&s);
            if (size == NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid zone size \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            if (size < (ssize_t) (8 * ngx_pagesize)) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"zone \"%V\" is too small", &value[i]);
                return NGX_CONF_ERROR;
            }

            shm_zone = ngx_shared_memory_add(cf, &name, size,&ngx_http_access_control_module);

            if (shm_zone == NULL) {
                return NGX_CONF_ERROR;
            }

            if (shm_zone->data) {
                ctx = shm_zone->data;

                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "%V \"%V\" is already bound to variable \"%V\"",
                                   &cmd->name, &name, &ctx->var);
                return NGX_CONF_ERROR;
            }
            shm_zone->init = ngx_http_access_control_init_zone;
            shm_zone->data = ctx;

            continue;
        }

        if (ngx_strncmp(value[i].data, "burst=", 6) == 0) {

            burst = ngx_atoi(value[i].data + 6, value[i].len - 6);
            if (burst <= 0) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid burst rate \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "nodelay", 7) == 0) {
            nodelay = 1;
            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\"", &value[i]);
        return NGX_CONF_ERROR;
    }

    if (shm_zone == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "\"%V\" must have \"zone\" parameter",
                           &cmd->name);
        return NGX_CONF_ERROR;
    }

    if (shm_zone->data == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "unknown access_control_zone \"%V\"",
                           &shm_zone->shm.name);
        return NGX_CONF_ERROR;
    }

    limits = lrcf->limits.elts;

    if (limits == NULL) {
        if (ngx_array_init(&lrcf->limits, cf->pool, 1,
                           sizeof(ngx_http_access_control_limit_t))
            != NGX_OK)
        {
            return NGX_CONF_ERROR;
        }
    }

    for (i = 0; i < lrcf->limits.nelts; i++) {
        if (shm_zone == limits[i].shm_zone) {
            return "is duplicate";
        }
    }

    limit = ngx_array_push(&lrcf->limits);
    if (limit == NULL) {
        return NGX_CONF_ERROR;
    }

    limit->shm_zone = shm_zone;
    limit->burst = burst < NGX_MAX_BURST_MULTIPLE ? burst : NGX_MAX_BURST_MULTIPLE;
    limit->nodelay = nodelay;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_access_control_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_access_control_handler;

    return NGX_OK;
}

static ngx_str_t ngx_http_access_control_sql =ngx_string("SELECT A.KEY,S.URI,A.MAXTPH,A.SID FROM FMAP_ACCESSCONTROL_T A,FMAP_SERVICE_T S WHERE  A.ISAPPROVED =1 AND A.SID = S.SID");
static ngx_str_t ngx_http_access_control_db = ngx_string("orcl");
static ngx_str_t ngx_http_access_control_un = ngx_string("sde");
static ngx_str_t ngx_http_access_control_pw = ngx_string("sde");
static OCIEnv     *envhp = (OCIEnv *)0;
static OCIError   *errhp = (OCIError *)0;
static OCISession *authp = (OCISession *)0;
static OCIServer  *srvhp = (OCIServer *)0;
static OCISvcCtx  *svchp = (OCISvcCtx *)0;
static OCIStmt    *stmthp = (OCIStmt *)0;

static char *
ngx_http_access_control_dbconf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t                  *value;
    value= cf->args->elts;

    if(cf->args->nelts < 4){
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"parameter is missing ,count of parameter must greater than or equal 4");
        return NGX_CONF_ERROR;
    }
    else if(cf->args->nelts > 5){
      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"invalid parameter \"%V\",count of parameter less than or equal 5",&value[2]);
     return NGX_CONF_ERROR;
    }

    ngx_http_access_control_db.data = value[1].data;
    ngx_http_access_control_db.len = value[1].len;
    ngx_http_access_control_un.data = value[2].data;
    ngx_http_access_control_un.len = value[2].len;
    ngx_http_access_control_pw.data = value[3].data;
    ngx_http_access_control_pw.len = value[3].len;

    if (cf->args->nelts == 5)
    {
        ngx_http_access_control_sql.data = value[4].data;
        ngx_http_access_control_sql.len =  value[4].len;
    }



    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_limit_init_keypool(ngx_conf_t *cf,ngx_http_access_control_ctx_t  *ctx)
{
//    ngx_hash_init_t      hash_init;
//  ngx_hash_t*          hash;
//  ngx_array_t*         hashkeyarray;
    ngx_hash_key_t*      arr_node;
    ngx_str_t*           keyarr_node;
    ngx_str_t*           urisidarr_node;
    ngx_hash_keylimit_t* keylimit;
    ngx_hash_keylimit_t* keylimittemp;
    size_t               i,k=0;
    //  ngx_str_t            urisid;

    sword swResult = 0;
    sword errcode = 0;
    OCIDefine *hDefine1 =  (OCIDefine *) 0;
    OCIDefine *hDefine2 = (OCIDefine *) 0;
    OCIDefine *hDefine3 = (OCIDefine *) 0;
    OCIDefine *hDefine4 = (OCIDefine *) 0;

    errcode = OCIEnvCreate((OCIEnv **)&envhp, (ub4) OCI_DEFAULT,
            (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
            (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
            (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0);
    if (errcode != 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Create OCIEnv Failed!");
        return NGX_ERROR;
    }

    //ngx_log_debug0(NGX_LOG_DEBUG_CORE, cf->log, 0, "Create OCIEnv Success!");
    //ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0,"Funomap project create OCIEnv success!");
    //dbconn ++;

    checkerr(errhp,OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0),cf->log);
    OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,(size_t) 0, (dvoid **) 0);
    OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,(size_t) 0, (dvoid **) 0);
    checkerr(errhp,OCIServerAttach( srvhp, errhp, (text *)ngx_http_access_control_db.data,ngx_http_access_control_db.len, 0),cf->log);
    OCIAttrSet( (dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *)srvhp,(ub4) 0, OCI_ATTR_SERVER, (OCIError *) errhp);
    OCIHandleAlloc((dvoid *) envhp, (dvoid **)&authp,(ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);
    OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,(dvoid *) ngx_http_access_control_un.data, (ub4) ngx_http_access_control_un.len,(ub4) OCI_ATTR_USERNAME, errhp);
    OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,(dvoid *) ngx_http_access_control_pw.data, (ub4) ngx_http_access_control_un.len,(ub4) OCI_ATTR_PASSWORD, errhp);
    checkerr(errhp,OCISessionBegin ( svchp,  errhp, authp, OCI_CRED_RDBMS,(ub4) OCI_DEFAULT),cf->log);
    OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,(dvoid *) authp, (ub4) 0,(ub4) OCI_ATTR_SESSION, errhp);
    if(!envhp || !svchp || !srvhp)
    {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"connect db fail in access_control module !");
        return NGX_ERROR;
    }


    OCIHandleAlloc( envhp, (dvoid**)&stmthp,OCI_HTYPE_STMT, 0, 0);
    checkerr(errhp, OCIStmtPrepare(stmthp, errhp,(const OraText*) ngx_http_access_control_sql.data,ngx_http_access_control_sql.len, OCI_NTV_SYNTAX, OCI_DEFAULT),cf->log);

    keylimittemp = (ngx_hash_keylimit_t*) ngx_pcalloc(cf->pool, sizeof(ngx_hash_keylimit_t));
    keylimittemp->key.data = (text *) ngx_pcalloc(cf->pool, 33);
    keylimittemp->key.len = 33;
    keylimittemp->uri.data = (text *) ngx_pcalloc(cf->pool, 65);
    keylimittemp->uri.len = 65;
    keylimittemp->sid.data = (text *) ngx_pcalloc(cf->pool, 7);
    keylimittemp->sid.len = 7;

    OCIDefineByPos(stmthp,&hDefine1,errhp,1,keylimittemp->key.data,keylimittemp->key.len, SQLT_CHR, NULL, NULL, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp,&hDefine2,errhp,2,keylimittemp->uri.data,keylimittemp->uri.len, SQLT_CHR, NULL, NULL, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp,&hDefine3,errhp,3,&keylimittemp->maxtps,sizeof(keylimittemp->maxtps), SQLT_INT, NULL, NULL, NULL, OCI_DEFAULT);
    OCIDefineByPos(stmthp,&hDefine4,errhp,4,keylimittemp->sid.data,keylimittemp->sid.len, SQLT_CHR, NULL, NULL, NULL, OCI_DEFAULT);
    checkerr(errhp,OCIStmtExecute( svchp, stmthp, errhp, (ub4) 1, (ub4) 0,(OCISnapshot *) NULL,(OCISnapshot *) NULL, (ub4)OCI_STMT_SCROLLABLE_READONLY ),cf->log);
//  swResult = OCIStmtExecute( svchp, stmthp, errhp, (ub4) 1, (ub4) 0,(OCISnapshot *) NULL,(OCISnapshot *) NULL, (ub4)OCI_STMT_SCROLLABLE_READONLY );
//

//      ctx->keyhashpool = ngx_create_pool(1024*100, cf->log);
//      ctx->keyhash = (ngx_hash_t*) ngx_pcalloc(ctx->keyhashpool, sizeof(ctx->keyhash));
//      hash_init.hash                = ctx->keyhash;                                     // hash结构
//      hash_init.key                 = &ngx_hash_key_lc;                                 // hash算法函数
//      hash_init.max_size            = 1024*10;                                         // max_size
//      hash_init.bucket_size         = 64;                                               // ngx_align(64, ngx_cacheline_size);
//      hash_init.name                = "RestRequestLimit_DataPool";                      // 在log里会用到
//      hash_init.pool                = ctx->keyhashpool;
//      hash_init.temp_pool           = NULL;
//    ctx->keyhash = (ngx_hash_t*) ngx_pcalloc(cf->pool, sizeof(ctx->keyhash));
//    hash_init.hash                = ctx->keyhash;                                     // hash结构
//    hash_init.key                 = &ngx_hash_key_lc;                                 // hash算法函数
//    hash_init.max_size            = 1024*10;                                         // max_size
//    hash_init.bucket_size         = 1024*100;                                         // this param make errs in Linux if too low;
//    hash_init.name                = "RestRequestLimit_DataPool";                      // 在log里会用到
//    hash_init.pool                = cf->pool;
//    hash_init.temp_pool           = NULL;

    ctx->hashkeyarray = NULL;//key+sid,Raw data from oci could not be duplicate.
    ctx->keyarray = NULL;//key,Raw data from oci is duplicate.
    ctx->urisidarray = NULL;//uri+sid,Raw data from oci is duplicate.
    ctx->hashkeyarray = ngx_array_create(cf->pool, 200, sizeof(ngx_hash_key_t));
    ctx->keyarray = ngx_array_create(cf->pool, 50, sizeof(ngx_str_t));
    ctx->urisidarray = ngx_array_create(cf->pool, 200, sizeof(ngx_str_t));
    if(!ctx->hashkeyarray || !ctx->keyarray || !ctx->urisidarray)
    {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Creat hashkeyarray or keyarray or urisidarray Failed!");
        return NGX_ERROR;
    }

//    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0,"Creat hashkeyarray or keyarray or urisidarray success!");

    while (swResult == OCI_SUCCESS || swResult == OCI_SUCCESS_WITH_INFO ){

        keylimit = (ngx_hash_keylimit_t*) ngx_pcalloc(cf->pool, sizeof(ngx_hash_keylimit_t));
        for(i=0;i<33;i++){
        if((keylimittemp->key.data)[i]==' '){
            break;///the key which ended by ' ' is not limited in 32 chars.
            }
        }
        keylimit->key.data = (text *) ngx_pcalloc(cf->pool, i);
        keylimit->key.len = i;
        ngx_memcpy(keylimit->key.data,keylimittemp->key.data,i);

        for(i=0;i<65;i++){
        if((keylimittemp->uri.data)[i]==' '){
            break;
            }
        }
        keylimit->uri.data = (text *) ngx_pcalloc(cf->pool, i);
        keylimit->uri.len = i;
        ngx_memcpy(keylimit->uri.data,keylimittemp->uri.data,i);

        for(i=0;i<7;i++){
        if((keylimittemp->sid.data)[i]==' '){
            break;
            }
        }

        keylimit->sid.data = (text *) ngx_pcalloc(cf->pool, i);
        keylimit->sid.len = i;
        ngx_memcpy(keylimit->sid.data,keylimittemp->sid.data,i);

        keylimit->maxtps = keylimittemp->maxtps;

        k = 0;
        for(i =0; i < ctx->keyarray->nelts; i++){
            if( ngx_strncmp(((ngx_str_t*)ctx->keyarray->elts)[i].data, keylimit->key.data,((ngx_str_t*)ctx->keyarray->elts)[i].len) == 0 ){
                k ++;
                break;
            }
        }
        if(k ==0 ){
            keyarr_node = (ngx_str_t*) ngx_array_push(ctx->keyarray);
            keyarr_node->data = keylimit->key.data;
            keyarr_node->len = keylimit->key.len;
//      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"new verifykey:\"%V\"", keyarr_node);
        }

        k = 0;
        for(i =0; i < ctx->urisidarray->nelts; i++)
        {
            if( ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data,keylimit->uri.data,keylimit->uri.len) == 0
                    &&ngx_strncmp(((ngx_str_t*)ctx->urisidarray->elts)[i].data+keylimit->uri.len,keylimit->sid.data,keylimit->sid.len) == 0){
                k++;
                break;
            }
        }
        if(k == 0){
            urisidarr_node = (ngx_str_t*) ngx_array_push(ctx->urisidarray);
            urisidarr_node->data = (text *) ngx_pcalloc(cf->pool, keylimit->uri.len + keylimit->sid.len);
//      sprintf((char*)urisidarr_node->data, "%s%s", (char*)keylimit->uri.data, (char*)keylimit->sid.data);
            ngx_memcpy(ngx_cpymem(urisidarr_node->data,keylimit->uri.data,keylimit->uri.len),keylimit->sid.data,keylimit->sid.len);
            urisidarr_node->len = keylimit->uri.len + keylimit->sid.len;
//      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"urisid:\"%V\"",urisidarr_node);
        }

        arr_node = (ngx_hash_key_t*) ngx_array_push(ctx->hashkeyarray);
        arr_node->key.data = (text *) ngx_pcalloc(cf->pool, keylimit->key.len + keylimit->sid.len);
//      sprintf((char*)arr_node->key.data, "%s%s", (char*)keylimit->key.data, (char*)keylimit->sid.data);
        ngx_memcpy(ngx_cpymem(arr_node->key.data,keylimit->key.data,keylimit->key.len),keylimit->sid.data,keylimit->sid.len);
        arr_node->key.len =keylimit->key.len + keylimit->sid.len;


//      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"hash's key:\"%V\"",&arr_node->key);
        arr_node->key_hash = ngx_hash_key_lc(arr_node->key.data,arr_node->key.len);//(arr_node->key.data, arr_node->key.len);ngx_hash_key_lc
        arr_node->value = (void*) keylimit->maxtps;
//      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"key_hash:\"%d\"",arr_node->key_hash);

        ngx_memzero(keylimittemp->key.data, 33);
        ngx_memzero(keylimittemp->uri.data, 65);
        ngx_memzero(keylimittemp->sid.data, 7);
        swResult=OCIStmtFetch2(stmthp,errhp,1,OCI_FETCH_NEXT,1,OCI_DEFAULT);
     };//swResult!=OCI_NO_DATA || swResult);

//    if (ngx_hash_init(&hash_init, (ngx_hash_key_t*) ctx->hashkeyarray->elts, ctx->hashkeyarray->nelts)!=NGX_OK){
//        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Hash KeyPool Init Failed!");
//        return NGX_ERROR;
//    }

//    ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0,"Fumomap Accesscontrol module maybe success initialize !");

    if(envhp)
    {
        checkerr(errhp,OCISessionEnd(svchp,errhp,authp,OCI_DEFAULT),cf->cycle->log);
        checkerr(errhp,OCIServerDetach(srvhp,errhp,OCI_DEFAULT),cf->cycle->log);
        checkerr(errhp,OCIHandleFree(envhp, OCI_HTYPE_ENV),cf->cycle->log);
        envhp = NULL;

    }
    
    return NGX_OK;

}
static void checkerr(OCIError *errhp, sword status,ngx_log_t *log)
{
  text errbuf[512];
  sb4 errcode = 0;

  switch (status)
  {
  case OCI_SUCCESS:
    break;
  case OCI_SUCCESS_WITH_INFO:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_SUCCESS_WITH_INFO\n");
    break;
  case OCI_NEED_DATA:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_NEED_DATA\n");
    break;
  case OCI_NO_DATA:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_NODATA\n");
    break;
  case OCI_ERROR:
    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
                        errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - %.*s\n", 512, errbuf);
    break;
  case OCI_INVALID_HANDLE:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_INVALID_HANDLE\n");
    break;
  case OCI_STILL_EXECUTING:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_STILL_EXECUTE\n");
    break;
  case OCI_CONTINUE:
    ngx_log_error_core(NGX_LOG_ERR, log, 0,"Error - OCI_CONTINUE\n");
    break;
  default:
    break;
  }
}

void cleanOCIEnv(ngx_cycle_t *cycle)
{
//  ngx_log_debug0(NGX_LOG_DEBUG_CORE, cycle->log, 0, "OCIEnv clean;");
    ngx_http_access_control_ctx_t  *ctx;

//  if(cycle->dbconn && cycle->envhp)
//  {
//      checkerr(cycle->errhp,OCISessionEnd(cycle->svchp,cycle->errhp,cycle->authp,OCI_DEFAULT),cycle->log);
//      checkerr(cycle->errhp,OCIServerDetach(cycle->srvhp,cycle->errhp,OCI_DEFAULT),cycle->log);
//      checkerr(cycle->errhp,OCIHandleFree( cycle->envhp, OCI_HTYPE_ENV),cycle->log);
//
//      cycle->dbconn--;
//      cycle->envhp = NULL;
//
//  }

    ctx = (ngx_http_access_control_ctx_t *)ngx_get_conf(cycle->conf_ctx,ngx_http_access_control_module);
    if(ctx ==  NULL)
        return;
    if(ctx->hashkeyarray)
        ngx_array_destroy(ctx->hashkeyarray);
    if(ctx->keyarray)
        ngx_array_destroy(ctx->keyarray);
    if(ctx->urisidarray)
        ngx_array_destroy(ctx->urisidarray);
//  if(ctx->keyhashpool)
//      ngx_destroy_pool(ctx->keyhashpool);

    return;

}
