#include "mysql_helper.h"

#define TRADE_UPDATE_Q1 \
		"SELECT t_exec_name\n" \
		"FROM trade\n" \
		"WHERE t_id = %ld"

#define TRADE_UPDATE_QH1 \
		"SELECT REPLACE('%s', ' X ', ' ')"

#define TRADE_UPDATE_QH2 \
		"SELECT REPLACE('%s', ' ', ' X ')"

#define TRADE_UPDATE_Q2 \
		"UPDATE trade\n" \
		"SET t_exec_name = '%s'\n" \
		"WHERE t_id = %ld"

#define TRADE_UPDATE_Q3 \
		"SELECT t_bid_price, t_exec_name, t_is_cash, tt_is_mrkt,\n" \
		"       t_trade_price\n" \
		"FROM trade, trade_type\n" \
		"WHERE t_id = %ld\n" \
		"  AND t_tt_id = tt_id"

#define TRADE_UPDATE_Q4 \
		"SELECT se_amt, se_cash_due_date, se_cash_type\n" \
		"FROM settlement\n" \
		"WHERE se_t_id = %ld"

#define TRADE_UPDATE_Q5 \
		"SELECT ct_amt, ct_dts, ct_name\n" \
		"FROM cash_transaction\n" \
		"WHERE ct_t_id = %ld"

#define TRADE_UPDATE_Q6 \
		"SELECT th_dts, th_st_id\n" \
		"FROM trade_history\n" \
		"WHERE th_t_id = %ld\n" \
		"ORDER BY th_dts\n" \
		"LIMIT 3"


struct TTradeUpdateFrame1Input{
		uint64_t              trade_id[64];
    uint32_t               max_trades;
    uint32_t               max_updates;
};

struct TTradeUpdateFrame1Output{
};

void execute(const TTradeUpdateFrame1Input *pIn,
		TTradeUpdateFrame1Output *pOut);
