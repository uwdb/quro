#include "FindFlightDB.h"
#include "TxnBaseDB.h"

void CFindFlightDB::DoFindFlight(TFindFlightTxnInput* pIn, TFindFlightTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CFindFlightDB::execute(TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;
}

