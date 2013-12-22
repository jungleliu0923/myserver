/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver.cpp
 * @author liujun05(com@baidu.com)
 * @date 2013/12/20 11:46:16
 * @brief 
 *  
 **/


#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<sys/errno.h>
#include<sys/socket.h>

#include "myserver.h"



int my_sever_create_fd(my_server server)
{
    struct sockaddr_in addr4; //IPv4地址结构
    socklen_t addrlen; //地址信息长
    struct linger optval1;
    bzero(&addr4, sizeof(addr4));
    inet_pton(AF_INET, SERVER_IP, &(addr4.sin_addr));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(server->listen_port);
    server->server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > server->server_sock ) 
    {
        cout << "create socket fail\n";
        return -1;
    }
    
    //设置SO_REUSEADDR选项(服务器快速重起)
    unsigned int optval = 1;
    setsockopt(server->server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    optval1.l_onoff = 1;
    optval1.l_linger = 60;
    setsockopt(server->server_sock, SOL_SOCKET, SO_LINGER, &optval1, sizeof(struct linger));

    if (0 > bind(server->server_sock, (struct sockaddr *)&addr4, sizeof(addr4))) {
        cout << "bind fail\n";
        return -1;
    }

    if (0 > listen(server->server_sock, server->back_log)) {
        cout << "listen fail\n";
        return -1;
    }
    return 0;
}

int my_server_init_thread_data(my_server server)
{
    if(NULL == server->thread_data)
    {
        server->thread_data = (my_server_thread_data) malloc(sizeof(my_server_thread_data_t) * server->pthread_num);
    }
    memset(server->thread_data, 0, sizeof(my_server_thread_data_t)* server->pthread_num);
    for(int i=0; i<server->pthread_num; i++)
    {
        server->thread_data[i].par_server = (void*) server;
    }
    return 0;
}

int my_server_init_pool(my_server server)
{
    if(NULL == server->pool)
    {
        server->pool = (my_server_epoll)malloc(sizeof(my_server_epoll_t) );
    }
    memset(server->pool, 0, sizeof(my_server_epoll_t));
    pthread_mutex_init(&server->pool->ready_mutex, NULL);
    pthread_cond_init(&server->pool->ready_cond, NULL);

    //server pool
    server->pool->pool_size = server->pool_size;
    server->pool->sockets = (epoll_sock_item) malloc(sizeof(epoll_sock_item_t) * server->pool_size);
    server->pool->ep_events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * server->pool_size);

    server->pool->epfd = epoll_create(server->pool_size);
    if(server->pool->epfd < 0)
    {
        cout << "epoll_create fail\n";
        return -1;
    }
    
    //server pool user数目为0
    server->pool->pool_use_size = 0;
    
    server->pool->timeout = server->timeout;

    return 0;
}


my_server my_server_create(const char* path, const char* file_name, const char* server_name)
{
    my_server server = (my_server) malloc(sizeof(my_server_t));
    if(server == NULL)
    {
        cout << "create server fail\n";
        return NULL;
    }
 
    strncpy(server->server_name, server_name, sizeof(server->server_name));
    server->pthread_num = 10;
    server->listen_port = 2000;
    server->back_log = 2048;
    server->is_run = true;
    server->pool_size = 200;

    int ret = -1;
    ret = my_sever_create_fd(server);
    ret = my_server_init_thread_data(server);
    ret = my_server_init_pool(server);
    return server;
}



int my_server_run(my_server server)
{
    int i;
    int ret;
    //初始化接受线程(将数据放入队列)
    ret = pthread_create(&server->pool->main, NULL, pool_queue_produce, server);
    //初始化处理线程(消费队列)
    for(i=0; i<server->pthread_num; i++)
    {
        ret = pthread_create(&server->thread_data[i].pid, NULL, pool_queue_consumer, &server->thread_data[i]);
    }
    //join
    pthread_join(server->pool->main, NULL);
    for(i=0; i<server->pthread_num; i++)
    {
        pthread_join(server->thread_data[i].pid, NULL); 
    }
    return 0;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
