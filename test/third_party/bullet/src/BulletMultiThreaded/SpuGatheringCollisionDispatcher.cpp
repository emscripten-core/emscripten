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

#include "SpuGatheringCollisionDispatcher.h"
#include "SpuCollisionTaskProcess.h"


#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"
#include "BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.h"
#include "SpuContactManifoldCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "LinearMath/btQuickprof.h"
#include "BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuCollisionShapes.h"





SpuGatheringCollisionDispatcher::SpuGatheringCollisionDispatcher(class	btThreadSupportInterface*	threadInterface, unsigned int	maxNumOutstandingTasks,btCollisionConfiguration* collisionConfiguration)
:btCollisionDispatcher(collisionConfiguration),
m_spuCollisionTaskProcess(0),
m_threadInterface(threadInterface),
m_maxNumOutstandingTasks(maxNumOutstandingTasks)
{
	
}


bool	SpuGatheringCollisionDispatcher::supportsDispatchPairOnSpu(int proxyType0,int proxyType1)
{
	bool supported0 = (
		(proxyType0 == BOX_SHAPE_PROXYTYPE) ||
		(proxyType0 == TRIANGLE_SHAPE_PROXYTYPE) ||
		(proxyType0 == SPHERE_SHAPE_PROXYTYPE) ||
		(proxyType0 == CAPSULE_SHAPE_PROXYTYPE) ||
		(proxyType0 == CYLINDER_SHAPE_PROXYTYPE) ||
//		(proxyType0 == CONE_SHAPE_PROXYTYPE) ||
		(proxyType0 == TRIANGLE_MESH_SHAPE_PROXYTYPE) ||
		(proxyType0 == CONVEX_HULL_SHAPE_PROXYTYPE)||
		(proxyType0 == STATIC_PLANE_PROXYTYPE)||
		(proxyType0 == COMPOUND_SHAPE_PROXYTYPE)
		);

	bool supported1 = (
		(proxyType1 == BOX_SHAPE_PROXYTYPE) ||
		(proxyType1 == TRIANGLE_SHAPE_PROXYTYPE) ||
		(proxyType1 == SPHERE_SHAPE_PROXYTYPE) ||
		(proxyType1 == CAPSULE_SHAPE_PROXYTYPE) ||
		(proxyType1 == CYLINDER_SHAPE_PROXYTYPE) ||
//		(proxyType1 == CONE_SHAPE_PROXYTYPE) ||
		(proxyType1 == TRIANGLE_MESH_SHAPE_PROXYTYPE) ||
		(proxyType1 == CONVEX_HULL_SHAPE_PROXYTYPE) ||
		(proxyType1 == STATIC_PLANE_PROXYTYPE) ||
		(proxyType1 == COMPOUND_SHAPE_PROXYTYPE)
		);

	
	return supported0 && supported1;
}



SpuGatheringCollisionDispatcher::~SpuGatheringCollisionDispatcher()
{
	if (m_spuCollisionTaskProcess)
		delete m_spuCollisionTaskProcess;
	
}

#include "stdio.h"



///interface for iterating all overlapping collision pairs, no matter how those pairs are stored (array, set, map etc)
///this is useful for the collision dispatcher.
class btSpuCollisionPairCallback : public btOverlapCallback
{
	const btDispatcherInfo& m_dispatchInfo;
	SpuGatheringCollisionDispatcher*	m_dispatcher;

public:

	btSpuCollisionPairCallback(const btDispatcherInfo& dispatchInfo, SpuGatheringCollisionDispatcher*	dispatcher)
	:m_dispatchInfo(dispatchInfo),
	m_dispatcher(dispatcher)
	{
	}

	virtual bool	processOverlap(btBroadphasePair& collisionPair)
	{


		//PPU version
		//(*m_dispatcher->getNearCallback())(collisionPair,*m_dispatcher,m_dispatchInfo);

		//only support discrete collision detection for now, we could fallback on PPU/unoptimized version for TOI/CCD
		btAssert(m_dispatchInfo.m_dispatchFunc == btDispatcherInfo::DISPATCH_DISCRETE);

		//by default, Bullet will use this near callback
		{
			///userInfo is used to determine if the SPU has to handle this case or not (skip PPU tasks)
			if (!collisionPair.m_internalTmpValue)
			{
				collisionPair.m_internalTmpValue = 1;
			}
			if (!collisionPair.m_algorithm)
			{
				btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
				btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;

				btCollisionAlgorithmConstructionInfo ci;
				ci.m_dispatcher1 = m_dispatcher;
				ci.m_manifold = 0;

				if (m_dispatcher->needsCollision(colObj0,colObj1))
				{
					int	proxyType0 = colObj0->getCollisionShape()->getShapeType();
					int	proxyType1 = colObj1->getCollisionShape()->getShapeType();
					bool supportsSpuDispatch = m_dispatcher->supportsDispatchPairOnSpu(proxyType0,proxyType1) 
						&& ((colObj0->getCollisionFlags() & btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING) == 0)
						&& ((colObj1->getCollisionFlags() & btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING) == 0);

					if (proxyType0 == COMPOUND_SHAPE_PROXYTYPE)
					{
						btCompoundShape* compound = (btCompoundShape*)colObj0->getCollisionShape();
						if (compound->getNumChildShapes()>MAX_SPU_COMPOUND_SUBSHAPES)
						{
							//printf("PPU fallback, compound->getNumChildShapes(%d)>%d\n",compound->getNumChildShapes(),MAX_SPU_COMPOUND_SUBSHAPES);
							supportsSpuDispatch = false;
						}
					}

					if (proxyType1 == COMPOUND_SHAPE_PROXYTYPE)
					{
						btCompoundShape* compound = (btCompoundShape*)colObj1->getCollisionShape();
						if (compound->getNumChildShapes()>MAX_SPU_COMPOUND_SUBSHAPES)
						{
							//printf("PPU fallback, compound->getNumChildShapes(%d)>%d\n",compound->getNumChildShapes(),MAX_SPU_COMPOUND_SUBSHAPES);
							supportsSpuDispatch = false;
						}
					}

					if (supportsSpuDispatch)
					{

						int so = sizeof(SpuContactManifoldCollisionAlgorithm);
#ifdef ALLOCATE_SEPARATELY
						void* mem = btAlignedAlloc(so,16);//m_dispatcher->allocateCollisionAlgorithm(so);
#else
						void* mem = m_dispatcher->allocateCollisionAlgorithm(so);
#endif
						collisionPair.m_algorithm = new(mem) SpuContactManifoldCollisionAlgorithm(ci,colObj0,colObj1);
						collisionPair.m_internalTmpValue =  2;
					} else
					{
						collisionPair.m_algorithm = m_dispatcher->findAlgorithm(colObj0,colObj1);
						collisionPair.m_internalTmpValue = 3;
					}
				} 
			}
		}
		return false;
	}
};

