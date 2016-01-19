#include "mysql_helper.h"

#define GET_USER \
"SELECT BID, NAME FROM USER WHERE USER_ID = %ld"

#define UPDATE_USER \
"UPDATE USER SET BID = BID + 1 \n" \
"	WHERE USER_ID = %ld"

#define INSERT_BID \
"INSERT INTO BIDREC (" \
" USER_ID, BID, PID, PRICE, DESCRIPTION ) " \
" VALUES (%ld, %ld, %ld, %f, '%s')"

#define SELECT_ITEM \
"SELECT MAX_PRICE, DESCRIPTION \n" \
"  FROM BIDITEM \n" \
"  WHERE PID = %ld"

#define UPDATE_ITEM \
"UPDATE BIDITEM SET \n" \
"  MAX_PRICE = %f \n" \
"  WHERE PID = %d"


void execute(uint64_t item_id, uint64_t user_id, double bid_price);
