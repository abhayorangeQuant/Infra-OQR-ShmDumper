#include "iostream"
#include "fstream"
#include "string"
#include "time.h"
#include "iomanip"

#include "MakeBar.h"

MakeBar::MakeBar()
{
	strcpy(m_szFileName, "");
	strcpy(m_szTickerName, "");
	m_bCheckFutureValue = true;
	m_bSaveData = false;
	m_eExchangeType = NONE_EXCHANGE;
	m_iStartSeconds = 0;
	m_Bars.push_back(new Bar(m_szTickerName, m_eExchangeType, m_iStartSeconds, m_szFileName, 60));
}

MakeBar::MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader)
{
	strcpy(m_szFileName, "");
	strcpy(m_szTickerName, szTicker);
	m_eExchangeType = eExchangeType;
	m_bCheckFutureValue = true;
	m_bSaveData = false;
	
	// note this will make 9:00 as default bar start time, this is an important feild in setting file
	int iBarStartHour = reader->GetInt("state", "BarStartHour", 9);
	int iBarStartMin = reader->GetInt("state", "BarStartMin", 0);
	m_iStartSeconds = (iBarStartHour*60 + iBarStartMin)*60;
	m_Bars.push_back(new Bar(m_szTickerName, m_eExchangeType, m_iStartSeconds, m_szFileName, 60));
}

#ifdef TRADE_SIM

MakeBar::MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader, long startTime)
{
	strcpy(m_szFileName, "");
	strcpy(m_szTickerName, szTicker);
	m_eExchangeType = eExchangeType;
	m_bCheckFutureValue = true;
	m_bSaveData = false;
	m_lStartEpoch = startTime;
//	std::cout << "in makebar " << m_szTickerName << startTime << std::endl;
	int iBarStartHour = reader->GetInt("state", "BarStartHour", 9);
	int iBarStartMin = reader->GetInt("state", "BarStartMin", 0);
	m_iStartSeconds = (iBarStartHour*60 + iBarStartMin)*60;
	m_Bars.push_back(new Bar(m_szTickerName, m_eExchangeType, m_iStartSeconds, m_szFileName, 60));
}

#endif

MakeBar::MakeBar(char* szTicker, exchange_type eExchangeType, SettingsReader* reader, char* szFileName)
{
	strcpy(m_szFileName, szFileName);
	strcpy(m_szTickerName, szTicker);
	strcat(m_szFileName, szTicker);
	m_eExchangeType = eExchangeType;

	m_bCheckFutureValue = true;
	m_bSaveData = false;
	int iBarStartHour = reader->GetInt("state", "BarStartHour", 9);
	int iBarStartMin = reader->GetInt("state", "BarStartMin", 0);
	m_iStartSeconds = (iBarStartHour*60 + iBarStartMin)*60;
	m_Bars.push_back(new Bar(m_szTickerName, m_eExchangeType, m_iStartSeconds, m_szFileName, 60));
}

MakeBar::~MakeBar()
{
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		delete m_Bars[ii];
	}
}

void MakeBar::PrintBar(long lFrameInterval, int iStartBarIndex, int iEndBarIndex = 0, bool bNewLine = true)
{
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		if(m_Bars[ii]->GetFrameInterval() == lFrameInterval)
		{
			m_Bars[ii]->PrintBar(iStartBarIndex, iEndBarIndex, bNewLine);
			break;
		}
	}
}

int MakeBar::UpdateBar(MarketUpdates* update)
{
	int iBarUpdate = 0;
	m_lastupdate = *update;
	/*
	long lUpdateTime = update->m_lLastTradeTime;
	double dPrice = update->m_dLastTradePrice;
	long lVolume;

	if((update->m_lLastTradeTime - m_lPreviousTime) > 12*60*60)
	{
		lVolume = update->m_lLastTradeQuantity;
	}
	else
	{
		lVolume = (update->m_ulTotalTradedQuantity - m_lPreviousTTQ);
	}
	
	m_lPreviousTime = update->m_lLastTradeTime;
	m_lPreviousTTQ = update->m_ulTotalTradedQuantity;

	if(lVolume <=0 )
	{
		// no new trade hence ignore update
		return -1;
	}
	*/
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		if(m_Bars[ii]->UpdateBar(update) == -1)
		{
			iBarUpdate = -1;
		}
	}
	return iBarUpdate;
}

