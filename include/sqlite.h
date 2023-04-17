/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sqlite.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(03/30/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/30/23 22:11:35"
 *                 
 ********************************************************************************/

#ifndef __SQLITE_H
#define __SQLITE_H
#include "sqlite3.h"
#include "get_data.h"
#include "zlog.h"

extern  zlog_category_t *zc;

typedef struct sqlite_s{
	sqlite3 *db;
	char filename[128];
	char table_name[128];
}sqlite_t;

int sqlite_open(sqlite_t *sqt);
int sqlite_check_table(sqlite_t *sqt);
int sqlite_create_table(sqlite_t *sqt);
int sqlite_add(sqlite_t *sqt,data_t *dt);
int sqlite_del(sqlite_t *sqt,data_t *dt);
int sqlite_check_data(sqlite_t *sqt,data_t *dt);
int sqlite_getRows(sqlite_t *sqt);
int get_sqlData(sqlite_t *sqt,data_t *dt);

#endif


