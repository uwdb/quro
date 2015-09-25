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
	int length;
	int ol_amount;
	int o_c_id;
	int no_o_id;
/*
		for (d_id = 1; d_id <= 10; d_id++){
			no_o_id = 0;
      sprintf(query, DELIVERY_1, w_id, d_id);
			
      r = dbt5_sql_execute(query, &result, "DELIVERY_1");
			if(r==1 && result.result_set)
      { 
        dbt5_sql_fetchrow(&result);
        no_o_id = atol(dbt5_sql_getvalue(&result, 0, length));  //NO_O_ID
        dbt5_sql_close_cursor(&result);
      }
      else
      { 
        continue;
      }

      if (no_o_id>0)
      {
        sprintf(query, DELIVERY_2, no_o_id, w_id, d_id);

       	r = dbt5_sql_execute(query, &result, "DELIVERY_2");
				if(!r)
        {
          string fail_msg("delivery 2 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_3, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_3");
				if(r==1 && result.result_set)
        { 
          dbt5_sql_fetchrow(&result);
          o_c_id = atol(dbt5_sql_getvalue(&result, 0, length));  //O_C_ID 
          dbt5_sql_close_cursor(&result);
          
          if (!o_c_id)
          {
							string fail_msg("delivery error 2");
							throw fail_msg.c_str();
          }
        }
        else //error
        {
						string fail_msg("delivery 3 fails");
						throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_4, o_carrier_id, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_4");
        if(!r){
          string fail_msg("delivery 4 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_5, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_5");
				if(!r){
          string fail_msg("delivery 5 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_6, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_6");
				if(r==1 && result.result_set)
        { 
          dbt5_sql_fetchrow(&result);
          ol_amount = atol(dbt5_sql_getvalue(&result, 0, length));  //OL_AMOUNT
          dbt5_sql_close_cursor(&result);

          if (!ol_amount)
          {
            string fail_msg("delivery error 3");
						throw fail_msg.c_str();
          }
        }
        else //error
        {
						string fail_msg("delivery 6 fails");
						throw fail_msg.c_str();
        }

        snprintf(query, 250,  DELIVERY_7, ol_amount, o_c_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_7");
        if(!r){
        	string fail_msg("delivery 7 fails");
					throw fail_msg.c_str();
				}
      }
    }
*/

	for (d_id = 1; d_id <= 10; d_id++){
			no_o_id = 0;
      sprintf(query, DELIVERY_1, w_id, d_id);
			
      r = dbt5_sql_execute(query, &result, "DELIVERY_1");
			if(r==1 && result.result_set)
      { 
        dbt5_sql_fetchrow(&result);
        no_o_id = atol(dbt5_sql_getvalue(&result, 0, length));  //NO_O_ID
        dbt5_sql_close_cursor(&result);
      }
      else
      { 
        continue;
      }

      if (no_o_id>0)
      {
        sprintf(query, DELIVERY_2, no_o_id, w_id, d_id);

       	r = dbt5_sql_execute(query, &result, "DELIVERY_2");
				if(!r)
        {
          string fail_msg("delivery 2 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_3, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_3");
				if(r==1 && result.result_set)
        { 
          dbt5_sql_fetchrow(&result);
          o_c_id = atol(dbt5_sql_getvalue(&result, 0, length));  //O_C_ID 
          dbt5_sql_close_cursor(&result);
          
          if (!o_c_id)
          {
							string fail_msg("delivery error 2");
							throw fail_msg.c_str();
          }
        }
        else //error
        {
						string fail_msg("delivery 3 fails");
						throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_4, o_carrier_id, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_4");
        if(!r){
          string fail_msg("delivery 4 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_5, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_5");
				if(!r){
          string fail_msg("delivery 5 fails");
					throw fail_msg.c_str();
        }

        sprintf(query, DELIVERY_6, no_o_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_6");
				if(r==1 && result.result_set)
        { 
          dbt5_sql_fetchrow(&result);
          ol_amount = atol(dbt5_sql_getvalue(&result, 0, length));  //OL_AMOUNT
          dbt5_sql_close_cursor(&result);

          if (!ol_amount)
          {
            string fail_msg("delivery error 3");
						throw fail_msg.c_str();
          }
        }
        else //error
        {
						string fail_msg("delivery 6 fails");
						throw fail_msg.c_str();
        }

        snprintf(query, 250,  DELIVERY_7, ol_amount, o_c_id, w_id, d_id);

        r = dbt5_sql_execute(query, &result, "DELIVERY_7");
        if(!r){
        	string fail_msg("delivery 7 fails");
					throw fail_msg.c_str();
				}
      }
    }


	pOut->status = CBaseTxnErr::SUCCESS;

    return ;
}
