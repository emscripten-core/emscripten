/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_WHEEL_JOINT_H
#define B2_WHEEL_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

/// Wheel joint definition. This requires defining a line of
/// motion using an axis and an anchor point. The definition uses local
/// anchor points and a local axis so that the initial configuration
/// can violate the constraint slightly. The joint translation is zero
/// when the local anchor points coincide in world space. Using local
/// anchors and a local axis helps when saving and loading a game.
// emscripten - b2WheelJointDef: add functions to set/get base class members
struct b2WheelJointDef : public b2JointDef
{
	b2WheelJointDef()
	{
		type = e_wheelJoint;
		localAnchorA.SetZero();
		localAnchorB.SetZero();
		localAxisA.Set(1.0f, 0.0f);
		enableMotor = false;
		maxMotorTorque = 0.0f;
		motorSpeed = 0.0f;
		frequencyHz = 2.0f;
		dampingRatio = 0.7f;
	}

	/// Initialize the bodies, anchors, axis, and reference angle using the world
	/// anchor and world axis.
	void Initialize(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor, const b2Vec2& axis);

	/// The local anchor point relative to bodyA's origin.
	b2Vec2 localAnchorA;

	/// The local anchor point relative to bodyB's origin.
	b2Vec2 localAnchorB;

	/// The local translation axis in bodyA.
	b2Vec2 localAxisA;

	/// Enable/disable the joint motor.
	bool enableMotor;

	/// The maximum motor torque, usually in N-m.
	float32 maxMotorTorque;

	/// The desired motor speed in radians per second.
	float32 motorSpeed;

	/// Suspension frequency, zero indicates no suspension
	float32 frequencyHz;

	/// Suspension damping ratio, one indicates critical damping
	float32 dampingRatio;

	// to generate javascript bindings
	void set_bodyA(b2Body* b) { bodyA = b; }
	void set_bodyB(b2Body* b) { bodyB = b; }
	void set_collideConnected(bool b) { collideConnected = b; }
	b2Body* get_bodyA(b2Body* b) { return bodyA; }
	b2Body* get_bodyB(b2Body* b) { return bodyB; }
	bool get_collideConnected(bool b) { return collideConnected; }
};

/// A wheel joint. This joint provides two degrees of freedom: translation
/// along an axis fixed in bodyA and rotation in the plane. You can use a
/// joint limit to restrict the range of motion and a joint motor to drive
/// the rotation or to model rotational friction.
/// This joint is designed for vehicle suspensions.
// emscripten - b2WheelJoint: make constructor public
class b2WheelJoint : public b2Joint
{
public:
	void GetDefinition(b2WheelJointDef* def) const;

	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	/// The local anchor point relative to bodyA's origin.
	const b2Vec2& GetLocalAnchorA() const { return m_localAnchorA; }

	/// The local anchor point relative to bodyB's origin.
	const b2Vec2& GetLocalAnchorB() const  { return m_localAnchorB; }

	/// The local joint axis relative to bodyA.
	const b2Vec2& GetLocalAxisA() const { return m_localXAxisA; }

	/// Get the current joint translation, usually in meters.
	float32 GetJointTranslation() const;

	/// Get the current joint translation speed, usually in meters per second.
	float32 GetJointSpeed() const;

	/// Is the joint motor enabled?
	bool IsMotorEnabled() const;

	/// Enable/disable the joint motor.
	void EnableMotor(bool flag);

	/// Set the motor speed, usually in radians per second.
	void SetMotorSpeed(float32 speed);

	/// Get the motor speed, usually in radians per second.
	float32 GetMotorSpeed() const;

	/// Set/Get the maximum motor force, usually in N-m.
	void SetMaxMotorTorque(float32 torque);
	float32 GetMaxMotorTorque() const;

	/// Get the current motor torque given the inverse time step, usually in N-m.
	float32 GetMotorTorque(float32 inv_dt) const;

	/// Set/Get the spring frequency in hertz. Setting the frequency to zero disables the spring.
	void SetSpringFrequencyHz(float32 hz);
	float32 GetSpringFrequencyHz() const;

	/// Set/Get the spring damping ratio
	void SetSpringDampingRatio(float32 ratio);
	float32 GetSpringDampingRatio() const;

	/// Dump to b2Log
	void Dump();

	b2WheelJoint(const b2WheelJointDef* def);

protected:

	friend class b2Joint;

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	float32 m_frequencyHz;
	float32 m_dampingRatio;

	// Solver shared
	b2Vec2 m_localAnchorA;
	b2Vec2 m_localAnchorB;
	b2Vec2 m_localXAxisA;
	b2Vec2 m_localYAxisA;

	float32 m_impulse;
	float32 m_motorImpulse;
	float32 m_springImpulse;

	float32 m_maxMotorTorque;
	float32 m_motorSpeed;
	bool m_enableMotor;

	// Solver temp
	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_localCenterA;
	b2Vec2 m_localCenterB;
	float32 m_invMassA;
	float32 m_invMassB;
	float32 m_invIA;
	float32 m_invIB;

	b2Vec2 m_ax, m_ay;
	float32 m_sAx, m_sBx;
	float32 m_sAy, m_sBy;

	float32 m_mass;
	float32 m_motorMass;
	float32 m_springMass;

	float32 m_bias;
	float32 m_gamma;
};

inline float32 b2WheelJoint::GetMotorSpeed() const
{
	return m_motorSpeed;
}

inline float32 b2WheelJoint::GetMaxMotorTorque() const
{
	return m_maxMotorTorque;
}

inline void b2WheelJoint::SetSpringFrequencyHz(float32 hz)
{
	m_frequencyHz = hz;
}

inline float32 b2WheelJoint::GetSpringFrequencyHz() const
{
	return m_frequencyHz;
}

inline void b2WheelJoint::SetSpringDampingRatio(float32 ratio)
{
	m_dampingRatio = ratio;
}

inline float32 b2WheelJoint::GetSpringDampingRatio() const
{
	return m_dampingRatio;
}

#endif
