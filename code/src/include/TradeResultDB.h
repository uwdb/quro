/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * 07 July 2006
 */

#ifndef TRADE_RESULT_DB_H
#define TRADE_RESULT_DB_H

#include "TxnHarnessDBInterface.h"

#include "TxnBaseDB.h"
#include "DBConnection.h"
using namespace TPCE;
#define TRADE_RESULT1_1 \
		"SELECT t_ca_id, t_tt_id, t_s_symb, t_qty, t_chrg,\n" \
		"       CASE WHEN t_lifo = true\n" \
		"            THEN 1\n" \
		"            ELSE 0 END,\n" \
		"       CASE WHEN t_is_cash = true\n" \
		"            THEN 1\n" \
		"            ELSE 0 END\n" \
		"FROM trade\n" \
		"WHERE t_id = %ld"

#define TRADE_RESULT1_2 \
		"SELECT tt_name,\n" \
		"       CASE WHEN tt_is_sell = true\n" \
		"            THEN 1\n" \
		"            ELSE 0 END,\n" \
		"       CASE WHEN tt_is_mrkt = true\n" \
		"            THEN 1\n" \
		"            ELSE 0 END\n" \
		"FROM trade_type\n" \
		"WHERE tt_id = '%s'"

#define TRADE_RESULT1_3 \
		"SELECT hs_qty\n" \
		"FROM holding_summary\n" \
		"WHERE hs_ca_id = %ld\n" \
		"  AND hs_s_symb = '%s'"

#define TRADE_RESULT2_1 \
		"SELECT ca_b_id, ca_c_id, ca_tax_st\n" \
		"FROM customer_account\n" \
		"WHERE ca_id = %ld\n" \
		"FOR UPDATE"

#define TRADE_RESULT2_2a \
		"INSERT INTO holding_summary(hs_ca_id, hs_s_symb, hs_qty)\n" \
		"VALUES(%ld, '%s', %d)"

#define TRADE_RESULT2_2b \
		"UPDATE holding_summary\n" \
		"SET hs_qty = %d\n" \
		"WHERE hs_ca_id = %ld\n " \
		"  AND hs_s_symb = '%s'"

#define TRADE_RESULT2_3a \
		"SELECT h_t_id, h_qty, h_price\n" \
		"FROM holding\n" \
		"WHERE h_ca_id = %ld\n" \
		"  AND h_s_symb = '%s'\n" \
		"ORDER BY h_dts DESC\n" \
		"FOR UPDATE"

#define TRADE_RESULT2_3b \
		"SELECT h_t_id, h_qty, h_price\n" \
		"FROM holding\n" \
		"WHERE h_ca_id = %ld\n" \
		"  AND h_s_symb = '%s'\n" \
		"ORDER BY h_dts ASC\n" \
		"FOR UPDATE"

#define TRADE_RESULT2_4a \
		"INSERT INTO holding_history(hh_h_t_id, hh_t_id, hh_before_qty,\n" \
		"                            hh_after_qty)\n" \
		"VALUES(%ld, %ld, %d, %d)"

#define TRADE_RESULT2_5a \
		"UPDATE holding\n" \
		"SET h_qty = %d\n" \
		"WHERE h_t_id = %ld"

#define TRADE_RESULT2_5b \
		"DELETE FROM holding\n" \
		"WHERE h_t_id = %ld"

#define TRADE_RESULT2_7a \
		"INSERT INTO holding(h_t_id, h_ca_id, h_s_symb, h_dts, h_price,\n" \
		"                    h_qty)\n" \
		"VALUES (%ld, %ld, '%s', '%s', %f, %d)"

#define TRADE_RESULT2_7b \
		"DELETE FROM holding_summary\n" \
		"WHERE hs_ca_id = %ld\n" \
		"  AND hs_s_symb = '%s'"

#define TRADE_RESULT2_8a \
		"INSERT INTO holding_summary(hs_ca_id, hs_s_symb, hs_qty)\n" \
		"VALUES (%ld, '%s', %d)"

