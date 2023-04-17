#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include<stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <signal.h>
#include <getopt.h>
#include"sqlite.h"
#include"get_data.h"


zlog_category_t *zc=NULL;
bool pro_stop=false;

void proStop()
{
	pro_stop=true;
}

struct option long_options[]=
{
	{"port",required_argument,NULL,'p'},
	{"daemon",no_argument,NULL,'d'},
	{"help",no_argument,NULL,'h'},
	{NULL,0,NULL,0}
};

void messg_help()
{
	printf("-p(--port):specify the server port\n");
	printf("-d(--daemon):set process to daemon\n");
	printf("-h(--help):printf the help messege\n");
}

int main(int argc, char *argv[])
{
	int 				sockfd=-1;
	int 				rv=-1;
	int 				val=1;
	int 				clifd=-1;
	struct sockaddr_in 	servaddr;
	struct sockaddr_in 	cliaddr;
	int 				port=-1;
	char 				buf[1024];
	socklen_t 			len=sizeof(cliaddr);
	struct epoll_event 	ev;
	struct epoll_event 	rev[1024];
	int 				epfd=-1;
	int 				fds=-1;
	int 				rctl=-1;
	int 				newfd=-1;
	char 			   *str=NULL;
	char 			   *s[3]={0};
	data_t 				dt={0};
	sqlite_t 			sq={0};
	char 				table_name[128];
	int 				opt=0;
	int 				daemon_run=0;


	if (argc<2)
	{
		printf("please add %s [port]!\n\n",argv[0]);
		
		messg_help();
		
		return 0;
	}

	while ((opt=getopt_long(argc,argv,"p:dh",long_options,NULL)) != -1)
	{
		
		switch (opt)
		{

			case 'p':
				port=atoi(optarg);
				break;
			case 'd':
				daemon_run=1;
				break;
			case 'h':
				messg_help();
				return 0;
			default:
				messg_help();
				return 0;
		}
	}

	if (port<0)
	{
		printf("Please enter the correct format!\n\n");
		
		messg_help();
		
		return 0;
	}

	if(zlog_init("my_zlog.conf")<0)
	{
		printf("zlog init fail:%s\n",strerror(errno));
		return -1;
	}

	if(daemon_run==1)
	{
		if((zc=zlog_get_category("my_zlog")) == NULL)
		{
			printf("zlog get category fail:%s\n",strerror(errno));
			return -1;
		}

		if(daemon(1,0)<0)
		{
			printf("set daemon fail:%s\n",strerror(errno));
			return -1;
		}

	}
	else
	{	
		if((zc=zlog_get_category("use_stdout")) == NULL)
		{
			printf("zlog get category fail:%s\n",strerror(errno));
			return -1;
		}
	}


	signal(SIGTERM,proStop);

	signal(SIGINT,proStop);

	strncpy(sq.filename,"server_sql.db",sizeof(sq.filename));
	
	if(sqlite_open(&sq)<0)
	{
		zlog_error(zc,"sqlite open or create fail\n");
		return -1;
	}


	if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		zlog_error(zc,"socket fail:%s\n",strerror(errno));
		return -1;
	}

	memset(&servaddr,0,sizeof(servaddr));

	servaddr.sin_family=AF_INET;
	
	servaddr.sin_port=htons(port);
	
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	rv=setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void *)&val,sizeof(val));
	
	if(rv == -1)
	{
		zlog_error(zc,"set socket opt fail:%s\n",strerror(errno));
		close(sockfd);
		return -1;
	}

	if(bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		zlog_error(zc,"server[%d] bind port[%d] fail:%s\n",sockfd,port,strerror(errno));
		close(sockfd);
		return -2;
	}
	
	
	if(listen(sockfd,1024)<0)
	{
		zlog_error(zc,"listen port[%d] fail:%s\n",port,strerror(errno));
		close(sockfd);
		return -3;
	}

	

	zlog_notice(zc,"start to listen port[%d] ......\n",port);


	ev.data.fd=sockfd;

	ev.events=EPOLLIN;

	epfd=epoll_create(1024);
	
	if(epfd<0)
	{
		zlog_error(zc,"epoll fail:%s\n",strerror(errno));
		close(sockfd);
		return -4;
	}
	
	
	if(epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev)<0)
	{
		zlog_error(zc,"epoll_ctl fail:%s\n",strerror(errno));
		close(sockfd);
		return -5;
	}
    

    while(!pro_stop)
    {
        fds=epoll_wait(epfd,rev,1024,-1);

		if(fds<0)
		{
			if(errno == EINTR)
			{
				break;
			}

			zlog_error(zc,"epoll_waite fail:%s\n",strerror(errno));
			close(sockfd);
			return -6;
		}

		for(int i=0;i<fds;i++)
		{
			
			 if(rev[i].data.fd == sockfd)
			{
				clifd=accept(sockfd,(struct sockaddr *)&cliaddr,&len);

				if(clifd<0)
				{
					zlog_error(zc,"server accept fail:%s\n",strerror(errno));
					close(sockfd);
					return -7;
				}

				zlog_notice(zc,"client(%d)[Ip:%s,port:%d] connect success!\n",clifd,inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
				
				ev.data.fd=clifd;

				ev.events=EPOLLIN;
				
				if(epoll_ctl(epfd,EPOLL_CTL_ADD,clifd,&ev)<0)
				{
					zlog_error(zc,"epoll_ctl fail:%s\n",strerror(errno));
					close(sockfd);
					close(clifd);
					return -8;
				}
				
				
		
			}
			else if(rev[i].events & EPOLLIN)
			{
				if(rev[i].data.fd<0)
				{
					continue;
				}

				newfd=rev[i].data.fd;
				
				memset(buf,0,sizeof(buf));
		        
				rv=read(newfd,buf,sizeof(buf));

		        
				if(rv<0)
		        {
					if(errno == ECONNRESET)
					{
						epoll_ctl(epfd,EPOLL_CTL_DEL,newfd,&ev);
						
						zlog_warn(zc,"client exit!\n");
						
						close(newfd);
						
						rev[i].data.fd=-1;
						
						continue;
					}
					else
					{
						zlog_error(zc,"read from client[%d] fail:%s\n",newfd,strerror(errno));
						
						epoll_ctl(epfd,EPOLL_CTL_DEL,newfd,&ev);
						
						close(newfd);
						
						rev[i].data.fd=-1;
						
						continue;
					}
			            
		         }
				else if(rv == 0)
				{
					zlog_warn(zc,"client[%d] disconnect!\n",newfd);
					
					epoll_ctl(epfd,EPOLL_CTL_DEL,newfd,&ev);
					
					close(newfd);
					
					rev[i].data.fd=-1;
					
					continue;
				}
				else
				{
					str=strtok(buf,"|");
					
					int i=0;
					
					while(str)
					{
						s[i]=str;
						str=strtok(NULL,"|");
						i++;
					}

					strncpy(dt.id,s[0],sizeof(dt.id));
					strncpy(dt.time,s[2],sizeof(dt.time));
					dt.temperature=atof(s[1]);
					
					memset(sq.table_name,0,sizeof(sq.table_name));
					strncpy(sq.table_name,s[0],sizeof(sq.table_name));

					if(sqlite_check_table(&sq) != 1)
					{
						if(sqlite_create_table(&sq)<0)
						{
							zlog_error(zc,"sqlite create table[%s] fail\n",sq.table_name);
							return -1;
						}
					}


					if(sqlite_check_data(&sq,&dt) != 1 && strlen(buf)>0)
					{
						if(sqlite_add(&sq,&dt)<0)
						{
							zlog_error(zc,"add data to sql fail\n");
							close(sockfd);
							close(newfd);
							sqlite3_close(sq.db);
							return -9;
						}

						zlog_info(zc,"add data(%s|%.2f|%s) to sql success!\n",dt.id,dt.temperature,dt.time);
					}

				}	
			}
		
		}//end of for()
    
	}//end of while()

	close(sockfd);
	close(epfd);

	if(sq.db != NULL)
	{
		sqlite3_close(sq.db);
	}
	
	zlog_fini();
	
	return 0;
}

