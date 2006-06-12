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
 * - Doug Johnson
 */

/******************************************************************************
*	Description:		Implementation of the MEETickerTape class.
*						See MEETickerTape.h for a description.
******************************************************************************/

#include "../inc/MEETickerTape.h"

using namespace TPCE;

const int CMEETickerTape::LIMIT_TRIGGER_TRADE_QTY = 375;
const int CMEETickerTape::RANDOM_TRADE_QTY_1 = 325;
const int CMEETickerTape::RANDOM_TRADE_QTY_2 = 425;

RNGSEED CMEETickerTape::GetRNGSeed( void )
{
	return( m_rnd.GetSeed() );
}

void CMEETickerTape::SetRNGSeed( RNGSEED RNGSeed )
{
	m_rnd.SetSeed( RNGSeed );
}

void CMEETickerTape::Initialize( void )
{
	// Set up status and trade types for Market-Feed input
	//
	// Submitted
	strncpy(m_TxnInput.StatusAndTradeType.status_submitted, 
			"SBMT",
			sizeof(m_TxnInput.StatusAndTradeType.status_submitted));
	// Limit-Buy
	strncpy(m_TxnInput.StatusAndTradeType.type_limit_buy, 
			"TLB",
			sizeof(m_TxnInput.StatusAndTradeType.type_limit_buy));
	// Limit-Sell
	strncpy(m_TxnInput.StatusAndTradeType.type_limit_sell, 
			"TLS",
			sizeof(m_TxnInput.StatusAndTradeType.type_limit_sell));
	// Stop-Loss
	strncpy(m_TxnInput.StatusAndTradeType.type_stop_loss, 
			"TSL",
			sizeof(m_TxnInput.StatusAndTradeType.type_stop_loss));
}

// Constructor - use default RNG seed
CMEETickerTape::CMEETickerTape( CMEESUTInterface* pSUT, CMEEPriceBoard*	pPriceBoard, CDateTime* pBaseTime, CDateTime* pCurrentTime )
	: m_pSUT( pSUT )
	, m_pPriceBoard( pPriceBoard )
	, m_BatchIndex( 0 )
	, m_pBaseTime( pBaseTime )
	, m_pCurrentTime( pCurrentTime )
	, m_rnd( RNGSeedBaseMEETickerTape )
	, m_Enabled( true )
{
	Initialize();
}

// Constructor - RNG seed provided
CMEETickerTape::CMEETickerTape( CMEESUTInterface* pSUT, CMEEPriceBoard*	pPriceBoard, CDateTime* pBaseTime, CDateTime* pCurrentTime, RNGSEED RNGSeed )
	: m_pSUT( pSUT )
	, m_pPriceBoard( pPriceBoard )
	, m_BatchIndex( 0 )
	, m_pBaseTime( pBaseTime )
	, m_pCurrentTime( pCurrentTime )
	, m_rnd( RNGSeed )
	, m_Enabled( true )
{
	Initialize();
}

CMEETickerTape::~CMEETickerTape( void )
{
}

bool CMEETickerTape::DisableTicker( void )
{
	m_Enabled = false;
	return( ! m_Enabled );
}

bool CMEETickerTape::EnableTicker( void )
{
	m_Enabled = true;
	return( m_Enabled );
}

void CMEETickerTape::AddEntry( PTickerEntry pTickerEntry )
{
	if( m_Enabled )
	{
		AddToBatch( pTickerEntry);
		AddArtificialEntries( );
	}
}

