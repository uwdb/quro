#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
using namespace std;

int scale_factor = 1;
#define FF_PERC 0.08
//========start helper functions=========
double deg2rad(double deg) {
    return (deg * 3.1415926 / 180.0);
}
double rad2deg(double rad) {
    return (rad * 180.0 / 3.1415926);
}
double distance(double lat0, double lon0, double lat1, double lon1) {
    double theta = lon0 - lon1;
    double dist = sin(deg2rad(lat0)) * sin(deg2rad(lat1)) + cos(deg2rad(lat0)) * cos(deg2rad(lat1)) * cos(deg2rad(theta));
    dist = acos(dist);
    dist = rad2deg(dist);
    return (dist * 60 * 1.1515);
}

struct fast_getRandomom{
	unsigned long seed;
	fast_getRandomom(){
			seed = 1284;
	}
	inline unsigned long
  next()
  {
    return ((unsigned long) next(32) << 32) + next(32);
  }
	inline double
  next_uniform()
  {
    return (((unsigned long) next(26) << 27) + next(27)) / (double) (1L << 53);
  }
	inline unsigned long
  next(unsigned int bits)
  {
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    return (unsigned long) (seed >> (48 - bits));
  }
};
fast_getRandomom rd;
long unsigned int getRandom(){
		return rd.next();
}
string GenerateRandomString(int len, bool fixed_len){
		string s;
		size_t length;
		if(fixed_len) length = len;
		else length = getRandom()%len;
		for(int i=0; i<length; i++){
			int rnd = getRandom()%60;
			char c;
			if(rnd<26)
				c='a'+rnd;
			else if(rnd<52)
				c='A'+rnd-26;
			else if(rnd==53)
				c=' ';
			else
				c='.';
			s.append(1, c);
		}
		return s;
}
float GenerateRandomFloat(){
		return (getRandom()%65536)/3;
}
struct TimeStamp{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		float second;
		string to_str(){
			char str[100]={0};
			sprintf(str, "%d-%d-%d %d:%d:%f", year, month, day, hour, minute, second);
			string s(str);
			return s;
		}
};
TimeStamp GenerateRandomTimestamp(){
		TimeStamp ts;
		ts.year = 2011+getRandom()%4;
		ts.month = getRandom()%12+1;
		ts.day = getRandom()%30+1;
		ts.hour = getRandom()%24;
		ts.minute = getRandom()%60;
		ts.second = 0;
		return ts;
}

vector<long unsigned int> ff_set;
//========end helper functions===========

char filepath[50] = "./seats_data";
char outfilepath[50] = "/data/sanchez/results/silo/results/SEATS_DATA";
struct CCountry{
	string c_name;
	string c_code2;
	string c_code3;
};
CCountry countries[300];
map<string, long unsigned int> countryMapping;
size_t numCountries = 0;

struct CAirport{
	string ap_code;
  string ap_name;
  string ap_city;
  string ap_postal_code;
  string country_code;
	long unsigned int ap_co_id;
  float ap_longitude;
  float ap_latitude;
	string c_ap_gmt_offset;
  float ap_gmt_offset;
  int ap_wac;
};
CAirport airports[10000];
size_t numAirports = 0;

struct CAirline{
		string al_iata_code;
    string al_icao_code;
    string al_call_sign;
    string al_name;
   	string country_code;
		long unsigned int al_co_id;
};
CAirline airlines[1300];
size_t numAirlines = 1250;

struct CCustomer{
		string c_id_str;
};
CCustomer customers[1000000*10];
size_t numCustomers;

struct CFlight{
		long unsigned int f_al_id;
		long unsigned int f_depart_ap_id;
		long unsigned int f_arrive_ap_id;
};
size_t numFlights;
//===========start loading functions=============

void load_country(){
		ifstream infile;
		char filename[100];
		sprintf(filename, "%s/table.country.csv", filepath);
    infile.open(filename);

		sprintf(filename, "%s/Country.txt", outfilepath);
		ofstream outfile;
		outfile.open(filename);
    char line[1000];
		int i=0;
    while(infile.getline(line, 1000) && strlen(line)>0){
    		char c_name[128];
        char c_code2[128];
       	char c_code3[128];
        sscanf(line,"\"%[^\",]\",\"%2s\",\"%3s\"", c_name, c_code2, c_code3);
				countries[i].c_name.assign(c_name);
				countries[i].c_code2.assign(c_code2);
				countries[i].c_code3.assign(c_code3);
				outfile<<i+1<<"|"<<c_name<<"|"<<c_code2<<"|"<<c_code3<<endl;
				i++;
				countryMapping[countries[i-1].c_name] = i;
		}
		numCountries = i;
		infile.close();
		outfile.close();
}

