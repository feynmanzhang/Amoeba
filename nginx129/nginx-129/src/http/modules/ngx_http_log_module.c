
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#if (NGX_ZLIB)
#include <zlib.h>
#endif

#include <oci.h>

typedef struct ngx_http_log_op_s  ngx_http_log_op_t;

typedef u_char *(*ngx_http_log_op_run_pt) (ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);

typedef size_t (*ngx_http_log_op_getlen_pt) (ngx_http_request_t *r,
    uintptr_t data);


struct ngx_http_log_op_s {
    size_t                      len;
    ngx_http_log_op_getlen_pt   getlen;
    ngx_http_log_op_run_pt      run;
    uintptr_t                   data;
};


typedef struct {
    ngx_str_t                   name;
    ngx_array_t                *flushes;
    ngx_array_t                *ops;        /* array of ngx_http_log_op_t */
} ngx_http_log_fmt_t;


typedef struct {
    ngx_array_t                 formats;    /* array of ngx_http_log_fmt_t */
    ngx_array_t                 dblog_formats;///zll array of ngx_http_log_fmt_t */
    ngx_uint_t                  combined_used; /* unsigned  combined_used:1 */
    ngx_uint_t                  dblog_combined_used;///zll unsigned  combined_used:1 */
} ngx_http_log_main_conf_t;


typedef struct {
    u_char                     *start;
    u_char                     *pos;
    u_char                     *last;

    ngx_event_t                *event;
    ngx_msec_t                  flush;
    ngx_int_t                   gzip;
} ngx_http_log_buf_t;


typedef struct {
    ngx_array_t                *lengths;
    ngx_array_t                *values;
} ngx_http_log_script_t;


typedef struct {
    ngx_open_file_t            *file;
    ngx_http_log_script_t      *script;
    time_t                      disk_full_time;
    time_t                      error_log_time;
    ngx_http_log_fmt_t         *format;
} ngx_http_log_t;


typedef struct{
    ngx_http_log_fmt_t         *format;
}ngx_http_dblog_t;

typedef struct {
    ngx_array_t                *logs;       /* array of ngx_http_log_t */
    ngx_array_t                *dblogs;    ///zll for db logs

    ngx_open_file_cache_t      *open_file_cache;
    time_t                      open_file_cache_valid;
    ngx_uint_t                  open_file_cache_min_uses;

    ngx_uint_t                  off;        /* unsigned  off:1 */
    ngx_uint_t                  dblogoff;   /* unsigned  off:1 */
} ngx_http_log_loc_conf_t;




typedef struct {
    ngx_str_t                   name;
    size_t                      len;
    ngx_http_log_op_run_pt      run;
} ngx_http_log_var_t;


static void ngx_http_log_write(ngx_http_request_t *r, ngx_http_log_t *log,
    u_char *buf, size_t len);
static ssize_t ngx_http_log_script_write(ngx_http_request_t *r,
    ngx_http_log_script_t *script, u_char **name, u_char *buf, size_t len);

#if (NGX_ZLIB)
static ssize_t ngx_http_log_gzip(ngx_fd_t fd, u_char *buf, size_t len,
    ngx_int_t level, ngx_log_t *log);

static void *ngx_http_log_gzip_alloc(void *opaque, u_int items, u_int size);
static void ngx_http_log_gzip_free(void *opaque, void *address);
#endif

static void ngx_http_log_flush(ngx_open_file_t *file, ngx_log_t *log);
static void ngx_http_log_flush_handler(ngx_event_t *ev);

