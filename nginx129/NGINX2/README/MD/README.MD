修改版NGINX实施部署文档
========
##NGINX编译安装
###LINUX环境(Redhat 5.5)


#####1.安装oracle instantclient  
下载链接:[oracle instantclient下载](http://www.oracle.com/technetwork/topics/linuxsoft-082809.html)。  
下载并安装安装basic、sqlplus和devel这三个包(目前Linux下已通过测试Version 10.2.0.5版本,**注意32位与64位区别**):  
```
oracle-instantclient-basic-10.2.0.5-1.i386.rpm 
oracle-instantclient-sqlplus-10.2.0.5-1.i386.rpm  
oracle-instantclient-devel-10.2.0.5-1.i386.rpm
```  

#####2.在下载目录下rpm命令执行  
`#rpm -ilv oracle-instantclient-basic-10.2.0.5-1.i386.rpm oracle-instantclient-sqlplus-10.2.0.5-1.i386.rpm oracle-instantclient-devel-10.2.0.5-1.i386.rpm`   
安装后，执行命令  
`#whereis oracle`  
回显如下结果:  
oracle: /usr/lib/oracle /usr/include/oracle /usr/share/oracle  
说明oracle client已经安装到以上目录了，接着配置环境变量。  


#####3.配置环境变量  
`#vi ~/.bash_profile`  
在.bash_profile中添加如下行:  
32位:  
```
export LD_LIBRARY_PATH=/usr/lib/oracle/10.2.0.5/client/lib
export TNS_ADMIN=/usr/lib/oracle/10.2.0.5/client
export ORACLE_HOME=/usr/lib/oracle/10.2.0.5/client
```
64位:  
```
export LD_LIBRARY_PATH=/usr/lib/oracle/10.2.0.5/client64/lib
export TNS_ADMIN=/usr/lib/oracle/10.2.0.5/client64
export ORACLE_HOME=/usr/lib/oracle/10.2.0.5/client64
```
然后执行命令,是环境变量配置生效:  
`source ~/.bash_profile`    
测试命令:   
`echo $LD_LIBRARY_PATH`   



#####4.oracle客户端连接配置  
在如下路径新增tnsnames.ora文件:    
32位:`/usr/lib/oracle/10.2.0.5/client/tnsnames.ora`  
64位:`/usr/lib/oracle/10.2.0.5/client64/tnsnames.ora`  
并添加如下配置:  
```
ORCL =
  (DESCRIPTION =
    (ADDRESS_LIST =
      (ADDRESS = (PROTOCOL = TCP)(HOST = 172.16.16.194)(PORT = 1521))
    )
    (CONNECT_DATA =
      (SERVICE_NAME = orcl)
    )
  )
```


#####5.测试客户端数据连接  
命令如下:  
32位:`sqlplus sde/sde@orcl172`  
64位:`sqlplus64 sde/sde@orcl172`  



###NGXIN编译安装


######1.官方参考  
[Building nginx from Sources](http://nginx.org/en/docs/configure.html)  
#####2.代码说明
平台改造版NGINX是基于Nignx 1.2.9版本做修改，修改增加如下文件:
```
src/core/nginx.c
src/core/ngx_cycle.c
src/core/ngx_cycle.h
src/core/ngx_hash.h
src/http/ngx_http_request.h
src/http/modules/ngx_http_access_control_module.c
src/http/modules/ngx_http_log_module.c
src/http/modules/config
src/os/win32/ngx_process_cycle.c
```
其中`src/http/modules/config`做为新增模块`src/http/modules/ngx_http_access_control_module.c`的编译配置文件，内容如下:  
```
ngx_addon_name=ngx_http_access_control_module
HTTP_MODULES="$HTTP_MODULES ngx_http_access_control_module"
NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_access_control_module.c"
```
######3.编译生成    
+   切换到NGINX工程目录下:  
  `#cd ~/nginx`  
*   linux需将模块配置文件由dos转为unix文本格式,执行命令:  
`dos2unix src/http/modules/config` 
*   生成makefile文件(64位oracle instantclient):  
```
bash ./configure  --with-ld-opt="-L /usr/lib/oracle/10.2.0.5/client64/lib/ -lclntsh" --with-cc-opt="-I /usr/include/oracle/10.2.0.5/client64" --sbin-path=/usr/local/nginx/nginx --conf-path=/usr/local/nginx/nginx.conf --pid-path=/usr/local/nginx/nginx.pid --with-http_ssl_module --with-http_stub_status_module --add-module=src/http/modules --with-debug
```  
  其中，`--with-debug`用于调试运行，支持nginx的debug层级日志。  
-    依次执行命令:  
  编译：`make`  
  安装：`make install`


###配置文件nginx.conf
1.新增模块的配置  
```
#编辑访问日志入库sql,db日志与文件日志相互独立
#支持http、server和location上下文，一般在http上下文配置
dblog_format dbmain "INSERT INTO fmap_accesslog_t(remoteaddr, timestamp, uri, sid, httpstatus, key,costtime) VALUES(\'$remote_addr\',TO_TIMESTAMP_TZ(\'$time_local\', \'dd/mon/yyyy:hh24:mi:ss tzhtzm\', \'NLS_DATE_LANGUAGE = AMERICAN\'), \'$request_uri\', \'$request_sid\',$status,\'$request_key\',$request_time)"; 

#设置db连接实例、用户名、密码和入库sql名称
#支持main、server和location上下文，一般在main和server上下文配置
access_dblog orcl sde sde dbmain;

#关闭db日志
access_dblog off;

#设置访问控制的db连接实例、用户名、密码和授权数据获取sql
#仅适配http上下文
accesscontrol_dbconf orcl sde sde "SELECT A.KEY,S.URI,A.MAXTPH,A.SID FROM FMAP_ACCESSCONTROL_T A,FMAP_SERVICE_T S WHERE  A.ISAPPROVED =1 AND A.SID = S.SID";

#设置共享内存
#aczone:设置共享内存的名称(只做识别作用)和大小
#burst:设置最高突发次数(3次/s)
#nodelay:设置是否延迟burst突发请求的处理，nodelay表示不延迟请求，超量请求则直接返回错误码；没有设置nodelay则会延迟超量请求。
#支持server和location上下文
access_control aczone=fmap:10m burst=3 nodelay;

#设置访问控制日志的层级，如info，error等，属不常用设置
#支持server和location上下文
access_control_log_level info;

#设置访问控制中被限制处理请求的错误码
#支持server和location上下文
access_control_status 511;

#Sets the maximum number of requests that can be served through one keep-alive connection. After the maximum number of requests are made, the connection is closed.  
#syntax: keepalive_requests number;  
#default:  keepalive_requests 100;  
#context:  http, server, location  
keepalive_requests 10000;
```
2.精化模块的设置
``` 
#Disables keep-alive connections with misbehaving browsers. The browser parameters specify which browsers will be affected. The value msie6 disables keep-alive connections with old versions of MSIE, once a POST request is received. The value safari disables keep-alive connections with Safari and Safari-like browsers on Mac OS X and Mac OS X-like operating systems. The value none enables keep-alive connections with all browsers.Prior to version 1.1.18, the value safari matched all Safari and Safari-like browsers on all operating systems, and keep-alive connections with them were disabled by default.
#context:  http, server, location
keepalive_disable msie6;    #default

#default: keepalive_requests 100;
#context:  http, server, location
#This directive appeared in version 0.8.0.
#Sets the maximum number of requests that can be served through one keep-alive connection. After the maximum number of requests are made, the connection is closed.
keepalive_requests 10000;

#开启或关闭在错误信息的“Server”响应头中输出nginx版本号。
#默认值:  server_tokens on;
server_tokens off;

#状态监控，nginx编译需加上“ --with-http_stub_status_module”。
#通过"http://ip:post/status"访问监控状态，返回值说明：
Active connections    //当前 Nginx 正处理的活动连接数。
server accepts handled requests x y z//总共处理了x 个连接 , 成功创建处理y个连接,总共处理了z个请求。
Reading //nginx 读取到客户端的 Header 信息数。
Writing //nginx 返回给客户端的 Header 信息数。
Waiting //开启 keep-alive 的情况下，这个值等于 active - (reading + writing)，意思就是 Nginx 已经处理完正在等候下一次请求指令的驻留连接
#conf文件配置如下：
location /status {
stub_status on;
}

```

###改造模块说明

1.日志入库模块日志表表结构--ngx_http_log_module.c  

|字段 | 类型 | 注释 |
|--- | --- | --- |
| REMOTEADDR | VARCHAR2(64) | 用户IP    | 
| TIMESTAMP  | DATE         | 访问时间  |
| URI        | VARCHAR2(64) | 访问连接串|
| SID        | VARCHAR2(6)  | 服务ID    |
| HTTPSTATUS | INTEGER      | 响应状态码|
| KEY        | VARCHAR2(32) | 授权码    |
| COSTTIME   | NUMBER(16，3)| 响应耗时  |

2.访问控制模块主要HTTP响应状态码说明--ngx_http_access_control_module.c  

| 状态码 |  说明 |
|  ---   |  ---  |
|  200   |  正常 |
|  201   |  正常，但查询结果为空 |
|  400   |  非法访问，bad request，一般为URI和SID不正确|
|  401   |  非法访问，Authorization Required，key不正确 |
|  403   |  非法访问，Forbidden，对指定资源没有访问权限 |
|  404   |  非法访问，not found，请求的资源部存在，一般为缺少?标示符 |
|  405   |  非法访问，Method not allowed，使用不支持的http方法访问(服务仅支持get和post请求) |
|  500   |  服务端异常，解析post请求体为空 |
|  511   |  服务端异常，客户端请求超出配置并发数，服务端拒绝响应 |
