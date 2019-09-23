#pragma once

#include <GLES3/gl31.h>

#if USE_WEBGL2_COMPUTE
#if GL_MAX_FEATURE_LEVEL != 30
#error "cannot resolve conflicting USE_WEBGL* settings"
#endif
#endif

GL_APICALL void GL_APIENTRY emscripten_glDispatchCompute (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
GL_APICALL void GL_APIENTRY emscripten_glDispatchComputeIndirect (GLintptr indirect);
GL_APICALL void GL_APIENTRY emscripten_glDrawArraysIndirect (GLenum mode, const void *indirect);
GL_APICALL void GL_APIENTRY emscripten_glDrawElementsIndirect (GLenum mode, GLenum type, const void *indirect);
GL_APICALL void GL_APIENTRY emscripten_glFramebufferParameteri (GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY emscripten_glGetFramebufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetProgramInterfaceiv (GLuint program, GLenum programInterface, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetProgramResourceiv (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
GL_APICALL GLuint GL_APIENTRY emscripten_glGetProgramResourceIndex (GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glGetProgramResourceName (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
GL_APICALL GLint GL_APIENTRY emscripten_glGetProgramResourceLocation (GLuint program, GLenum programInterface, const GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1i (GLuint program, GLint location, GLint v0);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2i (GLuint program, GLint location, GLint v0, GLint v1);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4i (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1ui (GLuint program, GLint location, GLuint v0);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2ui (GLuint program, GLint location, GLuint v0, GLuint v1);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4ui (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1f (GLuint program, GLint location, GLfloat v0);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2f (GLuint program, GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4f (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1iv (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2iv (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3iv (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4iv (GLuint program, GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1uiv (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2uiv (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3uiv (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4uiv (GLuint program, GLint location, GLsizei count, const GLuint *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform1fv (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform2fv (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform3fv (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniform4fv (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix2x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix3x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix2x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix4x2fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix3x4fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glProgramUniformMatrix4x3fv (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glBindImageTexture (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
GL_APICALL void GL_APIENTRY emscripten_glGetBooleani_v (GLenum target, GLuint index, GLboolean *data);
GL_APICALL void GL_APIENTRY emscripten_glMemoryBarrier (GLbitfield barriers);
GL_APICALL void GL_APIENTRY emscripten_glMemoryBarrierByRegion (GLbitfield barriers);
GL_APICALL void GL_APIENTRY emscripten_glTexStorage2DMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GL_APICALL void GL_APIENTRY emscripten_glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY emscripten_glGetMultisamplefv (GLenum pname, GLuint index, GLfloat *val);
GL_APICALL void GL_APIENTRY emscripten_glSampleMaski (GLuint maskNumber, GLbitfield mask);
GL_APICALL void GL_APIENTRY emscripten_glBindVertexBuffer (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttribFormat (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttribIFormat (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttribBinding (GLuint attribindex, GLuint bindingindex);
GL_APICALL void GL_APIENTRY emscripten_glVertexBindingDivisor (GLuint bindingindex, GLuint divisor);
