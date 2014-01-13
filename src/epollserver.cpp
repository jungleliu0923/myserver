/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file epollserver.cpp
 * @author liujun05(com@baidu.com)
 * @date 2014/01/12 16:38:08
 * @brief 
 *  
 **/
#include "mylog.h"
#include "socket.h"
#include "epollserver.h"
#include "util.h"
#include <pthread.h>

pthread_key_t g_server_key;
pthread_once_t  g_server_once=PTHREAD_ONCE_INIT;

static void gen_server_key()
{
    pthread_key_create(&g_server_key, NULL);
}

/* 生产者线程 */

void* epoll_main(void *param)
{
    my_server_t* server = (my_server_t*) param;
    
    my_log_thread_init();
    my_log_set_mod(server->server_conf->server_name);
    int n, i,ret;
    struct epoll_event event;
    struct epoll_event *events;
    struct sockaddr_in clientaddr;
    socklen_t clilent;

    int epfd = epoll_create(server->server_conf->pool_size);
    event.data.fd = server->server_fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl (epfd, EPOLL_CTL_ADD, server->server_fd, &event);
    events = (struct epoll_event*)calloc (server->server_conf->pool_size, sizeof(event));

    while (server->is_run)
    {
        n = epoll_wait (epfd, events, server->server_conf->pool_size, -1);
        for (i = 0; i < n; i++)
        {
            my_log_reset_start_time();
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                MY_LOG_WARNNING("epoll error");
                close (events[i].data.fd);
            }
            else if (events[i].data.fd == server->server_fd)
            {
                while (true)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept (server->server_fd, &in_addr, &in_len);
                    if (infd == -1)
                    {
                        if ((errno == EAGAIN) ||  (errno == EWOULDBLOCK))
                        {
                            break;
                        }
                        else
                        {
                            MY_LOG_WARNNING("accept fail");
                            break;
                        }
                    }

                    ret = getnameinfo (&in_addr, in_len,
                            hbuf, sizeof hbuf,
                            sbuf, sizeof sbuf,
                            NI_NUMERICHOST | NI_NUMERICSERV);
                    if (ret == 0)
                    {
                        MY_LOG_TRACE("accepted connection on descriptor %d (host=%s, port=%s)", infd, hbuf, sbuf);
                    }

                    my_log_set_logid(infd);
                    my_log_set_reqip( hbuf);

                    ret = set_socket_non_blocking (infd);
                    if (ret == -1)
                    {
                        MY_LOG_WARNNING("set_socket_non_blocking fail");
                        break;
                    }
                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    //等待读
                    ret = epoll_ctl (epfd, EPOLL_CTL_ADD, infd, &event);
                    if (ret == -1)
                    {
                        MY_LOG_WARNNING("epoll_ctl fail");
                        break;
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                if(getpeername(events[i].data.fd, (struct sockaddr*)&clientaddr, &clilent ) != 0 )
                {
                    my_log_set_reqip("UNKNOW");
                }
                else
                {
                    my_log_set_reqip( inet_ntoa(*(struct in_addr *)&clientaddr.sin_addr.s_addr) );
                }
                my_log_set_logid(events[i].data.fd);
                //放入队列
                pthread_mutex_lock(&server->mutex_ready);
                en_queue(server->queue, events[i].data.fd);
                pthread_cond_signal(&server->cond_ready);
                pthread_mutex_unlock(&server->mutex_ready);
                MY_LOG_TRACE("succ add %d", events[i].data.fd);
            }
            else
            {
                //do nothing
            }
        }
    }

    free (events);
    close (server->server_fd);
    return NULL;
}

/* 消费者线程 */
void* epoll_process(void* param)
{
    pthread_once(&g_server_once, gen_server_key);
    my_server_t* server = (my_server_t*) param;
    my_log_thread_init();
    my_log_set_mod(server->server_conf->server_name);

    user_thread_data_t *data = NULL;
    socklen_t clilent;
    struct sockaddr_in clientaddr;
    char reqip[MAX_FILE_NAME_LEN];

    while(server->is_run)
    {
        my_log_reset_start_time();
        data = (user_thread_data_t*) malloc( sizeof(user_thread_data_t));
        data->write_data = NULL;
        data->read_data = NULL;

        pthread_mutex_lock(&server->mutex_ready);
        while( empty(server->queue) )
        {
            pthread_cond_wait(&server->cond_ready, &server->mutex_ready);
        }

        int fd = de_queue(server->queue);

        pthread_mutex_unlock(&server->mutex_ready);

        my_log_set_logid(fd);
        if(getpeername( fd, (struct sockaddr*)&clientaddr, &clilent ) != 0 )
        {
            my_log_set_reqip("UNKNOW");
        }
        else
        {
            my_log_set_reqip( inet_ntoa(*(struct in_addr *)&clientaddr.sin_addr.s_addr) );
        }

        data->read_data = malloc(server->server_conf->read_size);
        memset(data->read_data, 0, server->server_conf->read_size);
        pthread_setspecific(g_server_key, (void*)data);

        data->fd = fd;
        int read_size = read(fd, data->read_data, server->server_conf->read_size);

        if (0 > read_size)
        {
            if (errno != EAGAIN)
            {
                if( data != NULL)
                {
                    free(data);
                    data = NULL;
                }
                MY_LOG_WARNNING("read error");
            }
            close (fd);
        }
        else if (read_size == 0)
        {
            MY_LOG_WARNNING("close fd %d", fd);
            if( data != NULL)
            {
                free(data);
                data = NULL;
            }
            close (fd);
        }
        else
        {
            data->read_size = read_size;
            data->write_data = malloc(server->server_conf->write_size);
            data->write_size = server->server_conf->write_size;
            memset(data->write_data, 0,  data->write_size);

            int ret = 0;
            //如果有用户回调，那么就执行回调函数
            if( server->user_callback == NULL)
            {
                MY_LOG_WARNNING("user call back is null");
            }
            else
            {
                ret = server->user_callback();
            }

            //处理结果，返回为JSON
            my_server_process_writeback(data, ret, server->server_conf->write_size );

            //写回
            int write_size = write(fd, data->write_data, data->write_size);
            if( 0 <= write_size)
            {
                MY_LOG_WARNNING("write %d fail, or clinet close", fd);
                if( data != NULL)
                {
                    free(data);
                    data = NULL;
                }
                close(fd);
            }
        }
    }
    return NULL;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
