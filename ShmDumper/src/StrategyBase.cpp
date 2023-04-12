#include "iostream"
#include "string.h"
#include <unistd.h>

#include "StrategyBase.h"
#include "ErrorCodes.h"

uint64_t getMyTime();

StrategyBase::StrategyBase(SettingsReader* reader)
{
	Settings = reader;

    char temp[200];	
	m_iMDCount = reader->GetInt("shm", "MDCount", 0);
	int iShmKey;
	long lShmSize;
	for(int ii=0; ii<m_iMDCount; ii++)
	{
		sprintf(temp, "MDShmKey_%d", ii+1);
		iShmKey = reader->GetInt("shm", std::string(temp), 0);
		sprintf(temp, "MDShmSize_%d", ii+1);
		lShmSize = reader->GetLong("shm", std::string(temp), 0);
		MDConnection[ii] = new ShmSPMCQueueConsumer((key_t)iShmKey, lShmSize, sizeof(MarketUpdates));
	}

	//int iShmKey = reader->GetInt("md", "shmKey", 0);
	//long lElementCount = reader->GetLong("md", "shmSize", 10000);
	//MDConnection = new ShmSPMCQueueConsumer((key_t)iShmKey, lElementCount, sizeof(MarketUpdates));
	DBConnection = new DBConnector(reader);

	m_bMDConnected = false;
}

StrategyBase::~StrategyBase()
{
	m_bMDConnected = false;

	//delete MDConnection;
	delete DBConnection;
}

bool StrategyBase::ReadShmData()
{
	
	MarketUpdates* update = NULL;

    for(int ii = 0; ii < m_iMDCount; ++ii)
    {
	    update = (MarketUpdates*)MDConnection[ii]->pop_front();
	    if(update != NULL)
	    {
		    MarketDataUpdate(update);
		    return true;	
	    }
    }
	return false;
}

