/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "btDiscreteDynamicsWorld.h"

//collision detection
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/BroadphaseCollision/btSimpleBroadphase.h"
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"
#include "LinearMath/btTransformUtil.h"
#include "LinearMath/btQuickprof.h"

//rigidbody & constraints
#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletDynamics/ConstraintSolver/btContactSolverInfo.h"
#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
#include "BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h"
#include "BulletDynamics/ConstraintSolver/btHingeConstraint.h"
#include "BulletDynamics/ConstraintSolver/btConeTwistConstraint.h"
#include "BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h"
#include "BulletDynamics/ConstraintSolver/btSliderConstraint.h"
#include "BulletDynamics/ConstraintSolver/btContactConstraint.h"


#include "LinearMath/btIDebugDraw.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"


#include "BulletDynamics/Dynamics/btActionInterface.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btMotionState.h"

#include "LinearMath/btSerializer.h"

#if 0
btAlignedObjectArray<btVector3> debugContacts;
btAlignedObjectArray<btVector3> debugNormals;
int startHit=2;
int firstHit=startHit;
#endif



btDiscreteDynamicsWorld::btDiscreteDynamicsWorld(btDispatcher* dispatcher,btBroadphaseInterface* pairCache,btConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration)
:btDynamicsWorld(dispatcher,pairCache,collisionConfiguration),
m_constraintSolver(constraintSolver),
m_gravity(0,-10,0),
m_localTime(0),
m_synchronizeAllMotionStates(false),
m_profileTimings(0)
{
	if (!m_constraintSolver)
	{
		void* mem = btAlignedAlloc(sizeof(btSequentialImpulseConstraintSolver),16);
		m_constraintSolver = new (mem) btSequentialImpulseConstraintSolver;
		m_ownsConstraintSolver = true;
	} else
	{
		m_ownsConstraintSolver = false;
	}

	{
		void* mem = btAlignedAlloc(sizeof(btSimulationIslandManager),16);
		m_islandManager = new (mem) btSimulationIslandManager();
	}

	m_ownsIslandManager = true;
}


btDiscreteDynamicsWorld::~btDiscreteDynamicsWorld()
{
	//only delete it when we created it
	if (m_ownsIslandManager)
	{
		m_islandManager->~btSimulationIslandManager();
		btAlignedFree( m_islandManager);
	}
	if (m_ownsConstraintSolver)
	{

		m_constraintSolver->~btConstraintSolver();
		btAlignedFree(m_constraintSolver);
	}
}

void	btDiscreteDynamicsWorld::saveKinematicState(btScalar timeStep)
{
///would like to iterate over m_nonStaticRigidBodies, but unfortunately old API allows
///to switch status _after_ adding kinematic objects to the world
///fix it for Bullet 3.x release
	for (int i=0;i<m_collisionObjects.size();i++)
	{
		btCollisionObject* colObj = m_collisionObjects[i];
		btRigidBody* body = btRigidBody::upcast(colObj);
		if (body && body->getActivationState() != ISLAND_SLEEPING)
		{
			if (body->isKinematicObject())
			{
				//to calculate velocities next frame
				body->saveKinematicState(timeStep);
			}
		}
	}

}

void	btDiscreteDynamicsWorld::debugDrawWorld()
{
	BT_PROFILE("debugDrawWorld");

	btCollisionWorld::debugDrawWorld();

	bool drawConstraints = false;
	if (getDebugDrawer())
	{
		int mode = getDebugDrawer()->getDebugMode();
		if(mode  & (btIDebugDraw::DBG_DrawConstraints | btIDebugDraw::DBG_DrawConstraintLimits))
		{
			drawConstraints = true;
		}
	}
	if(drawConstraints)
	{
		for(int i = getNumConstraints()-1; i>=0 ;i--)
		{
			btTypedConstraint* constraint = getConstraint(i);
			debugDrawConstraint(constraint);
		}
	}



	if (getDebugDrawer() && getDebugDrawer()->getDebugMode() & (btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb))
	{
		int i;

		if (getDebugDrawer() && getDebugDrawer()->getDebugMode())
		{
			for (i=0;i<m_actions.size();i++)
			{
				m_actions[i]->debugDraw(m_debugDrawer);
			}
		}
	}
}

void	btDiscreteDynamicsWorld::clearForces()
{
	///@todo: iterate over awake simulation islands!
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		//need to check if next line is ok
		//it might break backward compatibility (people applying forces on sleeping objects get never cleared and accumulate on wake-up
		body->clearForces();
	}
}	

///apply gravity, call this once per timestep
void	btDiscreteDynamicsWorld::applyGravity()
{
	///@todo: iterate over awake simulation islands!
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		if (body->isActive())
		{
			body->applyGravity();
		}
	}
}


void	btDiscreteDynamicsWorld::synchronizeSingleMotionState(btRigidBody* body)
{
	btAssert(body);

	if (body->getMotionState() && !body->isStaticOrKinematicObject())
	{
		//we need to call the update at least once, even for sleeping objects
		//otherwise the 'graphics' transform never updates properly
		///@todo: add 'dirty' flag
		//if (body->getActivationState() != ISLAND_SLEEPING)
		{
			btTransform interpolatedTransform;
			btTransformUtil::integrateTransform(body->getInterpolationWorldTransform(),
				body->getInterpolationLinearVelocity(),body->getInterpolationAngularVelocity(),m_localTime*body->getHitFraction(),interpolatedTransform);
			body->getMotionState()->setWorldTransform(interpolatedTransform);
		}
	}
}


