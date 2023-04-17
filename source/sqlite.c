/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/29/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/29/23 08:30:01"
 *                 
 ********************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include"sqlite3.h"
#include"sqlite.h"
#include"get_data.h"
#include"zlog.h"
#include"semaphore.h"


int sqlite_open(sqlite_t *sqt)
{
	char *errmsg=NULL;

	if((semid=sem_init())<0)
	{
	
		zlog_error(zc,"semaphore init fail:%s\n",strerror(errno));
		return -1;
	}
	
	if(sqlite3_open(sqt->filename,&(sqt->db))!= SQLITE_OK)
	{
		zlog_error(zc,"%s\n",sqlite3_errmsg(sqt->db));
		return -2;
	}

	return 0;
}



int sqlite_check_table(sqlite_t *sqt)
{
	char sql[128];
	char *errmsg=NULL;

	memset(sql,0,sizeof(sql));
	
	sprintf(sql,"select * from %s;",sqt->table_name);
	
	if(sqlite3_exec(sqt->db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		return 0;
	}

	return 1;
}


int sqlite_create_table(sqlite_t *sqt)
{
	char sql[128];
	char *errmsg=NULL;

	memset(sql,0,sizeof(sql));
		
	sprintf(sql,"create table %s(sn,temperature,time);",sqt->table_name);
		
	sqlite3_busy_timeout(sqt->db,1000);
		
	if(sqlite3_exec(sqt->db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}



 
int sqlite_add(sqlite_t *sqt,data_t *dt)
{

	char sql[128];
	char *errmsg=NULL;

	sprintf(sql,"insert into %s values('%s',%.2f,'%s');",sqt->table_name,dt->id,dt->temperature,dt->time);
  	
	sqlite3_busy_timeout(sqt->db,1000);
	
	sem_P(semid);
	
	if(sqlite3_exec(sqt->db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	
	sem_V(semid);
	
	return 0;
}



int sqlite_del(sqlite_t *sqt,data_t *dt)
{
	char sql[1024];
	char *errmsg=NULL;

	sprintf(sql,"delete from %s where sn='%s' and temperature=%.2f and time='%s';",
			sqt->table_name,dt->id,dt->temperature,dt->time);
	
	sqlite3_busy_timeout(sqt->db,1000);
	
	sem_P(semid);
	
	if(sqlite3_exec(sqt->db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	
	sem_V(semid);
	
	return 0;
}




int sqlite_getRows(sqlite_t *sqt)
{
	char sql[128];
	char *errmsg=NULL;
	char **azResult=NULL;
	int nrow=0;
	int ncolumn=0;

	sprintf(sql,"select * from %s;",sqt->table_name);
	
	if(sqlite3_get_table(sqt->db,sql,&azResult,&nrow,&ncolumn,&errmsg)!=SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	
	return nrow;
}



//check if the data is in the sqlite
int sqlite_check_data(sqlite_t *sqt,data_t *dt)
{
	char temperature[64];
	char sql[1024];
	char *errmsg=NULL;
	char **azResult=NULL;
	int nrow=0;
	int ncolumn=0;
	int flag=-1;
	
	sprintf(sql,"select * from %s;",sqt->table_name);
	
	if(sqlite3_get_table(sqt->db,sql,&azResult,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	int i=(nrow+1)*ncolumn;
	
	while(i>3)
	{
				
		sprintf(azResult[i-2],"%.2f",atof(azResult[i-2]));
		
		memset(temperature,0,sizeof(temperature));
		
		sprintf(temperature,"%.2f",dt->temperature);

		if(strcmp(azResult[i-3],dt->id)==0 && atof(azResult[i-2])==atof(temperature) && strcmp(azResult[i-1],dt->time)==0)
		{
			flag=1;
			break;
		}
		
		i=i-3;
	}

	return flag;
}



//get the first record from sqlite
int get_sqlData(sqlite_t *sqt,data_t *dt)
{
	char sql[128];
	char *errmsg=NULL;
	char **azResult=NULL;
	int nrow=0;
	int ncolumn=0;

	sprintf(sql,"select * from %s;",sqt->table_name);
	
	if(sqlite3_get_table(sqt->db,sql,&azResult,&nrow,&ncolumn,&errmsg) != SQLITE_OK)
	{
		zlog_error(zc,"%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	
	strncpy(dt->time,azResult[ncolumn+2],sizeof(dt->time));
	
	dt->temperature=atof(azResult[ncolumn+1]);
	
	strncpy(dt->id,azResult[ncolumn],sizeof(dt->id));

	return 0;
}

