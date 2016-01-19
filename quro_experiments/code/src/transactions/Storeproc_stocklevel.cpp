#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"

void CTPCCDB::DoStocklevel(TStocklevelTxnInput* pIn, TStocklevelTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TStocklevelTxnInput* pIn, TStocklevelTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TStocklevelTxnInput* pIn, TStocklevelTxnOutput* pOut){
	char query[1024];
	
	sprintf(query, "call stock_level(%d, %d, %d, @low_stock)", pIn->w_id, pIn->d_id, pIn->threshold);
	
	sql_result_t result;
	int r = dbt5_sql_execute(query, &result, "STOCK_LEVEL STOREDPROC");
	if(!r){
			string fail_msg("stock level fails");
			throw fail_msg.c_str();
	}
	return ;
}
