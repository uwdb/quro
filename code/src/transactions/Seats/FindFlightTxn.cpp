#include "FindFlightDB.h"
#include "TxnBaseDB.h"

void CFindFlightDB::DoFindFlight(TFindFlightTxnInput* pIn, TFindFlightTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CFindFlightDB::execute(TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;
	pDB->outfile<<"get input: "<<pIn->depart_aid<<", "<<pIn->arrive_aid<<","<<pIn->distance<<endl;
}

