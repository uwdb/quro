/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2010 Rilson Nascimento
 *
 * Base class for emulator-SUT interface
 * 13 August 2006
 */

#ifndef BASE_INTERFACE_H
#define BASE_INTERFACE_H

#include "locking.h"

#include "CommonStructs.h"
#include "CSocket.h"

#ifndef WORKLOAD_TPCE
#include <thread>
#include <mutex>
#endif

using namespace TPCE;

class CBaseInterface
{
protected:
#ifdef WORKLOAD_TPCE
	bool talkToSUT(PMsgDriverBrokerage);
#elif WORKLOAD_SEATS
	bool talkToSUT(PMsgDriverSeats);
#elif WORKLOAD_BID
	bool talkToSUT(PMsgDriverBid);
#endif
	void logErrorMessage(const string);

	char *m_szBHAddress;
	int m_iBHlistenPort;
#ifdef WORKLOAD_TPCE
	CMutex *m_pLogLock;
	CMutex *m_pMixLock;
#else
	mutex *m_pLogLock;
	mutex *m_pMixLock;
#endif
	ofstream *m_pfLog; // error log file
	ofstream *m_pfMix; // mix log file

private:
	CSocket	*sock;
	void logResponseTime(int, int, double);

public:
#ifdef WORKLOAD_TPCE
	CBaseInterface(char *, const int, ofstream *, ofstream *, CMutex *,
			CMutex *);
#else
	CBaseInterface(char *, const int, ofstream *, ofstream *, mutex *,
			mutex *);
#endif
	~CBaseInterface(void);
	bool biConnect();
	bool biDisconnect();
};

#endif // BASE_INTERFACE_H
