/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  semaphore.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/09/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/09/23 19:26:43"
 *                 
 ********************************************************************************/

#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include "zlog.h"
extern zlog_category_t *zc;

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
};

static int semid;

int sem_init();
int sem_P(int semid);
int sem_V(int semid);

#endif

