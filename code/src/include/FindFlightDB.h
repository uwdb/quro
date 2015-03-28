#ifndef FIND_FLIGHT_DB_H
#define FIND_FLIGHT_DB_H

#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CFindFlightDB: public CTxnBaseDB
{
public:
		CFindFlightDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) { };
		~CFindFlightDB() {};
		void DoFindFlight(TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut);

		void execute(const TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut);
};

#define GET_NEARBY_AIRPORT \
"SELECT * \n" \
"  FROM AIRPORT_DISTANCE \n" \
" WHERE D_AP_ID0 = %d\n" \
"   AND D_DISTANCE <= %f\n " \
" ORDER BY D_DISTANCE ASC "

#define GET_AIRPORT_INFO \
"SELECT AP_CODE, AP_NAME, AP_CITY, AP_LONGITUDE, AP_LATITUDE, \n" \
" CO_ID, CO_NAME, CO_CODE_2, CO_CODE_3 \n" \
" FROM AIRPORT, COUNTRY \n" \
" WHERE AP_ID = %d AND AP_CO_ID = CO_ID "

#define GET_FLIGHT \
"SELECT F_ID, F_AL_ID, \n" \
" F_DEPART_AP_ID, F_DEPART_TIME, F_ARRIVE_AP_ID, F_ARRIVE_TIME, \n" \
" AL_NAME, AL_IATTR00, AL_IATTR01 \n" \
" FROM FLIGHT_INFO, AIRLINE \n" \
" WHERE F_DEPART_AP_ID = %d \n" \
" AND F_DEPART_TIME >= %s AND F_DEPART_TIME <= %s \n " \
" AND F_AL_ID = AL_ID \n" \
" AND F_ARRIVE_AP_ID IN %s"


#endif
