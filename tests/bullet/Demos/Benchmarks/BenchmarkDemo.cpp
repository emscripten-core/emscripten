/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



// Collision Radius
#define COLLISION_RADIUS 0.0f

#include "BenchmarkDemo.h"
#ifdef USE_GRAPHICAL_BENCHMARK
#include "GlutStuff.h"
#endif //USE_GRAPHICAL_BENCHMARK

///btBulletDynamicsCommon.h is the main Bullet include file, contains most common include files.
#include "btBulletDynamicsCommon.h"
#include <stdio.h> //printf debugging
#include "Taru.mdl"
#include "landscape.mdl"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#ifdef USE_PARALLEL_DISPATCHER_BENCHMARK
#include "BulletMultiThreaded/SpuGatheringCollisionDispatcher.h"
#include "BulletMultiThreaded/SequentialThreadSupport.h"
#include "BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"
#endif

#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"


#ifdef USE_PARALLEL_DISPATCHER_BENCHMARK
#ifdef _WIN32
#include "BulletMultiThreaded/Win32ThreadSupport.h"
#elif defined (USE_PTHREADS)
#include "BulletMultiThreaded/PosixThreadSupport.h"
#endif
#include "BulletMultiThreaded/SpuGatheringCollisionDispatcher.h"
#include "BulletMultiThreaded/btParallelConstraintSolver.h"




btThreadSupportInterface* createSolverThreadSupport(int maxNumThreads)
{
//#define SEQUENTIAL
#ifdef SEQUENTIAL
	SequentialThreadSupport::SequentialThreadConstructionInfo tci("solverThreads",SolverThreadFunc,SolverlsMemoryFunc);
	SequentialThreadSupport* threadSupport = new SequentialThreadSupport(tci);
	threadSupport->startSPU();
#else

#ifdef _WIN32
	Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("solverThreads",SolverThreadFunc,SolverlsMemoryFunc,maxNumThreads);
	Win32ThreadSupport* threadSupport = new Win32ThreadSupport(threadConstructionInfo);
	threadSupport->startSPU();
#elif defined (USE_PTHREADS)
	PosixThreadSupport::ThreadConstructionInfo solverConstructionInfo("solver", SolverThreadFunc,
																	  SolverlsMemoryFunc, maxNumThreads);
	
	PosixThreadSupport* threadSupport = new PosixThreadSupport(solverConstructionInfo);
	
#else
	SequentialThreadSupport::SequentialThreadConstructionInfo tci("solverThreads",SolverThreadFunc,SolverlsMemoryFunc);
	SequentialThreadSupport* threadSupport = new SequentialThreadSupport(tci);
	threadSupport->startSPU();
#endif
	
#endif

	return threadSupport;
}
#endif

class btRaycastBar2
{
public:
	btVector3 source[NUMRAYS];
	btVector3 dest[NUMRAYS];
	btVector3 direction[NUMRAYS];
	btVector3 hit[NUMRAYS];
	btVector3 normal[NUMRAYS];

	int frame_counter;
	int ms;
	int sum_ms;
	int sum_ms_samples;
	int min_ms;
	int max_ms;

#ifdef USE_BT_CLOCK
	btClock frame_timer;
#endif //USE_BT_CLOCK

	btScalar dx;
	btScalar min_x;
	btScalar max_x;
	btScalar max_y;
	btScalar sign;

	btRaycastBar2 ()
	{
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
	}



	btRaycastBar2 (btScalar ray_length, btScalar z,btScalar max_y)
	{
		frame_counter = 0;
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
		dx = 10.0;
		min_x = 0;
		max_x = 0;
		this->max_y = max_y;
		sign = 1.0;
		btScalar dalpha = 2*SIMD_2_PI/NUMRAYS;
		for (int i = 0; i < NUMRAYS; i++)
		{
			btScalar alpha = dalpha * i;
			// rotate around by alpha degrees y 
			btQuaternion q(btVector3(0.0, 1.0, 0.0), alpha);
			direction[i] = btVector3(1.0, 0.0, 0.0);
			direction[i] = quatRotate(q , direction[i]);
			direction[i] = direction[i] * ray_length;
			
			
			source[i] = btVector3(min_x, max_y, z);
			dest[i] = source[i] + direction[i];
			dest[i][1]=-1000;
			normal[i] = btVector3(1.0, 0.0, 0.0);
		}
	}

	void move (btScalar dt)
	{
		if (dt > btScalar(1.0/60.0))
			dt = btScalar(1.0/60.0);
		for (int i = 0; i < NUMRAYS; i++)
		{
			source[i][0] += dx * dt * sign;
			dest[i][0] += dx * dt * sign;
		}
		if (source[0][0] < min_x)
			sign = 1.0;
		else if (source[0][0] > max_x)
			sign = -1.0;
	}

