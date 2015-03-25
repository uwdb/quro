/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * 30 July 2006
 */

#include "MarketExchange.h"
#include <sys/time.h>
// worker thread
void *MarketWorkerThread(void* data)
{
	PMarketThreadParam pThrParam = reinterpret_cast<PMarketThreadParam>(data);

	CSocket sockDrv;
	sockDrv.setSocketFd(pThrParam->iSockfd); // client socket

	PTradeRequest pMessage = new TTradeRequest;
	memset(pMessage, 0, sizeof(TTradeRequest)); // zero the structure

#ifndef NO_DEBUG_INFO
	ostringstream msg;
	msg<<"MarketWorkerThread start iSockfd = "<<pThrParam->iSockfd<<std::endl;
	pThrParam->pMarketExchange->logErrorMessage(msg.str());
#endif
	do {
		try {
			sockDrv.dbt5Receive(reinterpret_cast<void*>(pMessage),
					sizeof(TTradeRequest));

			// submit trade request
			pThrParam->pMarketExchange->m_pCMEE[pThrParam->t_id]->SubmitTradeRequest(pMessage);
//			timeval t1;
//			gettimeofday(&t1, NULL);
//			ostringstream msg1;
//			msg1<<"send request, socketfd = "<<pThrParam->iSockfd<<", time = "<<t1.tv_sec<<","<<t1.tv_usec<<endl;
//			pThrParam->pMarketExchange->logErrorMessage(msg1.str());
		} catch(CSocketErr *pErr) {
			sockDrv.dbt5Disconnect(); // close connection

			ostringstream osErr;
			osErr << time(NULL) <<
					" Trade Request not submitted to Market Exchange" << endl <<
					"Error: "<<pErr->ErrorText() << endl;
			pThrParam->pMarketExchange->logErrorMessage(osErr.str());
			delete pErr;

			// The socket is closed, break and let this thread die.
			break;
		}
	} while (true);

	delete pMessage;
	delete pThrParam;
	return NULL;
}

// entry point for worker thread
void EntryMarketWorkerThread(void *data)
{
	PMarketThreadParam pThrParam = reinterpret_cast<PMarketThreadParam>(data);

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
#ifndef NO_DEBUG_INFO
		ostringstream msg;
		msg<<"MarketWatcher, creating thread"<<endl;
		pThrParam->pMarketExchange->logErrorMessage(msg.str());
#endif
		// create the thread in the detached state
		status = pthread_create(&threadID, &threadAttribute,
				&MarketWorkerThread, data);

		if (status != 0) {
			throw new CThreadErr(CThreadErr::ERR_THREAD_CREATE);
		}
	} catch(CThreadErr *pErr) {
		// close recently accepted connection, to release threads
		close(pThrParam->iSockfd);

		ostringstream osErr;
		osErr << "Error: " << pErr->ErrorText() <<
			" at MarketExchange::entryMarketWorkerThread" << endl <<
			"accepted socket connection closed" << endl;
		pThrParam->pMarketExchange->logErrorMessage(osErr.str());
		delete pErr;
	}
}

// Constructor
CMarketExchange::CMarketExchange(char *szFileLoc,
		TIdent iConfiguredCustomerCount, TIdent iActiveCustomerCount,
		int iListenPort, int iUsers, char *szBHaddr, int iBHlistenPort,
		char *outputDirectory)
: m_iListenPort(iListenPort)
{
	char filename[iMaxPath + 1];
	snprintf(filename, iMaxPath, "%s/MarketExchange.log", outputDirectory);
	m_pLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);

	snprintf(filename, iMaxPath, "%s/MarketExchange_Error.log",
			outputDirectory);
	m_fLog.open(filename, ios::out);
	snprintf(filename, iMaxPath, "%s/%s", outputDirectory, MEE_MIX_LOG_NAME);
	m_fMix.open(filename, ios::out);
#ifndef NO_DEBUG_INFO
	ostringstream msg;
	msg<<"starting: szBHaddr = "<<szBHaddr<<", iBHlistenPort = "<<iBHlistenPort<<endl;
	logErrorMessage(msg.str());
#endif
	// Initialize MEESUT
	//iUsers = (iUsers>>2)>1?(iUsers>>2):1;
	Users = iUsers;
	assert(iUsers <= 128);
	for(int i=0; i<iUsers; i++){
			m_pCMEESUT[i] = new CMEESUT(szBHaddr, iBHlistenPort, &m_fLog, &m_fMix,
					&m_LogLock, &m_MixLock);
	}
#ifndef NO_DEBUG_INFO
	msg<<"finish CMEESUT"<<endl;
	msg<<"inputFiles initialize: "<<iConfiguredCustomerCount<<", "<<iActiveCustomerCount<<", "<<szFileLoc<<endl;
	logErrorMessage(msg.str());
#endif

	// Initialize MEE
	CInputFiles inputFiles;
	inputFiles.Initialize(eDriverEGenLoader, iConfiguredCustomerCount,
			iActiveCustomerCount, szFileLoc);
	for(int i=0; i<iUsers; i++){
			m_pCMEE[i] = new CMEE(0, m_pCMEESUT[i], m_pLog, inputFiles, 1);
			m_pCMEE[i]->SetBaseTime();
	}
}

// Destructor
CMarketExchange::~CMarketExchange()
{
//	delete m_pCMEE;
//	delete m_pCMEESUT;

	m_fMix.close();
	m_fLog.close();

	delete m_pLog;
}

void CMarketExchange::startListener(void)
{
	int acc_socket;
	PMarketThreadParam pThrParam;

	m_Socket.dbt5Listen(m_iListenPort);
#ifndef NO_DEBUG_INFO
	ostringstream msg;
	msg<<"start listener"<<endl;
	logErrorMessage(msg.str());
#endif
	int t_cnt = 0;
	while (true) {
		acc_socket = 0;
		try {
			acc_socket = m_Socket.dbt5Accept();
#ifndef NO_DEBUG_INFO
			ostringstream msg;
			msg<<"MarketExchange: connectionCnt = "<<t_cnt<<", socket = "<<acc_socket<<", iListenPort = "<<m_iListenPort<<endl;
			logErrorMessage(msg.str());
#endif

			// create new parameter structure
			pThrParam = new TMarketThreadParam;
			// zero the structure
			memset(pThrParam, 0, sizeof(TMarketThreadParam));

			pThrParam->iSockfd = acc_socket;
			pThrParam->pMarketExchange = this;

			pThrParam->t_id = t_cnt%Users;
			t_cnt++;
			// call entry point
			EntryMarketWorkerThread(reinterpret_cast<void*>(pThrParam));
		} catch(CSocketErr *pErr) {
			ostringstream osErr;
			osErr << "Problem to accept socket connection" << endl <<
					"Error: " << pErr->ErrorText() << " at " <<
					"MarketExchange::startListener" << endl;
			logErrorMessage(osErr.str());
			delete pErr;
		}
	}
}

void CMarketExchange::logErrorMessage(const string sErr)
{
	m_LogLock.lock();
	cout << sErr;
	m_fLog << sErr;
	m_fLog.flush();
	m_LogLock.unlock();
}
