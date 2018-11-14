#ifdef __EMSCRIPTEN_PTHREADS__

#include <emscripten/threading.h>
#include <emscripten.h>
#include <string.h>
#include <stdlib.h>

#include "webgl1.h"
#include "webgl2.h"

ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glReadBuffer, GLenum);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glDrawRangeElements, GLenum, GLuint, GLuint, GLsizei, GLenum, const void *); // TODO: Not async if rendering from client side memory
VOID_SYNC_GL_FUNCTION_10(EM_FUNC_SIG_VIIIIIIIIII, void, glTexImage3D, GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
VOID_SYNC_GL_FUNCTION_11(EM_FUNC_SIG_VIIIIIIIIIII, void, glTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const void *);
ASYNC_GL_FUNCTION_9(EM_FUNC_SIG_VIIIIIIIII, void, glCopyTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
VOID_SYNC_GL_FUNCTION_9(EM_FUNC_SIG_VIIIIIIIII, void, glCompressedTexImage3D, GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const void *);
VOID_SYNC_GL_FUNCTION_11(EM_FUNC_SIG_VIIIIIIIIIII, void, glCompressedTexSubImage3D, GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const void *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGenQueries, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteQueries, GLsizei, const GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsQuery, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBeginQuery, GLenum, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glEndQuery, GLenum);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryiv, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetQueryObjectuiv, GLuint, GLenum, GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glUnmapBuffer, GLenum);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetBufferPointerv, GLenum, GLenum, void **);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDrawBuffers, GLsizei, const GLenum *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix2x3fv, GLint, GLsizei, GLboolean, const GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix3x2fv, GLint, GLsizei, GLboolean, const GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix2x4fv, GLint, GLsizei, GLboolean, const GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix4x2fv, GLint, GLsizei, GLboolean, const GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix3x4fv, GLint, GLsizei, GLboolean, const GLfloat *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniformMatrix4x3fv, GLint, GLsizei, GLboolean, const GLfloat *);
ASYNC_GL_FUNCTION_10(EM_FUNC_SIG_VIIIIIIIIII, void, glBlitFramebuffer, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glRenderbufferStorageMultisample, GLenum, GLsizei, GLenum, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glFramebufferTextureLayer, GLenum, GLenum, GLuint, GLint, GLint);
RET_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void *, glMapBufferRange, GLenum, GLintptr, GLsizeiptr, GLbitfield);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glFlushMappedBufferRange, GLenum, GLintptr, GLsizeiptr);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glBindVertexArray, GLuint);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteVertexArrays, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGenVertexArrays, GLsizei, GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsVertexArray, GLuint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetIntegeri_v, GLenum, GLuint, GLint *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glBeginTransformFeedback, GLenum);
ASYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glEndTransformFeedback);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glBindBufferRange, GLenum, GLuint, GLuint, GLintptr, GLsizeiptr);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glBindBufferBase, GLenum, GLuint, GLuint);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glTransformFeedbackVaryings, GLuint, GLsizei, const GLchar *const*, GLenum);
VOID_SYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIII, void, glGetTransformFeedbackVarying, GLuint, GLuint, GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glVertexAttribIPointer, GLuint, GLint, GLenum, GLsizei, const void *); // TODO: Not async if not rendering from client side memory
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetVertexAttribIiv, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetVertexAttribIuiv, GLuint, GLenum, GLuint *);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glVertexAttribI4i, GLuint, GLint, GLint, GLint, GLint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glVertexAttribI4ui, GLuint, GLuint, GLuint, GLuint, GLuint);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttribI4iv, GLuint, const GLint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttribI4uiv, GLuint, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetUniformuiv, GLuint, GLint, GLuint *);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_III, GLint, glGetFragDataLocation, GLuint, const GLchar *);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glUniform1ui, GLint, GLuint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform2ui, GLint, GLuint, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glUniform3ui, GLint, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glUniform4ui, GLint, GLuint, GLuint, GLuint, GLuint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform1uiv, GLint, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform2uiv, GLint, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform3uiv, GLint, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniform4uiv, GLint, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glClearBufferiv, GLenum, GLint, const GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glClearBufferuiv, GLenum, GLint, const GLuint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glClearBufferfv, GLenum, GLint, const GLfloat *);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIFI, void, glClearBufferfi, GLenum, GLint, GLfloat, GLint);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_III, const GLubyte *, glGetStringi, GLenum, GLuint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glCopyBufferSubData, GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetUniformIndices, GLuint, GLsizei, const GLchar *const*, GLuint *);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetActiveUniformsiv, GLuint, GLsizei, const GLuint *, GLenum, GLint *);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_III, GLuint, glGetUniformBlockIndex, GLuint, const GLchar *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetActiveUniformBlockiv, GLuint, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetActiveUniformBlockName, GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniformBlockBinding, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glDrawArraysInstanced, GLenum, GLint, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glDrawElementsInstanced, GLenum, GLsizei, GLenum, const void *, GLsizei); // TODO: Not async if rendering from client side memory
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_III, GLsync, glFenceSync, GLenum, GLbitfield);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsSync, GLsync);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDeleteSync, GLsync);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_IIII, GLenum, glClientWaitSync, GLsync, GLbitfield, GLuint64); // XXX TODO: 64-bit integer proxying, this is not right, proxying as 32-bit
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glWaitSync, GLsync, GLbitfield, GLuint64); // XXX TODO: 64-bit integer proxying, this is not right, proxying as 32-bit
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGetInteger64v, GLenum, GLint64 *);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetSynciv, GLsync, GLenum, GLsizei, GLsizei *, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetInteger64i_v, GLenum, GLuint, GLint64 *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetBufferParameteri64v, GLenum, GLenum, GLint64 *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGenSamplers, GLsizei, GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteSamplers, GLsizei, const GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsSampler, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindSampler, GLuint, GLuint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glSamplerParameteri, GLuint, GLenum, GLint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glSamplerParameteriv, GLuint, GLenum, const GLint *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIF, void, glSamplerParameterf, GLuint, GLenum, GLfloat);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glSamplerParameterfv, GLuint, GLenum, const GLfloat *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetSamplerParameteriv, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetSamplerParameterfv, GLuint, GLenum, GLfloat *);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttribDivisor, GLuint, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glBindTransformFeedback, GLenum, GLuint);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDeleteTransformFeedbacks, GLsizei, const GLuint *);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glGenTransformFeedbacks, GLsizei, GLuint *);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsTransformFeedback, GLuint);
ASYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glPauseTransformFeedback);
ASYNC_GL_FUNCTION_0(EM_FUNC_SIG_V, void, glResumeTransformFeedback);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetProgramBinary, GLuint, GLsizei, GLsizei *, GLenum *, void *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glProgramBinary, GLuint, GLenum, const void *, GLsizei);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glProgramParameteri, GLuint, GLenum, GLint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glInvalidateFramebuffer, GLenum, GLsizei, const GLenum *);
VOID_SYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIII, void, glInvalidateSubFramebuffer, GLenum, GLsizei, const GLenum *, GLint, GLint, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glTexStorage2D, GLenum, GLsizei, GLenum, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glTexStorage3D, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetInternalformativ, GLenum, GLenum, GLenum, GLsizei, GLint *);