	void cast (btCollisionWorld* cw)
	{
#ifdef USE_BT_CLOCK
		frame_timer.reset ();
#endif //USE_BT_CLOCK

#ifdef BATCH_RAYCASTER
		if (!gBatchRaycaster)
			return;

		gBatchRaycaster->clearRays ();
		for (int i = 0; i < NUMRAYS; i++)
		{
			gBatchRaycaster->addRay (source[i], dest[i]);
		}
		gBatchRaycaster->performBatchRaycast ();
		for (int i = 0; i < gBatchRaycaster->getNumRays (); i++)
		{
				const SpuRaycastTaskWorkUnitOut& out = (*gBatchRaycaster)[i];
				hit[i].setInterpolate3(source[i],dest[i],out.hitFraction);
				normal[i] = out.hitNormal;
				normal[i].normalize ();
		}
#else
		for (int i = 0; i < NUMRAYS; i++)
		{
			btCollisionWorld::ClosestRayResultCallback cb(source[i], dest[i]);
			
			cw->rayTest (source[i], dest[i], cb);
			if (cb.hasHit ())
			{
				hit[i] = cb.m_hitPointWorld;
				normal[i] = cb.m_hitNormalWorld;
				normal[i].normalize ();
			} else {
				hit[i] = dest[i];
				normal[i] = btVector3(1.0, 0.0, 0.0);
			}

		}
#ifdef USE_BT_CLOCK
		ms += frame_timer.getTimeMilliseconds ();
#endif //USE_BT_CLOCK
		frame_counter++;
		if (frame_counter > 50)
		{
			min_ms = ms < min_ms ? ms : min_ms;
			max_ms = ms > max_ms ? ms : max_ms;
			sum_ms += ms;
			sum_ms_samples++;
			btScalar mean_ms = (btScalar)sum_ms/(btScalar)sum_ms_samples;
			//printf("%d rays in %d ms %d %d %f\n", NUMRAYS * frame_counter, ms, min_ms, max_ms, mean_ms);
			ms = 0;
			frame_counter = 0;
		}
#endif
	}

	void draw ()
	{
#ifdef USE_GRAPHICAL_BENCHMARK
		glDisable (GL_LIGHTING);
		glColor3f (0.0, 1.0, 0.0);
		glBegin (GL_LINES);
		int i;

		for (i = 0; i < NUMRAYS; i++)
		{
			glVertex3f (source[i][0], source[i][1], source[i][2]);
			glVertex3f (hit[i][0], hit[i][1], hit[i][2]);
		}
		glEnd ();
		glColor3f (1.0, 1.0, 1.0);
		glBegin (GL_LINES);
		for (i = 0; i < NUMRAYS; i++)
		{
			glVertex3f (hit[i][0], hit[i][1], hit[i][2]);
			glVertex3f (hit[i][0] + normal[i][0], hit[i][1] + normal[i][1], hit[i][2] + normal[i][2]);
		}
		glEnd ();
		glColor3f (0.0, 1.0, 1.0);
		glBegin (GL_POINTS);
		for ( i = 0; i < NUMRAYS; i++)
		{
			glVertex3f (hit[i][0], hit[i][1], hit[i][2]);
		}
		glEnd ();
		glEnable (GL_LIGHTING);
#endif //USE_GRAPHICAL_BENCHMARK

	}
};


static btRaycastBar2 raycastBar;


void BenchmarkDemo::clientMoveAndDisplay()
{
#ifdef USE_GRAPHICAL_BENCHMARK
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
#endif //USE_GRAPHICAL_BENCHMARK

	//simple dynamics world doesn't handle fixed-time-stepping
	//float ms = getDeltaTimeMicroseconds();
	
	///step the simulation
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(btScalar(1./60.));
		//optional but useful: debug drawing
		m_dynamicsWorld->debugDrawWorld();
	}
		
	if (m_benchmark==7)
	{
		castRays();

		raycastBar.draw();
	
	}

	renderme(); 

#ifdef USE_GRAPHICAL_BENCHMARK
	glFlush();

	swapBuffers();
#endif //USE_GRAPHICAL_BENCHMARK

}



void BenchmarkDemo::displayCallback(void) 
{

#ifdef USE_GRAPHICAL_BENCHMARK
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	
	renderme();

	//optional but useful: debug drawing to detect problems
	if (m_dynamicsWorld)
		m_dynamicsWorld->debugDrawWorld();

	glFlush();
	swapBuffers();
#endif //USE_GRAPHICAL_BENCHMARK
}




void	BenchmarkDemo::initPhysics()
{

	setCameraDistance(btScalar(100.));

	///collision configuration contains default setup for memory, collision setup
	btDefaultCollisionConstructionInfo cci;
	cci.m_defaultMaxPersistentManifoldPoolSize = 32768;
	m_collisionConfiguration = new btDefaultCollisionConfiguration(cci);

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);
	
	m_dispatcher->setDispatcherFlags(btCollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);

#if USE_PARALLEL_DISPATCHER_BENCHMARK

	int maxNumOutstandingTasks = 4;
