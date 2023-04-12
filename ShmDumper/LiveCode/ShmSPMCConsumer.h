#ifndef _SHM_SPMC_CONSUMER
#define _SHM_SPMC_CONSUMER

#include <sys/shm.h> 
#include <sys/stat.h> 

class ShmSPMCQueueConsumer
{
private:
	key_t	m_ShmKey;
	int		m_iShmId;
	long 	m_lMaxElements;
	char*	m_pShmPtr;
	long*	m_pElementCounter;
	long 	m_lReadCounter;
	int	m_iElementSize;
	char* 	m_pShmDataReader;
	
public:

	char* m_ShmCurrentData;
	ShmSPMCQueueConsumer(key_t, long, int);
	~ShmSPMCQueueConsumer();
	char* pop_front();
};

#endif
