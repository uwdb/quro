#include <stdlib.h>
#include "DBConnection.h"
#include "TradeOrderDB.h"

#ifndef DB_PGSQL
long int my_atol(char* str){
	int len = strlen(str)-1;
	unsigned long int r = 0;
	unsigned long int base = 1;
	while(len>=0){
		r = r + base*(str[len]-'0');
		cout<<"str[len] = "<<str[len]<<", r = "<<r<<endl;
		base = base*10;
		len--;
	}
	return r;
}
long int my_atof(char* str){

}

void CDBConnection::execute(PTradeOrderTxnInput pIn,
				PTradeOrderIntermediate pInter,
				PTradeOrderTxnOutput pOut)
{

		char query1_1[256];
		char query1_2[256];

		char query1_3[256];
		char query2_1[256];
		char query3_1[256];
		char query3_2[256];
		char query3_3[256];
		char query3_4[256];
		char query3_5[256];
		char query3_6[256];
		char query3_7[256];
		char query3_80[256];
		char query3_90[256];
		char query3_81[256];
		char query3_91[256];
		char query3_10[256];
		char query3_11[256];
		char query3_12[256];
		char query3_13[256];
		char query3_14[256];
		char query4_1[256];
		char query4_2[256];
		char query4_3[256];
		char query4_4[256];
		char query4_5[256];

		sql_result_t result1, result2;

	 	sql_result_t	result3, result4, result5, result6, result7, result8;
		sql_result_t result9, result10, result11, result12_0a, result12_0b, result12_1a, result12_1b, result12, result13, result14, result15;
		sql_result_t result16, result17, result18, result19, result20, result21, result22;

		int length;
		char* val;

		int r_1, r_2, r_3, r_4, r_5, r_6, r_7, r_8, r_9, r_10, r_11, r_12_0a, r_12_0b, r_12_1a,  r_12_1b, r_13;
		int r_14, r_15, r_16, r_17, r_18, r_19, r_20, r_21, r_22;
		long unsigned int acct_id =  pIn->acct_id;
		long unsigned int broker_id;
		long unsigned int cust_id;
		long unsigned int trade_id;
		unsigned long next_t_id;
		int tax_status;
		char cust_f_name[30] = {0};
		char cust_l_name[30] = {0};
		int cust_tier;
		char tax_id[10] = {0};
		char broker_name[100] = {0};
		char co_name[100] = {0};
		char acct_name[50] = {0};
		char s_name[70] = {0};

		long unsigned int co_id;
		char ap_acl[6] = {0};

		char symbol[20] = {0};
		bool type_is_sell = false;
		bool type_is_market = false;
		long int hs_qty;
		long int needed_qty = pIn->trade_qty;
		long int hold_qty;
		double buy_value = 0.0;
		double sell_value = 0.0;
		double hold_price;
		double acct_bal;
		double acct_assets;
		char exch_id[7] = {0};

		double market_price;
		double requested_price = pIn->requested_price;

		int is_lifo = pIn->is_lifo;
		int type_is_margin = pIn->type_is_margin;
		bool is_cash = (type_is_margin==0);
		char status_id[10] = {0};

		double tax_rates = 0.0;
		double tax_amount = 0.0;
		double charge_amount = 0.0;
		double comm_amount;
		double comm_rate;
		size_t num_rows = 0;
		size_t cnt = 0;
		string rollback_fail_msg;
		char exec_name[100];

		sprintf(exec_name, "%s %s", pIn->exec_f_name, pIn->exec_l_name);

		sprintf(query1_1, SQLTOF1_1, pIn->acct_id);
		//CLANG_PROFILE(query1_1);
		r_1 = dbt5_sql_execute(query1_1, &result1, "TRADE_ORDER_1");
		if(r_1==1 && result1.result_set){
			dbt5_sql_fetchrow(&result1);
			strncpy(acct_name, dbt5_sql_getvalue(&result1, 0, length), length);
		 	broker_id = my_atol(dbt5_sql_getvalue(&result1, 1, length));
			cust_id = my_atol(dbt5_sql_getvalue(&result1, 2, length));
			cout<<"str cust_id = "<<dbt5_sql_getvalue(&result1, 2, length)<<", cust_id = "<<cust_id<<endl;
			tax_status = my_atol(dbt5_sql_getvalue(&result1, 3, length));
			dbt5_sql_close_cursor(&result1);
		}else{
			string fail_msg("trade order 1 fail");
			throw fail_msg.c_str();
		}

		sprintf(query1_2, SQLTOF1_2, cust_id);
		//CLANG_PROFILE(query1_2);
		r_2 = dbt5_sql_execute(query1_2, &result2, "TRADE_ORDER_2");
		if(r_2==1 && result2.result_set){
			dbt5_sql_fetchrow(&result2);
			strncpy(cust_f_name, dbt5_sql_getvalue(&result2, 0, length), length);
			strncpy(cust_l_name, dbt5_sql_getvalue(&result2, 1, length), length);
			cust_tier = my_atol(dbt5_sql_getvalue(&result2, 2, length));
			strncpy(tax_id, dbt5_sql_getvalue(&result2, 3, length), length);
			dbt5_sql_close_cursor(&result2);
		}else{
			string fail_msg("trade order 2 fail");
			throw fail_msg.c_str();
		}


		sprintf(query1_3, SQLTOF1_3, broker_id);
		//CLANG_PROFILE(query1_3);
		r_3 = dbt5_sql_execute(query1_3, &result3, "TRADE_ORDER_3");
		if(r_3==1 && result3.result_set){
			dbt5_sql_fetchrow(&result3);
			strncpy(broker_name, dbt5_sql_getvalue(&result3, 0, length), length);
			dbt5_sql_close_cursor(&result3);
		}else{
			string fail_msg("trade order 3 fail");
			throw fail_msg.c_str();
		}


		if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
			sprintf(query2_1, SQLTOF2_1, pIn->acct_id, pIn->exec_f_name, pIn->exec_l_name, pIn->exec_tax_id);
			//CLANG_PROFILE(query2_1);
			r_4 = dbt5_sql_execute(query2_1, &result4, "TRADE_ORDER_4");
			if(r_4==1 && result4.result_set){
					dbt5_sql_fetchrow(&result4);
					strncpy(ap_acl, dbt5_sql_getvalue(&result4, 0, length), length);
					dbt5_sql_close_cursor(&result4);
			}else{
					string fail_msg("trade order 2_1 fail");
					throw fail_msg.c_str();
			}
		}


		//--------------------Frame 3-----------------------
		if(strlen(pIn->symbol)==0){
				sprintf(query3_1, SQLTOF3_1a, pIn->co_name);
				//CLANG_PROFILE(query3_1);
				r_5 = dbt5_sql_execute(query3_1, &result5, "TRADE_ORDER_4");
				if(r_5==1 && result5.result_set){
						dbt5_sql_fetchrow(&result5);
						co_id = my_atol(dbt5_sql_getvalue(&result5, 0, length));
						dbt5_sql_close_cursor(&result5);
				}else{
						string fail_msg("trade order 3_1 fail");
						throw fail_msg.c_str();
				}

				sprintf(query3_2, SQLTOF3_2a, co_id, pIn->issue);
				//CLANG_PROFILE(query3_2);
				r_6 = dbt5_sql_execute(query3_2, &result6, "TRADE_ORDER_5");
				if(r_6==1 && result6.result_set){
						dbt5_sql_fetchrow(&result6);
						strncpy(exch_id, dbt5_sql_getvalue(&result6, 0, length), 6);
						strncpy(s_name, dbt5_sql_getvalue(&result6, 1, length), length);
						strncpy(symbol, dbt5_sql_getvalue(&result6, 2, length), length);
						dbt5_sql_close_cursor(&result6);
				}else{
						string fail_msg("trade order 3_2 fail");
						throw fail_msg.c_str();
				}

		}else{
				sprintf(query3_3, SQLTOF3_1b, pIn->symbol);
				//CLANG_PROFILE(query3_3);
				r_7 = dbt5_sql_execute(query3_3, &result7, "TRADE_ORDER_4b");
				if(r_7==1 && result7.result_set){
						dbt5_sql_fetchrow(&result7);
						strcpy(symbol, pIn->symbol);
						co_id = my_atol(dbt5_sql_getvalue(&result7, 0, length));
						strncpy(exch_id, dbt5_sql_getvalue(&result7, 1, length), 6);
						strncpy(s_name, dbt5_sql_getvalue(&result7, 2, length), length);
						dbt5_sql_close_cursor(&result7);
				}else{
						string fail_msg("trade order 3_3 fail");
						throw fail_msg.c_str();
				}

				sprintf(query3_4, SQLTOF3_2b, co_id);
				//CLANG_PROFILE(query3_4);
				r_8 = dbt5_sql_execute(query3_4, &result8, "TRADE_ORDER_5b");
				if(r_8==1 && result8.result_set){
						dbt5_sql_fetchrow(&result8);
						strncpy(co_name, dbt5_sql_getvalue(&result8, 0, length), length);
						dbt5_sql_close_cursor(&result8);
				}else{
						string fail_msg("trade order 3_4 fail");
						throw fail_msg.c_str();
				}

		}

		sprintf(query3_5, SQLTOF3_3, symbol);
		//CLANG_PROFILE(query3_5);
		r_9 = dbt5_sql_execute(query3_5, &result9, "TRADE_ORDER_6");
		if(r_9==1 && result9.result_set){
				dbt5_sql_fetchrow(&result9);
				market_price = atof(dbt5_sql_getvalue(&result9, 0, length));
				dbt5_sql_close_cursor(&result9);
		}else{
				string fail_msg("trade order 3_5 fail");
				throw fail_msg.c_str();
		}

		sprintf(query3_6, SQLTOF3_4, pIn->trade_type_id);
		//CLANG_PROFILE(query3_6);
		r_10 = dbt5_sql_execute(query3_6, &result10, "TRADE_ORDER_7");
		if(r_10==1 && result10.result_set){
				dbt5_sql_fetchrow(&result10);
				type_is_sell = my_atol(dbt5_sql_getvalue(&result10, 1, length));
				type_is_market = my_atol(dbt5_sql_getvalue(&result10, 0, length));
				dbt5_sql_close_cursor(&result10);
		}else{
				string fail_msg("trade order 3_6 fail");
				throw fail_msg.c_str();
		}


		if(type_is_market){
				requested_price = market_price;
		}else{
				requested_price = pIn->requested_price;
		}

		sprintf(query3_7, SQLTOF3_5, pIn->acct_id, symbol);
		//CLANG_PROFILE(query3_7);
		r_11 = dbt5_sql_execute(query3_7, &result11, "TRADE_ORDER_8");
		if(r_11==1 && result11.result_set){
				dbt5_sql_fetchrow(&result11);
				hs_qty = my_atol(dbt5_sql_getvalue(&result11, 0, length));
				dbt5_sql_close_cursor(&result11);
		}else{
				string fail_msg("trade order 3_7 fail");
				throw fail_msg.c_str();
		}



		//--------------------------------
		if(type_is_sell){
				if(hs_qty > 0){
						if(is_lifo){
							sprintf(query3_80, SQLTOF3_6a, pIn->acct_id, symbol);
							//CLANG_PROFILE(query3_80);
							r_12_0a = dbt5_sql_execute(query3_80, &result12_0a, "TRADE_ORDER_9");
							if(r_12_0a && result12_0a.result_set){
									num_rows = result12_0a.num_rows;
							}else{
									string fail_msg("trade order 3_8 fail");
									throw fail_msg.c_str();
							}
							result12 = result12_0a;
						}else{
							sprintf(query3_90, SQLTOF3_6b, pIn->acct_id, symbol);
							//CLANG_PROFILE(query3_90);
							r_12_0b = dbt5_sql_execute(query3_90, &result12_0b, "TRADE_ORDER_10");
							if(r_12_0b && result12_0b.result_set){
									num_rows = result12_0b.num_rows;
							}else{
									string fail_msg("trade order 3_9 fail");
									throw fail_msg.c_str();
							}
							result12 = result12_0b;
						}
						while(needed_qty>0 && cnt < num_rows){
								//read from holding results
								dbt5_sql_fetchrow(&result12);

								hold_qty = my_atol(dbt5_sql_getvalue(&result12, 0, length));
								hold_price = atof(dbt5_sql_getvalue(&result12, 1, length));
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
							sprintf(query3_81, SQLTOF3_6a, pIn->acct_id, symbol);
							//CLANG_PROFILE(query3_81);
							r_12_1a = dbt5_sql_execute(query3_81, &result12_1a, "TRADE_ORDER_9");
							if(r_12_1a && result12_1a.result_set){
									num_rows = result12_1a.num_rows;
							}else{
									string fail_msg("trade order 3_8 fail");
									throw fail_msg.c_str();
							}
							result12 = result12_1a;
						}else{
							sprintf(query3_91, SQLTOF3_6b, pIn->acct_id, symbol);
							//CLANG_PROFILE(query3_91);
							r_12_1b = dbt5_sql_execute(query3_91, &result12_1b, "TRADE_ORDER_10");
							if(r_12_1b && result12_1b.result_set){
									num_rows = result12_1b.num_rows;
							}else{
									string fail_msg("trade order 3_9 fail");
									throw fail_msg.c_str();
							}
							result12 = result12_1b;
						}
						while(needed_qty>0 && cnt < num_rows){
								dbt5_sql_fetchrow(&result12);

								hold_qty = my_atol(dbt5_sql_getvalue(&result12, 0, length));
								hold_price = atof(dbt5_sql_getvalue(&result12, 1, length));
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
			sprintf(query3_10, SQLTOF3_7, cust_id);
			//CLANG_PROFILE(query3_10);
			r_13 = dbt5_sql_execute(query3_10, &result13, "TRADE_ORDER_11");
			if(r_13==1 && result13.result_set){
					dbt5_sql_fetchrow(&result13);
					tax_rates = atof(dbt5_sql_getvalue(&result13, 0, length));
					dbt5_sql_close_cursor(&result13);
			}else{
					string fail_msg("trade order 3_7 fail");
					throw fail_msg.c_str();
			}
			tax_amount = (sell_value - buy_value) * tax_rates;
		}

		sprintf(query3_11, SQLTOF3_8, cust_tier, pIn->trade_type_id, exch_id, pIn->trade_qty, pIn->trade_qty);
		//CLANG_PROFILE(query3_11);
		r_14 = dbt5_sql_execute(query3_11, &result14, "TRADE_ORDER_12");
		if(r_14==1 && result14.result_set){
			dbt5_sql_fetchrow(&result14);
			comm_rate = atof(dbt5_sql_getvalue(&result14, 0, length));
			dbt5_sql_close_cursor(&result14);
		}else{
			string fail_msg("trade order 3_8 fail");
			throw fail_msg.c_str();
		}

		sprintf(query3_12, SQLTOF3_9, cust_tier, pIn->trade_type_id);
		//CLANG_PROFILE(query3_12);
		r_15 = dbt5_sql_execute(query3_12, &result15, "TRADE_ORDER_13");
		if(r_15==1 && result15.result_set){
			dbt5_sql_fetchrow(&result15);
			charge_amount = atof(dbt5_sql_getvalue(&result15, 0, length));
			dbt5_sql_close_cursor(&result15);
		}else{
			string fail_msg("trade order 3_9 fail");
			throw fail_msg.c_str();
		}


		if(type_is_margin){
			sprintf(query3_13, SQLTOF3_10, pIn->acct_id);
			//CLANG_PROFILE(query3_13);
			r_16 = dbt5_sql_execute(query3_13, &result16, "TRADE_ORDER_14");
			if(r_16==1 && result16.result_set){
					dbt5_sql_fetchrow(&result16);
					acct_bal = atof(dbt5_sql_getvalue(&result16, 0, length));
					dbt5_sql_close_cursor(&result16);
			}else{
					string fail_msg("trade order 3_10 fail");
					throw fail_msg.c_str();
			}

			sprintf(query3_14, SQLTOF3_11, pIn->acct_id);
			//CLANG_PROFILE(query3_14);
			r_17 = dbt5_sql_execute(query3_14, &result17, "TRADE_ORDER_15");
			if(r_17==1 && result17.result_set){
					dbt5_sql_fetchrow(&result17);
					acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result17, 0, length));
					dbt5_sql_close_cursor(&result17);
			}else{
					if(result17.num_rows==0){
							acct_assets = acct_bal;
					}
			}
		}
		if(type_is_margin){
				strcpy(status_id, pIn->st_submitted_id);
		}else{
				strcpy(status_id, pIn->st_pending_id);
		}

		comm_amount = comm_rate / 100
				* pIn->trade_qty
				* requested_price;
		comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

		sprintf(query4_1, SQLTOF4_0);
		r_18 = dbt5_sql_execute(query4_1, &result18, "TRADE_ORDER_16");
		if(!r_18){
			string fail_msg("trade order frame 4 query 1 fails");
			throw fail_msg.c_str();
		}

		sprintf(query4_2, SQLTOF4_00);
		r_19 = dbt5_sql_execute(query4_2, &result19, "TRADE_ORDER_17");
		if(r_19==1 && result19.result_set){
			dbt5_sql_fetchrow(&result19);
			next_t_id = my_atol(dbt5_sql_getvalue(&result19, 0, length)); \
			dbt5_sql_close_cursor(&result19);
		}else{
			string fail_msg("trade order frame 4 query 1_1 fails");
			throw fail_msg.c_str();
		}


		sprintf(query4_3, SQLTOF4_1, next_t_id, status_id, pIn->trade_type_id, is_cash, symbol, pIn->trade_qty, requested_price, pIn->acct_id, exec_name, charge_amount, comm_amount, pIn->is_lifo);
		//CLANG_PROFILE(query4_3);
	r_20 = dbt5_sql_execute(query4_3, &result20, "TRADE_ORDER_18");
	if(!r_20){
			string fail_msg("trade order frame 4 query 4_1 fail");
			throw fail_msg.c_str();
	}


		if(type_is_market == 0){
			sprintf(query4_4, SQLTOF4_2, next_t_id, pIn->trade_type_id, symbol, pIn->trade_qty, requested_price, broker_id);
			//CLANG_PROFILE(query4_4);
			r_21 = dbt5_sql_execute(query4_4, &result21, "TRADE_ORDER_19");
			if(!r_21){
					string fail_msg("trade order frame 4 query 2 fail");
					throw fail_msg.c_str();
			}
		}


	 	sprintf(query4_5, SQLTOF4_3, next_t_id, status_id);
		//CLANG_PROFILE(query4_5);
		r_22 = dbt5_sql_execute(query4_5, &result22, "TRADE_ORDER_20");
		if(!r_22){
				string fail_msg("trade order frame 4 query 3 fail");
				throw fail_msg.c_str();
		}

		if(pIn->roll_it_back){
				rollback_fail_msg.assign("force roll back");
				throw rollback_fail_msg.c_str();
		}

		pOut->buy_value = buy_value;
		pOut->sell_value = sell_value;
		pOut->tax_amount = tax_amount;

		pOut->trade_id = next_t_id;
		strncpy(pInter->symbol, symbol, sizeof(pInter->symbol));
		pInter->trade_id = trade_id;
		pInter->trade_qty = pIn->trade_qty;
		pInter->type_is_market = type_is_market;


		pInter->is_lifo = is_lifo;
		pInter->trade_is_cash = is_cash;
		pInter->charge = charge_amount;
		pInter->acct_id = acct_id;



}
#endif