#ifdef _WIN32
	Win32ThreadSupport* threadSupportCollision = new Win32ThreadSupport(Win32ThreadSupport::Win32ThreadConstructionInfo(	"collision",processCollisionTask,	createCollisionLocalStoreMemory,maxNumOutstandingTasks));
#elif defined (USE_PTHREADS)
        PosixThreadSupport::ThreadConstructionInfo collisionConstructionInfo( "collision",processCollisionTask,       createCollisionLocalStoreMemory,maxNumOutstandingTasks);
	PosixThreadSupport* threadSupportCollision = new PosixThreadSupport(collisionConstructionInfo);
#endif
	//SequentialThreadSupport::SequentialThreadConstructionInfo sci("spuCD",	processCollisionTask,	createCollisionLocalStoreMemory);
	//SequentialThreadSupport* seq = new SequentialThreadSupport(sci);
	m_dispatcher = new	SpuGatheringCollisionDispatcher(threadSupportCollision,1,m_collisionConfiguration);
#endif


	///the maximum size of the collision world. Make sure objects stay within these boundaries
	///Don't make the world AABB size too large, it will harm simulation quality and performance
	btVector3 worldAabbMin(-1000,-1000,-1000);
	btVector3 worldAabbMax(1000,1000,1000);
	
	btHashedOverlappingPairCache* pairCache = new btHashedOverlappingPairCache();
	m_overlappingPairCache = new btAxisSweep3(worldAabbMin,worldAabbMax,3500,pairCache);
//	m_overlappingPairCache = new btSimpleBroadphase();
//	m_overlappingPairCache = new btDbvtBroadphase();
	

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
#ifdef USE_PARALLEL_DISPATCHER_BENCHMARK

	btThreadSupportInterface* thread = createSolverThreadSupport(4);
	btConstraintSolver* sol = new btParallelConstraintSolver(thread);
#else
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
#endif //USE_PARALLEL_DISPATCHER_BENCHMARK
	
	
	m_solver = sol;

	btDiscreteDynamicsWorld* dynamicsWorld;
	m_dynamicsWorld = dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_overlappingPairCache,m_solver,m_collisionConfiguration);
	
#ifdef USE_PARALLEL_DISPATCHER_BENCHMARK
	dynamicsWorld->getSimulationIslandManager()->setSplitIslands(false);
#endif //USE_PARALLEL_DISPATCHER_BENCHMARK

	///the following 3 lines increase the performance dramatically, with a little bit of loss of quality
	m_dynamicsWorld->getSolverInfo().m_solverMode |=SOLVER_ENABLE_FRICTION_DIRECTION_CACHING; //don't recalculate friction values each frame
	dynamicsWorld->getSolverInfo().m_numIterations = 5; //few solver iterations 
	m_defaultContactProcessingThreshold = 0.f;//used when creating bodies: body->setContactProcessingThreshold(...);
	

	m_dynamicsWorld->setGravity(btVector3(0,-10,0));

	if (m_benchmark<5)
	{
		///create a few basic rigid bodies
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(250.),btScalar(50.),btScalar(250.)));
	//	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),0);
		
		m_collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0,-50,0));

		//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
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
			m_dynamicsWorld->addRigidBody(body);
		}
	}

	switch (m_benchmark)
	{
		case 1:
			{
				createTest1();
				break;
			}
		case 2:
			{
				createTest2();
				break;
			}
		case 3:
			{
				createTest3();
				break;
			}
		case 4:
			{
				createTest4();
				break;
			}
		case 5:
			{
				createTest5();
				break;
			}
		case 6:
		{
			createTest6();
			break;
		}
		case 7:
		{
			createTest7();
			break;
		}


	default:
		{
		}			
	}


	clientResetScene();
}
	

void	BenchmarkDemo::createTest1()
{
	// 3000
	int size = 8;
	const float cubeSize = 1.0f;
	float spacing = cubeSize;
	btVector3 pos(0.0f, cubeSize * 2,0.f);
	float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

	btBoxShape* blockShape = new btBoxShape(btVector3(cubeSize-COLLISION_RADIUS,cubeSize-COLLISION_RADIUS,cubeSize-COLLISION_RADIUS));
	btVector3 localInertia(0,0,0);
	float mass = 2.f;
	blockShape->calculateLocalInertia(mass,localInertia);
	
	btTransform trans;
	trans.setIdentity();

	for(int k=0;k<47;k++) {
		for(int j=0;j<size;j++) {
			pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
			for(int i=0;i<size;i++) {
				pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);

				trans.setOrigin(pos);
				btRigidBody* cmbody;
				cmbody= localCreateRigidBody(mass,trans,blockShape);
			}
		}
		offset -= 0.05f * spacing * (size-1);
//		spacing *= 1.01f;
		pos[1] += (cubeSize * 2.0f + spacing);
	}
}


///////////////////////////////////////////////////////////////////////////////
// Pyramid 3

