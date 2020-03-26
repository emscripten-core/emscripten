# checkbuild.m4                                                -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_SPLIT_TUPLE(TUPLE, OUTPUT-VARIABLES)
#	Split a build-tuple into its component parts.  A build tuple is
#	constructed by CS_CREATE_TUPLE() and is comprised of compiler flags,
#	linker flags, and library references.  OUTPUT-VARIABLES is a
#	comma-delimited list of shell variables which should receive the
#	extracted compiler flags, linker flags, and library references,
#	respectively.
#------------------------------------------------------------------------------
AC_DEFUN([CS_SPLIT_TUPLE],
    [CS_SPLIT([$1], [cs_dummy,$2], [@])
    m4_map([_CS_SPLIT_TUPLE], [$2])])

AC_DEFUN([_CS_SPLIT_TUPLE],
    [$1=`echo $$1 | sed 'y%@%:@% %'`
    ])



#------------------------------------------------------------------------------
# CS_CREATE_TUPLE([CFLAGS], [LFLAGS], [LIBS])
#	Construct a build-tuple which is comprised of compiler flags, linker
#	flags, and library references.  Build tuples are encoded so as to
#	preserve whitespace in each component.  This makes it possible for
#	macros (such as CS_BUILD_IFELSE) which employ build tuples to accept
#	whitespace-delimited lists of tuples, and for shell "for" statements to
#	iterate over tuple lists without compromising whitespace embedded
#	within individual flags or library references.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CREATE_TUPLE], [`echo @$1@$2@$3 | sed 'y% %@%:@%'`])



#------------------------------------------------------------------------------
# CS_LANG_CFLAGS
#	Return the literal string CFLAGS if the current language is C.  Return
#	the literal string CXXFLAGS if the current language is C++.  Generic
#	compiler test macros which need to modify or save the compiler flags
#	can invoke this macro to get the name of the compiler flags environment
#	variable (either CFLAGS or CXXFLAGS) depending upon the current
#	language.  For example:
#		CS_LANG_CFLAGS="$CS_LANG_CFLAGS -Wall"
#	With C, this expands to:
#		CFLAGS="$CFLAGS -Wall"
#	With C++, it expands to:
#		CXXFLAGS="$CXXFLAGS -Wall"
#------------------------------------------------------------------------------
AC_DEFUN([CS_LANG_CFLAGS], [AC_LANG_CASE([C], [CFLAGS], [C++], [CXXFLAGS])])



#------------------------------------------------------------------------------
# CS_BUILD_IFELSE([PROGRAM], [FLAGS], [LANGUAGE], [ACTION-IF-BUILT],
#                 [ACTION-IF-NOT-BUILT], [OTHER-CFLAGS], [OTHER-LFLAGS],
#                 [OTHER-LIBS], [INHIBIT-OTHER-FLAGS], [ERROR-REGEX])
#	Try building a program using the supplied compiler flags, linker flags,
#	and library references.  PROGRAM is typically a program composed via
#	AC_LANG_PROGRAM().  PROGRAM may be omitted if you are interested only
#	in learning if the compiler or linker respects certain flags.  LANGUAGE
#	is typically either C or C++ and specifies which compiler to use for
#	the test.  If LANGUAGE is omitted, C is used.  FLAGS is a whitespace
#	delimited list of build tuples.  Tuples are created with
#	CS_CREATE_TUPLE() and are composed of up to three elements each.  The
#	first element represents compiler flags, the second linker flags, and
#	the third libraries used when linking the program.  Each tuple from
#	FLAGS is attempted in order.  If you want a build attempted with no
#	special flags prior to builds with specialized flags, create an empty
#	tuple with CS_CREATE_TUPLE() at the start of the FLAGS list.  If the
#	build is successful, then the shell variables cs_build_ok is set to
#	"yes", cs_build_cflags, cs_build_lflags, and cs_build_libs are set to
#	the tuple elements which resulted in the successful build, and
#	ACTION-IF-BUILT is invoked.  Upon successful build, no further tuples
#	are consulted.  If no tuple results in a successful build, then
#	cs_build_ok is set to "no" and ACTION-IF-NOT-BUILT is invoked.
#	OTHER-CFLAGS, OTHER-LFLAGS, and OTHER-LIBS specify additional compiler
#	flags, linker flags, and libraries which should be used with each tuple
#	build attempt.  Upon successful build, these additional flags are also
#	reflected in the variables cs_build_cflags, cs_build_lflags, and
#	cs_build_libs unless INHIBIT-OTHER-FLAGS is a non-empty string.  The
#	optional ERROR-REGEX places an additional constraint upon the build
#	check.  If specified, ERROR-REGEX, which is a standard `grep' regular
#	expression, is applied to output captured from the compiler and linker.
#	If ERROR-REGEX matches, then the build is deemed a failure, and
#	cs_build_ok is set to "no".  This facility is useful for broken build
#	tools which emit an error message yet still return success as a result.
#	In such cases, it should be possible to detect the failure by scanning
#	the tools' output.
#
# IMPLEMENTATION NOTES
#
#	In Autoconf 2.57 and earlier, AC_LINK_IFELSE() invokes AC_TRY_EVAL(),
#	which does not provide access to the captured output.  To work around
#	this limitation, we temporarily re-define AC_TRY_EVAL() as
#	_AC_EVAL_STDERR(), which leaves the captured output in conftest.err
#	(which we must also delete).  In Autoconf 2.58, however,
#	AC_LINK_IFELSE() instead already invokes _AC_EVAL_STDERR() on our
#	behalf, however we must be careful to apply ERROR-REGEX within the
#	invocation AC_LINK_IFELSE(), since AC_LINK_IFELSE() deletes
#	conftest.err before it returns.
#------------------------------------------------------------------------------
AC_DEFUN([CS_BUILD_IFELSE],
    [AC_LANG_PUSH(m4_default([$3],[C]))
    cs_cflags_save="$CS_LANG_CFLAGS"
    cs_lflags_save="$LDFLAGS"
    cs_libs_save="$LIBS"
    cs_build_ok=no
    m4_ifval([$10], [m4_pushdef([AC_TRY_EVAL], [_AC_EVAL_STDERR]($$[1]))])

    for cs_build_item in m4_default([$2],[CS_CREATE_TUPLE()])
    do
	CS_SPLIT_TUPLE(
	    [$cs_build_item],[cs_cflags_test,cs_lflags_test,cs_libs_test])
	CS_LANG_CFLAGS="$cs_cflags_test $6 $cs_cflags_save"
	LDFLAGS="$cs_lflags_test $7 $cs_lflags_save"
	LIBS="$cs_libs_test $8 $cs_libs_save"
	AC_LINK_IFELSE(m4_default([$1], [AC_LANG_PROGRAM([],[])]),
	    [m4_ifval([$10],
		[AS_IF([AC_TRY_COMMAND(
		    [grep "AS_ESCAPE([$10])" conftest.err >/dev/null 2>&1])],
		    [cs_build_ok=no], [cs_build_ok=yes])],
		[cs_build_ok=yes])])
	AS_IF([test $cs_build_ok = yes], [break])
    done

    m4_ifval([$10], [m4_popdef([AC_TRY_EVAL]) rm -f conftest.err])
    CS_LANG_CFLAGS=$cs_cflags_save
    LDFLAGS=$cs_lflags_save
    LIBS=$cs_libs_save
    AC_LANG_POP(m4_default([$3],[C]))

    AS_IF([test $cs_build_ok = yes],
	[cs_build_cflags=CS_TRIM([$cs_cflags_test[]m4_ifval([$9],[],[ $6])])
	cs_build_lflags=CS_TRIM([$cs_lflags_test[]m4_ifval([$9],[],[ $7])])
	cs_build_libs=CS_TRIM([$cs_libs_test[]m4_ifval([$9],[],[ $8])])
	$4],
	[$5])])



#------------------------------------------------------------------------------
# CS_CHECK_BUILD(MESSAGE, CACHE-VAR, [PROGRAM], [FLAGS], [LANGUAGE],
#                [ACTION-IF-BUILT], [ACTION-IF-NOT-BUILT], [IGNORE-CACHE],
#                [OTHER-CFLAGS], [OTHER-LFLAGS], [OTHER-LIBS],
#                [INHIBIT-OTHER-FLAGS], [ERROR-REGEX])
#	Like CS_BUILD_IFELSE() but also prints "checking" and result messages,
#	and optionally respects the cache.  Sets CACHE-VAR to "yes" upon
#	success, else "no" upon failure.  Additionally, sets CACHE-VAR_cflags,
#	CACHE-VAR_lflags, and CACHE-VAR_libs to the values which resulted in a
#	successful build.  If IGNORE-CACHE is "yes", then the cache variables
#	are ignored upon entry to this macro, however they are still set to
#	appropriate values upon exit.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_BUILD],
    [AS_IF([test "$8" != yes],
	[AC_CACHE_CHECK([$1], [$2],
	    [CS_BUILD_IFELSE([$3], [$4], [$5],
		[$2=yes
		$2_cflags=$cs_build_cflags
		$2_lflags=$cs_build_lflags
		$2_libs=$cs_build_libs],
		[$2=no], [$9], [$10], [$11], [$12], [$13])])],
	[AC_MSG_CHECKING([$1])
	    CS_BUILD_IFELSE([$3], [$4], [$5],
		[$2=yes
		$2_cflags=$cs_build_cflags
		$2_lflags=$cs_build_lflags
		$2_libs=$cs_build_libs],
		[$2=no], [$9], [$10], [$11], [$12], [$13])
	    AC_MSG_RESULT([$$2])])
    AS_IF([test $$2 = yes], [$6],
	[$2_cflags=''
	$2_lflags=''
	$2_libs=''
	$7])])



#------------------------------------------------------------------------------
# CS_CHECK_BUILD_FLAGS(MESSAGE, CACHE-VAR, FLAGS, [LANGUAGE],
#                     [ACTION-IF-RECOGNIZED], [ACTION-IF-NOT-RECOGNIZED],
#                     [OTHER-CFLAGS], [OTHER-LFLAGS], [OTHER-LIBS],
#                     [ERROR-REGEX])
#	Like CS_CHECK_BUILD(), but checks only if the compiler or linker
#	recognizes a command-line option or options.  MESSAGE is the "checking"
#	message.  CACHE-VAR is the shell cache variable which receives the flag
#	or flags recognized by the compiler or linker.  FLAGS is a
#	whitespace-delimited list of build tuples created with
#	CS_CREATE_TUPLE().  Each tuple from FLAGS is attempted in order until
#	one is found which is recognized by the compiler.  After that, no
#	further flags are checked.  LANGUAGE is typically either C or C++ and
#	specifies which compiler to use for the test.  If LANGUAGE is omitted,
#	C is used.  If a command-line option is recognized, then CACHE-VAR is
#	set to the composite value of $cs_build_cflags, $cs_build_lflags, and
#	$cs_build_libs of the FLAGS element which succeeded (not including the
#	"other" flags) and ACTION-IF-RECOGNIZED is invoked.  If no options are
#	recognized, then CACHE-VAR is set to the empty string, and
#	ACTION-IF-NOT-RECOGNIZED is invoked. As a convenience, in case
#	comparing CACHE-VAR against the empty string to test for failure is
#	undesirable, a second variable named CACHE-VAR_ok is set to the literal
#	"no" upon failure, and to the same value as CACHE-VAR upon success.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_BUILD_FLAGS],
    [AC_CACHE_CHECK([$1], [$2_ok],
	[CS_BUILD_IFELSE([], [$3], [$4],
	    [$2=CS_TRIM([$cs_build_cflags $cs_build_lflags $cs_build_libs])
	    $2_ok="$$2"],
	    [$2=''
	    $2_ok=no], [$7], [$8], [$9], [Y], [$10])])
    AS_IF([test "$$2_ok" != no], [$5], [$6])])
#==============================================================================
# Copyright (C)2003-2006 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_LINK
#	Checks for common tools related to linking.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_LINK],
    [
    # The default RANLIB in Jambase is wrong on some platforms, and is also
    # unsuitable during cross-compilation, so we set the value unconditionally
    # (sixth argument of CS_EMIT_BUILD_PROPERTY).
    AC_PROG_RANLIB
    CS_EMIT_BUILD_PROPERTY([RANLIB], [$RANLIB], [], [], [], [Y])
    
    CS_CHECK_TOOLS([DLLTOOL], [dlltool])
    CS_EMIT_BUILD_PROPERTY([CMD.DLLTOOL], [$DLLTOOL])
    
    CS_CHECK_TOOLS([DLLWRAP], [dllwrap])
    CS_EMIT_BUILD_PROPERTY([CMD.DLLWRAP], [$DLLWRAP])
    
    CS_CHECK_TOOLS([WINDRES], [windres])
    CS_EMIT_BUILD_PROPERTY([CMD.WINDRES], [$WINDRES])
    
    CS_CHECK_TOOLS([STRINGS], [strings])
    CS_EMIT_BUILD_PROPERTY([CMD.STRINGS], [$STRINGS])

    CS_CHECK_TOOLS([OBJCOPY], [objcopy])
    CS_EMIT_BUILD_PROPERTY([CMD.OBJCOPY], [$OBJCOPY])
    
    CS_CHECK_LIBTOOL
    CS_EMIT_BUILD_PROPERTY([LIBTOOL], [$LIBTOOL])
    CS_EMIT_BUILD_PROPERTY([APPLE_LIBTOOL], [$APPLE_LIBTOOL])
    ])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_BASIC
#	Checks for basic tools for building things.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_BASIC],
    [CS_CHECK_MKDIR
    CS_EMIT_BUILD_PROPERTY([CMD.MKDIR], [$MKDIR])
    CS_EMIT_BUILD_PROPERTY([CMD.MKDIRS], [$MKDIRS])

    CS_CHECK_PROGS([INSTALL], [install])
    CS_EMIT_BUILD_PROPERTY([INSTALL], [$INSTALL])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_DOC_TEXINFO
#	Checks for tools to generate documentation from texinfo files.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_DOC_TEXINFO],
    [CS_CHECK_PROGS([TEXI2DVI], [texi2dvi])
    CS_EMIT_BUILD_PROPERTY([CMD.TEXI2DVI], [$TEXI2DVI])

    CS_CHECK_PROGS([TEXI2PDF], [texi2pdf])
    CS_EMIT_BUILD_PROPERTY([CMD.TEXI2PDF], [$TEXI2PDF])

    CS_CHECK_PROGS([DVIPS], [dvips])
    CS_EMIT_BUILD_PROPERTY([CMD.DVIPS], [$DVIPS])

    CS_CHECK_PROGS([DVIPDF], [dvipdf])
    CS_EMIT_BUILD_PROPERTY([CMD.DVIPDF], [$DVIPDF])

    CS_CHECK_PROGS([MAKEINFO], [makeinfo])
    CS_EMIT_BUILD_PROPERTY([CMD.MAKEINFO], [$MAKEINFO])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_TOOLS_DOC_DOXYGEN
#	Checks for tools to generate source documentation via doxygen.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_TOOLS_DOC_DOXYGEN],
    [CS_CHECK_PROGS([DOXYGEN], [doxygen])
    CS_EMIT_BUILD_PROPERTY([CMD.DOXYGEN], [$DOXYGEN])

    CS_CHECK_TOOLS([DOT], [dot])
    CS_EMIT_BUILD_PROPERTY([CMD.DOT], [$DOT])])


#------------------------------------------------------------------------------
# CS_CHECK_COMMON_LIBS
#       Check for typical required libraries (libm, libmx, libdl, libnsl).
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_COMMON_LIBS],
    [AC_LANG_PUSH([C])
    AC_CHECK_LIB([m], [pow], [cs_cv_libm_libs=-lm], [cs_cv_libm_libs=])
    AC_CHECK_LIB([m], [cosf], [cs_cv_libm_libs=-lm])
    AC_CHECK_LIB([mx], [cosf])
    AC_CHECK_LIB([dl], [dlopen], [cs_cv_libdl_libs=-ldl], [cs_cv_libdl_libs=])
    AC_CHECK_LIB([nsl], [gethostbyname])
    AC_LANG_POP([C])])
