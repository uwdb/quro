#include "CSmallbank.h"
#include "Smallbank_const.h"

CSMALLBANK::CSMALLBANK(char* addr, const int iListenPort, ofstream* pflog,
		ofstream* pfmix, mutex* pLogLock, mutex* pMixLock, int iScaleFactor)
: CBaseInterface(addr, iListenPort, pflog, pfmix, pLogLock, pMixLock)
{
		srand (time(NULL));
		long unsigned int rnd = (rand()*rand())%6124908536;
		r.set_seed(rnd);
		total_account_num = TOTAL_ACCOUNT_NUM*iScaleFactor;
}


bool CSMALLBANK::SendPayment(TSendPaymentTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverSmallbank));

		request.TxnType = SANDPAYMENT;
	memcpy(&(request.TxnInput), pTxnInput,
			sizeof(request.TxnInput));

	return talkToSUT(&request);

}

void CSMALLBANK::GenerateSendPaymentInput(){
}


void CSMALLBANK::DoTxn(){
		double rnd = r.next_uniform();
		if(rnd < SEND_PAYMENT_PERC){
					GenerateSendPaymentInput();
					SendPayment(&sendpaymentInput);
		}
		else if(rnd < SEND_PAYMENT_PERC
									+ AMALGAMATE_PERC){
		}

}
