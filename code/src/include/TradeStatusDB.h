/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * 13 June 2006
 */

#ifndef TRADE_STATUS_DB_H
#define TRADE_STATUS_DB_H

#include "TxnHarnessDBInterface.h"

#include "TxnBaseDB.h"
#include "DBConnection.h"

#define TRADE_STATUS_1 \
		"SELECT t_id, t_dts, st_name, tt_name, t_s_symb, t_qty, \n" \
		"       t_exec_name, t_chrg, s_name, ex_name\n" \
		"FROM trade, status_type, trade_type, security, exchange\n" \
		"WHERE t_ca_id = %ld\n" \
		"  AND st_id = t_st_id\n" \
		"  AND tt_id = t_tt_id\n" \
		"  AND s_symb = t_s_symb\n" \
		"  AND ex_id = s_ex_id\n" \
		"ORDER BY t_dts DESC\n" \
		"LIMIT 50"

#define TRADE_STATUS_2 \
		"SELECT c_l_name, c_f_name, b_name\n" \
		"FROM customer_account, customer, broker\n" \
		"WHERE ca_id = %ld\n" \
		"  AND c_id = ca_c_id\n" \
		"  AND b_id = ca_b_id"



using namespace TPCE;

class CTradeStatusDB : public CTxnBaseDB, public CTradeStatusDBInterface
{
public:
	CTradeStatusDB(CDBConnection *pDBConn) : CTxnBaseDB(pDBConn) {};

	~CTradeStatusDB() {};

	virtual void DoTradeStatusFrame1(const TTradeStatusFrame1Input *pIn,
			TTradeStatusFrame1Output *pOut);

	// Function to pass any exception thrown inside
	// database class frame implementation
	// back into the database class
	void Cleanup(void* pException) {};
};

#endif	// TRADE_STATUS_DB_H
