#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#define TOTAL_ACCT_NUM 100000

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

char filepath[100] = "/data/sanchez/results/silo/results/SMALLBANK_DATA"

int main(){
	char filename_acct[100] = {0};
	char filename_savings[100] = {0};
	char filename_checking[100] = {0};
	sprintf(filename_acct, "%s/Accounts.txt", filepath);
	sprintf(filename_savings, "%s/Savings.txt", filepath);
	sprintf(filename_checking, "%s/Checking.txt", filepath);

	ofstream outfile_acct(filename_acct);
	ofstream outfile_savings(filename_savings);
	ofstream outfile_checking(filename_checking);
	for(int i=0; i<TOTAL_ACCT_NUM; i++){
		string info = get_random_string(60);
		outfile_acct<<(i+1)<<"|"<<info<<endl;
		float rand1 = (float)(rand%65535)/65536.0 * 12000.0;
		float rand2 = (float)(rand%65535)/65536.0 * 240000.0;
		outfile_savings<<(i+1)<<"|"<<rand1<<endl;
		outfile_checking<<(i+1)<<"|"<<rand2<<endl;
	}
	return 0;
}
