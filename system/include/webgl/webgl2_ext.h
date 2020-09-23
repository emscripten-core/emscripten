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
WEBGL_APICALL void GL_APIENTRY emscripten_glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseInstance);
WEBGL_APICALL void GL_APIENTRY glDrawArraysInstancedBaseInstanceWEBGL(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance);
WEBGL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, GLsizei count, GLenum type, const void *offset, GLsizei instanceCount, GLint baseVertex, GLuint baseInstance);

// 47. https://www.khronos.org/registry/webgl/extensions/WEBGL_multi_draw_instanced_base_vertex_base_instance/
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawArraysInstancedBaseInstanceWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, const GLsizei* instanceCounts, const GLuint* baseInstances, GLsizei drawCount);
WEBGL_APICALL void GL_APIENTRY emscripten_glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, const GLsizei* instanceCounts, const GLint* baseVertices, const GLuint* baseInstances, GLsizei drawCount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawArraysInstancedBaseInstanceWEBGL(GLenum mode, const GLint* firsts, const GLsizei* counts, const GLsizei* instanceCounts, const GLuint* baseInstances, GLsizei drawCount);
WEBGL_APICALL void GL_APIENTRY glMultiDrawElementsInstancedBaseVertexBaseInstanceWEBGL(GLenum mode, const GLsizei* counts, GLenum type, const GLvoid* const* offsets, const GLsizei* instanceCounts, const GLint* baseVertices, const GLuint* baseinstances, GLsizei drawCount);
