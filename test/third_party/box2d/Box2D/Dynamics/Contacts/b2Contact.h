/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
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

#ifndef B2_CONTACT_H
#define B2_CONTACT_H

#include <Box2D/Common/b2Math.h>
#include <Box2D/Collision/b2Collision.h>
#include <Box2D/Collision/Shapes/b2Shape.h>
#include <Box2D/Dynamics/b2Fixture.h>

class b2Body;
class b2Contact;
class b2Fixture;
class b2World;
class b2BlockAllocator;
class b2StackAllocator;
class b2ContactListener;

/// Friction mixing law. The idea is to allow either fixture to drive the restitution to zero.
/// For example, anything slides on ice.
inline float32 b2MixFriction(float32 friction1, float32 friction2)
{
	return std::sqrt(friction1 * friction2);
}

/// Restitution mixing law. The idea is allow for anything to bounce off an inelastic surface.
/// For example, a superball bounces on anything.
inline float32 b2MixRestitution(float32 restitution1, float32 restitution2)
{
	return restitution1 > restitution2 ? restitution1 : restitution2;
}

typedef b2Contact* b2ContactCreateFcn(	b2Fixture* fixtureA, int32 indexA,
										b2Fixture* fixtureB, int32 indexB,
										b2BlockAllocator* allocator);
typedef void b2ContactDestroyFcn(b2Contact* contact, b2BlockAllocator* allocator);

struct b2ContactRegister
{
	b2ContactCreateFcn* createFcn;
	b2ContactDestroyFcn* destroyFcn;
	bool primary;
};

/// A contact edge is used to connect bodies and contacts together
/// in a contact graph where each body is a node and each contact
/// is an edge. A contact edge belongs to a doubly linked list
/// maintained in each attached body. Each contact has two contact
/// nodes, one for each attached body.
// emscripten - b2ContactEdge: add constructor
struct b2ContactEdge
{
	b2Body* other;			///< provides quick access to the other body attached.
	b2Contact* contact;		///< the contact
	b2ContactEdge* prev;	///< the previous contact edge in the body's contact list
	b2ContactEdge* next;	///< the next contact edge in the body's contact list

  b2ContactEdge() {}
};

/// The class manages contact between two shapes. A contact exists for each overlapping
/// AABB in the broad-phase (except if filtered). Therefore a contact object may exist
/// that has no contact points.
class b2Contact
{
public:

	/// Get the contact manifold. Do not modify the manifold unless you understand the
	/// internals of Box2D.
	b2Manifold* GetManifold();
	const b2Manifold* GetManifold() const;

	/// Get the world manifold.
	void GetWorldManifold(b2WorldManifold* worldManifold) const;

	/// Is this contact touching?
	bool IsTouching() const;

	/// Enable/disable this contact. This can be used inside the pre-solve
	/// contact listener. The contact is only disabled for the current
	/// time step (or sub-step in continuous collisions).
	void SetEnabled(bool flag);

	/// Has this contact been disabled?
	bool IsEnabled() const;

	/// Get the next contact in the world's contact list.
	b2Contact* GetNext();
	const b2Contact* GetNext() const;

	/// Get fixture A in this contact.
	b2Fixture* GetFixtureA();
	const b2Fixture* GetFixtureA() const;

	/// Get the child primitive index for fixture A.
	int32 GetChildIndexA() const;

	/// Get fixture B in this contact.
	b2Fixture* GetFixtureB();
	const b2Fixture* GetFixtureB() const;

	/// Get the child primitive index for fixture B.
	int32 GetChildIndexB() const;

	/// Override the default friction mixture. You can call this in b2ContactListener::PreSolve.
	/// This value persists until set or reset.
	void SetFriction(float32 friction);

	/// Get the friction.
	float32 GetFriction() const;

	/// Reset the friction mixture to the default value.
	void ResetFriction();

	/// Override the default restitution mixture. You can call this in b2ContactListener::PreSolve.
	/// The value persists until you set or reset.
	void SetRestitution(float32 restitution);

	/// Get the restitution.
	float32 GetRestitution() const;

	/// Reset the restitution to the default value.
	void ResetRestitution();

	/// Evaluate this contact with your own manifold and transforms.
	virtual void Evaluate(b2Manifold* manifold, const b2Transform& xfA, const b2Transform& xfB) = 0;

protected:
	friend class b2ContactManager;
	friend class b2World;
	friend class b2ContactSolver;
	friend class b2Body;
	friend class b2Fixture;

	// Flags stored in m_flags
	enum
	{
		// Used when crawling contact graph when forming islands.
		e_islandFlag		= 0x0001,

        // Set when the shapes are touching.
		e_touchingFlag		= 0x0002,

