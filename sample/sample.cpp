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
#include <signal.h>
#include <iostream>
using namespace std;

my_server_t* server;

int my_callback()
{
    uint32 read_size = my_server_get_read_size();
    //cout << "read_size is " << read_size << endl;

    char* read_data = (char*)my_server_get_read_buf();
    //cout << "read buf is " << read_data << endl;
   
    char* write_data = (char*)my_server_get_write_buf();
    uint32 write_size = my_server_get_write_size();

    write_size = snprintf(write_data, write_size, "input is [%s], return succ", read_data);
    my_server_set_write_size(write_size);

    return 0;
}

void signal_process(int sig_no)
{
    cout << "closing server\n";
    my_server_close(server);
    cout << "colsed server\n";
}

int main()
{
    my_log_init("./log", "sample.log", "sample.log.wf", 16);
    signal(SIGINT,signal_process);
    server = my_server_create("./conf/", "myserver.conf", "sample");
    if(server == NULL)
    {
        cout << "create sever fail\n";
        return -1;
    }
    my_server_set_callback(server, my_callback);
    my_server_run(server);
    return 0;
}



/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