#define TRADE_RESULT2_8b \
		"UPDATE holding_summary\n" \
		"SET hs_qty = %d\n" \
		"WHERE hs_ca_id = %ld\n" \
		"  AND hs_s_symb = '%s'"

#define TRADE_RESULT3_1 \
		"SELECT SUM(tx_rate)\n" \
		"FROM taxrate\n" \
		"WHERE tx_id IN (SELECT cx_tx_id\n" \
		"                FROM customer_taxrate\n" \
		"                WHERE cx_c_id = %ld)\n"

#define TRADE_RESULT_HELPER \
		"SELECT now();"

#define TRADE_RESULT3_2 \
		"UPDATE trade\n" \
		"SET t_tax = %f\n" \
		"WHERE t_id = %ld"

#define TRADE_RESULT4_1 \
		"SELECT s_ex_id, s_name\n" \
		"FROM security\n" \
		"WHERE s_symb = '%s'"

#define TRADE_RESULT4_2 \
		"SELECT c_tier\n" \
		"FROM customer\n" \
		"WHERE c_id = %ld"

#define TRADE_RESULT4_3 \
		"SELECT cr_rate\n" \
		"FROM commission_rate\n" \
		"WHERE cr_c_tier = %d\n" \
		"  AND cr_tt_id = '%s'\n" \
		"  AND cr_ex_id = '%s'\n" \
		"  AND cr_from_qty <= %d\n" \
		"  AND cr_to_qty >= %d\n" \
		"LIMIT 1"

#define TRADE_RESULT5_1 \
		"UPDATE trade\n" \
		"SET t_comm = %f,\n" \
		"    t_dts = '%s',\n" \
		"    t_st_id = '%s',\n" \
		"    t_trade_price = %f\n" \
		"WHERE t_id = %ld"

#define TRADE_RESULT5_2 \
		"INSERT INTO trade_history(th_t_id, th_dts, th_st_id)\n" \
		"VALUES (%ld, '%s', '%s')"

#define TRADE_RESULT5_3 \
		"UPDATE broker\n" \
		"SET b_comm_total = b_comm_total + %f,\n" \
		"    b_num_trades = b_num_trades + 1\n" \
		"WHERE b_id = %ld"

#define TRADE_RESULT6_1 \
		"INSERT INTO settlement(se_t_id, se_cash_type, se_cash_due_date,\n " \
		"                       se_amt)\n" \
		"VALUES (%ld, '%s', '%s', %f)"

#define TRADE_RESULT6_2 \
		"UPDATE customer_account\n" \
		"SET ca_bal = ca_bal + %f\n" \
		"WHERE ca_id = %ld"

#define TRADE_RESULT6_3 \
		"INSERT INTO cash_transaction(ct_dts, ct_t_id, ct_amt, ct_name)\n" \
		"VALUES ('%s', %ld, %f, '%s %d shared of %s')"

#define TRADE_RESULT6_4 \
		"SELECT ca_bal\n" \
		"FROM customer_account\n" \
		"WHERE ca_id = %ld"

class CTradeResultDB : public CTxnBaseDB, public CTradeResultDBInterface
{
public:
	CTradeResultDB(CDBConnection *pDBConn) : CTxnBaseDB(pDBConn) {};
	~CTradeResultDB() {};

#ifndef REORDER
	virtual void DoTradeResultFrame1(const TTradeResultFrame1Input *pIn,
			TTradeResultFrame1Output *pOut);
	virtual void DoTradeResultFrame2(const TTradeResultFrame2Input *pIn,
			TTradeResultFrame2Output *pOut);
	virtual void DoTradeResultFrame3(const TTradeResultFrame3Input *pIn,
			TTradeResultFrame3Output *pOut);
	virtual void DoTradeResultFrame4(const TTradeResultFrame4Input *pIn,
			TTradeResultFrame4Output *pOut);
	virtual void DoTradeResultFrame5(const TTradeResultFrame5Input *pIn);
	virtual void DoTradeResultFrame6(const TTradeResultFrame6Input *pIn,
			TTradeResultFrame6Output *pOut);
#else
	virtual void DoTradeResultFrame(PTradeResultTxnInput pIn, PTradeResultTxnOutput pOut);
#endif
	// Function to pass any exception thrown inside
	// database class frame implementation
	// back into the database class
	void Cleanup(void* pException) {};
};
#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#define ADD_FAIL_QUERY_NODE(f, q, commit) \
				add_profile_node(f, q, 0, commit)