void	btDiscreteDynamicsWorld::synchronizeMotionStates()
{
	BT_PROFILE("synchronizeMotionStates");
	if (m_synchronizeAllMotionStates)
	{
		//iterate  over all collision objects
		for ( int i=0;i<m_collisionObjects.size();i++)
		{
			btCollisionObject* colObj = m_collisionObjects[i];
			btRigidBody* body = btRigidBody::upcast(colObj);
			if (body)
				synchronizeSingleMotionState(body);
		}
	} else
	{
		//iterate over all active rigid bodies
		for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
		{
			btRigidBody* body = m_nonStaticRigidBodies[i];
			if (body->isActive())
				synchronizeSingleMotionState(body);
		}
	}
}


int	btDiscreteDynamicsWorld::stepSimulation( btScalar timeStep,int maxSubSteps, btScalar fixedTimeStep)
{
	startProfiling(timeStep);

	BT_PROFILE("stepSimulation");

	int numSimulationSubSteps = 0;

	if (maxSubSteps)
	{
		//fixed timestep with interpolation
		m_localTime += timeStep;
		if (m_localTime >= fixedTimeStep)
		{
			numSimulationSubSteps = int( m_localTime / fixedTimeStep);
			m_localTime -= numSimulationSubSteps * fixedTimeStep;
		}
	} else
	{
		//variable timestep
		fixedTimeStep = timeStep;
		m_localTime = timeStep;
		if (btFuzzyZero(timeStep))
		{
			numSimulationSubSteps = 0;
			maxSubSteps = 0;
		} else
		{
			numSimulationSubSteps = 1;
			maxSubSteps = 1;
		}
	}

	//process some debugging flags
	if (getDebugDrawer())
	{
		btIDebugDraw* debugDrawer = getDebugDrawer ();
		gDisableDeactivation = (debugDrawer->getDebugMode() & btIDebugDraw::DBG_NoDeactivation) != 0;
	}
	if (numSimulationSubSteps)
	{

		//clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
		int clampedSimulationSteps = (numSimulationSubSteps > maxSubSteps)? maxSubSteps : numSimulationSubSteps;

		saveKinematicState(fixedTimeStep*clampedSimulationSteps);

		applyGravity();

		

		for (int i=0;i<clampedSimulationSteps;i++)
		{
			internalSingleStepSimulation(fixedTimeStep);
			synchronizeMotionStates();
		}

	} else
	{
		synchronizeMotionStates();
	}

	clearForces();

#ifndef BT_NO_PROFILE
	CProfileManager::Increment_Frame_Counter();
#endif //BT_NO_PROFILE
	
	return numSimulationSubSteps;
}

void	btDiscreteDynamicsWorld::internalSingleStepSimulation(btScalar timeStep)
{
	
	BT_PROFILE("internalSingleStepSimulation");

	if(0 != m_internalPreTickCallback) {
		(*m_internalPreTickCallback)(this, timeStep);
	}	

	///apply gravity, predict motion
	predictUnconstraintMotion(timeStep);

	btDispatcherInfo& dispatchInfo = getDispatchInfo();

	dispatchInfo.m_timeStep = timeStep;
	dispatchInfo.m_stepCount = 0;
	dispatchInfo.m_debugDraw = getDebugDrawer();


	///perform collision detection
	performDiscreteCollisionDetection();

	if (getDispatchInfo().m_useContinuous)
		addSpeculativeContacts(timeStep);


	calculateSimulationIslands();

	
	getSolverInfo().m_timeStep = timeStep;
	


	///solve contact and other joint constraints
	solveConstraints(getSolverInfo());
	
	///CallbackTriggers();

	///integrate transforms
	integrateTransforms(timeStep);

	///update vehicle simulation
	updateActions(timeStep);
	
	updateActivationState( timeStep );

	if(0 != m_internalTickCallback) {
		(*m_internalTickCallback)(this, timeStep);
	}	
}

void	btDiscreteDynamicsWorld::setGravity(const btVector3& gravity)
{
	m_gravity = gravity;
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		if (body->isActive() && !(body->getFlags() &BT_DISABLE_WORLD_GRAVITY))
		{
			body->setGravity(gravity);
		}
	}
}

btVector3 btDiscreteDynamicsWorld::getGravity () const
{
	return m_gravity;
}

void	btDiscreteDynamicsWorld::addCollisionObject(btCollisionObject* collisionObject,short int collisionFilterGroup,short int collisionFilterMask)
{
	btCollisionWorld::addCollisionObject(collisionObject,collisionFilterGroup,collisionFilterMask);
}

void	btDiscreteDynamicsWorld::removeCollisionObject(btCollisionObject* collisionObject)
{
	btRigidBody* body = btRigidBody::upcast(collisionObject);
	if (body)
		removeRigidBody(body);
	else
		btCollisionWorld::removeCollisionObject(collisionObject);
}

void	btDiscreteDynamicsWorld::removeRigidBody(btRigidBody* body)
{
	m_nonStaticRigidBodies.remove(body);
	btCollisionWorld::removeCollisionObject(body);
}


void	btDiscreteDynamicsWorld::addRigidBody(btRigidBody* body)
{
	if (!body->isStaticOrKinematicObject() && !(body->getFlags() &BT_DISABLE_WORLD_GRAVITY))
	{
		body->setGravity(m_gravity);
	}

	if (body->getCollisionShape())
	{
		if (!body->isStaticObject())
		{
			m_nonStaticRigidBodies.push_back(body);
		} else
		{
			body->setActivationState(ISLAND_SLEEPING);
		}

		bool isDynamic = !(body->isStaticObject() || body->isKinematicObject());
		short collisionFilterGroup = isDynamic? short(btBroadphaseProxy::DefaultFilter) : short(btBroadphaseProxy::StaticFilter);
		short collisionFilterMask = isDynamic? 	short(btBroadphaseProxy::AllFilter) : 	short(btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);

		addCollisionObject(body,collisionFilterGroup,collisionFilterMask);
	}
}

