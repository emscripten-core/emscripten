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

#include "btBoxBoxCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "btBoxBoxDetector.h"

#define USE_PERSISTENT_CONTACTS 1

btBoxBoxCollisionAlgorithm::btBoxBoxCollisionAlgorithm(btPersistentManifold* mf,const btCollisionAlgorithmConstructionInfo& ci,btCollisionObject* obj0,btCollisionObject* obj1)
: btActivatingCollisionAlgorithm(ci,obj0,obj1),
m_ownManifold(false),
m_manifoldPtr(mf)
{
	if (!m_manifoldPtr && m_dispatcher->needsCollision(obj0,obj1))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(obj0,obj1);
		m_ownManifold = true;
	}
}

btBoxBoxCollisionAlgorithm::~btBoxBoxCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void btBoxBoxCollisionAlgorithm::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	if (!m_manifoldPtr)
		return;

	btCollisionObject*	col0 = body0;
	btCollisionObject*	col1 = body1;
	btBoxShape* box0 = (btBoxShape*)col0->getCollisionShape();
	btBoxShape* box1 = (btBoxShape*)col1->getCollisionShape();



	/// report a contact. internally this will be kept persistent, and contact reduction is done
	resultOut->setPersistentManifold(m_manifoldPtr);
#ifndef USE_PERSISTENT_CONTACTS	
	m_manifoldPtr->clearManifold();
#endif //USE_PERSISTENT_CONTACTS

	btDiscreteCollisionDetectorInterface::ClosestPointInput input;
	input.m_maximumDistanceSquared = BT_LARGE_FLOAT;
	input.m_transformA = body0->getWorldTransform();
	input.m_transformB = body1->getWorldTransform();

	btBoxBoxDetector detector(box0,box1);
	detector.getClosestPoints(input,*resultOut,dispatchInfo.m_debugDraw);

#ifdef USE_PERSISTENT_CONTACTS
	//  refreshContactPoints is only necessary when using persistent contact points. otherwise all points are newly added
	if (m_ownManifold)
	{
		resultOut->refreshContactPoints();
	}
#endif //USE_PERSISTENT_CONTACTS

}

btScalar btBoxBoxCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* /*body0*/,btCollisionObject* /*body1*/,const btDispatcherInfo& /*dispatchInfo*/,btManifoldResult* /*resultOut*/)
{
	//not yet
	return 1.f;
}