#define GETTIME gettimeofday(&t1, NULL);
#else
#define ADD_QUERY_NODE(f, q, commit)
#define ADD_FAIL_QUERY_NODE(f, q, commit)
#define GETTIME
#endif

#define RETURN_ERROR(msg) \
				string fail_msg(msg); \
				fail_msg.append(query); \
				throw fail_msg.c_str();


#define TRADE_RESULT_F1Q1 sprintf(query, TRADE_RESULT1_1, trade_id); \
  GETTIME; \
  r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_1"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			acct_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
			val = dbt5_sql_getvalue(&result, 1, length); \
			strncpy(type_id, val, length); \
			val = dbt5_sql_getvalue(&result, 2, length); \
			strncpy(symbol, val, length); \
			trade_qty = atol(dbt5_sql_getvalue(&result, 3, length)); \
			charge = atof(dbt5_sql_getvalue(&result, 4, length)); \
			is_lifo = atoi(dbt5_sql_getvalue(&result, 5, length)); \
			trade_is_cash = atoi(dbt5_sql_getvalue(&result, 6, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 1, 1); \
	}else{ \
				ADD_FAIL_QUERY_NODE(1, 1, 0); \
				RETURN_ERROR("trade result frame1 query 1 fails..."); \
	} 

#define TRADE_RESULT_F1Q2 sprintf(query, TRADE_RESULT1_2, type_id); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_2"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			strncpy(type_name, val, length); \
			type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length)); \
			type_is_market = atoi(dbt5_sql_getvalue(&result, 2, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 2, 1); \
	}else{ \
			ADD_FAIL_QUERY_NODE(1, 2, 0); \
			FAIL_MSG("trade result frame1 query 2 fails"); \
	} 

#define TRADE_RESULT_F1Q3 sprintf(query, TRADE_RESULT1_3, acct_id, symbol); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT1_3"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			hs_qty = atol(dbt5_sql_getvalue(&result, 0, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(1, 3, 1); \
	}else{ \
			FAIL_MSG("trade result frame1 query 3 fails"); \
	}

#define TRADE_RESULT_F2Q0 sprintf(query, TRADE_RESULT_HELPER); \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT_HELPER"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			dbt5_sql_close_cursor(&result); \
			strncpy(now_dts, val, length); \
	}

#define TRADE_RESULT_F2Q1 sprintf(query, TRADE_RESULT2_1, acct_id); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT2_1"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			broker_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
			cust_id = atol(dbt5_sql_getvalue(&result, 1, length)); \
			tax_status = atoi(dbt5_sql_getvalue(&result, 2, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(2, 1, 1); \
	}else{ \
			FAIL_MSG("trade result frame2 query 1 fails"); \
	}

#define TRADE_RESULT_F2Q2 sprintf(query, TRADE_RESULT2_2a, acct_id, symbol, (-1)*trade_qty); \
					r=0; \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_2a")){ \
							FAIL_MSG("trade result frame2 query 2 fails"); \
					} \
					ADD_QUERY_NODE(2, 2, 1);

#define TRADE_RESULT_F2Q3 sprintf(query, TRADE_RESULT2_2b, hs_qty-trade_qty, acct_id, symbol); \
					r=0; \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_2b")){ \
							ADD_FAIL_QUERY_NODE(2, 3, 0); \
							FAIL_MSG("trade result frame2 query 3 fails"); \
					} \
					ADD_QUERY_NODE(2, 3, 1);

#define TRADE_RESULT_F2Q4 sprintf(query, TRADE_RESULT2_3a, acct_id, symbol); \
							GETTIME;  \
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a"); \
							if(r==1 && result_t.result_set){ \
									num_rows = result_t.num_rows; \
									ADD_QUERY_NODE(2, 4, 1); \
							}else{ \
									ADD_FAIL_QUERY_NODE(2, 4, 0); \
									FAIL_MSG("trade result frame2 query 4 fails"); \
							} 

#define TRADE_RESULT_F2Q5 sprintf(query, TRADE_RESULT2_3b, acct_id, symbol); \
							GETTIME;  \
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3b"); \
							if(r==1 && result_t.result_set){ \
									num_rows = result_t.num_rows; \
									ADD_QUERY_NODE(2, 5, 1); \
							}else{ \
											ADD_FAIL_QUERY_NODE(2, 5, 0); \
									FAIL_MSG("trade result frame2 query 5 fails"); \
							}
#define TRADE_RESULT_F2Q4a sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, before_qty, after_qty); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
											ADD_FAIL_QUERY_NODE(2, 24, 0); \
											FAIL_MSG("trade result frame2 query 24 fails"); \
									} \
									ADD_QUERY_NODE(2, 24, 1);


