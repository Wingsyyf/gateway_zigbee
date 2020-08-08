#include "md5.h"
#include "curl/curl.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>  
#include "gateway.h"

#define READ_DATA_SIZE			1024
#define MD5_SIZE				16
#define MD5_STR_LEN				(MD5_SIZE * 2)
#define NET_KEY		    		"kw3j5k32ur38rnerkKJHk83"  //密钥
#define TIMER_STANDARD_TIME 	3600//(s)		

#define  RUNLOG          "run-log.txt"
#define  ERRLOG          "err-log.txt"


/*************************************************************
*--------------------Global Variable Symbol-------------------
*
* 
*************************************************************/
/*struct memory {
   char *response;
   size_t size;
 };*/

int shift;
char* client;
int shift1;
char* client1;
int shift2;
char* client2;
//struct memory client;


/*************************************************************
*----------------Define Local Function HEAD-------------------
*
* 
*************************************************************/
int Compute_file_md5(const char *file_path, char *value);
int Compute_string_md5(unsigned char *dest_str, unsigned int dest_len, char *md5_str);

size_t read_data(void *ptr,size_t size,size_t nmemb,void* stream);
size_t read_data1(void *ptr,size_t size,size_t nmemb,void* stream);
size_t read_data2(void *ptr,size_t size,size_t nmemb,void* stream);
int port_update(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2);
int port_update1(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2);
int port_update2(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2);

/*************************************************************
*----------------Compute_string_md5 Function-------------------
*将字符串MD5加密
* 
*************************************************************/

int Compute_string_md5(unsigned char *dest_str, unsigned int dest_len, char *md5_str)
{
	int i;
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;
	MD5Init(&md5);
	MD5Update(&md5, dest_str, dest_len);
	MD5Final(&md5, md5_value);
	for(i = 0; i < MD5_SIZE; i++){
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	return 0;
} 
/*************************************************************
*----------------Compute_file_md5 Function -------------------
* 将文件MD5加密
* 
*************************************************************/
int Compute_file_md5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;
	fd = open(file_path, O_RDONLY);
	if (-1 == fd){
		perror("open");
		return -1;
	}
	MD5Init(&md5);
	while (1){
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret){
			perror("read");
			return -1;
		}
		MD5Update(&md5, data, ret);
		if (0 == ret || ret < READ_DATA_SIZE){
			break;
		}
	} 
	close(fd); 
	MD5Final(&md5, md5_value);
	for(i = 0; i < MD5_SIZE; i++){
		snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end
	return 0;
}

/*************************************************************
*----------------read_data Function -------------------
* 回调函数，curl接收数据的回调函数0
* 
*************************************************************/
size_t read_data0(void *ptr,size_t size,size_t nmemb,void* stream){
	int res_size;
    res_size = size * nmemb;
    client = realloc(client, shift+res_size + 1);
    memcpy(client + shift, ptr, res_size);
    shift += res_size;
	return size * nmemb;
}


