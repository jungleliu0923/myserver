/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver_define.h
 * @author liujun05(liujun0923@zju.edu.cn)
 * @date 2014/01/05 16:46:13
 * @brief 
 *  
 **/


#ifndef  __MYSERVER_DEFINE_H_
#define  __MYSERVER_DEFINE_H_

#include "queue.h"
#include "pthread.h"

#ifndef UNSIGNED_LONG
#define uint32 unsigned int
#endif

#ifndef UNSIGNED_LONG_LONG
#define uint64 unsigned long long
#endif

#define MAX_FILE_NAME_LEN 1024
#define MAX_CONF_ITEM_LEN 1024

//回调函数
typedef int (* callback_proc)();

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
    uint32 backlog;             //服务器监听
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
    pthread_t tmain;                //线程生产线程
    pthread_t* threads;             //线程池
    pthread_mutex_t mutex_ready;    //线程互斥锁
    pthread_cond_t cond_ready;      //线程触发锁
    link_queue_t* queue;
    int server_fd;                  //服务器监听socket
    bool is_run;                    //服务器是否运行
    my_server_conf_t* server_conf;  //服务器配置
    callback_proc user_callback;    //用户自定义回调
} my_server_t;

#define SERVER_ERROR_UNKNOW -1
#define SERVER_WRITE_SIZE_TOO_LARGE -2

#define WRITE_ERRNO "errno"
#define WRITE_DATA "data"

#endif  //__MYSERVER_DEFINE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
