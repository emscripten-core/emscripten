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

#include "Win32ThreadSupport.h"

#ifdef USE_WIN32_THREADING

#include <windows.h>

#include "SpuCollisionTaskProcess.h"

#include "SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"



///The number of threads should be equal to the number of available cores
///@todo: each worker should be linked to a single core, using SetThreadIdealProcessor.

///Win32ThreadSupport helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
///Setup and initialize SPU/CELL/Libspe2
Win32ThreadSupport::Win32ThreadSupport(const Win32ThreadConstructionInfo & threadConstructionInfo)
{
	m_maxNumTasks = threadConstructionInfo.m_numThreads;
	startThreads(threadConstructionInfo);
}

///cleanup/shutdown Libspe2
Win32ThreadSupport::~Win32ThreadSupport()
{
	stopSPU();
}




#include <stdio.h>

DWORD WINAPI Thread_no_1( LPVOID lpParam ) 
{

	Win32ThreadSupport::btSpuStatus* status = (Win32ThreadSupport::btSpuStatus*)lpParam;

	
	while (1)
	{
		WaitForSingleObject(status->m_eventStartHandle,INFINITE);
		
		void* userPtr = status->m_userPtr;

		if (userPtr)
		{
			btAssert(status->m_status);
			status->m_userThreadFunc(userPtr,status->m_lsMemory);
			status->m_status = 2;
			SetEvent(status->m_eventCompletetHandle);
		} else
		{
			//exit Thread
			status->m_status = 3;
			printf("Thread with taskId %i with handle %p exiting\n",status->m_taskId, status->m_threadHandle);
			SetEvent(status->m_eventCompletetHandle);
			break;
		}
		
	}

	printf("Thread TERMINATED\n");
	return 0;

}

///send messages to SPUs
void Win32ThreadSupport::sendRequest(uint32_t uiCommand, ppu_address_t uiArgument0, uint32_t taskId)
{
	///	gMidphaseSPU.sendRequest(CMD_GATHER_AND_PROCESS_PAIRLIST, (ppu_address_t) &taskDesc);
	
	///we should spawn an SPU task here, and in 'waitForResponse' it should wait for response of the (one of) the first tasks that finished
	


	switch (uiCommand)
	{
	case 	CMD_GATHER_AND_PROCESS_PAIRLIST:
		{


//#define SINGLE_THREADED 1
#ifdef SINGLE_THREADED

			btSpuStatus&	spuStatus = m_activeSpuStatus[0];
			spuStatus.m_userPtr=(void*)uiArgument0;
			spuStatus.m_userThreadFunc(spuStatus.m_userPtr,spuStatus.m_lsMemory);
			HANDLE handle =0;
#else


			btSpuStatus&	spuStatus = m_activeSpuStatus[taskId];
			btAssert(taskId>=0);
			btAssert(int(taskId)<m_activeSpuStatus.size());

			spuStatus.m_commandId = uiCommand;
			spuStatus.m_status = 1;
			spuStatus.m_userPtr = (void*)uiArgument0;

			///fire event to start new task
			SetEvent(spuStatus.m_eventStartHandle);

#endif //CollisionTask_LocalStoreMemory

			

			break;
		}
	default:
		{
			///not implemented
			btAssert(0);
		}

	};


}


///check for messages from SPUs
void Win32ThreadSupport::waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1)
{
	///We should wait for (one of) the first tasks to finish (or other SPU messages), and report its response
	
	///A possible response can be 'yes, SPU handled it', or 'no, please do a PPU fallback'


	btAssert(m_activeSpuStatus.size());

	int last = -1;
#ifndef SINGLE_THREADED
	DWORD res = WaitForMultipleObjects(m_completeHandles.size(), &m_completeHandles[0], FALSE, INFINITE);
	btAssert(res != WAIT_FAILED);
	last = res - WAIT_OBJECT_0;

	btSpuStatus& spuStatus = m_activeSpuStatus[last];
	btAssert(spuStatus.m_threadHandle);
	btAssert(spuStatus.m_eventCompletetHandle);

	//WaitForSingleObject(spuStatus.m_eventCompletetHandle, INFINITE);
	btAssert(spuStatus.m_status > 1);
	spuStatus.m_status = 0;

	///need to find an active spu
	btAssert(last>=0);

#else
	last=0;
	btSpuStatus& spuStatus = m_activeSpuStatus[last];
#endif //SINGLE_THREADED

	

	*puiArgument0 = spuStatus.m_taskId;
	*puiArgument1 = spuStatus.m_status;


}


