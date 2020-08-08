#include "gateway.h"
#include "myport.h"
#include "myuart.h"
#include "databuffifo.h"


//typedef struct arg_struct ARG ;

void* uart_pthread_func(void *arg);
void* port_pthread_func(void *arg);
void* port1_pthread_func(void *arg);
void* port2_pthread_func(void *arg);

typedef struct arg_struct {
    //Queue *handle;
    pthread_mutex_t m_mutex;
    pthread_mutex_t net_mutex;
    pthread_mutex_t net1_mutex;
}arg_struct;

//pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

char *timecmd="ntpclient -s -d -c 1 -i 5 -h 120.25.108.11";

Queue *handle;


void main(int argc, char *argv[]){
    //FILE *fperror = fopen("err-log.txt","w");
    printf("main run begin\n");
    write_log(RUNLOG,"main run begin\n");
   
/*************************************************************
*--------------------FUNCTION---------------------------------
*                 * 校准时间进程 *
* 
*************************************************************/
    int tcr;
    pid_t timepid=fork();
    if(timepid<0){
        //printf("error with fork timepid\n");
        write_log(ERRLOG,"error with fork timepid\n");
    }
    else if(timepid==0){       
        while(1){
            tcr=system(timecmd);
            if(tcr==-1){
                perror("Error: ");
            }  
            sleep(TIMER_STANDARD_TIME);
	    }
    }

/*************************************************************
*--------------------FUNCTION---------------------------------
*                   * 主进程 *
* 
*************************************************************/
    else
    {
        pthread_t uart_pthread,port_pthread,port1_pthread,port2_pthread;
      
        arg_struct arg;

        handle = queue_create();

        pthread_mutex_init(&m_mutex, NULL);
        pthread_mutex_init(&net_mutex, NULL);
        pthread_mutex_init(&net1_mutex, NULL);

        if (pthread_create(&uart_pthread, NULL, (void*)uart_pthread_func, NULL) != 0) {

            write_log(ERRLOG,"uart_pthread create error\n");
            exit(EXIT_FAILURE);
        }
        if (pthread_create(&port_pthread, NULL, (void*)port_pthread_func, NULL) != 0) {

            write_log(ERRLOG,"port_pthread create error\n");
            exit(EXIT_FAILURE);
        }
        if (pthread_create(&port1_pthread, NULL, (void*)port1_pthread_func, NULL) != 0) {

            write_log(ERRLOG,"port1_pthread create error\n");
            exit(EXIT_FAILURE);
        }
        if (pthread_create(&port2_pthread, NULL, (void*)port2_pthread_func, NULL) != 0) {

            write_log(ERRLOG,"port1_pthread create error\n");
            exit(EXIT_FAILURE);
        }
        
        pthread_join(uart_pthread,NULL);
        pthread_join(port_pthread,NULL);
        pthread_join(port1_pthread,NULL);
        pthread_join(port2_pthread,NULL);

        pthread_mutex_destroy(&m_mutex); 
        pthread_mutex_destroy(&net_mutex); 
        pthread_mutex_destroy(&net1_mutex); 
    }
    
    exit(0);

}