#define TRADE_RESULT_F2Q6 sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty-needed_qty); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
											ADD_FAIL_QUERY_NODE(2, 6, 0); \
											FAIL_MSG("trade result frame2 query 6 fails"); \
									} \
									ADD_QUERY_NODE(2, 6, 1);

#define TRADE_RESULT_F2Q7 sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a")){ \
											ADD_FAIL_QUERY_NODE(2, 7, 0); \
											FAIL_MSG("trade result frame2 query 7 fails"); \
									} \
									ADD_QUERY_NODE(2, 7, 1);

#define TRADE_RESULT_F2Q8 sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
											ADD_FAIL_QUERY_NODE(2, 8, 0); \
											FAIL_MSG("trade result frame2 query 8 fails"); \
									} \
									ADD_QUERY_NODE(2, 8, 1);

#define TRADE_RESULT_F2Q9	sprintf(query, TRADE_RESULT2_5b, hold_id); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b")){ \
											ADD_FAIL_QUERY_NODE(2, 9, 0); \
											FAIL_MSG("trade result frame2 query 9 fails"); \
									} \
									ADD_QUERY_NODE(2, 9, 1);

#define TRADE_RESULT_F2Q10 sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, (-1)*needed_qty); \
					GETTIME;  \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
							ADD_FAIL_QUERY_NODE(2, 10, 0); \
							FAIL_MSG("trade result frame2 query 10 fails"); \
					} \
					ADD_QUERY_NODE(2, 10, 1);

#define TRADE_RESULT_F2Q11 sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, (-1)*needed_qty); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){ \
							ADD_FAIL_QUERY_NODE(2, 11, 0); \
							FAIL_MSG("trade result frame2 query 11 fails"); \
					} \
					ADD_QUERY_NODE(2, 11, 1);

#define TRADE_RESULT_F2Q12 sprintf(query, TRADE_RESULT2_7b, acct_id, symbol); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b")){ \
							ADD_FAIL_QUERY_NODE(2, 12, 0); \
							FAIL_MSG("trade result frame2 query 12 fails"); \
					} \
					ADD_QUERY_NODE(2, 12, 1);

#define TRADE_RESULT_F2Q13 sprintf(query, TRADE_RESULT2_8a, acct_id, symbol, trade_qty); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_8a")){ \
							ADD_FAIL_QUERY_NODE(2, 13, 0); \
							FAIL_MSG("trade result frame2 query 13 fails"); \
					} \
					ADD_QUERY_NODE(2, 13, 1); 

#define TRADE_RESULT_F2Q14 sprintf(query, TRADE_RESULT2_8b, hs_qty+trade_qty, acct_id, symbol); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_8b")){ \
							ADD_FAIL_QUERY_NODE(2, 14, 0); \
							FAIL_MSG("trade result frame2 query 14 fails"); \
					} \
					ADD_QUERY_NODE(2, 14, 1);

