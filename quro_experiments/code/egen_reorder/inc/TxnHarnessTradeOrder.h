/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a part of a
 * benchmark specification maintained by the TPC.
 *
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

/*
*   Trade Order transaction class.
*
*/
#ifndef TXN_HARNESS_TRADE_ORDER_H
#define TXN_HARNESS_TRADE_ORDER_H

#include "TxnHarnessDBInterface.h"

namespace TPCE
{

class CTradeOrder
{
    CTradeOrderDBInterface* m_db;
    CSendToMarketInterface* m_pSendToMarket;

public:
    CTradeOrder(CTradeOrderDBInterface *pDB, CSendToMarketInterface *pSendToMarket)
        : m_db(pDB),
        m_pSendToMarket(pSendToMarket)
    {
    };

    void DoTxn( PTradeOrderTxnInput pTxnInput, PTradeOrderTxnOutput pTxnOutput )
    {
        // Initialization
        TTradeRequest           TradeRequestForMEE; //sent to MEE

				TTradeOrderIntermediate pInter;
        TXN_HARNESS_SET_STATUS_SUCCESS;

        m_db->DoTradeOrderFrame(pTxnInput, &pInter, pTxnOutput);

				if(!pTxnInput->roll_it_back){
            // Send to Market Exchange Emulator
            //
            TradeRequestForMEE.price_quote = pInter.requested_price;
            strncpy(TradeRequestForMEE.symbol, pInter.symbol, sizeof(TradeRequestForMEE.symbol));
            TradeRequestForMEE.trade_id = pInter.trade_id;
            TradeRequestForMEE.trade_qty = pInter.trade_qty;
            strncpy( TradeRequestForMEE.trade_type_id, pTxnInput->trade_type_id, sizeof( TradeRequestForMEE.trade_type_id ));
            //TradeRequestForMEE.eTradeType = pTxnInput->eSTMTradeType;
            if( pInter.type_is_market )
            {
                TradeRequestForMEE.eAction = eMEEProcessOrder;
            }
            else
            {
                TradeRequestForMEE.eAction = eMEESetLimitOrderTrigger;
            }

            TradeRequestForMEE.acct_id = pInter.acct_id;
			TradeRequestForMEE.is_lifo = pInter.is_lifo;
			TradeRequestForMEE.trade_is_cash = pInter.trade_is_cash;
			TradeRequestForMEE.charge = pInter.charge;

			m_pSendToMarket->SendToMarketFromHarness(TradeRequestForMEE); // maybe should check the return code here
        }
    }
};

}   // namespace TPCE

#endif //TXN_HARNESS_TRADE_ORDER_H
