/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  semaphore.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/09/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/09/23 19:11:13"
 *                 
 ********************************************************************************/
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include"zlog.h"
#include "semaphore.h"

int sem_init()
{
	key_t key;
	int semid;
	union semun sem_union;

	if((key=ftok("/dev/zero",1))<0)
	{
		zlog_error(zc,"ftok fail:%s\n",strerror(errno));
		return -1;
	}

	if((semid=semget(key,1,IPC_CREAT|0644))<0)
	{
		zlog_error(zc,"semget fail:%s\n",strerror(errno));
		return -2;
	}

	sem_union.val=1;
	if(semctl(semid,0,SETVAL,sem_union)<0)
	{
		zlog_error(zc,"semctl fail:%s\n",strerror(errno));
		return -3;
	}

	return semid;
}



int sem_P(int semid)
{
	struct sembuf buf;
	buf.sem_num=0;
	buf.sem_op=-1;
	buf.sem_flg=SEM_UNDO;
	if(semop(semid,&buf,1)<0)
	{
		zlog_error(zc,"semop P faile:%s\n",strerror(errno));
		return -1;
	}

	return 0;
}



int sem_V(int semid)
{
	struct sembuf buf;
	buf.sem_num=0;
	buf.sem_op=1;
	buf.sem_flg=SEM_UNDO;
	if(semop(semid,&buf,1)<0)
	{
		zlog_error(zc,"semop V fail:%s\n",strerror(errno));
		return -1;
	}

	return 0;
}

