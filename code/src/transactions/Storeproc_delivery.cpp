#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"

void CTPCCDB::DoDelivery(TDeliveryTxnInput *pIn, TDeliveryTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TDeliveryTxnInput* pIn, TDeliveryTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TDeliveryTxnInput* pIn, TDeliveryTxnOutput* pOut){
	char query[1024];
	int d_id;
	int w_id = pIn->w_id;
	int o_carrier_id = pIn->o_carrier_id;
	sql_result_t result;	
	int r;

	char stmt[512];

	/* Create the query and execute it. */
	sprintf(stmt, "call delivery(%d, %d)",
		pIn->w_id, pIn->o_carrier_id);

  r = dbt5_sql_execute(stmt, &result, "DELIVERY_STOREPROC");
	if(!r) 
  {
		string fail_msg("delivery storeproc fails");
		throw fail_msg.c_str();
  }
	return ;
}
