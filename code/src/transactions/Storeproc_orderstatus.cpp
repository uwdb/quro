#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"

void CTPCCDB::DoOrderstatus(TOrderstatusTxnInput *pIn, TOrderstatusTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TOrderstatusTxnInput* pIn, TOrderstatusTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TOrderstatusTxnInput* pIn, TOrderstatusTxnOutput* pOut){
	char query[1024];
	
	sprintf(query, "call order_status(%d, %d, %d, '%s')", pIn->c_id, pIn->c_w_id, pIn->c_d_id, pIn->c_last);
	
	sql_result_t result;
	int r = dbt5_sql_execute(query, &result, "ORDER_STATUS_STOREDPROC");
		if(!r){
				string fail_msg("orderstatus fails");
				throw fail_msg.c_str();
		} 
		return ;
}
