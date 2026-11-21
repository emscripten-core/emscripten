#pragma once

#include "webgl2.h"
#include <emscripten/html5.h>

// 33. https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query_webgl2/
#ifndef EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2
#define EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2 1
#define GL_QUERY_COUNTER_BITS_EXT 0x8864
#define GL_TIME_ELAPSED_EXT 0x88BF
#define GL_TIMESTAMP_EXT 0x8E28
#define GL_GPU_DISJOINT_EXT 0x8FBB
WEBGL_APICALL void GL_APIENTRY glQueryCounterEXT(GLuint query, GLenum target);
#endif /* EMSCRIPTEN_GL_EXT_disjoint_timer_query_webgl2 */

// 36. https://www.khronos.org/registry/webgl/extensions/OVR_multiview2/
#ifndef EMSCRIPTEN_GL_OVR_multiview2
#define EMSCRIPTEN_GL_OVR_multiview2 1
// To enable: call
bool emscripten_webgl_enable_OVR_multiview2(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
// or link with -sGL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 and
// call emscripten_webgl_enable_extension(ctx, "OVR_multiview2");
//
// Shaders must enable the extension with "#extension GL_OVR_multiview2 : require".
// WebGL does not support the older "GL_OVR_multiview" shader extension name, and
// only 2D array textures can be attached with glFramebufferTextureMultiviewOVR().
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_NUM_VIEWS_OVR 0x9630
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_BASE_VIEW_INDEX_OVR 0x9632
#define GL_MAX_VIEWS_OVR 0x9631
#define GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR 0x9633
WEBGL_APICALL void GL_APIENTRY emscripten_glFramebufferTextureMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
WEBGL_APICALL void GL_APIENTRY glFramebufferTextureMultiviewOVR(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews);
#endif /* EMSCRIPTEN_GL_OVR_multiview2 */

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
  const GLint* _Nonnull firsts,
  const GLsizei* _Nonnull counts,
  const GLsizei* _Nonnull instanceCounts,
  const GLuint* _Nonnull baseInstances,
  GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode,
  const GLsizei* _Nonnull counts,
  GLenum type,
  const GLvoid* const* _Nonnull offsets,
  const GLsizei* _Nonnull instanceCounts,
  const GLint* _Nonnull baseVertices,
  const GLuint* _Nonnull baseInstances,
  GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysInstancedBaseInstanceWEBGL(
  GLenum mode,
 const GLint* _Nonnull firsts,
 const GLsizei* _Nonnull counts,
 const GLsizei* _Nonnull instanceCounts,
 const GLuint* _Nonnull baseInstances,
 GLsizei drawCount);

WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(
  GLenum mode,
  const GLsizei* _Nonnull counts,
  GLenum type,
  const GLvoid* const* _Nonnull offsets,
  const GLsizei* _Nonnull instanceCounts,
  const GLint* _Nonnull baseVertices,
  const GLuint* _Nonnull baseInstances,
  GLsizei drawCount);

#endif /* EMSCRIPTEN_GL_WEBGL_multi_draw_instanced_base_vertex_base_instance */
