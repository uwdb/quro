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
	char query[1024] = {0};
	int w_id = pIn->w_id;
	int d_id = pIn->d_id;
	int threshold = pIn->threshold;
	sql_result_t result;
	int r;
	int length;
	int d_next_o_id = 0;
	int low_stock = 0;

	TIME_VAR;
	TXN_BEGIN;
#ifndef QURO
	sprintf(query, STOCK_LEVEL_1, w_id, d_id);
	r = dbt5_sql_execute(query, &result, "STOCK_LEVEL_1");

	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		d_next_o_id = atol(dbt5_sql_getvalue(&result, 0, length));
		dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg;
		fail_msg.assign("stock level 1 fail\n");
		throw fail_msg.c_str();
	}
	sprintf(query, STOCK_LEVEL_2, d_id, w_id, threshold, d_next_o_id - 20,
		d_next_o_id - 1);

    r = dbt5_sql_execute(query, &result, "STOCK_LEVEL_2");
		if(r==1 && result.result_set){
     		dbt5_sql_fetchrow(&result);

       	low_stock = atoi(dbt5_sql_getvalue(&result, 0, length)); //LOW_STOCK
        dbt5_sql_close_cursor(&result);
    }else{
        string fail_msg("stock level 2 fail");
				throw fail_msg.c_str();
		}
#else
	sprintf(query, STOCK_LEVEL_1, w_id, d_id);
	r = dbt5_sql_execute(query, &result, "STOCK_LEVEL_1");

	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		d_next_o_id = atol(dbt5_sql_getvalue(&result, 0, length));
		dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg;
		fail_msg.assign("stock level 1 fail\n");
		throw fail_msg.c_str();
	}
	sprintf(query, STOCK_LEVEL_2, d_id, w_id, threshold, d_next_o_id - 20,
		d_next_o_id - 1);

    r = dbt5_sql_execute(query, &result, "STOCK_LEVEL_2");
		if(r==1 && result.result_set){
     		dbt5_sql_fetchrow(&result);

       	low_stock = atoi(dbt5_sql_getvalue(&result, 0, length)); //LOW_STOCK
        dbt5_sql_close_cursor(&result);
    }else{
        string fail_msg("stock level 2 fail");
				throw fail_msg.c_str();
		}

#endif
	pOut->status = CBaseTxnErr::SUCCESS;

	TXN_END(3);
		return ;
}


	