static u_char *ngx_http_log_pipe(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_time(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_iso8601(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_msec(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_request_time(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_status(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_bytes_sent(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_body_bytes_sent(ngx_http_request_t *r,
    u_char *buf, ngx_http_log_op_t *op);
static u_char *ngx_http_log_request_length(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_request_sid(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static u_char *ngx_http_log_request_key(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);

static ngx_int_t ngx_http_log_variable_compile(ngx_conf_t *cf,
    ngx_http_log_op_t *op, ngx_str_t *value);
static size_t ngx_http_log_variable_getlen(ngx_http_request_t *r,
    uintptr_t data);
static u_char *ngx_http_log_variable(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op);
static uintptr_t ngx_http_log_escape(u_char *dst, u_char *src, size_t size);


static void *ngx_http_log_create_main_conf(ngx_conf_t *cf);
static void *ngx_http_log_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_log_merge_loc_conf(ngx_conf_t *cf, void *parent,
    void *child);

static char *ngx_http_log_set_dblog(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_http_log_set_dblog_format(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

static char *ngx_http_log_set_log(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_http_log_set_format(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_http_log_compile_format(ngx_conf_t *cf,
    ngx_array_t *flushes, ngx_array_t *ops, ngx_array_t *args, ngx_uint_t s);
static char *ngx_http_log_open_file_cache(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static ngx_int_t ngx_http_log_init(ngx_conf_t *cf);

static void ngx_write_dblog(ngx_http_request_t *r, ngx_log_t *log, u_char *buf,size_t len);
static void checkerr(OCIError *errhp, sword status,ngx_log_t *log);
//static void cleanup();
static void cleanOCIEnv(ngx_cycle_t *cycle);

static ngx_command_t  ngx_http_log_commands[] = {

    //zll
    { ngx_string("dblog_format"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_2MORE,
      ngx_http_log_set_dblog_format,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },
    { ngx_string("access_dblog"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
                        |NGX_HTTP_LMT_CONF|NGX_CONF_1MORE,
      ngx_http_log_set_dblog,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("log_format"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_2MORE,
      ngx_http_log_set_format,
      NGX_HTTP_MAIN_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("access_log"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
                        |NGX_HTTP_LMT_CONF|NGX_CONF_1MORE,
      ngx_http_log_set_log,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("open_log_file_cache"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1234,
      ngx_http_log_open_file_cache,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_log_module_ctx = {
    NULL,                                  /* preconfiguration */
    ngx_http_log_init,                     /* postconfiguration */

    ngx_http_log_create_main_conf,         /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_log_create_loc_conf,          /* create location configuration */
    ngx_http_log_merge_loc_conf            /* merge location configuration */
};


ngx_module_t  ngx_http_log_module = {
    NGX_MODULE_V1,
    &ngx_http_log_module_ctx,              /* module context */
    ngx_http_log_commands,                 /* module directives */
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


static ngx_str_t  ngx_http_access_log = ngx_string(NGX_HTTP_LOG_PATH);
static ngx_str_t  ngx_http_combined_fmt =
    ngx_string("$remote_addr - $remote_user [$time_local] "
               "\"$request\" $status $body_bytes_sent "
               "\"$http_referer\" \"$http_user_agent\" \"$http_x_forwarded_for\"");

//static ngx_str_t  ngx_http_combined_sql_test =
//      ngx_string("INSERT INTO fmap_accesslog_t(remoteaddr, timestamp, uri, param, httpstatus, useragent) VALUES('127.0.0.2', null,null, null, 200, 'firefox')");

//static ngx_str_t  ngx_http_combined_sql =
//    ngx_string("INSERT INTO access_log_t(remote_addr, time_local, request, status, user_agent)"
//               "VALUES('$remote_addr', TO_TIMESTAMP_TZ('$time_local', 'dd/mon/yyyy:hh24:mi:ss tzhtzm', 'NLS_DATE_LANGUAGE = AMERICAN'), '$request', $status, '$http_user_agent')");

static ngx_str_t  ngx_http_combined_sql =
    ngx_string("INSERT INTO fmap_accesslog_t(remoteaddr, timestamp, uri, sid, httpstatus, key,costtime)"
            " VALUES('$remote_addr',TO_TIMESTAMP_TZ('$time_local', 'dd/mon/yyyy:hh24:mi:ss tzhtzm', 'NLS_DATE_LANGUAGE = AMERICAN'), '$uri', '$request_sid', $status , '$request_key','$request_time')");


static ngx_http_log_var_t  ngx_http_log_vars[] = {
    { ngx_string("pipe"), 1, ngx_http_log_pipe },
    { ngx_string("time_local"), sizeof("28/Sep/1970:12:00:00 +0600") - 1,
                          ngx_http_log_time },
    { ngx_string("time_iso8601"), sizeof("1970-09-28T12:00:00+06:00") - 1,
                          ngx_http_log_iso8601 },
    { ngx_string("msec"), NGX_TIME_T_LEN + 4, ngx_http_log_msec },
    { ngx_string("request_time"), NGX_TIME_T_LEN + 4,
                          ngx_http_log_request_time },
    { ngx_string("status"), NGX_INT_T_LEN, ngx_http_log_status },
    { ngx_string("bytes_sent"), NGX_OFF_T_LEN, ngx_http_log_bytes_sent },
    { ngx_string("body_bytes_sent"), NGX_OFF_T_LEN,
                          ngx_http_log_body_bytes_sent },
    { ngx_string("request_length"), NGX_SIZE_T_LEN,
                          ngx_http_log_request_length },
    { ngx_string("request_sid"),6,
                          ngx_http_log_request_sid},
    { ngx_string("request_key"),32,
                          ngx_http_log_request_key},

    { ngx_null_string, 0, NULL }
};


static ngx_int_t
ngx_http_log_handler(ngx_http_request_t *r)
{
    u_char                   *line, *p;
    size_t                    len;
    ngx_uint_t                i, l;
    ngx_http_log_t           *log;
    ngx_http_dblog_t         *dblog;
    ngx_http_log_op_t        *op;
    ngx_http_log_buf_t       *buffer;
    ngx_http_log_loc_conf_t  *lcf;


    lcf = ngx_http_get_module_loc_conf(r, ngx_http_log_module);

    if (lcf->off&&lcf->dblogoff) {
        return NGX_OK;
    }

    if(!lcf->off)
    {
        log = lcf->logs->elts;
        for (l = 0; l < lcf->logs->nelts; l++) {

            if (ngx_time() == log[l].disk_full_time) {

                /*
                 * on FreeBSD writing to a full filesystem with enabled softupdates
                 * may block process for much longer time than writing to non-full
                 * filesystem, so we skip writing to a log for one second
                 */

                continue;
            }

            ngx_http_script_flush_no_cacheable_variables(r, log[l].format->flushes);

            len = 0;
            op = log[l].format->ops->elts;
            for (i = 0; i < log[l].format->ops->nelts; i++) {
                if (op[i].len == 0) {
                    len += op[i].getlen(r, op[i].data);

                } else {
                    len += op[i].len;
                }
            }

            len += NGX_LINEFEED_SIZE;

            buffer = log[l].file ? log[l].file->data : NULL;

            if (buffer) {

                if (len > (size_t) (buffer->last - buffer->pos)) {
                    ngx_http_log_write(r, &log[l], buffer->start,
                                       buffer->pos - buffer->start);
    //              if(!lcf->dblogoff)
    //                ngx_write_dblog(r, &log[l], buffer->start,
    //                                   buffer->pos - buffer->start);

                    buffer->pos = buffer->start;
                }

                if (len <= (size_t) (buffer->last - buffer->pos)) {

                    p = buffer->pos;

                    if (buffer->event && p == buffer->start) {
                        ngx_add_timer(buffer->event, buffer->flush);
                    }

                    for (i = 0; i < log[l].format->ops->nelts; i++) {
                        p = op[i].run(r, p, &op[i]);
                    }

                    ngx_linefeed(p);

                    buffer->pos = p;

                    continue;
                }

                if (buffer->event && buffer->event->timer_set) {
                    ngx_del_timer(buffer->event);
                }
            }

            line = ngx_pnalloc(r->pool, len);
            if (line == NULL) {
                return NGX_ERROR;
            }

            p = line;

            for (i = 0; i < log[l].format->ops->nelts; i++) {
                p = op[i].run(r, p, &op[i]);
            }

            ngx_linefeed(p);

            ngx_http_log_write(r, &log[l], line, p - line);
        }
    }


    if(!lcf->dblogoff)
    {
        dblog = lcf->dblogs->elts;
        for (l = 0; l < lcf->dblogs->nelts; l++) {
            len = 0;
            op = dblog[l].format->ops->elts;
            for (i = 0; i < dblog[l].format->ops->nelts; i++) {
                if (op[i].len == 0) {
                    len += op[i].getlen(r, op[i].data);

                } else {
                    len += op[i].len;
                }
            }

            len += NGX_LINEFEED_SIZE;

            line = ngx_pnalloc(r->pool, len);
            if (line == NULL) {
                return NGX_ERROR;
            }
            //ngx_log_error(NGX_LOG_NOTICE, r->connection->log, 0,"r->request_sid:\"%V\"",&r->request_sid);
            p = line;

            for (i = 0; i < dblog[l].format->ops->nelts; i++) {
                p = op[i].run(r, p, &op[i]);
            }

            ngx_linefeed(p);

            ngx_write_dblog(r, r->connection->log, line, p - line);
        }
    }


    return NGX_OK;
}


static void
ngx_http_log_write(ngx_http_request_t *r, ngx_http_log_t *log, u_char *buf,
    size_t len)
{
    u_char              *name;
    time_t               now;
    ssize_t              n;
    ngx_err_t            err;
#if (NGX_ZLIB)
    ngx_http_log_buf_t  *buffer;
#endif

    if (log->script == NULL) {
        name = log->file->name.data;

#if (NGX_ZLIB)
        buffer = log->file->data;

        if (buffer && buffer->gzip) {
            n = ngx_http_log_gzip(log->file->fd, buf, len, buffer->gzip,
                                  r->connection->log);
        } else {
            n = ngx_write_fd(log->file->fd, buf, len);
        }
#else
        n = ngx_write_fd(log->file->fd, buf, len);
#endif

    } else {
        name = NULL;
        n = ngx_http_log_script_write(r, log->script, &name, buf, len);
    }

    if (n == (ssize_t) len) {
        return;
    }

    now = ngx_time();

    if (n == -1) {
        err = ngx_errno;

        if (err == NGX_ENOSPC) {
            log->disk_full_time = now;
        }

        if (now - log->error_log_time > 59) {
            ngx_log_error(NGX_LOG_ALERT, r->connection->log, err,
                          ngx_write_fd_n " to \"%s\" failed", name);

            log->error_log_time = now;
        }

        return;
    }

    if (now - log->error_log_time > 59) {
        ngx_log_error(NGX_LOG_ALERT, r->connection->log, 0,
                      ngx_write_fd_n " to \"%s\" was incomplete: %z of %uz",
                      name, n, len);

        log->error_log_time = now;
    }
}


static ssize_t
ngx_http_log_script_write(ngx_http_request_t *r, ngx_http_log_script_t *script,
    u_char **name, u_char *buf, size_t len)
{
    size_t                     root;
    ssize_t                    n;
    ngx_str_t                  log, path;
    ngx_open_file_info_t       of;
    ngx_http_log_loc_conf_t   *llcf;
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    if (!r->root_tested) {

        /* test root directory existence */

        if (ngx_http_map_uri_to_path(r, &path, &root, 0) == NULL) {
            /* simulate successful logging */
            return len;
        }

        path.data[root] = '\0';

        ngx_memzero(&of, sizeof(ngx_open_file_info_t));

        of.valid = clcf->open_file_cache_valid;
        of.min_uses = clcf->open_file_cache_min_uses;
        of.test_dir = 1;
        of.test_only = 1;
        of.errors = clcf->open_file_cache_errors;
        of.events = clcf->open_file_cache_events;

        if (ngx_http_set_disable_symlinks(r, clcf, &path, &of) != NGX_OK) {
            /* simulate successful logging */
            return len;
        }

        if (ngx_open_cached_file(clcf->open_file_cache, &path, &of, r->pool)
            != NGX_OK)
        {
            if (of.err == 0) {
                /* simulate successful logging */
                return len;
            }

            ngx_log_error(NGX_LOG_ERR, r->connection->log, of.err,
                          "testing \"%s\" existence failed", path.data);

            /* simulate successful logging */
            return len;
        }

        if (!of.is_dir) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, NGX_ENOTDIR,
                          "testing \"%s\" existence failed", path.data);

            /* simulate successful logging */
            return len;
        }
    }

    if (ngx_http_script_run(r, &log, script->lengths->elts, 1,
                            script->values->elts)
        == NULL)
    {
        /* simulate successful logging */
        return len;
    }

    log.data[log.len - 1] = '\0';
    *name = log.data;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http log \"%s\"", log.data);

    llcf = ngx_http_get_module_loc_conf(r, ngx_http_log_module);

    ngx_memzero(&of, sizeof(ngx_open_file_info_t));

    of.log = 1;
    of.valid = llcf->open_file_cache_valid;
    of.min_uses = llcf->open_file_cache_min_uses;
    of.directio = NGX_OPEN_FILE_DIRECTIO_OFF;

    if (ngx_http_set_disable_symlinks(r, clcf, &log, &of) != NGX_OK) {
        /* simulate successful logging */
        return len;
    }

    if (ngx_open_cached_file(llcf->open_file_cache, &log, &of, r->pool)
        != NGX_OK)
    {
        ngx_log_error(NGX_LOG_CRIT, r->connection->log, ngx_errno,
                      "%s \"%s\" failed", of.failed, log.data);
        /* simulate successful logging */
        return len;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http log #%d", of.fd);

    n = ngx_write_fd(of.fd, buf, len);

    return n;
}


#if (NGX_ZLIB)

static ssize_t
ngx_http_log_gzip(ngx_fd_t fd, u_char *buf, size_t len, ngx_int_t level,
    ngx_log_t *log)
{
    int          rc, wbits, memlevel;
    u_char      *out;
    size_t       size;
    ssize_t      n;
    z_stream     zstream;
    ngx_err_t    err;
    ngx_pool_t  *pool;

    wbits = MAX_WBITS;
    memlevel = MAX_MEM_LEVEL - 1;

    while ((ssize_t) len < ((1 << (wbits - 1)) - 262)) {
        wbits--;
        memlevel--;
    }

    /*
     * This is a formula from deflateBound() for conservative upper bound of
     * compressed data plus 18 bytes of gzip wrapper.
     */

    size = len + ((len + 7) >> 3) + ((len + 63) >> 6) + 5 + 18;

    ngx_memzero(&zstream, sizeof(z_stream));

    pool = ngx_create_pool(256, log);
    if (pool == NULL) {
        /* simulate successful logging */
        return len;
    }

    pool->log = log;

    zstream.zalloc = ngx_http_log_gzip_alloc;
    zstream.zfree = ngx_http_log_gzip_free;
    zstream.opaque = pool;

    out = ngx_pnalloc(pool, size);
    if (out == NULL) {
        goto done;
    }

    zstream.next_in = buf;
    zstream.avail_in = len;
    zstream.next_out = out;
    zstream.avail_out = size;

    rc = deflateInit2(&zstream, (int) level, Z_DEFLATED, wbits + 16, memlevel,
                      Z_DEFAULT_STRATEGY);

    if (rc != Z_OK) {
        ngx_log_error(NGX_LOG_ALERT, log, 0, "deflateInit2() failed: %d", rc);
        goto done;
    }

    ngx_log_debug4(NGX_LOG_DEBUG_HTTP, log, 0,
                   "deflate in: ni:%p no:%p ai:%ud ao:%ud",
                   zstream.next_in, zstream.next_out,
                   zstream.avail_in, zstream.avail_out);

    rc = deflate(&zstream, Z_FINISH);

    if (rc != Z_STREAM_END) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "deflate(Z_FINISH) failed: %d", rc);
        goto done;
    }

    ngx_log_debug5(NGX_LOG_DEBUG_HTTP, log, 0,
                   "deflate out: ni:%p no:%p ai:%ud ao:%ud rc:%d",
                   zstream.next_in, zstream.next_out,
                   zstream.avail_in, zstream.avail_out,
                   rc);

    size -= zstream.avail_out;

    rc = deflateEnd(&zstream);

    if (rc != Z_OK) {
        ngx_log_error(NGX_LOG_ALERT, log, 0, "deflateEnd() failed: %d", rc);
        goto done;
    }

    n = ngx_write_fd(fd, out, size);

    if (n != (ssize_t) size) {
        err = (n == -1) ? ngx_errno : 0;

        ngx_destroy_pool(pool);

        ngx_set_errno(err);
        return -1;
    }

done:

    ngx_destroy_pool(pool);

    /* simulate successful logging */
    return len;
}


static void *
ngx_http_log_gzip_alloc(void *opaque, u_int items, u_int size)
{
    ngx_pool_t *pool = opaque;

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, pool->log, 0,
                   "gzip alloc: n:%ud s:%ud", items, size);

    return ngx_palloc(pool, items * size);
}


static void
ngx_http_log_gzip_free(void *opaque, void *address)
{
#if 0
    ngx_pool_t *pool = opaque;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, pool->log, 0, "gzip free: %p", address);
#endif
}

#endif


static void
ngx_http_log_flush(ngx_open_file_t *file, ngx_log_t *log)
{
    size_t               len;
    ssize_t              n;
    ngx_http_log_buf_t  *buffer;

    buffer = file->data;

    len = buffer->pos - buffer->start;

    if (len == 0) {
        return;
    }

#if (NGX_ZLIB)
    if (buffer->gzip) {
        n = ngx_http_log_gzip(file->fd, buffer->start, len, buffer->gzip, log);
    } else {
        n = ngx_write_fd(file->fd, buffer->start, len);
    }
#else
    n = ngx_write_fd(file->fd, buffer->start, len);
#endif

    if (n == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      ngx_write_fd_n " to \"%s\" failed",
                      file->name.data);

    } else if ((size_t) n != len) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      ngx_write_fd_n " to \"%s\" was incomplete: %z of %uz",
                      file->name.data, n, len);
    }

    buffer->pos = buffer->start;

    if (buffer->event && buffer->event->timer_set) {
        ngx_del_timer(buffer->event);
    }
}


static void
ngx_http_log_flush_handler(ngx_event_t *ev)
{
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "http log buffer flush handler");

    ngx_http_log_flush(ev->data, ev->log);
}


static u_char *
ngx_http_log_copy_short(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    size_t     len;
    uintptr_t  data;

    len = op->len;
    data = op->data;

    while (len--) {
        *buf++ = (u_char) (data & 0xff);
        data >>= 8;
    }

    return buf;
}


static u_char *
ngx_http_log_copy_long(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    return ngx_cpymem(buf, (u_char *) op->data, op->len);
}


static u_char *
ngx_http_log_pipe(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    if (r->pipeline) {
        *buf = 'p';
    } else {
        *buf = '.';
    }

    return buf + 1;
}


static u_char *
ngx_http_log_time(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    return ngx_cpymem(buf, ngx_cached_http_log_time.data,
                      ngx_cached_http_log_time.len);
}

static u_char *
ngx_http_log_iso8601(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    return ngx_cpymem(buf, ngx_cached_http_log_iso8601.data,
                      ngx_cached_http_log_iso8601.len);
}

static u_char *
ngx_http_log_msec(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    ngx_time_t  *tp;

    tp = ngx_timeofday();

    return ngx_sprintf(buf, "%T.%03M", tp->sec, tp->msec);
}


static u_char *
ngx_http_log_request_time(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    ngx_time_t      *tp;
    ngx_msec_int_t   ms;

    tp = ngx_timeofday();

    ms = (ngx_msec_int_t)
             ((tp->sec - r->start_sec) * 1000 + (tp->msec - r->start_msec));
    ms = ngx_max(ms, 0);

    return ngx_sprintf(buf, "%d.%03d", ms / 1000, ms % 1000);//it is odd!!!  //return ngx_sprintf(buf, "%T.%03M", ms / 1000, ms % 1000);
}


static u_char *
ngx_http_log_status(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    ngx_uint_t  status;

    if (r->err_status) {
        status = r->err_status;

    } else if (r->headers_out.status) {
        status = r->headers_out.status;

    } else if (r->http_version == NGX_HTTP_VERSION_9) {
        status = 9;

    } else {
        status = 0;
    }

    return ngx_sprintf(buf, "%03ui", status);
}


static u_char *
ngx_http_log_bytes_sent(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    return ngx_sprintf(buf, "%O", r->connection->sent);
}


/*
 * although there is a real $body_bytes_sent variable,
 * this log operation code function is more optimized for logging
 */

static u_char *
ngx_http_log_body_bytes_sent(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    off_t  length;

    length = r->connection->sent - r->header_size;

    if (length > 0) {
        return ngx_sprintf(buf, "%O", length);
    }

    *buf = '0';

    return buf + 1;
}


static u_char *
ngx_http_log_request_length(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    return ngx_sprintf(buf, "%O", r->request_length);
}

static u_char *
ngx_http_log_request_sid(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    size_t     len,i;
    u_char*  data;

    if(r->request_sid.len == 0)
        return buf;

    len = r->request_sid.len;
    data = r->request_sid.data;


    for (i = 0;i<len;i++)
    {
        *buf++ = data[i];
    }

    return buf;
}

static u_char *
ngx_http_log_request_key(ngx_http_request_t *r, u_char *buf,
    ngx_http_log_op_t *op)
{
    size_t     len,i;
    u_char*  data;

    if(r->request_key.len == 0)
        return buf;

    len = r->request_key.len;
    data = r->request_key.data;


    for (i = 0;i<len;i++)
    {
        *buf++ = data[i];
    }

    return buf;
}



static ngx_int_t
ngx_http_log_variable_compile(ngx_conf_t *cf, ngx_http_log_op_t *op,
    ngx_str_t *value)
{
    ngx_int_t  index;

    index = ngx_http_get_variable_index(cf, value);
    if (index == NGX_ERROR) {
        return NGX_ERROR;
    }

    op->len = 0;
    op->getlen = ngx_http_log_variable_getlen;
    op->run = ngx_http_log_variable;
    op->data = index;

    return NGX_OK;
}


static size_t
ngx_http_log_variable_getlen(ngx_http_request_t *r, uintptr_t data)
{
    uintptr_t                   len;
    ngx_http_variable_value_t  *value;

    value = ngx_http_get_indexed_variable(r, data);

    if (value == NULL || value->not_found) {
        return 1;
    }

    len = ngx_http_log_escape(NULL, value->data, value->len);

    value->escape = len ? 1 : 0;

    return value->len + len * 3;
}


static u_char *
ngx_http_log_variable(ngx_http_request_t *r, u_char *buf, ngx_http_log_op_t *op)
{
    ngx_http_variable_value_t  *value;

    value = ngx_http_get_indexed_variable(r, op->data);

    if (value == NULL || value->not_found) {
        *buf = '-';
        return buf + 1;
    }

    if (value->escape == 0) {
        return ngx_cpymem(buf, value->data, value->len);

    } else {
        return (u_char *) ngx_http_log_escape(buf, value->data, value->len);
    }
}


static uintptr_t
ngx_http_log_escape(u_char *dst, u_char *src, size_t size)
{
    ngx_uint_t      n;
    static u_char   hex[] = "0123456789ABCDEF";

    static uint32_t   escape[] = {
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                    /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000004, /* 0000 0000 0000 0000  0000 0000 0000 0100 */

                    /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x10000000, /* 0001 0000 0000 0000  0000 0000 0000 0000 */

                    /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x80000000, /* 1000 0000 0000 0000  0000 0000 0000 0000 */

        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };


    if (dst == NULL) {

        /* find the number of the characters to be escaped */

        n = 0;

        while (size) {
            if (escape[*src >> 5] & (1 << (*src & 0x1f))) {
                n++;
            }
            src++;
            size--;
        }

        return (uintptr_t) n;
    }

    while (size) {
        if (escape[*src >> 5] & (1 << (*src & 0x1f))) {
            *dst++ = '\\';
            *dst++ = 'x';
            *dst++ = hex[*src >> 4];
            *dst++ = hex[*src & 0xf];
            src++;

        } else {
            *dst++ = *src++;
        }
        size--;
    }

    return (uintptr_t) dst;
}


static void *
ngx_http_log_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_log_main_conf_t  *conf;

    ngx_http_log_fmt_t  *fmt;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_log_main_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&conf->formats, cf->pool, 4, sizeof(ngx_http_log_fmt_t))
        != NGX_OK)
    {
        return NULL;
    }

    fmt = ngx_array_push(&conf->formats);
    if (fmt == NULL) {
        return NULL;
    }

    ngx_str_set(&fmt->name, "combined");

    fmt->flushes = NULL;

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_http_log_op_t));
    if (fmt->ops == NULL) {
        return NULL;
    }

    ///zll
    if (ngx_array_init(&conf->dblog_formats, cf->pool, 4, sizeof(ngx_http_log_fmt_t))
        != NGX_OK)
    {
        return NULL;
    }

    fmt = ngx_array_push(&conf->dblog_formats);
    if (fmt == NULL) {
        return NULL;
    }

    ngx_str_set(&fmt->name, "dblog_combined");

    fmt->flushes = NULL;

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_http_log_op_t));
    if (fmt->ops == NULL) {
        return NULL;
    }

    return conf;
}


static void *
ngx_http_log_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_log_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_log_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->open_file_cache = NGX_CONF_UNSET_PTR;

    return conf;
}


static char *
ngx_http_log_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_log_loc_conf_t *prev = parent;
    ngx_http_log_loc_conf_t *conf = child;

    ngx_http_log_t            *log;
    ngx_http_dblog_t          *dblog;
    ngx_http_log_fmt_t        *fmt;
    ngx_http_log_main_conf_t  *lmcf;

    if (conf->open_file_cache == NGX_CONF_UNSET_PTR) {

        conf->open_file_cache = prev->open_file_cache;
        conf->open_file_cache_valid = prev->open_file_cache_valid;
        conf->open_file_cache_min_uses = prev->open_file_cache_min_uses;

        if (conf->open_file_cache == NGX_CONF_UNSET_PTR) {
            conf->open_file_cache = NULL;
        }
    }



    if (conf->logs || conf->off) {
        goto mergedblog;
    }

    conf->logs = prev->logs;
    conf->off = prev->off;

    if (conf->logs || conf->off) {
        goto mergedblog;
    }


    conf->logs = ngx_array_create(cf->pool, 2, sizeof(ngx_http_log_t));
    if (conf->logs == NULL) {
        return NGX_CONF_ERROR;
    }

    log = ngx_array_push(conf->logs);
    if (log == NULL) {
        return NGX_CONF_ERROR;
    }

    log->file = ngx_conf_open_file(cf->cycle, &ngx_http_access_log);
    if (log->file == NULL) {
        return NGX_CONF_ERROR;
    }

    log->script = NULL;
    log->disk_full_time = 0;
    log->error_log_time = 0;

    lmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_log_module);
    fmt = lmcf->formats.elts;

    /* the default "combined" format */
    log->format = &fmt[0];
    lmcf->combined_used = 1;

    mergedblog:

    if (conf->dblogs ||conf->dblogoff) {
        return NGX_CONF_OK;
    }

    conf->dblogs = prev->dblogs;
    conf->dblogoff = prev->dblogoff;

    if (conf->dblogs ||conf->dblogoff) {
        return NGX_CONF_OK;
    }


    conf->dblogs = ngx_array_create(cf->pool, 2, sizeof(ngx_http_dblog_t));
    if (conf->dblogs == NULL) {
        return NGX_CONF_ERROR;
    }

    dblog = ngx_array_push(conf->dblogs);
    if (dblog == NULL) {
        return NGX_CONF_ERROR;
    }

//      dblog->file = ngx_conf_open_file(cf->cycle, &ngx_http_access_log);
//      if (dblog->file == NULL) {
//          return NGX_CONF_ERROR;
//      }
//
//      dblog->script = NULL;
//      dblog->disk_full_time = 0;
//      dblog->error_log_time = 0;

    lmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_log_module);
    fmt = lmcf->dblog_formats.elts;

    /* the default "combined" format */
    dblog->format = &fmt[0];
    lmcf->dblog_combined_used = 1;


    return NGX_CONF_OK;
}


static char *
ngx_http_log_set_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_loc_conf_t *llcf = conf;

    ssize_t                     size;
    ngx_int_t                   gzip;
    ngx_uint_t                  i, n;
    ngx_msec_t                  flush;
    ngx_str_t                  *value, name, s;
    ngx_http_log_t             *log;
    ngx_http_log_buf_t         *buffer;
    ngx_http_log_fmt_t         *fmt;
    ngx_http_log_main_conf_t   *lmcf;
    ngx_http_script_compile_t   sc;

    value = cf->args->elts;

    if (ngx_strcmp(value[1].data, "off") == 0) {
        llcf->off = 1;
        if (cf->args->nelts == 2) {
            return NGX_CONF_OK;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\"", &value[2]);
        return NGX_CONF_ERROR;
    }

    if (llcf->logs == NULL) {
        llcf->logs = ngx_array_create(cf->pool, 2, sizeof(ngx_http_log_t));
        if (llcf->logs == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    lmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_log_module);

    log = ngx_array_push(llcf->logs);
    if (log == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(log, sizeof(ngx_http_log_t));

    n = ngx_http_script_variables_count(&value[1]);

    if (n == 0) {
        log->file = ngx_conf_open_file(cf->cycle, &value[1]);
        if (log->file == NULL) {
            return NGX_CONF_ERROR;
        }

    } else {
        if (ngx_conf_full_name(cf->cycle, &value[1], 0) != NGX_OK) {
            return NGX_CONF_ERROR;
        }

        log->script = ngx_pcalloc(cf->pool, sizeof(ngx_http_log_script_t));
        if (log->script == NULL) {
            return NGX_CONF_ERROR;
        }

        ngx_memzero(&sc, sizeof(ngx_http_script_compile_t));

        sc.cf = cf;
        sc.source = &value[1];
        sc.lengths = &log->script->lengths;
        sc.values = &log->script->values;
        sc.variables = n;
        sc.complete_lengths = 1;
        sc.complete_values = 1;

        if (ngx_http_script_compile(&sc) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    if (cf->args->nelts >= 3) {
        name = value[2];

        if (ngx_strcmp(name.data, "combined") == 0) {
            lmcf->combined_used = 1;
        }

    } else {
        ngx_str_set(&name, "combined");
        lmcf->combined_used = 1;
    }

    fmt = lmcf->formats.elts;
    for (i = 0; i < lmcf->formats.nelts; i++) {
        if (fmt[i].name.len == name.len
            && ngx_strcasecmp(fmt[i].name.data, name.data) == 0)
        {
            log->format = &fmt[i];
            break;
        }
    }

    if (log->format == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "unknown log format \"%V\"", &name);
        return NGX_CONF_ERROR;
    }

    size = 0;
    flush = 0;
    gzip = 0;

    for (i = 3; i < cf->args->nelts; i++) {

        if (ngx_strncmp(value[i].data, "buffer=", 7) == 0) {
            s.len = value[i].len - 7;
            s.data = value[i].data + 7;

            size = ngx_parse_size(&s);

            if (size == NGX_ERROR || size == 0) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid buffer size \"%V\"", &s);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "flush=", 6) == 0) {
            s.len = value[i].len - 6;
            s.data = value[i].data + 6;

            flush = ngx_parse_time(&s, 0);

            if (flush == (ngx_msec_t) NGX_ERROR || flush == 0) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid flush time \"%V\"", &s);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "gzip", 4) == 0
            && (value[i].len == 4 || value[i].data[4] == '='))
        {
#if (NGX_ZLIB)
            if (size == 0) {
                size = 64 * 1024;
            }

            if (value[i].len == 4) {
                gzip = Z_BEST_SPEED;
                continue;
            }

            s.len = value[i].len - 5;
            s.data = value[i].data + 5;

            gzip = ngx_atoi(s.data, s.len);

            if (gzip < 1 || gzip > 9) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid compression level \"%V\"", &s);
                return NGX_CONF_ERROR;
            }

            continue;

