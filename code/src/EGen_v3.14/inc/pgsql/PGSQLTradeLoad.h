/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a preliminary
 * version of a benchmark specification being developed by the TPC. The
 * Work is being made available to the public for review and comment only.
 * The TPC reserves all right, title, and interest to the Work as provided
 * under U.S. and international laws, including without limitation all patent
 * and trademark rights therein.
 *
 * No Warranty
 *
 * 1.1 TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE INFORMATION
 *     CONTAINED HEREIN IS PROVIDED "AS IS" AND WITH ALL FAULTS, AND THE
 *     AUTHORS AND DEVELOPERS OF THE WORK HEREBY DISCLAIM ALL OTHER
 *     WARRANTIES AND CONDITIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
 *     INCLUDING, BUT NOT LIMITED TO, ANY (IF ANY) IMPLIED WARRANTIES,
 *     DUTIES OR CONDITIONS OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
 *     PURPOSE, OF ACCURACY OR COMPLETENESS OF RESPONSES, OF RESULTS, OF
 *     WORKMANLIKE EFFORT, OF LACK OF VIRUSES, AND OF LACK OF NEGLIGENCE.
 *     ALSO, THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,
 *     QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR NON-INFRINGEMENT
 *     WITH REGARD TO THE WORK.
 * 1.2 IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THE WORK BE LIABLE TO
 *     ANY OTHER PARTY FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO THE
 *     COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST PROFITS, LOSS
 *     OF USE, LOSS OF DATA, OR ANY INCIDENTAL, CONSEQUENTIAL, DIRECT,
 *     INDIRECT, OR SPECIAL DAMAGES WHETHER UNDER CONTRACT, TORT, WARRANTY,
 *     OR OTHERWISE, ARISING IN ANY WAY OUT OF THIS OR ANY OTHER AGREEMENT
 *     RELATING TO THE WORK, WHETHER OR NOT SUCH AUTHOR OR DEVELOPER HAD
 *     ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Contributors
 * - Sergey Vasilevskiy
 */

// 2006 Rilson Nascimento


/*
*	Database loader class for TRADE table.
*/
#ifndef PGSQL_TRADE_LOAD_H
#define PGSQL_TRADE_LOAD_H

namespace TPCE
{

class CPGSQLTradeLoad : public CPGSQLLoader <TRADE_ROW>
{	

public:
	CPGSQLTradeLoad(char *szHost, char *szDBName, char *szPostmasterPort, char *szTable = "TRADE")
		: CPGSQLLoader<TRADE_ROW>(szHost, szDBName, szPostmasterPort, szTable)
	{
	};


	virtual void WriteNextRecord(PT next_record)
	{
		CopyRow(next_record);	//copy to the bound location inside this class first
	
		buf.push_back(stringify(m_row.T_ID));
		buf.push_back(m_row.T_DTS.ToStr(iDateTimeFmt));
		buf.push_back(m_row.T_ST_ID);
		buf.push_back(m_row.T_TT_ID);
		//buf.push_back((m_row.T_IS_CASH ? "true" : "false"));
		buf.push_back(stringify(m_row.T_IS_CASH));
		buf.push_back(m_row.T_S_SYMB);
		buf.push_back(stringify(m_row.T_QTY));
		buf.push_back(stringify(m_row.T_BID_PRICE));
		buf.push_back(stringify(m_row.T_CA_ID));
		buf.push_back(m_row.T_EXEC_NAME);
		buf.push_back(stringify(m_row.T_TRADE_PRICE));
		buf.push_back(stringify(m_row.T_CHRG));
		buf.push_back(stringify(m_row.T_COMM));
		buf.push_back(stringify(m_row.T_TAX));
		//buf.push_back((m_row.T_LIFO ? "true" : "false"));
		buf.push_back(stringify(m_row.T_LIFO));

		m_TW->insert(buf);
		buf.clear();
	}

	virtual void FinishLoad()
	{
		m_TW->complete();
		ostringstream osCall;

		result R( m_Txn->exec("select max(t_id) from trade"));
		result::const_iterator c = R.begin();
		osCall<<"SELECT setval('seq_trade_id',"<<c[0]<<")";
		m_Txn->exec(osCall.str());

		m_Txn->commit();
		delete m_TW;
		delete m_Txn;
	
		Disconnect();	// While destructor is not being called
	}

};

}	// namespace TPCE

#endif //PGSQL_TRADE_LOAD_H
