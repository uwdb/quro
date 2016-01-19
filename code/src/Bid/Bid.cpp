#include "Bid.h"
#include "CommonStructs.h"
#include "DBConnection.h"

#include "BiddingDB.h"
//#include "UpdateCustomer.h"
//#include "UpdateReservation.h"

#include "util.h"
using namespace std;

#include <signal.h>
#include <sys/time.h>

extern CDBConnection* pDBClist[1024];
extern int connectionCnt;
void signal_kill_handler(int signum){
	timeval t2;
	gettimeofday(&t2, NULL);
	for(int i=0; i<connectionCnt; i++){
			CDBConnection* ptr = (CDBConnection*)pDBClist[i];
			double timerange = difftimeval(t2, t_start_values[i]);
			cout<<"txn cnt for connection "<<i<<": commit "<<ptr->txn_cnt<<", abort "<<ptr->abort_cnt<<", timerange = "<<timerange<<", txn_time = "<<ptr->txn_time<<endl;
//				cout<<"txn cnt for connection "<<i<<": "<<ptr->txn_cnt<<endl;
	}
	exit(signum);
}

void *workerThread(void *data)
{
	signal(SIGTERM, signal_kill_handler);

try {
		PThreadParameter pThrParam = reinterpret_cast<PThreadParameter>(data);

		CSocket sockDrv;
		sockDrv.setSocketFd(pThrParam->iSockfd); // client socket

		PMsgDriverBid pMessage = new TMsgDriverBid;
		memset(pMessage, 0, sizeof(TMsgDriverBid)); // zero the structure

		TMsgBidDriver Reply; // return message
		INT32 iRet = 0; // transaction return code
		CDBConnection *pDBConnection = NULL;

		pDBConnection = new CDBConnection(
			pThrParam->pBid,
			pThrParam->pBid->mysql_dbname,
			pThrParam->pBid->mysql_host,
			pThrParam->pBid->mysql_user,
			pThrParam->pBid->mysql_pass,
			pThrParam->pBid->mysql_port_t,
			pThrParam->pBid->mysql_socket_t);
#ifdef CAL_RESP_TIME
			timeval t1, t2;
			double exec_time;
			pDBClist[pThrParam->t_id] = pDBConnection;
			pDBConnection->init_profile_node(pThrParam->t_id, pThrParam->outputDir);
#endif

		CBiddingDB biddingDB(pDBConnection);

		int txn_cnt = 0;
		int abort_cnt = 0;
		double txn_time = 0;
		bool commit = true;
		double receiving_time = 0;
		do {
			try {
				sockDrv.dbt5Receive(reinterpret_cast<void *>(pMessage),
						sizeof(TMsgDriverBid));
				pDBConnection->outfile.flush();
			} catch(CSocketErr *pErr) {
				sockDrv.dbt5Disconnect();

				delete pErr;

				// The socket has been closed, break and let this thread die.
				break;
			}
			timeval t1, t2;
			double exec_time;
		 	gettimeofday(&t1, NULL);

			commit = true;
			iRet = CBaseTxnErr::SUCCESS;
			try {
				//  Parse Txn type
					iRet = pThrParam->pBid->RunBidding(&pMessage->TxnInput, biddingDB);
				txn_cnt++;
				pDBConnection->txn_cnt = txn_cnt;
			}catch (const char *str) {
			pDBConnection->rollback();
//#ifdef CAL_RESP_TIME
//			gettimeofday(&t2, NULL);
//			exec_time = difftimeval(t2, t1);
//			txn_time += exec_time;
//#ifdef PROFILE_EACH_QUERY
//			pDBConnection->print_profile_query();
//#endif
//			pDBConnection->outfile.flush();
//#endif
//				pDBConnection->outfile<<"error: "<<str<<endl;

				iRet = CBaseTxnErr::EXPECTED_ROLLBACK;

				commit = false;
				abort_cnt++;
				pDBConnection->abort_cnt = abort_cnt;
				//XXX:debug for trade result
			}
			gettimeofday(&t2, NULL);
			exec_time = difftimeval(t2, t1);
			txn_time += exec_time;
			pDBConnection->txn_time = txn_time;
#ifdef CAL_RESP_TIME
//			pDBConnection->outfile<<commit<<" start=( "<<t1.tv_sec<<" "<<t1.tv_usec<<" ), end=( "<<t2.tv_sec<<" "<<t2.tv_usec<<" ), "<<exec_time<<", txn_cnt = "<<txn_cnt<<"total: "<<txn_time<<endl;
#ifdef PROFILE_EACH_QUERY
//			pDBConnection->print_profile_query();
#endif
			//pDBConnection->outfile<<"commit txn "<<txn_cnt<<endl;
//			pDBConnection->outfile.flush();

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

BidRunner::BidRunner(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory)
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
	snprintf(filename, iMaxPath, "%s/BidError.log",
			outputDirectory);
	char s[1000];
	m_fLog.open(filename, ios::out);
	strcpy(outputDir, outputDirectory);
	logErrorMessage(s, false);
}


int BidRunner::RunBidding(TBiddingTxnInput* pTxnInput, CBiddingDB &bidding){
	TBiddingTxnOutput bidOutput;

	bidding.DoBidding(pTxnInput, &bidOutput);

	return bidOutput.status;
}

// Listener
void BidRunner::startListener(void)
{
	int acc_socket;
	PThreadParameter pThrParam = NULL;

	m_Socket.dbt5Listen(m_iListenPort);
	ostringstream msg1;
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
			pThrParam->pBid = this;
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
void BidRunner::logErrorMessage(const string sErr, bool bScreen)
{
	m_LogLock.lock();
	if (bScreen) cout << sErr;
	m_fLog << sErr;
	m_fLog.flush();
	m_LogLock.unlock();
}
