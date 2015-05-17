/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * This class represents the workload driver
 * 03 August 2006
 */

#ifndef DRIVER_H
#define DRIVER_H
#ifdef WORKLOAD_TPCE
#include "EGenLogFormatterTab.h"
#include "EGenLogger.h"
#include "InputFlatFilesStructure.h"
#include "DMSUT.h"
#include "locking.h"
#elif WORKLOAD_SEATS
#include <iostream>
#include <fstream>
#include "locking.h"
#include <thread>
#include <mutex>
#include "util.h"
using namespace std;
#define TIdent int64_t
#define INT32 int32_t
#define iMaxPath 50
#define iMaxHostname 20
#endif

using namespace TPCE;

class CDriver
{
private:
#ifdef WORKLOAD_TPCE
	CLogFormatTab m_fmt;
	CEGenLogger *m_pLog;
	CInputFiles m_InputFiles;
	PDriverCETxnSettings m_pDriverCETxnSettings;
	CMutex m_LogLock;
#else
	mutex m_LogLock;
#endif
	ofstream m_fLog; // error log file
	ofstream m_fMix; // mix log file

	void logErrorMessage(const string);

	friend void *customerWorkerThread(void *);
	// entry point for driver worker thread
	friend void entryCustomerWorkerThread(void *, int);

	friend void *dmWorkerThread(void *);
	friend void entryDMWorkerThread(CDriver *);

public:
	char szInDir[iMaxPath + 1];
	TIdent iConfiguredCustomerCount;
	TIdent iActiveCustomerCount;
	INT32 iScaleFactor;
	INT32 iDaysOfInitialTrades;
	UINT32 iSeed;
	char szBHaddr[iMaxHostname + 1];
	int iBHlistenPort;
	int iUsers;
	int iPacingDelay;
	char outputDirectory[iMaxPath + 1];
#ifdef WORKLOAD_TPCE
	CDMSUT *m_pCDMSUT;
	CDM *m_pCDM;
	CMutex m_MixLock;
#else
	mutex m_MixLock;
#endif

	CDriver(char *, TIdent, TIdent, INT32, INT32, UINT32, char *, int, int,
			int, char *);
	~CDriver();

	void runTest(int, int);
};

//parameter structure for the threads
typedef struct TCustomerThreadParam
{
	CDriver *pDriver;
	uint64_t* flight_ids;
} *PCustomerThreadParam;


#endif // DRIVER_H
