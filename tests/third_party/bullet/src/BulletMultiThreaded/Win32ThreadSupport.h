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

#include "LinearMath/btScalar.h"
#include "PlatformDefinitions.h"

#ifdef USE_WIN32_THREADING  //platform specific defines are defined in PlatformDefinitions.h

#ifndef BT_WIN32_THREAD_SUPPORT_H
#define BT_WIN32_THREAD_SUPPORT_H

#include "LinearMath/btAlignedObjectArray.h"

#include "btThreadSupportInterface.h"


typedef void (*Win32ThreadFunc)(void* userPtr,void* lsMemory);
typedef void* (*Win32lsMemorySetupFunc)();


///Win32ThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class Win32ThreadSupport : public btThreadSupportInterface 
{
public:
	///placeholder, until libspe2 support is there
	struct	btSpuStatus
	{
		uint32_t	m_taskId;
		uint32_t	m_commandId;
		uint32_t	m_status;

		Win32ThreadFunc	m_userThreadFunc;
		void*	m_userPtr; //for taskDesc etc
		void*	m_lsMemory; //initialized using Win32LocalStoreMemorySetupFunc

		void*	m_threadHandle; //this one is calling 'Win32ThreadFunc'

		void*	m_eventStartHandle;
		char	m_eventStartHandleName[32];

		void*	m_eventCompletetHandle;
		char	m_eventCompletetHandleName[32];
		

	};
private:

	btAlignedObjectArray<btSpuStatus>	m_activeSpuStatus;
	btAlignedObjectArray<void*>			m_completeHandles;
	
	int m_maxNumTasks;
public:
	///Setup and initialize SPU/CELL/Libspe2

	struct	Win32ThreadConstructionInfo
	{
		Win32ThreadConstructionInfo(char* uniqueName,
									Win32ThreadFunc userThreadFunc,
									Win32lsMemorySetupFunc	lsMemoryFunc,
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
		Win32ThreadFunc			m_userThreadFunc;
		Win32lsMemorySetupFunc	m_lsMemoryFunc;
		int						m_numThreads;
		int						m_threadStackSize;

	};



	Win32ThreadSupport(const Win32ThreadConstructionInfo& threadConstructionInfo);

///cleanup/shutdown Libspe2
	virtual	~Win32ThreadSupport();

	void	startThreads(const Win32ThreadConstructionInfo&	threadInfo);


///send messages to SPUs
	virtual	void sendRequest(uint32_t uiCommand, ppu_address_t uiArgument0, uint32_t uiArgument1);

///check for messages from SPUs
	virtual	void waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1);

	virtual bool isTaskCompleted(unsigned int *puiArgument0, unsigned int *puiArgument1, int timeOutInMilliseconds);

///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual	void startSPU();

///tell the task scheduler we are done with the SPU tasks
	virtual	void stopSPU();

	virtual	void	setNumTasks(int numTasks)
	{
		m_maxNumTasks = numTasks;
	}

	virtual int getNumTasks() const
	{
		return m_maxNumTasks;
	}

	virtual void*	getThreadLocalMemory(int taskId)
	{
		return m_activeSpuStatus[taskId].m_lsMemory;
	}
	virtual btBarrier*	createBarrier();

	virtual btCriticalSection* createCriticalSection();

};

#endif //BT_WIN32_THREAD_SUPPORT_H

#endif //USE_WIN32_THREADING
