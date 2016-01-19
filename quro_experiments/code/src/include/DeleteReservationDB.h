#ifndef DELETE_RESERVATION_DB_H
#define DELETE_RESERVATION_DB_H

#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CDeleteReservationDB: public CTxnBaseDB
{
public:
	CDeleteReservationDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) { };
	~CDeleteReservationDB() {}
	void DoDeleteReservation(TDeleteReservationTxnInput* pIn, TDeleteReservationTxnOutput* pOut);
	void execute(const TDeleteReservationTxnInput* pIn, TDeleteReservationTxnOutput* pOut);
};

#define DR_GET_FLIGHT \
"SELECT F_AL_ID, F_SEATS_LEFT, \n" \
"       F_IATTR00, F_IATTR01,  F_IATTR02, F_IATTR03, \n" \
"       F_IATTR04, F_IATTR05,  F_IATTR06, F_IATTR07 \n" \
"  FROM FLIGHT \n" \
" WHERE F_ID = %d"

#define DR_GETCUS_ID \
"SELECT C_ID \n" \
"  FROM CUSTOMER \n" \
" WHERE C_ID_STR = %s"

#define DR_GETCUS_NUMBER \
"SELECT C_ID, FF_AL_ID \n" \
"  FROM CUSTOMER, FREQUENT_FLYER \n" \
" WHERE FF_C_ID_STR = %s AND FF_C_ID = C_ID"

#define DR_GETCUS_RESERVATION \
"SELECT C_SATTR00, C_SATTR02, C_SATTR04, \n" \
"       C_IATTR00, C_IATTR02, C_IATTR04, C_IATTR06, \n" \
"       F_SEATS_LEFT, \n"  \
"       R_ID, R_SEAT, R_PRICE, R_IATTR00 \n" \
"  FROM CUSTOMER, FLIGHT, RESERVATION \n" \
" WHERE C_ID = %d AND C_ID = R_C_ID \n" \
"   AND F_ID = %d AND F_ID = R_F_ID "

#define DR_DELETE_RESERVATION \
"DELETE FROM RESERVATION \n" \
" WHERE R_ID = %d AND R_C_ID = %d AND R_F_ID = %d"

#define DR_UPDATE_FLIGHT \
"UPDATE FLIGHT \n" \
"   SET F_SEATS_LEFT = F_SEATS_LEFT + 1 \n" \
" WHERE F_ID = %d"

#define DR_UPDATE_CUSTOMER \
"UPDATE CUSTOMER \n" \
"   SET C_BALANCE = C_BALANCE + %f, \n" \
"       C_IATTR00 = %s, \n" \
"       C_IATTR10 = C_IATTR10 - 1, \n" \
"       C_IATTR11 = C_IATTR10 - 1 \n" \
" WHERE C_ID = %d"

#define DR_UPDATE_FF \
"UPDATE FREQUENT_FLYER \n" \
"   SET FF_IATTR10 = FF_IATTR10 - 1 \n" \
" WHERE FF_C_ID = %d \n" \
"   AND FF_AL_ID = %d"
#endif
