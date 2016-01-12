#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <cstdint>
//#include "simple_bid.h"
#include "simple_neworder.h"
//#include "simple_payment.h"

int main(){
/*
	uint64_t item_id = rand()%65536;
	uint64_t user_id = rand()%128;
	double bid_price = ((double)(rand()%128)/128)*65536;
	execute(item_id, user_id, bid_price);
*/

/*
	TPaymentTxnInput pIn;
	pIn.w_id = rand()%4;
	pIn.d_id = rand()%40;
	pIn.c_id = rand()%65536;
	pIn.c_w_id = rand()%4;
	pIn.c_d_id = rand()%40;
	pIn.h_amount = rand()%128;

	TPaymentTxnOutput pOut;
	execute(&pIn, &pOut);
*/

	TNewOrderTxnInput pIn;
	TNewOrderTxnOutput pOut;
	execute(&pIn, &pOut);
	return 0;

}