void	btDiscreteDynamicsWorld::addRigidBody(btRigidBody* body, short group, short mask)
{
	if (!body->isStaticOrKinematicObject() && !(body->getFlags() &BT_DISABLE_WORLD_GRAVITY))
	{
		body->setGravity(m_gravity);
	}

	if (body->getCollisionShape())
	{
		if (!body->isStaticObject())
		{
			m_nonStaticRigidBodies.push_back(body);
		}
		 else
		{
			body->setActivationState(ISLAND_SLEEPING);
		}
		addCollisionObject(body,group,mask);
	}
}


void	btDiscreteDynamicsWorld::updateActions(btScalar timeStep)
{
	BT_PROFILE("updateActions");
	
	for ( int i=0;i<m_actions.size();i++)
	{
		m_actions[i]->updateAction( this, timeStep);
	}
}
	
	
void	btDiscreteDynamicsWorld::updateActivationState(btScalar timeStep)
{
	BT_PROFILE("updateActivationState");

	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		if (body)
		{
			body->updateDeactivation(timeStep);

			if (body->wantsSleeping())
			{
				if (body->isStaticOrKinematicObject())
				{
					body->setActivationState(ISLAND_SLEEPING);
				} else
				{
					if (body->getActivationState() == ACTIVE_TAG)
						body->setActivationState( WANTS_DEACTIVATION );
					if (body->getActivationState() == ISLAND_SLEEPING) 
					{
						body->setAngularVelocity(btVector3(0,0,0));
						body->setLinearVelocity(btVector3(0,0,0));
					}

				}
			} else
			{
				if (body->getActivationState() != DISABLE_DEACTIVATION)
					body->setActivationState( ACTIVE_TAG );
			}
		}
	}
}

void	btDiscreteDynamicsWorld::addConstraint(btTypedConstraint* constraint,bool disableCollisionsBetweenLinkedBodies)
{
	m_constraints.push_back(constraint);
	if (disableCollisionsBetweenLinkedBodies)
	{
		constraint->getRigidBodyA().addConstraintRef(constraint);
		constraint->getRigidBodyB().addConstraintRef(constraint);
	}
}

void	btDiscreteDynamicsWorld::removeConstraint(btTypedConstraint* constraint)
{
	m_constraints.remove(constraint);
	constraint->getRigidBodyA().removeConstraintRef(constraint);
	constraint->getRigidBodyB().removeConstraintRef(constraint);
}

void	btDiscreteDynamicsWorld::addAction(btActionInterface* action)
{
	m_actions.push_back(action);
}

void	btDiscreteDynamicsWorld::removeAction(btActionInterface* action)
{
	m_actions.remove(action);
}


void	btDiscreteDynamicsWorld::addVehicle(btActionInterface* vehicle)
{
	addAction(vehicle);
}

void	btDiscreteDynamicsWorld::removeVehicle(btActionInterface* vehicle)
{
	removeAction(vehicle);
}

void	btDiscreteDynamicsWorld::addCharacter(btActionInterface* character)
{
	addAction(character);
}

void	btDiscreteDynamicsWorld::removeCharacter(btActionInterface* character)
{
	removeAction(character);
}


SIMD_FORCE_INLINE	int	btGetConstraintIslandId(const btTypedConstraint* lhs)
{
	int islandId;
	
	const btCollisionObject& rcolObj0 = lhs->getRigidBodyA();
	const btCollisionObject& rcolObj1 = lhs->getRigidBodyB();
	islandId= rcolObj0.getIslandTag()>=0?rcolObj0.getIslandTag():rcolObj1.getIslandTag();
	return islandId;

}


class btSortConstraintOnIslandPredicate
{
	public:

		bool operator() ( const btTypedConstraint* lhs, const btTypedConstraint* rhs )
		{
			int rIslandId0,lIslandId0;
			rIslandId0 = btGetConstraintIslandId(rhs);
			lIslandId0 = btGetConstraintIslandId(lhs);
			return lIslandId0 < rIslandId0;
		}
};



