
INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
	${AMD_OPENCL_INCLUDES}
)

ADD_DEFINITIONS(-DUSE_AMD_OPENCL)
ADD_DEFINITIONS(-DCL_PLATFORM_AMD)



SET(BulletSoftBodyOpenCLSolvers_SRCS
	../btSoftBodySolver_OpenCL.cpp
	../btSoftBodySolver_OpenCLSIMDAware.cpp
	../btSoftBodySolverOutputCLtoGL.cpp
)

SET(BulletSoftBodyOpenCLSolvers_HDRS
	../btSoftBodySolver_OpenCL.h
	../btSoftBodySolver_OpenCLSIMDAware.h
	../../CPU/btSoftBodySolverData.h
	../btSoftBodySolverVertexData_OpenCL.h
	../btSoftBodySolverTriangleData_OpenCL.h
	../btSoftBodySolverLinkData_OpenCL.h
	../btSoftBodySolverLinkData_OpenCLSIMDAware.h
	../btSoftBodySolverBuffer_OpenCL.h
	../btSoftBodySolverVertexBuffer_OpenGL.h
	../btSoftBodySolverOutputCLtoGL.h
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
    LIST(APPEND BulletSoftBodyOpenCLSolvers_OpenCLC "../OpenCLC/${f}.cl")
endforeach(f) 



ADD_LIBRARY(BulletSoftBodySolvers_OpenCL_AMD
	${BulletSoftBodyOpenCLSolvers_SRCS} 
	${BulletSoftBodyOpenCLSolvers_HDRS} 
	${BulletSoftBodyOpenCLSolvers_OpenCLC}
)

SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_AMD PROPERTIES VERSION ${BULLET_VERSION})
SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_AMD PROPERTIES SOVERSION ${BULLET_VERSION})
IF (BUILD_SHARED_LIBS)
	TARGET_LINK_LIBRARIES(BulletSoftBodySolvers_OpenCL_AMD BulletSoftBody)
ENDIF (BUILD_SHARED_LIBS)


IF (INSTALL_LIBS)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
			IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletSoftBodySolvers_OpenCL_AMD DESTINATION .)
			ELSE (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletSoftBodySolvers_OpenCL_AMD DESTINATION lib${LIB_SUFFIX})
#headers are already installed by BulletMultiThreaded library
			ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
		ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)

		IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_AMD PROPERTIES FRAMEWORK true)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_OpenCL_AMD PROPERTIES PUBLIC_HEADER "${BulletSoftBodyOpenCLSolvers_HDRS}")
		ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
	ENDIF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
ENDIF (INSTALL_LIBS)
