/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file queue.h
 * @author liujun05(com@baidu.com)
 * @date 2014/01/13 16:08:38
 * @brief 
 *  
 **/




#ifndef  __QUEUE_H_
#define  __QUEUE_H_

#include <stdio.h>

typedef int ElemType;

struct Node
{
	ElemType data;
	struct Node* next;
};

typedef struct _link_queue_t{
	Node* front;
	Node* rear;
}link_queue_t;

//初始化队列
bool init_queue(link_queue_t* &queue);
void en_queue(link_queue_t* &queue,ElemType e);
ElemType de_queue(link_queue_t* &queue);
ElemType queue_front(link_queue_t* &queue);
bool empty(link_queue_t* &queue);
int queue_size(link_queue_t* &queue);


#endif  //__QUEUE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