#define TRADE_RESULT_F2Q15 sprintf(query, TRADE_RESULT2_3a, acct_id, symbol); \
							GETTIME;  \
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a"); \
							if(r==1 && result_t.result_set){ \
									num_rows = result_t.num_rows; \
									ADD_QUERY_NODE(2, 15, 1); \
							}else{ \
											ADD_FAIL_QUERY_NODE(2, 15, 0); \
									FAIL_MSG("trade result frame2 query 15 fails"); \
							}

#define TRADE_RESULT_F2Q16 sprintf(query, TRADE_RESULT2_3a, acct_id, symbol); \
							GETTIME;  \
							r = dbt5_sql_execute(query, &result_t, "TRADE_RESULT2_3a"); \
							if(r==1 && result_t.result_set){ \
									num_rows = result_t.num_rows; \
									ADD_QUERY_NODE(2, 16, 1); \
							}else{ \
											ADD_FAIL_QUERY_NODE(2, 16, 0); \
									FAIL_MSG("trade result frame2 query 16 fails"); \
							}

#define TRADE_RESULT_F2Q17 sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, hold_qty+needed_qty); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
											ADD_FAIL_QUERY_NODE(2, 17, 0); \
											FAIL_MSG("trade result frame2 query 17 fails"); \
									} \
									ADD_QUERY_NODE(2, 17, 1);

#define TRADE_RESULT_F2Q18	sprintf(query, TRADE_RESULT2_5a, hold_qty-needed_qty, hold_id); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5a")){ \
											ADD_FAIL_QUERY_NODE(2, 18, 0); \
											FAIL_MSG("trade result frame2 query 18 fails"); \
									} \
									ADD_QUERY_NODE(2, 18, 1);


#define TRADE_RESULT_F2Q19  sprintf(query, TRADE_RESULT2_4a, hold_id, trade_id, hold_qty, 0); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_REUSLT2_4a")){ \
											ADD_FAIL_QUERY_NODE(2, 19, 0); \
											FAIL_MSG("trade result frame2 query 19 fails"); \
									} \
									ADD_QUERY_NODE(2, 19, 1);
									
#define TRADE_RESULT_F2Q20 sprintf(query, TRADE_RESULT2_5b, hold_id); \
									GETTIME; \
									if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_5b")){ \
											ADD_FAIL_QUERY_NODE(2, 20, 0); \
											FAIL_MSG("trade result frame2 query 20 fails"); \
									} \
									ADD_QUERY_NODE(2, 20, 1);

#define TRADE_RESULT_F2Q21 sprintf(query, TRADE_RESULT2_4a, trade_id, trade_id, 0, needed_qty); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_4a")){ \
							ADD_FAIL_QUERY_NODE(2, 21, 0); \
							FAIL_MSG("trade result frame2 query 21 fails"); \
					} \
					ADD_QUERY_NODE(2, 21, 1);

#define TRADE_RESULT_F2Q22 sprintf(query, TRADE_RESULT2_7a, trade_id, acct_id, symbol, now_dts, trade_price, needed_qty); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7a")){ \
							ADD_FAIL_QUERY_NODE(2, 22, 0); \
							FAIL_MSG("trade result frame2 query 22 fails"); \
					} \
					ADD_QUERY_NODE(2, 22, 1);

#define TRADE_RESULT_F2Q23	sprintf(query, TRADE_RESULT2_7b, acct_id, symbol); \
					GETTIME; \
					if(!dbt5_sql_execute(query, &result, "TRADE_RESULT2_7b")){ \
							ADD_FAIL_QUERY_NODE(2, 23, 0); \
							FAIL_MSG("trade result frame2 query 23 fails"); \
					} \
					ADD_QUERY_NODE(2, 23, 1);

#define TRADE_RESULT_F3Q1 sprintf(query, TRADE_RESULT3_1, cust_id); \
			GETTIME; \
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT3_1"); \
			if(r==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					tax_rate = atof(dbt5_sql_getvalue(&result, 0, length)); \
					dbt5_sql_close_cursor(&result); \
					ADD_QUERY_NODE(3, 1, 1); \
			}else{ \
					FAIL_MSG("trade result frame3 query 1 fails"); \
			}

