/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * 25 July 2006
 */

#include "BrokerageHouse.h"
#include "CommonStructs.h"
#include "DBConnection.h"

#include "BrokerVolumeDB.h"
#include "CustomerPositionDB.h"
#include "DataMaintenanceDB.h"
#include "MarketFeedDB.h"
#include "MarketWatchDB.h"
#include "SecurityDetailDB.h"
#include "TradeCleanupDB.h"
#include "TradeLookupDB.h"
#include "TradeOrderDB.h"
#include "TradeResultDB.h"
#include "TradeStatusDB.h"
#include "TradeUpdateDB.h"

#include <signal.h>
#include <sys/time.h>

extern CDBConnection* pDBClist[1024];
extern timeval t_start_values[1024];
extern int connectionCnt;

void signal_kill_handler(int signum){
/*	for(int i=0; i<connectionCnt; i++){
			CDBConnection* ptr = (CDBConnection*)pDBClist[i];
			profile_node *cur = ptr->head;
			double total_exec = 0;
			while(cur!=NULL){
					double exec_time = difftimeval(cur->end, cur->start);
					ptr->outfile<<"start=( "<<cur->start.tv_sec<<" "<<cur->start.tv_usec<<" ), end=( "<<cur->end.tv_sec<<" "<<cur->end.tv_usec<<" ), "<<exec_time<<endl;
					cur = cur->next;
					total_exec += exec_time;
			}
			ptr->outfile<<endl<<endl;
			ptr->outfile<<"total exec: "<<total_exec<<endl;
			ptr->outfile.flush();
	}*/
	timeval t2;
	gettimeofday(&t2, NULL);
	for(int i=0; i<connectionCnt; i++){
			CDBConnection* ptr = (CDBConnection*)pDBClist[i];
			double timerange = difftimeval(t2, t_start_values[i]);
			cout<<"txn cnt for connection "<<i<<": "<<ptr->txn_cnt<<", timerange = "<<timerange<<endl;
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
#ifndef NO_DEBUG_INFO
		ostringstream msg2;
		msg2<<"workerThread "<<pThrParam->t_id<<", start, iSockfd = "<<pThrParam->iSockfd<<endl;
		pThrParam->pBrokerageHouse->logErrorMessage(msg2.str());
#endif
		PMsgDriverBrokerage pMessage = new TMsgDriverBrokerage;
		memset(pMessage, 0, sizeof(TMsgDriverBrokerage)); // zero the structure

		TMsgBrokerageDriver Reply; // return message
		INT32 iRet = 0; // transaction return code
		CDBConnection *pDBConnection = NULL;

#ifdef DB_PGSQL
		// new database connection
		pDBConnection = new CDBConnection(
				pThrParam->pBrokerageHouse->m_szHost,
				pThrParam->pBrokerageHouse->m_szDBName,
				pThrParam->pBrokerageHouse->m_szDBPort);
#else
	pDBConnection = new CDBConnection(
			pThrParam->pBrokerageHouse,
			pThrParam->pBrokerageHouse->mysql_dbname,
			pThrParam->pBrokerageHouse->mysql_host,
			pThrParam->pBrokerageHouse->mysql_user,
			pThrParam->pBrokerageHouse->mysql_pass,
			pThrParam->pBrokerageHouse->mysql_port_t,
			pThrParam->pBrokerageHouse->mysql_socket_t);
			pDBClist[pThrParam->t_id] = pDBConnection;
#ifdef CAL_RESP_TIME
			pDBConnection->init_profile_node(pThrParam->t_id, pThrParam->outputDir);
#endif
#endif


		pDBConnection->setBrokerageHouse(pThrParam->pBrokerageHouse);
		CSendToMarket sendToMarket = CSendToMarket(
				&(pThrParam->pBrokerageHouse->m_fLog));
#ifdef NO_MEE_FOR_TRADERESULT
		sendToMarket.m_pCMEE = pThrParam->pBrokerageHouse->m_pCMEE[pThrParam->t_id%pThrParam->pBrokerageHouse->iUsers];
#endif
		CMarketFeedDB marketFeedDB(pDBConnection);
		CMarketFeed marketFeed = CMarketFeed(&marketFeedDB, &sendToMarket);
		CTradeOrderDB tradeOrderDB(pDBConnection);
		CTradeOrder tradeOrder = CTradeOrder(&tradeOrderDB, &sendToMarket);

		// Initialize all classes that will be used to execute transactions.
		CBrokerVolumeDB brokerVolumeDB(pDBConnection);
		CBrokerVolume brokerVolume = CBrokerVolume(&brokerVolumeDB);
		CCustomerPositionDB customerPositionDB(pDBConnection);
		CCustomerPosition customerPosition = CCustomerPosition(&customerPositionDB);
		CMarketWatchDB marketWatchDB(pDBConnection);
		CMarketWatch marketWatch = CMarketWatch(&marketWatchDB);
		CSecurityDetailDB securityDetailDB = CSecurityDetailDB(pDBConnection);
		CSecurityDetail securityDetail = CSecurityDetail(&securityDetailDB);
		CTradeLookupDB tradeLookupDB(pDBConnection);
		CTradeLookup tradeLookup = CTradeLookup(&tradeLookupDB);
		CTradeStatusDB tradeStatusDB(pDBConnection);
		CTradeStatus tradeStatus = CTradeStatus(&tradeStatusDB);
		CTradeUpdateDB tradeUpdateDB(pDBConnection);
		CTradeUpdate tradeUpdate = CTradeUpdate(&tradeUpdateDB);
		CDataMaintenanceDB dataMaintenanceDB(pDBConnection);
		CDataMaintenance dataMaintenance = CDataMaintenance(&dataMaintenanceDB);
		CTradeCleanupDB tradeCleanupDB(pDBConnection);
		CTradeCleanup tradeCleanup = CTradeCleanup(&tradeCleanupDB);
		CTradeResultDB tradeResultDB(pDBConnection);
		CTradeResult tradeResult = CTradeResult(&tradeResultDB);

		int txn_cnt = 0;
		double txn_time = 0;
		bool commit = true;
		double receiving_time = 0;
//		gettimeofday(&(pDBConnection->t1), NULL);
		do {
			try {
				//gettimeofday(&tt1, NULL);
				sockDrv.dbt5Receive(reinterpret_cast<void *>(pMessage),
						sizeof(TMsgDriverBrokerage));
				//gettimeofday(&tt2, NULL);
				//if(txn_cnt > 0 && difftimeval(tt2, tt1)>1)pDBConnection->outfile<<"END"<<endl;
				//pDBConnection->outfile.flush();
			} catch(CSocketErr *pErr) {
				sockDrv.dbt5Disconnect();

				ostringstream osErr;
				osErr << "Error on Receive: " << pErr->ErrorText() <<
						" at BrokerageHouse::workerThread" << endl;
				pThrParam->pBrokerageHouse->logErrorMessage(osErr.str());
				delete pErr;

				// The socket has been closed, break and let this thread die.
				break;
			}
loop:
#ifdef CAL_RESP_TIME
			timeval t1, t2;
			double exec_time;
		 	gettimeofday(&t1, NULL);
#endif

			commit = true;
			iRet = CBaseTxnErr::SUCCESS;
			try {
				//  Parse Txn type
				switch (pMessage->TxnType) {

				/*case BROKER_VOLUME:
					iRet = pThrParam->pBrokerageHouse->RunBrokerVolume(
							&(pMessage->TxnInput.BrokerVolumeTxnInput),
							brokerVolume);
					break;
				case CUSTOMER_POSITION:
					iRet = pThrParam->pBrokerageHouse->RunCustomerPosition(
							&(pMessage->TxnInput.CustomerPositionTxnInput),
							customerPosition);
					break;
				case MARKET_FEED:
					iRet = pThrParam->pBrokerageHouse->RunMarketFeed(
							&(pMessage->TxnInput.MarketFeedTxnInput), marketFeed);
					break;
				case MARKET_WATCH:
					iRet = pThrParam->pBrokerageHouse->RunMarketWatch(
							&(pMessage->TxnInput.MarketWatchTxnInput), marketWatch);
					break;
				case SECURITY_DETAIL:
					iRet = pThrParam->pBrokerageHouse->RunSecurityDetail(
							&(pMessage->TxnInput.SecurityDetailTxnInput),
							securityDetail);
					break;
				case TRADE_LOOKUP:
					iRet = pThrParam->pBrokerageHouse->RunTradeLookup(
							&(pMessage->TxnInput.TradeLookupTxnInput), tradeLookup);
					break;*/
				case TRADE_ORDER:
					iRet = pThrParam->pBrokerageHouse->RunTradeOrder(
							&(pMessage->TxnInput.TradeOrderTxnInput), tradeOrder);
					break;
				case TRADE_RESULT:
//					iRet = pThrParam->pBrokerageHouse->RunTradeResult(
//							&(pMessage->TxnInput.TradeResultTxnInput), tradeResult);
					break;
				//case TRADE_STATUS:
				//	iRet = pThrParam->pBrokerageHouse->RunTradeStatus(
				//			&(pMessage->TxnInput.TradeStatusTxnInput),
				//			tradeStatus);
				//	break;
				case TRADE_UPDATE:
					iRet = pThrParam->pBrokerageHouse->RunTradeUpdate(
							&(pMessage->TxnInput.TradeUpdateTxnInput), tradeUpdate);
					break;
				/*case DATA_MAINTENANCE:
					iRet = pThrParam->pBrokerageHouse->RunDataMaintenance(
							&(pMessage->TxnInput.DataMaintenanceTxnInput),
							dataMaintenance);
					break;
				case TRADE_CLEANUP:
					iRet = pThrParam->pBrokerageHouse->RunTradeCleanup(
							&(pMessage->TxnInput.TradeCleanupTxnInput),
							tradeCleanup);
					break;*/
				default:
					//cout << "wrong txn type" << endl;
					iRet = ERR_TYPE_WRONGTXN;
				}
					txn_cnt++;
					pDBConnection->txn_cnt = txn_cnt;
					if(txn_cnt==1)gettimeofday(&(t_start_values[pThrParam->t_id]), NULL);
			} catch (const char *str) {

			pDBConnection->rollback();
#ifdef CAL_RESP_TIME
			gettimeofday(&t2, NULL);
			exec_time = difftimeval(t2, t1);
			txn_time += exec_time;
			//pDBConnection->append_profile_node(t1, t2, pMessage->TxnType, false);
			pDBConnection->outfile<<"error: "<<str<<endl;
//#ifdef PROFILE_EACH_QUERY
//			pDBConnection->print_profile_query();
//#endif
			pDBConnection->outfile.flush();
#endif
				//ostringstream msg;
				//msg << time(NULL) << " " << (long long) pthread_self() << " " <<
				//		szTransactionName[pMessage->TxnType] << "; "<<str<<endl;
				//pThrParam->pBrokerageHouse->logErrorMessage(msg.str());
				iRet = CBaseTxnErr::EXPECTED_ROLLBACK;

				commit = false;
				//XXX:debug for trade result
			}
#ifdef CAL_RESP_TIME
			gettimeofday(&t2, NULL);
			exec_time = difftimeval(t2, t1);
			txn_time += exec_time;

//			pDBConnection->append_profile_node(t1, t2, pMessage->TxnType, true);
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

				ostringstream osErr;
				osErr << "Error on Send: " << pErr->ErrorText() <<
						" at BrokerageHouse::workerThread" << endl;
				pThrParam->pBrokerageHouse->logErrorMessage(osErr.str());
				delete pErr;

				// The socket has been closed, break and let this thread die.
				break;
			}
		} while (true);

//		delete pDBConnection; // close connection with the database
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
#ifndef NO_DEBUG_INFO
		ostringstream msg;
		msg<<"entryWorkerThread, start initiate worker thread"<<endl;
		pThrParam->pBrokerageHouse->logErrorMessage(msg.str());
#endif
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

		ostringstream osErr;
		osErr << "Error: " << pErr->ErrorText() << " at " <<
				"BrokerageHouse::entryWorkerThread" << endl <<
				"accepted socket connection closed" << endl;
		pThrParam->pBrokerageHouse->logErrorMessage(osErr.str());
		delete pThrParam;
		delete pErr;
	}
}

