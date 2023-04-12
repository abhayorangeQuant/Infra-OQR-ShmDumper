#include "iostream"
#include "fstream"
#include "string"
#include "time.h"
#include "iomanip"

#include "Bar.h"

Bar::Bar()
{
	strcpy(m_szFileName, "");
	strcpy(m_szTickerName, "");
	m_lTimeFrameInterval = 60;
	m_bCheckFutureValue = true;
	m_eExchangeType = NONE_EXCHANGE;
	m_bSaveData = false;
	m_bSaveDB = false;
	m_bPrintBar = false;

	m_StartEpoch = 0;
	m_iBarIndex = -1;
	m_lPreviousTime = 0;
	m_lPreviousTTQ = 0;

	// for 24 hours market
	m_lMaxbarCount = (24*60*60)/m_lTimeFrameInterval;

	m_plTime = NULL;
	m_pdHigh = NULL;
	m_pdLow = NULL;
	m_pdOpen = NULL;
	m_pdClose = NULL;
	m_pdHighLow = NULL;
	m_pdTypicalPrice = NULL;
	m_plVolume = NULL;
	m_plOpenInterest = NULL;
	m_DBConnection = NULL;
	m_dbMinData = new DBMinutelyData();
}

Bar::Bar(char* szTicker, exchange_type eExchangeType, int iStartSec, char* szFileName, long lTimeFrameInterval)
{
	strcpy(m_szFileName, szFileName);
	strcpy(m_szTickerName, szTicker);
	m_eExchangeType = eExchangeType;
	m_lTimeFrameInterval = lTimeFrameInterval;
	m_bCheckFutureValue = true;

	m_iStartSeconds = iStartSec;
	m_iBarIndex = -1;
	m_lPreviousTime = 0;
	// for 24 hours market
	m_lMaxbarCount = (24*60*60)/m_lTimeFrameInterval;

	m_plTime = NULL;
	m_pdHigh = NULL;
	m_pdLow = NULL;
	m_pdOpen = NULL;
	m_pdClose = NULL;
	m_pdHighLow = NULL;
	m_pdTypicalPrice = NULL;
	m_plVolume = NULL;
	m_plOpenInterest = NULL;
	m_DBConnection = NULL;
	m_bSaveDB = false;
	m_bPrintBar = false;
	m_dbMinData = new DBMinutelyData();
}

Bar::~Bar()
{
	free(m_plTime);
	free(m_pdHigh);
	free(m_pdLow);
	free(m_pdOpen);
	free(m_pdClose);
	free(m_pdHighLow);
	free(m_pdTypicalPrice);
	free(m_plVolume);
	free(m_plOpenInterest);
	delete m_dbMinData;
}

void Bar::AllocateMemory()
{
	std::cout << "Allocating memory for " << m_szTickerName << "|" << " bar count " << m_lMaxbarCount << std::endl;
	m_plTime = (time_t*)calloc(m_lMaxbarCount, sizeof(time_t));
	m_pdHigh = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_pdLow = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_pdOpen = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_pdClose = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_pdHighLow = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_pdTypicalPrice = (double*)calloc(m_lMaxbarCount, sizeof(double));
	m_plVolume = (long*)calloc(m_lMaxbarCount, sizeof(long));
	m_plOpenInterest = (long*)calloc(m_lMaxbarCount, sizeof(long));
}

void Bar::PrintBar(int iStartBarIndex, int iEndBarIndex = 0, bool bNewLine = true)
{
	struct tm* timeinfo = NULL;
	long lTimeFrameInterval;

	if(m_bPrintBar == false)
	{
		return;
	}

	if(!iEndBarIndex)
	{
		iEndBarIndex = iStartBarIndex+1;
	}

	for(int kk=iStartBarIndex; kk<iEndBarIndex; kk++)
	{
		timeinfo = localtime((const time_t*)&m_plTime[kk]);
		std::cout << m_szTickerName << " , " << m_lTimeFrameInterval << " , " ;
		std::cout.precision(2);
		std::cout << std::fixed << std::showpoint;
		std::cout << timeinfo->tm_mon+1 << "/" << timeinfo->tm_mday << "/" << timeinfo->tm_year+1900 << " , " 
			<< std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_min << " , "
			<< m_pdOpen[kk] << " , "
			<< m_pdHigh[kk] << " , "
			<< m_pdLow[kk] << " , "
			<< m_pdClose[kk] << " , "
			<< m_plVolume[kk] << " , "
			<< m_plOpenInterest[kk]; 
			if(bNewLine)
			{
				std::cout << std::endl;
			}
			else
			{
				std::cout << " , ";
			}
	}
}

