/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file sample.cpp
 * @author liujun05(com@baidu.com)
 * @date 2014/01/05 22:06:01
 * @brief 
 *  
 **/

#include "myserver.h"
#include <unistd.h>

int main()
{
    my_log_init("./log", "sample.log", "sample.log.wf", 16);
    my_server_t* server = my_server_create("./conf/", "myserver.conf", "sample");
    if(server == NULL)
    {
        cout << "create sever fail\n";
        return -1;
    }
    my_server_run(server);
    sleep(1);
    my_server_close(server);
    return 0;
}

















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
