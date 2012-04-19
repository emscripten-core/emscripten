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
extern int emscripten_run_script_int(const char *script);
extern void emscripten_async_run_script(const char *script, int millis);

/*
 * Set a C function as the main event loop. The JS environment
 * will call that function at a specified number of frames per
 * second. Setting 0 as the fps will use the default browser
 * frame rate.
 */
extern void emscripten_set_main_loop(void (*func)(), int fps);
extern void emscripten_cancel_main_loop();

/*
 * Call a C function asynchronously, that is, after returning
 * control to the JS event loop. This is done by a setTimeout.
 * When building natively this becomes a simple direct call,
 * after SDL_Delay (you must include SDL.h for that).
 */
#if EMSCRIPTEN
extern void emscripten_async_call(void (*func)(), int millis);
#else
void emscripten_async_call(void (*func)(), int millis) {
  SDL_Delay(millis);
  func();
}
#endif

/*
 * This macro-looking function will cause Emscripten to
 * generate a comment in the generated code.
 * XXX This is deprecated for now, because it requires us to
 *     hold all global vars in memory. We need a better solution.
 */
//extern void EMSCRIPTEN_COMMENT(const char *text);

/*
 * Profiling tools.
 * INIT must be called first, with the maximum identifier that
 * will be used. BEGIN will add some code that marks
 * the beginning of a section of code whose run time you
 * want to measure. END will finish such a section. Note: If you
 * call begin but not end, you will get invalid data!
 * The profiling data will be written out if you call Profile.dump().
 */
extern void EMSCRIPTEN_PROFILE_INIT(int max);
extern void EMSCRIPTEN_PROFILE_BEGIN(int id);
extern void EMSCRIPTEN_PROFILE_END(int id);

#ifdef __cplusplus
}
#endif

