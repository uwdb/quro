#include "Bid.h"
#include "TxnBaseDB.h"
#include "Bid_const.h"
#include "DBConnection.h"

#define FAIL_MSG(msg) \
				string fail_msg(msg); \
				fail_msg.append("\n"); \
				fail_msg.append(query); \
				if (r==2) fail_msg.append("\tempty result"); \
				cout<<"ERROR: "<<fail_msg<<endl; \
				throw fail_msg.c_str();

#ifdef PROFILE_EACH_QUERY
#define ADD_QUERY_NODE(f, q, commit) \
				gettimeofday(&t2, NULL); \
				add_profile_node(f, q, difftimeval(t2, t1), commit);
#define GETTIME gettimeofday(&t1, NULL);
#else
#define ADD_QUERY_NODE(f, q, commit)
#define GETTIME
#endif
string get_random_string(int length){
	char c[1000] = {0};
	int len = length/2 + rand()%(length/2);
	for(int i=0; i<len; i++){
		c[i] = rand()%26+'a';
	}
	string s(c);
	return s;
}
struct FIELDHIST{
	map<string, int> field_hist;
};

map<string, FIELDHIST> table_hist;
int profile_cnt = 0;
string tbl_name;
string param_str;

#define CLANG_PROFILE \
tbl_name = returnTableName(query); \
param_str = parseQueryParams(query); \
cout<<"tbl_name = "<<tbl_name<<endl; \
cout<<"\tparam_str = "<<param_str<<endl; \
fflush(stdout); \
if(table_hist.find(tbl_name) != table_hist.end()){ \
	if(table_hist[tbl_name].field_hist.find(param_str) != table_hist[tbl_name].field_hist.end()) \
		table_hist[tbl_name].field_hist[param_str] = table_hist[tbl_name].field_hist[param_str] + 1;  \
	else table_hist[tbl_name].field_hist[param_str] = 1; \
} \
else { \
	FIELDHIST fh; \
	fh.field_hist[param_str] = 1; \
	table_hist[tbl_name] = fh; \
	} \
profile_cnt++; \
if(profile_cnt%10000 == 0){\
	profile_cnt = 1; \
	cout<<"===============10000 add=============="<<endl;\
	for(map<string, FIELDHIST>::iterator it = table_hist.begin(); it != table_hist.end(); it++){ \
			cout<<it->first<<", size = "<<it->second.field_hist.size()<<endl; \
	} \
}

string parseQueryParams(char* query){
	if(strstr(query, "SELECT")!=NULL || strstr(query, "DELETE")!=NULL || strstr(query, "UPDATE")){
			char* p_where = strstr(query, "WHERE");
			char* temp_p = p_where+6;
			char str[1000] = {0};
			char i = 0;
			while(strstr(temp_p, "AND")!=NULL){
					if(*temp_p == 'A' && *(temp_p+1) == 'N' && *(temp_p+2) == 'D'){
							temp_p = temp_p + 3;
					}
					if(*temp_p == '\n')
							temp_p++;
					str[i] = *temp_p;
					temp_p++;
					i++;
			}
			while(strlen(temp_p)>0 && *temp_p != '\n'){
					str[i] = *temp_p;
					temp_p++;
					i++;
			}
			str[i] = 0;
			string result(str);
			return result;
	}
}
void splitByBlank(vector<string>& vec, string str){
			char s[1000] = {0};
			int i = 0;
			int j = 0;
			while(i < str.length()){
					if(str[i]==' ' || str[i]=='\n'){
							string s1(s);
							vec.push_back(s1);
							memset(s, 0, sizeof(char)*1000);
							j = 0;
							i++;
					}else{
							s[j] = str[i];
							i++, j++;
					}
			}
			string s1(s);
			vec.push_back(s1);
}

string returnTableName(char* query){
		vector<string> words;
		string lit_str(query);
		splitByBlank(words, lit_str);
		int i=0;
		if(strcmp(words[0].c_str(), "SELECT")==0){
				while(i<words.size() && strcmp(words[i].c_str(), "FROM")!=0)
						i++;
				assert(i+1<words.size());
				i++;
				string table_name(words[i]);
				return table_name;
		}else if(strcmp(words[0].c_str(), "UPDATE")==0){
				i=1;
				string table_name(words[i]);
				return table_name;
		}else if(strcmp(words[0].c_str(), "DELETE") == 0){
				while(i<words.size() && strcmp(words[i].c_str(), "FROM")!=0)
						i++;
				assert(i+1<words.size());
				i++;
				string table_name(words[i]);
				return table_name;
		}
}

void CBiddingDB::DoBidding(TBiddingTxnInput* pIn, TBiddingTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CBiddingDB::execute(const TBiddingTxnInput* pIn, TBiddingTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}

void CDBConnection::execute(const TBiddingTxnInput* pIn, TBiddingTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;

	char query[4096];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;

	string user_name;
	string item_description;
	string rec_description = get_random_string(128);
	uint64_t bid;
	uint64_t pid;
	double cur_price;
	double bid_price;

	sprintf(query, GET_USER, pIn->user_id);
	GETTIME;
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_USERS");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		ADD_QUERY_NODE(1, 1, 1);
		bid = atol(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		user_name.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_USER FAIL");
	}

	sprintf(query, SELECT_ITEM, pIn->item_id);
	GETTIME;
	//CLANG_PROFILE;
	r = dbt5_sql_execute(query, &result, "GET_ITEM");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		ADD_QUERY_NODE(1, 2, 1);
		cur_price = atof(dbt5_sql_getvalue(&result, 0, length));
		val = dbt5_sql_getvalue(&result, 1, length);
		item_description.assign(val);
		dbt5_sql_close_cursor(&result);
	}else{
		FAIL_MSG("GET_ITEM FAIL");
	}
/*
	if(double(rand()%128)/128.0 < HIGHER_BID_THRESHOLD)
					bid_price = cur_price + double(rand()%16)/16.0;
	else
					bid_price = cur_price - double(rand()%128)/128.0;


	if(bid_price > cur_price){
		BIDDING_UPDATE_ITEM;
	}

	BIDDING_UPDATE_USER;

	BIDDING_INSERT_REC;
*/
/*
	BIDDING_INSERT_REC;
	BIDDING_UPDATE_USER;
	if(bid_price > cur_price){
		BIDDING_UPDATE_ITEM;
	}
*/
}




