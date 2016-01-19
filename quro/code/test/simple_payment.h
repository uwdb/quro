#include "mysql_helper.h"

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



struct TPaymentTxnInput{
	int w_id;
	int d_id;
	int c_w_id;
	int c_d_id;
	int c_id;
	int h_amount;
};

struct TPaymentTxnOutput{
	int status;
};

//void execute(TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut);

