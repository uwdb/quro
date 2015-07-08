/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * 11 July 2006
 */

#ifndef TRADE_UPDATE_DB_H
#define TRADE_UPDATE_DB_H

#include "TxnHarnessDBInterface.h"

#include "TxnBaseDB.h"
#include "DBConnection.h"

using namespace TPCE;


class CTradeUpdateDB : public CTxnBaseDB, public CTradeUpdateDBInterface
{
public:
	CTradeUpdateDB(CDBConnection *pDBConn) : CTxnBaseDB(pDBConn) {};
	~CTradeUpdateDB() {};

	virtual void DoTradeUpdateFrame1(const TTradeUpdateFrame1Input *pIn,
			TTradeUpdateFrame1Output *pOut);
	virtual void DoTradeUpdateFrame2(const TTradeUpdateFrame2Input *pIn,
			TTradeUpdateFrame2Output *pOut);
	virtual void DoTradeUpdateFrame3(const TTradeUpdateFrame3Input *pIn,
			TTradeUpdateFrame3Output *pOut);

	// Function to pass any exception thrown inside
	// database class frame implementation
	// back into the database class
	void Cleanup(void* pException) {};
};

#define SBTEST_TEST \
	"SELECT k, pad\n" \
	"FROM sbtest\n" \
	"WHERE id = %d"

#define TRADE_UPDATE_Q1 \
		"SELECT t_exec_name\n" \
		"FROM trade\n" \
		"WHERE t_id = %ld"

#define TRADE_UPDATE_QH1 \
		"SELECT REPLACE('%s', ' X ', ' ')"

#define TRADE_UPDATE_QH2 \
		"SELECT REPLACE('%s', ' ', ' X ')"

#define TRADE_UPDATE_Q2 \
		"UPDATE trade\n" \
		"SET t_exec_name = '%s'\n" \
		"WHERE t_id = %ld"

#define TRADE_UPDATE_Q3 \
		"SELECT t_bid_price, t_exec_name, t_is_cash, tt_is_mrkt,\n" \
		"       t_trade_price\n" \
		"FROM trade, trade_type\n" \
		"WHERE t_id = %ld\n" \
		"  AND t_tt_id = tt_id"

#define TRADE_UPDATE_Q4 \
		"SELECT se_amt, se_cash_due_date, se_cash_type\n" \
		"FROM settlement\n" \
		"WHERE se_t_id = %ld"

#define TRADE_UPDATE_Q5 \
		"SELECT ct_amt, ct_dts, ct_name\n" \
		"FROM cash_transaction\n" \
		"WHERE ct_t_id = %ld"

#define TRADE_UPDATE_Q6 \
		"SELECT th_dts, th_st_id\n" \
		"FROM trade_history\n" \
		"WHERE th_t_id = %ld\n" \
		"ORDER BY th_dts\n" \
		"LIMIT 3"

#ifdef PROFILE_EACH_QUERY
#define GETTIME gettimeofday(&t1, NULL);
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#else
#define GETTIME
#define ADD_QUERY_NODE(f, q, commit)
#endif

#define RETURN_ERROR(msg) \
				string fail_msg(msg); \
				fail_msg.append(query); \
				throw fail_msg.c_str();

#define TRADEUPDATE_F1Q1  sprintf(query, TRADE_UPDATE_Q1, t_id); \
		GETTIME; \
		if(dbt5_sql_execute(query, &result, "TRADE_UPDATE_1")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			exec_name.assign(val); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 1, 1); \
		} else { \
			RETURN_ERROR("trade update frame 1 query 1 fails");  \
		}

#define TRADEUPDATE_F1Q2 sprintf(query, TRADE_UPDATE_Q2, exec_name.c_str(), t_id);  \
		GETTIME; \
		if(!dbt5_sql_execute(query, &result, "TRADE_UPDATE_2")){ \
			RETURN_ERROR("trade update frame 1 query 2 fails"); \
		} \
		ADD_QUERY_NODE(1, 2, 1);

#define TRADEUPDATE_F1Q3 sprintf(query, TRADE_UPDATE_Q3, t_id); \
		GETTIME; \
		if(dbt5_sql_execute(query, &result, "TRADE_UPDATE_3")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			bid_price = atof(dbt5_sql_getvalue(&result, 0, length)); \
			exec_name.assign(dbt5_sql_getvalue(&result, 1, length)); \
			is_cash = atoi(dbt5_sql_getvalue(&result, 2, length)); \
			is_market = atoi(dbt5_sql_getvalue(&result, 3, length)); \
			trade_price = atof(dbt5_sql_getvalue(&result, 4, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 3, 1); \
		}else { \
			RETURN_ERROR("trade update frame 1 query 3 fails"); \
		}

#define TRADEUPDATE_F1Q4 sprintf(query, TRADE_UPDATE_Q4, t_id);  \
		GETTIME; \
		if(dbt5_sql_execute(query, &result, "TRADE_UPDATE_4")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result);  \
			settlement_amount = atof(dbt5_sql_getvalue(&result, 0, length)); \
			val = dbt5_sql_getvalue(&result, 1, length); \
			sscanf(val, "%d-%d-%d %d:%d:%f", &set_year, &set_month, &set_day, &set_hour, &set_min, &set_sec); \
			val = dbt5_sql_getvalue(&result, 2, length); \
			strncpy(set_cash_type, val, length); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 4, 1); \
		}else{ \
			RETURN_ERROR("trade update frame 1 query 4 fails"); \
		}

#define TRADEUPDATE_F1Q5 sprintf(query, TRADE_UPDATE_Q5, t_id); \
		GETTIME; \
		if(dbt5_sql_execute(query, &result, "TRADE_UPDATE_5")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			cash_amount = atof(dbt5_sql_getvalue(&result, 0, length)); \
			val = dbt5_sql_getvalue(&result, 1, length); \
			sscanf(val, "%d-%d-%d %d:%d:%f", &cash_year, &cash_month, &cash_day, &cash_hour, &cash_min, &cash_sec); \
			val = dbt5_sql_getvalue(&result, 2, length); \
			strncpy(cash_name, val, length); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 5, 1); \
		}else { \
			RETURN_ERROR("trade update frame 1 query 5 fails"); \
		}

#define TRADEUPDATE_F1Q6 sprintf(query, TRADE_UPDATE_Q6, t_id);  \
		GETTIME; \
		if(dbt5_sql_execute(query, &result_t, "TRADE_UPDATE_6")==1 && result_t.result_set){ \
			num_rows = result_t.num_rows; \
			for(int j=0; j<num_rows; j++){ \
				dbt5_sql_fetchrow(&result_t); \
				val = dbt5_sql_getvalue(&result_t, 0, length); \
				sscanf(val, "%d-%d-%d %d:%d:%f",  \
					&trade_history_dts_year[j],  \
					&trade_history_dts_month[j], \
					&trade_history_dts_day[j], \
					&trade_history_dts_hour[j], \
					&trade_history_dts_min[j], \
					&trade_history_dts_sec[j]); \
				val = dbt5_sql_getvalue(&result_t, 1, length); \
				strncpy(trade_history_status_id[j], val, length); \
				ADD_QUERY_NODE(1, 6, 1); \
			} \
			dbt5_sql_close_cursor(&result_t); \
		}else { \
			RETURN_ERROR("trade update frame 1 query 6 fails"); \
		}


#endif	// TRADE_UPDATE_DB_H
