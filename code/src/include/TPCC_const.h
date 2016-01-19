//Standard TPC-C benchmark, mix of transaction setting
//#define NEW_ORDER_PERC 0.45
//#define PAYMENT_PERC 0.43
//#define DELIVERY_PERC 0.04
//#define STOCKLEVEL_PERC 0.04
//#define ORDERSTATUS_PERC 0.04

//Or a mix of only new order and payment txn
#define NEW_ORDER_PERC 0.5
#define PAYMENT_PERC 0.5
#define DELIVERY_PERC 0
#define STOCKLEVEL_PERC 0
#define ORDERSTATUS_PERC 0

//To use the original implementation, comment out the following line
#define QURO

#define D_ID_MAX 10
#define C_ID_UNKNOWN 0
#define O_CARRIER_ID_MAX 10

#define C_LAST_LEN 12
#define C_LAST_SYL_MAX 10
