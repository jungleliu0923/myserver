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

void* epoll_main(void *param);
void* epoll_process(void* param) ;

/* 获得线程读大小 */
uint32 lj_server_get_read_size();

/* 获得线程读数据 */
void* lj_server_get_read_buf();

/* 获得线程最大写大小*/
uint32 lj_server_get_write_size();
/* 获得线程最大写数据 */
void* lj_server_get_write_buf();

/* 设置线程实际返回写数据大小 */
int lj_server_set_write_size(uint32 write_size);


#endif  //__EPOLLSERVER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
