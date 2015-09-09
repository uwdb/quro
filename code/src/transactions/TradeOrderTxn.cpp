#include <stdlib.h>
#include "DBConnection.h"
#include "TradeOrderDB.h"

#ifndef DB_PGSQL

void CDBConnection::execute(PTradeOrderTxnInput pIn,
				PTradeOrderIntermediate pInter,
				PTradeOrderTxnOutput pOut)
{

		char query[1024];
		sql_result_t result;
		sql_result_t result_t;
		sql_result_t result_rename;
		double t_time;
		int length;
		char* val;
		int r;

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

		GETTIME;
		sprintf(query, SQLTOF1_1, pIn->acct_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_1");
		fflush(stdout);
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			//var = dbt5_sql_getvalue(&result, 0, length);
			strcpy(acct_name, dbt5_sql_getvalue(&result, 0, length));
		 	broker_id = atol(dbt5_sql_getvalue(&result, 1, length));
			cust_id = atol(dbt5_sql_getvalue(&result, 2, length));
			tax_status = atoi(dbt5_sql_getvalue(&result, 3, length));
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order 1 fail");
			throw fail_msg.c_str();
		}
		GETPROFILE(0);

		GETTIME;
		sprintf(query, SQLTOF1_2, cust_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_2");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			strcpy(cust_f_name, dbt5_sql_getvalue(&result, 0, length));
			strcpy(cust_l_name, dbt5_sql_getvalue(&result, 1, length));
			cust_tier = atoi(dbt5_sql_getvalue(&result, 2, length));
			strcpy(tax_id, dbt5_sql_getvalue(&result, 3, length));
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order 2 fail");
			throw fail_msg.c_str();
		}
		GETPROFILE(1);

		GETTIME;
		sprintf(query, SQLTOF1_3, broker_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_3");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			strcpy(broker_name, dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order 3 fail");
			throw fail_msg.c_str();
		}
		GETPROFILE(2);


		if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
			GETTIME;
			sprintf(query, SQLTOF2_1, pIn->acct_id, pIn->exec_f_name, pIn->exec_l_name, pIn->exec_tax_id);
			r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					strcpy(ap_acl, dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					string fail_msg("trade order 2_1 fail");
					throw fail_msg.c_str();
			}
			GETPROFILE(3);
		}


		//--------------------Frame 3-----------------------
		if(strlen(pIn->symbol)==0){
				GETTIME;
				sprintf(query, SQLTOF3_1a, pIn->co_name);
				r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4");
				if(r==1 && result.result_set){
						dbt5_sql_fetchrow(&result);
						co_id = atol(dbt5_sql_getvalue(&result, 0, length));
						dbt5_sql_close_cursor(&result);
				}else{
						string fail_msg("trade order 3_1 fail");
						throw fail_msg.c_str();
				}
				GETPROFILE(4);

				GETTIME;
				sprintf(query, SQLTOF3_2a, co_id, pIn->issue);
				r = dbt5_sql_execute(query, &result, "TRADE_ORDER_5");
				if(r==1 && result.result_set){
						dbt5_sql_fetchrow(&result);
						strcpy(exch_id, dbt5_sql_getvalue(&result, 0, length));
						strcpy(s_name, dbt5_sql_getvalue(&result, 1, length));
						strcpy(symbol, dbt5_sql_getvalue(&result, 2, length));
						dbt5_sql_close_cursor(&result);
				}else{
						string fail_msg("trade order 3_2 fail");
						throw fail_msg.c_str();
				}
				GETPROFILE(5);
		}else{
				GETTIME;
				sprintf(query, SQLTOF3_1b, pIn->symbol);
				r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4b");
				if(r==1 && result.result_set){
						dbt5_sql_fetchrow(&result);
						strcpy(symbol, pIn->symbol);
						co_id = atol(dbt5_sql_getvalue(&result, 0, length));
						strcpy(exch_id, dbt5_sql_getvalue(&result, 1, length));
						strcpy(s_name, dbt5_sql_getvalue(&result, 2, length));
						dbt5_sql_close_cursor(&result);
				}else{
						string fail_msg("trade order 3_3 fail");
						throw fail_msg.c_str();
				}
				GETPROFILE(6);

				GETTIME;
				sprintf(query, SQLTOF3_2b, co_id);
				r = dbt5_sql_execute(query, &result, "TRADE_ORDER_5b");
				if(r==1 && result.result_set){
						dbt5_sql_fetchrow(&result);
						strcpy(co_name, dbt5_sql_getvalue(&result, 0, length));
						dbt5_sql_close_cursor(&result);
				}else{
						string fail_msg("trade order 3_4 fail");
						throw fail_msg.c_str();
				}
				GETPROFILE(7);
		}

		GETTIME;
		sprintf(query, SQLTOF3_3, symbol);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_6");
		if(r==1 && result.result_set){
				dbt5_sql_fetchrow(&result);
				market_price = atol(dbt5_sql_getvalue(&result, 0, length));
				dbt5_sql_close_cursor(&result);
		}else{
				string fail_msg("trade order 3_5 fail");
				throw fail_msg.c_str();
		}
		GETPROFILE(8);

		GETTIME;
		sprintf(query, SQLTOF3_4, pIn->trade_type_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_7");
		if(r==1 && result.result_set){
				dbt5_sql_fetchrow(&result);
				type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
				type_is_market = atoi(dbt5_sql_getvalue(&result, 0, length));
				dbt5_sql_close_cursor(&result);
		}else{
				string fail_msg("trade order 3_6 fail");
				throw fail_msg.c_str();
		}
		GETPROFILE(9);


		if(type_is_market){
				requested_price = market_price;
		}else{
				requested_price = pIn->requested_price;
		}
		GETTIME;
		sprintf(query, SQLTOF3_5, pIn->acct_id, symbol);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_8");
		if(r==1 && result.result_set){
				dbt5_sql_fetchrow(&result);
				hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
				dbt5_sql_close_cursor(&result);
		}else{
				string fail_msg("trade order 3_7 fail");
				throw fail_msg.c_str();
		}
		GETPROFILE(10);



		//--------------------------------
		if(type_is_sell){
				if(hs_qty > 0){
						if(is_lifo){
							GETTIME;
							sprintf(query, SQLTOF3_6a, pIn->acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_ORDER_9");
							if(r && result.result_set){
									num_rows = result.num_rows;
							}else{
									string fail_msg("trade order 3_8 fail");
									throw fail_msg.c_str();
							}
							GETPROFILE(11);
						}else{
							GETTIME;
							sprintf(query, SQLTOF3_6b, pIn->acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_ORDER_10");
							if(r && result.result_set){
									num_rows = result.num_rows;
							}else{
									string fail_msg("trade order 3_9 fail");
									throw fail_msg.c_str();
							}
							GETPROFILE(12);
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
							GETTIME;
							sprintf(query, SQLTOF3_6a, pIn->acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_ORDER_9");
							if(r && result.result_set){
									num_rows = result.num_rows;
							}else{
									string fail_msg("trade order 3_8 fail");
									throw fail_msg.c_str();
							}
							GETPROFILE(11);
						}else{
							GETTIME;
							sprintf(query, SQLTOF3_6b, pIn->acct_id, symbol);
							r = dbt5_sql_execute(query, &result, "TRADE_ORDER_10");
							if(r && result.result_set){
									num_rows = result.num_rows;
							}else{
									string fail_msg("trade order 3_9 fail");
									throw fail_msg.c_str();
							}
							GETPROFILE(12);
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
			GETTIME;
			sprintf(query, SQLTOF3_7, cust_id);
			r = dbt5_sql_execute(query, &result, "TRADE_ORDER_11");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					tax_rates = atof(dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					string fail_msg("trade order 3_7 fail");
					throw fail_msg.c_str();
			}
			tax_amount = (sell_value - buy_value) * tax_rates;
			GETPROFILE(13);
		}

		GETTIME;
		sprintf(query, SQLTOF3_8, cust_tier, pIn->trade_type_id, exch_id, pIn->trade_qty, pIn->trade_qty);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_12");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order 3_8 fail");
			throw fail_msg.c_str();
		}
		GETPROFILE(14);

		GETTIME;
		sprintf(query, SQLTOF3_9, cust_tier, pIn->trade_type_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_13");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			charge_amount = atof(dbt5_sql_getvalue(&result, 0, length));
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order 3_9 fail");
			throw fail_msg.c_str();
		}
		GETPROFILE(15);


		if(type_is_margin){
			GETTIME;
			sprintf(query, SQLTOF3_10, pIn->acct_id);
			r = dbt5_sql_execute(query, &result, "TRADE_ORDER_14");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					string fail_msg("trade order 3_10 fail");
					throw fail_msg.c_str();
			}
			GETPROFILE(16);

			GETTIME;
			sprintf(query, SQLTOF3_11, pIn->acct_id);
			r = dbt5_sql_execute(query, &result, "TRADE_ORDER_15");
			if(r==1 && result.result_set){
					dbt5_sql_fetchrow(&result);
					acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result, 0, length));
					dbt5_sql_close_cursor(&result);
			}else{
					if(result.num_rows==0){
							acct_assets = acct_bal;
					}
			}
			GETPROFILE(17);
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

		GETTIME;
		sprintf(query, SQLTOF4_0);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_16");
		if(!r){
			string fail_msg("trade order frame 4 query 1 fails");
			throw fail_msg.c_str();
		}
		GETPROFILE(18);

		GETTIME;
		sprintf(query, SQLTOF4_00);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_17");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			next_t_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
			dbt5_sql_close_cursor(&result);
		}else{
			string fail_msg("trade order frame 4 query 1_1 fails");
			throw fail_msg.c_str();
		}

/*
		sprintf(query, SQLTOF4_1, next_t_id, status_id, pIn->trade_type_id, is_cash, symbol, pIn->trade_qty, requested_price, pIn->acct_id, exec_name, charge_amount, comm_amount, pIn->is_lifo);
	r = dbt5_sql_execute(query, &result, "TRADE_ORDER_18");
	if(!r){
			string fail_msg("trade order frame 4 query 4_1 fail");
			throw fail_msg.c_str();
	}
	GETPROFILE(19);
*/

		if(type_is_market == 0){
			GETTIME;
			sprintf(query, SQLTOF4_2, next_t_id, pIn->trade_type_id, symbol, pIn->trade_qty, requested_price, broker_id);
			r = dbt5_sql_execute(query, &result, "TRADE_ORDER_19");
			if(!r){
					string fail_msg("trade order frame 4 query 2 fail");
					throw fail_msg.c_str();
			}
			GETPROFILE(20);
		}


		GETTIME;
	 	sprintf(query, SQLTOF4_3, next_t_id, status_id);
		r = dbt5_sql_execute(query, &result, "TRADE_ORDER_20");
		if(!r){
				string fail_msg("trade order frame 4 query 3 fail");
				throw fail_msg.c_str();
		}
		GETPROFILE(21);

		if(pIn->roll_it_back){
				rollback_fail_msg.assign("force roll back");
				throw rollback_fail_msg.c_str();
		}

		sprintf(query, SQLTOF4_1, next_t_id, status_id, pIn->trade_type_id, is_cash, symbol, pIn->trade_qty, requested_price, pIn->acct_id, exec_name, charge_amount, comm_amount, pIn->is_lifo);
	r = dbt5_sql_execute(query, &result, "TRADE_ORDER_18");
	if(!r){
			string fail_msg("trade order frame 4 query 4_1 fail");
			throw fail_msg.c_str();
	}
	GETPROFILE(19);


		pOut->buy_value = buy_value;
		pOut->sell_value = sell_value;
		pOut->tax_amount = tax_amount;
		pOut->trade_id = next_t_id;
		strncpy(pInter->symbol, symbol, sizeof(pInter->symbol));
		pInter->trade_id = next_t_id;
		pInter->trade_qty = pIn->trade_qty;
		pInter->type_is_market = type_is_market;


		pInter->is_lifo = is_lifo;
		pInter->trade_is_cash = is_cash;
		pInter->charge = charge_amount;
		pInter->acct_id = acct_id;

		return ;



/*
//=============================================
//=============================================
//=============================================
	string fail_msg;
		sprintf(query, SQLTOF3_4, pIn->trade_type_id);

//conflict index = 1702
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_7");


if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length));
type_is_market = atoi(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_6 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(!(strlen(pIn->symbol)==0)){
	sprintf(query, SQLTOF3_1b, pIn->symbol);


}

if(!(strlen(pIn->symbol)==0)){
	//conflict index = 4362
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4b");


}


if(!(strlen(pIn->symbol)==0)){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(symbol, pIn->symbol);
co_id = atol(dbt5_sql_getvalue(&result, 0, length));
strcpy(exch_id, dbt5_sql_getvalue(&result, 1, length));
strcpy(s_name, dbt5_sql_getvalue(&result, 2, length));
dbt5_sql_close_cursor(&result);

}


}
if(!(strlen(pIn->symbol)==0)){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_3 fail");


}


}
if(!(strlen(pIn->symbol)==0)){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
if(type_is_margin){
	sprintf(query, SQLTOF3_10, pIn->acct_id);


}

if(type_is_margin){
	//conflict index = 4651
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_14");


}


if(type_is_margin){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

acct_bal = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(type_is_margin){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_10 fail");


}


}
if(type_is_margin){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}

sprintf(query, SQLTOF1_1, pIn->acct_id);

//conflict index = 4651
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_1");


if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(acct_name, dbt5_sql_getvalue(&result, 0, length));
broker_id = atol(dbt5_sql_getvalue(&result, 1, length));
cust_id = atol(dbt5_sql_getvalue(&result, 2, length));
tax_status = atoi(dbt5_sql_getvalue(&result, 3, length));
dbt5_sql_close_cursor(&result);
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4");



}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 1 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(strlen(pIn->symbol)==0){
	sprintf(query, SQLTOF3_1a, pIn->co_name);


}

if(strlen(pIn->symbol)==0){
	//conflict index = 5555
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4");


}


if(strlen(pIn->symbol)==0){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

co_id = atol(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(strlen(pIn->symbol)==0){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_1 fail");


}


}
if(strlen(pIn->symbol)==0){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
if(strlen(pIn->symbol)==0){
	sprintf(query, SQLTOF3_2a, co_id, pIn->issue);


}

if(strlen(pIn->symbol)==0){
	//conflict index = 4362
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_5");


}


if(strlen(pIn->symbol)==0){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(exch_id, dbt5_sql_getvalue(&result, 0, length));
strcpy(s_name, dbt5_sql_getvalue(&result, 1, length));
strcpy(symbol, dbt5_sql_getvalue(&result, 2, length));
dbt5_sql_close_cursor(&result);

}


}
if(strlen(pIn->symbol)==0){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_2 fail");


}


}
if(strlen(pIn->symbol)==0){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
sprintf(query, SQLTOF3_5, pIn->acct_id, symbol);

QUERY_BEGIN_Q11:
//conflict index = 4678
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_8");

QUERY_END_Q11:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

hs_qty = atol(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_7 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}

if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	sprintf(query, SQLTOF3_6a, pIn->acct_id, symbol);


}


}


}

QUERY_BEGIN_Q12:
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_9");


}


}


}

QUERY_END_Q12:

if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(r && result.result_set){
	num_rows = result.num_rows;


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(!(r && result.result_set)){
	fail_msg.assign("trade order 3_8 fail");


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(is_lifo){
	if(!(r && result.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	sprintf(query, SQLTOF3_6b, pIn->acct_id, symbol);


}


}


}

QUERY_BEGIN_Q13:
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_10");


}


}


}

QUERY_END_Q13:

if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(r && result.result_set){
	num_rows = result.num_rows;


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(!(r && result.result_set)){
	fail_msg.assign("trade order 3_9 fail");


}


}


}


}
if(type_is_sell){
	if(hs_qty > 0){
	if(!(is_lifo)){
	if(!(r && result.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	sprintf(query, SQLTOF3_6a, pIn->acct_id, symbol);


}


}


}

QUERY_BEGIN_Q14:
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_9");


}


}


}

QUERY_END_Q14:

if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(r && result.result_set){
	num_rows = result.num_rows;


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(!(r && result.result_set)){
	fail_msg.assign("trade order 3_8 fail");


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(is_lifo){
	if(!(r && result.result_set)){
	throw fail_msg.c_str();


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	sprintf(query, SQLTOF3_6b, pIn->acct_id, symbol);


}


}


}

QUERY_BEGIN_Q15:
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	//conflict index = 4944
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_10");


}


}


}

QUERY_END_Q15:

if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(r && result.result_set){
	num_rows = result.num_rows;


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(!(r && result.result_set)){
	fail_msg.assign("trade order 3_9 fail");


}


}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	if(!(is_lifo)){
	if(!(r && result.result_set)){
	throw fail_msg.c_str();


}


}


}


}

if(!(strlen(pIn->symbol)==0)){
	sprintf(query, SQLTOF3_2b, co_id);


}

QUERY_BEGIN_Q8:
if(!(strlen(pIn->symbol)==0)){
	//conflict index = 5555
r = dbt5_sql_execute(query, &result_rename, "TRADE_ORDER_5b");


}

QUERY_END_Q8:

if(!(strlen(pIn->symbol)==0)){
	if(r==1 && result_rename.result_set){
	dbt5_sql_fetchrow(&result_rename);

strcpy(co_name, dbt5_sql_getvalue(&result_rename, 0, length));
dbt5_sql_close_cursor(&result_rename);

}


}
if(!(strlen(pIn->symbol)==0)){
	if(!(r==1 && result_rename.result_set)){
	fail_msg.assign("trade order 3_4 fail");


}


}
if(!(strlen(pIn->symbol)==0)){
	if(!(r==1 && result_rename.result_set)){
	throw fail_msg.c_str();


}


}
sprintf(query, SQLTOF3_3, symbol);

QUERY_BEGIN_Q9:
//conflict index = 8510
r = dbt5_sql_execute(query, &result_rename, "TRADE_ORDER_6");

QUERY_END_Q9:

if(r==1 && result_rename.result_set){
	dbt5_sql_fetchrow(&result_rename);

market_price = atol(dbt5_sql_getvalue(&result_rename, 0, length));
dbt5_sql_close_cursor(&result_rename);

}
if(!(r==1 && result_rename.result_set)){
	fail_msg.assign("trade order 3_5 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(type_is_market){
	requested_price = market_price;


}
if(!(type_is_market)){
	requested_price = pIn->requested_price;


}

if(type_is_sell){
	if(hs_qty > 0){
	while(needed_qty>0 && cnt < num_rows){
	dbt5_sql_fetchrow(&result);

hold_qty = atol(dbt5_sql_getvalue(&result, 0, length));
hold_price = atof(dbt5_sql_getvalue(&result, 1, length));
cnt++;
if(hold_qty > needed_qty){
	buy_value = buy_value + (needed_qty * hold_price);

sell_value = sell_value + (needed_qty * requested_price);
needed_qty = 0;

}
if(!(hold_qty > needed_qty)){
	buy_value = buy_value + (hold_qty * hold_price);

sell_value = sell_value + (hold_qty * requested_price);
needed_qty = needed_qty - hold_qty;

}

}


}


}
if(!(type_is_sell)){
	if(hs_qty < 0){
	while(needed_qty>0 && cnt < num_rows){
	dbt5_sql_fetchrow(&result);

hold_qty = atol(dbt5_sql_getvalue(&result, 0, length));
hold_price = atof(dbt5_sql_getvalue(&result, 1, length));
cnt++;
if(hold_qty + needed_qty < 0){
	sell_value = sell_value + (needed_qty * hold_price);

buy_value = buy_value + (needed_qty * requested_price);
needed_qty = 0;

}
if(!(hold_qty + needed_qty < 0)){
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


}

QUERY_BEGIN_Q16:
if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
	//conflict index = 1702
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_11");


}

QUERY_END_Q16:

if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

tax_rates = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_7 fail");


}


}
if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
if(type_is_margin){
	sprintf(query, SQLTOF3_11, pIn->acct_id);


}

QUERY_BEGIN_Q20:
if(type_is_margin){
	//conflict index = 8510
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_15");


}

QUERY_END_Q20:

if(type_is_margin){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(type_is_margin){
	if(!(r==1 && result.result_set)){
	if(result.num_rows==0){
	acct_assets = acct_bal;


}


}


}
sprintf(query, SQLTOF1_2, cust_id);

QUERY_BEGIN_Q2:
//conflict index = 11428
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_2");

QUERY_END_Q2:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(cust_f_name, dbt5_sql_getvalue(&result, 0, length));
strcpy(cust_l_name, dbt5_sql_getvalue(&result, 1, length));
cust_tier = atoi(dbt5_sql_getvalue(&result, 2, length));
strcpy(tax_id, dbt5_sql_getvalue(&result, 3, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 2 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, SQLTOF3_9, cust_tier, pIn->trade_type_id);

QUERY_BEGIN_Q18:
//conflict index = 170
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_13");

QUERY_END_Q18:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

charge_amount = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_9 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
	sprintf(query, SQLTOF2_1, pIn->acct_id, pIn->exec_f_name, pIn->exec_l_name, pIn->exec_tax_id);


}

QUERY_BEGIN_Q4:
if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
	//conflict index = 32520
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_4");


}

QUERY_END_Q4:

if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
	if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(ap_acl, dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}


}
if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
	if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 2_1 fail");


}


}
if(strcmp(pIn->exec_l_name, cust_l_name)
            || strcmp(pIn->exec_f_name, cust_f_name)
            || strcmp(pIn->exec_tax_id, tax_id)){
	if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}


}
sprintf(query, SQLTOF3_8, cust_tier, pIn->trade_type_id, exch_id, pIn->trade_qty, pIn->trade_qty);

QUERY_BEGIN_Q17:
//conflict index = 125000
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_12");

QUERY_END_Q17:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

comm_rate = atof(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3_8 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, SQLTOF1_3, broker_id);

QUERY_BEGIN_Q3:
//conflict index = 333333
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_3");

QUERY_END_Q3:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

strcpy(broker_name, dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order 3 fail");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
sprintf(query, SQLTOF4_0);

QUERY_BEGIN_Q21:
//conflict index = 4000000
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_16");

QUERY_END_Q21:

if(!r){
	fail_msg.assign("trade order frame 4 query 1 fails");


}
if(!r){
	throw fail_msg.c_str();


}
sprintf(query, SQLTOF4_00);

QUERY_BEGIN_Q22:
//conflict index = 4000000
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_17");

QUERY_END_Q22:

if(r==1 && result.result_set){
	dbt5_sql_fetchrow(&result);

next_t_id = atol(dbt5_sql_getvalue(&result, 0, length));
dbt5_sql_close_cursor(&result);

}
if(!(r==1 && result.result_set)){
	fail_msg.assign("trade order frame 4 query 1_1 fails");


}
if(!(r==1 && result.result_set)){
	throw fail_msg.c_str();


}
if(type_is_market == 0){
	sprintf(query, SQLTOF4_2, next_t_id, pIn->trade_type_id, symbol, pIn->trade_qty, requested_price, broker_id);


}

QUERY_BEGIN_Q24:
if(type_is_market == 0){
	//conflict index = 399
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_19");


}

QUERY_END_Q24:

if(type_is_market == 0){
	if(!r){
	fail_msg.assign("trade order frame 4 query 2 fail");


}


}
if(type_is_market == 0){
	if(!r){
	throw fail_msg.c_str();


}


}
if(type_is_margin){
	strcpy(status_id, pIn->st_submitted_id);


}
if(!(type_is_margin)){
	strcpy(status_id, pIn->st_pending_id);


}
sprintf(query, SQLTOF4_3, next_t_id, status_id);

QUERY_BEGIN_Q25:
//conflict index = 400000
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_20");

QUERY_END_Q25:

if(!r){
	fail_msg.assign("trade order frame 4 query 3 fail");


}
if(!r){
	throw fail_msg.c_str();


}
sprintf(exec_name, "%s %s", pIn->exec_f_name, pIn->exec_l_name);
comm_amount = comm_rate / 100
				* pIn->trade_qty
				* requested_price;
comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;
comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;
sprintf(query, SQLTOF4_1, next_t_id, status_id, pIn->trade_type_id, is_cash, symbol, pIn->trade_qty, requested_price, pIn->acct_id, exec_name, charge_amount, comm_amount, pIn->is_lifo);

QUERY_BEGIN_Q23:
//conflict index = 500000
r = dbt5_sql_execute(query, &result, "TRADE_ORDER_18");

QUERY_END_Q23:

if(!r){
	fail_msg.assign("trade order frame 4 query 4_1 fail");


}
if(!r){
	throw fail_msg.c_str();


}
if(sell_value > buy_value && ((tax_status == 1 ) || ( tax_status == 2))){
	tax_amount = (sell_value - buy_value) * tax_rates;


}
if(pIn->roll_it_back){
	rollback_fail_msg.assign("force roll back");


}
if(pIn->roll_it_back){
	throw rollback_fail_msg.c_str();


}


pOut->buy_value = buy_value;
pOut->sell_value = sell_value;
pOut->tax_amount = tax_amount;
pOut->trade_id = next_t_id;
strncpy(pInter->symbol, symbol, sizeof(pInter->symbol));
pInter->trade_id = next_t_id;
pInter->trade_qty = pIn->trade_qty;
pInter->type_is_market = type_is_market;
pInter->is_lifo = is_lifo;
pInter->trade_is_cash = is_cash;
pInter->charge = charge_amount;
pInter->acct_id = acct_id;

return;
*/
}
#endif
