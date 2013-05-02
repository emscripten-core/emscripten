/*
* Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#include <Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.h>
#include <Box2D/Common/b2BlockAllocator.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Collision/Shapes/b2ChainShape.h>
#include <Box2D/Collision/Shapes/b2EdgeShape.h>

#include <new>
using namespace std;

b2Contact* b2ChainAndPolygonContact::Create(b2Fixture* fixtureA, int32 indexA, b2Fixture* fixtureB, int32 indexB, b2BlockAllocator* allocator)
{
	void* mem = allocator->Allocate(sizeof(b2ChainAndPolygonContact));
	return new (mem) b2ChainAndPolygonContact(fixtureA, indexA, fixtureB, indexB);
}

void b2ChainAndPolygonContact::Destroy(b2Contact* contact, b2BlockAllocator* allocator)
{
	((b2ChainAndPolygonContact*)contact)->~b2ChainAndPolygonContact();
	allocator->Free(contact, sizeof(b2ChainAndPolygonContact));
}

b2ChainAndPolygonContact::b2ChainAndPolygonContact(b2Fixture* fixtureA, int32 indexA, b2Fixture* fixtureB, int32 indexB)
: b2Contact(fixtureA, indexA, fixtureB, indexB)
{
	b2Assert(m_fixtureA->GetType() == b2Shape::e_chain);
	b2Assert(m_fixtureB->GetType() == b2Shape::e_polygon);
}

void b2ChainAndPolygonContact::Evaluate(b2Manifold* manifold, const b2Transform& xfA, const b2Transform& xfB)
{
	b2ChainShape* chain = (b2ChainShape*)m_fixtureA->GetShape();
	b2EdgeShape edge;
	chain->GetChildEdge(&edge, m_indexA);
	b2CollideEdgeAndPolygon(	manifold, &edge, xfA,
								(b2PolygonShape*)m_fixtureB->GetShape(), xfB);
}
