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

#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Dynamics/b2World.h>
#include <Box2D/Dynamics/Contacts/b2Contact.h>
#include <Box2D/Dynamics/Joints/b2Joint.h>

b2Body::b2Body(const b2BodyDef* bd, b2World* world)
{
	b2Assert(bd->position.IsValid());
	b2Assert(bd->linearVelocity.IsValid());
	b2Assert(b2IsValid(bd->angle));
	b2Assert(b2IsValid(bd->angularVelocity));
	b2Assert(b2IsValid(bd->angularDamping) && bd->angularDamping >= 0.0f);
	b2Assert(b2IsValid(bd->linearDamping) && bd->linearDamping >= 0.0f);

	m_flags = 0;

	if (bd->bullet)
	{
		m_flags |= e_bulletFlag;
	}
	if (bd->fixedRotation)
	{
		m_flags |= e_fixedRotationFlag;
	}
	if (bd->allowSleep)
	{
		m_flags |= e_autoSleepFlag;
	}
	if (bd->awake)
	{
		m_flags |= e_awakeFlag;
	}
	if (bd->active)
	{
		m_flags |= e_activeFlag;
	}

	m_world = world;

	m_xf.p = bd->position;
	m_xf.q.Set(bd->angle);

	m_sweep.localCenter.SetZero();
	m_sweep.c0 = m_xf.p;
	m_sweep.c = m_xf.p;
	m_sweep.a0 = bd->angle;
	m_sweep.a = bd->angle;
	m_sweep.alpha0 = 0.0f;

	m_jointList = NULL;
	m_contactList = NULL;
	m_prev = NULL;
	m_next = NULL;

	m_linearVelocity = bd->linearVelocity;
	m_angularVelocity = bd->angularVelocity;

	m_linearDamping = bd->linearDamping;
	m_angularDamping = bd->angularDamping;
	m_gravityScale = bd->gravityScale;

	m_force.SetZero();
	m_torque = 0.0f;

	m_sleepTime = 0.0f;

	m_type = bd->type;

	if (m_type == b2_dynamicBody)
	{
		m_mass = 1.0f;
		m_invMass = 1.0f;
	}
	else
	{
		m_mass = 0.0f;
		m_invMass = 0.0f;
	}

	m_I = 0.0f;
	m_invI = 0.0f;

	m_userData = bd->userData;

	m_fixtureList = NULL;
	m_fixtureCount = 0;
}

b2Body::~b2Body()
{
	// shapes and joints are destroyed in b2World::Destroy
}

void b2Body::SetType(b2BodyType type)
{
	b2Assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

	if (m_type == type)
	{
		return;
	}

	m_type = type;

	ResetMassData();

	if (m_type == b2_staticBody)
	{
		m_linearVelocity.SetZero();
		m_angularVelocity = 0.0f;
		m_sweep.a0 = m_sweep.a;
		m_sweep.c0 = m_sweep.c;
		SynchronizeFixtures();
	}

	SetAwake(true);

	m_force.SetZero();
	m_torque = 0.0f;

	// Since the body type changed, we need to flag contacts for filtering.
	for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
	{
		f->Refilter();
	}
}

b2Fixture* b2Body::CreateFixture(const b2FixtureDef* def)
{
	b2Assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return NULL;
	}

	b2BlockAllocator* allocator = &m_world->m_blockAllocator;

	void* memory = allocator->Allocate(sizeof(b2Fixture));
	b2Fixture* fixture = new (memory) b2Fixture;
	fixture->Create(allocator, this, def);

	if (m_flags & e_activeFlag)
	{
		b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		fixture->CreateProxies(broadPhase, m_xf);
	}

	fixture->m_next = m_fixtureList;
	m_fixtureList = fixture;
	++m_fixtureCount;

	fixture->m_body = this;

	// Adjust mass properties if needed.
	if (fixture->m_density > 0.0f)
	{
		ResetMassData();
	}

	// Let the world know we have a new fixture. This will cause new contacts
	// to be created at the beginning of the next time step.
	m_world->m_flags |= b2World::e_newFixture;

	return fixture;
}

b2Fixture* b2Body::CreateFixture(const b2Shape* shape, float32 density)
{
	b2FixtureDef def;
	def.shape = shape;
	def.density = density;

	return CreateFixture(&def);
}

