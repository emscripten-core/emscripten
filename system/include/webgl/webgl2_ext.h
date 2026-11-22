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

// 56. https://registry.khronos.org/webgl/extensions/OES_sample_variables/
#ifndef EMSCRIPTEN_GL_OES_sample_variables
#define EMSCRIPTEN_GL_OES_sample_variables 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_sample_variables");
// Shaders must enable the extension with "#extension GL_OES_sample_variables : require".
// <no functions or defines exposed>
#endif /* EMSCRIPTEN_GL_OES_sample_variables */

// 57. https://registry.khronos.org/webgl/extensions/OES_shader_multisample_interpolation/
#ifndef EMSCRIPTEN_GL_OES_shader_multisample_interpolation
#define EMSCRIPTEN_GL_OES_shader_multisample_interpolation 1
// To enable: call emscripten_webgl_enable_extension(ctx, "OES_shader_multisample_interpolation");
// Shaders must enable the extension with
// "#extension GL_OES_shader_multisample_interpolation : require".
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES 0x8E5D
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_OES_shader_multisample_interpolation */

// 59. https://registry.khronos.org/webgl/extensions/EXT_render_snorm/
#ifndef EMSCRIPTEN_GL_EXT_render_snorm
#define EMSCRIPTEN_GL_EXT_render_snorm 1
// To enable: call emscripten_webgl_enable_extension(ctx, "EXT_render_snorm");
// Makes the internal formats GL_R8_SNORM, GL_RG8_SNORM and GL_RGBA8_SNORM
// color-renderable, and if EXT_texture_norm16 is also enabled, the formats
// GL_R16_SNORM_EXT, GL_RG16_SNORM_EXT and GL_RGBA16_SNORM_EXT as well.
// <no functions or defines exposed>
#endif /* EMSCRIPTEN_GL_EXT_render_snorm */

// 60. https://registry.khronos.org/webgl/extensions/WEBGL_render_shared_exponent/
#ifndef EMSCRIPTEN_GL_WEBGL_render_shared_exponent
#define EMSCRIPTEN_GL_WEBGL_render_shared_exponent 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_render_shared_exponent");
// Makes the internal format GL_RGB9_E5 color-renderable.
// <no functions or defines exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_render_shared_exponent */

// 61. https://registry.khronos.org/webgl/extensions/WEBGL_stencil_texturing/
#ifndef EMSCRIPTEN_GL_WEBGL_stencil_texturing
#define EMSCRIPTEN_GL_WEBGL_stencil_texturing 1
// To enable: call emscripten_webgl_enable_extension(ctx, "WEBGL_stencil_texturing");
#define GL_DEPTH_STENCIL_TEXTURE_MODE_WEBGL 0x90EA
#define GL_STENCIL_INDEX_WEBGL 0x1901
// <no functions exposed>
#endif /* EMSCRIPTEN_GL_WEBGL_stencil_texturing */
