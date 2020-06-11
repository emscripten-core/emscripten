/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <webgl/webgl1.h>

#ifdef __cplusplus
#if !defined(__DEFINED_pthread_t)
typedef unsigned long pthread_t;
#define __DEFINED_pthread_t
#endif
#else
#if !defined(__DEFINED_pthread_t)
typedef struct __pthread * pthread_t;
#define __DEFINED_pthread_t
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EM_BOOL
#define EM_BOOL int
#endif

#ifndef EMSCRIPTEN_RESULT
#define EMSCRIPTEN_RESULT int
#endif

#define EMSCRIPTEN_EVENT_WEBGLCONTEXTLOST      31
#define EMSCRIPTEN_EVENT_WEBGLCONTEXTRESTORED  32

typedef int EMSCRIPTEN_WEBGL_CONTEXT_HANDLE;

typedef int EMSCRIPTEN_WEBGL_CONTEXT_PROXY_MODE;
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW 0
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK 1
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS   2

typedef int EM_WEBGL_POWER_PREFERENCE;
#define EM_WEBGL_POWER_PREFERENCE_DEFAULT 0
#define EM_WEBGL_POWER_PREFERENCE_LOW_POWER 1
#define EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE 2

typedef struct EmscriptenWebGLContextAttributes {
  EM_BOOL alpha;
  EM_BOOL depth;
  EM_BOOL stencil;
  EM_BOOL antialias;
  EM_BOOL premultipliedAlpha;
  EM_BOOL preserveDrawingBuffer;
  EM_WEBGL_POWER_PREFERENCE powerPreference;
  EM_BOOL failIfMajorPerformanceCaveat;

  int majorVersion;
  int minorVersion;

  EM_BOOL enableExtensionsByDefault;
  EM_BOOL explicitSwapControl;
  EMSCRIPTEN_WEBGL_CONTEXT_PROXY_MODE proxyContextToMainThread;
  EM_BOOL renderViaOffscreenBackBuffer;
} EmscriptenWebGLContextAttributes;

extern void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes *attributes);

extern EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char *target, const EmscriptenWebGLContextAttributes *attributes);

extern EMSCRIPTEN_RESULT emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_get_current_context(void);

extern EMSCRIPTEN_RESULT emscripten_webgl_get_drawing_buffer_size(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, int *width, int *height);

extern EMSCRIPTEN_RESULT emscripten_webgl_get_context_attributes(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, EmscriptenWebGLContextAttributes *outAttributes);

extern EMSCRIPTEN_RESULT emscripten_webgl_destroy_context(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EM_BOOL emscripten_webgl_enable_extension(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, const char *extension);

extern EM_BOOL emscripten_webgl_enable_ANGLE_instanced_arrays(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EM_BOOL emscripten_webgl_enable_OES_vertex_array_object(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EM_BOOL emscripten_webgl_enable_WEBGL_draw_buffers(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EM_BOOL emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

typedef EM_BOOL (*em_webgl_context_callback)(int eventType, const void *reserved, void *userData);
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextlost_callback_on_thread(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback, pthread_t targetThread);
extern EMSCRIPTEN_RESULT emscripten_set_webglcontextrestored_callback_on_thread(const char *target, void *userData, EM_BOOL useCapture, em_webgl_context_callback callback, pthread_t targetThread);

extern EM_BOOL emscripten_is_webgl_context_lost(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

extern EMSCRIPTEN_RESULT emscripten_webgl_commit_frame(void);

extern EM_BOOL emscripten_supports_offscreencanvas(void);

// Returns function pointers to WebGL 1 functions. Please avoid using this function ever - all WebGL1/GLES2 functions, even those for WebGL1 extensions, are available to user code via static linking. Calling GL functions
// via function pointers obtained here is slow, and using this function can greatly increase resulting compiled program size. This functionality is available only for easier program code porting purposes, but be aware
// that calling this is causing a noticeable performance and compiled code size hit.
extern void *emscripten_webgl1_get_proc_address(const char *name);

// Returns function pointers to WebGL 2 functions. Please avoid using this function ever - all WebGL2/GLES3 functions, even those for WebGL2 extensions, are available to user code via static linking. Calling GL functions
// via function pointers obtained here is slow, and using this function can greatly increase resulting compiled program size. This functionality is available only for easier program code porting purposes, but be aware
// that calling this is causing a noticeable performance and compiled code size hit.
extern void *emscripten_webgl2_get_proc_address(const char *name);

// Combines emscripten_webgl1_get_proc_address() and emscripten_webgl2_get_proc_address() to return function pointers to both WebGL1 and WebGL2 functions. Same drawbacks apply.
extern void *emscripten_webgl_get_proc_address(const char *name);

#define emscripten_set_webglcontextlost_callback(target, userData, useCapture, callback)      emscripten_set_webglcontextlost_callback_on_thread(     (target), (userData), (useCapture), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)
#define emscripten_set_webglcontextrestored_callback(target, userData, useCapture, callback)  emscripten_set_webglcontextrestored_callback_on_thread( (target), (userData), (useCapture), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)

#define GLint int
#define GLenum int

#define EMSCRIPTEN_WEBGL_PARAM_TYPE int
#define EMSCRIPTEN_WEBGL_PARAM_TYPE_INT   0
#define EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT 1

char  *emscripten_webgl_get_supported_extensions(void);

double emscripten_webgl_get_shader_parameter_d(GLint shader, GLenum param);
char  *emscripten_webgl_get_shader_info_log_utf8(GLint shader);
char  *emscripten_webgl_get_shader_source_utf8(GLint shader);

double emscripten_webgl_get_program_parameter_d(GLint program, GLenum param);
char  *emscripten_webgl_get_program_info_log_utf8(GLint program);

double emscripten_webgl_get_vertex_attrib_d(int index, GLenum param);
GLint emscripten_webgl_get_vertex_attrib_o(int index, GLenum param);
int emscripten_webgl_get_vertex_attrib_v(int index, GLenum param, void *dst, int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);

double emscripten_webgl_get_uniform_d(GLint program, int location);
int emscripten_webgl_get_uniform_v(GLint program, int location, void *dst, int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);

int emscripten_webgl_get_parameter_v(GLenum param, void *dst, int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);
double emscripten_webgl_get_parameter_d(GLenum param);
GLint emscripten_webgl_get_parameter_o(GLenum param);
char *emscripten_webgl_get_parameter_utf8(GLenum param);
void emscripten_webgl_get_parameter_i64v(GLenum param, int64_t *dst);

#undef GLint
#undef GLenum

#ifdef __cplusplus
} // ~extern "C"
#endif
