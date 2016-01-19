#ifndef CBID_H
#define CBID_H
#include "CommonStructs.h"
#include "util.h"
#include "BaseInterface.h"
#include <mutex>

class CBID : public CBaseInterface
{
public:
		TMsgDriverBid request;
		fast_random r;
		TBiddingTxnInput biddingInput;
		int total_user_num;
		int total_item_num;

		CBID(char* addr, const int iListenPort, ofstream* pflog, ofstream* pfmix,
			mutex* pLogLock, mutex* pMixLock, int iScaleFactor);


		void GenerateBiddingInput();

		bool Bidding(TBiddingTxnInput* pTxnInput);

		void DoTxn();
};

#endif
