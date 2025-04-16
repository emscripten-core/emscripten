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



#include "btCollisionDispatcher.h"


#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"
#include "LinearMath/btPoolAllocator.h"
#include "BulletCollision/CollisionDispatch/btCollisionConfiguration.h"

int gNumManifold = 0;

#ifdef BT_DEBUG
#include <stdio.h>
#endif


btCollisionDispatcher::btCollisionDispatcher (btCollisionConfiguration* collisionConfiguration): 
m_dispatcherFlags(btCollisionDispatcher::CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD),
	m_collisionConfiguration(collisionConfiguration)
{
	int i;

	setNearCallback(defaultNearCallback);
	
	m_collisionAlgorithmPoolAllocator = collisionConfiguration->getCollisionAlgorithmPool();

	m_persistentManifoldPoolAllocator = collisionConfiguration->getPersistentManifoldPool();

	for (i=0;i<MAX_BROADPHASE_COLLISION_TYPES;i++)
	{
		for (int j=0;j<MAX_BROADPHASE_COLLISION_TYPES;j++)
		{
			m_doubleDispatch[i][j] = m_collisionConfiguration->getCollisionAlgorithmCreateFunc(i,j);
			btAssert(m_doubleDispatch[i][j]);
		}
	}
	
	
}


void btCollisionDispatcher::registerCollisionCreateFunc(int proxyType0, int proxyType1, btCollisionAlgorithmCreateFunc *createFunc)
{
	m_doubleDispatch[proxyType0][proxyType1] = createFunc;
}

btCollisionDispatcher::~btCollisionDispatcher()
{
}

btPersistentManifold*	btCollisionDispatcher::getNewManifold(void* b0,void* b1) 
{ 
	gNumManifold++;
	
	//btAssert(gNumManifold < 65535);
	

	btCollisionObject* body0 = (btCollisionObject*)b0;
	btCollisionObject* body1 = (btCollisionObject*)b1;

	//optional relative contact breaking threshold, turned on by default (use setDispatcherFlags to switch off feature for improved performance)
	
	btScalar contactBreakingThreshold =  (m_dispatcherFlags & btCollisionDispatcher::CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD) ? 
		btMin(body0->getCollisionShape()->getContactBreakingThreshold(gContactBreakingThreshold) , body1->getCollisionShape()->getContactBreakingThreshold(gContactBreakingThreshold))
		: gContactBreakingThreshold ;

	btScalar contactProcessingThreshold = btMin(body0->getContactProcessingThreshold(),body1->getContactProcessingThreshold());
		
	void* mem = 0;
	
	if (m_persistentManifoldPoolAllocator->getFreeCount())
	{
		mem = m_persistentManifoldPoolAllocator->allocate(sizeof(btPersistentManifold));
	} else
	{
		//we got a pool memory overflow, by default we fallback to dynamically allocate memory. If we require a contiguous contact pool then assert.
		if ((m_dispatcherFlags&CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION)==0)
		{
			mem = btAlignedAlloc(sizeof(btPersistentManifold),16);
		} else
		{
			btAssert(0);
			//make sure to increase the m_defaultMaxPersistentManifoldPoolSize in the btDefaultCollisionConstructionInfo/btDefaultCollisionConfiguration
			return 0;
		}
	}
	btPersistentManifold* manifold = new(mem) btPersistentManifold (body0,body1,0,contactBreakingThreshold,contactProcessingThreshold);
	manifold->m_index1a = m_manifoldsPtr.size();
	m_manifoldsPtr.push_back(manifold);

	return manifold;
}

void btCollisionDispatcher::clearManifold(btPersistentManifold* manifold)
{
	manifold->clearManifold();
}

	
void btCollisionDispatcher::releaseManifold(btPersistentManifold* manifold)
{
	
	gNumManifold--;

	//printf("releaseManifold: gNumManifold %d\n",gNumManifold);
	clearManifold(manifold);

	int findIndex = manifold->m_index1a;
	btAssert(findIndex < m_manifoldsPtr.size());
	m_manifoldsPtr.swap(findIndex,m_manifoldsPtr.size()-1);
	m_manifoldsPtr[findIndex]->m_index1a = findIndex;
	m_manifoldsPtr.pop_back();

	manifold->~btPersistentManifold();
	if (m_persistentManifoldPoolAllocator->validPtr(manifold))
	{
		m_persistentManifoldPoolAllocator->freeMemory(manifold);
	} else
	{
		btAlignedFree(manifold);
	}
	
}

	

