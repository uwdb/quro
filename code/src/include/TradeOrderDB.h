/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * 03 July 2006
 */

#ifndef TRADE_ORDER_DB_H
#define TRADE_ORDER_DB_H

#include "TxnHarnessDBInterface.h"

#include "TxnBaseDB.h"
#include "DBConnection.h"
using namespace TPCE;

#define SQLTOF1_1 \
		"SELECT ca_name, ca_b_id, ca_c_id, ca_tax_st\n" \
		"FROM customer_account\n" \
		"WHERE ca_id = %ld"

#define SQLTOF1_2 \
		"SELECT c_f_name, c_l_name, c_tier, c_tax_id\n" \
		"FROM customer\n" \
		"WHERE c_id = %ld"

#define SQLTOF1_3 \
		"SELECT b_name\n" \
		"FROM broker\n" \
		"WHERE b_id = %ld"

#define SQLTOF2_1 \
		"SELECT ap_acl\n" \
		"FROM account_permission\n" \
		"WHERE ap_ca_id = %ld\n" \
		"  AND ap_f_name = '%s'\n" \
		"  AND ap_l_name = '%s'\n" \
		"  AND ap_tax_id = '%s'"

#define SQLTOF3_1a \
		"SELECT co_id\n" \
		"FROM company\n" \
		"WHERE co_name = '%s'"

#define SQLTOF3_2a \
		"SELECT s_ex_id, s_name, s_symb\n" \
		"FROM security\n" \
		"WHERE s_co_id = %ld\n" \
		"  AND s_issue = '%s'"

#define SQLTOF3_1b \
		"SELECT s_co_id, s_ex_id, s_name\n" \
		"FROM security\n" \
		"WHERE s_symb = '%s'\n"

#define SQLTOF3_2b \
		"SELECT co_name\n" \
		"FROM company\n" \
		"WHERE co_id = %ld"

#define SQLTOF3_3 \
		"SELECT lt_price\n" \
		"FROM last_trade\n" \
		"WHERE lt_s_symb = '%s'"

#define SQLTOF3_4 \
		"SELECT tt_is_mrkt, tt_is_sell\n" \
		"FROM trade_type\n" \
		"WHERE tt_id = '%s'"

#define SQLTOF3_5 \
		"SELECT hs_qty\n" \
		"FROM holding_summary\n" \
		"WHERE hs_ca_id = %ld\n" \
		"  AND hs_s_symb = '%s'"

#define SQLTOF3_6a \
		"SELECT h_qty, h_price\n" \
		"FROM holding\n" \
		"WHERE h_ca_id = %ld\n" \
		"  AND h_s_symb = '%s'\n" \
		"ORDER BY h_dts DESC"

#define SQLTOF3_6b \
		"SELECT h_qty, h_price\n" \
		"FROM holding\n" \
		"WHERE h_ca_id = %ld\n" \
		"  AND h_s_symb = '%s'\n" \
		"ORDER BY h_dts ASC"

#define SQLTOF3_7 \
		"SELECT sum(tx_rate)\n" \
		"FROM taxrate\n" \
		"WHERE tx_id in (\n" \
		"                SELECT cx_tx_id\n" \
		"                FROM customer_taxrate\n" \
		"                WHERE cx_c_id = %ld)\n"

#define SQLTOF3_8 \
		"SELECT cr_rate\n" \
		"FROM commission_rate\n" \
		"WHERE cr_c_tier = %d\n" \
		"  AND cr_tt_id = '%s'\n" \
		"  AND cr_ex_id = '%s' \n" \
		"  AND cr_from_qty <= %d\n" \
		"  AND cr_to_qty >= %d"

#define SQLTOF3_9 \
		"SELECT ch_chrg\n" \
		"FROM charge\n" \
		"WHERE ch_c_tier = %d\n" \
		"  AND ch_tt_id = '%s'\n"

#define SQLTOF3_10 \
		"SELECT ca_bal\n" \
		"FROM customer_account\n" \
		"WHERE ca_id = %ld"

#define SQLTOF3_11 \
		"SELECT sum(hs_qty * lt_price)\n" \
		"FROM holding_summary, last_trade\n" \
		"WHERE hs_ca_id = %ld\n" \
		"  AND lt_s_symb = hs_s_symb"

#define SQLTOF4_0 \
		"INSERT INTO seq_trade_id VALUE()"

#define SQLTOF4_00 \
		"SELECT MAX(id) FROM seq_trade_id"

