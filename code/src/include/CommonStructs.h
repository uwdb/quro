/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2010 Mark Wong
 *
 * 29 July 2006
 */

#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include "DateTime.h"

using namespace TPCE;
#ifdef WORKLOAD_TPCE
#include "CE.h"

// Transaction types
enum eTxnType
{
	NULL_TXN = -1,
	SECURITY_DETAIL,
	BROKER_VOLUME,
	CUSTOMER_POSITION,
	MARKET_WATCH,
	TRADE_STATUS,
	TRADE_LOOKUP,
	TRADE_ORDER,
	TRADE_UPDATE,
	MARKET_FEED,
	TRADE_RESULT,
	DATA_MAINTENANCE,
	TRADE_CLEANUP
};

// structure of the message Driver --> Brokerage House
typedef struct TMsgDriverBrokerage
{
	eTxnType	TxnType;
	union
	{
		TBrokerVolumeTxnInput		BrokerVolumeTxnInput;
		TCustomerPositionTxnInput	CustomerPositionTxnInput;
		TMarketFeedTxnInput		MarketFeedTxnInput;
		TMarketWatchTxnInput		MarketWatchTxnInput;
		TTradeLookupTxnInput		TradeLookupTxnInput;
		TSecurityDetailTxnInput		SecurityDetailTxnInput;
		TTradeOrderTxnInput		TradeOrderTxnInput;
		TTradeResultTxnInput		TradeResultTxnInput;
		TTradeStatusTxnInput		TradeStatusTxnInput;
		TTradeUpdateTxnInput		TradeUpdateTxnInput;
		TDataMaintenanceTxnInput	DataMaintenanceTxnInput;
		TTradeCleanupTxnInput		TradeCleanupTxnInput;
	} TxnInput;
} *PMsgDriverBrokerage;

// structure of the message Brokerage House --> Driver
typedef struct TMsgBrokerageDriver
{
	int			iStatus;
} *PMsgBrokerageDriver;
#elif WORKLOAD_SEATS
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
enum eTxnType
{
		NULL_TXN = -1,
		FIND_FLIGHT,
		NEW_RESERVATION,
		UPDATE_CUSTOMER,
		UPDATE_RESERVATION
};

typedef struct TFindFlightTxnInput
{
		long unsigned int depart_aid;
		long unsigned int arrive_aid;
		TIMESTAMP_STRUCT start_date;
		TIMESTAMP_STRUCT end_date;
		long unsigned int distance;
};

typedef struct TNewReservationTxnInput
{
		long unsigned int r_id;
		long unsigned int c_id;
		long unsigned int f_id;
		long unsigned int seatnum;
		double price;
		TIMESTAMP_STRUCT ts;
};

typedef struct TUpdateCustomerTxnInput
{
		long unsigned int c_id;
		char* c_id_str;
		long unsigned int update_ff;
};

typedef struct TUpdateReservationTxnInput
{
		long unsigned int r_id;
		long unsigned int c_id;
		long unsigned int seatnum;
		char* attr_idx;
		long unsigned int attr_val;
};

typedef struct TFindFlightTxnOutput
{
		size_t num_results;
		long unsigned int depart_ap_code;
		char depart_ap_city[64];
		long unsigned int depart_ap_country;
		char depart_ap_name[128];
		long unsigned int arrive_ap_code[3];
		char arrive_ap_city[3][64];
		long unsigned int arrive_ap_country[3];
		char arrive_ap_name[3][128];
		int status;
};

typedef struct TNewReservationTxnOutput
{
		int status;
};

typedef struct TUpdateCustomerTxnOutput
{
		int status;
};

typedef struct TUpdateReservationTxnOutput
{
		int status;
};

typedef struct TMsgDriverSeats
{
		eTxnType TxnType;
		union
		{
				TFindFlightTxnInput FindFlightTxnInput;
				TNewReservationTxnInput NewReservationTxnInput;
				TUpdateCustomerTxnInput UpdateCustomerTxnInput;
				TUpdateReservationTxnInput UpdateReservationTxnInput;
		}TxnInput;
} *PMsgDriverSeats;

typedef struct TMsgSeatsDriver
{
	int			iStatus;
} *PMsgSeatsDriver;

#endif

#endif	//COMMON_STRUCTS_H
