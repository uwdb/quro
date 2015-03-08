#include "DBConnection.h"
#include "TradeStatusDB.h"

#ifndef DB_PGSQL
void CDBConnection::execute(const TTradeStatusFrame1Input *pIn,
		TTradeStatusFrame1Output *pOut)
{
	char query[4096];
	sql_result_t result;
	int length;
	char* val;

	sprintf(query, TRADE_STATUS_1, pIn->acct_id);
	if(dbt5_sql_execute(query, &result, "TRADE_STATUS_1") && result.result_set){
			pOut->num_found = result.num_rows;
			for(int i=0; i<result.num_rows; i++){
						//snprintf(query, TRADE_STATUS_2, pIn->acct_id);
						dbt5_sql_fetchrow(&result);
						pOut->charge[i] = atof(dbt5_sql_getvalue(&result, 7, length));

						val = dbt5_sql_getvalue(&result, 9, length);
						strncpy(pOut->ex_name[i], val, length);

						val = dbt5_sql_getvalue(&result, 6, length);
						strncpy(pOut->exec_name[i], val, length);

						val = dbt5_sql_getvalue(&result, 8, length);
						strncpy(pOut->s_name[i], val, length);

						val = dbt5_sql_getvalue(&result, 2, length);
						strncpy(pOut->status_name[i], val, length);

						val = dbt5_sql_getvalue(&result, 4, length);
						strncpy(pOut->symbol[i], val, length);

						val = dbt5_sql_getvalue(&result, 1, length);
						sscanf(val, "%hd-%hd-%hd %hd:%hd:%hd",
									&pOut->trade_dts[i].year,
									&pOut->trade_dts[i].month,
									&pOut->trade_dts[i].day,
									&pOut->trade_dts[i].hour,
									&pOut->trade_dts[i].minute,
									&pOut->trade_dts[i].second);

						pOut->trade_id[i] = atol(dbt5_sql_getvalue(&result, 0, length));
						pOut->trade_qty[i] = atoi(dbt5_sql_getvalue(&result, 5, length));
						val = dbt5_sql_getvalue(&result, 3, length);
						strncpy(pOut->type_name[i], val, length);
			}
	}

	sprintf(query, TRADE_STATUS_2, pIn->acct_id);
	if(dbt5_sql_execute(query, &result, "TRADE_STATUS_2") && result.result_set){
			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(pOut->broker_name, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(pOut->cust_f_name, val, length);

			val = dbt5_sql_getvalue(&result, 0, length);
			strncpy(pOut->cust_l_name, val, length);


			dbt5_sql_close_cursor(&result);

	}
}
#endif
