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

//#define __CELLOS_LV2__ 1
#define __BT_SKIP_UINT64_H 1

#define USE_SAMPLE_PROCESS 1
#ifdef USE_SAMPLE_PROCESS


#include "MiniCLTaskScheduler.h"
#include <stdio.h>

#ifdef __SPU__



void	SampleThreadFunc(void* userPtr,void* lsMemory)
{
	//do nothing
	printf("hello world\n");
}


void*	SamplelsMemoryFunc()
{
	//don't create local store memory, just return 0
	return 0;
}


#else


#include "BulletMultiThreaded/btThreadSupportInterface.h"

//#	include "SPUAssert.h"
#include <string.h>

#include "MiniCL/cl_platform.h"

extern "C" {
	extern char SPU_SAMPLE_ELF_SYMBOL[];
}


MiniCLTaskScheduler::MiniCLTaskScheduler(btThreadSupportInterface*	threadInterface,  int maxNumOutstandingTasks)
:m_threadInterface(threadInterface),
m_maxNumOutstandingTasks(maxNumOutstandingTasks)
{

	m_taskBusy.resize(m_maxNumOutstandingTasks);
	m_spuSampleTaskDesc.resize(m_maxNumOutstandingTasks);

	m_kernels.resize(0);

	for (int i = 0; i < m_maxNumOutstandingTasks; i++)
	{
		m_taskBusy[i] = false;
	}
	m_numBusyTasks = 0;
	m_currentTask = 0;

	m_initialized = false;

	m_threadInterface->startSPU();


}

MiniCLTaskScheduler::~MiniCLTaskScheduler()
{
	m_threadInterface->stopSPU();
	
}



void	MiniCLTaskScheduler::initialize()
{
#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("MiniCLTaskScheduler::initialize()\n");
#endif //DEBUG_SPU_TASK_SCHEDULING
	
	for (int i = 0; i < m_maxNumOutstandingTasks; i++)
	{
		m_taskBusy[i] = false;
	}
	m_numBusyTasks = 0;
	m_currentTask = 0;
	m_initialized = true;

}


void MiniCLTaskScheduler::issueTask(int firstWorkUnit, int lastWorkUnit, MiniCLKernel* kernel)
{

#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("MiniCLTaskScheduler::issueTask (m_currentTask= %d\)n", m_currentTask);
#endif //DEBUG_SPU_TASK_SCHEDULING

	m_taskBusy[m_currentTask] = true;
	m_numBusyTasks++;

	MiniCLTaskDesc& taskDesc = m_spuSampleTaskDesc[m_currentTask];
	{
		// send task description in event message
		taskDesc.m_firstWorkUnit = firstWorkUnit;
		taskDesc.m_lastWorkUnit = lastWorkUnit;
		taskDesc.m_kernel = kernel;
		//some bookkeeping to recognize finished tasks
		taskDesc.m_taskId = m_currentTask;
		
//		for (int i=0;i<MINI_CL_MAX_ARG;i++)
		for (unsigned int i=0; i < kernel->m_numArgs; i++)
		{
			taskDesc.m_argSizes[i] = kernel->m_argSizes[i];
			if (taskDesc.m_argSizes[i])
			{
				taskDesc.m_argData[i] = kernel->m_argData[i];
//				memcpy(&taskDesc.m_argData[i],&argData[MINICL_MAX_ARGLENGTH*i],taskDesc.m_argSizes[i]);
			}
		}
	}


	m_threadInterface->sendRequest(1, (ppu_address_t) &taskDesc, m_currentTask);

	// if all tasks busy, wait for spu event to clear the task.
	
	if (m_numBusyTasks >= m_maxNumOutstandingTasks)
	{
		unsigned int taskId;
		unsigned int outputSize;

		for (int i=0;i<m_maxNumOutstandingTasks;i++)
	  {
		  if (m_taskBusy[i])
		  {
			  taskId = i;
			  break;
		  }
	  }
		m_threadInterface->waitForResponse(&taskId, &outputSize);

		//printf("PPU: after issue, received event: %u %d\n", taskId, outputSize);

		postProcess(taskId, outputSize);

		m_taskBusy[taskId] = false;

		m_numBusyTasks--;
	}

	// find new task buffer
	for (int i = 0; i < m_maxNumOutstandingTasks; i++)
	{
		if (!m_taskBusy[i])
		{
			m_currentTask = i;
			break;
		}
	}
}


///Optional PPU-size post processing for each task
void MiniCLTaskScheduler::postProcess(int taskId, int outputSize)
{

}


void MiniCLTaskScheduler::flush()
{
#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("\nSpuCollisionTaskProcess::flush()\n");
#endif //DEBUG_SPU_TASK_SCHEDULING
	

	// all tasks are issued, wait for all tasks to be complete
	while(m_numBusyTasks > 0)
	{
// Consolidating SPU code
	  unsigned int taskId;
	  unsigned int outputSize;
	  
	  for (int i=0;i<m_maxNumOutstandingTasks;i++)
	  {
		  if (m_taskBusy[i])
		  {
			  taskId = i;
			  break;
		  }
	  }
	  {
			
		  m_threadInterface->waitForResponse(&taskId, &outputSize);
	  }

		//printf("PPU: flushing, received event: %u %d\n", taskId, outputSize);

		postProcess(taskId, outputSize);

		m_taskBusy[taskId] = false;

		m_numBusyTasks--;
	}


}



