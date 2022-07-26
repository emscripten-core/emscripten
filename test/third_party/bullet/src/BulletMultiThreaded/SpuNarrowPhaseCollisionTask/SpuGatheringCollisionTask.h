/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SPU_GATHERING_COLLISION_TASK_H
#define SPU_GATHERING_COLLISION_TASK_H

#include "../PlatformDefinitions.h"
//#define DEBUG_SPU_COLLISION_DETECTION 1


///Task Description for SPU collision detection
struct SpuGatherAndProcessPairsTaskDesc 
{
	ppu_address_t	m_inPairPtr;//m_pairArrayPtr;
	//mutex variable
	uint32_t	m_someMutexVariableInMainMemory;

	ppu_address_t	m_dispatcher;

	uint32_t	numOnLastPage;

	uint16_t numPages;
	uint16_t taskId;
	bool m_useEpa;

	struct	CollisionTask_LocalStoreMemory*	m_lsMemory; 
}

#if  defined(__CELLOS_LV2__) || defined(USE_LIBSPE2)
__attribute__ ((aligned (128)))
#endif
;


void	processCollisionTask(void* userPtr, void* lsMemory);

void*	createCollisionLocalStoreMemory();


#if defined(USE_LIBSPE2) && defined(__SPU__)
#include "../SpuLibspe2Support.h"
#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <SpuFakeDma.h>

//#define DEBUG_LIBSPE2_SPU_TASK



int main(unsigned long long speid, addr64 argp, addr64 envp)
{
	printf("SPU: hello \n");
	
	ATTRIBUTE_ALIGNED128(btSpuStatus status);
	ATTRIBUTE_ALIGNED16( SpuGatherAndProcessPairsTaskDesc taskDesc ) ;
	unsigned int received_message = Spu_Mailbox_Event_Nothing;
    bool shutdown = false;

	cellDmaGet(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
	cellDmaWaitTagStatusAll(DMA_MASK(3));

	status.m_status = Spu_Status_Free;
	status.m_lsMemory.p = createCollisionLocalStoreMemory();

	cellDmaLargePut(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
	cellDmaWaitTagStatusAll(DMA_MASK(3));
	
	
	while ( btLikely( !shutdown ) )
	{
		
		received_message = spu_read_in_mbox();
		
		if( btLikely( received_message == Spu_Mailbox_Event_Task ))
		{
#ifdef DEBUG_LIBSPE2_SPU_TASK
			printf("SPU: received Spu_Mailbox_Event_Task\n");
#endif //DEBUG_LIBSPE2_SPU_TASK

			// refresh the status
			cellDmaGet(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
			cellDmaWaitTagStatusAll(DMA_MASK(3));
		
			btAssert(status.m_status==Spu_Status_Occupied);
			
			cellDmaGet(&taskDesc, status.m_taskDesc.p, sizeof(SpuGatherAndProcessPairsTaskDesc), DMA_TAG(3), 0, 0);
			cellDmaWaitTagStatusAll(DMA_MASK(3));
#ifdef DEBUG_LIBSPE2_SPU_TASK		
			printf("SPU:processCollisionTask\n");	
#endif //DEBUG_LIBSPE2_SPU_TASK
			processCollisionTask((void*)&taskDesc, taskDesc.m_lsMemory);
			
#ifdef DEBUG_LIBSPE2_SPU_TASK
			printf("SPU:finished processCollisionTask\n");
#endif //DEBUG_LIBSPE2_SPU_TASK
		}
		else
		{
#ifdef DEBUG_LIBSPE2_SPU_TASK
			printf("SPU: received ShutDown\n");
#endif //DEBUG_LIBSPE2_SPU_TASK
			if( btLikely( received_message == Spu_Mailbox_Event_Shutdown ) )
			{
				shutdown = true;
			}
			else
			{
				//printf("SPU - Sth. recieved\n");
			}
		}

		// set to status free and wait for next task
		status.m_status = Spu_Status_Free;
		cellDmaLargePut(&status, argp.ull, sizeof(btSpuStatus), DMA_TAG(3), 0, 0);
		cellDmaWaitTagStatusAll(DMA_MASK(3));		
				
		
  	}

	printf("SPU: shutdown\n");
  	return 0;
}
#endif // USE_LIBSPE2


#endif //SPU_GATHERING_COLLISION_TASK_H