#define SQLTOF4_1 \
		"INSERT INTO trade(t_id, t_dts, t_st_id, t_tt_id, t_is_cash,\n" \
		"                  t_s_symb, t_qty, t_bid_price, t_ca_id,\n" \
		"                  t_exec_name, t_trade_price, t_chrg, t_comm, \n" \
		"                  t_tax, t_lifo)\n" \
		"VALUES (%ld, now(), '%s', '%s', %d, '%s',\n" \
		"        %d, %8.2f, %ld, '%s', NULL, %10.2f, %10.2f, 0, %d)"

#define SQLTOF4_2 \
		"INSERT INTO trade_request(tr_t_id, tr_tt_id, tr_s_symb, tr_qty,\n" \
		"                          tr_bid_price, tr_b_id)\n" \
		"VALUES (%ld, '%s', '%s', %d, %8.2f, %ld)"

#define SQLTOF4_3 \
		"INSERT INTO trade_history(th_t_id, th_dts, th_st_id)\n" \
		"VALUES(%ld, now(), '%s')"

class CTradeOrderDB : public CTxnBaseDB, public CTradeOrderDBInterface
{
public:
	CTradeOrderDB(CDBConnection *pDBConn) : CTxnBaseDB(pDBConn) {};
	~CTradeOrderDB() {};
#ifdef REORDER
	virtual void DoTradeOrderFrame(PTradeOrderTxnInput pIn,
									PTradeOrderIntermediate pInter,
									PTradeOrderTxnOutput pOut);
#else
	virtual void DoTradeOrderFrame1(const TTradeOrderFrame1Input *pIn,
			TTradeOrderFrame1Output *pOut);
	virtual void DoTradeOrderFrame2(const TTradeOrderFrame2Input *pIn,
			TTradeOrderFrame2Output *pOut);
	virtual void DoTradeOrderFrame3(const TTradeOrderFrame3Input *pIn,
			TTradeOrderFrame3Output *pOut);
	virtual void DoTradeOrderFrame4(const TTradeOrderFrame4Input *pIn,
			TTradeOrderFrame4Output *pOut);
	virtual void DoTradeOrderFrame5();
	virtual void DoTradeOrderFrame6();
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
#else
#define ADD_QUERY_NODE(f, q, commit)
#endif

#define RETURN_ERROR(msg) \
				string fail_msg(msg); \
				throw fail_msg.c_str();

#define TRADEORDER_F1Q1  sprintf(query, SQLTOF1_1, acct_id);\
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_1")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			ADD_QUERY_NODE(1, 1, 1); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			strncpy(acct_name, val, length); \
		 	broker_id = atol(dbt5_sql_getvalue(&result, 1, length)); \
			cust_id = atol(dbt5_sql_getvalue(&result, 2, length)); \
			tax_status = atoi(dbt5_sql_getvalue(&result, 3, length)); \
	}else{ \
			RETURN_ERROR("trade order frame1 query 1 fails"); \
	}

#define TRADEORDER_F1Q2 sprintf(query, SQLTOF1_2, cust_id); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_2")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			ADD_QUERY_NODE(1, 2, 1); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			strncpy(cust_f_name, val, length); \
			val = dbt5_sql_getvalue(&result, 1, length); \
			strncpy(cust_l_name, val, length); \
			cust_tier = atoi(dbt5_sql_getvalue(&result, 2, length)); \
			val = dbt5_sql_getvalue(&result, 2, length); \
			strncpy(tax_id, val, length); \
	}else{ \
			RETURN_ERROR("trade order frame1 query 2 fails"); \
	}

#define TRADEORDER_F1Q3 sprintf(query, SQLTOF1_3, broker_id); \
	gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_3")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			strncpy(broker_name, val, length); \
			ADD_QUERY_NODE(1, 3, 1); \
	}else{ \
			RETURN_ERROR("trade order frame1 query 3 fails"); \
	}

#define TRADEORDER_F2Q1  if(strcmp(exec_l_name, cust_l_name) \
            || strcmp(exec_f_name, cust_f_name) \
            || strcmp(exec_tax_id, tax_id)){ \
			sprintf(query, SQLTOF2_1, acct_id, exec_f_name, exec_l_name, exec_tax_id); \
				gettimeofday(&t1, NULL); \
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4")==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					val = dbt5_sql_getvalue(&result, 0, length); \
					strncpy(ap_acl, val, length); \
					ADD_QUERY_NODE(2, 1, 1); \
			}else{ \
					RETURN_ERROR("trade order frame2 query 1 fails"); \
			} \
	}

#define TRADEORDER_F3Q1 sprintf(query, SQLTOF3_1a, co_name); \
			gettimeofday(&t1, NULL); \
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4")==1 && result.result_set){ \
				dbt5_sql_fetchrow(&result); \
				co_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
				ADD_QUERY_NODE(3, 1, 1); \
		}else{ \
				RETURN_ERROR("trade order frame3 query 1 fails"); \
		}

