/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#if GL_ENABLE_GET_PROC_ADDRESS

// GL proc address retrieval

#include <string.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5_webgl.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#ifdef LEGACY_GL_EMULATION

#include <webgl/webgl1.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2.h>

// Use emscripten_ versions of gl functions, to avoid name collisions

// Functions declared here are just the ones that we don't declare in our webgl
// headers.

/*
 * Miscellaneous
 */

GLAPI void GLAPIENTRY emscripten_glAlphaFunc( GLenum func, GLclampf ref );

GLAPI void GLAPIENTRY emscripten_glPolygonMode( GLenum face, GLenum mode );

GLAPI void GLAPIENTRY emscripten_glClipPlane( GLenum plane, const GLdouble *equation );

GLAPI void GLAPIENTRY emscripten_glDrawBuffer( GLenum mode );


GLAPI void GLAPIENTRY emscripten_glEnableClientState( GLenum cap );  /* 1.1 */

GLAPI void GLAPIENTRY emscripten_glDisableClientState( GLenum cap );  /* 1.1 */


/*
 * Transformation
 */

GLAPI void GLAPIENTRY emscripten_glMatrixMode( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glOrtho( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val );

GLAPI void GLAPIENTRY emscripten_glFrustum( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val );

GLAPI void GLAPIENTRY emscripten_glPushMatrix( void );

GLAPI void GLAPIENTRY emscripten_glPopMatrix( void );

GLAPI void GLAPIENTRY emscripten_glLoadIdentity( void );

GLAPI void GLAPIENTRY emscripten_glLoadMatrixd( const GLdouble *m );
GLAPI void GLAPIENTRY emscripten_glLoadMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY emscripten_glMultMatrixd( const GLdouble *m );
GLAPI void GLAPIENTRY emscripten_glMultMatrixf( const GLfloat *m );

GLAPI void GLAPIENTRY emscripten_glRotated( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z );
GLAPI void GLAPIENTRY emscripten_glRotatef( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY emscripten_glScaled( GLdouble x, GLdouble y, GLdouble z );
GLAPI void GLAPIENTRY emscripten_glScalef( GLfloat x, GLfloat y, GLfloat z );

GLAPI void GLAPIENTRY emscripten_glTranslated( GLdouble x, GLdouble y, GLdouble z );
GLAPI void GLAPIENTRY emscripten_glTranslatef( GLfloat x, GLfloat y, GLfloat z );

/*
 * Drawing Functions
 */

GLAPI void GLAPIENTRY emscripten_glBegin( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glEnd( void );


GLAPI void GLAPIENTRY emscripten_glVertex2d( GLdouble x, GLdouble y );
GLAPI void GLAPIENTRY emscripten_glVertex2f( GLfloat x, GLfloat y );
GLAPI void GLAPIENTRY emscripten_glVertex2i( GLint x, GLint y );
GLAPI void GLAPIENTRY emscripten_glVertex2s( GLshort x, GLshort y );

GLAPI void GLAPIENTRY emscripten_glVertex3d( GLdouble x, GLdouble y, GLdouble z );
GLAPI void GLAPIENTRY emscripten_glVertex3f( GLfloat x, GLfloat y, GLfloat z );
GLAPI void GLAPIENTRY emscripten_glVertex3i( GLint x, GLint y, GLint z );
GLAPI void GLAPIENTRY emscripten_glVertex3s( GLshort x, GLshort y, GLshort z );

GLAPI void GLAPIENTRY emscripten_glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
GLAPI void GLAPIENTRY emscripten_glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
GLAPI void GLAPIENTRY emscripten_glVertex4i( GLint x, GLint y, GLint z, GLint w );
GLAPI void GLAPIENTRY emscripten_glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w );

GLAPI void GLAPIENTRY emscripten_glVertex2dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glVertex2fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glVertex2iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glVertex2sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glVertex3dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glVertex3fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glVertex3iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glVertex3sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glVertex4dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glVertex4fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glVertex4iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glVertex4sv( const GLshort *v );


GLAPI void GLAPIENTRY emscripten_glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz );
GLAPI void GLAPIENTRY emscripten_glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz );
GLAPI void GLAPIENTRY emscripten_glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz );
GLAPI void GLAPIENTRY emscripten_glNormal3i( GLint nx, GLint ny, GLint nz );
GLAPI void GLAPIENTRY emscripten_glNormal3s( GLshort nx, GLshort ny, GLshort nz );