void BenchmarkDemo::createWall(const btVector3& offsetPosition,int stackSize,const btVector3& boxSize)
{
	
	btBoxShape* blockShape = new btBoxShape(btVector3(boxSize[0]-COLLISION_RADIUS,boxSize[1]-COLLISION_RADIUS,boxSize[2]-COLLISION_RADIUS));

	float mass = 1.f;
	btVector3 localInertia(0,0,0);
	blockShape->calculateLocalInertia(mass,localInertia);

//	btScalar  diffX = boxSize[0] * 1.0f;
	btScalar  diffY = boxSize[1] * 1.0f;
	btScalar  diffZ = boxSize[2] * 1.0f;

	btScalar  offset = -stackSize * (diffZ * 2.0f) * 0.5f;
	btVector3 pos(0.0f, diffY, 0.0f);

	btTransform trans;
	trans.setIdentity();

	while(stackSize) {
		for(int i=0;i<stackSize;i++) {
			pos[2] = offset + (float)i * (diffZ * 2.0f);

		trans.setOrigin(offsetPosition + pos);
		localCreateRigidBody(mass,trans,blockShape);
		
		}
		offset += diffZ;
		pos[1] += (diffY * 2.0f);
		stackSize--;
	}
}

void BenchmarkDemo::createPyramid(const btVector3& offsetPosition,int stackSize,const btVector3& boxSize)
{
	btScalar space = 0.0001f;
	
	btVector3 pos(0.0f, boxSize[1], 0.0f);

	btBoxShape* blockShape = new btBoxShape(btVector3(boxSize[0]-COLLISION_RADIUS,boxSize[1]-COLLISION_RADIUS,boxSize[2]-COLLISION_RADIUS));
	btTransform trans;
	trans.setIdentity();

	btScalar mass = 1.f;
	btVector3 localInertia(0,0,0);
	blockShape->calculateLocalInertia(mass,localInertia);

	
	btScalar diffX = boxSize[0]*1.02f;
	btScalar diffY = boxSize[1]*1.02f;
	btScalar diffZ = boxSize[2]*1.02f;
	
	btScalar offsetX = -stackSize * (diffX * 2.0f + space) * 0.5f;
	btScalar offsetZ = -stackSize * (diffZ * 2.0f + space) * 0.5f;
	while(stackSize) {
		for(int j=0;j<stackSize;j++) {
			pos[2] = offsetZ + (float)j * (diffZ * 2.0f + space);
			for(int i=0;i<stackSize;i++) {
				pos[0] = offsetX + (float)i * (diffX * 2.0f + space);
				trans.setOrigin(offsetPosition + pos);
				this->localCreateRigidBody(mass,trans,blockShape);


			}
		}
		offsetX += diffX;
		offsetZ += diffZ;
		pos[1] += (diffY * 2.0f + space);
		stackSize--;
	}

}

 const btVector3 rotate( const btQuaternion& quat, const btVector3 & vec )
{
    float tmpX, tmpY, tmpZ, tmpW;
    tmpX = ( ( ( quat.getW() * vec.getX() ) + ( quat.getY() * vec.getZ() ) ) - ( quat.getZ() * vec.getY() ) );
    tmpY = ( ( ( quat.getW() * vec.getY() ) + ( quat.getZ() * vec.getX() ) ) - ( quat.getX() * vec.getZ() ) );
    tmpZ = ( ( ( quat.getW() * vec.getZ() ) + ( quat.getX() * vec.getY() ) ) - ( quat.getY() * vec.getX() ) );
    tmpW = ( ( ( quat.getX() * vec.getX() ) + ( quat.getY() * vec.getY() ) ) + ( quat.getZ() * vec.getZ() ) );
    return btVector3(
        ( ( ( ( tmpW * quat.getX() ) + ( tmpX * quat.getW() ) ) - ( tmpY * quat.getZ() ) ) + ( tmpZ * quat.getY() ) ),
        ( ( ( ( tmpW * quat.getY() ) + ( tmpY * quat.getW() ) ) - ( tmpZ * quat.getX() ) ) + ( tmpX * quat.getZ() ) ),
        ( ( ( ( tmpW * quat.getZ() ) + ( tmpZ * quat.getW() ) ) - ( tmpX * quat.getY() ) ) + ( tmpY * quat.getX() ) )
    );
}

void BenchmarkDemo::createTowerCircle(const btVector3& offsetPosition,int stackSize,int rotSize,const btVector3& boxSize)
{

	btBoxShape* blockShape = new btBoxShape(btVector3(boxSize[0]-COLLISION_RADIUS,boxSize[1]-COLLISION_RADIUS,boxSize[2]-COLLISION_RADIUS));

	btTransform trans;
	trans.setIdentity();

	float mass = 1.f;
	btVector3 localInertia(0,0,0);
	blockShape->calculateLocalInertia(mass,localInertia);


	float radius = 1.3f * rotSize * boxSize[0] / SIMD_PI;

	// create active boxes
	btQuaternion rotY(0,1,0,0);
	float posY = boxSize[1];

	for(int i=0;i<stackSize;i++) {
		for(int j=0;j<rotSize;j++) {
		

			trans.setOrigin(offsetPosition+  rotate(rotY,btVector3(0.0f , posY, radius)));
			trans.setRotation(rotY);
			localCreateRigidBody(mass,trans,blockShape);

			rotY *= btQuaternion(btVector3(0,1,0),SIMD_PI/(rotSize*btScalar(0.5)));
		}

		posY += boxSize[1] * 2.0f;
		rotY *= btQuaternion(btVector3(0,1,0),SIMD_PI/(float)rotSize);
	}
	
}

