/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * 30 July 2006
 */

#include "TxnHarnessSendToMarket.h"
#include <sys/time.h>
CSendToMarket::CSendToMarket(ofstream* pfile, int MEport)
: m_pfLog(pfile), m_MEport(MEport)
{
	// FIXME: This addr needs to be configurable.
	char addr[iMaxHostname + 1];
	strncpy(addr, "localhost", iMaxHostname);
#ifndef NO_MEE_FOR_TRADERESULT
	m_Socket = new CSocket(addr, m_MEport);
	m_Socket->dbt5Connect();
#endif
}

CSendToMarket::~CSendToMarket()
{
	m_Socket->dbt5Disconnect();
	delete m_Socket;
}

bool CSendToMarket::SendToMarket(TTradeRequest &trade_mes)
{
	try {
		// send Trade Request to MEE
#ifdef NO_MEE_FOR_TRADERESULT
		m_pCMEE->SubmitTradeRequest(&trade_mes);
#else
		m_Socket->dbt5Send(reinterpret_cast<void *>(&trade_mes),
				sizeof(TTradeRequest));
#endif
	} catch (CSocketErr *pErr) {
		m_Socket->dbt5Disconnect();	// close connection
		timeval t1;
		gettimeofday(&t1, NULL);
		ostringstream osErr;
		osErr << "Cannot send to market, time "<<t1.tv_sec<<", "<<t1.tv_usec<<", m_Socket = "<<m_Socket->getSocketFd() << endl <<
				"Error: " << pErr->ErrorText() <<
				" at CSendToMarket::SendToMarket" << endl;
		delete pErr;
		LogErrorMessage(osErr.str());
		return false;
	}

	return true;
}

// LogErrorMessage
void CSendToMarket::LogErrorMessage(const string sErr)
{
	m_LogLock.lock();
	cout << sErr;
	*(m_pfLog) << sErr;
	m_pfLog->flush();
	m_LogLock.unlock();
}
