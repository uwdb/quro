#include "mysql_helper.h"

//Most of these are empty functions, as a simple wrapper...

int mysql_query(char query[1024], char* result){
	int r = rand()%128;
  return r;
}

int dbt5_sql_execute(char query[1024], sql_result_t* result, const char* description){
	char* c = new char[1024];
  memset(c, 0, sizeof(char)*1024);
	if(mysql_query(query, c)){
  	result->result_set = c;
  	return 1;
	}else{
		result->result_set = 0;
		return 0;
	}
}

char* dbt5_sql_getvalue(sql_result_t* result, int field, int length){
	char* c = new char[1024];
	memcpy(c, (const void*)(result->result_set+field*length), length);
	return c;
}

void dbt5_sql_fetchrow(sql_result_t* result){
}

void dbt5_sql_close_cursor(sql_result_t* result){
}

string get_random_string(int length){
  char s[1024] = {0};
	for(int i=0; i<length; i++)
		s[i] = (char)(i*2%128);
	string r(s);
	return r;
}
