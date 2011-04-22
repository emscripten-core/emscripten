/*
Bullet Continuous Collision Detection and Physics Library, Copyright (c) 2007 Erwin Coumans

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/


#include "MiniCLTask.h"
#include "BulletMultiThreaded/PlatformDefinitions.h"
#include "BulletMultiThreaded/SpuFakeDma.h"
#include "LinearMath/btMinMax.h"
#include "MiniCLTask.h"
#include "MiniCL/MiniCLTaskScheduler.h"


#ifdef __SPU__
#include <spu_printf.h>
#else
#include <stdio.h>
#define spu_printf printf
#endif

int gMiniCLNumOutstandingTasks = 0;

struct MiniCLTask_LocalStoreMemory
{
	
};


//-- MAIN METHOD
void processMiniCLTask(void* userPtr, void* lsMemory)
{
	//	BT_PROFILE("processSampleTask");

	MiniCLTask_LocalStoreMemory* localMemory = (MiniCLTask_LocalStoreMemory*)lsMemory;

	MiniCLTaskDesc* taskDescPtr = (MiniCLTaskDesc*)userPtr;
	MiniCLTaskDesc& taskDesc = *taskDescPtr;

	for (unsigned int i=taskDesc.m_firstWorkUnit;i<taskDesc.m_lastWorkUnit;i++)
	{
		taskDesc.m_kernel->m_launcher(&taskDesc, i);
	}

//	printf("Compute Unit[%d] executed kernel %d work items [%d..%d)\n",taskDesc.m_taskId,taskDesc.m_kernelProgramId,taskDesc.m_firstWorkUnit,taskDesc.m_lastWorkUnit);
	
}


#if defined(__CELLOS_LV2__) || defined (LIBSPE2)

ATTRIBUTE_ALIGNED16(MiniCLTask_LocalStoreMemory	gLocalStoreMemory);

void* createMiniCLLocalStoreMemory()
{
	return &gLocalStoreMemory;
}
#else
void* createMiniCLLocalStoreMemory()
{
	return new MiniCLTask_LocalStoreMemory;
};

#endif
