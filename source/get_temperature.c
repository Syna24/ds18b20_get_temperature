/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temperature.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/10/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/10/23 22:08:43"
 *                 
 ********************************************************************************/
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "get_temperature.h"

int get_temperature(float *temp)
{
	DIR* dir=NULL;
    struct dirent* dit=NULL;
    char device_path[128]="/sys/bus/w1/devices/";
	char device[128];
    int fd=-1;
    char buf[1024];
    char *ptr=NULL;
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
			strncpy(device,dit->d_name,sizeof(device));
            flag=1;
			break;
        }
    }
    
    if(flag<0)
    {
        printf("can not find file[28-]!\n");
		return -2;
    }

    closedir(dir);

    strncat(device_path,device,sizeof(device_path)-strlen(device_path));
    strncat(device_path,"/w1_slave",sizeof(device_path)-strlen(device_path));
    
    memset(buf,0,sizeof(buf));

    if((fd=open(device_path,O_RDONLY))<0)
    {
        printf("open 'w1_slave' fail:%s\n",strerror(errno));
		close(fd);
		return -3;
    }

    if((read(fd,buf,sizeof(buf)))<0)
    {
        printf("read data fail:%s\n",strerror(errno));
		close(fd);
		return -4;
    }
    
	close(fd);

    if((ptr=strstr(buf,"t=")) == NULL)
    {
    	printf("can not find 't='\n");
		return -5;
    }

    ptr=ptr+2;

    *temp=atof(ptr)/1000;

    return 0;
}

