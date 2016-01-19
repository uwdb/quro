#ifndef CTPCC_H
#define CTPCC_H
#include "CommonStructs.h"
#include "util.h"
#include "BaseInterface.h"
#include <mutex>


class CTPCC: public CBaseInterface
{
public:
		TMsgDriverTPCC request;
		TNewOrderTxnInput noInput;
		TPaymentTxnInput pmtInput;
		TDeliveryTxnInput deliveryInput;
		TOrderstatusTxnInput orderstatusInput;
		TStocklevelTxnInput stocklevelInput;
		fast_random r;
		int table_cardinality_warehouses;

		CTPCC(char* addr, const int iListenPort, ofstream* pflog, ofstream* pfmix,
					mutex* pLogLock, mutex* pMixLock, int _tbl_card_wh);

		void GenerateNewOrderInput();
		void GeneratePaymentInput();
		void GenerateDeliveryInput();
		void GenerateStocklevelInput();
		void GenerateOrderstatusInput();

		bool NewOrder(TNewOrderTxnInput* pTxnInput);
		bool Payment(TPaymentTxnInput* pTxnInput);
		bool Delivery(TDeliveryTxnInput* pTxnInput);
		bool Stocklevel(TStocklevelTxnInput* pTxnInput);
		bool Orderstatus(TOrderstatusTxnInput* pTxnInput);

		void DoTxn();
};

#endif
