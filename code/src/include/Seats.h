#ifndef SEATS_RUNNER
#define SEATS_RUNNER

#define CAL_RESP_TIME
#include <fstream>
using namespace std;

#include "locking.h"
#include "TxnHarnessStructs.h"
#include "FindFlightDB.h"
#include "NewReservationDB.h"
#include "CommonStructs.h"
#include "DBT5Consts.h"
#include "CSocket.h"

class CFindFlightDB;
class CNewReservationDB;

class SeatsRunner
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

	int RunFindFlight(TFindFlightTxnInput* pTxnInput,
									CFindFlightDB &FindFlight);
	int RunNewReservation(TNewReservationTxnInput* pTxnInput, 
									CNewReservationDB &NewReservation);

	friend void *workerThread(void *);

public:
	SeatsRunner(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory);

	void logErrorMessage(const string sErr, bool bScreen = true);

	void startListener(void);

};

//parameter structure for the threads
typedef struct TThreadParameter
{
#ifdef WORKLOAD_TPCE
	CBrokerageHouse* pBrokerageHouse;
#elif WORKLOAD_SEATS
	SeatsRunner* pSeats;
#endif
	int iSockfd;
	int t_id;
	char outputDir[256];
} *PThreadParameter;

#endif