#else
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "nginx was built without zlib support");
            return NGX_CONF_ERROR;
#endif
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\"", &value[i]);
        return NGX_CONF_ERROR;
    }

    if (flush && size == 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "no buffer is defined for access_log \"%V\"",
                           &value[1]);
        return NGX_CONF_ERROR;
    }

    if (size) {

        if (log->script) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "buffered logs cannot have variables in name");
            return NGX_CONF_ERROR;
        }

        if (log->file->data) {
            buffer = log->file->data;

            if (buffer->last - buffer->start != size
                || buffer->flush != flush
                || buffer->gzip != gzip)
            {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "access_log \"%V\" already defined "
                                   "with conflicting parameters",
                                   &value[1]);
                return NGX_CONF_ERROR;
            }

            return NGX_CONF_OK;
        }

        buffer = ngx_pcalloc(cf->pool, sizeof(ngx_http_log_buf_t));
        if (buffer == NULL) {
            return NGX_CONF_ERROR;
        }

        buffer->start = ngx_pnalloc(cf->pool, size);
        if (buffer->start == NULL) {
            return NGX_CONF_ERROR;
        }

        buffer->pos = buffer->start;
        buffer->last = buffer->start + size;

        if (flush) {
            buffer->event = ngx_pcalloc(cf->pool, sizeof(ngx_event_t));
            if (buffer->event == NULL) {
                return NGX_CONF_ERROR;
            }

            buffer->event->data = log->file;
            buffer->event->handler = ngx_http_log_flush_handler;
            buffer->event->log = &cf->cycle->new_log;

            buffer->flush = flush;
        }

        buffer->gzip = gzip;

        log->file->flush = ngx_http_log_flush;
        log->file->data = buffer;
    }

    return NGX_CONF_OK;
}


