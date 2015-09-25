/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * PostgreSQL connection class
 * 13 June 2006
 */

#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H
#ifdef DB_PGSQL
#include <libpq-fe.h>
#else
#include <mysql.h>
#endif
#include "TxnHarnessStructs.h"
#include "TxnHarnessSendToMarket.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#ifdef WORKLOAD_TPCE
#include "BrokerageHouse.h"
#elif WORKLOAD_SEATS
#include "Seats.h"
#elif WORKLOAD_BID
#include "Bid.h"
#elif WORKLOAD_TPCC
#include "TPCC.h"
#endif
#include "DBT5Consts.h"
#include "CommonStructs.h"


using namespace TPCE;
#ifdef WORKLOAD_SEATS
class SeatsRunner;
#elif WORKLOAD_BID
class BidRunner;
#elif WORKLOAD_TPCC
class TPCCRunner;
#endif

#define TABLE_PROFILE
//#define LOG_ERROR_MESSAGE(arg...) logErrorMessage(arg...)
//#define LOG_ERROR_MESSAGE logErrorMessage
#define LOG_ERROR_MESSAGE(arg...)

#ifndef DB_PGSQL
struct sql_result_t
{
  MYSQL_RES * result_set;
  MYSQL_ROW current_row;
  unsigned int num_fields;
  unsigned int num_rows;
  unsigned long * lengths;
  char * query;
};
#endif

#ifdef CAL_RESP_TIME
struct profile_node
{
	eTxnType type;
	timeval start;
	timeval end;
	bool commit;
	profile_node* next;
	profile_node(){
			type = NULL_TXN;
			commit = false;
			next = NULL;
	}
};
#endif
#ifdef PROFILE_EACH_QUERY
struct p_query{
		size_t frame;
		size_t query;
		double exec_time;
		bool commit;
		p_query(){
				frame = 0;
				query = 0;
				exec_time = 0.0;
				commit = false;
		}
		void set(size_t _frame, size_t _query, double _exec_time, bool _commit){
				frame = _frame;
				query = _query;
				exec_time = _exec_time;
				commit = _commit;
		}
};
#endif
class CDBConnection
{
private:
#ifdef DB_PGSQL
	PGconn *m_Conn;
#else
	MYSQL *dbc;  //mysql db
	char mysql_dbname[32];
	char mysql_host[32];
	char mysql_user[32];
	char mysql_pass[32];
	char mysql_port_t[32];
	char mysql_socket_t[256];
#endif

	char szConnectStr[iMaxConnectString + 1];
	char name[16];
#ifdef WORKLOAD_TPCE
	CBrokerageHouse *bh;
#elif WORKLOAD_SEATS
	SeatsRunner *seats;
#elif WORKLOAD_BID
	BidRunner *bid;
#elif WORKLOAD_TPCC
	TPCCRunner *tpcc;
#endif

	TTradeRequest m_TriggeredLimitOrders;


public:
#ifdef DB_PGSQL
	CDBConnection(const char *, const char *, const char *);
#else
#ifdef WORKLOAD_TPCE
	CDBConnection(CBrokerageHouse *bh, char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket);
#elif WORKLOAD_SETAS
	CDBConnection(SeatsRunner *_seats, char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket);
#elif WORKLOAD_BID
	CDBConnection(BidRunner *_bid, char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket);
#elif WORKLOAD_TPCC
	CDBConnection(TPCCRunner *_bid, char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket);
#endif
#endif

	int txn_cnt;
	double txn_time;
	int abort_cnt;
	timeval t1, t2;

	~CDBConnection();

	void begin();

	void commit();
	void connect();
	char *escape(string);
	void disconnect();

	ofstream outfile;

	int q_cnt;
#ifdef CAL_RESP_TIME
	profile_node* head;
	profile_node* cur;
	void init_profile_node(int t_id, char* outputDir);
	void append_profile_node(timeval _start, timeval _end, eTxnType _type, bool _commit);
#endif
#ifdef TIMEBREAK
	map<string, double> tb;
#endif
#ifdef TIMEPROFILE
	double _time[100];
	int _cnt[100];
	double _var[100];
#endif
#ifdef PROFILE_EACH_QUERY
	p_query queries[200];
//	size_t q_cnt;

	double exec_time;

	inline void add_profile_node(size_t _frame, size_t _query, double _exec_time, bool _commit){
			queries[q_cnt].set(_frame, _query, _exec_time, _commit);
			q_cnt++;
	}
	inline void print_profile_query(){
			for(size_t i=0; i<q_cnt; i++){
					outfile<<queries[i].frame<<"_"<<queries[i].query<<" ("<<queries[i].exec_time<<") ";
			}
			outfile<<endl;
			q_cnt = 0;
	}
#endif
#ifdef WORKLOAD_TPCE
	//PGresult *exec(const char *);
	void exec(const char *);

	void execute(const TBrokerVolumeFrame1Input *,
			TBrokerVolumeFrame1Output *);

	void execute(const TCustomerPositionFrame1Input *,
			TCustomerPositionFrame1Output *);
	void execute(const TCustomerPositionFrame2Input *,
			TCustomerPositionFrame2Output *);

	void execute(const TDataMaintenanceFrame1Input *);

	void execute(const TMarketFeedFrame1Input *, TMarketFeedFrame1Output *,
			CSendToMarketInterface *);

	void execute(const TMarketWatchFrame1Input *, TMarketWatchFrame1Output *);

	void execute(const TSecurityDetailFrame1Input *,
			TSecurityDetailFrame1Output *);

