#include "DBConnection.h"
#include "TradeResultDB.h"

#ifndef DB_PGSQL
#ifdef TABLE_PROFILE
map<string, FIELDHIST> table_hist;
int profile_cnt = 0;
string tbl_name;
string param_str;
#endif


void CDBConnection::execute(PTradeResultTxnInput pIn,
		PTradeResultTxnOutput pOut)
{

	char query[1024];
	sql_result_t result, result_t;
	int length;
	int r;
	double t_time;

	long unsigned int trade_id = pIn->trade_id;
	double trade_price = pIn->trade_price;

	long unsigned int acct_id;
	long int trade_qty;
	double charge;
	int is_lifo;
	char type_id[10] = {0};
	char symbol[20] = {0};
	char type_name[20] = {0};

	int trade_is_cash;
	bool type_is_sell = false;
	bool type_is_market = false;
	long int hs_qty;
	long unsigned int hold_id;
	long int hold_qty;
	double hold_price;
	long int needed_qty;
	double buy_value = 0.0;
	double sell_value = 0.0;
	char now_dts[100]={0};

	double tax_rate;
	long broker_id;
	long cust_id;
	int tax_status;
	TIMESTAMP_STRUCT trade_dts;
	char st_completed_id[5] = "CMPT";
	double tax_amount = 0.0;

	char s_name[200] = {0};
	float comm_rate;
	double comm_amount = 0.0;
	char sec_ex_id[20]={0};
	double acct_bal;
	int cust_tier;

	char cash_type[10]={0};
	double se_amount;
	TIMESTAMP_STRUCT due_date;

	char due_dts[20]={0};
	CDateTime due_date_time;

	int num_rows0 = 0, num_rows1 = 0;
	int cnt0 = 0, cnt1 = 0;
	int num_rows = 0;
	int cnt = 0;

/*
	GETTIME;
	sprintf(query, TRADE_RESULT1_1, trade_id);

  r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			acct_id = atol(dbt5_sql_getvalue(&result, 0, length));
			strcpy(type_id, dbt5_sql_getvalue(&result, 1, length));
			strcpy(symbol, dbt5_sql_getvalue(&result, 2, length));
			trade_qty = atol(dbt5_sql_getvalue(&result, 3, length));
			charge = atof(dbt5_sql_getvalue(&result, 4, length));
			is_lifo = atoi(dbt5_sql_getvalue(&result, 5, length));
			trade_is_cash = atoi(dbt5_sql_getvalue(&result, 6, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 1");
			throw fail_msg.c_str();
	}
	GETPROFILE(22);

	GETTIME;
	sprintf(query, TRADE_RESULT1_2, type_id);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_2");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			strcpy(type_name, dbt5_sql_getvalue(&result, 0, length));
			type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
			type_is_market = atoi(dbt5_sql_getvalue(&result, 2, length));
			dbt5_sql_close_cursor(&result);
	}else{
string fail_msg("query fail 2");
throw fail_msg.c_str();
	}
	GETPROFILE(23);

	GETTIME;
	sprintf(query, TRADE_RESULT1_3, acct_id, symbol);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 3");
			throw fail_msg.c_str();
	}
	GETPROFILE(24);

	GETTIME;
	sprintf(query, TRADE_RESULT_HELPER);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT_HELPER");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			strcpy(now_dts, dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 4");
			throw fail_msg.c_str();
	}
	GETPROFILE(25);

	GETTIME;
	sprintf(query, TRADE_RESULT2_1, acct_id);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			broker_id = atol(dbt5_sql_getvalue(&result, 0, length));
			cust_id = atol(dbt5_sql_getvalue(&result, 1, length));
			tax_status = atoi(dbt5_sql_getvalue(&result, 2, length));
			dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg("query fail 5");
		throw fail_msg.c_str();
	}
	GETPROFILE(26);

		sscanf(now_dts, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&trade_dts.year,
			&trade_dts.month,
			&trade_dts.day,
			&trade_dts.hour,
			&trade_dts.minute,
			&trade_dts.second);

	if(type_is_sell){
			if(hs_qty == 0){
					GETTIME;
					sprintf(query, TRADE_RESULT2_2a, acct_id, symbol, (-1)*trade_qty);
					r=dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a");
					if(!r){
							string fail_msg("query fail 6");
							throw fail_msg.c_str();
					}
					GETPROFILE(27);
			}else if(hs_qty != trade_qty){
					GETTIME;
					sprintf(query, TRADE_RESULT2_2b, hs_qty-trade_qty, acct_id, symbol);
					r=dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a");
					if(!r){
							string fail_msg("query fail 7");
							throw fail_msg.c_str();
					}
					GETPROFILE(27);
			}

			needed_qty = trade_qty;

			if(hs_qty > 0){
					if(is_lifo){
							GETTIME;
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows0 = result.num_rows;
							}else{
									string fail_msg("query fail 8");
									throw fail_msg.c_str();
							}
							GETPROFILE(28);
					}else{
							GETTIME;
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows0 = result.num_rows;
							}else{
									string fail_msg("query fail 9");
									throw fail_msg.c_str();
							}
							GETPROFILE(28);
					}
					while(needed_qty > 0 && cnt0 < num_rows0){
							dbt5_sql_fetchrow(&result);
							cnt0++;

							hold_id = atol(dbt5_sql_getvalue(&result, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result, 2, length));

							if(hold_qty > needed_qty){
									GETTIME;
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty-needed_qty);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_4a");
									if(!r){
													string fail_msg("query fail 10");
													throw fail_msg.c_str();
									}
									GETPROFILE(29);

									GETTIME;
									sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_5a");
									if(!r){
											string fail_msg("query fail 11");
											throw fail_msg.c_str();
									}
									GETPROFILE(30);

									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{
									GETTIME;
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_4a");
									if(!r){
											string fail_msg("query fail 12");
											throw fail_msg.c_str();
									}
									GETPROFILE(29);

									GETTIME;
									sprintf(query, TRADE_RESULT2_5b, hold_id);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_5b");
									if(!r){
											string fail_msg("query fail 13");
											throw fail_msg.c_str();
									}
									GETPROFILE(30);

									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){

				GETTIME;
				sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty);
				r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");
				if(!r){
						string fail_msg("query fail 14");
						throw fail_msg.c_str();
				}
				GETPROFILE(31);

				GETTIME;
				sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, (-1)*needed_qty);
				r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a");
				if(!r){
						string fail_msg("query fail 15");
						throw fail_msg.c_str();
				}
				GETPROFILE(32);
			}else if(hs_qty == trade_qty){
					GETTIME;
					sprintf(query, TRADE_RESULT2_7b, acct_id, symbol);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b");
					if(!r){
							string fail_msg("query fail 16");
							throw fail_msg.c_str();
					}
					GETPROFILE(33);
			}
	}
	else{
			if(hs_qty == 0){
					GETTIME;
 					sprintf(query, TRADE_RESULT2_8a, acct_id, symbol, trade_qty);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_8a");
					if(!r){
							string fail_msg("query fail 16");
							throw fail_msg.c_str();
					}
					GETPROFILE(32);
			}else{
					GETTIME;
 					sprintf(query, TRADE_RESULT2_8b, hs_qty+trade_qty, acct_id, symbol);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_8b");
					if(!r){
							string fail_msg("query fail 17");
							throw fail_msg.c_str();
					}
					GETPROFILE(32);
			}

			if(hs_qty < 0){
					if(is_lifo){
							GETTIME;
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows1 = result.num_rows;
							}else{
									string fail_msg("query fail 18");
									throw fail_msg.c_str();
							}
							GETPROFILE(28);
					}else{
							GETTIME;
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_3a");
							if(r==1 && result.result_set){
									num_rows1 = result.num_rows;
							}else{
									string fail_msg("query fail 19");
									throw fail_msg.c_str();
							}
							GETPROFILE(28);
					}
					while(needed_qty>0 && cnt1<num_rows1){
							dbt5_sql_fetchrow(&result);
							cnt1++;
							hold_id = atol(dbt5_sql_getvalue(&result, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result, 2, length));
							if(hold_qty > needed_qty){
									GETTIME;
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty+needed_qty);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_4a");
									if(!r){
											string fail_msg("query fail 20");
											throw fail_msg.c_str();
									}
									GETPROFILE(29);

									GETTIME;
									sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_5a");
									if(!r){
											string fail_msg("query fail 21");
											throw fail_msg.c_str();
									}
									GETPROFILE(30);

									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{
									GETTIME;
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
									r = dbt5_sql_execute(query, &result_t, "TRADE_REUSLT2_4a");
									if(!r){
											string fail_msg("query fail 22");
											throw fail_msg.c_str();
									}
									GETPROFILE(29);

									GETTIME;
									sprintf(query, TRADE_RESULT2_5b, hold_id);
									r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_5b");
									if(!r){
											string fail_msg("query fail 23");
											throw fail_msg.c_str();
									}
									GETPROFILE(30);

									hold_qty = (-1)*hold_qty;
									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){
					GETTIME;
					sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");
					if(!r){
							string fail_msg("query fail 24");
							throw fail_msg.c_str();
					}
					GETPROFILE(31);

					GETTIME;
					sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, needed_qty);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a");
					if(!r){
							string fail_msg("query fail 25");
							throw fail_msg.c_str();
					}
					GETPROFILE(32);
			}else if((-1)*hs_qty == trade_qty){
					GETTIME;
					sprintf(query, TRADE_RESULT2_7b, acct_id, symbol);
					r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b");
					if(!r){
							string fail_msg("query fail 26");
							throw fail_msg.c_str();
					}
					GETPROFILE(33);
			}
	}

	if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
			GETTIME;
			sprintf(query, TRADE_RESULT3_1, cust_id);
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT3_1");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					tax_rate = atof(dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					string fail_msg("query fail 27");
					throw fail_msg.c_str();
			}
			GETPROFILE(34);

			GETTIME;
			sprintf(query, TRADE_RESULT3_2, tax_rate*(sell_value - buy_value), trade_id);
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT3_2");
			if(!r){
					string fail_msg("query fail 28");
					throw fail_msg.c_str();
			}
			GETPROFILE(35);

			tax_amount = tax_rate*(sell_value - buy_value);
	}

	GETTIME;
	sprintf(query, TRADE_RESULT4_1, symbol);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			strcpy(sec_ex_id, dbt5_sql_getvalue(&result, 0, length));
			strcpy(s_name, dbt5_sql_getvalue(&result, 1, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 29");
			throw fail_msg.c_str();
	}
	GETPROFILE(36);

	GETTIME;
	sprintf(query, TRADE_RESULT4_2, cust_id);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_2");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			cust_tier = atoi(dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 30");
			throw fail_msg.c_str();
	}
	GETPROFILE(37);

	GETTIME;
	sprintf(query, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
	}else{
			string fail_msg("query fail 31");
			throw fail_msg.c_str();
	}
	GETPROFILE(38);


	comm_amount = ( comm_rate / 100.00 ) * ( trade_qty * trade_price );
  // round up for correct precision (cents only)
  comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

	GETTIME;
	sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_1");
	if(!r){
			string fail_msg("query fail 32");
			throw fail_msg.c_str();
	}
	GETPROFILE(39);

	GETTIME;
	sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_2");
	if(!r){
			string fail_msg("query fail 33");
			throw fail_msg.c_str();
	}
	GETPROFILE(40);

	sprintf(query, TRADE_RESULT5_3, comm_amount, broker_id);
	GETTIME;
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_3");
	if(!r){
		string fail_msg("query fail 34");
		throw fail_msg.c_str();
	}
	GETPROFILE(45);

	if (type_is_sell)
  {
      se_amount = trade_qty * trade_price
                                        - charge
                                        - comm_amount;
  }
  else
  {
      se_amount = -1 * ( trade_qty * trade_price
                                                + charge
                                                + comm_amount);
  }


  if (tax_status == 1)
  {
      se_amount -= tax_amount;
  }

	if(trade_is_cash){
			sprintf(cash_type, "Cash Account");
	}else{
			sprintf(cash_type, "Margin");
	}

	due_date_time.Set(trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second, 0);
	due_date_time.Add(2, 0);
	due_date_time.SetHMS(0,0,0,0);
	due_date_time.GetTimeStamp(&due_date);

	sprintf(due_dts, "%d-%d-%d %d:%d:%d", due_date.year, due_date.month, due_date.day, due_date.hour, due_date.minute, due_date.second);

	GETTIME;
	sprintf(query, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount);
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_1");
	if(!r){
			string fail_msg("query fail 35");
			throw fail_msg.c_str();
	}
	GETPROFILE(41);

	if(trade_is_cash){
			GETTIME;
			sprintf(query, TRADE_RESULT6_2, se_amount, acct_id);
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_2");
			if(!r){
					string fail_msg("query fail 36");
					throw fail_msg.c_str();
			}
			GETPROFILE(42);

			GETTIME;
			sprintf(query, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name);
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_3");
			if(!r){
					string fail_msg("query fail 37");
					throw fail_msg.c_str();
			}
			GETPROFILE(43);

			GETTIME;
			sprintf(query, TRADE_RESULT6_4, acct_id);
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					string fail_msg("query fail 38");
					throw fail_msg.c_str();
			}
			GETPROFILE(44);

	}

	pOut->acct_id = acct_id;
	pOut->acct_bal = acct_bal;
	pOut->load_unit = cust_id;

	return ;
*/

	string fail_msg;
	sprintf(query, TRADE_RESULT_HELPER);
	char query_24[512];
	char query_26[512];
	char query_27[512];
	char query_29[512];
	char query_32[512];

QUERY_BEGIN_Q4:
//conflict index = 399
r = dbt5_sql_execute(query, &result, "TRADE_RESULT_HELPER");

QUERY_END_Q4:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(now_dts, dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 4");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}

//sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
//
//QUERY_BEGIN_Q34:
////conflict index = 400000
//r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_2");
//
//QUERY_END_Q34:
//
//if(!r){
//	fail_msg.assign("query fail 33");
//
//
//}
//if(!r){
//	throw fail_msg.c_str();
//
//
//}
sprintf(query, TRADE_RESULT1_1, trade_id);

QUERY_BEGIN_Q1:
//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_1");

QUERY_END_Q1:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

acct_id = atol(dbt5_sql_getvalue(&result, 0, length));
strcpy(type_id, dbt5_sql_getvalue(&result, 1, length));
strcpy(symbol, dbt5_sql_getvalue(&result, 2, length));
trade_qty = atol(dbt5_sql_getvalue(&result, 3, length));
charge = atof(dbt5_sql_getvalue(&result, 4, length));
is_lifo = atoi(dbt5_sql_getvalue(&result, 5, length));
trade_is_cash = atoi(dbt5_sql_getvalue(&result, 6, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 1");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, TRADE_RESULT1_2, type_id);

QUERY_BEGIN_Q2:
//conflict index = 1702
r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_2");

QUERY_END_Q2:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(type_name, dbt5_sql_getvalue(&result, 0, length));
type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
type_is_market = atoi(dbt5_sql_getvalue(&result, 2, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 2");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, TRADE_RESULT4_1, symbol);

QUERY_BEGIN_Q30:
//conflict index = 4362
r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1");

QUERY_END_Q30:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(sec_ex_id, dbt5_sql_getvalue(&result, 0, length));
strcpy(s_name, dbt5_sql_getvalue(&result, 1, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 29");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(trade_is_cash){
	sprintf(query, TRADE_RESULT6_4, acct_id);


}

QUERY_BEGIN_Q38:
if(trade_is_cash){
	//conflict index = 4651
r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4");


}

QUERY_END_Q38:

if(trade_is_cash){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(trade_is_cash){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 37");


}


}
if(trade_is_cash){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
sprintf(query, TRADE_RESULT2_1, acct_id);

QUERY_BEGIN_Q5:
//conflict index = 4651
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_1");

QUERY_END_Q5:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

broker_id = atol(dbt5_sql_getvalue(&result, 0, length));
cust_id = atol(dbt5_sql_getvalue(&result, 1, length));
tax_status = atoi(dbt5_sql_getvalue(&result, 2, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 5");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, TRADE_RESULT1_3, acct_id, symbol);

QUERY_BEGIN_Q3:
//conflict index = 4678
r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_3");

QUERY_END_Q3:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 3");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(type_is_sell){
	if(hs_qty == 0){
	sprintf(query, TRADE_RESULT2_2a, acct_id, symbol, (-1)*trade_qty);


}


}

QUERY_BEGIN_Q6:
if(type_is_sell){
	if(hs_qty == 0){
	//conflict index = 4678
r=dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a");


}


}

QUERY_END_Q6:

if(type_is_sell){
	if(hs_qty == 0){
	if(!r){
	fail_msg.assign("query fail 6");


}


}


}
if(type_is_sell){
	if(hs_qty == 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}
if(!(type_is_sell)){
	if(hs_qty == 0){
	sprintf(query, TRADE_RESULT2_8a, acct_id, symbol, trade_qty);


}


}

QUERY_BEGIN_Q17:
if(!(type_is_sell)){
	if(hs_qty == 0){
	//conflict index = 4678
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_8a");


}


}

QUERY_END_Q17:

if(!(type_is_sell)){
	if(hs_qty == 0){
	if(!r){
	fail_msg.assign("query fail 16");


}


}


}
if(!(type_is_sell)){
	if(hs_qty == 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q9:
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");


}


}


}

QUERY_END_Q9:

if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(r==1 && result_t.result_set){
	num_rows = result_t.num_rows;


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(!(r==1 && result_t.result_set)){
	fail_msg.assign("query fail 9");


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(!(r==1 && result_t.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q20:
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");


}


}


}

QUERY_END_Q20:

if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(r==1 && result_t.result_set){
	num_rows = result_t.num_rows;


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(!(r==1 && result_t.result_set)){
	fail_msg.assign("query fail 19");


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(!(r==1 && result_t.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q19:
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");


}


}


}

QUERY_END_Q19:

if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(r==1 && result_t.result_set){
	num_rows = result_t.num_rows;


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(!(r==1 && result_t.result_set)){
	fail_msg.assign("query fail 18");


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(!(r==1 && result_t.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q8:
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a");


}


}


}

QUERY_END_Q8:

if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(r==1 && result_t.result_set){
	num_rows = result_t.num_rows;


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(!(r==1 && result_t.result_set)){
	fail_msg.assign("query fail 8");


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(!(r==1 && result_t.result_set)){
	throw fail_msg.c_str();


}


}


}


}
sprintf(query, TRADE_RESULT4_2, cust_id);

QUERY_BEGIN_Q31:
//conflict index = 11428
r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_2");

QUERY_END_Q31:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

cust_tier = atoi(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 30");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(!(type_is_sell)){
	if(!(hs_qty == 0)){
	sprintf(query, TRADE_RESULT2_8b, hs_qty+trade_qty, acct_id, symbol);


}


}

QUERY_BEGIN_Q18:
if(!(type_is_sell)){
	if(!(hs_qty == 0)){
	//conflict index = 14678
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_8b");


}


}

QUERY_END_Q18:

if(!(type_is_sell)){
	if(!(hs_qty == 0)){
	if(!r){
	fail_msg.assign("query fail 17");


}


}


}
if(!(type_is_sell)){
	if(!(hs_qty == 0)){
	if(!r){
	throw fail_msg.c_str();


}


}


}
if(type_is_sell){
	if(!(hs_qty == 0)){
	if(hs_qty != trade_qty){
	sprintf(query, TRADE_RESULT2_2b, hs_qty-trade_qty, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q7:
if(type_is_sell){
	if(!(hs_qty == 0)){
	if(hs_qty != trade_qty){
	//conflict index = 14678
r=dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a");


}


}


}

QUERY_END_Q7:

if(type_is_sell){
	if(!(hs_qty == 0)){
	if(hs_qty != trade_qty){
	if(!r){
	fail_msg.assign("query fail 7");


}


}


}


}
if(type_is_sell){
	if(!(hs_qty == 0)){
	if(hs_qty != trade_qty){
	if(!r){
	throw fail_msg.c_str();


}


}


}


}
sprintf(query, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty);

QUERY_BEGIN_Q32:
//conflict index = 125000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3");

QUERY_END_Q32:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 31");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
comm_amount = ( comm_rate / 100.00 ) * ( trade_qty * trade_price );
comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;
comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;
if(type_is_sell){
	se_amount = trade_qty * trade_price
                                        - charge
                                        - comm_amount;


}
if(!(type_is_sell)){
	se_amount = -1 * ( trade_qty * trade_price
                                                + charge
                                                + comm_amount);


}

if(trade_is_cash){
	sprintf(query, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name);


}

QUERY_BEGIN_Q37:
if(trade_is_cash){
	//conflict index = 1632
r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_3");


}

QUERY_END_Q37:

if(trade_is_cash){
	if(!r){
	fail_msg.assign("query fail 36");


}


}
if(trade_is_cash){
	if(!r){
	throw fail_msg.c_str();


}


}
if(trade_is_cash){
	sprintf(cash_type, "Cash Account");


}
if(!(trade_is_cash)){
	sprintf(cash_type, "Margin");


}
sprintf(due_dts, "%d-%d-%d %d:%d:%d", due_date.year, due_date.month, due_date.day, due_date.hour, due_date.minute, due_date.second);
sprintf(query, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount);

QUERY_BEGIN_Q35:
//conflict index = 1701
r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_1");

QUERY_END_Q35:

if(!r){
	fail_msg.assign("query fail 34");


}
if(!r){
	throw fail_msg.c_str();


}
if(trade_is_cash){
	sprintf(query, TRADE_RESULT6_2, se_amount, acct_id);


}

QUERY_BEGIN_Q36:
if(trade_is_cash){
	//conflict index = 14651
r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_2");


}

QUERY_END_Q36:

if(trade_is_cash){
	if(!r){
	fail_msg.assign("query fail 35");


}


}
if(trade_is_cash){
	if(!r){
	throw fail_msg.c_str();


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	while(needed_qty>0 && cnt<num_rows){
	dbt5_sql_fetchrow(&result_t);

cnt++;
hold_id = atol(dbt5_sql_getvalue(&result_t, 0, length));
hold_qty = atol(dbt5_sql_getvalue(&result_t, 1, length));
hold_price = atof(dbt5_sql_getvalue(&result_t, 2, length));
if(hold_qty > needed_qty){
	sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty+needed_qty);

//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");
if(!r){
	fail_msg.assign("query fail 20");

throw fail_msg.c_str();

}
sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
//conflict index = 14944
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a");
if(!r){
	fail_msg.assign("query fail 21");

throw fail_msg.c_str();

}
buy_value = buy_value + (needed_qty * hold_price);
sell_value = sell_value + (needed_qty * trade_price);
needed_qty = 0;

}
if(!(hold_qty > needed_qty)){
	sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);

//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_REUSLT2_4a");
if(!r){
	fail_msg.assign("query fail 22");

throw fail_msg.c_str();

}
sprintf(query, TRADE_RESULT2_5b, hold_id);
//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b");
if(!r){
	fail_msg.assign("query fail 23");

throw fail_msg.c_str();

}
hold_qty = (-1)*hold_qty;
buy_value = buy_value + (hold_qty * hold_price);
sell_value = sell_value + (hold_qty * trade_price);
needed_qty = needed_qty - hold_qty;

}

}


}


}
if(!(type_is_sell)){
	if(needed_qty > 0){
	sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty);


}


}
if(!(type_is_sell)){
	if(needed_qty > 0){
	sprintf(query_24, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, needed_qty);


}


}
if(!(type_is_sell)){
	if(!(needed_qty > 0)){
	if((-1)*hs_qty == trade_qty){
	sprintf(query_26, TRADE_RESULT2_7b, acct_id, symbol);


}


}


}

QUERY_BEGIN_Q27:
if(!(type_is_sell)){
	if(!(needed_qty > 0)){
	if((-1)*hs_qty == trade_qty){
	//conflict index = 4678
r = dbt5_sql_execute(query_26, &result, "TRADE_RESULT2_7b");


}


}


}

QUERY_END_Q27:

if(!(type_is_sell)){
	if(!(needed_qty > 0)){
	if((-1)*hs_qty == trade_qty){
	if(!r){
	fail_msg.assign("query fail 26");


}


}


}


}
if(!(type_is_sell)){
	if(!(needed_qty > 0)){
	if((-1)*hs_qty == trade_qty){
	if(!r){
	throw fail_msg.c_str();


}


}


}


}

QUERY_BEGIN_Q26:
if(!(type_is_sell)){
	if(needed_qty > 0){
	//conflict index = 4944
r = dbt5_sql_execute(query_24, &result, "TRADE_RESULT2_7a");


}


}

QUERY_END_Q26:

if(!(type_is_sell)){
	if(needed_qty > 0){
	if(!r){
	fail_msg.assign("query fail 25");


}


}


}
if(!(type_is_sell)){
	if(needed_qty > 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}

QUERY_BEGIN_Q25:
if(!(type_is_sell)){
	if(needed_qty > 0){
	//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");


}


}

QUERY_END_Q25:

if(!(type_is_sell)){
	if(needed_qty > 0){
	if(!r){
	fail_msg.assign("query fail 24");


}


}


}
if(!(type_is_sell)){
	if(needed_qty > 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}
if(type_is_sell){
	needed_qty = trade_qty;


}
if(type_is_sell){
	if(hs_qty > 0){
	while(needed_qty > 0 && cnt < num_rows){
	dbt5_sql_fetchrow(&result_t);

cnt++;
hold_id = atol(dbt5_sql_getvalue(&result_t, 0, length));
hold_qty = atol(dbt5_sql_getvalue(&result_t, 1, length));
hold_price = atof(dbt5_sql_getvalue(&result_t, 2, length));
if(hold_qty > needed_qty){
	sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty-needed_qty);

//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");
if(!r){
	fail_msg.assign("query fail 10");

throw fail_msg.c_str();

}
sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
//conflict index = 14944
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a");
if(!r){
	fail_msg.assign("query fail 11");

throw fail_msg.c_str();

}
buy_value = buy_value + (needed_qty * hold_price);
sell_value = sell_value + (needed_qty * trade_price);
needed_qty = 0;

}
if(!(hold_qty > needed_qty)){
	sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);

//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");
if(!r){
	fail_msg.assign("query fail 12");

throw fail_msg.c_str();

}
sprintf(query, TRADE_RESULT2_5b, hold_id);
//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b");
if(!r){
	fail_msg.assign("query fail 13");

throw fail_msg.c_str();

}
buy_value = buy_value + (hold_qty * hold_price);
sell_value = sell_value + (hold_qty * trade_price);
needed_qty = needed_qty - hold_qty;

}

}


}


}
if(type_is_sell){
	if(needed_qty > 0){
	sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty);


}


}
if(type_is_sell){
	if(needed_qty > 0){
	sprintf(query_27, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, (-1)*needed_qty);


}


}
if(type_is_sell){
	if(!(needed_qty > 0)){
	if(hs_qty == trade_qty){
	sprintf(query_29, TRADE_RESULT2_7b, acct_id, symbol);


}


}


}
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	sprintf(query_32, TRADE_RESULT3_1, cust_id);


}

QUERY_BEGIN_Q28:
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	//conflict index = 1702
r = dbt5_sql_execute(query_32, &result, "TRADE_RESULT3_1");


}

QUERY_END_Q28:

if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

tax_rate = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("query fail 27");


}


}
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}

QUERY_BEGIN_Q16:
if(type_is_sell){
	if(!(needed_qty > 0)){
	if(hs_qty == trade_qty){
	//conflict index = 4678
r = dbt5_sql_execute(query_29, &result, "TRADE_RESULT2_7b");


}


}


}

QUERY_END_Q16:

if(type_is_sell){
	if(!(needed_qty > 0)){
	if(hs_qty == trade_qty){
	if(!r){
	fail_msg.assign("query fail 16");


}


}


}


}
if(type_is_sell){
	if(!(needed_qty > 0)){
	if(hs_qty == trade_qty){
	if(!r){
	throw fail_msg.c_str();


}


}


}


}

QUERY_BEGIN_Q15:
if(type_is_sell){
	if(needed_qty > 0){
	//conflict index = 4944
r = dbt5_sql_execute(query_27, &result, "TRADE_RESULT2_7a");


}


}

QUERY_END_Q15:

if(type_is_sell){
	if(needed_qty > 0){
	if(!r){
	fail_msg.assign("query fail 15");


}


}


}
if(type_is_sell){
	if(needed_qty > 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}

QUERY_BEGIN_Q14:
if(type_is_sell){
	if(needed_qty > 0){
	//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a");


}


}

QUERY_END_Q14:

if(type_is_sell){
	if(needed_qty > 0){
	if(!r){
	fail_msg.assign("query fail 14");


}


}


}
if(type_is_sell){
	if(needed_qty > 0){
	if(!r){
	throw fail_msg.c_str();


}


}


}
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	sprintf(query, TRADE_RESULT3_2, tax_rate*(sell_value - buy_value), trade_id);


}

QUERY_BEGIN_Q29:
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	//conflict index = 510000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT3_2");


}

QUERY_END_Q29:

if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	if(!r){
	fail_msg.assign("query fail 28");


}


}
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	if(!r){
	throw fail_msg.c_str();


}


}
sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);

QUERY_BEGIN_Q33:
//conflict index = 510000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_1");

QUERY_END_Q33:

if(!r){
	fail_msg.assign("query fail 32");


}
if(!r){
	throw fail_msg.c_str();


}
sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);

QUERY_BEGIN_Q34:
//conflict index = 400000
r = dbt5_sql_execute(query, &result, "TRADE_RESULT5_2");

QUERY_END_Q34:

if(!r){
	fail_msg.assign("query fail 33");


}
if(!r){
	throw fail_msg.c_str();


}

sscanf(now_dts, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&trade_dts.year,
			&trade_dts.month,
			&trade_dts.day,
			&trade_dts.hour,
			&trade_dts.minute,
			&trade_dts.second);
if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
	tax_amount = tax_rate*(sell_value - buy_value);


}
if(tax_status == 1){
	se_amount -= tax_amount;


}
due_date_time.Set(trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second, 0);
due_date_time.Add(2, 0);
due_date_time.SetHMS(0,0,0,0);
due_date_time.GetTimeStamp(&due_date);
pOut->acct_id = acct_id;
pOut->acct_bal = acct_bal;
pOut->load_unit = cust_id;
return;

}
#endif