void b2Body::DestroyFixture(b2Fixture* fixture)
{
	b2Assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

	b2Assert(fixture->m_body == this);

	// Remove the fixture from this body's singly linked list.
	b2Assert(m_fixtureCount > 0);
	b2Fixture** node = &m_fixtureList;
	bool found = false;
	while (*node != NULL)
	{
		if (*node == fixture)
		{
			*node = fixture->m_next;
			found = true;
			break;
		}

		node = &(*node)->m_next;
	}

	// You tried to remove a shape that is not attached to this body.
	b2Assert(found);

	// Destroy any contacts associated with the fixture.
	b2ContactEdge* edge = m_contactList;
	while (edge)
	{
		b2Contact* c = edge->contact;
		edge = edge->next;

		b2Fixture* fixtureA = c->GetFixtureA();
		b2Fixture* fixtureB = c->GetFixtureB();

		if (fixture == fixtureA || fixture == fixtureB)
		{
			// This destroys the contact and removes it from
			// this body's contact list.
			m_world->m_contactManager.Destroy(c);
		}
	}

	b2BlockAllocator* allocator = &m_world->m_blockAllocator;

	if (m_flags & e_activeFlag)
	{
		b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		fixture->DestroyProxies(broadPhase);
	}

	fixture->Destroy(allocator);
	fixture->m_body = NULL;
	fixture->m_next = NULL;
	fixture->~b2Fixture();
	allocator->Free(fixture, sizeof(b2Fixture));

	--m_fixtureCount;

	// Reset the mass data.
	ResetMassData();
}

void b2Body::ResetMassData()
{
	// Compute mass data from shapes. Each shape has its own density.
	m_mass = 0.0f;
	m_invMass = 0.0f;
	m_I = 0.0f;
	m_invI = 0.0f;
	m_sweep.localCenter.SetZero();

	// Static and kinematic bodies have zero mass.
	if (m_type == b2_staticBody || m_type == b2_kinematicBody)
	{
		m_sweep.c0 = m_xf.p;
		m_sweep.c = m_xf.p;
		m_sweep.a0 = m_sweep.a;
		return;
	}

	b2Assert(m_type == b2_dynamicBody);

	// Accumulate mass over all fixtures.
	b2Vec2 localCenter = b2Vec2_zero;
	for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
	{
		if (f->m_density == 0.0f)
		{
			continue;
		}

		b2MassData massData;
		f->GetMassData(&massData);
		m_mass += massData.mass;
		localCenter += massData.mass * massData.center;
		m_I += massData.I;
	}

	// Compute center of mass.
	if (m_mass > 0.0f)
	{
		m_invMass = 1.0f / m_mass;
		localCenter *= m_invMass;
	}
	else
	{
		// Force all dynamic bodies to have a positive mass.
		m_mass = 1.0f;
		m_invMass = 1.0f;
	}

	if (m_I > 0.0f && (m_flags & e_fixedRotationFlag) == 0)
	{
		// Center the inertia about the center of mass.
		m_I -= m_mass * b2Dot(localCenter, localCenter);
		b2Assert(m_I > 0.0f);
		m_invI = 1.0f / m_I;

	}
	else
	{
		m_I = 0.0f;
		m_invI = 0.0f;
	}

	// Move center of mass.
	b2Vec2 oldCenter = m_sweep.c;
	m_sweep.localCenter = localCenter;
	m_sweep.c0 = m_sweep.c = b2Mul(m_xf, m_sweep.localCenter);

	// Update center of mass velocity.
	m_linearVelocity += b2Cross(m_angularVelocity, m_sweep.c - oldCenter);
}

void b2Body::SetMassData(const b2MassData* massData)
{
	b2Assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

	if (m_type != b2_dynamicBody)
	{
		return;
	}

	m_invMass = 0.0f;
	m_I = 0.0f;
	m_invI = 0.0f;

	m_mass = massData->mass;
	if (m_mass <= 0.0f)
	{
		m_mass = 1.0f;
	}

	m_invMass = 1.0f / m_mass;

	if (massData->I > 0.0f && (m_flags & b2Body::e_fixedRotationFlag) == 0)
	{
		m_I = massData->I - m_mass * b2Dot(massData->center, massData->center);
		b2Assert(m_I > 0.0f);
		m_invI = 1.0f / m_I;
	}

	// Move center of mass.
	b2Vec2 oldCenter = m_sweep.c;
	m_sweep.localCenter =  massData->center;
	m_sweep.c0 = m_sweep.c = b2Mul(m_xf, m_sweep.localCenter);

	// Update center of mass velocity.
	m_linearVelocity += b2Cross(m_angularVelocity, m_sweep.c - oldCenter);
}

