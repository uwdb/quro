#include "Bid.h"
#include "TxnBaseDB.h"
#include "Bid_const.h"
#include "DBConnection.h"

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


	sprintf(query, SELECT_ITEM, pIn->item_id);
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_ITEM");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		cur_price = atof(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		item_description.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_ITEM FAIL");
	}

	sprintf(query, GET_USER, pIn->user_id);
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_USERS");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		bid = atol(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		user_name.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_USER FAIL");
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

	sprintf(query, GET_USER, pIn->user_id);
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_USERS");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		bid = atol(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		user_name.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_USER FAIL");
	}

	BIDDING_UPDATE_USER;

	sprintf(query, SELECT_ITEM, pIn->item_id);
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_ITEM");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		cur_price = atof(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		item_description.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_ITEM FAIL");
	}


	BIDDING_INSERT_REC;
	if(bid_price > cur_price){
		BIDDING_UPDATE_ITEM;
	}
*/
}