// Extensions:
GL_APICALL void GL_APIENTRY glVertexAttribDivisorNV(GLuint index, GLuint divisor) { glVertexAttribDivisor(index, divisor); }
GL_APICALL void GL_APIENTRY glVertexAttribDivisorEXT(GLuint index, GLuint divisor) { glVertexAttribDivisor(index, divisor); }
GL_APICALL void GL_APIENTRY glVertexAttribDivisorARB(GLuint index, GLuint divisor) { glVertexAttribDivisor(index, divisor); }
GL_APICALL void GL_APIENTRY glVertexAttribDivisorANGLE(GLuint index, GLuint divisor) { glVertexAttribDivisor(index, divisor); }
GL_APICALL void GL_APIENTRY glDrawArraysInstancedNV(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { glDrawArraysInstanced(mode, first, count, instancecount); }
GL_APICALL void GL_APIENTRY glDrawArraysInstancedEXT(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { glDrawArraysInstanced(mode, first, count, instancecount); }
GL_APICALL void GL_APIENTRY glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { glDrawArraysInstanced(mode, first, count, instancecount); }
GL_APICALL void GL_APIENTRY glDrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) { glDrawArraysInstanced(mode, first, count, instancecount); }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedNV(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) { glDrawElementsInstanced(mode, count, type, indices, instancecount); }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) { glDrawElementsInstanced(mode, count, type, indices, instancecount); }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) { glDrawElementsInstanced(mode, count, type, indices, instancecount); }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount) { glDrawElementsInstanced(mode, count, type, indices, instancecount); }
GL_APICALL void GL_APIENTRY glBindVertexArrayOES(GLuint array) { glBindVertexArray(array); }
GL_APICALL void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint *arrays) { glDeleteVertexArrays(n, arrays); }
GL_APICALL void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint *arrays) { glGenVertexArrays(n, arrays); }
GL_APICALL GLboolean GL_APIENTRY glIsVertexArrayOES(GLuint array) { return glIsVertexArray(array); }
GL_APICALL void GL_APIENTRY glDrawBuffersEXT(GLsizei n, const GLenum *bufs) { glDrawBuffers(n, bufs); }
GL_APICALL void GL_APIENTRY glDrawBuffersWEBGL(GLsizei n, const GLenum *bufs) { glDrawBuffers(n, bufs); }

#endif // ~__EMSCRIPTEN_PTHREADS__
