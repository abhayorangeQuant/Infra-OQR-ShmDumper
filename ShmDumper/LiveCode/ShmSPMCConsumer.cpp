#include "iostream"
#include <sys/shm.h> 
#include <sys/stat.h> 
#include "string.h"
#include "assert.h"
#include "errno.h"

#include "ShmSPMCConsumer.h"

ShmSPMCQueueConsumer::ShmSPMCQueueConsumer(key_t shmKey, long lElementCount, int iElementSize)
{
	m_ShmKey = shmKey;
	m_lMaxElements = lElementCount;
	m_iElementSize = iElementSize;

	long lShmSize = m_lMaxElements*m_iElementSize + sizeof(long);

	if((m_iShmId = shmget(m_ShmKey, lShmSize, 0444)) < 0)
	{
		std::cerr << "Shared memory get failure for key " << m_ShmKey << " and size " << lShmSize << std::endl;
		std::cerr << strerror(errno) << std::endl;
		assert("Shared memory get failure" && 0);
	}	
	// Attach to the shared memory
	if((m_pShmPtr = (char*)shmat(m_iShmId, NULL, 0)) == (char *) -1) 
	{
		std::cerr << "Shared Memory attach failure for key " << m_ShmKey << " and size " << lShmSize << std::endl;
		std::cerr << strerror(errno) << std::endl;
		assert("Shared memory attach failure" && 0);
	}
	m_ShmCurrentData = new char[m_iElementSize];

	m_pElementCounter = (long*)m_pShmPtr;
	m_lReadCounter = *m_pElementCounter;
	m_pShmDataReader = (char*)(m_pShmPtr + sizeof(long) + (m_lReadCounter%m_lMaxElements)*m_iElementSize);
}

ShmSPMCQueueConsumer::~ShmSPMCQueueConsumer()
{
	delete m_ShmCurrentData;
	// detach shared memory
	shmdt(m_pShmPtr);
}

char* ShmSPMCQueueConsumer::pop_front()
{
	// polling for the data
	//while(m_lReadCounter == *m_pElementCounter);
	if(m_lReadCounter == *m_pElementCounter)
	{
		return NULL;
	}
	
	// not passing the shared memory pointer but a local reference copy
	memcpy((void*)m_ShmCurrentData, (void*)m_pShmDataReader, m_iElementSize);
	// increment the local read counter
	m_lReadCounter++;
	
	if(m_lReadCounter % m_lMaxElements == 0)
	{
		m_pShmDataReader = (char*)(m_pShmPtr + sizeof(long));
	}
	else
	{
		m_pShmDataReader += m_iElementSize;
	}
	
	// returning a class element so that it exists even after the function call
	return m_ShmCurrentData;
}
