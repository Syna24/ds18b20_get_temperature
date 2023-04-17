#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include<stdlib.h>
#include<linux/tcp.h>
#include<time.h>
#include<signal.h>
#include<stdbool.h>
#include <getopt.h>
#include "socket.h"
#include"get_data.h"
#include "sqlite.h"



zlog_category_t *zc=NULL;

bool pro_stop=false;

void proStop()
{
	pro_stop=true;
}

struct option long_options[]=
{
	{"host",required_argument,NULL,'H'},
	{"port",required_argument,NULL,'p'},
	{"interval",required_argument,NULL,'i'},
	{"sn",required_argument,NULL,'s'},
	{"daemon",no_argument,NULL,'d'},
	{"help",no_argument,NULL,'h'},
	{NULL,0,NULL,0}
};

void messg_help()
{
	printf("-H(--host):specify the server host address\n");
	printf("-p(--port):specify the server port\n");
	printf("-i(--interval):specify the sample interval(s)\n");
	printf("-s(--sn):specify the deviece sn\n");
	printf("-d(--daemon):set process to daemon\n");
	printf("-h(--help):printf the help messege\n");
}


int main(int argc, char *argv[])
{
	int 		opt=0;
	socket_t 	sock={0};
	sqlite_t	sq={0};
	int 		sample_interval=-1;
	int 		daemon_run=0;
	char 		out_way[64];
	time_t		pretime={0};
	time_t		nowtime={0};
	int			reconnect=-1;
	data_t 		dt={0};
	data_t 		sql_dt={0};
	char 	   *sn=NULL;
	int 		send_flag=-1;	
	

	if(argc<4)
	{
		printf("Please add %s [server_ip] [server_port] [sn] [sample_interval]!\n\n",argv[0]);
		
		messg_help();
		
		return 0;
	}
	

	while((opt=getopt_long(argc,argv,"H:p:i:s:dh",long_options,NULL)) != -1)
	{
		switch(opt)
		{
			case 'H':
				strncpy(sock.ipaddr,optarg,sizeof(sock.ipaddr));
				break;
			case 'p':
				sock.port=atoi(optarg);
				break;
			case 'i':
				sample_interval=atoi(optarg);
				break;
			case 's':
				sn=optarg;
				strncpy(sq.filename,"client_sql.db",sizeof(sq.filename));
				strncpy(sq.table_name,optarg,sizeof(sq.table_name));
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
	
	if(sock.ipaddr[0] == 0 || sock.port==0 || sample_interval==0 || sn == NULL)
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

	memset(out_way,0,sizeof(out_way));

	if(daemon_run == 1)
	{
		if((zc=zlog_get_category("my_zlog")) == NULL)
		{
			printf("zlog get category fail:%s\n",strerror(errno));
			return -1;
		}
		if(daemon(1,0)<0)
		{
			zlog_error(zc,"set daemon fail:%s\n",strerror(errno));
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
	

	signal(SIGINT,proStop);
	signal(SIGTERM,proStop);

	if(sqlite_open(&sq)<0)
	{
		printf("sqlite open or create fail\n");
		return -1;
	}

	if(sqlite_check_table(&sq) == 0)
	{
		if(sqlite_create_table(&sq)<0)
		{
			zlog_error(zc,"sqlite create table[%s] fail\n",sq.table_name);
			return -1;
		}
	}

	time(&pretime);

	while(!pro_stop)
	{
		if(socket_init(&sock)<0)
		{
			zlog_error(zc,"socket init fail!\n");
			sqlite3_close(sq.db);
			return -1;
		}

        if(socket_connect(&sock)<0) 
        {
            close(sock.sockfd);
            time(&nowtime);

            if(nowtime-pretime<sample_interval)
            {
                reconnect=1;
            }
            else
            {
                reconnect=0;
            }
        }
        else
        {
            zlog_notice(zc,"client[%d] connect server[ip:%s,port:%d] success!\n",sock.sockfd,sock.ipaddr,sock.port);
            reconnect=0;
        }

		while(reconnect==0 && !pro_stop)
		{
			if(get_data(&dt)<0)
			{
				zlog_error(zc,"client[%d] get data fail\n",sock.sockfd);
				close(sock.sockfd);
				sqlite3_close(sq.db);
				break;
			}

			strncpy(dt.id,sn,sizeof(dt.id));
            time(&pretime);
			send_flag=1;
			
            while((nowtime-pretime)<sample_interval && !pro_stop)
            {
                if(socket_check(&sock)<0)	//socket is disconnecting
                {
                    if(sqlite_check_data(&sq,&dt) != 1) 
                    {
                        if(sqlite_add(&sq,&dt)<0)
                        {
                            zlog_error(zc,"add data to sql fail\n");
							close(sock.sockfd);
							sqlite3_close(sq.db);
                            return -1;
                        }

                        zlog_info(zc,"client[%d] add data(%s|%.2f|%s) to sql success!\n",
                                        sock.sockfd,dt.id,dt.temperature,dt.time);
                    }
					
					zlog_warn(zc,"client[%d] start to reconnect...\n",sock.sockfd);
                    reconnect=1;
                    break;
                }

				if(send_flag == 1)
				{

                	if (socket_write(&sock,&dt)<0)
                	{
						if(errno == 104)
						{
							continue;
						}

                    	zlog_error(zc,"send data fail\n");
						close(sock.sockfd);
						sqlite3_close(sq.db);
						time(&nowtime);
                    	continue;
                	}

                	zlog_info(zc,"client[%d] send sample_data(%s|%.2f|%s) to server success!\n",
                               	 sock.sockfd,dt.id,dt.temperature,dt.time);
				}

				send_flag=0;

                while(sqlite_getRows(&sq)>0 && !pro_stop)
                {
					time(&nowtime);

					if(nowtime-pretime >= sample_interval)
					{
						break;
					}

                    if(get_sqlData(&sq,&sql_dt)<0)
					{
						printf("get sqlite data fail\n");
						return -1;
					}

                    if(socket_write(&sock,&sql_dt)<0)
                    {
						if(errno == 104)
						{
							break;
						}

                        zlog_error(zc,"(sql)write to socket[%d] fail\n",sock.sockfd);
						close(sock.sockfd);
						sqlite3_close(sq.db);
                        break;
                    }

                    zlog_info(zc,"client[%d] send sqlite_data(%s|%.2f|%s) to server success!\n",
                                    sock.sockfd,sql_dt.id,sql_dt.temperature,sql_dt.time);

                    if(sqlite_del(&sq,&sql_dt)<0)
                    {
                        zlog_error(zc,"delete sqlite data fail\n");
						close(sock.sockfd);
						sqlite3_close(sq.db);
                        return -1;
                    }

                    zlog_info(zc,"client[%d] delete sqlite_data(%s|%.2f|%s) success!\n",
                                    sock.sockfd,sql_dt.id,sql_dt.temperature,sql_dt.time);
                }//end of while(sqlite_getRows(sq)>0....)

                time(&nowtime);

            }//end of while((nowtime-pretime)<sample_time...)

		}//end of while(reconnect==0...)
	
	}//end of while(pro_stop...)

	close(sock.sockfd);
	
	sqlite3_close(sq.db);

	zlog_fini();

	return 0;
}