/*************************************************************
*--------------------FUNCTION---------------------------------
*                   * 串口线程 *
* 
*************************************************************/  
void* uart_pthread_func(void *arg){
//*************(打开/读)文件用的变量*****************************
	int fduart2;
	int len;
	int readlen;
	char readbuff[256];//串口读取数据缓存
	fd_set rd;
//**********串口协议解析用的变量************************************	
	unsigned char cha[1];
	unsigned char ch;
	unsigned char check_temp;
	int uart_rdlenrightflag=0;          
	int uart_rdsuccessflag=0;           
	int uart_rd_enableflag=0;           
	int uart_rdlen=0;               
	int uart_rdlentemp=0;

/*************************************************************
*--------------------FUNCTION---------------------------------
*                   * 打开串口 *
* 
*************************************************************/           
        fduart2 = open_port();
        set_speed(fduart2,115200);
        if (set_Parity(fduart2,8,1,'N') == FALSE){
            //fprintf(stderr, "Set Parity Error\n");
            write_log(ERRLOG,"Set Parity Error\n");
            //exit(1);
        }
        //write_log(RUNLOG,"uart_pthread run\n");   
        //printf("\n");   
        //ARG * p = (ARG *) arg ;
    //*****************开始读数据******************************************
        while (1){   
            FD_ZERO(&rd);
            FD_SET(fduart2,&rd);
            while(FD_ISSET(fduart2,&rd)){
                if(select(fduart2+1,&rd,NULL,NULL,NULL) < 0){
                    //fprintf(stderr,"uart select error!\n");
                    write_log(ERRLOG,"uart select error!\n");
                }	
                else{
                    //清空readbuff
                    for(int i=0;i<256;i++){
                        readbuff[i]=0;
                    }		
                    while((read(fduart2,cha,1))>0){
                        ch=cha[0];
                        //验证数据头×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                        if(ch==0xac){
                            uart_rd_enableflag=1;
                            uart_rdlentemp++;    
                        }
                        //读数据长度×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                        else if(uart_rd_enableflag==1&&uart_rdlentemp==1){
                            uart_rdlen=ch;
                            uart_rdlentemp++;  
                        }
                        //读有效数据×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                        else if(uart_rd_enableflag==1&&uart_rdlentemp>=2&&uart_rdlentemp<=uart_rdlen+1){
                            readbuff[uart_rdlentemp-2]=ch;
                            uart_rdlentemp++;

                        }
                        //校验(长度取反)×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                        else if(uart_rd_enableflag==1&&uart_rdlentemp==uart_rdlen+2){
                            check_temp = ~uart_rdlen;
                            //printf("check_temp=%x\n",check_temp);
                            //printf("ch=%x\n",ch);
                            if(ch==check_temp){
                                uart_rdlenrightflag=1;
                                uart_rdlentemp++;
                            }
                            else{
                                uart_rdlenrightflag=0;          
                                uart_rdsuccessflag=0;           
                                uart_rd_enableflag=0;           
                                uart_rdlen=0;               
                                uart_rdlentemp=0;
                                //printf("rd ~len failed\n");	
                                write_log(ERRLOG,"rd ~len failed\n");
                            }
                        }
                        //验证数据尾×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                        else if(uart_rd_enableflag==1&&uart_rdlentemp==uart_rdlen+3){
                            if(( ch==0xef )&&( uart_rdlenrightflag==1 )){
                                uart_rdsuccessflag=1;
                                uart_rdlentemp=0;
                            }
                            else{
                                uart_rdlenrightflag=0;
                                uart_rdsuccessflag=0;
                                uart_rd_enableflag=0;
                                uart_rdlen=0;
                                uart_rdlentemp=0;
                                //printf("rd tail failed\n");
                                write_log(ERRLOG,"rd tail failed\n");
                            }
                        }
                        else{
                            uart_rdsuccessflag=0;
                            uart_rd_enableflag=0;
                            uart_rdlen=0;
                            uart_rdlentemp=0;
                            }
                    }
                    //读取数据串成功×××××××××××××××××××××××××××××××××××××××××××××××××××××××××
                    if(uart_rdsuccessflag==1){
                        uart_rdsuccessflag=0;
                        //printf("rd success\n");
                        //write_log(RUNLOG,"rd success\n");
                        if(readbuff[0]==UPDATE_DATA){
                            char readbufftemp[4]={0};
                            for(int buff_i=0;buff_i<4;buff_i++){
                                readbufftemp[buff_i]=readbuff[buff_i+1];
                            }                               
                            //临界区----------------------------------------------
                            //写入数据--------------------------------------------
                            pthread_mutex_lock(&m_mutex);
                            if(is_queue_full(handle)==0){
                                
                                printf("readbufftemp=%s \n",readbufftemp);
                                while(en_queue(handle,readbufftemp)!=0){
                                    write_log(ERRLOG,"en_queue error\n");
                                }
                                //printf("en_queue:front=%d,rear=%d\n",handle->front,handle->rear);
                                //write_log(RUNLOG,"uart_en_queue success\n");
                                printf("uart_en_queue success\n");
                                
                            }
                            else if(is_queue_full(handle)==1){
                                write_log(ERRLOG,"uart en_queue faild-queue_full\n");
                            } 
                            pthread_mutex_unlock(&m_mutex); 
                        }	
                    }
                }
            }   
        } 	
}

