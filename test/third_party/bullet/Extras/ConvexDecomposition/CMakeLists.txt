INCLUDE_DIRECTORIES(
 ${BULLET_PHYSICS_SOURCE_DIR}/Extras/ConvexDecomposition ${BULLET_PHYSICS_SOURCE_DIR}/src
)

SET(ConvexDecomposition_SRCS
	bestfitobb.cpp
	ConvexBuilder.cpp
	cd_wavefront.cpp
	fitsphere.cpp
	meshvolume.cpp
	raytri.cpp
	vlookup.cpp
	bestfit.cpp
	cd_hull.cpp
	ConvexDecomposition.cpp
	concavity.cpp
	float_math.cpp
	planetri.cpp
	splitplane.cpp
)

SET(ConvexDecomposition_HDRS
	ConvexDecomposition.h
	cd_vector.h
	concavity.h
	bestfitobb.h
	ConvexBuilder.h
	cd_wavefront.h
	fitsphere.h
	meshvolume.h
	raytri.h
	vlookup.h
	bestfit.h
	cd_hull.h
)

ADD_LIBRARY(ConvexDecomposition ${ConvexDecomposition_SRCS} ${ConvexDecomposition_HDRS})
SET_TARGET_PROPERTIES(ConvexDecomposition PROPERTIES VERSION ${BULLET_VERSION})
SET_TARGET_PROPERTIES(ConvexDecomposition PROPERTIES SOVERSION ${BULLET_VERSION})

IF (BUILD_SHARED_LIBS)
  TARGET_LINK_LIBRARIES(ConvexDecomposition BulletCollision LinearMath)
ENDIF (BUILD_SHARED_LIBS)

IF (INSTALL_EXTRA_LIBS)
	IF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
		#FILES_MATCHING requires CMake 2.6
		IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)
			IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS ConvexDecomposition DESTINATION .)
			ELSE (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
				INSTALL(TARGETS ConvexDecomposition DESTINATION lib${LIB_SUFFIX})
				INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
DESTINATION ${INCLUDE_INSTALL_DIR} FILES_MATCHING PATTERN "*.h"  PATTERN
".svn" EXCLUDE PATTERN "CMakeFiles" EXCLUDE)
			ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
		ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.5)

		IF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
			SET_TARGET_PROPERTIES(ConvexDecomposition PROPERTIES FRAMEWORK true)
			SET_TARGET_PROPERTIES(ConvexDecomposition PROPERTIES PUBLIC_HEADER "${ConvexDecomposition_HDRS}")
		ENDIF (APPLE AND BUILD_SHARED_LIBS AND FRAMEWORK)
	ENDIF (NOT INTERNAL_CREATE_DISTRIBUTABLE_MSVC_PROJECTFILES)
ENDIF (INSTALL_EXTRA_LIBS)
