/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file pool.cpp
 * @author liujun05(com@baidu.com)
 * @date 2013/12/22 15:46:49
 * @brief 
 *  
 **/


#include "myserver.h"

/*
 * 将数据放入队列
 */
void* pool_queue_produce(void *param)
{
    my_server server = (my_server)param;
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    while(server->is_run)
    {
        epoll_produce((void*)server, (struct sockaddr *)&caddr, &clen);
    }
}

/*
 * epoll生成
 */
int epoll_produce(void* server_void,  struct sockaddr *addr, socklen_t *addrlen)
{
    my_server server = (my_server) server_void;
    int ret = 0;
    my_server_epoll pool = server->pool;
    int num = epoll_wait(pool->epfd, pool->ep_events, pool->pool_size, pool->timeout);
    for(int i=0; i<num; i++)
    {
        int idx = pool->ep_events[i].data.fd;
        //接受数据
        if (pool->ep_events[i].data.fd == 0) 
        {
            int sock_cli = accept(server->server_sock, (struct sockaddr *)&addr,  addrlen);
            epoll_add(pool, sock_cli);
        }
        else if(pool->ep_events[i].data.fd > 0)
        {
            //如果可读
            if (pool->ep_events[i].events & EPOLLIN) 
            {
                //将句柄加入到就绪队列
                pool_add(pool, idx);
                //将句柄从epoll中移除
                epoll_del(pool, idx);
            }
        }
        else //发生错误
        {
            pool_del(pool, idx);
            pool->pool_use_size --;
        }
    }
}

/*
 * epoll add
 */
int epoll_add(my_server_epoll pool, int sock)
{
    int idx = -1;
    for (int i=1; i<(int)pool->pool_size; ++i) {
        if (pool->sockets[i].status == IDLE) {
            idx = i;
            break;
        }
    }
    if (idx < 0) 
    {
        cout << "pool size little\n";
        return -1;
    }

    pool->sockets[idx].status = READY;
    pool->sockets[idx].sock = sock;
    pool->pool_use_size++;
    
    struct epoll_event ev;
    ev.data.fd = idx;
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;

    if (epoll_ctl(pool->epfd, EPOLL_CTL_ADD, sock, &ev) < 0) {
        return -1;
    }
    return idx;
}

/*
 * epoll del
 */
int epoll_del(my_server_epoll pool, int idx)
{
	struct epoll_event ev;
	if (pool->epfd < 0) {
		cout << "invalid epoll fd %d" << pool->epfd << endl;
		return -1;
	}
	ev.data.fd = idx;
	ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
	int sock = pool->sockets[idx].sock;

	if (epoll_ctl(pool->epfd, EPOLL_CTL_DEL, sock, &ev) < 0) {
		//U_WARN( "epoll_ctl delete socket %d failed. %m, %d, index %d", sock, errno, idx);
		return -1;
	}
	return 0;
}

/*
 *pool add
 */
int pool_add(my_server_epoll pool, int idx)
{
    pthread_mutex_lock(&pool->ready_mutex);
    pool->s_queue.push(idx);
    pool->sockets[idx].status = BUSY;
    pthread_cond_signal(&pool->ready_cond);
    pthread_mutex_unlock(&pool->ready_mutex); 
    return 0;
}

/*
 * pool del
 */
int pool_del(my_server_epoll pool, int idx)
{
    if( pool->sockets[idx].sock > 0){
        close(pool->sockets[idx].sock);
    }
    pool->sockets[idx].status = IDLE;
    pool->sockets[idx].sock = -1;
    return 0;
}

/*
 * pool pop
 */
int pool_pop(my_server_epoll pool,int *idx)
{
    struct timeval now;
    struct timespec timeout;

    pthread_mutex_lock(&pool->ready_mutex);
    while( pool->s_queue.size() == 0){
        gettimeofday(&now, 0);
        timeout.tv_sec = now.tv_sec + 5;
        timeout.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&pool->ready_cond, &pool->ready_mutex, &timeout);
    }

    int cur_index = pool->s_queue.front();
    pool->s_queue.pop();
    *idx = cur_index;
    pthread_mutex_lock(&pool->ready_mutex);
    return *idx;
}

/*
 * 消费队列
 */
void* pool_queue_consumer(void *param)
{    
    my_server_thread_data data = (my_server_thread_data)param;
    //获取sever
    my_server server = (my_server) data -> par_server;
    int idx;
    int ret;

    while(server->is_run)
    {
        if( pool_pop(server->pool, &idx) != 0){
            continue;
        }
        data->fd = server->pool->sockets[idx].sock;
        //data->ip = server->pool->sockets[idx].ip;
        int ret = my_server_handle(data);
        if(ret == 0)
        {
            epoll_del(server->pool, idx);
            pool_del(server->pool, idx);
        }
    }

    pthread_exit(NULL);
    return NULL;
}


/*
 * 队列中的执行函数
 */
int my_server_handle(my_server_thread_data data)
{
    //读取数据
    if(data->cur_read_size = recv(data->fd, data->read_buf, data->read_size, 0) == -1 && errno != EAGAIN )
    {
        return -1;
    }

    //自定义函数
    my_server server = (my_server) data -> par_server;
    int ret = server->my_server_callback();

    if( data->cur_write_size > 0){
        //写回
        if ( send(data->fd , data->write_buf, data->cur_write_size, 0) == -1 && (errno != EAGAIN) && (errno != EWOULDBLOCK) ) 
        {
            return -1;
        }
    }
    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