// Constructor
#ifdef DB_PGSQL
CBrokerageHouse::CBrokerageHouse(const char *szHost, const char *szDBName,
		const char *szDBPort, const int iListenPort, char *outputDirectory)
: m_iListenPort(iListenPort)
{
	strncpy(m_szHost, szHost, iMaxHostname);
	m_szHost[iMaxHostname] = '\0';
	strncpy(m_szDBName, szDBName, iMaxDBName);
	m_szDBName[iMaxDBName] = '\0';
	strncpy(m_szDBPort, szDBPort, iMaxPort);
	m_szDBPort[iMaxPort] = '\0';
#else
#ifdef NO_MEE_FOR_TRADERESULT
CBrokerageHouse::CBrokerageHouse(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *_outputDirectory, char* _szFileLoc, char* _szBHaddr, int _iActiveCustomerCount, int _iConfiguredCustomerCount, int _iUsers)
#else
CBrokerageHouse::CBrokerageHouse(char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket, const int iListenPort, char *outputDirectory)
#endif
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

#endif

	char filename[iMaxPath + 1];
	snprintf(filename, iMaxPath, "%s/BrokerageHouse_Error.log",
			outputDirectory);
	char s[1000];
	m_fLog.open(filename, ios::out);
#ifdef NO_MEE_FOR_TRADERESULT
	strcpy(outputDir, _outputDirectory);
#else
	strcpy(outputDir, outputDirectory);
#endif
	logErrorMessage(s, false);
#ifdef NO_MEE_FOR_TRADERESULT
	szFileLoc = _szFileLoc;
	szBHaddr = _szBHaddr;
	outputDirectory = _outputDirectory;
	iActiveCustomerCount = _iActiveCustomerCount;
	iConfiguredCustomerCount = _iConfiguredCustomerCount;
	iUsers = _iUsers;
#endif
}

