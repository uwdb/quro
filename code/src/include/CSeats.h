#ifndef CSEATS_H
#define CSEATS_H
#include "CommonStructs.h"
#include "util.h"
#include "BaseInterface.h"
#include <mutex>
#define FIND_FLIGHT_PERC  0.2
#define NEW_RESERVATION_PERC 0.4
#define UPDATE_CUSTOMER_PERC 0.4
#define UPDATE_RESERVATION_PERC 0.2

class CSEATS : public CBaseInterface
{
public:
		TMsgDriverSeats request;
		fast_random r;
		TFindFlightTxnInput ffInput;
		TNewReservationTxnInput nrInput;
		TUpdateCustomerTxnInput ucInput;
		TUpdateReservationTxnInput urInput;

		CSEATS(char* addr, const int iListenPort, ofstream* pflog, ofstream* pfmix,
			mutex* pLogLock, mutex* pMixLock);


		void GenerateFindFlightInput();
		void GenerateNewReservationInput();
		void GenerateUpdateCustomerInput();
		void GenerateUpdateReservationInput();

		bool FindFlight(TFindFlightTxnInput* pTxnInput);
		bool NewReservation(TNewReservationTxnInput* pTxnInput);
		bool UpdateCustomer(TUpdateCustomerTxnInput* pTxnInput);
		bool UpdateReservation(TUpdateReservationTxnInput* pTxnInput);

		void DoTxn();
};

#endif
