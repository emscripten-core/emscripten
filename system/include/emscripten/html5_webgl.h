/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uintptr_t EMSCRIPTEN_WEBGL_CONTEXT_HANDLE;

typedef int EMSCRIPTEN_WEBGL_CONTEXT_PROXY_MODE;
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW 0
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_FALLBACK 1
#define EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS   2

typedef int EM_WEBGL_POWER_PREFERENCE;
#define EM_WEBGL_POWER_PREFERENCE_DEFAULT 0
#define EM_WEBGL_POWER_PREFERENCE_LOW_POWER 1
#define EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE 2

typedef struct EmscriptenWebGLContextAttributes {
  bool alpha;
  bool depth;
  bool stencil;
  bool antialias;
  bool premultipliedAlpha;
  bool preserveDrawingBuffer;
  EM_WEBGL_POWER_PREFERENCE powerPreference;
  bool failIfMajorPerformanceCaveat;

  int majorVersion;
  int minorVersion;

  bool enableExtensionsByDefault;
  bool explicitSwapControl;
  EMSCRIPTEN_WEBGL_CONTEXT_PROXY_MODE proxyContextToMainThread;
  bool renderViaOffscreenBackBuffer;
} EmscriptenWebGLContextAttributes;

void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes *attributes __attribute__((nonnull)));

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char *target __attribute__((nonnull)), const EmscriptenWebGLContextAttributes * _Nonnull attributes);

EMSCRIPTEN_RESULT emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_get_current_context(void);

EMSCRIPTEN_RESULT emscripten_webgl_get_drawing_buffer_size(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, int *width __attribute__((nonnull)), int *height __attribute__((nonnull)));

EMSCRIPTEN_RESULT emscripten_webgl_get_context_attributes(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, EmscriptenWebGLContextAttributes *outAttributes __attribute__((nonnull)));

