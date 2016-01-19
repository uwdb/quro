#include "DBConnection.h"
#include "TradeOrderDB.h"

#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#endif

#ifndef DB_PGSQL
void CDBConnection::execute(const TTradeOrderFrame1Input *pIn,
		TTradeOrderFrame1Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;


	sprintf(query, SQLTOF1_1, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_1")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(pOut->acct_name, val, length);

		 	pOut->broker_id = atol(dbt5_sql_getvalue(&result, 1, length));

			pOut->cust_id = atol(dbt5_sql_getvalue(&result, 2, length));

			pOut->tax_status = atoi(dbt5_sql_getvalue(&result, 3, length));

			pOut->num_found = result.num_rows;
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 1, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 1 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF1_2, pOut->cust_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_2")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(pOut->cust_f_name, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(pOut->cust_l_name, val, length);

			pOut->cust_tier = atoi(dbt5_sql_getvalue(&result, 2, length));

			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(pOut->tax_id, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 2, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 2 fails");
			throw fail_msg.c_str();
	}


	sprintf(query, SQLTOF1_3, pOut->broker_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif

	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_3")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(pOut->broker_name, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 3, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 3 fails");
			throw fail_msg.c_str();
	}

	dbt5_sql_close_cursor(&result);
}

void CDBConnection::execute(const TTradeOrderFrame2Input *pIn,
		TTradeOrderFrame2Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;

	sprintf(query, SQLTOF2_1, pIn->acct_id, pIn->exec_f_name, pIn->exec_l_name, pIn->exec_tax_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				val = dbt5_sql_getvalue(&result, 0, length);
				strncpy(pOut->ap_acl, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(2, 1, 1);
#endif
	}else{
			string fail_msg("trade order frame2 query 1 fails");
			throw fail_msg.c_str();
	}
	dbt5_sql_close_cursor(&result);
}

void CDBConnection::execute(const TTradeOrderFrame3Input *pIn,
		TTradeOrderFrame3Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	long unsigned int co_id;
	bool tt_is_sell = false;
	bool tt_is_mrkt = false;
	long int hs_qty;
	long int needed_qty = pIn->trade_qty;
	long int hold_qty;
	double buy_value = 0.0;
	double sell_value = 0.0;
	double hold_price;
	double acct_bal;
	char exch_id[7] = {0};

	if(strlen(pIn->symbol)==0){
		sprintf(query, SQLTOF3_1a, pIn->co_name);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				co_id = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 1, 1);
#endif
		}else{
				outfile<<"error query:"<<query<<endl;
				outfile.flush();
				string fail_msg("trade order frame3 query 1 fails");
				throw fail_msg.c_str();
		}

		sprintf(query, SQLTOF3_2a, co_id, pIn->issue);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_5")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				val = dbt5_sql_getvalue(&result, 0, length);
				strncpy(exch_id, val, 6);

				val = dbt5_sql_getvalue(&result, 1, length);
				strncpy(pOut->s_name, val, length);

				val = dbt5_sql_getvalue(&result, 2, length);
				strncpy(pOut->symbol, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 2, 1);
#endif
		}else{
				string fail_msg("trade order frame3 query 2 fails");
				throw fail_msg.c_str();
		}

	}else{
		strcpy(pOut->symbol, pIn->symbol);
		sprintf(query, SQLTOF3_1b, pIn->symbol);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4b")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				co_id = atol(dbt5_sql_getvalue(&result, 0, length));

				val = dbt5_sql_getvalue(&result, 1, length);
				strncpy(exch_id, val, 6);

				val = dbt5_sql_getvalue(&result, 2, length);
				strncpy(pOut->s_name, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 3, 1);
#endif
		}else{
				string fail_msg("trade order frame3 query 3 fails");
				throw fail_msg.c_str();
		}

		sprintf(query, SQLTOF3_2b, co_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_5b")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				val = dbt5_sql_getvalue(&result, 0, length);
				strncpy(pOut->co_name, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 4, 1);
#endif
		}else{
				string fail_msg("trade order frame3 query 4 fails");
				throw fail_msg.c_str();
		}

	}

	sprintf(query, SQLTOF3_3, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_6")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->market_price = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 5, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 5 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF3_4, pIn->trade_type_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_7")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			tt_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
			tt_is_mrkt = atoi(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 6, 1);
