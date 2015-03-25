#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int scale_factor = 1;

char filepath[50] = "";
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
  stringcountry_code;
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
size_t numAirlines = 0;

size_t numCustomers;
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
				countries[i].c_name.assgin(c_name);
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
}

void load_airline(){
		size_t i=0;
    ifstream infile;
	  char line[1000];
		char filename[100];
		sprintf(filename, "%s/table.airline.csv.gz", filepath);
    infile.open(filename);
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
				i++;
		}
   	numAirlines = i;
}

void load_customer(){
		numCustomers = scale_factor * 10 * 10 * 10 * 10 * 10 * 10;

}

