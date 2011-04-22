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

#ifndef BT_SPU_CONTACTMANIFOLD_COLLISION_ALGORITHM_H
#define BT_SPU_CONTACTMANIFOLD_COLLISION_ALGORITHM_H

#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "BulletCollision/CollisionDispatch/btCollisionCreateFunc.h"
#include "BulletCollision/BroadphaseCollision/btDispatcher.h"
#include "LinearMath/btTransformUtil.h"

class btPersistentManifold;

//#define USE_SEPDISTANCE_UTIL 1

/// SpuContactManifoldCollisionAlgorithm  provides contact manifold and should be processed on SPU.
ATTRIBUTE_ALIGNED16(class) SpuContactManifoldCollisionAlgorithm : public btCollisionAlgorithm
{
	btVector3	m_shapeDimensions0;
	btVector3	m_shapeDimensions1;
	btPersistentManifold*	m_manifoldPtr;
	int		m_shapeType0;
	int		m_shapeType1;
	float	m_collisionMargin0;
	float	m_collisionMargin1;

	btCollisionObject*	m_collisionObject0;
	btCollisionObject*	m_collisionObject1;
	
	

	
public:
	
	virtual void processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

	virtual btScalar calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut);

	
	SpuContactManifoldCollisionAlgorithm(const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* body0,btCollisionObject* body1);
#ifdef USE_SEPDISTANCE_UTIL
	btConvexSeparatingDistanceUtil	m_sepDistance;
#endif //USE_SEPDISTANCE_UTIL

	virtual ~SpuContactManifoldCollisionAlgorithm();

	virtual	void	getAllContactManifolds(btManifoldArray&	manifoldArray)
	{
		if (m_manifoldPtr)
			manifoldArray.push_back(m_manifoldPtr);
	}

	btPersistentManifold*	getContactManifoldPtr()
	{
		return m_manifoldPtr;
	}

	btCollisionObject*	getCollisionObject0()
	{
		return m_collisionObject0;
	}
	
	btCollisionObject*	getCollisionObject1()
	{
		return m_collisionObject1;
	}

	int		getShapeType0() const
	{
		return m_shapeType0;
	}

	int		getShapeType1() const
	{
		return m_shapeType1;
	}
	float	getCollisionMargin0() const
	{
		return m_collisionMargin0;
	}
	float	getCollisionMargin1() const
	{
		return m_collisionMargin1;
	}

	const btVector3&	getShapeDimensions0() const
	{
		return m_shapeDimensions0;
	}

	const btVector3&	getShapeDimensions1() const
	{
		return m_shapeDimensions1;
	}

	struct CreateFunc :public 	btCollisionAlgorithmCreateFunc
	{
		virtual	btCollisionAlgorithm* CreateCollisionAlgorithm(btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* body0,btCollisionObject* body1)
		{
			void* mem = ci.m_dispatcher1->allocateCollisionAlgorithm(sizeof(SpuContactManifoldCollisionAlgorithm));
			return new(mem) SpuContactManifoldCollisionAlgorithm(ci,body0,body1);
		}
	};

};

#endif //BT_SPU_CONTACTMANIFOLD_COLLISION_ALGORITHM_H
