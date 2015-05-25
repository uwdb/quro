#include "Bid.h"
#include "TxnBaseDB.h"
#include "Bid_const.h"
#include "DBConnection.h"

#define FAIL_MSG(msg) \
				string fail_msg(msg); \
				fail_msg.append("\n"); \
				fail_msg.append(query); \
				if (r==2) fail_msg.append("\tempty result"); \
				throw fail_msg.c_str();

#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#define GETTIME gettimeofday(&t1, NULL);
#else
#define ADD_QUERY_NODE(f, q, commit)
#define GETTIME
#endif
string get_random_string(int length){
	char c[1000] = {0};
	int len = length/2 + rand()%(length/2);
	for(int i=0; i<len; i++){
		c[i] = rand()%26+'a';
	}
	string s(c);
	return s;
}

void CBiddingDB::DoBidding(TBiddingTxnInput* pIn, TBiddingTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CBiddingDB::execute(const TBiddingTxnInput* pIn, TBiddingTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TBiddingTxnInput* pIn, TBiddingTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;

	char query[4096];
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
	double bid_price;

	sprintf(query, GET_USER, pIn->user_id);
	GETTIME;
	r = dbt5_sql_execute(query, &result, "GET_USERS");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		ADD_QUERY_NODE(1, 1, 1);
		bid = atol(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		user_name.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_USER FAIL");
	}

	sprintf(query, SELECT_ITEM, pIn->item_id);
	GETTIME;
	r = dbt5_sql_execute(query, &result, "GET_ITEM");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		ADD_QUERY_NODE(1, 2, 1);
		cur_price = atof(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		item_description.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_ITEM FAIL");
	}

	if(double(rand()%128)/128.0 < HIGHER_BID_THRESHOLD)
					bid_price = cur_price + double(rand()%16)/16.0;
	else
					bid_price = cur_price - double(rand()%128)/128.0;


	if(bid_price > cur_price){
		BIDDING_UPDATE_ITEM;
	}

	BIDDING_UPDATE_USER;

	BIDDING_INSERT_REC;

/*
	BIDDING_INSERT_REC;
	BIDDING_UPDATE_USER;
	if(bid_price > cur_price){
		BIDDING_UPDATE_ITEM;
	}
*/
}




