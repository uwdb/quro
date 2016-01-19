#ifndef CSMALLBANK_H
#define CSMALLBANK_H
#include "CommonStructs.h"
#include "util.h"
#include "BaseInterface.h"
#include <mutex>

class CSMALLBANK : public CBaseInterface
{
public:
		TMsgDriverSmallbank request;
		fast_random r;
		SendPaymentTxnInput sendpaymentInput;
		int total_account_num;

		CSMALLBANK(char* addr, const int iListenPort, ofstream* pflog, ofstream* pfmix,
			mutex* pLogLock, mutex* pMixLock, int iScaleFactor);


		void GenerateSendPaymentInput();

		bool SendPayment(TSendPaymentTxnInput* pTxnInput);

		void DoTxn();
};

#endif
