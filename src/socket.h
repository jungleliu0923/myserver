/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file socket.h
 * @author liujun05(com@baidu.com)
 * @date 2014/01/12 16:38:36
 * @brief 
 *  
 **/




#ifndef  __SOCKET_H_
#define  __SOCKET_H_

#include "myserver_define.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

int set_socket_non_blocking (int sockek_fd);
int create_server_socket(int port, uint32 backlog);

#endif  //__SOCKET_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
