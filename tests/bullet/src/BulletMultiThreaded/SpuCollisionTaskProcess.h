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

#ifndef BT_SPU_COLLISION_TASK_PROCESS_H
#define BT_SPU_COLLISION_TASK_PROCESS_H

#include <assert.h>

#include "LinearMath/btScalar.h"

#include "PlatformDefinitions.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h" // for definitions processCollisionTask and createCollisionLocalStoreMemory

#include "btThreadSupportInterface.h"


//#include "SPUAssert.h"
#include <string.h>


#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btConvexShape.h"

#include "LinearMath/btAlignedAllocator.h"

#include <stdio.h>


#define DEBUG_SpuCollisionTaskProcess 1


#define CMD_GATHER_AND_PROCESS_PAIRLIST	1

class btCollisionObject;
class btPersistentManifold;
class btDispatcher;


/////Task Description for SPU collision detection
//struct SpuGatherAndProcessPairsTaskDesc
//{
//	uint64_t	inPtr;//m_pairArrayPtr;
//	//mutex variable
//	uint32_t	m_someMutexVariableInMainMemory;
//
//	uint64_t	m_dispatcher;
//
//	uint32_t	numOnLastPage;
//
//	uint16_t numPages;
//	uint16_t taskId;
//
//	struct	CollisionTask_LocalStoreMemory*	m_lsMemory; 
//}
//
//#if  defined(__CELLOS_LV2__) || defined(USE_LIBSPE2)
//__attribute__ ((aligned (16)))
//#endif
//;


///MidphaseWorkUnitInput stores individual primitive versus mesh collision detection input, to be processed by the SPU.
ATTRIBUTE_ALIGNED16(struct) SpuGatherAndProcessWorkUnitInput
{
	uint64_t m_pairArrayPtr;
	int		m_startIndex;
	int		m_endIndex;
};




/// SpuCollisionTaskProcess handles SPU processing of collision pairs.
/// Maintains a set of task buffers.
/// When the task is full, the task is issued for SPUs to process.  Contact output goes into btPersistentManifold
/// associated with each task.
/// When PPU issues a task, it will look for completed task buffers
/// PPU will do postprocessing, dependent on workunit output (not likely)
class SpuCollisionTaskProcess
{

  unsigned char  *m_workUnitTaskBuffers;


	// track task buffers that are being used, and total busy tasks
	btAlignedObjectArray<bool>	m_taskBusy;
	btAlignedObjectArray<SpuGatherAndProcessPairsTaskDesc>	m_spuGatherTaskDesc;

	class	btThreadSupportInterface*	m_threadInterface;

	unsigned int	m_maxNumOutstandingTasks;

	unsigned int   m_numBusyTasks;

	// the current task and the current entry to insert a new work unit
	unsigned int   m_currentTask;
	unsigned int   m_currentPage;
	unsigned int   m_currentPageEntry;

	bool m_useEpa;

#ifdef DEBUG_SpuCollisionTaskProcess
	bool m_initialized;
#endif
	void issueTask2();
	//void postProcess(unsigned int taskId, int outputSize);

public:
	SpuCollisionTaskProcess(btThreadSupportInterface*	threadInterface, unsigned int maxNumOutstandingTasks);
	
	~SpuCollisionTaskProcess();
	
	///call initialize in the beginning of the frame, before addCollisionPairToTask
	void initialize2(bool useEpa = false);

	///batch up additional work to a current task for SPU processing. When batch is full, it issues the task.
	void addWorkToTask(void* pairArrayPtr,int startIndex,int endIndex);

	///call flush to submit potential outstanding work to SPUs and wait for all involved SPUs to be finished
	void flush2();

	/// set the maximum number of SPU tasks allocated
	void	setNumTasks(int maxNumTasks);

	int		getNumTasks() const
	{
		return m_maxNumOutstandingTasks;
	}
};



#define MIDPHASE_TASK_PTR(task) (&m_workUnitTaskBuffers[0] + MIDPHASE_WORKUNIT_TASK_SIZE*task)
#define MIDPHASE_ENTRY_PTR(task,page,entry) (MIDPHASE_TASK_PTR(task) + MIDPHASE_WORKUNIT_PAGE_SIZE*page + sizeof(SpuGatherAndProcessWorkUnitInput)*entry)
#define MIDPHASE_OUTPUT_PTR(task) (&m_contactOutputBuffers[0] + MIDPHASE_MAX_CONTACT_BUFFER_SIZE*task)
#define MIDPHASE_TREENODES_PTR(task) (&m_complexShapeBuffers[0] + MIDPHASE_COMPLEX_SHAPE_BUFFER_SIZE*task)


#define MIDPHASE_WORKUNIT_PAGE_SIZE (16)
//#define MIDPHASE_WORKUNIT_PAGE_SIZE (128)

#define MIDPHASE_NUM_WORKUNIT_PAGES 1
#define MIDPHASE_WORKUNIT_TASK_SIZE (MIDPHASE_WORKUNIT_PAGE_SIZE*MIDPHASE_NUM_WORKUNIT_PAGES)
#define MIDPHASE_NUM_WORKUNITS_PER_PAGE (MIDPHASE_WORKUNIT_PAGE_SIZE / sizeof(SpuGatherAndProcessWorkUnitInput))
#define MIDPHASE_NUM_WORKUNITS_PER_TASK (MIDPHASE_NUM_WORKUNITS_PER_PAGE*MIDPHASE_NUM_WORKUNIT_PAGES)


#endif // BT_SPU_COLLISION_TASK_PROCESS_H

