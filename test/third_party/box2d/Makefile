# Makefile for generating a Box2D library using Emscripten.

O = Box2D
OBJECTS = \
  $(O)/Collision/b2BroadPhase.o \
  $(O)/Collision/b2CollideCircle.o \
  $(O)/Collision/b2CollideEdge.o \
  $(O)/Collision/b2CollidePolygon.o \
  $(O)/Collision/b2Collision.o \
  $(O)/Collision/b2Distance.o \
  $(O)/Collision/b2DynamicTree.o \
  $(O)/Collision/b2TimeOfImpact.o \
  $(O)/Collision/Shapes/b2ChainShape.o \
  $(O)/Collision/Shapes/b2CircleShape.o \
  $(O)/Collision/Shapes/b2EdgeShape.o \
  $(O)/Collision/Shapes/b2PolygonShape.o \
  $(O)/Common/b2BlockAllocator.o \
  $(O)/Common/b2Draw.o \
  $(O)/Common/b2Math.o \
  $(O)/Common/b2Settings.o \
  $(O)/Common/b2StackAllocator.o \
  $(O)/Common/b2Timer.o \
  $(O)/Dynamics/b2Body.o \
  $(O)/Dynamics/b2ContactManager.o \
  $(O)/Dynamics/b2Fixture.o \
  $(O)/Dynamics/b2Island.o \
  $(O)/Dynamics/b2World.o \
  $(O)/Dynamics/b2WorldCallbacks.o \
  $(O)/Dynamics/Contacts/b2ChainAndCircleContact.o \
  $(O)/Dynamics/Contacts/b2ChainAndPolygonContact.o \
  $(O)/Dynamics/Contacts/b2CircleContact.o \
  $(O)/Dynamics/Contacts/b2Contact.o \
  $(O)/Dynamics/Contacts/b2ContactSolver.o \
  $(O)/Dynamics/Contacts/b2EdgeAndCircleContact.o \
  $(O)/Dynamics/Contacts/b2EdgeAndPolygonContact.o \
  $(O)/Dynamics/Contacts/b2PolygonAndCircleContact.o \
  $(O)/Dynamics/Contacts/b2PolygonContact.o \
  $(O)/Dynamics/Joints/b2DistanceJoint.o \
  $(O)/Dynamics/Joints/b2FrictionJoint.o \
  $(O)/Dynamics/Joints/b2GearJoint.o \
  $(O)/Dynamics/Joints/b2Joint.o \
  $(O)/Dynamics/Joints/b2MouseJoint.o \
  $(O)/Dynamics/Joints/b2PrismaticJoint.o \
  $(O)/Dynamics/Joints/b2PulleyJoint.o \
  $(O)/Dynamics/Joints/b2RevoluteJoint.o \
  $(O)/Dynamics/Joints/b2RopeJoint.o \
  $(O)/Dynamics/Joints/b2WeldJoint.o \
  $(O)/Dynamics/Joints/b2WheelJoint.o \
  $(O)/Rope/b2Rope.o

all: box2d.a

%.o: %.cpp
	$(CXX) $(CFLAGS) -I. $< -o $@ -O2 -c -fno-exceptions -fno-rtti -Wno-null-conversion

box2d.a: $(OBJECTS)
	$(AR) rvs $@ $(OBJECTS)

clean:
	rm box2d.a

