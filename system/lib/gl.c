// GL proc address retrieval

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
GLAPI void APIENTRY emscripten_glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
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
  // remove EXT|ARB|OES suffixes
  char *end = strstr(name, "EXT");
  if (end) *end = 0;
  end = strstr(name, "ARB");
  if (end) *end = 0;
  end = strstr(name, "OES");
  if (end) *end = 0;
  // misc renamings
  if (!strcmp(name, "glCreateProgramObject")) name = "glCreateProgram";
  else if (!strcmp(name, "glUseProgramObject")) name = "glUseProgram";
  else if (!strcmp(name, "glCreateShaderObject")) name = "glCreateShader";
  else if (!strcmp(name, "glAttachObject")) name = "glAttachShader";
  else if (!strcmp(name, "glDetachObject")) name = "glDetachShader";
  // main list
  if (!strcmp(name, "glPixelStorei")) return emscripten_glPixelStorei;
  else if (!strcmp(name, "glGetString")) return emscripten_glGetString;
  else if (!strcmp(name, "glGetIntegerv")) return emscripten_glGetIntegerv;
  else if (!strcmp(name, "glGetFloatv")) return emscripten_glGetFloatv;
  else if (!strcmp(name, "glGetBooleanv")) return emscripten_glGetBooleanv;
  else if (!strcmp(name, "glGenTextures")) return emscripten_glGenTextures;
  else if (!strcmp(name, "glDeleteTextures")) return emscripten_glDeleteTextures;
  else if (!strcmp(name, "glCompressedTexImage2D")) return emscripten_glCompressedTexImage2D;
  else if (!strcmp(name, "glCompressedTexSubImage2D")) return emscripten_glCompressedTexSubImage2D;
  else if (!strcmp(name, "glTexImage2D")) return emscripten_glTexImage2D;
  else if (!strcmp(name, "glTexSubImage2D")) return emscripten_glTexSubImage2D;
  else if (!strcmp(name, "glReadPixels")) return emscripten_glReadPixels;
  else if (!strcmp(name, "glBindTexture")) return emscripten_glBindTexture;
  else if (!strcmp(name, "glGetTexParameterfv")) return emscripten_glGetTexParameterfv;
  else if (!strcmp(name, "glGetTexParameteriv")) return emscripten_glGetTexParameteriv;
  else if (!strcmp(name, "glTexParameterfv")) return emscripten_glTexParameterfv;
  else if (!strcmp(name, "glTexParameteriv")) return emscripten_glTexParameteriv;
  else if (!strcmp(name, "glIsTexture")) return emscripten_glIsTexture;
  else if (!strcmp(name, "glGenBuffers")) return emscripten_glGenBuffers;
  else if (!strcmp(name, "glDeleteBuffers")) return emscripten_glDeleteBuffers;
  else if (!strcmp(name, "glGetBufferParameteriv")) return emscripten_glGetBufferParameteriv;
  else if (!strcmp(name, "glBufferData")) return emscripten_glBufferData;
  else if (!strcmp(name, "glBufferSubData")) return emscripten_glBufferSubData;
  else if (!strcmp(name, "glIsBuffer")) return emscripten_glIsBuffer;
  else if (!strcmp(name, "glGenRenderbuffers")) return emscripten_glGenRenderbuffers;
  else if (!strcmp(name, "glDeleteRenderbuffers")) return emscripten_glDeleteRenderbuffers;
  else if (!strcmp(name, "glBindRenderbuffer")) return emscripten_glBindRenderbuffer;
  else if (!strcmp(name, "glGetRenderbufferParameteriv")) return emscripten_glGetRenderbufferParameteriv;
  else if (!strcmp(name, "glIsRenderbuffer")) return emscripten_glIsRenderbuffer;
  else if (!strcmp(name, "glGetUniformfv")) return emscripten_glGetUniformfv;
  else if (!strcmp(name, "glGetUniformiv")) return emscripten_glGetUniformiv;
  else if (!strcmp(name, "glGetUniformLocation")) return emscripten_glGetUniformLocation;
  else if (!strcmp(name, "glGetVertexAttribfv")) return emscripten_glGetVertexAttribfv;
  else if (!strcmp(name, "glGetVertexAttribiv")) return emscripten_glGetVertexAttribiv;
  else if (!strcmp(name, "glGetVertexAttribPointerv")) return emscripten_glGetVertexAttribPointerv;
  else if (!strcmp(name, "glGetActiveUniform")) return emscripten_glGetActiveUniform;
  else if (!strcmp(name, "glUniform1f")) return emscripten_glUniform1f;
  else if (!strcmp(name, "glUniform2f")) return emscripten_glUniform2f;
  else if (!strcmp(name, "glUniform3f")) return emscripten_glUniform3f;
  else if (!strcmp(name, "glUniform4f")) return emscripten_glUniform4f;
  else if (!strcmp(name, "glUniform1i")) return emscripten_glUniform1i;
  else if (!strcmp(name, "glUniform2i")) return emscripten_glUniform2i;
  else if (!strcmp(name, "glUniform3i")) return emscripten_glUniform3i;
  else if (!strcmp(name, "glUniform4i")) return emscripten_glUniform4i;
  else if (!strcmp(name, "glUniform1iv")) return emscripten_glUniform1iv;
  else if (!strcmp(name, "glUniform2iv")) return emscripten_glUniform2iv;
  else if (!strcmp(name, "glUniform3iv")) return emscripten_glUniform3iv;
  else if (!strcmp(name, "glUniform4iv")) return emscripten_glUniform4iv;
  else if (!strcmp(name, "glUniform1fv")) return emscripten_glUniform1fv;
  else if (!strcmp(name, "glUniform2fv")) return emscripten_glUniform2fv;
  else if (!strcmp(name, "glUniform3fv")) return emscripten_glUniform3fv;
  else if (!strcmp(name, "glUniform4fv")) return emscripten_glUniform4fv;
  else if (!strcmp(name, "glUniformMatrix2fv")) return emscripten_glUniformMatrix2fv;
  else if (!strcmp(name, "glUniformMatrix3fv")) return emscripten_glUniformMatrix3fv;
  else if (!strcmp(name, "glUniformMatrix4fv")) return emscripten_glUniformMatrix4fv;
  else if (!strcmp(name, "glBindBuffer")) return emscripten_glBindBuffer;
  else if (!strcmp(name, "glVertexAttrib1fv")) return emscripten_glVertexAttrib1fv;
  else if (!strcmp(name, "glVertexAttrib2fv")) return emscripten_glVertexAttrib2fv;
  else if (!strcmp(name, "glVertexAttrib3fv")) return emscripten_glVertexAttrib3fv;
  else if (!strcmp(name, "glVertexAttrib4fv")) return emscripten_glVertexAttrib4fv;
  else if (!strcmp(name, "glGetAttribLocation")) return emscripten_glGetAttribLocation;
  else if (!strcmp(name, "glGetActiveAttrib")) return emscripten_glGetActiveAttrib;
  else if (!strcmp(name, "glCreateShader")) return emscripten_glCreateShader;
  else if (!strcmp(name, "glDeleteShader")) return emscripten_glDeleteShader;
  else if (!strcmp(name, "glGetAttachedShaders")) return emscripten_glGetAttachedShaders;
  else if (!strcmp(name, "glShaderSource")) return emscripten_glShaderSource;
  else if (!strcmp(name, "glGetShaderSource")) return emscripten_glGetShaderSource;
  else if (!strcmp(name, "glCompileShader")) return emscripten_glCompileShader;
  else if (!strcmp(name, "glGetShaderInfoLog")) return emscripten_glGetShaderInfoLog;
  else if (!strcmp(name, "glGetShaderiv")) return emscripten_glGetShaderiv;
  else if (!strcmp(name, "glGetProgramiv")) return emscripten_glGetProgramiv;
  else if (!strcmp(name, "glIsShader")) return emscripten_glIsShader;
  else if (!strcmp(name, "glCreateProgram")) return emscripten_glCreateProgram;
  else if (!strcmp(name, "glDeleteProgram")) return emscripten_glDeleteProgram;
  else if (!strcmp(name, "glAttachShader")) return emscripten_glAttachShader;
  else if (!strcmp(name, "glDetachShader")) return emscripten_glDetachShader;
  else if (!strcmp(name, "glGetShaderPrecisionFormat")) return emscripten_glGetShaderPrecisionFormat;
  else if (!strcmp(name, "glLinkProgram")) return emscripten_glLinkProgram;
  else if (!strcmp(name, "glGetProgramInfoLog")) return emscripten_glGetProgramInfoLog;
  else if (!strcmp(name, "glUseProgram")) return emscripten_glUseProgram;
  else if (!strcmp(name, "glValidateProgram")) return emscripten_glValidateProgram;
  else if (!strcmp(name, "glIsProgram")) return emscripten_glIsProgram;
  else if (!strcmp(name, "glBindAttribLocation")) return emscripten_glBindAttribLocation;
  else if (!strcmp(name, "glBindFramebuffer")) return emscripten_glBindFramebuffer;
  else if (!strcmp(name, "glGenFramebuffers")) return emscripten_glGenFramebuffers;
  else if (!strcmp(name, "glDeleteFramebuffers")) return emscripten_glDeleteFramebuffers;
  else if (!strcmp(name, "glFramebufferRenderbuffer")) return emscripten_glFramebufferRenderbuffer;
  else if (!strcmp(name, "glFramebufferTexture2D")) return emscripten_glFramebufferTexture2D;
  else if (!strcmp(name, "glGetFramebufferAttachmentParameteriv")) return emscripten_glGetFramebufferAttachmentParameteriv;
  else if (!strcmp(name, "glIsFramebuffer")) return emscripten_glIsFramebuffer;
  else if (!strcmp(name, "glDeleteObject")) return emscripten_glDeleteObjectARB;
  else if (!strcmp(name, "glGetObjectParameteriv")) return emscripten_glGetObjectParameterivARB;
  else if (!strcmp(name, "glGetInfoLog")) return emscripten_glGetInfoLogARB;
  else if (!strcmp(name, "glBindProgram")) return emscripten_glBindProgramARB;
  else if (!strcmp(name, "glGetPointerv")) return emscripten_glGetPointerv;
  else if (!strcmp(name, "glDrawRangeElements")) return emscripten_glDrawRangeElements;
  else if (!strcmp(name, "glEnableClientState")) return emscripten_glEnableClientState;
  else if (!strcmp(name, "glVertexPointer")) return emscripten_glVertexPointer;
  else if (!strcmp(name, "glTexCoordPointer")) return emscripten_glTexCoordPointer;
  else if (!strcmp(name, "glNormalPointer")) return emscripten_glNormalPointer;
  else if (!strcmp(name, "glColorPointer")) return emscripten_glColorPointer;
  else if (!strcmp(name, "glClientActiveTexture")) return emscripten_glClientActiveTexture;
  else if (!strcmp(name, "glGenVertexArrays")) return emscripten_glGenVertexArrays;
  else if (!strcmp(name, "glDeleteVertexArrays")) return emscripten_glDeleteVertexArrays;
  else if (!strcmp(name, "glBindVertexArray")) return emscripten_glBindVertexArray;
  else if (!strcmp(name, "glMatrixMode")) return emscripten_glMatrixMode;
  else if (!strcmp(name, "glLoadIdentity")) return emscripten_glLoadIdentity;
  else if (!strcmp(name, "glLoadMatrixf")) return emscripten_glLoadMatrixf;
  else if (!strcmp(name, "glFrustum")) return emscripten_glFrustum;
  else if (!strcmp(name, "glRotatef")) return emscripten_glRotatef;
  else if (!strcmp(name, "glVertexAttribPointer")) return emscripten_glVertexAttribPointer;
  else if (!strcmp(name, "glEnableVertexAttribArray")) return emscripten_glEnableVertexAttribArray;
  else if (!strcmp(name, "glDisableVertexAttribArray")) return emscripten_glDisableVertexAttribArray;
  else if (!strcmp(name, "glDrawArrays")) return emscripten_glDrawArrays;
  else if (!strcmp(name, "glDrawElements")) return emscripten_glDrawElements;
  else if (!strcmp(name, "glShaderBinary")) return emscripten_glShaderBinary;
  else if (!strcmp(name, "glReleaseShaderCompiler")) return emscripten_glReleaseShaderCompiler;
  else if (!strcmp(name, "glGetError")) return emscripten_glGetError;
  else if (!strcmp(name, "glVertexAttribDivisor")) return emscripten_glVertexAttribDivisor;
  else if (!strcmp(name, "glDrawArraysInstanced")) return emscripten_glDrawArraysInstanced;
  else if (!strcmp(name, "glDrawElementsInstanced")) return emscripten_glDrawElementsInstanced;
  else if (!strcmp(name, "glFinish")) return emscripten_glFinish;
  else if (!strcmp(name, "glFlush")) return emscripten_glFlush;
  else if (!strcmp(name, "glClearDepth")) return emscripten_glClearDepth;
  else if (!strcmp(name, "glClearDepthf")) return emscripten_glClearDepthf;
  else if (!strcmp(name, "glDepthFunc")) return emscripten_glDepthFunc;
  else if (!strcmp(name, "glEnable")) return emscripten_glEnable;
  else if (!strcmp(name, "glDisable")) return emscripten_glDisable;
  else if (!strcmp(name, "glFrontFace")) return emscripten_glFrontFace;
  else if (!strcmp(name, "glCullFace")) return emscripten_glCullFace;
  else if (!strcmp(name, "glClear")) return emscripten_glClear;
  else if (!strcmp(name, "glLineWidth")) return emscripten_glLineWidth;
  else if (!strcmp(name, "glClearStencil")) return emscripten_glClearStencil;
  else if (!strcmp(name, "glDepthMask")) return emscripten_glDepthMask;
  else if (!strcmp(name, "glStencilMask")) return emscripten_glStencilMask;
  else if (!strcmp(name, "glCheckFramebufferStatus")) return emscripten_glCheckFramebufferStatus;
  else if (!strcmp(name, "glGenerateMipmap")) return emscripten_glGenerateMipmap;
  else if (!strcmp(name, "glActiveTexture")) return emscripten_glActiveTexture;
  else if (!strcmp(name, "glBlendEquation")) return emscripten_glBlendEquation;
  else if (!strcmp(name, "glIsEnabled")) return emscripten_glIsEnabled;
  else if (!strcmp(name, "glBlendFunc")) return emscripten_glBlendFunc;
  else if (!strcmp(name, "glBlendEquationSeparate")) return emscripten_glBlendEquationSeparate;
  else if (!strcmp(name, "glDepthRange")) return emscripten_glDepthRange;
  else if (!strcmp(name, "glDepthRangef")) return emscripten_glDepthRangef;
  else if (!strcmp(name, "glStencilMaskSeparate")) return emscripten_glStencilMaskSeparate;
  else if (!strcmp(name, "glHint")) return emscripten_glHint;
  else if (!strcmp(name, "glPolygonOffset")) return emscripten_glPolygonOffset;
  else if (!strcmp(name, "glVertexAttrib1f")) return emscripten_glVertexAttrib1f;
  else if (!strcmp(name, "glSampleCoverage")) return emscripten_glSampleCoverage;
  else if (!strcmp(name, "glTexParameteri")) return emscripten_glTexParameteri;
  else if (!strcmp(name, "glTexParameterf")) return emscripten_glTexParameterf;
  else if (!strcmp(name, "glVertexAttrib2f")) return emscripten_glVertexAttrib2f;
  else if (!strcmp(name, "glStencilFunc")) return emscripten_glStencilFunc;
  else if (!strcmp(name, "glStencilOp")) return emscripten_glStencilOp;
  else if (!strcmp(name, "glViewport")) return emscripten_glViewport;
  else if (!strcmp(name, "glClearColor")) return emscripten_glClearColor;
  else if (!strcmp(name, "glScissor")) return emscripten_glScissor;
  else if (!strcmp(name, "glVertexAttrib3f")) return emscripten_glVertexAttrib3f;
  else if (!strcmp(name, "glColorMask")) return emscripten_glColorMask;
  else if (!strcmp(name, "glRenderbufferStorage")) return emscripten_glRenderbufferStorage;
  else if (!strcmp(name, "glBlendFuncSeparate")) return emscripten_glBlendFuncSeparate;
  else if (!strcmp(name, "glBlendColor")) return emscripten_glBlendColor;
  else if (!strcmp(name, "glStencilFuncSeparate")) return emscripten_glStencilFuncSeparate;
  else if (!strcmp(name, "glStencilOpSeparate")) return emscripten_glStencilOpSeparate;
  else if (!strcmp(name, "glVertexAttrib4f")) return emscripten_glVertexAttrib4f;
  else if (!strcmp(name, "glCopyTexImage2D")) return emscripten_glCopyTexImage2D;
  else if (!strcmp(name, "glCopyTexSubImage2D")) return emscripten_glCopyTexSubImage2D;
  else if (!strcmp(name, "glDrawBuffers")) return emscripten_glDrawBuffers;

  fprintf(stderr, "bad name in getProcAddress: %s | %s\n", name_, name);
  return 0;
}

