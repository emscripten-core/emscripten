
INCLUDE_DIRECTORIES(
${BULLET_PHYSICS_SOURCE_DIR}/src
)




SET(BulletSoftBodyOpenCLSolvers_SRCS
	../btSoftBodySolver_OpenCL.cpp
	../btSoftBodySolver_OpenCLSIMDAware.cpp
)

SET(BulletSoftBodyOpenCLSolvers_HDRS
	../btSoftBodySolver_OpenCL.h
	../../CPU/btSoftBodySolverData.h
	../btSoftBodySolverVertexData_OpenCL.h
	../btSoftBodySolverTriangleData_OpenCL.h
	../btSoftBodySolverLinkData_OpenCL.h
	../btSoftBodySolverBuffer_OpenCL.h
)

# OpenCL and HLSL Shaders.
# Build rules generated to stringify these into headers
# which are needed by some of the sources
SET(BulletSoftBodyOpenCLSolvers_Shaders
#	OutputToVertexArray
	UpdateNormals
	Integrate
	UpdatePositions
	UpdateNodes
	SolvePositions
	UpdatePositionsFromVelocities
	ApplyForces
	PrepareLinks
	VSolveLinks
)

foreach(f ${BulletSoftBodyOpenCLSolvers_Shaders})
    LIST(APPEND BulletSoftBodyOpenCLSolvers_OpenCLC "../OpenCLC10/${f}.cl")
endforeach(f) 



ADD_LIBRARY(BulletSoftBodySolvers_OpenCL_Apple
	${BulletSoftBodyOpenCLSolvers_SRCS} 
	${BulletSoftBodyOpenCLSolvers_HDRS} 
	${BulletSoftBodyOpenCLSolvers_OpenCLC}
)

SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_Apple PROPERTIES VERSION ${BULLET_VERSION})
SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_Apple PROPERTIES SOVERSION ${BULLET_VERSION})
IF (BUILD_SHARED_LIBS)
	IF (APPLE AND (BUILD_SHARED_LIBS OR FRAMEWORK) )
		SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_Apple PROPERTIES LINK_FLAGS "-framework OpenCL")
	ENDIF (APPLE AND (BUILD_SHARED_LIBS OR FRAMEWORK) )
	TARGET_LINK_LIBRARIES(BulletSoftBodySolvers_OpenCL_Apple BulletSoftBody)
ENDIF (BUILD_SHARED_LIBS)


IF (INSTALL_LIBS)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
			IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletSoftBodySolvers_OpenCL_Apple  DESTINATION .)
			ELSE (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletSoftBodySolvers_OpenCL_Apple  DESTINATION lib${LIB_SUFFIX})
#headers are already installed by BulletMultiThreaded library
			ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
		ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)

		IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_Apple PROPERTIES FRAMEWORK true)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_Apple PROPERTIES PUBLIC_HEADER "${BulletSoftBodyOpenCLSolvers_HDRS}")
		ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
	ENDIF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
ENDIF (INSTALL_LIBS)
