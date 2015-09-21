#ifndef UPDATE_RESERVATION_DB_H
#define UPDATE_RESERVATION_DB_H

#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CUpdateReservationDB: public CTxnBaseDB
{
public:
	CUpdateReservationDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) { };
	~CUpdateReservationDB() {};

	void DoUpdateReservation(TUpdateReservationTxnInput* pIn, TUpdateReservationTxnOutput* pOut);

	void execute(const TUpdateReservationTxnInput* pIn, TUpdateReservationTxnOutput* pOut);
};

#define CHECK_SEAT \
"SELECT R_ID \n" \
"  FROM RESERVATION \n" \
" WHERE R_F_ID = %d AND R_SEAT = %d"

#define CHECK_CUSTOMER \
"SELECT R_ID \n" \
"  FROM RESERVATION \n" \
" WHERE R_F_ID = %d AND R_C_ID = %d"

#define UPDATE_RESERVATION \
"UPDATE RESERVATION " \
"SET R_SEAT = %d, R_UPDATED = %s, R_IATTR00 = %s \n" \
" WHERE R_ID = %d AND R_C_ID = %d AND R_F_ID = %d"

#endif