#endif

	}else{
			string fail_msg("trade order frame3 query 6 fails");
			throw fail_msg.c_str();
	}

	if(tt_is_mrkt){
			pOut->requested_price = pOut->market_price;
	}else{
			pOut->requested_price = pIn->requested_price;
	}


	sprintf(query, SQLTOF3_5, pIn->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_8")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 7, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 7 fails");
			throw fail_msg.c_str();
	}

	size_t num_rows = 0;
	size_t cnt = 0;

	if(tt_is_sell){
			if(hs_qty > 0){
					if(pIn->is_lifo){

							sprintf(query, SQLTOF3_6a, pIn->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
								gettimeofday(&t1, NULL);
#endif
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_9") && result.result_set){
									num_rows = result.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_QUERY_NODE(3, 8, 1);
#endif
							}else{
									string fail_msg("trade order frame3 query 8 fails");
									throw fail_msg.c_str();
							}
					}else{
							sprintf(query, SQLTOF3_6b, pIn->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
								gettimeofday(&t1, NULL);
#endif
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_10") && result.result_set){
									num_rows = result.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_QUERY_NODE(3, 9, 1);
#endif
							}else{
									string fail_msg("trade order frame3 query 9 fails");
									throw fail_msg.c_str();
							}
					}
				while(needed_qty>0 && cnt < num_rows){
					//read from holding results
					dbt5_sql_fetchrow(&result);

					hold_qty = atol(dbt5_sql_getvalue(&result, 0, length));
					hold_price = atof(dbt5_sql_getvalue(&result, 1, length));
					cnt++;
						if(hold_qty > needed_qty){
								buy_value = buy_value + (needed_qty * hold_price);
								sell_value = sell_value + (needed_qty * pIn->requested_price);
								needed_qty = 0;
						}else{
								buy_value = buy_value + (hold_qty * hold_price);
								sell_value = sell_value + (hold_qty * pIn->requested_price);
								needed_qty = needed_qty - hold_qty;
					}
				}
			}
	}else{
			if (hs_qty < 0){

					if(pIn->is_lifo){
							sprintf(query, SQLTOF3_6a, pIn->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
								gettimeofday(&t1, NULL);
#endif
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_9") && result.result_set){
									num_rows = result.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_QUERY_NODE(3, 10, 1);
#endif
							}else{
									string fail_msg("trade order frame3 query 10 fails");
									throw fail_msg.c_str();
							}
					}else{
							sprintf(query, SQLTOF3_6b, pIn->acct_id, pOut->symbol);
#ifdef PROFILE_EACH_QUERY
								gettimeofday(&t1, NULL);
#endif
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_10") && result.result_set){
									num_rows = result.num_rows;
#ifdef PROFILE_EACH_QUERY
									ADD_QUERY_NODE(3, 11, 1);
#endif
							}else{
									string fail_msg("trade order frame3 query 11 fails");
									throw fail_msg.c_str();
							}
					}
					while(needed_qty>0 && cnt < num_rows){
						dbt5_sql_fetchrow(&result);

						hold_qty = atol(dbt5_sql_getvalue(&result, 0, length));
						hold_price = atof(dbt5_sql_getvalue(&result, 1, length));
						cnt++;
							//read from holding results
							if(hold_qty + needed_qty < 0){
									sell_value = sell_value + (needed_qty * hold_price);
									buy_value = buy_value + (needed_qty * pIn->requested_price);
									needed_qty = 0;
							}else{
									hold_qty = 0-hold_qty;
									sell_value = sell_value + (hold_qty * hold_price);
									buy_value = buy_value + (hold_qty * pIn->requested_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
	}

	double tax_rates = 0.0;
	if(sell_value > buy_value && ((pIn->tax_status == 1 ) || (pIn->tax_status == 2))){
			sprintf(query, SQLTOF3_7, pIn->cust_id);
#ifdef PROFILE_EACH_QUERY
				gettimeofday(&t1, NULL);
#endif
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_11")==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					tax_rates = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_QUERY_NODE(3, 12, 1);
#endif
			}else{
					string fail_msg("trade order frame3 query 12 fails");
					throw fail_msg.c_str();
			}
			pOut->tax_amount = (sell_value - buy_value) * tax_rates;
	}

	sprintf(query, SQLTOF3_8, pIn->cust_tier, pIn->trade_type_id, exch_id, pIn->trade_qty, pIn->trade_qty);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_12")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 13, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 13 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF3_9, pIn->cust_tier, pIn->trade_type_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_13")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->charge_amount = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 14, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 14 fails");
			throw fail_msg.c_str();
	}

	if(pIn->type_is_margin){
			sprintf(query, SQLTOF3_10, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
				gettimeofday(&t1, NULL);
#endif
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_14")==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_QUERY_NODE(3, 15, 1);
#endif
			}else{
					string fail_msg("trade order frame3 query 15 fails");
					throw fail_msg.c_str();
			}

			sprintf(query, SQLTOF3_11, pIn->acct_id);
#ifdef PROFILE_EACH_QUERY
				gettimeofday(&t1, NULL);
#endif
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_15")==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					pOut->acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_QUERY_NODE(3, 16, 1);
#endif
			}else{
					if(result.num_rows==0){
							pOut->acct_assets = acct_bal;
					}
			}
	}
	if(pIn->type_is_margin){
			strcpy(pOut->status_id, pIn->st_submitted_id);
	}else{
			strcpy(pOut->status_id, pIn->st_pending_id);
	}
	pOut->buy_value = buy_value;
	pOut->sell_value = sell_value;
	pOut->type_is_market = tt_is_mrkt;
	pOut->type_is_sell = tt_is_sell;

}

