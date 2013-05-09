These files are from compiler-rt,

Last Changed Rev: 179380
Last Changed Date: 2013-04-12 07:57:03 -0700 (Fri, 12 Apr 2013)

===========================================================================

Changes:

  * add emscripten endianness to int_endianness.h
  * add rem functions

===========================================================================

Compile with something like

./emcc system/lib/compiler-rt/*.c -Isystem/lib/compiler-rt/ -o rt.bc
./emcc -O2 -s ASM_JS=1 -g rt.bc -s LINKABLE=1
manually replace Math_imul with Math.imul

