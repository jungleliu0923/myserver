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

#include "socket.h"
#include "myserver.h"

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

    snprintf(conf_item, sizeof(conf_item), "_server_%s_backlog", server_name );
    MY_CONF_GET_UINT32_DEFAULT(conf_item, my_ins, server_conf->backlog, 20);

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

    server->server_fd = create_server_socket(server_conf->server_port, server_conf->backlog);
    
    if(server->server_fd < 0)
    {
        MY_LOG_FATAL("create socket fail");
        free(server);
        return NULL;
    }
    
    server->threads = (pthread_t*) malloc( sizeof(pthread_t) * server_conf->thread_num );

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
    
    init_queue(server->queue);

    pthread_create(&server->tmain, NULL, epoll_main, server);
    for(uint32 i=0;i<server->server_conf->thread_num; i++)
    {
        pthread_create(&server->threads[i], NULL, epoll_process, server);
    }

    pthread_join(server->tmain, NULL);
    for(uint32 i=0;i<server->server_conf->thread_num; i++)
    {
        pthread_join(server->threads[i], NULL);
    }
    
    return 0;
}

/* 关闭一个服务器*/
int my_server_close(my_server_t* server)
{
    server->is_run = false;
    
    for(uint32 i=0; i<server->server_conf->thread_num; i++)
    {
        pthread_cond_signal(&server->cond_ready);
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


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
