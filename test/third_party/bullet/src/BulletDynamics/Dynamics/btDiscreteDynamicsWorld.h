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


#ifndef BT_DISCRETE_DYNAMICS_WORLD_H
#define BT_DISCRETE_DYNAMICS_WORLD_H

#include "btDynamicsWorld.h"

class btDispatcher;
class btOverlappingPairCache;
class btConstraintSolver;
class btSimulationIslandManager;
class btTypedConstraint;
class btActionInterface;

class btIDebugDraw;
#include "LinearMath/btAlignedObjectArray.h"


///btDiscreteDynamicsWorld provides discrete rigid body simulation
///those classes replace the obsolete CcdPhysicsEnvironment/CcdPhysicsController
class btDiscreteDynamicsWorld : public btDynamicsWorld
{
protected:

	btConstraintSolver*	m_constraintSolver;

	btSimulationIslandManager*	m_islandManager;

	btAlignedObjectArray<btTypedConstraint*> m_constraints;

	btAlignedObjectArray<btRigidBody*> m_nonStaticRigidBodies;

	btVector3	m_gravity;

	//for variable timesteps
	btScalar	m_localTime;
	//for variable timesteps

	bool	m_ownsIslandManager;
	bool	m_ownsConstraintSolver;
	bool	m_synchronizeAllMotionStates;

	btAlignedObjectArray<btActionInterface*>	m_actions;
	
	int	m_profileTimings;

	virtual void	predictUnconstraintMotion(btScalar timeStep);
	
	virtual void	integrateTransforms(btScalar timeStep);
		
	virtual void	addSpeculativeContacts(btScalar timeStep);

	virtual void	calculateSimulationIslands();

	virtual void	solveConstraints(btContactSolverInfo& solverInfo);
	
	void	updateActivationState(btScalar timeStep);

	void	updateActions(btScalar timeStep);

	void	startProfiling(btScalar timeStep);

	virtual void	internalSingleStepSimulation( btScalar timeStep);


	virtual void	saveKinematicState(btScalar timeStep);

	void	serializeRigidBodies(btSerializer* serializer);

public:


	///this btDiscreteDynamicsWorld constructor gets created objects from the user, and will not delete those
	btDiscreteDynamicsWorld(btDispatcher* dispatcher,btBroadphaseInterface* pairCache,btConstraintSolver* constraintSolver,btCollisionConfiguration* collisionConfiguration);

	virtual ~btDiscreteDynamicsWorld();

	///if maxSubSteps > 0, it will interpolate motion between fixedTimeStep's
	virtual int	stepSimulation( btScalar timeStep,int maxSubSteps=1, btScalar fixedTimeStep=btScalar(1.)/btScalar(60.));


	virtual void	synchronizeMotionStates();

	///this can be useful to synchronize a single rigid body -> graphics object
	void	synchronizeSingleMotionState(btRigidBody* body);

	virtual void	addConstraint(btTypedConstraint* constraint, bool disableCollisionsBetweenLinkedBodies=false);

	virtual void	removeConstraint(btTypedConstraint* constraint);

	virtual void	addAction(btActionInterface*);

	virtual void	removeAction(btActionInterface*);
	
	btSimulationIslandManager*	getSimulationIslandManager()
	{
		return m_islandManager;
	}

	const btSimulationIslandManager*	getSimulationIslandManager() const 
	{
		return m_islandManager;
	}

	btCollisionWorld*	getCollisionWorld()
	{
		return this;
	}

	virtual void	setGravity(const btVector3& gravity);

	virtual btVector3 getGravity () const;

	virtual void	addCollisionObject(btCollisionObject* collisionObject,short int collisionFilterGroup=btBroadphaseProxy::StaticFilter,short int collisionFilterMask=btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);

	virtual void	addRigidBody(btRigidBody* body);

	virtual void	addRigidBody(btRigidBody* body, short group, short mask);

	virtual void	removeRigidBody(btRigidBody* body);

	///removeCollisionObject will first check if it is a rigid body, if so call removeRigidBody otherwise call btCollisionWorld::removeCollisionObject
	virtual void	removeCollisionObject(btCollisionObject* collisionObject);


	void	debugDrawConstraint(btTypedConstraint* constraint);

	virtual void	debugDrawWorld();

	virtual void	setConstraintSolver(btConstraintSolver* solver);

	virtual btConstraintSolver* getConstraintSolver();
	
	virtual	int		getNumConstraints() const;

	virtual btTypedConstraint* getConstraint(int index)	;

	virtual const btTypedConstraint* getConstraint(int index) const;

	
	virtual btDynamicsWorldType	getWorldType() const
	{
		return BT_DISCRETE_DYNAMICS_WORLD;
	}
	
	///the forces on each rigidbody is accumulating together with gravity. clear this after each timestep.
	virtual void	clearForces();

	///apply gravity, call this once per timestep
	virtual void	applyGravity();

	virtual void	setNumTasks(int numTasks)
	{
        (void) numTasks;
	}

	///obsolete, use updateActions instead
	virtual void updateVehicles(btScalar timeStep)
	{
		updateActions(timeStep);
	}

	///obsolete, use addAction instead
	virtual void	addVehicle(btActionInterface* vehicle);
	///obsolete, use removeAction instead
	virtual void	removeVehicle(btActionInterface* vehicle);
	///obsolete, use addAction instead
	virtual void	addCharacter(btActionInterface* character);
	///obsolete, use removeAction instead
	virtual void	removeCharacter(btActionInterface* character);

	void	setSynchronizeAllMotionStates(bool synchronizeAll)
	{
		m_synchronizeAllMotionStates = synchronizeAll;
	}
	bool getSynchronizeAllMotionStates() const
	{
		return m_synchronizeAllMotionStates;
	}

	///Preliminary serialization test for Bullet 2.76. Loading those files requires a separate parser (see Bullet/Demos/SerializeDemo)
	virtual	void	serialize(btSerializer* serializer);

};

#endif //BT_DISCRETE_DYNAMICS_WORLD_H
