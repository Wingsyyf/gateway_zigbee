#ifndef __GATEWAY_H__
#define __GATEWAY_H__
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>  

#define  UPDATE_DATA     0xF1
#define  RUNLOG          "run-log.txt"
#define  ERRLOG          "err-log.txt"


void* uart_pthread_func(void *arg);
void* port_pthread_func(void *arg);
void* port1_pthread_func(void *arg);
void* port2_pthread_func(void *arg);
int write_log(char* fp, char* str);

int write_run_info(char uart_str_eqpnum,char uart_str_eqpstate,char uart_str_stepkey0,char uart_str_stepkey1);

#endif