EMSCRIPTEN_RESULT emscripten_webgl_destroy_context(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_extension(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context, const char *extension __attribute__((nonnull)));

bool emscripten_webgl_enable_ANGLE_instanced_arrays(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_OES_vertex_array_object(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_WEBGL_draw_buffers(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_WEBGL_multi_draw(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_EXT_polygon_offset_clamp(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_EXT_clip_control(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

bool emscripten_webgl_enable_WEBGL_polygon_mode(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

typedef bool (*em_webgl_context_callback)(int eventType, const void *reserved, void *userData);
EMSCRIPTEN_RESULT emscripten_set_webglcontextlost_callback_on_thread(const char *target __attribute__((nonnull)), void *userData, bool useCapture, em_webgl_context_callback callback, pthread_t targetThread);
EMSCRIPTEN_RESULT emscripten_set_webglcontextrestored_callback_on_thread(const char *target __attribute__((nonnull)), void *userData, bool useCapture, em_webgl_context_callback callback, pthread_t targetThread);

bool emscripten_is_webgl_context_lost(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);

EMSCRIPTEN_RESULT emscripten_webgl_commit_frame(void);

bool emscripten_supports_offscreencanvas(void);

// Returns function pointers to WebGL 1 functions. Please avoid using this function ever - all WebGL1/GLES2 functions, even those for WebGL1 extensions, are available to user code via static linking. Calling GL functions
// via function pointers obtained here is slow, and using this function can greatly increase resulting compiled program size. This functionality is available only for easier program code porting purposes, but be aware
// that calling this is causing a noticeable performance and compiled code size hit.
void *emscripten_webgl1_get_proc_address(const char *name __attribute__((nonnull)));

// Returns function pointers to WebGL 2 functions. Please avoid using this function ever - all WebGL2/GLES3 functions, even those for WebGL2 extensions, are available to user code via static linking. Calling GL functions
// via function pointers obtained here is slow, and using this function can greatly increase resulting compiled program size. This functionality is available only for easier program code porting purposes, but be aware
// that calling this is causing a noticeable performance and compiled code size hit.
void *emscripten_webgl2_get_proc_address(const char *name __attribute__((nonnull)));

// Combines emscripten_webgl1_get_proc_address() and emscripten_webgl2_get_proc_address() to return function pointers to both WebGL1 and WebGL2 functions. Same drawbacks apply.
void *emscripten_webgl_get_proc_address(const char *name __attribute__((nonnull)));

#define emscripten_set_webglcontextlost_callback(target, userData, useCapture, callback)      emscripten_set_webglcontextlost_callback_on_thread(     (target), (userData), (useCapture), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)
#define emscripten_set_webglcontextrestored_callback(target, userData, useCapture, callback)  emscripten_set_webglcontextrestored_callback_on_thread( (target), (userData), (useCapture), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)

#define GLint int
#define GLenum int
#define GLint64 long long int

#define EMSCRIPTEN_WEBGL_PARAM_TYPE int
#define EMSCRIPTEN_WEBGL_PARAM_TYPE_INT   0
#define EMSCRIPTEN_WEBGL_PARAM_TYPE_FLOAT 1

// Calls GLctx.getSupportedExtensions():
// Returns a newly allocated string that contains all supported WebGL extensions by the currently active WebGL context,
// separated by a space character ' '. Call free() to deallocate the string.
char  *emscripten_webgl_get_supported_extensions(void);

// Calls GLctx.getShaderParameter():
// Returns a parameter of a WebGL shader as a double.
// Call this function only for values of 'param' that return a Number type.
double emscripten_webgl_get_shader_parameter_d(GLint shader, GLenum param);

// Calls GLctx.getShaderInfoLog():
// Returns a newly allocated string that contains the shader info log of the given shader. Call free() to deallocate the string.
char  *emscripten_webgl_get_shader_info_log_utf8(GLint shader);

// Calls GLctx.getShaderSource():
// Returns a newly allocated string that contains the shader source the given shader. Call free() to deallocate the string.
char  *emscripten_webgl_get_shader_source_utf8(GLint shader);

// Calls GLctx.getProgramParameter():
// Returns a parameter of a WebGL shader program as a double.
// Call this function only for values of 'param' that return a Number type.
double emscripten_webgl_get_program_parameter_d(GLint program, GLenum param);

// Calls GLctx.getProgramInfoLog():
// Returns a newly allocated string that contains the info log of the given program. Call free() to deallocate the string.
char  *emscripten_webgl_get_program_info_log_utf8(GLint program);

// Calls GLctx.getVertexAttrib():
// Returns the given vertex attribute as a double.
// Call this function only for values of 'param' that return a Number type.
double emscripten_webgl_get_vertex_attrib_d(int index, GLenum param);

// Calls GLctx.getVertexAttrib():
// Returns the WebGL object name bound to the given vertex attribute.
// Call this function only for values of 'param' that return a WebGL object type.
GLint emscripten_webgl_get_vertex_attrib_o(int index, GLenum param);

// Calls GLctx.getVertexAttrib():
// Gets an array of currently active vertex attributes.
// Call this function only for values of 'param' that return an array of types.
// Use dstType to specify whether to read an array of ints or floats.
// The function writes at most dstLength array elements to array dst.
// The actual length of the state array is returned (not the number of elements written)
int emscripten_webgl_get_vertex_attrib_v(int index, GLenum param, void *dst __attribute__((nonnull)), int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);

// Calls GLctx.getUniform():
// Returns the value of a uniform set in a program in the given location.
// Call this function only for scalar uniform types. (float and int)
double emscripten_webgl_get_uniform_d(GLint program, int location);

// Calls GLctx.getUniform():
// Gets an array set to an uniform in a program in the given location.
// Call this function only for array uniform types. (vec2, ivec2 and so on)
// Use dstType to specify whether to read in ints or floats.
// The function writes at most dstLength array elements to array dst.
// The actual length of the state array is returned (not the number of elements written)
int emscripten_webgl_get_uniform_v(GLint program, int location, void *dst __attribute__((nonnull)), int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);

// Calls GLctx.getParameter():
// Gets an array of state set to the active WebGL context.
// Call this function only for values of 'param' that return an array of types.
// Use dstType to specify whether to read in ints or floats.
// The function writes at most dstLength array elements to array dst.
// The actual length of the state array is returned (not the number of elements written)
int emscripten_webgl_get_parameter_v(GLenum param, void *dst __attribute__((nonnull)), int dstLength, EMSCRIPTEN_WEBGL_PARAM_TYPE dstType);

// Calls GLctx.getParameter():
// Returns the given WebGL context state as double.
// Call this function only for values of 'param' that return a Number type.
double emscripten_webgl_get_parameter_d(GLenum param);

// Calls GLctx.getParameter():
// Returns the WebGL object name bound to the given WebGL context state binding point.
// Call this function only for values of 'param' that return a WebGL object type.
GLint emscripten_webgl_get_parameter_o(GLenum param);

// Calls GLctx.getParameter():
// Returns a newly allocated string containing the WebGL state associated with the given parameter.
// Call free() to deallocate the string.
// Call this function only for values of 'param' that return a WebGL string type.
char *emscripten_webgl_get_parameter_utf8(GLenum param);

// Calls GLctx.getParameter():
// Returns the given WebGL context state as GLint64, written to the given heap location.
// Call this function only for values of 'param' that return a WebGL Number type.
void emscripten_webgl_get_parameter_i64v(GLenum param, GLint64 *dst __attribute__((nonnull)));

#undef GLint
#undef GLenum
#undef GLint64

#ifdef __cplusplus
} // ~extern "C"
#endif
