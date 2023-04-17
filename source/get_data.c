/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_data.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/11/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/11/23 10:33:23"
 *                 
 ********************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "get_data.h"
#include "get_sn.h"
#include "get_temperature.h"
#include "get_time.h"


int get_data(data_t *dt)
{
	
	/*
	if(get_sn(&(dt->id))<0)
	{
		zlog_error(zc,"get sn fail\n");
		return -1;
	}

	*/

	if(get_temperature(&(dt->temperature))<0)
	{
		zlog_error(zc,"get temperature fail\n");
		return -2;
	}

	if(get_time(dt->time)<0)
	{
		zlog_error(zc,"get time fail\n");
		return -3;
	}

	//strncpy(dt->id,sn,sizeof(dt->id));
	//dt->temperature=tmp;
	//strncpy(dt->time,time,sizeof(dt->time));

	return 0;

}

