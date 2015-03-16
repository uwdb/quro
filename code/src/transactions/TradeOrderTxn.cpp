#include "DBConnection.h"
#include "TradeOrderDB.h"

#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#endif

#ifndef DB_PGSQL
void CDBConnection::execute(PTradeOrderTxnInput pIn,
		PTradeOrderIntermediate pInter,
		PTradeOrderTxnOutput pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;

	long unsigned int acct_id =  pIn->acct_id;
	char exec_f_name[30] = {0};
	char exec_l_name[30] = {0};
	char exec_tax_id[20] = {0};
	strncpy(exec_f_name, pIn->exec_f_name, sizeof(pIn->exec_f_name));
	strncpy(exec_l_name, pIn->exec_l_name, sizeof(pIn->exec_l_name));
	strncpy(exec_tax_id, pIn->exec_tax_id, sizeof(pIn->exec_tax_id));
	long unsigned int broker_id;
	long unsigned int cust_id;
	long unsigned int trade_id;
	unsigned long next_t_id;
	int tax_status;
	char cust_f_name[30] = {0};
	char cust_l_name[30] = {0};
	char exec_name[100] = {0};
	sprintf(exec_name, "%s %s", pIn->exec_f_name, pIn->exec_l_name);
	int cust_tier;
	char tax_id[10] = {0};
	char broker_name[100] = {0};
	char co_name[100] = {0};
	strncpy(co_name, pIn->co_name, sizeof(pIn->co_name));
	char acct_name[50] = {0};
	char s_name[70] = {0};

	long unsigned int co_id;
	char issue[10] = {0};
	strncpy(issue, pIn->issue, sizeof(pIn->issue));
	char ap_acl[6] = {0};

	char symbol[20] = {0};
	strncpy(symbol, pIn->symbol, sizeof(pIn->symbol));
	bool type_is_sell = false;
	bool type_is_market = false;
	long int hs_qty;
	long int trade_qty = pIn->trade_qty;
	long int needed_qty = trade_qty;
	long int hold_qty;
	double buy_value = 0.0;
	double sell_value = 0.0;
	double hold_price;
	double acct_bal;
	double acct_assets;
	char exch_id[7] = {0};

	double market_price;
	double requested_price = pIn->requested_price;
	char trade_type_id[5] = {0};
	strncpy(trade_type_id, pIn->trade_type_id, sizeof(pIn->trade_type_id));

	int is_lifo = pIn->is_lifo;
	int type_is_margin = pIn->type_is_margin;
	bool is_cash = (type_is_margin==0);
	char st_submitted_id[5] = {0};
	strncpy(st_submitted_id, pIn->st_submitted_id, sizeof(pIn->st_submitted_id));
	char st_pending_id[5] = {0};
	strncpy(st_pending_id, pIn->st_pending_id, sizeof(pIn->st_pending_id));
	char status_id[10] = {0};

	double tax_rates = 0.0;
	double tax_amount = 0.0;
	double charge_amount = 0.0;
	double comm_amount;
	double comm_rate;

	sprintf(query, SQLTOF1_1, acct_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_1")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(acct_name, val, length);

		 	broker_id = atol(dbt5_sql_getvalue(&result, 1, length));

			cust_id = atol(dbt5_sql_getvalue(&result, 2, length));

			tax_status = atoi(dbt5_sql_getvalue(&result, 3, length));

#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 1, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 1 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF1_2, cust_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_2")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(cust_f_name, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(cust_l_name, val, length);

			cust_tier = atoi(dbt5_sql_getvalue(&result, 2, length));

			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(tax_id, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 2, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 2 fails");
			throw fail_msg.c_str();
	}


	sprintf(query, SQLTOF1_3, broker_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif

	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_3")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(broker_name, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(1, 3, 1);
#endif
	}else{
			string fail_msg("trade order frame1 query 3 fails");
			throw fail_msg.c_str();
	}

//--------------------Frame 2------------------
	if(strcmp(exec_l_name, cust_l_name)
            || strcmp(exec_f_name, cust_f_name)
            || strcmp(exec_tax_id, tax_id)){

			sprintf(query, SQLTOF2_1, acct_id, exec_f_name, exec_l_name, exec_tax_id);
#ifdef PROFILE_EACH_QUERY
				gettimeofday(&t1, NULL);
#endif
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4")==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					val = dbt5_sql_getvalue(&result, 0, length);
					strncpy(ap_acl, val, length);
#ifdef PROFILE_EACH_QUERY
					ADD_QUERY_NODE(2, 1, 1);
#endif
			}else{
					string fail_msg("trade order frame2 query 1 fails");
					throw fail_msg.c_str();
			}
	}
//--------------------Frame 3-----------------------
	if(strlen(symbol)==0){
		sprintf(query, SQLTOF3_1a, co_name);
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

		sprintf(query, SQLTOF3_2a, co_id, issue);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_5")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				val = dbt5_sql_getvalue(&result, 0, length);
				strncpy(exch_id, val, 6);

				val = dbt5_sql_getvalue(&result, 1, length);
				strncpy(s_name, val, length);

				val = dbt5_sql_getvalue(&result, 2, length);
				strncpy(symbol, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 2, 1);
#endif
		}else{
				string fail_msg("trade order frame3 query 2 fails");
				throw fail_msg.c_str();
		}

	}else{
		sprintf(query, SQLTOF3_1b, symbol);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4b")==1 && result.result_set){
				dbt5_sql_fetchrow(&result);

				co_id = atol(dbt5_sql_getvalue(&result, 0, length));

				val = dbt5_sql_getvalue(&result, 1, length);
				strncpy(exch_id, val, 6);

				val = dbt5_sql_getvalue(&result, 2, length);
				strncpy(s_name, val, length);
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
				strncpy(co_name, val, length);
#ifdef PROFILE_EACH_QUERY
				ADD_QUERY_NODE(3, 4, 1);
#endif
		}else{
				string fail_msg("trade order frame3 query 4 fails");
				throw fail_msg.c_str();
		}

	}

	sprintf(query, SQLTOF3_3, symbol);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_6")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			market_price = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 5, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 5 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF3_4, trade_type_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_7")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
			type_is_market = atoi(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 6, 1);