# checkcppunit.m4                                              -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_CPPUNIT([EMITTER])
#	Check if CppUnit (http://cppunit.sourceforge.net/), the unit-testing
#	framework is available. The shell variable cs_cv_libcppunit is set to
#	"yes" if CppUnit is discovered, else "no".  If available, then the
#	variables cs_cv_libcppunit_cflags, cs_cv_libcppunit_lflags, and
#	cs_cv_libcppunit_libs are set. If EMITTER is provided, then
#	CS_EMIT_BUILD_RESULT() is invoked with EMITTER in order to record the
#	results in an output file. As a convenience, if EMITTER is the literal
#	value "emit" or "yes", then CS_EMIT_BUILD_RESULT()'s default emitter
#	will be used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_CPPUNIT],
    [CS_CHECK_LIB_WITH([cppunit],
	[AC_LANG_PROGRAM([[#include <cppunit/ui/text/TestRunner.h>]],
	    [CppUnit::TextUi::TestRunner r; r.run();])],
	[], [C++])
	
    AS_IF([test $cs_cv_libcppunit = yes],
	[CS_CHECK_BUILD([if cppunit is sufficiently recent],
	    [cs_cv_libcppunit_recent],
	    [AC_LANG_PROGRAM(
		[[#include <cppunit/BriefTestProgressListener.h>]], 
		[CppUnit::BriefTestProgressListener b; b.startTest(0);])],
	    [], [C++],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libcppunit], [CPPUNIT],
		CS_EMITTER_OPTIONAL([$1]))], [], [],
	    [$cs_cv_libcppunit_cflags],
	    [$cs_cv_libcppunit_lflags],
	    [$cs_cv_libcppunit_libs])])])
# checklib.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# cs_lib_paths_default
#	Whitespace delimited list of directory tuples in which to search, by
#	default, for external libraries.  Each list item can specify an
#	include|library directory tuple (for example, "/usr/include|/usr/lib"),
#	or a single directory (for example, "/usr").  If the second form is
#	used, then "include" and "lib" subdirectories of the directory are
#	searched.  If the library resources are not found, then the directory
#	itself is searched.  Thus, "/proj" is shorthand for
#	"/proj/include|/proj/lib /proj|/proj".
#
# Present Cases:
#	/usr/local -- Not all compilers search here by default, so we specify
#		it manually.
#	/sw -- Fink, the MacOS/X manager of Unix packages, installs here by
#		default.
#	/opt/local -- DarwinPorts installs here by default.
#------------------------------------------------------------------------------
m4_define([cs_lib_paths_default],
    [/usr/local/include|/usr/local/lib \
    /sw/include|/sw/lib \
    /opt/local/include|/opt/local/lib \
    /opt/include|/opt/lib])



#------------------------------------------------------------------------------
# cs_pkg_paths_default
#	Comma delimited list of additional directories in which the
#	`pkg-config' command should search for its `.pc' files.
#
# Present Cases:
#	/usr/local/lib/pkgconfig -- Although a common location for .pc files
#		installed by "make install", many `pkg-config' commands neglect
#		to search here automatically.
#	/sw/lib/pkgconfig -- Fink, the MacOS/X manager of Unix packages,
#		installs .pc files here by default.
#	/opt/local/lib/pkgconfig -- DarwinPorts installs .pc files here by
#		default.
#------------------------------------------------------------------------------
m4_define([cs_pkg_paths_default],
    [/usr/local/lib/pkgconfig,
    /sw/lib/pkgconfig,
    /opt/local/lib/pkgconfig,
    /opt/lib/pkgconfig])



#------------------------------------------------------------------------------
# CS_CHECK_LIB_WITH(LIBRARY, PROGRAM, [SEARCH-LIST], [LANGUAGE],
#                   [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND], [OTHER-CFLAGS],
#                   [OTHER-LFLAGS], [OTHER-LIBS], [ALIASES])
#	Very roughly similar in concept to AC_CHECK_LIB(), but allows caller to
#	to provide list of directories in which to search for LIBRARY; allows
#	user to override library location via --with-LIBRARY=dir; and consults
#	`pkg-config' (if present) and `LIBRARY-config' (if present, i.e.
#	`sdl-config') in order to obtain compiler and linker flags.  LIBRARY is
#	the name of the library or MacOS/X framework which is to be located
#	(for example, "readline" for `libreadline.a' or `readline.framework').
#	PROGRAM, which is typically composed with AC_LANG_PROGRAM(), is a
#	program which references at least one function or symbol in LIBRARY.
#	SEARCH-LIST is a whitespace-delimited list of paths in which to search
#	for the library and its header files, in addition to those searched by
#	the compiler and linker by default, and those referenced by the
#	cs_lib_paths_default macro.  Each list item can specify an
#	`include|library' directory tuple (for example,
#	"/usr/include|/usr/lib"), or a single directory (for example, "/usr").
#	If the second form is used, then "include" and "lib" subdirectories of
#	the directory are searched.  If the library resources are not found,
#	then the directory itself is searched.  Thus, "/proj" is shorthand for
#	"/proj/include|/proj/lib /proj|/proj".  Items in the search list can
#	include wildcards.  SEARCH-LIST can be overridden by the user with the
#	--with-LIBRARY=dir option, in which case only "dir/include|dir/lib" and
#	"dir|dir" are searched.  If SEARCH-LIST is omitted and the user did not
#	override the search list via --with-LIBRARY=dir, then only the
#	directories normally searched by the compiler and the directories
#	mentioned via cs_lib_paths_default are searched.  LANGUAGE is typically
#	either C or C++ and specifies which compiler to use for the test.  If
#	LANGUAGE is omitted, C is used.  OTHER-CFLAGS, OTHER-LFLAGS, and
#	OTHER-LIBS can specify additional compiler flags, linker flags, and
#	libraries needed to successfully link with LIBRARY.  The optional
#	ALIASES is a comma-delimited list of library names for which to search
#	in case LIBRARY is not located (for example "[sdl1.2, sdl12]" for
#	libsdl1.2.a, sdl1.2.framework, libsdl12.a, and sdl12.framework).  If
#	the library or one of its aliases is found and can be successfully
#	linked into a program, then the shell cache variable cs_cv_libLIBRARY
#	is set to "yes"; cs_cv_libLIBRARY_cflags, cs_cv_libLIBRARY_lflags, and
#	cs_cv_libLIBRARY_libs are set, respectively, to the compiler flags
#	(including OTHER-CFLAGS), linker flags (including OTHER-LFLAGS), and
#	library references (including OTHER-LIBS) which resulted in a
#	successful build; and ACTION-IF-FOUND is invoked.  If the library was
#	not found or was unlinkable, or if the user disabled the library via
#	--without-LIBRARY, then cs_cv_libLIBRARY is set to "no" and
#	ACTION-IF-NOT-FOUND is invoked.  Note that the exported shell variable
#	names are always composed from LIBRARY regardless of whether the test
#	succeeded because the primary library was discovered or one of the
#	aliases.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIB_WITH],
    [AC_ARG_WITH([$1], [AC_HELP_STRING([--with-$1=dir],
	[specify location of lib$1 if not detected automatically; searches
	dir/include, dir/lib, and dir])])

    # Backward compatibility: Recognize --with-lib$1 as alias for --with-$1.
    AS_IF([test -n "$with_lib$1" && test -z "$with_$1"],
	[with_$1="$with_lib$1"])

    AS_IF([test -z "$with_$1"], [with_$1=yes])
    AS_IF([test "$with_$1" != no],
	[# If --with-$1 value is same as cached value, then assume other
	 # cached values are also valid; otherwise, ignore all cached values.
	AS_IF([test "$with_$1" != "$cs_cv_with_$1"],
	    [cs_ignore_cache=yes], [cs_ignore_cache=no])

	cs_check_lib_flags=''
	AS_IF([test $with_$1 = yes],
	    [m4_foreach([cs_check_lib_alias], [$1, $10],
		[_CS_CHECK_LIB_PKG_CONFIG_FLAGS([cs_check_lib_flags],
		    cs_check_lib_alias)
		_CS_CHECK_LIB_CONFIG_FLAGS([cs_check_lib_flags],
		    cs_check_lib_alias)
		])])

	AS_IF([test $with_$1 != yes],
	    [cs_check_lib_paths=$with_$1],
	    [cs_check_lib_paths="| cs_lib_paths_default $3"])
	m4_foreach([cs_check_lib_alias], [$1, $10],
	    [_CS_CHECK_LIB_CREATE_FLAGS([cs_check_lib_flags],
		cs_check_lib_alias, [$cs_check_lib_paths])
	    ])

	CS_CHECK_BUILD([for lib$1], [cs_cv_lib$1], [$2], [$cs_check_lib_flags],
	    [$4], [], [], [$cs_ignore_cache], [$7], [$8], [$9])],
	[cs_cv_lib$1=no])

    cs_cv_with_$1="$with_$1"
    AS_IF([test "$cs_cv_lib$1" = yes], [$5], [$6])])



#------------------------------------------------------------------------------
# CS_CHECK_PKG_CONFIG
#	Check if the `pkg-config' command is available and reasonably recent.
#	This program acts as a central repository of build flags for various
#	packages.  For example, to determine the compiler flags for FreeType2
#	use, "pkg-config --cflags freetype2"; and "pkg-config --libs freetype2"
#	to determine the linker flags. If `pkg-config' is found and is
#	sufficiently recent, PKG_CONFIG is set and AC_SUBST() invoked.
#------------------------------------------------------------------------------
m4_define([CS_PKG_CONFIG_MIN], [0.9.0])
AC_DEFUN([CS_CHECK_PKG_CONFIG],
    [AS_IF([test "$cs_prog_pkg_config_checked" != yes],
	[CS_CHECK_TOOLS([PKG_CONFIG], [pkg-config])
	_CS_CHECK_PKG_CONFIG_PREPARE_PATH
	cs_prog_pkg_config_checked=yes])
    AS_IF([test -z "$cs_cv_prog_pkg_config_ok"],
	[AS_IF([test -n "$PKG_CONFIG"],
	    [AS_IF([$PKG_CONFIG --atleast-pkgconfig-version=CS_PKG_CONFIG_MIN],
		[cs_cv_prog_pkg_config_ok=yes],
		[cs_cv_prog_pkg_config_ok=no])],
	    [cs_cv_prog_pkg_config_ok=no])])])

AC_DEFUN([_CS_CHECK_PKG_CONFIG_PREPARE_PATH],
    [PKG_CONFIG_PATH="m4_foreach([cs_pkg_path], [cs_pkg_paths_default],
	[cs_pkg_path$PATH_SEPARATOR])$PKG_CONFIG_PATH"
    export PKG_CONFIG_PATH])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_PKG_CONFIG_FLAGS(VARIABLE, LIBRARY)
#	Helper macro for CS_CHECK_LIB_WITH().  Checks if `pkg-config' knows
#	about LIBRARY and, if so, appends a build tuple consisting of the
#	compiler and linker flags reported by `pkg-config' to the list of
#	tuples stored in the shell variable VARIABLE.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_PKG_CONFIG_FLAGS],
    [CS_CHECK_PKG_CONFIG
    AS_IF([test $cs_cv_prog_pkg_config_ok = yes],
	[AC_CACHE_CHECK([if $PKG_CONFIG recognizes $2], [_CS_CLPCF_CVAR([$2])],
	    [AS_IF([$PKG_CONFIG --exists $2],
		[_CS_CLPCF_CVAR([$2])=yes], [_CS_CLPCF_CVAR([$2])=no])])
	AS_IF([test $_CS_CLPCF_CVAR([$2]) = yes],
	    [_CS_CHECK_LIB_CONFIG_PROG_FLAGS([$1], [pkg_config_$2],
		[$PKG_CONFIG], [$2])])])])

AC_DEFUN([_CS_CLPCF_CVAR], [AS_TR_SH([cs_cv_prog_pkg_config_$1])])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CONFIG_FLAGS(VARIABLE, LIBRARY)
#	Helper macro for CS_CHECK_LIB_WITH().  Checks if `LIBRARY-config'
#	(i.e. `sdl-config') exists and, if so, appends a build tuple consisting
#	of the compiler and linker flags reported by `LIBRARY-config' to the
#	list of tuples stored in the shell variable VARIABLE.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CONFIG_FLAGS],
    [CS_CHECK_TOOLS(_CS_CLCF_SHVAR([$2]), [$2-config])
    AS_IF([test -n "$_CS_CLCF_SHVAR([$2])"],
	[AS_IF([test -z "$_CS_CLCF_CVAR([$2])"],
	    [AS_IF([$_CS_CLCF_SHVAR([$2]) --cflags --libs >/dev/null 2>&1],
		[_CS_CLCF_CVAR([$2])=yes], [_CS_CLCF_CVAR([$2])=no])])
	AS_IF([test $_CS_CLCF_CVAR([$2]) = yes],
	    [_CS_CHECK_LIB_CONFIG_PROG_FLAGS([$1], [config_$2],
		[$_CS_CLCF_SHVAR([$2])])])])])

AC_DEFUN([_CS_CLCF_CVAR], [AS_TR_SH([cs_cv_prog_config_$1_ok])])
AC_DEFUN([_CS_CLCF_SHVAR], [m4_toupper(AS_TR_SH([CONFIG_$1]))])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CONFIG_PROG_FLAGS(VARIABLE, TAG, CONFIG-PROGRAM, [ARGS])
#	Helper macro for _CS_CHECK_LIB_PKG_CONFIG_FLAGS() and
#	_CS_CHECK_LIB_CONFIG_FLAGS(). CONFIG-PROGRAM is a command which
#	responds to the --cflags and --libs options and returns suitable
#	compiler and linker flags for some package. ARGS, if supplied, is
#	passed to CONFIG-PROGRAM after the --cflags or --libs argument. The
#	results of the --cflags and --libs options are packed into a build
#	tuple and appended to the list of tuples stored in the shell variable
#	VARIABLE. TAG is used to compose the name of the cache variable. A good
#	choice for TAG is some unique combination of the library name and
#	configuration program.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CONFIG_PROG_FLAGS],
    [AS_IF([test -z "$_CS_CLCPF_CVAR([$2])"],
	[cs_check_lib_cflag=CS_RUN_PATH_NORMALIZE([$3 --cflags $4])
	cs_check_lib_lflag=''
	cs_check_lib_libs=CS_RUN_PATH_NORMALIZE([$3 --libs $4])
	_CS_CLCPF_CVAR([$2])=CS_CREATE_TUPLE(
	    [$cs_check_lib_cflag],
	    [$cs_check_lib_lflag],
	    [$cs_check_lib_libs])])
    $1="$$1 $_CS_CLCPF_CVAR([$2])"])

AC_DEFUN([_CS_CLCPF_CVAR], [AS_TR_SH([cs_cv_prog_$1_flags])])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CREATE_FLAGS(VARIABLE, LIBRARY, PATHS)
#	Helper macro for CS_CHECK_LIB_WITH().  Constructs a list of build
#	tuples suitable for CS_CHECK_BUILD() and appends the tuple list to the
#	shell variable VARIABLE.  LIBRARY and PATHS have the same meanings as
#	the like-named arguments of CS_CHECK_LIB_WITH().
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CREATE_FLAGS],
    [for cs_lib_item in $3
    do
	case $cs_lib_item in
	    *\|*) CS_SPLIT(
		    [$cs_lib_item], [cs_check_incdir,cs_check_libdir], [|])
		_CS_CHECK_LIB_CREATE_FLAG([$1],
		    [$cs_check_incdir], [$cs_check_libdir], [$2])
		;;
	    *)  _CS_CHECK_LIB_CREATE_FLAG([$1],
		    [$cs_lib_item/include], [$cs_lib_item/lib], [$2])
		_CS_CHECK_LIB_CREATE_FLAG(
		    [$1], [$cs_lib_item], [$cs_lib_item], [$2])
		;;
	esac
    done])



