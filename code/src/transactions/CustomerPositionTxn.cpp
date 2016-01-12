#include <stdlib.h>
#include "DBConnection.h"
#include "CustomerPositionDB.h"

#ifndef DB_PGSQL

void CDBConnection::execute(const TCustomerPositionTxnInput* pIn,
			const TCustomerPostionTxnOutput*){
		char query[1024];
		sql_result_t result;
		sql_resutt_t result_t;
		int num_rows = 0;
		int r;
		int length;
		char* val;
/*
		long unsigned int acct_id_idx = pIn->acct_id_idx;
		long unsigned int cust_id = pIn->cust_id;
		bool get_history = pIn->get_history;
		
		if(cust_id == 0){
				sprintf(query, CUSPOS_Q1, pIn->tax_id);
		}		
		
		sprintf(query, CUSPOS_Q2, cust_id);

		sprintf(query, CUSPOS_Q3, cust_id);
	
		for(int i=0; i<num_rows; i++){
				
		}
*/
}
#endif