#endif

	}else{
			string fail_msg("trade order frame3 query 6 fails");
			throw fail_msg.c_str();
	}

	if(type_is_market){
			requested_price = market_price;
	}else{
			requested_price = requested_price;
	}


	sprintf(query, SQLTOF3_5, acct_id, symbol);
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

//--------------------------------
	if(type_is_sell){
			if(hs_qty > 0){
					if(is_lifo){

							sprintf(query, SQLTOF3_6a, acct_id, symbol);
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
							sprintf(query, SQLTOF3_6b, acct_id, symbol);
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
								sell_value = sell_value + (needed_qty * requested_price);
								needed_qty = 0;
						}else{
								buy_value = buy_value + (hold_qty * hold_price);
								sell_value = sell_value + (hold_qty * requested_price);
								needed_qty = needed_qty - hold_qty;
					}
				}
			}
	}else{
			if (hs_qty < 0){

					if(is_lifo){
							sprintf(query, SQLTOF3_6a, acct_id, symbol);
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
							sprintf(query, SQLTOF3_6b, acct_id, symbol);
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
									buy_value = buy_value + (needed_qty * requested_price);
									needed_qty = 0;
							}else{
									hold_qty = 0-hold_qty;
									sell_value = sell_value + (hold_qty * hold_price);
									buy_value = buy_value + (hold_qty * requested_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
	}

	if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
			sprintf(query, SQLTOF3_7, cust_id);
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
			tax_amount = (sell_value - buy_value) * tax_rates;
	}

	sprintf(query, SQLTOF3_8, cust_tier,  trade_type_id, exch_id, trade_qty, trade_qty);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_12")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 13, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 13 fails");
			throw fail_msg.c_str();
	}

	sprintf(query, SQLTOF3_9, cust_tier,  trade_type_id);
#ifdef PROFILE_EACH_QUERY
		gettimeofday(&t1, NULL);
#endif
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_13")==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			charge_amount = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_QUERY_NODE(3, 14, 1);
#endif
	}else{
			string fail_msg("trade order frame3 query 14 fails");
			throw fail_msg.c_str();
	}

	if(type_is_margin){
			sprintf(query, SQLTOF3_10, acct_id);
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

			sprintf(query, SQLTOF3_11, acct_id);
#ifdef PROFILE_EACH_QUERY
				gettimeofday(&t1, NULL);
#endif
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_15")==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_QUERY_NODE(3, 16, 1);
#endif
			}else{
					if(result.num_rows==0){
							acct_assets = acct_bal;
					}
			}
	}
	if(type_is_margin){
			strcpy(status_id, st_submitted_id);
	}else{
			strcpy(status_id, st_pending_id);
	}

//------------------Frame 4-----------------------
	comm_amount = comm_rate / 100
                                          * trade_qty
                                          * requested_price;
	comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

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
	sprintf(query, SQLTOF4_1, next_t_id, status_id,  trade_type_id, is_cash, symbol, trade_qty, requested_price, acct_id, exec_name, charge_amount, comm_amount, is_lifo);
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

	if(type_is_market == 0){
			sprintf(query, SQLTOF4_2, next_t_id, trade_type_id, symbol, trade_qty, requested_price, broker_id);

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

	sprintf(query, SQLTOF4_3, next_t_id, status_id);
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
	trade_id = next_t_id;

	if(pIn->roll_it_back){
			string msg("force roll back");
			throw msg.c_str();
	}

	pOut->buy_value = buy_value;
	pOut->sell_value = sell_value;
	pOut->tax_amount = tax_amount;
	pOut->trade_id = trade_id;
	pInter->type_is_market = type_is_market;
	strncpy(pInter->symbol, symbol, sizeof(pInter->symbol));
	pInter->trade_id = trade_id;
	pInter->trade_qty = trade_qty;
	pInter->type_is_market = type_is_market;
}
#endif
