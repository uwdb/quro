#ifndef TPCC_RUNNER
#define TPCC_RUNNER

#define PROFILE_EACH_QUERY
#define CAL_RESP_TIME
//#define TABLE_PROFILE
//#define TIMEBREAK
//#define TIMEPROFILE
//#define TYPEBREAK
#include <fstream>
using namespace std;

#include "locking.h"
#include "TxnHarnessStructs.h"
#include "TPCCDB.h"
#include "CommonStructs.h"
#include "CSocket.h"

class CNewOrderDB;
class CPaymentDB;

class TPCCRunner
{
private:
	int m_iListenPort;
	CSocket m_Socket;
	CMutex m_LogLock;
	ofstream m_fLog;

	char mysql_dbname[32];
	char mysql_host[32];
	char mysql_user[32];
	char mysql_pass[32];
	char mysql_port_t[32];
	char mysql_socket_t[256];

	char outputDir[256];

	friend void entryWorkerThread(void *); // entry point for worker thread

	int RunNewOrder(TNewOrderTxnInput* pTxnInput,
									CTPCCDB &tpcc);
	int RunPayment(TPaymentTxnInput* pTxnInput,
									CTPCCDB &tpcc);
	int RunDelivery(TDeliveryTxnInput* pTxnInput,
									CTPCCDB &tpcc);
	int RunOrderstatus(TOrderstatusTxnInput* pTxnInput,
									CTPCCDB &tpcc);
	int RunStocklevel(TStocklevelTxnInput* pTxnInput,
									CTPCCDB &tpcc);

	friend void *workerThread(void *);

public:
	TPCCRunner(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory);

	void logErrorMessage(const string sErr, bool bScreen = true);

	void startListener(void);

};

//parameter structure for the threads
typedef struct TThreadParameter
{
	TPCCRunner* pTPCC;
	int iSockfd;
	int t_id;
	char outputDir[256];
} *PThreadParameter;

#ifdef TIMEBREAK
#define SETPROFILING \
sprintf(query, "SET profiling = 1"); \
	dbt5_sql_execute(query, &result, "PROFILE");
#else
#define SETPROFILING

#endif

#ifdef TYPEBREAK
#define TIME_VAR timeval t_start1, t_end1; double type_t_time=0.0;
#define TXN_BEGIN \
gettimeofday(&t_start1, NULL);
#define TXN_END(f) \
gettimeofday(&t_end1, NULL); \
type_t_time = difftimeval(t_end1, t_start1); \
_type_time[f] += type_t_time; \
_type_cnt[f] ++;
#else 
#define TIME_VAR
#define TXN_BEGIN
#define TXN_END(f)
#endif

#ifdef TIMEBREAK
#define EXECUTEPROFILING \
sprintf(query, "show profile"); \
if(dbt5_sql_execute(query, &result, "PROF") && result.result_set){ \
	int num_rows = result.num_rows; \
	q_cnt++; \
	for(int k=0; k<num_rows; k++){ \
		dbt5_sql_fetchrow(&result); \
		char* val; \
		val = dbt5_sql_getvalue(&result, 0, length); \
		string f(val); \
		double time = atof(dbt5_sql_getvalue(&result, 3, length)); \
		if(tb.find(f) != tb.end()) \
			tb[f] = tb[f] + time; \
		else \
			tb[f] = time; \
	} \
}
#else
#define EXECUTEPROFILING

#ifdef TIMEPROFILE
#define GETTIME \
gettimeofday(&t1, NULL);
#define GETPROFILE(f) \
gettimeofday(&t2, NULL); \
t_time = difftimeval(t2, t1); \
_time[f] += t_time; \
_cnt[f] ++;
#else
#define GETTIME
#define GETPROFILE(f)
#endif

#endif

#endif