void	btDiscreteDynamicsWorld::solveConstraints(btContactSolverInfo& solverInfo)
{
	BT_PROFILE("solveConstraints");
	
	struct InplaceSolverIslandCallback : public btSimulationIslandManager::IslandCallback
	{

		btContactSolverInfo&	m_solverInfo;
		btConstraintSolver*		m_solver;
		btTypedConstraint**		m_sortedConstraints;
		int						m_numConstraints;
		btIDebugDraw*			m_debugDrawer;
		btStackAlloc*			m_stackAlloc;
		btDispatcher*			m_dispatcher;
		
		btAlignedObjectArray<btCollisionObject*> m_bodies;
		btAlignedObjectArray<btPersistentManifold*> m_manifolds;
		btAlignedObjectArray<btTypedConstraint*> m_constraints;


		InplaceSolverIslandCallback(
			btContactSolverInfo& solverInfo,
			btConstraintSolver*	solver,
			btTypedConstraint** sortedConstraints,
			int	numConstraints,
			btIDebugDraw*	debugDrawer,
			btStackAlloc*			stackAlloc,
			btDispatcher* dispatcher)
			:m_solverInfo(solverInfo),
			m_solver(solver),
			m_sortedConstraints(sortedConstraints),
			m_numConstraints(numConstraints),
			m_debugDrawer(debugDrawer),
			m_stackAlloc(stackAlloc),
			m_dispatcher(dispatcher)
		{

		}


		InplaceSolverIslandCallback& operator=(InplaceSolverIslandCallback& other)
		{
			btAssert(0);
			(void)other;
			return *this;
		}
		virtual	void	ProcessIsland(btCollisionObject** bodies,int numBodies,btPersistentManifold**	manifolds,int numManifolds, int islandId)
		{
			if (islandId<0)
			{
				if (numManifolds + m_numConstraints)
				{
					///we don't split islands, so all constraints/contact manifolds/bodies are passed into the solver regardless the island id
					m_solver->solveGroup( bodies,numBodies,manifolds, numManifolds,&m_sortedConstraints[0],m_numConstraints,m_solverInfo,m_debugDrawer,m_stackAlloc,m_dispatcher);
				}
			} else
			{
					//also add all non-contact constraints/joints for this island
				btTypedConstraint** startConstraint = 0;
				int numCurConstraints = 0;
				int i;
				
				//find the first constraint for this island
				for (i=0;i<m_numConstraints;i++)
				{
					if (btGetConstraintIslandId(m_sortedConstraints[i]) == islandId)
					{
						startConstraint = &m_sortedConstraints[i];
						break;
					}
				}
				//count the number of constraints in this island
				for (;i<m_numConstraints;i++)
				{
					if (btGetConstraintIslandId(m_sortedConstraints[i]) == islandId)
					{
						numCurConstraints++;
					}
				}

				if (m_solverInfo.m_minimumSolverBatchSize<=1)
				{
					///only call solveGroup if there is some work: avoid virtual function call, its overhead can be excessive
					if (numManifolds + numCurConstraints)
					{
						m_solver->solveGroup( bodies,numBodies,manifolds, numManifolds,startConstraint,numCurConstraints,m_solverInfo,m_debugDrawer,m_stackAlloc,m_dispatcher);
					}
				} else
				{
					
					for (i=0;i<numBodies;i++)
						m_bodies.push_back(bodies[i]);
					for (i=0;i<numManifolds;i++)
						m_manifolds.push_back(manifolds[i]);
					for (i=0;i<numCurConstraints;i++)
						m_constraints.push_back(startConstraint[i]);
					if ((m_constraints.size()+m_manifolds.size())>m_solverInfo.m_minimumSolverBatchSize)
					{
						processConstraints();
					} else
					{
						//printf("deferred\n");
					}
				}
			}
		}
		void	processConstraints()
		{
			if (m_manifolds.size() + m_constraints.size()>0)
			{
				m_solver->solveGroup( &m_bodies[0],m_bodies.size(), &m_manifolds[0], m_manifolds.size(), &m_constraints[0], m_constraints.size() ,m_solverInfo,m_debugDrawer,m_stackAlloc,m_dispatcher);
			}
			m_bodies.resize(0);
			m_manifolds.resize(0);
			m_constraints.resize(0);

		}

	};

	

	//sorted version of all btTypedConstraint, based on islandId
	btAlignedObjectArray<btTypedConstraint*>	sortedConstraints;
	sortedConstraints.resize( m_constraints.size());
	int i; 
	for (i=0;i<getNumConstraints();i++)
	{
		sortedConstraints[i] = m_constraints[i];
	}

//	btAssert(0);
		
	

	sortedConstraints.quickSort(btSortConstraintOnIslandPredicate());
	
	btTypedConstraint** constraintsPtr = getNumConstraints() ? &sortedConstraints[0] : 0;
	
	InplaceSolverIslandCallback	solverCallback(	solverInfo,	m_constraintSolver, constraintsPtr,sortedConstraints.size(),	m_debugDrawer,m_stackAlloc,m_dispatcher1);
	
	m_constraintSolver->prepareSolve(getCollisionWorld()->getNumCollisionObjects(), getCollisionWorld()->getDispatcher()->getNumManifolds());
	
	/// solve all the constraints for this island
	m_islandManager->buildAndProcessIslands(getCollisionWorld()->getDispatcher(),getCollisionWorld(),&solverCallback);

	solverCallback.processConstraints();

	m_constraintSolver->allSolved(solverInfo, m_debugDrawer, m_stackAlloc);
}




void	btDiscreteDynamicsWorld::calculateSimulationIslands()
{
	BT_PROFILE("calculateSimulationIslands");

	getSimulationIslandManager()->updateActivationState(getCollisionWorld(),getCollisionWorld()->getDispatcher());

	{
		int i;
		int numConstraints = int(m_constraints.size());
		for (i=0;i< numConstraints ; i++ )
		{
			btTypedConstraint* constraint = m_constraints[i];

			const btRigidBody* colObj0 = &constraint->getRigidBodyA();
			const btRigidBody* colObj1 = &constraint->getRigidBodyB();

			if (((colObj0) && (!(colObj0)->isStaticOrKinematicObject())) &&
				((colObj1) && (!(colObj1)->isStaticOrKinematicObject())))
			{
				if (colObj0->isActive() || colObj1->isActive())
				{

					getSimulationIslandManager()->getUnionFind().unite((colObj0)->getIslandTag(),
						(colObj1)->getIslandTag());
				}
			}
		}
	}

	//Store the island id in each body
	getSimulationIslandManager()->storeIslandActivationState(getCollisionWorld());

	
}




class btClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:

	btCollisionObject* m_me;
	btScalar m_allowedPenetration;
	btOverlappingPairCache* m_pairCache;
	btDispatcher* m_dispatcher;

