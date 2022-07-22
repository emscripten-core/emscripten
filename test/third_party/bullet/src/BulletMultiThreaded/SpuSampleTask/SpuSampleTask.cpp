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


#include "SpuSampleTask.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "../PlatformDefinitions.h"
#include "../SpuFakeDma.h"
#include "LinearMath/btMinMax.h"

#ifdef __SPU__
#include <spu_printf.h>
#else
#include <stdio.h>
#define spu_printf printf
#endif

#define MAX_NUM_BODIES 8192

struct SampleTask_LocalStoreMemory
{
	ATTRIBUTE_ALIGNED16(char gLocalRigidBody [sizeof(btRigidBody)+16]);
	ATTRIBUTE_ALIGNED16(void* gPointerArray[MAX_NUM_BODIES]);

};




//-- MAIN METHOD
void processSampleTask(void* userPtr, void* lsMemory)
{
	//	BT_PROFILE("processSampleTask");

	SampleTask_LocalStoreMemory* localMemory = (SampleTask_LocalStoreMemory*)lsMemory;

	SpuSampleTaskDesc* taskDescPtr = (SpuSampleTaskDesc*)userPtr;
	SpuSampleTaskDesc& taskDesc = *taskDescPtr;

	switch (taskDesc.m_sampleCommand)
	{
	case CMD_SAMPLE_INTEGRATE_BODIES:
		{
			btTransform predictedTrans;
			btCollisionObject** eaPtr = (btCollisionObject**)taskDesc.m_mainMemoryPtr;

			int batchSize = taskDesc.m_sampleValue;
			if (batchSize>MAX_NUM_BODIES)
			{
				spu_printf("SPU Error: exceed number of bodies, see MAX_NUM_BODIES in SpuSampleTask.cpp\n");
				break;
			}
			int dmaArraySize = batchSize*sizeof(void*);

			uint64_t ppuArrayAddress = reinterpret_cast<uint64_t>(eaPtr);

			//			spu_printf("array location is at %llx, batchSize = %d, DMA size = %d\n",ppuArrayAddress,batchSize,dmaArraySize);

			if (dmaArraySize>=16)
			{
				cellDmaLargeGet((void*)&localMemory->gPointerArray[0], ppuArrayAddress  , dmaArraySize, DMA_TAG(1), 0, 0);	
				cellDmaWaitTagStatusAll(DMA_MASK(1));
			} else
			{
				stallingUnalignedDmaSmallGet((void*)&localMemory->gPointerArray[0], ppuArrayAddress  , dmaArraySize);
			}


			for ( int i=0;i<batchSize;i++)
			{
				///DMA rigid body

				void* localPtr = &localMemory->gLocalRigidBody[0];
				void* shortAdd = localMemory->gPointerArray[i];
				uint64_t ppuRigidBodyAddress = reinterpret_cast<uint64_t>(shortAdd);

				//	spu_printf("cellDmaGet at CMD_SAMPLE_INTEGRATE_BODIES from %llx to %llx\n",ppuRigidBodyAddress,localPtr);

				int dmaBodySize = sizeof(btRigidBody);

				cellDmaGet((void*)localPtr, ppuRigidBodyAddress  , dmaBodySize, DMA_TAG(1), 0, 0);	
				cellDmaWaitTagStatusAll(DMA_MASK(1));


				float timeStep = 1.f/60.f;

				btRigidBody* body = (btRigidBody*) localPtr;//btRigidBody::upcast(colObj);
				if (body)
				{
					if (body->isActive() && (!body->isStaticOrKinematicObject()))
					{
						body->predictIntegratedTransform(timeStep, predictedTrans);
						body->proceedToTransform( predictedTrans);
						void* ptr = (void*)localPtr;
						//	spu_printf("cellDmaLargePut from %llx to LS %llx\n",ptr,ppuRigidBodyAddress);

						cellDmaLargePut(ptr, ppuRigidBodyAddress  , dmaBodySize, DMA_TAG(1), 0, 0);
						cellDmaWaitTagStatusAll(DMA_MASK(1));

					}
				}

			}
			break;
		}


	case CMD_SAMPLE_PREDICT_MOTION_BODIES:
		{
			btTransform predictedTrans;
			btCollisionObject** eaPtr = (btCollisionObject**)taskDesc.m_mainMemoryPtr;

			int batchSize = taskDesc.m_sampleValue;
			int dmaArraySize = batchSize*sizeof(void*);

			if (batchSize>MAX_NUM_BODIES)
			{
				spu_printf("SPU Error: exceed number of bodies, see MAX_NUM_BODIES in SpuSampleTask.cpp\n");
				break;
			}

			uint64_t ppuArrayAddress = reinterpret_cast<uint64_t>(eaPtr);

			//			spu_printf("array location is at %llx, batchSize = %d, DMA size = %d\n",ppuArrayAddress,batchSize,dmaArraySize);

			if (dmaArraySize>=16)
			{
				cellDmaLargeGet((void*)&localMemory->gPointerArray[0], ppuArrayAddress  , dmaArraySize, DMA_TAG(1), 0, 0);	
				cellDmaWaitTagStatusAll(DMA_MASK(1));
			} else
			{
				stallingUnalignedDmaSmallGet((void*)&localMemory->gPointerArray[0], ppuArrayAddress  , dmaArraySize);
			}


			for ( int i=0;i<batchSize;i++)
			{
				///DMA rigid body

				void* localPtr = &localMemory->gLocalRigidBody[0];
				void* shortAdd = localMemory->gPointerArray[i];
				uint64_t ppuRigidBodyAddress = reinterpret_cast<uint64_t>(shortAdd);

				//	spu_printf("cellDmaGet at CMD_SAMPLE_INTEGRATE_BODIES from %llx to %llx\n",ppuRigidBodyAddress,localPtr);

				int dmaBodySize = sizeof(btRigidBody);

				cellDmaGet((void*)localPtr, ppuRigidBodyAddress  , dmaBodySize, DMA_TAG(1), 0, 0);	
				cellDmaWaitTagStatusAll(DMA_MASK(1));


				float timeStep = 1.f/60.f;

				btRigidBody* body = (btRigidBody*) localPtr;//btRigidBody::upcast(colObj);
				if (body)
				{
					if (!body->isStaticOrKinematicObject())
					{
						if (body->isActive())
						{
							body->integrateVelocities( timeStep);
							//damping
							body->applyDamping(timeStep);

							body->predictIntegratedTransform(timeStep,body->getInterpolationWorldTransform());

							void* ptr = (void*)localPtr;
							cellDmaLargePut(ptr, ppuRigidBodyAddress  , dmaBodySize, DMA_TAG(1), 0, 0);
							cellDmaWaitTagStatusAll(DMA_MASK(1));
						}
					}
				}

			}
			break;
		}
	


	default:
		{

		}
	};
}


#if defined(__CELLOS_LV2__) || defined (LIBSPE2)

ATTRIBUTE_ALIGNED16(SampleTask_LocalStoreMemory	gLocalStoreMemory);

void* createSampleLocalStoreMemory()
{
	return &gLocalStoreMemory;
}
#else
void* createSampleLocalStoreMemory()
{
	return new SampleTask_LocalStoreMemory;
};

#endif
