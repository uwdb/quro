#include "NewReservationDB.h"
#include "TxnBaseDB.h"
#include "util.h"
#include "DBConnection.h"
#include <vector>

void CUpdateReservationDB:DoUpdateReservation(TUpdateReservationTxnInput* pIn, TUpdateReservationTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CUpdateReservation::execute(const TUpdateReservationTxnInput* pIn, TUpdateReservationTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TUpdateReservationTxnInput* pIn, TUpdateReservationTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;


	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;
	uint64_t r_f_id = pIn->flight_id;
	uint64_t r_seat = pIn->seat;
	uint64_t c_id = pIn->customer_id;
	uint64_t r_id;
	string attr_val("random attr");
	TIMESTAMP cur_dts;
	char cur_dts_string[100];
	uint64_t new_r_f_id, new_r_id, new_r_seat;

	sprintf(cur_dts_string, "%hd-%hd-%hd %hd:%hd:%hd.%*d",
			&cur_dts.year,
			&cur_dts.month,
			&cur_dts.day,
			&cur_dts.hour,
			&cur_dts.minute,
			&cur_dts.second);


	sprintf(query, CHECK_SEAT, r_f_id, r_seat);
	r = dbt5_sql_execute(query, &result, "CHECK_SEAT");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);

		r_id = atol(dbt5_sql_getvalue(&result, 0, length));
	}else{
		string fail_msg("check seats failed");
		throw fail_msg.c_str();
	}

	sprintf(query, CHECK_CUSTOMER, r_f_id, c_id);
	r = dbt5_sql_execute(query, &result, "CHECK_CUSTOMER");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);

		new_r_id = atol(dbt5_sql_getvalue(&result, 0, length));

	}else{
		string fail_msg("check customer failed");
		throw fail_msg.c_str();
	}

	sprintf(query, UPDATE_RESERVATION, r_seat, cur_dts_string, attr_val.c_str(), r_id, c_id, f_id);
	r = dbt5_sql_execute(query, &result, "UPDATE_RESERVATION");
	if (!r){
		string fail_msg("update reservation failed");
		throw fail_msg.c_str();
	}


}