#define TRADEORDER_F3Q2 sprintf(query, SQLTOF3_2a, co_id, issue); \
			gettimeofday(&t1, NULL); \
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_5")==1 && result.result_set){ \
				dbt5_sql_fetchrow(&result); \
				val = dbt5_sql_getvalue(&result, 0, length); \
				strncpy(exch_id, val, 6); \
				val = dbt5_sql_getvalue(&result, 1, length); \
				strncpy(s_name, val, length); \
				val = dbt5_sql_getvalue(&result, 2, length); \
				strncpy(symbol, val, length); \
				ADD_QUERY_NODE(3, 2, 1); \
		}else{ \
				RETURN_ERROR("trade order frame3 query 2 fails"); \
		}

#define TRADEORDER_F3Q3 sprintf(query, SQLTOF3_1b, symbol); \
			gettimeofday(&t1, NULL); \
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_4b")==1 && result.result_set){ \
				dbt5_sql_fetchrow(&result); \
				co_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
				val = dbt5_sql_getvalue(&result, 1, length); \
				strncpy(exch_id, val, 6); \
				val = dbt5_sql_getvalue(&result, 2, length); \
				strncpy(s_name, val, length); \
				ADD_QUERY_NODE(3, 3, 1); \
		}else{ \
				RETURN_ERROR("trade order frame3 query 3 fails"); \
		}

#define TRADEORDER_F3Q4 sprintf(query, SQLTOF3_2b, co_id); \
			gettimeofday(&t1, NULL); \
		if(dbt5_sql_execute(query, &result, "TRADE_ORDER_5b")==1 && result.result_set){ \
				dbt5_sql_fetchrow(&result); \
				val = dbt5_sql_getvalue(&result, 0, length); \
				strncpy(co_name, val, length); \
				ADD_QUERY_NODE(3, 4, 1); \
		}else{ \
				RETURN_ERROR("trade order frame3 query 4 fails"); \
		}

#define TRADEORDER_F3Q5 sprintf(query, SQLTOF3_3, symbol); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_6")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			market_price = atol(dbt5_sql_getvalue(&result, 0, length)); \
			ADD_QUERY_NODE(3, 5, 1); \
	}else{ \
			RETURN_ERROR("trade order frame3 query 5 fails"); \
	}

#define TRADEORDER_F3Q6 sprintf(query, SQLTOF3_4, trade_type_id); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_7")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			type_is_sell = atoi(dbt5_sql_getvalue(&result, 1, length)); \
			type_is_market = atoi(dbt5_sql_getvalue(&result, 0, length)); \
			ADD_QUERY_NODE(3, 6, 1); \
	}else{ \
			RETURN_ERROR("trade order frame3 query 6 fails"); \
	}

#define TRADEORDER_F3Q7 sprintf(query, SQLTOF3_5, acct_id, symbol); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_8")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			hs_qty = atol(dbt5_sql_getvalue(&result, 0, length)); \
			ADD_QUERY_NODE(3, 7, 1); \
	}else{ \
			RETURN_ERROR("trade order frame3 query 7 fails"); \
	}

#define TRADEORDER_F3Q8 sprintf(query, SQLTOF3_6a, acct_id, symbol); \
								gettimeofday(&t1, NULL); \
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_9") && result.result_set){ \
									num_rows = result.num_rows; \
									ADD_QUERY_NODE(3, 8, 1); \
							}else{ \
									RETURN_ERROR("trade order frame3 query 8 fails");  \
							}

#define TRADEORDER_F3Q9 sprintf(query, SQLTOF3_6b, acct_id, symbol); \
								gettimeofday(&t1, NULL); \
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_10") && result.result_set){ \
									num_rows = result.num_rows; \
									ADD_QUERY_NODE(3, 9, 1); \
							}else{ \
									RETURN_ERROR("trade order frame3 query 9 fails");  \
							}

#define TRADEORDER_F3Q10 sprintf(query, SQLTOF3_6a, acct_id, symbol); \
								gettimeofday(&t1, NULL); \
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_9") && result.result_set){ \
									num_rows = result.num_rows; \
									ADD_QUERY_NODE(3, 10, 1); \
							}else{ \
									RETURN_ERROR("trade order frame3 query 10 fails");  \
							}

