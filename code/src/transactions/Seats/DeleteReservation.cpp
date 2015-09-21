#include "DeleteReservationDB.h"
#include "TxnBaseDB.h"
#include "util.h"
#include "DBConnection.h"
#include <vector>

void CDeleteReservationDB::DoDeleteReservation(TDeleteReservationTxnInput* pIn, TDeleteReservationTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CDeleteReservationDB::execute(const TDeleteReservationTxnInput* pIn, TDeleteReservationTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TDeleteReservationTxnInput* pIn, TDeleteReservationTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;

	char query[4096];
	sql_result_t result;
	int length;
	char* val;

	uint64_t f_id = pIn->f_id;
	string c_id_str = pIn->c_id_str;
	string ff_c_id_str = pIn->ff_c_id_str;
	uint64_t c_id = pIn->c_id;
	uint64_t ff_al_id;
	char c_iattr0[100] = {0};
	int seats_left;
	uint64_t r_id;
	float r_price;

	sprintf(query, DR_GET_FLIGHT, f_id);
	r = dbt5_sql_execute(query, &result, "DR_GET_FLIGHT");
	if(r==1 && result.result_set){
	}else{
		string fail_msg("get flight fails"):
		throw fail_msg.c_str();
	}

	if(c_id != 0){
		sprintf(query, DR_GETCUS_ID, c_id_str.c_str());
		r = dbt5_sql_execute(query, &result, "DR_GETCUST_ID");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			c_id = atol(dbt5_sql_getvalue(&result, 0, length));
		}else{
			string fail_msg("get cust by id fails");
			throw fail_msg.c_str();
		}
	}else{
		sprintf(query, DR_GETCUS_NUMBER, ff_c_id_str.c_str());
		r = dbt5_sql_execute(query, &result, "DR_GETCUS_NUMBER");
		if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
			c_id = atol(dbt5_sql_getvalue(&result, 0, length));
			ff_al_id = atol(dbt5_sql_getvalue(&result, 1, length));
		}else{
			string fail_msg("get cust by number fails");
			throw fail_msg.c_str();
		}
	}

	sprintf(query, DR_GETCUS_RESERVATION, c_id, f_id);
	r = dbt5_sql_execute(query, &result, "DR_GETCUS_RESERVATION");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		strcpy(c_iattr0, dbt5_sql_getvalue(&result, 0, length));
		seats_left = atol(dbt5_sql_getvalue(&result, 7, length));
		r_id = atol(dbt5_sql_getvalue(&result, 8, legnth));
		r_price = atof(dbt5_sql_getvalue(&result, 9, length));
	}else{
		string fail_msg("get customer reservation fails");
		throw fail_msg.c_str();
	}

	sprintf(query, DR_DELETE_RESERVATION, r_id, c_id, f_id);
	r = dbt5_sql_execute(query, &result, "DR_DELETE_RESERVATION");
	if(!r){
		string fail_msg("delete reservation fails");
		throw fail_msg.c_str();
	}

	sprintf(query, DR_UPDATE_FLIGHT, f_id);
	r = dbt5_sql_execute(query, &result, "DR_UPDATE_FLIGHT");
	if(!r){
		string fail_msg("update flight fails");
		throw fail_msg.c_str();
	}

	sprintf(query, DR_UPDATE_CUSTOMER, (-1)*r_price, c_iattr0, c_id);
	r = dbt5_sql_execute(query, &result, "DR_UPDATE_CUSTOMER");
	if(!r){
		string fail_msg("update customer fails");
		throw fail_msg.c_str();
	}

	if(ff_al_id != 0){
		sprintf(query, DR_UPDATE_FF, c_id, ff_al_id);
		r = dbt5_sql_execute(query, &result, "DR_UPDATE_FF");
		if(!r){
			string fail_msg("update frequent flyer fail");
			throw fail_msg.c_str();
		}
	}
}