void Bar::SaveBar(int iStartBarIndex, int iEndBarIndex = 0, bool bNewLine = true)
{
	struct tm* timeinfo = NULL;
	long lTimeFrameInterval;

	if(m_lTimeFrameInterval != 60)
	{
		// save data only for 1 min bars
		return;
	}

	if(m_bSaveData && m_fpDataFile.good())
	{
		if(!iEndBarIndex)
		{
			iEndBarIndex = iStartBarIndex+1;
		}

		for(int kk=iStartBarIndex; kk<iEndBarIndex; kk++)
		{
			timeinfo = localtime((const time_t*)&m_plTime[kk]);
			m_fpDataFile << m_plTime[kk] << " , " << m_szTickerName << " , " ;
			m_fpDataFile.precision(2);
			m_fpDataFile << std::fixed << std::showpoint;
			m_fpDataFile << timeinfo->tm_mon+1 << "/" << timeinfo->tm_mday << "/" << timeinfo->tm_year+1900 << " , " 
				<< std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_min << " , "
				<< m_pdOpen[kk] << " , "
				<< m_pdHigh[kk] << " , "
				<< m_pdLow[kk] << " , "
				<< m_pdClose[kk] << " , "
				<< m_plVolume[kk] << " , "
				<< m_plOpenInterest[kk]; 
				if(bNewLine)
				{
					m_fpDataFile << std::endl;
				}
				else
				{
					m_fpDataFile << " , ";
				}
		}
	}
	
	if(m_bSaveDB && m_DBConnection != NULL)
	{
		if(!iEndBarIndex)
		{
			iEndBarIndex = iStartBarIndex+1;
		}

		for(int kk=iStartBarIndex; kk<iEndBarIndex; kk++)
		{

			m_dbMinData->clear();
			m_dbMinData->m_ulExchangeTimeStamp = m_plTime[kk]; 
			strcpy(m_dbMinData->m_szScripCode, m_szTickerName);
			m_dbMinData->m_eExchangeType = m_eExchangeType;
			m_dbMinData->m_dPriceOpen = m_pdOpen[kk];
			m_dbMinData->m_dPriceHigh = m_pdHigh[kk];
			m_dbMinData->m_dPriceLow = m_pdLow[kk]; 
			m_dbMinData->m_dPriceClose = m_pdClose[kk];
			m_dbMinData->m_lVolume = m_plVolume[kk];
			m_dbMinData->m_lOpenInterest = m_plOpenInterest[kk];
			m_DBConnection->insertMinutelyData(m_dbMinData);
		}
	}
}

inline long Bar::DayStartSecs(long lTime)
{
	long retTime = 0;
	retTime = (long)(lTime/(24*60*60));
	retTime = (retTime*24*60*60)-19800;
	return retTime;
}

