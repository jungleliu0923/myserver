/***************************************************************************
 * 
 * Copyright (c) 2014 zju && Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver.cpp
 * @author liujun05(liujun0923@zju.edu.cn)
 * @date 2014/01/05 16:46:13
 * @brief 
 *  
 **/

#include "myserver.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

static pthread_key_t g_server_key;
static pthread_once_t  g_server_once=PTHREAD_ONCE_INIT;

static void gen_server_key()
{
    pthread_key_create(&g_server_key, NULL);
}


/* 服务器设置为非阻塞 */
int set_nonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        MY_LOG_WARNNING("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        MY_LOG_WARNNING("fcntl(sock,SETFL,opts)");
        return -1;
    }
    return 0;
}

/* 初始化配置文件*/
my_server_conf_t* my_server_init_conf(const char* conf_path, const char* conf_file, const char* server_name)
{
    if(NULL == conf_path || NULL == conf_file || NULL == server_name )
    {
        MY_LOG_FATAL("get server conf fail, path is null or conf_file is null or server_name is null");
        return NULL;
    }
    MY_CONF_INS* my_ins = my_conf_init(conf_path, conf_file);
    if(NULL == my_ins)
    {
        MY_LOG_FATAL("get server conf fail, path is %s, conf_file is %s", conf_path, conf_file);
        return NULL;
    }

    my_server_conf_t* server_conf = (my_server_conf_t*) malloc(sizeof(my_server_conf_t) );
    if( NULL == server_conf)
    {
        return NULL;
    }

    strncpy(server_conf->server_name, server_name, sizeof(server_conf->server_name));

    char conf_item[MAX_CONF_ITEM_LEN];
    snprintf(conf_item, sizeof(conf_item), "_server_%s_cut_off_len", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->cut_off_len, 20000);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_server_port", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->server_port, 2000);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_listen_back", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->listen_back, 20);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_pool_size", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->pool_size, 200);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_thread_num", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->thread_num, 20);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_timeout", server_name );
    MY_CONF_GET_INT32_DEFAULT(conf_item, my_ins, server_conf->timeout, 0);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_read_size", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->read_size, 100);

    snprintf(conf_item, sizeof(conf_item), "_server_%s_write_size", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->write_size, 100);

    return server_conf;
}


