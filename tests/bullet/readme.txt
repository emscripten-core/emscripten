The files in this directory were generated from the Bullet Physics library, version 2.77,

  http://bulletphysics.org/

which is zlib licensed.

To get the build system to generate .ll files, this was added to the CMakeLists.txt:

   SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

+  SET(CMAKE_C_COMPILER "/..PATH../llvm-gcc-4.2-2.8.source/cbuild/install/bin/llvm-gcc")
+  SET(CMAKE_CXX_COMPILER "/..PATH../llvm-gcc-4.2-2.8.source/cbuild/install/bin/llvm-g++")
+  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -emit-llvm)
+  SET(CMAKE_LINKER "/..PATH../llvm-2.8/cbuild/Release/bin/llvm-link")
+  SET(CMAKE_CXX_LINKER "/..PATH../llvm-2.8/cbuild/Release/bin/llvm-link")
+  SET(CMAKE_C_LINK_EXECUTABLE "/..PATH../llvm-2.8/cbuild/Release/bin/llvm-link")
+  SET(CMAKE_CXX_LINK_EXECUTABLE "/..PATH../llvm-2.8/cbuild/Release/bin/llvm-link")
+  SET(CMAKE_AR "/..PATH../llvm-2.8/cbuild/Release/bin/llvm-link")
+  SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -S -o <TARGET> <LINK_FLAGS> <OBJECTS>")
+  SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> -S -o <TARGET> <LINK_FLAGS> <OBJECTS>")
+  SET(CMAKE_RANLIB "echo") # Hackish way to disable it

   MESSAGE("CMAKE_CXX_FLAGS_DEBUG="+${CMAKE_CXX_FLAGS_DEBUG})


Afterwards they were combined using llvm-link, and disassembled with

  llvm-dis -show-annotations

