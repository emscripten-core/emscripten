#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

static GLuint _GL_ARRAY_BUFFER_BINDING = 0;
static GLuint _GL_ELEMENT_ARRAY_BUFFER_BINDING = 0;

GL_APICALL void GL_APIENTRY _emscripten_glBindBuffer(GLenum target, GLuint buffer);
GL_APICALL void GL_APIENTRY _emscripten_glVertexAttribDivisor(GLuint index, GLuint divisor);
GL_APICALL void GL_APIENTRY _emscripten_glEnableVertexAttribArray(GLuint index);
GL_APICALL void GL_APIENTRY _emscripten_glDisableVertexAttribArray(GLuint index);
GL_APICALL void GL_APIENTRY _emscripten_glUseProgram(GLuint program);
GL_APICALL void GL_APIENTRY _emscripten_glActiveTexture(GLenum texture);
GL_APICALL void GL_APIENTRY _emscripten_glDrawArrays(GLenum mode, GLint first, GLsizei count);
GL_APICALL void GL_APIENTRY _emscripten_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
GL_APICALL void GL_APIENTRY _emscripten_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
GL_APICALL void GL_APIENTRY _emscripten_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
GL_APICALL void GL_APIENTRY _emscripten_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
GL_APICALL void GL_APIENTRY _emscripten_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
GL_APICALL void GL_APIENTRY _emscripten_glBindTexture(GLenum target, GLuint texture);
GL_APICALL void GL_APIENTRY _emscripten_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
GL_APICALL void GL_APIENTRY _emscripten_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY _emscripten_glTexParameteri(GLenum target, GLenum pname, GLint param);
GL_APICALL void GL_APIENTRY _emscripten_glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY _emscripten_glGenerateMipmap(GLenum target);
GL_APICALL void GL_APIENTRY _emscripten_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
GL_APICALL void GL_APIENTRY _emscripten_glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
GL_APICALL void GL_APIENTRY _emscripten_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);


GL_APICALL void GL_APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
  if (target == GL_ARRAY_BUFFER)
  {
    if (_GL_ARRAY_BUFFER_BINDING == buffer) return;
    _emscripten_glBindBuffer(GL_ARRAY_BUFFER, buffer);
    _GL_ARRAY_BUFFER_BINDING = buffer;
  }
  else if (target == GL_ELEMENT_ARRAY_BUFFER)
  {
    if (_GL_ELEMENT_ARRAY_BUFFER_BINDING == buffer) return;
    _emscripten_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    _GL_ELEMENT_ARRAY_BUFFER_BINDING = buffer;
  }
  else
  {
    _emscripten_glBindBuffer(target, buffer);
  }
}

#define _GL_MAX_VERTEX_ATTRIBS 64

static uint8_t _GL_VERTEX_ATTRIB_ARRAY_DIVISOR[_GL_MAX_VERTEX_ATTRIBS];

GL_APICALL void GL_APIENTRY glVertexAttribDivisor(GLuint index, GLuint divisor)
{
  if (_GL_VERTEX_ATTRIB_ARRAY_DIVISOR[index] == (uint8_t)divisor) return;
  _emscripten_glVertexAttribDivisor(index, divisor);
  _GL_VERTEX_ATTRIB_ARRAY_DIVISOR[index] = (uint8_t)divisor;
}

static uint32_t _GL_VERTEX_ATTRIB_ARRAY_ENABLED;
static uint32_t _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED;
static uint32_t _GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED;

static inline void applyGlEnableVertexAttribArrays()
{
  _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED &= ~_GL_VERTEX_ATTRIB_ARRAY_ENABLED;
  _GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED &= _GL_VERTEX_ATTRIB_ARRAY_ENABLED;

  _GL_VERTEX_ATTRIB_ARRAY_ENABLED |= _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED;
  _GL_VERTEX_ATTRIB_ARRAY_ENABLED &= ~_GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED;

  int index = 0;
  while(_GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED)
  {
    if ((_GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED & 1)) _emscripten_glEnableVertexAttribArray(index);
    ++index;
    _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED >>= 1;
  }

  index = 0;
  while(_GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED)
  {
    if ((_GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED & 1)) _emscripten_glDisableVertexAttribArray(index);
    ++index;
    _GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED >>= 1;
  }
}