/*************************************************************
*--------------------FUNCTION---------------------------------
*                 * 调用网络接口0 *
* 
*************************************************************/ 
int port_update0(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2){

//--------------------netport Variable------------------------
    
  	int ret;
	struct timeval tv;
	char *timebufs;
	char timebufs1[10];
	char timebufs2[3];
	long timebufl;
	char *verify;
	char *key=NET_KEY;
	time_t unixt;
	char unixtstr[100];
	char md5_str[MD5_STR_LEN + 1];
	
	cJSON *json;
	CURLcode res;

	char *Ceqpnum="EQP_Ment_Num=";
	char *Ceqpstate="&EQP_State=";
	char *Ceqptime="&EQP_Time=";
	char *eqptime;
	char eqptimebufY[4];
	char eqptimebufM[2];
	char eqptimebufD[2];
	char *Ceqpstepkey="&EQP_StepKey=";
	//char *eqpstepkey="CS";
	char *Ctimechar="&Timestamp=";
	char *Cverifychar="&VerifyCode=";

	char mysitehead[512]="http://gcfile.jiepei.com:8888/api/Alot/AddEQPLog?";
	
    //char mytestsite[512]="http://pcbapi.jiepei.com/api/Pcb/GetProcessCard?BusinessOrderNo=J2w-1506974";
	char *mysite;

//获取当前年月日
 	time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
	//printf("md5_str=%s\n",md5_str);
	if((tm_now->tm_mon+1)==0){
		eqptimebufM[0]='1';
		eqptimebufM[1]='2';
	}
	else if((tm_now->tm_mon+1)>0&&(tm_now->tm_mon+1)<10){
		eqptimebufM[0]='0';
		sprintf(&eqptimebufM[1], "%d", tm_now->tm_mon+1);
	}
	else{
		sprintf(eqptimebufM, "%d", tm_now->tm_mon+1);
	}
	sprintf(eqptimebufY, "%d", tm_now->tm_year+1900);
	//sprintf(eqptimebufM, "%d", tm_now->tm_mon);
	sprintf(eqptimebufD, "%d", tm_now->tm_mday);
	eqptime = (char *) malloc(10);
	for(int i=0;i<10;i++){
		eqptime[i]=0;
	}
	strcat(eqptime,eqptimebufY);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufM);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufD);
	//printf("eqptime:%s,sizeof=%d\n",eqptime,sizeof(eqptime));

	//printf("eqp_stepkey:%c%c\n",eqp_stepkey_1,eqp_stepkey_2);
	//获取时间戳
	gettimeofday(&tv,NULL);
	sprintf(timebufs1, "%ld", tv.tv_sec);
	sprintf(timebufs2, "%ld", tv.tv_usec/1000);
	
	timebufs = (char *) malloc(13);
	for(int i=0;i<13;i++){
		timebufs[i]=0;
	}
	strcat(timebufs,timebufs1);
	strcat(timebufs,timebufs2);
	//printf("timebufs:%s\n",timebufs);
	//密钥和时间戳拼接为验证串
	verify = (char *) malloc(100);
	for(int i=0;i<100;i++){
		verify[i]=0;
	}	
	strcat(verify,key);
	strcat(verify,timebufs);
	//printf("verifycode:%s\n",verify);
	//md5加密 
	Compute_string_md5((unsigned char *)verify, strlen(verify), md5_str);
	for(int j=0;md5_str[j]!='\0';j++){
		if(md5_str[j]>='A'&&md5_str[j]<='Z'){
				md5_str[j]=md5_str[j]+32;
			}
	}
	//printf("MD5-verifycode:%s\n",md5_str);
	
	//拼接字符串-----------------------------------------------------------------	
	mysite = (char *) malloc(1024);
	for(int i=0;i<1024;i++){
		mysite[i]=0;
	}
	//head
	strcat(mysite,mysitehead);
	//eqpnum
	strcat(mysite,Ceqpnum);
	
	char eqp_num_temp[1];
	sprintf(eqp_num_temp,"%d",eqp_num);
	strcat(mysite,eqp_num_temp);
	//eqpstate
	strcat(mysite,Ceqpstate);
	char eqp_state_temp[1];
	sprintf(eqp_state_temp,"%d",eqp_state);
	strcat(mysite,eqp_state_temp);
	//eqptime
	strcat(mysite,Ceqptime);
	strcat(mysite,eqptime);
	//eqpstetpkey
	strcat(mysite,Ceqpstepkey);
	char eqp_stepkey_temp[2];
	sprintf(eqp_stepkey_temp,"%c%c",eqp_stepkey_1,eqp_stepkey_2);
	strcat(mysite,eqp_stepkey_temp);
	//strcat(mysite,eqp_stepkey_1);
	//strcat(mysite,eqp_stepkey_2);
	
	//timestamp
	strcat(mysite,Ctimechar);
	strcat(mysite,timebufs);
	
	//verifykey
	strcat(mysite,Cverifychar);
	strcat(mysite,md5_str);
	printf("mysite: %s\n",mysite);

	//curl_function------------------------------------------------------------
	CURL *curl=curl_easy_init();
	if(curl==NULL){
		//printf("curl init error\n");
		write_log(ERRLOG,"curl init error\n");		
		return -1;
	}

		if(curl){
			//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			//printf("mysite1\n");
			curl_easy_setopt(curl, CURLOPT_URL, mysite);
			//printf("mysite2\n");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_data0);
			//printf("mysite3\n");
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, client);

			//curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT , 5);
			//printf("mysite4\n");

			res=curl_easy_perform(curl);
			
			//printf("mysite5\n");
			
			curl_easy_cleanup(curl);
			printf("thread0:---------------------------------\n");
			//pthread_mutex_lock(&net_mutex);
			for(int i=0;i<shift;i++){
				printf("%c",client[i]);
			}
			printf("\n");
			shift=0;
			//pthread_mutex_unlock(&net_mutex);

			json=cJSON_Parse(client); //获取整个大的句柄
			
			if(!json){
				//printf("9\n");
				return -1;
			}

			cJSON *itemc = cJSON_GetObjectItem(json,"code"); 
			printf("%s:%d\n",itemc->string,itemc->valueint);
			if(((itemc->valueint)==0)){
				//write_log(ERRLOG,"json code = -1 and try 3 times\n");
				return -1;
			}
			else if((itemc->valueint)==1)
			{
				//write_log(RUNLOG,"curl success\n");
				cJSON *itemm = cJSON_GetObjectItem(json,"message");
				printf("%s:%s\n",itemm->string,itemm->valuestring);
				cJSON *itemd = cJSON_GetObjectItem(json,"data");
				printf("%s:%s\n",itemd->string,itemd->valuestring);	
				
			}	
			cJSON_Delete(json);			
		}
	
	free(timebufs);
	free(verify);
	free(eqptime);
	free(mysite);

	return 0;
	
}



