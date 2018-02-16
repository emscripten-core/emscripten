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

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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

GLAPI const GLubyte * GLAPIENTRY emscripten_glGetStringi( GLenum name, GLuint index);

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

GLAPI void GLAPIENTRY emscripten_glTexStorage2D( GLenum target, GLsizei levels,
 	                                GLenum internalformat, GLsizei width,
 	                                GLsizei height );

GLAPI void GLAPIENTRY emscripten_glTexStorage3D( GLenum target, GLsizei levels,
 	                                GLenum internalformat, GLsizei width,
 	                                GLsizei height,	GLsizei depth );

GLAPI void GLAPIENTRY emscripten_glTexStorage2D( GLenum target, GLsizei levels,
                                                 GLenum internalformat, GLsizei width,
                                                 GLsizei height );

GLAPI void GLAPIENTRY emscripten_glTexStorage3D( GLenum target, GLsizei levels,
                                                 GLenum internalformat, GLsizei width,
                                                 GLsizei height, GLsizei depth );

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


GLAPI void APIENTRY emscripten_glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha);
GLAPI void APIENTRY emscripten_glDrawBuffers (GLsizei n, const GLenum *bufs);
GLAPI void APIENTRY emscripten_glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GLAPI void APIENTRY emscripten_glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask);
GLAPI void APIENTRY emscripten_glStencilMaskSeparate (GLenum face, GLuint mask);
GLAPI void APIENTRY emscripten_glAttachShader (GLuint program, GLuint shader);
GLAPI void APIENTRY emscripten_glBindAttribLocation (GLuint program, GLuint index, const GLchar *name);
GLAPI void APIENTRY emscripten_glCompileShader (GLuint shader);
GLAPI GLuint APIENTRY emscripten_glCreateProgram (void);
GLAPI GLuint APIENTRY emscripten_glCreateShader (GLenum type);
GLAPI void APIENTRY emscripten_glDeleteProgram (GLuint program);
GLAPI void APIENTRY emscripten_glDeleteShader (GLuint shader);
GLAPI void APIENTRY emscripten_glDetachShader (GLuint program, GLuint shader);
GLAPI void APIENTRY emscripten_glDisableVertexAttribArray (GLuint index);
GLAPI void APIENTRY emscripten_glEnableVertexAttribArray (GLuint index);
GLAPI void APIENTRY emscripten_glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GLAPI void APIENTRY emscripten_glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GLAPI void APIENTRY emscripten_glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
GLAPI GLint APIENTRY emscripten_glGetAttribLocation (GLuint program, const GLchar *name);
GLAPI void APIENTRY emscripten_glGetProgramiv (GLuint program, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void APIENTRY emscripten_glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void APIENTRY emscripten_glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GLAPI GLint APIENTRY emscripten_glGetUniformLocation (GLuint program, const GLchar *name);
GLAPI void APIENTRY emscripten_glGetUniformfv (GLuint program, GLint location, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetUniformiv (GLuint program, GLint location, GLint *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribdv (GLuint index, GLenum pname, GLdouble *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid* *pointer);
GLAPI GLboolean APIENTRY emscripten_glIsProgram (GLuint program);
GLAPI GLboolean APIENTRY emscripten_glIsShader (GLuint shader);
GLAPI void APIENTRY emscripten_glLinkProgram (GLuint program);
GLAPI void APIENTRY emscripten_glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
GLAPI void APIENTRY emscripten_glUseProgram (GLuint program);
GLAPI void APIENTRY emscripten_glUniform1f (GLint location, GLfloat v0);
GLAPI void APIENTRY emscripten_glUniform2f (GLint location, GLfloat v0, GLfloat v1);
GLAPI void APIENTRY emscripten_glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLAPI void APIENTRY emscripten_glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLAPI void APIENTRY emscripten_glUniform1i (GLint location, GLint v0);
GLAPI void APIENTRY emscripten_glUniform2i (GLint location, GLint v0, GLint v1);
GLAPI void APIENTRY emscripten_glUniform3i (GLint location, GLint v0, GLint v1, GLint v2);
GLAPI void APIENTRY emscripten_glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GLAPI void APIENTRY emscripten_glUniform1fv (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform2fv (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform3fv (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform4fv (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform1iv (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform2iv (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform3iv (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform4iv (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glValidateProgram (GLuint program);
GLAPI void APIENTRY emscripten_glVertexAttrib1d (GLuint index, GLdouble x);
GLAPI void APIENTRY emscripten_glVertexAttrib1dv (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib1f (GLuint index, GLfloat x);
GLAPI void APIENTRY emscripten_glVertexAttrib1fv (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib1s (GLuint index, GLshort x);
GLAPI void APIENTRY emscripten_glVertexAttrib1sv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2d (GLuint index, GLdouble x, GLdouble y);
GLAPI void APIENTRY emscripten_glVertexAttrib2dv (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y);
GLAPI void APIENTRY emscripten_glVertexAttrib2fv (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2s (GLuint index, GLshort x, GLshort y);
GLAPI void APIENTRY emscripten_glVertexAttrib2sv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3d (GLuint index, GLdouble x, GLdouble y, GLdouble z);
GLAPI void APIENTRY emscripten_glVertexAttrib3dv (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY emscripten_glVertexAttrib3fv (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3s (GLuint index, GLshort x, GLshort y, GLshort z);
GLAPI void APIENTRY emscripten_glVertexAttrib3sv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nbv (GLuint index, const GLbyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Niv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nsv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nub (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nubv (GLuint index, const GLubyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nuiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4Nusv (GLuint index, const GLushort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4bv (GLuint index, const GLbyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4d (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void APIENTRY emscripten_glVertexAttrib4dv (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void APIENTRY emscripten_glVertexAttrib4fv (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4iv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4s (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
GLAPI void APIENTRY emscripten_glVertexAttrib4sv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4ubv (GLuint index, const GLubyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4uiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4usv (GLuint index, const GLushort *v);
GLAPI void APIENTRY emscripten_glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);


GLAPI void APIENTRY emscripten_glUniformMatrix2x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix3x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix2x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix4x2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix3x4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix4x3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);


GLAPI void APIENTRY emscripten_glColorMaski (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
GLAPI void APIENTRY emscripten_glGetBooleani_v (GLenum target, GLuint index, GLboolean *data);
GLAPI void APIENTRY emscripten_glGetIntegeri_v (GLenum target, GLuint index, GLint *data);
GLAPI void APIENTRY emscripten_glEnablei (GLenum target, GLuint index);
GLAPI void APIENTRY emscripten_glDisablei (GLenum target, GLuint index);
GLAPI GLboolean APIENTRY emscripten_glIsEnabledi (GLenum target, GLuint index);
GLAPI void APIENTRY emscripten_glBeginTransformFeedback (GLenum primitiveMode);
GLAPI void APIENTRY emscripten_glEndTransformFeedback (void);
GLAPI void APIENTRY emscripten_glBindBufferRange (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GLAPI void APIENTRY emscripten_glBindBufferBase (GLenum target, GLuint index, GLuint buffer);
GLAPI void APIENTRY emscripten_glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode);
GLAPI void APIENTRY emscripten_glGetTransformFeedbackVarying (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GLAPI void APIENTRY emscripten_glClampColor (GLenum target, GLenum clamp);
GLAPI void APIENTRY emscripten_glBeginConditionalRender (GLuint id, GLenum mode);
GLAPI void APIENTRY emscripten_glEndConditionalRender (void);
GLAPI void APIENTRY emscripten_glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY emscripten_glGetVertexAttribIiv (GLuint index, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribIuiv (GLuint index, GLenum pname, GLuint *params);
GLAPI void APIENTRY emscripten_glVertexAttribI1i (GLuint index, GLint x);
GLAPI void APIENTRY emscripten_glVertexAttribI2i (GLuint index, GLint x, GLint y);
GLAPI void APIENTRY emscripten_glVertexAttribI3i (GLuint index, GLint x, GLint y, GLint z);
GLAPI void APIENTRY emscripten_glVertexAttribI4i (GLuint index, GLint x, GLint y, GLint z, GLint w);
GLAPI void APIENTRY emscripten_glVertexAttribI1ui (GLuint index, GLuint x);
GLAPI void APIENTRY emscripten_glVertexAttribI2ui (GLuint index, GLuint x, GLuint y);
GLAPI void APIENTRY emscripten_glVertexAttribI3ui (GLuint index, GLuint x, GLuint y, GLuint z);
GLAPI void APIENTRY emscripten_glVertexAttribI4ui (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GLAPI void APIENTRY emscripten_glVertexAttribI1iv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI2iv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI3iv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4iv (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI1uiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI2uiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI3uiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4uiv (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4bv (GLuint index, const GLbyte *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4sv (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4ubv (GLuint index, const GLubyte *v);
GLAPI void APIENTRY emscripten_glVertexAttribI4usv (GLuint index, const GLushort *v);
GLAPI void APIENTRY emscripten_glGetUniformuiv (GLuint program, GLint location, GLuint *params);
GLAPI void APIENTRY emscripten_glBindFragDataLocation (GLuint program, GLuint color, const GLchar *name);
GLAPI GLint APIENTRY emscripten_glGetFragDataLocation (GLuint program, const GLchar *name);
GLAPI void APIENTRY emscripten_glUniform1ui (GLint location, GLuint v0);
GLAPI void APIENTRY emscripten_glUniform2ui (GLint location, GLuint v0, GLuint v1);
GLAPI void APIENTRY emscripten_glUniform3ui (GLint location, GLuint v0, GLuint v1, GLuint v2);
GLAPI void APIENTRY emscripten_glUniform4ui (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GLAPI void APIENTRY emscripten_glUniform1uiv (GLint location, GLsizei count, const GLuint *value);
GLAPI void APIENTRY emscripten_glUniform2uiv (GLint location, GLsizei count, const GLuint *value);
GLAPI void APIENTRY emscripten_glUniform3uiv (GLint location, GLsizei count, const GLuint *value);
GLAPI void APIENTRY emscripten_glUniform4uiv (GLint location, GLsizei count, const GLuint *value);
GLAPI void APIENTRY emscripten_glTexParameterIiv (GLenum target, GLenum pname, const GLint *params);
GLAPI void APIENTRY emscripten_glTexParameterIuiv (GLenum target, GLenum pname, const GLuint *params);
GLAPI void APIENTRY emscripten_glGetTexParameterIiv (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetTexParameterIuiv (GLenum target, GLenum pname, GLuint *params);
GLAPI void APIENTRY emscripten_glClearBufferiv (GLenum buffer, GLint drawbuffer, const GLint *value);
GLAPI void APIENTRY emscripten_glClearBufferuiv (GLenum buffer, GLint drawbuffer, const GLuint *value);
GLAPI void APIENTRY emscripten_glClearBufferfv (GLenum buffer, GLint drawbuffer, const GLfloat *value);
GLAPI void APIENTRY emscripten_glClearBufferfi (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);


GLAPI void APIENTRY emscripten_glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GLAPI void APIENTRY emscripten_glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
GLAPI void APIENTRY emscripten_glTexBuffer (GLenum target, GLenum internalformat, GLuint buffer);
GLAPI void APIENTRY emscripten_glPrimitiveRestartIndex (GLuint index);


GLAPI void APIENTRY emscripten_glActiveTextureARB (GLenum texture);
GLAPI void APIENTRY emscripten_glClientActiveTextureARB (GLenum texture);
GLAPI void APIENTRY emscripten_glMultiTexCoord1dARB (GLenum target, GLdouble s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1dvARB (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1fARB (GLenum target, GLfloat s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1fvARB (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1iARB (GLenum target, GLint s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1ivARB (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord1sARB (GLenum target, GLshort s);
GLAPI void APIENTRY emscripten_glMultiTexCoord1svARB (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2dARB (GLenum target, GLdouble s, GLdouble t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2dvARB (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2fARB (GLenum target, GLfloat s, GLfloat t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2fvARB (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2iARB (GLenum target, GLint s, GLint t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2ivARB (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord2sARB (GLenum target, GLshort s, GLshort t);
GLAPI void APIENTRY emscripten_glMultiTexCoord2svARB (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3dARB (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3dvARB (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3fARB (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3fvARB (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3iARB (GLenum target, GLint s, GLint t, GLint r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3ivARB (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord3sARB (GLenum target, GLshort s, GLshort t, GLshort r);
GLAPI void APIENTRY emscripten_glMultiTexCoord3svARB (GLenum target, const GLshort *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4dARB (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4dvARB (GLenum target, const GLdouble *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4fARB (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4fvARB (GLenum target, const GLfloat *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4iARB (GLenum target, GLint s, GLint t, GLint r, GLint q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4ivARB (GLenum target, const GLint *v);
GLAPI void APIENTRY emscripten_glMultiTexCoord4sARB (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GLAPI void APIENTRY emscripten_glMultiTexCoord4svARB (GLenum target, const GLshort *v);


GLAPI void APIENTRY emscripten_glLoadTransposeMatrixfARB (const GLfloat *m);
GLAPI void APIENTRY emscripten_glLoadTransposeMatrixdARB (const GLdouble *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixfARB (const GLfloat *m);
GLAPI void APIENTRY emscripten_glMultTransposeMatrixdARB (const GLdouble *m);


GLAPI void APIENTRY emscripten_glCompressedTexImage3DARB (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexImage2DARB (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexImage1DARB (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage3DARB (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage2DARB (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glCompressedTexSubImage1DARB (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GLAPI void APIENTRY emscripten_glGetCompressedTexImageARB (GLenum target, GLint level, GLvoid *img);


GLAPI void APIENTRY emscripten_glVertexAttrib1dARB (GLuint index, GLdouble x);
GLAPI void APIENTRY emscripten_glVertexAttrib1dvARB (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib1fARB (GLuint index, GLfloat x);
GLAPI void APIENTRY emscripten_glVertexAttrib1fvARB (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib1sARB (GLuint index, GLshort x);
GLAPI void APIENTRY emscripten_glVertexAttrib1svARB (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2dARB (GLuint index, GLdouble x, GLdouble y);
GLAPI void APIENTRY emscripten_glVertexAttrib2dvARB (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2fARB (GLuint index, GLfloat x, GLfloat y);
GLAPI void APIENTRY emscripten_glVertexAttrib2fvARB (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib2sARB (GLuint index, GLshort x, GLshort y);
GLAPI void APIENTRY emscripten_glVertexAttrib2svARB (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3dARB (GLuint index, GLdouble x, GLdouble y, GLdouble z);
GLAPI void APIENTRY emscripten_glVertexAttrib3dvARB (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3fARB (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GLAPI void APIENTRY emscripten_glVertexAttrib3fvARB (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib3sARB (GLuint index, GLshort x, GLshort y, GLshort z);
GLAPI void APIENTRY emscripten_glVertexAttrib3svARB (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NbvARB (GLuint index, const GLbyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NivARB (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NsvARB (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NubARB (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GLAPI void APIENTRY emscripten_glVertexAttrib4NubvARB (GLuint index, const GLubyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NuivARB (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4NusvARB (GLuint index, const GLushort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4bvARB (GLuint index, const GLbyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4dARB (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void APIENTRY emscripten_glVertexAttrib4dvARB (GLuint index, const GLdouble *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4fARB (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void APIENTRY emscripten_glVertexAttrib4fvARB (GLuint index, const GLfloat *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4ivARB (GLuint index, const GLint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4sARB (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
GLAPI void APIENTRY emscripten_glVertexAttrib4svARB (GLuint index, const GLshort *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4ubvARB (GLuint index, const GLubyte *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4uivARB (GLuint index, const GLuint *v);
GLAPI void APIENTRY emscripten_glVertexAttrib4usvARB (GLuint index, const GLushort *v);
GLAPI void APIENTRY emscripten_glVertexAttribPointerARB (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY emscripten_glEnableVertexAttribArrayARB (GLuint index);
GLAPI void APIENTRY emscripten_glDisableVertexAttribArrayARB (GLuint index);
GLAPI void APIENTRY emscripten_glProgramStringARB (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
GLAPI void APIENTRY emscripten_glBindProgramARB (GLenum target, GLuint program);
GLAPI void APIENTRY emscripten_glDeleteProgramsARB (GLsizei n, const GLuint *programs);
GLAPI void APIENTRY emscripten_glGenProgramsARB (GLsizei n, GLuint *programs);
GLAPI void APIENTRY emscripten_glProgramEnvParameter4dARB (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void APIENTRY emscripten_glProgramEnvParameter4dvARB (GLenum target, GLuint index, const GLdouble *params);
GLAPI void APIENTRY emscripten_glProgramEnvParameter4fARB (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void APIENTRY emscripten_glProgramEnvParameter4fvARB (GLenum target, GLuint index, const GLfloat *params);
GLAPI void APIENTRY emscripten_glProgramLocalParameter4dARB (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GLAPI void APIENTRY emscripten_glProgramLocalParameter4dvARB (GLenum target, GLuint index, const GLdouble *params);
GLAPI void APIENTRY emscripten_glProgramLocalParameter4fARB (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GLAPI void APIENTRY emscripten_glProgramLocalParameter4fvARB (GLenum target, GLuint index, const GLfloat *params);
GLAPI void APIENTRY emscripten_glGetProgramEnvParameterdvARB (GLenum target, GLuint index, GLdouble *params);
GLAPI void APIENTRY emscripten_glGetProgramEnvParameterfvARB (GLenum target, GLuint index, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetProgramLocalParameterdvARB (GLenum target, GLuint index, GLdouble *params);
GLAPI void APIENTRY emscripten_glGetProgramLocalParameterfvARB (GLenum target, GLuint index, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetProgramivARB (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetProgramStringARB (GLenum target, GLenum pname, GLvoid *string);
GLAPI void APIENTRY emscripten_glGetVertexAttribdvARB (GLuint index, GLenum pname, GLdouble *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribfvARB (GLuint index, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribivARB (GLuint index, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetVertexAttribPointervARB (GLuint index, GLenum pname, GLvoid* *pointer);
GLAPI GLboolean APIENTRY emscripten_glIsProgramARB (GLuint program);


GLAPI void APIENTRY emscripten_glBindBufferARB (GLenum target, GLuint buffer);
GLAPI void APIENTRY emscripten_glDeleteBuffersARB (GLsizei n, const GLuint *buffers);
GLAPI void APIENTRY emscripten_glGenBuffersARB (GLsizei n, GLuint *buffers);
GLAPI GLboolean APIENTRY emscripten_glIsBufferARB (GLuint buffer);
GLAPI void APIENTRY emscripten_glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
GLAPI void APIENTRY emscripten_glBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
GLAPI void APIENTRY emscripten_glGetBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
GLAPI GLvoid* APIENTRY emscripten_glMapBufferARB (GLenum target, GLenum access);
GLAPI GLboolean APIENTRY emscripten_glUnmapBufferARB (GLenum target);
GLAPI void APIENTRY emscripten_glGetBufferParameterivARB (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetBufferPointervARB (GLenum target, GLenum pname, GLvoid* *params);


GLAPI void APIENTRY emscripten_glGenQueriesARB (GLsizei n, GLuint *ids);
GLAPI void APIENTRY emscripten_glDeleteQueriesARB (GLsizei n, const GLuint *ids);
GLAPI GLboolean APIENTRY emscripten_glIsQueryARB (GLuint id);
GLAPI void APIENTRY emscripten_glBeginQueryARB (GLenum target, GLuint id);
GLAPI void APIENTRY emscripten_glEndQueryARB (GLenum target);
GLAPI void APIENTRY emscripten_glGetQueryivARB (GLenum target, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetQueryObjectivARB (GLuint id, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetQueryObjectuivARB (GLuint id, GLenum pname, GLuint *params);


GLAPI void APIENTRY emscripten_glDeleteObjectARB (GLhandleARB obj);
GLAPI GLhandleARB APIENTRY emscripten_glGetHandleARB (GLenum pname);
GLAPI void APIENTRY emscripten_glDetachObjectARB (GLhandleARB containerObj, GLhandleARB attachedObj);
GLAPI GLhandleARB APIENTRY emscripten_glCreateShaderObjectARB (GLenum shaderType);
GLAPI void APIENTRY emscripten_glShaderSourceARB (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
GLAPI void APIENTRY emscripten_glCompileShaderARB (GLhandleARB shaderObj);
GLAPI GLhandleARB APIENTRY emscripten_glCreateProgramObjectARB (void);
GLAPI void APIENTRY emscripten_glAttachObjectARB (GLhandleARB containerObj, GLhandleARB obj);
GLAPI void APIENTRY emscripten_glLinkProgramARB (GLhandleARB programObj);
GLAPI void APIENTRY emscripten_glUseProgramObjectARB (GLhandleARB programObj);
GLAPI void APIENTRY emscripten_glValidateProgramARB (GLhandleARB programObj);
GLAPI void APIENTRY emscripten_glUniform1fARB (GLint location, GLfloat v0);
GLAPI void APIENTRY emscripten_glUniform2fARB (GLint location, GLfloat v0, GLfloat v1);
GLAPI void APIENTRY emscripten_glUniform3fARB (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GLAPI void APIENTRY emscripten_glUniform4fARB (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GLAPI void APIENTRY emscripten_glUniform1iARB (GLint location, GLint v0);
GLAPI void APIENTRY emscripten_glUniform2iARB (GLint location, GLint v0, GLint v1);
GLAPI void APIENTRY emscripten_glUniform3iARB (GLint location, GLint v0, GLint v1, GLint v2);
GLAPI void APIENTRY emscripten_glUniform4iARB (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GLAPI void APIENTRY emscripten_glUniform1fvARB (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform2fvARB (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform3fvARB (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform4fvARB (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniform1ivARB (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform2ivARB (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform3ivARB (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniform4ivARB (GLint location, GLsizei count, const GLint *value);
GLAPI void APIENTRY emscripten_glUniformMatrix2fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix3fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glUniformMatrix4fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY emscripten_glGetObjectParameterfvARB (GLhandleARB obj, GLenum pname, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetObjectParameterivARB (GLhandleARB obj, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetInfoLogARB (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
GLAPI void APIENTRY emscripten_glGetAttachedObjectsARB (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
GLAPI GLint APIENTRY emscripten_glGetUniformLocationARB (GLhandleARB programObj, const GLcharARB *name);
GLAPI void APIENTRY emscripten_glGetActiveUniformARB (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
GLAPI void APIENTRY emscripten_glGetUniformfvARB (GLhandleARB programObj, GLint location, GLfloat *params);
GLAPI void APIENTRY emscripten_glGetUniformivARB (GLhandleARB programObj, GLint location, GLint *params);
GLAPI void APIENTRY emscripten_glGetShaderSourceARB (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);


GLAPI void APIENTRY emscripten_glBindAttribLocationARB (GLhandleARB programObj, GLuint index, const GLcharARB *name);
GLAPI void APIENTRY emscripten_glGetActiveAttribARB (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
GLAPI GLint APIENTRY emscripten_glGetAttribLocationARB (GLhandleARB programObj, const GLcharARB *name);


GLAPI void APIENTRY emscripten_glDrawArraysInstancedARB (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GLAPI void APIENTRY emscripten_glDrawElementsInstancedARB (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);


GLAPI GLboolean APIENTRY emscripten_glIsRenderbuffer (GLuint renderbuffer);
GLAPI void APIENTRY emscripten_glBindRenderbuffer (GLenum target, GLuint renderbuffer);
GLAPI void APIENTRY emscripten_glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers);
GLAPI void APIENTRY emscripten_glGenRenderbuffers (GLsizei n, GLuint *renderbuffers);
GLAPI void APIENTRY emscripten_glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GLAPI void APIENTRY emscripten_glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params);
GLAPI GLboolean APIENTRY emscripten_glIsFramebuffer (GLuint framebuffer);
GLAPI void APIENTRY emscripten_glBindFramebuffer (GLenum target, GLuint framebuffer);
GLAPI void APIENTRY emscripten_glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
GLAPI void APIENTRY emscripten_glGenFramebuffers (GLsizei n, GLuint *framebuffers);
GLAPI GLenum APIENTRY emscripten_glCheckFramebufferStatus (GLenum target);
GLAPI void APIENTRY emscripten_glFramebufferTexture1D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLAPI void APIENTRY emscripten_glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLAPI void APIENTRY emscripten_glFramebufferTexture3D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GLAPI void APIENTRY emscripten_glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GLAPI void APIENTRY emscripten_glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGenerateMipmap (GLenum target);
GLAPI void APIENTRY emscripten_glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GLAPI void APIENTRY emscripten_glRenderbufferStorageMultisample (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GLAPI void APIENTRY emscripten_glFramebufferTextureLayer (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);


GLAPI void APIENTRY emscripten_glBindVertexArray (GLuint array);
GLAPI void APIENTRY emscripten_glDeleteVertexArrays (GLsizei n, const GLuint *arrays);
GLAPI void APIENTRY emscripten_glGenVertexArrays (GLsizei n, GLuint *arrays);
GLAPI GLboolean APIENTRY emscripten_glIsVertexArray (GLuint array);


GLAPI void APIENTRY emscripten_glGetUniformIndices (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices);
GLAPI void APIENTRY emscripten_glGetActiveUniformsiv (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetActiveUniformName (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
GLAPI GLuint APIENTRY emscripten_glGetUniformBlockIndex (GLuint program, const GLchar *uniformBlockName);
GLAPI void APIENTRY emscripten_glGetActiveUniformBlockiv (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GLAPI void APIENTRY emscripten_glGetActiveUniformBlockName (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
GLAPI void APIENTRY emscripten_glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);


GLAPI void APIENTRY emscripten_glReleaseShaderCompiler (void);
GLAPI void APIENTRY emscripten_glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length);
GLAPI void APIENTRY emscripten_glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
GLAPI void APIENTRY emscripten_glDepthRangef (GLclampf n, GLclampf f);
GLAPI void APIENTRY emscripten_glClearDepthf (GLclampf d);


GLAPI void APIENTRY emscripten_glVertexAttribDivisor (GLuint index, GLuint divisor);


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
  // misc renamings
  if (!strcmp(name, "glCreateProgramObject")) name = "glCreateProgram";
  else if (!strcmp(name, "glUseProgramObject")) name = "glUseProgram";
  else if (!strcmp(name, "glCreateShaderObject")) name = "glCreateShader";
  else if (!strcmp(name, "glAttachObject")) name = "glAttachShader";
  else if (!strcmp(name, "glDetachObject")) name = "glDetachShader";
  // main list, autogenerated from the earlier part of the file using
  /*
for line in open('a').readlines():
  if "emscripten" not in line: continue
  line = line.replace(' * ', ' ').replace('const ', '').replace('(', ' ')
  func = line.split(' ')[3].strip().replace('(', '')
  short = func.replace('EXT', '').replace('ARB', '').replace('OES', '').replace('ANGLE', '').replace('emscripten_', '')
  print '  if (!strcmp(name, "%s")) return %s;' % (short, func);
  */
  if (!strcmp(name, "glClearIndex")) return emscripten_glClearIndex;
  if (!strcmp(name, "glClearColor")) return emscripten_glClearColor;
  if (!strcmp(name, "glClear")) return emscripten_glClear;
  if (!strcmp(name, "glIndexMask")) return emscripten_glIndexMask;
  if (!strcmp(name, "glColorMask")) return emscripten_glColorMask;
  if (!strcmp(name, "glAlphaFunc")) return emscripten_glAlphaFunc;
  if (!strcmp(name, "glBlendFunc")) return emscripten_glBlendFunc;
  if (!strcmp(name, "glLogicOp")) return emscripten_glLogicOp;
  if (!strcmp(name, "glCullFace")) return emscripten_glCullFace;
  if (!strcmp(name, "glFrontFace")) return emscripten_glFrontFace;
  if (!strcmp(name, "glPointSize")) return emscripten_glPointSize;
  if (!strcmp(name, "glLineWidth")) return emscripten_glLineWidth;
  if (!strcmp(name, "glLineStipple")) return emscripten_glLineStipple;
  if (!strcmp(name, "glPolygonMode")) return emscripten_glPolygonMode;
  if (!strcmp(name, "glPolygonOffset")) return emscripten_glPolygonOffset;
  if (!strcmp(name, "glPolygonStipple")) return emscripten_glPolygonStipple;
  if (!strcmp(name, "glGetPolygonStipple")) return emscripten_glGetPolygonStipple;
  if (!strcmp(name, "glEdgeFlag")) return emscripten_glEdgeFlag;
  if (!strcmp(name, "glEdgeFlagv")) return emscripten_glEdgeFlagv;
  if (!strcmp(name, "glScissor")) return emscripten_glScissor;
  if (!strcmp(name, "glClipPlane")) return emscripten_glClipPlane;
  if (!strcmp(name, "glGetClipPlane")) return emscripten_glGetClipPlane;
  if (!strcmp(name, "glDrawBuffer")) return emscripten_glDrawBuffer;
  if (!strcmp(name, "glReadBuffer")) return emscripten_glReadBuffer;
  if (!strcmp(name, "glEnable")) return emscripten_glEnable;
  if (!strcmp(name, "glDisable")) return emscripten_glDisable;
  if (!strcmp(name, "glIsEnabled")) return emscripten_glIsEnabled;
  if (!strcmp(name, "glEnableClientState")) return emscripten_glEnableClientState;
  if (!strcmp(name, "glDisableClientState")) return emscripten_glDisableClientState;
  if (!strcmp(name, "glGetBooleanv")) return emscripten_glGetBooleanv;
  if (!strcmp(name, "glGetDoublev")) return emscripten_glGetDoublev;
  if (!strcmp(name, "glGetFloatv")) return emscripten_glGetFloatv;
  if (!strcmp(name, "glGetIntegerv")) return emscripten_glGetIntegerv;
  if (!strcmp(name, "glPushAttrib")) return emscripten_glPushAttrib;
  if (!strcmp(name, "glPopAttrib")) return emscripten_glPopAttrib;
  if (!strcmp(name, "glPushClientAttrib")) return emscripten_glPushClientAttrib;
  if (!strcmp(name, "glPopClientAttrib")) return emscripten_glPopClientAttrib;
  if (!strcmp(name, "glRenderMode")) return emscripten_glRenderMode;
  if (!strcmp(name, "glGetError")) return emscripten_glGetError;
  if (!strcmp(name, "glGetString")) return emscripten_glGetString;
  if (!strcmp(name, "glGetStringi")) return emscripten_glGetStringi;
  if (!strcmp(name, "glFinish")) return emscripten_glFinish;
  if (!strcmp(name, "glFlush")) return emscripten_glFlush;
  if (!strcmp(name, "glHint")) return emscripten_glHint;
  if (!strcmp(name, "glClearDepth")) return emscripten_glClearDepth;
  if (!strcmp(name, "glDepthFunc")) return emscripten_glDepthFunc;
  if (!strcmp(name, "glDepthMask")) return emscripten_glDepthMask;
  if (!strcmp(name, "glDepthRange")) return emscripten_glDepthRange;
  if (!strcmp(name, "glClearAccum")) return emscripten_glClearAccum;
  if (!strcmp(name, "glAccum")) return emscripten_glAccum;
  if (!strcmp(name, "glMatrixMode")) return emscripten_glMatrixMode;
  if (!strcmp(name, "glOrtho")) return emscripten_glOrtho;
  if (!strcmp(name, "glFrustum")) return emscripten_glFrustum;
  if (!strcmp(name, "glViewport")) return emscripten_glViewport;
  if (!strcmp(name, "glPushMatrix")) return emscripten_glPushMatrix;
  if (!strcmp(name, "glPopMatrix")) return emscripten_glPopMatrix;
  if (!strcmp(name, "glLoadIdentity")) return emscripten_glLoadIdentity;
  if (!strcmp(name, "glLoadMatrixd")) return emscripten_glLoadMatrixd;
  if (!strcmp(name, "glLoadMatrixf")) return emscripten_glLoadMatrixf;
  if (!strcmp(name, "glMultMatrixd")) return emscripten_glMultMatrixd;
  if (!strcmp(name, "glMultMatrixf")) return emscripten_glMultMatrixf;
  if (!strcmp(name, "glRotated")) return emscripten_glRotated;
  if (!strcmp(name, "glRotatef")) return emscripten_glRotatef;
  if (!strcmp(name, "glScaled")) return emscripten_glScaled;
  if (!strcmp(name, "glScalef")) return emscripten_glScalef;
  if (!strcmp(name, "glTranslated")) return emscripten_glTranslated;
  if (!strcmp(name, "glTranslatef")) return emscripten_glTranslatef;
  if (!strcmp(name, "glIsList")) return emscripten_glIsList;
  if (!strcmp(name, "glDeleteLists")) return emscripten_glDeleteLists;
  if (!strcmp(name, "glGenLists")) return emscripten_glGenLists;
  if (!strcmp(name, "glNewList")) return emscripten_glNewList;
  if (!strcmp(name, "glEndList")) return emscripten_glEndList;
  if (!strcmp(name, "glCallList")) return emscripten_glCallList;
  if (!strcmp(name, "glCallLists")) return emscripten_glCallLists;
  if (!strcmp(name, "glListBase")) return emscripten_glListBase;
  if (!strcmp(name, "glBegin")) return emscripten_glBegin;
  if (!strcmp(name, "glEnd")) return emscripten_glEnd;
  if (!strcmp(name, "glVertex2d")) return emscripten_glVertex2d;
  if (!strcmp(name, "glVertex2f")) return emscripten_glVertex2f;
  if (!strcmp(name, "glVertex2i")) return emscripten_glVertex2i;
  if (!strcmp(name, "glVertex2s")) return emscripten_glVertex2s;
  if (!strcmp(name, "glVertex3d")) return emscripten_glVertex3d;
  if (!strcmp(name, "glVertex3f")) return emscripten_glVertex3f;
  if (!strcmp(name, "glVertex3i")) return emscripten_glVertex3i;
  if (!strcmp(name, "glVertex3s")) return emscripten_glVertex3s;
  if (!strcmp(name, "glVertex4d")) return emscripten_glVertex4d;
  if (!strcmp(name, "glVertex4f")) return emscripten_glVertex4f;
  if (!strcmp(name, "glVertex4i")) return emscripten_glVertex4i;
  if (!strcmp(name, "glVertex4s")) return emscripten_glVertex4s;
  if (!strcmp(name, "glVertex2dv")) return emscripten_glVertex2dv;
  if (!strcmp(name, "glVertex2fv")) return emscripten_glVertex2fv;
  if (!strcmp(name, "glVertex2iv")) return emscripten_glVertex2iv;
  if (!strcmp(name, "glVertex2sv")) return emscripten_glVertex2sv;
  if (!strcmp(name, "glVertex3dv")) return emscripten_glVertex3dv;
  if (!strcmp(name, "glVertex3fv")) return emscripten_glVertex3fv;
  if (!strcmp(name, "glVertex3iv")) return emscripten_glVertex3iv;
  if (!strcmp(name, "glVertex3sv")) return emscripten_glVertex3sv;
  if (!strcmp(name, "glVertex4dv")) return emscripten_glVertex4dv;
  if (!strcmp(name, "glVertex4fv")) return emscripten_glVertex4fv;
  if (!strcmp(name, "glVertex4iv")) return emscripten_glVertex4iv;
  if (!strcmp(name, "glVertex4sv")) return emscripten_glVertex4sv;
  if (!strcmp(name, "glNormal3b")) return emscripten_glNormal3b;
  if (!strcmp(name, "glNormal3d")) return emscripten_glNormal3d;
  if (!strcmp(name, "glNormal3f")) return emscripten_glNormal3f;
  if (!strcmp(name, "glNormal3i")) return emscripten_glNormal3i;
  if (!strcmp(name, "glNormal3s")) return emscripten_glNormal3s;
  if (!strcmp(name, "glNormal3bv")) return emscripten_glNormal3bv;
  if (!strcmp(name, "glNormal3dv")) return emscripten_glNormal3dv;
  if (!strcmp(name, "glNormal3fv")) return emscripten_glNormal3fv;
  if (!strcmp(name, "glNormal3iv")) return emscripten_glNormal3iv;
  if (!strcmp(name, "glNormal3sv")) return emscripten_glNormal3sv;
  if (!strcmp(name, "glIndexd")) return emscripten_glIndexd;
  if (!strcmp(name, "glIndexf")) return emscripten_glIndexf;
  if (!strcmp(name, "glIndexi")) return emscripten_glIndexi;
  if (!strcmp(name, "glIndexs")) return emscripten_glIndexs;
  if (!strcmp(name, "glIndexub")) return emscripten_glIndexub;
  if (!strcmp(name, "glIndexdv")) return emscripten_glIndexdv;
  if (!strcmp(name, "glIndexfv")) return emscripten_glIndexfv;
  if (!strcmp(name, "glIndexiv")) return emscripten_glIndexiv;
  if (!strcmp(name, "glIndexsv")) return emscripten_glIndexsv;
  if (!strcmp(name, "glIndexubv")) return emscripten_glIndexubv;
  if (!strcmp(name, "glColor3b")) return emscripten_glColor3b;
  if (!strcmp(name, "glColor3d")) return emscripten_glColor3d;
  if (!strcmp(name, "glColor3f")) return emscripten_glColor3f;
  if (!strcmp(name, "glColor3i")) return emscripten_glColor3i;
  if (!strcmp(name, "glColor3s")) return emscripten_glColor3s;
  if (!strcmp(name, "glColor3ub")) return emscripten_glColor3ub;
  if (!strcmp(name, "glColor3ui")) return emscripten_glColor3ui;
  if (!strcmp(name, "glColor3us")) return emscripten_glColor3us;
  if (!strcmp(name, "glColor4b")) return emscripten_glColor4b;
  if (!strcmp(name, "glColor4d")) return emscripten_glColor4d;
  if (!strcmp(name, "glColor4f")) return emscripten_glColor4f;
  if (!strcmp(name, "glColor4i")) return emscripten_glColor4i;
  if (!strcmp(name, "glColor4s")) return emscripten_glColor4s;
  if (!strcmp(name, "glColor4ub")) return emscripten_glColor4ub;
  if (!strcmp(name, "glColor4ui")) return emscripten_glColor4ui;
  if (!strcmp(name, "glColor4us")) return emscripten_glColor4us;
  if (!strcmp(name, "glColor3bv")) return emscripten_glColor3bv;
  if (!strcmp(name, "glColor3dv")) return emscripten_glColor3dv;
  if (!strcmp(name, "glColor3fv")) return emscripten_glColor3fv;
  if (!strcmp(name, "glColor3iv")) return emscripten_glColor3iv;
  if (!strcmp(name, "glColor3sv")) return emscripten_glColor3sv;
  if (!strcmp(name, "glColor3ubv")) return emscripten_glColor3ubv;
  if (!strcmp(name, "glColor3uiv")) return emscripten_glColor3uiv;
  if (!strcmp(name, "glColor3usv")) return emscripten_glColor3usv;
  if (!strcmp(name, "glColor4bv")) return emscripten_glColor4bv;
  if (!strcmp(name, "glColor4dv")) return emscripten_glColor4dv;
  if (!strcmp(name, "glColor4fv")) return emscripten_glColor4fv;
  if (!strcmp(name, "glColor4iv")) return emscripten_glColor4iv;
  if (!strcmp(name, "glColor4sv")) return emscripten_glColor4sv;
  if (!strcmp(name, "glColor4ubv")) return emscripten_glColor4ubv;
  if (!strcmp(name, "glColor4uiv")) return emscripten_glColor4uiv;
  if (!strcmp(name, "glColor4usv")) return emscripten_glColor4usv;
  if (!strcmp(name, "glTexCoord1d")) return emscripten_glTexCoord1d;
  if (!strcmp(name, "glTexCoord1f")) return emscripten_glTexCoord1f;
  if (!strcmp(name, "glTexCoord1i")) return emscripten_glTexCoord1i;
  if (!strcmp(name, "glTexCoord1s")) return emscripten_glTexCoord1s;
  if (!strcmp(name, "glTexCoord2d")) return emscripten_glTexCoord2d;
  if (!strcmp(name, "glTexCoord2f")) return emscripten_glTexCoord2f;
  if (!strcmp(name, "glTexCoord2i")) return emscripten_glTexCoord2i;
  if (!strcmp(name, "glTexCoord2s")) return emscripten_glTexCoord2s;
  if (!strcmp(name, "glTexCoord3d")) return emscripten_glTexCoord3d;
  if (!strcmp(name, "glTexCoord3f")) return emscripten_glTexCoord3f;
  if (!strcmp(name, "glTexCoord3i")) return emscripten_glTexCoord3i;
  if (!strcmp(name, "glTexCoord3s")) return emscripten_glTexCoord3s;
  if (!strcmp(name, "glTexCoord4d")) return emscripten_glTexCoord4d;
  if (!strcmp(name, "glTexCoord4f")) return emscripten_glTexCoord4f;
  if (!strcmp(name, "glTexCoord4i")) return emscripten_glTexCoord4i;
  if (!strcmp(name, "glTexCoord4s")) return emscripten_glTexCoord4s;
  if (!strcmp(name, "glTexCoord1dv")) return emscripten_glTexCoord1dv;
  if (!strcmp(name, "glTexCoord1fv")) return emscripten_glTexCoord1fv;
  if (!strcmp(name, "glTexCoord1iv")) return emscripten_glTexCoord1iv;
  if (!strcmp(name, "glTexCoord1sv")) return emscripten_glTexCoord1sv;
  if (!strcmp(name, "glTexCoord2dv")) return emscripten_glTexCoord2dv;
  if (!strcmp(name, "glTexCoord2fv")) return emscripten_glTexCoord2fv;
  if (!strcmp(name, "glTexCoord2iv")) return emscripten_glTexCoord2iv;
  if (!strcmp(name, "glTexCoord2sv")) return emscripten_glTexCoord2sv;
  if (!strcmp(name, "glTexCoord3dv")) return emscripten_glTexCoord3dv;
  if (!strcmp(name, "glTexCoord3fv")) return emscripten_glTexCoord3fv;
  if (!strcmp(name, "glTexCoord3iv")) return emscripten_glTexCoord3iv;
  if (!strcmp(name, "glTexCoord3sv")) return emscripten_glTexCoord3sv;
  if (!strcmp(name, "glTexCoord4dv")) return emscripten_glTexCoord4dv;
  if (!strcmp(name, "glTexCoord4fv")) return emscripten_glTexCoord4fv;
  if (!strcmp(name, "glTexCoord4iv")) return emscripten_glTexCoord4iv;
  if (!strcmp(name, "glTexCoord4sv")) return emscripten_glTexCoord4sv;
  if (!strcmp(name, "glRasterPos2d")) return emscripten_glRasterPos2d;
  if (!strcmp(name, "glRasterPos2f")) return emscripten_glRasterPos2f;
  if (!strcmp(name, "glRasterPos2i")) return emscripten_glRasterPos2i;
  if (!strcmp(name, "glRasterPos2s")) return emscripten_glRasterPos2s;
  if (!strcmp(name, "glRasterPos3d")) return emscripten_glRasterPos3d;
  if (!strcmp(name, "glRasterPos3f")) return emscripten_glRasterPos3f;
  if (!strcmp(name, "glRasterPos3i")) return emscripten_glRasterPos3i;
  if (!strcmp(name, "glRasterPos3s")) return emscripten_glRasterPos3s;
  if (!strcmp(name, "glRasterPos4d")) return emscripten_glRasterPos4d;
  if (!strcmp(name, "glRasterPos4f")) return emscripten_glRasterPos4f;
  if (!strcmp(name, "glRasterPos4i")) return emscripten_glRasterPos4i;
  if (!strcmp(name, "glRasterPos4s")) return emscripten_glRasterPos4s;
  if (!strcmp(name, "glRasterPos2dv")) return emscripten_glRasterPos2dv;
  if (!strcmp(name, "glRasterPos2fv")) return emscripten_glRasterPos2fv;
  if (!strcmp(name, "glRasterPos2iv")) return emscripten_glRasterPos2iv;
  if (!strcmp(name, "glRasterPos2sv")) return emscripten_glRasterPos2sv;
  if (!strcmp(name, "glRasterPos3dv")) return emscripten_glRasterPos3dv;
  if (!strcmp(name, "glRasterPos3fv")) return emscripten_glRasterPos3fv;
  if (!strcmp(name, "glRasterPos3iv")) return emscripten_glRasterPos3iv;
  if (!strcmp(name, "glRasterPos3sv")) return emscripten_glRasterPos3sv;
  if (!strcmp(name, "glRasterPos4dv")) return emscripten_glRasterPos4dv;
  if (!strcmp(name, "glRasterPos4fv")) return emscripten_glRasterPos4fv;
  if (!strcmp(name, "glRasterPos4iv")) return emscripten_glRasterPos4iv;
  if (!strcmp(name, "glRasterPos4sv")) return emscripten_glRasterPos4sv;
  if (!strcmp(name, "glRectd")) return emscripten_glRectd;
  if (!strcmp(name, "glRectf")) return emscripten_glRectf;
  if (!strcmp(name, "glRecti")) return emscripten_glRecti;
  if (!strcmp(name, "glRects")) return emscripten_glRects;
  if (!strcmp(name, "glRectdv")) return emscripten_glRectdv;
  if (!strcmp(name, "glRectfv")) return emscripten_glRectfv;
  if (!strcmp(name, "glRectiv")) return emscripten_glRectiv;
  if (!strcmp(name, "glRectsv")) return emscripten_glRectsv;
  if (!strcmp(name, "glVertexPointer")) return emscripten_glVertexPointer;
  if (!strcmp(name, "glNormalPointer")) return emscripten_glNormalPointer;
  if (!strcmp(name, "glColorPointer")) return emscripten_glColorPointer;
  if (!strcmp(name, "glIndexPointer")) return emscripten_glIndexPointer;
  if (!strcmp(name, "glTexCoordPointer")) return emscripten_glTexCoordPointer;
  if (!strcmp(name, "glEdgeFlagPointer")) return emscripten_glEdgeFlagPointer;
  if (!strcmp(name, "glGetPointerv")) return emscripten_glGetPointerv;
  if (!strcmp(name, "glArrayElement")) return emscripten_glArrayElement;
  if (!strcmp(name, "glDrawArrays")) return emscripten_glDrawArrays;
  if (!strcmp(name, "glDrawElements")) return emscripten_glDrawElements;
  if (!strcmp(name, "glInterleavedArrays")) return emscripten_glInterleavedArrays;
  if (!strcmp(name, "glShadeModel")) return emscripten_glShadeModel;
  if (!strcmp(name, "glLightf")) return emscripten_glLightf;
  if (!strcmp(name, "glLighti")) return emscripten_glLighti;
  if (!strcmp(name, "glLightfv")) return emscripten_glLightfv;
  if (!strcmp(name, "glLightiv")) return emscripten_glLightiv;
  if (!strcmp(name, "glGetLightfv")) return emscripten_glGetLightfv;
  if (!strcmp(name, "glGetLightiv")) return emscripten_glGetLightiv;
  if (!strcmp(name, "glLightModelf")) return emscripten_glLightModelf;
  if (!strcmp(name, "glLightModeli")) return emscripten_glLightModeli;
  if (!strcmp(name, "glLightModelfv")) return emscripten_glLightModelfv;
  if (!strcmp(name, "glLightModeliv")) return emscripten_glLightModeliv;
  if (!strcmp(name, "glMaterialf")) return emscripten_glMaterialf;
  if (!strcmp(name, "glMateriali")) return emscripten_glMateriali;
  if (!strcmp(name, "glMaterialfv")) return emscripten_glMaterialfv;
  if (!strcmp(name, "glMaterialiv")) return emscripten_glMaterialiv;
  if (!strcmp(name, "glGetMaterialfv")) return emscripten_glGetMaterialfv;
  if (!strcmp(name, "glGetMaterialiv")) return emscripten_glGetMaterialiv;
  if (!strcmp(name, "glColorMaterial")) return emscripten_glColorMaterial;
  if (!strcmp(name, "glPixelZoom")) return emscripten_glPixelZoom;
  if (!strcmp(name, "glPixelStoref")) return emscripten_glPixelStoref;
  if (!strcmp(name, "glPixelStorei")) return emscripten_glPixelStorei;
  if (!strcmp(name, "glPixelTransferf")) return emscripten_glPixelTransferf;
  if (!strcmp(name, "glPixelTransferi")) return emscripten_glPixelTransferi;
  if (!strcmp(name, "glPixelMapfv")) return emscripten_glPixelMapfv;
  if (!strcmp(name, "glPixelMapuiv")) return emscripten_glPixelMapuiv;
  if (!strcmp(name, "glPixelMapusv")) return emscripten_glPixelMapusv;
  if (!strcmp(name, "glGetPixelMapfv")) return emscripten_glGetPixelMapfv;
  if (!strcmp(name, "glGetPixelMapuiv")) return emscripten_glGetPixelMapuiv;
  if (!strcmp(name, "glGetPixelMapusv")) return emscripten_glGetPixelMapusv;
  if (!strcmp(name, "glBitmap")) return emscripten_glBitmap;
  if (!strcmp(name, "glReadPixels")) return emscripten_glReadPixels;
  if (!strcmp(name, "glDrawPixels")) return emscripten_glDrawPixels;
  if (!strcmp(name, "glCopyPixels")) return emscripten_glCopyPixels;
  if (!strcmp(name, "glStencilFunc")) return emscripten_glStencilFunc;
  if (!strcmp(name, "glStencilMask")) return emscripten_glStencilMask;
  if (!strcmp(name, "glStencilOp")) return emscripten_glStencilOp;
  if (!strcmp(name, "glClearStencil")) return emscripten_glClearStencil;
  if (!strcmp(name, "glTexGend")) return emscripten_glTexGend;
  if (!strcmp(name, "glTexGenf")) return emscripten_glTexGenf;
  if (!strcmp(name, "glTexGeni")) return emscripten_glTexGeni;
  if (!strcmp(name, "glTexGendv")) return emscripten_glTexGendv;
  if (!strcmp(name, "glTexGenfv")) return emscripten_glTexGenfv;
  if (!strcmp(name, "glTexGeniv")) return emscripten_glTexGeniv;
  if (!strcmp(name, "glGetTexGendv")) return emscripten_glGetTexGendv;
  if (!strcmp(name, "glGetTexGenfv")) return emscripten_glGetTexGenfv;
  if (!strcmp(name, "glGetTexGeniv")) return emscripten_glGetTexGeniv;
  if (!strcmp(name, "glTexEnvf")) return emscripten_glTexEnvf;
  if (!strcmp(name, "glTexEnvi")) return emscripten_glTexEnvi;
  if (!strcmp(name, "glTexEnvfv")) return emscripten_glTexEnvfv;
  if (!strcmp(name, "glTexEnviv")) return emscripten_glTexEnviv;
  if (!strcmp(name, "glGetTexEnvfv")) return emscripten_glGetTexEnvfv;
  if (!strcmp(name, "glGetTexEnviv")) return emscripten_glGetTexEnviv;
  if (!strcmp(name, "glTexParameterf")) return emscripten_glTexParameterf;
  if (!strcmp(name, "glTexParameteri")) return emscripten_glTexParameteri;
  if (!strcmp(name, "glTexParameterfv")) return emscripten_glTexParameterfv;
  if (!strcmp(name, "glTexParameteriv")) return emscripten_glTexParameteriv;
  if (!strcmp(name, "glGetTexParameterfv")) return emscripten_glGetTexParameterfv;
  if (!strcmp(name, "glGetTexParameteriv")) return emscripten_glGetTexParameteriv;
  if (!strcmp(name, "glGetTexLevelParameterfv")) return emscripten_glGetTexLevelParameterfv;
  if (!strcmp(name, "glGetTexLevelParameteriv")) return emscripten_glGetTexLevelParameteriv;
  if (!strcmp(name, "glTexImage1D")) return emscripten_glTexImage1D;
  if (!strcmp(name, "glTexImage2D")) return emscripten_glTexImage2D;
  if (!strcmp(name, "glGetTexImage")) return emscripten_glGetTexImage;
  if (!strcmp(name, "glTexStorage2D")) return emscripten_glTexStorage2D;
  if (!strcmp(name, "glTexStorage3D")) return emscripten_glTexStorage3D;
  if (!strcmp(name, "glTexStorage2D")) return emscripten_glTexStorage2D;
  if (!strcmp(name, "glTexStorage3D")) return emscripten_glTexStorage3D;
  if (!strcmp(name, "glGenTextures")) return emscripten_glGenTextures;
  if (!strcmp(name, "glDeleteTextures")) return emscripten_glDeleteTextures;
  if (!strcmp(name, "glBindTexture")) return emscripten_glBindTexture;
  if (!strcmp(name, "glPrioritizeTextures")) return emscripten_glPrioritizeTextures;
  if (!strcmp(name, "glAreTexturesResident")) return emscripten_glAreTexturesResident;
  if (!strcmp(name, "glIsTexture")) return emscripten_glIsTexture;
  if (!strcmp(name, "glTexSubImage1D")) return emscripten_glTexSubImage1D;
  if (!strcmp(name, "glTexSubImage2D")) return emscripten_glTexSubImage2D;
  if (!strcmp(name, "glCopyTexImage1D")) return emscripten_glCopyTexImage1D;
  if (!strcmp(name, "glCopyTexImage2D")) return emscripten_glCopyTexImage2D;
  if (!strcmp(name, "glCopyTexSubImage1D")) return emscripten_glCopyTexSubImage1D;
  if (!strcmp(name, "glCopyTexSubImage2D")) return emscripten_glCopyTexSubImage2D;
  if (!strcmp(name, "glMap1d")) return emscripten_glMap1d;
  if (!strcmp(name, "glMap1f")) return emscripten_glMap1f;
  if (!strcmp(name, "glMap2d")) return emscripten_glMap2d;
  if (!strcmp(name, "glMap2f")) return emscripten_glMap2f;
  if (!strcmp(name, "glGetMapdv")) return emscripten_glGetMapdv;
  if (!strcmp(name, "glGetMapfv")) return emscripten_glGetMapfv;
  if (!strcmp(name, "glGetMapiv")) return emscripten_glGetMapiv;
  if (!strcmp(name, "glEvalCoord1d")) return emscripten_glEvalCoord1d;
  if (!strcmp(name, "glEvalCoord1f")) return emscripten_glEvalCoord1f;
  if (!strcmp(name, "glEvalCoord1dv")) return emscripten_glEvalCoord1dv;
  if (!strcmp(name, "glEvalCoord1fv")) return emscripten_glEvalCoord1fv;
  if (!strcmp(name, "glEvalCoord2d")) return emscripten_glEvalCoord2d;
  if (!strcmp(name, "glEvalCoord2f")) return emscripten_glEvalCoord2f;
  if (!strcmp(name, "glEvalCoord2dv")) return emscripten_glEvalCoord2dv;
  if (!strcmp(name, "glEvalCoord2fv")) return emscripten_glEvalCoord2fv;
  if (!strcmp(name, "glMapGrid1d")) return emscripten_glMapGrid1d;
  if (!strcmp(name, "glMapGrid1f")) return emscripten_glMapGrid1f;
  if (!strcmp(name, "glMapGrid2d")) return emscripten_glMapGrid2d;
  if (!strcmp(name, "glMapGrid2f")) return emscripten_glMapGrid2f;
  if (!strcmp(name, "glEvalPoint1")) return emscripten_glEvalPoint1;
  if (!strcmp(name, "glEvalPoint2")) return emscripten_glEvalPoint2;
  if (!strcmp(name, "glEvalMesh1")) return emscripten_glEvalMesh1;
  if (!strcmp(name, "glEvalMesh2")) return emscripten_glEvalMesh2;
  if (!strcmp(name, "glFogf")) return emscripten_glFogf;
  if (!strcmp(name, "glFogi")) return emscripten_glFogi;
  if (!strcmp(name, "glFogfv")) return emscripten_glFogfv;
  if (!strcmp(name, "glFogiv")) return emscripten_glFogiv;
  if (!strcmp(name, "glFeedbackBuffer")) return emscripten_glFeedbackBuffer;
  if (!strcmp(name, "glPassThrough")) return emscripten_glPassThrough;
  if (!strcmp(name, "glSelectBuffer")) return emscripten_glSelectBuffer;
  if (!strcmp(name, "glInitNames")) return emscripten_glInitNames;
  if (!strcmp(name, "glLoadName")) return emscripten_glLoadName;
  if (!strcmp(name, "glPushName")) return emscripten_glPushName;
  if (!strcmp(name, "glPopName")) return emscripten_glPopName;
  if (!strcmp(name, "glDrawRangeElements")) return emscripten_glDrawRangeElements;
  if (!strcmp(name, "glTexImage3D")) return emscripten_glTexImage3D;
  if (!strcmp(name, "glTexSubImage3D")) return emscripten_glTexSubImage3D;
  if (!strcmp(name, "glCopyTexSubImage3D")) return emscripten_glCopyTexSubImage3D;
  if (!strcmp(name, "glColorTable")) return emscripten_glColorTable;
  if (!strcmp(name, "glColorSubTable")) return emscripten_glColorSubTable;
  if (!strcmp(name, "glColorTableParameteriv")) return emscripten_glColorTableParameteriv;
  if (!strcmp(name, "glColorTableParameterfv")) return emscripten_glColorTableParameterfv;
  if (!strcmp(name, "glCopyColorSubTable")) return emscripten_glCopyColorSubTable;
  if (!strcmp(name, "glCopyColorTable")) return emscripten_glCopyColorTable;
  if (!strcmp(name, "glGetColorTable")) return emscripten_glGetColorTable;
  if (!strcmp(name, "glGetColorTableParameterfv")) return emscripten_glGetColorTableParameterfv;
  if (!strcmp(name, "glGetColorTableParameteriv")) return emscripten_glGetColorTableParameteriv;
  if (!strcmp(name, "glBlendEquation")) return emscripten_glBlendEquation;
  if (!strcmp(name, "glBlendColor")) return emscripten_glBlendColor;
  if (!strcmp(name, "glHistogram")) return emscripten_glHistogram;
  if (!strcmp(name, "glResetHistogram")) return emscripten_glResetHistogram;
  if (!strcmp(name, "glGetHistogram")) return emscripten_glGetHistogram;
  if (!strcmp(name, "glGetHistogramParameterfv")) return emscripten_glGetHistogramParameterfv;
  if (!strcmp(name, "glGetHistogramParameteriv")) return emscripten_glGetHistogramParameteriv;
  if (!strcmp(name, "glMinmax")) return emscripten_glMinmax;
  if (!strcmp(name, "glResetMinmax")) return emscripten_glResetMinmax;
  if (!strcmp(name, "glGetMinmax")) return emscripten_glGetMinmax;
  if (!strcmp(name, "glGetMinmaxParameterfv")) return emscripten_glGetMinmaxParameterfv;
  if (!strcmp(name, "glGetMinmaxParameteriv")) return emscripten_glGetMinmaxParameteriv;
  if (!strcmp(name, "glConvolutionFilter1D")) return emscripten_glConvolutionFilter1D;
  if (!strcmp(name, "glConvolutionFilter2D")) return emscripten_glConvolutionFilter2D;
  if (!strcmp(name, "glConvolutionParameterf")) return emscripten_glConvolutionParameterf;
  if (!strcmp(name, "glConvolutionParameterfv")) return emscripten_glConvolutionParameterfv;
  if (!strcmp(name, "glConvolutionParameteri")) return emscripten_glConvolutionParameteri;
  if (!strcmp(name, "glConvolutionParameteriv")) return emscripten_glConvolutionParameteriv;
  if (!strcmp(name, "glCopyConvolutionFilter1D")) return emscripten_glCopyConvolutionFilter1D;
  if (!strcmp(name, "glCopyConvolutionFilter2D")) return emscripten_glCopyConvolutionFilter2D;
  if (!strcmp(name, "glGetConvolutionFilter")) return emscripten_glGetConvolutionFilter;
  if (!strcmp(name, "glGetConvolutionParameterfv")) return emscripten_glGetConvolutionParameterfv;
  if (!strcmp(name, "glGetConvolutionParameteriv")) return emscripten_glGetConvolutionParameteriv;
  if (!strcmp(name, "glSeparableFilter2D")) return emscripten_glSeparableFilter2D;
  if (!strcmp(name, "glGetSeparableFilter")) return emscripten_glGetSeparableFilter;
  if (!strcmp(name, "glActiveTexture")) return emscripten_glActiveTexture;
  if (!strcmp(name, "glClientActiveTexture")) return emscripten_glClientActiveTexture;
  if (!strcmp(name, "glCompressedTexImage1D")) return emscripten_glCompressedTexImage1D;
  if (!strcmp(name, "glCompressedTexImage2D")) return emscripten_glCompressedTexImage2D;
  if (!strcmp(name, "glCompressedTexImage3D")) return emscripten_glCompressedTexImage3D;
  if (!strcmp(name, "glCompressedTexSubImage1D")) return emscripten_glCompressedTexSubImage1D;
  if (!strcmp(name, "glCompressedTexSubImage2D")) return emscripten_glCompressedTexSubImage2D;
  if (!strcmp(name, "glCompressedTexSubImage3D")) return emscripten_glCompressedTexSubImage3D;
  if (!strcmp(name, "glGetCompressedTexImage")) return emscripten_glGetCompressedTexImage;
  if (!strcmp(name, "glMultiTexCoord1d")) return emscripten_glMultiTexCoord1d;
  if (!strcmp(name, "glMultiTexCoord1dv")) return emscripten_glMultiTexCoord1dv;
  if (!strcmp(name, "glMultiTexCoord1f")) return emscripten_glMultiTexCoord1f;
  if (!strcmp(name, "glMultiTexCoord1fv")) return emscripten_glMultiTexCoord1fv;
  if (!strcmp(name, "glMultiTexCoord1i")) return emscripten_glMultiTexCoord1i;
  if (!strcmp(name, "glMultiTexCoord1iv")) return emscripten_glMultiTexCoord1iv;
  if (!strcmp(name, "glMultiTexCoord1s")) return emscripten_glMultiTexCoord1s;
  if (!strcmp(name, "glMultiTexCoord1sv")) return emscripten_glMultiTexCoord1sv;
  if (!strcmp(name, "glMultiTexCoord2d")) return emscripten_glMultiTexCoord2d;
  if (!strcmp(name, "glMultiTexCoord2dv")) return emscripten_glMultiTexCoord2dv;
  if (!strcmp(name, "glMultiTexCoord2f")) return emscripten_glMultiTexCoord2f;
  if (!strcmp(name, "glMultiTexCoord2fv")) return emscripten_glMultiTexCoord2fv;
  if (!strcmp(name, "glMultiTexCoord2i")) return emscripten_glMultiTexCoord2i;
  if (!strcmp(name, "glMultiTexCoord2iv")) return emscripten_glMultiTexCoord2iv;
  if (!strcmp(name, "glMultiTexCoord2s")) return emscripten_glMultiTexCoord2s;
  if (!strcmp(name, "glMultiTexCoord2sv")) return emscripten_glMultiTexCoord2sv;
  if (!strcmp(name, "glMultiTexCoord3d")) return emscripten_glMultiTexCoord3d;
  if (!strcmp(name, "glMultiTexCoord3dv")) return emscripten_glMultiTexCoord3dv;
  if (!strcmp(name, "glMultiTexCoord3f")) return emscripten_glMultiTexCoord3f;
  if (!strcmp(name, "glMultiTexCoord3fv")) return emscripten_glMultiTexCoord3fv;
  if (!strcmp(name, "glMultiTexCoord3i")) return emscripten_glMultiTexCoord3i;
  if (!strcmp(name, "glMultiTexCoord3iv")) return emscripten_glMultiTexCoord3iv;
  if (!strcmp(name, "glMultiTexCoord3s")) return emscripten_glMultiTexCoord3s;
  if (!strcmp(name, "glMultiTexCoord3sv")) return emscripten_glMultiTexCoord3sv;
  if (!strcmp(name, "glMultiTexCoord4d")) return emscripten_glMultiTexCoord4d;
  if (!strcmp(name, "glMultiTexCoord4dv")) return emscripten_glMultiTexCoord4dv;
  if (!strcmp(name, "glMultiTexCoord4f")) return emscripten_glMultiTexCoord4f;
  if (!strcmp(name, "glMultiTexCoord4fv")) return emscripten_glMultiTexCoord4fv;
  if (!strcmp(name, "glMultiTexCoord4i")) return emscripten_glMultiTexCoord4i;
  if (!strcmp(name, "glMultiTexCoord4iv")) return emscripten_glMultiTexCoord4iv;
  if (!strcmp(name, "glMultiTexCoord4s")) return emscripten_glMultiTexCoord4s;
  if (!strcmp(name, "glMultiTexCoord4sv")) return emscripten_glMultiTexCoord4sv;
  if (!strcmp(name, "glLoadTransposeMatrixd")) return emscripten_glLoadTransposeMatrixd;
  if (!strcmp(name, "glLoadTransposeMatrixf")) return emscripten_glLoadTransposeMatrixf;
  if (!strcmp(name, "glMultTransposeMatrixd")) return emscripten_glMultTransposeMatrixd;
  if (!strcmp(name, "glMultTransposeMatrixf")) return emscripten_glMultTransposeMatrixf;
  if (!strcmp(name, "glSampleCoverage")) return emscripten_glSampleCoverage;
  if (!strcmp(name, "glActiveTexture")) return emscripten_glActiveTextureARB;
  if (!strcmp(name, "glClientActiveTexture")) return emscripten_glClientActiveTextureARB;
  if (!strcmp(name, "glMultiTexCoord1d")) return emscripten_glMultiTexCoord1dARB;
  if (!strcmp(name, "glMultiTexCoord1dv")) return emscripten_glMultiTexCoord1dvARB;
  if (!strcmp(name, "glMultiTexCoord1f")) return emscripten_glMultiTexCoord1fARB;
  if (!strcmp(name, "glMultiTexCoord1fv")) return emscripten_glMultiTexCoord1fvARB;
  if (!strcmp(name, "glMultiTexCoord1i")) return emscripten_glMultiTexCoord1iARB;
  if (!strcmp(name, "glMultiTexCoord1iv")) return emscripten_glMultiTexCoord1ivARB;
  if (!strcmp(name, "glMultiTexCoord1s")) return emscripten_glMultiTexCoord1sARB;
  if (!strcmp(name, "glMultiTexCoord1sv")) return emscripten_glMultiTexCoord1svARB;
  if (!strcmp(name, "glMultiTexCoord2d")) return emscripten_glMultiTexCoord2dARB;
  if (!strcmp(name, "glMultiTexCoord2dv")) return emscripten_glMultiTexCoord2dvARB;
  if (!strcmp(name, "glMultiTexCoord2f")) return emscripten_glMultiTexCoord2fARB;
  if (!strcmp(name, "glMultiTexCoord2fv")) return emscripten_glMultiTexCoord2fvARB;
  if (!strcmp(name, "glMultiTexCoord2i")) return emscripten_glMultiTexCoord2iARB;
  if (!strcmp(name, "glMultiTexCoord2iv")) return emscripten_glMultiTexCoord2ivARB;
  if (!strcmp(name, "glMultiTexCoord2s")) return emscripten_glMultiTexCoord2sARB;
  if (!strcmp(name, "glMultiTexCoord2sv")) return emscripten_glMultiTexCoord2svARB;
  if (!strcmp(name, "glMultiTexCoord3d")) return emscripten_glMultiTexCoord3dARB;
  if (!strcmp(name, "glMultiTexCoord3dv")) return emscripten_glMultiTexCoord3dvARB;
  if (!strcmp(name, "glMultiTexCoord3f")) return emscripten_glMultiTexCoord3fARB;
  if (!strcmp(name, "glMultiTexCoord3fv")) return emscripten_glMultiTexCoord3fvARB;
  if (!strcmp(name, "glMultiTexCoord3i")) return emscripten_glMultiTexCoord3iARB;
  if (!strcmp(name, "glMultiTexCoord3iv")) return emscripten_glMultiTexCoord3ivARB;
  if (!strcmp(name, "glMultiTexCoord3s")) return emscripten_glMultiTexCoord3sARB;
  if (!strcmp(name, "glMultiTexCoord3sv")) return emscripten_glMultiTexCoord3svARB;
  if (!strcmp(name, "glMultiTexCoord4d")) return emscripten_glMultiTexCoord4dARB;
  if (!strcmp(name, "glMultiTexCoord4dv")) return emscripten_glMultiTexCoord4dvARB;
  if (!strcmp(name, "glMultiTexCoord4f")) return emscripten_glMultiTexCoord4fARB;
  if (!strcmp(name, "glMultiTexCoord4fv")) return emscripten_glMultiTexCoord4fvARB;
  if (!strcmp(name, "glMultiTexCoord4i")) return emscripten_glMultiTexCoord4iARB;
  if (!strcmp(name, "glMultiTexCoord4iv")) return emscripten_glMultiTexCoord4ivARB;
  if (!strcmp(name, "glMultiTexCoord4s")) return emscripten_glMultiTexCoord4sARB;
  if (!strcmp(name, "glMultiTexCoord4sv")) return emscripten_glMultiTexCoord4svARB;
  if (!strcmp(name, "glBlendColor")) return emscripten_glBlendColor;
  if (!strcmp(name, "glBlendEquation")) return emscripten_glBlendEquation;
  if (!strcmp(name, "glDrawRangeElements")) return emscripten_glDrawRangeElements;
  if (!strcmp(name, "glTexImage3D")) return emscripten_glTexImage3D;
  if (!strcmp(name, "glCopyTexSubImage3D")) return emscripten_glCopyTexSubImage3D;
  if (!strcmp(name, "glColorTable")) return emscripten_glColorTable;
  if (!strcmp(name, "glColorTableParameterfv")) return emscripten_glColorTableParameterfv;
  if (!strcmp(name, "glColorTableParameteriv")) return emscripten_glColorTableParameteriv;
  if (!strcmp(name, "glCopyColorTable")) return emscripten_glCopyColorTable;
  if (!strcmp(name, "glGetColorTable")) return emscripten_glGetColorTable;
  if (!strcmp(name, "glGetColorTableParameterfv")) return emscripten_glGetColorTableParameterfv;
  if (!strcmp(name, "glGetColorTableParameteriv")) return emscripten_glGetColorTableParameteriv;
  if (!strcmp(name, "glColorSubTable")) return emscripten_glColorSubTable;
  if (!strcmp(name, "glCopyColorSubTable")) return emscripten_glCopyColorSubTable;
  if (!strcmp(name, "glConvolutionFilter1D")) return emscripten_glConvolutionFilter1D;
  if (!strcmp(name, "glConvolutionFilter2D")) return emscripten_glConvolutionFilter2D;
  if (!strcmp(name, "glConvolutionParameterf")) return emscripten_glConvolutionParameterf;
  if (!strcmp(name, "glConvolutionParameterfv")) return emscripten_glConvolutionParameterfv;
  if (!strcmp(name, "glConvolutionParameteri")) return emscripten_glConvolutionParameteri;
  if (!strcmp(name, "glConvolutionParameteriv")) return emscripten_glConvolutionParameteriv;
  if (!strcmp(name, "glCopyConvolutionFilter1D")) return emscripten_glCopyConvolutionFilter1D;
  if (!strcmp(name, "glCopyConvolutionFilter2D")) return emscripten_glCopyConvolutionFilter2D;
  if (!strcmp(name, "glGetConvolutionFilter")) return emscripten_glGetConvolutionFilter;
  if (!strcmp(name, "glGetConvolutionParameterfv")) return emscripten_glGetConvolutionParameterfv;
  if (!strcmp(name, "glGetConvolutionParameteriv")) return emscripten_glGetConvolutionParameteriv;
  if (!strcmp(name, "glGetSeparableFilter")) return emscripten_glGetSeparableFilter;
  if (!strcmp(name, "glSeparableFilter2D")) return emscripten_glSeparableFilter2D;
  if (!strcmp(name, "glGetHistogram")) return emscripten_glGetHistogram;
  if (!strcmp(name, "glGetHistogramParameterfv")) return emscripten_glGetHistogramParameterfv;
  if (!strcmp(name, "glGetHistogramParameteriv")) return emscripten_glGetHistogramParameteriv;
  if (!strcmp(name, "glGetMinmax")) return emscripten_glGetMinmax;
  if (!strcmp(name, "glGetMinmaxParameterfv")) return emscripten_glGetMinmaxParameterfv;
  if (!strcmp(name, "glGetMinmaxParameteriv")) return emscripten_glGetMinmaxParameteriv;
  if (!strcmp(name, "glHistogram")) return emscripten_glHistogram;
  if (!strcmp(name, "glMinmax")) return emscripten_glMinmax;
  if (!strcmp(name, "glResetHistogram")) return emscripten_glResetHistogram;
  if (!strcmp(name, "glResetMinmax")) return emscripten_glResetMinmax;
  if (!strcmp(name, "glActiveTexture")) return emscripten_glActiveTexture;
  if (!strcmp(name, "glSampleCoverage")) return emscripten_glSampleCoverage;
  if (!strcmp(name, "glCompressedTexImage3D")) return emscripten_glCompressedTexImage3D;
  if (!strcmp(name, "glCompressedTexImage2D")) return emscripten_glCompressedTexImage2D;
  if (!strcmp(name, "glCompressedTexImage1D")) return emscripten_glCompressedTexImage1D;
  if (!strcmp(name, "glCompressedTexSubImage3D")) return emscripten_glCompressedTexSubImage3D;
  if (!strcmp(name, "glCompressedTexSubImage2D")) return emscripten_glCompressedTexSubImage2D;
  if (!strcmp(name, "glCompressedTexSubImage1D")) return emscripten_glCompressedTexSubImage1D;
  if (!strcmp(name, "glGetCompressedTexImage")) return emscripten_glGetCompressedTexImage;
  if (!strcmp(name, "glClientActiveTexture")) return emscripten_glClientActiveTexture;
  if (!strcmp(name, "glMultiTexCoord1d")) return emscripten_glMultiTexCoord1d;
  if (!strcmp(name, "glMultiTexCoord1dv")) return emscripten_glMultiTexCoord1dv;
  if (!strcmp(name, "glMultiTexCoord1f")) return emscripten_glMultiTexCoord1f;
  if (!strcmp(name, "glMultiTexCoord1fv")) return emscripten_glMultiTexCoord1fv;
  if (!strcmp(name, "glMultiTexCoord1i")) return emscripten_glMultiTexCoord1i;
  if (!strcmp(name, "glMultiTexCoord1iv")) return emscripten_glMultiTexCoord1iv;
  if (!strcmp(name, "glMultiTexCoord1s")) return emscripten_glMultiTexCoord1s;
  if (!strcmp(name, "glMultiTexCoord1sv")) return emscripten_glMultiTexCoord1sv;
  if (!strcmp(name, "glMultiTexCoord2d")) return emscripten_glMultiTexCoord2d;
  if (!strcmp(name, "glMultiTexCoord2dv")) return emscripten_glMultiTexCoord2dv;
  if (!strcmp(name, "glMultiTexCoord2f")) return emscripten_glMultiTexCoord2f;
  if (!strcmp(name, "glMultiTexCoord2fv")) return emscripten_glMultiTexCoord2fv;
  if (!strcmp(name, "glMultiTexCoord2i")) return emscripten_glMultiTexCoord2i;
  if (!strcmp(name, "glMultiTexCoord2iv")) return emscripten_glMultiTexCoord2iv;
  if (!strcmp(name, "glMultiTexCoord2s")) return emscripten_glMultiTexCoord2s;
  if (!strcmp(name, "glMultiTexCoord2sv")) return emscripten_glMultiTexCoord2sv;
  if (!strcmp(name, "glMultiTexCoord3d")) return emscripten_glMultiTexCoord3d;
  if (!strcmp(name, "glMultiTexCoord3dv")) return emscripten_glMultiTexCoord3dv;
  if (!strcmp(name, "glMultiTexCoord3f")) return emscripten_glMultiTexCoord3f;
  if (!strcmp(name, "glMultiTexCoord3fv")) return emscripten_glMultiTexCoord3fv;
  if (!strcmp(name, "glMultiTexCoord3i")) return emscripten_glMultiTexCoord3i;
  if (!strcmp(name, "glMultiTexCoord3iv")) return emscripten_glMultiTexCoord3iv;
  if (!strcmp(name, "glMultiTexCoord3s")) return emscripten_glMultiTexCoord3s;
  if (!strcmp(name, "glMultiTexCoord3sv")) return emscripten_glMultiTexCoord3sv;
  if (!strcmp(name, "glMultiTexCoord4d")) return emscripten_glMultiTexCoord4d;
  if (!strcmp(name, "glMultiTexCoord4dv")) return emscripten_glMultiTexCoord4dv;
  if (!strcmp(name, "glMultiTexCoord4f")) return emscripten_glMultiTexCoord4f;
  if (!strcmp(name, "glMultiTexCoord4fv")) return emscripten_glMultiTexCoord4fv;
  if (!strcmp(name, "glMultiTexCoord4i")) return emscripten_glMultiTexCoord4i;
  if (!strcmp(name, "glMultiTexCoord4iv")) return emscripten_glMultiTexCoord4iv;
  if (!strcmp(name, "glMultiTexCoord4s")) return emscripten_glMultiTexCoord4s;
  if (!strcmp(name, "glMultiTexCoord4sv")) return emscripten_glMultiTexCoord4sv;
  if (!strcmp(name, "glLoadTransposeMatrixf")) return emscripten_glLoadTransposeMatrixf;
  if (!strcmp(name, "glLoadTransposeMatrixd")) return emscripten_glLoadTransposeMatrixd;
  if (!strcmp(name, "glMultTransposeMatrixf")) return emscripten_glMultTransposeMatrixf;
  if (!strcmp(name, "glMultTransposeMatrixd")) return emscripten_glMultTransposeMatrixd;
  if (!strcmp(name, "glBlendFuncSeparate")) return emscripten_glBlendFuncSeparate;
  if (!strcmp(name, "glMultiDrawArrays")) return emscripten_glMultiDrawArrays;
  if (!strcmp(name, "glMultiDrawElements")) return emscripten_glMultiDrawElements;
  if (!strcmp(name, "glPointParameterf")) return emscripten_glPointParameterf;
  if (!strcmp(name, "glPointParameterfv")) return emscripten_glPointParameterfv;
  if (!strcmp(name, "glPointParameteri")) return emscripten_glPointParameteri;
  if (!strcmp(name, "glPointParameteriv")) return emscripten_glPointParameteriv;
  if (!strcmp(name, "glFogCoordf")) return emscripten_glFogCoordf;
  if (!strcmp(name, "glFogCoordfv")) return emscripten_glFogCoordfv;
  if (!strcmp(name, "glFogCoordd")) return emscripten_glFogCoordd;
  if (!strcmp(name, "glFogCoorddv")) return emscripten_glFogCoorddv;
  if (!strcmp(name, "glFogCoordPointer")) return emscripten_glFogCoordPointer;
  if (!strcmp(name, "glSecondaryColor3b")) return emscripten_glSecondaryColor3b;
  if (!strcmp(name, "glSecondaryColor3bv")) return emscripten_glSecondaryColor3bv;
  if (!strcmp(name, "glSecondaryColor3d")) return emscripten_glSecondaryColor3d;
  if (!strcmp(name, "glSecondaryColor3dv")) return emscripten_glSecondaryColor3dv;
  if (!strcmp(name, "glSecondaryColor3f")) return emscripten_glSecondaryColor3f;
  if (!strcmp(name, "glSecondaryColor3fv")) return emscripten_glSecondaryColor3fv;
  if (!strcmp(name, "glSecondaryColor3i")) return emscripten_glSecondaryColor3i;
  if (!strcmp(name, "glSecondaryColor3iv")) return emscripten_glSecondaryColor3iv;
  if (!strcmp(name, "glSecondaryColor3s")) return emscripten_glSecondaryColor3s;
  if (!strcmp(name, "glSecondaryColor3sv")) return emscripten_glSecondaryColor3sv;
  if (!strcmp(name, "glSecondaryColor3ub")) return emscripten_glSecondaryColor3ub;
  if (!strcmp(name, "glSecondaryColor3ubv")) return emscripten_glSecondaryColor3ubv;
  if (!strcmp(name, "glSecondaryColor3ui")) return emscripten_glSecondaryColor3ui;
  if (!strcmp(name, "glSecondaryColor3uiv")) return emscripten_glSecondaryColor3uiv;
  if (!strcmp(name, "glSecondaryColor3us")) return emscripten_glSecondaryColor3us;
  if (!strcmp(name, "glSecondaryColor3usv")) return emscripten_glSecondaryColor3usv;
  if (!strcmp(name, "glSecondaryColorPointer")) return emscripten_glSecondaryColorPointer;
  if (!strcmp(name, "glWindowPos2d")) return emscripten_glWindowPos2d;
  if (!strcmp(name, "glWindowPos2dv")) return emscripten_glWindowPos2dv;
  if (!strcmp(name, "glWindowPos2f")) return emscripten_glWindowPos2f;
  if (!strcmp(name, "glWindowPos2fv")) return emscripten_glWindowPos2fv;
  if (!strcmp(name, "glWindowPos2i")) return emscripten_glWindowPos2i;
  if (!strcmp(name, "glWindowPos2iv")) return emscripten_glWindowPos2iv;
  if (!strcmp(name, "glWindowPos2s")) return emscripten_glWindowPos2s;
  if (!strcmp(name, "glWindowPos2sv")) return emscripten_glWindowPos2sv;
  if (!strcmp(name, "glWindowPos3d")) return emscripten_glWindowPos3d;
  if (!strcmp(name, "glWindowPos3dv")) return emscripten_glWindowPos3dv;
  if (!strcmp(name, "glWindowPos3f")) return emscripten_glWindowPos3f;
  if (!strcmp(name, "glWindowPos3fv")) return emscripten_glWindowPos3fv;
  if (!strcmp(name, "glWindowPos3i")) return emscripten_glWindowPos3i;
  if (!strcmp(name, "glWindowPos3iv")) return emscripten_glWindowPos3iv;
  if (!strcmp(name, "glWindowPos3s")) return emscripten_glWindowPos3s;
  if (!strcmp(name, "glWindowPos3sv")) return emscripten_glWindowPos3sv;
  if (!strcmp(name, "glGenQueries")) return emscripten_glGenQueries;
  if (!strcmp(name, "glDeleteQueries")) return emscripten_glDeleteQueries;
  if (!strcmp(name, "glIsQuery")) return emscripten_glIsQuery;
  if (!strcmp(name, "glBeginQuery")) return emscripten_glBeginQuery;
  if (!strcmp(name, "glEndQuery")) return emscripten_glEndQuery;
  if (!strcmp(name, "glGetQueryiv")) return emscripten_glGetQueryiv;
  if (!strcmp(name, "glGetQueryObjectiv")) return emscripten_glGetQueryObjectiv;
  if (!strcmp(name, "glGetQueryObjectuiv")) return emscripten_glGetQueryObjectuiv;
  if (!strcmp(name, "glBindBuffer")) return emscripten_glBindBuffer;
  if (!strcmp(name, "glDeleteBuffers")) return emscripten_glDeleteBuffers;
  if (!strcmp(name, "glGenBuffers")) return emscripten_glGenBuffers;
  if (!strcmp(name, "glIsBuffer")) return emscripten_glIsBuffer;
  if (!strcmp(name, "glBufferData")) return emscripten_glBufferData;
  if (!strcmp(name, "glBufferSubData")) return emscripten_glBufferSubData;
  if (!strcmp(name, "glGetBufferSubData")) return emscripten_glGetBufferSubData;
  if (!strcmp(name, "glMapBuffer")) return emscripten_glMapBuffer;
  if (!strcmp(name, "glUnmapBuffer")) return emscripten_glUnmapBuffer;
  if (!strcmp(name, "glGetBufferParameteriv")) return emscripten_glGetBufferParameteriv;
  if (!strcmp(name, "glGetBufferPointerv")) return emscripten_glGetBufferPointerv;
  if (!strcmp(name, "glBlendEquationSeparate")) return emscripten_glBlendEquationSeparate;
  if (!strcmp(name, "glDrawBuffers")) return emscripten_glDrawBuffers;
  if (!strcmp(name, "glStencilOpSeparate")) return emscripten_glStencilOpSeparate;
  if (!strcmp(name, "glStencilFuncSeparate")) return emscripten_glStencilFuncSeparate;
  if (!strcmp(name, "glStencilMaskSeparate")) return emscripten_glStencilMaskSeparate;
  if (!strcmp(name, "glAttachShader")) return emscripten_glAttachShader;
  if (!strcmp(name, "glBindAttribLocation")) return emscripten_glBindAttribLocation;
  if (!strcmp(name, "glCompileShader")) return emscripten_glCompileShader;
  if (!strcmp(name, "glCreateProgram")) return emscripten_glCreateProgram;
  if (!strcmp(name, "glCreateShader")) return emscripten_glCreateShader;
  if (!strcmp(name, "glDeleteProgram")) return emscripten_glDeleteProgram;
  if (!strcmp(name, "glDeleteShader")) return emscripten_glDeleteShader;
  if (!strcmp(name, "glDetachShader")) return emscripten_glDetachShader;
  if (!strcmp(name, "glDisableVertexAttribArray")) return emscripten_glDisableVertexAttribArray;
  if (!strcmp(name, "glEnableVertexAttribArray")) return emscripten_glEnableVertexAttribArray;
  if (!strcmp(name, "glGetActiveAttrib")) return emscripten_glGetActiveAttrib;
  if (!strcmp(name, "glGetActiveUniform")) return emscripten_glGetActiveUniform;
  if (!strcmp(name, "glGetAttachedShaders")) return emscripten_glGetAttachedShaders;
  if (!strcmp(name, "glGetAttribLocation")) return emscripten_glGetAttribLocation;
  if (!strcmp(name, "glGetProgramiv")) return emscripten_glGetProgramiv;
  if (!strcmp(name, "glGetProgramInfoLog")) return emscripten_glGetProgramInfoLog;
  if (!strcmp(name, "glGetShaderiv")) return emscripten_glGetShaderiv;
  if (!strcmp(name, "glGetShaderInfoLog")) return emscripten_glGetShaderInfoLog;
  if (!strcmp(name, "glGetShaderSource")) return emscripten_glGetShaderSource;
  if (!strcmp(name, "glGetUniformLocation")) return emscripten_glGetUniformLocation;
  if (!strcmp(name, "glGetUniformfv")) return emscripten_glGetUniformfv;
  if (!strcmp(name, "glGetUniformiv")) return emscripten_glGetUniformiv;
  if (!strcmp(name, "glGetVertexAttribdv")) return emscripten_glGetVertexAttribdv;
  if (!strcmp(name, "glGetVertexAttribfv")) return emscripten_glGetVertexAttribfv;
  if (!strcmp(name, "glGetVertexAttribiv")) return emscripten_glGetVertexAttribiv;
  if (!strcmp(name, "glGetVertexAttribPointerv")) return emscripten_glGetVertexAttribPointerv;
  if (!strcmp(name, "glIsProgram")) return emscripten_glIsProgram;
  if (!strcmp(name, "glIsShader")) return emscripten_glIsShader;
  if (!strcmp(name, "glLinkProgram")) return emscripten_glLinkProgram;
  if (!strcmp(name, "glShaderSource")) return emscripten_glShaderSource;
  if (!strcmp(name, "glUseProgram")) return emscripten_glUseProgram;
  if (!strcmp(name, "glUniform1f")) return emscripten_glUniform1f;
  if (!strcmp(name, "glUniform2f")) return emscripten_glUniform2f;
  if (!strcmp(name, "glUniform3f")) return emscripten_glUniform3f;
  if (!strcmp(name, "glUniform4f")) return emscripten_glUniform4f;
  if (!strcmp(name, "glUniform1i")) return emscripten_glUniform1i;
  if (!strcmp(name, "glUniform2i")) return emscripten_glUniform2i;
  if (!strcmp(name, "glUniform3i")) return emscripten_glUniform3i;
  if (!strcmp(name, "glUniform4i")) return emscripten_glUniform4i;
  if (!strcmp(name, "glUniform1fv")) return emscripten_glUniform1fv;
  if (!strcmp(name, "glUniform2fv")) return emscripten_glUniform2fv;
  if (!strcmp(name, "glUniform3fv")) return emscripten_glUniform3fv;
  if (!strcmp(name, "glUniform4fv")) return emscripten_glUniform4fv;
  if (!strcmp(name, "glUniform1iv")) return emscripten_glUniform1iv;
  if (!strcmp(name, "glUniform2iv")) return emscripten_glUniform2iv;
  if (!strcmp(name, "glUniform3iv")) return emscripten_glUniform3iv;
  if (!strcmp(name, "glUniform4iv")) return emscripten_glUniform4iv;
  if (!strcmp(name, "glUniformMatrix2fv")) return emscripten_glUniformMatrix2fv;
  if (!strcmp(name, "glUniformMatrix3fv")) return emscripten_glUniformMatrix3fv;
  if (!strcmp(name, "glUniformMatrix4fv")) return emscripten_glUniformMatrix4fv;
  if (!strcmp(name, "glValidateProgram")) return emscripten_glValidateProgram;
  if (!strcmp(name, "glVertexAttrib1d")) return emscripten_glVertexAttrib1d;
  if (!strcmp(name, "glVertexAttrib1dv")) return emscripten_glVertexAttrib1dv;
  if (!strcmp(name, "glVertexAttrib1f")) return emscripten_glVertexAttrib1f;
  if (!strcmp(name, "glVertexAttrib1fv")) return emscripten_glVertexAttrib1fv;
  if (!strcmp(name, "glVertexAttrib1s")) return emscripten_glVertexAttrib1s;
  if (!strcmp(name, "glVertexAttrib1sv")) return emscripten_glVertexAttrib1sv;
  if (!strcmp(name, "glVertexAttrib2d")) return emscripten_glVertexAttrib2d;
  if (!strcmp(name, "glVertexAttrib2dv")) return emscripten_glVertexAttrib2dv;
  if (!strcmp(name, "glVertexAttrib2f")) return emscripten_glVertexAttrib2f;
  if (!strcmp(name, "glVertexAttrib2fv")) return emscripten_glVertexAttrib2fv;
  if (!strcmp(name, "glVertexAttrib2s")) return emscripten_glVertexAttrib2s;
  if (!strcmp(name, "glVertexAttrib2sv")) return emscripten_glVertexAttrib2sv;
  if (!strcmp(name, "glVertexAttrib3d")) return emscripten_glVertexAttrib3d;
  if (!strcmp(name, "glVertexAttrib3dv")) return emscripten_glVertexAttrib3dv;
  if (!strcmp(name, "glVertexAttrib3f")) return emscripten_glVertexAttrib3f;
  if (!strcmp(name, "glVertexAttrib3fv")) return emscripten_glVertexAttrib3fv;
  if (!strcmp(name, "glVertexAttrib3s")) return emscripten_glVertexAttrib3s;
  if (!strcmp(name, "glVertexAttrib3sv")) return emscripten_glVertexAttrib3sv;
  if (!strcmp(name, "glVertexAttrib4Nbv")) return emscripten_glVertexAttrib4Nbv;
  if (!strcmp(name, "glVertexAttrib4Niv")) return emscripten_glVertexAttrib4Niv;
  if (!strcmp(name, "glVertexAttrib4Nsv")) return emscripten_glVertexAttrib4Nsv;
  if (!strcmp(name, "glVertexAttrib4Nub")) return emscripten_glVertexAttrib4Nub;
  if (!strcmp(name, "glVertexAttrib4Nubv")) return emscripten_glVertexAttrib4Nubv;
  if (!strcmp(name, "glVertexAttrib4Nuiv")) return emscripten_glVertexAttrib4Nuiv;
  if (!strcmp(name, "glVertexAttrib4Nusv")) return emscripten_glVertexAttrib4Nusv;
  if (!strcmp(name, "glVertexAttrib4bv")) return emscripten_glVertexAttrib4bv;
  if (!strcmp(name, "glVertexAttrib4d")) return emscripten_glVertexAttrib4d;
  if (!strcmp(name, "glVertexAttrib4dv")) return emscripten_glVertexAttrib4dv;
  if (!strcmp(name, "glVertexAttrib4f")) return emscripten_glVertexAttrib4f;
  if (!strcmp(name, "glVertexAttrib4fv")) return emscripten_glVertexAttrib4fv;
  if (!strcmp(name, "glVertexAttrib4iv")) return emscripten_glVertexAttrib4iv;
  if (!strcmp(name, "glVertexAttrib4s")) return emscripten_glVertexAttrib4s;
  if (!strcmp(name, "glVertexAttrib4sv")) return emscripten_glVertexAttrib4sv;
  if (!strcmp(name, "glVertexAttrib4ubv")) return emscripten_glVertexAttrib4ubv;
  if (!strcmp(name, "glVertexAttrib4uiv")) return emscripten_glVertexAttrib4uiv;
  if (!strcmp(name, "glVertexAttrib4usv")) return emscripten_glVertexAttrib4usv;
  if (!strcmp(name, "glVertexAttribPointer")) return emscripten_glVertexAttribPointer;
  if (!strcmp(name, "glUniformMatrix2x3fv")) return emscripten_glUniformMatrix2x3fv;
  if (!strcmp(name, "glUniformMatrix3x2fv")) return emscripten_glUniformMatrix3x2fv;
  if (!strcmp(name, "glUniformMatrix2x4fv")) return emscripten_glUniformMatrix2x4fv;
  if (!strcmp(name, "glUniformMatrix4x2fv")) return emscripten_glUniformMatrix4x2fv;
  if (!strcmp(name, "glUniformMatrix3x4fv")) return emscripten_glUniformMatrix3x4fv;
  if (!strcmp(name, "glUniformMatrix4x3fv")) return emscripten_glUniformMatrix4x3fv;
  if (!strcmp(name, "glColorMaski")) return emscripten_glColorMaski;
  if (!strcmp(name, "glGetBooleani_v")) return emscripten_glGetBooleani_v;
  if (!strcmp(name, "glGetIntegeri_v")) return emscripten_glGetIntegeri_v;
  if (!strcmp(name, "glEnablei")) return emscripten_glEnablei;
  if (!strcmp(name, "glDisablei")) return emscripten_glDisablei;
  if (!strcmp(name, "glIsEnabledi")) return emscripten_glIsEnabledi;
  if (!strcmp(name, "glBeginTransformFeedback")) return emscripten_glBeginTransformFeedback;
  if (!strcmp(name, "glEndTransformFeedback")) return emscripten_glEndTransformFeedback;
  if (!strcmp(name, "glBindBufferRange")) return emscripten_glBindBufferRange;
  if (!strcmp(name, "glBindBufferBase")) return emscripten_glBindBufferBase;
  if (!strcmp(name, "glTransformFeedbackVaryings")) return emscripten_glTransformFeedbackVaryings;
  if (!strcmp(name, "glGetTransformFeedbackVarying")) return emscripten_glGetTransformFeedbackVarying;
  if (!strcmp(name, "glClampColor")) return emscripten_glClampColor;
  if (!strcmp(name, "glBeginConditionalRender")) return emscripten_glBeginConditionalRender;
  if (!strcmp(name, "glEndConditionalRender")) return emscripten_glEndConditionalRender;
  if (!strcmp(name, "glVertexAttribIPointer")) return emscripten_glVertexAttribIPointer;
  if (!strcmp(name, "glGetVertexAttribIiv")) return emscripten_glGetVertexAttribIiv;
  if (!strcmp(name, "glGetVertexAttribIuiv")) return emscripten_glGetVertexAttribIuiv;
  if (!strcmp(name, "glVertexAttribI1i")) return emscripten_glVertexAttribI1i;
  if (!strcmp(name, "glVertexAttribI2i")) return emscripten_glVertexAttribI2i;
  if (!strcmp(name, "glVertexAttribI3i")) return emscripten_glVertexAttribI3i;
  if (!strcmp(name, "glVertexAttribI4i")) return emscripten_glVertexAttribI4i;
  if (!strcmp(name, "glVertexAttribI1ui")) return emscripten_glVertexAttribI1ui;
  if (!strcmp(name, "glVertexAttribI2ui")) return emscripten_glVertexAttribI2ui;
  if (!strcmp(name, "glVertexAttribI3ui")) return emscripten_glVertexAttribI3ui;
  if (!strcmp(name, "glVertexAttribI4ui")) return emscripten_glVertexAttribI4ui;
  if (!strcmp(name, "glVertexAttribI1iv")) return emscripten_glVertexAttribI1iv;
  if (!strcmp(name, "glVertexAttribI2iv")) return emscripten_glVertexAttribI2iv;
  if (!strcmp(name, "glVertexAttribI3iv")) return emscripten_glVertexAttribI3iv;
  if (!strcmp(name, "glVertexAttribI4iv")) return emscripten_glVertexAttribI4iv;
  if (!strcmp(name, "glVertexAttribI1uiv")) return emscripten_glVertexAttribI1uiv;
  if (!strcmp(name, "glVertexAttribI2uiv")) return emscripten_glVertexAttribI2uiv;
  if (!strcmp(name, "glVertexAttribI3uiv")) return emscripten_glVertexAttribI3uiv;
  if (!strcmp(name, "glVertexAttribI4uiv")) return emscripten_glVertexAttribI4uiv;
  if (!strcmp(name, "glVertexAttribI4bv")) return emscripten_glVertexAttribI4bv;
  if (!strcmp(name, "glVertexAttribI4sv")) return emscripten_glVertexAttribI4sv;
  if (!strcmp(name, "glVertexAttribI4ubv")) return emscripten_glVertexAttribI4ubv;
  if (!strcmp(name, "glVertexAttribI4usv")) return emscripten_glVertexAttribI4usv;
  if (!strcmp(name, "glGetUniformuiv")) return emscripten_glGetUniformuiv;
  if (!strcmp(name, "glBindFragDataLocation")) return emscripten_glBindFragDataLocation;
  if (!strcmp(name, "glGetFragDataLocation")) return emscripten_glGetFragDataLocation;
  if (!strcmp(name, "glUniform1ui")) return emscripten_glUniform1ui;
  if (!strcmp(name, "glUniform2ui")) return emscripten_glUniform2ui;
  if (!strcmp(name, "glUniform3ui")) return emscripten_glUniform3ui;
  if (!strcmp(name, "glUniform4ui")) return emscripten_glUniform4ui;
  if (!strcmp(name, "glUniform1uiv")) return emscripten_glUniform1uiv;
  if (!strcmp(name, "glUniform2uiv")) return emscripten_glUniform2uiv;
  if (!strcmp(name, "glUniform3uiv")) return emscripten_glUniform3uiv;
  if (!strcmp(name, "glUniform4uiv")) return emscripten_glUniform4uiv;
  if (!strcmp(name, "glTexParameterIiv")) return emscripten_glTexParameterIiv;
  if (!strcmp(name, "glTexParameterIuiv")) return emscripten_glTexParameterIuiv;
  if (!strcmp(name, "glGetTexParameterIiv")) return emscripten_glGetTexParameterIiv;
  if (!strcmp(name, "glGetTexParameterIuiv")) return emscripten_glGetTexParameterIuiv;
  if (!strcmp(name, "glClearBufferiv")) return emscripten_glClearBufferiv;
  if (!strcmp(name, "glClearBufferuiv")) return emscripten_glClearBufferuiv;
  if (!strcmp(name, "glClearBufferfv")) return emscripten_glClearBufferfv;
  if (!strcmp(name, "glClearBufferfi")) return emscripten_glClearBufferfi;
  if (!strcmp(name, "glDrawArraysInstanced")) return emscripten_glDrawArraysInstanced;
  if (!strcmp(name, "glDrawElementsInstanced")) return emscripten_glDrawElementsInstanced;
  if (!strcmp(name, "glTexBuffer")) return emscripten_glTexBuffer;
  if (!strcmp(name, "glPrimitiveRestartIndex")) return emscripten_glPrimitiveRestartIndex;
  if (!strcmp(name, "glActiveTexture")) return emscripten_glActiveTextureARB;
  if (!strcmp(name, "glClientActiveTexture")) return emscripten_glClientActiveTextureARB;
  if (!strcmp(name, "glMultiTexCoord1d")) return emscripten_glMultiTexCoord1dARB;
  if (!strcmp(name, "glMultiTexCoord1dv")) return emscripten_glMultiTexCoord1dvARB;
  if (!strcmp(name, "glMultiTexCoord1f")) return emscripten_glMultiTexCoord1fARB;
  if (!strcmp(name, "glMultiTexCoord1fv")) return emscripten_glMultiTexCoord1fvARB;
  if (!strcmp(name, "glMultiTexCoord1i")) return emscripten_glMultiTexCoord1iARB;
  if (!strcmp(name, "glMultiTexCoord1iv")) return emscripten_glMultiTexCoord1ivARB;
  if (!strcmp(name, "glMultiTexCoord1s")) return emscripten_glMultiTexCoord1sARB;
  if (!strcmp(name, "glMultiTexCoord1sv")) return emscripten_glMultiTexCoord1svARB;
  if (!strcmp(name, "glMultiTexCoord2d")) return emscripten_glMultiTexCoord2dARB;
  if (!strcmp(name, "glMultiTexCoord2dv")) return emscripten_glMultiTexCoord2dvARB;
  if (!strcmp(name, "glMultiTexCoord2f")) return emscripten_glMultiTexCoord2fARB;
  if (!strcmp(name, "glMultiTexCoord2fv")) return emscripten_glMultiTexCoord2fvARB;
  if (!strcmp(name, "glMultiTexCoord2i")) return emscripten_glMultiTexCoord2iARB;
  if (!strcmp(name, "glMultiTexCoord2iv")) return emscripten_glMultiTexCoord2ivARB;
  if (!strcmp(name, "glMultiTexCoord2s")) return emscripten_glMultiTexCoord2sARB;
  if (!strcmp(name, "glMultiTexCoord2sv")) return emscripten_glMultiTexCoord2svARB;
  if (!strcmp(name, "glMultiTexCoord3d")) return emscripten_glMultiTexCoord3dARB;
  if (!strcmp(name, "glMultiTexCoord3dv")) return emscripten_glMultiTexCoord3dvARB;
  if (!strcmp(name, "glMultiTexCoord3f")) return emscripten_glMultiTexCoord3fARB;
  if (!strcmp(name, "glMultiTexCoord3fv")) return emscripten_glMultiTexCoord3fvARB;
  if (!strcmp(name, "glMultiTexCoord3i")) return emscripten_glMultiTexCoord3iARB;
  if (!strcmp(name, "glMultiTexCoord3iv")) return emscripten_glMultiTexCoord3ivARB;
  if (!strcmp(name, "glMultiTexCoord3s")) return emscripten_glMultiTexCoord3sARB;
  if (!strcmp(name, "glMultiTexCoord3sv")) return emscripten_glMultiTexCoord3svARB;
  if (!strcmp(name, "glMultiTexCoord4d")) return emscripten_glMultiTexCoord4dARB;
  if (!strcmp(name, "glMultiTexCoord4dv")) return emscripten_glMultiTexCoord4dvARB;
  if (!strcmp(name, "glMultiTexCoord4f")) return emscripten_glMultiTexCoord4fARB;
  if (!strcmp(name, "glMultiTexCoord4fv")) return emscripten_glMultiTexCoord4fvARB;
  if (!strcmp(name, "glMultiTexCoord4i")) return emscripten_glMultiTexCoord4iARB;
  if (!strcmp(name, "glMultiTexCoord4iv")) return emscripten_glMultiTexCoord4ivARB;
  if (!strcmp(name, "glMultiTexCoord4s")) return emscripten_glMultiTexCoord4sARB;
  if (!strcmp(name, "glMultiTexCoord4sv")) return emscripten_glMultiTexCoord4svARB;
  if (!strcmp(name, "glLoadTransposeMatrixf")) return emscripten_glLoadTransposeMatrixfARB;
  if (!strcmp(name, "glLoadTransposeMatrixd")) return emscripten_glLoadTransposeMatrixdARB;
  if (!strcmp(name, "glMultTransposeMatrixf")) return emscripten_glMultTransposeMatrixfARB;
  if (!strcmp(name, "glMultTransposeMatrixd")) return emscripten_glMultTransposeMatrixdARB;
  if (!strcmp(name, "glCompressedTexImage3D")) return emscripten_glCompressedTexImage3DARB;
  if (!strcmp(name, "glCompressedTexImage2D")) return emscripten_glCompressedTexImage2DARB;
  if (!strcmp(name, "glCompressedTexImage1D")) return emscripten_glCompressedTexImage1DARB;
  if (!strcmp(name, "glCompressedTexSubImage3D")) return emscripten_glCompressedTexSubImage3DARB;
  if (!strcmp(name, "glCompressedTexSubImage2D")) return emscripten_glCompressedTexSubImage2DARB;
  if (!strcmp(name, "glCompressedTexSubImage1D")) return emscripten_glCompressedTexSubImage1DARB;
  if (!strcmp(name, "glGetCompressedTexImage")) return emscripten_glGetCompressedTexImageARB;
  if (!strcmp(name, "glVertexAttrib1d")) return emscripten_glVertexAttrib1dARB;
  if (!strcmp(name, "glVertexAttrib1dv")) return emscripten_glVertexAttrib1dvARB;
  if (!strcmp(name, "glVertexAttrib1f")) return emscripten_glVertexAttrib1fARB;
  if (!strcmp(name, "glVertexAttrib1fv")) return emscripten_glVertexAttrib1fvARB;
  if (!strcmp(name, "glVertexAttrib1s")) return emscripten_glVertexAttrib1sARB;
  if (!strcmp(name, "glVertexAttrib1sv")) return emscripten_glVertexAttrib1svARB;
  if (!strcmp(name, "glVertexAttrib2d")) return emscripten_glVertexAttrib2dARB;
  if (!strcmp(name, "glVertexAttrib2dv")) return emscripten_glVertexAttrib2dvARB;
  if (!strcmp(name, "glVertexAttrib2f")) return emscripten_glVertexAttrib2fARB;
  if (!strcmp(name, "glVertexAttrib2fv")) return emscripten_glVertexAttrib2fvARB;
  if (!strcmp(name, "glVertexAttrib2s")) return emscripten_glVertexAttrib2sARB;
  if (!strcmp(name, "glVertexAttrib2sv")) return emscripten_glVertexAttrib2svARB;
  if (!strcmp(name, "glVertexAttrib3d")) return emscripten_glVertexAttrib3dARB;
  if (!strcmp(name, "glVertexAttrib3dv")) return emscripten_glVertexAttrib3dvARB;
  if (!strcmp(name, "glVertexAttrib3f")) return emscripten_glVertexAttrib3fARB;
  if (!strcmp(name, "glVertexAttrib3fv")) return emscripten_glVertexAttrib3fvARB;
  if (!strcmp(name, "glVertexAttrib3s")) return emscripten_glVertexAttrib3sARB;
  if (!strcmp(name, "glVertexAttrib3sv")) return emscripten_glVertexAttrib3svARB;
  if (!strcmp(name, "glVertexAttrib4Nbv")) return emscripten_glVertexAttrib4NbvARB;
  if (!strcmp(name, "glVertexAttrib4Niv")) return emscripten_glVertexAttrib4NivARB;
  if (!strcmp(name, "glVertexAttrib4Nsv")) return emscripten_glVertexAttrib4NsvARB;
  if (!strcmp(name, "glVertexAttrib4Nub")) return emscripten_glVertexAttrib4NubARB;
  if (!strcmp(name, "glVertexAttrib4Nubv")) return emscripten_glVertexAttrib4NubvARB;
  if (!strcmp(name, "glVertexAttrib4Nuiv")) return emscripten_glVertexAttrib4NuivARB;
  if (!strcmp(name, "glVertexAttrib4Nusv")) return emscripten_glVertexAttrib4NusvARB;
  if (!strcmp(name, "glVertexAttrib4bv")) return emscripten_glVertexAttrib4bvARB;
  if (!strcmp(name, "glVertexAttrib4d")) return emscripten_glVertexAttrib4dARB;
  if (!strcmp(name, "glVertexAttrib4dv")) return emscripten_glVertexAttrib4dvARB;
  if (!strcmp(name, "glVertexAttrib4f")) return emscripten_glVertexAttrib4fARB;
  if (!strcmp(name, "glVertexAttrib4fv")) return emscripten_glVertexAttrib4fvARB;
  if (!strcmp(name, "glVertexAttrib4iv")) return emscripten_glVertexAttrib4ivARB;
  if (!strcmp(name, "glVertexAttrib4s")) return emscripten_glVertexAttrib4sARB;
  if (!strcmp(name, "glVertexAttrib4sv")) return emscripten_glVertexAttrib4svARB;
  if (!strcmp(name, "glVertexAttrib4ubv")) return emscripten_glVertexAttrib4ubvARB;
  if (!strcmp(name, "glVertexAttrib4uiv")) return emscripten_glVertexAttrib4uivARB;
  if (!strcmp(name, "glVertexAttrib4usv")) return emscripten_glVertexAttrib4usvARB;
  if (!strcmp(name, "glVertexAttribPointer")) return emscripten_glVertexAttribPointerARB;
  if (!strcmp(name, "glEnableVertexAttribArray")) return emscripten_glEnableVertexAttribArrayARB;
  if (!strcmp(name, "glDisableVertexAttribArray")) return emscripten_glDisableVertexAttribArrayARB;
  if (!strcmp(name, "glProgramString")) return emscripten_glProgramStringARB;
  if (!strcmp(name, "glBindProgram")) return emscripten_glBindProgramARB;
  if (!strcmp(name, "glDeletePrograms")) return emscripten_glDeleteProgramsARB;
  if (!strcmp(name, "glGenPrograms")) return emscripten_glGenProgramsARB;
  if (!strcmp(name, "glProgramEnvParameter4d")) return emscripten_glProgramEnvParameter4dARB;
  if (!strcmp(name, "glProgramEnvParameter4dv")) return emscripten_glProgramEnvParameter4dvARB;
  if (!strcmp(name, "glProgramEnvParameter4f")) return emscripten_glProgramEnvParameter4fARB;
  if (!strcmp(name, "glProgramEnvParameter4fv")) return emscripten_glProgramEnvParameter4fvARB;
  if (!strcmp(name, "glProgramLocalParameter4d")) return emscripten_glProgramLocalParameter4dARB;
  if (!strcmp(name, "glProgramLocalParameter4dv")) return emscripten_glProgramLocalParameter4dvARB;
  if (!strcmp(name, "glProgramLocalParameter4f")) return emscripten_glProgramLocalParameter4fARB;
  if (!strcmp(name, "glProgramLocalParameter4fv")) return emscripten_glProgramLocalParameter4fvARB;
  if (!strcmp(name, "glGetProgramEnvParameterdv")) return emscripten_glGetProgramEnvParameterdvARB;
  if (!strcmp(name, "glGetProgramEnvParameterfv")) return emscripten_glGetProgramEnvParameterfvARB;
  if (!strcmp(name, "glGetProgramLocalParameterdv")) return emscripten_glGetProgramLocalParameterdvARB;
  if (!strcmp(name, "glGetProgramLocalParameterfv")) return emscripten_glGetProgramLocalParameterfvARB;
  if (!strcmp(name, "glGetProgramiv")) return emscripten_glGetProgramivARB;
  if (!strcmp(name, "glGetProgramString")) return emscripten_glGetProgramStringARB;
  if (!strcmp(name, "glGetVertexAttribdv")) return emscripten_glGetVertexAttribdvARB;
  if (!strcmp(name, "glGetVertexAttribfv")) return emscripten_glGetVertexAttribfvARB;
  if (!strcmp(name, "glGetVertexAttribiv")) return emscripten_glGetVertexAttribivARB;
  if (!strcmp(name, "glGetVertexAttribPointerv")) return emscripten_glGetVertexAttribPointervARB;
  if (!strcmp(name, "glIsProgram")) return emscripten_glIsProgramARB;
  if (!strcmp(name, "glBindBuffer")) return emscripten_glBindBufferARB;
  if (!strcmp(name, "glDeleteBuffers")) return emscripten_glDeleteBuffersARB;
  if (!strcmp(name, "glGenBuffers")) return emscripten_glGenBuffersARB;
  if (!strcmp(name, "glIsBuffer")) return emscripten_glIsBufferARB;
  if (!strcmp(name, "glBufferData")) return emscripten_glBufferDataARB;
  if (!strcmp(name, "glBufferSubData")) return emscripten_glBufferSubDataARB;
  if (!strcmp(name, "glGetBufferSubData")) return emscripten_glGetBufferSubDataARB;
  if (!strcmp(name, "glMapBuffer")) return emscripten_glMapBufferARB;
  if (!strcmp(name, "glUnmapBuffer")) return emscripten_glUnmapBufferARB;
  if (!strcmp(name, "glGetBufferParameteriv")) return emscripten_glGetBufferParameterivARB;
  if (!strcmp(name, "glGetBufferPointerv")) return emscripten_glGetBufferPointervARB;
  if (!strcmp(name, "glGenQueries")) return emscripten_glGenQueriesARB;
  if (!strcmp(name, "glDeleteQueries")) return emscripten_glDeleteQueriesARB;
  if (!strcmp(name, "glIsQuery")) return emscripten_glIsQueryARB;
  if (!strcmp(name, "glBeginQuery")) return emscripten_glBeginQueryARB;
  if (!strcmp(name, "glEndQuery")) return emscripten_glEndQueryARB;
  if (!strcmp(name, "glGetQueryiv")) return emscripten_glGetQueryivARB;
  if (!strcmp(name, "glGetQueryObjectiv")) return emscripten_glGetQueryObjectivARB;
  if (!strcmp(name, "glGetQueryObjectuiv")) return emscripten_glGetQueryObjectuivARB;
  if (!strcmp(name, "glDeleteObject")) return emscripten_glDeleteObjectARB;
  if (!strcmp(name, "glGetHandle")) return emscripten_glGetHandleARB;
  if (!strcmp(name, "glDetachObject")) return emscripten_glDetachObjectARB;
  if (!strcmp(name, "glCreateShaderObject")) return emscripten_glCreateShaderObjectARB;
  if (!strcmp(name, "glShaderSource")) return emscripten_glShaderSourceARB;
  if (!strcmp(name, "glCompileShader")) return emscripten_glCompileShaderARB;
  if (!strcmp(name, "glCreateProgramObject")) return emscripten_glCreateProgramObjectARB;
  if (!strcmp(name, "glAttachObject")) return emscripten_glAttachObjectARB;
  if (!strcmp(name, "glLinkProgram")) return emscripten_glLinkProgramARB;
  if (!strcmp(name, "glUseProgramObject")) return emscripten_glUseProgramObjectARB;
  if (!strcmp(name, "glValidateProgram")) return emscripten_glValidateProgramARB;
  if (!strcmp(name, "glUniform1f")) return emscripten_glUniform1fARB;
  if (!strcmp(name, "glUniform2f")) return emscripten_glUniform2fARB;
  if (!strcmp(name, "glUniform3f")) return emscripten_glUniform3fARB;
  if (!strcmp(name, "glUniform4f")) return emscripten_glUniform4fARB;
  if (!strcmp(name, "glUniform1i")) return emscripten_glUniform1iARB;
  if (!strcmp(name, "glUniform2i")) return emscripten_glUniform2iARB;
  if (!strcmp(name, "glUniform3i")) return emscripten_glUniform3iARB;
  if (!strcmp(name, "glUniform4i")) return emscripten_glUniform4iARB;
  if (!strcmp(name, "glUniform1fv")) return emscripten_glUniform1fvARB;
  if (!strcmp(name, "glUniform2fv")) return emscripten_glUniform2fvARB;
  if (!strcmp(name, "glUniform3fv")) return emscripten_glUniform3fvARB;
  if (!strcmp(name, "glUniform4fv")) return emscripten_glUniform4fvARB;
  if (!strcmp(name, "glUniform1iv")) return emscripten_glUniform1ivARB;
  if (!strcmp(name, "glUniform2iv")) return emscripten_glUniform2ivARB;
  if (!strcmp(name, "glUniform3iv")) return emscripten_glUniform3ivARB;
  if (!strcmp(name, "glUniform4iv")) return emscripten_glUniform4ivARB;
  if (!strcmp(name, "glUniformMatrix2fv")) return emscripten_glUniformMatrix2fvARB;
  if (!strcmp(name, "glUniformMatrix3fv")) return emscripten_glUniformMatrix3fvARB;
  if (!strcmp(name, "glUniformMatrix4fv")) return emscripten_glUniformMatrix4fvARB;
  if (!strcmp(name, "glGetObjectParameterfv")) return emscripten_glGetObjectParameterfvARB;
  if (!strcmp(name, "glGetObjectParameteriv")) return emscripten_glGetObjectParameterivARB;
  if (!strcmp(name, "glGetInfoLog")) return emscripten_glGetInfoLogARB;
  if (!strcmp(name, "glGetAttachedObjects")) return emscripten_glGetAttachedObjectsARB;
  if (!strcmp(name, "glGetUniformLocation")) return emscripten_glGetUniformLocationARB;
  if (!strcmp(name, "glGetActiveUniform")) return emscripten_glGetActiveUniformARB;
  if (!strcmp(name, "glGetUniformfv")) return emscripten_glGetUniformfvARB;
  if (!strcmp(name, "glGetUniformiv")) return emscripten_glGetUniformivARB;
  if (!strcmp(name, "glGetShaderSource")) return emscripten_glGetShaderSourceARB;
  if (!strcmp(name, "glBindAttribLocation")) return emscripten_glBindAttribLocationARB;
  if (!strcmp(name, "glGetActiveAttrib")) return emscripten_glGetActiveAttribARB;
  if (!strcmp(name, "glGetAttribLocation")) return emscripten_glGetAttribLocationARB;
  if (!strcmp(name, "glDrawArraysInstanced")) return emscripten_glDrawArraysInstancedARB;
  if (!strcmp(name, "glDrawElementsInstanced")) return emscripten_glDrawElementsInstancedARB;
  if (!strcmp(name, "glIsRenderbuffer")) return emscripten_glIsRenderbuffer;
  if (!strcmp(name, "glBindRenderbuffer")) return emscripten_glBindRenderbuffer;
  if (!strcmp(name, "glDeleteRenderbuffers")) return emscripten_glDeleteRenderbuffers;
  if (!strcmp(name, "glGenRenderbuffers")) return emscripten_glGenRenderbuffers;
  if (!strcmp(name, "glRenderbufferStorage")) return emscripten_glRenderbufferStorage;
  if (!strcmp(name, "glGetRenderbufferParameteriv")) return emscripten_glGetRenderbufferParameteriv;
  if (!strcmp(name, "glIsFramebuffer")) return emscripten_glIsFramebuffer;
  if (!strcmp(name, "glBindFramebuffer")) return emscripten_glBindFramebuffer;
  if (!strcmp(name, "glDeleteFramebuffers")) return emscripten_glDeleteFramebuffers;
  if (!strcmp(name, "glGenFramebuffers")) return emscripten_glGenFramebuffers;
  if (!strcmp(name, "glCheckFramebufferStatus")) return emscripten_glCheckFramebufferStatus;
  if (!strcmp(name, "glFramebufferTexture1D")) return emscripten_glFramebufferTexture1D;
  if (!strcmp(name, "glFramebufferTexture2D")) return emscripten_glFramebufferTexture2D;
  if (!strcmp(name, "glFramebufferTexture3D")) return emscripten_glFramebufferTexture3D;
  if (!strcmp(name, "glFramebufferRenderbuffer")) return emscripten_glFramebufferRenderbuffer;
  if (!strcmp(name, "glGetFramebufferAttachmentParameteriv")) return emscripten_glGetFramebufferAttachmentParameteriv;
  if (!strcmp(name, "glGenerateMipmap")) return emscripten_glGenerateMipmap;
  if (!strcmp(name, "glBlitFramebuffer")) return emscripten_glBlitFramebuffer;
  if (!strcmp(name, "glRenderbufferStorageMultisample")) return emscripten_glRenderbufferStorageMultisample;
  if (!strcmp(name, "glFramebufferTextureLayer")) return emscripten_glFramebufferTextureLayer;
  if (!strcmp(name, "glBindVertexArray")) return emscripten_glBindVertexArray;
  if (!strcmp(name, "glDeleteVertexArrays")) return emscripten_glDeleteVertexArrays;
  if (!strcmp(name, "glGenVertexArrays")) return emscripten_glGenVertexArrays;
  if (!strcmp(name, "glIsVertexArray")) return emscripten_glIsVertexArray;
  if (!strcmp(name, "glGetUniformIndices")) return emscripten_glGetUniformIndices;
  if (!strcmp(name, "glGetActiveUniformsiv")) return emscripten_glGetActiveUniformsiv;
  if (!strcmp(name, "glGetActiveUniformName")) return emscripten_glGetActiveUniformName;
  if (!strcmp(name, "glGetUniformBlockIndex")) return emscripten_glGetUniformBlockIndex;
  if (!strcmp(name, "glGetActiveUniformBlockiv")) return emscripten_glGetActiveUniformBlockiv;
  if (!strcmp(name, "glGetActiveUniformBlockName")) return emscripten_glGetActiveUniformBlockName;
  if (!strcmp(name, "glUniformBlockBinding")) return emscripten_glUniformBlockBinding;
  if (!strcmp(name, "glReleaseShaderCompiler")) return emscripten_glReleaseShaderCompiler;
  if (!strcmp(name, "glShaderBinary")) return emscripten_glShaderBinary;
  if (!strcmp(name, "glGetShaderPrecisionFormat")) return emscripten_glGetShaderPrecisionFormat;
  if (!strcmp(name, "glDepthRangef")) return emscripten_glDepthRangef;
  if (!strcmp(name, "glClearDepthf")) return emscripten_glClearDepthf;
  if (!strcmp(name, "glVertexAttribDivisor")) return emscripten_glVertexAttribDivisor;
  return 0;
}