void	SpuGatheringCollisionDispatcher::dispatchAllCollisionPairs(btOverlappingPairCache* pairCache,const btDispatcherInfo& dispatchInfo, btDispatcher* dispatcher) 
{

	if (dispatchInfo.m_enableSPU)
	{
		m_maxNumOutstandingTasks = m_threadInterface->getNumTasks();

		{
			BT_PROFILE("processAllOverlappingPairs");

			if (!m_spuCollisionTaskProcess)
				m_spuCollisionTaskProcess = new SpuCollisionTaskProcess(m_threadInterface,m_maxNumOutstandingTasks);
		
			m_spuCollisionTaskProcess->setNumTasks(m_maxNumOutstandingTasks);
	//		printf("m_maxNumOutstandingTasks =%d\n",m_maxNumOutstandingTasks);

			m_spuCollisionTaskProcess->initialize2(dispatchInfo.m_useEpa);
			
		
			///modified version of btCollisionDispatcher::dispatchAllCollisionPairs:
			{
				btSpuCollisionPairCallback	collisionCallback(dispatchInfo,this);

				pairCache->processAllOverlappingPairs(&collisionCallback,dispatcher);
			}
		}

		//send one big batch
		int numTotalPairs = pairCache->getNumOverlappingPairs();
		
		btBroadphasePair* pairPtr = pairCache->getOverlappingPairArrayPtr();
		int i;
		{
			int pairRange =	SPU_BATCHSIZE_BROADPHASE_PAIRS;
			if (numTotalPairs < (m_spuCollisionTaskProcess->getNumTasks()*SPU_BATCHSIZE_BROADPHASE_PAIRS))
			{
				pairRange = (numTotalPairs/m_spuCollisionTaskProcess->getNumTasks())+1;
			}

			BT_PROFILE("addWorkToTask");
			for (i=0;i<numTotalPairs;)
			{
				//Performance Hint: tweak this number during benchmarking
				
				int endIndex = (i+pairRange) < numTotalPairs ? i+pairRange : numTotalPairs;
				m_spuCollisionTaskProcess->addWorkToTask(pairPtr,i,endIndex);
				i = endIndex;
			}
		}

		{
			BT_PROFILE("PPU fallback");
			//handle PPU fallback pairs
			for (i=0;i<numTotalPairs;i++)
			{
				btBroadphasePair& collisionPair = pairPtr[i];
				if (collisionPair.m_internalTmpValue == 3)
				{
					if (collisionPair.m_algorithm)
					{
						btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
						btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;

						if (dispatcher->needsCollision(colObj0,colObj1))
						{
							btManifoldResult contactPointResult(colObj0,colObj1);
							
							if (dispatchInfo.m_dispatchFunc == 		btDispatcherInfo::DISPATCH_DISCRETE)
							{
								//discrete collision detection query
								collisionPair.m_algorithm->processCollision(colObj0,colObj1,dispatchInfo,&contactPointResult);
							} else
							{
								//continuous collision detection query, time of impact (toi)
								btScalar toi = collisionPair.m_algorithm->calculateTimeOfImpact(colObj0,colObj1,dispatchInfo,&contactPointResult);
								if (dispatchInfo.m_timeOfImpact > toi)
									dispatchInfo.m_timeOfImpact = toi;

							}
						}
					}
				}
			}
		}
		{
			BT_PROFILE("flush2");
			//make sure all SPU work is done
			m_spuCollisionTaskProcess->flush2();
		}

	} else
	{
		///PPU fallback
		///!Need to make sure to clear all 'algorithms' when switching between SPU and PPU
		btCollisionDispatcher::dispatchAllCollisionPairs(pairCache,dispatchInfo,dispatcher);
	}
}
