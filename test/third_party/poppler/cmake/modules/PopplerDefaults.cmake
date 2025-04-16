# enable the testing facilities
enable_testing()

# put the include directories of the sources before other include paths
# (eg, system includes)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# colored output
set(CMAKE_COLOR_MAKEFILE ON)

# CMake policy #0002: we can have multiple targets with the same name (for the unit tests)
cmake_policy(SET CMP0002 OLD)
# CMake policy #0011 (>= 2.6.3): make policy changes (as done here) taking effect for all the tree
if(POLICY CMP0011)
    cmake_policy(SET CMP0011 OLD)
endif(POLICY CMP0011)