#------------------------------------------------------------------------------
# _CS_CHECK_LIB_CREATE_FLAG(VARIABLE, HEADER-DIR, LIBRARY-DIR, LIBRARY)
#	Helper macro for _CS_CHECK_LIB_CREATE_FLAGS().  Constructs build tuples
#	suitable for CS_CHECK_BUILD() for given header and library directories,
#	and appends the tuples to the shell variable VARIABLE. Synthesizes
#	tuples which check for LIBRARY as a MacOS/X framework, and a standard
#	link library.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_CHECK_LIB_CREATE_FLAG],
   [AS_IF([test -n "$2"], [cs_check_lib_cflag="-I$2"], [cs_check_lib_cflag=''])
    AS_IF([test -n "$3"], [cs_check_lib_lflag="-L$3"], [cs_check_lib_lflag=''])
    AS_IF([test -n "$4"],
	[cs_check_lib_libs="-l$4"
	cs_check_lib_framework="-framework $4"],
	[cs_check_lib_libs=''
	cs_check_lib_framework=''])
    $1="$$1
	CS_CREATE_TUPLE(
	    [$cs_check_lib_cflag],
	    [$cs_check_lib_lflag],
	    [$cs_check_lib_framework])
	CS_CREATE_TUPLE(
	    [$cs_check_lib_cflag],
	    [$cs_check_lib_lflag],
	    [$cs_check_lib_libs])"])
# checklibtool.m4                                              -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_LIBTOOL
#	Find and identify the various implementations of libtool.  In
#	particular, this macro is aware of GNU libtool and Apple's libtool
#	(which serves a completely different purpose).  On MacOS/X, GNU libtool
#	is typically named glibtool, however a user might also use Fink to
#	install the unadorned libtool; and the Fink-installed version might
#	shadow Apple's own libtool if it appears in the PATH before the Apple
#	tool. This macro jumps through the necessary hoops to distinguish and
#	locate the various implementations. Sets the shell variable LIBTOOL to
#	the located GNU libtool (if any), and APPLE_LIBTOOL to the located
#	Apple libtool. Invokes AC_SUBST() for LIBTOOL and APPLE_LIBTOOL.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIBTOOL],
[# GNU: Search for libtool before glibtool since Fink version is likely newer.
m4_define([cs_lt_path_gnu],
    [/sw/bin$PATH_SEPARATOR/usr/local/bin$PATH_SEPARATOR$PATH])
AS_IF([test -z "$LIBTOOL"],
    [CS_CHECK_TOOLS([LIBTOOL_TEST], [libtool glibtool gnulibtool], [],
	[cs_lt_path_gnu])
    AS_IF([test -n "$LIBTOOL_TEST"],
	[CS_PATH_PROG([LIBTOOL_PATH], [$LIBTOOL_TEST], [], [cs_lt_path_gnu])
	CS_LIBTOOL_CLASSIFY([$LIBTOOL_PATH],
	    [LIBTOOL="$LIBTOOL_PATH"],
	    [AS_IF([test -z "$APPLE_LIBTOOL"], [APPLE_LIBTOOL="$LIBTOOL_PATH"])
	    CS_CHECK_TOOLS([LIBTOOL], [glibtool gnulibtool])])])])
AC_SUBST([LIBTOOL])

# Apple: Ensure that Apple libtool will be found before GNU libtool from Fink.
m4_define([cs_lt_path_apple],[/bin$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR$PATH])
AS_IF([test -z "$APPLE_LIBTOOL"],
    [CS_PATH_PROG([CS_LT_APPLE], [libtool], [], [cs_lt_path_apple])
    CS_LIBTOOL_CLASSIFY([$CS_LT_APPLE], [],
	[APPLE_LIBTOOL="$CS_LT_APPLE"])])
AC_SUBST([APPLE_LIBTOOL])])

AC_DEFUN([CS_LIBTOOL_CLASSIFY],
    [AS_IF([test -n "$1"],
	[AC_MSG_CHECKING([classification of $1])
	CS_LIBTOOL_GNU_IFELSE([$1],
	    [AC_MSG_RESULT([gnu])
	    $2],
	    [AC_MSG_RESULT([apple])
	    $3])])])

AC_DEFUN([CS_LIBTOOL_GNU_IFELSE],
    [AS_IF([AC_RUN_LOG([$1 --version 1>&2])], [$2], [$3])])
#==============================================================================
# Copyright (C)2003-2006 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_OPENGL
#	Check for OpenGL.
#
# IMPLEMENTATION NOTES
#
# Some Mesa installations require pthread, so pthread flags are employed if
# available.
#
# The check for opengl32 needs to precede other checks because Cygwin users
# often have Mesa installed, and Mesa's OpenGL library is compiled without the
# __stdcall flags which results in link errors, whereas Microsoft's native
# opengl32 works fine.  Conversely, some Unix implementations have Wine
# installed (Windows emulation layer) which includes an opengl32.so library.
# We need to avoid detection of this library on Unix since it would cause an
# undesirable dependence upon Wine.
#
# Many OpenGL libraries on Unix already contain GLX, so there is no separate
# GLX library, thus we first check for GLX using the discovered OpenGL library
# before attempting to locate a separate GLX-specific library.  
#
# On MacOS/X, some users have XFree86 installed which creates a link from
# /usr/include/GL to /usr/X11R6/include/GL.  We want to ignore this directory
# and instead check for Apple's OpenGL.framework, if we are not cross-building
# for Darwin.  We accomplish this by placing the OpenGL.framework test ahead of
# the other tests.
#
# At least one user (Jorrit) has a strange installation in which inclusion of
# <windows.h> fails if an int32 is not present, thus we must take this into
# account.
#------------------------------------------------------------------------------
m4_define([cs_define_int32],
    [[#if !HAVE_TYPE_INT32
    typedef long int32;
    #endif
    ]])

