#include "CBid.h"
#include "Bid_const.h"

CBID::CBID(char* addr, const int iListenPort, ofstream* pflog,
		ofstream* pfmix, mutex* pLogLock, mutex* pMixLock, int iScaleFactor)
: CBaseInterface(addr, iListenPort, pflog, pfmix, pLogLock, pMixLock)
{
		srand (time(NULL));
		long unsigned int rnd = (rand()*rand())%6124908536;
		r.set_seed(rnd);
		total_user_num = TOTAL_USER_NUM*iScaleFactor;
		total_item_num = TOTAL_ITEM_NUM*iScaleFactor;
}


bool CBID::Bidding(TBiddingTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverBid));

		request.TxnType = BIDDING;
	memcpy(&(request.TxnInput), pTxnInput,
			sizeof(request.TxnInput));

	return talkToSUT(&request);

}

void CBID::GenerateBiddingInput(){
		biddingInput.user_id = rand()%total_user_num;
		biddingInput.item_id = rand()%total_item_num;
		biddingInput.price = double(rand()%128)/128.0 * PRICE_FACTOR;
}


void CBID::DoTxn(){
		//generate TxnTYpe
		GenerateBiddingInput();
		Bidding(&biddingInput);
}
