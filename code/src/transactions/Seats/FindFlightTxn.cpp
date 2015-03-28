#include "FindFlightDB.h"
#include "TxnBaseDB.h"
#include "util.h"
#include "DBConnection.h"
#include <vector>

void CFindFlightDB::DoFindFlight(TFindFlightTxnInput* pIn, TFindFlightTxnOutput *pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}
void CFindFlightDB::execute(const TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut){
	pDB->execute(pIn, pOut);
}
void CDBConnection::execute(const TFindFlightTxnInput* pIn, TFindFlightTxnOutput* pOut){

	pOut->status = CBaseTxnErr::SUCCESS;

	char query[4096];
	sql_result_t result;
	sql_result_t result_t;
	int length;
	char* val;
	int r = 0;

	vector<unsigned long int> ap_ids;
	ap_ids.push_back(pIn->arrive_aid);
	if(pIn->distance>0){
			sprintf(query, GET_NEARBY_AIRPORT, pIn->depart_aid, pIn->distance);

			r = dbt5_sql_execute(query, &result, "GET_NEARBY_AIRPORT");
			if(r==1 && result.result_set){
					for(size_t i=0; i<min(result.num_rows, 2); i++){
								dbt5_sql_fetchrow(&result);
								unsigned long int ap_id = atol(dbt5_sql_getvalue(&result, 1, length));
								ap_ids.push_back(ap_id);
					}
			}else{
					return ;
			}
	}

	sprintf(query, GET_AIRPORT_INFO, pIn->depart_aid);
	r = dbt5_sql_execute(query, &result, "GET_AIRPORT_INFO");
	if(r==1 && result.result_set){
			dbt5_sql_fetchrow(&result);

			pOut->depart_ap_code = atol(dbt5_sql_getvalue(&result, 0, length));

			val = dbt5_sql_getvalue(&result, 2, length);
			strncpy(pOut->depart_ap_city, val, length);

			val = dbt5_sql_getvalue(&result, 1, length);
			strncpy(pOut->depart_ap_name, val, length);

			pOut->depart_ap_country = atol(dbt5_sql_getvalue(&result, 5, length));

	}

	char sub_query[100];
	if(ap_ids.size()==1) sprintf(sub_query, "(%d)", ap_ids[0]);
	else if(ap_ids.size()==2) sprintf(sub_query, "(%d, %d)", ap_ids[0], ap_ids[1]);
	else if(ap_ids.size()==3) sprintf(sub_query, "(%d, %d, %d)", ap_ids[0], ap_ids[1], ap_ids[2]);

	sprintf(query, GET_FLIGHT, pIn->depart_aid, toStr(pIn->start_date).c_str(), toStr(pIn->end_date).c_str(), sub_query);

	r = dbt5_sql_execute(query, &result, "GET_FLIGHT");

	pOut->num_results = result.num_rows;
	for(size_t i=0; i<result.num_rows; i++){
			dbt5_sql_fetchrow(&result);
			int ar_ap_id = atol(dbt5_sql_getvalue(&result, 4, length));

			sprintf(query, GET_AIRPORT_INFO, ar_ap_id);
			r = dbt5_sql_execute(query, &result_t, "GET_AIRPORT_INFO");
			if(r==1 && result_t.result_set){
					dbt5_sql_fetchrow(&result_t);

					pOut->arrive_ap_code[i] = atol(dbt5_sql_getvalue(&result_t, 0, length));

					val = dbt5_sql_getvalue(&result_t, 2, length);
					strncpy(pOut->arrive_ap_city[i], val, length);

					val = dbt5_sql_getvalue(&result_t, 1, length);
					strncpy(pOut->arrive_ap_name[i], val, length);

					pOut->arrive_ap_country[i] = atol(dbt5_sql_getvalue(&result_t, 5, length));
			}
	}

}

