/***************************************************************************
 * 
 * Copyright (c) 2013 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver_define.h
 * @author liujun05(com@baidu.com)
 * @date 2013/12/22 15:47:26
 * @brief 
 *  
 **/




#ifndef  __MYSERVER_DEFINE_H_
#define  __MYSERVER_DEFINE_H_

#include<iostream>
using namespace std;

#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#ifndef UNSIGNED_LONG
#define uint32 unsigned int
#endif

#ifndef UNSIGNED_LONG_LONG
#define uint64 unsigned long long
#endif


#define LISTEN_MAX 200
#define SERVER_NAME_LEN 1024
#define THREAD_MAX 20
#define SERVER_IP "127.0.0.1"

#define IDLE 0
#define READY  1
#define BUSY 2

typedef int (* callback_func)();


#endif  //__MYSERVER_DEFINE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
