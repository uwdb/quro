#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"

const char s_dist[10][11] = {
        "s_dist_01", "s_dist_02", "s_dist_03", "s_dist_04", "s_dist_05",
        "s_dist_06", "s_dist_07", "s_dist_08", "s_dist_09", "s_dist_10"
};

void CTPCCDB::DoNewOrder(TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput *pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut){
	char stmt[512];
  int r;
	sql_result_t result;
	int rollback;
	int length;

	/* Create the query and execute it. */
	sprintf(stmt,
                 "call new_order(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\
                                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\
                                 %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\
                          %d, %d, @rc)",
		pIn->w_id, pIn->d_id, pIn->c_id, pIn->o_all_local,
		pIn->o_ol_cnt,
		pIn->order_line[0].ol_i_id,
		pIn->order_line[0].ol_supply_w_id,
		pIn->order_line[0].ol_quantity,
		pIn->order_line[1].ol_i_id,
		pIn->order_line[1].ol_supply_w_id,
		pIn->order_line[1].ol_quantity,
		pIn->order_line[2].ol_i_id,
		pIn->order_line[2].ol_supply_w_id,
		pIn->order_line[2].ol_quantity,
		pIn->order_line[3].ol_i_id,
		pIn->order_line[3].ol_supply_w_id,
		pIn->order_line[3].ol_quantity,
		pIn->order_line[4].ol_i_id,
		pIn->order_line[4].ol_supply_w_id,
		pIn->order_line[4].ol_quantity,
		pIn->order_line[5].ol_i_id,
		pIn->order_line[5].ol_supply_w_id,
		pIn->order_line[5].ol_quantity,
		pIn->order_line[6].ol_i_id,
		pIn->order_line[6].ol_supply_w_id,
		pIn->order_line[6].ol_quantity,
		pIn->order_line[7].ol_i_id,
		pIn->order_line[7].ol_supply_w_id,
		pIn->order_line[7].ol_quantity,
		pIn->order_line[8].ol_i_id,
		pIn->order_line[8].ol_supply_w_id,
		pIn->order_line[8].ol_quantity,
		pIn->order_line[9].ol_i_id,
		pIn->order_line[9].ol_supply_w_id,
		pIn->order_line[9].ol_quantity,
		pIn->order_line[10].ol_i_id,
		pIn->order_line[10].ol_supply_w_id,
		pIn->order_line[10].ol_quantity,
		pIn->order_line[11].ol_i_id,
		pIn->order_line[11].ol_supply_w_id,
		pIn->order_line[11].ol_quantity,
		pIn->order_line[12].ol_i_id,
		pIn->order_line[12].ol_supply_w_id,
		pIn->order_line[12].ol_quantity,
		pIn->order_line[13].ol_i_id,
		pIn->order_line[13].ol_supply_w_id,
		pIn->order_line[13].ol_quantity,
		pIn->order_line[14].ol_i_id,
		pIn->order_line[14].ol_supply_w_id,
		pIn->order_line[14].ol_quantity);


		r = dbt5_sql_execute(stmt, &result, "NEW_ORDER_SP");
		if(!r){
				string fail_msg("execution fails");
				throw fail_msg.c_str();
		}

		r = dbt5_sql_execute("select @rc", &result, "NEW_ORDER_SP_RC");
		if(r==1){
			dbt5_sql_fetchrow(&result);
			rollback = atoi(dbt5_sql_getvalue(&result, 0, length));
			if(rollback){
				string fail_msg("rollback");
				throw fail_msg.c_str();
			}
		}else{
			string fail_msg("NEW_ORDER_SP_RC fails");
			throw fail_msg.c_str();
		}

		return ;
}