void load_airport(){
	size_t i=0;
  ifstream infile;
	ofstream outfile;
	char filename[100];
	sprintf(filename, "%s/table.airport.csv", filepath);
	char line[1000];
	infile.open(filename);

	sprintf(filename, "%s/Airport.txt", outfilepath);
	outfile.open(filename);
  while(infile.getline(line, 1000) && strlen(line)>0){
  		char ap_code[128];
      char ap_name[256];
      char ap_city[128];
      char ap_postal_code[64]={0};
      char country_code[64]={0};
      float ap_longitude;
      float ap_latitude;
	    char c_ap_gmt_offset[20]={0};
      float ap_gmt_offset=0;
      int ap_wac;
      sscanf(line, "\"%3s\",\"%[^\",]\",\"%[^\",]\",%[^,],\"%3s\",%f,%f,%[^,],%d",ap_code,ap_name,ap_city,ap_postal_code,country_code,&ap_longitude,&ap_latitude,c_ap_gmt_offset,&ap_wac);
	    //reading file is horrible!
	    if(strlen(ap_postal_code)==0)
	  	sscanf(line, "\"%3s\",\"%[^\",]\",\"%[^\",]\",,\"%3s\",%f,%f,%[^,],%d",ap_code,ap_name,ap_city,country_code,&ap_longitude,&ap_latitude,c_ap_gmt_offset,&ap_wac);
      if(strlen(c_ap_gmt_offset)==0)
          ap_gmt_offset=0;
      else
          sscanf(c_ap_gmt_offset,"%f",&ap_gmt_offset);
			airports[i].ap_code.assign(ap_code);
			airports[i].ap_name.assign(ap_name);
			airports[i].ap_city.assign(ap_city);
			airports[i].ap_postal_code.assign(ap_postal_code);
			airports[i].country_code.assign(country_code);
			airports[i].ap_longitude = ap_longitude;
			airports[i].ap_latitude = ap_latitude;
			airports[i].c_ap_gmt_offset.assign(c_ap_gmt_offset);
			airports[i].ap_gmt_offset = ap_gmt_offset;
			airports[i].ap_wac = ap_wac;

			long unsigned int ap_co_id = countryMapping[airports[i].country_code];
			airports[i].ap_co_id = ap_co_id;
			outfile<<i+1<<"|"<<ap_code<<"|"<<ap_name<<"|"<<ap_city<<"|"<<ap_postal_code<<"|"<<ap_co_id<<"|"<<ap_longitude<<"|"<<ap_latitude<<"|"<<ap_gmt_offset<<"|"<<ap_wac<<"|";
			for(int k=0; k<16; k++){
				outfile<<getRandom()%4087192;
				if(k<15)outfile<<"|";
			}
			outfile<<endl;

      i++;
	}
  numAirports = i;
	infile.close();
	outfile.close();

	cout<<"start counting distance"<<endl;
	sprintf(filename, "%s/AirportDistance.txt", outfilepath);
	outfile.open(filename);
	for(size_t i=1; i<=numAirports; i++){
			for(size_t j=i+1; j<=numAirports; j++){
					double dist = distance(airports[i-1].ap_latitude, airports[i-1].ap_longitude, airports[j-1].ap_latitude, airports[j-1].ap_longitude);
					outfile<<i<<"|"<<j<<"|"<<dist<<endl;
			}
	}
	outfile.close();
	cout<<"end counting distance"<<endl;
}

void load_airline(){
		size_t i=0;
    ifstream infile;
	  char line[1000];
		char filename[100];
		sprintf(filename, "%s/table.airline.csv", filepath);
    infile.open(filename);

		ofstream outfile;
		sprintf(filename, "%s/Airline.txt", outfilepath);
		outfile.open(filename);

		ofstream outfile2;
		sprintf(filename, "%s/FrequentFlyer.txt", outfilepath);
		outfile2.open(filename);

		unsigned int cust_range = numCustomers / numAirlines;
		while(infile.getline(line, 1000) && strlen(line)>0){
    		char al_iata_code[64];
        char al_icao_code[64];
       	char al_call_sign[64]={0};
        char al_name[256];
        char country_code[64];
       	sscanf(line,"\"%2s\",\"%3s\",%[^,],\"%[^\",]\",\"%3s\"",al_iata_code,al_icao_code,al_call_sign,al_name,country_code);
				if(strlen(al_call_sign)==0)
		    		sscanf(line,"\"%2s\",\"%3s\",,\"%[^\",]\",\"%3s\"",al_iata_code,al_icao_code,al_name,country_code);
        airlines[i].al_iata_code.assign(al_iata_code);
				airlines[i].al_icao_code.assign(al_icao_code);
				airlines[i].al_call_sign.assign(al_call_sign);
				airlines[i].al_name.assign(al_name);
				airlines[i].country_code.assign(country_code);

				long unsigned int al_co_id = countryMapping[airlines[i].country_code];
				airlines[i].al_co_id = al_co_id;

				outfile<<i+1<<"|"<<al_iata_code<<"|"<<al_icao_code<<"|"<<al_call_sign<<"|"<<al_co_id<<"|";

				for(int k=0; k<15; k++){
						outfile<<getRandom()%301993875;
						if(k<15)outfile<<"|";
				}
				outfile<<endl;

				for(unsigned int k=0; k<cust_range; k++){
						if(float(getRandom()%numCustomers)/float(numCustomers) < FF_PERC){
								unsigned int c_id = i*cust_range + k + 1;
								ff_set.push_back(c_id);
								//register a frequent flyer
								outfile2<<c_id<<"|"<<i+1<<"|"<<customers[c_id-1].c_id_str<<"|";

								for(size_t k=0; k<4; k++)
										outfile2<<GenerateRandomString(32, false)<<"|";
								for(size_t k=0; k<16; k++){
										outfile2<<getRandom();
										if(k<15)outfile2<<"|";
								}
								outfile2<<endl;
						}
				}
				i++;
		}
   	numAirlines = i;
		infile.close();
		outfile.close();
		outfile2.close();
}

