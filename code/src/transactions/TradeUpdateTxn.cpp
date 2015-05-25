#include "DBConnection.h"
#include "TradeUpdateDB.h"

#ifndef DB_PGSQL

#ifdef PROFILE_EACH_QUERY
#define ADD_PROFILE_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#endif

#define FAIL_MSG(msg) \
				string fail_msg(msg); \
				if (r==2) fail_msg.append("\tempty result"); \
				throw fail_msg.c_str();

void CDBConnection::execute(const TTradeUpdateFrame1Input *pIn,
		TTradeUpdateFrame1Output *pOut)
{
	char query[4096];
	sql_result_t result;
	sql_result_t result_t;
	int length;
	char* val;
	int r = 0;


	int num_found = pIn->max_trades;
	int max_updates = pIn->max_updates;
	int num_updated = 0;
	uint64_t t_id = 0;
	string exec_name;
	size_t is_cash = 0;
	size_t is_market = 0;
	double bid_price = 0;
	double trade_price = 0;
	size_t set_year, set_month, set_day, set_hour, set_min;
	double set_sec = 0;
	char set_cash_type[40] = {0};
	double settlement_amount;
	double cash_amount;
	size_t cash_year, cash_month, cash_day, cash_hour, cash_min;
	double cash_sec;
	char cash_name[40] = {0};
	size_t trade_history_dts_year[3] = {0};
	size_t trade_history_dts_month[3] = {0};
	size_t trade_history_dts_day[3] = {0};
	size_t trade_history_dts_hour[3] = {0};
	size_t trade_history_dts_min[3] = {0};
	double trade_history_dts_sec[3] = {0};
	char trade_history_status_id[3][10] = {0};
	int num_rows = 0;


	for(int i=0; i<pIn->max_trades; i++){

		t_id = pIn->trade_id[i];

		if(num_updated < max_updates)
		{
			TRADEUPDATE_F1Q1;

			bool findX = false;
			for(int j=0; j<exec_name.length(); j++){
				if(exec_name[j] == 'X'){
					exec_name[j] = ' ';
					findX = true;
					break;
				}
			}
			if(findX == false){
				for(int j=0; j<exec_name.length(); j++){
					if(exec_name[j] == ' '){
						exec_name[j] = 'X';
					}
				}
			}

			TRADEUPDATE_F1Q2;
			num_updated++;
		}

//		sprintf(query, "SET SESSION profiling = 1");
//		dbt5_sql_execute(query, &result, "PROFILE");

		//getTradeType
		TRADEUPDATE_F1Q3;


		//getSettlement
		TRADEUPDATE_F1Q4;

		if(is_cash){
			TRADEUPDATE_F1Q5;
		}
		//getTradeHistory
		TRADEUPDATE_F1Q6;


		pOut->trade_info[i].bid_price = bid_price;
		pOut->trade_info[i].cash_transaction_amount = cash_amount;
		pOut->trade_info[i].settlement_amount = settlement_amount;
		pOut->trade_info[i].trade_price = trade_price;
		pOut->trade_info[i].is_cash = is_cash;
		pOut->trade_info[i].trade_history_dts[0].year = trade_history_dts_year[0];
		pOut->trade_info[i].trade_history_dts[0].month = trade_history_dts_month[0];
		pOut->trade_info[i].trade_history_dts[0].day = trade_history_dts_day[0];
		pOut->trade_info[i].trade_history_dts[0].hour = trade_history_dts_hour[0];
		pOut->trade_info[i].trade_history_dts[0].minute = trade_history_dts_min[0];
		pOut->trade_info[i].trade_history_dts[0].second = trade_history_dts_sec[0];

		pOut->trade_info[i].trade_history_dts[1].year = trade_history_dts_year[1];
		pOut->trade_info[i].trade_history_dts[1].month = trade_history_dts_month[1];
		pOut->trade_info[i].trade_history_dts[1].day = trade_history_dts_day[1];
		pOut->trade_info[i].trade_history_dts[1].hour = trade_history_dts_hour[1];
		pOut->trade_info[i].trade_history_dts[1].minute = trade_history_dts_min[1];
		pOut->trade_info[i].trade_history_dts[1].second = trade_history_dts_sec[1];

		pOut->trade_info[i].trade_history_dts[2].year = trade_history_dts_year[2];
		pOut->trade_info[i].trade_history_dts[2].month = trade_history_dts_month[2];
		pOut->trade_info[i].trade_history_dts[2].day = trade_history_dts_day[2];
		pOut->trade_info[i].trade_history_dts[2].hour = trade_history_dts_hour[2];
		pOut->trade_info[i].trade_history_dts[2].minute = trade_history_dts_min[2];
		pOut->trade_info[i].trade_history_dts[2].second = trade_history_dts_sec[2];

		pOut->trade_info[i].cash_transaction_dts.year = cash_year;
		pOut->trade_info[i].cash_transaction_dts.month = cash_month;
		pOut->trade_info[i].cash_transaction_dts.day = cash_day;
		pOut->trade_info[i].cash_transaction_dts.hour = cash_hour;
		pOut->trade_info[i].cash_transaction_dts.minute = cash_min;
		pOut->trade_info[i].cash_transaction_dts.second = cash_sec;


		pOut->trade_info[i].settlement_cash_due_date.year = set_year;
		pOut->trade_info[i].settlement_cash_due_date.month = set_month;
		pOut->trade_info[i].settlement_cash_due_date.day = set_day;
		pOut->trade_info[i].settlement_cash_due_date.hour = set_hour;
		pOut->trade_info[i].settlement_cash_due_date.minute = set_min;
		pOut->trade_info[i].settlement_cash_due_date.second = set_sec;


		strcpy(pOut->trade_info[i].cash_transaction_name, cash_name);
		strcpy(pOut->trade_info[i].exec_name, exec_name.c_str());
		strcpy(pOut->trade_info[i].settlement_cash_type, set_cash_type);;
		strcpy(pOut->trade_info[i].trade_history_status_id[0], trade_history_status_id[0]);
		strcpy(pOut->trade_info[i].trade_history_status_id[1], trade_history_status_id[1]);
		strcpy(pOut->trade_info[i].trade_history_status_id[2], trade_history_status_id[2]);

	}

/*
	for(int i=0; i<pIn->max_trades; i++){

		t_id = pIn->trade_id[i];
		if(num_updated < max_updates){
			TRADEUPDATE_F1Q1;

			bool findX = false;
			for(int j=0; j<exec_name.length(); j++){
				if(exec_name[j] == 'X'){
					exec_name[j] = ' ';
					findX = true;
					break;
				}
			}
			if(findX == false){
				for(int j=0; j<exec_name.length(); j++){
					if(exec_name[j] == ' '){
						exec_name[j] = 'X';
					}
				}
			}

			TRADEUPDATE_F1Q2;
			num_updated++;
		}
		strcpy(pOut->trade_info[i].exec_name, exec_name.c_str());

	}
*/

	pOut->num_found = num_found;
	pOut->num_updated = num_updated;

}
#endif
