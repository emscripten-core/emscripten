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


//#define DEBUG_SPU_TASK_SCHEDULING 1


//class OptimizedBvhNode;

#include "SpuCollisionTaskProcess.h"




void	SpuCollisionTaskProcess::setNumTasks(int maxNumTasks)
{
	if (int(m_maxNumOutstandingTasks) != maxNumTasks)
	{
		m_maxNumOutstandingTasks = maxNumTasks;
		m_taskBusy.resize(m_maxNumOutstandingTasks);
		m_spuGatherTaskDesc.resize(m_maxNumOutstandingTasks);

		for (int i = 0; i < m_taskBusy.size(); i++)
		{
			m_taskBusy[i] = false;
		}

		///re-allocate task memory buffers
		if (m_workUnitTaskBuffers != 0)
		{
			btAlignedFree(m_workUnitTaskBuffers);
		}
		
		m_workUnitTaskBuffers = (unsigned char *)btAlignedAlloc(MIDPHASE_WORKUNIT_TASK_SIZE*m_maxNumOutstandingTasks, 128);
	}
	
}



SpuCollisionTaskProcess::SpuCollisionTaskProcess(class	btThreadSupportInterface*	threadInterface, unsigned int	maxNumOutstandingTasks)
:m_threadInterface(threadInterface),
m_maxNumOutstandingTasks(0)
{
	m_workUnitTaskBuffers = (unsigned char *)0;
	setNumTasks(maxNumOutstandingTasks);
	m_numBusyTasks = 0;
	m_currentTask = 0;
	m_currentPage = 0;
	m_currentPageEntry = 0;

#ifdef DEBUG_SpuCollisionTaskProcess
	m_initialized = false;
#endif

	m_threadInterface->startSPU();

	//printf("sizeof vec_float4: %d\n", sizeof(vec_float4));
	printf("sizeof SpuGatherAndProcessWorkUnitInput: %d\n", int(sizeof(SpuGatherAndProcessWorkUnitInput)));

}

SpuCollisionTaskProcess::~SpuCollisionTaskProcess()
{
	
	if (m_workUnitTaskBuffers != 0)
	{
		btAlignedFree(m_workUnitTaskBuffers);
		m_workUnitTaskBuffers = 0;
	}
	


	m_threadInterface->stopSPU();
	
}



void SpuCollisionTaskProcess::initialize2(bool useEpa)
{

#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("SpuCollisionTaskProcess::initialize()\n");
#endif //DEBUG_SPU_TASK_SCHEDULING
	
	for (int i = 0; i < int (m_maxNumOutstandingTasks); i++)
	{
		m_taskBusy[i] = false;
	}
	m_numBusyTasks = 0;
	m_currentTask = 0;
	m_currentPage = 0;
	m_currentPageEntry = 0;
	m_useEpa = useEpa;

#ifdef DEBUG_SpuCollisionTaskProcess
	m_initialized = true;
	btAssert(MIDPHASE_NUM_WORKUNITS_PER_TASK*sizeof(SpuGatherAndProcessWorkUnitInput) <= MIDPHASE_WORKUNIT_TASK_SIZE);
#endif
}


void SpuCollisionTaskProcess::issueTask2()
{

#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("SpuCollisionTaskProcess::issueTask (m_currentTask= %d\n)", m_currentTask);
#endif //DEBUG_SPU_TASK_SCHEDULING

	m_taskBusy[m_currentTask] = true;
	m_numBusyTasks++;


	SpuGatherAndProcessPairsTaskDesc& taskDesc = m_spuGatherTaskDesc[m_currentTask];
	taskDesc.m_useEpa = m_useEpa;

	{
		// send task description in event message
		// no error checking here...
		// but, currently, event queue can be no larger than NUM_WORKUNIT_TASKS.
	
		taskDesc.m_inPairPtr = reinterpret_cast<uint64_t>(MIDPHASE_TASK_PTR(m_currentTask));
	
		taskDesc.taskId = m_currentTask;
		taskDesc.numPages = m_currentPage+1;
		taskDesc.numOnLastPage = m_currentPageEntry;
	}



	m_threadInterface->sendRequest(CMD_GATHER_AND_PROCESS_PAIRLIST, (ppu_address_t) &taskDesc,m_currentTask);

	// if all tasks busy, wait for spu event to clear the task.
	

	if (m_numBusyTasks >= m_maxNumOutstandingTasks)
	{
		unsigned int taskId;
		unsigned int outputSize;

		
		for (int i=0;i<int (m_maxNumOutstandingTasks);i++)
		  {
			  if (m_taskBusy[i])
			  {
				  taskId = i;
				  break;
			  }
		  }

	  btAssert(taskId>=0);

	  
		m_threadInterface->waitForResponse(&taskId, &outputSize);

//		printf("issueTask taskId %d completed, numBusy=%d\n",taskId,m_numBusyTasks);

		//printf("PPU: after issue, received event: %u %d\n", taskId, outputSize);

		//postProcess(taskId, outputSize);

		m_taskBusy[taskId] = false;

		m_numBusyTasks--;
	}
	
}