/*************************************************************
*----------------read_data Function -------------------
* 回调函数，curl接收数据的回调函数1
* 
*************************************************************/
size_t read_data1(void *ptr,size_t size,size_t nmemb,void* stream){
	int res_size;
    res_size = size * nmemb;
    client1 = realloc(client1, shift1+res_size + 1);
    memcpy(client1 + shift1, ptr, res_size);
    shift1 += res_size;
	return size * nmemb;
}


/*************************************************************
*--------------------FUNCTION---------------------------------
*                 * 调用网络接口1 *
* 
*************************************************************/ 
int port_update1(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2){

//--------------------netport Variable------------------------
    
  	int ret;
	struct timeval tv;
	char *timebufs;
	char timebufs1[10];
	char timebufs2[3];
	long timebufl;
	char *verify;
	char *key=NET_KEY;
	time_t unixt;
	char unixtstr[100];
	char md5_str[MD5_STR_LEN + 1];
	
	cJSON *json;
	CURLcode res;

	char *Ceqpnum="EQP_Ment_Num=";
	char *Ceqpstate="&EQP_State=";
	char *Ceqptime="&EQP_Time=";
	char *eqptime;
	char eqptimebufY[4];
	char eqptimebufM[2];
	char eqptimebufD[2];
	char *Ceqpstepkey="&EQP_StepKey=";
	//char *eqpstepkey="CS";
	char *Ctimechar="&Timestamp=";
	char *Cverifychar="&VerifyCode=";

	char mysitehead[512]="http://gcfile.jiepei.com:8888/api/Alot/AddEQPLog?";
	
    //char mytestsite[512]="http://pcbapi.jiepei.com/api/Pcb/GetProcessCard?BusinessOrderNo=J2w-1506974";
	char *mysite;

//获取当前年月日
 	time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
	//printf("md5_str=%s\n",md5_str);
	if((tm_now->tm_mon+1)==0){
		eqptimebufM[0]='1';
		eqptimebufM[1]='2';
	}
	else if((tm_now->tm_mon+1)>0&&(tm_now->tm_mon+1)<10){
		eqptimebufM[0]='0';
		sprintf(&eqptimebufM[1], "%d", tm_now->tm_mon+1);
	}
	else{
		sprintf(eqptimebufM, "%d", tm_now->tm_mon+1);
	}
	sprintf(eqptimebufY, "%d", tm_now->tm_year+1900);
	//sprintf(eqptimebufM, "%d", tm_now->tm_mon);
	sprintf(eqptimebufD, "%d", tm_now->tm_mday);
	eqptime = (char *) malloc(10);
	for(int i=0;i<10;i++){
		eqptime[i]=0;
	}
	strcat(eqptime,eqptimebufY);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufM);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufD);
	//printf("eqptime:%s,sizeof=%d\n",eqptime,sizeof(eqptime));

	//printf("eqp_stepkey:%c%c\n",eqp_stepkey_1,eqp_stepkey_2);
	//获取时间戳
	gettimeofday(&tv,NULL);
	sprintf(timebufs1, "%ld", tv.tv_sec);
	sprintf(timebufs2, "%ld", tv.tv_usec/1000);
	
	timebufs = (char *) malloc(13);
	for(int i=0;i<13;i++){
		timebufs[i]=0;
	}
	strcat(timebufs,timebufs1);
	strcat(timebufs,timebufs2);
	//printf("timebufs:%s\n",timebufs);
	//密钥和时间戳拼接为验证串
	verify = (char *) malloc(100);
	for(int i=0;i<100;i++){
		verify[i]=0;
	}	
	strcat(verify,key);
	strcat(verify,timebufs);
	//printf("verifycode:%s\n",verify);
	//md5加密 
	Compute_string_md5((unsigned char *)verify, strlen(verify), md5_str);
	for(int j=0;md5_str[j]!='\0';j++){
		if(md5_str[j]>='A'&&md5_str[j]<='Z'){
				md5_str[j]=md5_str[j]+32;
			}
	}
	//printf("MD5-verifycode:%s\n",md5_str);
	
	//拼接字符串-----------------------------------------------------------------	
	mysite = (char *) malloc(1024);
	for(int i=0;i<1024;i++){
		mysite[i]=0;
	}
	//head
	strcat(mysite,mysitehead);
	//eqpnum
	strcat(mysite,Ceqpnum);
	
	char eqp_num_temp[1];
	sprintf(eqp_num_temp,"%d",eqp_num);
	strcat(mysite,eqp_num_temp);
	//eqpstate
	strcat(mysite,Ceqpstate);
	char eqp_state_temp[1];
	sprintf(eqp_state_temp,"%d",eqp_state);
	strcat(mysite,eqp_state_temp);
	//eqptime
	strcat(mysite,Ceqptime);
	strcat(mysite,eqptime);
	//eqpstetpkey
	strcat(mysite,Ceqpstepkey);
	char eqp_stepkey_temp[2];
	sprintf(eqp_stepkey_temp,"%c%c",eqp_stepkey_1,eqp_stepkey_2);
	strcat(mysite,eqp_stepkey_temp);
	//strcat(mysite,eqp_stepkey_1);
	//strcat(mysite,eqp_stepkey_2);
	
	//timestamp
	strcat(mysite,Ctimechar);
	strcat(mysite,timebufs);
	
	//verifykey
	strcat(mysite,Cverifychar);
	strcat(mysite,md5_str);
	printf("mysite: %s\n",mysite);

	//curl_function------------------------------------------------------------
	CURL *curl=curl_easy_init();
	if(curl==NULL){
		//printf("curl init error\n");
		write_log(ERRLOG,"curl init error\n");		
		return -1;
	}

		if(curl){
			//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			//printf("mysite1\n");
			curl_easy_setopt(curl, CURLOPT_URL, mysite);
			//printf("mysite2\n");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_data1);
			//printf("mysite3\n");
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, client1);

			//curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT , 5);
			//printf("mysite4\n");

			res=curl_easy_perform(curl);
			
			//printf("mysite5\n");
			
			curl_easy_cleanup(curl);
			printf("thread1:---------------------------------\n");
			//pthread_mutex_lock(&net_mutex);
			for(int i=0;i<shift1;i++){
				printf("%c",client1[i]);
			}
			printf("\n");
			shift1=0;
			//pthread_mutex_unlock(&net_mutex);
			//printf("0\n");
			json=cJSON_Parse(client1); //获取整个大的句柄
			//printf("1\n");
			
			if(!json){
				//printf("9\n");
				return -1;
			}
			//printf("json.type=%d,\n",json->type);
			cJSON *itemc = cJSON_GetObjectItem(json,"code"); 
					
			printf("%s:%d\n",itemc->string,itemc->valueint);

			if(((itemc->valueint)==0)){
				//write_log(ERRLOG,"json code = 0 and try 3 times\n");
				return -1;
			}
			
			else if((itemc->valueint)==1)
			{
				write_log(RUNLOG,"curl success\n");
				cJSON *itemm = cJSON_GetObjectItem(json,"message");
				printf("%s:%s\n",itemm->string,itemm->valuestring);
				cJSON *itemd = cJSON_GetObjectItem(json,"data");
				printf("%s:%s\n",itemd->string,itemd->valuestring);	
				
			}	
			cJSON_Delete(json);			
		}
	
	free(timebufs);
	free(verify);
	free(eqptime);
	free(mysite);

	return 0;
	
}