void	BenchmarkDemo::createTest2()
{
	setCameraDistance(btScalar(50.));
	const float cubeSize = 1.0f;

	createPyramid(btVector3(-20.0f,0.0f,0.0f),12,btVector3(cubeSize,cubeSize,cubeSize));
	createWall(btVector3(-2.0f,0.0f,0.0f),12,btVector3(cubeSize,cubeSize,cubeSize));
	createWall(btVector3(4.0f,0.0f,0.0f),12,btVector3(cubeSize,cubeSize,cubeSize));
	createWall(btVector3(10.0f,0.0f,0.0f),12,btVector3(cubeSize,cubeSize,cubeSize));
	createTowerCircle(btVector3(25.0f,0.0f,0.0f),8,24,btVector3(cubeSize,cubeSize,cubeSize));
	
}




// Enrico: Shouldn't these three variables be real constants and not defines?

#ifndef M_PI
#define M_PI       btScalar(3.14159265358979323846)
#endif

#ifndef M_PI_2
#define M_PI_2     btScalar(1.57079632679489661923)
#endif

#ifndef M_PI_4
#define M_PI_4     btScalar(0.785398163397448309616)
#endif

class RagDoll
{
	enum
	{
		BODYPART_PELVIS = 0,
		BODYPART_SPINE,
		BODYPART_HEAD,

		BODYPART_LEFT_UPPER_LEG,
		BODYPART_LEFT_LOWER_LEG,

		BODYPART_RIGHT_UPPER_LEG,
		BODYPART_RIGHT_LOWER_LEG,

		BODYPART_LEFT_UPPER_ARM,
		BODYPART_LEFT_LOWER_ARM,

		BODYPART_RIGHT_UPPER_ARM,
		BODYPART_RIGHT_LOWER_ARM,

		BODYPART_COUNT
	};

	enum
	{
		JOINT_PELVIS_SPINE = 0,
		JOINT_SPINE_HEAD,

		JOINT_LEFT_HIP,
		JOINT_LEFT_KNEE,

		JOINT_RIGHT_HIP,
		JOINT_RIGHT_KNEE,

		JOINT_LEFT_SHOULDER,
		JOINT_LEFT_ELBOW,

		JOINT_RIGHT_SHOULDER,
		JOINT_RIGHT_ELBOW,

		JOINT_COUNT
	};

	btDynamicsWorld* m_ownerWorld;
	btCollisionShape* m_shapes[BODYPART_COUNT];
	btRigidBody* m_bodies[BODYPART_COUNT];
	btTypedConstraint* m_joints[JOINT_COUNT];

