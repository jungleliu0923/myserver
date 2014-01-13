/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file socket.cpp
 * @author liujun05(com@baidu.com)
 * @date 2014/01/11 19:55:24
 * @brief 
 *  
 **/

#include "mylog.h"
#include "myserver_define.h"
#include "socket.h"

int set_socket_non_blocking (int sockek_fd)
{
    int flags, s;

    flags = fcntl (sockek_fd, F_GETFL, 0);
    if (flags == -1)
    {
        MY_LOG_WARNNING("fcntl error");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sockek_fd, F_SETFL, flags);
    if (s == -1)
    {
        MY_LOG_WARNNING("fcntl error");
        return -1;
    }
    return 0;
}


int create_server_socket(int port, uint32 backlog)
{
    int socket_fd;
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( 0 > socket_fd)
    {
        MY_LOG_WARNNING("create socket fail");
        return -1;
    }

    int opt=1;
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if( 0 > bind(socket_fd, (sockaddr *) &serveraddr, sizeof(serveraddr)))
    {
        MY_LOG_WARNNING("bind socket fail");
		return -1;
    }
    if ( 0 > set_socket_non_blocking(socket_fd) )
    {
        MY_LOG_WARNNING("set set_socket_non_blocking fail");
        return -1;
    }

    if( 0 > listen(socket_fd, backlog))
    {
        MY_LOG_WARNNING("listen fail");
        return -1;
    }
    return  socket_fd;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
