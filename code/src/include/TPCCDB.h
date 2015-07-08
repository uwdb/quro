#ifndef TPCC_DB_H
#define TPCC_DB_H
#include "DBConnection.h"
#include "TxnBaseDB.h"
#include "CommonStructs.h"

class CTPCCDB: public CTxnBaseDB
{
public:
		CTPCCDB(CDBConnection *pDBConn): CTxnBaseDB(pDBConn) {};
		void DoNewOrder(TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut);
		void DoPayment(TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut);
		void execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut);
		void execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut);
};

#define PAYMENT_1 \
	"SELECT w_name, w_street_1, w_street_2, w_city, w_state, w_zip\n" \
	"FROM warehouse\n" \
	"WHERE w_id = %d"

#define PAYMENT_2 \
	"UPDATE warehouse\n" \
	"SET w_ytd = w_ytd + %f\n" \
	"WHERE w_id = %d"

#define PAYMENT_3 \
	"SELECT d_name, d_street_1, d_street_2, d_city, d_state, d_zip\n" \
	"FROM district\n" \
	"WHERE d_id = %d\n" \
	"  AND d_w_id = %d"

#define PAYMENT_4 \
	"UPDATE district\n" \
	"SET d_ytd = d_ytd + %f\n" \
	"WHERE d_id = %d\n" \
	"  AND d_w_id = %d"

#define PAYMENT_5 \
	"SELECT c_id\n" \
	"FROM customer\n" \
	"WHERE c_w_id = %d\n" \
	"  AND c_d_id = %d\n" \
	"  AND c_last = '%s'\n" \
	"ORDER BY c_first ASC"

#define PAYMENT_6 \
	"SELECT c_first, c_middle, c_last, c_street_1, c_street_2, c_city,\n" \
	"       c_state, c_zip, c_phone, c_since, c_credit,\n" \
	"       c_credit_lim, c_discount, c_balance, c_data, c_ytd_payment\n" \
	"FROM customer\n" \
	"WHERE c_w_id = %d\n" \
	"  AND c_d_id = %d\n" \
	"  AND c_id = %d"

#define PAYMENT_7_GC \
	"UPDATE customer\n" \
	"SET c_balance = c_balance - %f,\n" \
	"    c_ytd_payment = c_ytd_payment + 1\n" \
	"WHERE c_id = %d\n" \
	"  AND c_w_id = %d\n" \
	"  AND c_d_id = %d"

#define PAYMENT_7_BC \
	"UPDATE customer\n" \
	"SET c_balance = c_balance - %f,\n" \
	"    c_ytd_payment = c_ytd_payment + 1,\n" \
	"    c_data = '%s'\n" \
	"WHERE c_id = %d\n" \
	"  AND c_w_id = %d\n" \
	"  AND c_d_id = %d"

#define PAYMENT_8 \
	"INSERT INTO history (h_c_id, h_c_d_id, h_c_w_id, h_d_id, h_w_id,\n" \
	"                     h_date, h_amount, h_data)\n" \
	"VALUES (%d, %d, %d, %d, %d, current_timestamp, %f, '%s    %s')"

#define NEW_ORDER_1 \
	"SELECT w_tax\n" \
	"FROM warehouse\n" \
	"WHERE w_id = %d"

#define NEW_ORDER_2 \
	"SELECT d_tax, d_next_o_id\n" \
	"FROM district \n" \
	"WHERE d_w_id = %d\n" \
	"  AND d_id = %d\n" \
	"FOR UPDATE"

#define NEW_ORDER_3 \
	"UPDATE district\n" \
	"SET d_next_o_id = d_next_o_id + 1\n" \
	"WHERE d_w_id = %d\n" \
	"  AND d_id = %d"

#define NEW_ORDER_4 \
	"SELECT c_discount, c_last, c_credit\n" \
	"FROM customer\n" \
	"WHERE c_w_id = %d\n" \
	"  AND c_d_id = %d\n" \
	"  AND c_id = %d"

#define NEW_ORDER_5 \
	"INSERT INTO new_order (no_o_id, no_w_id, no_d_id)\n" \
        "VALUES (%d, %d, %d)"

#define NEW_ORDER_6 \
	"INSERT INTO orders (o_id, o_d_id, o_w_id, o_c_id, o_entry_d,\n" \
	"                    o_carrier_id, o_ol_cnt, o_all_local)\n" \
	"VALUES (%d, %d, %d, %d, current_timestamp, NULL, %d, %d)"

#define NEW_ORDER_7 \
	"SELECT i_price, i_name, i_data\n" \
	"FROM item\n" \
	"WHERE i_id = %d"

#define NEW_ORDER_8 \
	"SELECT s_quantity, %s, s_data\n" \
	"FROM stock\n" \
	"WHERE s_i_id = %d\n" \
	"  AND s_w_id = %d"

#define NEW_ORDER_9 \
	"UPDATE stock\n" \
	"SET s_quantity = s_quantity - %d\n" \
	"WHERE s_i_id = %d\n" \
	"  AND s_w_id = %d"

#define NEW_ORDER_10 \
	"INSERT INTO order_line (ol_o_id, ol_d_id, ol_w_id, ol_number,\n" \
	"                        ol_i_id, ol_supply_w_id, ol_delivery_d,\n" \
	"                        ol_quantity, ol_amount, ol_dist_info)\n" \
	"VALUES (%d, %d, %d, %d, %d, %d, NULL, %d, %f, '%s')"


#endif
