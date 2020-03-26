
INCLUDE_DIRECTORIES(
${BULLET_PHYSICS_SOURCE_DIR}/src
)

SET(DXSDK_DIR $ENV{DXSDK_DIR})
SET(DX11_INCLUDE_PATH  "${DIRECTX_SDK_BASE_DIR}/Include" CACHE DOCSTRING "Microsoft directX SDK include path")


INCLUDE_DIRECTORIES(
${DX11_INCLUDE_PATH} "../cpu/"
${VECTOR_MATH_INCLUDE}
)

SET(BulletSoftBodyDX11Solvers_SRCS
	btSoftBodySolver_DX11.cpp
	btSoftBodySolver_DX11SIMDAware.cpp
)

SET(BulletSoftBodyDX11Solvers_HDRS
	btSoftBodySolver_DX11.h
	btSoftBodySolver_DX11SIMDAware.h
	../cpu/btSoftBodySolverData.h
	btSoftBodySolverVertexData_DX11.h
	btSoftBodySolverTriangleData_DX11.h
	btSoftBodySolverLinkData_DX11.h
	btSoftBodySolverLinkData_DX11SIMDAware.h
	btSoftBodySolverBuffer_DX11.h
	btSoftBodySolverVertexBuffer_DX11.h

)

# OpenCL and HLSL Shaders.
# Build rules generated to stringify these into headers
# which are needed by some of the sources
SET(BulletSoftBodyDX11Solvers_Shaders
	OutputToVertexArray
	UpdateNormals
	Integrate
	UpdatePositions
	UpdateNodes
	ComputeBounds
	SolvePositions
	SolvePositionsSIMDBatched
	SolveCollisionsAndUpdateVelocities
	SolveCollisionsAndUpdateVelocitiesSIMDBatched
	UpdatePositionsFromVelocities
	ApplyForces
	PrepareLinks
	VSolveLinks
)

foreach(f ${BulletSoftBodyDX11Solvers_Shaders})
    LIST(APPEND BulletSoftBodyDX11Solvers_HLSL "HLSL/${f}.hlsl")
endforeach(f) 



ADD_LIBRARY(BulletSoftBodySolvers_DX11  ${BulletSoftBodyDX11Solvers_SRCS} ${BulletSoftBodyDX11Solvers_HDRS} ${BulletSoftBodyDX11Solvers_HLSL})
SET_TARGET_PROPERTIES(BulletSoftBodySolvers_DX11 PROPERTIES VERSION ${BULLET_VERSION})
SET_TARGET_PROPERTIES(BulletSoftBodySolvers_DX11 PROPERTIES SOVERSION ${BULLET_VERSION})
IF (BUILD_SHARED_LIBS)
	TARGET_LINK_LIBRARIES(BulletSoftBodySolvers_DX11 BulletSoftBody BulletDynamics)
ENDIF (BUILD_SHARED_LIBS)


IF (INSTALL_LIBS)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
			IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS BulletSoftBodySolvers_DX11 DESTINATION .)
			ELSE (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
			INSTALL(TARGETS BulletSoftBodySolvers_DX11 DESTINATION lib${LIB_SUFFIX})
#headers are already installed by BulletMultiThreaded library
			ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
		ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)

		IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_DX11 PROPERTIES FRAMEWORK true)
			SET_TARGET_PROPERTIES(BulletSoftBodySolvers_DX11 PROPERTIES PUBLIC_HEADER "${BulletSoftBodyDX11Solvers_HDRS}")
		ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
	ENDIF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
ENDIF (INSTALL_LIBS)
