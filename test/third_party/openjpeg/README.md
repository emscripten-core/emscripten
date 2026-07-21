This directory contains openjpeg. See README and LICENSE files for details.

Emscripten changes:
  * j2k_lib.c: use gettimeofday instead of getrusage
  * tcd.c: init numbps to 0 to avoid warnings
  * codec/CMakeLists.txt: Force getopt.c to be used

