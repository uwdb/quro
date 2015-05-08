#include "CSeats.h"
#include "Seats_const.h"

CSEATS::CSEATS(char* addr, const int iListenPort, ofstream* pflog,
		ofstream* pfmix, mutex* pLogLock, mutex* pMixLock)
: CBaseInterface(addr, iListenPort, pflog, pfmix, pLogLock, pMixLock)
{
		long unsigned int rnd = (rand()*rand())%6124908536;
		r.set_seed(rnd);
}


bool CSEATS::FindFlight(TFindFlightTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverSeats));

		request.TxnType = FIND_FLIGHT;
	memcpy(&(request.TxnInput.FindFlightTxnInput), pTxnInput,
			sizeof(request.TxnInput.FindFlightTxnInput));

	return talkToSUT(&request);

}

bool CSEATS::NewReservation(TNewReservationTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverSeats));

		request.TxnType = NEW_RESERVATION;
	memcpy(&(request.TxnInput.NewReservationTxnInput), pTxnInput,
			sizeof(request.TxnInput.NewReservationTxnInput));

	return talkToSUT(&request);

}

bool CSEATS::UpdateCustomer(TUpdateCustomerTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverSeats));

		request.TxnType = UPDATE_CUSTOMER;
	memcpy(&(request.TxnInput.UpdateCustomerTxnInput), pTxnInput,
			sizeof(request.TxnInput.UpdateCustomerTxnInput));

	return talkToSUT(&request);

}

bool CSEATS::UpdateReservation(TUpdateReservationTxnInput* pTxnInput){
		memset(&request, 0, sizeof(struct TMsgDriverSeats));

		request.TxnType = UPDATE_RESERVATION;
	memcpy(&(request.TxnInput.UpdateReservationTxnInput), pTxnInput,
			sizeof(request.TxnInput.UpdateReservationTxnInput));

	return talkToSUT(&request);

}
void CSEATS::GenerateFindFlightInput(){
		ffInput.depart_aid = getRandomAirportId(r);
		ffInput.arrive_aid = getRandomAirportId(r);
		if(r.next_uniform()<PROB_GET_NEARBY_AIRPORT)
				ffInput.distance = getRandomDist(r);
		else
				ffInput.distance = -1;
		ffInput.start_date = GenerateRandomTimestamp(r);
		ffInput.end_date = addDay(ffInput.start_date, r.next()%28+20);
}
void CSEATS::GenerateNewReservationInput(){
		nrInput.r_id = get_random(r, 1048576);
		nrInput.c_id = getCustomerId(r);
		nrInput.f_id = get_random(r, numFlights)+1;
		nrInput.seatnum = 50+get_random(r, 200);
		nrInput.price = r.next_uniform()*500;
}
void CSEATS::GenerateUpdateCustomerInput(){
}
void CSEATS::GenerateUpdateReservationInput(){
}

void CSEATS::DoTxn(){
		//generate TxnTYpe
		double rnd = r.next_uniform();
		if(rnd < FIND_FLIGHT_PERC){
				GenerateFindFlightInput();
				FindFlight(&ffInput);
		}
		else if(rnd < FIND_FLIGHT_PERC
										+ NEW_RESERVATION_PERC){
				GenerateNewReservationInput();
				NewReservation(&nrInput);
		}
		else if(rnd < FIND_FLIGHT_PERC
										+ NEW_RESERVATION_PERC
										+ UPDATE_CUSTOMER_PERC){
				GenerateUpdateCustomerInput();
				UpdateCustomer(&ucInput);
		}
		else if(rnd < FIND_FLIGHT_PERC
										+ NEW_RESERVATION_PERC
										+ UPDATE_CUSTOMER_PERC
										+ UPDATE_RESERVATION_PERC){
				GenerateUpdateReservationInput();
				UpdateReservation(&urInput);
		}
}
