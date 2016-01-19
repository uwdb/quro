#include "CTPCC.h"
#include "TPCC_const.h"

const char *c_last_syl[C_LAST_SYL_MAX] =
{
	"BAR", "OUGHT", "ABLE", "PRI", "PRES", "ESE", "ANTI", "CALLY", "ATION",
	"EING"
};

int get_c_last(char *c_last, int i)
{
	char tmp[4];

	c_last[0] = '\0';

	if (i < 0 || i > 999)
	{
		return 0;
	}

	/* Ensure the number is padded with leading 0's if it's less than 100. */
	sprintf(tmp, "%03d", i);

	strcat(c_last, c_last_syl[tmp[0] - '0']);
	strcat(c_last, c_last_syl[tmp[1] - '0']);
	strcat(c_last, c_last_syl[tmp[2] - '0']);
	return 1;
}

CTPCC::CTPCC(char* addr, const int iListenPort, ofstream* pflog,
		ofstream* pfmix, mutex* pLogLock, mutex* pMixLock, int _tbl_card_wh)
: CBaseInterface(addr, iListenPort, pflog, pfmix, pLogLock, pMixLock)
{
		srand (time(NULL));
		long unsigned int rnd = (rand()*rand())%6124908536;
		r.set_seed(rnd);
		table_cardinality_warehouses = _tbl_card_wh;
}


bool CTPCC::NewOrder(TNewOrderTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverTPCC));

		request.TxnType = NEWORDER;
	memcpy(&(request.TxnInput.neworderTxnInput), pTxnInput,
			sizeof(request.TxnInput.neworderTxnInput));

	return talkToSUT(&request);

}


bool CTPCC::Payment(TPaymentTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverTPCC));

		request.TxnType = PAYMENT;
	memcpy(&(request.TxnInput.paymentTxnInput), pTxnInput,
			sizeof(request.TxnInput.paymentTxnInput));

	return talkToSUT(&request);

}

bool CTPCC::Delivery(TDeliveryTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverTPCC));

		request.TxnType = DELIVERY;
	memcpy(&(request.TxnInput.deliveryTxnInput), pTxnInput,
			sizeof(request.TxnInput.deliveryTxnInput));

	return talkToSUT(&request);

}

bool CTPCC::Stocklevel(TStocklevelTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverTPCC));

		request.TxnType = STOCKLEVEL;
	memcpy(&(request.TxnInput.stocklevelTxnInput), pTxnInput,
			sizeof(request.TxnInput.stocklevelTxnInput));

	return talkToSUT(&request);

}

bool CTPCC::Orderstatus(TOrderstatusTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverTPCC));

		request.TxnType = ORDERSTATUS;
	memcpy(&(request.TxnInput.orderstatusTxnInput), pTxnInput,
			sizeof(request.TxnInput.orderstatusTxnInput));

	return talkToSUT(&request);

}


void CTPCC::GenerateNewOrderInput(){
	int w_id = rand()%table_cardinality_warehouses + 1;
	int i;
	noInput.w_id = w_id;
	noInput.d_id = get_random(r, D_ID_MAX) + 1;
	noInput.c_id = get_nurand(r, 1023, 1, 3000);
	noInput.o_ol_cnt = get_random(r, 10) + 6;
	for (i = 0; i < noInput.o_ol_cnt; i++) {
		noInput.order_line[i].ol_i_id = /*get_nurand(8191, 1, 100000)*/get_random(r, 100000);
		if (table_cardinality_warehouses > 1) {
			if (get_random(r, 100) > 0) {
				noInput.order_line[i].ol_supply_w_id = w_id;
			} else {
				noInput.order_line[i].ol_supply_w_id =
					get_random(
					r, table_cardinality_warehouses) + 1;
				if (noInput.order_line[i].ol_supply_w_id >=
					w_id) {
					++(noInput.order_line[i].ol_supply_w_id);
				}
			}
		} else {
			noInput.order_line[i].ol_supply_w_id = 1;
		}
		noInput.order_line[i].ol_quantity = get_random(r, 10) + 1;
	}

	/* Use an invalid i_id 1% of the time. */
	if (get_random(r, 100) == 0) {
		noInput.order_line[noInput.o_ol_cnt - 1].ol_i_id = 0;
	}

}
void CTPCC::GenerateDeliveryInput(){
	deliveryInput.w_id = rand()%table_cardinality_warehouses + 1;
	deliveryInput.o_carrier_id = get_random(r, O_CARRIER_ID_MAX) + 1;
}