GLAPI void GLAPIENTRY emscripten_glNormal3bv( const GLbyte *v );
GLAPI void GLAPIENTRY emscripten_glNormal3dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glNormal3fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glNormal3iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glNormal3sv( const GLshort *v );


GLAPI void GLAPIENTRY emscripten_glColor3b( GLbyte red, GLbyte green, GLbyte blue );
GLAPI void GLAPIENTRY emscripten_glColor3d( GLdouble red, GLdouble green, GLdouble blue );
GLAPI void GLAPIENTRY emscripten_glColor3f( GLfloat red, GLfloat green, GLfloat blue );
GLAPI void GLAPIENTRY emscripten_glColor3i( GLint red, GLint green, GLint blue );
GLAPI void GLAPIENTRY emscripten_glColor3s( GLshort red, GLshort green, GLshort blue );
GLAPI void GLAPIENTRY emscripten_glColor3ub( GLubyte red, GLubyte green, GLubyte blue );
GLAPI void GLAPIENTRY emscripten_glColor3ui( GLuint red, GLuint green, GLuint blue );
GLAPI void GLAPIENTRY emscripten_glColor3us( GLushort red, GLushort green, GLushort blue );

GLAPI void GLAPIENTRY emscripten_glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha );
GLAPI void GLAPIENTRY emscripten_glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha );
GLAPI void GLAPIENTRY emscripten_glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
GLAPI void GLAPIENTRY emscripten_glColor4i( GLint red, GLint green, GLint blue, GLint alpha );
GLAPI void GLAPIENTRY emscripten_glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha );
GLAPI void GLAPIENTRY emscripten_glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha );
GLAPI void GLAPIENTRY emscripten_glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha );
GLAPI void GLAPIENTRY emscripten_glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha );


GLAPI void GLAPIENTRY emscripten_glColor3bv( const GLbyte *v );
GLAPI void GLAPIENTRY emscripten_glColor3dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glColor3fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glColor3iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glColor3sv( const GLshort *v );
GLAPI void GLAPIENTRY emscripten_glColor3ubv( const GLubyte *v );
GLAPI void GLAPIENTRY emscripten_glColor3uiv( const GLuint *v );
GLAPI void GLAPIENTRY emscripten_glColor3usv( const GLushort *v );

GLAPI void GLAPIENTRY emscripten_glColor4bv( const GLbyte *v );
GLAPI void GLAPIENTRY emscripten_glColor4dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glColor4fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glColor4iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glColor4sv( const GLshort *v );
GLAPI void GLAPIENTRY emscripten_glColor4ubv( const GLubyte *v );
GLAPI void GLAPIENTRY emscripten_glColor4uiv( const GLuint *v );
GLAPI void GLAPIENTRY emscripten_glColor4usv( const GLushort *v );


GLAPI void GLAPIENTRY emscripten_glTexCoord1d( GLdouble s );
GLAPI void GLAPIENTRY emscripten_glTexCoord1f( GLfloat s );
GLAPI void GLAPIENTRY emscripten_glTexCoord1i( GLint s );
GLAPI void GLAPIENTRY emscripten_glTexCoord1s( GLshort s );

GLAPI void GLAPIENTRY emscripten_glTexCoord2d( GLdouble s, GLdouble t );
GLAPI void GLAPIENTRY emscripten_glTexCoord2f( GLfloat s, GLfloat t );
GLAPI void GLAPIENTRY emscripten_glTexCoord2i( GLint s, GLint t );
GLAPI void GLAPIENTRY emscripten_glTexCoord2s( GLshort s, GLshort t );