public:
	btClosestNotMeConvexResultCallback (btCollisionObject* me,const btVector3& fromA,const btVector3& toA,btOverlappingPairCache* pairCache,btDispatcher* dispatcher) : 
	  btCollisionWorld::ClosestConvexResultCallback(fromA,toA),
		m_me(me),
		m_allowedPenetration(0.0f),
		m_pairCache(pairCache),
		m_dispatcher(dispatcher)
	{
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult,bool normalInWorldSpace)
	{
		if (convexResult.m_hitCollisionObject == m_me)
			return 1.0f;

		//ignore result if there is no contact response
		if(!convexResult.m_hitCollisionObject->hasContactResponse())
			return 1.0f;

		btVector3 linVelA,linVelB;
		linVelA = m_convexToWorld-m_convexFromWorld;
		linVelB = btVector3(0,0,0);//toB.getOrigin()-fromB.getOrigin();

		btVector3 relativeVelocity = (linVelA-linVelB);
		//don't report time of impact for motion away from the contact normal (or causes minor penetration)
		if (convexResult.m_hitNormalLocal.dot(relativeVelocity)>=-m_allowedPenetration)
			return 1.f;

		return ClosestConvexResultCallback::addSingleResult (convexResult, normalInWorldSpace);
	}

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
	{
		//don't collide with itself
		if (proxy0->m_clientObject == m_me)
			return false;

		///don't do CCD when the collision filters are not matching
		if (!ClosestConvexResultCallback::needsCollision(proxy0))
			return false;

		btCollisionObject* otherObj = (btCollisionObject*) proxy0->m_clientObject;

		//call needsResponse, see http://code.google.com/p/bullet/issues/detail?id=179
		if (m_dispatcher->needsResponse(m_me,otherObj))
		{
#if 0
			///don't do CCD when there are already contact points (touching contact/penetration)
			btAlignedObjectArray<btPersistentManifold*> manifoldArray;
			btBroadphasePair* collisionPair = m_pairCache->findPair(m_me->getBroadphaseHandle(),proxy0);
			if (collisionPair)
			{
				if (collisionPair->m_algorithm)
				{
					manifoldArray.resize(0);
					collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
					for (int j=0;j<manifoldArray.size();j++)
					{
						btPersistentManifold* manifold = manifoldArray[j];
						if (manifold->getNumContacts()>0)
							return false;
					}
				}
			}
#endif
			return true;
		}

		return false;
	}


};

///internal debugging variable. this value shouldn't be too high
int gNumClampedCcdMotions=0;

void	btDiscreteDynamicsWorld::integrateTransforms(btScalar timeStep)
{
	BT_PROFILE("integrateTransforms");
	btTransform predictedTrans;
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		body->setHitFraction(1.f);

		if (body->isActive() && (!body->isStaticOrKinematicObject()))
		{

			body->predictIntegratedTransform(timeStep, predictedTrans);
			
			btScalar squareMotion = (predictedTrans.getOrigin()-body->getWorldTransform().getOrigin()).length2();

			

			if (getDispatchInfo().m_useContinuous && body->getCcdSquareMotionThreshold() && body->getCcdSquareMotionThreshold() < squareMotion)
			{
				BT_PROFILE("CCD motion clamping");
				if (body->getCollisionShape()->isConvex())
				{
					gNumClampedCcdMotions++;
#ifdef USE_STATIC_ONLY
					class StaticOnlyCallback : public btClosestNotMeConvexResultCallback
					{
					public:

						StaticOnlyCallback (btCollisionObject* me,const btVector3& fromA,const btVector3& toA,btOverlappingPairCache* pairCache,btDispatcher* dispatcher) : 
						  btClosestNotMeConvexResultCallback(me,fromA,toA,pairCache,dispatcher)
						{
						}

					  	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
						{
							btCollisionObject* otherObj = (btCollisionObject*) proxy0->m_clientObject;
							if (!otherObj->isStaticOrKinematicObject())
								return false;
							return btClosestNotMeConvexResultCallback::needsCollision(proxy0);
						}
					};

					StaticOnlyCallback sweepResults(body,body->getWorldTransform().getOrigin(),predictedTrans.getOrigin(),getBroadphase()->getOverlappingPairCache(),getDispatcher());
#else
					btClosestNotMeConvexResultCallback sweepResults(body,body->getWorldTransform().getOrigin(),predictedTrans.getOrigin(),getBroadphase()->getOverlappingPairCache(),getDispatcher());
#endif
					//btConvexShape* convexShape = static_cast<btConvexShape*>(body->getCollisionShape());
					btSphereShape tmpSphere(body->getCcdSweptSphereRadius());//btConvexShape* convexShape = static_cast<btConvexShape*>(body->getCollisionShape());
					sweepResults.m_allowedPenetration=getDispatchInfo().m_allowedCcdPenetration;

					sweepResults.m_collisionFilterGroup = body->getBroadphaseProxy()->m_collisionFilterGroup;
					sweepResults.m_collisionFilterMask  = body->getBroadphaseProxy()->m_collisionFilterMask;
					btTransform modifiedPredictedTrans = predictedTrans;
					modifiedPredictedTrans.setBasis(body->getWorldTransform().getBasis());

					convexSweepTest(&tmpSphere,body->getWorldTransform(),modifiedPredictedTrans,sweepResults);
					if (sweepResults.hasHit() && (sweepResults.m_closestHitFraction < 1.f))
					{
						
						//printf("clamped integration to hit fraction = %f\n",fraction);
						body->setHitFraction(sweepResults.m_closestHitFraction);
						body->predictIntegratedTransform(timeStep*body->getHitFraction(), predictedTrans);
						body->setHitFraction(0.f);
						body->proceedToTransform( predictedTrans);

#if 0
						btVector3 linVel = body->getLinearVelocity();

						btScalar maxSpeed = body->getCcdMotionThreshold()/getSolverInfo().m_timeStep;
						btScalar maxSpeedSqr = maxSpeed*maxSpeed;
						if (linVel.length2()>maxSpeedSqr)
						{
							linVel.normalize();
							linVel*= maxSpeed;
							body->setLinearVelocity(linVel);
							btScalar ms2 = body->getLinearVelocity().length2();
							body->predictIntegratedTransform(timeStep, predictedTrans);

							btScalar sm2 = (predictedTrans.getOrigin()-body->getWorldTransform().getOrigin()).length2();
							btScalar smt = body->getCcdSquareMotionThreshold();
							printf("sm2=%f\n",sm2);
						}
#else
						//response  between two dynamic objects without friction, assuming 0 penetration depth
						btScalar appliedImpulse = 0.f;
						btScalar depth = 0.f;
						appliedImpulse = resolveSingleCollision(body,sweepResults.m_hitCollisionObject,sweepResults.m_hitPointWorld,sweepResults.m_hitNormalWorld,getSolverInfo(), depth);
						

#endif

        				continue;
					}
				}
			}
			

			body->proceedToTransform( predictedTrans);
		}
	}
}

