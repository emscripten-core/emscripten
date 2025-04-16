#----------------------------------------------------------------------------
# Autoconf input script. Invoke the ./autogen.sh script to generate a
# configure script from this file.
#----------------------------------------------------------------------------
AC_PREREQ([2.54])

#----------------------------------------------------------------------------
# Initialize Autoconf.
#----------------------------------------------------------------------------
AC_INIT(
    [bullet],
    [2.78],
    [bullet@erwincoumans.com])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL

case "$host" in
        *-*-mingw*|*-*-cygwin*)
                AC_DEFINE(PLATFORM_WIN32, 1, [Platform is Win32])
                opengl_LIBS="-lunsupported_platform"
                PLATFORM_STRING="Win32"
                ;;
        *-*-linux*)
                AC_DEFINE(PLATFORM_LINUX, 1, [Platform is Linux])
                opengl_LIBS="-lGL -lGLU -lglut"
                PLATFORM_STRING="Linux"
                ;;
        *-*-darwin*)
                AC_DEFINE(PLATFORM_APPLE, 1, [Platform is Apple])
                opengl_LIBS="-framework AGL -framework OpenGL -framework GLUT"
                PLATFORM_STRING="Apple"
                ;;
        *)
                AC_MSG_WARN([*** Please add $host to configure.ac checks!])
                ;;
esac
AC_SUBST(opengl_LIBS)

case "$host" in
        i?86-* | k?-* | athlon-* | pentium*-)
                AC_DEFINE(ARCH_X86, 1, [Architecture is x86])
                ARCH_SPECIFIC_CFLAGS=""
                ARCH_STRING="X86"
                ;;
        x86_64-*)
                AC_DEFINE(ARCH_X86_64, 1, [Architecture is x86-64])
                ARCH_SPECIFIC_CFLAGS="-DUSE_ADDR64"
                ARCH_STRING="X86-64"
                ;;
        ppc-* | powerpc-*)
                AC_DEFINE(ARCH_PPC, 1, [Architecture is PowerPC])
                ARCH_SPECIFIC_CFLAGS=""
                ARCH_STRING="PowerPC"
                ;;
        *)
                AC_MSG_ERROR([Unknown Architecture])
                ;;
esac
AC_C_BIGENDIAN


#----------------------------------------------------------------------------
# Setup for the configuration header.
#----------------------------------------------------------------------------
AC_CONFIG_HEADERS([config.h])
#----------------------------------------------------------------------------
# Package configuration switches.
#----------------------------------------------------------------------------
AC_ARG_ENABLE([multithreaded],
	[AC_HELP_STRING([--enable-multithreaded],
		[build BulletMultiThreaded (default NO)])],
	[disable_multithreaded=no], [disable_multithreaded=yes])
AC_MSG_CHECKING([BulletMultiThreaded])
AS_IF([test "$disable_multithreaded" = yes], [build_multithreaded=no], [build_multithreaded=yes])
AC_MSG_RESULT([$build_multithreaded])
AM_CONDITIONAL([CONDITIONAL_BUILD_MULTITHREADED], [test "$build_multithreaded" = yes])

AC_ARG_ENABLE([demos],
    [AS_HELP_STRING([--disable-demos],
            [disable Bullet demos])],
    [],
    [enable_demos=yes])
AM_CONDITIONAL([CONDITIONAL_BUILD_DEMOS], [false])

dnl Check for OpenGL and GLUT
if test "x$drawstuff" = "xOSX"; then
  AC_DEFINE([HAVE_APPLE_OPENGL_FRAMEWORK], [1],
            [Use the Apple OpenGL framework.])
  GL_LIBS="-framework GLUT -framework OpenGL -framework Carbon -framework AGL"
  have_glut=yes
else
  have_gl_headers=yes
  AC_CHECK_HEADERS(GL/gl.h GL/glu.h GL/glext.h GL/glut.h, ,
        [have_gl_headers=no],
       [[#ifdef WIN32
         #include <windows.h>
         #endif
         #if HAVE_GL_GL_H
         #include <GL/gl.h>
         #endif
             #if HAVE_GL_GLU_H
         #include <GL/glu.h>
         #endif
       ]])
  have_gl=no
  have_glu=no
  have_glut=no
  TEMP_LDFLAGS="$LDFLAGS"
  AC_CHECK_LIB(GL, main, [GL_LIBS="-lGL"; have_gl=yes])
  AC_CHECK_LIB(GLU, main, [GL_LIBS="-lGLU $GL_LIBS"; have_glu=yes], , -lGL)
  AC_CHECK_LIB(GLUT, main, [GL_LIBS="-lGLUT -LGLU  $GL_LIBS"; have_glut=yes], ,-lGLUT)
  AC_CHECK_LIB(opengl32, main, [GL_LIBS="-lopengl32"; have_gl=yes])
  AC_CHECK_LIB(glu32, main, [GL_LIBS="-lglu32 $GL_LIBS"; have_glu=yes], , -lopengl32)
  LDFLAGS="$TEMP_LDFLAGS"
  if test $have_gl = no -o $have_glu = no -o $have_gl_headers = no; then
    if test x$enable_demos = xyes; then
      AC_MSG_WARN([Demos and Extras will not be built because OpenGL and GLUT doesn't seem to work. See `config.log' for details.])
    fi
    enable_demos=no
  else
  AC_MSG_NOTICE([Found OpenGL])
  fi
fi
AC_SUBST(GL_LIBS)


if test "x$enable_demos" != xno; then
    AC_MSG_NOTICE([Building Bullet demos])
    AM_CONDITIONAL([CONDITIONAL_BUILD_DEMOS],[true])
fi



AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--enable-debug],
	[build with debugging information (default NO)])],
    [], [enable_debug=no])

AC_MSG_CHECKING([build mode])
AS_IF([test $enable_debug = yes], [build_mode=debug], [build_mode=optimize])
AC_MSG_RESULT([$build_mode])



CFLAGS="$ARCH_SPECIFIC_CFLAGS $CFLAGS"
CXXFLAGS="$ARCH_SPECIFIC_CFLAGS $CXXFLAGS $CFLAGS"
#----------------------------------------------------------------------------
# Emit generated files.
#----------------------------------------------------------------------------
AC_CONFIG_FILES([bullet.pc Makefile src/Makefile Extras/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([

Please type 'make' to build Bullet
])
