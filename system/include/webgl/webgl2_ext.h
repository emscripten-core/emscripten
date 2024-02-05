#pragma once

#include "webgl2.h"

// 33. https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query_webgl2/
#ifndef EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2
#define EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2 1
#define GL_QUERY_COUNTER_BITS_EXT 0x8864
#define GL_TIME_ELAPSED_EXT 0x88BF
#define GL_TIMESTAMP_EXT 0x8E28
#define GL_GPU_DISJOINT_EXT 0x8FBB
WEBGL_APICALL void GL_APIENTRY glQueryCounterEXT(GLuint query, GLenum target);
#endif /* EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2 */

// 46. https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_instanced_base_vertex_base_instance/
#ifndef EMSCRIPTEN_GL_WEBGL_draw_instanced_base_vertex_base_instance
#define EMSCRIPTEN_GL_WEBGL_draw_instanced_base_vertex_base_instance 1

WEBGL_APICALL void GL_APIENTRY emscripten_glDrawArraysInstancedBaseInstanceWEBGL(
  GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance);

WEBGL_APICALL void GL_APIENTRY emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseInstance);

WEBGL_APICALL void GL_APIENTRY glDrawArraysInstancedBaseInstanceWEBGL(
  GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance);

WEBGL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseInstance);

#endif /* EMSCRIPTEN_GL_WEBGL_draw_instanced_base_vertex_base_instance */

// 47. https://www.khronos.org/registry/webgl/extensions/WEBGL_multi_draw_instanced_base_vertex_base_instance/
#ifndef EMSCRIPTEN_GL_WEBGL_multi_draw_instanced_base_vertex_base_instance
#define EMSCRIPTEN_GL_WEBGL_multi_draw_instanced_base_vertex_base_instance 1

WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysInstancedBaseInstanceWEBGL(
  GLenum mode,
  const GLint* firsts __attribute__((nonnull)),
  const GLsizei* counts __attribute__((nonnull)),
  const GLsizei* instanceCounts __attribute__((nonnull)),
  const GLuint* baseInstances __attribute__((nonnull)),
  GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode,
  const GLsizei* counts __attribute__((nonnull)),
  GLenum type,
  const GLvoid* const* offsets __attribute__((nonnull)),
  const GLsizei* instanceCounts __attribute__((nonnull)),
  const GLint* baseVertices __attribute__((nonnull)),
  const GLuint* baseInstances __attribute__((nonnull)),
  GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysInstancedBaseInstanceWEBGL(
  GLenum mode,
 const GLint* firsts __attribute__((nonnull)),
 const GLsizei* counts __attribute__((nonnull)),
 const GLsizei* instanceCounts __attribute__((nonnull)),
 const GLuint* baseInstances __attribute__((nonnull)),
 GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode,
  const GLsizei* counts __attribute__((nonnull)),
  GLenum type,
  const GLvoid* const* offsets __attribute__((nonnull)),
  const GLsizei* instanceCounts __attribute__((nonnull)),
  const GLint* baseVertices __attribute__((nonnull)),
  const GLuint* baseinstances __attribute__((nonnull)),
  GLsizei drawCount);

#endif /* EMSCRIPTEN_GL_WEBGL_multi_draw_instanced_base_vertex_base_instance */