void	btDiscreteDynamicsWorld::addSpeculativeContacts(btScalar timeStep)
{
	BT_PROFILE("addSpeculativeContacts");
	btTransform predictedTrans;
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		body->setHitFraction(1.f);

		if (body->isActive() && (!body->isStaticOrKinematicObject()))
		{
			body->predictIntegratedTransform(timeStep, predictedTrans);
			btScalar squareMotion = (predictedTrans.getOrigin()-body->getWorldTransform().getOrigin()).length2();

			if (body->getCcdSquareMotionThreshold() && body->getCcdSquareMotionThreshold() < squareMotion)
			{
				BT_PROFILE("search speculative contacts");
				if (body->getCollisionShape()->isConvex())
				{
					gNumClampedCcdMotions++;
					
					btClosestNotMeConvexResultCallback sweepResults(body,body->getWorldTransform().getOrigin(),predictedTrans.getOrigin(),getBroadphase()->getOverlappingPairCache(),getDispatcher());
					//btConvexShape* convexShape = static_cast<btConvexShape*>(body->getCollisionShape());
					btSphereShape tmpSphere(body->getCcdSweptSphereRadius());//btConvexShape* convexShape = static_cast<btConvexShape*>(body->getCollisionShape());

					sweepResults.m_collisionFilterGroup = body->getBroadphaseProxy()->m_collisionFilterGroup;
					sweepResults.m_collisionFilterMask  = body->getBroadphaseProxy()->m_collisionFilterMask;
					btTransform modifiedPredictedTrans;
					modifiedPredictedTrans = predictedTrans;
					modifiedPredictedTrans.setBasis(body->getWorldTransform().getBasis());

					convexSweepTest(&tmpSphere,body->getWorldTransform(),modifiedPredictedTrans,sweepResults);
					if (sweepResults.hasHit() && (sweepResults.m_closestHitFraction < 1.f))
					{
						btBroadphaseProxy* proxy0 = body->getBroadphaseHandle();
						btBroadphaseProxy* proxy1 = sweepResults.m_hitCollisionObject->getBroadphaseHandle();
						btBroadphasePair* pair = sweepResults.m_pairCache->findPair(proxy0,proxy1);
						if (pair)
						{
							if (pair->m_algorithm)
							{
								btManifoldArray contacts;
								pair->m_algorithm->getAllContactManifolds(contacts);
								if (contacts.size())
								{
									btManifoldResult result(body,sweepResults.m_hitCollisionObject);
									result.setPersistentManifold(contacts[0]);

									btVector3 vec = (modifiedPredictedTrans.getOrigin()-body->getWorldTransform().getOrigin());
									vec*=sweepResults.m_closestHitFraction;
									
									btScalar lenSqr = vec.length2();
									btScalar depth = 0.f;
									btVector3 pointWorld = sweepResults.m_hitPointWorld;
									if (lenSqr>SIMD_EPSILON)
									{
										depth = btSqrt(lenSqr);
										pointWorld -= vec;
										vec /= depth;
									}

									if (contacts[0]->getBody0()==body)
									{
										result.addContactPoint(sweepResults.m_hitNormalWorld,pointWorld,depth);
#if 0
										debugContacts.push_back(sweepResults.m_hitPointWorld);//sweepResults.m_hitPointWorld);
										debugNormals.push_back(sweepResults.m_hitNormalWorld);
#endif
									} else
									{
										//swapped
										result.addContactPoint(-sweepResults.m_hitNormalWorld,pointWorld,depth);
										//sweepResults.m_hitPointWorld,depth);
										
#if 0
										if (1)//firstHit==1)
										{
											firstHit=0;
											debugNormals.push_back(sweepResults.m_hitNormalWorld);
											debugContacts.push_back(pointWorld);//sweepResults.m_hitPointWorld);
											debugNormals.push_back(sweepResults.m_hitNormalWorld);
											debugContacts.push_back(sweepResults.m_hitPointWorld);
										}
										firstHit--;
#endif
									}
								}

							} else
							{
								//no algorithm, use dispatcher to create one

							}


						} else
						{
							//add an overlapping pair
							//printf("pair missing\n");

						}
					}
				}
			}
			
		}
	}
}





void	btDiscreteDynamicsWorld::predictUnconstraintMotion(btScalar timeStep)
{
	BT_PROFILE("predictUnconstraintMotion");
	for ( int i=0;i<m_nonStaticRigidBodies.size();i++)
	{
		btRigidBody* body = m_nonStaticRigidBodies[i];
		if (!body->isStaticOrKinematicObject())
		{
			body->integrateVelocities( timeStep);
			//damping
			body->applyDamping(timeStep);

			body->predictIntegratedTransform(timeStep,body->getInterpolationWorldTransform());
		}
	}
}


