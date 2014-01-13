/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file test_cliet.cpp
 * @author liujun05(com@baidu.com)
 * @date 2014/01/10 21:10:55
 * @brief 
 *  
 **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include<arpa/inet.h>

#include <iostream>
#include <pthread.h>
using namespace std;

#define MAX_THREAD_NUM 7
#define RECV_LEN 1024

typedef struct cfg_main_t
{
	char host[1024];
	int port;
}cfg_main;

cfg_main g_cfg;

void* thread_test(void* args)
{
    int sockfd;
    struct sockaddr_in dest_addr; /* will hold the destination addr */
    if((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cout << "create socket error\n";
        return NULL;
    }
    
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(g_cfg.port);
    dest_addr.sin_addr.s_addr = inet_addr(g_cfg.host); 
    bzero(&(dest_addr.sin_zero), 8); 

    if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("connect fail with server_port[%s:%d]", g_cfg.host, g_cfg.port);
        return NULL;
    }
    
    char *sendmsg = "消息内容";
    char recv[RECV_LEN];
    if( write(sockfd, sendmsg, strlen(sendmsg) + 1) < 0 )
    {
        cout << "send messgage fail\n";
        return NULL;
    }

    if( read(sockfd, recv, RECV_LEN) < 0 )
    {
        cout << "recv fail\n";
    }
    cout << "read data is " <<  recv << endl;


    close(sockfd);
    pthread_exit(NULL);    
}

void show_usage()
{
	cout << "usage: ./client -h [host](default:127.0.0.1) -p [port]" << endl;
}

/** 解析参数 */
int main_parse_option(int argc, char **argv)
{
    int c;
    //reset 获取参数的位置，多次调用时这个会出现问题
    while ((c = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (c)
        {
        case 'h':
            sprintf(g_cfg.host, optarg);
            break;
        case 'p':
			g_cfg.port = atoi(optarg);
            break;
        default:
			show_usage();
            fflush(stdout);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if( 0 != main_parse_option(argc, argv) )
	{
		return -1;
	}

    if( strlen(g_cfg.host) == 0 ){
        strncpy(g_cfg.host, "127.0.0.1", 1024);
    }

    if( g_cfg.port <= 0){
        show_usage();
        return -1;
    }

    pthread_t th[MAX_THREAD_NUM];
    for(int i=0; i<MAX_THREAD_NUM; i++)
    {
        pthread_create(&th[i], NULL, thread_test, (void*)argv);
    }
    
    for(int i=0; i<MAX_THREAD_NUM; i++)
    {
        pthread_join(th[i], NULL);
    }
    return 0;
}





/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