int Bar::UpdateBar(MarketUpdates* update)
{
	m_lUpdateTime = update->m_lLastTradeTime;
	if(m_lUpdateTime == 0)
	{
		// at start there might be no trades hence 0
		m_lUpdateTime = update->m_ulExchangeTimeStamp;
	}
	double dPrice = update->m_dLastTradePrice;
	long lVolume;

	if(!m_lPreviousTime)
	{
		//strcpy(m_szTickerName, update->m_szScripName);
		RemoveOldBars(m_lUpdateTime, m_lTimeFrameInterval);
	}

	if(((update->m_lLastTradeTime - m_lPreviousTime) > 12*60*60) && update->m_ulTotalTradedQuantity != 0)
	{
		lVolume = update->m_lLastTradeQuantity;
	}
	else
	{
		lVolume = (update->m_ulTotalTradedQuantity - m_lPreviousTTQ);
	}
	
	m_lPreviousTime = m_lUpdateTime;
	m_lPreviousTTQ = update->m_ulTotalTradedQuantity;

	if(lVolume <=0)
	{
		// no new trade hence ignore update
		return -1;
	}

	if((m_iBarIndex == -1) || ((m_lUpdateTime - DayStartSecs(m_lUpdateTime) - m_iStartSeconds)/m_lTimeFrameInterval != (m_plTime[m_iBarIndex] - DayStartSecs(m_plTime[m_iBarIndex]) - m_iStartSeconds)/m_lTimeFrameInterval))
	{
		//create new bar
		m_iBarIndex++;
		m_plTime[m_iBarIndex] = ((long)((m_lUpdateTime - DayStartSecs(m_lUpdateTime) - m_iStartSeconds)/m_lTimeFrameInterval))*m_lTimeFrameInterval + m_iStartSeconds + DayStartSecs(m_lUpdateTime);

		m_pdHigh[m_iBarIndex] = dPrice;
		m_pdLow[m_iBarIndex] = dPrice;
		m_pdOpen[m_iBarIndex] = dPrice;
		m_pdClose[m_iBarIndex] = dPrice;
		m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex]+m_pdLow[m_iBarIndex])/2;
		m_pdTypicalPrice[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex] + m_pdClose[m_iBarIndex])/3;
		m_plVolume[m_iBarIndex] = lVolume;
		m_plOpenInterest[m_iBarIndex] = update->m_dOpenInterest;
		if(m_iBarIndex != 0)
		{
			PrintBar(m_iBarIndex-1);
			if((m_bSaveData && m_fpDataFile.good()) || (m_bSaveDB && m_DBConnection != NULL))
			{
				SaveBar(m_iBarIndex-1);
			}
		}
	}
	else
	{
		if(m_pdHigh[m_iBarIndex] < dPrice)
		{
			m_pdHigh[m_iBarIndex] = dPrice;
			m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex])*0.5;
		}
		else if(m_pdLow[m_iBarIndex] > dPrice)
		{
			m_pdLow[m_iBarIndex] = dPrice;
			m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex])*0.5;
		}
		m_pdClose[m_iBarIndex] = dPrice;
		m_pdTypicalPrice[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex] + m_pdClose[m_iBarIndex])/3;
		m_plVolume[m_iBarIndex] += lVolume;
		m_plOpenInterest[m_iBarIndex] = update->m_dOpenInterest;
	}
	return 0;
}

int Bar::UpdateBar(long lUpdateTime, double dOpen, double dHigh, double dLow, double dClose, long lVolume, long lOpenInterest)
{
	
	if(lVolume <=0 )
	{
		// no new trade hence ignore update
		return -1;
	}

	if((m_iBarIndex == -1) || ((lUpdateTime - DayStartSecs(lUpdateTime) - m_iStartSeconds)/m_lTimeFrameInterval != (m_plTime[m_iBarIndex] - DayStartSecs(m_plTime[m_iBarIndex]) - m_iStartSeconds)/m_lTimeFrameInterval))
	{
		//create new bar
		m_iBarIndex++;
		m_plTime[m_iBarIndex] = ((long)((lUpdateTime - DayStartSecs(lUpdateTime) - m_iStartSeconds)/m_lTimeFrameInterval))*m_lTimeFrameInterval + m_iStartSeconds + DayStartSecs(lUpdateTime);


		m_pdHigh[m_iBarIndex] = dHigh;
		m_pdLow[m_iBarIndex] = dLow;
		m_pdOpen[m_iBarIndex] = dOpen;
		m_pdClose[m_iBarIndex] = dClose;
		m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex]+m_pdLow[m_iBarIndex])/2;
		m_pdTypicalPrice[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex] + m_pdClose[m_iBarIndex])/3;
		m_plVolume[m_iBarIndex] = lVolume;
		m_plOpenInterest[m_iBarIndex] = lOpenInterest;
		if(m_iBarIndex != 0)
		{
			PrintBar(m_iBarIndex-1);
		}
	}
	else
	{
		if(m_pdHigh[m_iBarIndex] < dHigh)
		{
			m_pdHigh[m_iBarIndex] = dHigh;
			m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex])*0.5;
		}
		if(m_pdLow[m_iBarIndex] > dLow)
		{
			m_pdLow[m_iBarIndex] = dLow;
			m_pdHighLow[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex])*0.5;
		}
		m_pdClose[m_iBarIndex] = dClose;
		m_pdTypicalPrice[m_iBarIndex] = (m_pdHigh[m_iBarIndex] + m_pdLow[m_iBarIndex] + m_pdClose[m_iBarIndex])/3;
		m_plVolume[m_iBarIndex] += lVolume;
		m_plOpenInterest[m_iBarIndex] = lOpenInterest;
	}
	return 0;
}

