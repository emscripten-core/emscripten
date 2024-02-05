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

#ifndef CHARACTER_COLLISION_H
#define CHARACTER_COLLISION_H

/// This is a test of typical character collision scenarios. This does not
/// show how you should implement a character in your application.
/// Instead this is used to test smooth collision on edge chains.
class CharacterCollision : public Test
{
public:
	CharacterCollision()
	{
		// Ground body
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2EdgeShape shape;
			shape.Set(b2Vec2(-20.0f, 0.0f), b2Vec2(20.0f, 0.0f));
			ground->CreateFixture(&shape, 0.0f);
		}

		// Collinear edges with no adjacency information.
		// This shows the problematic case where a box shape can hit
		// an internal vertex.
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2EdgeShape shape;
			shape.Set(b2Vec2(-8.0f, 1.0f), b2Vec2(-6.0f, 1.0f));
			ground->CreateFixture(&shape, 0.0f);
			shape.Set(b2Vec2(-6.0f, 1.0f), b2Vec2(-4.0f, 1.0f));
			ground->CreateFixture(&shape, 0.0f);
			shape.Set(b2Vec2(-4.0f, 1.0f), b2Vec2(-2.0f, 1.0f));
			ground->CreateFixture(&shape, 0.0f);
		}

		// Chain shape
		{
			b2BodyDef bd;
			bd.angle = 0.25f * b2_pi;
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 vs[4];
			vs[0].Set(5.0f, 7.0f);
			vs[1].Set(6.0f, 8.0f);
			vs[2].Set(7.0f, 8.0f);
			vs[3].Set(8.0f, 7.0f);
			b2ChainShape shape;
			shape.CreateChain(vs, 4);
			ground->CreateFixture(&shape, 0.0f);
		}

		// Square tiles. This shows that adjacency shapes may
		// have non-smooth collision. There is no solution
		// to this problem.
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(1.0f, 1.0f, b2Vec2(4.0f, 3.0f), 0.0f);
			ground->CreateFixture(&shape, 0.0f);
			shape.SetAsBox(1.0f, 1.0f, b2Vec2(6.0f, 3.0f), 0.0f);
			ground->CreateFixture(&shape, 0.0f);
			shape.SetAsBox(1.0f, 1.0f, b2Vec2(8.0f, 3.0f), 0.0f);
			ground->CreateFixture(&shape, 0.0f);
		}

		// Square made from an edge loop. Collision should be smooth.
		{
			b2BodyDef bd;
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 vs[4];
			vs[0].Set(-1.0f, 3.0f);
			vs[1].Set(1.0f, 3.0f);
			vs[2].Set(1.0f, 5.0f);
			vs[3].Set(-1.0f, 5.0f);
			b2ChainShape shape;
			shape.CreateLoop(vs, 4);
			ground->CreateFixture(&shape, 0.0f);
		}

		// Edge loop. Collision should be smooth.
		{
			b2BodyDef bd;
			bd.position.Set(-10.0f, 4.0f);
			b2Body* ground = m_world->CreateBody(&bd);

			b2Vec2 vs[10];
			vs[0].Set(0.0f, 0.0f);
			vs[1].Set(6.0f, 0.0f);
			vs[2].Set(6.0f, 2.0f);
			vs[3].Set(4.0f, 1.0f);
			vs[4].Set(2.0f, 2.0f);
			vs[5].Set(0.0f, 2.0f);
			vs[6].Set(-2.0f, 2.0f);
			vs[7].Set(-4.0f, 3.0f);
			vs[8].Set(-6.0f, 2.0f);
			vs[9].Set(-6.0f, 0.0f);
			b2ChainShape shape;
			shape.CreateLoop(vs, 10);
			ground->CreateFixture(&shape, 0.0f);
		}

		// Square character 1
		{
			b2BodyDef bd;
			bd.position.Set(-3.0f, 8.0f);
			bd.type = b2_dynamicBody;
			bd.fixedRotation = true;
			bd.allowSleep = false;

			b2Body* body = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.5f, 0.5f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			body->CreateFixture(&fd);
		}

		// Square character 2
		{
			b2BodyDef bd;
			bd.position.Set(-5.0f, 5.0f);
			bd.type = b2_dynamicBody;
			bd.fixedRotation = true;
			bd.allowSleep = false;

			b2Body* body = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.25f, 0.25f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			body->CreateFixture(&fd);
		}

		// Hexagon character
		{
			b2BodyDef bd;
			bd.position.Set(-5.0f, 8.0f);
			bd.type = b2_dynamicBody;
			bd.fixedRotation = true;
			bd.allowSleep = false;

			b2Body* body = m_world->CreateBody(&bd);

			float32 angle = 0.0f;
			float32 delta = b2_pi / 3.0f;
			b2Vec2 vertices[6];
			for (int32 i = 0; i < 6; ++i)
			{
				vertices[i].Set(0.5f * cosf(angle), 0.5f * sinf(angle));
				angle += delta;
			}

			b2PolygonShape shape;
			shape.Set(vertices, 6);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			body->CreateFixture(&fd);
		}

		// Circle character
		{
			b2BodyDef bd;
			bd.position.Set(3.0f, 5.0f);
			bd.type = b2_dynamicBody;
			bd.fixedRotation = true;
			bd.allowSleep = false;

			b2Body* body = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = 0.5f;

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			body->CreateFixture(&fd);
		}

		// Circle character
		{
			b2BodyDef bd;
			bd.position.Set(-7.0f, 6.0f);
			bd.type = b2_dynamicBody;
			bd.allowSleep = false;

			m_character = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = 0.25f;

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 20.0f;
			fd.friction = 1.0f;
			m_character->CreateFixture(&fd);
		}
	}

	void Step(Settings* settings)
	{
		b2Vec2 v = m_character->GetLinearVelocity();
		v.x = -5.0f;
		m_character->SetLinearVelocity(v);

		Test::Step(settings);
		m_debugDraw.DrawString(5, m_textLine, "This tests various character collision shapes.");
		m_textLine += 15;
		m_debugDraw.DrawString(5, m_textLine, "Limitation: square and hexagon can snag on aligned boxes.");
		m_textLine += 15;
		m_debugDraw.DrawString(5, m_textLine, "Feature: edge chains have smooth collision inside and out.");
		m_textLine += 15;
	}

	static Test* Create()
	{
		return new CharacterCollision;
	}

	b2Body* m_character;
};

#endif
