/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * This class represents the workload driver
 * 03 August 2006
 */

#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <iostream>
#include <fstream>
using namespace std;
#include "locking.h"

#ifdef WORKLOAD_TPCE
#include "EGenLogger.h"
#include "InputFlatFilesStructure.h"
#include "locking.h"
#include "CESUT.h"
using namespace TPCE;
#elif WORKLOAD_SEATS /* WORKLOAD_TPCE */
#include "CSeats.h"
#include <thread>
#include <mutex>
#elif WORKLOAD_BID
#include "CBid.h"
#include <thread>
#include <mutex>
#elif WORKLOAD_TPCC
#include "CTPCC.h"
#include <thread>
#include <mutex>
#endif /* WORKLOAF_TPCE */

class CCustomer
{
	int m_iUsers;
	int m_iPacingDelay;
#ifdef WORKLOAD_TPCE
	CLogFormatTab m_fmt;
	CEGenLogger *m_pLog;
	CInputFiles m_InputFiles;
	CCESUT * m_pCCESUT;
	CCE * m_pCCE;
	PDriverCETxnSettings m_pDriverCETxnSettings;
	CMutex m_LogLock;
	ofstream m_fLog; // error log file
#elif WORKLOAD_SEATS
	CSEATS *m_pSEATS;
	ofstream m_fLog; // error log file
	mutex m_LogLock;
#elif WORKLOAD_BID
	CBID *m_pBID;
	ofstream m_fLog;
	mutex m_LogLock;
#elif WORKLOAD_TPCC
	CTPCC *m_pTPCC;
	ofstream m_fLog;
	mutex m_LogLock;
#endif

private:
	void LogErrorMessage(const string);

	friend void *CustomerWorkerThread(void *);
	// entry point for driver worker thread
	friend void EntryCustomerWorkerThread(void *, int);

	friend void *DMWorkerThread(void *);
	friend void EntryDMWorkerThread(CCustomer *);
public:
#ifdef WORKLOAD_TPCE
	CCustomer(char *szInDir, TIdent iConfiguredCustomerCount,
			TIdent iActiveCustomerCount, INT32 iScaleFactor,
			INT32 iDaysOfInitialTrades, UINT32 UniqueId, char *szBHaddr,
			int iBHlistenPort, int iUsers, int iPacingDelay,
			char *outputDirectory, ofstream *m_fMix, CMutex *m_MixLock);
#elif WORKLOAD_SEATS
	CCustomer(char *szInDir, TIdent iConfiguredCustomerCount,
			TIdent iActiveCustomerCount, INT32 iScaleFactor,
			INT32 iDaysOfInitialTrades, UINT32 UniqueId, char *szBHaddr,
			int iBHlistenPort, int iUsers, int iPacingDelay,
			char *outputDirectory, ofstream *m_fMix, mutex *m_MixLock, uint64_t* flight_ids);
#elif WORKLOAD_BID
	CCustomer(char *szInDir, TIdent iConfiguredCustomerCount,
			TIdent iActiveCustomerCount, INT32 iScaleFactor,
			INT32 iDaysOfInitialTrades, UINT32 UniqueId, char *szBHaddr,
			int iBHlistenPort, int iUsers, int iPacingDelay,
			char *outputDirectory, ofstream *m_fMix, mutex *m_MixLock);
#elif WORKLOAD_TPCC
	CCustomer(char *szInDir, TIdent iConfiguredCustomerCount,
			TIdent iActiveCustomerCount, INT32 iScaleFactor,
			INT32 iDaysOfInitialTrades, UINT32 UniqueId, char *szBHaddr,
			int iBHlistenPort, int iUsers, int iPacingDelay,
			char *outputDirectory, ofstream *m_fMix, mutex *m_MixLock);

#endif
	~CCustomer();

	void DoTxn();
	void RunTest(int, int);
};

#endif // CUSTOMER_H
