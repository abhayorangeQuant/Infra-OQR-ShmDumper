#include "iostream"
#include "pthread.h"
#include "fstream"
#include <unistd.h>

#include "Strategy.h"
#include "ErrorCodes.h"

int iCounter;

using namespace std;

TickerMap g_Tickers;

extern char szMailids[1000];
extern int EmailAlert(char*, char*);

bool fexists(const char *filename)
{
	ifstream ifile(filename);
	return ifile.good();
}

Strategy::Strategy(SettingsReader* reader, bool CheckSanity): StrategyBase(reader)
{
	m_reader = reader;
	char temp[200], filename[500];

    m_bCheckSanity = CheckSanity;
    
	m_szFilePath = reader->GetString("state", "FilePath", "");
	m_iStartHour = reader->GetInt("record", "StartHour", 0); 
	m_iEndHour = reader->GetInt("record", "EndHour", 0); 
	m_bSaveToFile = reader->GetBool("state", "SaveToFile", true);

	std::cout << "SaveToFile|" << m_bSaveToFile << std::endl;
	
	if(m_bSaveToFile)
	{
		time_t rawtime;
		struct tm * timeinfo;

		time (&rawtime);
		timeinfo = localtime (&rawtime);

		strftime (temp,200,"marketdata_%Y_%m_%d",timeinfo);
		strcpy(filename, m_szFilePath.c_str());
		strcat(filename, temp);
		strcpy(temp, filename);
		int append = 1;
		while(fexists(filename))
		{
			sprintf(filename, "%s_%d", temp, append);
			append++;
		}
		std::cout << "Storing data in file " << filename << std::endl;
		marketDataFile = fopen(filename, "a+b");
	}

/*	ExchangeAlertCounters exchAlert;
	exchAlert.clear();

	m_ExchangeCounter[(exchange_type)MCX] = exchAlert;
*/
	std::string szTemp = reader->GetString("state", "EmailId", "");
        strcpy(szMailids, szTemp.c_str());	
}

Strategy::~Strategy()
{

}

int Strategy::Run()
{
	bool bExit = false;
	long lLoopCounter = 0;
	long lDataCounter = 0;
	volatile long lSleepCounter = 0;
	while(true)
	{	
		lLoopCounter++;	
		if(ReadShmData())
		{
			lDataCounter = 0;
			lSleepCounter = 0;
		}
		else
		{
			lDataCounter++;
			lSleepCounter++;
		}
		if(lSleepCounter >= 10)
		{
			usleep(1);
			lSleepCounter = 0;
		}
		if(lDataCounter >= 5000)
		{
			// this means data has stopped coming check if time to exit
			time_t rawtime;
			struct tm * timeinfo;
	
			time (&rawtime);
			timeinfo = localtime (&rawtime);

			if(!(timeinfo->tm_hour >= m_iStartHour && timeinfo->tm_hour <= m_iEndHour))
			{
				break;
			}
			lDataCounter = 0;
		}
		if(lLoopCounter >= 10000000)
		{
			time_t rawtime;
			time (&rawtime);

			// check for each exchange last update time
			for(ExchangeCounterMapIter exch = m_ExchangeCounter.begin(); exch!= m_ExchangeCounter.end(); exch++)
			{
				if(exch->second.m_ulExchangeTimeStamp >= exch->second.m_ulStartMin && exch->second.m_ulExchangeTimeStamp <= exch->second.m_ulEndMin)
				{
					if((rawtime - exch->second.m_ulExchangeTimeStamp) >= (exch->second.m_lCounter*120+120))
					{
						exch->second.m_lCounter++;
						std::cout << "No data for count " << exch->second.m_lCounter << " for exchange " << exch->first << std::endl;		
						if(exch->second.m_iMailAlertCount <= 5)
						{
							char szMailBody[200];
							sprintf(szMailBody, "No data for last 2 mins for exchange %d", exch->first);
							EmailAlert("MDRecorder Alert", szMailBody);
							if(exch->second.m_iMailAlertCount == 5)
							{
								EmailAlert("MDRecorder Alert", "No data for last consecutive 10 mins, MDRecorder won't send any more alerts");
							}
						}
						exch->second.m_iMailAlertCount++;
					}
				}
			}	
			lLoopCounter = 0;
		}
	}
	
	if(m_bSaveToFile)
	{	
		fclose(marketDataFile);
	}
	return 0;
}

int Strategy::ProcessResponse(StrategyResponse* response)
{
	std::cout << "Response|" << std::endl;
	response->print();
	return 0;
}

