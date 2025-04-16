/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2007 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_GENERIC_6DOF_SPRING_CONSTRAINT_H
#define BT_GENERIC_6DOF_SPRING_CONSTRAINT_H


#include "LinearMath/btVector3.h"
#include "btTypedConstraint.h"
#include "btGeneric6DofConstraint.h"


/// Generic 6 DOF constraint that allows to set spring motors to any translational and rotational DOF

/// DOF index used in enableSpring() and setStiffness() means:
/// 0 : translation X
/// 1 : translation Y
/// 2 : translation Z
/// 3 : rotation X (3rd Euler rotational around new position of X axis, range [-PI+epsilon, PI-epsilon] )
/// 4 : rotation Y (2nd Euler rotational around new position of Y axis, range [-PI/2+epsilon, PI/2-epsilon] )
/// 5 : rotation Z (1st Euler rotational around Z axis, range [-PI+epsilon, PI-epsilon] )

class btGeneric6DofSpringConstraint : public btGeneric6DofConstraint
{
protected:
	bool		m_springEnabled[6];
	btScalar	m_equilibriumPoint[6];
	btScalar	m_springStiffness[6];
	btScalar	m_springDamping[6]; // between 0 and 1 (1 == no damping)
	void internalUpdateSprings(btConstraintInfo2* info);
public: 
    btGeneric6DofSpringConstraint(btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameInA, const btTransform& frameInB ,bool useLinearReferenceFrameA);
	void enableSpring(int index, bool onOff);
	void setStiffness(int index, btScalar stiffness);
	void setDamping(int index, btScalar damping);
	void setEquilibriumPoint(); // set the current constraint position/orientation as an equilibrium point for all DOF
	void setEquilibriumPoint(int index);  // set the current constraint position/orientation as an equilibrium point for given DOF
	void setEquilibriumPoint(int index, btScalar val);

	virtual void setAxis( const btVector3& axis1, const btVector3& axis2);

	virtual void getInfo2 (btConstraintInfo2* info);

	virtual	int	calculateSerializeBufferSize() const;
	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual	const char*	serialize(void* dataBuffer, btSerializer* serializer) const;

};


///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct btGeneric6DofSpringConstraintData
{
	btGeneric6DofConstraintData	m_6dofData;
	
	int			m_springEnabled[6];
	float		m_equilibriumPoint[6];
	float		m_springStiffness[6];
	float		m_springDamping[6];
};

SIMD_FORCE_INLINE	int	btGeneric6DofSpringConstraint::calculateSerializeBufferSize() const
{
	return sizeof(btGeneric6DofSpringConstraintData);
}

	///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE	const char*	btGeneric6DofSpringConstraint::serialize(void* dataBuffer, btSerializer* serializer) const
{
	btGeneric6DofSpringConstraintData* dof = (btGeneric6DofSpringConstraintData*)dataBuffer;
	btGeneric6DofConstraint::serialize(&dof->m_6dofData,serializer);

	int i;
	for (i=0;i<6;i++)
	{
		dof->m_equilibriumPoint[i] = m_equilibriumPoint[i];
		dof->m_springDamping[i] = m_springDamping[i];
		dof->m_springEnabled[i] = m_springEnabled[i]? 1 : 0;
		dof->m_springStiffness[i] = m_springStiffness[i];
	}
	return "btGeneric6DofConstraintData";
}

#endif // BT_GENERIC_6DOF_SPRING_CONSTRAINT_H