#ifdef NO_MEE_FOR_TRADERESULT
void CBrokerageHouse::startFakeMEE(){
	char filename[iMaxPath + 1];

	snprintf(filename, iMaxPath, "%s/MarketExchange.log", outputDirectory);
	m_pLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);

	snprintf(filename, iMaxPath, "%s/%s", outputDirectory, "mee_mix.log");
	ofstream m_fMix;
	m_fMix.open(filename, ios::out);

	CInputFiles inputFiles;
	inputFiles.Initialize(eDriverEGenLoader, iConfiguredCustomerCount,
			iActiveCustomerCount, szFileLoc);
	for(int i=0; i<iUsers; i++){
		m_pCMEESUT[i] = new CMEESUT(szBHaddr, iBrokerageHousePort, &m_fLog, &m_fMix,
					&m_meeLogLock[i], &m_MixLock[i]);
		m_pCMEE[i] = new CMEE(0, m_pCMEESUT[i], m_pLog, inputFiles, 1);
		m_pCMEE[i]->SetBaseTime();
	}
}
#endif

// Destructor
CBrokerageHouse::~CBrokerageHouse()
{
	m_Socket.closeListenerSocket();
	m_fLog.close();
}

void CBrokerageHouse::dumpInputData(PBrokerVolumeTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << endl;
	for (int i = 0; i < max_broker_list_len; i++) {
		msg << "broker_list[" << i << "] = " << pTxnInput->broker_list[i] <<
				endl;
		msg << "sector_name[" << i << "] = " << pTxnInput->sector_name[i] <<
				endl;
	}
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PCustomerPositionTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() <<
			" CustomerPosition" << endl;
	msg << "acct_id_idx = " << pTxnInput->acct_id_idx << endl;
	msg << "cust_id = " << pTxnInput->cust_id << endl;
	msg << "get_history = " << pTxnInput->get_history << endl;
	msg << "tax_id = " << pTxnInput->tax_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PDataMaintenanceTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() <<
			" DataMaintenance" << endl;
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	msg << "c_id = " << pTxnInput->c_id << endl;
	msg << "co_id = " << pTxnInput->co_id << endl;
	msg << "day_of_month = " << pTxnInput->day_of_month << endl;
	msg << "vol_incr = " << pTxnInput->vol_incr << endl;
	msg << "symbol = " << pTxnInput->symbol << endl;
	msg << "table_name = " << pTxnInput->table_name << endl;
	msg << "tx_id = " << pTxnInput->tx_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeCleanupTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() <<
			" TradeCleanup" << endl;
	msg << "start_trade_id = " << pTxnInput->start_trade_id << endl;
	msg << "st_canceled_id = " << pTxnInput->st_canceled_id << endl;
	msg << "st_pending_id = " << pTxnInput->st_pending_id << endl;
	msg << "st_submitted_id = " << pTxnInput->st_submitted_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PMarketWatchTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " MarketWatch" <<
			endl;
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	msg << "c_id = " << pTxnInput->c_id << endl;
	msg << "ending_co_id = " << pTxnInput->ending_co_id << endl;
	msg << "starting_co_id = " << pTxnInput->starting_co_id << endl;
	msg << "start_day = " << pTxnInput->start_day.year << "-" <<
			pTxnInput->start_day.month << "-" <<
			pTxnInput->start_day.day << " " << pTxnInput->start_day.hour <<
			":" << pTxnInput->start_day.minute << ":" <<
			pTxnInput->start_day.second << "." <<
			pTxnInput->start_day.fraction << endl;
	msg << "industry_name = " << pTxnInput->industry_name << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PMarketFeedTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " MarketFeed" <<
			endl;
	msg << "StatusAndTradeType.status_submitted = " <<
			pTxnInput->StatusAndTradeType.status_submitted << endl;
	msg << "StatusAndTradeType.type_limit_buy = " <<
			pTxnInput->StatusAndTradeType.type_limit_buy << endl;
	msg << "StatusAndTradeType.type_limit_sell = " <<
			pTxnInput->StatusAndTradeType .type_limit_sell << endl;
	msg << "StatusAndTradeType.type_stop_loss = " <<
			pTxnInput->StatusAndTradeType .type_stop_loss << endl;
	msg << "zz_padding1 = " << pTxnInput->zz_padding1 << endl;
	msg << "zz_padding2 = " << pTxnInput->zz_padding2 << endl;
	for (int i = 0; i < max_feed_len; i++) {
		msg << "Entries[" << i << "].price_quote = " <<
				pTxnInput->Entries[i].price_quote << endl;
		msg << "Entries[" << i << "].trade_qty = " <<
				pTxnInput->Entries[i].trade_qty << endl;
		msg << "Entries[" << i << "].symbol = " <<
				pTxnInput->Entries[i].symbol << endl;
	}
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PSecurityDetailTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() <<
			" SecurityDetail" << endl;
	msg << "max_rows_to_return = " << pTxnInput->max_rows_to_return << endl;;
	msg << "access_lob_flag = " << pTxnInput->access_lob_flag << endl;;
	msg << "start_day = " << pTxnInput->start_day.year << "-" <<
			pTxnInput->start_day.month << "-" <<
			pTxnInput->start_day.day << " " << pTxnInput->start_day.hour <<
			":" << pTxnInput->start_day.minute << ":" <<
			pTxnInput->start_day.second << "." <<
			pTxnInput->start_day.fraction << endl;
	msg << "symbol = " << pTxnInput->symbol << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeStatusTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " TradeStatus" <<
			endl;
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeLookupTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " TradeLookup" <<
			endl;
	for (int i = 0; i < TradeLookupFrame1MaxRows; i++) {
		msg << "trade_id[" << i << "] = " << pTxnInput->trade_id[i] << endl;
	}
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	msg << "max_acct_id = " << pTxnInput->max_acct_id << endl;
	msg << "frame_to_execute = " << pTxnInput->frame_to_execute << endl;
	msg << "max_trades = " << pTxnInput->max_trades << endl;
	msg << "end_trade_dts = " << pTxnInput->end_trade_dts.year << "-" <<
			pTxnInput->end_trade_dts.month << "-" <<
			pTxnInput->end_trade_dts.day << " " <<
			pTxnInput->end_trade_dts.hour <<
			":" << pTxnInput->end_trade_dts.minute << ":" <<
			pTxnInput->end_trade_dts.second << "." <<
			pTxnInput->end_trade_dts.fraction << endl;
	msg << "start_trade_dts = " << pTxnInput->start_trade_dts.year << "-" <<
			pTxnInput->start_trade_dts.month << "-" <<
			pTxnInput->start_trade_dts.day << " " <<
			pTxnInput->start_trade_dts.hour <<
			":" << pTxnInput->start_trade_dts.minute << ":" <<
			pTxnInput->start_trade_dts.second << "." <<
			pTxnInput->start_trade_dts.fraction << endl;
	msg << "symbol = " << pTxnInput->symbol << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeOrderTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " TradeOrder" <<
			endl;
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	msg << "is_lifo = " << pTxnInput->is_lifo << endl;
	msg << "roll_it_back = " << pTxnInput->roll_it_back << endl;
	msg << "trade_qty = " << pTxnInput->trade_qty << endl;
	msg << "type_is_margin = " << pTxnInput->type_is_margin << endl;
	msg << "co_name = " << pTxnInput->co_name << endl;
	msg << "exec_f_name = " << pTxnInput->exec_f_name << endl;
	msg << "exec_l_name = " << pTxnInput->exec_l_name << endl;
	msg << "exec_tax_id = " << pTxnInput->exec_tax_id << endl;
	msg << "issue = " << pTxnInput->issue << endl;
	msg << "st_pending_id = " << pTxnInput->st_pending_id << endl;
	msg << "st_submitted_id = " << pTxnInput->st_submitted_id << endl;
	msg << "symbol = " << pTxnInput->symbol << endl;
	msg << "trade_type_id = " << pTxnInput->trade_type_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeResultTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " TradeResult" <<
			endl;
	msg << "trade_price = " << pTxnInput->trade_price << endl;
	msg << "trade_id = " << pTxnInput->trade_id << endl;
	logErrorMessage(msg.str(), false);
}

