#include <emscripten/threading.h>
#include <stdlib.h> // for abort()
#include <string.h>

#include "webgl1.h"
#include "webgl1_ext.h"
#include "webgl2.h"
#include "webgl2_compute.h"

#if defined(__EMSCRIPTEN_PTHREADS__) && defined(__EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glDispatchCompute, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glDispatchComputeIndirect, GLintptr);
VOID_SYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glDrawArraysIndirect, GLenum, const void *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glDrawElementsIndirect, GLenum, GLenum, const void *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glFramebufferParameteri, GLenum, GLenum, GLint);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetFramebufferParameteriv, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetProgramInterfaceiv, GLuint, GLenum, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_8(EM_FUNC_SIG_VIIIIIIII, void, glGetProgramResourceiv, GLuint, GLenum, GLuint, GLsizei, const GLenum *, GLsizei, GLsizei *, GLint *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_IIII, GLuint, glGetProgramResourceIndex, GLuint, GLenum, const GLchar *);
VOID_SYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glGetProgramResourceName, GLuint, GLenum, GLuint, GLsizei, GLsizei *, GLchar *);
RET_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_IIII, GLint, glGetProgramResourceLocation, GLuint, GLenum, const GLchar *);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glProgramUniform1i, GLuint, GLint, GLint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glProgramUniform2i, GLuint, GLint, GLint, GLint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glProgramUniform3i, GLuint, GLint, GLint, GLint, GLint);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glProgramUniform4i, GLuint, GLint, GLint, GLint, GLint, GLint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glProgramUniform1ui, GLuint, GLint, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glProgramUniform2ui, GLuint, GLint, GLuint, GLuint);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glProgramUniform3ui, GLuint, GLint, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glProgramUniform4ui, GLuint, GLint, GLuint, GLuint, GLuint, GLuint);
ASYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIIF, void, glProgramUniform1f, GLuint, GLint, GLfloat);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIFF, void, glProgramUniform2f, GLuint, GLint, GLfloat, GLfloat);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIFFF, void, glProgramUniform3f, GLuint, GLint, GLfloat, GLfloat, GLfloat);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIFFFF, void, glProgramUniform4f, GLuint, GLint, GLfloat, GLfloat, GLfloat, GLfloat);

void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
  abort(); // TODO
}

void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
  abort(); // TODO
}

void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
  abort(); // TODO
}

void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
  abort(); // TODO
}

void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
  abort(); // TODO
}

void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
  abort(); // TODO
}

void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
  abort(); // TODO
}

void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
  abort(); // TODO
}

void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
  abort(); // TODO
}

ASYNC_GL_FUNCTION_7(EM_FUNC_SIG_VIIIIIII, void, glBindImageTexture, GLuint, GLuint, GLint, GLboolean, GLint, GLenum, GLenum);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetBooleani_v, GLenum, GLuint, GLboolean *);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glMemoryBarrier, GLbitfield);
ASYNC_GL_FUNCTION_1(EM_FUNC_SIG_VI, void, glMemoryBarrierByRegion, GLbitfield);
ASYNC_GL_FUNCTION_6(EM_FUNC_SIG_VIIIIII, void, glTexStorage2DMultisample, GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLboolean);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetTexLevelParameteriv, GLenum, GLint, GLenum, GLint *);
VOID_SYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glGetTexLevelParameterfv, GLenum, GLint, GLenum, GLfloat *);
VOID_SYNC_GL_FUNCTION_3(EM_FUNC_SIG_VIII, void, glGetMultisamplefv, GLenum, GLuint, GLfloat *);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glSampleMaski, GLuint, GLbitfield);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glBindVertexBuffer, GLuint, GLuint, GLintptr, GLsizei);
ASYNC_GL_FUNCTION_5(EM_FUNC_SIG_VIIIII, void, glVertexAttribFormat, GLuint, GLint, GLenum, GLboolean, GLuint);
ASYNC_GL_FUNCTION_4(EM_FUNC_SIG_VIIII, void, glVertexAttribIFormat, GLuint, GLint, GLenum, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexAttribBinding, GLuint, GLuint);
ASYNC_GL_FUNCTION_2(EM_FUNC_SIG_VII, void, glVertexBindingDivisor, GLuint, GLuint);

