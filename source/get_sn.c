/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_SN.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/10/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/10/23 22:55:33"
 *                 
 ********************************************************************************/
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

int get_sn(char *sn)
{
	DIR* dir=NULL;
    struct dirent* dit=NULL;
    char device_path[128]="/sys/bus/w1/devices/";
    int flag=-1;


    if((dir=opendir(device_path))==NULL)
    {
        printf("open dir[%s] fail: %s\n",device_path,strerror(errno));    
    	return -1;
	}

    while((dit=readdir(dir))!=NULL)
    {
        if(strstr(dit->d_name,"28-")!=NULL)
        {

            strncpy(sn,dit->d_name,128);
            flag=1;
			break;
        }
    }
    
    if(flag<0)
    {
        printf("can not find file[28-]!\n");
		return -2;
    }

	return 0;
}

