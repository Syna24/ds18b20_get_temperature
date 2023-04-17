#ifndef __GET_DATA_H
#define __GET_DATA_H

#include"zlog.h"

extern  zlog_category_t *zc;

typedef struct data_s{
	char id[256];
	float temperature;
	char time[256];
}data_t;

int get_data(data_t *dt);
#endif

