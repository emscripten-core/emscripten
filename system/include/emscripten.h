/**
 * This file contains a few useful things for compiling C/C++ code
 * with Emscripten, an LLVM-to-JavaScript compiler.
 *
 * The code can be used permissively under the MIT license.
 *
 * http://emscripten.org
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Interface to the underlying JS engine. This function will
 * eval() the given script.
 */
extern void emscripten_run_script(const char *script);

/*
 * This macro-looking function will cause Emscripten to
 * generate a comment in the generated code.
 */
extern void EMSCRIPTEN_COMMENT(const char *text);

#ifdef __cplusplus
}
#endif

