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

#ifndef PINBALL_H
#define PINBALL_H

/// This tests bullet collision and provides an example of a gameplay scenario.
/// This also uses a loop shape.
class Pinball : public Test
{
public:
	Pinball()
	{
		// Ground body
		b2Body* ground = NULL;
		{
			b2BodyDef bd;
			ground = m_world->CreateBody(&bd);

			b2Vec2 vs[5];
			vs[0].Set(0.0f, -2.0f);
			vs[1].Set(8.0f, 6.0f);
			vs[2].Set(8.0f, 20.0f);
			vs[3].Set(-8.0f, 20.0f);
			vs[4].Set(-8.0f, 6.0f);

			b2ChainShape loop;
			loop.CreateLoop(vs, 5);
			b2FixtureDef fd;
			fd.shape = &loop;
			fd.density = 0.0f;
			ground->CreateFixture(&fd);
		}

		// Flippers
		{
			b2Vec2 p1(-2.0f, 0.0f), p2(2.0f, 0.0f);

			b2BodyDef bd;
			bd.type = b2_dynamicBody;

			bd.position = p1;
			b2Body* leftFlipper = m_world->CreateBody(&bd);

			bd.position = p2;
			b2Body* rightFlipper = m_world->CreateBody(&bd);

			b2PolygonShape box;
			box.SetAsBox(1.75f, 0.1f);

			b2FixtureDef fd;
			fd.shape = &box;
			fd.density = 1.0f;

			leftFlipper->CreateFixture(&fd);
			rightFlipper->CreateFixture(&fd);

			b2RevoluteJointDef jd;
			jd.bodyA = ground;
			jd.localAnchorB.SetZero();
			jd.enableMotor = true;
			jd.maxMotorTorque = 1000.0f;
			jd.enableLimit = true;

			jd.motorSpeed = 0.0f;
			jd.localAnchorA = p1;
			jd.bodyB = leftFlipper;
			jd.lowerAngle = -30.0f * b2_pi / 180.0f;
			jd.upperAngle = 5.0f * b2_pi / 180.0f;
			m_leftJoint = (b2RevoluteJoint*)m_world->CreateJoint(&jd);

			jd.motorSpeed = 0.0f;
			jd.localAnchorA = p2;
			jd.bodyB = rightFlipper;
			jd.lowerAngle = -5.0f * b2_pi / 180.0f;
			jd.upperAngle = 30.0f * b2_pi / 180.0f;
			m_rightJoint = (b2RevoluteJoint*)m_world->CreateJoint(&jd);
		}

		// Circle character
		{
			b2BodyDef bd;
			bd.position.Set(1.0f, 15.0f);
			bd.type = b2_dynamicBody;
			bd.bullet = true;

			m_ball = m_world->CreateBody(&bd);

			b2CircleShape shape;
			shape.m_radius = 0.2f;

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.density = 1.0f;
			m_ball->CreateFixture(&fd);
		}

		m_button = false;
	}

	void Step(Settings* settings)
	{
		if (m_button)
		{
			m_leftJoint->SetMotorSpeed(20.0f);
			m_rightJoint->SetMotorSpeed(-20.0f);
		}
		else
		{
			m_leftJoint->SetMotorSpeed(-10.0f);
			m_rightJoint->SetMotorSpeed(10.0f);
		}

		Test::Step(settings);

		m_debugDraw.DrawString(5, m_textLine, "Press 'a' to control the flippers");
		m_textLine += 15;

	}

	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case 'a':
		case 'A':
			m_button = true;
			break;
		}
	}

	void KeyboardUp(unsigned char key)
	{
		switch (key)
		{
		case 'a':
		case 'A':
			m_button = false;
			break;
		}
	}

	static Test* Create()
	{
		return new Pinball;
	}

	b2RevoluteJoint* m_leftJoint;
	b2RevoluteJoint* m_rightJoint;
	b2Body* m_ball;
	bool m_button;
};

#endif
