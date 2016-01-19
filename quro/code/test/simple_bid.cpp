#include "simple_bid.h"
#include "mysql_helper.h"


void execute(uint64_t item_id, uint64_t user_id, double bid_price){
  char query[4096];
	char query1[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	string user_name;
	string item_description;
	string rec_description = get_random_string(128);
	uint64_t bid;
	uint64_t pid;
	double cur_price;


	sprintf(query, SELECT_ITEM, item_id);
	r = dbt5_sql_execute(query, &result, "query0");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		cur_price = atof(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		item_description.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		string msg("GET_ITEM FAIL");
		throw msg.c_str();
	}

	sprintf(query, GET_USER, user_id);
	r = dbt5_sql_execute(query, &result, "query1");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		bid = atol(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		user_name.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		string msg("GET_USER FAIL");
		throw msg.c_str();
	}


	if(bid_price > cur_price){
			sprintf(query1, UPDATE_ITEM, bid_price, item_id); 
			r = dbt5_sql_execute(query1, &result, "query2");
			if(!r){
				string msg("UPDATE ITEM FAIL");
				throw msg.c_str();
			}
	}

	sprintf(query, UPDATE_USER, user_id); 
	r = dbt5_sql_execute(query, &result, "query3");
  if(!r){
		string msg("UPDATE USER FAIL");
		throw msg.c_str();
	}

	sprintf(query, INSERT_BID, user_id, bid+1, item_id, bid_price, rec_description.c_str()); 
	r = dbt5_sql_execute(query, &result, "query4");
	if(!r){
		string msg("INSERT REC FAIL");
		throw msg.c_str();
	}

	return ;
}


	

