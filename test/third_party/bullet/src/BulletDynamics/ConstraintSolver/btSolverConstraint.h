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

#ifndef BT_SOLVER_CONSTRAINT_H
#define BT_SOLVER_CONSTRAINT_H

class	btRigidBody;
#include "LinearMath/btVector3.h"
#include "LinearMath/btMatrix3x3.h"
#include "btJacobianEntry.h"

//#define NO_FRICTION_TANGENTIALS 1
#include "btSolverBody.h"


///1D constraint along a normal axis between bodyA and bodyB. It can be combined to solve contact and friction constraints.
ATTRIBUTE_ALIGNED64 (struct)	btSolverConstraint
{
	BT_DECLARE_ALIGNED_ALLOCATOR();

	btVector3		m_relpos1CrossNormal;
	btVector3		m_contactNormal;

	btVector3		m_relpos2CrossNormal;
	//btVector3		m_contactNormal2;//usually m_contactNormal2 == -m_contactNormal

	btVector3		m_angularComponentA;
	btVector3		m_angularComponentB;
	
	mutable btSimdScalar	m_appliedPushImpulse;
	mutable btSimdScalar	m_appliedImpulse;
	
	
	btScalar	m_friction;
	btScalar	m_jacDiagABInv;
	union
	{
		int	m_numConsecutiveRowsPerKernel;
		btScalar	m_unusedPadding0;
	};

	union
	{
		int			m_frictionIndex;
		btScalar	m_unusedPadding1;
	};
	union
	{
		btRigidBody*	m_solverBodyA;
		int				m_companionIdA;
	};
	union
	{
		btRigidBody*	m_solverBodyB;
		int				m_companionIdB;
	};
	
	union
	{
		void*		m_originalContactPoint;
		btScalar	m_unusedPadding4;
	};

	btScalar		m_rhs;
	btScalar		m_cfm;
	btScalar		m_lowerLimit;
	btScalar		m_upperLimit;

	btScalar		m_rhsPenetration;

	enum		btSolverConstraintType
	{
		BT_SOLVER_CONTACT_1D = 0,
		BT_SOLVER_FRICTION_1D
	};
};

typedef btAlignedObjectArray<btSolverConstraint>	btConstraintArray;


#endif //BT_SOLVER_CONSTRAINT_H



