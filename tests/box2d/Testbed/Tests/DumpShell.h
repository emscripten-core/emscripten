/*
* Copyright (c) 2011 Erin Catto http://www.box2d.org
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

#ifndef DUMP_SHELL_H
#define DUMP_SHELL_H

// This test holds worlds dumped using b2World::Dump.
class DumpShell : public Test
{
public:

	DumpShell()
	{

b2Vec2 g(0.000000000000000e+00f, 0.000000000000000e+00f);
m_world->SetGravity(g);
b2Body** bodies = (b2Body**)b2Alloc(3 * sizeof(b2Body*));
b2Joint** joints = (b2Joint**)b2Alloc(2 * sizeof(b2Joint*));
{
  b2BodyDef bd;
  bd.type = b2BodyType(2);
  bd.position.Set(1.304347801208496e+01f, 2.500000000000000e+00f);
  bd.angle = 0.000000000000000e+00f;
  bd.linearVelocity.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  bd.angularVelocity = 0.000000000000000e+00f;
  bd.linearDamping = 5.000000000000000e-01f;
  bd.angularDamping = 5.000000000000000e-01f;
  bd.allowSleep = bool(4);
  bd.awake = bool(2);
  bd.fixedRotation = bool(0);
  bd.bullet = bool(0);
  bd.active = bool(32);
  bd.gravityScale = 1.000000000000000e+00f;
  bodies[0] = m_world->CreateBody(&bd);

  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+00f;
    fd.restitution = 5.000000000000000e-01f;
    fd.density = 1.000000000000000e+01f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2PolygonShape shape;
    b2Vec2 vs[8];
    vs[0].Set(-6.900000095367432e+00f, -3.000000119209290e-01f);
    vs[1].Set(2.000000029802322e-01f, -3.000000119209290e-01f);
    vs[2].Set(2.000000029802322e-01f, 2.000000029802322e-01f);
    vs[3].Set(-6.900000095367432e+00f, 2.000000029802322e-01f);
    shape.Set(vs, 4);

    fd.shape = &shape;

    bodies[0]->CreateFixture(&fd);
  }
}
{
  b2BodyDef bd;
  bd.type = b2BodyType(2);
  bd.position.Set(8.478260636329651e-01f, 2.500000000000000e+00f);
  bd.angle = 0.000000000000000e+00f;
  bd.linearVelocity.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  bd.angularVelocity = 0.000000000000000e+00f;
  bd.linearDamping = 5.000000000000000e-01f;
  bd.angularDamping = 5.000000000000000e-01f;
  bd.allowSleep = bool(4);
  bd.awake = bool(2);
  bd.fixedRotation = bool(0);
  bd.bullet = bool(0);
  bd.active = bool(32);
  bd.gravityScale = 1.000000000000000e+00f;
  bodies[1] = m_world->CreateBody(&bd);

  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+00f;
    fd.restitution = 5.000000000000000e-01f;
    fd.density = 1.000000000000000e+01f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2PolygonShape shape;
    b2Vec2 vs[8];
    vs[0].Set(-3.228000104427338e-01f, -2.957000136375427e-01f);
    vs[1].Set(6.885900020599365e+00f, -3.641000092029572e-01f);
    vs[2].Set(6.907599925994873e+00f, 3.271999955177307e-01f);
    vs[3].Set(-3.228000104427338e-01f, 2.825999855995178e-01f);
    shape.Set(vs, 4);

    fd.shape = &shape;

    bodies[1]->CreateFixture(&fd);
  }
}

{
  b2BodyDef bd;
  bd.type = b2BodyType(0);
  bd.position.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  bd.angle = 0.000000000000000e+00f;
  bd.linearVelocity.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  bd.angularVelocity = 0.000000000000000e+00f;
  bd.linearDamping = 0.000000000000000e+00f;
  bd.angularDamping = 0.000000000000000e+00f;
  bd.allowSleep = bool(4);
  bd.awake = bool(2);
  bd.fixedRotation = bool(0);
  bd.bullet = bool(0);
  bd.active = bool(32);
  bd.gravityScale = 1.000000000000000e+00f;
  bodies[2] = m_world->CreateBody(&bd);

  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+01f;
    fd.restitution = 0.000000000000000e+00f;
    fd.density = 0.000000000000000e+00f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2EdgeShape shape;
    shape.m_radius = 9.999999776482582e-03f;
    shape.m_vertex0.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex1.Set(4.452173995971680e+01f, 1.669565200805664e+01f);
    shape.m_vertex2.Set(4.452173995971680e+01f, 0.000000000000000e+00f);
    shape.m_vertex3.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_hasVertex0 = bool(0);
    shape.m_hasVertex3 = bool(0);

    fd.shape = &shape;

    bodies[2]->CreateFixture(&fd);
  }
  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+01f;
    fd.restitution = 0.000000000000000e+00f;
    fd.density = 0.000000000000000e+00f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2EdgeShape shape;
    shape.m_radius = 9.999999776482582e-03f;
    shape.m_vertex0.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex1.Set(0.000000000000000e+00f, 1.669565200805664e+01f);
    shape.m_vertex2.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex3.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_hasVertex0 = bool(0);
    shape.m_hasVertex3 = bool(0);

    fd.shape = &shape;

    bodies[2]->CreateFixture(&fd);
  }
  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+01f;
    fd.restitution = 0.000000000000000e+00f;
    fd.density = 0.000000000000000e+00f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2EdgeShape shape;
    shape.m_radius = 9.999999776482582e-03f;
    shape.m_vertex0.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex1.Set(0.000000000000000e+00f, 1.669565200805664e+01f);
    shape.m_vertex2.Set(4.452173995971680e+01f, 1.669565200805664e+01f);
    shape.m_vertex3.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_hasVertex0 = bool(0);
    shape.m_hasVertex3 = bool(0);

    fd.shape = &shape;

    bodies[2]->CreateFixture(&fd);
  }
  {
    b2FixtureDef fd;
    fd.friction = 1.000000000000000e+01f;
    fd.restitution = 0.000000000000000e+00f;
    fd.density = 0.000000000000000e+00f;
    fd.isSensor = bool(0);
    fd.filter.categoryBits = uint16(1);
    fd.filter.maskBits = uint16(65535);
    fd.filter.groupIndex = int16(0);
    b2EdgeShape shape;
    shape.m_radius = 9.999999776482582e-03f;
    shape.m_vertex0.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex1.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_vertex2.Set(4.452173995971680e+01f, 0.000000000000000e+00f);
    shape.m_vertex3.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
    shape.m_hasVertex0 = bool(0);
    shape.m_hasVertex3 = bool(0);

    fd.shape = &shape;

    bodies[2]->CreateFixture(&fd);
  }
}

{
  b2PrismaticJointDef jd;
  jd.bodyA = bodies[1];
  jd.bodyB = bodies[0];
  jd.collideConnected = bool(0);
  jd.localAnchorA.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  jd.localAnchorB.Set(-1.219565200805664e+01f, 0.000000000000000e+00f);
  jd.localAxisA.Set(-1.219565200805664e+01f, 0.000000000000000e+00f);
  jd.referenceAngle = 0.000000000000000e+00f;
  jd.enableLimit = bool(1);
  jd.lowerTranslation = -2.000000000000000e+01f;
  jd.upperTranslation = 0.000000000000000e+00f;
  jd.enableMotor = bool(1);
  jd.motorSpeed = 0.000000000000000e+00f;
  jd.maxMotorForce = 1.000000000000000e+01f;
  joints[0] = m_world->CreateJoint(&jd);
}
{
  b2RevoluteJointDef jd;
  jd.bodyA = bodies[1];
  jd.bodyB = bodies[2];
  jd.collideConnected = bool(0);
  jd.localAnchorA.Set(0.000000000000000e+00f, 0.000000000000000e+00f);
  jd.localAnchorB.Set(8.478260636329651e-01f, 2.500000000000000e+00f);
  jd.referenceAngle = 0.000000000000000e+00f;
  jd.enableLimit = bool(0);
  jd.lowerAngle = 0.000000000000000e+00f;
  jd.upperAngle = 0.000000000000000e+00f;
  jd.enableMotor = bool(0);
  jd.motorSpeed = 0.000000000000000e+00f;
  jd.maxMotorTorque = 0.000000000000000e+00f;
  joints[1] = m_world->CreateJoint(&jd);
}
b2Free(joints);
b2Free(bodies);
joints = NULL;
bodies = NULL;


	}

	static Test* Create()
	{
		return new DumpShell;
	}
};

#endif
