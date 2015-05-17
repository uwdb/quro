#ifndef NEW_RESERVATION_DB_H
#define NEW_RESERVATION_DB_H

#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CNewReservationDB: public CTxnBaseDB
{
public:
		CNewReservationDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) { };
		~CNewReservationDB() {};
		void DoNewReservation(TNewReservationTxnInput* pIn, TNewReservationTxnOutput* pOut);

		void execute(const TNewReservationTxnInput* pIn, TNewReservationTxnOutput* pOut);
};

#define NR_GET_FLIGHT \
"SELECT F_AL_ID, F_SEATS_LEFT, F_SEATS_TOTAL \n" \
"  FROM FLIGHT, AIRLINE \n" \
" WHERE F_ID = %d AND F_AL_ID = AL_ID"

#define NR_GET_CUSTOMER \
"SELECT C_BASE_AP_ID, C_BALANCE, C_SATTR00 \n" \
"  FROM CUSTOMER WHERE C_ID = %d"

#define NR_CHECK_SEAT \
"SELECT R_ID  FROM RESERVATION \n" \
" WHERE R_F_ID = %d and R_SEAT = %d"

#define NR_CHECK_CUSTOMER \
"SELECT R_ID FROM RESERVATION \n" \
" WHERE R_F_ID = %d AND R_C_ID = %d"

#define NR_UPDATE_FLIGHT \
"UPDATE FLIGHT SET F_SEATS_LEFT = F_SEATS_LEFT - 1 \n" \
" WHERE F_ID = %d "

#define NR_UPDATE_CUSTOMER \
"UPDATE CUSTOMER \n" \
"   SET C_IATTR10 = C_IATTR10 + 1, \n" \
"   		C_IATTR11 = C_IATTR11 + 1, \n" \
"       C_IATTR12 = %d, \n" \
"       C_IATTR13 = %d, \n" \
"       C_IATTR14 = %d, \n" \
"       C_IATTR15 = %d \n" \
" WHERE C_ID = %d "

#define NR_UPDATE_FREQUENT_FLYER \
"UPDATE FREQUENT_FLYER \n" \
"   SET FF_IATTR10 = FF_IATTR10 + 1, \n" \
"       FF_IATTR11 = %d, \n" \
"       FF_IATTR12 = %d, \n" \
"       FF_IATTR13 = %d, \n" \
"       FF_IATTR14 = %d \n" \
" WHERE FF_C_ID = %d \n" \
"   AND FF_AL_ID = %d"

#define NR_INSERT_RESERVATION \
"INSERT INTO RESERVATION (" \
"   R_ID, " \
"   R_C_ID, " \
"   R_F_ID, " \
"   R_SEAT, " \
"   R_PRICE, " \
"   R_IATTR00, " \
"   R_IATTR01, " \
"   R_IATTR02, " \
"   R_IATTR03, " \
"   R_IATTR04, " \
"   R_IATTR05, " \
"   R_IATTR06, " \
"   R_IATTR07, " \
"   R_IATTR08 " \
") VALUES (%d, %d, %d, %d, %f, %d, %d, %d, %d, %d, %d, %d, %d, %d)"


#define NR_GETFLIGHT sprintf(query, NR_GET_FLIGHT, pIn->f_id); \
	GETTIME; \
	r = dbt5_sql_execute(query, &result, "GET_FLIGHT"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			ADD_QUERY_NODE(1, 1, 1); \
			al_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
			seats_left = atol(dbt5_sql_getvalue(&result, 1, length)); \
			seat_total = atol(dbt5_sql_getvalue(&result, 2, length)); \
					seatnum = seat_total-seats_left+1; \
	}else{ \
			FAIL_MSG("NR GET_FLIGHT fails..."); \
	} \

#define NR_CHECKSEAT sprintf(query, NR_CHECK_SEAT, pIn->f_id, seatnum); \
	GETTIME; \
	r= dbt5_sql_execute(query, &result, "GET_SEAT"); \
	if(r==1 && result.result_set){ \
			return ; \
	} \
	ADD_QUERY_NODE(1, 2, 1); \

#define NR_CHECKCUSTOMER sprintf(query, NR_CHECK_CUSTOMER, pIn->f_id, pIn->c_id); \
	GETTIME; \
	r= dbt5_sql_execute(query, &result, "GET_CUSTOMER"); \
	if(r==1 && result.result_set){ \
			return ; \
	} \
	ADD_QUERY_NODE(1, 3, 1);

#define NR_GETCUSTOMER sprintf(query, NR_GET_CUSTOMER, pIn->c_id); \
	GETTIME; \
	r= dbt5_sql_execute(query, &result, "GET_CUSTOMER"); \
	if(r==1 && result.result_set){ \
			dbt5_sql_fetchrow(&result); \
			c_base_ap_id = atol(dbt5_sql_getvalue(&result, 0, length)); \
			c_balance = atof(dbt5_sql_getvalue(&result, 1, length)); \
			ADD_QUERY_NODE(1, 4, 1); \
	}else{ \
			FAIL_MSG("NR_GET_CUSTOMER fails..."); \
	}


#endif