# CS_GL_INCLUDE(CPP-MACRO,FALLBACK,HEADER)
AC_DEFUN([CS_GL_INCLUDE],
    [[#if HAVE_WINDOWS_H
    #if !HAVE_TYPE_INT32
    typedef long int32;
    #endif
    #include <windows.h>
    #endif
    #ifndef CS_HEADER_GLOBAL
    #define CS_HEADER_GLOBAL(X,Y) CS_HEADER_GLOBAL_COMPOSE(X,Y)
    #define CS_HEADER_GLOBAL_COMPOSE(X,Y) <X/Y>
    #endif
    #ifdef $1
    #include CS_HEADER_GLOBAL($1,$3)
    #else
    #include <$2/$3>
    #endif]])

AC_DEFUN([CS_CHECK_OPENGL],
    [AC_REQUIRE([CS_CHECK_HOST])
    AC_REQUIRE([CS_CHECK_COMMON_LIBS])
    AC_REQUIRE([CS_CHECK_PTHREAD])
    AC_REQUIRE([AC_PATH_X])
    AC_REQUIRE([AC_PATH_XTRA])
    AC_CHECK_TYPE([int32], [AC_DEFINE([HAVE_TYPE_INT32], [], 
	[Whether the int32 type is available])], [])
    AC_CHECK_HEADERS([windows.h], [], [], [cs_define_int32])
    
    # Apply plaform-specific flags if necessary.
    cs_gl_plat_cflags=''
    cs_gl_plat_lflags=''
    cs_gl_plat_libs=''
    AS_IF([test -n "$cs_cv_libm_cflags$cs_cv_libm_lflags$cs_cv_libm_libs"],
	[cs_gl_plat_cflags="$cs_cv_libm_cflags $cs_gl_plat_cflags"
	cs_gl_plat_lflags="$cs_cv_libm_lflags $cs_gl_plat_lflags"
	cs_gl_plat_libs="$cs_cv_libm_libs $cs_gl_plat_libs"])
    AS_IF([test $cs_cv_sys_pthread = yes],
	[cs_gl_plat_cflags="$cs_cv_sys_pthread_cflags $cs_gl_plat_cflags"
	cs_gl_plat_lflags="$cs_cv_sys_pthread_lflags $cs_gl_plat_lflags"
	cs_gl_plat_libs="$cs_cv_sys_pthread_libs $cs_gl_plat_libs"])
    AS_IF([test "$no_x" != yes],
	[cs_gl_plat_cflags="$X_CFLAGS $cs_gl_plat_cflags"
	cs_gl_plat_lflags="$cs_gl_plat_lflags"
	cs_gl_plat_libs="
	    $X_PRE_LIBS $X_LIBS -lX11 -lXext $X_EXTRA_LIBS $cs_gl_plat_libs"])

    # Mesa requested?
    AC_ARG_WITH([mesa], [AC_HELP_STRING([--with-mesa],
	    [use Mesa OpenGL library if available (default YES)])],
	    [], [with_mesa=yes])
    
    AS_IF([test $with_mesa != no],
	[cs_mesa_gl=CS_CREATE_TUPLE([],[],[-lMesaGL])])
    
    # MacOS/X or Darwin?
    AS_IF([test "x$cs_host_macosx" = "xyes"],
	[cs_osx_gl=CS_CREATE_TUPLE([-DCS_OPENGL_PATH=OpenGL],[],[-framework OpenGL])])
    AS_IF([test "x$cs_host_macosx" = "xyes"],
          [cs_gl_plat_lflags="$cs_plat_lflags -Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"])

    # Windows?
    AS_IF([test $cs_host_family = windows],
	[cs_win32_gl=CS_CREATE_TUPLE([],[],[-lopengl32])])
    
    # Check for OpenGL.
    CS_CHECK_BUILD([for OpenGL], [cs_cv_libgl],
	[AC_LANG_PROGRAM([CS_GL_INCLUDE([CS_OPENGL_PATH],[GL],[gl.h])],[glEnd()])],
	[$cs_win32_gl \
	$cs_osx_gl \
	CS_CREATE_TUPLE([],[],[-lGL]) \
	CS_CREATE_TUPLE([],[],[-lgl]) \
	$cs_mesa_gl], [],
	[CS_EMIT_BUILD_RESULT([cs_cv_libgl], [GL])], [], [],
	[$cs_gl_plat_cflags], [$cs_gl_plat_lflags], [$cs_gl_plat_libs])])


#------------------------------------------------------------------------------
# CS_CHECK_GLU
#	Check for GLU.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_GLU],
    [AC_REQUIRE([CS_CHECK_OPENGL])
    AS_IF([test $cs_cv_libgl = yes],
        [AS_IF([test $with_mesa != no],
	    [cs_mesa_glu=CS_CREATE_TUPLE([],[],[-lMesaGLU])])
	
	# MacOS/X or Darwin?
	AS_IF([test "x$cs_host_macosx" = "xyes"],
	    [cs_osx_glu=CS_CREATE_TUPLE([-DCS_GLU_PATH=OpenGL],[],[-framework OpenGL])])
	
	# Windows?
	AS_IF([test $cs_host_family = windows],
	    [cs_win32_glu=CS_CREATE_TUPLE([],[],[-lglu32])])
    
	# Check for GLU.
	CS_CHECK_BUILD([for GLU], [cs_cv_libglu],
	    [AC_LANG_PROGRAM(
		[CS_GL_INCLUDE([CS_GLU_PATH],[GL],[glu.h])], [gluNewQuadric()])],
	    [$cs_osx_glu \
	    CS_CREATE_TUPLE() \
	    $cs_win32_glu \
	    CS_CREATE_TUPLE([],[],[-lGLU]) \
	    CS_CREATE_TUPLE([],[],[-lglu]) \
	    $cs_mesa_glu], [],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libglu], [GLU])], [], [],
	    [$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])])])


#------------------------------------------------------------------------------
# CS_CHECK_GLX
#	Check for GLX.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_GLX],
    [AC_REQUIRE([CS_CHECK_OPENGL])
    AS_IF([test $cs_cv_libgl = yes],
        [AS_IF([test $with_mesa != no],
            [cs_mesa_glx=CS_CREATE_TUPLE([],[],[-lMesaGLX])])
	
        # Check for GLX.
	AS_IF([test "$no_x" != yes],
	    [CS_CHECK_BUILD([for GLX], [cs_cv_libglx],
		[AC_LANG_PROGRAM([[#include <GL/glx.h>]], [glXWaitGL()])],
		[CS_CREATE_TUPLE() \
		CS_CREATE_TUPLE([],[],[-lGLX]) \
		CS_CREATE_TUPLE([],[],[-lglx]) \
		$cs_mesa_glx], [],
		[CS_EMIT_BUILD_RESULT([cs_cv_libglx], [GLX])], [], [],
		[$cs_cv_libgl_cflags], [$cs_cv_libgl_lflags], [$cs_cv_libgl_libs])])])])
    

#------------------------------------------------------------------------------
# CS_CHECK_GLXEXT([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#	Check for GLX extensions.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_GLXEXT],
    [AC_REQUIRE([CS_CHECK_GLX])
    AS_IF([test x$cs_cv_libglx = "xyes"],
	[# Check for GLX extensions.
	CS_CHECK_BUILD([for GLX extensions], [cs_cv_libglx_extensions],
	    [AC_LANG_PROGRAM(
		[[#define GLX_GLXEXT_PROTOTYPES
		#include <GL/glx.h>]],
		[glXGetProcAddressARB(0)])],
	    [CS_CREATE_TUPLE(
		[$cs_cv_libglx_cflags],
		[$cs_cv_libglx_lflags],
		[$cs_cv_libglx_libs])],
	    [], [$1], [$2])])])



#------------------------------------------------------------------------------
# CS_CHECK_GLUT
#	Check for GLUT.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_GLUT],
    [AC_REQUIRE([CS_CHECK_GLU])
    AS_IF([test x$cs_cv_libglu = "xyes"],
        [# MacOS/X or Darwin?
	AS_IF([test "x$cs_host_macosx" = "xyes"],
	    [cs_osx_glut=CS_CREATE_TUPLE([-DCS_GLUT_PATH=GLUT],[],[-framework GLUT])])
	
	# Windows?
	AS_IF([test $cs_host_family = windows],
	    [cs_win32_glut=CS_CREATE_TUPLE([],[],[-lglut32])])
    
	# Check for GLUT.
	CS_CHECK_BUILD([for GLUT], [cs_cv_libglut],
	    [AC_LANG_PROGRAM(
		[CS_GL_INCLUDE([CS_GLUT_PATH],[GL],[glut.h])], [glutSwapBuffers()])],
	    [$cs_osx_glut \
	    CS_CREATE_TUPLE() \
	    $cs_win32_glut \
	    CS_CREATE_TUPLE([],[],[-lGLUT]) \
	    CS_CREATE_TUPLE([],[],[-lglut])], [],
	    [CS_EMIT_BUILD_RESULT([cs_cv_libglut], [GLUT])], [], [],
	    [$cs_cv_libgl_cflags $cs_cv_libglu_cflags], 
	    [$cs_cv_libgl_lflags $cs_cv_libglu_lflags], 
	    [$cs_cv_libgl_libs   $cs_cv_libglu_libs])])])

# checkpic.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_COMPILER_PIC([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                 [ACTION-IF-NOT-FOUND])
#	Check if compiler can be instructed to produce
#	position-independent-code (PIC).  This feature is required by some
#	platforms when building plugin modules and shared libraries.  If
#	LANGUAGE is not provided, then `C' is assumed (other options include
#	`C++').  If CACHE-VAR is not provided, then it defaults to the name
#	"cs_cv_prog_compiler_pic".  If a PIC-enabling option (such as `-fPIC')
#	is discovered, then it is assigned to CACHE-VAR and ACTION-IF-FOUND is
#	invoked; otherwise the empty string is assigned to CACHE-VAR and
#	ACTION-IF-NOT-FOUND is invoked.
#
# IMPLEMENTATION NOTES
#
#	On some platforms (such as Windows), the -fPIC option is superfluous
#	and emits a warning "-fPIC ignored for target (all code is position
#	independent)", despite the fact that the compiler accepts the option
#	and returns a success code. We want to re-interpret the warning as a
#	failure in order to avoid unnecessary compiler diagnostics in case the
#	client inserts the result of this check into CFLAGS, for instance. We
#	do so by attempting to promote warnings to errors using the result of
#	CS_COMPILER_ERRORS(). As an extra safe-guard, we also scan the compiler
#	output for an appropriate diagnostic because some gcc warnings fail to
#	promote to error status despite use of -Werror.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_PIC],
    [CS_COMPILER_ERRORS([$1],
	[m4_default([$2_werror],[cs_cv_prog_compiler_pic_werror])])
    CS_CHECK_BUILD_FLAGS(
	[how to enable m4_default([$1],[C]) PIC generation],
	[m4_default([$2],[cs_cv_prog_compiler_pic])],
	[CS_CREATE_TUPLE([-fPIC])], [$1], [$3], [$4],
	[m4_default([$$2_werror],[$cs_cv_prog_compiler_pic_werror])], [], [],
	[fPIC])])

# Backward-compatiblity alias.
AC_DEFUN([CS_CHECK_COMPILER_PIC], [CS_COMPILER_PIC([$1],[$2],[$3],[$4])])
# checkprog.m4                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# cs_bin_paths_default
#	Comma delimited list of additional directories in which tools and
#	commands might be found.
#
# Present Cases:
#	/usr/local/bin -- Although a common location for executables, it is
#		now-and-then absent from the default PATH setting.
#	/sw/bin -- Fink, the MacOS/X manager of Unix packages, installs
#		executables here.
#------------------------------------------------------------------------------
m4_define([cs_bin_paths_default], [/usr/local/bin, /sw/bin])


#------------------------------------------------------------------------------
# CS_CHECK_PROG(VARIABLE, PROGRAM, VALUE-IF-FOUND, [VALUE-IF-NOT-FOUND],
#		[PATH], [REJECT])
#	Simple wrapper for AC_CHECK_PROG() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PROG],
    [_CS_PROG_PATH_PREPARE
    AC_CHECK_PROG([$1], [$2], [$3], [$4],
	m4_ifval([$5], [_CS_PROG_CLIENT_PATH([$5])]), [$6])])


#------------------------------------------------------------------------------
# CS_CHECK_PROGS(VARIABLE, PROGRAMS, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_CHECK_PROGS() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PROGS],
    [_CS_PROG_PATH_PREPARE
    AC_CHECK_PROGS([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# CS_CHECK_TOOL(VARIABLE, TOOL, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_CHECK_TOOL() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_TOOL],
    [_CS_PROG_PATH_PREPARE
    AC_CHECK_TOOL([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# CS_CHECK_TOOLS(VARIABLE, TOOLS, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_CHECK_TOOLS() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_TOOLS],
    [_CS_PROG_PATH_PREPARE
    AC_CHECK_TOOLS([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# CS_PATH_PROG(VARIABLE, PROGRAM, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_PATH_PROG() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_PROG],
    [_CS_PROG_PATH_PREPARE
    AC_PATH_PROG([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# CS_PATH_PROGS(VARIABLE, PROGRAMS, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_PATH_PROGS() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_PROGS],
    [_CS_PROG_PATH_PREPARE
    AC_PATH_PROGS([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# CS_PATH_TOOL(VARIABLE, TOOL, [VALUE-IF-NOT-FOUND], [PATH])
#	Simple wrapper for AC_PATH_TOOL() which ensures that the search path
#	is augmented by the directories mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_TOOL],
    [_CS_PROG_PATH_PREPARE
    AC_PATH_TOOL([$1], [$2], [$3],
	m4_ifval([$4], [_CS_PROG_CLIENT_PATH([$4])]))])


#------------------------------------------------------------------------------
# _CS_PROG_PATH_PREPARE
#	Ensure that the PATH environment variable mentions the set of
#	directories listed in cs_bin_paths_default. These directories may not
#	appear by default in the typical PATH, yet they might be common
#	locations for tools and commands.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_PROG_PATH_PREPARE],
    [AS_REQUIRE([_AS_PATH_SEPARATOR_PREPARE])
    AS_IF([test "$cs_prog_path_prepared" != yes],
	[cs_prog_path_prepared=yes
	PATH="$PATH[]m4_foreach([cs_bin_path], [cs_bin_paths_default],
	[$PATH_SEPARATOR[]cs_bin_path])"
	export PATH])])


#------------------------------------------------------------------------------
# _CS_PROG_CLIENT_PATH(CLIENT-PATH)
#	Given a client-supplied replacement for PATH, augment the list by
#	appending the locations mentioned in cs_bin_paths_default.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_PROG_CLIENT_PATH],
    [AS_REQUIRE([_AS_PATH_SEPARATOR_PREPARE])dnl
    $1[]m4_foreach([cs_bin_path], [cs_bin_paths_default],
	[$PATH_SEPARATOR[]cs_bin_path])])
# checkpthread.m4                                              -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_PTHREAD([REJECT-MASK])
#	Check for pthread.  Also check if the pthread implementation supports
#	the recursive and timed mutex extensions. (Timed mutexes are needed for
#	the NPTL: New Posix Thread Library on GNU/Linux if the mutex is going
#	to be used with any of the timed condition-wait functions.) The shell
#	variable cs_cv_sys_pthread is set to "yes" if pthread is available,
#	else "no". If available, then the variables cs_cv_sys_pthread_cflags,
#	cs_cv_sys_pthread_lflags, and cs_cv_sys_pthread_libs are set. (As a
#	convenience, these variables can be emitted to an output file with
#	CS_EMIT_BUILD_RESULT() by passing "cs_cv_sys_pthread" as its CACHE-VAR
#	argument.)  If the recursive mutex extension is supported, then
#	cs_cv_sys_pthread_mutex_recursive will be set with the literal name of
#	the constant which must be passed to pthread_mutexattr_settype() to
#	enable this feature. The constant name will be typically
#	PTHREAD_MUTEX_RECURSIVE or PTHREAD_MUTEX_RECURSIVE_NP. If the recursive
#	mutex extension is not available, then
#	cs_cv_sys_pthread_mutex_recursive will be set to "no".  If the timed
#	mutex extension is supported, then cs_cv_sys_pthread_mutex_timed will
#	be set with the literal name of the constant which must be passed to
#	pthread_mutexattr_settype() to enable this feature. The constant name
#	will be typically PTHREAD_MUTEX_TIMED or PTHREAD_MUTEX_TIMED_NP. If the
#	timed mutex extension is not available, then
#	cs_cv_sys_pthread_mutex_timed will be set to "no".  REJECT-MASK can be
#	used to limit the platforms on which the pthread test is performed. It
#	is compared against $host_os; matches are rejected. If omitted, then
#	the test is performed on all platforms. Examples: To avoid testing on
#	Cygwin, use "cygwin*"; to avoid testing on Cygwin and AIX, use
#	"cygwin*|aix*".
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PTHREAD],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_os in
	m4_ifval([$1],
	[$1)
	    cs_cv_sys_pthread=no
	    ;;
	])
	*)
	    CS_CHECK_BUILD([for pthread], [cs_cv_sys_pthread],
		[AC_LANG_PROGRAM(
		    [[#include <pthread.h>
		    #include <semaphore.h>
		    void* worker(void* p) { (void)p; return p; }]],
		    [pthread_t tid;
		    sem_t sem;
		    pthread_create(&tid, 0, worker, 0);
		    sem_init(&sem, 0, 0);
		    sem_destroy(&sem);])],
		[cs_pthread_flags])
	    ;;
    esac
    _CS_CHECK_MUTEX_FEATURE([PTHREAD_MUTEX_RECURSIVE],
	[cs_cv_sys_pthread_mutex_recursive], [for pthread recursive mutexes])])

# _CS_CHECK_MUTEX_FEATURE(FEATURE, CACHE-VAR, MESSAGE)
AC_DEFUN([_CS_CHECK_MUTEX_FEATURE],
    [AS_IF([test $cs_cv_sys_pthread = yes],
	[AC_CACHE_CHECK([$3], [$2],
	    [CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [[#include <pthread.h>]],
		    [pthread_mutexattr_t attr;
		    pthread_mutexattr_settype(&attr, CS_MUTEX_FEATURE);])],
		[CS_CREATE_TUPLE([-DCS_MUTEX_FEATURE=$1]) \
		CS_CREATE_TUPLE([-DCS_MUTEX_FEATURE=$1_NP])],
		[],
		[$2=`echo $cs_build_cflags | sed 's/.*\($1_*N*P*\).*/\1/'`],
		[$2=no],
		[$cs_cv_sys_pthread_cflags -D_GNU_SOURCE],
		[$cs_cv_sys_pthread_lflags],
		[$cs_cv_sys_pthread_libs])])],
	[$2=no])])

#------------------------------------------------------------------------------
# CS_CHECK_PTHREAD_ATFORK(CACHE-VAR)
#     Checks whether the pthread library contains pthread_atfork(). Sets
#     CACHE-VAR to "yes" or "no", according to the test result.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_PTHREAD_ATFORK],
    [AS_IF([test $cs_cv_sys_pthread = yes],
	[AC_CACHE_CHECK([for pthread_atfork support], [$1],
	    [CS_BUILD_IFELSE(
		[AC_LANG_PROGRAM(
		    [[#include <pthread.h>]],
		    [pthread_atfork (0, 0, 0);])],
		[], [],
		[$1=yes], [$1=no],
		[$cs_cv_sys_pthread_cflags -D_GNU_SOURCE],
		[$cs_cv_sys_pthread_lflags],
		[$cs_cv_sys_pthread_libs])])],
	[$1=no])])

m4_define([cs_pthread_flags],
    [CS_CREATE_TUPLE() \
    CS_CREATE_TUPLE([], [], [-lpthread]) \
    CS_CREATE_TUPLE([], [], [-lpthread -lrt]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], []) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lpthread]) \
    CS_CREATE_TUPLE([-pthread], [-pthread], [-lc_r])])
# checktt2.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004,2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_TEMPLATE_TOOLKIT2([EMITTER])
#	Check if Template Toolkit 2 (http://www.tt2.org/) is available. The
#	shell variable cs_cv_perl_tt2 is set to "yes" if the package is
#	discovered, else "no". Also sets the shell variable TTREE to the name
#	path of the 'ttree' utility program and invokes AC_SUBST().  If EMITTER
#	is provided and the package was discovered, then
#	CS_EMIT_BUILD_PROPERTY() is invoked with EMITTER in order to record the
#	value of the TTREE variable in an output file. As a convenience, if
#	EMITTER is the literal value "emit" or "yes", then
#	CS_EMIT_BUILD_RESULT()'s default emitter will be used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_TEMPLATE_TOOLKIT2],
    [CS_CHECK_PROGS([PERL], [perl5 perl])
    AS_IF([test -n "$PERL"],
	[AC_CACHE_CHECK([for TemplateToolkit], [cs_cv_perl_tt2],
	    [AS_IF([AC_RUN_LOG(
		[$PERL -M'Template 2.11' -MTemplate::Plugin -e 0 1>&2])],
		[cs_cv_perl_tt2=yes],
		[cs_cv_perl_tt2=no])])
	CS_PATH_PROGS([TTREE], [ttree])
	AS_IF([test $cs_cv_perl_tt2 = yes && test -n "$TTREE"],
	    [CS_EMIT_BUILD_PROPERTY([TTREE], [$TTREE], [], [],
		CS_EMITTER_OPTIONAL([$1]))])])])
# compiler.m4                                                  -*- Autoconf -*-
#=============================================================================
# Copyright (C)2003 by Matze Braun <matze@braunis.de>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#=============================================================================

#-----------------------------------------------------------------------------
# Detection of C and C++ compilers and setting flags
#
# CS_PROG_CC
#       Detects the C compiler.  Also takes care of the CFLAGS, CPPFLAGS and CC
#       environment variables.  This will filter out all -g and -O from the
#       CFLAGS variable because Autoconf's -g and -O defaults are not always
#       desired.  This will also set the CMD.CC and COMPILER.CFLAGS variables
#       in Jamconfig
# CS_PROG_CXX
#       Detects the C++ compiler.  Also takes care of the CXXFLAGS, CPPFLAGS
#       and CXX environment variables.  This will filter out all -g and -O from
#       the CXXFLAGS variable because Autoconf's -g and -O defaults are not
#       always desired.  This will also set the CMD.C++ and COMPILER.C++FLAGS
#       variables in Jamconfig
# CS_PROG_LINK
#	Tries to determine a linker.  This is done by checking if a C++ or
#       Objecctive-C++ compiler is available in which case it is used for
#       linking; otherwise the C or Objective-C compiler is used.  This also
#       sets the CMD.LINK and COMPILER.LFLAGS variables in Jamconfig and
#       respects the LDFLAGS environment variable.  Finally, checks if linker
#	recognizes -shared and sets PLUGIN.LFLAGS; and checks if linker
#	recognizes -soname and sets PLUGIN.LFLAGS.USE_SONAME to "yes".
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PROG_CC],[
    CFLAGS="$CFLAGS" # Filter undesired flags
    AS_IF([test -n "$CC"],[
	CS_EMIT_BUILD_PROPERTY([CMD.CC], [$CC])
	CS_EMIT_BUILD_PROPERTY([COMPILER.CFLAGS], [$CPPFLAGS $CFLAGS], [+])
	
	# Check if compiler recognizes -pipe directive.
	CS_EMIT_BUILD_FLAGS([if $CC accepts -pipe], [cs_cv_prog_cc_pipe],
	  [CS_CREATE_TUPLE([-pipe])], [C], [COMPILER.CFLAGS], [+])
    ])
])

AC_DEFUN([CS_PROG_CXX],[
    CXXFLAGS="$CXXFLAGS" # Filter undesired flags
    AS_IF([test -n "$CXX"],[
	CS_EMIT_BUILD_PROPERTY([CMD.C++], [$CXX])

	CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS], [$CPPFLAGS $CXXFLAGS], [+])

        # Check if compiler can be instructed to produce position-independent-code
        # (PIC).  This feature is required by some platforms when building plugin
        # modules and shared libraries.
	CS_COMPILER_PIC([C++], [cs_cv_prog_cxx_pic],
	    [CS_EMIT_BUILD_PROPERTY([COMPILER.C++FLAGS.PIC],
		[$cs_cv_prog_cxx_pic])])
    ])
])

AC_DEFUN([CS_PROG_LINK],[
    AC_REQUIRE([CS_PROG_CXX])
    AS_IF([test -n "$CXX"],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.C++)])])],
	[CS_EMIT_BUILD_PROPERTY([CMD.LINK], [AS_ESCAPE([$(CMD.CC)])])])

    CS_EMIT_BUILD_PROPERTY([COMPILER.LFLAGS], [$LDFLAGS], [+])

    # Check if compiler/linker recognizes -shared directive which is needed for
    # linking plugin modules.  Unfortunately, the Apple compiler (and possibly
    # others) requires extra effort.  Even though the compiler does not recognize
    # the -shared option, it nevertheless returns a "success" result after emitting
    # the warning "unrecognized option `-shared'".  Worse, even -Werror fails to
    # promote the warning to an error, so we must instead scan the compiler's
    # output for an appropriate diagnostic.
    CS_CHECK_BUILD_FLAGS([if -shared is accepted], [cs_cv_prog_link_shared],
	[CS_CREATE_TUPLE([-shared $cs_cv_prog_cxx_pic])], [C++],
	[CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS], [-shared], [+])], [],
	[], [], [], [shared])

    # Check if linker recognizes -soname which is used to assign a name internally
    # to plugin modules.
    CS_CHECK_BUILD([if -soname is accepted], [cs_cv_prog_link_soname], [],
	[CS_CREATE_TUPLE([-Wl,-soname,foobar])], [C++],
	[CS_EMIT_BUILD_PROPERTY([PLUGIN.LFLAGS.USE_SONAME], [yes])])
])
#------------------------------------------------------------------------------
# Determine host platform.  Recognized families: Unix, Windows, MacOS/X.
# Orginial Macros Copyright (C)2003 Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Determine host CPU.
#
# CS_CHECK_HOST_CPU
#       Set the shell variable cs_host_cpu to a normalized form of the CPU name
#       returned by config.guess/config.sub.  Typically, Crystal Space's
#       conception of CPU name is the same as that returned by
#       config.guess/config.sub, but there may be exceptions as seen in the
#       `case' statement.  Also takes the normalized name, uppercases it to
#       form a name suitable for the C preprocessor.  Additionally sets the
#       TARGET.PROCESSOR Jamconfig property.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST_CPU],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    case $host_cpu in
        [[Ii][3-9]86*|[Xx]86*]) cs_host_cpu=x86 ;;
        *) cs_host_cpu=$host_cpu ;;
    esac
    cs_host_cpu_normalized="AS_TR_CPP([$cs_host_cpu])"
    CS_JAMCONFIG_PROPERTY([TARGET.PROCESSOR], [$cs_host_cpu_normalized])
    ])


#------------------------------------------------------------------------------
# CS_CHECK_HOST
#       Sets the shell variables cs_host_target cs_host_family,
#       cs_host_os_normalized, and cs_host_os_normalized_uc.  Emits appropriate
#       CS_PLATFORM_UNIX, CS_PLATFORM_WIN32, CS_PLATFORM_MACOSX via
#       AC_DEFINE(), and TARGET.OS and TARGET.OS.NORMALIZED to Jamconfig.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_HOST],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    CS_CHECK_HOST_CPU
    cs_host_os_normalized=''
    case $host_os in
        mingw*|cygwin*)
            cs_host_target=win32gcc
            cs_host_family=windows
            ;;
        darwin*)
            _CS_CHECK_HOST_DARWIN
            ;;
        *)
            # Everything else is assumed to be Unix or Unix-like.
            cs_host_target=unix
            cs_host_family=unix
	    ;;
    esac

    case $cs_host_family in
	windows)
            AC_DEFINE([CS_PLATFORM_WIN32], [],
		[Define when compiling for Win32])
	    AS_IF([test -z "$cs_host_os_normalized"],
		[cs_host_os_normalized='Win32'])
	    ;;
	unix)
            AC_DEFINE([CS_PLATFORM_UNIX], [],
		[Define when compiling for Unix and Unix-like (i.e. MacOS/X)])
	    AS_IF([test -z "$cs_host_os_normalized"],
		[cs_host_os_normalized='Unix'])
	    ;;
    esac

    cs_host_os_normalized_uc="AS_TR_CPP([$cs_host_os_normalized])"
    CS_JAMCONFIG_PROPERTY([TARGET.OS], [$cs_host_os_normalized_uc])
    CS_JAMCONFIG_PROPERTY([TARGET.OS.NORMALIZED], [$cs_host_os_normalized])
])

