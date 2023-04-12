#ifndef _MY_SQL_H_
#define _MY_SQL_H_

#include <mysql/mysql.h> 
#include <stdio.h> 
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "stdint.h"
#include "map"
#include "SettingFileReader.h"
#include "StrategyMsgs.h"

using namespace std;

typedef struct _contractData
{
        void clear()
        {
                memset(m_szScripCode, 0, sizeof(m_szScripCode));
                m_fStrikePrice = 0;
                m_iToken = 0;
                m_iExchangeType = 0;
                m_iLotSize = 0;
                m_iTickSize = 0;
                m_fPriceFactor = 0;
                m_fPriceMultiplier = 0;
		m_iPriceBand = 0;
		m_fClosePrice = 0;
		m_dTradedValue = 0;
		m_lTradedContracts = 0;
		m_lOpenInterest = 0;
        }
        _contractData()
        {
                clear();
        }
        char    m_szScripCode[50];
        char    m_szOptionType[3];
        float   m_fStrikePrice;
        int     m_iToken;
        int     m_iExchangeType;
        int     m_iLotSize;
        int     m_iTickSize;
        float   m_fPriceFactor;
        float   m_fPriceMultiplier;
	int	m_iPriceBand;
	float 	m_fClosePrice;
	double 	m_dTradedValue;
	long 	m_lTradedContracts;
	long 	m_lOpenInterest;
}ContractData;

typedef std::map<std::string, ContractData> SymbolNameMap;
typedef std::map<std::string, ContractData>::iterator SymbolNameMapIter;
typedef std::map<long, ContractData> TokenNameMap;
typedef std::map<long, ContractData>::iterator TokenNameMapIter;

typedef struct _trade_struct
{
	_trade_struct()
	{
		memset(m_szStrategyName, 0, sizeof(m_szStrategyName));
		memset(m_szClientCode, 0, sizeof(m_szClientCode));
		memset(m_szScripCode, 0, sizeof(m_szScripCode));
		memset(m_szOptionType, 0, sizeof(m_szOptionType));
		m_eExchangeType = NONE_EXCHANGE;
		m_lExpiryDate = 0;
		m_iStrikePrice = 0;
		m_dPrice = 0;
		m_lQuantity = 0;
		memset(m_szTimeStamp, 0, sizeof(m_szTimeStamp));
		memset(m_szTradeDate, 0, sizeof(m_szTradeDate));
		m_ullExchangeOrderID = 0;
		m_ulLocalOrderID = 0;
		m_ulExchangeTradeID = 0;
		m_cTransactionType = '0';
	}
	void clear()
	{
		_trade_struct();
	}
	char m_szStrategyName[20];
	char m_szClientCode[20];
	char m_szScripCode[50];
	exchange_type m_eExchangeType;
	long m_lExpiryDate;
	char m_szOptionType[2];
	int m_iStrikePrice;
	double m_dPrice;
	long m_lQuantity;
	char m_szTimeStamp[20];
	char m_szTradeDate[20];
	char m_cTransactionType;
	unsigned long long m_ullExchangeOrderID;
	unsigned long m_ulLocalOrderID;
	unsigned long m_ulExchangeTradeID;
}DBTrade;

typedef struct _minutely_data_struct
{
	_minutely_data_struct()
	{
		m_ulExchangeTimeStamp = 0;	
		memset(m_szScripCode, 0, sizeof(m_szScripCode));
		m_eExchangeType = NONE_EXCHANGE;
		m_dPriceOpen = 0;
		m_dPriceHigh = 0;
		m_dPriceLow = 0;
		m_dPriceClose = 0;
		m_lVolume = 0;
		m_lOpenInterest = 0;
	}
	void clear()
	{
		_minutely_data_struct();
	}
	unsigned long m_ulExchangeTimeStamp;
	char m_szScripCode[50];
	exchange_type m_eExchangeType;
	double m_dPriceOpen;
	double m_dPriceHigh;
	double m_dPriceLow;
	double m_dPriceClose;
	long m_lVolume;
	long m_lOpenInterest;
}DBMinutelyData;

class DBConnector
{
private:
	MYSQL m_MySql;
	pthread_mutex_t* m_MySqlLock;
public:
	bool insertTradeData(DBTrade*);
	bool insertMinutelyData(DBMinutelyData*);
	// this query will udpate contract details and update the map with it (in case of EQ will update price bands too)
	int fetchContractDataPriceBands(int);
	int fetchContractData(char*, int);
	int fetchTradedValue(int);

	// Please note the pointer of records below needs to be deleted by the caller
	int fetchMinutelyDataRecords_records(DBMinutelyData**, int, char*, int);
	int fetchMinutelyDataRecords_epoch(DBMinutelyData**, int, char*, unsigned long, unsigned long);

#ifdef TRADE_SIM	
	int fetchMinutelyDataRecords_sim(DBMinutelyData**, int, char*, unsigned long, int);
	unsigned long fetchMinDataSimulator(DBMinutelyData**, int, unsigned long, unsigned long,  bool bCountOnly = false);
#endif
	int fetchTradeRecords(DBTrade**, char*, char*);
	DBConnector(SettingsReader*);
};
#endif
