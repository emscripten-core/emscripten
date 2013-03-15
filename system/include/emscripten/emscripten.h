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
 * **DEPRECATED**: Use EXPORTED_FUNCTIONS instead, which will work
 *                 with closure, asm.js, etc. For example
 *                   -s EXPORTED_FUNCTIONS=["_main", "myfunc"]
 */
/* #define EMSCRIPTEN_KEEPALIVE __attribute__((used)) */

/*
 * Interface to the underlying JS engine. This function will
 * eval() the given script.
 */
extern void emscripten_run_script(const char *script);
extern int emscripten_run_script_int(const char *script);
extern char *emscripten_run_script_string(const char *script); // uses a single buffer - shared between calls!
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
 *
 * @simulate_infinite_loop If true, this function will throw an
 *    exception in order to stop execution of the caller. This
 *    will lead to the main loop being entered instead of code
 *    after the call to emscripten_set_main_loop being run, which
 *    is the closest we can get to simulating an infinite loop
 *    (we do something similar in glutMainLoop in GLUT). If this
 *    parameter is false, then the behavior is the same as it
 *    was before this parameter was added to the API, which is
 *    that execution continues normally. Note that in both cases
 *    we do not run global destructors, atexit, etc., since we
 *    know the main loop will still be running, but if we do
 *    not simulate an infinite loop then the stack will be unwound.
 *    That means that if simulate_infinite_loop is false, and
 *    you created an object on the stack, it will be cleaned up
 *    before the main loop will be called the first time.
 */
#if EMSCRIPTEN
extern void emscripten_set_main_loop(void (*func)(), int fps, int simulate_infinite_loop);
extern void emscripten_pause_main_loop();
extern void emscripten_resume_main_loop();
extern void emscripten_cancel_main_loop();
#else
#define emscripten_set_main_loop(func, fps, simulateInfiniteLoop) \
  while (1) { func(); usleep(1000000/fps); }
#define emscripten_cancel_main_loop() exit(1);
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
 * Load file from url in asynchronous way. In addition to
 * fetching the URL from the network, the contents are
 * prepared so that the data is usable in IMG_Load and
 * so forth (we asynchronously do the work to make the
 * browser decode the image or audio and so forth).
 * When file is ready then 'onload' callback will called.
 * If any error occurred 'onerror' will called.
 * The callbacks are called with the file as their argument.
 */
void emscripten_async_wget(const char* url, const char* file, void (*onload)(const char*), void (*onerror)(const char*));

/*
 * Data version of emscripten_async_wget. Instead of writing
 * to a file, it writes to a buffer directly in memory.
 * This avoids the overhead of using the emulated
 * filesystem, note however that since files are not used,
 * It cannot do the 'prepare' stage to set things up for
 * IMG_Load and so forth (IMG_Load etc. work on files).
 *
 * @param arg User-defined data that is passed to the callbacks,
 *
 * @param onload Callback on success, with the @arg that
 *               was provided to this function, a pointer
 *               to a buffer with the data, and the size
 *               of the buffer. As in the worker API, the
 *               data buffer only lives during the
 *               callback, so you should use it or copy
 *               it during that time and not later.
 *
 * @param onerror An optional callback on failure, with the
 *                @arg that was provided to this function.
 *
 */
void emscripten_async_wget_data(const char* url, void *arg, void (*onload)(void*, void*, int), void (*onerror)(void*));

/*
 * More feature-complete version of emscripten_async_wget. Note:
 * this version is experimental.
 *
 * The requesttype is 'GET' or 'POST',
 * If is post request, param is the post parameter 
 * like key=value&key2=value2.
 * The param 'arg' is a pointer will be pass to the callback
 * When file is ready then 'onload' callback will called.
 * During the download 'onprogress' callback will called.
 * If any error occurred 'onerror' will called.
 * The callbacks are called with an object pointer give in parameter 
 * and file if is a success, the progress value during progress
 * and http status code if is an error.
 */
void emscripten_async_wget2(const char* url, const char* file,  const char* requesttype, const char* param, void *arg, void (*onload)(void*, const char*), void (*onerror)(void*, int), void (*onprogress)(void*, int));

/*
 * Prepare a file in asynchronous way. This does just the
 * preparation part of emscripten_async_wget, that is, it
 * works on file data already present, and asynchronously
 * prepares it for use in IMG_Load, Mix_LoadWAV, etc.
 * When file is loaded then 'onload' callback will called.
 * If any error occurred 'onerror' will called.
 * The callbacks are called with the file as their argument.
 * @return 0 if successful, -1 if the file does not exist
 */
