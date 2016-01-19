#include "DBConnection.h"
#include "TradeResultDB.h"

#ifndef DB_PGSQL

#ifdef PROFILE_EACH_QUERY
#define ADD_PROFILE_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#endif

#define FAIL_MSG(msg) \
				string fail_msg(msg); \
				throw fail_msg.c_str();

void CDBConnection::execute(const TTradeResultFrame1Input *pIn,
		TTradeResultFrame1Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	sprintf(query, TRADE_RESULT1_1, pIn->trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
  r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->acct_id = atol(dbt5_sql_getvalue(&result, 0, length));

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(pOut->type_id, val, length);

			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(pOut->symbol, val, length);

			pOut->trade_qty = atol(dbt5_sql_getvalue(&result, 3, length));

			pOut->charge = atof(dbt5_sql_getvalue(&result, 4, length));

			pOut->is_lifo = atoi(dbt5_sql_getvalue(&result, 5, length));
			pOut->trade_is_cash = atoi(dbt5_sql_getvalue(&result, 6, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 1, 1);
#endif
	}else{
				FAIL_MSG("trade result frame1 query 1 fails...");
	}


	sprintf(query, TRADE_RESULT1_2, pOut->type_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_2");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(pOut->type_name, val, length);

			pOut->type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
			pOut->type_is_market = atoi(dbt5_sql_getvalue(&result, 2, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 2, 1);
#endif
	}else{
			FAIL_MSG("trade result frame1 query 2 fails");
	}


	sprintf(query, TRADE_RESULT1_3, pOut->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 3, 1);
#endif
	}else{
			FAIL_MSG("trade result frame1 query 3 fails");
	}

}

void CDBConnection::execute(const TTradeResultFrame2Input *pIn,
		TTradeResultFrame2Output *pOut)
{

	char query[4096];
	sql_result_t result;
	sql_result_t result_t;
	int length;
	char* val;
	int r = 0;
	bool tt_is_sell = false;
	bool tt_is_mrkt = false;
	long unsigned int hold_id;
	long int hold_qty;
	double hold_price;
	long int needed_qty = pIn->trade_qty;
	double buy_value = 0.0;
	double sell_value = 0.0;
	char now_dts[100]={0};
	sprintf(query, TRADE_RESULT_HELPER);

	r = dbt5_sql_execute(query, &result, "TRADE_RESULT_HELPER");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(now_dts, val, length);
	}

	sprintf(query, TRADE_RESULT2_1, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->broker_id = atol(dbt5_sql_getvalue(&result, 0, length));
			pOut->cust_id = atol(dbt5_sql_getvalue(&result, 1, length));

			pOut->tax_status = atoi(dbt5_sql_getvalue(&result, 2, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(2, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame2 query 1 fails");
	}


	if(pIn->type_is_sell){
			if(pIn->hs_qty == 0){
					sprintf(query, TRADE_RESULT2_2a, pIn->acct_id, pIn->symbol, (-1)*pIn->trade_qty);
					r=0;
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a")){
							FAIL_MSG("trade result frame2 query 2 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 2, 1);
#endif
			}else if(pIn->hs_qty != pIn->trade_qty){
					sprintf(query, TRADE_RESULT2_2b, pIn->hs_qty-pIn->trade_qty, pIn->acct_id, pIn->symbol);
					r=0;
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_2b")){
							FAIL_MSG("trade result frame2 query 3 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 3, 1);
#endif
			}

			size_t num_rows = 0;
			size_t cnt = 0;
			if(pIn->hs_qty > 0){
					if(pIn->is_lifo){
							sprintf(query, TRADE_RESULT2_3a, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
							gettimeofday(&t1, NULL);
#endif
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows = result_t.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 4, 1);
#endif
							}else{
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 4, 0);
#endif
									FAIL_MSG("trade result frame2 query 4 fails");
							}
					}else{
							sprintf(query, TRADE_RESULT2_3b, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
							gettimeofday(&t1, NULL);
#endif
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3b");
							if(r==1 && result.result_set){
									num_rows = result_t.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 5, 1);
#endif
							}else{
#ifdef PROFILE_EACH_QUERY
											ADD_PROFILE_NODE(2, 5, 0);
#endif
									FAIL_MSG("trade result frame2 query 5 fails");
							}
					}
					r = 0;
					while(needed_qty > 0 && cnt < num_rows){
							dbt5_sql_fetchrow(&result_t);
							cnt++;

							hold_id = atol(dbt5_sql_getvalue(&result_t, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result_t, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result_t, 2, length));

							if(hold_qty > needed_qty){
									sprintf(query, TRADE_RESULT2_4a, hold_id, pIn->trade_id, hold_qty, hold_qty-needed_qty);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
											FAIL_MSG("trade result frame2 query 6 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 6, 1);
#endif
									sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a")){
											FAIL_MSG("trade result frame2 query 7 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 7, 1);
#endif
									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * pIn->trade_price);
									needed_qty = 0;
							}else{
									sprintf(query, TRADE_RESULT2_4a, hold_id, pIn->trade_id, hold_qty, 0);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
											FAIL_MSG("trade result frame2 query 8 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 8, 1);
#endif
									sprintf(query, TRADE_RESULT2_5b, hold_id);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b")){
											FAIL_MSG("trade result frame2 query 9 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 9, 1);
#endif
									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * pIn->trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){
					sprintf(query, TRADE_RESULT2_4a, pIn->trade_id, pIn->trade_id, 0, (-1)*needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
							FAIL_MSG("trade result frame2 query 10 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 10, 1);
#endif
					sprintf(query, TRADE_RESULT2_7a, pIn->trade_id, pIn->acct_id, pIn->symbol, now_dts, pIn->trade_price, (-1)*needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){
							FAIL_MSG("trade result frame2 query 11 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 11, 1);
#endif
			}else if(pIn->hs_qty == pIn->trade_qty){
					sprintf(query, TRADE_RESULT2_7b, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b")){
							FAIL_MSG("trade result frame2 query 12 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 12, 1);
#endif
			}
	}
	//type_is_market
	else{
			if(pIn->hs_qty == 0){
					sprintf(query, TRADE_RESULT2_8a, pIn->acct_id, pIn->symbol, pIn->trade_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_8a")){
							FAIL_MSG("trade result frame2 query 13 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 13, 1);
#endif
			}else{
					sprintf(query, TRADE_RESULT2_8b, pIn->hs_qty+pIn->trade_qty, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_8b")){
							FAIL_MSG("trade result frame2 query 14 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 14, 1);
#endif
			}
			size_t num_rows = 0;
			size_t cnt = 0;

			if(pIn->hs_qty < 0){
					if(pIn->is_lifo){
							sprintf(query, TRADE_RESULT2_3a, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
							gettimeofday(&t1, NULL);
#endif
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows = result_t.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 15, 1);
#endif
							}else{
#ifdef PROFILE_EACH_QUERY
											ADD_PROFILE_NODE(2, 15, 0);
#endif
									FAIL_MSG("trade result frame2 query 15 fails");
							}
					}else{
							sprintf(query, TRADE_RESULT2_3a, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
							gettimeofday(&t1, NULL);
#endif
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows = result_t.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 16, 1);
#endif
							}else{
#ifdef PROFILE_EACH_QUERY
											ADD_PROFILE_NODE(2, 16, 0);
#endif
									FAIL_MSG("trade result frame2 query 16 fails");
							}
					}
					while(needed_qty>0 && cnt<num_rows){
							dbt5_sql_fetchrow(&result_t);
							cnt++;
							hold_id = atol(dbt5_sql_getvalue(&result_t, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result_t, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result_t, 2, length));
							if(hold_qty > needed_qty){
									sprintf(query, TRADE_RESULT2_4a, hold_id, pIn->trade_id, hold_qty, hold_qty+needed_qty);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
											FAIL_MSG("trade result frame2 query 17 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 17, 1);
#endif
									sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a")){
											FAIL_MSG("trade result frame2 query 18 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 18, 1);
#endif

									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * pIn->trade_price);
									needed_qty = 0;
							}else{
									sprintf(query, TRADE_RESULT2_4a, hold_id, pIn->trade_id, hold_qty, 0);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_REUSLT2_4a")){
											FAIL_MSG("trade result frame2 query 19 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 19, 1);
#endif
									sprintf(query, TRADE_RESULT2_5b, hold_id);
#ifdef PROFILE_EACH_QUERY
									gettimeofday(&t1, NULL);
#endif
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b")){
											FAIL_MSG("trade result frame2 query 20 fails");
									}
#ifdef PROFILE_EACH_QUERY
									ADD_PROFILE_NODE(2, 20, 1);
#endif
									hold_qty = (-1)*hold_qty;
									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * pIn->trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){
					sprintf(query, TRADE_RESULT2_4a, pIn->trade_id, pIn->trade_id, 0, needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
							FAIL_MSG("trade result frame2 query 21 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 21, 1);
#endif

					sprintf(query, TRADE_RESULT2_7a, pIn->trade_id, pIn->acct_id, pIn->symbol, now_dts, pIn->trade_price, needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){
							FAIL_MSG("trade result frame2 query 22 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 22, 1);
#endif
			}else if((-1)*pIn->hs_qty == pIn->trade_qty){
					sprintf(query, TRADE_RESULT2_7b, pIn->acct_id, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b")){
							FAIL_MSG("trade result frame2 query 23 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 23, 1);
#endif
			}
	}
	pOut->buy_value = buy_value;
	pOut->sell_value = sell_value;

	sscanf(now_dts, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&pOut->trade_dts.year,
			&pOut->trade_dts.month,
			&pOut->trade_dts.day,
			&pOut->trade_dts.hour,
			&pOut->trade_dts.minute,
			&pOut->trade_dts.second);

}

void CDBConnection::execute(const TTradeResultFrame3Input *pIn,
		TTradeResultFrame3Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	double tax_rate;
	sprintf(query, TRADE_RESULT3_1, pIn->cust_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT3_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			tax_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(3, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame3 query 1 fails");
	}


	sprintf(query, TRADE_RESULT3_2, tax_rate*(pIn->sell_value - pIn->buy_value), pIn->trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT3_2")){
			FAIL_MSG("trade result frame3 query 2 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(3, 2, 1);
#endif
	pOut->tax_amount = tax_rate*(pIn->sell_value - pIn->buy_value);
}

void CDBConnection::execute(const TTradeResultFrame4Input *pIn,
		TTradeResultFrame4Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	char sec_ex_id[20]={0};
	int cust_tier;
	int r = 0;

	sprintf(query, TRADE_RESULT4_1, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(sec_ex_id, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(pOut->s_name, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 1 fails");
	}


	sprintf(query, TRADE_RESULT4_2, pIn->cust_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_2");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			cust_tier = atoi(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 2, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 2 fails");
	}


	sprintf(query, TRADE_RESULT4_3, cust_tier, pIn->type_id, sec_ex_id, pIn->trade_qty, pIn->trade_qty);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 3, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 3 fails");
	}
}

void CDBConnection::execute(const TTradeResultFrame5Input *pIn)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	char now_dts[20]={0};
	sprintf(now_dts, "%d-%d-%d %d:%d:%d", pIn->trade_dts.year, pIn->trade_dts.month, pIn->trade_dts.day, pIn->trade_dts.hour, pIn->trade_dts.minute, pIn->trade_dts.second);

	sprintf(query, TRADE_RESULT5_1, pIn->comm_amount, now_dts, pIn->st_completed_id, pIn->trade_price, pIn->trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_1")){
			FAIL_MSG("trade result frame5 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 1, 1);
#endif

	sprintf(query, TRADE_RESULT5_2, pIn->trade_id, now_dts, pIn->st_completed_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_2")){
			FAIL_MSG("trade result frame5 query 2 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 2, 1);
#endif

	sprintf(query, TRADE_RESULT5_3, pIn->comm_amount, pIn->broker_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_3")){
			FAIL_MSG("trade result frame5 query 3 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 3, 1);
#endif
}

void CDBConnection::execute(const TTradeResultFrame6Input *pIn,
		TTradeResultFrame6Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	char cash_type[10]={0};
	if(pIn->trade_is_cash){
			sprintf(cash_type, "Cash Account");
	}else{
			sprintf(cash_type, "Margin");
	}

	char due_dts[20]={0};
	sprintf(due_dts, "%d-%d-%d %d:%d:%d", pIn->due_date.year, pIn->due_date.month, pIn->due_date.day, pIn->due_date.hour, pIn->due_date.minute, pIn->due_date.second);

	sprintf(query, TRADE_RESULT6_1, pIn->trade_id, cash_type, due_dts, pIn->se_amount);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_1")){
			FAIL_MSG("trade result frame6 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(6, 1, 1);
#endif

	char now_dts[20]={0};
	sprintf(now_dts, "%d-%d-%d %d:%d:%d", pIn->trade_dts.year, pIn->trade_dts.month, pIn->trade_dts.day, pIn->trade_dts.hour, pIn->trade_dts.minute, pIn->trade_dts.second);

	if(pIn->trade_is_cash){
			sprintf(query, TRADE_RESULT6_2, pIn->se_amount, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_2")){
					FAIL_MSG("trade result frame6 query 2 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 2, 1);
#endif

			sprintf(query, TRADE_RESULT6_3, now_dts, pIn->trade_id, pIn->se_amount, pIn->type_name, pIn->trade_qty, pIn->s_name);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_3")){
					FAIL_MSG("trade result frame6 query 3 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 3, 1);
#endif

			sprintf(query, TRADE_RESULT6_4, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					pOut->acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(6, 4, 1);
#endif
			}else{
					FAIL_MSG("trade result frame6 query4 fails");
			}

	}
}
#endif
