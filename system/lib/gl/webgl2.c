/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#if defined(__EMSCRIPTEN_FULL_ES3__) || MAX_WEBGL_VERSION >= 2

#include <emscripten/threading.h>
#include <emscripten.h>
#include <string.h>
#include <stdlib.h>

#include <webgl/webgl1.h>
#include <webgl/webgl2.h>

#include "webgl_internal.h"

#if defined(__EMSCRIPTEN_PTHREADS__) && defined(__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

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
#ifdef __EMSCRIPTEN_FULL_ES3__
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glUnmapBuffer, GLenum);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetBufferPointerv, GLenum, GLenum, void **);
#endif
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
#ifdef __EMSCRIPTEN_FULL_ES3__
RET_PTR_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_PIIII, void *, glMapBufferRange, GLenum, GLintptr, GLsizeiptr, GLbitfield);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glFlushMappedBufferRange, GLenum, GLintptr, GLsizeiptr);
#endif
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
RET_PTR_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_PII, const GLubyte *, glGetStringi, GLenum, GLuint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glCopyBufferSubData, GLenum, GLenum, GLintptr, GLintptr, GLsizeiptr);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetUniformIndices, GLuint, GLsizei, const GLchar *const*, GLuint *);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetActiveUniformsiv, GLuint, GLsizei, const GLuint *, GLenum, GLint *);
RET_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_III, GLuint, glGetUniformBlockIndex, GLuint, const GLchar *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetActiveUniformBlockiv, GLuint, GLuint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glGetActiveUniformBlockName, GLuint, GLuint, GLsizei, GLsizei *, GLchar *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glUniformBlockBinding, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glDrawArraysInstanced, GLenum, GLint, GLsizei, GLsizei);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glDrawElementsInstanced, GLenum, GLsizei, GLenum, const void *, GLsizei); // TODO: Not async if rendering from client side memory
RET_PTR_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_PII, GLsync, glFenceSync, GLenum, GLbitfield);
RET_SYNC_GL_FUNCTION_1(EM_FUNC_SIG_II, GLboolean, glIsSync, GLsync);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDeleteSync, GLsync);
GLenum glClientWaitSync(GLsync p0, GLbitfield p1, GLuint64 p2) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext))
    return emscripten_glClientWaitSync(p0, p1, p2 & 0xFFFFFFFF, (p2 >> 32) & 0xFFFFFFFF);
  else
    return (GLenum)emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IIIII, &emscripten_glClientWaitSync, p0, p1, p2 & 0xFFFFFFFF, (p2 >> 32) & 0xFFFFFFFF);
}
void glWaitSync(GLsync p0, GLbitfield p1, GLuint64 p2) {
  GL_FUNCTION_TRACE();
  if (pthread_getspecific(currentThreadOwnsItsWebGLContext))
    emscripten_glWaitSync(p0, p1, p2 & 0xFFFFFFFF, (p2 >> 32) & 0xFFFFFFFF);
  else
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, &emscripten_glWaitSync, p0, p1, p2 & 0xFFFFFFFF, (p2 >> 32) & 0xFFFFFFFF);
}
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

// Extensions that are aliases for the proxying functions defined above.
// Normally these aliases get defined in library_webgl.js but when building with
// __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__ we want to intercept them in native
// code and redirect them to thier proxying couterparts.
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

#endif // ~__EMSCRIPTEN_PTHREADS__) && __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__

#if GL_ENABLE_GET_PROC_ADDRESS

