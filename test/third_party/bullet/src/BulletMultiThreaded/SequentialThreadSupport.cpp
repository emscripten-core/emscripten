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

#include "SequentialThreadSupport.h"


#include "SpuCollisionTaskProcess.h"
#include "SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"

SequentialThreadSupport::SequentialThreadSupport(SequentialThreadConstructionInfo& threadConstructionInfo)
{
	startThreads(threadConstructionInfo);
}

///cleanup/shutdown Libspe2
SequentialThreadSupport::~SequentialThreadSupport()
{
	stopSPU();
}

#include <stdio.h>

///send messages to SPUs
void SequentialThreadSupport::sendRequest(uint32_t uiCommand, ppu_address_t uiArgument0, uint32_t taskId)
{
	switch (uiCommand)
	{
	case 	CMD_GATHER_AND_PROCESS_PAIRLIST:
		{
			btSpuStatus&	spuStatus = m_activeSpuStatus[0];
			spuStatus.m_userPtr=(void*)uiArgument0;
			spuStatus.m_userThreadFunc(spuStatus.m_userPtr,spuStatus.m_lsMemory);
		}
	break;
	default:
		{
			///not implemented
			btAssert(0 && "Not implemented");
		}

	};


}

///check for messages from SPUs
void SequentialThreadSupport::waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1)
{
	btAssert(m_activeSpuStatus.size());
	btSpuStatus& spuStatus = m_activeSpuStatus[0];
	*puiArgument0 = spuStatus.m_taskId;
	*puiArgument1 = spuStatus.m_status;
}

void SequentialThreadSupport::startThreads(SequentialThreadConstructionInfo& threadConstructionInfo)
{
	m_activeSpuStatus.resize(1);
	printf("STS: Not starting any threads\n");
	btSpuStatus& spuStatus = m_activeSpuStatus[0];
	spuStatus.m_userPtr = 0;
	spuStatus.m_taskId = 0;
	spuStatus.m_commandId = 0;
	spuStatus.m_status = 0;
	spuStatus.m_lsMemory = threadConstructionInfo.m_lsMemoryFunc();
	spuStatus.m_userThreadFunc = threadConstructionInfo.m_userThreadFunc;
	printf("STS: Created local store at %p for task %s\n", spuStatus.m_lsMemory, threadConstructionInfo.m_uniqueName);
}

void SequentialThreadSupport::startSPU()
{
}

void SequentialThreadSupport::stopSPU()
{
	m_activeSpuStatus.clear();
}

void SequentialThreadSupport::setNumTasks(int numTasks)
{
	printf("SequentialThreadSupport::setNumTasks(%d) is not implemented and has no effect\n",numTasks);
}




class btDummyBarrier : public btBarrier
{
private:
		
public:
	btDummyBarrier()
	{
	}
	
	virtual ~btDummyBarrier()
	{
	}
	
	void sync()
	{
	}
	
	virtual void setMaxCount(int n) {}
	virtual int  getMaxCount() {return 1;}
};

class btDummyCriticalSection : public btCriticalSection
{
	
public:
	btDummyCriticalSection()
	{
	}
	
	virtual ~btDummyCriticalSection()
	{
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
		mCommonBuff[0] = 1;
	}
	
	void unlock()
	{
		mCommonBuff[0] = 0;
	}
};




btBarrier*	SequentialThreadSupport::createBarrier()
{
	return new btDummyBarrier();
}

btCriticalSection* SequentialThreadSupport::createCriticalSection()
{
	return new btDummyCriticalSection();
	
}