void load_customer(){
		ofstream outfile;
		char filename[100];
		sprintf(filename, "%s/Customer.txt", outfilepath);
		outfile.open(filename);
		for(size_t i=0; i<numCustomers; i++){
				outfile<<i+1<<"|";
				string c_id_str = GenerateRandomString(48, false);
				customers[i].c_id_str.assign(c_id_str);
				outfile<<c_id_str<<"|";
				outfile<<(getRandom()%numAirports+1)<<"|";
				outfile<<GenerateRandomFloat()<<"|";
				outfile<<GenerateRandomString(32, false)<<"|";
				for(size_t k=1; k<20; k++)
					outfile<<GenerateRandomString(8, true)<<"|";
				for(size_t k=0; k<20; k++){
					outfile<<getRandom();
					if(k<19)outfile<<"|";
				}
				outfile<<endl;
		}
		outfile.close();
}

void load_flight(){

	ofstream outfile;
	char filename[100];
	sprintf(filename, "%s/Flight.txt", outfilepath);
	outfile.open(filename);

	ofstream outfile2;
	sprintf(filename, "%s/Reservation.txt", outfilepath);
	outfile2.open(filename);

	ofstream outfile3;
	sprintf(filename, "%s/FlightInfo.txt", outfilepath);
	outfile3.open(filename);
	long unsigned int reservation_id = 1;
	for(size_t i=0; i<numFlights; i++){
			outfile<<i+1<<"|";
			long unsigned int f_al_id = getRandom()%numAirlines+1;
			long unsigned int f_depart_ap_id = getRandom()%numAirports+1;
			long unsigned int f_arrive_ap_id = getRandom()%numAirports+1;
			int seats_total = 50+getRandom()%200;

			float reservation_perc = 0.6+float(getRandom()%400)/1000.0;
			int num_reservation = seats_total*reservation_perc;

			int seats_left = seats_total - num_reservation;
			float base_price = GenerateRandomFloat();
			TimeStamp ts = GenerateRandomTimestamp();
			TimeStamp ts2 = ts;
			ts2.hour = (ts2.hour+getRandom()%5)%24;
			ts2.minute = (ts2.minute+getRandom()%60)%60;

			outfile<<f_al_id<<"|"<<f_depart_ap_id<<"|"<<ts.to_str()<<"|"<<f_arrive_ap_id<<"|"<<ts2.to_str()<<"|"<<base_price<<"|"<<seats_total<<"|"<<seats_left<<"|";

			outfile3<<i+1<<"|"<<f_al_id<<"|"<<f_depart_ap_id<<"|"<<ts.to_str()<<"|"<<f_arrive_ap_id<<"|"<<ts2.to_str()<<"|"<<seats_total<<endl;
			//cout<<f_al_id<<"|"<<f_depart_ap_id<<"|"<<ts.to_str()<<"|"<<f_arrive_ap_id<<"|"<<ts2.to_str()<<"|"<<base_price<<"|"<<seats_total<<"|"<<seats_left<<"|";
			for(int k=0; k<30; k++){
					outfile<<getRandom();
					if(k<29)outfile<<"|";
			}
			outfile<<endl;
			//reservation
			for(int k=0; k<num_reservation; k++){
					long unsigned int c_id = ff_set[getRandom()%ff_set.size()];
					float price = base_price + getRandom()%200;
					outfile2<<reservation_id<<"|"<<c_id<<"|"<<i+1<<"|"<<k+1<<"|"<<price<<"|";
					for(int j=0; j<9; j++){
							outfile2<<getRandom();
							if(j<8)outfile<<"|";
					}
					outfile2<<endl;
			}

	}
	outfile.close();
	outfile2.close();
	outfile3.close();
}

int main(){
		numCustomers = scale_factor * 10 * 10 * 10 * 10 * 10 * 10;
		numFlights = scale_factor * 10 * 10 * 10 * 10 * 10;
//		numCustomers = scale_factor * 10000;
//		numFlights = scale_factor * 1000;
		cout<<"numCustomers: "<<numCustomers<<", numFlights = "<<numFlights<<endl;
		load_country();
		cout<<"finish loading country"<<endl;
		load_airport();
		cout<<"finish loading airport"<<endl;
		load_customer();
		cout<<"finish loading customer"<<endl;
		load_airline();
		cout<<"finish loading airline"<<endl;
		load_flight();
		cout<<"finish loading flight"<<endl;
		return 0;
}
