#### Source code Dirs
VPATH =				\
../BroadphaseCollision		\
../CollisionDispatch		\
../NarrowPhaseCollision		\
../CollisionShapes

ROOT = ../../..

#### Library
LIBRARY_ppu = bulletcollision.a

#### Compiler flags
CPPFLAGS        = 		\
-DUSE_LIBSPE2	\
-I../BroadphaseCollision		\
-I../CollisionDispath		\
-I../NarrowPhaseCollision		\
-I../CollisionShapes		\
-I$(ROOT)/src/			\
-I$(SDKINC)

#### Optimization level flags
#CC_OPT_LEVEL =  $(CC_OPT_LEVEL_DEBUG)
CC_OPT_LEVEL =  -O3

##### Objects to be archived in lib

OBJS = 						\
btAxisSweep3.o					\
btQuantizedBvh.o				\
btBroadphaseProxy.o				\
btCollisionAlgorithm.o				\
btDispatcher.o					\
btDbvtBroadphase.o				\
btDbvt.o					\
btOverlappingPairCache.o			\
btSimpleBroadphase.o				\
btContinuousConvexCollision.o			\
btConvexCast.o					\
btGjkConvexCast.o				\
btGjkEpa2.o					\
btGjkEpaPenetrationDepthSolver.o		\
btGjkPairDetector.o				\
btDefaultCollisionConfiguration.o		\
btMinkowskiPenetrationDepthSolver.o		\
btPersistentManifold.o				\
btRaycastCallback.o				\
btSubSimplexConvexCast.o			\
btVoronoiSimplexSolver.o			\
btCollisionDispatcher.o				\
btCollisionObject.o				\
btCollisionWorld.o				\
btCompoundCollisionAlgorithm.o			\
btBoxBoxCollisionAlgorithm.o			\
btBoxBoxDetector.o				\
btConvexPlaneCollisionAlgorithm.o		\
btConvexConcaveCollisionAlgorithm.o		\
btConvexConvexAlgorithm.o			\
btDefaultCollisionConfiguration.o		\
btEmptyCollisionAlgorithm.o			\
btManifoldResult.o				\
btSimulationIslandManager.o			\
btSphereBoxCollisionAlgorithm.o			\
btSphereSphereCollisionAlgorithm.o		\
btSphereTriangleCollisionAlgorithm.o		\
btActivatingCollisionAlgorithm.o		\
btUnionFind.o					\
SphereTriangleDetector.o			\
btBoxShape.o					\
btBvhTriangleMeshShape.o			\
btCapsuleShape.o				\
btCollisionShape.o				\
btCompoundShape.o				\
btConcaveShape.o				\
btConeShape.o					\
btConvexHullShape.o				\
btConvexShape.o					\
btConvexInternalShape.o				\
btConvexTriangleMeshShape.o			\
btCylinderShape.o				\
btEmptyShape.o					\
btHeightfieldTerrainShape.o			\
btMinkowskiSumShape.o				\
btMultiSphereShape.o				\
btOptimizedBvh.o				\
btPolyhedralConvexShape.o			\
btSphereShape.o					\
btStaticPlaneShape.o				\
btStridingMeshInterface.o			\
btTetrahedronShape.o				\
btTriangleBuffer.o				\
btTriangleCallback.o				\
btTriangleIndexVertexArray.o			\
btTriangleMesh.o				\
btTriangleMeshShape.o				\
btUniformScalingShape.o

#### Install directories
INSTALL_DIR	= $(ROOT)/lib/ibmsdk
INSTALL_FILES	= $(LIBRARY_ppu)

IBM_CELLSDK_VERSION := $(shell if [ -d /opt/cell ]; then echo "3.0"; fi)

ifeq ("$(IBM_CELLSDK_VERSION)","3.0")
        CELL_TOP ?= /opt/cell/sdk
        include $(CELL_TOP)/buildutils/make.footer
else
        CELL_TOP ?= /opt/ibm/cell-sdk/prototype
        include $(CELL_TOP)/make.footer
endif

