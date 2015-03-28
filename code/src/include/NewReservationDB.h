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
"SELECT R_ID  FROM RESERVATION \n"
" WHERE R_F_ID = %d and R_SEAT = %d"

#define NR_CHECK_CUSTOMER \
"SELECT R_ID FROM RESERVATION \n" \
" WHERE R_F_ID = %d AND R_C_ID = %d"

#define NR_UPDATE_FLIGHT \
"UPDATE FLIGHT SET F_SEATS_LEFT = F_SEATS_LEFT - 1 \n" \n 
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
"       FF_IATTR11 = ?, \n" \
"       FF_IATTR12 = ?, \n" \
"       FF_IATTR13 = ?, \n" \
"       FF_IATTR14 = ? \n" \
" WHERE FF_C_ID = ? \n" \
"   AND FF_AL_ID = ?"

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


#endif