/*--------------------FUNCTION---------------------------------
*                   *  接口0线程 *
* 
*************************************************************/
void* port_pthread_func(void *arg){
    //printf("port_pthread_func start\n");
        //Queue *handle=(Queue *) arg;
        write_log(RUNLOG,"port0_pthread run\n");  
        arg=(arg_struct*) arg;
        //Queue *handle=(Queue *)(((arg_struct*)arg)->handle);

        
        while(1){
            char portbufftemp[4];
            char prot_step_key[2];
            int flag=0;
            //printf("net handle:%d,%d,%s\n",handle->front,handle->rear,handle->data);
            //printf("netport 1 success\n");
            //临界区----------------------------------------------
            //读取数据--------------------------------------------
            //判断出队是否完成
            //Queue *handle = (Queue *)arg;
            //ARG* p = (ARG *) arg;
            
            //printf("handle=%d,%d,%s\n",handle->front,handle->rear,handle->data);
            pthread_mutex_lock(&m_mutex);
            if(is_queue_empty(handle)==0){
                //printf("netport 2 success\n");
                
                //printf("netport 3 success\n");
                if(de_queue(handle, &portbufftemp)!=0){
                    //fprintf(stderr,"de_queue netport error\n");
                    write_log(ERRLOG,"de_queue netport error\n");
                }
                else{
                    flag=1;
                    //printf("netport 4 success\n");
                    //printf("de_queue:front=%d,rear=%d\n",handle->front,handle->rear);
                    printf("port0_pthread de_queue success--------------------------\n");
                } 
                
            }
            pthread_mutex_unlock(&m_mutex);
            //if(is_queue_empty(handle)==1){printf("netport 21 success\n");}
             //printf("netport 2 success\n");
            //信号量释放临界区
            int temp_flag=-1;
            int temp_cnt=0;
            while((temp_flag==-1)&&(flag==1)&&(temp_cnt<3)){
                prot_step_key[0]=portbufftemp[0];
                prot_step_key[1]=portbufftemp[1];
                //pthread_mutex_lock(&net_mutex);
                temp_flag=port_update0(portbufftemp[2],portbufftemp[3],portbufftemp[0], portbufftemp[1]);
                //pthread_mutex_unlock(&net_mutex);
                if(temp_flag==-1){
                    temp_cnt++;                  
                    //write_log(ERRLOG,"error with port_update\n");
                }  
                else if(temp_flag==0){
                    flag=0;
                    temp_cnt=0;
                }
                else if(temp_cnt==3){
                    write_log(ERRLOG,"json error and try 3 times\n");
                }
                             
            }
            //usleep(100000);
            //sleep(1);
            
        }
        
}

