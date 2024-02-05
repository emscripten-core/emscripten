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

#ifndef EDGE_TEST_H
#define EDGE_TEST_H

class EdgeTest : public Test
{
public:

	EdgeTest()
	{
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 v1(-10.0f, 0.0f), v2(-7.0f, -2.0f), v3(-4.0f, 0.0f);
			b2Vec2 v4(0.0f, 0.0f), v5(4.0f, 0.0f), v6(7.0f, 2.0f), v7(10.0f, 0.0f);

			b2EdgeShape shape;

			shape.Set(v1, v2);
			shape.m_hasVertex3 = true;
			shape.m_vertex3 = v3;
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(v2, v3);
			shape.m_hasVertex0 = true;
			shape.m_hasVertex3 = true;
			shape.m_vertex0 = v1;
			shape.m_vertex3 = v4;
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(v3, v4);
			shape.m_hasVertex0 = true;
			shape.m_hasVertex3 = true;
			shape.m_vertex0 = v2;
			shape.m_vertex3 = v5;
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(v4, v5);
			shape.m_hasVertex0 = true;
			shape.m_hasVertex3 = true;
			shape.m_vertex0 = v3;
			shape.m_vertex3 = v6;
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(v5, v6);
			shape.m_hasVertex0 = true;
			shape.m_hasVertex3 = true;
			shape.m_vertex0 = v4;
			shape.m_vertex3 = v7;
			ground->CreateFixture(&shape, 0.0f);

			shape.Set(v6, v7);
			shape.m_hasVertex0 = true;
			shape.m_vertex0 = v5;
			ground->CreateFixture(&shape, 0.0f);
		}

		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(-0.5f, 0.6f);
			bd.allowSleep = false;
			b2Body* body = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = 0.5f;

			body->CreateFixture(&shape, 1.0f);
		}

		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(1.0f, 0.6f);
			bd.allowSleep = false;
			b2Body* body = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.5f);

			body->CreateFixture(&shape, 1.0f);
		}
	}

	static Test* Create()
	{
		return new EdgeTest;
	}
};

#endif
