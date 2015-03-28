#include "NewReservationDB.h"
#include "TxnBaseDB.h"
#include "util.h"
#include "DBConnection.h"
#include <vector>

#define FAIL_MSG(msg) \
				string fail_msg(msg); \
				fail_msg.append("\n"); \
				fail_msg.append(query); \
				if (r==2) fail_msg.append("\tempty result"); \
				throw fail_msg.c_str();


void CNewReservationDB::DoNewReservation(TNewReservationTxnInput* pIn, TNewReservationTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}
void CNewReservationDB::execute(const TNewReservationTxnInput* pIn, TNewReservationTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}
void CDBConnection::execute(const TNewReservationTxnInput* pIn, TNewReservationTxnOutput* pOut){

	pOut->status = CBaseTxnErr::SUCCESS;

	char query[4096];
	sql_result_t result;
	sql_result_t result_t;
	int length;
	char* val;
	int r = 0;
	
	int seatnum = pIn->seatnum;
	int seat_total = 0;
	int al_id;
	int seats_left; 
	int c_base_ap_id;
	float c_balance;

	sprintf(query, NR_GET_FLIGHT, pIn->f_id);
	r = dbt5_sql_execute(query, &result, "GET_FLIGHT");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);
	
			al_id = atol(dbt5_sql_getvalue(&result, 0, length));
			seats_left = atol(dbt5_sql_getvalue(&reuslt, 1, length));
			seat_total = atol(dbt5_sql_getvalue(&result, 2, length));

			if(seatnum > seat_total){
					seatnum = 50 + rand()%(seat_total-50);
			}
	}else{
			FAIL_MSG("NR GET_FLIGHT fails...");
	}


	sprintf(query, NR_CHECK_SEAT, pIn->f_id, seatnum);
	r= dbt5_sql_execute(query, &result, "GET_CUSTOMER");
	if(r==1 && result.result_set){
			//seat occupied
			//return ;
			FAIL_MSG("SEAT occupied");
	}
	
	sprintf(query, CHECKCUSTOMER, pIn->f_id, pIn->c_id);
	r= dbt5_sql_execute(query, &result, "GET_CUSTOMER");
	if(r==1 && result.result_set){
			//customer already have a seat
			//return ;
			FAIL_MSG("customer already have a seat");
	}
	
	sprintf(query, NR_GET_CUSTOMER, pIn->c_id);
	r= dbt5_sql_execute(query, &result, "GET_CUSTOMER");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			c_base_ap_id = atol(dbt5_sql_getvalue(&result, 0, length));
			c_balance = atof(dbt5_sql_getvalue(&result, 1, length));
	}else{
			FAIL_MSG("NR_GET_CUSTOMER fails...");
	}

	int rnd = rand()%65536;
	sprintf(query, NR_INSERT_RESERVATION, pIn->r_id, pIn->c_id, pIn->f_id, seatnum, pIn->price, rnd, rnd+10, rnd+20, rnd+30, rnd+40, rnd+50, rnd+60, rnd+70, rnd+80);

	if(!dbt5_sql_execute(query, &result, "INSERT_RESERVATION")){
			FAIL_MSG("NR_INSERT_RESERVATION fails...");
	}

	sprintf(query, NR_UPDATE_FREQUENT_FLYER, rnd+1, rnd+2, rnd+3, rnd+4, pIn->c_id, al_id);
	dbt5_sql_execute(query, &result, "UPDATE_FREQUENT_FLYER");

	sprintf(query, NR_UPDATE_CUSTOMER, rnd+200, rnd+300, rnd+400, rnd+500, pIn->c_id);
	if(!dbt5_sql_execute(query, &result, "UPDATE_CUSTOMER")){
			FAIL_MSG("NR_UPDATE_CUSTOMER fails...");
	}

	sprintf(query, NR_UPDATE_FLIGHT, pIn->f_id);
	if(!dbt5_sql_execute(query, &result, "UPDATE_FLIGHT")){
			FAIL_MSG("NR_UPDATE_FLIGHT");
	}

}

