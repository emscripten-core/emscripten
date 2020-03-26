/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2010 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_TYPED_CONSTRAINT_H
#define BT_TYPED_CONSTRAINT_H

class btRigidBody;
#include "LinearMath/btScalar.h"
#include "btSolverConstraint.h"

class btSerializer;

//Don't change any of the existing enum values, so add enum types at the end for serialization compatibility
enum btTypedConstraintType
{
	POINT2POINT_CONSTRAINT_TYPE=3,
	HINGE_CONSTRAINT_TYPE,
	CONETWIST_CONSTRAINT_TYPE,
	D6_CONSTRAINT_TYPE,
	SLIDER_CONSTRAINT_TYPE,
	CONTACT_CONSTRAINT_TYPE,
	D6_SPRING_CONSTRAINT_TYPE,
	MAX_CONSTRAINT_TYPE
};


enum btConstraintParams
{
	BT_CONSTRAINT_ERP=1,
	BT_CONSTRAINT_STOP_ERP,
	BT_CONSTRAINT_CFM,
	BT_CONSTRAINT_STOP_CFM
};

#if 1
	#define btAssertConstrParams(_par) btAssert(_par) 
#else
	#define btAssertConstrParams(_par)
#endif


///TypedConstraint is the baseclass for Bullet constraints and vehicles
class btTypedConstraint : public btTypedObject
{
	int	m_userConstraintType;

	union
	{
		int	m_userConstraintId;
		void* m_userConstraintPtr;
	};

	btScalar	m_breakingImpulseThreshold;
	bool		m_isEnabled;


	bool m_needsFeedback;

	btTypedConstraint&	operator=(btTypedConstraint&	other)
	{
		btAssert(0);
		(void) other;
		return *this;
	}

protected:
	btRigidBody&	m_rbA;
	btRigidBody&	m_rbB;
	btScalar	m_appliedImpulse;
	btScalar	m_dbgDrawSize;

	///internal method used by the constraint solver, don't use them directly
	btScalar getMotorFactor(btScalar pos, btScalar lowLim, btScalar uppLim, btScalar vel, btScalar timeFact);
	
	static btRigidBody& getFixedBody();

public:

	virtual ~btTypedConstraint() {};
	btTypedConstraint(btTypedConstraintType type, btRigidBody& rbA);
	btTypedConstraint(btTypedConstraintType type, btRigidBody& rbA,btRigidBody& rbB);

	struct btConstraintInfo1 {
		int m_numConstraintRows,nub;
	};

	struct btConstraintInfo2 {
		// integrator parameters: frames per second (1/stepsize), default error
		// reduction parameter (0..1).
		btScalar fps,erp;

		// for the first and second body, pointers to two (linear and angular)
		// n*3 jacobian sub matrices, stored by rows. these matrices will have
		// been initialized to 0 on entry. if the second body is zero then the
		// J2xx pointers may be 0.
		btScalar *m_J1linearAxis,*m_J1angularAxis,*m_J2linearAxis,*m_J2angularAxis;

		// elements to jump from one row to the next in J's
		int rowskip;

		// right hand sides of the equation J*v = c + cfm * lambda. cfm is the
		// "constraint force mixing" vector. c is set to zero on entry, cfm is
		// set to a constant value (typically very small or zero) value on entry.
		btScalar *m_constraintError,*cfm;

		// lo and hi limits for variables (set to -/+ infinity on entry).
		btScalar *m_lowerLimit,*m_upperLimit;

		// findex vector for variables. see the LCP solver interface for a
		// description of what this does. this is set to -1 on entry.
		// note that the returned indexes are relative to the first index of
		// the constraint.
		int *findex;
		// number of solver iterations
		int m_numIterations;

		//damping of the velocity
		btScalar	m_damping;
	};

	///internal method used by the constraint solver, don't use them directly
	virtual void	buildJacobian() {};