static char *
ngx_http_log_set_format(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_main_conf_t *lmcf = conf;

    ngx_str_t           *value;
    ngx_uint_t           i;
    ngx_http_log_fmt_t  *fmt;

    if (cf->cmd_type != NGX_HTTP_MAIN_CONF) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "the \"log_format\" directive may be used "
                           "only on \"http\" level");
    }

    value = cf->args->elts;

    fmt = lmcf->formats.elts;
    for (i = 0; i < lmcf->formats.nelts; i++) {
        if (fmt[i].name.len == value[1].len
            && ngx_strcmp(fmt[i].name.data, value[1].data) == 0)
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "duplicate \"log_format\" name \"%V\"",
                               &value[1]);
            return NGX_CONF_ERROR;
        }
    }

    fmt = ngx_array_push(&lmcf->formats);
    if (fmt == NULL) {
        return NGX_CONF_ERROR;
    }

    fmt->name = value[1];

    fmt->flushes = ngx_array_create(cf->pool, 4, sizeof(ngx_int_t));
    if (fmt->flushes == NULL) {
        return NGX_CONF_ERROR;
    }

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_http_log_op_t));
    if (fmt->ops == NULL) {
        return NGX_CONF_ERROR;
    }

    return ngx_http_log_compile_format(cf, fmt->flushes, fmt->ops, cf->args, 2);
}


