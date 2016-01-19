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
	int r;
	int length;
	char c_last[200];
	sql_result_t result;
	int c_id = pIn->c_id;
	int c_w_id = pIn->c_w_id;
	int c_d_id = pIn->c_d_id;
	int my_c_id = 0;
	char c_first[200];
	char c_middle[200];
	float c_balance;
	int o_id;
	int o_carrier_id;
	int ol_i_id[15];
	int ol_supply_w_id[15];
	int ol_quantity[15];
	int ol_amount[15];
	int ol_delivery_d[15];

	TIME_VAR;
	TXN_BEGIN;

	if (c_id == 0) 
        {
          sprintf(query, ORDER_STATUS_1, c_w_id, c_d_id, pIn->c_last);

          r = dbt5_sql_execute(query, &result, "ORDER_STATUS_1");
					if(r==1 && result.result_set)
          {
            if (result.num_rows > 1 )
            {
              int skip_rows=result.num_rows/2;
              while (skip_rows && dbt5_sql_fetchrow(&result))
              {
                skip_rows--;
              }   
            }
            else
            {
              dbt5_sql_fetchrow(&result);
            }
            my_c_id = atol(dbt5_sql_getvalue(&result, 0, length)); //TMP_C_ID
            dbt5_sql_close_cursor(&result);

          }
          else //error
          {
            	string fail_msg("order status 1 fail\n");
							throw fail_msg.c_str();
          }
        } 
        else{
          my_c_id = c_id;
				}
/*
				sprintf(query, ORDER_STATUS_2, c_w_id, c_d_id, my_c_id);

        r = dbt5_sql_execute(query, &result, "ORDER_STATUS_2");
				if(r==1 && result.result_set)
        {
          dbt5_sql_fetchrow(&result);
          
          strcpy(c_first, dbt5_sql_getvalue(&result, 0, length)); //C_FIRST C_MIDDLE MY_C_BALANCE C_BALANCE
          strcpy(c_middle, dbt5_sql_getvalue(&result, 1, length));
          
					strcpy(c_last, dbt5_sql_getvalue(&result, 2, length));
          
					c_balance = atof(dbt5_sql_getvalue(&result, 3, length));

          dbt5_sql_close_cursor(&result);
        }
        else //error
        {
					string fail_msg("order status 2 fail");
					throw fail_msg.c_str();
        }
*/
	sprintf(query, ORDER_STATUS_3, c_w_id, c_d_id, my_c_id);

       	r = dbt5_sql_execute(query, &result, "ORDER_STATUS_3");
				if(r==1 && result.result_set)
        {
          dbt5_sql_fetchrow(&result);

          o_id = atol(dbt5_sql_getvalue(&result, 0, length)); //O_ID O_CARRIER_ID O_ENTRY_D O_OL_CNT

          //o_carrier_id = atol(dbt5_sql_getvalue(&result, 1, length));

          dbt5_sql_close_cursor(&result);
        }
        else //error
        {
					string fail_msg("order status 3 fail");
					throw fail_msg.c_str();
        }

	sprintf(query, ORDER_STATUS_4, c_w_id, c_d_id, o_id);

	r = dbt5_sql_execute(query, &result, "ORDER_STATUS_4");
				if(r==1  && result.result_set)
        {
          int i= 0;
          while (i<15 && dbt5_sql_fetchrow(&result) && result.result_set)
          { 
            ol_i_id[i]= atol(dbt5_sql_getvalue(&result, 0, length));
            ol_supply_w_id[i]= atol(dbt5_sql_getvalue(&result, 1, length));
            ol_quantity[i]= atol(dbt5_sql_getvalue(&result, 2, length));
            ol_amount[i]= atol(dbt5_sql_getvalue(&result, 3, length));
            ol_delivery_d[i]= atol(dbt5_sql_getvalue(&result, 4, length));
            i++;
          }
          dbt5_sql_close_cursor(&result);
        }
        else //error
        {
					string fail_msg("order status 4 fail");
					throw fail_msg.c_str();
        }

		sprintf(query, ORDER_STATUS_2, c_w_id, c_d_id, my_c_id);

        r = dbt5_sql_execute(query, &result, "ORDER_STATUS_2");
				if(r==1 && result.result_set)
        {
          dbt5_sql_fetchrow(&result);
          
          strcpy(c_first, dbt5_sql_getvalue(&result, 0, length)); //C_FIRST C_MIDDLE MY_C_BALANCE C_BALANCE
          strcpy(c_middle, dbt5_sql_getvalue(&result, 1, length));
          
					strcpy(c_last, dbt5_sql_getvalue(&result, 2, length));
          
					c_balance = atof(dbt5_sql_getvalue(&result, 3, length));

          dbt5_sql_close_cursor(&result);
        }
        else //error
        {
					string fail_msg("order status 2 fail");
					throw fail_msg.c_str();
        }

	TXN_END(2);
		return ;
}

