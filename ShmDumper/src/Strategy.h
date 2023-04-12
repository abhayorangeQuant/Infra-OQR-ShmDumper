#ifndef _STRATEGY_HEADER
#define _STRATEGY_HEADER

#include "map"

#include "Ticker.h"
#include "StrategyBase.h"

typedef std::map<unsigned long, Ticker*> TickerMap;
typedef std::map<unsigned long, Ticker*>::iterator TickerMapIter;

typedef struct _exchangeAlertCounters
{
	void clear()
	{
		m_lCounter = 0;
		m_iMailAlertCount = 0;
		m_ulExchangeTimeStamp = 0;
	}
	_exchangeAlertCounters()
	{
		clear();
		m_ulStartMin = -1;
		m_ulEndMin = -1;
		m_ulExchangeTimeStamp = 0;
	}
	long m_lCounter;
	unsigned long m_ulExchangeTimeStamp;
	int m_iMailAlertCount;
	unsigned long m_ulStartMin;
	unsigned long m_ulEndMin;
}ExchangeAlertCounters;

typedef std::map<exchange_type, ExchangeAlertCounters> ExchangeCounterMap;
typedef std::map<exchange_type, ExchangeAlertCounters>::iterator ExchangeCounterMapIter;


class Strategy : public StrategyBase
{
private:
	int m_iStartHour;
	int m_iEndHour;
	SettingsReader* m_reader;
	std::string m_szFilePath;
	MarketUpdates m_lastUpdate;
	FILE* marketDataFile;
	bool m_bSaveToFile;
	bool m_bCheckSanity;
	ExchangeCounterMap m_ExchangeCounter;
protected:

public:
	Strategy(SettingsReader*, bool);
	~Strategy();
	
	int ProcessResponse(StrategyResponse*);
	int MarketDataUpdate(MarketUpdates*);

	int Run();
	bool CheckSanity(MarketUpdates*);
	SettingsReader* GetSettingsReader()
	{
		return m_reader;
	}
};
#endif
