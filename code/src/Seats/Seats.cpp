#include "Seats.h"
#include "CommonStructs.h"
#include "DBConnection.h"

#include "FindFlightDB.h"
//#include "NewReservation.h"
//#include "UpdateCustomer.h"
//#include "UpdateReservation.h"

#include <signal.h>
#include <sys/time.h>

extern CDBConnection* pDBClist[1024];
extern int connectionCnt;

void *workerThread(void *data)
{
try {
		PThreadParameter pThrParam = reinterpret_cast<PThreadParameter>(data);

		CSocket sockDrv;
		sockDrv.setSocketFd(pThrParam->iSockfd); // client socket

		PMsgDriverSeats pMessage = new TMsgDriverSeats;
		memset(pMessage, 0, sizeof(TMsgDriverSeats)); // zero the structure

		TMsgSeatsDriver Reply; // return message
		INT32 iRet = 0; // transaction return code
		CDBConnection *pDBConnection = NULL;

		pDBConnection = new CDBConnection(
			pThrParam->pSeats,
			pThrParam->pSeats->mysql_dbname,
			pThrParam->pSeats->mysql_host,
			pThrParam->pSeats->mysql_user,
			pThrParam->pSeats->mysql_pass,
			pThrParam->pSeats->mysql_port_t,
			pThrParam->pSeats->mysql_socket_t);
#ifdef CAL_RESP_TIME
			pDBClist[pThrParam->t_id] = pDBConnection;
			pDBConnection->init_profile_node(pThrParam->t_id, pThrParam->outputDir);
#endif

		CFindFlightDB findFlightDB(pDBConnection);

		int txn_cnt = 0;
		double txn_time = 0;
		bool commit = true;
		double receiving_time = 0;
		do {
			try {
				timeval tt1, tt2;
				gettimeofday(&tt1, NULL);
				sockDrv.dbt5Receive(reinterpret_cast<void *>(pMessage),
						sizeof(TMsgDriverSeats));
				gettimeofday(&tt2, NULL);
				receiving_time += difftimeval(tt2, tt1);
				if(txn_cnt > 0 && difftimeval(tt2, tt1)>1)pDBConnection->outfile<<"END"<<endl;
				pDBConnection->outfile.flush();
			} catch(CSocketErr *pErr) {
				sockDrv.dbt5Disconnect();

				delete pErr;

				// The socket has been closed, break and let this thread die.
				break;
			}
			commit = true;
			iRet = CBaseTxnErr::SUCCESS;
			try {
				//  Parse Txn type
				switch (pMessage->TxnType) {
					case FIND_FLIGHT:
							iRet = pThrParam->pSeats->RunFindFlight(&(pMessage->TxnInput.FindFlightTxnInput), findFlightDB);
							break;
					default:
							iRet = ERR_TYPE_WRONGTXN;
				}
				txn_cnt++;
			}catch (const char *str) {
			pDBConnection->rollback();
#ifdef CAL_RESP_TIME
			gettimeofday(&t2, NULL);
			exec_time = difftimeval(t2, t1);
			txn_time += exec_time;
#ifdef PROFILE_EACH_QUERY
			pDBConnection->print_profile_query();
#endif
			pDBConnection->outfile.flush();
#endif
				iRet = CBaseTxnErr::EXPECTED_ROLLBACK;

				commit = false;
				//XXX:debug for trade result
			}
#ifdef CAL_RESP_TIME
			gettimeofday(&t2, NULL);
			exec_time = difftimeval(t2, t1);
			txn_time += exec_time;

			pDBConnection->outfile<<commit<<" start=( "<<t1.tv_sec<<" "<<t1.tv_usec<<" ), end=( "<<t2.tv_sec<<" "<<t2.tv_usec<<" ), "<<exec_time<<", txn_cnt = "<<txn_cnt<<"total: "<<txn_time<<endl;
#ifdef PROFILE_EACH_QUERY
			pDBConnection->print_profile_query();
#endif
			pDBConnection->outfile.flush();

#endif

			// send status to driver
			Reply.iStatus = iRet;
			try {
				sockDrv.dbt5Send(reinterpret_cast<void *>(&Reply), sizeof(Reply));
			} catch(CSocketErr *pErr) {
				sockDrv.dbt5Disconnect();

				delete pErr;

				// The socket has been closed, break and let this thread die.
				break;
			}
		} while (true);

		delete pDBConnection; // close connection with the database
		close(pThrParam->iSockfd); // close socket connection with the driver
		delete pThrParam;
		delete pMessage;
	} catch (CSocketErr *err) {
	}
	return NULL;
}

