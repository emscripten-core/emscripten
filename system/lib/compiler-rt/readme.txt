These files are from compiler-rt,

Last Changed Rev: 266813
Last Changed Date: Tue Apr 19 13:29:59 2016

===========================================================================

 No changes from upstream (but not all files are included).

===========================================================================

Compile with something like

./emcc system/lib/compiler-rt/lib/builtins/*.c -Isystem/lib/compiler-rt/lib/builtins -o rt.bc
./emcc -O2 -s ASM_JS=1 -g rt.bc -s LINKABLE=1
manually replace Math_imul with Math.imul