int Bar::SetSaveData(bool bSaveFlag, bool bSaveDB = false)
{
	m_bSaveData = bSaveFlag;
	m_bSaveDB = bSaveDB;
	return 0;
}

int Bar::SetPrintBar(bool bPrintBar)
{
	m_bPrintBar = bPrintBar;
	return 0;
}

double Bar::GetPriceFromTime(ePriceType priceType, long lEpochTime)
{
	// Linear search
	int iIndex = m_iBarIndex;
	while(iIndex >= 0)
	{
		if(m_plTime[iIndex] <= lEpochTime)
		{
			break;
		}       
		iIndex--;
	}
/*
	// binary search
	int iEndIndex = m_iBarIndex;
	int iStartIndex = 0;
	int iIndex = 0;
	while(iStartIndex < iEndIndex && (iStartIndex < m_iBarIndex) && (iEndIndex >= 0))
	{
		iIndex = (iStartIndex + iEndIndex)/2;
		if(m_plTime[iIndex] <= lEpochTime && m_plTime[iIndex+1] > lEpochTime)
                {       
                        break;
                }
		if(m_plTime[iIndex] > lEpochTime)
		{
			iStartIndex = iIndex+1;
		}
		else
		{
			iEndIndex = iIndex-1;
		}
	}
*/
	switch(priceType)
	{
		case PriceClose:        return m_pdClose[iIndex];       break;
		case PriceHigh:         return m_pdHigh[iIndex];        break;
		case PriceLow:          return m_pdLow[iIndex];         break;
		case PriceHighLow:      return m_pdHighLow[iIndex]; break;
		case PriceTypical:      return m_pdTypicalPrice[iIndex];    break;
		default:                return m_pdClose[iIndex];       break;
	}
}

void Bar::RemoveOldBars(long lTimeNow, long lFrameInterval)
{
	/*
	int iStart = m_iBarIndex;
	for(int i = m_iBarIndex; i>=0; i--)
	{
		if(m_plTime[i] > lTimeNow)
		{
			m_iBarIndex--;
		}
		else
		{
			break;
		}
	}
	if(!m_iBarIndex)
	{
		m_iBarIndex = -1;
	}
	std::cout << "Removed " << (iStart-m_iBarIndex) << std::endl;
	*/

	std::cout << "Old bars first entry as " << m_plTime[0] << " and last entry as " <<  m_plTime[m_iBarIndex] << " time now " << lTimeNow << std::endl; 
	int iStart = m_iBarIndex;
	for(int i = 0; i<= iStart; i++)
	{
		if(m_plTime[i] < lTimeNow)
		{
			m_iBarIndex = i;
		}
		else
		{
			std::cout << "Old bars removed from " << m_plTime[i] << " , " << i << " , " << m_plTime[i-1] << std::endl;
			break;
		}
	}
	if(m_iBarIndex == 0)
	{
		m_iBarIndex = -1;
	}
	std::cout << "Removed " << (iStart-m_iBarIndex) << " from interval " << lFrameInterval << std::endl;
}

