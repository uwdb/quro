#include "DBConnection.h"
#include "TradeOrderDB.h"


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

	TRADEORDER_F1Q1;

	TRADEORDER_F1Q2;

	TRADEORDER_F1Q3;

//--------------------Frame 2------------------
	TRADEORDER_F2Q1;
//--------------------Frame 3-----------------------
	if(strlen(symbol)==0){

		TRADEORDER_F3Q1;
		TRADEORDER_F3Q2;

	}else{

		TRADEORDER_F3Q3;
		TRADEORDER_F3Q4;

	}

	TRADEORDER_F3Q5;
	TRADEORDER_F3Q6;

	if(type_is_market){
			requested_price = market_price;
	}else{
			requested_price = requested_price;
	}

	TRADEORDER_F3Q13;

	TRADEORDER_F3Q14;

	if(type_is_margin){

			TRADEORDER_F3Q15;
			TRADEORDER_F3Q16;
	}
	if(type_is_margin){
			strcpy(status_id, st_submitted_id);
	}else{
			strcpy(status_id, st_pending_id);
	}


	TRADEORDER_F3Q7;

	size_t num_rows = 0;
	size_t cnt = 0;

//--------------------------------
	if(type_is_sell){
			if(hs_qty > 0){
					if(is_lifo){
							TRADEORDER_F3Q8;
					}else{
							TRADEORDER_F3Q9;
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
							TRADEORDER_F3Q10;
					}else{
							TRADEORDER_F3Q11;
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
			TRADEORDER_F3Q12;
			tax_amount = (sell_value - buy_value) * tax_rates;
	}


//------------------Frame 4-----------------------
	comm_amount = comm_rate / 100
                                          * trade_qty
                                          * requested_price;
	comm_amount = (double)((int)(100.00 * comm_amount + 0.5)) / 100.00;

	//FIXME: now_dts and trade_id
	TRADEORDER_F4Q1;
	TRADEORDER_F4Q2;

	TRADEORDER_F4Q3;

	if(type_is_market == 0){
			TRADEORDER_F4Q4;
	}

	TRADEORDER_F4Q5;

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
