#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
using namespace std;

#ifndef MYSQL_HEADER_H_
#define MYSQL_HEADER_H_

struct sql_result_t{
	char* result_set;
};


int mysql_query(char query[1024], char* result);
int dbt5_sql_execute(char query[1024], sql_result_t* result, const char* description);
char* dbt5_sql_getvalue(sql_result_t* result, int field, int length);
void dbt5_sql_fetchrow(sql_result_t* result);
void dbt5_sql_close_cursor(sql_result_t* result);
string get_random_string(int length);

#endif
