#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"

void CTPCCDB::DoPayment(TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput *pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut){
	char stmt[512];

	/* Create the query and execute it. */
	sprintf(stmt, "call payment(%d, %d, %d, %d, %d, '%s', %f)",
		pIn->w_id, pIn->d_id, pIn->c_id, pIn->c_w_id, pIn->c_d_id,
		pIn->c_last, pIn->h_amount);

	int r;
	sql_result_t result;
	r = dbt5_sql_execute(stmt, &result, "PAYMENT proc");
	if(!r){
		string fail_msg("payment sp fails");
		throw fail_msg.c_str();
	}
	return ;
}
