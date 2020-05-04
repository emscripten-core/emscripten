#pragma once

#include "webgl2.h"

// 33. https://www.khronos.org/registry/webgl/extensions/EXT_disjoint_timer_query_webgl2/
#define GL_QUERY_COUNTER_BITS_EXT 0x8864
#define GL_TIME_ELAPSED_EXT 0x88BF
#define GL_TIMESTAMP_EXT 0x8E28
#define GL_GPU_DISJOINT_EXT 0x8FBB
WEBGL_APICALL void GL_APIENTRY emscripten_webgl2_queryCounterEXT(GLuint query, GLenum target);

// 46. https://www.khronos.org/registry/webgl/extensions/WEBGL_draw_instanced_base_vertex_base_instance/
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawArraysInstancedBaseInstanceWEBGL(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance);
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseinstance);
WEBGL_APICALL void GL_APIENTRY glDrawArraysInstancedBaseInstanceWEBGL(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance);
WEBGL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseinstance);