bool b2Body::ShouldCollide(const b2Body* other) const
{
	// At least one body should be dynamic.
	if (m_type != b2_dynamicBody && other->m_type != b2_dynamicBody)
	{
		return false;
	}

	// Does a joint prevent collision?
	for (b2JointEdge* jn = m_jointList; jn; jn = jn->next)
	{
		if (jn->other == other)
		{
			if (jn->joint->m_collideConnected == false)
			{
				return false;
			}
		}
	}

	return true;
}

void b2Body::SetTransform(const b2Vec2& position, float32 angle)
{
	b2Assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

	m_xf.q.Set(angle);
	m_xf.p = position;

	m_sweep.c = b2Mul(m_xf, m_sweep.localCenter);
	m_sweep.a = angle;

	m_sweep.c0 = m_sweep.c;
	m_sweep.a0 = angle;

	b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
	for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
	{
		f->Synchronize(broadPhase, m_xf, m_xf);
	}

	m_world->m_contactManager.FindNewContacts();
}

void b2Body::SynchronizeFixtures()
{
	b2Transform xf1;
	xf1.q.Set(m_sweep.a0);
	xf1.p = m_sweep.c0 - b2Mul(xf1.q, m_sweep.localCenter);

	b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
	for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
	{
		f->Synchronize(broadPhase, xf1, m_xf);
	}
}

void b2Body::SetActive(bool flag)
{
	b2Assert(m_world->IsLocked() == false);

	if (flag == IsActive())
	{
		return;
	}

	if (flag)
	{
		m_flags |= e_activeFlag;

		// Create all proxies.
		b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
		{
			f->CreateProxies(broadPhase, m_xf);
		}

		// Contacts are created the next time step.
	}
	else
	{
		m_flags &= ~e_activeFlag;

		// Destroy all proxies.
		b2BroadPhase* broadPhase = &m_world->m_contactManager.m_broadPhase;
		for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
		{
			f->DestroyProxies(broadPhase);
		}

		// Destroy the attached contacts.
		b2ContactEdge* ce = m_contactList;
		while (ce)
		{
			b2ContactEdge* ce0 = ce;
			ce = ce->next;
			m_world->m_contactManager.Destroy(ce0->contact);
		}
		m_contactList = NULL;
	}
}

void b2Body::Dump()
{
	int32 bodyIndex = m_islandIndex;

	b2Log("{\n");
	b2Log("  b2BodyDef bd;\n");
	b2Log("  bd.type = b2BodyType(%d);\n", m_type);
	b2Log("  bd.position.Set(%.15lef, %.15lef);\n", m_xf.p.x, m_xf.p.y);
	b2Log("  bd.angle = %.15lef;\n", m_sweep.a);
	b2Log("  bd.linearVelocity.Set(%.15lef, %.15lef);\n", m_linearVelocity.x, m_linearVelocity.y);
	b2Log("  bd.angularVelocity = %.15lef;\n", m_angularVelocity);
	b2Log("  bd.linearDamping = %.15lef;\n", m_linearDamping);
	b2Log("  bd.angularDamping = %.15lef;\n", m_angularDamping);
	b2Log("  bd.allowSleep = bool(%d);\n", m_flags & e_autoSleepFlag);
	b2Log("  bd.awake = bool(%d);\n", m_flags & e_awakeFlag);
	b2Log("  bd.fixedRotation = bool(%d);\n", m_flags & e_fixedRotationFlag);
	b2Log("  bd.bullet = bool(%d);\n", m_flags & e_bulletFlag);
	b2Log("  bd.active = bool(%d);\n", m_flags & e_activeFlag);
	b2Log("  bd.gravityScale = %.15lef;\n", m_gravityScale);
	b2Log("  bodies[%d] = m_world->CreateBody(&bd);\n", m_islandIndex);
	b2Log("\n");
	for (b2Fixture* f = m_fixtureList; f; f = f->m_next)
	{
		b2Log("  {\n");
		f->Dump(bodyIndex);
		b2Log("  }\n");
	}
	b2Log("}\n");
}