AC_DEFUN([_CS_CHECK_HOST_DARWIN],
    [AC_REQUIRE([CS_PROG_CC])
    AC_REQUIRE([CS_PROG_CXX])

    # Both MacOS/X and Darwin are identified via $host_os as "darwin".  We need
    # a way to distinguish between the two.  If Carbon.h is present, then
    # assume MacOX/S; if not, assume Darwin.  If --with-x=yes was invoked, and
    # Carbon.h is present, then assume that user wants to cross-build for
    # Darwin even though build host is MacOS/X.
    # IMPLEMENTATION NOTE *1*
    # The QuickTime 7.0 installer removes <CarbonSound/CarbonSound.h>, which
    # causes #include <Carbon/Carbon.h> to fail unconditionally. Re-installing
    # the QuickTime SDK should restore the header, however not all developers
    # know to do this, so we work around the problem of the missing
    # CarbonSound.h by #defining __CARBONSOUND__ in the test in order to
    # prevent Carbon.h from attempting to #include the missing header.
    # IMPLEMENTATION NOTE *2*
    # At least one MacOS/X user switches between gcc 2.95 and gcc 3.3 with a
    # script which toggles the values of CC, CXX, and CPP.  Unfortunately, CPP
    # was being set to run the preprocessor directly ("cpp", for instance)
    # rather than running it via the compiler ("gcc -E", for instance).  The
    # problem with running the preprocessor directly is that __APPLE__ and
    # __GNUC__ are not defined, which causes the Carbon.h check to fail.  We
    # avoid this problem by supplying a non-empty fourth argument to
    # AC_CHECK_HEADER(), which causes it to test compile the header only (which
    # is a more robust test), rather than also testing it via the preprocessor.

    AC_DEFINE([__CARBONSOUND__], [],
	[Avoid problem caused by missing <Carbon/CarbonSound.h>])
    AC_CHECK_HEADER([Carbon/Carbon.h],
	[cs_host_macosx=yes], [cs_host_macosx=no], [/* force compile */])

    AS_IF([test $cs_host_macosx = yes],
	[AC_MSG_CHECKING([for --with-x])
	AS_IF([test "${with_x+set}" = set && test "$with_x" = "yes"],
	    [AC_MSG_RESULT([yes (assume Darwin)])
	    cs_host_macosx=no],
	    [AC_MSG_RESULT([no])])])

    AS_IF([test $cs_host_macosx = yes],
	[cs_host_target=macosx
	cs_host_family=unix
	cs_host_os_normalized='MacOS/X'
        AC_DEFINE([CS_PLATFORM_MACOSX], [],
	    [Define when compiling for MacOS/X])

	AC_CACHE_CHECK([for Objective-C compiler], [cs_cv_prog_objc],
	    [cs_cv_prog_objc="$CC"])
	CS_JAMCONFIG_PROPERTY([CMD.OBJC], [$cs_cv_prog_objc])
	AC_CACHE_CHECK([for Objective-C++ compiler], [cs_cv_prog_objcxx],
	    [cs_cv_prog_objcxx="$CXX"])
	CS_JAMCONFIG_PROPERTY([CMD.OBJC++], [$cs_cv_prog_objcxx])],

	[cs_host_target=unix
	cs_host_family=unix])])
# diagnose.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_MSG_ERROR(ERROR-DESCRIPTION, [EXIT-STATUS])
#	A convenience wrapper for AC_MSG_ERROR() which invokes AC_CACHE_SAVE()
#	before aborting the script.  Saving the cache should make subsequent
#	re-invocations of the configure script faster once the user has
#	corrected the problem(s) which caused the failure.
#------------------------------------------------------------------------------
AC_DEFUN([CS_MSG_ERROR],
    [AC_CACHE_SAVE
    AC_MSG_ERROR([$1], [$2])])
# embed.m4                                                     -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003,2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_META_INFO_EMBED([EMITTER], [GPL-OKAY])
#	Determine if plugin meta-information should be embedded or if it should
#	exist in a stand-alone .csplugin file, and check if necessary tools and
#	libraries are present.  Sets the shell variable
#	enable_meta_info_embedding to "yes" if the user requested embedding or
#	if it was enabled by default; otherwise sets it to "no".
#
#	If EMITTER is provided, then a subset of the following variables
#	(depending upon platform and availability) are recorded by invoking
#	CS_EMIT_BUILD_PROPERTY() with EMITTER.  As a convenience, if EMITTER is
#	the literal value "emit" or "yes", then CS_EMIT_BUILD_RESULT()'s
#	default emitter will be used.
#
#	EMBED_META := yes or no
#	EMBED_META.CFLAGS := compiler flags
#	EMBED_META.LFLAGS := linker flags
#	CMD.WINDRES := windres.exe
#	OBJCOPY.AVAILABLE := yes or no
#	CMD.OBJCOPY := objcopy.exe
#	LIBBFD.AVAILABLE := yes or no
#	LIBBFD.CFLAGS := libbfd compiler flags
#	LIBBFD.LFLAGS := libbfd linker flags
#	ELF.AVAILABLE := yes or no
#
#	In general, clients need only concern themselves with the various
#	EMBED_META-related variables. For building plugin modules, utilize
#	EMBED_META.CFLAGS when compiling, and EMBED_META.LFLAGS when linking.
#
#	On Unix, when CS' own ELF metadata reader can't be used (because the
#	necessary header file elf.h was not found) embedding is accomplished
#	via libbfd, which carries a GPL license. Projects which carry licenses
#	not compatible with GPL should consider carefully before enabling
#	embedding on Unix. If your project is GPL-compatible, then set GPL-OKAY
#	to "yes". This will indicate that it is safe to use libbfd if the ELF
#	reader can not be used.  If your project is not GPL-compatible, then
#	set it to "no" in order to disable embedding on Unix if the ELF reader
#	is not usable. (The user can still manually override the setting via
#	the --enable-meta-info-embedding option.)
#
# IMPLEMENTATION NOTES
#
#	Recent versions of Mingw supply libbfd and libiberty.  Since Crystal
#	Space uses native Win32 API for meta-information embedding on Windows,
#	we do not require these libraries on Windows.  More importantly, users
#	do not want to see these GPL-licensed libraries appear in the link
#	statement for plugin modules, thus we explicitly disable the libbfd
#	test on Windows.
#------------------------------------------------------------------------------
AC_DEFUN([CS_META_INFO_EMBED],
    [AC_REQUIRE([AC_CANONICAL_HOST])
    _CS_META_INFO_EMBED_ENABLE([$1], [$2])
    AS_IF([test $enable_meta_info_embedding = yes],
        [_CS_META_INFO_EMBED_TOOLS([$1])
        AS_IF([test $cs_header_elf_h = yes],
	    [CS_EMIT_BUILD_PROPERTY([ELF.AVAILABLE], [yes], [], [],
		CS_EMITTER_OPTIONAL([$1]))],
            [case $host_os in
	        mingw*|cygwin*) ;;
		*)
		    CS_CHECK_LIBBFD([$1],
			[CS_EMIT_BUILD_PROPERTY([EMBED_META.CFLAGS],
			    [$cs_cv_libbfd_ok_cflags], [+], [],
			    CS_EMITTER_OPTIONAL([$1]))
			CS_EMIT_BUILD_PROPERTY([EMBED_META.LFLAGS],
			    [$cs_cv_libbfd_ok_lflags $cs_cv_libbfd_ok_libs],
			    [+], [], CS_EMITTER_OPTIONAL([$1]))])
		    ;;
	    esac])])])


#------------------------------------------------------------------------------
# _CS_META_INFO_EMBED_ENABLE([EMITTER], [GPL-OKAY])
#	Helper for CS_META_INFO_EMBED which adds an
#	--enable-meta-info-embedding option to the configure script allowing
#	the user to control embedding.  Sets the shell variable
#	enable_meta_info_embedding to yes or no.
#
# IMPLEMENTATION NOTES
#
#	On Unix, embedding is enabled by default if elf.h is found and disabled
#	by default unless overridden via GPL-OKAY because libbfd carries a GPL
#	license which may be incompatible with a project's own license (such as
#	LGPL).
#------------------------------------------------------------------------------
AC_DEFUN([_CS_META_INFO_EMBED_ENABLE],
    [AC_REQUIRE([CS_CHECK_HOST])
    AC_CHECK_HEADERS([elf.h], [cs_header_elf_h=yes], [cs_header_elf_h=no])
    AC_MSG_CHECKING([whether to embed plugin meta-information])
    case $cs_host_target in
	unix) AS_IF([test $cs_header_elf_h = yes],
              [cs_embed_meta_info_default=yes],
              [cs_embed_meta_info_default=m4_ifval([$2],[$2],[no])]) ;;
	*) cs_embed_meta_info_default=yes ;;
    esac
    AC_ARG_ENABLE([meta-info-embedding],
	[AC_HELP_STRING([--enable-meta-info-embedding],
	    [store plugin meta-information directly inside plugin modules if
	    supported by platform; if disabled, meta-information is stored in
	    stand-alone .csplugin files; this option is enabled by default for
	    non-Unix platforms and on Unix platforms with ELF-format object
	    files; it is disabled by default on Unix platforms if ELF is not
	    available and the project uses a non-GPL-compatible license (such
	    as LGPL) since the non-ELF Unix embedding technology requires the
	    GPL-licensed libbfd library; if ELF is not available, enable this
	    option on Unix only if you are certain you want a GPL-licensed
	    library infecting your project])],
	[], [enable_meta_info_embedding=$cs_embed_meta_info_default])
    AC_MSG_RESULT([$enable_meta_info_embedding])
    CS_EMIT_BUILD_PROPERTY([EMBED_META], [$enable_meta_info_embedding],
	[], [], CS_EMITTER_OPTIONAL([$1]))])



#------------------------------------------------------------------------------
# _CS_META_INFO_EMBED_TOOLS([EMITTER])
#	Helper for CS_META_INFO_EMBED() which searches for tools required for
#	plugin meta-info embedding.
#------------------------------------------------------------------------------
AC_DEFUN([_CS_META_INFO_EMBED_TOOLS],
    [CS_CHECK_TOOLS([WINDRES], [windres])
    CS_EMIT_BUILD_PROPERTY([CMD.WINDRES], [$WINDRES], [], [],
	CS_EMITTER_OPTIONAL([$1]))

    CS_CHECK_TOOLS([OBJCOPY], [objcopy])
    AS_IF([test -n "$OBJCOPY"],
        [CS_EMIT_BUILD_PROPERTY([OBJCOPY.AVAILABLE], [yes], [], [],
	    CS_EMITTER_OPTIONAL([$1]))
        CS_EMIT_BUILD_PROPERTY([CMD.OBJCOPY], [$OBJCOPY], [], [],
	    CS_EMITTER_OPTIONAL([$1]))])])



#------------------------------------------------------------------------------
# CS_CHECK_LIBBFD([EMITTER], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#	Exhaustive check for a usable GPL-licensed libbfd, the Binary File
#	Descriptor library, a component of binutils, which allows low-level
#	manipulation of executable and object files.  If EMITTER is provided,
#	then the following variables are recorded by invoking
#	CS_EMIT_BUILD_PROPERTY() with EMITTER.  As a convenience, if EMITTER is
#	the literal value "emit" or "yes", then CS_EMIT_BUILD_RESULT()'s
#	default emitter will be used.
#
#	LIBBFD.AVAILABLE := yes or no
#	LIBBFD.CFLAGS := libbfd compiler flags
#	LIBBFD.LFLAGS := libbfd linker flags
#
#	The shell variable cs_cv_libbfd_ok is set to yes if a usable libbfd was
#	discovered, else no. If found, the additional shell variables
#	cs_cv_libbfd_ok_cflags, cs_cv_libbfd_ok_lflags, and
#	cs_cv_libbfd_ok_libs are also set.
#
# WARNING
#
#	libbfd carries a GPL license which is incompatible with the LGPL
#	license of Crystal Space. Do not use this library with projects under
#	less restrictive licenses, such as LGPL.
#
# IMPLEMENTATION NOTES
#
#	It seems that some platforms have two version of libiberty installed:
#	one from binutils and one from gcc.  The binutils version resides in
#	/usr/lib, whereas the gcc version resides in the gcc installation
#	directory.  The gcc version, by default, takes precedence at link time
#	over the binutils version.  Unfortunately, in broken cases, the gcc
#	version of libiberty is missing htab_create_alloc() which is required
#	by some libbfd functions.  The extensive secondary check of libbfd
#	catches this anomalous case of broken gcc libiberty.  It turns out that
#	it is possible to make the linker prefer the binutils version by
#	specifying -L/usr/lib, thus the extensive test attempts to do so in an
#	effort to resolve this unfortunate issue.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_LIBBFD],
    [CS_CHECK_LIB_WITH([bfd],
	[AC_LANG_PROGRAM([[#include <bfd.h>]], [bfd_init();])],
	[], [], [], [], [], [], [-liberty])

    AS_IF([test $cs_cv_libbfd = yes],
	[CS_CHECK_BUILD([if libbfd is usable], [cs_cv_libbfd_ok],
	    [AC_LANG_PROGRAM([[#include <bfd.h>]],
		[bfd* p;
		asection* s;
		bfd_init();
		p = bfd_openr(0,0);
		bfd_check_format(p,bfd_object);
		bfd_get_section_by_name(p,0);
		bfd_section_size(p,s);
		bfd_get_section_contents(p,s,0,0,0);
		bfd_close(p);])],
	    [CS_CREATE_TUPLE() CS_CREATE_TUPLE([],[-L/usr/lib],[])],
	    [], [], [], [],
	    [$cs_cv_libbfd_cflags],
	    [$cs_cv_libbfd_lflags],
	    [$cs_cv_libbfd_libs])],
	[cs_cv_libbfd_ok=no])

    AS_IF([test $cs_cv_libbfd_ok = yes],
	[CS_EMIT_BUILD_RESULT([cs_cv_libbfd_ok], [LIBBFD],
	    CS_EMITTER_OPTIONAL([$1]))
	$2],
	[$3])])
# emit.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003-2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_EMIT_BUILD_PROPERTY(KEY, VALUE, [APPEND], [EMPTY-OKAY], [EMITTER],
#                        [UNCONDITIONAL])
#	A utility function which invokes an emitter to record the KEY/VALUE
#	tuple if VALUE is not the empty string (after leading and trailing
#	whitespace is stripped). If EMPTY-OKAY is not an empty string, then the
#	property is emitted even if VALUE is empty; that is, it is emitted
#	unconditionally.  If APPEND is the empty string, then the emitter sets
#	the key's value directly (though it may be overridden by the
#	environment), otherwise the emitter appends VALUE to the existing value
#	of the key.  EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	KEY/VALUE tuple; it should also accept APPEND as an optional third
#	argument. If EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.  Some
#	emitters accept an optional fourth argument, UNCONDITIONAL, which
#	instructs it to set KEY's value unconditionally, even if KEY already
#	had been assigned a value via some other mechanism (such as imported
#	from the environment, or from Jambase, in the case of
#	CS_JAMCONFIG_PROPERTY).
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_PROPERTY],
    [cs_build_prop_val="$2"
    cs_build_prop_val=CS_TRIM([$cs_build_prop_val])
    m4_ifval([$4],
	[CS_JAMCONFIG_PROPERTY([$1], [$cs_build_prop_val], [$3])],
	AS_IF([test -n "$cs_build_prop_val"],
	    [m4_default([$5],[CS_JAMCONFIG_PROPERTY])(
		[$1], [$cs_build_prop_val], [$3], [$6])]))])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_RESULT(CACHE-VAR, PREFIX, [EMITTER])
#	Record the results of CS_CHECK_BUILD() or CS_CHECK_LIB_WITH() via some
#	emitter.  If CACHE-VAR indicates that the build succeeded, then the
#	following properties are emitted:
#
#	PREFIX.AVAILABLE = yes
#	PREFIX.CFLAGS = $CACHE-VAR_cflags
#	PREFIX.LFLAGS = $CACHE-VAR_lflags $CACHE-VAR_libs
#
#	EMITTER is a macro name, such as CS_JAMCONFIG_PROPERTY or
#	CS_MAKEFILE_PROPERTY, which performs the actual task of emitting the
#	KEY/VALUE tuple. If EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_RESULT],
    [AS_IF([test "$$1" = yes],
	[CS_EMIT_BUILD_PROPERTY([$2.AVAILABLE], [yes], [], [], [$3])
	CS_EMIT_BUILD_PROPERTY([$2.CFLAGS], [$$1_cflags], [], [], [$3])
	CS_EMIT_BUILD_PROPERTY([$2.LFLAGS], [$$1_lflags $$1_libs],
	    [], [], [$3])])])



#------------------------------------------------------------------------------
# CS_EMIT_BUILD_FLAGS(MESSAGE, CACHE-VAR, FLAGS, [LANGUAGE], EMITTER-KEY,
#                     [APPEND], [ACTION-IF-RECOGNIZED],
#                     [ACTION-IF-NOT-RECOGNIZED], [EMITTER])
#	A convenience wrapper for CS_CHECK_BUILD_FLAGS() which also records the
#	results via CS_EMIT_BUILD_PROPERTY().  Checks if the compiler or linker
#	recognizes a command-line option.  MESSAGE is the "checking" message.
#	CACHE-VAR is the shell cache variable which receives the flag
#	recognized by the compiler or linker, or "no" if the flag was not
#	recognized.  FLAGS is a whitespace- delimited list of build tuples
#	created with CS_CREATE_TUPLE().  Each tuple from FLAGS is attempted in
#	order until one is found which is recognized by the compiler.  After
#	that, no further flags are checked.  LANGUAGE is typically either C or
#	C++ and specifies which compiler to use for the test.  If LANGUAGE is
#	omitted, C is used.  EMITTER-KEY is the name to pass as the emitter's
#	"key" argument if a usable flag is encountered.  If APPEND is not the
#	empty string, then the discovered flag is appended to the existing
#	value of the EMITTER-KEY.  If the command-line option was recognized,
#	then ACTION-IF-RECOGNIZED is invoked, otherwise
#	ACTION-IF-NOT-RECOGNIZED is invoked.  EMITTER is a macro name, such as
#	CS_JAMCONFIG_PROPERTY or CS_MAKEFILE_PROPERTY, which performs the
#	actual task of emitting the KEY/VALUE tuple; it should also accept
#	APPEND as an optional third argument. If EMITTER is omitted,
#	CS_JAMCONFIG_PROPERTY is used.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_BUILD_FLAGS],
    [CS_CHECK_BUILD_FLAGS([$1], [$2], [$3], [$4],
	[CS_EMIT_BUILD_PROPERTY([$5], [$$2], [$6], [], [$9])
	    $7],
	[$8])])



#------------------------------------------------------------------------------
# CS_EMITTER_OPTIONAL([EMITTER])
#	The CS_EMIT_FOO() macros optionally accept an emitter. If no emitter is
#	supplied to those macros, then a default emitter is chosen.  Other
#	macros, however, which perform testing and optionally emit the results
#	may wish to interpret an omitted EMITTER as a request not to emit the
#	results. CS_EMITTER_OPTIONAL() is a convenience macro to help in these
#	cases. It should be passed to one of the CS_EMIT_FOO() macros in place
#	of the literal EMITTER argument. It functions by re-interpretating
#	EMITTER as follows:
#
#	- If EMITTER is omitted, then CS_NULL_EMITTER is returned, effectively
#	  disabling output by the CS_EMIT_FOO() macro.
#	- If EMITTER is the literal string "emit" or "yes", then it returns an
#	  empty string, which signals to the CS_EMIT_FOO() macro that is should
#	  use its default emitter.
#	- Any other value for EMITTER is passed along as-is to the
#	  CS_EMIT_FOO() macro.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMITTER_OPTIONAL],
    [m4_case([$1],
	[], [[CS_NULL_EMITTER]],
	[emit], [],
	[yes], [],
	[[$1]])])



