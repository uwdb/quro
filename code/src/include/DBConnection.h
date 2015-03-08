/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * PostgreSQL connection class
 * 13 June 2006
 */

#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H
#ifdef DB_PGSQL
#include <libpq-fe.h>
#else
#include <mysql.h>
#endif
#include "TxnHarnessStructs.h"
#include "TxnHarnessSendToMarket.h"

#include "BrokerageHouse.h"
#include "DBT5Consts.h"
using namespace TPCE;

//#define LOG_ERROR_MESSAGE(arg...) logErrorMessage(arg...)
#define LOG_ERROR_MESSAGE logErrorMessage

#ifndef DB_PGSQL
struct sql_result_t
{
  MYSQL_RES * result_set;
  MYSQL_ROW current_row;
  unsigned int num_fields;
  unsigned int num_rows;
  unsigned long * lengths;
  char * query;
};
#endif

class CDBConnection
{
private:
#ifdef DB_PGSQL
	PGconn *m_Conn;
#else
	MYSQL *dbc;  //mysql db
	char mysql_dbname[32];
	char mysql_host[32];
	char mysql_user[32];
	char mysql_pass[32];
	char mysql_port_t[32];
	char mysql_socket_t[256];
#endif

	char szConnectStr[iMaxConnectString + 1];
	char name[16];

	CBrokerageHouse *bh;

	TTradeRequest m_TriggeredLimitOrders;


public:
#ifdef DB_PGSQL
	CDBConnection(const char *, const char *, const char *);
#else
	CDBConnection(CBrokerageHouse *bh, char *_mysql_dbname, char *_mysql_host, char * _mysql_user, char * _mysql_pass, char *_mysql_port, char * _mysql_socket);
#endif

	~CDBConnection();

	void begin();

	void commit();
	void connect();
	char *escape(string);
	void disconnect();

	//PGresult *exec(const char *);
	void exec(const char *);

	void execute(const TBrokerVolumeFrame1Input *,
			TBrokerVolumeFrame1Output *);

	void execute(const TCustomerPositionFrame1Input *,
			TCustomerPositionFrame1Output *);
	void execute(const TCustomerPositionFrame2Input *,
			TCustomerPositionFrame2Output *);

	void execute(const TDataMaintenanceFrame1Input *);

	void execute(const TMarketFeedFrame1Input *, TMarketFeedFrame1Output *,
			CSendToMarketInterface *);

	void execute(const TMarketWatchFrame1Input *, TMarketWatchFrame1Output *);

	void execute(const TSecurityDetailFrame1Input *,
			TSecurityDetailFrame1Output *);

	void execute(const TTradeCleanupFrame1Input *);

	void execute(const TTradeLookupFrame1Input *, TTradeLookupFrame1Output *);
	void execute(const TTradeLookupFrame2Input *, TTradeLookupFrame2Output *);
	void execute(const TTradeLookupFrame3Input *, TTradeLookupFrame3Output *);
	void execute(const TTradeLookupFrame4Input *, TTradeLookupFrame4Output *);

	void execute(const TTradeOrderFrame1Input *, TTradeOrderFrame1Output *);
	void execute(const TTradeOrderFrame2Input *, TTradeOrderFrame2Output *);
	void execute(const TTradeOrderFrame3Input *, TTradeOrderFrame3Output *);
	void execute(const TTradeOrderFrame4Input *, TTradeOrderFrame4Output *);

	void execute(const TTradeResultFrame1Input *, TTradeResultFrame1Output *);
	void execute(const TTradeResultFrame2Input *, TTradeResultFrame2Output *);
	void execute(const TTradeResultFrame3Input *, TTradeResultFrame3Output *);
	void execute(const TTradeResultFrame4Input *, TTradeResultFrame4Output *);
	void execute(const TTradeResultFrame5Input *);
	void execute(const TTradeResultFrame6Input *, TTradeResultFrame6Output *);

	void execute(const TTradeStatusFrame1Input *, TTradeStatusFrame1Output *);

	void execute(const TTradeUpdateFrame1Input *, TTradeUpdateFrame1Output *);
	void execute(const TTradeUpdateFrame2Input *, TTradeUpdateFrame2Output *);
	void execute(const TTradeUpdateFrame3Input *, TTradeUpdateFrame3Output *);

	void reconnect();

	void rollback();

	void setBrokerageHouse(CBrokerageHouse *);

	void logErrorMessage(const char* c, ...);
#ifdef DB_PGSQL
	void setReadCommitted();
	void setReadUncommitted();
	void setRepeatableRead();
	void setSerializable();
#else
int dbt5_sql_execute(char * query,
                     sql_result_t * sql_result, char * query_name);
int dbt5_sql_close_cursor(sql_result_t * sql_result);
int dbt5_sql_fetchrow(sql_result_t * sql_result);
char * dbt5_sql_getvalue(sql_result_t * sql_result,
                         int field);
char * dbt5_sql_getvalue(sql_result_t * sql_result, int field, int& length);

#endif
};

#endif //DB_CONNECTION_H