///check for messages from SPUs
bool Win32ThreadSupport::isTaskCompleted(unsigned int *puiArgument0, unsigned int *puiArgument1, int timeOutInMilliseconds)
{
	///We should wait for (one of) the first tasks to finish (or other SPU messages), and report its response
	
	///A possible response can be 'yes, SPU handled it', or 'no, please do a PPU fallback'


	btAssert(m_activeSpuStatus.size());

	int last = -1;
#ifndef SINGLE_THREADED
	DWORD res = WaitForMultipleObjects(m_completeHandles.size(), &m_completeHandles[0], FALSE, timeOutInMilliseconds);
	
	if ((res != STATUS_TIMEOUT) && (res != WAIT_FAILED))
	{
		
		btAssert(res != WAIT_FAILED);
		last = res - WAIT_OBJECT_0;

		btSpuStatus& spuStatus = m_activeSpuStatus[last];
		btAssert(spuStatus.m_threadHandle);
		btAssert(spuStatus.m_eventCompletetHandle);

		//WaitForSingleObject(spuStatus.m_eventCompletetHandle, INFINITE);
		btAssert(spuStatus.m_status > 1);
		spuStatus.m_status = 0;

		///need to find an active spu
		btAssert(last>=0);

	#else
		last=0;
		btSpuStatus& spuStatus = m_activeSpuStatus[last];
	#endif //SINGLE_THREADED

		

		*puiArgument0 = spuStatus.m_taskId;
		*puiArgument1 = spuStatus.m_status;

		return true;
	} 

	return false;
}


void Win32ThreadSupport::startThreads(const Win32ThreadConstructionInfo& threadConstructionInfo)
{

	m_activeSpuStatus.resize(threadConstructionInfo.m_numThreads);
	m_completeHandles.resize(threadConstructionInfo.m_numThreads);

	m_maxNumTasks = threadConstructionInfo.m_numThreads;

	for (int i=0;i<threadConstructionInfo.m_numThreads;i++)
	{
		printf("starting thread %d\n",i);

		btSpuStatus&	spuStatus = m_activeSpuStatus[i];

		LPSECURITY_ATTRIBUTES lpThreadAttributes=NULL;
		SIZE_T dwStackSize=threadConstructionInfo.m_threadStackSize;
		LPTHREAD_START_ROUTINE lpStartAddress=&Thread_no_1;
		LPVOID lpParameter=&spuStatus;
		DWORD dwCreationFlags=0;
		LPDWORD lpThreadId=0;

		spuStatus.m_userPtr=0;

		sprintf(spuStatus.m_eventStartHandleName,"eventStart%s%d",threadConstructionInfo.m_uniqueName,i);
		spuStatus.m_eventStartHandle = CreateEventA (0,false,false,spuStatus.m_eventStartHandleName);

		sprintf(spuStatus.m_eventCompletetHandleName,"eventComplete%s%d",threadConstructionInfo.m_uniqueName,i);
		spuStatus.m_eventCompletetHandle = CreateEventA (0,false,false,spuStatus.m_eventCompletetHandleName);

		m_completeHandles[i] = spuStatus.m_eventCompletetHandle;

		HANDLE handle = CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,	dwCreationFlags,lpThreadId);
		SetThreadPriority(handle,THREAD_PRIORITY_HIGHEST);
		//SetThreadPriority(handle,THREAD_PRIORITY_TIME_CRITICAL);

		SetThreadAffinityMask(handle, 1<<i);

		spuStatus.m_taskId = i;
		spuStatus.m_commandId = 0;
		spuStatus.m_status = 0;
		spuStatus.m_threadHandle = handle;
		spuStatus.m_lsMemory = threadConstructionInfo.m_lsMemoryFunc();
		spuStatus.m_userThreadFunc = threadConstructionInfo.m_userThreadFunc;

		printf("started thread %d with threadHandle %p\n",i,handle);
		
	}

}

void Win32ThreadSupport::startSPU()
{
}