	///internal method used by the constraint solver, don't use them directly
	virtual	void	setupSolverConstraint(btConstraintArray& ca, int solverBodyA,int solverBodyB, btScalar timeStep)
	{
        (void)ca;
        (void)solverBodyA;
        (void)solverBodyB;
        (void)timeStep;
	}
	
	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo1 (btConstraintInfo1* info)=0;

	///internal method used by the constraint solver, don't use them directly
	virtual void getInfo2 (btConstraintInfo2* info)=0;

	///internal method used by the constraint solver, don't use them directly
	void	internalSetAppliedImpulse(btScalar appliedImpulse)
	{
		m_appliedImpulse = appliedImpulse;
	}
	///internal method used by the constraint solver, don't use them directly
	btScalar	internalGetAppliedImpulse()
	{
		return m_appliedImpulse;
	}


	btScalar	getBreakingImpulseThreshold() const
	{
		return 	m_breakingImpulseThreshold;
	}

	void	setBreakingImpulseThreshold(btScalar threshold)
	{
		m_breakingImpulseThreshold = threshold;
	}

	bool	isEnabled() const
	{
		return m_isEnabled;
	}

	void	setEnabled(bool enabled)
	{
		m_isEnabled=enabled;
	}


	///internal method used by the constraint solver, don't use them directly
	virtual	void	solveConstraintObsolete(btRigidBody& /*bodyA*/,btRigidBody& /*bodyB*/,btScalar	/*timeStep*/) {};

	
	const btRigidBody& getRigidBodyA() const
	{
		return m_rbA;
	}
	const btRigidBody& getRigidBodyB() const
	{
		return m_rbB;
	}

	btRigidBody& getRigidBodyA() 
	{
		return m_rbA;
	}
	btRigidBody& getRigidBodyB()
	{
		return m_rbB;
	}

	int getUserConstraintType() const
	{
		return m_userConstraintType ;
	}

	void	setUserConstraintType(int userConstraintType)
	{
		m_userConstraintType = userConstraintType;
	};

	void	setUserConstraintId(int uid)
	{
		m_userConstraintId = uid;
	}

	int getUserConstraintId() const
	{
		return m_userConstraintId;
	}

	void	setUserConstraintPtr(void* ptr)
	{
		m_userConstraintPtr = ptr;
	}

	void*	getUserConstraintPtr()
	{
		return m_userConstraintPtr;
	}

	int getUid() const
	{
		return m_userConstraintId;   
	} 

	bool	needsFeedback() const
	{
		return m_needsFeedback;
	}

	///enableFeedback will allow to read the applied linear and angular impulse
	///use getAppliedImpulse, getAppliedLinearImpulse and getAppliedAngularImpulse to read feedback information
	void	enableFeedback(bool needsFeedback)
	{
		m_needsFeedback = needsFeedback;
	}

	///getAppliedImpulse is an estimated total applied impulse. 
	///This feedback could be used to determine breaking constraints or playing sounds.
	btScalar	getAppliedImpulse() const
	{
		btAssert(m_needsFeedback);
		return m_appliedImpulse;
	}

	btTypedConstraintType getConstraintType () const
	{
		return btTypedConstraintType(m_objectType);
	}
	
	void setDbgDrawSize(btScalar dbgDrawSize)
	{
		m_dbgDrawSize = dbgDrawSize;
	}
	btScalar getDbgDrawSize()
	{
		return m_dbgDrawSize;
	}

	///override the default global value of a parameter (such as ERP or CFM), optionally provide the axis (0..5). 
	///If no axis is provided, it uses the default axis for this constraint.
	virtual	void	setParam(int num, btScalar value, int axis = -1) = 0;

	///return the local value of parameter
	virtual	btScalar getParam(int num, int axis = -1) const = 0;
	
	virtual	int	calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual	const char*	serialize(void* dataBuffer, btSerializer* serializer) const;

};