// Returns a function pointer to the given WebGL 2 extension function, when queried without
// a GL extension suffix such as "EXT", "OES", or "ANGLE". This function is used by
// emscripten_GetProcAddress() to implement legacy GL emulation semantics for portability.
void *_webgl2_match_ext_proc_address_without_suffix(const char *name) {
  RETURN_FN_WITH_SUFFIX(glVertexAttribDivisor, EXT);
  RETURN_FN_WITH_SUFFIX(glVertexAttribDivisor, ARB);
  RETURN_FN_WITH_SUFFIX(glVertexAttribDivisor, ANGLE);
  RETURN_FN_WITH_SUFFIX(glDrawArraysInstanced, EXT);
  RETURN_FN_WITH_SUFFIX(glDrawArraysInstanced, ARB);
  RETURN_FN_WITH_SUFFIX(glDrawArraysInstanced, ANGLE);
  RETURN_FN_WITH_SUFFIX(glDrawElementsInstanced, NV);
  RETURN_FN_WITH_SUFFIX(glDrawElementsInstanced, EXT);
  RETURN_FN_WITH_SUFFIX(glDrawElementsInstanced, ARB);
  RETURN_FN_WITH_SUFFIX(glDrawElementsInstanced, ANGLE);
  RETURN_FN_WITH_SUFFIX(glBindVertexArray, OES);
  RETURN_FN_WITH_SUFFIX(glDeleteVertexArrays, OES);
  RETURN_FN_WITH_SUFFIX(glGenVertexArrays, OES);
  RETURN_FN_WITH_SUFFIX(glIsVertexArray, OES);
  RETURN_FN_WITH_SUFFIX(glDrawBuffers, EXT);
  RETURN_FN_WITH_SUFFIX(glDrawBuffers, WEBGL);

  return 0;
}

