/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "btContinuousDynamicsWorld.h"
#include "LinearMath/btQuickprof.h"

//collision detection
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/BroadphaseCollision/btSimpleBroadphase.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"

//rigidbody & constraints
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/ConstraintSolver/btContactSolverInfo.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"



#include <stdio.h>

btContinuousDynamicsWorld::btContinuousDynamicsWorld(btDispatcher* dispatcher,btBroadphaseInterface* pairCache,btConstraintSolver* constraintSolver,btCollisionConfiguration* collisionConfiguration)
:btDiscreteDynamicsWorld(dispatcher,pairCache,constraintSolver,collisionConfiguration)
{
}

btContinuousDynamicsWorld::~btContinuousDynamicsWorld()
{
}

	
void	btContinuousDynamicsWorld::internalSingleStepSimulation( btScalar timeStep)
{
	
	startProfiling(timeStep);
	
	if(0 != m_internalPreTickCallback) {
		(*m_internalPreTickCallback)(this, timeStep);
	}


	///update aabbs information
	updateAabbs();
	//static int frame=0;
//	printf("frame %d\n",frame++);

	///apply gravity, predict motion
	predictUnconstraintMotion(timeStep);

	btDispatcherInfo& dispatchInfo = getDispatchInfo();

	dispatchInfo.m_timeStep = timeStep;
	dispatchInfo.m_stepCount = 0;
	dispatchInfo.m_debugDraw = getDebugDrawer();

	///perform collision detection
	performDiscreteCollisionDetection();

	calculateSimulationIslands();

	
	getSolverInfo().m_timeStep = timeStep;
	


	///solve contact and other joint constraints
	solveConstraints(getSolverInfo());
	
	///CallbackTriggers();
	calculateTimeOfImpacts(timeStep);

	btScalar toi = dispatchInfo.m_timeOfImpact;
//	if (toi < 1.f)
//		printf("toi = %f\n",toi);
	if (toi < 0.f)
		printf("toi = %f\n",toi);


	///integrate transforms
	integrateTransforms(timeStep * toi);

	///update vehicle simulation
	updateActions(timeStep);

	updateActivationState( timeStep );
	
	if(0 != m_internalTickCallback) {
		(*m_internalTickCallback)(this, timeStep);
	}
}

void	btContinuousDynamicsWorld::calculateTimeOfImpacts(btScalar timeStep)
{
		///these should be 'temporal' aabbs!
		updateTemporalAabbs(timeStep);
		
		///'toi' is the global smallest time of impact. However, we just calculate the time of impact for each object individually.
		///so we handle the case moving versus static properly, and we cheat for moving versus moving
		btScalar toi = 1.f;
		
	
		btDispatcherInfo& dispatchInfo = getDispatchInfo();
		dispatchInfo.m_timeStep = timeStep;
		dispatchInfo.m_timeOfImpact = 1.f;
		dispatchInfo.m_stepCount = 0;
		dispatchInfo.m_dispatchFunc = btDispatcherInfo::DISPATCH_CONTINUOUS;

		///calculate time of impact for overlapping pairs


		btDispatcher* dispatcher = getDispatcher();
		if (dispatcher)
			dispatcher->dispatchAllCollisionPairs(m_broadphasePairCache->getOverlappingPairCache(),dispatchInfo,m_dispatcher1);

		toi = dispatchInfo.m_timeOfImpact;

		dispatchInfo.m_dispatchFunc = btDispatcherInfo::DISPATCH_DISCRETE;

}

void	btContinuousDynamicsWorld::updateTemporalAabbs(btScalar timeStep)
{

	btVector3 temporalAabbMin,temporalAabbMax;

	for ( int i=0;i<m_collisionObjects.size();i++)
	{
		btCollisionObject* colObj = m_collisionObjects[i];
		
		btRigidBody* body = btRigidBody::upcast(colObj);
		if (body)
		{
			body->getCollisionShape()->getAabb(m_collisionObjects[i]->getWorldTransform(),temporalAabbMin,temporalAabbMax);
			const btVector3& linvel = body->getLinearVelocity();

			//make the AABB temporal
			btScalar temporalAabbMaxx = temporalAabbMax.getX();
			btScalar temporalAabbMaxy = temporalAabbMax.getY();
			btScalar temporalAabbMaxz = temporalAabbMax.getZ();
			btScalar temporalAabbMinx = temporalAabbMin.getX();
			btScalar temporalAabbMiny = temporalAabbMin.getY();
			btScalar temporalAabbMinz = temporalAabbMin.getZ();

			// add linear motion
			btVector3 linMotion = linvel*timeStep;
		
			if (linMotion.x() > 0.f)
				temporalAabbMaxx += linMotion.x(); 
			else
				temporalAabbMinx += linMotion.x();
			if (linMotion.y() > 0.f)
				temporalAabbMaxy += linMotion.y(); 
			else
				temporalAabbMiny += linMotion.y();
			if (linMotion.z() > 0.f)
				temporalAabbMaxz += linMotion.z(); 
			else
				temporalAabbMinz += linMotion.z();

			//add conservative angular motion
			btScalar angularMotion(0);// = angvel.length() * GetAngularMotionDisc() * timeStep;
			btVector3 angularMotion3d(angularMotion,angularMotion,angularMotion);
			temporalAabbMin = btVector3(temporalAabbMinx,temporalAabbMiny,temporalAabbMinz);
			temporalAabbMax = btVector3(temporalAabbMaxx,temporalAabbMaxy,temporalAabbMaxz);

			temporalAabbMin -= angularMotion3d;
			temporalAabbMax += angularMotion3d;

			m_broadphasePairCache->setAabb(body->getBroadphaseHandle(),temporalAabbMin,temporalAabbMax,m_dispatcher1);
		}
	}

	//update aabb (of all moved objects)

	m_broadphasePairCache->calculateOverlappingPairs(m_dispatcher1);
	


}