void SpuCollisionTaskProcess::addWorkToTask(void* pairArrayPtr,int startIndex,int endIndex)
{
#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("#");
#endif //DEBUG_SPU_TASK_SCHEDULING
	
#ifdef DEBUG_SpuCollisionTaskProcess
	btAssert(m_initialized);
	btAssert(m_workUnitTaskBuffers);

#endif

	bool batch = true;

	if (batch)
	{
		if (m_currentPageEntry == MIDPHASE_NUM_WORKUNITS_PER_PAGE)
		{
			if (m_currentPage == MIDPHASE_NUM_WORKUNIT_PAGES-1)
			{
				// task buffer is full, issue current task.
				// if all task buffers busy, this waits until SPU is done.
				issueTask2();

				// find new task buffer
				for (unsigned int i = 0; i < m_maxNumOutstandingTasks; i++)
				{
					if (!m_taskBusy[i])
					{
						m_currentTask = i;
						//init the task data

						break;
					}
				}

				m_currentPage = 0;
			}
			else
			{
				m_currentPage++;
			}

			m_currentPageEntry = 0;
		}
	}

	{



		SpuGatherAndProcessWorkUnitInput &wuInput = 
			*(reinterpret_cast<SpuGatherAndProcessWorkUnitInput*>
			(MIDPHASE_ENTRY_PTR(m_currentTask, m_currentPage, m_currentPageEntry)));
		
		wuInput.m_pairArrayPtr = reinterpret_cast<uint64_t>(pairArrayPtr);
		wuInput.m_startIndex = startIndex;
		wuInput.m_endIndex = endIndex;

		
	
		m_currentPageEntry++;

		if (!batch)
		{
			issueTask2();

			// find new task buffer
			for (unsigned int i = 0; i < m_maxNumOutstandingTasks; i++)
			{
				if (!m_taskBusy[i])
				{
					m_currentTask = i;
					//init the task data

					break;
				}
			}

			m_currentPage = 0;
			m_currentPageEntry =0;
		}
	}
}


void 
SpuCollisionTaskProcess::flush2()
{
#ifdef DEBUG_SPU_TASK_SCHEDULING
	printf("\nSpuCollisionTaskProcess::flush()\n");
#endif //DEBUG_SPU_TASK_SCHEDULING
	
	// if there's a partially filled task buffer, submit that task
	if (m_currentPage > 0 || m_currentPageEntry > 0)
	{
		issueTask2();
	}


	// all tasks are issued, wait for all tasks to be complete
	while(m_numBusyTasks > 0)
	{
	  // Consolidating SPU code
	  unsigned int taskId=-1;
	  unsigned int outputSize;
	  
	  for (int i=0;i<int (m_maxNumOutstandingTasks);i++)
	  {
		  if (m_taskBusy[i])
		  {
			  taskId = i;
			  break;
		  }
	  }

	  btAssert(taskId>=0);

	
	  {
			
		// SPURS support.
		  m_threadInterface->waitForResponse(&taskId, &outputSize);
	  }
//		 printf("flush2 taskId %d completed, numBusy =%d \n",taskId,m_numBusyTasks);
		//printf("PPU: flushing, received event: %u %d\n", taskId, outputSize);

		//postProcess(taskId, outputSize);

		m_taskBusy[taskId] = false;

		m_numBusyTasks--;
	}


}