void	btDiscreteDynamicsWorld::startProfiling(btScalar timeStep)
{
	(void)timeStep;

#ifndef BT_NO_PROFILE
	CProfileManager::Reset();
#endif //BT_NO_PROFILE

}




	

void btDiscreteDynamicsWorld::debugDrawConstraint(btTypedConstraint* constraint)
{
	bool drawFrames = (getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_DrawConstraints) != 0;
	bool drawLimits = (getDebugDrawer()->getDebugMode() & btIDebugDraw::DBG_DrawConstraintLimits) != 0;
	btScalar dbgDrawSize = constraint->getDbgDrawSize();
	if(dbgDrawSize <= btScalar(0.f))
	{
		return;
	}

	switch(constraint->getConstraintType())
	{
		case POINT2POINT_CONSTRAINT_TYPE:
			{
				btPoint2PointConstraint* p2pC = (btPoint2PointConstraint*)constraint;
				btTransform tr;
				tr.setIdentity();
				btVector3 pivot = p2pC->getPivotInA();
				pivot = p2pC->getRigidBodyA().getCenterOfMassTransform() * pivot; 
				tr.setOrigin(pivot);
				getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				// that ideally should draw the same frame	
				pivot = p2pC->getPivotInB();
				pivot = p2pC->getRigidBodyB().getCenterOfMassTransform() * pivot; 
				tr.setOrigin(pivot);
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
			}
			break;
		case HINGE_CONSTRAINT_TYPE:
			{
				btHingeConstraint* pHinge = (btHingeConstraint*)constraint;
				btTransform tr = pHinge->getRigidBodyA().getCenterOfMassTransform() * pHinge->getAFrame();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				tr = pHinge->getRigidBodyB().getCenterOfMassTransform() * pHinge->getBFrame();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				btScalar minAng = pHinge->getLowerLimit();
				btScalar maxAng = pHinge->getUpperLimit();
				if(minAng == maxAng)
				{
					break;
				}
				bool drawSect = true;
				if(minAng > maxAng)
				{
					minAng = btScalar(0.f);
					maxAng = SIMD_2_PI;
					drawSect = false;
				}
				if(drawLimits) 
				{
					btVector3& center = tr.getOrigin();
					btVector3 normal = tr.getBasis().getColumn(2);
					btVector3 axis = tr.getBasis().getColumn(0);
					getDebugDrawer()->drawArc(center, normal, axis, dbgDrawSize, dbgDrawSize, minAng, maxAng, btVector3(0,0,0), drawSect);
				}
			}
			break;
		case CONETWIST_CONSTRAINT_TYPE:
			{
				btConeTwistConstraint* pCT = (btConeTwistConstraint*)constraint;
				btTransform tr = pCT->getRigidBodyA().getCenterOfMassTransform() * pCT->getAFrame();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				tr = pCT->getRigidBodyB().getCenterOfMassTransform() * pCT->getBFrame();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				if(drawLimits)
				{
					//const btScalar length = btScalar(5);
					const btScalar length = dbgDrawSize;
					static int nSegments = 8*4;
					btScalar fAngleInRadians = btScalar(2.*3.1415926) * (btScalar)(nSegments-1)/btScalar(nSegments);
					btVector3 pPrev = pCT->GetPointForAngle(fAngleInRadians, length);
					pPrev = tr * pPrev;
					for (int i=0; i<nSegments; i++)
					{
						fAngleInRadians = btScalar(2.*3.1415926) * (btScalar)i/btScalar(nSegments);
						btVector3 pCur = pCT->GetPointForAngle(fAngleInRadians, length);
						pCur = tr * pCur;
						getDebugDrawer()->drawLine(pPrev, pCur, btVector3(0,0,0));

						if (i%(nSegments/8) == 0)
							getDebugDrawer()->drawLine(tr.getOrigin(), pCur, btVector3(0,0,0));

						pPrev = pCur;
					}						
					btScalar tws = pCT->getTwistSpan();
					btScalar twa = pCT->getTwistAngle();
					bool useFrameB = (pCT->getRigidBodyB().getInvMass() > btScalar(0.f));
					if(useFrameB)
					{
						tr = pCT->getRigidBodyB().getCenterOfMassTransform() * pCT->getBFrame();
					}
					else
					{
						tr = pCT->getRigidBodyA().getCenterOfMassTransform() * pCT->getAFrame();
					}
					btVector3 pivot = tr.getOrigin();
					btVector3 normal = tr.getBasis().getColumn(0);
					btVector3 axis1 = tr.getBasis().getColumn(1);
					getDebugDrawer()->drawArc(pivot, normal, axis1, dbgDrawSize, dbgDrawSize, -twa-tws, -twa+tws, btVector3(0,0,0), true);

				}
			}
			break;
		case D6_SPRING_CONSTRAINT_TYPE:
		case D6_CONSTRAINT_TYPE:
			{
				btGeneric6DofConstraint* p6DOF = (btGeneric6DofConstraint*)constraint;
				btTransform tr = p6DOF->getCalculatedTransformA();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				tr = p6DOF->getCalculatedTransformB();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				if(drawLimits) 
				{
					tr = p6DOF->getCalculatedTransformA();
					const btVector3& center = p6DOF->getCalculatedTransformB().getOrigin();
					btVector3 up = tr.getBasis().getColumn(2);
					btVector3 axis = tr.getBasis().getColumn(0);
					btScalar minTh = p6DOF->getRotationalLimitMotor(1)->m_loLimit;
					btScalar maxTh = p6DOF->getRotationalLimitMotor(1)->m_hiLimit;
					btScalar minPs = p6DOF->getRotationalLimitMotor(2)->m_loLimit;
					btScalar maxPs = p6DOF->getRotationalLimitMotor(2)->m_hiLimit;
					getDebugDrawer()->drawSpherePatch(center, up, axis, dbgDrawSize * btScalar(.9f), minTh, maxTh, minPs, maxPs, btVector3(0,0,0));
					axis = tr.getBasis().getColumn(1);
					btScalar ay = p6DOF->getAngle(1);
					btScalar az = p6DOF->getAngle(2);
					btScalar cy = btCos(ay);
					btScalar sy = btSin(ay);
					btScalar cz = btCos(az);
					btScalar sz = btSin(az);
					btVector3 ref;
					ref[0] = cy*cz*axis[0] + cy*sz*axis[1] - sy*axis[2];
					ref[1] = -sz*axis[0] + cz*axis[1];
					ref[2] = cz*sy*axis[0] + sz*sy*axis[1] + cy*axis[2];
					tr = p6DOF->getCalculatedTransformB();
					btVector3 normal = -tr.getBasis().getColumn(0);
					btScalar minFi = p6DOF->getRotationalLimitMotor(0)->m_loLimit;
					btScalar maxFi = p6DOF->getRotationalLimitMotor(0)->m_hiLimit;
					if(minFi > maxFi)
					{
						getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, -SIMD_PI, SIMD_PI, btVector3(0,0,0), false);
					}
					else if(minFi < maxFi)
					{
						getDebugDrawer()->drawArc(center, normal, ref, dbgDrawSize, dbgDrawSize, minFi, maxFi, btVector3(0,0,0), true);
					}
					tr = p6DOF->getCalculatedTransformA();
					btVector3 bbMin = p6DOF->getTranslationalLimitMotor()->m_lowerLimit;
					btVector3 bbMax = p6DOF->getTranslationalLimitMotor()->m_upperLimit;
					getDebugDrawer()->drawBox(bbMin, bbMax, tr, btVector3(0,0,0));
				}
			}
			break;
		case SLIDER_CONSTRAINT_TYPE:
			{
				btSliderConstraint* pSlider = (btSliderConstraint*)constraint;
				btTransform tr = pSlider->getCalculatedTransformA();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				tr = pSlider->getCalculatedTransformB();
				if(drawFrames) getDebugDrawer()->drawTransform(tr, dbgDrawSize);
				if(drawLimits)
				{
					btTransform tr = pSlider->getUseLinearReferenceFrameA() ? pSlider->getCalculatedTransformA() : pSlider->getCalculatedTransformB();
					btVector3 li_min = tr * btVector3(pSlider->getLowerLinLimit(), 0.f, 0.f);
					btVector3 li_max = tr * btVector3(pSlider->getUpperLinLimit(), 0.f, 0.f);
					getDebugDrawer()->drawLine(li_min, li_max, btVector3(0, 0, 0));
					btVector3 normal = tr.getBasis().getColumn(0);
					btVector3 axis = tr.getBasis().getColumn(1);
					btScalar a_min = pSlider->getLowerAngLimit();
					btScalar a_max = pSlider->getUpperAngLimit();
					const btVector3& center = pSlider->getCalculatedTransformB().getOrigin();
					getDebugDrawer()->drawArc(center, normal, axis, dbgDrawSize, dbgDrawSize, a_min, a_max, btVector3(0,0,0), true);
				}
			}
			break;
		default : 
			break;
	}
	return;
}





