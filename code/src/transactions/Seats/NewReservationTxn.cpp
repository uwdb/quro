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

#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#define GETTIME gettimeofday(&t1, NULL);
#else
#define ADD_QUERY_NODE(f, q, commit)
#define GETTIME
#endif


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

	int rnd = rand()%65536;

//========ORIG===========

	NR_GETFLIGHT;

	NR_CHECKSEAT;

	NR_CHECKCUSTOMER;

	NR_GETCUSTOMER;

	sprintf(query, NR_UPDATE_FLIGHT, pIn->f_id);
	GETTIME;
	if(!dbt5_sql_execute(query, &result, "UPDATE_FLIGHT")){
			FAIL_MSG("NR_UPDATE_FLIGHT");
	}
	ADD_QUERY_NODE(1, 5, 1);

//	sprintf(query, NR_INSERT_RESERVATION, pIn->r_id, pIn->c_id, pIn->f_id, seatnum, pIn->price, rnd, rnd+10, rnd+20, rnd+30, rnd+40, rnd+50, rnd+60, rnd+70, rnd+80);
//
//	GETTIME;
//	if(!dbt5_sql_execute(query, &result, "INSERT_RESERVATION")){
//			FAIL_MSG("NR_INSERT_RESERVATION fails...");
//	}
//	ADD_QUERY_NODE(1, 8, 1);



	sprintf(query, NR_UPDATE_CUSTOMER, rnd+200, rnd+300, rnd+400, rnd+500, pIn->c_id);
	GETTIME;
	if(!dbt5_sql_execute(query, &result, "UPDATE_CUSTOMER")){
			FAIL_MSG("NR_UPDATE_CUSTOMER fails...");
	}
	ADD_QUERY_NODE(1, 6, 1);

	sprintf(query, NR_UPDATE_FREQUENT_FLYER, rnd+1, rnd+2, rnd+3, rnd+4, pIn->c_id, al_id);
	GETTIME;
	dbt5_sql_execute(query, &result, "UPDATE_FREQUENT_FLYER");
	ADD_QUERY_NODE(1, 7, 1);

	sprintf(query, NR_INSERT_RESERVATION, pIn->r_id, pIn->c_id, pIn->f_id, seatnum, pIn->price, rnd, rnd+10, rnd+20, rnd+30, rnd+40, rnd+50, rnd+60, rnd+70, rnd+80);

	GETTIME;
	if(!dbt5_sql_execute(query, &result, "INSERT_RESERVATION")){
			FAIL_MSG("NR_INSERT_RESERVATION fails...");
	}
	ADD_QUERY_NODE(1, 8, 1);

//===========END ORIGIN=========


/*
//--------REORDER----------
	NR_CHECKCUSTOMER;

	NR_GETCUSTOMER;

	NR_GETFLIGHT;

	NR_CHECKSEAT;


	sprintf(query, NR_UPDATE_FREQUENT_FLYER, rnd+1, rnd+2, rnd+3, rnd+4, pIn->c_id, al_id);
	GETTIME;
	dbt5_sql_execute(query, &result, "UPDATE_FREQUENT_FLYER");
	ADD_QUERY_NODE(1, 7, 1);

	sprintf(query, NR_UPDATE_CUSTOMER, rnd+200, rnd+300, rnd+400, rnd+500, pIn->c_id);
	GETTIME;
	if(!dbt5_sql_execute(query, &result, "UPDATE_CUSTOMER")){
			FAIL_MSG("NR_UPDATE_CUSTOMER fails...");
	}
	ADD_QUERY_NODE(1, 6, 1);

	sprintf(query, NR_UPDATE_FLIGHT, pIn->f_id);
	GETTIME;
	if(!dbt5_sql_execute(query, &result, "UPDATE_FLIGHT")){
			FAIL_MSG("NR_UPDATE_FLIGHT");
	}
	ADD_QUERY_NODE(1, 5, 1);

	sprintf(query, NR_INSERT_RESERVATION, pIn->r_id, pIn->c_id, pIn->f_id, seatnum, pIn->price, rnd, rnd+10, rnd+20, rnd+30, rnd+40, rnd+50, rnd+60, rnd+70, rnd+80);
	GETTIME;
	if(!dbt5_sql_execute(query, &result, "INSERT_RESERVATION")){
			FAIL_MSG("NR_INSERT_RESERVATION fails...");
	}
	ADD_QUERY_NODE(1, 8, 1);
*/

}

