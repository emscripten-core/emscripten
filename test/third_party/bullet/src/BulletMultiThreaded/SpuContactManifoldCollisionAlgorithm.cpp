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

#include "SpuContactManifoldCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btPolyhedralConvexShape.h"




void SpuContactManifoldCollisionAlgorithm::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	btAssert(0);
}

btScalar SpuContactManifoldCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	btAssert(0);
	return 1.f;
}

#ifndef __SPU__
SpuContactManifoldCollisionAlgorithm::SpuContactManifoldCollisionAlgorithm(const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* body0,btCollisionObject* body1)
:btCollisionAlgorithm(ci)
#ifdef USE_SEPDISTANCE_UTIL
,m_sepDistance(body0->getCollisionShape()->getAngularMotionDisc(),body1->getCollisionShape()->getAngularMotionDisc())
#endif //USE_SEPDISTANCE_UTIL
{
	m_manifoldPtr = m_dispatcher->getNewManifold(body0,body1);
	m_shapeType0 = body0->getCollisionShape()->getShapeType();
	m_shapeType1 = body1->getCollisionShape()->getShapeType();
	m_collisionMargin0 = body0->getCollisionShape()->getMargin();
	m_collisionMargin1 = body1->getCollisionShape()->getMargin();
	m_collisionObject0 = body0;
	m_collisionObject1 = body1;

	if (body0->getCollisionShape()->isPolyhedral())
	{
		btPolyhedralConvexShape* convex0 = (btPolyhedralConvexShape*)body0->getCollisionShape();
		m_shapeDimensions0 = convex0->getImplicitShapeDimensions();
	}
	if (body1->getCollisionShape()->isPolyhedral())
	{
		btPolyhedralConvexShape* convex1 = (btPolyhedralConvexShape*)body1->getCollisionShape();
		m_shapeDimensions1 = convex1->getImplicitShapeDimensions();
	}
}
#endif //__SPU__


SpuContactManifoldCollisionAlgorithm::~SpuContactManifoldCollisionAlgorithm()
{
	if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
}
