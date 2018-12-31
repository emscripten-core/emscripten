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
#ifndef BENCHMARK_DEMO_H
#define BENCHMARK_DEMO_H


#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btTransform.h"

class btDynamicsWorld;

#define NUMRAYS 500

class btRigidBody;
class btBroadphaseInterface;
class btCollisionShape;
class btOverlappingPairCache;
class btCollisionDispatcher;
class btConstraintSolver;
struct btCollisionAlgorithmCreateFunc;
class btDefaultCollisionConfiguration;


#ifndef USE_GRAPHICAL_BENCHMARK
///empty placeholder
class DemoApplication
{
protected:

	btDynamicsWorld* m_dynamicsWorld;
	btScalar	m_defaultContactProcessingThreshold;

public:
	DemoApplication()
	:m_defaultContactProcessingThreshold(BT_LARGE_FLOAT)
	{
	}
	virtual void myinit() {}
	virtual btDynamicsWorld* getDynamicsWorld()
	{
		return m_dynamicsWorld;
	}

	btScalar	getDeltaTimeMicroseconds()
	{
		return 1.f;
	}

	void	renderme() {}
	void	setCameraDistance(btScalar dist){}
	void	clientResetScene(){}
	btRigidBody*	localCreateRigidBody(float mass, const btTransform& startTransform,btCollisionShape* shape);

};
///BenchmarkDemo is provides several performance tests
#define PlatformDemoApplication DemoApplication
#else //USE_GRAPHICAL_BENCHMARK

#ifdef _WINDOWS
#include "Win32DemoApplication.h"
#define PlatformDemoApplication Win32DemoApplication
#else
#include "GlutDemoApplication.h"
#define PlatformDemoApplication GlutDemoApplication
#endif

#endif //USE_GRAPHICAL_BENCHMARK


class BenchmarkDemo : public PlatformDemoApplication
{

	//keep the collision shapes, for deletion/cleanup
	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;

	btAlignedObjectArray<class RagDoll*>	m_ragdolls;

	btBroadphaseInterface*	m_overlappingPairCache;

	btCollisionDispatcher*	m_dispatcher;

	btConstraintSolver*	m_solver;

	btDefaultCollisionConfiguration* m_collisionConfiguration;
	
	int	m_benchmark;

	void	createTest1();
	void	createTest2();
	void	createTest3();
	void	createTest4();
	void	createTest5();
	void	createTest6();
	void	createTest7();

	void createWall(const btVector3& offsetPosition,int stackSize,const btVector3& boxSize);
	void createPyramid(const btVector3& offsetPosition,int stackSize,const btVector3& boxSize);
	void createTowerCircle(const btVector3& offsetPosition,int stackSize,int rotSize,const btVector3& boxSize);
	void createLargeMeshBody();


	class SpuBatchRaycaster* m_batchRaycaster;
	class btThreadSupportInterface* m_batchRaycasterThreadSupport;

	void castRays();
	void initRays();

	public:

	BenchmarkDemo(int benchmark)
	:m_benchmark(benchmark)
	{
	}
	virtual ~BenchmarkDemo()
	{
		exitPhysics();
	}
	void	initPhysics();

	void	exitPhysics();

	virtual void clientMoveAndDisplay();

	virtual void displayCallback();
	


	
};

class BenchmarkDemo1 : public BenchmarkDemo
{
public:
	BenchmarkDemo1()
		:BenchmarkDemo(1)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo1* demo = new BenchmarkDemo1;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};

class BenchmarkDemo2 : public BenchmarkDemo
{
public:
	BenchmarkDemo2()
		:BenchmarkDemo(2)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo2* demo = new BenchmarkDemo2;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};

class BenchmarkDemo3 : public BenchmarkDemo
{
public:
	BenchmarkDemo3()
		:BenchmarkDemo(3)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo3* demo = new BenchmarkDemo3;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};

class BenchmarkDemo4 : public BenchmarkDemo
{
public:
	BenchmarkDemo4()
		:BenchmarkDemo(4)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo4* demo = new BenchmarkDemo4;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};


class BenchmarkDemo5 : public BenchmarkDemo
{
public:
	BenchmarkDemo5()
		:BenchmarkDemo(5)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo5* demo = new BenchmarkDemo5;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};


class BenchmarkDemo6 : public BenchmarkDemo
{
public:
	BenchmarkDemo6()
		:BenchmarkDemo(6)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo6* demo = new BenchmarkDemo6;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};

class BenchmarkDemo7 : public BenchmarkDemo
{
public:
	BenchmarkDemo7()
		:BenchmarkDemo(7)
	{
	}

	static DemoApplication* Create()
	{
		BenchmarkDemo7* demo = new BenchmarkDemo7;
		demo->myinit();
		demo->initPhysics();
		return demo;
	}
};

#endif //BENCHMARK_DEMO_H

