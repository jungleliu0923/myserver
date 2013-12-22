/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file pool.h
 * @author liujun05(com@baidu.com)
 * @date 2013/12/22 15:43:35
 * @brief 
 *  
 **/




#ifndef  __POOL_H_
#define  __POOL_H_

#include "myserver_define.h"
#include <sys/epoll.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <queue>
using namespace std;

/** epoll 数据 */
typedef struct _epoll_sock_item_t
{
    int sock;                               //当前执行的socket fd
    int status;                             //当前运行状态  
    in_addr_t ip;                           //client ip
} epoll_sock_item_t, *epoll_sock_item;

/* epoll 数据 */
typedef struct _my_server_pool_t
{
    int epfd;                               //epoll fd
    struct epoll_event *ep_events;          //epoll events
    epoll_sock_item sockets;                //epoll 数据
    uint32 pool_size;                       //接受队列大小
    uint32 pool_use_size;                   //队列使用数目
    uint32 timeout;                         //epoll的超时时间
    queue<int> s_queue;                     //数据队列
    pthread_mutex_t ready_mutex;            //数据准备mutex
    pthread_cond_t ready_cond;              //开始执行消费过程的mutex_cond
    pthread_t main;                         //生产接受函数产生队列
} my_server_epoll_t, *my_server_epoll;


/**   线程池数据 */
typedef struct _my_server_thread_data_t
{
    pthread_t pid;                          //thread_id
    int fd;                                 //线程fd
    void* read_buf;                         //读buf
    uint32 read_size;                       //读大小
    uint32 cur_read_size;                   //实际读取大小
    void* write_buf;                        //写buf 
    uint32 write_size;                      //写大小
    uint32 cur_write_size;                  //世界写大小

    void* par_server;                       //当前服务器

} my_server_thread_data_t, *my_server_thread_data;

int my_server_handle(my_server_thread_data data);
void* pool_queue_consumer(void *param);
void* pool_queue_produce(void *param);
int pool_pop(my_server_epoll pool,int *idx);
int pool_del(my_server_epoll pool, int idx);
int pool_add(my_server_epoll pool, int idx);
int epoll_del(my_server_epoll pool, int idx);
int epoll_add(my_server_epoll pool, int sock);
int epoll_produce(void* server,  struct sockaddr *addr, socklen_t *addrlen);

#endif  //__POOL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
