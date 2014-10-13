/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "btBulletDynamicsCommon.h"

#include <iostream>

#include <emscripten.h>

/// This is a Hello World program for running a basic Bullet physics simulation

btDiscreteDynamicsWorld* dynamicsWorld;

extern "C" {
void EMSCRIPTEN_KEEPALIVE addBody();
}

int main(int argc, char** argv)
{

	int i;

	///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	btCollisionDispatcher* dispatcher = new	btCollisionDispatcher(collisionConfiguration);

	///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0,-10,0));

	///create a few basic rigid bodies
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));

	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	collisionShapes.push_back(groundShape);

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,-56,0));

	{
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}

  for (int i = 0; i < 20; i++) addBody();

  EM_ASM({
    startSimulation();
  });

  emscripten_exit_with_live_runtime();
}

/// Do some simulation

extern "C" {

void EMSCRIPTEN_KEEPALIVE addBody() {
	//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
	btCollisionShape* colShape = new btSphereShape(btScalar(1.));

	/// Create Dynamic Objects
	btTransform startTransform;

	btScalar	mass(1.f);

	btVector3 localInertia(0,0,0);
  colShape->calculateLocalInertia(mass,localInertia);

  static int i = 0, j = 0, k = 0, counter = 0;
  counter++;
  if (counter % 3 == 0) i++;
  else if (counter % 3 == 1) j++;
  else k++;

	{
		//create a dynamic rigidbody

		startTransform.setIdentity();

		startTransform.setOrigin(btVector3(i*2, 2 + j*2, k*2));
	
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
	}
}

void EMSCRIPTEN_KEEPALIVE simulate() {
  for (int k = 0; k < 10; k++) {
  	dynamicsWorld->stepSimulation(1.f/60.f,10);
  }

  static int i = 0;
  if (i++ % 10 == 0) {
    std::cout << ">>>>>>>>>>>> objs = " << dynamicsWorld->getNumCollisionObjects() << std::endl;
	  //print positions of an object
	  for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
	  {
		  btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
		  btRigidBody* body = btRigidBody::upcast(obj);
		  if (body && body->getMotionState())
		  {
			  btTransform trans;
			  body->getMotionState()->getWorldTransform(trans);
			  std::cout << ">>>>>>>>>>>>>>>>>>>>> world pos = " << float(trans.getOrigin().getX()) << ", " << float(trans.getOrigin().getY()) << ", " << float(trans.getOrigin().getZ()) << std::endl;
		  }
      break;
	  }
  }
}

}

