#include "Ticker.h"
#include "Strategy.h"
#include "MakeBar.h"

Ticker::~Ticker()
{

}

Ticker::Ticker(Strategy* strategy, std::string szScripCode, exchange_type exchType)
{
	m_szScripCode = szScripCode;   
	m_exchType = exchType;

	m_bars = new MakeBar((char*)m_szScripCode.c_str(), m_exchType, strategy->GetSettingsReader());	
	m_bars->SetSaveData(false, true);
	m_bars->SetDBConnector(strategy->GetDBConnector(), DB_READ_OLD_DATA_WITH_RECORD_COUNT, 0, 0);
}

int Ticker::ProcessResponse(StrategyResponse* response)
{
	std::cout << "Ignored response " << response->m_eResponseType << std::endl;
	return 0;
}

int Ticker::MarketDataUpdate(MarketUpdates* update)
{
	//m_ulExchangeTimeStamp = update->m_ulExchangeTimeStamp;
	update->print();
	m_lastUpdate = *update;
	m_bars->UpdateBar(update);
	return 0;
}