void	btDiscreteDynamicsWorld::setConstraintSolver(btConstraintSolver* solver)
{
	if (m_ownsConstraintSolver)
	{
		btAlignedFree( m_constraintSolver);
	}
	m_ownsConstraintSolver = false;
	m_constraintSolver = solver;
}

btConstraintSolver* btDiscreteDynamicsWorld::getConstraintSolver()
{
	return m_constraintSolver;
}


int		btDiscreteDynamicsWorld::getNumConstraints() const
{
	return int(m_constraints.size());
}
btTypedConstraint* btDiscreteDynamicsWorld::getConstraint(int index)
{
	return m_constraints[index];
}
const btTypedConstraint* btDiscreteDynamicsWorld::getConstraint(int index) const
{
	return m_constraints[index];
}



void	btDiscreteDynamicsWorld::serializeRigidBodies(btSerializer* serializer)
{
	int i;
	//serialize all collision objects
	for (i=0;i<m_collisionObjects.size();i++)
	{
		btCollisionObject* colObj = m_collisionObjects[i];
		if (colObj->getInternalType() & btCollisionObject::CO_RIGID_BODY)
		{
			int len = colObj->calculateSerializeBufferSize();
			btChunk* chunk = serializer->allocate(len,1);
			const char* structType = colObj->serialize(chunk->m_oldPtr, serializer);
			serializer->finalizeChunk(chunk,structType,BT_RIGIDBODY_CODE,colObj);
		}
	}

	for (i=0;i<m_constraints.size();i++)
	{
		btTypedConstraint* constraint = m_constraints[i];
		int size = constraint->calculateSerializeBufferSize();
		btChunk* chunk = serializer->allocate(size,1);
		const char* structType = constraint->serialize(chunk->m_oldPtr,serializer);
		serializer->finalizeChunk(chunk,structType,BT_CONSTRAINT_CODE,constraint);
	}
}


void	btDiscreteDynamicsWorld::serialize(btSerializer* serializer)
{

	serializer->startSerialization();

	serializeRigidBodies(serializer);

	serializeCollisionObjects(serializer);

	serializer->finishSerialization();
}

