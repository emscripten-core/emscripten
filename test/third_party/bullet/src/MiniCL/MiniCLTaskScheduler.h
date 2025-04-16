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



#ifndef MINICL_TASK_SCHEDULER_H
#define MINICL_TASK_SCHEDULER_H

#include <assert.h>


#include "BulletMultiThreaded/PlatformDefinitions.h"

#include <stdlib.h>

#include "LinearMath/btAlignedObjectArray.h"


#include "MiniCLTask/MiniCLTask.h"

//just add your commands here, try to keep them globally unique for debugging purposes
#define CMD_SAMPLE_TASK_COMMAND 10

struct MiniCLKernel;

/// MiniCLTaskScheduler handles SPU processing of collision pairs.
/// When PPU issues a task, it will look for completed task buffers
/// PPU will do postprocessing, dependent on workunit output (not likely)
class MiniCLTaskScheduler
{
	// track task buffers that are being used, and total busy tasks
	btAlignedObjectArray<bool>	m_taskBusy;
	btAlignedObjectArray<MiniCLTaskDesc>	m_spuSampleTaskDesc;


	btAlignedObjectArray<const MiniCLKernel*>	m_kernels;


	int   m_numBusyTasks;

	// the current task and the current entry to insert a new work unit
	int   m_currentTask;

	bool m_initialized;

	void postProcess(int taskId, int outputSize);
	
	class	btThreadSupportInterface*	m_threadInterface;

	int	m_maxNumOutstandingTasks;



public:
	MiniCLTaskScheduler(btThreadSupportInterface*	threadInterface, int maxNumOutstandingTasks);
	
	~MiniCLTaskScheduler();
	
	///call initialize in the beginning of the frame, before addCollisionPairToTask
	void initialize();

	void issueTask(int firstWorkUnit, int lastWorkUnit, MiniCLKernel* kernel);

	///call flush to submit potential outstanding work to SPUs and wait for all involved SPUs to be finished
	void flush();

	class	btThreadSupportInterface*	getThreadSupportInterface()
	{
		return m_threadInterface;
	}

	int	findProgramCommandIdByName(const char* programName) const;

	int getMaxNumOutstandingTasks() const
	{
		return m_maxNumOutstandingTasks;
	}

	void registerKernel(MiniCLKernel* kernel)
	{
		m_kernels.push_back(kernel);
	}
};

typedef void (*kernelLauncherCB)(MiniCLTaskDesc* taskDesc, int guid);

struct	MiniCLKernel
{
	MiniCLTaskScheduler* m_scheduler;
	
//	int	m_kernelProgramCommandId;

	char	m_name[MINI_CL_MAX_KERNEL_NAME];
	unsigned int	m_numArgs;
	kernelLauncherCB	m_launcher;
	void* m_pCode;
	void updateLauncher();
	MiniCLKernel* registerSelf();

	void*	m_argData[MINI_CL_MAX_ARG];
	int				m_argSizes[MINI_CL_MAX_ARG];
};


#if defined(USE_LIBSPE2) && defined(__SPU__)
////////////////////MAIN/////////////////////////////
#include "../SpuLibspe2Support.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <SpuFakeDma.h>

void * SamplelsMemoryFunc();
void SampleThreadFunc(void* userPtr,void* lsMemory);

//#define DEBUG_LIBSPE2_MAINLOOP

int main(unsigned long long speid, addr64 argp, addr64 envp)
{
	printf("SPU is up \n");
	
	ATTRIBUTE_ALIGNED128(btSpuStatus status);
	ATTRIBUTE_ALIGNED16( SpuSampleTaskDesc taskDesc ) ;
	unsigned int received_message = Spu_Mailbox_Event_Nothing;
        bool shutdown = false;

	cellDmaGet(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
	cellDmaWaitTagStatusAll(DMA_MASK(3));

	status.m_status = Spu_Status_Free;
	status.m_lsMemory.p = SamplelsMemoryFunc();

	cellDmaLargePut(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
	cellDmaWaitTagStatusAll(DMA_MASK(3));
	
	
	while (!shutdown)
	{
		received_message = spu_read_in_mbox();
		

		
		switch(received_message)
		{
		case Spu_Mailbox_Event_Shutdown:
			shutdown = true;
			break; 
		case Spu_Mailbox_Event_Task:
			// refresh the status
#ifdef DEBUG_LIBSPE2_MAINLOOP
			printf("SPU recieved Task \n");
#endif //DEBUG_LIBSPE2_MAINLOOP
			cellDmaGet(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
			cellDmaWaitTagStatusAll(DMA_MASK(3));
		
			btAssert(status.m_status==Spu_Status_Occupied);
			
			cellDmaGet(&taskDesc, status.m_taskDesc.p, sizeof(SpuSampleTaskDesc), DMA_TAG(3), 0, 0);
			cellDmaWaitTagStatusAll(DMA_MASK(3));
			
			SampleThreadFunc((void*)&taskDesc, reinterpret_cast<void*> (taskDesc.m_mainMemoryPtr) );
			break;
		case Spu_Mailbox_Event_Nothing:
		default:
			break;
		}

		// set to status free and wait for next task
		status.m_status = Spu_Status_Free;
		cellDmaLargePut(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
		cellDmaWaitTagStatusAll(DMA_MASK(3));		
				
		
  	}
  	return 0;
}
//////////////////////////////////////////////////////
#endif



#endif // MINICL_TASK_SCHEDULER_H