// entry point for worker thread
void entryWorkerThread(void *data)
{
	PThreadParameter pThrParam = reinterpret_cast<PThreadParameter>(data);

	pthread_t threadID; // thread ID
	pthread_attr_t threadAttribute; // thread attribute

	try {
		// initialize the attribute object
		int status = pthread_attr_init(&threadAttribute);
		if (status != 0) {
			throw new CThreadErr(CThreadErr::ERR_THREAD_ATTR_INIT);
		}

		// set the detachstate attribute to detached
		status = pthread_attr_setdetachstate(&threadAttribute,
				PTHREAD_CREATE_DETACHED);
		if (status != 0) {
			throw new CThreadErr(CThreadErr::ERR_THREAD_ATTR_DETACH);
		}
		// create the thread in the detached state
		status = pthread_create(&threadID, &threadAttribute, &workerThread,
				data);

		if (status != 0) {
			throw new CThreadErr(CThreadErr::ERR_THREAD_CREATE);
		}
	} catch(CThreadErr *pErr) {
		// close recently accepted connection, to release driver threads
		close(pThrParam->iSockfd);

		delete pThrParam;
		delete pErr;
	}
}

SeatsRunner::SeatsRunner(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory)
: m_iListenPort(iListenPort)
{
	if (_mysql_dbname != NULL) {
		strcpy(mysql_dbname, _mysql_dbname);
	}
	if (_mysql_host != NULL) {
		strcpy(mysql_host, _mysql_host);
        }
	if (_mysql_user != NULL) {
		strcpy(mysql_user, _mysql_user);
        }
	if (_mysql_pass != NULL) {
		strcpy(mysql_pass, _mysql_pass);
	}
	if (_mysql_port != NULL) {
		strcpy(mysql_port_t, _mysql_port);
	}
	if (_mysql_socket != NULL) {
		strcpy(mysql_socket_t, _mysql_socket);
	}
	char filename[iMaxPath + 1];
	snprintf(filename, iMaxPath, "%s/SeatsError.log",
			outputDirectory);
	char s[1000];
	m_fLog.open(filename, ios::out);
	strcpy(outputDir, outputDirectory);
	logErrorMessage(s, false);

}

int SeatsRunner::RunFindFlight(TFindFlightTxnInput* pTxnInput, CFindFlightDB &findFlight){
	TFindFlightTxnOutput ffOutput;

	findFlight.DoFindFlight(pTxnInput, &ffOutput);

	return ffOutput.status;
}

// Listener
void SeatsRunner::startListener(void)
{
	int acc_socket;
	PThreadParameter pThrParam = NULL;

	m_Socket.dbt5Listen(m_iListenPort);
	ostringstream msg1;
	msg1<<"STARTLISTENER: m_iListenPort = "<<m_iListenPort<<endl;
	logErrorMessage(msg1.str(), false);

	int t_cnt = 0;
	while (true) {
		acc_socket = 0;
		try {
			acc_socket = m_Socket.dbt5Accept();

			pThrParam = new TThreadParameter;
			// zero the structure
			memset(pThrParam, 0, sizeof(TThreadParameter));

			pThrParam->iSockfd = acc_socket;
			pThrParam->pSeats = this;
			pThrParam->t_id = t_cnt;
			strcpy(pThrParam->outputDir, outputDir);
			t_cnt++;
			connectionCnt = t_cnt;
			assert(connectionCnt < 1024);
			// call entry point
			entryWorkerThread(reinterpret_cast<void *>(pThrParam));
		} catch (CSocketErr *pErr) {
			ostringstream osErr;
			osErr << "Problem accepting socket connection" << endl <<
					"Error: " << pErr->ErrorText() << " at " <<
					"BrokerageHouse::Listener" << endl;
			logErrorMessage(osErr.str());
			delete pErr;
			delete pThrParam;
		}
	}
}

// logErrorMessage
void SeatsRunner::logErrorMessage(const string sErr, bool bScreen)
{
	m_LogLock.lock();
	if (bScreen) cout << sErr;
	m_fLog << sErr;
	m_fLog.flush();
	m_LogLock.unlock();
}
