/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file epollserver.h
 * @author liujun05(com@baidu.com)
 * @date 2014/01/12 16:51:54
 * @brief 
 *  
 **/




#ifndef  __EPOLLSERVER_H_
#define  __EPOLLSERVER_H_


#include <sys/epoll.h>
#include "myserver_define.h"


extern pthread_key_t g_server_key;
extern pthread_once_t g_server_once;

/* 生产着线程 */
void* epoll_main(void *param);

/* 消费者线程 */
void* epoll_process(void* param) ;


#endif  //__EPOLLSERVER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