	btRigidBody* localCreateRigidBody (btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
	{
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			shape->calculateLocalInertia(mass,localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}

public:
	RagDoll (btDynamicsWorld* ownerWorld, const btVector3& positionOffset,btScalar scale)
		: m_ownerWorld (ownerWorld)
	{
		// Setup the geometry
		m_shapes[BODYPART_PELVIS] = new btCapsuleShape(btScalar(0.15)*scale, btScalar(0.20)*scale);
		m_shapes[BODYPART_SPINE] = new btCapsuleShape(btScalar(0.15)*scale, btScalar(0.28)*scale);
		m_shapes[BODYPART_HEAD] = new btCapsuleShape(btScalar(0.10)*scale, btScalar(0.05)*scale);
		m_shapes[BODYPART_LEFT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07)*scale, btScalar(0.45)*scale);
		m_shapes[BODYPART_LEFT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05)*scale, btScalar(0.37)*scale);
		m_shapes[BODYPART_RIGHT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07)*scale, btScalar(0.45)*scale);
		m_shapes[BODYPART_RIGHT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05)*scale, btScalar(0.37)*scale);
		m_shapes[BODYPART_LEFT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05)*scale, btScalar(0.33)*scale);
		m_shapes[BODYPART_LEFT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04)*scale, btScalar(0.25)*scale);
		m_shapes[BODYPART_RIGHT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05)*scale, btScalar(0.33)*scale);
		m_shapes[BODYPART_RIGHT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04)*scale, btScalar(0.25)*scale);

		// Setup all the rigid bodies
		btTransform offset; offset.setIdentity();
		offset.setOrigin(positionOffset);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.), btScalar(1.), btScalar(0.)));
		m_bodies[BODYPART_PELVIS] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_PELVIS]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.), btScalar(1.2), btScalar(0.)));
		m_bodies[BODYPART_SPINE] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_SPINE]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.), btScalar(1.6), btScalar(0.)));
		m_bodies[BODYPART_HEAD] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_HEAD]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(-0.18), btScalar(0.65), btScalar(0.)));
		m_bodies[BODYPART_LEFT_UPPER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(-0.18), btScalar(0.2), btScalar(0.)));
		m_bodies[BODYPART_LEFT_LOWER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.18), btScalar(0.65), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_UPPER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.18), btScalar(0.2), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_LOWER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(-0.35), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,M_PI_2);
		m_bodies[BODYPART_LEFT_UPPER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(-0.7), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,M_PI_2);
		m_bodies[BODYPART_LEFT_LOWER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_LOWER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.35), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,-M_PI_2);
		m_bodies[BODYPART_RIGHT_UPPER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(scale*btVector3(btScalar(0.7), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,-M_PI_2);
		m_bodies[BODYPART_RIGHT_LOWER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_LOWER_ARM]);

		// Setup some damping on the m_bodies
		for (int i = 0; i < BODYPART_COUNT; ++i)
		{
			m_bodies[i]->setDamping(btScalar(0.05), btScalar(0.85));
			m_bodies[i]->setDeactivationTime(btScalar(0.8));
			m_bodies[i]->setSleepingThresholds(btScalar(1.6), btScalar(2.5));
		}

		// Now setup the constraints
		btHingeConstraint* hingeC;
		btConeTwistConstraint* coneC;

		btTransform localA, localB;

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.15), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_SPINE], localA, localB);
		hingeC->setLimit(btScalar(-M_PI_4), btScalar(M_PI_2));
		m_joints[JOINT_PELVIS_SPINE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_PELVIS_SPINE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI_2); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.30), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_HEAD], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, M_PI_2);
		m_joints[JOINT_SPINE_HEAD] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_SPINE_HEAD], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,-M_PI_4*5); localA.setOrigin(scale*btVector3(btScalar(-0.18), btScalar(-0.10), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,-M_PI_4*5); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_LEFT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_LEFT_HIP] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_HIP], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_LEG], *m_bodies[BODYPART_LEFT_LOWER_LEG], localA, localB);
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_LEFT_KNEE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_KNEE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI_4); localA.setOrigin(scale*btVector3(btScalar(0.18), btScalar(-0.10), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_4); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.225), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_RIGHT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_RIGHT_HIP] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_HIP], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.225), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.185), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_LEG], *m_bodies[BODYPART_RIGHT_LOWER_LEG], localA, localB);
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_RIGHT_KNEE] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_KNEE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI); localA.setOrigin(scale*btVector3(btScalar(-0.2), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_LEFT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		m_joints[JOINT_LEFT_SHOULDER] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_SHOULDER], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_ARM], *m_bodies[BODYPART_LEFT_LOWER_ARM], localA, localB);
		hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
		m_joints[JOINT_LEFT_ELBOW] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_ELBOW], true);



		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,0); localA.setOrigin(scale*btVector3(btScalar(0.2), btScalar(0.15), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.18), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_RIGHT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		m_joints[JOINT_RIGHT_SHOULDER] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_SHOULDER], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(scale*btVector3(btScalar(0.), btScalar(0.18), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(scale*btVector3(btScalar(0.), btScalar(-0.14), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_ARM], *m_bodies[BODYPART_RIGHT_LOWER_ARM], localA, localB);
		hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
		m_joints[JOINT_RIGHT_ELBOW] = hingeC;
		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_ELBOW], true);
	}

	virtual	~RagDoll ()
	{
		int i;

		// Remove all constraints
		for ( i = 0; i < JOINT_COUNT; ++i)
		{
			m_ownerWorld->removeConstraint(m_joints[i]);
			delete m_joints[i]; m_joints[i] = 0;
		}

		// Remove all bodies and shapes
		for ( i = 0; i < BODYPART_COUNT; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);
			
			delete m_bodies[i]->getMotionState();

			delete m_bodies[i]; m_bodies[i] = 0;
			delete m_shapes[i]; m_shapes[i] = 0;
		}
	}
};