GLAPI void GLAPIENTRY emscripten_glTexCoord3d( GLdouble s, GLdouble t, GLdouble r );
GLAPI void GLAPIENTRY emscripten_glTexCoord3f( GLfloat s, GLfloat t, GLfloat r );
GLAPI void GLAPIENTRY emscripten_glTexCoord3i( GLint s, GLint t, GLint r );
GLAPI void GLAPIENTRY emscripten_glTexCoord3s( GLshort s, GLshort t, GLshort r );

GLAPI void GLAPIENTRY emscripten_glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q );
GLAPI void GLAPIENTRY emscripten_glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q );
GLAPI void GLAPIENTRY emscripten_glTexCoord4i( GLint s, GLint t, GLint r, GLint q );
GLAPI void GLAPIENTRY emscripten_glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q );

GLAPI void GLAPIENTRY emscripten_glTexCoord1dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord1fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord1iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord1sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glTexCoord2dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord2fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord2iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord2sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glTexCoord3dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord3fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord3iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord3sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glTexCoord4dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord4fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord4iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glTexCoord4sv( const GLshort *v );

/*
 * Vertex Arrays  (1.1)
 */

GLAPI void GLAPIENTRY emscripten_glVertexPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glColorPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glTexCoordPointer( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glGetPointerv( GLenum pname, GLvoid **params );

/*
 * Lighting
 */

GLAPI void GLAPIENTRY emscripten_glShadeModel( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glLightf( GLenum light, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glLighti( GLenum light, GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glLightfv( GLenum light, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glLightiv( GLenum light, GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glLightModelf( GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glLightModeli( GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glLightModelfv( GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glLightModeliv( GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glMaterialf( GLenum face, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glMateriali( GLenum face, GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glMaterialiv( GLenum face, GLenum pname, const GLint *params );

/*
 * Texture mapping
 */

GLAPI void GLAPIENTRY emscripten_glTexGend( GLenum coord, GLenum pname, GLdouble param );
GLAPI void GLAPIENTRY emscripten_glTexGenf( GLenum coord, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glTexGeni( GLenum coord, GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glTexGendv( GLenum coord, GLenum pname, const GLdouble *params );
GLAPI void GLAPIENTRY emscripten_glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glTexGeniv( GLenum coord, GLenum pname, const GLint *params );


GLAPI void GLAPIENTRY emscripten_glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetTexEnviv( GLenum target, GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glGetTexLevelParameterfv( GLenum target, GLint level, GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetTexLevelParameteriv( GLenum target, GLint level, GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glTexImage1D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels );

/* 1.1 functions */

GLAPI void GLAPIENTRY emscripten_glClientActiveTexture( GLenum texture );

GLAPI void GLAPIENTRY emscripten_glLoadTransposeMatrixd( const GLdouble m[16] );

GLAPI void GLAPIENTRY emscripten_glLoadTransposeMatrixf( const GLfloat m[16] );

GLAPI void GLAPIENTRY emscripten_glMultTransposeMatrixd( const GLdouble m[16] );

GLAPI void GLAPIENTRY emscripten_glMultTransposeMatrixf( const GLfloat m[16] );

GLAPI void APIENTRY emscripten_glClientActiveTexture (GLenum texture);
GLAPI void APIENTRY emscripten_glLoadTransposeMatrixf (const GLfloat *m);
GLAPI void APIENTRY emscripten_glLoadTransposeMatrixd (const GLdouble *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixf (const GLfloat *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixd (const GLdouble *m);


GLAPI void APIENTRY emscripten_glFogCoordf (GLfloat coord);
GLAPI void APIENTRY emscripten_glFogCoordfv (const GLfloat *coord);
GLAPI void APIENTRY emscripten_glFogCoordd (GLdouble coord);
GLAPI void APIENTRY emscripten_glFogCoorddv (const GLdouble *coord);
GLAPI void APIENTRY emscripten_glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer);

GLAPI void APIENTRY emscripten_glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params);


GLAPI void APIENTRY emscripten_glDeleteObject (GLhandleARB obj);
GLAPI void APIENTRY emscripten_glGetObjectParameteriv (GLhandleARB obj, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetInfoLog (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
GLAPI void APIENTRY emscripten_glBindProgram (GLenum target, GLuint program);

#define RETURN_GL_EMU_FN(functionName) if (!strcmp(name, #functionName)) return emscripten_##functionName;

void* emscripten_legacy_gl_emulation_GetProcAddress(const char *name) {
  // main list, autogenerated from the earlier part of the file using
  /*
for line in open('a').readlines():
  if "emscripten" not in line: continue
  line = line.replace(' * ', ' ').replace('const ', '').replace('(', ' ')
  func = line.split(' ')[3].strip().replace('(', '')
  short = func.replace('EXT', '').replace('ARB', '').replace('OES', '').replace('ANGLE', '').replace('emscripten_', '')
  print '  if (!strcmp(name, "%s")) return %s;' % (short, func);
  */

  // The following list contains only exactly those functions that library_glemu.js currently implements.
  // Others will return a null pointer.

  // misc renamings
  if (!strcmp(name, "glCreateProgramObject")) return emscripten_glCreateProgram;
  if (!strcmp(name, "glUseProgramObject")) return emscripten_glUseProgram;
  if (!strcmp(name, "glCreateShaderObject")) return emscripten_glCreateShader;
  if (!strcmp(name, "glAttachObject")) return emscripten_glAttachShader;
  if (!strcmp(name, "glDetachObject")) return emscripten_glDetachShader;

  RETURN_GL_EMU_FN(glDeleteObject);
  RETURN_GL_EMU_FN(glGetObjectParameteriv);
  RETURN_GL_EMU_FN(glGetInfoLog);
  RETURN_GL_EMU_FN(glBindProgram);
  RETURN_GL_EMU_FN(glGetPointerv);
  RETURN_GL_EMU_FN(glBegin);
  RETURN_GL_EMU_FN(glEnd);
  //RETURN_GL_EMU_FN(glVertex2d);
  RETURN_GL_EMU_FN(glVertex2f);
  RETURN_GL_EMU_FN(glVertex2i);
  //RETURN_GL_EMU_FN(glVertex2s);
  //RETURN_GL_EMU_FN(glVertex3d);
  RETURN_GL_EMU_FN(glVertex3f);
  RETURN_GL_EMU_FN(glVertex3i);
  //RETURN_GL_EMU_FN(glVertex3s);
  //RETURN_GL_EMU_FN(glVertex4d);
  RETURN_GL_EMU_FN(glVertex4f);
  RETURN_GL_EMU_FN(glVertex4i);
  //RETURN_GL_EMU_FN(glVertex4s);
  //RETURN_GL_EMU_FN(glVertex2dv);
  RETURN_GL_EMU_FN(glVertex2fv);
  //RETURN_GL_EMU_FN(glVertex2iv);
  //RETURN_GL_EMU_FN(glVertex2sv);
  //RETURN_GL_EMU_FN(glVertex3dv);
  RETURN_GL_EMU_FN(glVertex3fv);
  //RETURN_GL_EMU_FN(glVertex3iv);
  //RETURN_GL_EMU_FN(glVertex3sv);
  //RETURN_GL_EMU_FN(glVertex4dv);
  RETURN_GL_EMU_FN(glVertex4fv);
  //RETURN_GL_EMU_FN(glVertex4iv);
  //RETURN_GL_EMU_FN(glVertex4sv);
  //RETURN_GL_EMU_FN(glColor3b);
  RETURN_GL_EMU_FN(glColor3d);
  RETURN_GL_EMU_FN(glColor3f);
  //RETURN_GL_EMU_FN(glColor3i);
  //RETURN_GL_EMU_FN(glColor3s);
  RETURN_GL_EMU_FN(glColor3ub);
  RETURN_GL_EMU_FN(glColor3ui);
  RETURN_GL_EMU_FN(glColor3us);
  //RETURN_GL_EMU_FN(glColor4b);
  RETURN_GL_EMU_FN(glColor4d);
  RETURN_GL_EMU_FN(glColor4f);
  //RETURN_GL_EMU_FN(glColor4i);
  //RETURN_GL_EMU_FN(glColor4s);
  RETURN_GL_EMU_FN(glColor4ub);
  RETURN_GL_EMU_FN(glColor4ui);
  RETURN_GL_EMU_FN(glColor4us);
  //RETURN_GL_EMU_FN(glColor3bv);
  //RETURN_GL_EMU_FN(glColor3dv);
  RETURN_GL_EMU_FN(glColor3fv);
  //RETURN_GL_EMU_FN(glColor3iv);
  //RETURN_GL_EMU_FN(glColor3sv);
  RETURN_GL_EMU_FN(glColor3ubv);
  RETURN_GL_EMU_FN(glColor3uiv);
  RETURN_GL_EMU_FN(glColor3usv);
  //RETURN_GL_EMU_FN(glColor4bv);
  //RETURN_GL_EMU_FN(glColor4dv);
  RETURN_GL_EMU_FN(glColor4fv);
  //RETURN_GL_EMU_FN(glColor4iv);
  //RETURN_GL_EMU_FN(glColor4sv);
  RETURN_GL_EMU_FN(glColor4ubv);
  //RETURN_GL_EMU_FN(glColor4uiv);
  //RETURN_GL_EMU_FN(glColor4usv);
  //RETURN_GL_EMU_FN(glFogCoordf);
  //RETURN_GL_EMU_FN(glFogCoordfv);
  //RETURN_GL_EMU_FN(glFogCoordd);
  //RETURN_GL_EMU_FN(glFogCoorddv);
  //RETURN_GL_EMU_FN(glFogCoordPointer);
  RETURN_GL_EMU_FN(glPolygonMode);
  RETURN_GL_EMU_FN(glAlphaFunc);
  //RETURN_GL_EMU_FN(glNormal3b);
  //RETURN_GL_EMU_FN(glNormal3d);
  RETURN_GL_EMU_FN(glNormal3f);
  //RETURN_GL_EMU_FN(glNormal3i);
  //RETURN_GL_EMU_FN(glNormal3s);
  //RETURN_GL_EMU_FN(glNormal3bv);
  //RETURN_GL_EMU_FN(glNormal3dv);
  //RETURN_GL_EMU_FN(glNormal3fv);
  //RETURN_GL_EMU_FN(glNormal3iv);
  //RETURN_GL_EMU_FN(glNormal3sv);
  RETURN_GL_EMU_FN(glDrawRangeElements);
  RETURN_GL_EMU_FN(glEnableClientState);
  RETURN_GL_EMU_FN(glDisableClientState);
  RETURN_GL_EMU_FN(glVertexPointer);
  RETURN_GL_EMU_FN(glNormalPointer);
  RETURN_GL_EMU_FN(glColorPointer);
  RETURN_GL_EMU_FN(glTexCoordPointer);
  RETURN_GL_EMU_FN(glClientActiveTexture);
  RETURN_GL_EMU_FN(glMatrixMode);
  RETURN_GL_EMU_FN(glPushMatrix);
  RETURN_GL_EMU_FN(glPopMatrix);
  RETURN_GL_EMU_FN(glLoadIdentity);
  RETURN_GL_EMU_FN(glLoadMatrixd);
  RETURN_GL_EMU_FN(glLoadMatrixf);
  RETURN_GL_EMU_FN(glLoadTransposeMatrixf);
  RETURN_GL_EMU_FN(glLoadTransposeMatrixd);
  RETURN_GL_EMU_FN(glMultTransposeMatrixf);
  RETURN_GL_EMU_FN(glMultTransposeMatrixd);
  RETURN_GL_EMU_FN(glMultMatrixd);
  RETURN_GL_EMU_FN(glMultMatrixf);
  RETURN_GL_EMU_FN(glOrtho);
  RETURN_GL_EMU_FN(glFrustum);
  RETURN_GL_EMU_FN(glRotated);
  RETURN_GL_EMU_FN(glRotatef);
  RETURN_GL_EMU_FN(glScaled);
  RETURN_GL_EMU_FN(glScalef);
  RETURN_GL_EMU_FN(glTranslated);
  RETURN_GL_EMU_FN(glTranslatef);
  RETURN_GL_EMU_FN(glClipPlane);
  RETURN_GL_EMU_FN(glDrawBuffer);
  RETURN_GL_EMU_FN(glReadBuffer);
  //RETURN_GL_EMU_FN(glLightf);
  //RETURN_GL_EMU_FN(glLighti);
  RETURN_GL_EMU_FN(glLightfv);
  //RETURN_GL_EMU_FN(glLightiv);
  RETURN_GL_EMU_FN(glLightModelf);
  //RETURN_GL_EMU_FN(glLightModeli);
  RETURN_GL_EMU_FN(glLightModelfv);
  //RETURN_GL_EMU_FN(glLightModeliv);
  //RETURN_GL_EMU_FN(glMaterialf);
  //RETURN_GL_EMU_FN(glMateriali);
  RETURN_GL_EMU_FN(glMaterialfv);
  //RETURN_GL_EMU_FN(glMaterialiv);
  //RETURN_GL_EMU_FN(glTexGend);
  //RETURN_GL_EMU_FN(glTexGenf);
  RETURN_GL_EMU_FN(glTexGeni);
  //RETURN_GL_EMU_FN(glTexGendv);
  RETURN_GL_EMU_FN(glTexGenfv);
  //RETURN_GL_EMU_FN(glTexGeniv);
  RETURN_GL_EMU_FN(glGetTexEnvfv);
  RETURN_GL_EMU_FN(glGetTexEnviv);
  RETURN_GL_EMU_FN(glTexImage1D);
  //RETURN_GL_EMU_FN(glTexCoord1d);
  //RETURN_GL_EMU_FN(glTexCoord1f);
  //RETURN_GL_EMU_FN(glTexCoord1i);
  //RETURN_GL_EMU_FN(glTexCoord1s);
  //RETURN_GL_EMU_FN(glTexCoord2d);
  RETURN_GL_EMU_FN(glTexCoord2f);
  RETURN_GL_EMU_FN(glTexCoord2i);
  //RETURN_GL_EMU_FN(glTexCoord2s);
  //RETURN_GL_EMU_FN(glTexCoord3d);
  RETURN_GL_EMU_FN(glTexCoord3f);
  //RETURN_GL_EMU_FN(glTexCoord3i);
  //RETURN_GL_EMU_FN(glTexCoord3s);
  //RETURN_GL_EMU_FN(glTexCoord4d);
  RETURN_GL_EMU_FN(glTexCoord4f);
  //RETURN_GL_EMU_FN(glTexCoord4i);
  //RETURN_GL_EMU_FN(glTexCoord4s);
  //RETURN_GL_EMU_FN(glTexCoord1dv);
  //RETURN_GL_EMU_FN(glTexCoord1fv);
  //RETURN_GL_EMU_FN(glTexCoord1iv);
  //RETURN_GL_EMU_FN(glTexCoord1sv);
  //RETURN_GL_EMU_FN(glTexCoord2dv);
  RETURN_GL_EMU_FN(glTexCoord2fv);
  //RETURN_GL_EMU_FN(glTexCoord2iv);
  //RETURN_GL_EMU_FN(glTexCoord2sv);
  //RETURN_GL_EMU_FN(glTexCoord3dv);
  //RETURN_GL_EMU_FN(glTexCoord3fv);
  //RETURN_GL_EMU_FN(glTexCoord3iv);
  //RETURN_GL_EMU_FN(glTexCoord3sv);
  //RETURN_GL_EMU_FN(glTexCoord4dv);
  //RETURN_GL_EMU_FN(glTexCoord4fv);
  //RETURN_GL_EMU_FN(glTexCoord4iv);
  //RETURN_GL_EMU_FN(glTexCoord4sv);
  //RETURN_GL_EMU_FN(glGetTexLevelParameterfv);
  RETURN_GL_EMU_FN(glGetTexLevelParameteriv);
  RETURN_GL_EMU_FN(glShadeModel);

  // GL emulation library "sloppy" lookup:
  // WebGL 1 extensions are offered without their EXT suffixes.
  if (!strcmp(name, "glBindVertexArray")) return emscripten_glBindVertexArrayOES;
  if (!strcmp(name, "glDeleteVertexArrays")) return emscripten_glDeleteVertexArraysOES;
  if (!strcmp(name, "glGenVertexArrays")) return emscripten_glGenVertexArraysOES;
  if (!strcmp(name, "glIsVertexArray")) return emscripten_glIsVertexArrayOES;
  if (!strcmp(name, "glDrawBuffers")) return emscripten_glDrawBuffersWEBGL;
  if (!strcmp(name, "glDrawArraysInstanced")) return emscripten_glDrawArraysInstancedANGLE;
  if (!strcmp(name, "glDrawElementsInstanced")) return emscripten_glDrawElementsInstancedANGLE;
  if (!strcmp(name, "glVertexAttribDivisor")) return emscripten_glVertexAttribDivisorANGLE;
  if (!strcmp(name, "glGenQueries")) return emscripten_glGenQueriesEXT;
  if (!strcmp(name, "glDeleteQueries")) return emscripten_glDeleteQueriesEXT;
  if (!strcmp(name, "glIsQuery")) return emscripten_glIsQueryEXT;
  if (!strcmp(name, "glBeginQuery")) return emscripten_glBeginQueryEXT;
  if (!strcmp(name, "glEndQuery")) return emscripten_glEndQueryEXT;
  if (!strcmp(name, "glQueryCounter")) return emscripten_glQueryCounterEXT;
  if (!strcmp(name, "glGetQueryiv")) return emscripten_glGetQueryivEXT;
  if (!strcmp(name, "glGetQueryObjectiv")) return emscripten_glGetQueryObjectivEXT;
  if (!strcmp(name, "glGetQueryObjectuiv")) return emscripten_glGetQueryObjectuivEXT;
  if (!strcmp(name, "glGetQueryObjecti64v")) return emscripten_glGetQueryObjecti64vEXT;
  if (!strcmp(name, "glGetQueryObjectui64v")) return emscripten_glGetQueryObjectui64vEXT;

  return 0;
}
#endif

void *_webgl1_match_ext_proc_address_without_suffix(const char *name);
void *_webgl2_match_ext_proc_address_without_suffix(const char *name);

// "Sloppy" desktop OpenGL/mobile GLES emulating
// behavior: different functionality is available under
// different vendor suffixes. In emscripten_GetProcAddress()
// function, all these suffixes will be ignored when performing
// the function pointer lookup.
void* emscripten_GetProcAddress(const char *name_) {
  char *name = malloc(strlen(name_)+1);
  strcpy(name, name_);
  // remove EXT|ARB|OES|ANGLE suffixes
  char *end = strstr(name, "EXT");
  if (end) *end = 0;
  end = strstr(name, "ARB");
  if (end) *end = 0;
  end = strstr(name, "OES");
  if (end) *end = 0;
  end = strstr(name, "ANGLE");
  if (end) *end = 0;
  end = strstr(name, "WEBGL");
  if (end) *end = 0;

  void *ptr = emscripten_webgl1_get_proc_address(name);

  if (!ptr) ptr = _webgl1_match_ext_proc_address_without_suffix(name);

#if LEGACY_GL_EMULATION
  if (!ptr) ptr = emscripten_legacy_gl_emulation_GetProcAddress(name);
#endif
#if MAX_WEBGL_VERSION >= 2
  if (!ptr) ptr = emscripten_webgl2_get_proc_address(name);
  if (!ptr) ptr = _webgl2_match_ext_proc_address_without_suffix(name);
#endif

  free(name);
  return ptr;
}

extern void *emscripten_webgl_get_proc_address(const char *name)
{
  void *ptr = emscripten_webgl1_get_proc_address(name);
#if MAX_WEBGL_VERSION >= 2
  if (!ptr) ptr = emscripten_webgl2_get_proc_address(name);
#endif
  return ptr;
}

#endif
