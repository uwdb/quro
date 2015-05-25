/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * This class represents the Brokerage House
 * 25 July 2006
 */

#ifndef BROKERAGE_HOUSE_H
#define BROKERAGE_HOUSE_H

#include <fstream>
using namespace std;

#define CAL_RESP_TIME
#define PROFILE_EACH_QUERY

#include "locking.h"
#include "TxnHarnessStructs.h"
#include "TxnHarnessBrokerVolume.h"
#include "TxnHarnessCustomerPosition.h"
#include "TxnHarnessDataMaintenance.h"
#include "TxnHarnessMarketFeed.h"
#include "TxnHarnessMarketWatch.h"
#include "TxnHarnessSecurityDetail.h"
#include "TxnHarnessTradeCleanup.h"
#include "TxnHarnessTradeLookup.h"
#include "TxnHarnessTradeOrder.h"
#include "TxnHarnessTradeResult.h"
#include "TxnHarnessTradeStatus.h"
#include "TxnHarnessTradeUpdate.h"

#include "DBT5Consts.h"
#include "CSocket.h"

#ifdef NO_MEE_FOR_TRADERESULT
#include "MEESUT.h"
#include "EGenLogFormatterTab.h"
#include "EGenLogger.h"
#include "SecurityFile.h"
#endif
using namespace TPCE;

class CBrokerageHouse
{
private:
	int m_iListenPort;
	CSocket m_Socket;
	CMutex m_LogLock;
	ofstream m_fLog;
#ifdef DB_PGSQL
	char m_szHost[iMaxHostname + 1]; // host name
	char m_szDBName[iMaxDBName + 1]; // database name
	char m_szDBPort[iMaxPort + 1]; // PostgreSQL postmaster port
#else
	char mysql_dbname[32];
	char mysql_host[32];
	char mysql_user[32];
	char mysql_pass[32];
	char mysql_port_t[32];
	char mysql_socket_t[256];

	char outputDir[256];

#endif

#ifdef NO_MEE_FOR_TRADERESULT
	CMEE* m_pCMEE[128];
	CMEESUT* m_pCMEESUT[128];
	CLogFormatTab m_fmt;
	CEGenLogger *m_pLog;
	CMutex m_MixLock[128];
	CMutex m_meeLogLock[128];
	char* szFileLoc;
	char* szBHaddr;
	int iActiveCustomerCount;
	int iConfiguredCustomerCount;
	char* outputDirectory;
public:
	void startFakeMEE();
	int iUsers;
private:
#endif
	friend void entryWorkerThread(void *); // entry point for worker thread

	void dumpInputData(PBrokerVolumeTxnInput);
	void dumpInputData(PCustomerPositionTxnInput);
	void dumpInputData(PDataMaintenanceTxnInput);
	void dumpInputData(PTradeCleanupTxnInput);
	void dumpInputData(PMarketWatchTxnInput);
	void dumpInputData(PMarketFeedTxnInput);
	void dumpInputData(PSecurityDetailTxnInput);
	void dumpInputData(PTradeStatusTxnInput);
	void dumpInputData(PTradeLookupTxnInput);
	void dumpInputData(PTradeOrderTxnInput);
	void dumpInputData(PTradeResultTxnInput);
	void dumpInputData(PTradeUpdateTxnInput);

	INT32 RunBrokerVolume(PBrokerVolumeTxnInput pTxnInput,
			CBrokerVolume &BrokerVolume);
	INT32 RunCustomerPosition(PCustomerPositionTxnInput pTxnInput,
			CCustomerPosition &CustomerPosition);
	INT32 RunDataMaintenance(PDataMaintenanceTxnInput pTxnInput,
			CDataMaintenance &DataMaintenance);
	INT32 RunTradeCleanup(PTradeCleanupTxnInput pTxnInput,
			CTradeCleanup &TradeCleanup);
	INT32 RunMarketWatch(PMarketWatchTxnInput pTxnInput,
			CMarketWatch &MarketWatch);
	INT32 RunMarketFeed(PMarketFeedTxnInput pTxnInput,
			CMarketFeed &MarketFeed);
	INT32 RunSecurityDetail(PSecurityDetailTxnInput pTxnInput,
			CSecurityDetail &SecurityDetail);
	INT32 RunTradeStatus(PTradeStatusTxnInput pTxnInput,
			CTradeStatus &TradeStatus);
	INT32 RunTradeLookup(PTradeLookupTxnInput pTxnInput,
			CTradeLookup &TradeLookup);
	INT32 RunTradeOrder(PTradeOrderTxnInput pTxnInput,
			CTradeOrder &TradeOrder);
	INT32 RunTradeResult(PTradeResultTxnInput pTxnInput,
			CTradeResult &TradeResult);
	INT32 RunTradeUpdate(PTradeUpdateTxnInput pTxnInput,
			CTradeUpdate &TradeUpdate);

	friend void *workerThread(void *);

public:
#ifdef DB_PGSQL
	CBrokerageHouse(const char *, const char *, const char *, const int,
				char *);
#else
#ifdef NO_MEE_FOR_TRADERESULT
	CBrokerageHouse(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *_outputDirectory, char *_szFileLoc, char *_szBHaddr, int _iActiveCustomerCount, int _iConfiguredCustomerCount, int _iUsers);
#else
	CBrokerageHouse(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory);
#endif
#endif

	~CBrokerageHouse();

	void logErrorMessage(const string sErr, bool bScreen = true);

	void startListener(void);
};

//parameter structure for the threads
typedef struct TThreadParameter
{
	CBrokerageHouse* pBrokerageHouse;
	int iSockfd;
	int t_id;
	char outputDir[256];
} *PThreadParameter;

#endif // BROKERAGE_HOUSE_H