/*--------------------FUNCTION---------------------------------
*                   *  接口1线程 *
* 
*************************************************************/
void* port1_pthread_func(void *arg){
    //printf("port_pthread_func start\n");
        //Queue *handle=(Queue *) arg;
        write_log(RUNLOG,"port1_pthread run\n");  
        arg=(arg_struct*) arg;
        //Queue *handle=(Queue *)(((arg_struct*)arg)->handle);

       
        while(1){
            char portbufftemp[4];
            char prot_step_key[2];
             int flag=0;
            //printf("net handle:%d,%d,%s\n",handle->front,handle->rear,handle->data);
            //printf("netport 1 success\n");
            //临界区----------------------------------------------
            //读取数据--------------------------------------------
            //判断出队是否完成
            //Queue *handle = (Queue *)arg;
            //ARG* p = (ARG *) arg;
            
            //printf("handle=%d,%d,%s\n",handle->front,handle->rear,handle->data);
            pthread_mutex_lock(&m_mutex);
            if(is_queue_empty(handle)==0){
                //printf("netport 2 success\n");
                
                //printf("netport 3 success\n");
                if(de_queue(handle, &portbufftemp)!=0){
                    //fprintf(stderr,"de_queue netport error\n");
                    write_log(ERRLOG,"de_queue netport error\n");
                }
                else{
                    flag=1;
                    printf("port1_pthread de_queue success--------------------------\n");
                } 
                
            }
            pthread_mutex_unlock(&m_mutex);
            //if(is_queue_empty(handle)==1){printf("netport 21 success\n");}
             //printf("netport 2 success\n");
            //信号量释放临界区
            int temp_flag=-1;
            int temp_cnt=0;
            while((temp_flag==-1)&&(flag==1)&&(temp_cnt<3)){
                prot_step_key[0]=portbufftemp[0];
                prot_step_key[1]=portbufftemp[1];
                //pthread_mutex_lock(&net_mutex);
                temp_flag=port_update1(portbufftemp[2],portbufftemp[3],portbufftemp[0], portbufftemp[1]);
                //pthread_mutex_unlock(&net_mutex);
                if(temp_flag==-1){
                    temp_cnt++;                  
                    //write_log(ERRLOG,"error with port_update\n");
                }  
                else if(temp_flag==0){
                    flag=0;
                    temp_cnt=0;
                }
                else if(temp_cnt==3){
                    write_log(ERRLOG,"json error and try 3 times\n");
                }
                             
            }
            //usleep(100000);
            //sleep(1);
            
        }
        
}

/*--------------------FUNCTION---------------------------------
*                   *  接口2线程 *
* 
*************************************************************/
void* port2_pthread_func(void *arg){
    //printf("port_pthread_func start\n");
        //Queue *handle=(Queue *) arg;
        write_log(RUNLOG,"port2_pthread run\n");  
        arg=(arg_struct*) arg;
        //Queue *handle=(Queue *)(((arg_struct*)arg)->handle);

        
        while(1){
            int flag=0;
            char portbufftemp[4];
            char prot_step_key[2];
            //printf("net handle:%d,%d,%s\n",handle->front,handle->rear,handle->data);
            //printf("netport 1 success\n");
            //临界区----------------------------------------------
            //读取数据--------------------------------------------
            //判断出队是否完成
            //Queue *handle = (Queue *)arg;
            //ARG* p = (ARG *) arg;
            
            //printf("handle=%d,%d,%s\n",handle->front,handle->rear,handle->data);
            pthread_mutex_lock(&m_mutex);
            if(is_queue_empty(handle)==0){
                //printf("netport 2 success\n");
                
                //printf("netport 3 success\n");
                if(de_queue(handle, &portbufftemp)!=0){
                    //fprintf(stderr,"de_queue netport error\n");
                    write_log(ERRLOG,"de_queue netport error\n");
                }
                else{
                    flag=1;
                    printf("port2_pthread de_queue success--------------------------\n");
                }
                
            }
            pthread_mutex_unlock(&m_mutex);
            //if(is_queue_empty(handle)==1){printf("netport 21 success\n");}
             //printf("netport 2 success\n");
            //信号量释放临界区
            int temp_flag=-1;
            int temp_cnt=0;
            while((temp_flag==-1)&&(flag==1)&&(temp_cnt<3)){
                prot_step_key[0]=portbufftemp[0];
                prot_step_key[1]=portbufftemp[1];
                //pthread_mutex_lock(&net_mutex);
                temp_flag=port_update2(portbufftemp[2],portbufftemp[3],portbufftemp[0], portbufftemp[1]);
                //pthread_mutex_unlock(&net_mutex);

                if(temp_flag==-1){
                    temp_cnt++;                  
                    //write_log(ERRLOG,"error with port_update\n");
                }  
                else if(temp_flag==0){
                    flag=0;
                    temp_cnt=0;
                }
                else if(temp_cnt==3){
                    write_log(ERRLOG,"json error and try 3 times\n");
                }
                             
            }
            //usleep(100000);
            //sleep(1);
            
        }
        
}

