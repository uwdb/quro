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

	// Function to pass any exception thrown inside
	// database class frame implementation
	// back into the database class
	void Cleanup(void* pException) {};
};

#endif	// TRADE_RESULT_DB_H
