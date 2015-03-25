#ifndef FIND_FLIGHT_DB_H
#define FIND_FLIGHT_DB_H

#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CFindFlightDB: public CTxnBaseDB
{
public:
		CFindFlightDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) {};
		~CFindFlightDB() {};
		void DoFindFlight(TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut);

		void execute(TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut);
};
#endif
