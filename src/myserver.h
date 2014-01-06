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

#include <iostream>
using namespace std;

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
    int fd;
    uint32 read_size;
    char* read_data;
    uint32 write_size;
    char* write_data;
    char reqip[MAX_FILE_NAME_LEN]; //连接的ip
}user_thread_data_t;

typedef struct _my_server_conf_t
{
    uint32 cut_off_len;
    uint32 server_port;
    uint32 listen_back;
    uint32 pool_size;
    uint32 thread_num;
    int timeout;
    uint32 read_size;
    uint32 write_size;
    char server_name[MAX_FILE_NAME_LEN];
} my_server_conf_t;


typedef struct _my_server_t
{
    task_t* readhead;
    task_t* readtail;
    pthread_t tmain;
    pthread_t* threads;
    pthread_mutex_t mutex_ready;
    pthread_cond_t cond_ready;
    struct epoll_event* events;
    int epfd;
    int server_fd;
    bool is_run;
    my_server_conf_t* server_conf;
} my_server_t;

my_server_conf_t* my_server_init_conf(const char* conf_path, const char* conf_file, const char* server_name);
my_server_t* my_server_init_data(my_server_conf_t* server_conf);
my_server_t* my_server_create(const char* conf_path, const char* conf_file, const char* server_name);
int my_server_run(my_server_t* server);
int my_server_close(my_server_t* server);
int my_server_hold(my_server_t* server);
int my_server_resume(my_server_t* server);
void* epoll_main(void *args);
void* epool_handle(void *args);
void setnonblocking(int sock);

#endif  //__MYSERVER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
