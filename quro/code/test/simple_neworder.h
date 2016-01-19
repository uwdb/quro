#include "mysql_helper.h"
const char s_dist[10][11] = {
        "s_dist_01", "s_dist_02", "s_dist_03", "s_dist_04", "s_dist_05",
        "s_dist_06", "s_dist_07", "s_dist_08", "s_dist_09", "s_dist_10"
};

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


struct OrderlineStruct{
	int ol_supply_w_id;
	int ol_i_id;
	int ol_quantity;
};

struct TNewOrderTxnInput{
	int w_id;
	int d_id;
	int c_id;
	int o_all_local;
	int o_ol_cnt;
	OrderlineStruct order_line[15];
};

struct TNewOrderTxnOutput{
	string w_tax;
	string d_tax;
	int d_next_o_id;
	string c_discount;
	string c_last;
	string c_credit;
};

//void execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut);