#------------------------------------------------------------------------------
# CS_NULL_EMITTER(KEY, VALUE, [APPEND])
#	A do-nothing emitter suitable for use as the EMITTER argument of one of
#	the CS_EMIT_FOO() macros.
#------------------------------------------------------------------------------
AC_DEFUN([CS_NULL_EMITTER], [:
])



#------------------------------------------------------------------------------
# CS_SUBST_EMITTER(KEY, VALUE, [APPEND])
#	An emitter wrapped around AC_SUBST(). Invokes
#	AC_SUBST(AS_TR_SH(KEY),VALUE).  The APPEND argument is ignored.
#	Suitable for use as the EMITTER argument of one of the CS_EMIT_FOO()
#	macros.  The call to AS_TR_SH() ensures that KEY is transformed into a
#	valid shell variable. For instance, if a macro attempts to emit
#	MYLIB.CFLAGS and MYLIB.LFLAGS via CS_SUBST_EMITTER(), then the names
#	will be transformed to MYLIB_CFLAGS and MYLIB_LFLAGS, respectively, for
#	the invocation of AC_SUBST().
#------------------------------------------------------------------------------
AC_DEFUN([CS_SUBST_EMITTER], [AC_SUBST(AS_TR_SH([$1]),[$2])])



#------------------------------------------------------------------------------
# CS_DEFINE_EMITTER(KEY, VALUE, [APPEND])
#	An emitter wrapped around AC_DEFINE_UNQUOTED(). Invokes
#	AC_DEFINE_UNQUOTED(AS_TR_CPP(KEY),VALUE).  The APPEND argument is
#	ignored.  Suitable for use as the EMITTER argument of one of the
#	CS_EMIT_FOO() macros. The call to AS_TR_CPP() ensures that KEY is a
#	well-formed token for the C-preprocessor.
#------------------------------------------------------------------------------
AC_DEFUN([CS_DEFINE_EMITTER],
    [AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]),[$2],
	[Define when feature is available])])
