/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file util.h
 * @author liujun05(com@baidu.com)
 * @date 2014/01/12 23:25:26
 * @brief 
 *  
 **/




#ifndef  __UTIL_H_
#define  __UTIL_H_


#include "mylog.h"
#include "myserver_define.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

void my_server_process_writeback(user_thread_data_t* data, int user_app_ret, uint32 max_write_size);

#endif  //__UTIL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
