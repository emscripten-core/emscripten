INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
	${VECTOR_MATH_INCLUDE}
)

ADD_LIBRARY(BulletMultiThreaded
		PlatformDefinitions.h
		PpuAddressSpace.h
		SpuFakeDma.cpp
		SpuFakeDma.h
		SpuDoubleBuffer.h
		SpuLibspe2Support.cpp
		SpuLibspe2Support.h
		btThreadSupportInterface.cpp
		btThreadSupportInterface.h
		
		Win32ThreadSupport.cpp
		Win32ThreadSupport.h
		PosixThreadSupport.cpp
		PosixThreadSupport.h
		SequentialThreadSupport.cpp
		SequentialThreadSupport.h
		SpuSampleTaskProcess.h
		SpuSampleTaskProcess.cpp

		SpuCollisionObjectWrapper.cpp 
		SpuCollisionObjectWrapper.h 
		SpuCollisionTaskProcess.h
		SpuCollisionTaskProcess.cpp
		SpuGatheringCollisionDispatcher.h
		SpuGatheringCollisionDispatcher.cpp
		SpuContactManifoldCollisionAlgorithm.cpp
		SpuContactManifoldCollisionAlgorithm.h
		
		btParallelConstraintSolver.cpp
		btParallelConstraintSolver.h
		
		SpuNarrowPhaseCollisionTask/Box.h
		SpuNarrowPhaseCollisionTask/boxBoxDistance.cpp
		SpuNarrowPhaseCollisionTask/boxBoxDistance.h
#		SPURS_PEGatherScatterTask/SpuPEGatherScatterTask.cpp
#		SPURS_PEGatherScatterTask/SpuPEGatherScatterTask.h
#		SpuPEGatherScatterTaskProcess.cpp
#		SpuPEGatherScatterTaskProcess.h
		
		SpuNarrowPhaseCollisionTask/SpuContactResult.cpp
		SpuNarrowPhaseCollisionTask/SpuContactResult.h
		SpuNarrowPhaseCollisionTask/SpuMinkowskiPenetrationDepthSolver.cpp
		SpuNarrowPhaseCollisionTask/SpuMinkowskiPenetrationDepthSolver.h
		SpuNarrowPhaseCollisionTask/SpuConvexPenetrationDepthSolver.h
		SpuNarrowPhaseCollisionTask/SpuPreferredPenetrationDirections.h
		SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.cpp
		SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h
		SpuNarrowPhaseCollisionTask/SpuCollisionShapes.cpp
		SpuNarrowPhaseCollisionTask/SpuCollisionShapes.h


#Some GPU related stuff, mainly CUDA and perhaps OpenCL
		btGpu3DGridBroadphase.cpp
		btGpu3DGridBroadphase.h
		btGpu3DGridBroadphaseSharedCode.h
		btGpu3DGridBroadphaseSharedDefs.h
		btGpu3DGridBroadphaseSharedTypes.h
		btGpuDefines.h
		btGpuUtilsSharedCode.h
		btGpuUtilsSharedDefs.h
)
SET_TARGET_PROPERTIES(BulletMultiThreaded PROPERTIES VERSION ${BULLET_VERSION})
SET_TARGET_PROPERTIES(BulletMultiThreaded PROPERTIES SOVERSION ${BULLET_VERSION})


SUBDIRS(GpuSoftBodySolvers)


IF (BUILD_SHARED_LIBS)
	IF (UNIX)
		TARGET_LINK_LIBRARIES(BulletMultiThreaded BulletDynamics BulletCollision pthread)
	ELSE()
		TARGET_LINK_LIBRARIES(BulletMultiThreaded BulletDynamics BulletCollision)
	ENDIF()
ENDIF (BUILD_SHARED_LIBS)


IF (INSTALL_LIBS)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		#INSTALL of other files requires CMake 2.6
		IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
#			IF(INSTALL_EXTRA_LIBS)
				IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
					INSTALL(TARGETS BulletMultiThreaded DESTINATION .)
				ELSE (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletMultiThreaded DESTINATION lib${LIB_SUFFIX})
					INSTALL(DIRECTORY
${CMAKE_CURRENT_SOURCE_DIR} DESTINATION ${INCLUDE_INSTALL_DIR} FILES_MATCHING
PATTERN "*.h"  PATTERN ".svn" EXCLUDE PATTERN "CMakeFiles" EXCLUDE)
				ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
#			ENDIF (INSTALL_EXTRA_LIBS)
		ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
	ENDIF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
ENDIF (INSTALL_LIBS)