static char *
ngx_http_log_compile_format(ngx_conf_t *cf, ngx_array_t *flushes,
    ngx_array_t *ops, ngx_array_t *args, ngx_uint_t s)
{
    u_char              *data, *p, ch;
    size_t               i, len;
    ngx_str_t           *value, var;
    ngx_int_t           *flush;
    ngx_uint_t           bracket;
    ngx_http_log_op_t   *op;
    ngx_http_log_var_t  *v;

    value = args->elts;

    for ( /* void */ ; s < args->nelts; s++) {

        i = 0;

        while (i < value[s].len) {

            op = ngx_array_push(ops);
            if (op == NULL) {
                return NGX_CONF_ERROR;
            }

            data = &value[s].data[i];

            if (value[s].data[i] == '$') {

                if (++i == value[s].len) {
                    goto invalid;
                }

                if (value[s].data[i] == '{') {
                    bracket = 1;

                    if (++i == value[s].len) {
                        goto invalid;
                    }

                    var.data = &value[s].data[i];

                } else {
                    bracket = 0;
                    var.data = &value[s].data[i];
                }

                for (var.len = 0; i < value[s].len; i++, var.len++) {
                    ch = value[s].data[i];

                    if (ch == '}' && bracket) {
                        i++;
                        bracket = 0;
                        break;
                    }

                    if ((ch >= 'A' && ch <= 'Z')
                        || (ch >= 'a' && ch <= 'z')
                        || (ch >= '0' && ch <= '9')
                        || ch == '_')
                    {
                        continue;
                    }

                    break;
                }

                if (bracket) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "the closing bracket in \"%V\" "
                                       "variable is missing", &var);
                    return NGX_CONF_ERROR;
                }

                if (var.len == 0) {
                    goto invalid;
                }

                for (v = ngx_http_log_vars; v->name.len; v++) {

                    if (v->name.len == var.len
                        && ngx_strncmp(v->name.data, var.data, var.len) == 0)
                    {
                        op->len = v->len;
                        op->getlen = NULL;
                        op->run = v->run;
                        op->data = 0;

                        goto found;
                    }
                }

                if (ngx_http_log_variable_compile(cf, op, &var) != NGX_OK) {
                    return NGX_CONF_ERROR;
                }

                if (flushes) {

                    flush = ngx_array_push(flushes);
                    if (flush == NULL) {
                        return NGX_CONF_ERROR;
                    }

                    *flush = op->data; /* variable index */
                }

            found:

                continue;
            }

            i++;

            while (i < value[s].len && value[s].data[i] != '$') {
                i++;
            }

            len = &value[s].data[i] - data;

            if (len) {

                op->len = len;
                op->getlen = NULL;

                if (len <= sizeof(uintptr_t)) {
                    op->run = ngx_http_log_copy_short;
                    op->data = 0;

                    while (len--) {
                        op->data <<= 8;
                        op->data |= data[len];
                    }

                } else {
                    op->run = ngx_http_log_copy_long;

                    p = ngx_pnalloc(cf->pool, len);
                    if (p == NULL) {
                        return NGX_CONF_ERROR;
                    }

                    ngx_memcpy(p, data, len);
                    op->data = (uintptr_t) p;
                }
            }
        }
    }

    return NGX_CONF_OK;