void CTPCC::GenerateStocklevelInput(){
	stocklevelInput.w_id = rand()%table_cardinality_warehouses + 1;
	stocklevelInput.d_id = get_random(r, D_ID_MAX) + 1;
	stocklevelInput.threshold = get_random(r, 11) + 10;
}

void CTPCC::GenerateOrderstatusInput(){
	orderstatusInput.c_w_id = rand()%table_cardinality_warehouses + 1;
	orderstatusInput.c_d_id = get_random(r, D_ID_MAX) + 1;
	if(get_random(r, 100)< 60){
		orderstatusInput.c_id = C_ID_UNKNOWN;
		get_c_last(orderstatusInput.c_last, get_nurand(r, 255, 0, 999));
	}else{
		orderstatusInput.c_id = get_nurand(r, 1023, 1, 3000);
	}
}

void CTPCC::GeneratePaymentInput(){
	int w_id = rand()%table_cardinality_warehouses + 1;
	pmtInput.w_id = w_id;
	pmtInput.d_id = get_random(r, D_ID_MAX) + 1;

	/* Select a customer by last name 60%, byt c_id 40% of the time. */
	if (get_random(r, 100) < 60) {
		pmtInput.c_id = C_ID_UNKNOWN;
		get_c_last(pmtInput.c_last, get_nurand(r, 255, 0, 999));
	} else {
		pmtInput.c_id = get_nurand(r, 1023, 1, 3000);
	}

	if (get_random(r, 100) < 85) {
		pmtInput.c_w_id = w_id;
		pmtInput.c_d_id = pmtInput.d_id;
	} else {
		pmtInput.c_d_id = get_random(r, D_ID_MAX) + 1;
		if (table_cardinality_warehouses > 1) {
			/*
			 * Select a random warehouse that is not the same
			 * as this user's home warehouse by shifting the
			 * numbers slightly.
			 */
			pmtInput.c_w_id =
				get_random(r, table_cardinality_warehouses - 1)
					+ 1;
			if (pmtInput.c_w_id >= w_id) {
				pmtInput.c_w_id = (pmtInput.c_w_id + 1) %
						table_cardinality_warehouses;
			}
			if (!pmtInput.c_w_id)
			{
			  pmtInput.c_w_id = 1;
			}
		} else {
			pmtInput.c_w_id = 1;
		}
	}
	pmtInput.h_amount = (double) (get_random(r, 500000) + 100) / 100.0;

}


void CTPCC::DoTxn(){
		//generate TxnTYpe
		double rnd = r.next_uniform();
		if(rnd < NEW_ORDER_PERC){
					GenerateNewOrderInput();
					NewOrder(&noInput);
		}
		else if(rnd < PAYMENT_PERC
									+ NEW_ORDER_PERC){
					GeneratePaymentInput();
					Payment(&pmtInput);
		}

		else if(rnd < PAYMENT_PERC
									+ NEW_ORDER_PERC
									+ DELIVERY_PERC){
					GenerateDeliveryInput();
					Delivery(&deliveryInput);
		}
		else if(rnd < PAYMENT_PERC
									+ NEW_ORDER_PERC
									+ DELIVERY_PERC
									+ STOCKLEVEL_PERC){
					GenerateStocklevelInput();
					Stocklevel(&stocklevelInput);
		}
		else if(rnd < PAYMENT_PERC
									+ NEW_ORDER_PERC
									+ DELIVERY_PERC
									+ STOCKLEVEL_PERC
									+ ORDERSTATUS_PERC){
					GenerateOrderstatusInput();
					Orderstatus(&orderstatusInput);
		}

}
