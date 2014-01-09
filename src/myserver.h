/***************************************************************************
 * 
 * Copyright (c) 2014 zju && Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver.h
 * @author liujun05(liujun0923@zju.edu.cn)
 * @date 2014/01/05 16:46:13
 * @brief 
 *  
 **/


#ifndef  __MYSERVER_H_
#define  __MYSERVER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include "mylog.h"
#include "myconf.h"
#include "myserver_define.h"

//回调函数
typedef int (* callback_proc)();

//线程池任务队列结构体
typedef struct _task_t
{
    int fd; //需要读写的文件描述符
    char reqip[MAX_FILE_NAME_LEN]; //连接的ip
    struct _task_t *next; //下一个任务
} task_t;

//用于读写两个的两个方面传递参数
typedef struct _user_thread_data_t
{
    int fd;                         //传递的socket fd
    uint32 read_size;               //读的大小
    void* read_data;                //读的数据
    uint32 write_size;              //写的大小
    void* write_data;               //写的数据
    char reqip[MAX_FILE_NAME_LEN];  //连接的ip
}user_thread_data_t;

//服务器配置
typedef struct _my_server_conf_t
{
    uint32 cut_off_len;             //截断
    uint32 server_port;             //服务器端口
    uint32 listen_back;             //服务器监听
    uint32 pool_size;               //线程池大小
    uint32 thread_num;              //线程数目
    int timeout;                    //等待超时
    uint32 read_size;               //socket读大小
    uint32 write_size;              //socket写大小
    char server_name[MAX_FILE_NAME_LEN]; //服务器名字    
} my_server_conf_t;

//服务器数据
typedef struct _my_server_t
{
    task_t* readhead;               //线程头
    task_t* readtail;               //线程尾
    pthread_t tmain;                //线程生产线程
    pthread_t* threads;             //线程池
    pthread_mutex_t mutex_ready;    //线程互斥锁
    pthread_cond_t cond_ready;      //线程触发锁
    struct epoll_event* events;     //监听epool_events
    int epfd;                       //监听epfd
    int server_fd;                  //服务器监听socket
    bool is_run;                    //服务器是否运行
    my_server_conf_t* server_conf;  //服务器配置
    callback_proc user_callback;    //用户自定义回调
} my_server_t;

/* 服务器配置初始化 */
my_server_conf_t* my_server_init_conf(const char* conf_path, const char* conf_file, const char* server_name);

/* 服务器线程数据初始化 */
my_server_t* my_server_init_data(my_server_conf_t* server_conf);

/* 创建一个服务器*/
my_server_t* my_server_create(const char* conf_path, const char* conf_file, const char* server_name);

/* 运行一个服务器*/
int my_server_run(my_server_t* server);

/* 关闭一个服务器*/
int my_server_close(my_server_t* server);

/* 生产者线程 */
void* epoll_main(void *args);

/* 消费者处理线程 */
void* epool_handle(void *args);

/* 服务器设置为非阻塞 */
int set_nonblocking(int sock);

int my_server_set_callback(my_server_t* server, callback_proc call_func);

uint32 my_server_get_read_size();
void* my_server_get_read_buf();

uint32 my_server_get_write_size();
void* my_server_get_write_buf();
int my_server_set_write_size(uint32 write_size);
void my_server_process_writeback(user_thread_data_t* data, int user_app_ret, int max_write_size);

#endif  //__MYSERVER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
