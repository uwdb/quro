#include "DBConnection.h"
#include "TradeUpdateDB.h"

#ifndef DB_PGSQL

#ifdef TABLE_PROFILE
map<string, FIELDHIST> table_hist;
int profile_cnt = 0;
string tbl_name;
string param_str;
#endif

void CDBConnection::execute(const TTradeUpdateFrame1Input *pIn,
		TTradeUpdateFrame1Output *pOut)
{
	char query1[4096];
	char query2[4096];
	char query3[4096];
	char query4[4096];
	char query5[4096];
	char query6[4096];
	sql_result_t result1, result2, result3, result4, result5, result6;
	int length;
	int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;



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
	bool findX;

	cout<<"In TradeUpdate transaction!"<<endl;
	fflush(stdout);

	for(int i=0; i<pIn->max_trades; i++){

		cout<<"Trade id = "<<pIn->trade_id[i]<<", num_updated = "<<num_updated<<", max_updates = "<<max_updates<<endl;
		if(num_updated < max_updates)
		{
			sprintf(query1, TRADE_UPDATE_Q1, pIn->trade_id[i]);
			//CLANG_PROFILE(query1);
			r1 = dbt5_sql_execute(query1, &result1, "tpce_tu_1");
			if(r1==1 && result1.result_set){
					dbt5_sql_fetchrow(&result1);
					exec_name.assign(dbt5_sql_getvalue(&result1, 0, length));
					dbt5_sql_close_cursor(&result1);
			} else {
					string fail_msg("trade update frame 1 query 1 fails");
					throw fail_msg.c_str();
			}

		for(int j=0; j<exec_name.length(); j++){
				if(exec_name[j] == 'X'){
						exec_name[j] = 'X';
				}
		}

		sprintf(query2, TRADE_UPDATE_Q2, exec_name.c_str(), pIn->trade_id[i]);
		//CLANG_PROFILE(query2);
		r2 = dbt5_sql_execute(query2, &result2, "tpce_tu_2");
		if(!r2){
			string fail_msg("trade update frame 1 query 2 fails");
			throw fail_msg.c_str();
		}

			num_updated++;
		}


		sprintf(query3, TRADE_UPDATE_Q3, pIn->trade_id[i]);
		//CLANG_PROFILE(query3);
		r3 = dbt5_sql_execute(query3, &result3, "tpce_tu_3");
		if(r3==1 && result3.result_set){
			dbt5_sql_fetchrow(&result3);
			bid_price = atof(dbt5_sql_getvalue(&result3, 0, length));
			//exec_name.assign(dbt5_sql_getvalue(&result3, 1, length));
			is_cash = atoi(dbt5_sql_getvalue(&result3, 2, length));
			is_market = atoi(dbt5_sql_getvalue(&result3, 3, length));
			trade_price = atof(dbt5_sql_getvalue(&result3, 4, length));
			dbt5_sql_close_cursor(&result3);
		}else {
			string fail_msg("trade update frame 1 query 3 fails");
			throw fail_msg.c_str();
		}

		sprintf(query4, TRADE_UPDATE_Q4, pIn->trade_id[i]);
		//CLANG_PROFILE(query4);
		r4 = dbt5_sql_execute(query4, &result4, "tpce_tu_4");
		if(r4==1 && result4.result_set){
			dbt5_sql_fetchrow(&result4);
			settlement_amount = atof(dbt5_sql_getvalue(&result4, 0, length));
			char* val;
			val = dbt5_sql_getvalue(&result4, 1, length);
			sscanf(val, "%d-%d-%d %d:%d:%f", &set_year, &set_month, &set_day, &set_hour, &set_min, &set_sec);
			val = dbt5_sql_getvalue(&result4, 2, length);
			strncpy(set_cash_type, val, length);
			dbt5_sql_close_cursor(&result4);
		}else{
			string fail_msg("trade update frame 1 query 4 fails");
			throw fail_msg.c_str();
		}

		if(is_cash){
				sprintf(query5, TRADE_UPDATE_Q5, pIn->trade_id[i]);
				//CLANG_PROFILE(query5);
				r5 = dbt5_sql_execute(query5, &result5, "tpce_tu_5");
				if(r5==1 && result5.result_set){
					dbt5_sql_fetchrow(&result5);
					cash_amount = atof(dbt5_sql_getvalue(&result5, 0, length));
					char* val;
					val = dbt5_sql_getvalue(&result5, 1, length);
					sscanf(val, "%d-%d-%d %d:%d:%f", &cash_year, &cash_month, &cash_day, &cash_hour, &cash_min, &cash_sec);
					val = dbt5_sql_getvalue(&result5, 2, length);
					strncpy(cash_name, val, length);
					dbt5_sql_close_cursor(&result5);
				}else {
					string fail_msg("trade update frame 1 query 5 fails");
					throw fail_msg.c_str();
				}

		}

		sprintf(query6, TRADE_UPDATE_Q6, pIn->trade_id[i]);
		//CLANG_PROFILE(query6);
		r6 = dbt5_sql_execute(query6, &result6, "tpce_tu_6");
		if(r6==1 && result6.result_set){
			num_rows = result6.num_rows;
			for(int j=0; j<num_rows; j++){
				dbt5_sql_fetchrow(&result6);
				char* val;
				val = dbt5_sql_getvalue(&result6, 0, length);
				sscanf(val, "%d-%d-%d %d:%d:%f",
					&trade_history_dts_year[j],
					&trade_history_dts_month[j],
					&trade_history_dts_day[j],
					&trade_history_dts_hour[j],
					&trade_history_dts_min[j],
					&trade_history_dts_sec[j]);
				val = dbt5_sql_getvalue(&result6, 1, length);
//				strncpy(trade_history_status_id[j], val, length);
			}
			dbt5_sql_close_cursor(&result6);
		}else {
			string fail_msg("trade update frame 1 query 6 fails");
			throw fail_msg.c_str();
		}


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
		strcpy(pOut->trade_info[i].settlement_cash_type, set_cash_type);
	}


}
#endif
