#ifndef _BAR_H
#define _BAR_H

#include "iostream"
#include "fstream"
#include "iomanip"

#include "../Common/StrategyMsgs.h"
#include "../Database/DBConnector.h"

enum ePriceType
{
	PriceClose,
	PriceHigh,
	PriceLow,
	//(High+Low)/2
	PriceHighLow,
	// (High+Low+Close)/3
	PriceTypical,
	PriceCustom
};

// provision for one year
//#define MAX_BAR_COUNT 14*60*252
#define MAX_BAR_COUNT 14*60*10

#define DB_READ_OLD_DATA_WITH_EPOCH 1
#define DB_READ_OLD_DATA_WITH_RECORD_COUNT 2

class Bar
{

public:
	Bar();
	Bar(char* szTicker, exchange_type eExchangeType, int iStartSec, char* szFileName, long lTimeFrameInterval);
	~Bar();

	inline long DayStartSecs(long lTime);
	// please note this should be called for only one ticker
	// multiple instances for multiple tickers should be created
	int UpdateBar(MarketUpdates* update);
	// to load data read from file
	int UpdateBar(long lUpdateTime, double dOpen, double dHigh, double dLow, double dClose, long lVolume, long lOpenInterest);
	// to save data at every completed min
	int SetSaveData(bool bSaveFlag, bool bSaveDB);
	// to print bar data to screen
	int SetPrintBar(bool bPrintBar);

	// make sure this is called after database query for historic data
	long GetMaxBarCount() { return m_lMaxbarCount; }

	long GetFrameInterval() { return m_lTimeFrameInterval; }
	long GetLastUpdateTime() { return m_lUpdateTime; }
	// for indicators to access data
	int* GetBarIndex() { return &m_iBarIndex; }
	double* GetOpenArray() { return m_pdOpen; }
	double* GetHighArray() { return m_pdHigh; }
	double* GetLowArray() { return m_pdLow; }
	double* GetCloseArray() { return m_pdClose; }
	double* GetHighLowArray() { return m_pdHighLow; }
	double* GetTypicalArray() { return m_pdTypicalPrice; }
	long* GetVolumeArray() { return m_plVolume; }
	long* GetOpenInterestArray() { return m_plOpenInterest; }
	time_t* GetTimeArray() { return m_plTime; }
	long GetTimeSinceBarStart()
	{
		return((long)(m_lUpdateTime - m_plTime[m_iBarIndex]));
	}

	double GetPriceFromTime(ePriceType, long);

	int SaveDataToFile();
	int SaveDataToDB();
	int UpdateFromDBData(DBMinutelyData*, int, int);
	void SetDBConnector(DBConnector* DBConnection);
	void PrintBar(int iStartBarIndex, int iEndBarIndex, bool bNewLine);
	void SaveBar(int iStartBarIndex, int iEndBarIndex, bool bNewLine);

private:

	char m_szTickerName[50];
	bool m_bCheckFutureValue;
	char m_szFileName[500];
	bool m_bSaveData;
	bool m_bSaveDB;
	bool m_bPrintBar;
	long m_lTimeFrameInterval;
	long m_lUpdateTime;
	long m_StartEpoch;
	int m_iStartSeconds;
	int m_iBarIndex;
	exchange_type m_eExchangeType;
	std::fstream m_fpDataFile;
	DBConnector* m_DBConnection;

	// minutely data
	long m_lMaxbarCount;
	time_t* m_plTime;
	double* m_pdHigh;
	double* m_pdLow;
	double* m_pdOpen;
	double* m_pdClose;
	double* m_pdHighLow;
	double* m_pdTypicalPrice;
	long* m_plVolume;
	long* m_plOpenInterest;
	// this is to store the data in the database
	DBMinutelyData* m_dbMinData;
	long m_lPreviousTime;
	long m_lPreviousTTQ;

	int ReadDataFromFile();
	void RemoveOldBars(long, long);
	void AllocateMemory();
};

#endif