int MakeBar::ReadDataFromDB(DBConnector* DBConnection, int iMethod, int iOldRecordCount = 0, unsigned long ulOldEpoch = 0)
{
	int iRecordCount = 0;
	m_pdbMinData = NULL;

	if(DBConnection == NULL)
	{
		std::cout << "Unable to fetch old data from Database" << std::endl;
		return -1;
	}
	if(iMethod == DB_READ_OLD_DATA_WITH_EPOCH)
	{
#ifndef TRADE_SIM
		iRecordCount = DBConnection->fetchMinutelyDataRecords_epoch(&m_pdbMinData, m_eExchangeType, m_szTickerName, ulOldEpoch, 0);
#else
		iRecordCount = DBConnection->fetchMinutelyDataRecords_epoch(&m_pdbMinData, m_eExchangeType, m_szTickerName, ulOldEpoch, m_lStartEpoch);
#endif
	}
	else if(iMethod == DB_READ_OLD_DATA_WITH_RECORD_COUNT)
	{
#ifndef TRADE_SIM
		iRecordCount = DBConnection->fetchMinutelyDataRecords_records(&m_pdbMinData, m_eExchangeType, m_szTickerName, iOldRecordCount);
#else
		iRecordCount = DBConnection->fetchMinutelyDataRecords_sim(&m_pdbMinData, m_eExchangeType, m_szTickerName, m_lStartEpoch, iOldRecordCount);
#endif
	}
	else
	{
		std::cout << "Unknown method set not reading old data from database" << std::endl;
		iRecordCount = 0;
		m_pdbMinData = NULL;
	}
	return iRecordCount;
}

void MakeBar::SetDBConnector(DBConnector* DBConnection, int iMethod = DB_READ_OLD_DATA_WITH_RECORD_COUNT, int iOldRecordCount = 5000, unsigned long ulOldEpoch = 0)
{
	// so that we query database only once and not for all timesframes individually
	int iMaxRecord = ReadDataFromDB(DBConnection, iMethod, iOldRecordCount, ulOldEpoch);
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		// now this is required only for storing the data not for querying
		m_Bars[ii]->SetDBConnector(DBConnection);
		m_Bars[ii]->UpdateFromDBData(m_pdbMinData, iMethod, iMaxRecord);
	}
	if(m_pdbMinData != NULL)
	{
		// delete memory assigned by the DB connector
		delete m_pdbMinData;
	}
}

int MakeBar::SetPrintBar(bool bPrintBar)
{
	
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		m_Bars[ii]->SetPrintBar(bPrintBar);
	}
	return 0;
}
int MakeBar::CreateFrame(long lFrameInterval)
{
	int iReturnIndex = -1;
	
	for(int ii = 0; ii < m_Bars.size(); ii++)
	{
		if(m_Bars[ii]->GetFrameInterval() == lFrameInterval)
		{
			std::cout << "Timeframe already exists" << std::endl;
			return iReturnIndex;
		}
	}
	std::cout << "New time frame for interval " << lFrameInterval << " created" << std::endl;
	m_Bars.push_back(new Bar(m_szTickerName, m_eExchangeType, m_iStartSeconds, m_szFileName, lFrameInterval));
	iReturnIndex = m_Bars.size()-1;
	
	return iReturnIndex;
}

int MakeBar::SetSaveData(bool bSaveFlag, bool bSaveDB)
{
	m_bSaveData = bSaveFlag;
	m_bSaveDB = bSaveDB;
	// since only 1 min data will be saved which is created in constructor
	m_Bars[0]->SetSaveData(bSaveFlag, bSaveDB);
	return 0;
}


int MakeBar::SaveDataToFile()
{
	if(!strcmp(m_szFileName, ""))
	{
		return -1;
	}
	m_Bars[0]->SaveDataToFile();
	return 0;
}
