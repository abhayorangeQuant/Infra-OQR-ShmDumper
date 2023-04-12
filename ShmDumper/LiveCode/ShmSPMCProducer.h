#ifndef _SHM_SPMC_PRODUCER
#define _SHM_SPMC_PRODUCER

#include <sys/shm.h> 
#include <sys/stat.h> 


class ShmSPMCQueueProducer
{
private:
        key_t   m_ShmKey;
        int     m_iShmId;
	int	m_iElementSize;
        long    m_lMaxElements;
        char*   m_pShmPtr;
        long*   m_pElementCounter;
        char*        m_pShmDataPtr;
public:

        ShmSPMCQueueProducer(key_t, long, int);
        ~ShmSPMCQueueProducer();
        void push_back(char*);
};

#endif
