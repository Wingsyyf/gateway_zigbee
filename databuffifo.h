#ifndef __DATABUFFIFO_H__
#define __DATABUFFIFO_H__
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "gateway.h"
 
typedef char data_t[4];
#define  RUNLOG          "run-log.txt"
#define  ERRLOG          "err-log.txt"

#define N 1000 //定义队列最大存储元素数量-1 
typedef struct {
    data_t queue_data[N];
    int front;
    int rear;
}Queue;

Queue *queue_create(void );
int is_queue_empty(Queue *handle);
int is_queue_full(Queue *handle);
int en_queue(Queue *handle, data_t data);
int de_queue(Queue *handle, data_t *data);
int destroy_queue(Queue *handle);

#endif //__DATABUFFIFO_H__