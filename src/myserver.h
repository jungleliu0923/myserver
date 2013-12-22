/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver.h
 * @author liujun05(com@baidu.com)
 * @date 2013/12/21 11:03:36
 * @brief 
 *  
 **/




#ifndef  __MYSERVER_H_
#define  __MYSERVER_H_


#include <pthread.h>
#include "myserver_define.h"
#include "pool.h"



/** 服务器线程数据 */
typedef struct _my_server_t
{
    char server_name[SERVER_NAME_LEN];      //服务器名字
    int pthread_num;                        //服务器初始化相册个数
    
    int server_sock;                        //服务器监听socket
    uint32 listen_port;                     //服务器监听端口
    uint32 back_log;                        //服务器监听backlog
    
    my_server_epoll pool;                   //server epoll数据
    uint32 pool_size;                       //server epoll数量
    my_server_thread_data thread_data;      //线程池数据
    uint32 timeout;                         //超时

    callback_func my_server_callback;       //用户的回调函数

    bool is_run;                            //服务器是否运行
} my_server_t, *my_server;

int my_server_run(my_server server);
my_server my_server_create(const char* path, const char* file_name, const char* server_name);
int my_server_init_pool(my_server server);
int my_server_init_thread_data(my_server server);
int my_sever_create_fd(my_server server);

#endif  //__MYSERVER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
