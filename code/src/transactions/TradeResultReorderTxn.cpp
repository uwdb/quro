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
				if (r==2) fail_msg.append("\tempty result"); \
				outfile<<query<<endl; outfile.flush(); \
				throw fail_msg.c_str();

void CDBConnection::execute(PTradeResultTxnInput pIn,
		PTradeResultTxnOutput pOut)
{
	char query[4096];
	sql_result_t result;
	sql_result_t result_t;
	int length;
	char* val;
	int r = 0;

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

//--------------------Frame 1-----------------
	sprintf(query, TRADE_RESULT1_1, trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
  r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			acct_id = atol(dbt5_sql_getvalue(&result, 0, length));

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(type_id, val, length);

			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(symbol, val, length);

			trade_qty = atol(dbt5_sql_getvalue(&result, 3, length));

			charge = atof(dbt5_sql_getvalue(&result, 4, length));

			is_lifo = atoi(dbt5_sql_getvalue(&result, 5, length));
			trade_is_cash = atoi(dbt5_sql_getvalue(&result, 6, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 1, 1);
#endif
	}else{
				FAIL_MSG("trade result frame1 query 1 fails...");
	}


	sprintf(query, TRADE_RESULT1_2, type_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_2");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(type_name, val, length);

			type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
			type_is_market = atoi(dbt5_sql_getvalue(&result, 2, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 2, 1);
#endif
	}else{
			FAIL_MSG("trade result frame1 query 2 fails");
	}


	sprintf(query, TRADE_RESULT1_3, acct_id, symbol);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(1, 3, 1);
#endif
	}else{
			FAIL_MSG("trade result frame1 query 3 fails");
	}

//-------------FRAME 2--------------
	sprintf(query, TRADE_RESULT_HELPER);

	r = dbt5_sql_execute(query, &result, "TRADE_RESULT_HELPER");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(now_dts, val, length);
	}

	sprintf(query, TRADE_RESULT2_1, acct_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			broker_id = atol(dbt5_sql_getvalue(&result, 0, length));
			cust_id = atol(dbt5_sql_getvalue(&result, 1, length));

			tax_status = atoi(dbt5_sql_getvalue(&result, 2, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(2, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame2 query 1 fails");
	}

	sscanf(now_dts, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&trade_dts.year,
			&trade_dts.month,
			&trade_dts.day,
			&trade_dts.hour,
			&trade_dts.minute,
			&trade_dts.second);

//---------------------Frame 4----------------

	sprintf(query, TRADE_RESULT4_1, symbol);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(sec_ex_id, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(s_name, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 1 fails");
	}


	sprintf(query, TRADE_RESULT4_2, cust_id);
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


	sprintf(query, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 3, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 3 fails");
	}
//-----------------end Frame 4----------------------
//--------------------Frame 5--------------------
	comm_amount = ( comm_rate / 100.00 ) * ( trade_qty * trade_price );
  // round up for correct precision (cents only)
  comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

//	sprintf(now_dts, "%d-%d-%d %d:%d:%d", trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second);

//	sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);
//#ifdef PROFILE_EACH_QUERY
//	gettimeofday(&t1, NULL);
//#endif
//	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_1")){
//			FAIL_MSG("trade result frame5 query 1 fails");
//	}
//#ifdef PROFILE_EACH_QUERY
//	ADD_PROFILE_NODE(5, 1, 1);
//#endif

//	sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
//#ifdef PROFILE_EACH_QUERY
//	gettimeofday(&t1, NULL);
//#endif
//	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_2")){
//			FAIL_MSG("trade result frame5 query 2 fails");
//	}
//#ifdef PROFILE_EACH_QUERY
//	ADD_PROFILE_NODE(5, 2, 1);
//#endif

	sprintf(query, TRADE_RESULT5_3, comm_amount, broker_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_3")){
			FAIL_MSG("trade result frame5 query 3 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 3, 1);
#endif
//-----------------end Frame 5-----------------
//-----------------Frame 6--------------------
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

  // withhold tax only for certain account tax status
  if (tax_status == 1)
  {
      se_amount -= tax_amount;
  }

	if(trade_is_cash){
			sprintf(cash_type, "Cash Account");
	}else{
			sprintf(cash_type, "Margin");
	}

	char due_dts[20]={0};
	CDateTime due_date_time(&trade_dts);
	due_date_time.Add(2, 0);
	due_date_time.SetHMS(0,0,0,0);
	due_date_time.GetTimeStamp(&due_date);

	sprintf(due_dts, "%d-%d-%d %d:%d:%d", due_date.year, due_date.month, due_date.day, due_date.hour, due_date.minute, due_date.second);

	sprintf(query, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_1")){
			FAIL_MSG("trade result frame6 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(6, 1, 1);
#endif

//	sprintf(now_dts, "%d-%d-%d %d:%d:%d", trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second);

	if(trade_is_cash){
			sprintf(query, TRADE_RESULT6_2, se_amount, acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_2")){
					FAIL_MSG("trade result frame6 query 2 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 2, 1);
#endif

			sprintf(query, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_3")){
					FAIL_MSG("trade result frame6 query 3 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 3, 1);
#endif

			sprintf(query, TRADE_RESULT6_4, acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(6, 4, 1);
#endif
			}else{
					FAIL_MSG("trade result frame6 query4 fails");
			}

	}
//-----------------end Frame 6----------------

	if(type_is_sell){
			if(hs_qty == 0){
					sprintf(query, TRADE_RESULT2_2a, acct_id, symbol, (-1)*trade_qty);
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
			}else if(hs_qty != trade_qty){
					sprintf(query, TRADE_RESULT2_2b, hs_qty-trade_qty, acct_id, symbol);
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

			needed_qty = trade_qty;

			size_t num_rows = 0;
			size_t cnt = 0;
			if(hs_qty > 0){
					if(is_lifo){
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
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
							sprintf(query, TRADE_RESULT2_3b, acct_id, symbol);
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
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty-needed_qty);
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
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
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
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){

//					sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty);
//#ifdef PROFILE_EACH_QUERY
//					gettimeofday(&t1, NULL);
//#endif
//					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
//							FAIL_MSG("trade result frame2 query 10 fails");
//					}
//#ifdef PROFILE_EACH_QUERY
//					ADD_PROFILE_NODE(2, 10, 1);
//#endif

					sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, (-1)*needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){
							FAIL_MSG("trade result frame2 query 11 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 11, 1);
#endif
			}else if(hs_qty == trade_qty){
					sprintf(query, TRADE_RESULT2_7b, acct_id, symbol);
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
			if(hs_qty == 0){
					sprintf(query, TRADE_RESULT2_8a, acct_id, symbol, trade_qty);
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
					sprintf(query, TRADE_RESULT2_8b, hs_qty+trade_qty, acct_id, symbol);
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

			if(hs_qty < 0){
					if(is_lifo){
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
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
							sprintf(query, TRADE_RESULT2_3a, acct_id, symbol);
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
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty+needed_qty);
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
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{
									sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
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
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){
//					sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty);
//#ifdef PROFILE_EACH_QUERY
//					gettimeofday(&t1, NULL);
//#endif
//					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
//							FAIL_MSG("trade result frame2 query 21 fails");
//					}
//#ifdef PROFILE_EACH_QUERY
//					ADD_PROFILE_NODE(2, 21, 1);
//#endif

					sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){
							FAIL_MSG("trade result frame2 query 22 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 22, 1);
#endif
			}else if((-1)*hs_qty == trade_qty){
					sprintf(query, TRADE_RESULT2_7b, acct_id, symbol);
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


//----------------Frame 3---------------
	if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){
			sprintf(query, TRADE_RESULT3_1, cust_id);
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


			sprintf(query, TRADE_RESULT3_2, tax_rate*(sell_value - buy_value), trade_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT3_2")){
					FAIL_MSG("trade result frame3 query 2 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(3, 2, 1);
#endif
			tax_amount = tax_rate*(sell_value - buy_value);
}
/*
//---------------------Frame 4----------------

	sprintf(query, TRADE_RESULT4_1, symbol);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(sec_ex_id, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(s_name, val, length);
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 1, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 1 fails");
	}


	sprintf(query, TRADE_RESULT4_2, cust_id);
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


	sprintf(query, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(4, 3, 1);
#endif
	}else{
			FAIL_MSG("trade result frame4 query 3 fails");
	}

//--------------------Frame 5--------------------
	comm_amount = ( comm_rate / 100.00 ) * ( trade_qty * trade_price );
  // round up for correct precision (cents only)
  comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

//	sprintf(now_dts, "%d-%d-%d %d:%d:%d", trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second);

	sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_1")){
			FAIL_MSG("trade result frame5 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 1, 1);
#endif

	sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_2")){
			FAIL_MSG("trade result frame5 query 2 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 2, 1);
#endif

	sprintf(query, TRADE_RESULT5_3, comm_amount, broker_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_3")){
			FAIL_MSG("trade result frame5 query 3 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 3, 1);
#endif

//-----------------Frame 6--------------------
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

  // withhold tax only for certain account tax status
  if (tax_status == 1)
  {
      se_amount -= tax_amount;
  }

	if(trade_is_cash){
			sprintf(cash_type, "Cash Account");
	}else{
			sprintf(cash_type, "Margin");
	}

	char due_dts[20]={0};
	CDateTime due_date_time(&trade_dts);
	due_date_time.Add(2, 0);
	due_date_time.SetHMS(0,0,0,0);
	due_date_time.GetTimeStamp(&due_date);

	sprintf(due_dts, "%d-%d-%d %d:%d:%d", due_date.year, due_date.month, due_date.day, due_date.hour, due_date.minute, due_date.second);

	sprintf(query, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_1")){
			FAIL_MSG("trade result frame6 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(6, 1, 1);
#endif

//	sprintf(now_dts, "%d-%d-%d %d:%d:%d", trade_dts.year, trade_dts.month, trade_dts.day, trade_dts.hour, trade_dts.minute, trade_dts.second);

	if(trade_is_cash){
			sprintf(query, TRADE_RESULT6_2, se_amount, acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_2")){
					FAIL_MSG("trade result frame6 query 2 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 2, 1);
#endif

			sprintf(query, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_3")){
					FAIL_MSG("trade result frame6 query 3 fails");
			}
#ifdef PROFILE_EACH_QUERY
			ADD_PROFILE_NODE(6, 3, 1);
#endif

			sprintf(query, TRADE_RESULT6_4, acct_id);
#ifdef PROFILE_EACH_QUERY
			gettimeofday(&t1, NULL);
#endif
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);

					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(6, 4, 1);
#endif
			}else{
					FAIL_MSG("trade result frame6 query4 fails");
			}

	}
*/
	if(type_is_sell){
			if(needed_qty > 0){
					sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
							FAIL_MSG("trade result frame2 query 10 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 21, 1);
#endif
			}
	}else{
		if(needed_qty > 0){
					sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty);
#ifdef PROFILE_EACH_QUERY
					gettimeofday(&t1, NULL);
#endif
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){
							FAIL_MSG("trade result frame2 query 21 fails");
					}
#ifdef PROFILE_EACH_QUERY
					ADD_PROFILE_NODE(2, 21, 1);
#endif
			}

	}

	sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_1")){
			FAIL_MSG("trade result frame5 query 1 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 1, 1);
#endif

	sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
#ifdef PROFILE_EACH_QUERY
	gettimeofday(&t1, NULL);
#endif
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_2")){
			FAIL_MSG("trade result frame5 query 2 fails");
	}
#ifdef PROFILE_EACH_QUERY
	ADD_PROFILE_NODE(5, 2, 1);
#endif

//---------------set output-------------
	pOut->acct_id = acct_id;
	pOut->acct_bal = acct_bal;
	pOut->load_unit = cust_id;
}
#endif