#endif // ~(__EMSCRIPTEN_PTHREADS__ && __EMSCRIPTEN_OFFSCREEN_FRAMEBUFFER__)

// Returns a function pointer to the given WebGL 2 Compute extension function, when queried without
// a GL extension suffix such as "EXT", "OES", or "ANGLE". This function is used by
// emscripten_GetProcAddress() to implement legacy GL emulation semantics for portability.
void *_webgl2_compute_match_ext_proc_address_without_suffix(const char *name)
{
  return 0;
}

void *emscripten_webgl2_compute_get_proc_address(const char *name)
{
  RETURN_FN(glDispatchCompute);
  RETURN_FN(glDispatchComputeIndirect);
  RETURN_FN(glDrawArraysIndirect);
  RETURN_FN(glDrawElementsIndirect);
  RETURN_FN(glFramebufferParameteri);
  RETURN_FN(glGetFramebufferParameteriv);
  RETURN_FN(glGetProgramInterfaceiv);
  RETURN_FN(glGetProgramResourceiv);
  RETURN_FN(glGetProgramResourceIndex);
  RETURN_FN(glGetProgramResourceName);
  RETURN_FN(glGetProgramResourceLocation);
  RETURN_FN(glProgramUniform1i);
  RETURN_FN(glProgramUniform2i);
  RETURN_FN(glProgramUniform3i);
  RETURN_FN(glProgramUniform4i);
  RETURN_FN(glProgramUniform1ui);
  RETURN_FN(glProgramUniform2ui);
  RETURN_FN(glProgramUniform3ui);
  RETURN_FN(glProgramUniform4ui);
  RETURN_FN(glProgramUniform1f);
  RETURN_FN(glProgramUniform2f);
  RETURN_FN(glProgramUniform3f);
  RETURN_FN(glProgramUniform4f);
  RETURN_FN(glProgramUniform1iv);
  RETURN_FN(glProgramUniform2iv);
  RETURN_FN(glProgramUniform3iv);
  RETURN_FN(glProgramUniform4iv);
  RETURN_FN(glProgramUniform1uiv);
  RETURN_FN(glProgramUniform2uiv);
  RETURN_FN(glProgramUniform3uiv);
  RETURN_FN(glProgramUniform4uiv);
  RETURN_FN(glProgramUniform1fv);
  RETURN_FN(glProgramUniform2fv);
  RETURN_FN(glProgramUniform3fv);
  RETURN_FN(glProgramUniform4fv);
  RETURN_FN(glProgramUniformMatrix2fv);
  RETURN_FN(glProgramUniformMatrix3fv);
  RETURN_FN(glProgramUniformMatrix4fv);
  RETURN_FN(glProgramUniformMatrix2x3fv);
  RETURN_FN(glProgramUniformMatrix3x2fv);
  RETURN_FN(glProgramUniformMatrix2x4fv);
  RETURN_FN(glProgramUniformMatrix4x2fv);
  RETURN_FN(glProgramUniformMatrix3x4fv);
  RETURN_FN(glProgramUniformMatrix4x3fv);
  RETURN_FN(glBindImageTexture);
  RETURN_FN(glGetBooleani_v);
  RETURN_FN(glMemoryBarrier);
  RETURN_FN(glMemoryBarrierByRegion);
  RETURN_FN(glTexStorage2DMultisample);
  RETURN_FN(glGetTexLevelParameteriv);
  RETURN_FN(glGetTexLevelParameterfv);
  RETURN_FN(glGetMultisamplefv);
  RETURN_FN(glSampleMaski);
  RETURN_FN(glBindVertexBuffer);
  RETURN_FN(glVertexAttribFormat);
  RETURN_FN(glVertexAttribIFormat);
  RETURN_FN(glVertexAttribBinding);
  RETURN_FN(glVertexBindingDivisor);

  // WebGL 2 Compute extensions:
  // (currently none)

  return 0;
}