		// This contact can be disabled (by user)
		e_enabledFlag		= 0x0004,

		// This contact needs filtering because a fixture filter was changed.
		e_filterFlag		= 0x0008,

		// This bullet contact had a TOI event
		e_bulletHitFlag		= 0x0010,

		// This contact has a valid TOI in m_toi
		e_toiFlag			= 0x0020
	};

	/// Flag this contact for filtering. Filtering will occur the next time step.
	void FlagForFiltering();

	static void AddType(b2ContactCreateFcn* createFcn, b2ContactDestroyFcn* destroyFcn,
						b2Shape::Type typeA, b2Shape::Type typeB);
	static void InitializeRegisters();
	static b2Contact* Create(b2Fixture* fixtureA, int32 indexA, b2Fixture* fixtureB, int32 indexB, b2BlockAllocator* allocator);
	static void Destroy(b2Contact* contact, b2Shape::Type typeA, b2Shape::Type typeB, b2BlockAllocator* allocator);
	static void Destroy(b2Contact* contact, b2BlockAllocator* allocator);

	b2Contact() : m_fixtureA(NULL), m_fixtureB(NULL) {}
	b2Contact(b2Fixture* fixtureA, int32 indexA, b2Fixture* fixtureB, int32 indexB);
	virtual ~b2Contact() {}

	void Update(b2ContactListener* listener);

	static b2ContactRegister s_registers[b2Shape::e_typeCount][b2Shape::e_typeCount];
	static bool s_initialized;

	uint32 m_flags;

	// World pool and list pointers.
	b2Contact* m_prev;
	b2Contact* m_next;

	// Nodes for connecting bodies.
	b2ContactEdge m_nodeA;
	b2ContactEdge m_nodeB;

	b2Fixture* m_fixtureA;
	b2Fixture* m_fixtureB;

	int32 m_indexA;
	int32 m_indexB;

	b2Manifold m_manifold;

	int32 m_toiCount;
	float32 m_toi;

	float32 m_friction;
	float32 m_restitution;
};

inline b2Manifold* b2Contact::GetManifold()
{
	return &m_manifold;
}

inline const b2Manifold* b2Contact::GetManifold() const
{
	return &m_manifold;
}

inline void b2Contact::GetWorldManifold(b2WorldManifold* worldManifold) const
{
	const b2Body* bodyA = m_fixtureA->GetBody();
	const b2Body* bodyB = m_fixtureB->GetBody();
	const b2Shape* shapeA = m_fixtureA->GetShape();
	const b2Shape* shapeB = m_fixtureB->GetShape();

	worldManifold->Initialize(&m_manifold, bodyA->GetTransform(), shapeA->m_radius, bodyB->GetTransform(), shapeB->m_radius);
}

inline void b2Contact::SetEnabled(bool flag)
{
	if (flag)
	{
		m_flags |= e_enabledFlag;
	}
	else
	{
		m_flags &= ~e_enabledFlag;
	}
}

inline bool b2Contact::IsEnabled() const
{
	return (m_flags & e_enabledFlag) == e_enabledFlag;
}

inline bool b2Contact::IsTouching() const
{
	return (m_flags & e_touchingFlag) == e_touchingFlag;
}

inline b2Contact* b2Contact::GetNext()
{
	return m_next;
}

inline const b2Contact* b2Contact::GetNext() const
{
	return m_next;
}

inline b2Fixture* b2Contact::GetFixtureA()
{
	return m_fixtureA;
}

inline const b2Fixture* b2Contact::GetFixtureA() const
{
	return m_fixtureA;
}

inline b2Fixture* b2Contact::GetFixtureB()
{
	return m_fixtureB;
}

inline int32 b2Contact::GetChildIndexA() const
{
	return m_indexA;
}

inline const b2Fixture* b2Contact::GetFixtureB() const
{
	return m_fixtureB;
}

inline int32 b2Contact::GetChildIndexB() const
{
	return m_indexB;
}

inline void b2Contact::FlagForFiltering()
{
	m_flags |= e_filterFlag;
}

inline void b2Contact::SetFriction(float32 friction)
{
	m_friction = friction;
}

inline float32 b2Contact::GetFriction() const
{
	return m_friction;
}

inline void b2Contact::ResetFriction()
{
	m_friction = b2MixFriction(m_fixtureA->m_friction, m_fixtureB->m_friction);
}

inline void b2Contact::SetRestitution(float32 restitution)
{
	m_restitution = restitution;
}

inline float32 b2Contact::GetRestitution() const
{
	return m_restitution;
}

inline void b2Contact::ResetRestitution()
{
	m_restitution = b2MixRestitution(m_fixtureA->m_restitution, m_fixtureB->m_restitution);
}

#endif
