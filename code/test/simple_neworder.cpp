#include "simple_neworder.h"
#include "mysql_helper.h"

void execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut){
	char query1[1024];
	char query2[1024];
	char query3[1024];
	char query4[1024];
	char query5[1024];
	char query6[1024];
	char query7[1024];
	char query8[1024];
	char query9[1024];
	char query10[1024];
	char query[256];

	double t_time;
	sql_result_t result;
	sql_result_t result1, result2, result3, result4, result5, result6, result7, result8, result9, result10;
	int length;
	int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;

	int w_id = pIn->w_id;
	int d_id = pIn->d_id;
	int c_id = pIn->c_id;
	int o_all_local = pIn->o_all_local;
	int o_ol_cnt = pIn->o_ol_cnt;
	long d_next_o_id;

	int ol_i_id[15];
	int ol_supply_w_id[15];
	int ol_quantity[15];

	int i1, i;

	float ol_amount[15];
	float order_amount = 0.0;

	double i_price[15];
  string i_name[15];
  string i_data[15];
  long s_quantity[15];
  string my_s_dist[15];
  string s_data[15];
	string force_fail;

	for (i1 = 0; i1 < 15; i1++) {
		ol_i_id[i1] = pIn->order_line[i1].ol_i_id;
		ol_supply_w_id[i1] = pIn->order_line[i1].ol_supply_w_id;
		ol_quantity[i1] = pIn->order_line[i1].ol_quantity;
	}

	sprintf(query1, NEW_ORDER_1, w_id);
	r1 = dbt5_sql_execute(query1, &result1, "get wh");
	if(r1 == 1 && result1.result_set){
			dbt5_sql_fetchrow(&result1);
			pOut->w_tax.assign(dbt5_sql_getvalue(&result1, 0, length));
			dbt5_sql_close_cursor(&result1);
	}else{
			string fail_msg("get wh fail");
			throw fail_msg.c_str();
	}


	sprintf(query2, NEW_ORDER_2, w_id, d_id);
	r2 = dbt5_sql_execute(query2, &result2, "get dis");
	if(r2 == 1 && result2.result_set){
			dbt5_sql_fetchrow(&result2);
			pOut->d_tax.assign(dbt5_sql_getvalue(&result2, 0, length));
			d_next_o_id = atol(dbt5_sql_getvalue(&result2, 1, length));
			pOut->d_next_o_id = d_next_o_id;
			dbt5_sql_close_cursor(&result2);
	}else{
			string fail_msg("get dis fail");
			throw fail_msg.c_str();
	}

	sprintf(query3, NEW_ORDER_3, w_id, d_id);
	r3 = dbt5_sql_execute(query3, &result3, "update dis");
	if(!r3){
			string fail_msg("update dis fail");
			throw fail_msg.c_str();
	}

	sprintf(query4, NEW_ORDER_4, w_id, d_id, c_id);
	r4 = dbt5_sql_execute(query4, &result4, "get cus");
	if(r4 == 1 && result4.result_set){
			dbt5_sql_fetchrow(&result4);
			pOut->c_discount.assign(dbt5_sql_getvalue(&result4, 0, length));
			pOut->c_last.assign(dbt5_sql_getvalue(&result4, 1, length));
			pOut->c_credit.assign(dbt5_sql_getvalue(&result4, 2, length));
			dbt5_sql_close_cursor(&result4);
	}else{
			string fail_msg("get cus fail");
			throw fail_msg.c_str();
	}
	
	sprintf(query5, NEW_ORDER_5, d_next_o_id, w_id, d_id);
	r5 = dbt5_sql_execute(query5, &result5, "insert neworder");
	if(!r5){
			string fail_msg("insert neworder fail");
			throw fail_msg.c_str();
	}

	sprintf(query6, NEW_ORDER_6, d_next_o_id, d_id, w_id, c_id, o_ol_cnt, o_all_local);
	r6 = dbt5_sql_execute(query6, &result6, "insert orders");
	if(!r6){
			string fail_msg("insert orders fail");
			throw fail_msg.c_str();
	}

	for(i=0; i<o_ol_cnt; i++){
			if(ol_i_id[i] != 0){
					sprintf(query7, NEW_ORDER_7, ol_i_id[i]);
//					CLANG_PROFILE(query7);
					r7 = dbt5_sql_execute(query7, &result7, "get item");
					if(r7 == 1 && result7.result_set){
							dbt5_sql_fetchrow(&result7);
							i_price[i] = atol(dbt5_sql_getvalue(&result7, 0, length));
							i_name[i].assign(dbt5_sql_getvalue(&result7, 1, length));
							i_data[i].assign(dbt5_sql_getvalue(&result7, 2, length));
							dbt5_sql_close_cursor(&result7);
					}else{
							string fail_msg("get item");
							throw fail_msg.c_str();
					}
			}else{
					force_fail.assign("item fail");
					throw force_fail.c_str();
			}
			ol_amount[i] = i_price[i] * ol_quantity[i];
			sprintf(query8, NEW_ORDER_8, s_dist[d_id - 1], ol_i_id[i], w_id);
			r8 = dbt5_sql_execute(query8, &result8, "get stock");
			if(r8 == 1 && result8.result_set){
					dbt5_sql_fetchrow(&result8);
					s_quantity[i] = atol(dbt5_sql_getvalue(&result8, 0, length));
					my_s_dist[i].assign(dbt5_sql_getvalue(&result8, 1, length));
					s_data[i].assign(dbt5_sql_getvalue(&result8, 2, length));
					dbt5_sql_close_cursor(&result8);
			}else{
					string fail_msg("get stock fail");
					throw fail_msg.c_str();
			}

			order_amount += ol_amount[i];
			if(s_quantity[i] > ol_quantity[i] + 10){
					sprintf(query9, NEW_ORDER_9, ol_quantity[i], ol_i_id[i], w_id);
			}else{
					sprintf(query9, NEW_ORDER_9, ol_quantity[i]-91, ol_i_id[i], w_id);
			}
			r9 = dbt5_sql_execute(query9, &result9, "update stock");
			if(!r9){
					string fail_msg("update stock fail");
					throw fail_msg.c_str();
			}

			sprintf(query10, NEW_ORDER_10, d_next_o_id, d_id, w_id, i+1, ol_i_id[i],
                  ol_supply_w_id[i], ol_quantity[i], ol_amount[i], my_s_dist[i].c_str() );
			r10 = dbt5_sql_execute(query10, &result10, "insert orderline");
			if(!r10){
					string fail_msg("insert orderline fail");
					throw fail_msg.c_str();
			}
	}

	return ;
}


