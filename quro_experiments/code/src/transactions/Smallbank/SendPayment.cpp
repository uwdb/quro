#include "Smallbank.h"
#include "TxnBaseDB.h"
#include "DBConnection.h"

void CSendPaymentDB::DoSendPayment(TSendPaymentTxnInput* pIn, TSendPaymentTxnOutput* pOut){
	startTransaction();
	execute(pIn, pOut);
	commitTransaction();
}

void CSendPaymentDB::execute(const TSendPaymentTxnInput* pIn, TSendPaymentTxnOutput* pOut){
}

void CDBConnection::execute(const TSendPaymentTxnInput* pIn, TSendPaymentTxnOutput* pOut){
	pOut->status = CBaseTxnErr::SUCCESS;

	char query[1024];
	sql_result_t result;
	int length;
	char* val;
	int r = 0;
	uint64_t send_acct = pIn->send_acct;
	uint64_t dest_acct = pIn->dest_acct;
	float balance = 0;
	float amount = pIn->amount;

	sprintf(query, SP_GET_SENDACCOUNT, send_acct);
	r = dbt5_sql_execute(query, &result, "GET_SENDACCOUNT");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg("get sendaccount fails");
		throw fail_msg.c_str();
	}

	sprintf(query, SP_GET_DESTACCOUNT, dest_acct);
	r = dbt5_sql_execute(query, &result, "GET_DEST_ACCOUNT");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg("get destaccount fails");
		throw fail_msg.c_str();
	}

	sprintf(query, SP_GET_CHECKINGBALANCE, send_acct);
	r = dbt5_sql_execute(query, &result, "GET_CHECKING_BALANCE");
	if(r==1 && result.result_set){
		dbt5_sql_fetchrow(&result);
		dbt5_sql_close_cursor(&result);
	}else{
		string fail_msg("get checking balance fails");
		throw fail_msg.c_str();
	}

	sprintf(query, SP_UPDATE_CHECKINGBALANCE, amount*(-1), sned_acct);
	r = dbt5_sql_execute

}