void CBrokerageHouse::dumpInputData(PTradeUpdateTxnInput pTxnInput)
{
	ostringstream msg;
	msg << time(NULL) << " " << (long long) pthread_self() << " TradeUpdate" <<
			endl;
	for (int i = 0; i < TradeUpdateFrame1MaxRows; i++) {
		msg << "trade_id[" << i << "] = " << pTxnInput->trade_id[i] << endl;
	}
	msg << "acct_id = " << pTxnInput->acct_id << endl;
	msg << "max_acct_id = " << pTxnInput->max_acct_id << endl;
	msg << "frame_to_execute = " << pTxnInput->frame_to_execute << endl;
	msg << "max_trades = " << pTxnInput->max_trades << endl;
	msg << "trade_id = " << pTxnInput->trade_id << endl;
	msg << "end_trade_dts = " << pTxnInput->end_trade_dts.year << "-" <<
			pTxnInput->end_trade_dts.month << "-" <<
			pTxnInput->end_trade_dts.day << " " <<
			pTxnInput->end_trade_dts.hour <<
			":" << pTxnInput->end_trade_dts.minute << ":" <<
			pTxnInput->end_trade_dts.second << "." <<
			pTxnInput->end_trade_dts.fraction << endl;
	msg << "start_trade_dts = " << pTxnInput->start_trade_dts.year << "-" <<
			pTxnInput->start_trade_dts.month << "-" <<
			pTxnInput->start_trade_dts.day << " " <<
			pTxnInput->start_trade_dts.hour <<
			":" << pTxnInput->start_trade_dts.minute << ":" <<
			pTxnInput->start_trade_dts.second << "." <<
			pTxnInput->start_trade_dts.fraction << endl;
	msg << "symbol = " << pTxnInput->symbol << endl;
	logErrorMessage(msg.str(), false);
}