/*************************************************************
*----------------read_data Function -------------------
* 回调函数，curl接收数据的回调函数2
* 
*************************************************************/
size_t read_data2(void *ptr,size_t size,size_t nmemb,void* stream){
	int res_size;
    res_size = size * nmemb;
    client2 = realloc(client2, shift2+res_size + 1);
    memcpy(client2 + shift2, ptr, res_size);
    shift2 += res_size;
	return size * nmemb;
}


/*************************************************************
*--------------------FUNCTION---------------------------------
*                 * 调用网络接口2 *
* 
*************************************************************/ 
int port_update2(char eqp_num, char eqp_state, char eqp_stepkey_1, char eqp_stepkey_2){

//--------------------netport Variable------------------------
    
  	int ret;
	struct timeval tv;
	char *timebufs;
	char timebufs1[10];
	char timebufs2[3];
	long timebufl;
	char *verify;
	char *key=NET_KEY;
	time_t unixt;
	char unixtstr[100];
	char md5_str[MD5_STR_LEN + 1];
	
	cJSON *json;
	CURLcode res;

	char *Ceqpnum="EQP_Ment_Num=";
	char *Ceqpstate="&EQP_State=";
	char *Ceqptime="&EQP_Time=";
	char *eqptime;
	char eqptimebufY[4];
	char eqptimebufM[2];
	char eqptimebufD[2];
	char *Ceqpstepkey="&EQP_StepKey=";
	//char *eqpstepkey="CS";
	char *Ctimechar="&Timestamp=";
	char *Cverifychar="&VerifyCode=";

	char mysitehead[512]="http://gcfile.jiepei.com:8888/api/Alot/AddEQPLog?";
	
    //char mytestsite[512]="http://pcbapi.jiepei.com/api/Pcb/GetProcessCard?BusinessOrderNo=J2w-1506974";
	char *mysite;

//获取当前年月日
 	time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
	//printf("md5_str=%s\n",md5_str);
	if((tm_now->tm_mon+1)==0){
		eqptimebufM[0]='1';
		eqptimebufM[1]='2';
	}
	else if((tm_now->tm_mon+1)>0&&(tm_now->tm_mon+1)<10){
		eqptimebufM[0]='0';
		sprintf(&eqptimebufM[1], "%d", tm_now->tm_mon+1);
	}
	else{
		sprintf(eqptimebufM, "%d", tm_now->tm_mon+1);
	}
	sprintf(eqptimebufY, "%d", tm_now->tm_year+1900);
	//sprintf(eqptimebufM, "%d", tm_now->tm_mon);
	sprintf(eqptimebufD, "%d", tm_now->tm_mday);
	eqptime = (char *) malloc(10);
	for(int i=0;i<10;i++){
		eqptime[i]=0;
	}
	strcat(eqptime,eqptimebufY);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufM);
	strcat(eqptime,"-");
	strcat(eqptime,eqptimebufD);
	//printf("eqptime:%s,sizeof=%d\n",eqptime,sizeof(eqptime));

	//printf("eqp_stepkey:%c%c\n",eqp_stepkey_1,eqp_stepkey_2);
	//获取时间戳
	gettimeofday(&tv,NULL);
	sprintf(timebufs1, "%ld", tv.tv_sec);
	sprintf(timebufs2, "%ld", tv.tv_usec/1000);
	
	timebufs = (char *) malloc(13);
	for(int i=0;i<13;i++){
		timebufs[i]=0;
	}
	strcat(timebufs,timebufs1);
	strcat(timebufs,timebufs2);
	//printf("timebufs:%s\n",timebufs);
	//密钥和时间戳拼接为验证串
	verify = (char *) malloc(100);
	for(int i=0;i<100;i++){
		verify[i]=0;
	}	
	strcat(verify,key);
	strcat(verify,timebufs);
	//printf("verifycode:%s\n",verify);
	//md5加密 
	Compute_string_md5((unsigned char *)verify, strlen(verify), md5_str);
	for(int j=0;md5_str[j]!='\0';j++){
		if(md5_str[j]>='A'&&md5_str[j]<='Z'){
				md5_str[j]=md5_str[j]+32;
			}
	}
	//printf("MD5-verifycode:%s\n",md5_str);
	
	//拼接字符串-----------------------------------------------------------------	
	mysite = (char *) malloc(1024);
	for(int i=0;i<1024;i++){
		mysite[i]=0;
	}
	//head
	strcat(mysite,mysitehead);
	//eqpnum
	strcat(mysite,Ceqpnum);
	
	char eqp_num_temp[1];
	sprintf(eqp_num_temp,"%d",eqp_num);
	strcat(mysite,eqp_num_temp);
	//eqpstate
	strcat(mysite,Ceqpstate);
	char eqp_state_temp[1];
	sprintf(eqp_state_temp,"%d",eqp_state);
	strcat(mysite,eqp_state_temp);
	//eqptime
	strcat(mysite,Ceqptime);
	strcat(mysite,eqptime);
	//eqpstetpkey
	strcat(mysite,Ceqpstepkey);
	char eqp_stepkey_temp[2];
	sprintf(eqp_stepkey_temp,"%c%c",eqp_stepkey_1,eqp_stepkey_2);
	strcat(mysite,eqp_stepkey_temp);
	//strcat(mysite,eqp_stepkey_1);
	//strcat(mysite,eqp_stepkey_2);
	
	//timestamp
	strcat(mysite,Ctimechar);
	strcat(mysite,timebufs);
	
	//verifykey
	strcat(mysite,Cverifychar);
	strcat(mysite,md5_str);
	printf("mysite: %s\n",mysite);

	//curl_function------------------------------------------------------------
	CURL *curl=curl_easy_init();
	if(curl==NULL){
		//printf("curl init error\n");
		write_log(ERRLOG,"curl init error\n");		
		return -1;
	}

		if(curl){
			//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
			//printf("mysite1\n");
			curl_easy_setopt(curl, CURLOPT_URL, mysite);
			//printf("mysite2\n");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_data2);
			//printf("mysite3\n");
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, client2);

			//curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT , 5);
			//printf("mysite4\n");

			res=curl_easy_perform(curl);
			
			//printf("mysite5\n");
			
			curl_easy_cleanup(curl);
			printf("thread2:---------------------------------\n");
			//pthread_mutex_lock(&net_mutex);
			for(int i=0;i<shift2;i++){
				printf("%c",client2[i]);
			}
			printf("\n");
			shift2=0;

			//pthread_mutex_unlock(&net_mutex);

			json=cJSON_Parse(client2); //获取整个大的句柄

			if(!json){
				//printf("9\n");
				return -1;
			}

			cJSON *itemc = cJSON_GetObjectItem(json,"code"); 
			printf("%s:%d\n",itemc->string,itemc->valueint);
			if(((itemc->valueint)==0)){
				//write_log(ERRLOG,"json code = -1 and try 3 times\n");
				return -1;
			}
			else if((itemc->valueint)==1)
			{
				write_log(RUNLOG,"curl success\n");
				cJSON *itemm = cJSON_GetObjectItem(json,"message");
				printf("%s:%s\n",itemm->string,itemm->valuestring);
				cJSON *itemd = cJSON_GetObjectItem(json,"data");
				printf("%s:%s\n",itemd->string,itemd->valuestring);					
			}	
			cJSON_Delete(json);			
		}
	
	free(timebufs);
	free(verify);
	free(eqptime);
	free(mysite);

	return 0;
	
}