btCollisionAlgorithm* btCollisionDispatcher::findAlgorithm(btCollisionObject* body0,btCollisionObject* body1,btPersistentManifold* sharedManifold)
{
	
	btCollisionAlgorithmConstructionInfo ci;

	ci.m_dispatcher1 = this;
	ci.m_manifold = sharedManifold;
	btCollisionAlgorithm* algo = m_doubleDispatch[body0->getCollisionShape()->getShapeType()][body1->getCollisionShape()->getShapeType()]->CreateCollisionAlgorithm(ci,body0,body1);

	return algo;
}




bool	btCollisionDispatcher::needsResponse(btCollisionObject* body0,btCollisionObject* body1)
{
	//here you can do filtering
	bool hasResponse = 
		(body0->hasContactResponse() && body1->hasContactResponse());
	//no response between two static/kinematic bodies:
	hasResponse = hasResponse &&
		((!body0->isStaticOrKinematicObject()) ||(! body1->isStaticOrKinematicObject()));
	return hasResponse;
}

bool	btCollisionDispatcher::needsCollision(btCollisionObject* body0,btCollisionObject* body1)
{
	btAssert(body0);
	btAssert(body1);

	bool needsCollision = true;

#ifdef BT_DEBUG
	if (!(m_dispatcherFlags & btCollisionDispatcher::CD_STATIC_STATIC_REPORTED))
	{
		//broadphase filtering already deals with this
		if (body0->isStaticOrKinematicObject() && body1->isStaticOrKinematicObject())
		{
			m_dispatcherFlags |= btCollisionDispatcher::CD_STATIC_STATIC_REPORTED;
			printf("warning btCollisionDispatcher::needsCollision: static-static collision!\n");
		}
	}
#endif //BT_DEBUG

	if ((!body0->isActive()) && (!body1->isActive()))
		needsCollision = false;
	else if (!body0->checkCollideWith(body1))
		needsCollision = false;
	
	return needsCollision ;

}



///interface for iterating all overlapping collision pairs, no matter how those pairs are stored (array, set, map etc)
///this is useful for the collision dispatcher.
class btCollisionPairCallback : public btOverlapCallback
{
	const btDispatcherInfo& m_dispatchInfo;
	btCollisionDispatcher*	m_dispatcher;

public:

	btCollisionPairCallback(const btDispatcherInfo& dispatchInfo,btCollisionDispatcher*	dispatcher)
	:m_dispatchInfo(dispatchInfo),
	m_dispatcher(dispatcher)
	{
	}

	/*btCollisionPairCallback& operator=(btCollisionPairCallback& other)
	{
		m_dispatchInfo = other.m_dispatchInfo;
		m_dispatcher = other.m_dispatcher;
		return *this;
	}
	*/


	virtual ~btCollisionPairCallback() {}


	virtual bool	processOverlap(btBroadphasePair& pair)
	{
		(*m_dispatcher->getNearCallback())(pair,*m_dispatcher,m_dispatchInfo);

		return false;
	}
};



void	btCollisionDispatcher::dispatchAllCollisionPairs(btOverlappingPairCache* pairCache,const btDispatcherInfo& dispatchInfo,btDispatcher* dispatcher) 
{
	//m_blockedForChanges = true;

	btCollisionPairCallback	collisionCallback(dispatchInfo,this);

	pairCache->processAllOverlappingPairs(&collisionCallback,dispatcher);

	//m_blockedForChanges = false;

}




//by default, Bullet will use this near callback
void btCollisionDispatcher::defaultNearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
		btCollisionObject* colObj0 = (btCollisionObject*)collisionPair.m_pProxy0->m_clientObject;
		btCollisionObject* colObj1 = (btCollisionObject*)collisionPair.m_pProxy1->m_clientObject;

		if (dispatcher.needsCollision(colObj0,colObj1))
		{
			//dispatcher will keep algorithms persistent in the collision pair
			if (!collisionPair.m_algorithm)
			{
				collisionPair.m_algorithm = dispatcher.findAlgorithm(colObj0,colObj1);
			}

			if (collisionPair.m_algorithm)
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


void* btCollisionDispatcher::allocateCollisionAlgorithm(int size)
{
	if (m_collisionAlgorithmPoolAllocator->getFreeCount())
	{
		return m_collisionAlgorithmPoolAllocator->allocate(size);
	}
	
	//warn user for overflow?
	return	btAlignedAlloc(static_cast<size_t>(size), 16);
}

void btCollisionDispatcher::freeCollisionAlgorithm(void* ptr)
{
	if (m_collisionAlgorithmPoolAllocator->validPtr(ptr))
	{
		m_collisionAlgorithmPoolAllocator->freeMemory(ptr);
	} else
	{
		btAlignedFree(ptr);
	}
}
