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
#include <stdio.h>

/// This is a Hello World program for running a basic Bullet physics simulation

  btDefaultCollisionConfiguration* zz_collisionConfiguration;
  btCollisionDispatcher* zz_dispatcher;
  btBroadphaseInterface* zz_overlappingPairCache;
  btSequentialImpulseConstraintSolver* zz_solver;
  btDiscreteDynamicsWorld* zz_dynamicsWorld;

void zz_prepare()
{

  int i;

  ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
  zz_collisionConfiguration = new btDefaultCollisionConfiguration();

  ///use the default collision zz_dispatcher. For parallel processing you can use a diffent zz_dispatcher (see Extras/BulletMultiThreaded)
  zz_dispatcher = new  btCollisionDispatcher(zz_collisionConfiguration);

  ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  zz_overlappingPairCache = new btDbvtBroadphase();

  ///the default constraint zz_solver. For parallel processing you can use a different zz_solver (see Extras/BulletMultiThreaded)
  zz_solver = new btSequentialImpulseConstraintSolver;

  zz_dynamicsWorld = new btDiscreteDynamicsWorld(zz_dispatcher,zz_overlappingPairCache,zz_solver,zz_collisionConfiguration);

  zz_dynamicsWorld->setGravity(btVector3(0,-10,0));

  ///create a few basic rigid bodies
  btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(btScalar(0.),btScalar(1.),btScalar(0.)), 0);

  btTransform groundTransform;
  groundTransform.setIdentity();
  groundTransform.setOrigin(btVector3(0,-6,0));

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
    zz_dynamicsWorld->addRigidBody(body);
  }


  for (int i = 0; i < 5; i++)
  {
    //create a dynamic rigidbody

    btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    //btCollisionShape* colShape = new btSphereShape(btScalar(1.));

    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar  mass(1.f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0,0,0);
    if (isDynamic)
      colShape->calculateLocalInertia(mass,localInertia);

      startTransform.setOrigin(btVector3(0,4+i*2.1,0));
    
      //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
      btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
      btRigidBody* body = new btRigidBody(rbInfo);

      zz_dynamicsWorld->addRigidBody(body);
  }
}


/// Do some simulation


void zz_simulate(float diff) {
  zz_dynamicsWorld->stepSimulation(diff);
}

void zz_read(int j, btVector3& location, btQuaternion& rotation)
{
  {
    btCollisionObject* obj = zz_dynamicsWorld->getCollisionObjectArray()[j];
    btRigidBody* body = btRigidBody::upcast(obj);
    if (body && body->getMotionState())
    {
      btTransform trans;
      body->getMotionState()->getWorldTransform(trans);
      location = trans.getOrigin();
      rotation = trans.getRotation();
      //printf("world pos = %.5f,%.5f,%.5f\n",float(location.getX()),float(location.getY()),float(location.getZ()));
    }
  }
}

int main(int argc, char** argv)
{
  zz_prepare();
  zz_simulate(1/60.);//new btVector3);
  btVector3 location;
  btQuaternion rotation;
  zz_read(1, location, rotation);
  printf("world pos = %.5f,%.5f,%.5f   rot=%.5f,%.5f,%.5f,%.5f\n",float(location.getX()),float(location.getY()),float(location.getZ()),
      float(rotation.getX()), float(rotation.getY()), float(rotation.getZ()), float(rotation.getW())
    );
}

