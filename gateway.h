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

pthread_mutex_t m_mutex;
pthread_mutex_t net_mutex;
pthread_mutex_t net1_mutex;

int write_log(char* fp, char* str);

#endif