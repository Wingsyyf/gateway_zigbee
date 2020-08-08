#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <errno.h>


union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};

static int sem_id = 0;
 
static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();


static int set_semvalue()
{
	union semun sem_union;
 
	sem_union.val = 1;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
		return 0;
	printf("set_semvalue success\n");
	return 1;
}
 
static void del_semvalue()
{
	//删除信号量
	union semun sem_union;
 
	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}
 
static int semaphore_p()
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1;//P()
	sem_b.sem_flg = SEM_UNDO;
	
	int nRet;
	do
	{
		nRet =  semop(sem_id, &sem_b, 1); // P操作
	}while( (nRet == -1) && (errno == 4));

	if( (nRet == -1) && (errno != 4))
	{
		fprintf(stderr,"semaphore_p failed errno=%d\n",errno);
		return 0;
	}
	printf("semaphore_p0 success\n");
	return 1;
}

 
static int semaphore_v()
{
	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;//V()
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr,"semaphore_v failed\n");
		return 0;
	}
	return 1;
}

