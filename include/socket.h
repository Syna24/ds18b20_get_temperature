/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(04/14/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/14/23 09:07:36"
 *                 
 ********************************************************************************/
#ifndef __SOCKET_H
#define __SOCKET_H

#include <arpa/inet.h>
#include "get_data.h"
#include "zlog.h"

extern zlog_category_t *zc;

typedef struct socket_s{
	int sockfd;
	char ipaddr[128];
	int port;
	struct sockaddr_in sockaddr;
}socket_t;

int socket_init(socket_t *sock);
int socket_check(socket_t *sock);
int socket_connect(socket_t *sock);
int socket_write(socket_t *sock,data_t *data);

#endif