/* 服务器线程数据初始化 */
my_server_t* my_server_init_data(my_server_conf_t* server_conf)
{
    if( NULL == server_conf)
    {
        MY_LOG_FATAL("server_conf is NULL");
        return NULL;
    }

    my_server_t* server = (my_server_t*)malloc(sizeof(my_server_t) );
    if( NULL == server)
    {
        MY_LOG_FATAL("my_server_init_data server is NULL");
        return NULL;
    }
    
    pthread_mutex_init(&server->mutex_ready, NULL);
    pthread_cond_init(&server->cond_ready, NULL);
    
    struct sockaddr_in serveraddr;
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(server->server_fd < 0)
    {
        MY_LOG_FATAL("create socket fail");
        free(server);
        return NULL;
    }
    
    //把socket设置为非阻塞方式
    if ( -1 == set_nonblocking(server->server_fd))
    {
        free(server);
        return NULL;
    }
    //设置SO_REUSEADDR选项(服务器快速重起)
    unsigned int optval = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, 4);
    struct linger optval1;
    optval1.l_onoff = 1;
    optval1.l_linger = 60;
    setsockopt(server->server_fd, SOL_SOCKET, SO_LINGER, &optval1, sizeof(struct linger));

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(server_conf->server_port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if( 0 > bind(server->server_fd, (sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        MY_LOG_WARNNING("bind fail, port[%u]", server_conf->server_port);
        free(server);
        return NULL;
    }
   
    if(0 > listen(server->server_fd, server_conf->listen_back))
    {
        MY_LOG_WARNNING("listen fail");
        free(server);
        return NULL;
    }
    
    server->threads = (pthread_t*) malloc( sizeof(pthread_t) * server_conf->thread_num );
    server->events = (struct epoll_event*) malloc( sizeof(struct epoll_event) * server_conf->pool_size );

    server->readhead = NULL;
    server->readtail = NULL;
    server->server_conf = server_conf;
    return server;
}

/* 创建一个服务器*/
my_server_t* my_server_create(const char* conf_path, const char* conf_file, const char* server_name)
{
    my_log_set_mod(server_name);
    //初始化配置文件
    my_server_conf_t* server_conf = my_server_init_conf(conf_path, conf_file, server_name);
    if(server_conf == NULL)
    {
        return NULL;
    }

    my_server_t* server =  my_server_init_data(server_conf);
    return server;
}

/* 运行一个服务器*/
int my_server_run(my_server_t* server)
{
    server->is_run = true;
    
    for(uint32 i=0;i<server->server_conf->thread_num; i++)
    {
        pthread_create(&server->threads[i], NULL, epool_handle, server);
    }

    pthread_create(&server->tmain, NULL, epoll_main, server);

    return 0;
}

/* 关闭一个服务器*/
int my_server_close(my_server_t* server)
{
    server->is_run = false;
 
    for(uint32 i=0;i<server->server_conf->thread_num; i++)
    {
        pthread_cond_broadcast(&server->cond_ready);
        pthread_join(server->threads[i], NULL);
    }
    pthread_join(server->tmain, NULL);

    if( server-> threads != NULL)
    {
        free(server-> threads);
        server-> threads = NULL;
    }

    if( server-> events != NULL)
    {
        free(server-> events);
        server-> events = NULL;
    }

    if ( server-> readhead != NULL)
    {
        task_t* current = server->readhead;
        task_t* tmp;
        while(current != NULL)
        {
            tmp = current;
            current = current->next;
            free(tmp);
        }
    }
    
    free( server->server_conf);
    free(server);
    return 0;
}

/* 生产者线程 */
void* epoll_main(void *args)
{
    my_server_t* server = (my_server_t*) args;
    struct epoll_event ev;

    my_log_thread_init();
    my_log_set_mod(server->server_conf->server_name);
    
    int i, connfd, sockfd, nfds; 
    struct sockaddr_in clientaddr;
    task_t* new_task = NULL;
    user_thread_data_t* rdata = NULL;
    socklen_t clilen;
    
    //生成用于处理accept的epoll专用的文件描述符
    server->epfd = epoll_create(256);
    //设置与要处理的事件相关的文件描述符
    ev.data.fd = server->server_fd;
    //设置要处理的事件类型
    ev.events = EPOLLIN | EPOLLET;
    //注册epoll事件
    epoll_ctl(server->epfd, EPOLL_CTL_ADD, server->server_fd, &ev);

    while(server->is_run)
    {
        //等待epoll事件的发生
        nfds = epoll_wait(server->epfd, server->events, server->server_conf->pool_size, server->server_conf->timeout);
        //处理所发生的所有事件
        for (i = 0; i < nfds; ++i)
        {
            if (server->events[i].data.fd == server->server_fd)
            {
                connfd = accept(server->server_fd, (sockaddr *) &clientaddr, &clilen);
                if (connfd < 0)
                {
                    MY_LOG_FATAL("accept fd[%d] fail", server->server_fd);
                    return NULL;
                }
                if ( -1 == set_nonblocking(connfd) )
                {
                    continue;
                }
                //设置用于读操作的文件描述符
                ev.data.fd = connfd;
                //设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                //注册ev
                epoll_ctl(server->epfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            else if (server->events[i].events & EPOLLIN)
            {
                if ((sockfd = server->events[i].data.fd) < 0) 
                {
                    continue;
                }
                new_task = (task_t*) malloc(sizeof(task_t));
                new_task->fd = sockfd;
                new_task->next = NULL;

                if(getpeername(sockfd, (struct sockaddr*)&clientaddr, &clilen ) != 0 )
                {
                    strncpy(new_task->reqip, "UNKNOW", MAX_FILE_NAME_LEN);
                }
                else
                {
                    strncpy(new_task->reqip, inet_ntoa(*(struct in_addr *)&clientaddr.sin_addr.s_addr), MAX_FILE_NAME_LEN);
                }
                
                //添加新的读任务
                pthread_mutex_lock(&server->mutex_ready);
                if ( server == NULL )
                {
                    MY_LOG_WARNNING("myserver is closing");
                    continue;
                }

                if (server->readhead == NULL)
                {
                    server->readhead = new_task;
                    server->readtail = new_task;
                }
                else
                {
                    server->readtail->next = new_task;
                }
                //唤醒所有等待server->cond_ready条件的线程
                pthread_cond_broadcast(&server->cond_ready);
                pthread_mutex_unlock(&server->mutex_ready);
            } 
            else if (server->events[i].events & EPOLLOUT)
            {
                rdata = (user_thread_data_t*) server->events[i].data.ptr;
                sockfd = rdata->fd;
                
                my_log_set_logid(rdata->fd);
                my_log_set_reqip( rdata->reqip );
                
                //写数据
                MY_LOG_TRACE("write data is %s", rdata->write_data);
                write(sockfd, rdata->write_data, rdata->write_size);
                
                if(rdata->read_data != NULL)
                {
                    free(rdata->read_data);
                    rdata->read_data = NULL;
                }

                if(rdata->write_data != NULL)
                {
                    free(rdata->write_data);
                    rdata->write_data = NULL;
                }

                if( rdata != NULL)
                {
                    free(rdata);
                    rdata = NULL;
                }
                
                //设置用于读操作的文件描述符
                ev.data.fd = sockfd;
                //设置用于注测的读操作事件
                ev.events = EPOLLIN | EPOLLET;
                //修改sockfd上要处理的事件为EPOLIN
                epoll_ctl(server->epfd, EPOLL_CTL_MOD, sockfd, &ev);
            }
        }
    }
    pthread_exit(0);
}

/* 消费者处理线程 */
void* epool_handle(void *args)
{
    //线程数据初始化
    pthread_once(&g_server_once, gen_server_key);
    my_server_t* server = (my_server_t*) args;
    struct epoll_event ev;
    int fd = -1;
    //用于把读出来的数据传递出去
    user_thread_data_t *data = NULL;
    my_log_thread_init();
    my_log_set_mod(server->server_conf->server_name);
    
    while (server->is_run)
    {
        pthread_mutex_lock(&server->mutex_ready);
        //等待到任务队列不为空
        while (server->readhead == NULL)
        {
            if( server->is_run == false )
            {
                pthread_mutex_unlock(&server->mutex_ready);
                return NULL;
            }
            pthread_cond_wait(&server->cond_ready, &server->mutex_ready);
        }

        fd = server->readhead->fd;
        data = (user_thread_data_t*) malloc( sizeof(user_thread_data_t));
        data->write_data = NULL;
        data->read_data = NULL;
        strncpy(data->reqip, server->readhead->reqip, MAX_FILE_NAME_LEN);
        
        my_log_set_logid(fd);
        my_log_set_reqip( server->readhead->reqip );

        //从任务队列取出一个读任务
        task_t *tmp = server->readhead;
        server->readhead = server->readhead->next;
        free(tmp);
        tmp = NULL;
        pthread_mutex_unlock(&server->mutex_ready);
        data->read_data = malloc(server->server_conf->read_size);
        memset(data->read_data, 0, server->server_conf->read_size);
        pthread_setspecific(g_server_key, (void*)data);
        
        data->fd = fd;
        data->read_size = read(fd, data->read_data, server->server_conf->read_size);
        if ((int)data->read_size < 0)
        {
            if (errno == ECONNRESET)
            {
                MY_LOG_WARNNING("read ECONNRESET");
                close(fd);
            }
            else
            {
                MY_LOG_WARNNING("read data fail");
            }
            
            if( data->read_data != NULL)
            {
                free( data->read_data);
                data->read_data = NULL;
            }

            if( data->write_data != NULL)
            {
                free( data->write_data);
                data->write_data = NULL;
            }

            if (data != NULL) 
            {
                free( data);
                data = NULL;
            }
        } 
        else if (data->read_size == 0)
        {
            close(fd);
            MY_LOG_WARNNING("client close connect");
            if( data->read_data != NULL)
            {
                free( data->read_data);
                data->read_data = NULL;
            }
            
            if( data->write_data != NULL)
            {
                free( data->write_data);
                data->write_data =  NULL;
            }
            
            if (data != NULL) 
            {
                free( data);
                data = NULL;
            }
        }
        else
        {

            data->write_data = malloc(server->server_conf->write_size);
            data->write_size = server->server_conf->write_size;
            memset(data->write_data, 0,  data->write_size);

            int ret = 0;
            if( server->user_callback == NULL)
            {
                MY_LOG_WARNNING("user call back is null");
            }
            else 
            {
                ret = server->user_callback();
            } 

            my_server_process_writeback(data, ret, server->server_conf->write_size );
            
            //设置需要传递出去的数据
            ev.data.ptr = data;
            //设置用于注测的写操作事件
            ev.events = EPOLLOUT | EPOLLET;
            //修改sockfd上要处理的事件为EPOLLOUT
            epoll_ctl(server->epfd, EPOLL_CTL_MOD, fd, &ev);
        }
    }
    pthread_exit(0);
}

/* 获得线程读大小 */
uint32 my_server_get_read_size()
{
    void * ptr = pthread_getspecific(g_server_key);
    return ( (user_thread_data_t*)ptr) ->read_size;
}

/* 获得线程最大写大小*/
uint32 my_server_get_write_size()
{
    void * ptr = pthread_getspecific(g_server_key);
    //因为需要加一层壳，所以只返回实际值的0.8
    return ((user_thread_data_t*)ptr)->write_size * 4 / 5;
}

/* 获得线程读数据 */
void* my_server_get_read_buf()
{
    void * ptr = pthread_getspecific(g_server_key);
    return ((user_thread_data_t*)ptr)->read_data;
}

/* 获得线程写数据 */
void* my_server_get_write_buf()
{
    void * ptr = pthread_getspecific(g_server_key);
    return ((user_thread_data_t*)ptr)->write_data;
}

/* 设置线程实际返回写数据大小 */
int my_server_set_write_size(uint32 write_size)
{
    void * ptr = pthread_getspecific(g_server_key);
    if( NULL == ptr)
    {
        return -1;
    }
    else
    {
        user_thread_data_t* user_t_data = (user_thread_data_t*)ptr;
        user_t_data->write_size = write_size;
    }
    return 0;
}

/* 设置线程回调函数 */
int my_server_set_callback(my_server_t* server, callback_proc call_func)
{
    if(NULL == server)
    {
        return -1;
    }
    server->user_callback = call_func;
    return 0;
}

/* 线程处理回调 */
void my_server_process_writeback(user_thread_data_t* data, int user_app_ret, int max_write_size)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    document.AddMember(WRITE_ERRNO, user_app_ret , allocator);
    document.AddMember(WRITE_DATA, (char*)data->write_data , allocator);
    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);
    const char *jsonString = strbuf.GetString();
    data->write_size = strlen(jsonString);

    if( data->write_size > max_write_size )
    {
        MY_LOG_FATAL("server_conf write size[%u] less than exact size[%u]", data->write_size, max_write_size );
        rapidjson::Document document_error;
        document_error.SetObject();
        rapidjson::Document::AllocatorType& allocator_error = document_error.GetAllocator();
        document_error.AddMember(WRITE_ERRNO, SERVER_WRITE_SIZE_TOO_LARGE , allocator_error);
        document_error.AddMember(WRITE_DATA, (char*)"server_write_size too small" , allocator_error);
        rapidjson::StringBuffer strbuf_error;
        rapidjson::Writer<rapidjson::StringBuffer> writer_error(strbuf_error);
        document_error.Accept(writer_error);
        const char* jsonString_error = strbuf_error.GetString();
        memcpy(data->write_data, jsonString_error, strlen(jsonString_error) );
        data->write_size = strlen(jsonString_error);
    }
    else
    {
        memcpy(data->write_data, jsonString, strlen(jsonString) );
        data->write_size = strlen(jsonString);
    }
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