invalid:

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid parameter \"%s\"", data);

    return NGX_CONF_ERROR;
}


static char *
ngx_http_log_open_file_cache(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_loc_conf_t *llcf = conf;

    time_t       inactive, valid;
    ngx_str_t   *value, s;
    ngx_int_t    max, min_uses;
    ngx_uint_t   i;

    if (llcf->open_file_cache != NGX_CONF_UNSET_PTR) {
        return "is duplicate";
    }

    value = cf->args->elts;

    max = 0;
    inactive = 10;
    valid = 60;
    min_uses = 1;

    for (i = 1; i < cf->args->nelts; i++) {

        if (ngx_strncmp(value[i].data, "max=", 4) == 0) {

            max = ngx_atoi(value[i].data + 4, value[i].len - 4);
            if (max == NGX_ERROR) {
                goto failed;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "inactive=", 9) == 0) {

            s.len = value[i].len - 9;
            s.data = value[i].data + 9;

            inactive = ngx_parse_time(&s, 1);
            if (inactive == (time_t) NGX_ERROR) {
                goto failed;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "min_uses=", 9) == 0) {

            min_uses = ngx_atoi(value[i].data + 9, value[i].len - 9);
            if (min_uses == NGX_ERROR) {
                goto failed;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "valid=", 6) == 0) {

            s.len = value[i].len - 6;
            s.data = value[i].data + 6;

            valid = ngx_parse_time(&s, 1);
            if (valid == (time_t) NGX_ERROR) {
                goto failed;
            }

            continue;
        }

        if (ngx_strcmp(value[i].data, "off") == 0) {

            llcf->open_file_cache = NULL;

            continue;
        }

    failed:

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid \"open_log_file_cache\" parameter \"%V\"",
                           &value[i]);
        return NGX_CONF_ERROR;
    }

    if (llcf->open_file_cache == NULL) {
        return NGX_CONF_OK;
    }

    if (max == 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "\"open_log_file_cache\" must have \"max\" parameter");
        return NGX_CONF_ERROR;
    }

    llcf->open_file_cache = ngx_open_file_cache_init(cf->pool, max, inactive);

    if (llcf->open_file_cache) {

        llcf->open_file_cache_valid = valid;
        llcf->open_file_cache_min_uses = min_uses;

        return NGX_CONF_OK;
    }

    return NGX_CONF_ERROR;
}


static ngx_int_t
ngx_http_log_init(ngx_conf_t *cf)
{
    ngx_str_t                  *value;
    ngx_array_t                 a;
    ngx_http_handler_pt        *h;
    ngx_http_log_fmt_t         *fmt;
    ngx_http_log_main_conf_t   *lmcf;
    ngx_http_core_main_conf_t  *cmcf;

    lmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_log_module);

    if (lmcf->combined_used) {
        if (ngx_array_init(&a, cf->pool, 1, sizeof(ngx_str_t)) != NGX_OK) {
            return NGX_ERROR;
        }

        value = ngx_array_push(&a);
        if (value == NULL) {
            return NGX_ERROR;
        }

        *value = ngx_http_combined_fmt;
        fmt = lmcf->formats.elts;

        if (ngx_http_log_compile_format(cf, NULL, fmt->ops, &a, 0)
            != NGX_CONF_OK)
        {
            return NGX_ERROR;
        }
    }



    if (lmcf->dblog_combined_used ) {
        if (ngx_array_init(&a, cf->pool, 1, sizeof(ngx_str_t)) != NGX_OK) {
            return NGX_ERROR;
        }

        value = ngx_array_push(&a);
        if (value == NULL) {
            return NGX_ERROR;
        }

        *value = ngx_http_combined_sql;
        fmt = lmcf->dblog_formats.elts;

        if (ngx_http_log_compile_format(cf, NULL, fmt->ops, &a, 0)
            != NGX_CONF_OK)
        {
            return NGX_ERROR;
        }
    }



    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_log_handler;

    return NGX_OK;
}