#define TRADE_RESULT_F3Q2 sprintf(query, TRADE_RESULT3_2, tax_rate*(sell_value - buy_value), trade_id); \
			GETTIME; \
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT3_2")){ \
					FAIL_MSG("trade result frame3 query 2 fails"); \
			} \
			ADD_QUERY_NODE(3, 2, 1);

#define TRADE_RESULT_F4Q1 sprintf(query, TRADE_RESULT4_1, symbol); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_1"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			strncpy(sec_ex_id, val, length); \
			val = dbt5_sql_getvalue(&result, 1, length); \
			strncpy(s_name, val, length); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(4, 1, 1); \
	}else{ \
			FAIL_MSG("trade result frame4 query 1 fails"); \
	}

#define TRADE_RESULT_F4Q2 sprintf(query, TRADE_RESULT4_2, cust_id); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_2"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			cust_tier = atoi(dbt5_sql_getvalue(&result, 0, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(4, 2, 1); \
	}else{ \
			FAIL_MSG("trade result frame4 query 2 fails"); \
	}

#define TRADE_RESULT_F4Q3 	sprintf(query, TRADE_RESULT4_3, cust_tier, type_id, sec_ex_id, trade_qty, trade_qty); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "TRADE_RESULT4_3"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length)); \
			dbt5_sql_close_cursor(&result); \
			ADD_QUERY_NODE(4, 3, 1); \
	}else{ \
			FAIL_MSG("trade result frame4 query 3 fails"); \
	}

#define TRADE_RESULT_F5Q1 sprintf(query, TRADE_RESULT5_1, comm_amount, now_dts, st_completed_id, trade_price, trade_id); \
	GETTIME; \
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_1")){ \
			FAIL_MSG("trade result frame5 query 1 fails"); \
	} \
	ADD_QUERY_NODE(5, 1, 1);

#define TRADE_RESULT_F5Q2 sprintf(query, TRADE_RESULT5_2, trade_id, now_dts, st_completed_id); \
	GETTIME; \
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_2")){ \
			FAIL_MSG("trade result frame5 query 2 fails"); \
	} \
	ADD_QUERY_NODE(5, 2, 1);

#define TRADE_RESULT_F5Q3 sprintf(query, TRADE_RESULT5_3, comm_amount, broker_id); \
	GETTIME; \
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT5_3")){ \
			FAIL_MSG("trade result frame5 query 3 fails"); \
	} \
	ADD_QUERY_NODE(5, 3, 1);

#define TRADE_RESULT_F6Q1 	sprintf(query, TRADE_RESULT6_1, trade_id, cash_type, due_dts, se_amount); \
	GETTIME; \
	if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_1")){ \
			FAIL_MSG("trade result frame6 query 1 fails"); \
	} \
	ADD_QUERY_NODE(6, 1, 1);

#define TRADE_RESULT_F6Q2 sprintf(query, TRADE_RESULT6_2, se_amount, acct_id); \
			GETTIME; \
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_2")){ \
					FAIL_MSG("trade result frame6 query 2 fails"); \
			} \
			ADD_QUERY_NODE(6, 2, 1);
 
#define TRADE_RESULT_F6Q3 sprintf(query, TRADE_RESULT6_3, now_dts, trade_id, se_amount, type_name, trade_qty, s_name); \
			GETTIME; \
			if(!dbt5_sql_execute(query, &result, "TRADE_RESULT6_3")){ \
					FAIL_MSG("trade result frame6 query 3 fails"); \
			} \
			ADD_QUERY_NODE(6, 3, 1);

#define TRADE_RESULT_F6Q4 sprintf(query, TRADE_RESULT6_4, acct_id); \
			GETTIME; \
			r = dbt5_sql_execute(query, &result, "TRADE_RESULT6_4"); \
			if(r==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length)); \
					dbt5_sql_close_cursor(&result); \
					ADD_QUERY_NODE(6, 4, 1); \
			}else{ \
					FAIL_MSG("trade result frame6 query4 fails"); \
			}


#endif	// TRADE_RESULT_DB_H
