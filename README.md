myserver是一个基于epoll模型的异步通信、返回结果为JSON的高性能服务器。它使用C++编写，需要通过[mylog](http://www.oschina.net/p/mylog)以及[myconf](http://www.oschina.net/p/myconf)这二者做编译依赖。和外界交互为JSON格式(使用开源库[repidjson](http://code.google.com/p/rapidjson/))。用户只需要配置好服务器，开始服务器，然后写通过用户自定义回调函数就可以轻松搭建一个高性能服务器。<br>

# 1. 安装
## $ sh install.sh
会自动下载mylog以及myconf<br>

# 2. API<br>
## a. 创建一个服务器<br>
```
my_server_t* my_server_create(const char* conf_path, const char* conf_file, const char* server_name);
```
conf_path: 服务配置路径<br>
conf_file: 服务器配置文件<br>
server_name : webserver名称<br>

### 配置说明如下
```
_server_sample_server_port ： 服务器监听端口, 默认2000<br>
_server_sample_listen_back : 监听backlog，默认20
_server_sample_pool_size : 线程池大小，默认200
_server_sample_thread_num ：线程数，默认10
_server_sample_timeout ：epool超时，默认0
```

## b. 设置线程回调函数
```
int my_server_set_callback(my_server_t* server, callback_proc call_func);
```
server : 运行的服务器<br>
call_func : 自定义的回调函数，返回值必须是int<br>

## c. 运行一个服务器
```
int my_server_run(my_server_t* server);
```

## d. 获得线程读大小
```
uint32 my_server_get_read_size();
```

## e. 获得线程读数据
```
void* my_server_get_read_buf();
```

## f. 获得线程最大写大小
```
uint32 my_server_get_write_size();
```

## g. 获得线程写数据
```
void* my_server_get_write_buf();
```

## h. 设置线程实际返回写数据大小
```
int my_server_set_write_size(uint32 write_size);
```
这个必须有。<br>

## i. 关闭服务器
```
int my_server_close(my_server_t* server);
```
这个必须得有。<br>

# 3 范例（sample）
## a. 配置文件
```
_server_sample_cut_off_len : 20000
_server_sample_server_port : 2000
_server_sample_listen_back : 20
_server_sample_pool_size : 200
_server_sample_thread_num : 10
_server_sample_timeout : 0
```
## Makefile
```
MYLOG_PATH=$(WORKROOT)/mylog/output/
MYCONF_PATH=$(WORKROOT)/myconf/output/
MYSERVER_PATH=$(WORKROOT)/myserver/output/
MYSERVER_PRO_PATH=$(WORKROOT)/myserver/
MY_SERVER_THIRD=$(MYSERVER_PRO_PATH)/third/
RAPID_JSON_PATH=$(MY_SERVER_THIRD)/rapidjson/
```
WORKROOT 在Makefile里面有指定<br>

## b. 服务器代码sample.cpp（CTRL+C关闭）
```
#include "myserver.h"
#include <unistd.h>
#include <signal.h>
#include <iostream>
using namespace std;

my_server_t* server;

int my_callback()
{
    uint32 read_size = my_server_get_read_size();
    //cout << "read_size is " << read_size << endl;

    char* read_data = (char*)my_server_get_read_buf();
    //cout << "read buf is " << read_data << endl;
   
    char* write_data = (char*)my_server_get_write_buf();
    uint32 write_size = my_server_get_write_size();

    write_size = snprintf(write_data, write_size, "input is [%s], return succ", read_data);
    my_server_set_write_size(write_size);

    return 0;
}

void signal_process(int sig_no)
{
    cout << "closing server\n";
    my_server_close(server);
    cout << "colsed server\n";
}

int main()
{
    my_log_init("./log", "sample.log", "sample.log.wf", 16);
    signal(SIGINT,signal_process);
    server = my_server_create("./conf/", "myserver.conf", "sample");
    if(server == NULL)
    {
        cout << "create sever fail\n";
        return -1;
    }
    my_server_set_callback(server, my_callback);
    my_server_run(server);
    return 0;
}

```
## c. 客户端代码(php) client.php
```
<?php
$address = '127.0.0.1';
$port = 2000;

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if($socket == NULL){
    echo "socket create fail\n";
}
@$result = socket_connect($socket, $address, $port);
if($result == false){
    echo "socket connect fail\n";
    exit(1);
}

$in = "test input";
socket_write($socket, $in, strlen($in));

$out = socket_read($socket, 8192);
$res = json_decode($out,true);
var_dump($res);

socket_close($socket);
```
## d.客户端(c，可以调整并发)
```
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include<arpa/inet.h>

#include <iostream>
#include <pthread.h>
using namespace std;

#define MAX_THREAD_NUM 7
#define RECV_LEN 1024

typedef struct cfg_main_t
{
	char host[1024];
	int port;
}cfg_main;

cfg_main g_cfg;

void* thread_test(void* args)
{
    int sockfd;
    struct sockaddr_in dest_addr; /* will hold the destination addr */
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cout << "create socket error\n";
        return NULL;
    }
    
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(g_cfg.port);
    dest_addr.sin_addr.s_addr = inet_addr(g_cfg.host); 
    bzero(&(dest_addr.sin_zero), 8); 

    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("connect fail with server_port[%s:%d]", g_cfg.host, g_cfg.port);
        return NULL;
    }
    
    char *sendmsg = "消息内容";
    char recv[RECV_LEN];
    if( write(sockfd, sendmsg, strlen(sendmsg) + 1) < 0 )
    {
        cout << "send messgage fail\n";
        return NULL;
    }

    if( read(sockfd, recv, RECV_LEN) < 0 )
    {
        cout << "recv fail\n";
    }
    cout << "read data is " <<  recv << endl;


    close(sockfd);
    pthread_exit(NULL);    
}

void show_usage()
{
	cout << "usage: ./client -h [host](default:127.0.0.1) -p [port]" << endl;
}

/** 解析参数 */
int main_parse_option(int argc, char **argv)
{
    int c;
    //reset 获取参数的位置，多次调用时这个会出现问题
    while ((c = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (c)
        {
        case 'h':
            sprintf(g_cfg.host, optarg);
            break;
        case 'p':
			g_cfg.port = atoi(optarg);
            break;
        default:
			show_usage();
            fflush(stdout);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if( 0 != main_parse_option(argc, argv) )
	{
		return -1;
	}

    if( strlen(g_cfg.host) == 0 ){
        strncpy(g_cfg.host, "127.0.0.1", 1024);
    }

    if( g_cfg.port <= 0){
        show_usage();
        return -1;
    }

    pthread_t th[MAX_THREAD_NUM];
    for(int i=0; i<MAX_THREAD_NUM; i++)
    {
        pthread_create(&th[i], NULL, thread_test, (void*)argv);
    }
    
    for(int i=0; i<MAX_THREAD_NUM; i++)
    {
        pthread_join(th[i], NULL);
    }
    return 0;
}
```

## e. 执行结果
### 服务端日志
```
==> sample.log.wf <==
WARNING: 2014-01-09 16:03:56 [myconf.cpp:268] logid[0] reqid[] mod[sample] exec_time[281(us)] conf key[_server_sample_read_size] is empty , return default value 100
WARNING: 2014-01-09 16:03:56 [myconf.cpp:268] logid[0] reqid[] mod[sample] exec_time[292(us)] conf key[_server_sample_write_size] is empty , return default value 100

==> sample.log <==
TRACE: 2014-01-09 16:04:05 [myserver.cpp:343] logid[7] reqid[127.0.0.1] mod[sample] exec_time[8900489(us)] write data is {"errno":0,"data":"input is [test input], return succ"}

==> sample.log.wf <==
WARNING: 2014-01-09 16:04:05 [myserver.cpp:457] logid[7] reqid[127.0.0.1] mod[sample] exec_time[8901267(us)] client close connect
```
### f.客户端结果（php）
```
liujun05@cq01-rdqa-dev012.cq01:~/test/myserver/sample$ php client.php 
array(2) {
  ["errno"]=>
  int(0)
  ["data"]=>
  string(34) "input is [test input], return succ"
}
```
## g.客户端(c)
```
liujun05@cq01-rdqa-dev012.cq01:~/myserver/sample_client$ ./client -p2000
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
read data is {"errno":0,"data":"input is [消息内容], return succ"}
```