#define TRADEORDER_F3Q11 sprintf(query, SQLTOF3_6b, acct_id, symbol); \
								gettimeofday(&t1, NULL); \
							if(dbt5_sql_execute(query, &result, "TRADE_ORDER_10") && result.result_set){ \
									num_rows = result.num_rows; \
									ADD_QUERY_NODE(3, 11, 1); \
							}else{ \
									RETURN_ERROR("trade order frame3 query 11 fails");  \
							}

#define TRADEORDER_F3Q12 sprintf(query, SQLTOF3_7, cust_id); \
				gettimeofday(&t1, NULL); \
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_11")==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					tax_rates = atof(dbt5_sql_getvalue(&result, 0, length)); \
					ADD_QUERY_NODE(3, 12, 1); \
			}else{ \
					RETURN_ERROR("trade order frame3 query 12 fails"); \
			}

#define TRADEORDER_F3Q13 sprintf(query, SQLTOF3_8, cust_tier,  trade_type_id, exch_id, trade_qty, trade_qty); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_12")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			comm_rate = atof(dbt5_sql_getvalue(&result, 0, length)); \
			ADD_QUERY_NODE(3, 13, 1); \
	}else{ \
			RETURN_ERROR("trade order frame3 query 13 fails"); \
	}

#define TRADEORDER_F3Q14 sprintf(query, SQLTOF3_9, cust_tier,  trade_type_id); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_13")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			charge_amount = atof(dbt5_sql_getvalue(&result, 0, length)); \
			ADD_QUERY_NODE(3, 14, 1); \
	}else{ \
			RETURN_ERROR("trade order frame3 query 14 fails"); \
	}

#define TRADEORDER_F3Q15  sprintf(query, SQLTOF3_10, acct_id); \
				gettimeofday(&t1, NULL); \
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_14")==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					acct_bal = atof(dbt5_sql_getvalue(&result, 0, length)); \
					ADD_QUERY_NODE(3, 15, 1); \
			}else{ \
					RETURN_ERROR("trade order frame3 query 15 fails"); \
			}

#define TRADEORDER_F3Q16 sprintf(query, SQLTOF3_11, acct_id); \
				gettimeofday(&t1, NULL); \
			if(dbt5_sql_execute(query, &result, "TRADE_ORDER_15")==1 && result.result_set){ \
					dbt5_sql_fetchrow(&result); \
					acct_assets = acct_bal + atof(dbt5_sql_getvalue(&result, 0, length)); \
					ADD_QUERY_NODE(3, 16, 1); \
			}else{ \
					if(result.num_rows==0){ \
							acct_assets = acct_bal; \
					} \
			}

#define TRADEORDER_F4Q1 sprintf(query, SQLTOF4_0); \
		gettimeofday(&t1, NULL); \
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_16")){ \
			string fail_msg("trade order frame 4 query 1 fails"); \
			throw fail_msg.c_str(); \
	} \
	ADD_QUERY_NODE(4, 1, 1);

#define TRADEORDER_F4Q2 sprintf(query, SQLTOF4_00); \
		gettimeofday(&t1, NULL); \
	if(dbt5_sql_execute(query, &result, "TRADE_ORDER_17")==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			val = dbt5_sql_getvalue(&result, 0, length); \
			next_t_id = atol(val); \
			ADD_QUERY_NODE(4, 2, 1); \
	}else{ \
			RETURN_ERROR("trade order frame4 query 2 fails"); \
	}

#define TRADEORDER_F4Q3 sprintf(query, SQLTOF4_1, next_t_id, status_id,  trade_type_id, is_cash, symbol, trade_qty, requested_price, acct_id, exec_name, charge_amount, comm_amount, is_lifo); \
		gettimeofday(&t1, NULL); \
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_18")){ \
			RETURN_ERROR("trade order frame4 query 3 fails"); \
	} \
	ADD_QUERY_NODE(4, 3, 1);

#define TRADEORDER_F4Q4 sprintf(query, SQLTOF4_2, next_t_id, trade_type_id, symbol, trade_qty, requested_price, broker_id); \
			gettimeofday(&t1, NULL); \
			if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_19")){ \
					RETURN_ERROR("trade order frame4 query 4 fails"); \
			} \
			ADD_QUERY_NODE(4, 4, 1);

#define TRADEORDER_F4Q5 sprintf(query, SQLTOF4_3, next_t_id, status_id); \
		gettimeofday(&t1, NULL); \
	if(!dbt5_sql_execute(query, &result, "TRADE_ORDER_20")){ \
			RETURN_ERROR("trade order frame4 query 5 fails"); \
	} \
	ADD_QUERY_NODE(4, 5, 1);


#endif	// TRADE_ORDER_DB_H
