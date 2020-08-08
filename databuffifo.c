#include "databuffifo.h"

 
Queue *queue_create(void) //init 一个空队列
{
    Queue *handle = (Queue *)malloc(sizeof(Queue));
    if (NULL == handle) {
        //printf("malloc handle fail ! \n");
        write_log(ERRLOG,"malloc handle fail\n");
        return NULL;
    }
 
    handle->front = handle->rear = 0; //队首和队尾同时指向队列第一个元素
 
    return handle;
}
 
int is_queue_empty(Queue *handle)
{
    if (NULL == handle) {
        return -1;
    }
 
    return (handle->front == handle->rear ? 1 : 0); //空为1，否则为0
}
 
int is_queue_full(Queue *handle)
{
    if (NULL == handle) {
        return -1;
    }
 
    return ((handle->rear + 1) % N == handle->front ? 1 : 0); //满为1，否则为0
}
 
int en_queue(Queue *handle, data_t data)
{
    if (handle == NULL) {
        return -1; 
    }   
    if (is_queue_full(handle) == 1) { //满数据不能入队
        return -1;
    }   
    printf("en_queue1:front=%d,rear=%d\n",handle->front,handle->rear);

    //strcpy(handle->queue_data[handle->rear] ,data); //添加元素到队尾指向的位置***************************************************
    //handle->queue_data[handle->rear]=data;
    memcpy(handle->queue_data[handle->rear],data,4);
    printf("en_queue2:front=%d,rear=%d\n",handle->front,handle->rear);

    handle->rear = (handle->rear + 1) % N;
    
    
    //printf("uart en_queue success\n");
    return 0;
}
 
int de_queue(Queue *handle, data_t *data)
{
    if (NULL == handle) {
        //printf("de_queue handle = NULL\n");
        write_log(ERRLOG,"error: de_queue handle = NULL\n");
        return -1;
    }
 
    if (is_queue_empty(handle) == 1) { //空队不能出队
        //printf("error:de_queue queue_empty\n");
        write_log(ERRLOG,"error:de_queue queue_empty\n");
        return -2;
    }
 
    if (data != NULL) {
        printf("de_queue1:front=%d,rear=%d\n",handle->front,handle->rear);  
        //strcpy(*data ,handle->queue_data[handle->front]); //传队首元素的值出去
        memcpy(*data ,handle->queue_data[handle->front],4); //传队首元素的值出去
        printf("de_queue2:front=%d,rear=%d\n",handle->front,handle->rear);  
    }
    handle->front = (handle->front + 1)%N;
    /*if((handle->front)==N){
         handle->front=0;
    }*/

    
 
    return 0;
}
 
int destroy_queue(Queue *handle)  //销毁
{
    if (NULL != handle) {
        free(handle);
    }
 
    return 0;
}