/*--------------------FUNCTION---------------------------------
*                   *  写log   *
* 
*************************************************************/
int write_log(char* fp, char* str){

    int filepoint;
	char time_nowY[4];
	char time_nowMO[2];
	char time_nowD[2];
    char time_nowH[2];
    char time_nowMI[2];
    char time_nowS[2];
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    char time_now[20+strlen(str)];
    for(int i=0;i<20;i++){
		time_now[i]=0;
	}
    sprintf(time_nowY, "%d", tm_now->tm_year+1900);
	if((tm_now->tm_mon+1)==0){
		time_nowMO[0]='1';
		time_nowMO[1]='2';
	}
	else if((tm_now->tm_mon+1)>0&&(tm_now->tm_mon+1)<10){
		time_nowMO[0]='0';
		sprintf(&time_nowMO[1], "%d", tm_now->tm_mon+1);
	}
	else{
		sprintf(time_nowMO, "%d", tm_now->tm_mon+1);
	}
	sprintf(time_nowD, "%d", tm_now->tm_mday);
    if((tm_now->tm_hour)>=0&&(tm_now->tm_hour)<=9){
        time_nowH[0]='0';
        char time_nowH_temp='0'+tm_now->tm_hour;
        //printf("time_nowH_temp=%c\n",time_nowH_temp);
        time_nowMI[1]=time_nowH_temp;
        //sprintf(time_nowH[1], "%d", tm_now->tm_hour);
    }
    else{
        sprintf(time_nowH, "%d", tm_now->tm_hour);
    }
    
   
    if((tm_now->tm_min)>=0&&(tm_now->tm_min)<=9){
        time_nowMI[0]='0';
        char time_nowMI_temp='0'+tm_now->tm_min;
        //printf("time_nowMI_temp=%c\n",time_nowMI_temp);
        time_nowMI[1]=time_nowMI_temp;
        //sprintf(time_nowMI[1], "%d", tm_now->tm_min);
        //time_nowMI[1]=(char)tm_now->tm_min;
        //printf("%d\n",tm_now->tm_min);
    }
    else{
        sprintf(time_nowMI, "%d", tm_now->tm_min);
    }
    
    if((tm_now->tm_sec)>=0&&(tm_now->tm_sec)<=9){
        time_nowS[0]='0';
        char time_nowS_temp='0'+tm_now->tm_sec;

        time_nowS[1]=time_nowS_temp;

    }
    else{
        sprintf(time_nowS, "%d", tm_now->tm_sec);
    }
    
	strcat(time_now,time_nowY);
	strcat(time_now,"-");
	strcat(time_now,time_nowMO);
	strcat(time_now,"-");
	strcat(time_now,time_nowD);
    strcat(time_now," ");
    strcat(time_now,time_nowH);
    strcat(time_now,":");
    strcat(time_now,time_nowMI);
    strcat(time_now,":");
    strcat(time_now,time_nowS);
    strcat(time_now,"/");

    if((filepoint=open(fp, O_WRONLY | O_CREAT | O_APPEND, 0666))==-1){
        printf("open faild\n");
        return -1;
    }
    //filepoint=fopen(fp, "a+");
    //fprintf(fp,time_now,"\t",str);
    //write(filepoint,time_now,20);
    //pthread_mutex_lock(&m_mutex);
    strcat(time_now,str);
    write(filepoint,&time_now,strlen(time_now));
    //pthread_mutex_unlock(&m_mutex);
    //printf("filepoint =%s\n",time_now);
    //printf("filepoint sizeof=%d\n",strlen(time_now));

    close(filepoint);

    return 0;

}