///tell the task scheduler we are done with the SPU tasks
void Win32ThreadSupport::stopSPU()
{
	int i;
	for (i=0;i<m_activeSpuStatus.size();i++)
	{
		btSpuStatus& spuStatus = m_activeSpuStatus[i];
		if (spuStatus.m_status>0)
		{
			WaitForSingleObject(spuStatus.m_eventCompletetHandle, INFINITE);
		}
		

		spuStatus.m_userPtr = 0;
		SetEvent(spuStatus.m_eventStartHandle);
		WaitForSingleObject(spuStatus.m_eventCompletetHandle, INFINITE);

		CloseHandle(spuStatus.m_eventCompletetHandle);
		CloseHandle(spuStatus.m_eventStartHandle);
		CloseHandle(spuStatus.m_threadHandle);
	}

	m_activeSpuStatus.clear();
	m_completeHandles.clear();

}



class btWin32Barrier : public btBarrier
{
private:
	CRITICAL_SECTION mExternalCriticalSection;
	CRITICAL_SECTION mLocalCriticalSection;
	HANDLE mRunEvent,mNotifyEvent;
	int mCounter,mEnableCounter;
	int mMaxCount;

public:
	btWin32Barrier()
	{
		mCounter = 0;
		mMaxCount = 1;
		mEnableCounter = 0;
		InitializeCriticalSection(&mExternalCriticalSection);
		InitializeCriticalSection(&mLocalCriticalSection);
		mRunEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
		mNotifyEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	}

	virtual ~btWin32Barrier()
	{
		DeleteCriticalSection(&mExternalCriticalSection);
		DeleteCriticalSection(&mLocalCriticalSection);
		CloseHandle(mRunEvent);
		CloseHandle(mNotifyEvent);
	}

	void sync()
	{
		int eventId;

		EnterCriticalSection(&mExternalCriticalSection);

		//PFX_PRINTF("enter taskId %d count %d stage %d phase %d mEnableCounter %d\n",taskId,mCounter,debug&0xff,debug>>16,mEnableCounter);

		if(mEnableCounter > 0) {
			ResetEvent(mNotifyEvent);
			LeaveCriticalSection(&mExternalCriticalSection);
			WaitForSingleObject(mNotifyEvent,INFINITE); 
			EnterCriticalSection(&mExternalCriticalSection);
		}

		eventId = mCounter;
		mCounter++;

		if(eventId == mMaxCount-1) {
			SetEvent(mRunEvent);

			mEnableCounter = mCounter-1;
			mCounter = 0;
		}
		else {
			ResetEvent(mRunEvent);
			LeaveCriticalSection(&mExternalCriticalSection);
			WaitForSingleObject(mRunEvent,INFINITE); 
			EnterCriticalSection(&mExternalCriticalSection);
			mEnableCounter--;
		}

		if(mEnableCounter == 0) {
			SetEvent(mNotifyEvent);
		}

		//PFX_PRINTF("leave taskId %d count %d stage %d phase %d mEnableCounter %d\n",taskId,mCounter,debug&0xff,debug>>16,mEnableCounter);

		LeaveCriticalSection(&mExternalCriticalSection);
	}

	virtual void setMaxCount(int n) {mMaxCount = n;}
	virtual int  getMaxCount() {return mMaxCount;}
};

class btWin32CriticalSection : public btCriticalSection
{
private:
	CRITICAL_SECTION mCriticalSection;

public:
	btWin32CriticalSection()
	{
		InitializeCriticalSection(&mCriticalSection);
	}

	~btWin32CriticalSection()
	{
		DeleteCriticalSection(&mCriticalSection);
	}

	unsigned int getSharedParam(int i)
	{
		btAssert(i>=0&&i<31);
		return mCommonBuff[i+1];
	}

	void setSharedParam(int i,unsigned int p)
	{
		btAssert(i>=0&&i<31);
		mCommonBuff[i+1] = p;
	}

	void lock()
	{
		EnterCriticalSection(&mCriticalSection);
		mCommonBuff[0] = 1;
	}

	void unlock()
	{
		mCommonBuff[0] = 0;
		LeaveCriticalSection(&mCriticalSection);
	}
};


btBarrier*	Win32ThreadSupport::createBarrier()
{
	unsigned char* mem = (unsigned char*)btAlignedAlloc(sizeof(btWin32Barrier),16);
	btWin32Barrier* barrier = new(mem) btWin32Barrier();
	barrier->setMaxCount(getNumTasks());
	return barrier;
}

btCriticalSection* Win32ThreadSupport::createCriticalSection()
{
	unsigned char* mem = (unsigned char*) btAlignedAlloc(sizeof(btWin32CriticalSection),16);
	btWin32CriticalSection* cs = new(mem) btWin32CriticalSection();
	return cs;
}



#endif //USE_WIN32_THREADING


