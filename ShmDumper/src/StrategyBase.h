#ifndef _STRATEGY_BASE_HEADER
#define _STRATEGY_BASE_HEADER


#include "SettingFileReader.h"
#include "StrategyMsgs.h"
#include "DBConnector.h"

#include "ShmSPMCConsumer.h"

class StrategyBase
{
protected:
	SettingsReader* Settings;
	ShmSPMCQueueConsumer* MDConnection[13];	
	DBConnector* DBConnection;
    int m_iMDCount;
    
	bool m_bMDConnected;
	long m_ulExchangeTimeStamp;

	static void* StartShmReader(void* lpParam)
	{
		StrategyBase* caller = (StrategyBase*)lpParam;
		caller->ReadShmData();	
	}
	
public:
	StrategyBase(SettingsReader*);
	~StrategyBase();

	bool ReadShmData();

	DBConnector* GetDBConnector()
	{
		return DBConnection; 
	}
	virtual int ProcessResponse(StrategyResponse*) = 0;
	virtual int MarketDataUpdate(MarketUpdates*) = 0;
};
#endif
