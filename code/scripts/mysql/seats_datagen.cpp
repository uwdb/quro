#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

int scale_factor = 1;
#define FF_PERC 0.05
//========start helper functions=========
string GenerateRandomString(int len, bool fixed_len){
		string s;
		size_t length;
		if(fixed_len) length = len;
		else length = rand()%len;
		for(int i=0; i<length; i++){
			int rnd = rand()%60;
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
		return (rand()%65536)/3;
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
		ts.year = 2011+rand()%4;
		ts.month = rand()%12+1;
		ts.day = rand()%30+1;
		ts.hour = rand()%24;
		ts.minute = rand()%60;
		ts.second = 0;
		return ts;
}

vector<long unsigned int> ff_set;
//========end helper functions===========

char filepath[50] = "";
char outfilepath[50] = "";
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
				i++;
				countryMapping[countries[i-1].c_name] = i;
		}
		numCountries = i;
		infile.close();
}

void load_airport(){
	size_t i=0;
  ifstream infile;
	ofstream outfile;
	char filename[100];
	sprintf(filename, "%s/table.airport.csv.gz", filepath);
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
				outfile<<rand()%4087192;
				if(k<15)outfile<<"|";
			}
			outfile<<endl;

      i++;
	}
  numAirports = i;
	infile.close();
	outfile.close();
}

void load_airline(){
		size_t i=0;
    ifstream infile;
	  char line[1000];
		char filename[100];
		sprintf(filename, "%s/table.airline.csv.gz", filepath);
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
						outfile<<rand()%301993875;
						if(k<15)outfile<<"|";
				}
				
				for(unsigned int k=0; k<cust_range; k++){
						if(float(rand()%numCustomers)/float(numCustomers) < FF_PERC){
								unsigned int c_id = i*cust_range + k;
								ff_set.push_back(c_id);
								//register a frequent flyer
								outfile2<<c_id<<"|"<<i+1<<"|"<<customers[c_id-1].c_id_str<<"|";	
						
								for(size_t k=0; k<4; k++)
										outfile2<<GenerateRandomString(32, false)<<"|";
								for(size_t k=0; k<16; k++){
										outfile<<rand();
										if(k<15)outfile2<<"|";
								}
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
				outfile<<(rand()%numAirports+1)<<"|";
				outfile<<GenerateRandomFloat()<<"|";
				outfile<<GenerateRandomString(32, false)<<"|";
				for(size_t k=1; k<20; k++)
					outfile<<GenerateRandomString(8, true)<<"|";
				for(size_t k=0; k<20; k++){
					outfile<<rand();
					if(k<19)outfile<<"|";
				}
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
	long unsigned int reservation_id = 1;
	for(size_t i=0; i<numFlights; i++){
			outfile<<i+1<<"|";
			long unsigned int f_al_id = rand()%numAirlines+1;
			long unsigned int f_depart_ap_id = rand()%numAirports+1;
			long unsigned int f_arrive_ap_id = rand()%numAirports+1;
			int seats_total = 50+rand()%200;

			float reservation_perc = 0.6+float(rand()%400)/1000.0;
			int num_reservation = seats_total*reservation_perc;

			int seats_left = seats_total - num_reservation;
			float base_price = GenerateRandomFloat();
			TimeStamp ts = GenerateRandomTimestamp();
			TimeStamp ts2 = ts;
			ts2.hour = (ts2.hour+rand()%5)%24;
			ts2.minute = (ts2.minute+rand()%60)%60;
			
			outfile<<f_al_id<<"|"<<f_depart_ap_id<<"|"<<ts.to_str()<<"|"<<f_arrive_ap_id<<"|"<<ts2.to_str()<<"|"<<base_price<<"|"<<seats_total<<"|"<<seats_left<<"|";
			for(int k=0; k<30; k++){
					outfile<<rand();
					if(k<29)outfile<<"|";
			}

			//reservation
			for(int k=0; i<num_reservation; k++){
					long unsigned int c_id = ff_set[rand()%ff_set.size()];
					float price = base_price + rand()%200;
					outfile2<<reservation_id<<"|"<<c_id<<"|"<<i+1<<"|"<<k+1<<"|"<<price<<"|";
					for(int j=0; j<9; j++){
							outfile2<<rand();
							if(j<8)outfile<<"|";
					}
			}
			
	}
	outfile.close();
	outfile2.close();
}

int main(){
		numCustomers = scale_factor * 10 * 10 * 10 * 10 * 10 * 10;
		numFlights = scale_factor * 10 * 10 * 10 * 10 * 10;
		load_country();
		load_customer();
		load_airport();
		load_airline();
		load_flight();	
		return 0;
}
