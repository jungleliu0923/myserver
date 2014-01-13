/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file util.cpp
 * @author liujun05(com@baidu.com)
 * @date 2014/01/12 23:26:24
 * @brief 
 *  
 **/

#include "util.h"

/* 线程处理回调 */
void my_server_process_writeback(user_thread_data_t* data, int user_app_ret, uint32 max_write_size)
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