void	BenchmarkDemo::createTest3()
{
	setCameraDistance(btScalar(50.));

	int size = 16;

	float sizeX = 1.f;
	float sizeY = 1.f;

	//int rc=0;

	btScalar scale(3.5);
	btVector3 pos(0.0f, sizeY, 0.0f);
	while(size) {
		float offset = -size * (sizeX * 6.0f) * 0.5f;
		for(int i=0;i<size;i++) {
			pos[0] = offset + (float)i * (sizeX * 6.0f);

				RagDoll* ragDoll = new RagDoll (m_dynamicsWorld,pos,scale);
				m_ragdolls.push_back(ragDoll);
		}

		offset += sizeX;
		pos[1] += (sizeY * 7.0f);
		pos[2] -= sizeX * 2.0f;
		size--;
	}

}
void	BenchmarkDemo::createTest4()
{
	setCameraDistance(btScalar(50.));

	int size = 8;
	const float cubeSize = 1.5f;
	float spacing = cubeSize;
	btVector3 pos(0.0f, cubeSize * 2, 0.0f);
	float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;

	btConvexHullShape* convexHullShape = new btConvexHullShape();

	btScalar scaling(1);

	convexHullShape->setLocalScaling(btVector3(scaling,scaling,scaling));

	for (int i=0;i<TaruVtxCount;i++)
	{
		btVector3 vtx(TaruVtx[i*3],TaruVtx[i*3+1],TaruVtx[i*3+2]);
		convexHullShape->addPoint(vtx*btScalar(1./scaling));
	}

	//this will enable polyhedral contact clipping, better quality, slightly slower
	//convexHullShape->initializePolyhedralFeatures();

	btTransform trans;
	trans.setIdentity();

	float mass = 1.f;
	btVector3 localInertia(0,0,0);
	convexHullShape->calculateLocalInertia(mass,localInertia);

	for(int k=0;k<15;k++) {
		for(int j=0;j<size;j++) {
			pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
			for(int i=0;i<size;i++) {
				pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
				trans.setOrigin(pos);
				localCreateRigidBody(mass,trans,convexHullShape);		
			}
		}
		offset -= 0.05f * spacing * (size-1);
		spacing *= 1.01f;
		pos[1] += (cubeSize * 2.0f + spacing);
	}
}


///////////////////////////////////////////////////////////////////////////////
// LargeMesh

int LandscapeVtxCount[] = {
	Landscape01VtxCount,
	Landscape02VtxCount,
	Landscape03VtxCount,
	Landscape04VtxCount,
	Landscape05VtxCount,
	Landscape06VtxCount,
	Landscape07VtxCount,
	Landscape08VtxCount,
};

int LandscapeIdxCount[] = {
	Landscape01IdxCount,
	Landscape02IdxCount,
	Landscape03IdxCount,
	Landscape04IdxCount,
	Landscape05IdxCount,
	Landscape06IdxCount,
	Landscape07IdxCount,
	Landscape08IdxCount,
};

btScalar *LandscapeVtx[] = {
	Landscape01Vtx,
	Landscape02Vtx,
	Landscape03Vtx,
	Landscape04Vtx,
	Landscape05Vtx,
	Landscape06Vtx,
	Landscape07Vtx,
	Landscape08Vtx,
};

btScalar *LandscapeNml[] = {
	Landscape01Nml,
	Landscape02Nml,
	Landscape03Nml,
	Landscape04Nml,
	Landscape05Nml,
	Landscape06Nml,
	Landscape07Nml,
	Landscape08Nml,
};

btScalar* LandscapeTex[] = {
	Landscape01Tex,
	Landscape02Tex,
	Landscape03Tex,
	Landscape04Tex,
	Landscape05Tex,
	Landscape06Tex,
	Landscape07Tex,
	Landscape08Tex,
};

unsigned short  *LandscapeIdx[] = {
	Landscape01Idx,
	Landscape02Idx,
	Landscape03Idx,
	Landscape04Idx,
	Landscape05Idx,
	Landscape06Idx,
	Landscape07Idx,
	Landscape08Idx,
};

void BenchmarkDemo::createLargeMeshBody()
{
	btTransform trans;
	trans.setIdentity();

	for(int i=0;i<8;i++) {

		btTriangleIndexVertexArray* meshInterface = new btTriangleIndexVertexArray();
		btIndexedMesh part;

		part.m_vertexBase = (const unsigned char*)LandscapeVtx[i];
		part.m_vertexStride = sizeof(btScalar) * 3;
		part.m_numVertices = LandscapeVtxCount[i];
		part.m_triangleIndexBase = (const unsigned char*)LandscapeIdx[i];
		part.m_triangleIndexStride = sizeof( short) * 3;
		part.m_numTriangles = LandscapeIdxCount[i]/3;
		part.m_indexType = PHY_SHORT;

		meshInterface->addIndexedMesh(part,PHY_SHORT);

		bool	useQuantizedAabbCompression = true;
		btBvhTriangleMeshShape* trimeshShape = new btBvhTriangleMeshShape(meshInterface,useQuantizedAabbCompression);
		btVector3 localInertia(0,0,0);
		trans.setOrigin(btVector3(0,-25,0));

		btRigidBody* body = localCreateRigidBody(0,trans,trimeshShape);
		body->setFriction (btScalar(0.9));
		
	}
	
}


