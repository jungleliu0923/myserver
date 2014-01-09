/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file myserver_define.h
 * @author liujun05(liujun0923@zju.edu.cn)
 * @date 2014/01/05 16:46:13
 * @brief 
 *  
 **/


#ifndef  __MYSERVER_DEFINE_H_
#define  __MYSERVER_DEFINE_H_

#ifndef UNSIGNED_LONG
#define uint32 unsigned int
#endif

#ifndef UNSIGNED_LONG_LONG
#define uint64 unsigned long long
#endif

#define MAX_FILE_NAME_LEN 1024
#define MAX_CONF_ITEM_LEN 1024


#define SERVER_ERROR_UNKNOW -1
#define SERVER_WRITE_SIZE_TOO_LARGE -2

#define WRITE_ERRNO "errno"
#define WRITE_DATA "data"

#endif  //__MYSERVER_DEFINE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
