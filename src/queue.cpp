#include "queue.h"

bool init_queue(link_queue_t* &queue) 
{
	queue = new link_queue_t();
	if(!queue) 
	{
		return false;
	}

	queue->front = NULL;
	queue->rear  = NULL;
	return true;
}


//入队列
void en_queue(link_queue_t* &queue,ElemType e)
{
	Node* node = new Node();
	node->data = e;
	node->next = NULL;
	//如果队列为空
	if(queue->rear == NULL)
	{
		queue->front = node;
		queue->rear  = node;
	}
	else//如果队列不为空
	{
		queue->rear->next = node;
		queue->rear = node;
	}	
}

//出队列
ElemType de_queue(link_queue_t* &queue)
{
	if(queue->front == NULL)
	{
		return -1;
	}
	else
	{
		ElemType elem;
		Node* node = queue->front;
		elem = node->data;
		if(queue->front->next == NULL)
		{
			queue->front = NULL;
			queue->rear  = NULL;
		}
		else
		{
			queue->front = queue->front->next;
		}
		delete node;
		return elem;
	}
}

//获得队列头元素
ElemType queue_front(link_queue_t* &queue)
{
	if(queue->front!=NULL)
	{
		return queue->front->data;
	}
	else
	{
		return NULL;
	}
}

//判断队列空
bool empty(link_queue_t* &queue)
{
	if(queue->front == NULL)
		return true;
	else
		return false;
}

//求队列长度
int queue_size(link_queue_t* &queue)
{
	int i = 0;
	Node* p = queue->front;
	while(p!=NULL)
	{
		i++;
		p = p->next;
	}
	return i;
}

//清空队列
void clear_queue(link_queue_t* &queue)
{
	if(queue->front == NULL)
	{
		return;
	}
	else
	{
		Node* p = queue->front;
		Node* q = p->next;
		while(q)
		{
			delete p;
			p = NULL;
			p = q;
			q = q->next;
		}
		delete p;
		p = NULL;
		queue->front = NULL;
		queue->rear  = NULL;
	}
}

//销毁队列
void destory_queue(link_queue_t* &queue)
{
	clear_queue(queue);
	delete queue;
	queue = NULL;
}


