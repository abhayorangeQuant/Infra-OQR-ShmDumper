#include "iostream"
#include <sys/shm.h> 
#include <sys/stat.h> 
#include "string.h"
#include "assert.h"
#include "errno.h"

#include "ShmSPMCProducer.h"

ShmSPMCQueueProducer::ShmSPMCQueueProducer(key_t shmKey, long lElementCount, int iElementSize)
{
	m_ShmKey = shmKey;
	m_lMaxElements = lElementCount;
	m_iElementSize = iElementSize;

	// so that we can store required elements and a read writer counter
	long lShmSize = m_lMaxElements*m_iElementSize + sizeof(long);

	// Create Shared Memory
	if((m_iShmId = shmget(m_ShmKey, lShmSize, IPC_CREAT | IPC_EXCL | 0666 /*| S_IRUSR | S_IWUSR*/)) < 0) 
	{
		std::cerr << "Shared Memory creation failure for key " << m_ShmKey << " and size " << lShmSize << " error code " << m_iShmId << std::endl;
		std::cerr << strerror(errno) << std::endl;
		assert("Share memory creation failure" && 0);
	}

	// Attach to the shared memory
	if((m_pShmPtr = (char*)shmat(m_iShmId, NULL, 0)) == (char *) -1) 
	{
		std::cerr << "Shared Memory attach failure for key " << m_ShmKey << " and size " << lShmSize << std::endl;
		std::cerr << strerror(errno) << std::endl;
		assert("Share memory attach failure" && 0);
	}

	m_pElementCounter = (long*)m_pShmPtr;
	*m_pElementCounter = 0;

	m_pShmDataPtr = (char*)(m_pShmPtr + sizeof(long));

}

ShmSPMCQueueProducer::~ShmSPMCQueueProducer()
{
	// detach shared memory
	shmdt(m_pShmPtr);
	
	// delete shared memory
	shmctl(m_iShmId, IPC_RMID, NULL);
}

void ShmSPMCQueueProducer::push_back(char* data)
{
	// fair assumption that we have just one writer to the queue hence pointer is sane
	memcpy((void*)m_pShmDataPtr, (void*)data, m_iElementSize);
	
	// increment after memcpy so that its read after its written to
	// secondly we don't need atomic operation here since we have just one producer
	// aotmic operations are little slow so lets not use here
	++(*m_pElementCounter);
	
	if(*m_pElementCounter % m_lMaxElements == 0)
	{
		m_pShmDataPtr = (char*)(m_pShmPtr + sizeof(long));
	}
	else
	{
		m_pShmDataPtr += m_iElementSize;
	}
}
