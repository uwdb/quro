#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#define TOTAL_USER_NUM 1000000
#define TOTAL_ITEM_NUM 1000
#define INITIAL_PRICE 10
using namespace std;
string get_random_string(int length){
	char c[1000] = {0};
	int len = length/2 + rand()%(length/2);
	for(int i=0; i<len; i++){
		c[i] = rand()%26+'a';
	}
	string s(c);
	return s;
}
char filepath[100] = "/data/sanchez/results/silo/results/BID_DATA";
int main()
{
	double bid_price[TOTAL_ITEM_NUM] = {0};
	char filename[200] = {0};
	sprintf(filename, "%s/User.txt", filepath);
	ofstream outfile(filename);
	sprintf(filename, "%s/Bidrec.txt", filepath);
	ofstream outfile_rec(filename);
	for(int i=0; i<TOTAL_USER_NUM; i++){
		int bid = rand()%5+1;
		int k_field = rand()%61935923;
		//user_id || bid | k_field | name | addr | whatever
		string name = get_random_string(64);
		string addr = get_random_string(256);
		string whatever = get_random_string(32);
		outfile<<i<<"|"<<bid<<"|"<<k_field<<"|"<<name<<"|"<<addr<<"|"<<whatever<<endl;

		//user_id | bid || pid(item_id) | price | description
		for(int j=0; j<bid; j++){
				int pid = rand()%TOTAL_ITEM_NUM;
				string description = get_random_string(128);
				double price = (double(rand()%65536)/65536.0) * INITIAL_PRICE;
				bid_price[pid] = bid_price[pid]>price ? bid_price[pid] : price;
				outfile_rec<<i<<"|"<<j+1<<"|"<<pid<<"|"<<price<<"|"<<description<<endl;
		}
	}
	cout<<"finish loading customers and records"<<endl;
	outfile.close();

	sprintf(filename, "%s/Item.txt", filepath);
	outfile.open(filename);
	for(int i=0; i<TOTAL_ITEM_NUM; i++){
		int feature = rand()%256;
		//pid || feature | max_price | description
		string description = get_random_string(1024);
		double max_price = bid_price[i];
		outfile<<i<<"|"<<feature<<"|"<<max_price<<"|"<<description<<endl;
	}
	return 0;
}
