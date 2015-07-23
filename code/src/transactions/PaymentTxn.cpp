#include "TPCC.h"
#include "TPCCDB.h"
#include "TxnBaseDB.h"
#include "TPCC_const.h"
#include "DBConnection.h"
/*
#ifdef TABLE_PROFILE
map<string, FIELDHIST> table_hist;
int profile_cnt = 0;
string tbl_name;
string param_str;
#endif
*/
void CTPCCDB::DoPayment(TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CTPCCDB::execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput *pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut){
	char query_selectcus[1024];
	char query_selectcus2[1024];
	char query_updatecus[1024];
	char query_updatecus2[1024];
	char query_inserthis[1024];
	char query_selectwh[1024];
	char query_updatewh[1024];
	char query_selectdis[1024];
	char query_updatedis[1024];
	sql_result_t result1, result2, result3, result4, result5, result6, result7, result8, result9;
	int length;
	char* val;
	int r_1, r_2, r_3, r_4, r_5, r_6, r_7, r_8, r_9;

	char* w_name;
	char* w_street1;
	char* w_street2;
	char* w_city;
	char* w_state;
	char* w_zip;
	char* d_name;
	char* d_street1;
	char* d_street2;
	char* d_city;
	char* d_state;
	char* d_zip;
	char* c_first;
	char* c_last;
	char* c_credit;
	int c_id;
	char my_c_data[1000];
/*
	sprintf(query_selectwh, PAYMENT_1, pIn->w_id);
//	CLANG_PROFILE(query_selectwh);
	r_1 = dbt5_sql_execute(query_selectwh, &result1, "get_wh");
	if(r_1==1 && result1.result_set){
			dbt5_sql_fetchrow(&result1);
			w_name = dbt5_sql_getvalue(&result1, 0, length);
			w_street1 = dbt5_sql_getvalue(&result1, 1, length);
			w_street2 = dbt5_sql_getvalue(&result1, 2, length);
			w_city = dbt5_sql_getvalue(&result1, 3, length);
			w_state = dbt5_sql_getvalue(&result1, 4, length);
			w_zip = dbt5_sql_getvalue(&result1, 5, length);
			dbt5_sql_close_cursor(&result1);
	}else{
			string fail_msg("get_wh fail");
			throw fail_msg.c_str();
	}

		sprintf(query_updatewh, PAYMENT_2, pIn->h_amount, pIn->w_id);
//		CLANG_PROFILE(query_updatewh);
		r_2 = dbt5_sql_execute(query_updatewh, &result2, "update_wh");
		if(!r_2){
				string fail_msg("update_wh fail");
				throw fail_msg.c_str();
		}


		sprintf(query_selectdis, PAYMENT_3, pIn->d_id, pIn->w_id);
//		CLANG_PROFILE(query_selectdis);
		r_3 = dbt5_sql_execute(query_selectdis, &result3, "get_dis");
		if(r_3==1 && result3.result_set){
				dbt5_sql_fetchrow(&result3);
				d_name = dbt5_sql_getvalue(&result3, 0, length);
				d_street1 = dbt5_sql_getvalue(&result3, 1, length);
				d_street2 = dbt5_sql_getvalue(&result3, 2, length);
				d_city = dbt5_sql_getvalue(&result3, 3, length);
				d_state = dbt5_sql_getvalue(&result3, 4, length);
				d_zip = dbt5_sql_getvalue(&result3, 5, length);
				dbt5_sql_close_cursor(&result3);
		}else{
				string fail_msg("get_dis fail");
				throw fail_msg.c_str();
		}

		sprintf(query_updatedis, PAYMENT_4, pIn->h_amount, pIn->w_id, pIn->d_id);
//		CLANG_PROFILE(query_updatedis);
		r_4 = dbt5_sql_execute(query_updatedis, &result4, "update_dis");
		if(!r_4){
				string fail_msg("update_dis fail");
				throw fail_msg.c_str();
		}

		if(pIn->c_id == 0){
				sprintf(query_selectcus, PAYMENT_5, pIn->w_id, pIn->d_id, pIn->c_last);
//				CLANG_PROFILE(query_selectcus);
				r_5 = dbt5_sql_execute(query_selectcus, &result5, "get_cus");
				if(r_5==1 && result5.result_set){
						dbt5_sql_fetchrow(&result5);
						c_id = atol(dbt5_sql_getvalue(&result5, 0, length));
						dbt5_sql_close_cursor(&result5);
				}else{
						string fail_msg("get_cus fail");
						throw fail_msg.c_str();
				}
		}else{
				c_id = pIn->c_id;
		}

		sprintf(query_selectcus2, PAYMENT_6, pIn->c_w_id, pIn->c_d_id, c_id);
//		CLANG_PROFILE(query_selectcus2);
		r_6 = dbt5_sql_execute(query_selectcus2, &result6, "get_cus2");
		if(r_6==1 && result6.result_set){
				dbt5_sql_fetchrow(&result6);
				c_first = dbt5_sql_getvalue(&result6, 0, length);
				c_last = dbt5_sql_getvalue(&result6, 1, length);
				c_credit = dbt5_sql_getvalue(&result6, 10, length);
		}else{
				string fail_msg("get_cus2 fail");
				throw fail_msg.c_str();
		}

		if(c_credit[0] == 'G'){
				sprintf(query_updatecus, PAYMENT_7_GC, pIn->h_amount, c_id, pIn->c_w_id, pIn->c_d_id);
//				CLANG_PROFILE(query_updatecus);
				r_7 = dbt5_sql_execute(query_updatecus, &result7, "update_cusg");
				if(!r_7){
						string fail_msg("update_cusg fail");
						throw fail_msg.c_str();
				}
		}else{
				sprintf(my_c_data, "%d %d %d %d %d %f", c_id, pIn->c_d_id, pIn->c_w_id, pIn->d_id, pIn->w_id, pIn->h_amount);
				sprintf(query_updatecus2, PAYMENT_7_BC, pIn->h_amount, my_c_data, c_id, pIn->c_w_id, pIn->c_d_id);
//				CLANG_PROFILE(query_updatecus2);
				r_8 = dbt5_sql_execute(query_updatecus2, &result8, "update_cusb");
				if(!r_8){
						string fail_msg("update_cusb fail");
						throw fail_msg.c_str();
				}
		}

		sprintf(query_inserthis, PAYMENT_8, c_id, pIn->c_d_id, pIn->c_w_id, pIn->d_id, pIn->w_id, pIn->h_amount, w_name, d_name);
//		CLANG_PROFILE(query_inserthis);
		r_9 = dbt5_sql_execute(query_inserthis, &result9, "insert_his");
		if(!r_9){
				string fail_msg("insert_his fail");
				throw fail_msg.c_str();
		}
*/

	if(!(pIn->c_id == 0)){
			c_id = pIn->c_id;
	}



	if(pIn->c_id == 0){
				sprintf(query_selectcus, PAYMENT_5, pIn->w_id, pIn->d_id, pIn->c_last);
				r_5 = dbt5_sql_execute(query_selectcus, &result5, "get_cus");
				if(r_5==1 && result5.result_set){
						dbt5_sql_fetchrow(&result5);
						c_id = atol(dbt5_sql_getvalue(&result5, 0, length));
						dbt5_sql_close_cursor(&result5);
				}else{
						string fail_msg("get_cus fail");
						throw fail_msg.c_str();
				}
	}



		sprintf(query_selectcus2, PAYMENT_6, pIn->c_w_id, pIn->c_d_id, c_id);
		r_6 = dbt5_sql_execute(query_selectcus2, &result6, "get_cus2");
		if(r_6==1 && result6.result_set){
				dbt5_sql_fetchrow(&result6);
				c_first = dbt5_sql_getvalue(&result6, 0, length);
				c_last = dbt5_sql_getvalue(&result6, 1, length);
				c_credit = dbt5_sql_getvalue(&result6, 10, length);
		}else{
				string fail_msg("get_cus2 fail");
				throw fail_msg.c_str();
		}




		if(!(c_credit[0] == 'G')){
				sprintf(my_c_data, "%d %d %d %d %d %f", c_id, pIn->c_d_id, pIn->c_w_id, pIn->d_id, pIn->w_id, pIn->h_amount);
		}



		if(c_credit[0] == 'G'){
				sprintf(query_updatecus, PAYMENT_7_GC, pIn->h_amount, c_id, pIn->c_w_id, pIn->c_d_id);
				r_7 = dbt5_sql_execute(query_updatecus, &result7, "update_cusg");
				if(!r_7){
						string fail_msg("update_cusg fail");
						throw fail_msg.c_str();
				}
}



		if(!(c_credit[0] == 'G')){
				sprintf(query_updatecus2, PAYMENT_7_BC, pIn->h_amount, my_c_data, c_id, pIn->c_w_id, pIn->c_d_id);
				r_8 = dbt5_sql_execute(query_updatecus2, &result8, "update_cusb");
				if(!r_8){
						string fail_msg("update_cusb fail");
						throw fail_msg.c_str();
				}
}



		sprintf(query_selectdis, PAYMENT_3, pIn->d_id, pIn->w_id);
		r_3 = dbt5_sql_execute(query_selectdis, &result3, "get_dis");
		if(r_3==1 && result3.result_set){
				dbt5_sql_fetchrow(&result3);
				d_name = dbt5_sql_getvalue(&result3, 0, length);
				d_street1 = dbt5_sql_getvalue(&result3, 1, length);
				d_street2 = dbt5_sql_getvalue(&result3, 2, length);
				d_city = dbt5_sql_getvalue(&result3, 3, length);
				d_state = dbt5_sql_getvalue(&result3, 4, length);
				d_zip = dbt5_sql_getvalue(&result3, 5, length);
				dbt5_sql_close_cursor(&result3);
		}else{
				string fail_msg("get_dis fail");
				throw fail_msg.c_str();
		}



		sprintf(query_updatedis, PAYMENT_4, pIn->h_amount, pIn->w_id, pIn->d_id);
		r_4 = dbt5_sql_execute(query_updatedis, &result4, "update_dis");
		if(!r_4){
				string fail_msg("update_dis fail");
				throw fail_msg.c_str();
		}



	sprintf(query_selectwh, PAYMENT_1, pIn->w_id);
	r_1 = dbt5_sql_execute(query_selectwh, &result1, "get_wh");
	if(r_1==1 && result1.result_set){
			dbt5_sql_fetchrow(&result1);
			w_name = dbt5_sql_getvalue(&result1, 0, length);
			w_street1 = dbt5_sql_getvalue(&result1, 1, length);
			w_street2 = dbt5_sql_getvalue(&result1, 2, length);
			w_city = dbt5_sql_getvalue(&result1, 3, length);
			w_state = dbt5_sql_getvalue(&result1, 4, length);
			w_zip = dbt5_sql_getvalue(&result1, 5, length);
			dbt5_sql_close_cursor(&result1);
	}else{
			string fail_msg("get_wh fail");
			throw fail_msg.c_str();
	}



		sprintf(query_inserthis, PAYMENT_8, c_id, pIn->c_d_id, pIn->c_w_id, pIn->d_id, pIn->w_id, pIn->h_amount, w_name, d_name);
		r_9 = dbt5_sql_execute(query_inserthis, &result9, "insert_his");
		if(!r_9){
				string fail_msg("insert_his fail");
				throw fail_msg.c_str();
		}



		sprintf(query_updatewh, PAYMENT_2, pIn->h_amount, pIn->w_id);
		r_2 = dbt5_sql_execute(query_updatewh, &result2, "update_wh");
		if(!r_2){
				string fail_msg("update_wh fail");
				throw fail_msg.c_str();
		}


		pOut->w_name.assign(w_name);
		pOut->d_name.assign(d_name);
		pOut->c_credit.assign(c_credit);

		pOut->status = CBaseTxnErr::SUCCESS;


	return ;
}
