/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_time.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/11/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/11/23 10:13:00"
 *                 
 ********************************************************************************/
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>


int get_time(char *tm)
{
	time_t current_time;
	struct tm *now_time=NULL;

	time(&current_time);
	

	if(current_time<0)
	{
		printf("time() fail:%s\n",strerror(errno));
		return -1;
	}

	now_time=localtime(&current_time);

	if(now_time==NULL)
	{
		printf("localtime fail:%s\n",strerror(errno));
		return -2;
	}

	strftime(tm,128,"%Y/%m/%d-%H:%M:%S",now_time);

	return 0;


}