void	BenchmarkDemo::createTest5()
{
	setCameraDistance(btScalar(250.));
	btVector3 boxSize(1.5f,1.5f,1.5f);
	float boxMass = 1.0f;
	float sphereRadius = 1.5f;
	float sphereMass = 1.0f;
	float capsuleHalf = 2.0f;
	float capsuleRadius = 1.0f;
	float capsuleMass = 1.0f;

	{
		int size = 10;
		int height = 10;

		const float cubeSize = boxSize[0];
		float spacing = 2.0f;
		btVector3 pos(0.0f, 20.0f, 0.0f);
		float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;
		
		int numBodies = 0;

		for(int k=0;k<height;k++) {
			for(int j=0;j<size;j++) {
				pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
				for(int i=0;i<size;i++) {
					pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
					btVector3 bpos = btVector3(0,25,0) + btVector3(5.0f,1.0f,5.0f)*pos;
					int idx = rand() % 9;
					btTransform trans;
					trans.setIdentity();
					trans.setOrigin(bpos);

					switch(idx) {
						case 0:case 1:case 2:
						{
							float r = 0.5f * (idx+1);
							btBoxShape* boxShape = new btBoxShape(boxSize*r);
							localCreateRigidBody(boxMass*r,trans,boxShape);
						}
						break;

						case 3:case 4:case 5:
						{
							float r = 0.5f * (idx-3+1);
							btSphereShape* sphereShape = new btSphereShape(sphereRadius*r);
							localCreateRigidBody(sphereMass*r,trans,sphereShape);
						}
						break;

						case 6:case 7:case 8:
						{
							float r = 0.5f * (idx-6+1);
							btCapsuleShape* capsuleShape = new btCapsuleShape(capsuleRadius*r,capsuleHalf*r);
							localCreateRigidBody(capsuleMass*r,trans,capsuleShape);
						}
						break;
					}

					numBodies++;
				}
			}
			offset -= 0.05f * spacing * (size-1);
			spacing *= 1.1f;
			pos[1] += (cubeSize * 2.0f + spacing);
		}
	}

	createLargeMeshBody();
}
void	BenchmarkDemo::createTest6()
{
	setCameraDistance(btScalar(250.));

	btVector3 boxSize(1.5f,1.5f,1.5f);

	btConvexHullShape* convexHullShape = new btConvexHullShape();

	for (int i=0;i<TaruVtxCount;i++)
	{
		btVector3 vtx(TaruVtx[i*3],TaruVtx[i*3+1],TaruVtx[i*3+2]);
		convexHullShape->addPoint(vtx);
	}

	btTransform trans;
	trans.setIdentity();

	float mass = 1.f;
	btVector3 localInertia(0,0,0);
	convexHullShape->calculateLocalInertia(mass,localInertia);


	{
		int size = 10;
		int height = 10;

		const float cubeSize = boxSize[0];
		float spacing = 2.0f;
		btVector3 pos(0.0f, 20.0f, 0.0f);
		float offset = -size * (cubeSize * 2.0f + spacing) * 0.5f;
		
	
		for(int k=0;k<height;k++) {
			for(int j=0;j<size;j++) {
				pos[2] = offset + (float)j * (cubeSize * 2.0f + spacing);
				for(int i=0;i<size;i++) {
					pos[0] = offset + (float)i * (cubeSize * 2.0f + spacing);
					btVector3 bpos = btVector3(0,25,0) + btVector3(5.0f,1.0f,5.0f)*pos;
					trans.setOrigin(bpos);
					
					localCreateRigidBody(mass,trans,convexHullShape);
				}
			}
			offset -= 0.05f * spacing * (size-1);
			spacing *= 1.1f;
			pos[1] += (cubeSize * 2.0f + spacing);
		}
	}


	createLargeMeshBody();
}




void BenchmarkDemo::initRays()
{
	raycastBar = btRaycastBar2 (2500.0, 0,50.0);
}

void BenchmarkDemo::castRays()
{
	raycastBar.cast (m_dynamicsWorld);
}

void	BenchmarkDemo::createTest7()
{
	
	createTest6();
	setCameraDistance(btScalar(150.));
	initRays();
}

void	BenchmarkDemo::exitPhysics()
{
	int i;

	for (i=0;i<m_ragdolls.size();i++)
	{
		RagDoll* doll = m_ragdolls[i];
		delete doll;
	}
    m_ragdolls.clear();

	//cleanup in the reverse order of creation/initialization
    if (m_dynamicsWorld)
    {
        //remove the rigidbodies from the dynamics world and delete them
        for (i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
        {
            btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            m_dynamicsWorld->removeCollisionObject( obj );
            delete obj;
        }
    }

	//delete collision shapes
	for (int j=0;j<m_collisionShapes.size();j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
    m_collisionShapes.clear();

	//delete dynamics world
	delete m_dynamicsWorld;
    m_dynamicsWorld=0;

	//delete solver
	delete m_solver;
    m_solver=0;

	//delete broadphase
	delete m_overlappingPairCache;
    m_overlappingPairCache=0;

	//delete dispatcher
	delete m_dispatcher;
    m_dispatcher=0;

	delete m_collisionConfiguration;
    m_collisionConfiguration=0;

	
}



#ifndef USE_GRAPHICAL_BENCHMARK

btRigidBody*	DemoApplication::localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
	body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);
	m_dynamicsWorld->addRigidBody(body);

	return body;
}
#endif //USE_GRAPHICAL_BENCHMARK

