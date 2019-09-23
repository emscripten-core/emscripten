/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// GL proc address retrieval

#include <string.h>
#include <stdlib.h>
#include <emscripten.h>

#ifdef LEGACY_GL_EMULATION

#include <GL/gl.h>
#include <GL/glext.h>

#include "webgl1_ext.h"

// Define emscripten_ versions of gl functions, to avoid name collisions

/*
 * Miscellaneous
 */

GLAPI void GLAPIENTRY emscripten_glClearIndex( GLfloat c );

GLAPI void GLAPIENTRY emscripten_glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY emscripten_glClear( GLbitfield mask );

GLAPI void GLAPIENTRY emscripten_glIndexMask( GLuint mask );

GLAPI void GLAPIENTRY emscripten_glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );

GLAPI void GLAPIENTRY emscripten_glAlphaFunc( GLenum func, GLclampf ref );

GLAPI void GLAPIENTRY emscripten_glBlendFunc( GLenum sfactor, GLenum dfactor );

GLAPI void GLAPIENTRY emscripten_glLogicOp( GLenum opcode );

GLAPI void GLAPIENTRY emscripten_glCullFace( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glFrontFace( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glPointSize( GLfloat size );

GLAPI void GLAPIENTRY emscripten_glLineWidth( GLfloat width );

GLAPI void GLAPIENTRY emscripten_glLineStipple( GLint factor, GLushort pattern );

GLAPI void GLAPIENTRY emscripten_glPolygonMode( GLenum face, GLenum mode );

GLAPI void GLAPIENTRY emscripten_glPolygonOffset( GLfloat factor, GLfloat units );

GLAPI void GLAPIENTRY emscripten_glPolygonStipple( const GLubyte *mask );

GLAPI void GLAPIENTRY emscripten_glGetPolygonStipple( GLubyte *mask );

GLAPI void GLAPIENTRY emscripten_glEdgeFlag( GLboolean flag );

GLAPI void GLAPIENTRY emscripten_glEdgeFlagv( const GLboolean *flag );

GLAPI void GLAPIENTRY emscripten_glScissor( GLint x, GLint y, GLsizei width, GLsizei height);

GLAPI void GLAPIENTRY emscripten_glClipPlane( GLenum plane, const GLdouble *equation );

GLAPI void GLAPIENTRY emscripten_glGetClipPlane( GLenum plane, GLdouble *equation );

GLAPI void GLAPIENTRY emscripten_glDrawBuffer( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glReadBuffer( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glEnable( GLenum cap );

GLAPI void GLAPIENTRY emscripten_glDisable( GLenum cap );

GLAPI GLboolean GLAPIENTRY emscripten_glIsEnabled( GLenum cap );


GLAPI void GLAPIENTRY emscripten_glEnableClientState( GLenum cap );  /* 1.1 */

GLAPI void GLAPIENTRY emscripten_glDisableClientState( GLenum cap );  /* 1.1 */


GLAPI void GLAPIENTRY emscripten_glGetBooleanv( GLenum pname, GLboolean *params );

GLAPI void GLAPIENTRY emscripten_glGetDoublev( GLenum pname, GLdouble *params );

GLAPI void GLAPIENTRY emscripten_glGetFloatv( GLenum pname, GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glGetIntegerv( GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glPushAttrib( GLbitfield mask );

GLAPI void GLAPIENTRY emscripten_glPopAttrib( void );


GLAPI void GLAPIENTRY emscripten_glPushClientAttrib( GLbitfield mask );  /* 1.1 */

GLAPI void GLAPIENTRY emscripten_glPopClientAttrib( void );  /* 1.1 */


GLAPI GLint GLAPIENTRY emscripten_glRenderMode( GLenum mode );

GLAPI GLenum GLAPIENTRY emscripten_glGetError( void );

GLAPI const GLubyte * GLAPIENTRY emscripten_glGetString( GLenum name );

GLAPI void GLAPIENTRY emscripten_glFinish( void );

GLAPI void GLAPIENTRY emscripten_glFlush( void );

GLAPI void GLAPIENTRY emscripten_glHint( GLenum target, GLenum mode );


/*
 * Depth Buffer
 */

GLAPI void GLAPIENTRY emscripten_glClearDepth( GLclampd depth );

GLAPI void GLAPIENTRY emscripten_glDepthFunc( GLenum func );

GLAPI void GLAPIENTRY emscripten_glDepthMask( GLboolean flag );

GLAPI void GLAPIENTRY emscripten_glDepthRange( GLclampd near_val, GLclampd far_val );


/*
 * Accumulation Buffer
 */

GLAPI void GLAPIENTRY emscripten_glClearAccum( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );

GLAPI void GLAPIENTRY emscripten_glAccum( GLenum op, GLfloat value );


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

GLAPI void GLAPIENTRY emscripten_glViewport( GLint x, GLint y,
                                    GLsizei width, GLsizei height );

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
 * Display Lists
 */

GLAPI GLboolean GLAPIENTRY emscripten_glIsList( GLuint list );

GLAPI void GLAPIENTRY emscripten_glDeleteLists( GLuint list, GLsizei range );

GLAPI GLuint GLAPIENTRY emscripten_glGenLists( GLsizei range );

GLAPI void GLAPIENTRY emscripten_glNewList( GLuint list, GLenum mode );

GLAPI void GLAPIENTRY emscripten_glEndList( void );

GLAPI void GLAPIENTRY emscripten_glCallList( GLuint list );

GLAPI void GLAPIENTRY emscripten_glCallLists( GLsizei n, GLenum type,
                                     const GLvoid *lists );

GLAPI void GLAPIENTRY emscripten_glListBase( GLuint base );


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


GLAPI void GLAPIENTRY emscripten_glIndexd( GLdouble c );
GLAPI void GLAPIENTRY emscripten_glIndexf( GLfloat c );
GLAPI void GLAPIENTRY emscripten_glIndexi( GLint c );
GLAPI void GLAPIENTRY emscripten_glIndexs( GLshort c );
GLAPI void GLAPIENTRY emscripten_glIndexub( GLubyte c );  /* 1.1 */

GLAPI void GLAPIENTRY emscripten_glIndexdv( const GLdouble *c );
GLAPI void GLAPIENTRY emscripten_glIndexfv( const GLfloat *c );
GLAPI void GLAPIENTRY emscripten_glIndexiv( const GLint *c );
GLAPI void GLAPIENTRY emscripten_glIndexsv( const GLshort *c );
GLAPI void GLAPIENTRY emscripten_glIndexubv( const GLubyte *c );  /* 1.1 */

GLAPI void GLAPIENTRY emscripten_glColor3b( GLbyte red, GLbyte green, GLbyte blue );
GLAPI void GLAPIENTRY emscripten_glColor3d( GLdouble red, GLdouble green, GLdouble blue );
GLAPI void GLAPIENTRY emscripten_glColor3f( GLfloat red, GLfloat green, GLfloat blue );
GLAPI void GLAPIENTRY emscripten_glColor3i( GLint red, GLint green, GLint blue );
GLAPI void GLAPIENTRY emscripten_glColor3s( GLshort red, GLshort green, GLshort blue );
GLAPI void GLAPIENTRY emscripten_glColor3ub( GLubyte red, GLubyte green, GLubyte blue );
GLAPI void GLAPIENTRY emscripten_glColor3ui( GLuint red, GLuint green, GLuint blue );
GLAPI void GLAPIENTRY emscripten_glColor3us( GLushort red, GLushort green, GLushort blue );

GLAPI void GLAPIENTRY emscripten_glColor4b( GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha );
GLAPI void GLAPIENTRY emscripten_glColor4d( GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha );
GLAPI void GLAPIENTRY emscripten_glColor4f( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha );
GLAPI void GLAPIENTRY emscripten_glColor4i( GLint red, GLint green,
                                   GLint blue, GLint alpha );
GLAPI void GLAPIENTRY emscripten_glColor4s( GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha );
GLAPI void GLAPIENTRY emscripten_glColor4ub( GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha );
GLAPI void GLAPIENTRY emscripten_glColor4ui( GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha );
GLAPI void GLAPIENTRY emscripten_glColor4us( GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha );


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


GLAPI void GLAPIENTRY emscripten_glRasterPos2d( GLdouble x, GLdouble y );
GLAPI void GLAPIENTRY emscripten_glRasterPos2f( GLfloat x, GLfloat y );
GLAPI void GLAPIENTRY emscripten_glRasterPos2i( GLint x, GLint y );
GLAPI void GLAPIENTRY emscripten_glRasterPos2s( GLshort x, GLshort y );

GLAPI void GLAPIENTRY emscripten_glRasterPos3d( GLdouble x, GLdouble y, GLdouble z );
GLAPI void GLAPIENTRY emscripten_glRasterPos3f( GLfloat x, GLfloat y, GLfloat z );
GLAPI void GLAPIENTRY emscripten_glRasterPos3i( GLint x, GLint y, GLint z );
GLAPI void GLAPIENTRY emscripten_glRasterPos3s( GLshort x, GLshort y, GLshort z );

GLAPI void GLAPIENTRY emscripten_glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
GLAPI void GLAPIENTRY emscripten_glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
GLAPI void GLAPIENTRY emscripten_glRasterPos4i( GLint x, GLint y, GLint z, GLint w );
GLAPI void GLAPIENTRY emscripten_glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w );

GLAPI void GLAPIENTRY emscripten_glRasterPos2dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos2fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos2iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos2sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glRasterPos3dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos3fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos3iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos3sv( const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glRasterPos4dv( const GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos4fv( const GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos4iv( const GLint *v );
GLAPI void GLAPIENTRY emscripten_glRasterPos4sv( const GLshort *v );


GLAPI void GLAPIENTRY emscripten_glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
GLAPI void GLAPIENTRY emscripten_glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
GLAPI void GLAPIENTRY emscripten_glRecti( GLint x1, GLint y1, GLint x2, GLint y2 );
GLAPI void GLAPIENTRY emscripten_glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 );


GLAPI void GLAPIENTRY emscripten_glRectdv( const GLdouble *v1, const GLdouble *v2 );
GLAPI void GLAPIENTRY emscripten_glRectfv( const GLfloat *v1, const GLfloat *v2 );
GLAPI void GLAPIENTRY emscripten_glRectiv( const GLint *v1, const GLint *v2 );
GLAPI void GLAPIENTRY emscripten_glRectsv( const GLshort *v1, const GLshort *v2 );


/*
 * Vertex Arrays  (1.1)
 */

GLAPI void GLAPIENTRY emscripten_glVertexPointer( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glNormalPointer( GLenum type, GLsizei stride,
                                       const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glColorPointer( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glIndexPointer( GLenum type, GLsizei stride,
                                      const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glTexCoordPointer( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glEdgeFlagPointer( GLsizei stride, const GLvoid *ptr );

GLAPI void GLAPIENTRY emscripten_glGetPointerv( GLenum pname, GLvoid **params );

GLAPI void GLAPIENTRY emscripten_glArrayElement( GLint i );

GLAPI void GLAPIENTRY emscripten_glDrawArrays( GLenum mode, GLint first, GLsizei count );

GLAPI void GLAPIENTRY emscripten_glDrawElements( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY emscripten_glInterleavedArrays( GLenum format, GLsizei stride,
                                           const GLvoid *pointer );

/*
 * Lighting
 */

GLAPI void GLAPIENTRY emscripten_glShadeModel( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glLightf( GLenum light, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glLighti( GLenum light, GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glLightfv( GLenum light, GLenum pname,
                                 const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glLightiv( GLenum light, GLenum pname,
                                 const GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetLightfv( GLenum light, GLenum pname,
                                    GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetLightiv( GLenum light, GLenum pname,
                                    GLint *params );

GLAPI void GLAPIENTRY emscripten_glLightModelf( GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glLightModeli( GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glLightModelfv( GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glLightModeliv( GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glMaterialf( GLenum face, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glMateriali( GLenum face, GLenum pname, GLint param );
GLAPI void GLAPIENTRY emscripten_glMaterialfv( GLenum face, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glMaterialiv( GLenum face, GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetMaterialiv( GLenum face, GLenum pname, GLint *params );

GLAPI void GLAPIENTRY emscripten_glColorMaterial( GLenum face, GLenum mode );


/*
 * Raster functions
 */

GLAPI void GLAPIENTRY emscripten_glPixelZoom( GLfloat xfactor, GLfloat yfactor );

GLAPI void GLAPIENTRY emscripten_glPixelStoref( GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glPixelStorei( GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glPixelTransferf( GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glPixelTransferi( GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glPixelMapfv( GLenum map, GLsizei mapsize,
                                    const GLfloat *values );
GLAPI void GLAPIENTRY emscripten_glPixelMapuiv( GLenum map, GLsizei mapsize,
                                     const GLuint *values );
GLAPI void GLAPIENTRY emscripten_glPixelMapusv( GLenum map, GLsizei mapsize,
                                     const GLushort *values );

GLAPI void GLAPIENTRY emscripten_glGetPixelMapfv( GLenum map, GLfloat *values );
GLAPI void GLAPIENTRY emscripten_glGetPixelMapuiv( GLenum map, GLuint *values );
GLAPI void GLAPIENTRY emscripten_glGetPixelMapusv( GLenum map, GLushort *values );

GLAPI void GLAPIENTRY emscripten_glBitmap( GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap );

GLAPI void GLAPIENTRY emscripten_glReadPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels );

GLAPI void GLAPIENTRY emscripten_glDrawPixels( GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

GLAPI void GLAPIENTRY emscripten_glCopyPixels( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type );

/*
 * Stenciling
 */

GLAPI void GLAPIENTRY emscripten_glStencilFunc( GLenum func, GLint ref, GLuint mask );

GLAPI void GLAPIENTRY emscripten_glStencilMask( GLuint mask );

GLAPI void GLAPIENTRY emscripten_glStencilOp( GLenum fail, GLenum zfail, GLenum zpass );

GLAPI void GLAPIENTRY emscripten_glClearStencil( GLint s );



/*
 * Texture mapping
 */

GLAPI void GLAPIENTRY emscripten_glTexGend( GLenum coord, GLenum pname, GLdouble param );
GLAPI void GLAPIENTRY emscripten_glTexGenf( GLenum coord, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glTexGeni( GLenum coord, GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glTexGendv( GLenum coord, GLenum pname, const GLdouble *params );
GLAPI void GLAPIENTRY emscripten_glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glTexGeniv( GLenum coord, GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params );
GLAPI void GLAPIENTRY emscripten_glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetTexGeniv( GLenum coord, GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glTexEnvf( GLenum target, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glTexEnvi( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glTexEnvfv( GLenum target, GLenum pname, const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glTexEnviv( GLenum target, GLenum pname, const GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetTexEnviv( GLenum target, GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glTexParameterf( GLenum target, GLenum pname, GLfloat param );
GLAPI void GLAPIENTRY emscripten_glTexParameteri( GLenum target, GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glTexParameterfv( GLenum target, GLenum pname,
                                          const GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glTexParameteriv( GLenum target, GLenum pname,
                                          const GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetTexParameterfv( GLenum target,
                                           GLenum pname, GLfloat *params);
GLAPI void GLAPIENTRY emscripten_glGetTexParameteriv( GLenum target,
                                           GLenum pname, GLint *params );

GLAPI void GLAPIENTRY emscripten_glGetTexLevelParameterfv( GLenum target, GLint level,
                                                GLenum pname, GLfloat *params );
GLAPI void GLAPIENTRY emscripten_glGetTexLevelParameteriv( GLenum target, GLint level,
                                                GLenum pname, GLint *params );


GLAPI void GLAPIENTRY emscripten_glTexImage1D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

GLAPI void GLAPIENTRY emscripten_glTexImage2D( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels );

GLAPI void GLAPIENTRY emscripten_glGetTexImage( GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels );


/* 1.1 functions */

GLAPI void GLAPIENTRY emscripten_glGenTextures( GLsizei n, GLuint *textures );

GLAPI void GLAPIENTRY emscripten_glDeleteTextures( GLsizei n, const GLuint *textures);

GLAPI void GLAPIENTRY emscripten_glBindTexture( GLenum target, GLuint texture );

GLAPI void GLAPIENTRY emscripten_glPrioritizeTextures( GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities );

GLAPI GLboolean GLAPIENTRY emscripten_glAreTexturesResident( GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences );

GLAPI GLboolean GLAPIENTRY emscripten_glIsTexture( GLuint texture );


GLAPI void GLAPIENTRY emscripten_glTexSubImage1D( GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels );


GLAPI void GLAPIENTRY emscripten_glTexSubImage2D( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels );


GLAPI void GLAPIENTRY emscripten_glCopyTexImage1D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border );


GLAPI void GLAPIENTRY emscripten_glCopyTexImage2D( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border );


GLAPI void GLAPIENTRY emscripten_glCopyTexSubImage1D( GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width );


GLAPI void GLAPIENTRY emscripten_glCopyTexSubImage2D( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height );


/*
 * Evaluators
 */

GLAPI void GLAPIENTRY emscripten_glMap1d( GLenum target, GLdouble u1, GLdouble u2,
                               GLint stride,
                               GLint order, const GLdouble *points );
GLAPI void GLAPIENTRY emscripten_glMap1f( GLenum target, GLfloat u1, GLfloat u2,
                               GLint stride,
                               GLint order, const GLfloat *points );

GLAPI void GLAPIENTRY emscripten_glMap2d( GLenum target,
		     GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		     GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
		     const GLdouble *points );
GLAPI void GLAPIENTRY emscripten_glMap2f( GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points );

GLAPI void GLAPIENTRY emscripten_glGetMapdv( GLenum target, GLenum query, GLdouble *v );
GLAPI void GLAPIENTRY emscripten_glGetMapfv( GLenum target, GLenum query, GLfloat *v );
GLAPI void GLAPIENTRY emscripten_glGetMapiv( GLenum target, GLenum query, GLint *v );

GLAPI void GLAPIENTRY emscripten_glEvalCoord1d( GLdouble u );
GLAPI void GLAPIENTRY emscripten_glEvalCoord1f( GLfloat u );

GLAPI void GLAPIENTRY emscripten_glEvalCoord1dv( const GLdouble *u );
GLAPI void GLAPIENTRY emscripten_glEvalCoord1fv( const GLfloat *u );

GLAPI void GLAPIENTRY emscripten_glEvalCoord2d( GLdouble u, GLdouble v );
GLAPI void GLAPIENTRY emscripten_glEvalCoord2f( GLfloat u, GLfloat v );

GLAPI void GLAPIENTRY emscripten_glEvalCoord2dv( const GLdouble *u );
GLAPI void GLAPIENTRY emscripten_glEvalCoord2fv( const GLfloat *u );

GLAPI void GLAPIENTRY emscripten_glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 );
GLAPI void GLAPIENTRY emscripten_glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 );

GLAPI void GLAPIENTRY emscripten_glMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
                                   GLint vn, GLdouble v1, GLdouble v2 );
GLAPI void GLAPIENTRY emscripten_glMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
                                   GLint vn, GLfloat v1, GLfloat v2 );

GLAPI void GLAPIENTRY emscripten_glEvalPoint1( GLint i );

GLAPI void GLAPIENTRY emscripten_glEvalPoint2( GLint i, GLint j );

GLAPI void GLAPIENTRY emscripten_glEvalMesh1( GLenum mode, GLint i1, GLint i2 );

GLAPI void GLAPIENTRY emscripten_glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );


/*
 * Fog
 */

GLAPI void GLAPIENTRY emscripten_glFogf( GLenum pname, GLfloat param );

GLAPI void GLAPIENTRY emscripten_glFogi( GLenum pname, GLint param );

GLAPI void GLAPIENTRY emscripten_glFogfv( GLenum pname, const GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glFogiv( GLenum pname, const GLint *params );


/*
 * Selection and Feedback
 */

GLAPI void GLAPIENTRY emscripten_glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer );

GLAPI void GLAPIENTRY emscripten_glPassThrough( GLfloat token );

GLAPI void GLAPIENTRY emscripten_glSelectBuffer( GLsizei size, GLuint *buffer );

GLAPI void GLAPIENTRY emscripten_glInitNames( void );

GLAPI void GLAPIENTRY emscripten_glLoadName( GLuint name );

GLAPI void GLAPIENTRY emscripten_glPushName( GLuint name );

GLAPI void GLAPIENTRY emscripten_glPopName( void );


GLAPI void GLAPIENTRY emscripten_glDrawRangeElements( GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices );

GLAPI void GLAPIENTRY emscripten_glTexImage3D( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels );

GLAPI void GLAPIENTRY emscripten_glTexSubImage3D( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels);

GLAPI void GLAPIENTRY emscripten_glCopyTexSubImage3D( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height );


GLAPI void GLAPIENTRY emscripten_glColorTable( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table );

GLAPI void GLAPIENTRY emscripten_glColorSubTable( GLenum target,
                                       GLsizei start, GLsizei count,
                                       GLenum format, GLenum type,
                                       const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glColorTableParameteriv(GLenum target, GLenum pname,
                                              const GLint *params);

GLAPI void GLAPIENTRY emscripten_glColorTableParameterfv(GLenum target, GLenum pname,
                                              const GLfloat *params);

GLAPI void GLAPIENTRY emscripten_glCopyColorSubTable( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width );

GLAPI void GLAPIENTRY emscripten_glCopyColorTable( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width );

GLAPI void GLAPIENTRY emscripten_glGetColorTable( GLenum target, GLenum format,
                                       GLenum type, GLvoid *table );

GLAPI void GLAPIENTRY emscripten_glGetColorTableParameterfv( GLenum target, GLenum pname,
                                                  GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glGetColorTableParameteriv( GLenum target, GLenum pname,
                                                  GLint *params );

GLAPI void GLAPIENTRY emscripten_glBlendEquation( GLenum mode );

GLAPI void GLAPIENTRY emscripten_glBlendColor( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha );

GLAPI void GLAPIENTRY emscripten_glHistogram( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink );

GLAPI void GLAPIENTRY emscripten_glResetHistogram( GLenum target );

GLAPI void GLAPIENTRY emscripten_glGetHistogram( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values );

GLAPI void GLAPIENTRY emscripten_glGetHistogramParameterfv( GLenum target, GLenum pname,
						 GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glGetHistogramParameteriv( GLenum target, GLenum pname,
						 GLint *params );

GLAPI void GLAPIENTRY emscripten_glMinmax( GLenum target, GLenum internalformat,
				GLboolean sink );

GLAPI void GLAPIENTRY emscripten_glResetMinmax( GLenum target );

GLAPI void GLAPIENTRY emscripten_glGetMinmax( GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values );

GLAPI void GLAPIENTRY emscripten_glGetMinmaxParameterfv( GLenum target, GLenum pname,
					      GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glGetMinmaxParameteriv( GLenum target, GLenum pname,
					      GLint *params );

GLAPI void GLAPIENTRY emscripten_glConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image );

GLAPI void GLAPIENTRY emscripten_glConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image );

GLAPI void GLAPIENTRY emscripten_glConvolutionParameterf( GLenum target, GLenum pname,
	GLfloat params );

GLAPI void GLAPIENTRY emscripten_glConvolutionParameterfv( GLenum target, GLenum pname,
	const GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glConvolutionParameteri( GLenum target, GLenum pname,
	GLint params );

GLAPI void GLAPIENTRY emscripten_glConvolutionParameteriv( GLenum target, GLenum pname,
	const GLint *params );

GLAPI void GLAPIENTRY emscripten_glCopyConvolutionFilter1D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width );

GLAPI void GLAPIENTRY emscripten_glCopyConvolutionFilter2D( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);

GLAPI void GLAPIENTRY emscripten_glGetConvolutionFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *image );

GLAPI void GLAPIENTRY emscripten_glGetConvolutionParameterfv( GLenum target, GLenum pname,
	GLfloat *params );

GLAPI void GLAPIENTRY emscripten_glGetConvolutionParameteriv( GLenum target, GLenum pname,
	GLint *params );

GLAPI void GLAPIENTRY emscripten_glSeparableFilter2D( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column );

GLAPI void GLAPIENTRY emscripten_glGetSeparableFilter( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span );


GLAPI void GLAPIENTRY emscripten_glActiveTexture( GLenum texture );

GLAPI void GLAPIENTRY emscripten_glClientActiveTexture( GLenum texture );

GLAPI void GLAPIENTRY emscripten_glCompressedTexImage1D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glCompressedTexImage3D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glCompressedTexSubImage1D( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glCompressedTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glCompressedTexSubImage3D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data );

GLAPI void GLAPIENTRY emscripten_glGetCompressedTexImage( GLenum target, GLint lod, GLvoid *img );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1d( GLenum target, GLdouble s );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1dv( GLenum target, const GLdouble *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1f( GLenum target, GLfloat s );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1fv( GLenum target, const GLfloat *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1i( GLenum target, GLint s );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1iv( GLenum target, const GLint *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1s( GLenum target, GLshort s );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1sv( GLenum target, const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2d( GLenum target, GLdouble s, GLdouble t );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2dv( GLenum target, const GLdouble *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2f( GLenum target, GLfloat s, GLfloat t );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2fv( GLenum target, const GLfloat *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2i( GLenum target, GLint s, GLint t );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2iv( GLenum target, const GLint *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2s( GLenum target, GLshort s, GLshort t );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2sv( GLenum target, const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3d( GLenum target, GLdouble s, GLdouble t, GLdouble r );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3dv( GLenum target, const GLdouble *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3f( GLenum target, GLfloat s, GLfloat t, GLfloat r );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3fv( GLenum target, const GLfloat *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3i( GLenum target, GLint s, GLint t, GLint r );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3iv( GLenum target, const GLint *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3s( GLenum target, GLshort s, GLshort t, GLshort r );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3sv( GLenum target, const GLshort *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4d( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4dv( GLenum target, const GLdouble *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4f( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4fv( GLenum target, const GLfloat *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4i( GLenum target, GLint s, GLint t, GLint r, GLint q );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4iv( GLenum target, const GLint *v );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4s( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q );

GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4sv( GLenum target, const GLshort *v );


GLAPI void GLAPIENTRY emscripten_glLoadTransposeMatrixd( const GLdouble m[16] );

GLAPI void GLAPIENTRY emscripten_glLoadTransposeMatrixf( const GLfloat m[16] );

GLAPI void GLAPIENTRY emscripten_glMultTransposeMatrixd( const GLdouble m[16] );

GLAPI void GLAPIENTRY emscripten_glMultTransposeMatrixf( const GLfloat m[16] );

GLAPI void GLAPIENTRY emscripten_glSampleCoverage( GLclampf value, GLboolean invert );


GLAPI void GLAPIENTRY emscripten_glActiveTextureARB(GLenum texture);
GLAPI void GLAPIENTRY emscripten_glClientActiveTextureARB(GLenum texture);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1dARB(GLenum target, GLdouble s);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1dvARB(GLenum target, const GLdouble *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1fARB(GLenum target, GLfloat s);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1fvARB(GLenum target, const GLfloat *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1iARB(GLenum target, GLint s);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1ivARB(GLenum target, const GLint *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1sARB(GLenum target, GLshort s);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord1svARB(GLenum target, const GLshort *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2dvARB(GLenum target, const GLdouble *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2fvARB(GLenum target, const GLfloat *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2iARB(GLenum target, GLint s, GLint t);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2ivARB(GLenum target, const GLint *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord2svARB(GLenum target, const GLshort *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3dvARB(GLenum target, const GLdouble *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3fvARB(GLenum target, const GLfloat *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3ivARB(GLenum target, const GLint *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord3svARB(GLenum target, const GLshort *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4dvARB(GLenum target, const GLdouble *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4fvARB(GLenum target, const GLfloat *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4ivARB(GLenum target, const GLint *v);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GLAPI void GLAPIENTRY emscripten_glMultiTexCoord4svARB(GLenum target, const GLshort *v);


GLAPI void APIENTRY emscripten_glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GLAPI void APIENTRY emscripten_glBlendEquation (GLenum mode);
GLAPI void APIENTRY emscripten_glDrawRangeElements (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
GLAPI void APIENTRY emscripten_glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GLAPI void APIENTRY emscripten_glCopyTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);


GLAPI void APIENTRY emscripten_glColorTable (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
GLAPI void APIENTRY emscripten_glColorTableParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GLAPI void APIENTRY emscripten_glColorTableParameteriv (GLenum target, GLenum pname, const GLint *params);
GLAPI void APIENTRY emscripten_glCopyColorTable (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GLAPI void APIENTRY emscripten_glGetColorTable (GLenum target, GLenum format, GLenum type, GLvoid *table);
GLAPI void APIENTRY emscripten_glGetColorTableParameterfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetColorTableParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glColorSubTable (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCopyColorSubTable (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
GLAPI void APIENTRY emscripten_glConvolutionFilter1D (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
GLAPI void APIENTRY emscripten_glConvolutionFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
GLAPI void APIENTRY emscripten_glConvolutionParameterf (GLenum target, GLenum pname, GLfloat params);
GLAPI void APIENTRY emscripten_glConvolutionParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GLAPI void APIENTRY emscripten_glConvolutionParameteri (GLenum target, GLenum pname, GLint params);
GLAPI void APIENTRY emscripten_glConvolutionParameteriv (GLenum target, GLenum pname, const GLint *params);
GLAPI void APIENTRY emscripten_glCopyConvolutionFilter1D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GLAPI void APIENTRY emscripten_glCopyConvolutionFilter2D (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
GLAPI void APIENTRY emscripten_glGetConvolutionFilter (GLenum target, GLenum format, GLenum type, GLvoid *image);
GLAPI void APIENTRY emscripten_glGetConvolutionParameterfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetConvolutionParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetSeparableFilter (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
GLAPI void APIENTRY emscripten_glSeparableFilter2D (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
GLAPI void APIENTRY emscripten_glGetHistogram (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GLAPI void APIENTRY emscripten_glGetHistogramParameterfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetHistogramParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetMinmax (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GLAPI void APIENTRY emscripten_glGetMinmaxParameterfv (GLenum target, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetMinmaxParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glHistogram (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
GLAPI void APIENTRY emscripten_glMinmax (GLenum target, GLenum internalformat, GLboolean sink);
GLAPI void APIENTRY emscripten_glResetHistogram (GLenum target);
GLAPI void APIENTRY emscripten_glResetMinmax (GLenum target);


GLAPI void APIENTRY emscripten_glActiveTexture (GLenum texture);
GLAPI void APIENTRY emscripten_glSampleCoverage (GLclampf value, GLboolean invert);
GLAPI void APIENTRY emscripten_glCompressedTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glGetCompressedTexImage (GLenum target, GLint level, GLvoid *img);


GLAPI void APIENTRY emscripten_glClientActiveTexture (GLenum texture);
GLAPI void APIENTRY emscripten_glMultiTexCoord1d (GLenum target, GLdouble s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1dv (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1f (GLenum target, GLfloat s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1fv (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1i (GLenum target, GLint s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1iv (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1s (GLenum target, GLshort s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1sv (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2d (GLenum target, GLdouble s, GLdouble t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2dv (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2fv (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2i (GLenum target, GLint s, GLint t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2iv (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2s (GLenum target, GLshort s, GLshort t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2sv (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3d (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3dv (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3f (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3fv (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3i (GLenum target, GLint s, GLint t, GLint r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3iv (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3s (GLenum target, GLshort s, GLshort t, GLshort r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3sv (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4d (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4dv (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4fv (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4i (GLenum target, GLint s, GLint t, GLint r, GLint q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4iv (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4s (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4sv (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glLoadTransposeMatrixf (const GLfloat *m);
GLAPI void APIENTRY emscripten_glLoadTransposeMatrixd (const GLdouble *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixf (const GLfloat *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixd (const GLdouble *m);


GLAPI void APIENTRY emscripten_glBlendFuncSeparate (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GLAPI void APIENTRY emscripten_glMultiDrawArrays (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
GLAPI void APIENTRY emscripten_glMultiDrawElements (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
GLAPI void APIENTRY emscripten_glPointParameterf (GLenum pname, GLfloat param);
GLAPI void APIENTRY emscripten_glPointParameterfv (GLenum pname, const GLfloat *params);
GLAPI void APIENTRY emscripten_glPointParameteri (GLenum pname, GLint param);
GLAPI void APIENTRY emscripten_glPointParameteriv (GLenum pname, const GLint *params);


GLAPI void APIENTRY emscripten_glFogCoordf (GLfloat coord);
GLAPI void APIENTRY emscripten_glFogCoordfv (const GLfloat *coord);
GLAPI void APIENTRY emscripten_glFogCoordd (GLdouble coord);
GLAPI void APIENTRY emscripten_glFogCoorddv (const GLdouble *coord);
GLAPI void APIENTRY emscripten_glFogCoordPointer (GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY emscripten_glSecondaryColor3b (GLbyte red, GLbyte green, GLbyte blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3bv (const GLbyte *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3d (GLdouble red, GLdouble green, GLdouble blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3dv (const GLdouble *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3f (GLfloat red, GLfloat green, GLfloat blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3fv (const GLfloat *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3i (GLint red, GLint green, GLint blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3iv (const GLint *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3s (GLshort red, GLshort green, GLshort blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3sv (const GLshort *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3ub (GLubyte red, GLubyte green, GLubyte blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3ubv (const GLubyte *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3ui (GLuint red, GLuint green, GLuint blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3uiv (const GLuint *v);
GLAPI void APIENTRY emscripten_glSecondaryColor3us (GLushort red, GLushort green, GLushort blue);
GLAPI void APIENTRY emscripten_glSecondaryColor3usv (const GLushort *v);
GLAPI void APIENTRY emscripten_glSecondaryColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY emscripten_glWindowPos2d (GLdouble x, GLdouble y);
GLAPI void APIENTRY emscripten_glWindowPos2dv (const GLdouble *v);
GLAPI void APIENTRY emscripten_glWindowPos2f (GLfloat x, GLfloat y);
GLAPI void APIENTRY emscripten_glWindowPos2fv (const GLfloat *v);
GLAPI void APIENTRY emscripten_glWindowPos2i (GLint x, GLint y);
GLAPI void APIENTRY emscripten_glWindowPos2iv (const GLint *v);
GLAPI void APIENTRY emscripten_glWindowPos2s (GLshort x, GLshort y);
GLAPI void APIENTRY emscripten_glWindowPos2sv (const GLshort *v);
GLAPI void APIENTRY emscripten_glWindowPos3d (GLdouble x, GLdouble y, GLdouble z);
GLAPI void APIENTRY emscripten_glWindowPos3dv (const GLdouble *v);
GLAPI void APIENTRY emscripten_glWindowPos3f (GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY emscripten_glWindowPos3fv (const GLfloat *v);
GLAPI void APIENTRY emscripten_glWindowPos3i (GLint x, GLint y, GLint z);
GLAPI void APIENTRY emscripten_glWindowPos3iv (const GLint *v);
GLAPI void APIENTRY emscripten_glWindowPos3s (GLshort x, GLshort y, GLshort z);
GLAPI void APIENTRY emscripten_glWindowPos3sv (const GLshort *v);


GLAPI void APIENTRY emscripten_glGenQueries (GLsizei n, GLuint *ids);
GLAPI void APIENTRY emscripten_glDeleteQueries (GLsizei n, const GLuint *ids);
GLAPI GLboolean APIENTRY emscripten_glIsQuery (GLuint id);
GLAPI void APIENTRY emscripten_glBeginQuery (GLenum target, GLuint id);
GLAPI void APIENTRY emscripten_glEndQuery (GLenum target);
GLAPI void APIENTRY emscripten_glGetQueryiv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetQueryObjectiv (GLuint id, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params);
GLAPI void APIENTRY emscripten_glBindBuffer (GLenum target, GLuint buffer);
GLAPI void APIENTRY emscripten_glDeleteBuffers (GLsizei n, const GLuint *buffers);
GLAPI void APIENTRY emscripten_glGenBuffers (GLsizei n, GLuint *buffers);
GLAPI GLboolean APIENTRY emscripten_glIsBuffer (GLuint buffer);
GLAPI void APIENTRY emscripten_glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
GLAPI void APIENTRY emscripten_glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GLAPI void APIENTRY emscripten_glGetBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
GLAPI GLvoid* APIENTRY emscripten_glMapBuffer (GLenum target, GLenum access);
GLAPI GLboolean APIENTRY emscripten_glUnmapBuffer (GLenum target);
GLAPI void APIENTRY emscripten_glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetBufferPointerv (GLenum target, GLenum pname, GLvoid* *params);


GLAPI void APIENTRY emscripten_glAttachShader (GLuint program, GLuint shader);
GLAPI GLuint APIENTRY emscripten_glCreateProgram (void);
GLAPI GLuint APIENTRY emscripten_glCreateShader (GLenum type);
GLAPI void APIENTRY emscripten_glDetachShader (GLuint program, GLuint shader);
GLAPI void APIENTRY emscripten_glUseProgram (GLuint program);


GLAPI void APIENTRY emscripten_glDeleteObject (GLhandleARB obj);
GLAPI void APIENTRY emscripten_glGetObjectParameteriv (GLhandleARB obj, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetInfoLog (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
GLAPI void APIENTRY emscripten_glBindProgram (GLenum target, GLuint program);

#endif // ~LEGACY_GL_EMULATION

extern void *emscripten_webgl1_get_proc_address(const char *name);
extern void *_webgl1_match_ext_proc_address_without_suffix(const char *name);
extern void *emscripten_webgl2_get_proc_address(const char *name);
extern void *_webgl2_match_ext_proc_address_without_suffix(const char *name);
extern void *emscripten_webgl2_compute_get_proc_address(const char *name);
extern void *_webgl2_compute_match_ext_proc_address_without_suffix(const char *name);

#ifdef LEGACY_GL_EMULATION

#define RETURN_GL_EMU_FN(functionName) if (!strcmp(name, #functionName)) return emscripten_##functionName;

void* emscripten_legacy_gl_emulation_GetProcAddress(const char *name) {
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
  RETURN_GL_EMU_FN(glVertex2d);
  RETURN_GL_EMU_FN(glVertex2f);
  RETURN_GL_EMU_FN(glVertex2i);
  RETURN_GL_EMU_FN(glVertex2s);
  RETURN_GL_EMU_FN(glVertex3d);
  RETURN_GL_EMU_FN(glVertex3f);
  RETURN_GL_EMU_FN(glVertex3i);
  RETURN_GL_EMU_FN(glVertex3s);
  RETURN_GL_EMU_FN(glVertex4d);
  RETURN_GL_EMU_FN(glVertex4f);
  RETURN_GL_EMU_FN(glVertex4i);
  RETURN_GL_EMU_FN(glVertex4s);
  RETURN_GL_EMU_FN(glVertex2dv);
  RETURN_GL_EMU_FN(glVertex2fv);
  RETURN_GL_EMU_FN(glVertex2iv);
  RETURN_GL_EMU_FN(glVertex2sv);
  RETURN_GL_EMU_FN(glVertex3dv);
  RETURN_GL_EMU_FN(glVertex3fv);
  RETURN_GL_EMU_FN(glVertex3iv);
  RETURN_GL_EMU_FN(glVertex3sv);
  RETURN_GL_EMU_FN(glVertex4dv);
  RETURN_GL_EMU_FN(glVertex4fv);
  RETURN_GL_EMU_FN(glVertex4iv);
  RETURN_GL_EMU_FN(glVertex4sv);
  RETURN_GL_EMU_FN(glColor3b);
  RETURN_GL_EMU_FN(glColor3d);
  RETURN_GL_EMU_FN(glColor3f);
  RETURN_GL_EMU_FN(glColor3i);
  RETURN_GL_EMU_FN(glColor3s);
  RETURN_GL_EMU_FN(glColor3ub);
  RETURN_GL_EMU_FN(glColor3ui);
  RETURN_GL_EMU_FN(glColor3us);
  RETURN_GL_EMU_FN(glColor4b);
  RETURN_GL_EMU_FN(glColor4d);
  RETURN_GL_EMU_FN(glColor4f);
  RETURN_GL_EMU_FN(glColor4i);
  RETURN_GL_EMU_FN(glColor4s);
  RETURN_GL_EMU_FN(glColor4ub);
  RETURN_GL_EMU_FN(glColor4ui);
  RETURN_GL_EMU_FN(glColor4us);
  RETURN_GL_EMU_FN(glColor3bv);
  RETURN_GL_EMU_FN(glColor3dv);
  RETURN_GL_EMU_FN(glColor3fv);
  RETURN_GL_EMU_FN(glColor3iv);
  RETURN_GL_EMU_FN(glColor3sv);
  RETURN_GL_EMU_FN(glColor3ubv);
  RETURN_GL_EMU_FN(glColor3uiv);
  RETURN_GL_EMU_FN(glColor3usv);
  RETURN_GL_EMU_FN(glColor4bv);
  RETURN_GL_EMU_FN(glColor4dv);
  RETURN_GL_EMU_FN(glColor4fv);
  RETURN_GL_EMU_FN(glColor4iv);
  RETURN_GL_EMU_FN(glColor4sv);
  RETURN_GL_EMU_FN(glColor4ubv);
  RETURN_GL_EMU_FN(glColor4uiv);
  RETURN_GL_EMU_FN(glColor4usv);
  RETURN_GL_EMU_FN(glFogCoordf);
  RETURN_GL_EMU_FN(glFogCoordfv);
  RETURN_GL_EMU_FN(glFogCoordd);
  RETURN_GL_EMU_FN(glFogCoorddv);
  RETURN_GL_EMU_FN(glFogCoordPointer);
  RETURN_GL_EMU_FN(glPolygonMode);
  RETURN_GL_EMU_FN(glAlphaFunc);
  RETURN_GL_EMU_FN(glNormal3b);
  RETURN_GL_EMU_FN(glNormal3d);
  RETURN_GL_EMU_FN(glNormal3f);
  RETURN_GL_EMU_FN(glNormal3i);
  RETURN_GL_EMU_FN(glNormal3s);
  RETURN_GL_EMU_FN(glNormal3bv);
  RETURN_GL_EMU_FN(glNormal3dv);
  RETURN_GL_EMU_FN(glNormal3fv);
  RETURN_GL_EMU_FN(glNormal3iv);
  RETURN_GL_EMU_FN(glNormal3sv);
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
  RETURN_GL_EMU_FN(glDrawBuffer);
  RETURN_GL_EMU_FN(glReadBuffer);
  RETURN_GL_EMU_FN(glLightf);
  RETURN_GL_EMU_FN(glLighti);
  RETURN_GL_EMU_FN(glLightfv);
  RETURN_GL_EMU_FN(glLightiv);
  RETURN_GL_EMU_FN(glLightModelf);
  RETURN_GL_EMU_FN(glLightModeli);
  RETURN_GL_EMU_FN(glLightModelfv);
  RETURN_GL_EMU_FN(glLightModeliv);
  RETURN_GL_EMU_FN(glMaterialf);
  RETURN_GL_EMU_FN(glMateriali);
  RETURN_GL_EMU_FN(glMaterialfv);
  RETURN_GL_EMU_FN(glMaterialiv);
  RETURN_GL_EMU_FN(glTexGend);
  RETURN_GL_EMU_FN(glTexGenf);
  RETURN_GL_EMU_FN(glTexGeni);
  RETURN_GL_EMU_FN(glTexGendv);
  RETURN_GL_EMU_FN(glTexGenfv);
  RETURN_GL_EMU_FN(glTexGeniv);
  RETURN_GL_EMU_FN(glGetTexEnvfv);
  RETURN_GL_EMU_FN(glGetTexEnviv);
  RETURN_GL_EMU_FN(glTexImage1D);
  RETURN_GL_EMU_FN(glTexCoord1d);
  RETURN_GL_EMU_FN(glTexCoord1f);
  RETURN_GL_EMU_FN(glTexCoord1i);
  RETURN_GL_EMU_FN(glTexCoord1s);
  RETURN_GL_EMU_FN(glTexCoord2d);
  RETURN_GL_EMU_FN(glTexCoord2f);
  RETURN_GL_EMU_FN(glTexCoord2i);
  RETURN_GL_EMU_FN(glTexCoord2s);
  RETURN_GL_EMU_FN(glTexCoord3d);
  RETURN_GL_EMU_FN(glTexCoord3f);
  RETURN_GL_EMU_FN(glTexCoord3i);
  RETURN_GL_EMU_FN(glTexCoord3s);
  RETURN_GL_EMU_FN(glTexCoord4d);
  RETURN_GL_EMU_FN(glTexCoord4f);
  RETURN_GL_EMU_FN(glTexCoord4i);
  RETURN_GL_EMU_FN(glTexCoord4s);
  RETURN_GL_EMU_FN(glTexCoord1dv);
  RETURN_GL_EMU_FN(glTexCoord1fv);
  RETURN_GL_EMU_FN(glTexCoord1iv);
  RETURN_GL_EMU_FN(glTexCoord1sv);
  RETURN_GL_EMU_FN(glTexCoord2dv);
  RETURN_GL_EMU_FN(glTexCoord2fv);
  RETURN_GL_EMU_FN(glTexCoord2iv);
  RETURN_GL_EMU_FN(glTexCoord2sv);
  RETURN_GL_EMU_FN(glTexCoord3dv);
  RETURN_GL_EMU_FN(glTexCoord3fv);
  RETURN_GL_EMU_FN(glTexCoord3iv);
  RETURN_GL_EMU_FN(glTexCoord3sv);
  RETURN_GL_EMU_FN(glTexCoord4dv);
  RETURN_GL_EMU_FN(glTexCoord4fv);
  RETURN_GL_EMU_FN(glTexCoord4iv);
  RETURN_GL_EMU_FN(glTexCoord4sv);
  RETURN_GL_EMU_FN(glGetTexLevelParameterfv);
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

#endif // ~LEGACY_GL_EMULATION

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
#if GL_MAX_FEATURE_LEVEL >= 20
  if (!ptr) ptr = emscripten_webgl2_get_proc_address(name);
  if (!ptr) ptr = _webgl2_match_ext_proc_address_without_suffix(name);
#endif
#if GL_MAX_FEATURE_LEVEL >= 30
  if (!ptr) ptr = emscripten_webgl2_compute_get_proc_address(name);
  if (!ptr) ptr = _webgl2_compute_match_ext_proc_address_without_suffix(name);
#endif

  free(name);
  return ptr;
}

extern void *emscripten_webgl_get_proc_address(const char *name)
{
  void *ptr = emscripten_webgl1_get_proc_address(name);
#if GL_MAX_FEATURE_LEVEL >= 20
  if (!ptr) ptr = emscripten_webgl2_get_proc_address(name);
#endif
#if GL_MAX_FEATURE_LEVEL >= 30
  if (!ptr) ptr = emscripten_webgl2_compute_get_proc_address(name);
#endif
  return ptr;
}
