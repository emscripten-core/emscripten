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

#if !EMSCRIPTEN
#include <SDL/SDL.h> /* for SDL_Delay in async_call */
#endif

/*
 * Forces LLVM to not dead-code-eliminate a function. Note that
 * closure may still eliminate it at the JS level, for which you
 * should use EXPORTED_FUNCTIONS (see settings.js).
 *
 * Example usage:
 *   void EMSCRIPTEN_KEEPALIVE my_function() { .. }
 */
#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

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
 * second. Setting 0 or a negative value as the fps will use
 * the browser's requestAnimationFrame mechanism.
 *
 * Pausing and resuming the main loop is useful if your app
 * needs to perform some synchronous operation, for example
 * to load a file from the network. It might be wrong to
 * run the main loop before that finishes (the original
 * code assumes that), so you can break the code up into
 * asynchronous callbacks, but you must pause the main
 * loop until they complete.
 */
#if EMSCRIPTEN
extern void emscripten_set_main_loop(void (*func)(), int fps);
extern void emscripten_pause_main_loop();
extern void emscripten_resume_main_loop();
extern void emscripten_cancel_main_loop();
#else
#define emscripten_set_main_loop(func, fps) \
  while (1) func();
#endif

/*
 * Add a function to a queue of events that will execute
 * before the main loop will continue. The event is pushed
 * into the back of the queue. (Note that in the native version
 * of this we simply execute the function, so to keep semantics
 * identical be careful to not push while the queue is being
 * used.)
 *
 * Main loop blockers block the main loop from running, and
 * can be counted to show progress. In contrast, emscripten_async_calls
 * are not counted, do not block the main loop, and can fire
 * at specific time in the future.
 */
#if EMSCRIPTEN
extern void _emscripten_push_main_loop_blocker(void (*func)(void *), void *arg, const char *name);
extern void _emscripten_push_uncounted_main_loop_blocker(void (*func)(void *), void *arg, const char *name);
#else
inline void _emscripten_push_main_loop_blocker(void (*func)(void *), void *arg, const char *name) {
  func(arg);
}
inline void _emscripten_push_uncounted_main_loop_blocker(void (*func)(void *), void *arg, const char *name) {
  func(arg);
}
#endif
#define emscripten_push_main_loop_blocker(func, arg) \
  _emscripten_push_main_loop_blocker(func, arg, #func)
#define emscripten_push_uncounted_main_loop_blocker(func, arg) \
  _emscripten_push_uncounted_main_loop_blocker(func, arg, #func)

/*
 * Sets the number of blockers remaining until some user-relevant
 * event. This affects how we show progress. So if you set this
 * to 10, then push 10 blockers, as they complete the user will
 * see x/10 and so forth.
 */
#if EMSCRIPTEN
extern void emscripten_set_main_loop_expected_blockers(int num);
#else
inline void emscripten_set_main_loop_expected_blockers(int num) {}
#endif

/*
 * Call a C function asynchronously, that is, after returning
 * control to the JS event loop. This is done by a setTimeout.
 * When building natively this becomes a simple direct call,
 * after SDL_Delay (you must include SDL.h for that).
 *
 * If millis is negative, the browser's requestAnimationFrame
 * mechanism is used.
 */
#if EMSCRIPTEN
extern void emscripten_async_call(void (*func)(void *), void *arg, int millis);
#else
inline void emscripten_async_call(void (*func)(void *), void *arg, int millis) {
  if (millis) SDL_Delay(millis);
  func(arg);
}
#endif

/*
 * Hide the OS mouse cursor over the canvas. Note that SDL's
 * SDL_ShowCursor command shows and hides the SDL cursor, not
 * the OS one. This command is useful to hide the OS cursor
 * if your app draws its own cursor.
 */
void emscripten_hide_mouse();

/*
 * Resizes the pixel width and height of the <canvas> element
 * on the Emscripten web page.
 */
void emscripten_set_canvas_size(int width, int height);

/*
 * Returns the highest-precision representation of the
 * current time that the browser provides. This uses either
 * Date.now or performance.now. The result is *not* an
 * absolute time, and is only meaningful in comparison to
 * other calls to this function. The unit is ms.
 */
float emscripten_get_now();

/*
 * Simple random number generation in [0, 1), maps to Math.random().
 */
float emscripten_random();

/*
 * This macro-looking function will cause Emscripten to
 * generate a comment in the generated code.
 * XXX This is deprecated for now, because it requires us to
 *     hold all global vars in memory. We need a better solution.
 */
//extern void EMSCRIPTEN_COMMENT(const char *text);

/*
 * Emscripten file system api
 */

/*
 * Load file from url in asynchronous way. 
 * When file is loaded then 'onload' callback will called.
 * If any error occurred 'onerror' will called.
 * The callbacks are called with the file as their argument.
 */ 
void emscripten_async_wget(const char* url, const char* file, void (*onload)(const char*), void (*onerror)(const char*));

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