// Run Broker Volume transaction
INT32 CBrokerageHouse::RunBrokerVolume(PBrokerVolumeTxnInput pTxnInput,
		CBrokerVolume &brokerVolume)
{
	TBrokerVolumeTxnOutput bvOutput;
	memset(&bvOutput, 0, sizeof(TBrokerVolumeTxnOutput));

	try {
		brokerVolume.DoTxn(pTxnInput, &bvOutput);
	} catch (const exception &e) {
		logErrorMessage("BV EXCEPTION\n", false);
		bvOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (bvOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << bvOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return bvOutput.status;
}

// Run Customer Position transaction
INT32 CBrokerageHouse::RunCustomerPosition(PCustomerPositionTxnInput pTxnInput,
		CCustomerPosition &customerPosition)
{
	TCustomerPositionTxnOutput cpOutput;
	memset(&cpOutput, 0, sizeof(TCustomerPositionTxnOutput));

	try {
		customerPosition.DoTxn(pTxnInput, &cpOutput);
	} catch (const exception &e) {
		logErrorMessage("CP EXCEPTION\n", false);
		cpOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (cpOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << cpOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return cpOutput.status;
}

// Run Data Maintenance transaction
INT32 CBrokerageHouse::RunDataMaintenance(PDataMaintenanceTxnInput pTxnInput,
		CDataMaintenance &dataMaintenance)
{
	TDataMaintenanceTxnOutput dmOutput;
	memset(&dmOutput, 0, sizeof(TDataMaintenanceTxnOutput));

	try {
		dataMaintenance.DoTxn(pTxnInput, &dmOutput);
	} catch (const exception &e) {
		logErrorMessage("DM EXCEPTION\n", false);
		dmOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (dmOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << dmOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return dmOutput.status;
}

// Run Trade Cleanup transaction
INT32 CBrokerageHouse::RunTradeCleanup(PTradeCleanupTxnInput pTxnInput,
		CTradeCleanup &tradeCleanup)
{
	TTradeCleanupTxnOutput tcOutput;
	memset(&tcOutput, 0, sizeof(TTradeCleanupTxnOutput));

	try {
		tradeCleanup.DoTxn(pTxnInput, &tcOutput);
	} catch (const exception &e) {
		logErrorMessage("TC EXCEPTION\n", false);
		tcOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (tcOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << tcOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return tcOutput.status;
}

// Run Market Feed transaction
INT32 CBrokerageHouse::RunMarketFeed(PMarketFeedTxnInput pTxnInput,
		CMarketFeed &marketFeed)
{
	TMarketFeedTxnOutput mfOutput;
	memset(&mfOutput, 0, sizeof(TMarketFeedTxnOutput));

	try {
		marketFeed.DoTxn(pTxnInput, &mfOutput);
	} catch (const exception &e) {
		logErrorMessage("MF EXCEPTION\n", false);
		mfOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (mfOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << mfOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return mfOutput.status;
}

// Run Market Watch transaction
INT32 CBrokerageHouse::RunMarketWatch(PMarketWatchTxnInput pTxnInput,
		CMarketWatch &marketWatch)
{
	TMarketWatchTxnOutput mwOutput;
	memset(&mwOutput, 0, sizeof(TMarketWatchTxnOutput));

	try {
		marketWatch.DoTxn(pTxnInput, &mwOutput);
	} catch (const exception &e) {
		logErrorMessage("MW EXCEPTION\n", false);
		mwOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (mwOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << mwOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return mwOutput.status;
}

// Run Security Detail transaction
INT32 CBrokerageHouse::RunSecurityDetail(PSecurityDetailTxnInput pTxnInput,
		CSecurityDetail &securityDetail)
{
	TSecurityDetailTxnOutput sdOutput;
	memset(&sdOutput, 0, sizeof(TSecurityDetailTxnOutput));

	try {
		securityDetail.DoTxn(pTxnInput, &sdOutput);
	} catch (const exception &e) {
		logErrorMessage("SD EXCEPTION\n", false);
		sdOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (sdOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << sdOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return sdOutput.status;
}

// Run Trade Lookup transaction
INT32 CBrokerageHouse::RunTradeLookup(PTradeLookupTxnInput pTxnInput,
		CTradeLookup &tradeLookup)
{
	TTradeLookupTxnOutput tlOutput;
	memset(&tlOutput, 0, sizeof(TTradeLookupTxnOutput));

	try {
		tradeLookup.DoTxn(pTxnInput, &tlOutput);
	} catch (const exception &e) {
		logErrorMessage("TL EXCEPTION\n", false);
		tlOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (tlOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << tlOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return tlOutput.status;
}

// Run Trade Order transaction
INT32 CBrokerageHouse::RunTradeOrder(PTradeOrderTxnInput pTxnInput,
		CTradeOrder &tradeOrder)
{
	TTradeOrderTxnOutput toOutput;
	memset(&toOutput, 0, sizeof(TTradeOrderTxnOutput));

	try {
		tradeOrder.DoTxn(pTxnInput, &toOutput);
	} catch (const exception &e) {
		toOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (toOutput.status != CBaseTxnErr::SUCCESS &&
	    !(toOutput.status == CBaseTxnErr::EXPECTED_ROLLBACK && pTxnInput->roll_it_back)) {
		ostringstream msg;
		msg << __FILE__ << " (error) " << __LINE__ << " " << toOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return toOutput.status;
}

// Run Trade Result transaction
INT32 CBrokerageHouse::RunTradeResult(PTradeResultTxnInput pTxnInput,
		CTradeResult &tradeResult)
{
	TTradeResultTxnOutput trOutput;
	memset(&trOutput, 0, sizeof(TTradeResultTxnOutput));

	try {
		tradeResult.DoTxn(pTxnInput, &trOutput);
	} catch (const exception &e) {
		logErrorMessage("TR EXCEPTION\n", false);
		trOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (trOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << trOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return trOutput.status;
}

// Run Trade Status transaction
INT32 CBrokerageHouse::RunTradeStatus(PTradeStatusTxnInput pTxnInput,
		CTradeStatus &tradeStatus)
{
	TTradeStatusTxnOutput tsOutput;
	memset(&tsOutput, 0, sizeof(TTradeStatusTxnOutput));

	try {
		tradeStatus.DoTxn(pTxnInput, &tsOutput);
	} catch (const exception &e) {
		logErrorMessage("TS EXCEPTION\n", false);
		tsOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (tsOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " input: "<<pTxnInput->acct_id << " " << tsOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return tsOutput.status;
}

// Run Trade Update transaction
INT32 CBrokerageHouse::RunTradeUpdate(PTradeUpdateTxnInput pTxnInput,
		CTradeUpdate &tradeUpdate)
{
	TTradeUpdateTxnOutput tuOutput;
	memset(&tuOutput, 0, sizeof(TTradeUpdateTxnOutput));

	try {
		tradeUpdate.DoTxn(pTxnInput, &tuOutput);
	} catch (const exception &e) {
		logErrorMessage("TU EXCEPTION\n", false);
		tuOutput.status = CBaseTxnErr::EXPECTED_ROLLBACK;
	}

	if (tuOutput.status != CBaseTxnErr::SUCCESS) {
		ostringstream msg;
		msg << __FILE__ << " " << __LINE__ << " " << tuOutput.status << endl;
		logErrorMessage(msg.str(), false);
		dumpInputData(pTxnInput);
	}
	return tuOutput.status;
}

// Listener
void CBrokerageHouse::startListener(void)
{
	int acc_socket;
	PThreadParameter pThrParam = NULL;

	m_Socket.dbt5Listen(m_iListenPort);
	ostringstream msg1;
	msg1<<"STARTLISTENER: m_iListenPort = "<<m_iListenPort<<endl;
	logErrorMessage(msg1.str(), false);

#ifdef NO_MEE_FOR_TRADERESULT
	startFakeMEE();
#endif
	int t_cnt = 0;
	while (true) {
		acc_socket = 0;
		try {
			acc_socket = m_Socket.dbt5Accept();

			pThrParam = new TThreadParameter;
			// zero the structure
			memset(pThrParam, 0, sizeof(TThreadParameter));

			pThrParam->iSockfd = acc_socket;
			pThrParam->pBrokerageHouse = this;
			pThrParam->t_id = t_cnt;
			strcpy(pThrParam->outputDir, outputDir);
			t_cnt++;
			connectionCnt = t_cnt;
#ifndef NO_DEBUG_INFO
			ostringstream msg;
			msg<<"connectionCnt = "<<t_cnt<<", socket = "<<acc_socket<<endl;
			logErrorMessage(msg.str(), false);
#endif
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
void CBrokerageHouse::logErrorMessage(const string sErr, bool bScreen)
{
//	m_LogLock.lock();
//	if (bScreen) cout << sErr;
//	m_fLog << sErr;
//	m_fLog.flush();
//	m_LogLock.unlock();
}