void *emscripten_webgl2_get_proc_address(const char *name) {
  RETURN_FN(glReadBuffer);
  RETURN_FN(glDrawRangeElements);
  RETURN_FN(glTexImage3D);
  RETURN_FN(glTexSubImage3D);
  RETURN_FN(glCopyTexSubImage3D);
  RETURN_FN(glCompressedTexImage3D);
  RETURN_FN(glCompressedTexSubImage3D);
  RETURN_FN(glGenQueries);
  RETURN_FN(glDeleteQueries);
  RETURN_FN(glIsQuery);
  RETURN_FN(glBeginQuery);
  RETURN_FN(glEndQuery);
  RETURN_FN(glGetQueryiv);
  RETURN_FN(glGetQueryObjectuiv);
#ifdef __EMSCRIPTEN_FULL_ES3__
  RETURN_FN(glUnmapBuffer);
  RETURN_FN(glGetBufferPointerv);
#endif
  RETURN_FN(glDrawBuffers);
  RETURN_FN(glUniformMatrix2x3fv);
  RETURN_FN(glUniformMatrix3x2fv);
  RETURN_FN(glUniformMatrix2x4fv);
  RETURN_FN(glUniformMatrix4x2fv);
  RETURN_FN(glUniformMatrix3x4fv);
  RETURN_FN(glUniformMatrix4x3fv);
  RETURN_FN(glBlitFramebuffer);
  RETURN_FN(glRenderbufferStorageMultisample);
  RETURN_FN(glFramebufferTextureLayer);
#ifdef __EMSCRIPTEN_FULL_ES3__
  RETURN_FN(glMapBufferRange);
  RETURN_FN(glFlushMappedBufferRange);
#endif
  RETURN_FN(glBindVertexArray);
  RETURN_FN(glDeleteVertexArrays);
  RETURN_FN(glGenVertexArrays);
  RETURN_FN(glIsVertexArray);
  RETURN_FN(glGetIntegeri_v);
  RETURN_FN(glBeginTransformFeedback);
  RETURN_FN(glEndTransformFeedback);
  RETURN_FN(glBindBufferRange);
  RETURN_FN(glBindBufferBase);
  RETURN_FN(glTransformFeedbackVaryings);
  RETURN_FN(glGetTransformFeedbackVarying);
  RETURN_FN(glVertexAttribIPointer);
  RETURN_FN(glGetVertexAttribIiv);
  RETURN_FN(glGetVertexAttribIuiv);
  RETURN_FN(glVertexAttribI4i);
  RETURN_FN(glVertexAttribI4ui);
  RETURN_FN(glVertexAttribI4iv);
  RETURN_FN(glVertexAttribI4uiv);
  RETURN_FN(glGetUniformuiv);
  RETURN_FN(glGetFragDataLocation);
  RETURN_FN(glUniform1ui);
  RETURN_FN(glUniform2ui);
  RETURN_FN(glUniform3ui);
  RETURN_FN(glUniform4ui);
  RETURN_FN(glUniform1uiv);
  RETURN_FN(glUniform2uiv);
  RETURN_FN(glUniform3uiv);
  RETURN_FN(glUniform4uiv);
  RETURN_FN(glClearBufferiv);
  RETURN_FN(glClearBufferuiv);
  RETURN_FN(glClearBufferfv);
  RETURN_FN(glClearBufferfi);
  RETURN_FN(glGetStringi);
  RETURN_FN(glCopyBufferSubData);
  RETURN_FN(glGetUniformIndices);
  RETURN_FN(glGetActiveUniformsiv);
  RETURN_FN(glGetUniformBlockIndex);
  RETURN_FN(glGetActiveUniformBlockiv);
  RETURN_FN(glGetActiveUniformBlockName);
  RETURN_FN(glUniformBlockBinding);
  RETURN_FN(glDrawArraysInstanced);
  RETURN_FN(glDrawElementsInstanced);
  RETURN_FN(glFenceSync);
  RETURN_FN(glIsSync);
  RETURN_FN(glDeleteSync);
  RETURN_FN(glClientWaitSync);
  RETURN_FN(glWaitSync);
  RETURN_FN(glGetInteger64v);
  RETURN_FN(glGetSynciv);
  RETURN_FN(glGetInteger64i_v);
  RETURN_FN(glGetBufferParameteri64v);
  RETURN_FN(glGenSamplers);
  RETURN_FN(glDeleteSamplers);
  RETURN_FN(glIsSampler);
  RETURN_FN(glBindSampler);
  RETURN_FN(glSamplerParameteri);
  RETURN_FN(glSamplerParameteriv);
  RETURN_FN(glSamplerParameterf);
  RETURN_FN(glSamplerParameterfv);
  RETURN_FN(glGetSamplerParameteriv);
  RETURN_FN(glGetSamplerParameterfv);
  RETURN_FN(glVertexAttribDivisor);
  RETURN_FN(glBindTransformFeedback);
  RETURN_FN(glDeleteTransformFeedbacks);
  RETURN_FN(glGenTransformFeedbacks);
  RETURN_FN(glIsTransformFeedback);
  RETURN_FN(glPauseTransformFeedback);
  RETURN_FN(glResumeTransformFeedback);
  RETURN_FN(glGetProgramBinary);
  RETURN_FN(glProgramBinary);
  RETURN_FN(glProgramParameteri);
  RETURN_FN(glInvalidateFramebuffer);
  RETURN_FN(glInvalidateSubFramebuffer);
  RETURN_FN(glTexStorage2D);
  RETURN_FN(glTexStorage3D);
  RETURN_FN(glGetInternalformativ);
  RETURN_FN(glVertexAttribDivisorNV);
  RETURN_FN(glVertexAttribDivisorEXT);
  RETURN_FN(glVertexAttribDivisorARB);
  RETURN_FN(glVertexAttribDivisorANGLE);
  RETURN_FN(glDrawArraysInstancedNV);
  RETURN_FN(glDrawArraysInstancedEXT);
  RETURN_FN(glDrawArraysInstancedARB);
  RETURN_FN(glDrawArraysInstancedANGLE);
  RETURN_FN(glDrawElementsInstancedNV);
  RETURN_FN(glDrawElementsInstancedEXT);
  RETURN_FN(glDrawElementsInstancedARB);
  RETURN_FN(glDrawElementsInstancedANGLE);
  RETURN_FN(glBindVertexArrayOES);
  RETURN_FN(glDeleteVertexArraysOES);
  RETURN_FN(glGenVertexArraysOES);
  RETURN_FN(glIsVertexArrayOES);
  RETURN_FN(glDrawBuffersEXT);
  RETURN_FN(glDrawBuffersWEBGL);

  // WebGL 2 extensions:
  // (currently none)

  return 0;
}

#endif // GL_ENABLE_GET_PROC_ADDRESS

#endif // defined(__EMSCRIPTEN_FULL_ES3__) || MAX_WEBGL_VERSION >= 2
