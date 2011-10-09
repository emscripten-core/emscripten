/**
 * This file contains a few useful things for compiling C/C++ code
 * with Emscripten, an LLVM-to-JavaScript compiler.
 *
 * The code can be used permissively under the MIT license.
 *
 * http://emscripten.org
 */

// Interface to the underlying JS engine. This function will
// eval() the given script.
extern void emscripten_run_script(const char *script);