GL_APICALL void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
  _GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED &= ~(1U << index);
  _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED |= (1U << index);
}

GL_APICALL void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
  _GL_VERTEX_ATTRIB_ARRAY_pending_ENABLED &= ~(1U << index);
  _GL_VERTEX_ATTRIB_ARRAY_pending_DISABLED |= (1U << index);
}

static GLuint _GL_CURRENT_PROGRAM = 0;

GL_APICALL void GL_APIENTRY glUseProgram(GLuint program)
{
  if (program != _GL_CURRENT_PROGRAM)
  {
    _emscripten_glUseProgram(program);
    _GL_CURRENT_PROGRAM = program;
  }
}

static GLenum _GL_ACTIVE_TEXTURE = 0;
static GLenum _GL_pending_ACTIVE_TEXTURE = 0;

GL_APICALL void GL_APIENTRY glActiveTexture(GLenum texture)
{
  _GL_pending_ACTIVE_TEXTURE = texture;
}

static inline void applyGlActiveTexture()
{
  if (_GL_ACTIVE_TEXTURE == _GL_pending_ACTIVE_TEXTURE) return;
  _emscripten_glActiveTexture(_GL_pending_ACTIVE_TEXTURE);
  _GL_ACTIVE_TEXTURE = _GL_pending_ACTIVE_TEXTURE;
}

GL_APICALL void GL_APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
  applyGlEnableVertexAttribArrays();
  _emscripten_glDrawArrays(mode, first, count);
}

GL_APICALL void GL_APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
  applyGlEnableVertexAttribArrays();
  _emscripten_glDrawElements(mode, count, type, indices);
}

GL_APICALL void GL_APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
  applyGlEnableVertexAttribArrays();
  _emscripten_glDrawRangeElements(mode, start, end, count, type, indices);
}

GL_APICALL void GL_APIENTRY glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
  applyGlEnableVertexAttribArrays();
  _emscripten_glDrawArraysInstanced(mode, first, count, instancecount);
}

GL_APICALL void GL_APIENTRY glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
  applyGlEnableVertexAttribArrays();
  _emscripten_glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

#define _GL_MAX_TEXTURES 32

static GLuint _GL_TEXTURE_BINDING[_GL_MAX_TEXTURES];

GL_APICALL void GL_APIENTRY glBindTexture(GLenum target, GLuint texture)
{
  if (_GL_TEXTURE_BINDING[_GL_pending_ACTIVE_TEXTURE-GL_TEXTURE0] == texture) return;
  applyGlActiveTexture();
  _emscripten_glBindTexture(target, texture);
  _GL_TEXTURE_BINDING[_GL_pending_ACTIVE_TEXTURE-GL_TEXTURE0] = texture;
}

GL_APICALL void GL_APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
  applyGlActiveTexture();
  _emscripten_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

GL_APICALL void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
  applyGlActiveTexture();
  _emscripten_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GL_APICALL void GL_APIENTRY glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
  applyGlActiveTexture();
  _emscripten_glTexStorage2D(target, levels, internalformat, width, height);
}

GL_APICALL void GL_APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
  applyGlActiveTexture();
  _emscripten_glTexParameterf(target, pname, param);
}

GL_APICALL void GL_APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
  applyGlActiveTexture();
  _emscripten_glTexParameterfv(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
  applyGlActiveTexture();
  _emscripten_glTexParameteri(target, pname, param);
}

GL_APICALL void GL_APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
  applyGlActiveTexture();
  _emscripten_glTexParameteriv(target, pname, params);
}

GL_APICALL void GL_APIENTRY glGenerateMipmap(GLenum target)
{
  applyGlActiveTexture();
  _emscripten_glGenerateMipmap(target);
}