void CDBConnection::execute(const TTradeOrderFrame4Input *pIn,
		TTradeOrderFrame4Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;

	unsigned long next_t_id;

	//FIXME: now_dts and trade_id
	sprintf(query, SQLTOF4_0);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_16")){
			string fail_msg("trade order frame 4 query 1 fails");
			throw fail_msg.c_str();
	}
#ifdef PROFILE_EACH_QUERY
	ADD_QUERY_NODE(4, 1, 1);
#endif

	sprintf(query, SQLTOF4_00);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_17")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
//			next_t_id = atol(dbt5_sql_getvalue(&result, 0, length));
			val = dbt5_sql_getvalue(&result, 0, length);
//			outfile<<"val = "<<val<<endl;
//			outfile.flush();
			next_t_id = atol(val);
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(4, 2, 1);
#endif
	}else{
			string fail_msg("trade order frame4 query 2 fails");
			throw fail_msg.c_str();
	}
	/*
	outfile<<"start printing..."<<endl;
	outfile<<pIn->status_id<<endl;
	outfile<<pIn->trade_type_id<<endl;
	outfile<<pIn->is_cash<<endl;
	outfile<<pIn->symbol<<endl;
	outfile<<pIn->trade_qty<<endl;
	outfile<<pIn->requested_price<<endl;
	outfile<<pIn->acct_id<<endl;
	outfile<<pIn->exec_name<<endl;
	outfile<<pIn->charge_amount<<endl;
	outfile<<pIn->comm_amount<<endl;
	outfile<<pIn->is_lifo<<endl;
	outfile.flush();*/
	sprintf(query, SQLTOF4_1, next_t_id, pIn->status_id, pIn->trade_type_id, pIn->is_cash, pIn->symbol, pIn->trade_qty, pIn->requested_price, pIn->acct_id, pIn->exec_name, pIn->charge_amount, pIn->comm_amount, pIn->is_lifo);
//	outfile<<"query: "<<query<<endl;
//	outfile.flush();
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_18")){
			string fail_msg("trade order frame4 query 3 fails");
			throw fail_msg.c_str();
	}
#ifdef PROFILE_EACH_QUERY
	ADD_QUERY_NODE(4, 3, 1);
#endif

	if(pIn->type_is_market == 0){
			sprintf(query, SQLTOF4_2, next_t_id, pIn->trade_type_id, pIn->symbol, pIn->trade_qty, pIn->requested_price, pIn->broker_id);

#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_19")){
					outfile<<"query fails: "<<query<<endl;
					string fail_msg("trade order frame4 query 4 fails");
					throw fail_msg.c_str();
			}
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(4, 4, 1);
#endif
	}

	sprintf(query, SQLTOF4_3, next_t_id, pIn->status_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_20")){
			string fail_msg("trade order frame4 query 5 fails");
			throw fail_msg.c_str();
	}
#ifdef PROFILE_EACH_QUERY
	ADD_QUERY_NODE(4, 5, 1);
#endif
	pOut->trade_id = next_t_id;
}
#endif
