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

#endif	// TRADE_ORDER_DB_H