// returns angle in range [-SIMD_2_PI, SIMD_2_PI], closest to one of the limits 
// all arguments should be normalized angles (i.e. in range [-SIMD_PI, SIMD_PI])
SIMD_FORCE_INLINE btScalar btAdjustAngleToLimits(btScalar angleInRadians, btScalar angleLowerLimitInRadians, btScalar angleUpperLimitInRadians)
{
	if(angleLowerLimitInRadians >= angleUpperLimitInRadians)
	{
		return angleInRadians;
	}
	else if(angleInRadians < angleLowerLimitInRadians)
	{
		btScalar diffLo = btFabs(btNormalizeAngle(angleLowerLimitInRadians - angleInRadians));
		btScalar diffHi = btFabs(btNormalizeAngle(angleUpperLimitInRadians - angleInRadians));
		return (diffLo < diffHi) ? angleInRadians : (angleInRadians + SIMD_2_PI);
	}
	else if(angleInRadians > angleUpperLimitInRadians)
	{
		btScalar diffHi = btFabs(btNormalizeAngle(angleInRadians - angleUpperLimitInRadians));
		btScalar diffLo = btFabs(btNormalizeAngle(angleInRadians - angleLowerLimitInRadians));
		return (diffLo < diffHi) ? (angleInRadians - SIMD_2_PI) : angleInRadians;
	}
	else
	{
		return angleInRadians;
	}
}

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	btTypedConstraintData
{
	btRigidBodyData		*m_rbA;
	btRigidBodyData		*m_rbB;
	char	*m_name;

	int	m_objectType;
	int	m_userConstraintType;
	int	m_userConstraintId;
	int	m_needsFeedback;

	float	m_appliedImpulse;
	float	m_dbgDrawSize;

	int	m_disableCollisionsBetweenLinkedBodies;
	char	m_pad4[4];
	
};

SIMD_FORCE_INLINE	int	btTypedConstraint::calculateSerializeBufferSize() const
{
	return sizeof(btTypedConstraintData);
}



class btAngularLimit
{
private:
	btScalar 
		m_center,
		m_halfRange,
		m_softness,
		m_biasFactor,
		m_relaxationFactor,
		m_correction,
		m_sign;

	bool
		m_solveLimit;

public:
	/// Default constructor initializes limit as inactive, allowing free constraint movement
	btAngularLimit()
		:m_center(0.0f),
		m_halfRange(-1.0f),
		m_softness(0.9f),
		m_biasFactor(0.3f),
		m_relaxationFactor(1.0f),
		m_correction(0.0f),
		m_sign(0.0f),
		m_solveLimit(false)
	{}

	/// Sets all limit's parameters.
	/// When low > high limit becomes inactive.
	/// When high - low > 2PI limit is ineffective too becouse no angle can exceed the limit
	void set(btScalar low, btScalar high, btScalar _softness = 0.9f, btScalar _biasFactor = 0.3f, btScalar _relaxationFactor = 1.0f);

	/// Checks conastaint angle against limit. If limit is active and the angle violates the limit
	/// correction is calculated.
	void test(const btScalar angle);

	/// Returns limit's softness
	inline btScalar getSoftness() const
	{
		return m_softness;
	}

	/// Returns limit's bias factor
	inline btScalar getBiasFactor() const
	{
		return m_biasFactor;
	}

	/// Returns limit's relaxation factor
	inline btScalar getRelaxationFactor() const
	{
		return m_relaxationFactor;
	}

	/// Returns correction value evaluated when test() was invoked 
	inline btScalar getCorrection() const
	{
		return m_correction;
	}

	/// Returns sign value evaluated when test() was invoked 
	inline btScalar getSign() const
	{
		return m_sign;
	}

	/// Gives half of the distance between min and max limit angle
	inline btScalar getHalfRange() const
	{
		return m_halfRange;
	}

	/// Returns true when the last test() invocation recognized limit violation
	inline bool isLimit() const
	{
		return m_solveLimit;
	}

	/// Checks given angle against limit. If limit is active and angle doesn't fit it, the angle
	/// returned is modified so it equals to the limit closest to given angle.
	void fit(btScalar& angle) const;

	/// Returns correction value multiplied by sign value
	btScalar getError() const;

	btScalar getLow() const;

	btScalar getHigh() const;

};



#endif //BT_TYPED_CONSTRAINT_H