bool Strategy::CheckSanity(MarketUpdates* update)
{
    if(!m_bCheckSanity)
        return true;
    
    float limit = 0.3;
    
    bool result  = true;
    
    if(update->m_Bid[0].m_dPrice > update->m_Ask[0].m_dPrice)
    {
        std::cout << "Book is in crossed state" << std::endl;
        update->print();
        result = false;
    }
    
    if(update->m_lTotalBuyQuantity < 0 || update->m_lTotalSellQuantity < 0)
    {
        std::cout << "Total Buy/Sell Qty invalid.. " << std::endl;
        update->print();
        result = false;
    }
    
    //if(update->m_dDayAverage <= 0 || update->m_dDayClose <= 0 || update->m_dDayHigh <=0 || update->m_dDayLow <=0 || update->m_dDayOpen <=0 || update->m_dDayPrevClose <=0)
    if(update->m_dDayClose <= 0 || update->m_dDayHigh <=0 || update->m_dDayLow <=0 || update->m_dDayOpen <=0 || update->m_dDayPrevClose <=0)
    {
        std::cout << " OHLC invalid. ." << std::endl;
        update->print();
        result = false;
    }
    
    if(update->m_dLastTradePrice <=0 || update->m_dLastTradePrice <= update->m_Ask[0].m_dPrice * (1.0f-limit) || update->m_dLastTradePrice >= update->m_Ask[0].m_dPrice * (1.0f + limit)
        || update->m_dLastTradePrice <= update->m_Ask[0].m_dPrice * (1.0f-limit) || update->m_dLastTradePrice >= update->m_Ask[0].m_dPrice * (1.0f + limit) )
    {
        std::cout << " LTP might be wrong. ." << std::endl;
        update->print();
        result = false;
    } 
    
    return result;
}

int Strategy::MarketDataUpdate(MarketUpdates* update)
{
	//update->print();
	if(CheckSanity(update))
	{
	// we can do this here since recorder will get all updates
	    if(update->m_ulSequenceNumber != m_lastUpdate.m_ulSequenceNumber+1)
	    {
		    std::cout << "Update missed " << update->m_ulSequenceNumber << "|" << m_lastUpdate.m_ulSequenceNumber << std::endl;
	    }
	    iCounter++;
	    if(m_bSaveToFile)
	    {
		    fwrite((void*)update, sizeof(MarketUpdates), 1, marketDataFile);
		    if(iCounter > 1000)
		    {
			    iCounter = 0;
			    fflush(marketDataFile);
		    }
	    }
	
	    ExchangeCounterMapIter exch = m_ExchangeCounter.find(update->m_eExchangeType);
	    if(exch == m_ExchangeCounter.end())
	    {
		    ExchangeAlertCounters counter;
		    counter.clear();
		    time_t rawtime;
		
		    time (&rawtime);
			
		    switch(update->m_eExchangeType)
		    {
                /*
                   case NSE_CASH:
                   counter.m_ulStartMin = ((long)(rawtime/86400))*86400 + (9*60+15 - (5*60+30))*60;
                   counter.m_ulEndMin = ((long)(rawtime/86400))*86400 + (15*60+30 - (5*60+30))*60;  
                   break;
                   case NSE_FO:
                   counter.m_ulStartMin = ((long)(rawtime/86400))*86400 + (9*60+15 - (5*60+30))*60;
                   counter.m_ulEndMin = ((long)(rawtime/86400))*86400 + (15*60+30 - (5*60+30))*60;  
                   break;
                   case NSE_CDS:
                   counter.m_ulStartMin = ((long)(rawtime/86400))*86400 + (9*60+0 - (5*60+30))*60;
                   counter.m_ulEndMin = ((long)(rawtime/86400))*86400 + (17*60+0 - (5*60+30))*60;  
                   break;
                 */
                case MCX:
                    counter.m_ulStartMin = ((long)(rawtime/86400))*86400 + (9*60+0 - (5*60+30))*60;
                    counter.m_ulEndMin = ((long)(rawtime/86400))*86400 + (23*60+55 - (5*60+30))*60;  
                    break;
                default :
                    counter.m_ulStartMin = ((long)(rawtime/86400))*86400 + (9*60+0 - (5*60+30))*60;
                    counter.m_ulEndMin = ((long)(rawtime/86400))*86400 + (23*60+55 - (5*60+30))*60;  
                    break;
            }
		    m_ExchangeCounter[update->m_eExchangeType] = counter;
		    exch = m_ExchangeCounter.find(update->m_eExchangeType);
	    }
	    exch->second.m_ulExchangeTimeStamp = max(update->m_ulExchangeTimeStamp, exch->second.m_ulExchangeTimeStamp);
	    TickerMapIter iter;
	    if(update->m_iToken <= 99999999)
	    {   
		    iter = g_Tickers.find((long)update->m_eExchangeType*100000000 + update->m_iToken);
		    if(iter == g_Tickers.end())
		    {
			    g_Tickers[(long)update->m_eExchangeType*100000000 + update->m_iToken] = new Ticker(this, update->m_szScripCode, update->m_eExchangeType);
			    iter = g_Tickers.find((long)update->m_eExchangeType*100000000 + update->m_iToken);
		    }
	    }
	    // insert data only if timefilter satisfied
	    if(exch->second.m_ulExchangeTimeStamp >= exch->second.m_ulStartMin && exch->second.m_ulExchangeTimeStamp <= exch->second.m_ulEndMin)
	    {
		    exch->second.m_lCounter = 0;
		    exch->second.m_iMailAlertCount = 0;
		    if(update->m_iToken <= 99999999)
		    {
			    iter->second->MarketDataUpdate(update);
		    }
	    }
	    m_lastUpdate = *update;
	 }
	 return 0;
}
