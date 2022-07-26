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

#ifndef BT_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H
#define BT_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H

#include "btConstraintSolver.h"
class btIDebugDraw;
#include "btContactConstraint.h"
#include "btSolverBody.h"
#include "btSolverConstraint.h"
#include "btTypedConstraint.h"
#include "BulletCollision/NarrowPhaseCollision/btManifoldPoint.h"

///The btSequentialImpulseConstraintSolver is a fast SIMD implementation of the Projected Gauss Seidel (iterative LCP) method.
class btSequentialImpulseConstraintSolver : public btConstraintSolver
{
protected:

	btConstraintArray			m_tmpSolverContactConstraintPool;
	btConstraintArray			m_tmpSolverNonContactConstraintPool;
	btConstraintArray			m_tmpSolverContactFrictionConstraintPool;
	btAlignedObjectArray<int>	m_orderTmpConstraintPool;
	btAlignedObjectArray<int>	m_orderFrictionConstraintPool;
	btAlignedObjectArray<btTypedConstraint::btConstraintInfo1> m_tmpConstraintSizesPool;

	void setupFrictionConstraint(	btSolverConstraint& solverConstraint, const btVector3& normalAxis,btRigidBody* solverBodyA,btRigidBody* solverBodyIdB,
									btManifoldPoint& cp,const btVector3& rel_pos1,const btVector3& rel_pos2,
									btCollisionObject* colObj0,btCollisionObject* colObj1, btScalar relaxation, 
									btScalar desiredVelocity=0., btScalar cfmSlip=0.);

	btSolverConstraint&	addFrictionConstraint(const btVector3& normalAxis,btRigidBody* solverBodyA,btRigidBody* solverBodyB,int frictionIndex,btManifoldPoint& cp,const btVector3& rel_pos1,const btVector3& rel_pos2,btCollisionObject* colObj0,btCollisionObject* colObj1, btScalar relaxation, btScalar desiredVelocity=0., btScalar cfmSlip=0.);
	
	void setupContactConstraint(btSolverConstraint& solverConstraint, btCollisionObject* colObj0, btCollisionObject* colObj1, btManifoldPoint& cp, 
								const btContactSolverInfo& infoGlobal, btVector3& vel, btScalar& rel_vel, btScalar& relaxation, 
								btVector3& rel_pos1, btVector3& rel_pos2);

	void setFrictionConstraintImpulse( btSolverConstraint& solverConstraint, btRigidBody* rb0, btRigidBody* rb1, 
										 btManifoldPoint& cp, const btContactSolverInfo& infoGlobal);

	///m_btSeed2 is used for re-arranging the constraint rows. improves convergence/quality of friction
	unsigned long	m_btSeed2;

//	void	initSolverBody(btSolverBody* solverBody, btCollisionObject* collisionObject);
	btScalar restitutionCurve(btScalar rel_vel, btScalar restitution);

	void	convertContact(btPersistentManifold* manifold,const btContactSolverInfo& infoGlobal);


	void	resolveSplitPenetrationSIMD(
        btRigidBody& body1,
        btRigidBody& body2,
        const btSolverConstraint& contactConstraint);

	void	resolveSplitPenetrationImpulseCacheFriendly(
        btRigidBody& body1,
        btRigidBody& body2,
        const btSolverConstraint& contactConstraint);

	//internal method
	int	getOrInitSolverBody(btCollisionObject& body);

	void	resolveSingleConstraintRowGeneric(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& contactConstraint);

	void	resolveSingleConstraintRowGenericSIMD(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& contactConstraint);
	
	void	resolveSingleConstraintRowLowerLimit(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& contactConstraint);
	
	void	resolveSingleConstraintRowLowerLimitSIMD(btRigidBody& body1,btRigidBody& body2,const btSolverConstraint& contactConstraint);
		
protected:
	static btRigidBody& getFixedBody();
	
	virtual void solveGroupCacheFriendlySplitImpulseIterations(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);
	virtual btScalar solveGroupCacheFriendlyFinish(btCollisionObject** bodies ,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);
	btScalar solveSingleIteration(int iteration, btCollisionObject** bodies ,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);

	virtual btScalar solveGroupCacheFriendlySetup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);
	virtual btScalar solveGroupCacheFriendlyIterations(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifoldPtr, int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& infoGlobal,btIDebugDraw* debugDrawer,btStackAlloc* stackAlloc);


public:

	
	btSequentialImpulseConstraintSolver();
	virtual ~btSequentialImpulseConstraintSolver();

	virtual btScalar solveGroup(btCollisionObject** bodies,int numBodies,btPersistentManifold** manifold,int numManifolds,btTypedConstraint** constraints,int numConstraints,const btContactSolverInfo& info, btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,btDispatcher* dispatcher);
	

	
	///clear internal cached data and reset random seed
	virtual	void	reset();
	
	unsigned long btRand2();

	int btRandInt2 (int n);

	void	setRandSeed(unsigned long seed)
	{
		m_btSeed2 = seed;
	}
	unsigned long	getRandSeed() const
	{
		return m_btSeed2;
	}

};

#ifndef BT_PREFER_SIMD
typedef btSequentialImpulseConstraintSolver btSequentialImpulseConstraintSolverPrefered;
#endif


#endif //BT_SEQUENTIAL_IMPULSE_CONSTRAINT_SOLVER_H