	void execute(const TTradeCleanupFrame1Input *);

	void execute(const TTradeLookupFrame1Input *, TTradeLookupFrame1Output *);
	void execute(const TTradeLookupFrame2Input *, TTradeLookupFrame2Output *);
	void execute(const TTradeLookupFrame3Input *, TTradeLookupFrame3Output *);
	void execute(const TTradeLookupFrame4Input *, TTradeLookupFrame4Output *);
#ifdef REORDER
	void execute(PTradeResultTxnInput, PTradeResultTxnOutput);
  void execute(PTradeOrderTxnInput, PTradeOrderIntermediate, PTradeOrderTxnOutput);
#else
	void execute(const TTradeOrderFrame1Input *, TTradeOrderFrame1Output *);
	void execute(const TTradeOrderFrame2Input *, TTradeOrderFrame2Output *);
	void execute(const TTradeOrderFrame3Input *, TTradeOrderFrame3Output *);
	void execute(const TTradeOrderFrame4Input *, TTradeOrderFrame4Output *);

	void execute(const TTradeResultFrame1Input *, TTradeResultFrame1Output *);
	void execute(const TTradeResultFrame2Input *, TTradeResultFrame2Output *);
	void execute(const TTradeResultFrame3Input *, TTradeResultFrame3Output *);
	void execute(const TTradeResultFrame4Input *, TTradeResultFrame4Output *);
	void execute(const TTradeResultFrame5Input *);
	void execute(const TTradeResultFrame6Input *, TTradeResultFrame6Output *);
#endif
	void execute(const TTradeStatusFrame1Input *, TTradeStatusFrame1Output *);

	void execute(const TTradeUpdateFrame1Input *, TTradeUpdateFrame1Output *);
	void execute(const TTradeUpdateFrame2Input *, TTradeUpdateFrame2Output *);
	void execute(const TTradeUpdateFrame3Input *, TTradeUpdateFrame3Output *);

#elif WORKLOAD_SEATS
	void execute(const TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut);
	void execute(const TNewReservationTxnInput* pIn, TNewReservationTxnOutput* pOut);
#elif WORKLOAD_BID
	void execute(const TBiddingTxnInput* pIn, TBiddingTxnOutput* pOut);
#elif WORKLOAD_TPCC
	void execute(const TNewOrderTxnInput* pIn, TNewOrderTxnOutput* pOut);
	void execute(const TPaymentTxnInput* pIn, TPaymentTxnOutput* pOut);
	void execute(const TDeliveryTxnInput* pIn, TDeliveryTxnOutput* pOut);
	void execute(const TStocklevelTxnInput* pIn, TStocklevelTxnOutput* pOut);
	void execute(const TOrderstatusTxnInput* pIn, TOrderstatusTxnOutput* pOut);
#endif // WORKLOAD_TPCE

#ifdef WORKLOAD_TPCE
	void setBrokerageHouse(CBrokerageHouse *);
#elif WORKLOAD_SEATS
	void setSeatsRunner(SeatsRunner *);
#elif WORKLOAD_BID
	void setBidRunner(BidRunner *);
#elif WORKLOAD_TPCC
	void setTPCCRunner(TPCCRunner *);
#endif
	void logErrorMessage(const char* c, ...);
	void reconnect();
	void rollback();
#ifdef DB_PGSQL
	void setReadCommitted();
	void setReadUncommitted();
	void setRepeatableRead();
	void setSerializable();
#else
int dbt5_sql_execute(char * query,
                     sql_result_t * sql_result, char * query_name);
int dbt5_sql_close_cursor(sql_result_t * sql_result);
int dbt5_sql_fetchrow(sql_result_t * sql_result);
char * dbt5_sql_getvalue(sql_result_t * sql_result,
                         int field);
char * dbt5_sql_getvalue(sql_result_t * sql_result, int field, int& length);

#endif
};

extern CDBConnection* pDBClist[1024];
extern timeval t_start_values[1024];
extern int connectionCnt;

inline
double difftimeval(timeval rt1, timeval rt0)
{
	return (rt1.tv_sec - rt0.tv_sec) +
	(double) (rt1.tv_usec - rt0.tv_usec) / 1000000.00;
}

string get_random_string(int length);
struct FIELDHIST{
	map<string, int> field_hist;
};
#define CLANG_PROFILE(QUERYN) \
tbl_name = returnTableName(QUERYN); \
param_str = parseQueryParams(QUERYN); \
if(table_hist.find(tbl_name) != table_hist.end()){ \
	if(table_hist[tbl_name].field_hist.find(param_str) != table_hist[tbl_name].field_hist.end()) \
		table_hist[tbl_name].field_hist[param_str] = table_hist[tbl_name].field_hist[param_str] + 1;  \
	else table_hist[tbl_name].field_hist[param_str] = 1; \
} \
else if(tbl_name.length()>0 && param_str.length()>0){ \
	FIELDHIST fh; \
	fh.field_hist[param_str] = 1; \
	table_hist[tbl_name] = fh; \
} \
profile_cnt++; \
if(profile_cnt%1000 == 0){\
	profile_cnt = 1; \
	cout<<"=========OUTPUT PROFILE========="<<endl; \
	for(map<string, FIELDHIST>::iterator it = table_hist.begin(); it != table_hist.end(); it++){ \
			cout<<it->first<<", size = "<<it->second.field_hist.size()<<endl; \
	} \
}
string parseQueryParams(char* query);
void splitByBlank(vector<string>& vec, string str);
string returnTableName(char* query);


#endif //DB_CONNECTION_H