# headercache.m4                                               -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility for C-style #define properties.  The cache is stored in
# the shell variable cs_header_text.
#
# CS_HEADER_APPEND(TEXT)
#	Append text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_HEADER_PREPEND(TEXT)
#	Prepend text to the C header text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_HEADER_PROPERTY(KEY, [VALUE])
#	Append a line of the form "#define KEY VALUE" to the C header text
#	cache.  If the VALUE argument is omitted, then the appended line has
#	the simplified form "#define KEY".
#
# CS_HEADER_OUTPUT(FILENAME)
#	Instruct config.status to write the C header text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_HEADER_APPEND], [CS_TEXT_CACHE_APPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PREPEND], [CS_TEXT_CACHE_PREPEND([cs_header_text], [$1])])
AC_DEFUN([CS_HEADER_PROPERTY],
[CS_HEADER_APPEND([@%:@define $1[]m4_ifval([$2], [ $2], [])
])])
AC_DEFUN([CS_HEADER_OUTPUT], [CS_TEXT_CACHE_OUTPUT([cs_header_text], [$1])])
#-----------------------------------------------------------------------------
# installdirs.m4 (c) Matze Braun <matze@braunis.de>
# Macro for emitting the installation paths gathered by Autoconf.
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# CS_OUTPUT_INSTALLDIRS([EMITTER], [RAW-BACKSLASHES])
#   Emit installation directories collected by Autoconf.  EMITTER is a macro
#   name, such as CS_JAMCONFIG_PROPERTY or CS_MAKEFILE_PROPERTY, which performs
#   the actual task of emitting the KEY/VALUE tuple.  If EMITTER is omitted,
#   CS_JAMCONFIG_PROPERTY is used.  If RAW-BACKSLASHES is not provided, then
#   backslashes in emitted values are each escaped with an additional
#   backslash. If RAW-BACKSLASHES is not the null value, then backslashes are
#   emitted raw.  The following properties are emitted:
#
#       prefix
#       exec_prefix
#       bindir
#       sbindir
#       libexecdir
#       datadir
#       sysconfdir
#       sharedstatedir
#       localstatedir
#       libdir
#       includedir
#       oldincludedir
#       infodir
#       mandir
#-----------------------------------------------------------------------------
AC_DEFUN([CS_OUTPUT_INSTALLDIRS],[
# Handle the case when no prefix is given, and the special case when a path
# contains more than 2 slashes, these paths seem to be correct but Jam fails
# on them.
AS_IF([test $prefix = NONE],
    [cs_install_prefix="$ac_default_prefix"],
    [cs_install_prefix=`echo "$prefix" | sed -e 's:///*:/:g'`])
AS_IF([test $exec_prefix = NONE],
    [cs_install_exec_prefix="AS_ESCAPE([$(prefix)])"],
    [cs_install_exec_prefix=`echo "$exec_prefix" | sed -e 's:///*:/:g'`])

_CS_OUTPUT_INSTALL_DIRS([$1], [prefix],
    [CS_PREPARE_INSTALLPATH([$cs_install_prefix], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [exec_prefix],
    [CS_PREPARE_INSTALLPATH([$cs_install_exec_prefix], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [bindir],
    [CS_PREPARE_INSTALLPATH([$bindir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [sbindir],
    [CS_PREPARE_INSTALLPATH([$sbindir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [libexecdir],
    [CS_PREPARE_INSTALLPATH([$libexecdir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [datadir],
    [CS_PREPARE_INSTALLPATH([$datadir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [sysconfdir], 
    [CS_PREPARE_INSTALLPATH([$sysconfdir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [sharedstatedir], 
    [CS_PREPARE_INSTALLPATH([$sharedstatedir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [localstatedir], 
    [CS_PREPARE_INSTALLPATH([$localstatedir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [libdir],
    [CS_PREPARE_INSTALLPATH([$libdir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [includedir], 
    [CS_PREPARE_INSTALLPATH([$includedir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [oldincludedir], 
    [CS_PREPARE_INSTALLPATH([$oldincludedir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [infodir],
    [CS_PREPARE_INSTALLPATH([$infodir], [$2])])
_CS_OUTPUT_INSTALL_DIRS([$1], [mandir],
    [CS_PREPARE_INSTALLPATH([$mandir], [$2])])
])

AC_DEFUN([_CS_OUTPUT_INSTALL_DIRS],
    [m4_default([$1], [CS_JAMCONFIG_PROPERTY])([$2], [$3])])


#-----------------------------------------------------------------------------
# CS_PREPARE_INSTALLPATH(VALUE, [RAW-BACKSLASHES])
#   Transform variable references of the form ${bla} to $(bla) in VALUE and
#   correctly quotes backslashes.  This is needed if you need to emit some of
#   the paths from Autoconf. RAW-BACKSLASHES has the same meaning as in
#   CS_OUTPUT_INSTALLDIRS.
#-----------------------------------------------------------------------------
AC_DEFUN([CS_PREPARE_INSTALLPATH],
[`echo "$1" | sed 's/\${\([[a-zA-Z_][a-zA-Z_]]*\)}/$(\1)/g;m4_ifval([$2],
    [s/\\/\\\\/g], [s/\\\\/\\\\\\\\/g])'`])
# jamcache.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility for Jam-style properties.  The cache is stored in
# the shell variable cs_jamfile_text.
#
# CS_JAMCONFIG_APPEND(TEXT)
#	Append text to the Jam text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_JAMCONFIG_PREPEND(TEXT)
#	Prepend text to the Jam text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_JAMCONFIG_PROPERTY(KEY, VALUE, [APPEND], [UNCONDITIONAL])
#	Append a line of the form "KEY ?= VALUE" to the Jam text cache.  If the
#	APPEND argument is not the empty string, then VALUE is appended to the
#	existing value of KEY using the form "KEY += VALUE".  If the
#	UNCONDITIONAL argument is not empty, then the value of KEY is set
#	unconditionally "KEY = VALUE", rather than via "KEY ?= VALUE".  APPEND
#	takes precedence over UNCONDITIONAL.  Note that if VALUE references
#	other Jam variables, for example $(OBJS), then be sure to protect the
#	value with AS_ESCAPE().  For example:
#	CS_JAMCONFIG_PROPERTY([ALLOBJS], [AS_ESCAPE([$(OBJS) $(LIBOBJS)])])
#
# CS_JAMCONFIG_OUTPUT(FILENAME)
#	Instruct config.status to write the Jam text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_JAMCONFIG_APPEND],
    [CS_TEXT_CACHE_APPEND([cs_jamconfig_text], [$1])])
AC_DEFUN([CS_JAMCONFIG_PREPEND],
    [CS_TEXT_CACHE_PREPEND([cs_jamconfig_text], [$1])])
AC_DEFUN([CS_JAMCONFIG_PROPERTY],
    [CS_JAMCONFIG_APPEND(
	[$1 m4_ifval([$3], [+=], m4_ifval([$4], [=], [?=])) \"$2\" ;
])])
AC_DEFUN([CS_JAMCONFIG_OUTPUT],
    [CS_TEXT_CACHE_OUTPUT([cs_jamconfig_text], [$1])])
# makecache.m4                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility for makefile-style properties.  The cache is stored in
# the shell variable cs_makefile_text.
#
# CS_MAKEFILE_APPEND(TEXT)
#	Append text to the makefile text cache.  This is a cover for
#	CS_TEXT_CACHE_APPEND().
#
# CS_MAKEFILE_PREPEND(TEXT)
#	Prepend text to the makefile text cache.  This is a cover for
#	CS_TEXT_CACHE_PREPEND().
#
# CS_MAKEFILE_PROPERTY(KEY, VALUE, [APPEND])
#	Append a line of the form "KEY = VALUE" to the makefile text cache.  If
#	the APPEND argument is not the empty string, then VALUE is appended to
#	the existing value of KEY using the form "KEY += VALUE".  Note that if
#	VALUE references other makefile variables, for example $(OBJS), then be
#	sure to protect the value with AS_ESCAPE().  For example:
#	CS_MAKEFILE_PROPERTY([ALLOBJS], [AS_ESCAPE([$(OBJS) $(LIBOBJS)])])
#
# CS_MAKEFILE_OUTPUT(FILENAME)
#	Instruct config.status to write the makefile text cache to the given
#	filename.  This is a cover for CS_TEXT_CACHE_OUTPUT().
#------------------------------------------------------------------------------
AC_DEFUN([CS_MAKEFILE_APPEND],
    [CS_TEXT_CACHE_APPEND([cs_makefile_text], [$1])])
AC_DEFUN([CS_MAKEFILE_PREPEND],
    [CS_TEXT_CACHE_PREPEND([cs_makefile_text], [$1])])
AC_DEFUN([CS_MAKEFILE_PROPERTY],
    [CS_MAKEFILE_APPEND([$1 m4_ifval([$3], [+=], [=]) $2
])])
AC_DEFUN([CS_MAKEFILE_OUTPUT],[CS_TEXT_CACHE_OUTPUT([cs_makefile_text], [$1])])
# mkdir.m4                                                     -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_CHECK_MKDIR
#	Determine how to create a directory and a directory tree. Sets the
#	shell variable MKDIR to the command which creates a directory, and
#	MKDIRS to the command which creates a directory tree. Invokes
#	AC_SUBST() for MKDIR and MKDIRS.
#
# IMPLEMENTATION NOTES
#	We need to know the exact commands, so that we can emit them, thus the
#	AS_MKDIR_P function is not what we want to use here since it does not
#	provide access to the commands (and might not even discover suitable
#	commands).  First try "mkdir -p", then try the older "mkdirs".
#	Finally, if the mkdir command failed to recognize -p, then it might
#	have created a directory named "-p", so clean up that bogus directory.
#------------------------------------------------------------------------------
AC_DEFUN([CS_CHECK_MKDIR],
    [AC_CACHE_CHECK([how to create a directory], [cs_cv_shell_mkdir],
	[cs_cv_shell_mkdir='mkdir'])
    AC_SUBST([MKDIR], [$cs_cv_shell_mkdir])

    AC_CACHE_CHECK([how to create a directory tree], [cs_cv_shell_mkdir_p],
	[if $cs_cv_shell_mkdir -p . 2>/dev/null; then
	    cs_cv_shell_mkdir_p='mkdir -p'
	elif mkdirs . 2>/dev/null; then
	    cs_cv_shell_mkdir_p='mkdirs'
	fi
	test -d ./-p && rmdir ./-p])
    AS_VAR_SET_IF([cs_cv_shell_mkdir_p],
	[AC_SUBST([MKDIRS], [$cs_cv_shell_mkdir_p])],
	[CS_MSG_ERROR([do not know how to create a directory tree])])])



#------------------------------------------------------------------------------
# Replacement for AS_MKDIR_P() from m4sugar/m4sh.m4 which fixes two problems
# which are present in Autoconf 2.57 and probably all earlier 2.5x versions.
# This bug, along with a patch, was submitted to the Autoconf GNATS database by
# Eric Sunshine as #227 on 17-Dec-2002.  The bogus "-p" directory bug was fixed
# for Autoconf 2.58 on 26-Sep-2003.  The "mkdirs" optimization was not accepted
# (since it is unnecessary; it's only an optimization).
#
# 1) Removes bogus "-p" directory which the stock AS_MKDIR_P() leaves laying
#    around in the working directory if the mkdir command does not recognize
#    the -p option.
# 2) Takes advantage of the older "mkdirs" program if it exists and if "mkdir
#    -p" does not work.
#------------------------------------------------------------------------------
m4_defun([_AS_MKDIR_P_PREPARE],
[if mkdir -p . 2>/dev/null; then
  as_mkdir_p='mkdir -p'
elif mkdirs . 2>/dev/null; then
  as_mkdir_p='mkdirs'
else
  as_mkdir_p=''
fi
test -d ./-p && rmdir ./-p
])# _AS_MKDIR_P_PREPARE

m4_define([AS_MKDIR_P],
[AS_REQUIRE([_$0_PREPARE])dnl
{ if test -n "$as_mkdir_p"; then
    $as_mkdir_p $1
  else
    as_dir=$1
    as_dirs=
    while test ! -d "$as_dir"; do
      as_dirs="$as_dir $as_dirs"
      as_dir=`AS_DIRNAME("$as_dir")`
    done
    test ! -n "$as_dirs" || mkdir $as_dirs
  fi || AS_ERROR([cannot create directory $1]); }
])# AS_MKDIR_P
#==============================================================================
# packageinfo.m4
#    Macros for setting general info on the package, such as name and version
#    numbers and propagate them to the generated make and Jam property files.
#
# Copyright (C)2003 by Matthias Braun <matze@braunis.de>
# Copyright (C)2003,2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================

#------------------------------------------------------------------------------
# CS_PACKAGEINFO([LONGNAME], [COPYRIGHT, [HOMEPAGE])
#	Set additional information for the package.  Note that the version
#	number of your application should only contain numbers, because on
#	Windows you can only set numerical values in some of the file
#	properties (such as versioninfo .rc files).
#------------------------------------------------------------------------------
AC_DEFUN([CS_PACKAGEINFO],
    [PACKAGE_LONGNAME="[$1]"
    PACKAGE_COPYRIGHT="[$2]"
    PACKAGE_HOMEPAGE="[$3]"
])


#------------------------------------------------------------------------------
# CS_EMIT_PACKAGEINFO([EMITTER])
#	Emit extended package information using the provided EMITTER.  EMITTER
#	is a macro name, such as CS_JAMCONFIG_PROPERTY or CS_MAKEFILE_PROPERTY,
#	which performs the actual task of emitting the KEY/VALUE tuple.  If
#	EMITTER is omitted, CS_JAMCONFIG_PROPERTY is used.  For backward
#	compatibility, if EMITTER is the literal value "jam", then
#	CS_JAMCONFIG_PROPERTY is used; if it is "make", then
#	CS_MAKEFILE_PROPERTY is used; however use of these literal names is
#	highly discouraged.
#------------------------------------------------------------------------------
AC_DEFUN([CS_EMIT_PACKAGEINFO],
    [_CS_EMIT_PACKAGEINFO([$1], [PACKAGE_NAME], [$PACKAGE_NAME])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_VERSION], [$PACKAGE_VERSION])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_STRING], [$PACKAGE_STRING])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_BUGREPORT], [$PACKAGE_BUGREPORT])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_LONGNAME], [$PACKAGE_LONGNAME])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_HOMEPAGE], [$PACKAGE_HOMEPAGE])
    _CS_EMIT_PACKAGEINFO([$1], [PACKAGE_COPYRIGHT], [$PACKAGE_COPYRIGHT])
    for cs_veritem in m4_translit(AC_PACKAGE_VERSION, [.], [ ]); do
	_CS_EMIT_PACKAGEINFO([$1], [PACKAGE_VERSION_LIST], [$cs_veritem], [+])
    done
    ])

AC_DEFUN([_CS_EMIT_PACKAGEINFO],
    [m4_case([$1],
	[make], [CS_MAKEFILE_PROPERTY([$2], [$3], [$4])],
	[jam], [CS_JAMCONFIG_PROPERTY([$2], [$3], [$4])],
	[], [CS_JAMCONFIG_PROPERTY([$2], [$3], [$4])],
	[$1([$2], [$3], [$4])])])
# path.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2004 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_PATH_NORMALIZE(STRING)
#	Normalize a pathname at run-time by transliterating Windows/DOS
#	backslashes to forward slashes.  Also collapses whitespace.
#------------------------------------------------------------------------------
AC_DEFUN([CS_PATH_NORMALIZE],
[`echo "x$1" | tr '\\\\' '/' | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])


#------------------------------------------------------------------------------
# CS_RUN_PATH_NORMALIZE(COMMAND)
#	Normalize the pathname emitted by COMMAND by transliterating
#	Windows/DOS backslashes to forward slashes.  Also collapses whitespace.
#------------------------------------------------------------------------------
AC_DEFUN([CS_RUN_PATH_NORMALIZE],
[`AC_RUN_LOG([$1]) | tr '\\\\' '/' | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])
###############################################################################
# progver.m4
# Written by Norman Kramer <norman@users.sourceforge.net>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
###############################################################################
#
# From the input pattern we create regular expressions we send through sed
# to extract the version information from the standard input to sed.
# Then we extract from the resulting version string subparts.
# The same happens with the supplied version string. It too is split into its
# subparts according to the pattern.
# Then the subparts from the gathered version string and the supplied one are
# compared.
#
# How does the pattern look like ?
# It is a sequence of 9s and _s and separators.
# 9 denotes a non empty sequence of digits.
# _ denotes a non empty sequence of characters from the class [a-zA-Z].
# | everything behind is optional
# Everything else is treated as a separator.
# Consecutive 9s and _s are compressed to contain only one of each type.
# For instance "99_.9.__abc9_" will become "9_.9._abc9_".
#
# How we find the parts we compare ?
# From this transformed string we yield the parts we will later compare.
# We break up the string as follows:
# Any sequence of separators represent one breakup. Additional breakups are
# placed behind every 9 and _ .
# So the example from above will give:
#
# "99_.9.__abc9_"  ===compress==> "9_.9._abc9_" ===breakup==> "9" "_" "9" "_" "9" "_"
#
# How we create the regular expressions ?
# We take the compressed pattern and quote every separator.
# The we replace the 9s with [0-9][0-9]*
# and the _s with [a-zA-Z][a-zA-Z]* .
# The above example will become:
#
# "99_.9.__abc9_"  ===compress==> "9_.9._abc9_" ===rexify==>
# [0-9][0-9]*[a-zA-Z][a-zA-Z]*\.[0-9][0-9]*\.[a-zA-Z][a-zA-Z]*\a\b\c[0-9][0-9]*[a-zA-Z][a-zA-Z]*
#
# Voila.
#
# To yield the subparts from the string we additionally enclose the
# 9s and _s with \( and \).
#
###############################################################################

# ****************************************************************
# **                      helper definitions                    **
# ****************************************************************
m4_define([CS_VCHK_RUNTH], [m4_pushdef([i], [$1])m4_if($1,0,,[CS_VCHK_RUNTH(m4_decr($1), [$2])][$2])m4_popdef([i])])
m4_define([CS_VCHK_PREFIX], [])
m4_define([CS_VCHK_SUFFIX], [])
m4_define([CS_VCHK_GROUPPREFIX], [\(])
m4_define([CS_VCHK_GROUPSUFFIX], [\)])
m4_define([CS_VCHK_CHAR], [[[[a-zA-Z]]]])
m4_define([CS_VCHK_DIGIT], [[[0-9]]])
m4_define([CS_VCHK_SEQUENCE], [CS_VCHK_PREFIX[]CS_VCHK_SINGLE[]CS_VCHK_SINGLE[]*CS_VCHK_SUFFIX[]])
m4_define([CS_VCHK_OPTSEQUENCE], [CS_VCHK_PREFIX[]CS_VCHK_SINGLE[]*CS_VCHK_SUFFIX[]])
m4_define([CS_VCHK_REXSEQ], [m4_bpatsubst($1, [$2], [[]CS_VCHK_SEQUENCE[]])])
m4_define([CS_VCHK_GROUPINGON], [m4_pushdef([CS_VCHK_PREFIX], [CS_VCHK_GROUPPREFIX])m4_pushdef([CS_VCHK_SUFFIX], [CS_VCHK_GROUPSUFFIX])])
m4_define([CS_VCHK_GROUPINGOFF], [m4_popdef([CS_VCHK_SUFFIX])m4_popdef([CS_VCHK_PREFIX])])
m4_define([CS_VCHK_OPTON], [m4_pushdef([CS_VCHK_SEQUENCE], [CS_VCHK_OPTSEQUENCE])])
m4_define([CS_VCHK_OPTOFF], [m4_popdef([CS_VCHK_SEQUENCE])])
m4_define([CS_VCHK_RMOPT], [CS_VCHK_RMCHAR([$1], m4_index([$1], [|]))])
m4_define([CS_VCHK_RMCHAR], [m4_if($2,-1,[$1],m4_substr([$1], 0, $2)[]m4_substr([$1], m4_incr($2)))])
m4_define([CS_VCHK_RMALL], [m4_translit([$1], [|], [])])
m4_define([CS_VCHK_CUTOFF], [m4_if(m4_index($1,[|]),-1, [$1], [m4_substr($1, 0, m4_index($1,[|]))])])
m4_define([CS_VCHK_CYCLEOPT], [
m4_if($2,-1,, [m4_pushdef([i], CS_VCHK_CUTOFF([$1])) m4_pushdef([j], CS_VCHK_DUMMY_TAIL([$1])) CS_VCHK_CYCLEOPT( CS_VCHK_RMOPT([$1]), m4_index($1, [|]), [$3])$3 m4_popdef([i]) m4_popdef([j])])
])
m4_define([CS_VCHK_TAIL], [m4_if(m4_index($1,[|]),-1, [], [m4_substr($1, m4_incr(m4_index($1,[|])))])])
m4_define([CS_VCHK_DUMMY_COMPRESS], [m4_bpatsubst(m4_bpatsubst([$1], [__*], [A]), [99*], [0])])
m4_define([CS_VCHK_DUMMY_TAIL], [CS_VCHK_DUMMY_COMPRESS(m4_translit(CS_VCHK_TAIL([$1]), [|], []))])

# ****************************************************************
# **                      FlagsOn / FlagsOff                    **
# ****************************************************************
m4_define([CS_VCHK_FLAGSON],
[m4_if($#, 0, [],
       $1, [], [],
       [$1], [group], [CS_VCHK_GROUPINGON[]],
       [$1], [opt], [CS_VCHK_OPTON[]])dnl
m4_if($#, 0, [], $1, [], [], [CS_VCHK_FLAGSON(m4_shift($@))])])

m4_define([CS_VCHK_FLAGSOFF],
[m4_if($#, 0, [],
       $1, [], [],
       $1, [group], [CS_VCHK_GROUPINGOFF[]],
       [$1], [opt], [CS_VCHK_OPTOFF[]])dnl
m4_if($#, 0, [], $1, [], [], [CS_VCHK_FLAGSOFF(m4_shift($@))])])

# ****************************************************************
# **                      rexify / sedify                       **
# ****************************************************************
m4_define([CS_VCHK_REXIFY],
[m4_pushdef([CS_VCHK_SINGLE], [$1])dnl
CS_VCHK_FLAGSON(m4_shift(m4_shift(m4_shift($@))))dnl
CS_VCHK_REXSEQ([$3], [$2])dnl
CS_VCHK_FLAGSOFF(m4_shift(m4_shift(m4_shift($@))))dnl
m4_popdef([CS_VCHK_SINGLE])])

m4_define([CS_VCHK_QUOTESEP], [m4_bpatsubst($1, [[^9_]], [\\\&])])

m4_define([CS_VCHK_REXCHAR], [CS_VCHK_REXIFY([CS_VCHK_CHAR], [__*], $@)])
m4_define([CS_VCHK_REXDIGIT],  [CS_VCHK_REXIFY([CS_VCHK_DIGIT], [99*], $@)])
m4_define([CS_VCHK_SEDIFY], [CS_VCHK_REXDIGIT([CS_VCHK_REXCHAR([CS_VCHK_QUOTESEP([$1])], m4_shift($@))], m4_shift($@))])
m4_define([CS_VCHK_SEDEXPRALL], [/CS_VCHK_SEDIFY([$1])/!d;s/.*\(CS_VCHK_SEDIFY([$1])\).*/\1/;q])
m4_define([CS_VCHK_SEDEXPRNTH], [/CS_VCHK_SEDIFY([$1])/!d;s/.*CS_VCHK_SEDIFY([$1],[group]).*/\$2/])

# ****************************************************************
# **                      Pattern splitting                     **
# ****************************************************************
m4_define([CS_VCHK_SPLITSEP], [CS_VCHK_REXIFY([s], [[^9_][^9_]*], $@)])
m4_define([CS_VCHK_SPLITDIGIT], [CS_VCHK_REXIFY([d], [99*], $@)])
m4_define([CS_VCHK_SPLITCHAR], [CS_VCHK_REXIFY([c], [__*], $@)])

# ****************************************************************
# ** return a list of 's' 'd' 'c' 'e' chars denoting the kind   **
# ** pattern parts: separator, digit, char, end                 **
# ****************************************************************
m4_define([CS_VCHK_PATTERNLIST], [m4_pushdef([CS_VCHK_SEQUENCE], [CS_VCHK_SINGLE ])dnl
m4_translit(CS_VCHK_SPLITDIGIT([CS_VCHK_SPLITCHAR([CS_VCHK_SPLITSEP([$1])])]), [ ], m4_if([$2],[],[ ],[$2]))e[]dnl
m4_popdef([CS_VCHK_SEQUENCE])])

# ****************************************************************
# ** Build the shell commands we emit to the configure script.  **
# ****************************************************************
m4_define([CS_VCHK_PATCOUNT], [m4_len(m4_bpatsubst(CS_VCHK_PATTERNLIST([$1]), [[^dc]]))])

# ****************************************************************************************
# ** CS_VCHK_EXTRACTVERSION(EXTRACT_CALL, MIN_VERSION, PATTERN, PRGPREFIX, COMPARISION) **
# ****************************************************************************************
m4_define([CS_VCHK_EXTRACTVERSION],
[cs_prog_$4_is_version=
cs_prog_$4_min_version=
cs_prog_$4_is_suffix=
cs_prog_$4_min_suffix=
cs_prog_$4_is_suffix_done=
cs_prog_$4_min_suffix_done=
CS_VCHK_CYCLEOPT([$3], [], 
[test -z $cs_prog_$4_is_version && cs_prog_$4_is_version=`$1 | sed 'CS_VCHK_SEDEXPRALL([i])'`
test -n "$cs_prog_$4_is_version" && test -z $cs_prog_$4_is_suffix_done  && { cs_prog_$4_is_suffix_done=yes ; cs_prog_$4_is_suffix=j ; }
])
CS_VCHK_CYCLEOPT([$3], , 
[test -z $cs_prog_$4_min_version && cs_prog_$4_min_version=`echo $2 | sed 'CS_VCHK_SEDEXPRALL([i])'`
test -n "$cs_prog_$4_min_version" && test -z $cs_prog_$4_min_suffix_done  && { cs_prog_$4_min_suffix_done=yes ; cs_prog_$4_min_suffix=j ; }
])
CS_VCHK_RUNTH([CS_VCHK_PATCOUNT([$3])],
    [cs_prog_$4_is_ver_[]i=`echo ${cs_prog_$4_is_version}${cs_prog_$4_is_suffix} | sed 'CS_VCHK_SEDEXPRNTH([CS_VCHK_RMALL([$3])], [i])'`
])
CS_VCHK_RUNTH([CS_VCHK_PATCOUNT([$3])],
    [cs_prog_$4_min_ver_[]i=`echo $cs_prog_$4_min_version${cs_prog_$4_min_suffix} | sed 'CS_VCHK_SEDEXPRNTH([CS_VCHK_RMALL([$3])], [i])'`
])
cs_cv_prog_$4_version_ok=''
CS_VCHK_RUNTH([CS_VCHK_PATCOUNT([$3])],
[test -z "$cs_cv_prog_$4_version_ok" && { expr "$cs_prog_$4_is_ver_[]i" "$5" "$cs_prog_$4_min_ver_[]i" >/dev/null || cs_cv_prog_$4_version_ok=no ; }
test -z "$cs_cv_prog_$4_version_ok" && { expr "$cs_prog_$4_min_ver_[]i" "$5" "$cs_prog_$4_is_ver_[]i" >/dev/null || cs_cv_prog_$4_version_ok=yes ; }
])
AS_IF([test -z "$cs_cv_prog_$4_version_ok"], [cs_cv_prog_$4_version_ok=yes])
cs_cv_prog_$4_version_ok_annotated="$cs_cv_prog_$4_version_ok"
AS_IF([test -n "$cs_prog_$4_is_version"],
    [cs_cv_prog_$4_version_ok_annotated="$cs_cv_prog_$4_version_ok_annotated (version $cs_prog_$4_is_version)"])
])

##############################################################################
# CS_CHECK_PROG_VERSION(PROG, EXTRACT_CALL, VERSION, PATTERN,
#                       [ACTION-IF-OKAY], [ACTION-IF-NOT-OKAY], [CMP])
# Check the version of a program PROG.
# Version information is emitted by EXTRACT_CALL (for instance "bison -V").
# The discovered program version is compared against VERSION.
# The pattern of the version string matches PATTERN
# The extracted version and the supplied version are compared with the CMP
# operator. i.e. EXTRACTED_VERSION CMP SUPPLIED_VERSION
# CMP defaults to >= if not specified.
# ACTION-IF-OKAY is invoked if comparision yields true, otherwise
# ACTION-IF-NOT-OKAY is invoked.
#
# PATTERN literals: 9 .. marks a non empty sequence of digits
#                   _ .. marks a non empty sequence of characters from [a-zA-Z]
#                   | .. everything behind is optional
#                     .. everything else is taken as separator - it is better
#                        to not try stuff like space, slash or comma.
#
# The test results in cs_cv_prog_PROG_version_ok being either yes or no.
##############################################################################
AC_DEFUN([CS_CHECK_PROG_VERSION],
[AC_CACHE_CHECK([if $1 version m4_default([$7],[>=]) $3],
    [AS_TR_SH([cs_cv_prog_$1_version_ok_annotated])],
    [CS_VCHK_EXTRACTVERSION([$2], [$3], [$4], AS_TR_SH([$1]),
	m4_default([$7],[>=]))])
AS_IF([test "$AS_TR_SH([cs_cv_prog_$1_version_ok])" = yes], [$5], [$6])])
# qualify.m4                                                   -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_SYMBOL_QUALIFIER(MESSAGE, CACHE-VAR, QUALIFIERS, [SYMBOL], [LANG],
#		      [ACTION-IF-ACCEPTED], [ACTION-IF-NOT-ACCEPTED])
#	Test if a symbol can be qualified by one of the elements of the
#	comma-separated list of QUALIFIERS.  Examples of qualifiers include
#	__attribute__((deprecated)), __declspec(dllimport), etc. MESSAGE is the
#	"checking" message. CACHE-VAR is the variable which receives the
#	qualifier which succeeded, or the the literal "no" if none were
#	accepted. SYMBOL is the symbol to which the qualifier should be
#	applied. If omitted, then SYMBOL defaults to "void f();". LANG is the
#	language of the test, typically "C" or "C++". It defaults to "C" if
#	omitted. ACTION-IF-ACCEPTED is invoked after CACHE-VAR is set if one of
#	the qualifiers is accepted, else ACTION-IF-NOT-ACCEPTED is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_SYMBOL_QUALIFIER],
    [AC_CACHE_CHECK([$1], [$2],
	[$2='no'
	m4_foreach([cs_symbol_qualifier], [$3],
	    [AS_IF([test "$$2" = no],
		[CS_BUILD_IFELSE(
		    [AC_LANG_PROGRAM(
			[cs_symbol_qualifier m4_default([$4],[void f()]);],
			[])],
		    [], [$5], [$2='cs_symbol_qualifier'], [$2='no'])])])])
    AS_IF([test $$2 != no], [$6], [$7])])
# split.m4                                                     -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_SPLIT(LINE, [OUTPUT-VARIABLES], [DELIMITER], [FILLER])
#	Split LINE into individual tokens.  Tokens are delimited by DELIMITER,
#	which is the space character if omitted.  OUTPUT-VARIABLES is a
#	comma-delimited list of shell variables which should receive the
#	extracted tokens.  If there are too few tokens to fill the output
#	variables, then the excess variables will be assigned the empty string.
#	If there are too few output variables, then the excess tokens will be
#	ignored.  If OUTPUT-VARIABLES is omitted, then the split tokens will be
#	assigned to the shell meta-variables $1, $2, $3, etc.  When
#	OUTPUT-VARIABLES is omitted, FILLER is assigned to meta-variables in
#	cases where DELIMITER delimits a zero-length token.  FILLER defaults
#	to "filler".  For example, if DELIMITER is "+" and OUTPUT-VARIABLES is
#	omitted, given the line "one++three", $1 will be "one", $2 will be
#	"filler", and $3 will be "three".
#------------------------------------------------------------------------------
AC_DEFUN([CS_SPLIT],
    [m4_define([cs_split_filler], m4_default([$4],[filler]))
    set cs_split_filler `echo "$1" | awk 'BEGIN { FS="m4_default([$3],[ ])" }
	{ for (i=1; i <= NF; ++i)
	    { if ($i == "") print "cs_split_filler"; else print $i } }'`
    shift
    m4_map([_CS_SPLIT], [$2])])

AC_DEFUN([_CS_SPLIT],
    [AS_IF([test $[@%:@] -eq 0], [$1=''],
	[AS_IF([test "$[1]" = cs_split_filler], [$1=''], [$1=$[1]])
	shift])])
# textcache.m4                                                 -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# Text cache facility.  These macros provide a way to incrementally store
# arbitrary text in a shell variable, and to write the saved text to a file.
#
# CS_TEXT_CACHE_APPEND(VARIABLE, TEXT)
#	Append text to the contents of the named shell variable.  If the text
#	contains references to shell variables (such as $foo), then those
#	references will be expanded.  If expansion is not desired, then protect
#	the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_PREPEND(VARIABLE, TEXT)
#	Prepend text to the contents of the named shell variable.  If the text
#	contains references to shell variables (such as $foo), then those
#	references will be expanded.  If expansion is not desired, then protect
#	the text with AS_ESCAPE().
#
# CS_TEXT_CACHE_OUTPUT(VARIABLE, FILENAME)
#	Instruct config.status to write the contents of the named shell
#	variable to the given filename.  If the file resides in a directory,
#	the directory will be created, if necessary.  If the output file
#	already exists, and if the cached text is identical to the contents of
#	the existing file, then the existing file is left alone, thus its time
#	stamp remains unmolested.  This heuristic may help to minimize rebuilds
#	when the file is listed as a dependency in a makefile.
#
# *NOTE*
#	There is a bug in Autoconf 2.57 and probably all earlier 2.5x versions
#	which results in errors if AC_CONFIG_COMMANDS is invoked for a `tag'
#	which represents a file in a directory which does not yet exist.
#	Unfortunately, even invoking AS_MKDIR_P in the `cmd' portion of
#	AC_CONFIG_COMMANDS does not solve the problem because the generated
#	configure script attempts to access information about the directory
#	before AS_MKDIR_P has a chance to create it.  This forces us to invoke
#	AS_MKDIR_P in the third argument to AC_CONFIG_COMMANDS (the
#	`init-cmds') rather than the second (the `cmds').  This is undesirable
#	because it means that the directory will be created anytime
#	config.status is invoked (even for a simple --help), rather than being
#	created only when requested to output the text cache.  This bug was
#	submitted to the Autoconf GNATS database by Eric Sunshine as #228 on
#	27-Dec-2002.  It was fixed for Autoconf 2.58 on 26-Sep-2003.  The
#	official fix makes the assumption that `tag' always represents a file
#	(as opposed to some generic target), and creates the file's directory
#	is not present.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TEXT_CACHE_APPEND], [$1="${$1}$2"])
AC_DEFUN([CS_TEXT_CACHE_PREPEND], [$1="$2${$1}"])
AC_DEFUN([CS_TEXT_CACHE_OUTPUT],
    [AC_CONFIG_COMMANDS([$2],
	[echo $ECHO_N "$$1$ECHO_C" > $tmp/tcache
	AS_IF([diff $2 $tmp/tcache >/dev/null 2>&1],
	    [AC_MSG_NOTICE([$2 is unchanged])],
	    [rm -f $2
	    cp $tmp/tcache $2])
	rm -f $tmp/tcache],
	[$1='$$1'
	cs_dir=`AS_DIRNAME([$2])`
	AS_ESCAPE(AS_MKDIR_P([$cs_dir]), [$`\])])])
# trim.m4                                                      -*- Autoconf -*-
#==============================================================================
# Copyright (C)2003 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_TRIM(STRING)
#	Strip leading and trailing spaces from STRING and collapse internal
#	runs of multiple spaces to a single space.
#------------------------------------------------------------------------------
AC_DEFUN([CS_TRIM], [`echo x$1 | sed 's/^x//;s/   */ /g;s/^ //;s/ $//'`])
# warnings.m4                                                  -*- Autoconf -*-
#==============================================================================
# Copyright (C)2005 by Eric Sunshine <sunshine@sunshineco.com>
#
#    This library is free software; you can redistribute it and/or modify it
#    under the terms of the GNU Library General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or (at your
#    option) any later version.
#
#    This library is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
#    License for more details.
#
#    You should have received a copy of the GNU Library General Public License
#    along with this library; if not, write to the Free Software Foundation,
#    Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#==============================================================================
AC_PREREQ([2.56])

#------------------------------------------------------------------------------
# CS_COMPILER_WARNINGS([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                      [ACTION-IF-NOT-FOUND])
#	Check how to enable compilation warnings. If LANGUAGE is not provided,
#	then `C' is assumed (other options include `C++').  If CACHE-VAR is not
#	provided, then it defaults to the name
#	"cs_cv_prog_compiler_enable_warnings".  If an option for enabling
#	warnings (such as `-Wall') is discovered, then it is assigned to
#	CACHE-VAR and ACTION-IF-FOUND is invoked; otherwise the empty string is
#	assigned to CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#
# IMPLEMENTATION NOTES
#
#	On some platforms, it is more appropriate to use -Wmost rather than
#	-Wall even if the compiler understands both, thus we attempt -Wmost
#	before -Wall.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_WARNINGS],
    [CS_CHECK_BUILD_FLAGS(
	[how to enable m4_default([$1],[C]) compilation warnings],
	[m4_default([$2],[cs_cv_prog_compiler_enable_warnings])],
	[CS_CREATE_TUPLE([-Wmost]) CS_CREATE_TUPLE([-Wall])],
	[$1], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_COMPILER_ERRORS([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                    [ACTION-IF-NOT-FOUND])
#	Check how to promote compilation diganostics from warning to error
#	status. If LANGUAGE is not provided, then `C' is assumed (other options
#	include `C++').  If CACHE-VAR is not provided, then it defaults to the
#	name "cs_cv_prog_compiler_enable_errors".  If an option for performing
#	this promotion (such as `-Werror') is discovered, then it is assigned
#	to CACHE-VAR and ACTION-IF-FOUND is invoked; otherwise the empty string
#	is assigned to CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_ERRORS],
    [CS_CHECK_BUILD_FLAGS(
	[how to treat m4_default([$1],[C]) warnings as errors],
	[m4_default([$2],[cs_cv_prog_compiler_enable_errors])],
	[CS_CREATE_TUPLE([-Werror])], [$1], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_COMPILER_IGNORE_UNUSED([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                           [ACTION-IF-NOT-FOUND])
#	Check how to instruct compiler to ignore unused variables and
#	arguments.  This option may be useful for code generated by tools, such
#	as Swig, Bison, and Flex, over which the client has no control, yet
#	wishes to compile without excessive diagnostic spew.  If LANGUAGE is
#	not provided, then `C' is assumed (other options include `C++').  If
#	CACHE-VAR is not provided, then it defaults to the name
#	"cs_cv_prog_compiler_ignore_unused".  If an option (such as
#	`-Wno-unused') is discovered, then it is assigned to CACHE-VAR and
#	ACTION-IF-FOUND is invoked; otherwise the empty string is assigned to
#	CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_IGNORE_UNUSED],
    [CS_CHECK_BUILD_FLAGS(
	[how to suppress m4_default([$1],[C]) unused variable warnings],
	[m4_default([$2],[cs_cv_prog_compiler_ignore_unused])],
	[CS_CREATE_TUPLE([-Wno-unused])], [$1], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_COMPILER_IGNORE_UNINITIALIZED([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                                  [ACTION-IF-NOT-FOUND])
#	Check how to instruct compiler to ignore uninitialized variables.  This
#	option may be useful for code generated by tools, such as Swig, Bison,
#	and Flex, over which the client has no control, yet wishes to compile
#	without excessive diagnostic spew.  If LANGUAGE is not provided, then
#	`C' is assumed (other options include `C++').  If CACHE-VAR is not
#	provided, then it defaults to the name
#	"cs_cv_prog_compiler_ignore_uninitialized".  If an option (such as
#	`-Wno-uninitialized') is discovered, then it is assigned to CACHE-VAR
#	and ACTION-IF-FOUND is invoked; otherwise the empty string is assigned
#	to CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_IGNORE_UNINITIALIZED],
    [CS_CHECK_BUILD_FLAGS(
	[how to suppress m4_default([$1],[C]) uninitialized warnings],
	[m4_default([$2],
	    [cs_cv_prog_compiler_ignore_uninitialized_variables])],
	[CS_CREATE_TUPLE([-Wno-uninitialized])], [$1], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_COMPILER_IGNORE_PRAGMAS([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                            [ACTION-IF-NOT-FOUND])
#	Check how to instruct compiler to ignore unrecognized #pragma
#	directives.  This option may be useful for code which contains
#	unprotected #pragmas which are not understood by all compilers.  If
#	LANGUAGE is not provided, then `C' is assumed (other options include
#	`C++').  If CACHE-VAR is not provided, then it defaults to the name
#	"cs_cv_prog_compiler_ignore_unknown_pragmas".  If an option (such as
#	`-Wno-unknown-pragmas') is discovered, then it is assigned to CACHE-VAR
#	and ACTION-IF-FOUND is invoked; otherwise the empty string is assigned
#	to CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_IGNORE_PRAGMAS],
    [CS_CHECK_BUILD_FLAGS(
	[how to suppress m4_default([$1],[C]) unknown [#pragma] warnings],
	[m4_default([$2],[cs_cv_prog_compiler_ignore_unknown_pragmas])],
	[CS_CREATE_TUPLE([-Wno-unknown-pragmas])], [$1], [$3], [$4])])



#------------------------------------------------------------------------------
# CS_COMPILER_IGNORE_LONG_DOUBLE([LANGUAGE], [CACHE-VAR], [ACTION-IF-FOUND],
#                                [ACTION-IF-NOT-FOUND])
#	Check how to instruct compiler to suppress warnings about `long double'
#	usage.  This option may be useful for code generated by tools, such as
#	Swig, Bison, and Flex, over which the client has no control, yet wishes
#	to compile without excessive diagnostic spew.  If LANGUAGE is not
#	provided, then `C' is assumed (other options include `C++').  If
#	CACHE-VAR is not provided, then it defaults to the name
#	"cs_cv_prog_compiler_ignore_long_double".  If an option (such as
#	`-Wno-long-double') is discovered, then it is assigned to CACHE-VAR and
#	ACTION-IF-FOUND is invoked; otherwise the empty string is assigned to
#	CACHE-VAR and ACTION-IF-NOT-FOUND is invoked.
#------------------------------------------------------------------------------
AC_DEFUN([CS_COMPILER_IGNORE_LONG_DOUBLE],
    [CS_CHECK_BUILD_FLAGS(
	[how to suppress m4_default([$1],[C]) `long double' warnings],
	[m4_default([$2],[cs_cv_prog_compiler_ignore_long_double])],
	[CS_CREATE_TUPLE([-Wno-long-double])], [$1], [$3], [$4])])
