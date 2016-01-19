#include <fstream>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;
#define M 10000

int nQueries = 5;
int c_index[200] = {0};
int dep_cond[1000][2] = {0};
int rd_cond[1000][3] = {0};
int num_dep_cond = 0;
int num_rd_cond = 0;
int get_indicator_index(int q1, int q2){
	if(q1 > q2){
		int tmp = q2;
		q2 = q1;
		q1 = tmp;
	}
	int x = (nQueries+q2)*(nQueries-q2-1)/2 + (q2 - q1) - 1;
	return nQueries + x;	
}

int cnt_num_blank(char* line){
	int i = 0;
	int cnt = 0;
	while(i<strlen(line)){
		if(line[i]==' ')
			cnt++;
		i++;
	}
	return cnt;
}
int main(){
	ifstream infile("queryinfo.txt");
	infile>>nQueries;
	for(int i=0; i<nQueries; i++)
		infile>>c_index[i];
	char line[300];
	while(!infile.eof()){
		infile.getline(line, 300);
		if(cnt_num_blank(line)==1){
				sscanf(line, "%d %d", &dep_cond[num_dep_cond][0], &dep_cond[num_dep_cond][1]);
				num_dep_cond++;
		}else if(cnt_num_blank(line)==2){
				sscanf(line, "%d %d %d", &rd_cond[num_rd_cond][0], &rd_cond[num_rd_cond][1], &rd_cond[num_rd_cond][2]);
				num_rd_cond++;
		}
	}
	
	ofstream outfile("query.ilp");
	outfile<<"Maximize"<<endl;;
	for(int i=0; i<nQueries; i++){
		outfile<<c_index[i]<<" q"<<i;
		if(i != nQueries-1)
			outfile<<" + ";
	}
	outfile<<endl;

	int x;
	outfile<<"Subject To"<<endl;
	for(int i=0; i<nQueries; i++){
		for(int j=0; j<i; j++){
			x = i*nQueries+j;
			outfile<<"\thelper"<<i<<"_"<<j<<": ";
			outfile<<"q"<<i<<" - q"<<j<<" + "<<M<<" e"<<x<<" >= 1"<<endl;
			outfile<<"\thelper_"<<i<<"_"<<j<<" :";
			outfile<<"q"<<j<<" - q"<<i<<" - "<<M<<" e"<<x<<" >= "<<1-M<<endl;
		}
	}
	for(int i=0; i<num_dep_cond; i++){
		outfile<<"\tdefuse"<<i<<": ";
		outfile<<"q"<<dep_cond[i][1]<<" - q"<<dep_cond[i][0]<<" >= 1"<<endl;
	}
	for(int i=0; i<num_rd_cond; i++){
		outfile<<"\trd"<<i<<": ";
		outfile<<"q"<<rd_cond[i][0]<<" - q"<<rd_cond[i][2]<<" + "<<M<<" c"<<i<<" >= 1"<<endl;
		outfile<<"\trd_"<<i<<": ";
		outfile<<"q"<<rd_cond[i][2]<<" - q"<<rd_cond[i][1]<<" - "<<M<<" c"<<i<<" >= "<<1-M<<endl;
	}
	outfile<<"Bounds"<<endl;
/*
	for(int i=0; i<nQueries; i++)
		for(int j=0; j<i; j++){
				x = i*nQueries+j;
				outfile<<"\t 0 <= e"<<x<<" <= 1"<<endl;
		}
*/
	for(int i=0; i<nQueries; i++)
		outfile<<"0 <= q"<<i<<" <= "<<nQueries<<endl;
/*
	for(int i=0; i<num_rd_cond; i++){
		outfile<<"0 <= c"<<i<<" <= 1"<<endl;
	}
*/
	outfile<<"Generals"<<endl;
	for(int i=0; i<nQueries; i++)
		outfile<<"q"<<i<<" ";
	outfile<<endl;
	outfile<<"Binary"<<endl;
	for(int i=0; i<nQueries; i++)
		for(int j=0; j<i; j++)
			outfile<<"e"<<(i*nQueries+j)<<" ";
	for(int i=0; i<num_rd_cond; i++)
		outfile<<"c"<<i<<" ";
	outfile<<endl;
	outfile<<"End"<<endl;
	return 0;
}