static OCISPool   *poolhp = (OCISPool *)0; 
static OCIEnv     *envhp = (OCIEnv *)0;
static OCIError   *errhp = (OCIError *)0;
//static OCISession *authp = (OCISession *)0;
//static OCIServer  *srvhp = (OCIServer *)0;
//static OCISvcCtx  *svchp = (OCISvcCtx *)0;
static OCIAuthInfo *authp = (OCIAuthInfo *)0;
//sword errcode = 0;
//static text *dblink = (text *) "orcl";
//static text *username = (text *) "sde";
//static text *password = (text *) "sde";
//static sword status;
//static ngx_str_t OCIConnectionPoolName = ngx_string("OCIConnectionPool");
//static ngx_str_t PoolUserName = ngx_string("ocp");
//static ngx_str_t PoolPassword = ngx_string("ocp");

//static ngx_str_t OCISPoolName = ngx_string("OCISessionPool");
//static ngx_str_t SPoolUserName = ngx_string("osp");
//static ngx_str_t SPoolPassword = ngx_string("osp");
//static ngx_str_t AppUserName = ngx_string("FMAP");
//static ngx_str_t AppPassword = ngx_string("FMAP");
static int timeout = 30;
static OraText *poolName;
static ub4 poolNameLen;
//static CONST OraText *database = (text *)"orcl";
//static CONST OraText *appusername =(text *)"sde";
//static CONST OraText *apppassword =(text *)"sde";
static ub4 sessMin = 2;
static ub4 sessMax = 20;
static ub4 sessIncr = 2;

static void
ngx_write_dblog(ngx_http_request_t *r, ngx_log_t *log, u_char *buf,size_t len)
{
//      ngx_write_fd(log->file->fd, buf, len);
//      checkerr(errhp, OCIStmtPrepare(inserthp, errhp, (const OraText*)ngx_http_combined_sql.data,ngx_http_combined_sql.len,
//                  (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
    OCISvcCtx *svchp = (OCISvcCtx *) 0;\
    OCIStmt    *inserthp = (OCIStmt *)0;
 

    OCISessionGet(envhp, errhp, &svchp, authp,(OraText *)poolName, (ub4)strlen((char *)poolName), NULL, 0, NULL, NULL, NULL, OCI_SESSGET_SPOOL);
    OCIHandleAlloc( envhp, (dvoid**)&inserthp,OCI_HTYPE_STMT, 0, 0);

    checkerr(errhp, OCIStmtPrepare(inserthp, errhp, buf,len, OCI_NTV_SYNTAX, OCI_DEFAULT),log);
    checkerr(errhp,OCIStmtExecute(svchp, inserthp, errhp, 1, 0, 0, 0, OCI_DEFAULT),log);
    checkerr(errhp,OCITransCommit(svchp, errhp, 0),log);

    checkerr(errhp, OCIHandleFree((dvoid *) inserthp, OCI_HTYPE_STMT),log);
    checkerr(errhp, OCISessionRelease(svchp, errhp, NULL, 0, OCI_DEFAULT),log);
    


    /*
        if(!poolhp||!envhp || !errhp ||!authp ||!srvhp ||!svchp)
            return;

        if(!inserthp)
        {
            OCIHandleAlloc( envhp, (dvoid**)&inserthp,OCI_HTYPE_STMT, 0, 0);

        }

        checkerr(errhp, OCIStmtPrepare(inserthp, errhp, buf,len, OCI_NTV_SYNTAX, OCI_DEFAULT),log);

        checkerr(errhp,OCIStmtExecute(svchp, inserthp, errhp, 1, 0, 0, 0, OCI_DEFAULT),log);

        checkerr(errhp,OCITransCommit(svchp, errhp, 0),log);

        */

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

    if(cycle->dbconn && cycle->envhp && cycle->poolhp)
    {
        //destroy the session pool.
        OCISessionPoolDestroy(cycle->poolhp, cycle->errhp, OCI_DEFAULT);
        checkerr(cycle->errhp,OCIHandleFree(cycle->poolhp, OCI_HTYPE_SPOOL),cycle->log);
//        checkerr(cycle->errhp,OCISessionEnd(cycle->svchp,cycle->errhp,cycle->authp,OCI_DEFAULT),cycle->log);
//        checkerr(cycle->errhp,OCIServerDetach(cycle->srvhp,cycle->errhp,OCI_DEFAULT),cycle->log);
        checkerr(cycle->errhp,OCIHandleFree( cycle->envhp, OCI_HTYPE_ENV),cycle->log);
        checkerr(cycle->errhp,OCIHandleFree(cycle->authp, OCI_HTYPE_AUTHINFO),cycle->log);
        checkerr(cycle->errhp,OCIHandleFree(cycle->errhp, OCI_HTYPE_ERROR),cycle->log);

        cycle->dbconn--;
        cycle->poolhp = NULL;
        cycle->envhp = NULL;

    }

    return;

}


static char *
ngx_http_log_set_dblog(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_loc_conf_t *llcf = conf;

    ngx_uint_t                  i;
    ngx_str_t                  *value, name;
    ngx_http_dblog_t           *dblog;
    ngx_http_log_fmt_t         *fmt;
    ngx_http_log_main_conf_t   *lmcf;
    sword errcode = 0;
    value= cf->args->elts;

    if (ngx_strcmp(value[1].data, "off") == 0) {   //access_dblog off
        llcf->dblogoff =1;
        if (cf->args->nelts == 2) {
            return NGX_CONF_OK;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"invalid parameter \"%V\"", &value[2]);
        return NGX_CONF_ERROR;
    }else if(cf->args->nelts < 4){
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"parameter is missing ,count of parameter must greater than or equal 4");
        return NGX_CONF_ERROR;
    }
