#ifndef _MAKE_BAR_H
#define _MAKE_BAR_H

#include "iomanip"
#include "vector"

#include "../Common/StrategyMsgs.h"
#include "Bar.h"
#include "../Common/SettingFileReader.h"

class Indicators;

// we will maintain epoch as base index here
// min bar interval will of 1 min = 60sec
// should be able to create multiple timeframe bar using the base bar

class MakeBar
{

public:
	MakeBar();
	MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader);
#ifdef TRADE_SIM
	MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader, long startTime);
#endif
	MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader, char* szFileName);
	~MakeBar();

	// cache the latest update can be used by any class
	MarketUpdates m_lastupdate;
	// please note this should be called for only one ticker
	// multiple instances for multiple tickers should be created
	int UpdateBar(MarketUpdates* update);
	// to create a new time frame
	int CreateFrame(long lFrameInterval);
	// to save data at every completed min
	int SetSaveData(bool bSaveFlag, bool bSaveDB = false);
	void SetDBConnector(DBConnector* DBConnection, int, int, unsigned long);
	int SetPrintBar(bool bPrintBar);

	int SaveDataToFile();
	void PrintBar(long lFrameInterval, int iStartBarIndex, int iEndBarIndex, bool bNewLine);

	friend class Indicators;
	friend class Signal;
	friend class Symbol;

protected:	
	std::vector<Bar*> m_Bars;
	typedef std::vector<Bar*>::iterator BarIter;
	DBMinutelyData* m_pdbMinData;	
private:

	char m_szTickerName[50];
	exchange_type m_eExchangeType;
	bool m_bCheckFutureValue;
	char m_szFileName[500];
#ifdef TRADE_SIM
	long m_lStartEpoch;
#endif
	bool m_bSaveData;
	bool m_bSaveDB;
	long m_lPreviousTime;
	long m_lPreviousTTQ;	
	int m_iStartSeconds;

	int ReadDataFromDB(DBConnector*, int, int, unsigned long);
};

#endif
