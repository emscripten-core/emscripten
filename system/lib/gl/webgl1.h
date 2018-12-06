#pragma once

#include <GLES2/gl2.h>

GL_APICALL void GL_APIENTRY emscripten_glActiveTexture (GLenum texture);
GL_APICALL void GL_APIENTRY emscripten_glAttachShader (GLuint program, GLuint shader);
GL_APICALL void GL_APIENTRY emscripten_glBindAttribLocation (GLuint program, GLuint index, const GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glBindBuffer (GLenum target, GLuint buffer);
GL_APICALL void GL_APIENTRY emscripten_glBindFramebuffer (GLenum target, GLuint framebuffer);
GL_APICALL void GL_APIENTRY emscripten_glBindRenderbuffer (GLenum target, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY emscripten_glBindTexture (GLenum target, GLuint texture);
GL_APICALL void GL_APIENTRY emscripten_glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY emscripten_glBlendEquation (GLenum mode);
GL_APICALL void GL_APIENTRY emscripten_glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha);
GL_APICALL void GL_APIENTRY emscripten_glBlendFunc (GLenum sfactor, GLenum dfactor);
GL_APICALL void GL_APIENTRY emscripten_glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GL_APICALL void GL_APIENTRY emscripten_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
GL_APICALL void GL_APIENTRY emscripten_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
GL_APICALL GLenum GL_APIENTRY emscripten_glCheckFramebufferStatus (GLenum target);
GL_APICALL void GL_APIENTRY emscripten_glClear (GLbitfield mask);
GL_APICALL void GL_APIENTRY emscripten_glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_APICALL void GL_APIENTRY emscripten_glClearDepthf (GLfloat d);
GL_APICALL void GL_APIENTRY emscripten_glClearStencil (GLint s);
GL_APICALL void GL_APIENTRY emscripten_glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GL_APICALL void GL_APIENTRY emscripten_glCompileShader (GLuint shader);
GL_APICALL void GL_APIENTRY emscripten_glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY emscripten_glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
GL_APICALL void GL_APIENTRY emscripten_glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GL_APICALL void GL_APIENTRY emscripten_glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLuint GL_APIENTRY emscripten_glCreateProgram (void);
GL_APICALL GLuint GL_APIENTRY emscripten_glCreateShader (GLenum type);
GL_APICALL void GL_APIENTRY emscripten_glCullFace (GLenum mode);
GL_APICALL void GL_APIENTRY emscripten_glDeleteBuffers (GLsizei n, const GLuint *buffers);
GL_APICALL void GL_APIENTRY emscripten_glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
GL_APICALL void GL_APIENTRY emscripten_glDeleteProgram (GLuint program);
GL_APICALL void GL_APIENTRY emscripten_glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY emscripten_glDeleteShader (GLuint shader);
GL_APICALL void GL_APIENTRY emscripten_glDeleteTextures (GLsizei n, const GLuint *textures);
GL_APICALL void GL_APIENTRY emscripten_glDepthFunc (GLenum func);
GL_APICALL void GL_APIENTRY emscripten_glDepthMask (GLboolean flag);
GL_APICALL void GL_APIENTRY emscripten_glDepthRangef (GLfloat n, GLfloat f);
GL_APICALL void GL_APIENTRY emscripten_glDetachShader (GLuint program, GLuint shader);
GL_APICALL void GL_APIENTRY emscripten_glDisable (GLenum cap);
GL_APICALL void GL_APIENTRY emscripten_glDisableVertexAttribArray (GLuint index);
GL_APICALL void GL_APIENTRY emscripten_glDrawArrays (GLenum mode, GLint first, GLsizei count);
GL_APICALL void GL_APIENTRY emscripten_glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_APICALL void GL_APIENTRY emscripten_glEnable (GLenum cap);
GL_APICALL void GL_APIENTRY emscripten_glEnableVertexAttribArray (GLuint index);
GL_APICALL void GL_APIENTRY emscripten_glFinish (void);
GL_APICALL void GL_APIENTRY emscripten_glFlush (void);
GL_APICALL void GL_APIENTRY emscripten_glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GL_APICALL void GL_APIENTRY emscripten_glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GL_APICALL void GL_APIENTRY emscripten_glFrontFace (GLenum mode);
GL_APICALL void GL_APIENTRY emscripten_glGenBuffers (GLsizei n, GLuint *buffers);
GL_APICALL void GL_APIENTRY emscripten_glGenerateMipmap (GLenum target);
GL_APICALL void GL_APIENTRY emscripten_glGenFramebuffers (GLsizei n, GLuint *framebuffers);
GL_APICALL void GL_APIENTRY emscripten_glGenRenderbuffers (GLsizei n, GLuint *renderbuffers);
GL_APICALL void GL_APIENTRY emscripten_glGenTextures (GLsizei n, GLuint *textures);
GL_APICALL void GL_APIENTRY emscripten_glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GL_APICALL GLint GL_APIENTRY emscripten_glGetAttribLocation (GLuint program, const GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glGetBooleanv (GLenum pname, GLboolean *data);
GL_APICALL void GL_APIENTRY emscripten_glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL GLenum GL_APIENTRY emscripten_glGetError (void);
GL_APICALL void GL_APIENTRY emscripten_glGetFloatv (GLenum pname, GLfloat *data);
GL_APICALL void GL_APIENTRY emscripten_glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetIntegerv (GLenum pname, GLint *data);
GL_APICALL void GL_APIENTRY emscripten_glGetProgramiv (GLuint program, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY emscripten_glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GL_APICALL void GL_APIENTRY emscripten_glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GL_APICALL void GL_APIENTRY emscripten_glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GL_APICALL const GLubyte *GL_APIENTRY emscripten_glGetString (GLenum name);
GL_APICALL void GL_APIENTRY emscripten_glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY emscripten_glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetUniformfv (GLuint program, GLint location, GLfloat *params);
GL_APICALL void GL_APIENTRY emscripten_glGetUniformiv (GLuint program, GLint location, GLint *params);
GL_APICALL GLint GL_APIENTRY emscripten_glGetUniformLocation (GLuint program, const GLchar *name);
GL_APICALL void GL_APIENTRY emscripten_glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params);
GL_APICALL void GL_APIENTRY emscripten_glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glGetVertexAttribPointerv (GLuint index, GLenum pname, void **pointer);
GL_APICALL void GL_APIENTRY emscripten_glHint (GLenum target, GLenum mode);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsBuffer (GLuint buffer);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsEnabled (GLenum cap);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsFramebuffer (GLuint framebuffer);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsProgram (GLuint program);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsRenderbuffer (GLuint renderbuffer);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsShader (GLuint shader);
GL_APICALL GLboolean GL_APIENTRY emscripten_glIsTexture (GLuint texture);
GL_APICALL void GL_APIENTRY emscripten_glLineWidth (GLfloat width);
GL_APICALL void GL_APIENTRY emscripten_glLinkProgram (GLuint program);
GL_APICALL void GL_APIENTRY emscripten_glPixelStorei (GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY emscripten_glPolygonOffset (GLfloat factor, GLfloat units);
GL_APICALL void GL_APIENTRY emscripten_glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
GL_APICALL void GL_APIENTRY emscripten_glReleaseShaderCompiler (void);
GL_APICALL void GL_APIENTRY emscripten_glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY emscripten_glSampleCoverage (GLfloat value, GLboolean invert);
GL_APICALL void GL_APIENTRY emscripten_glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY emscripten_glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
GL_APICALL void GL_APIENTRY emscripten_glShaderSource (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
GL_APICALL void GL_APIENTRY emscripten_glStencilFunc (GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY emscripten_glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
GL_APICALL void GL_APIENTRY emscripten_glStencilMask (GLuint mask);
GL_APICALL void GL_APIENTRY emscripten_glStencilMaskSeparate (GLenum face, GLuint mask);
GL_APICALL void GL_APIENTRY emscripten_glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
GL_APICALL void GL_APIENTRY emscripten_glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GL_APICALL void GL_APIENTRY emscripten_glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY emscripten_glTexParameterf (GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY emscripten_glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY emscripten_glTexParameteri (GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY emscripten_glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY emscripten_glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY emscripten_glUniform1f (GLint location, GLfloat v0);
GL_APICALL void GL_APIENTRY emscripten_glUniform1fv (GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform1i (GLint location, GLint v0);
GL_APICALL void GL_APIENTRY emscripten_glUniform1iv (GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform2f (GLint location, GLfloat v0, GLfloat v1);
GL_APICALL void GL_APIENTRY emscripten_glUniform2fv (GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform2i (GLint location, GLint v0, GLint v1);
GL_APICALL void GL_APIENTRY emscripten_glUniform2iv (GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GL_APICALL void GL_APIENTRY emscripten_glUniform3fv (GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform3i (GLint location, GLint v0, GLint v1, GLint v2);
GL_APICALL void GL_APIENTRY emscripten_glUniform3iv (GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GL_APICALL void GL_APIENTRY emscripten_glUniform4fv (GLint location, GLsizei count, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GL_APICALL void GL_APIENTRY emscripten_glUniform4iv (GLint location, GLsizei count, const GLint *value);
GL_APICALL void GL_APIENTRY emscripten_glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GL_APICALL void GL_APIENTRY emscripten_glUseProgram (GLuint program);
GL_APICALL void GL_APIENTRY emscripten_glValidateProgram (GLuint program);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib1f (GLuint index, GLfloat x);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib1fv (GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib2fv (GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib3fv (GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttrib4fv (GLuint index, const GLfloat *v);
GL_APICALL void GL_APIENTRY emscripten_glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY emscripten_glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

#ifdef EMSCRIPTEN_WEBGL_TRACE
#define GL_FUNCTION_TRACE(func) printf(#func "\n")
#else
#define GL_FUNCTION_TRACE(func) ((void)0)
#endif

#define ASYNC_GL_FUNCTION_0(sig, ret, functionName) ret functionName(void) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName); }
#define ASYNC_GL_FUNCTION_1(sig, ret, functionName, t0) ret functionName(t0 p0) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0); }
#define ASYNC_GL_FUNCTION_2(sig, ret, functionName, t0, t1) ret functionName(t0 p0, t1 p1) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1); }
#define ASYNC_GL_FUNCTION_3(sig, ret, functionName, t0, t1, t2) ret functionName(t0 p0, t1 p1, t2 p2) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2); }
#define ASYNC_GL_FUNCTION_4(sig, ret, functionName, t0, t1, t2, t3) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3); }
#define ASYNC_GL_FUNCTION_5(sig, ret, functionName, t0, t1, t2, t3, t4) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4); }
#define ASYNC_GL_FUNCTION_6(sig, ret, functionName, t0, t1, t2, t3, t4, t5) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5); }
#define ASYNC_GL_FUNCTION_7(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6); }
#define ASYNC_GL_FUNCTION_8(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7); }
#define ASYNC_GL_FUNCTION_9(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
#define ASYNC_GL_FUNCTION_10(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
#define ASYNC_GL_FUNCTION_11(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); else emscripten_async_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }

#define RET_SYNC_GL_FUNCTION_0(sig, ret, functionName) ret functionName(void) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName); }
#define RET_SYNC_GL_FUNCTION_1(sig, ret, functionName, t0) ret functionName(t0 p0) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0); }
#define RET_SYNC_GL_FUNCTION_2(sig, ret, functionName, t0, t1) ret functionName(t0 p0, t1 p1) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1); }
#define RET_SYNC_GL_FUNCTION_3(sig, ret, functionName, t0, t1, t2) ret functionName(t0 p0, t1 p1, t2 p2) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2); }
#define RET_SYNC_GL_FUNCTION_4(sig, ret, functionName, t0, t1, t2, t3) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3); }
#define RET_SYNC_GL_FUNCTION_5(sig, ret, functionName, t0, t1, t2, t3, t4) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4); }
#define RET_SYNC_GL_FUNCTION_6(sig, ret, functionName, t0, t1, t2, t3, t4, t5) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5); }
#define RET_SYNC_GL_FUNCTION_7(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6); }
#define RET_SYNC_GL_FUNCTION_8(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7); }
#define RET_SYNC_GL_FUNCTION_9(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
#define RET_SYNC_GL_FUNCTION_10(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
#define RET_SYNC_GL_FUNCTION_11(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) return emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); else return (ret)emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }

#define VOID_SYNC_GL_FUNCTION_0(sig, ret, functionName) ret functionName(void) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName); }
#define VOID_SYNC_GL_FUNCTION_1(sig, ret, functionName, t0) ret functionName(t0 p0) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0); }
#define VOID_SYNC_GL_FUNCTION_2(sig, ret, functionName, t0, t1) ret functionName(t0 p0, t1 p1) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1); }
#define VOID_SYNC_GL_FUNCTION_3(sig, ret, functionName, t0, t1, t2) ret functionName(t0 p0, t1 p1, t2 p2) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2); }
#define VOID_SYNC_GL_FUNCTION_4(sig, ret, functionName, t0, t1, t2, t3) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3); }
#define VOID_SYNC_GL_FUNCTION_5(sig, ret, functionName, t0, t1, t2, t3, t4) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4); }
#define VOID_SYNC_GL_FUNCTION_6(sig, ret, functionName, t0, t1, t2, t3, t4, t5) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5); }
#define VOID_SYNC_GL_FUNCTION_7(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6); }
#define VOID_SYNC_GL_FUNCTION_8(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7); }
#define VOID_SYNC_GL_FUNCTION_9(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8); }
#define VOID_SYNC_GL_FUNCTION_10(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9); }
#define VOID_SYNC_GL_FUNCTION_11(sig, ret, functionName, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10) ret functionName(t0 p0, t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10) { GL_FUNCTION_TRACE(functionName); if (pthread_getspecific(currentThreadOwnsItsWebGLContext)) emscripten_##functionName(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); else emscripten_sync_run_in_main_runtime_thread(sig, &emscripten_##functionName, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }

extern pthread_key_t currentActiveWebGLContext;
extern pthread_key_t currentThreadOwnsItsWebGLContext;
