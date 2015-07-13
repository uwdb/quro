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
	char query1[1024];
	char query2[1024];
	char query3[1024];
	char query4[1024];
	char query5[1024];
	char query6[1024];
	char query7[1024];
	char query80[1024];
	char query81[1024];
	char query9[1024];
	char query10[1024];
	char query11[1024];
	char query12[1024];
	char query13[1024];
	char query14[1024];
	char query15[1024];
	char query16[1024];
	char query17[1024];
	char query180[1024];
	char query181[1024];
	char query19[1024];
	char query20[1024];
	char query21[1024];
	char query22[1024];
	char query23[1024];
	char query24[1024];
	char query25[1024];
	char query26[1024];
	char query27[1024];
	char query28[1024];
	char query29[1024];
	char query30[1024];
	char query31[1024];
	char query32[1024];
	char query33[1024];
	char query34[1024];
	char query35[1024];
	char query36[1024];
	sql_result_t result1, result2, result3, result4, result5, result6, result7, result8, result9, result10;
	sql_result_t result11, result12, result13, result14, result15, result16, result17, result18, result19, result20;
	sql_result_t result21, result22, result23, result24, result25, result26, result27, result28, result29, result30;
	sql_result_t result31, result32, result33, result34, result35, result36;
	sql_result_t result80, result81, result180, result181;
	int length;
	int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
	int r11, r12, r13, r14, r15, r16, r17, r18, r19, r20;
	int r21, r22, r23, r24, r25, r26, r27, r28, r29, r30;
	int r31, r32, r33, r34, r35, r36;
	int r80, r81, r180, r181;

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

	sprintf(query1, TRADE_RESULT1_1, trade_id);
	//CLANG_PROFILE(query1);
  r1 = dbt5_sql_execute(query1, &result1, "TRADE_RESULT1_1");
	if(r1==1 && result1.result_set){
			dbt5_sql_fetchrow(&result1);
			acct_id = atol(dbt5_sql_getvalue(&result1, 0, length));
			strncpy(type_id, dbt5_sql_getvalue(&result1, 1, length), length);
			strncpy(symbol, dbt5_sql_getvalue(&result1, 2, length), length);
			trade_qty = atol(dbt5_sql_getvalue(&result1, 3, length));
			charge = atof(dbt5_sql_getvalue(&result1, 4, length));
			is_lifo = atoi(dbt5_sql_getvalue(&result1, 5, length));
			trade_is_cash = atoi(dbt5_sql_getvalue(&result1, 6, length));
			dbt5_sql_close_cursor(&result1);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}

	sprintf(query2, TRADE_RESULT1_2, type_id);
	//CLANG_PROFILE(query2);
	r2 = dbt5_sql_execute(query2, &result2, "TRADE_RESULT1_2");
	if(r2==1 && result2.result_set){
			dbt5_sql_fetchrow(&result2);
			strncpy(type_name, dbt5_sql_getvalue(&result2, 0, length), length);
			type_is_sell = atoi(dbt5_sql_getvalue(&result2, 1, length));
			type_is_market = atoi(dbt5_sql_getvalue(&result2, 2, length));
			dbt5_sql_close_cursor(&result2);
	}else{
string fail_msg("query fail");
throw fail_msg.c_str();
	}

	sprintf(query3, TRADE_RESULT1_3, acct_id, symbol);
	//CLANG_PROFILE(query3);
	r3 = dbt5_sql_execute(query3, &result3, "TRADE_RESULT1_3");
	if(r3==1 && result3.result_set){
			dbt5_sql_fetchrow(&result3);
			hs_qty = atol(dbt5_sql_getvalue(&result3, 0, length));
			dbt5_sql_close_cursor(&result3);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}

	sprintf(query4, TRADE_RESULT_HELPER);
	//CLANG_PROFILE(query4);
	r4 = dbt5_sql_execute(query4, &result4, "TRADE_RESULT_HELPER");
	if(r4==1 && result4.result_set){
			dbt5_sql_fetchrow(&result4);
			strncpy(now_dts, dbt5_sql_getvalue(&result4, 0, length), length);
			dbt5_sql_close_cursor(&result4);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}

	sprintf(query5, TRADE_RESULT2_1, acct_id);
	//CLANG_PROFILE(query5);
	r5 = dbt5_sql_execute(query5, &result5, "TRADE_RESULT2_1");
	if(r5==1 && result5.result_set){
			dbt5_sql_fetchrow(&result5);
			broker_id = atol(dbt5_sql_getvalue(&result5, 0, length));
			cust_id = atol(dbt5_sql_getvalue(&result5, 1, length));
			tax_status = atoi(dbt5_sql_getvalue(&result5, 2, length));
			dbt5_sql_close_cursor(&result5);
	}else{
		string fail_msg("query fail");
		throw fail_msg.c_str();
	}

		sscanf(now_dts, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&trade_dts.year,
			&trade_dts.month,
			&trade_dts.day,
			&trade_dts.hour,
			&trade_dts.minute,
			&trade_dts.second);

	if(type_is_sell){
			if(hs_qty == 0){
					sprintf(query6, TRADE_RESULT2_2a, acct_id, symbol, (-1)*trade_qty);
					//CLANG_PROFILE(query6);
					r6=dbt5_sql_execute(query6, &result6, "TRADE_RESULT2_2a");
					if(!r6){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}else if(hs_qty != trade_qty){
					sprintf(query7, TRADE_RESULT2_2b, hs_qty-trade_qty, acct_id, symbol);
					//CLANG_PROFILE(query7);
					r7=dbt5_sql_execute(query7, &result7, "TRADE_RESULT2_2a");
					if(!r7){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}

			needed_qty = trade_qty;

			if(hs_qty > 0){
					if(is_lifo){
							sprintf(query80, TRADE_RESULT2_3a, acct_id, symbol);
							//CLANG_PROFILE(query80);
							r80 = dbt5_sql_execute(query80, &result80, "TRADE_RESULT2_3a");
							if(r80==1 && result80.result_set){
									num_rows0 = result80.num_rows;
							}else{
									string fail_msg("query fail");
									throw fail_msg.c_str();
							}
							result8 = result80;
					}else{
							sprintf(query81, TRADE_RESULT2_3a, acct_id, symbol);
							//CLANG_PROFILE(query81);
							r81 = dbt5_sql_execute(query81, &result81, "TRADE_RESULT2_3a");
							if(r81==1 && result81.result_set){
									num_rows0 = result81.num_rows;
							}else{
									string fail_msg("query fail");
									throw fail_msg.c_str();
							}
							result8 = result81;
					}
					while(needed_qty > 0 && cnt0 < num_rows0){
							dbt5_sql_fetchrow(&result8);
							cnt0++;

							hold_id = atol(dbt5_sql_getvalue(&result8, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result8, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result8, 2, length));

							if(hold_qty > needed_qty){

									sprintf(query9, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty-needed_qty);
									//CLANG_PROFILE(query9);
									r9 = dbt5_sql_execute(query9, &result9, "TRADE_RESULT2_4a");
									if(!r9){
													string fail_msg("query fail");
													throw fail_msg.c_str();
									}

									sprintf(query10, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
									//CLANG_PROFILE(query10);
									r10 = dbt5_sql_execute(query10, &result10, "TRADE_RESULT2_5a");
									if(!r10){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{

									sprintf(query11, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
									//CLANG_PROFILE(query11);
									r11 = dbt5_sql_execute(query11, &result11, "TRADE_RESULT2_4a");
									if(!r11){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									sprintf(query12, TRADE_RESULT2_5b, hold_id);
									//CLANG_PROFILE(query12);
									r12 = dbt5_sql_execute(query12, &result12, "TRADE_RESULT2_5b");
									if(!r12){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){

				sprintf(query13, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty);
				//CLANG_PROFILE(query13);
				r13 = dbt5_sql_execute(query13, &result13, "TRADE_RESULT2_4a");
				if(!r13){
						string fail_msg("query fail");
						throw fail_msg.c_str();
				}

				sprintf(query14, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, (-1)*needed_qty);
				//CLANG_PROFILE(query14);
				r14 = dbt5_sql_execute(query13, &result14, "TRADE_RESULT2_7a");
				if(!r14){
						string fail_msg("query fail");
						throw fail_msg.c_str();
				}
			}else if(hs_qty == trade_qty){
					sprintf(query15, TRADE_RESULT2_7b, acct_id, symbol);
					//CLANG_PROFILE(query15);
					r15 = dbt5_sql_execute(query15, &result15, "TRADE_RESULT2_7b");
					if(!r15){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}
	}
	else{
			if(hs_qty == 0){
 					sprintf(query16, TRADE_RESULT2_8a, acct_id, symbol, trade_qty);
					//CLANG_PROFILE(query16);
					r16 = dbt5_sql_execute(query16, &result16, "TRADE_RESULT2_8a");
					if(!r16){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}else{
 					sprintf(query17, TRADE_RESULT2_8b, hs_qty+trade_qty, acct_id, symbol);
					//CLANG_PROFILE(query17);
					r17 = dbt5_sql_execute(query17, &result17, "TRADE_RESULT2_8b");
					if(!r17){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}

			if(hs_qty < 0){
					if(is_lifo){
							sprintf(query180, TRADE_RESULT2_3a, acct_id, symbol);
							//CLANG_PROFILE(query180);
							r180 = dbt5_sql_execute(query180, &result180, "TRADE_RESULT2_3a");
							if(r180==1 && result180.result_set){
									num_rows1 = result180.num_rows;
							}else{
									string fail_msg("query fail");
									throw fail_msg.c_str();
							}
							result18 = result180;
					}else{
							sprintf(query181, TRADE_RESULT2_3a, acct_id, symbol);
							//CLANG_PROFILE(query181);
							r181 = dbt5_sql_execute(query181, &result180, "TRADE_RESULT2_3a");
							if(r181==1 && result181.result_set){
									num_rows1 = result181.num_rows;
							}else{
									string fail_msg("query fail");
									throw fail_msg.c_str();
							}
							result18 = result181;
					}
					while(needed_qty>0 && cnt1<num_rows1){
							dbt5_sql_fetchrow(&result18);
							cnt1++;
							hold_id = atol(dbt5_sql_getvalue(&result18, 0, length));
							hold_qty = atol(dbt5_sql_getvalue(&result18, 1, length));
							hold_price = atof(dbt5_sql_getvalue(&result18, 2, length));
							if(hold_qty > needed_qty){
									sprintf(query19, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty+needed_qty);
									//CLANG_PROFILE(query19);
									r19 = dbt5_sql_execute(query19, &result19, "TRADE_RESULT2_4a");
									if(!r19){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									sprintf(query20, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id);
									//CLANG_PROFILE(query20);
									r20 = dbt5_sql_execute(query20, &result20, "TRADE_RESULT2_5a");
									if(!r20){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									buy_value = buy_value + (needed_qty * hold_price);
									sell_value = sell_value + (needed_qty * trade_price);
									needed_qty = 0;
							}else{

									sprintf(query21, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0);
									//CLANG_PROFILE(query21);
									r21 = dbt5_sql_execute(query21, &result21, "TRADE_REUSLT2_4a");
									if(!r21){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									sprintf(query22, TRADE_RESULT2_5b, hold_id);
									//CLANG_PROFILE(query22);
									r22 = dbt5_sql_execute(query22, &result22, "TRADE_RESULT2_5b");
									if(!r22){
											string fail_msg("query fail");
											throw fail_msg.c_str();
									}

									hold_qty = (-1)*hold_qty;
									buy_value = buy_value + (hold_qty * hold_price);
									sell_value = sell_value + (hold_qty * trade_price);
									needed_qty = needed_qty - hold_qty;
							}
					}
			}
			if(needed_qty > 0){

					sprintf(query23, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty);
					//CLANG_PROFILE(query23);
					r23 = dbt5_sql_execute(query23, &result23, "TRADE_RESULT2_4a");
					if(!r23){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}

					sprintf(query24, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, needed_qty);
					//CLANG_PROFILE(query24);
					r24 = dbt5_sql_execute(query24, &result24, "TRADE_RESULT2_7a");
					if(!r24){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}

			}else if((-1)*hs_qty == trade_qty){

					sprintf(query25, TRADE_RESULT2_7b, acct_id, symbol);
					//CLANG_PROFILE(query25);
					r25 = dbt5_sql_execute(query25, &result25, "TRADE_RESULT2_7b");
					if(!r25){
							string fail_msg("query fail");
							throw fail_msg.c_str();
					}
			}
	}

	if((tax_status == 1 || tax_status == 2) && sell_value > buy_value){

			sprintf(query26, TRADE_RESULT3_1, cust_id);
			//CLANG_PROFILE(query26);
			r26 = dbt5_sql_execute(query26, &result26, "TRADE_RESULT3_1");
			if(r26==1 && result26.result_set){
					dbt5_sql_fetchrow(&result26);
					tax_rate = atof(dbt5_sql_getvalue(&result26, 0, length));
					dbt5_sql_close_cursor(&result26);
			}else{
					string fail_msg("query fail");
					throw fail_msg.c_str();
			}

			sprintf(query27, TRADE_RESULT3_2, tax_rate*(sell_value - buy_value), trade_id);
			//CLANG_PROFILE(query27);
			r27 = dbt5_sql_execute(query27, &result27, "TRADE_RESULT3_2");
			if(!r27){
					string fail_msg("query fail");
					throw fail_msg.c_str();
			}

			tax_amount = tax_rate*(sell_value - buy_value);
	}


	sprintf(query28, TRADE_RESULT4_1, symbol);
	//CLANG_PROFILE(query28);
	r28 = dbt5_sql_execute(query28, &result28, "TRADE_RESULT4_1");
	if(r28==1 && result28.result_set){
			dbt5_sql_fetchrow(&result28);
			strncpy(sec_ex_id, dbt5_sql_getvalue(&result28, 0, length), length);
			strncpy(s_name, dbt5_sql_getvalue(&result28, 1, length), length);
			dbt5_sql_close_cursor(&result28);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}


	sprintf(query29, TRADE_RESULT4_2, cust_id);
	//CLANG_PROFILE(query29);
	r29 = dbt5_sql_execute(query29, &result29, "TRADE_RESULT4_2");
	if(r29==1 && result29.result_set){
			dbt5_sql_fetchrow(&result29);
			cust_tier = atoi(dbt5_sql_getvalue(&result29, 0, length));
			dbt5_sql_close_cursor(&result29);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}


	sprintf(query30, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty);
	//CLANG_PROFILE(query30);
	r30 = dbt5_sql_execute(query30, &result30, "TRADE_RESULT4_3");
	if(r30==1 && result30.result_set){
			dbt5_sql_fetchrow(&result30);
			comm_rate = atof(dbt5_sql_getvalue(&result30, 0, length));
			dbt5_sql_close_cursor(&result30);
	}else{
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}


	comm_amount = ( comm_rate / 100.00 ) * ( trade_qty * trade_price );
  // round up for correct precision (cents only)
  comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;


	sprintf(query31, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id);
	//CLANG_PROFILE(query31);
	r31 = dbt5_sql_execute(query31, &result31, "TRADE_RESULT5_1");
	if(!r31){
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}


	sprintf(query32, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id);
	//CLANG_PROFILE(query32);
	r32 = dbt5_sql_execute(query32, &result32, "TRADE_RESULT5_2");
	if(!r32){
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}


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


	sprintf(query33, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount);
	//CLANG_PROFILE(query33);
	r33 = dbt5_sql_execute(query33, &result33, "TRADE_RESULT6_1");
	if(!r33){
			string fail_msg("query fail");
			throw fail_msg.c_str();
	}

	if(trade_is_cash){
			sprintf(query34, TRADE_RESULT6_2, se_amount, acct_id);
			//CLANG_PROFILE(query34);
			r34 = dbt5_sql_execute(query34, &result34, "TRADE_RESULT6_2");
			if(!r34){
					string fail_msg("query fail");
					throw fail_msg.c_str();
			}

			sprintf(query35, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name);
			//CLANG_PROFILE(query35);
			r35 = dbt5_sql_execute(query35, &result35, "TRADE_RESULT6_3");
			if(!r35){
					string fail_msg("query fail");
					throw fail_msg.c_str();
			}

			sprintf(query36, TRADE_RESULT6_4, acct_id);
			//CLANG_PROFILE(query36);
			r36 = dbt5_sql_execute(query36, &result36, "TRADE_RESULT6_4");
			if(r36==1 && result36.result_set){
					dbt5_sql_fetchrow(&result36);
					acct_bal = atof(dbt5_sql_getvalue(&result36, 0, length));
					dbt5_sql_close_cursor(&result36);
			}else{
					string fail_msg("query fail");
					throw fail_msg.c_str();
			}

	}

	pOut->acct_id = acct_id;
	pOut->acct_bal = acct_bal;
	pOut->load_unit = cust_id;

	return ;
}
#endif
