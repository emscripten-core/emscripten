/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://bulletphysics.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_POSIX_THREAD_SUPPORT_H
#define BT_POSIX_THREAD_SUPPORT_H


#include "LinearMath/btScalar.h"
#include "PlatformDefinitions.h"

#ifdef USE_PTHREADS //platform specifc defines are defined in PlatformDefinitions.h

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600 //for definition of pthread_barrier_t, see http://pages.cs.wisc.edu/~travitch/pthreads_primer.html
#endif //_XOPEN_SOURCE
#include <pthread.h>
#include <semaphore.h>



#include "LinearMath/btAlignedObjectArray.h"

#include "btThreadSupportInterface.h"


typedef void (*PosixThreadFunc)(void* userPtr,void* lsMemory);
typedef void* (*PosixlsMemorySetupFunc)();

// PosixThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class PosixThreadSupport : public btThreadSupportInterface 
{
public:
    typedef enum sStatus {
        STATUS_BUSY,
        STATUS_READY,
        STATUS_FINISHED
    } Status;

	// placeholder, until libspe2 support is there
	struct	btSpuStatus
	{
		uint32_t	m_taskId;
		uint32_t	m_commandId;
		uint32_t	m_status;

		PosixThreadFunc	m_userThreadFunc;
		void*	m_userPtr; //for taskDesc etc
		void*	m_lsMemory; //initialized using PosixLocalStoreMemorySetupFunc

                pthread_t thread;
                sem_t* startSemaphore;

        unsigned long threadUsed;
	};
private:

	btAlignedObjectArray<btSpuStatus>	m_activeSpuStatus;
public:
	///Setup and initialize SPU/CELL/Libspe2

	

	struct	ThreadConstructionInfo
	{
		ThreadConstructionInfo(char* uniqueName,
									PosixThreadFunc userThreadFunc,
									PosixlsMemorySetupFunc	lsMemoryFunc,
									int numThreads=1,
									int threadStackSize=65535
									)
									:m_uniqueName(uniqueName),
									m_userThreadFunc(userThreadFunc),
									m_lsMemoryFunc(lsMemoryFunc),
									m_numThreads(numThreads),
									m_threadStackSize(threadStackSize)
		{

		}

		char*					m_uniqueName;
		PosixThreadFunc			m_userThreadFunc;
		PosixlsMemorySetupFunc	m_lsMemoryFunc;
		int						m_numThreads;
		int						m_threadStackSize;

	};

	PosixThreadSupport(ThreadConstructionInfo& threadConstructionInfo);

///cleanup/shutdown Libspe2
	virtual	~PosixThreadSupport();

	void	startThreads(ThreadConstructionInfo&	threadInfo);


///send messages to SPUs
	virtual	void sendRequest(uint32_t uiCommand, ppu_address_t uiArgument0, uint32_t uiArgument1);

///check for messages from SPUs
	virtual	void waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1);

///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual	void startSPU();

///tell the task scheduler we are done with the SPU tasks
	virtual	void stopSPU();

	virtual void setNumTasks(int numTasks) {}

	virtual int getNumTasks() const
	{
		return m_activeSpuStatus.size();
	}

	virtual btBarrier* createBarrier();

	virtual btCriticalSection* createCriticalSection();
	
	virtual void*	getThreadLocalMemory(int taskId)
	{
		return m_activeSpuStatus[taskId].m_lsMemory;
	}

};

#endif // USE_PTHREADS

#endif // BT_POSIX_THREAD_SUPPORT_H


