/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
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

#ifndef B2_MOUSE_JOINT_H
#define B2_MOUSE_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

/// Mouse joint definition. This requires a world target point,
/// tuning parameters, and the time step.
// emscripten - b2MouseJointDef: add functions to set/get base class members
struct b2MouseJointDef : public b2JointDef
{
	b2MouseJointDef()
	{
		type = e_mouseJoint;
		target.Set(0.0f, 0.0f);
		maxForce = 0.0f;
		frequencyHz = 5.0f;
		dampingRatio = 0.7f;
	}

	/// The initial world target point. This is assumed
	/// to coincide with the body anchor initially.
	b2Vec2 target;

	/// The maximum constraint force that can be exerted
	/// to move the candidate body. Usually you will express
	/// as some multiple of the weight (multiplier * mass * gravity).
	float32 maxForce;

	/// The response speed.
	float32 frequencyHz;

	/// The damping ratio. 0 = no damping, 1 = critical damping.
	float32 dampingRatio;

	// to generate javascript bindings
	void set_bodyA(b2Body* b) { bodyA = b; }
	void set_bodyB(b2Body* b) { bodyB = b; }
	void set_collideConnected(bool b) { collideConnected = b; }
	b2Body* get_bodyA(b2Body* b) { return bodyA; }
	b2Body* get_bodyB(b2Body* b) { return bodyB; }
	bool get_collideConnected(bool b) { return collideConnected; }
};

/// A mouse joint is used to make a point on a body track a
/// specified world point. This a soft constraint with a maximum
/// force. This allows the constraint to stretch and without
/// applying huge forces.
/// NOTE: this joint is not documented in the manual because it was
/// developed to be used in the testbed. If you want to learn how to
/// use the mouse joint, look at the testbed.
// emscripten - b2MouseJoint: make constructor public
class b2MouseJoint : public b2Joint
{
public:

	/// Implements b2Joint.
	b2Vec2 GetAnchorA() const;

	/// Implements b2Joint.
	b2Vec2 GetAnchorB() const;

	/// Implements b2Joint.
	b2Vec2 GetReactionForce(float32 inv_dt) const;

	/// Implements b2Joint.
	float32 GetReactionTorque(float32 inv_dt) const;

	/// Use this to update the target point.
	void SetTarget(const b2Vec2& target);
	const b2Vec2& GetTarget() const;

	/// Set/get the maximum force in Newtons.
	void SetMaxForce(float32 force);
	float32 GetMaxForce() const;

	/// Set/get the frequency in Hertz.
	void SetFrequency(float32 hz);
	float32 GetFrequency() const;

	/// Set/get the damping ratio (dimensionless).
	void SetDampingRatio(float32 ratio);
	float32 GetDampingRatio() const;

	/// The mouse joint does not support dumping.
	void Dump() { b2Log("Mouse joint dumping is not supported.\n"); }

	b2MouseJoint(const b2MouseJointDef* def);

protected:
	friend class b2Joint;

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	b2Vec2 m_localAnchorB;
	b2Vec2 m_targetA;
	float32 m_frequencyHz;
	float32 m_dampingRatio;
	float32 m_beta;
	
	// Solver shared
	b2Vec2 m_impulse;
	float32 m_maxForce;
	float32 m_gamma;

	// Solver temp
	int32 m_indexA;
	int32 m_indexB;
	b2Vec2 m_rB;
	b2Vec2 m_localCenterB;
	float32 m_invMassB;
	float32 m_invIB;
	b2Mat22 m_mass;
	b2Vec2 m_C;
};

#endif
