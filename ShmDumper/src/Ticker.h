#ifndef _TICKER_HEADER
#define _TICKER_HEADER

#include "MakeBar.h"

class Strategy;

class Ticker
{
public:
	Ticker(Strategy*, std::string, exchange_type);
	~Ticker();
	int ProcessResponse(StrategyResponse*);
	int MarketDataUpdate(MarketUpdates*);
private:
	MakeBar* m_bars;

	std::string m_szScripCode;
	exchange_type m_exchType;
	MarketUpdates m_lastUpdate;

};

#endif