//    else if(cf->args->nelts > 4){
//      ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"invalid parameter \"%V\",count of parameter must be 4",&value[2]);
//      return NGX_CONF_ERROR;
//    }

    if (llcf->dblogs == NULL) {
        llcf->dblogs = ngx_array_create(cf->pool, 2, sizeof(ngx_http_dblog_t));
        if (llcf->dblogs == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    lmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_log_module);

    dblog = ngx_array_push(llcf->dblogs);
    if (dblog == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(dblog, sizeof(ngx_http_dblog_t));


    if (cf->args->nelts >= 5) {
        name = value[4];

        if (ngx_strcmp(name.data, "dblog_combined") == 0) {
            lmcf->dblog_combined_used = 1;
        }

    } else {
        ngx_str_set(&name, "dblog_combined");
        lmcf->dblog_combined_used = 1;
    }

    fmt = lmcf->dblog_formats.elts;
    for (i = 0; i < lmcf->dblog_formats.nelts; i++) {
        if (fmt[i].name.len == name.len
            && ngx_strcasecmp(fmt[i].name.data, name.data) == 0)
        {
            dblog->format = &fmt[i];
            break;
        }
    }

    if (dblog->format == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"unknown dblog format \"%V\"", &name);
        return NGX_CONF_ERROR;
    }

    ///zll ,init db env by oci secconnection pool
    if(!cf->cycle->dbconn)
    {
        errcode = OCIEnvCreate((OCIEnv **)&cf->cycle->envhp, (ub4) OCI_DEFAULT,(dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,(void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0);
        if (errcode != 0) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Create OCIEnv Failed!");
            return NGX_CONF_ERROR;
        }
        
        OCIHandleAlloc((dvoid *) cf->cycle->envhp, (dvoid **) &cf->cycle->errhp, OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0);
        OCIHandleAlloc((dvoid *) cf->cycle->envhp, (dvoid **) &cf->cycle->poolhp, OCI_HTYPE_SPOOL, (size_t) 0,(dvoid **) 0);

        /* Create the session pool */
        checkerr(cf->cycle->errhp, OCIAttrSet((dvoid *) cf->cycle->poolhp,(ub4) OCI_HTYPE_SPOOL, 
                (dvoid *) &timeout, (ub4)0, OCI_ATTR_SPOOL_TIMEOUT,  cf->cycle->errhp),cf->log);
        //errcode = OCISessionPoolCreate(cf->cycle->envhp, cf->cycle->errhp, cf->cycle->poolhp, (OraText **)&OCISPoolName.data,
        //        (ub4 *)&OCISPoolName.len, (text *)value[1].data,(ub4)value[1].len, 2, 20, 1,
        //        (OraText *)AppUserName.data,(ub4)AppUserName.len,
        //        (OraText *)AppPassword.data,(ub4)strlen((const signed char *)AppPassword.len),
        //        OCI_DEFAULT);
        //errcode = OCISessionPoolCreate(cf->cycle->envhp, cf->cycle->errhp,cf->cycle->poolhp, (OraText **)&poolName, 
        //      (ub4 *)&poolNameLen, database, (ub4)strlen((const signed char *)database),
        //      sessMin, sessMax, sessIncr,(OraText *)appusername,(ub4)strlen((const signed char *)appusername),
        //      (OraText *)apppassword,(ub4)strlen((const signed char *)apppassword),OCI_DEFAULT);

        errcode = OCISessionPoolCreate(cf->cycle->envhp, cf->cycle->errhp,cf->cycle->poolhp, (OraText **)&poolName, 
              (ub4 *)&poolNameLen,(text *)value[1].data,value[1].len,
              sessMin, sessMax, sessIncr,(text *)value[2].data,value[2].len,
              (text *)value[3].data,value[3].len,OCI_DEFAULT);
              
        if (errcode != 0) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Create OCISessionPool Failed!");
            return NGX_CONF_ERROR;
        }


        checkerr(errhp, OCIHandleAlloc((dvoid *) cf->cycle->envhp,(dvoid **)&cf->cycle->authp, (ub4) OCI_HTYPE_AUTHINFO,(size_t) 0, (dvoid **) 0),cf->log);
 
//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"zll3");
//        checkerr(errhp, OCIAttrSet((dvoid *) cf->cycle->authp,(ub4) OCI_HTYPE_AUTHINFO, (dvoid *) appusername, 
//                    (ub4) strlen((char *)appusername),(ub4) OCI_ATTR_USERNAME, errhp),cf->log);
//        checkerr(errhp,OCIAttrSet((dvoid *) cf->cycle->authp,(ub4) OCI_HTYPE_AUTHINFO, (dvoid *) apppassword, 
//                    (ub4) strlen((char *)apppassword),(ub4) OCI_ATTR_PASSWORD, errhp),cf->log);
//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"zll4");
        poolhp = cf->cycle->poolhp;
        envhp = cf->cycle->envhp;
        errhp = cf->cycle->errhp;
        authp = cf->cycle->authp;
    } else {
        poolhp = cf->cycle->poolhp;
        envhp = cf->cycle->envhp;
        errhp = cf->cycle->errhp;
        authp = cf->cycle->authp;
    }



/*
    //ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"zllllll %d",cf->cycle->dbconn);
    ///zll init db env
    if (!cf->cycle->dbconn)
    {
        errcode = OCIEnvCreate((OCIEnv **)&cf->cycle->envhp, (ub4) OCI_DEFAULT,
                (dvoid *) 0, (dvoid * (*)(dvoid *,size_t)) 0,
                (dvoid * (*)(dvoid *, dvoid *, size_t)) 0,
                (void (*)(dvoid *, dvoid *)) 0, (size_t) 0, (dvoid **) 0);
        if (errcode != 0) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,"Create OCIEnv Failed!");
            return NGX_CONF_ERROR;
        } else {
            ngx_log_debug0(NGX_LOG_DEBUG_HTTP, cf->log, 0, "Create OCIEnv Success!");
            cf->cycle->dbconn ++;

            checkerr(cf->cycle->errhp,OCIHandleAlloc( (dvoid *) cf->cycle->envhp, (dvoid **) &cf->cycle->errhp, OCI_HTYPE_ERROR,(size_t) 0, (dvoid **) 0),cf->log);
            OCIHandleAlloc( (dvoid *) cf->cycle->envhp, (dvoid **) &cf->cycle->srvhp, OCI_HTYPE_SERVER,(size_t) 0, (dvoid **) 0);
            OCIHandleAlloc( (dvoid *) cf->cycle->envhp, (dvoid **) &cf->cycle->svchp, OCI_HTYPE_SVCCTX,(size_t) 0, (dvoid **) 0);
            checkerr(cf->cycle->errhp,OCIServerAttach( cf->cycle->srvhp, cf->cycle->errhp, (text *)value[1].data,value[1].len, 0),cf->log);
            OCIAttrSet( (dvoid *) cf->cycle->svchp, OCI_HTYPE_SVCCTX, (dvoid *)cf->cycle->srvhp,(ub4) 0, OCI_ATTR_SERVER, (OCIError *) cf->cycle->errhp);
            OCIHandleAlloc((dvoid *) cf->cycle->envhp, (dvoid **)&cf->cycle->authp,(ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0);
            OCIAttrSet((dvoid *) cf->cycle->authp, (ub4) OCI_HTYPE_SESSION,(dvoid *) value[2].data, (ub4) value[2].len,(ub4) OCI_ATTR_USERNAME, cf->cycle->errhp);
            OCIAttrSet((dvoid *) cf->cycle->authp, (ub4) OCI_HTYPE_SESSION,(dvoid *) value[3].data, (ub4) value[3].len,(ub4) OCI_ATTR_PASSWORD, cf->cycle->errhp);
            checkerr(cf->cycle->errhp,OCISessionBegin ( cf->cycle->svchp,  cf->cycle->errhp, cf->cycle->authp, OCI_CRED_RDBMS,(ub4) OCI_DEFAULT),cf->log);
            OCIAttrSet((dvoid *) cf->cycle->svchp, (ub4) OCI_HTYPE_SVCCTX,(dvoid *) cf->cycle->authp, (ub4) 0,(ub4) OCI_ATTR_SESSION, cf->cycle->errhp);

            envhp = cf->cycle->envhp;
            errhp = cf->cycle->errhp;
            authp = cf->cycle->authp;
            srvhp = cf->cycle->srvhp;
            svchp = cf->cycle->svchp;

        }
    } else {
            envhp = cf->cycle->envhp;
            errhp = cf->cycle->errhp;
            authp = cf->cycle->authp;
            srvhp = cf->cycle->srvhp;
            svchp = cf->cycle->svchp;
    }
*/

    return NGX_CONF_OK;
}


///same as ngx_http_log_set_format()
static char *
ngx_http_log_set_dblog_format(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_log_main_conf_t *lmcf = conf;

    ngx_str_t           *value;
    ngx_uint_t           i;
    ngx_http_log_fmt_t  *fmt;

    if (cf->cmd_type != NGX_HTTP_MAIN_CONF) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "the \"dblog_format\" directive may be used "
                           "only on \"http\" level");
    }

    value = cf->args->elts;

    fmt = lmcf->dblog_formats.elts;
    for (i = 0; i < lmcf->dblog_formats.nelts; i++) {
        if (fmt[i].name.len == value[1].len
            && ngx_strcmp(fmt[i].name.data, value[1].data) == 0)
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "duplicate \"dblog_format\" name \"%V\"",
                               &value[1]);
            return NGX_CONF_ERROR;
        }
    }

    fmt = ngx_array_push(&lmcf->dblog_formats);
    if (fmt == NULL) {
        return NGX_CONF_ERROR;
    }

    fmt->name = value[1];

    fmt->flushes = ngx_array_create(cf->pool, 4, sizeof(ngx_int_t));
    if (fmt->flushes == NULL) {
        return NGX_CONF_ERROR;
    }

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_http_log_op_t));
    if (fmt->ops == NULL) {
        return NGX_CONF_ERROR;
    }

    return ngx_http_log_compile_format(cf, fmt->flushes, fmt->ops, cf->args, 2);
}