int Bar::UpdateFromDBData(DBMinutelyData* pdbMinData, int iMethod, int iRecordCount)
{
	// set epoch in this function
	m_StartEpoch = 0;
	m_iBarIndex = -1;
	m_lPreviousTTQ = 0;
	bool bFoundEODMarker = false;

	if(iRecordCount > 0)
	{
		// so that we allocate memory as per each symbol requirement, record count is from 1 min frame perspective
		if(m_lTimeFrameInterval == 60)
		{
			m_lMaxbarCount += (iRecordCount);
		}
		else
		{
			long lFrameRecordCount = 1;
			long lFrameTime = pdbMinData[0].m_ulExchangeTimeStamp;
			for(int i=1; i<iRecordCount; i++)
			{
				if(((pdbMinData[i].m_ulExchangeTimeStamp - DayStartSecs(pdbMinData[i].m_ulExchangeTimeStamp) - m_iStartSeconds)/m_lTimeFrameInterval != (lFrameTime - DayStartSecs(lFrameTime) - m_iStartSeconds)/m_lTimeFrameInterval))
				{
					lFrameTime = pdbMinData[i].m_ulExchangeTimeStamp;
					lFrameRecordCount++;
				}
			}
			std::cout << "Frame record count " << lFrameRecordCount << std::endl;
			// still inefficient for long history
			//m_lMaxbarCount += ((pdbMinData[iRecordCount-1].m_ulExchangeTimeStamp - pdbMinData[0].m_ulExchangeTimeStamp)/m_lTimeFrameInterval);
			m_lMaxbarCount += (lFrameRecordCount);
		}
	}
	if(m_pdOpen == NULL)
	{
		AllocateMemory();
	}

	if(iMethod == DB_READ_OLD_DATA_WITH_EPOCH)
	{
		// since you don't need to find start of the day for this type of data
		bFoundEODMarker = true;
	}	

	for(int i=0; i<iRecordCount; i++)
	{
		if(bFoundEODMarker)
		{
			UpdateBar(pdbMinData[i].m_ulExchangeTimeStamp, pdbMinData[i].m_dPriceOpen, pdbMinData[i].m_dPriceHigh, pdbMinData[i].m_dPriceLow, pdbMinData[i].m_dPriceClose, pdbMinData[i].m_lVolume, pdbMinData[i].m_lOpenInterest);
		}
		else
		{
			// so that the first bar start time is correct and hence the bar subsequently formed are of correction time 
			if((long)((pdbMinData[i].m_ulExchangeTimeStamp+19800)/(24*60*60)) != (long)((pdbMinData[i+1].m_ulExchangeTimeStamp+19800)/(24*60*60)))
			{
				bFoundEODMarker = true;
			}
		}
	}

	std::cout << "Read " << m_iBarIndex << " records from database " << std::endl;	
	return 0;
}

int Bar::SaveDataToFile()
{
	//TODO append to the file
	if(!strcmp(m_szFileName, ""))
	{
		return -1;
	}

	std::fstream fpDataFile;

	// as we must have read the previous data from file hence overwrite the whole file
	fpDataFile.open(m_szFileName, std::ios::out);

	// write all data to file
	int iStartBarIndex = 0;
	int iEndBarIndex = m_iBarIndex;
	
	for(int kk=iStartBarIndex; kk<iEndBarIndex; kk++)
	{
		fpDataFile << std::fixed << std::showpoint << std::cout.precision(2);
		fpDataFile << m_szTickerName << " , "
			<< m_plTime[kk] << " , " 
			<< m_pdOpen[kk] << " , "
			<< m_pdHigh[kk] << " , "
			<< m_pdLow[kk] << " , "
			<< m_pdClose[kk] << " , "
			<< m_plVolume[kk] << " , "
			<< m_plOpenInterest[kk] 
			<< std::endl;
	}

	fpDataFile.close();

	std::cout << iEndBarIndex << " records written to file " << m_szFileName << std::endl;
	return 0;
}

void Bar::SetDBConnector(DBConnector* DBConnection)
{
	m_DBConnection = DBConnection;
}