typedef void (*MiniCLKernelLauncher0)(int);
typedef void (*MiniCLKernelLauncher1)(void*, int);
typedef void (*MiniCLKernelLauncher2)(void*, void*, int);
typedef void (*MiniCLKernelLauncher3)(void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher4)(void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher5)(void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher6)(void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher7)(void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher8)(void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher9)(void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher10)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher11)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher12)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher13)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher14)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher15)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);
typedef void (*MiniCLKernelLauncher16)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*, int);


static void kernelLauncher0(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher0)(taskDesc->m_kernel->m_launcher))(guid);
}
static void kernelLauncher1(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher1)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												guid);
}
static void kernelLauncher2(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher2)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												guid);
}
static void kernelLauncher3(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher3)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												guid);
}
static void kernelLauncher4(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher4)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												guid);
}
static void kernelLauncher5(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher5)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												guid);
}
static void kernelLauncher6(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher6)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												guid);
}
static void kernelLauncher7(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher7)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												guid);
}
static void kernelLauncher8(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher8)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												guid);
}
static void kernelLauncher9(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher9)(taskDesc->m_kernel->m_pCode))(	taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												guid);
}
static void kernelLauncher10(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher10)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												guid);
}
static void kernelLauncher11(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher11)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												guid);
}
static void kernelLauncher12(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher12)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												taskDesc->m_argData[11], 
												guid);
}
static void kernelLauncher13(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher13)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												taskDesc->m_argData[11], 
												taskDesc->m_argData[12], 
												guid);
}
static void kernelLauncher14(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher14)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												taskDesc->m_argData[11], 
												taskDesc->m_argData[12], 
												taskDesc->m_argData[13], 
												guid);
}
static void kernelLauncher15(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher15)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												taskDesc->m_argData[11], 
												taskDesc->m_argData[12], 
												taskDesc->m_argData[13], 
												taskDesc->m_argData[14], 
												guid);
}
static void kernelLauncher16(MiniCLTaskDesc* taskDesc, int guid)
{
	((MiniCLKernelLauncher16)(taskDesc->m_kernel->m_pCode))(taskDesc->m_argData[0], 
												taskDesc->m_argData[1], 
												taskDesc->m_argData[2], 
												taskDesc->m_argData[3], 
												taskDesc->m_argData[4], 
												taskDesc->m_argData[5], 
												taskDesc->m_argData[6], 
												taskDesc->m_argData[7], 
												taskDesc->m_argData[8], 
												taskDesc->m_argData[9], 
												taskDesc->m_argData[10], 
												taskDesc->m_argData[11], 
												taskDesc->m_argData[12], 
												taskDesc->m_argData[13], 
												taskDesc->m_argData[14], 
												taskDesc->m_argData[15], 
												guid);
}

static kernelLauncherCB spLauncherList[MINI_CL_MAX_ARG+1] = 
{
	kernelLauncher0,
	kernelLauncher1,
	kernelLauncher2,
	kernelLauncher3,
	kernelLauncher4,
	kernelLauncher5,
	kernelLauncher6,
	kernelLauncher7,
	kernelLauncher8,
	kernelLauncher9,
	kernelLauncher10,
	kernelLauncher11,
	kernelLauncher12,
	kernelLauncher13,
	kernelLauncher14,
	kernelLauncher15,
	kernelLauncher16
};

void MiniCLKernel::updateLauncher()
{
	m_launcher = spLauncherList[m_numArgs];
}

struct MiniCLKernelDescEntry
{
	void* pCode;
	char* pName;
};
static MiniCLKernelDescEntry spKernelDesc[256];
static int sNumKernelDesc = 0;

MiniCLKernelDesc::MiniCLKernelDesc(void* pCode, char* pName)
{
	for(int i = 0; i < sNumKernelDesc; i++)
	{
		if(!strcmp(pName, spKernelDesc[i].pName))
		{	// already registered
			btAssert(spKernelDesc[i].pCode == pCode);
			return; 
		}
	}
	spKernelDesc[sNumKernelDesc].pCode = pCode;
	spKernelDesc[sNumKernelDesc].pName = pName;
	sNumKernelDesc++;
}


MiniCLKernel* MiniCLKernel::registerSelf()
{
	m_scheduler->registerKernel(this);
	for(int i = 0; i < sNumKernelDesc; i++)
	{
		if(!strcmp(m_name, spKernelDesc[i].pName))
		{
			m_pCode = spKernelDesc[i].pCode;
			return this;
		}
	}
	return NULL;
}

#endif


#endif //USE_SAMPLE_PROCESS
