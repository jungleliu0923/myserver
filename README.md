mysever是一个基于epool模型的异步通信、返回结果为JSON的高性能服务器。它使用C++编写，需要通过[mylog](http://www.oschina.net/p/mylog)以及[myconf](http://www.oschina.net/p/myconf)这二者做编译依赖。和外界交互为JSON格式(使用开源库[repidjson](http://code.google.com/p/rapidjson/))。用户只需要配置好服务器，开始服务器，然后写通过用户自定义回调函数就可以轻松搭建一个高性能服务器。<br>

# 1. 安装(mylog, myconf, mysever如果放在同一目录不需要修改makefile.env)<br>
## a. 下载编译依赖mylog<br>
$ git clone https://github.com/jungleliu0923/mylog (下载)<br>
$ cd mylog && make (编译)<br> 

## b. 下载编译依赖myconf<br>
$ git clone https://github.com/jungleliu0923/myconf(下载)<br>
$ cd myconf && make (编译请根据myconf说明指定相应的mylog路径)<br>

## c. 下载myserver<br>
$ git clone https://github.com/jungleliu0923/myserver(下载)<br>
$ cd myserer<br> 
$ make (编译, 在文件mysever/makefile.env指定mylog以及myconf的PATH)<br>
$ cd sample && make (编译范例)<br>

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

## b. 服务器代码sample.cpp
```
#include "myserver.h"
#include <unistd.h>
#include <iostream>
using namespace std;


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

int main()
{
    my_log_init("./log", "sample.log", "sample.log.wf", 16);
    my_server_t* server = my_server_create("./conf/", "myserver.conf", "sample");
    if(server == NULL)
    {
        cout << "create sever fail\n";
        return -1;
    }
    my_server_set_callback(server, my_callback);
    my_server_run(server);
    sleep(10);
    my_server_close(server);
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
## d. 执行结果
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
### 客户端结果
```
liujun05@cq01-rdqa-dev012.cq01:~/test/myserver/sample$ php client.php 
array(2) {
  ["errno"]=>
  int(0)
  ["data"]=>
  string(34) "input is [test input], return succ"
}
```
