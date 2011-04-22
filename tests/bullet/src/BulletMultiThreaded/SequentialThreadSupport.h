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


#ifndef BT_SEQUENTIAL_THREAD_SUPPORT_H
#define BT_SEQUENTIAL_THREAD_SUPPORT_H

#include "LinearMath/btAlignedObjectArray.h"

#include "btThreadSupportInterface.h"

typedef void (*SequentialThreadFunc)(void* userPtr,void* lsMemory);
typedef void* (*SequentiallsMemorySetupFunc)();



///The SequentialThreadSupport is a portable non-parallel implementation of the btThreadSupportInterface
///This is useful for debugging and porting SPU Tasks to other platforms.
class SequentialThreadSupport : public btThreadSupportInterface 
{
public:
	struct	btSpuStatus
	{
		uint32_t	m_taskId;
		uint32_t	m_commandId;
		uint32_t	m_status;

		SequentialThreadFunc	m_userThreadFunc;

		void*	m_userPtr; //for taskDesc etc
		void*	m_lsMemory; //initialized using SequentiallsMemorySetupFunc
	};
private:
	btAlignedObjectArray<btSpuStatus>	m_activeSpuStatus;
	btAlignedObjectArray<void*>			m_completeHandles;	
public:
	struct	SequentialThreadConstructionInfo
	{
		SequentialThreadConstructionInfo (char* uniqueName,
									SequentialThreadFunc userThreadFunc,
									SequentiallsMemorySetupFunc	lsMemoryFunc
									)
									:m_uniqueName(uniqueName),
									m_userThreadFunc(userThreadFunc),
									m_lsMemoryFunc(lsMemoryFunc)
		{

		}

		char*						m_uniqueName;
		SequentialThreadFunc		m_userThreadFunc;
		SequentiallsMemorySetupFunc	m_lsMemoryFunc;
	};

	SequentialThreadSupport(SequentialThreadConstructionInfo& threadConstructionInfo);
	virtual	~SequentialThreadSupport();
	void	startThreads(SequentialThreadConstructionInfo&	threadInfo);
///send messages to SPUs
	virtual	void sendRequest(uint32_t uiCommand, ppu_address_t uiArgument0, uint32_t uiArgument1);
///check for messages from SPUs
	virtual	void waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1);
///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual	void startSPU();
///tell the task scheduler we are done with the SPU tasks
	virtual	void stopSPU();

	virtual void setNumTasks(int numTasks);

	virtual int getNumTasks() const
	{
		return 1;
	}
	virtual btBarrier*	createBarrier();

	virtual btCriticalSection* createCriticalSection();
	

};

#endif //BT_SEQUENTIAL_THREAD_SUPPORT_H