void CMEETickerTape::PostLimitOrder( PTradeRequest pTradeRequest )
{
	eTradeTypeID	eTradeType;
	double		CurrentPrice = -1.0;
	PTickerEntry	pNewEntry = new TTickerEntry;

	eTradeType = ConvertTradeTypeIdToEnum( pTradeRequest->trade_type_id );

	pNewEntry->price_quote = pTradeRequest->price_quote;
	strncpy( pNewEntry->symbol, pTradeRequest->symbol, sizeof( pNewEntry->symbol ));
	pNewEntry->trade_qty = LIMIT_TRIGGER_TRADE_QTY;

	CurrentPrice = m_pPriceBoard->GetCurrentPrice( pTradeRequest->symbol ).DollarAmount();

	if((( eTradeType == eLimitBuy || eTradeType == eStopLoss ) &&
			CurrentPrice <= pTradeRequest->price_quote )
		|| 
			(( eTradeType == eLimitSell ) && 
			CurrentPrice >= pTradeRequest->price_quote ))
	{
		// Limit Order is in-the-money.
		pNewEntry->price_quote = CurrentPrice;
		// Make sure everything is up to date.
		m_LimitOrderTimers.ProcessExpiredTimers();
		// Now post the incoming entry.
		m_InTheMoneyLimitOrderQ.push( pNewEntry );
	}
	else
	{
		// Limit Order is not in-the-money.
		pNewEntry->price_quote = pTradeRequest->price_quote;
		double TriggerTimeDelay;
		double fCurrentTime = *m_pCurrentTime - *m_pBaseTime;

		// GetSubmissionTime returns a value relative to time 0, so we
		// need to substract off the value for the current time to get
		// the delay time relative to now.
		TriggerTimeDelay = m_pPriceBoard->GetSubmissionTime(pNewEntry->symbol,
															fCurrentTime,
															pNewEntry->price_quote,
															eTradeType)
							- fCurrentTime;
		m_LimitOrderTimers.StartTimer( TriggerTimeDelay,
			this, &CMEETickerTape::AddLimitTrigger, pNewEntry );
	}
}

void CMEETickerTape::AddLimitTrigger( PTickerEntry pTickerEntry )
{
	m_InTheMoneyLimitOrderQ.push( pTickerEntry );
}

void CMEETickerTape::AddArtificialEntries( void )
{
	TIdent			SecurityIndex;
	TTickerEntry		TickerEntry;
	int			TotalEntryCount = 0;
	static const int	PaddingLimitForAll = 1;		// MAX 1 (trigger+artificial) entries 
	static const int	PaddingLimitForTriggers = 1;	// MAX 1 triggered entries

	while ( TotalEntryCount < PaddingLimitForTriggers && !m_InTheMoneyLimitOrderQ.empty() )
	{
		PTickerEntry pEntry = m_InTheMoneyLimitOrderQ.front();
		AddToBatch( pEntry );
		delete pEntry;
		m_InTheMoneyLimitOrderQ.pop();
		TotalEntryCount++;
	}

	while ( TotalEntryCount < PaddingLimitForAll )
	{
		TickerEntry.trade_qty = ( m_rnd.RndPercent( 50 )) ? RANDOM_TRADE_QTY_1 : RANDOM_TRADE_QTY_2;

		SecurityIndex = m_rnd.RndInt64Range( 0, m_pPriceBoard->m_iNumberOfSecurities - 1 );
		TickerEntry.price_quote = (m_pPriceBoard->GetCurrentPrice( SecurityIndex )).DollarAmount();
		m_pPriceBoard->GetSymbol( SecurityIndex, TickerEntry.symbol, sizeof(TickerEntry.symbol) );

		AddToBatch( &TickerEntry );
		TotalEntryCount++;
	}
}

void CMEETickerTape::AddToBatch( PTickerEntry pTickerEntry )
{
	m_TxnInput.Entries[m_BatchIndex++] = *pTickerEntry;
	if( max_feed_len == m_BatchIndex )
	{
		// Buffer is full, time for Market-Feed.
		m_pSUT->MarketFeed( &m_TxnInput );
		m_BatchIndex = 0;
	}
}

eTradeTypeID CMEETickerTape::ConvertTradeTypeIdToEnum( char* pTradeType )
{
	// Convert character trade type to enumeration
	switch( pTradeType[0] )
	{
	case 'T':
		switch( pTradeType[1] )
		{
		case 'L':
			switch( pTradeType[2] )
			{
			case 'B':
				return( eLimitBuy );
				break;
			case 'S':
				return( eLimitSell );
				break;
			default:
				break;
			}
			break;
		case 'M':
			switch( pTradeType[2] )
			{
			case 'B':
				return( eMarketBuy );
				break;
			case 'S':
				return( eMarketSell );
				break;
			default:
				break;
			}
			break;
		case 'S':
			switch( pTradeType[2] )
			{
			case 'L':
				return( eStopLoss );
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	// Throw exception - should never get here
	assert(false);	// this should never happen

	return eMarketBuy;	// silence compiler warning about not all control paths returning a value
}