int emscripten_async_prepare(const char* file, void (*onload)(const char*), void (*onerror)(const char*));

/*
 * Data version of emscripten_async_prepare, which receives
 * raw data as input instead of a filename (this can prevent
 * the need to write data to a file first). onload and
 * onerror are called back with the given arg pointer as the
 * first parameter. onload also receives a second
 * parameter, which is a 'fake' filename which you can
 * then pass into IMG_Load (it is not an actual file,
 * but it identifies this image for IMG_Load to be able
 * to process it). Note that the user of this API is
 * responsible for free()ing the memory allocated for
 * the fake filename.
 * @suffix The file suffix, e.g. 'png' or 'jpg'.
 */
void emscripten_async_prepare_data(char* data, int size, const char *suffix, void *arg, void (*onload)(void*, const char*), void (*onerror)(void*));

/*
 * Worker API. Basically a wrapper around web workers, lets
 * you create workers and communicate with them.

 * Note that the current API is mainly focused on a main thread that
 * sends jobs to workers and waits for responses, i.e., in an
 * asymmetrical manner, there is no current API to send a message
 * without being asked for it from a worker to the main thread.
 *
 */

typedef int worker_handle;

/*
 * Create and destroy workers. A worker must be compiled separately
 * from the main program, and with the BUILD_AS_WORKER flag set to 1.
 */
worker_handle emscripten_create_worker(const char *url);
void emscripten_destroy_worker(worker_handle worker);

/*
 * Asynchronously call a worker.
 *
 * The worker function will be called with two parameters: a
 * data pointer, and a size.  The data block defined by the
 * pointer and size exists only during the callback and
 * _cannot_ be relied upon afterwards - if you need to keep some
 * of that information around, you need to copy it to a safe
 * location.
 *
 * The called worker function can return data, by calling
 * emscripten_worker_respond(). If called, and if a callback was
 * given, then the callback will be called with three arguments:
 * a data pointer, a size, and  * an argument that was provided
 * when calling emscripten_call_worker (to more easily associate
 * callbacks to calls). The data block defined by the data pointer
 * and size behave like the data block in the worker function -
 * it exists only during the callback.
 *
 * @funcname the name of the function in the worker. The function
 *           must be a C function (so no C++ name mangling), and
 *           must be exported (EXPORTED_FUNCTIONS).
 * @data the address of a block of memory to copy over
 * @size the size of the block of memory
 * @callback the callback with the response (can be null)
 * @arg an argument to be passed to the callback
 */
void emscripten_call_worker(worker_handle worker, const char *funcname, char *data, int size, void (*callback)(char *, int, void*), void *arg);

/*
 * Sends a response when in a worker call. Should only be
 * called once in each call.
 */
void emscripten_worker_respond(char *data, int size);

/*
 * Checks how many responses are being waited for from a worker. This
 * only counts calls to emscripten_call_worker that had a non-null
 * callback (if it's null, we do not have any tracking of a response),
 * and that the response was not yet received. It is a simple way to
 * check on the status of the worker to see how busy it is, and do
 * basic decisions about throttling.
 */
int emscripten_get_worker_queue_size(worker_handle worker);

/*
 * Select the networking backend to use. By default emscripten's
 * socket/networking implementation will use websockets, with this
 * function you can change that to WebRTC.
 * This function must be called before any network functions are
 * called.
 */
#define EMSCRIPTEN_NETWORK_WEBSOCKETS 0
#define EMSCRIPTEN_NETWORK_WEBRTC     1
void emscripten_set_network_backend(int backend);

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

/*
 * jcache-friendly printf. printf in general will receive a string
 * literal, which becomes a global constant, which invalidates all
 * jcache entries. emscripten_jcache_printf is parsed before
 * clang into something without any string literals, so you can
 * add such printouts to your code and only the (chunk containing
 * the) function you modify will be invalided and recompiled.
 *
 * Note in particular that you need to already have a call to this
 * function in your code *before* you add one and do an incremental
 * build, so that adding an external reference does not invalidate
 * everything.
 *
 * This function assumes the first argument is a string literal
 * (otherwise you don't need it), and the other arguments, if any,
 * are neither strings nor complex expressions (but just simple
 * variables). (You can create a variable to store a complex
 * expression on the previous line, if necessary.)
 */
#ifdef __cplusplus
void emscripten_jcache_printf(const char *format, ...);
void emscripten_jcache_printf_(...); /* internal use */
#endif

#ifdef __cplusplus
}
#endif

