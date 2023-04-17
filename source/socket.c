/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/14/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/14/23 09:04:09"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "socket.h"

int socket_init(socket_t *sock)
{
	sock->sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	if(sock->sockfd<0)
	{
		zlog_error(zc,"%s\n",strerror(errno));
		return -1;
	}

	memset(&(sock->sockaddr),0,sizeof(sock->sockaddr));
	sock->sockaddr.sin_family=AF_INET;
	sock->sockaddr.sin_port=htons(sock->port);

	inet_aton(sock->ipaddr,&(sock->sockaddr.sin_addr));

	return 0;
}


int socket_check(socket_t *sock)
{
	int status=-1;
	struct tcp_info info;
	int len=sizeof(info);

	getsockopt(sock->sockfd,IPPROTO_TCP,TCP_INFO,&info,(socklen_t *)&len);

	if(info.tcpi_state == 1)
	{
		status=1;
		return status;
	}

	return status;
}



int socket_connect(socket_t *sock)
{
	int rv=-1;

	rv=connect(sock->sockfd,(struct sockaddr *)&(sock->sockaddr),sizeof(sock->sockaddr));

	if(rv<0)
	{
		zlog_error(zc,"%s\n",strerror(errno));
		return -1;
	}

	return 0;

}


int socket_write(socket_t *sock,data_t *dt)
{
	char buf[1024];

	memset(buf,0,sizeof(buf));
	sprintf(buf,"%s|%.2f|%s",dt->id,dt->temperature,dt->time);

	if(write(sock->sockfd,buf,sizeof(buf))<0)
	{
		zlog_error(zc,"%s\n",strerror(errno));
		return -1;
	}
	
	return 0;

}

