dnl NOTE: This is a modified version of ax_cflags_warn_all_ansi.m4
dnl from the autoconf archive, updated to conform to more recent
dnl autoconf standards. The original header follows...

dnl @synopsis AX_CFLAGS_WARN_ALL_ANSI [(shellvar [,default, [A/NA]])]
dnl
dnl Try to find a compiler option that enables most reasonable
dnl warnings. This macro is directly derived from VL_PROG_CC_WARNINGS
dnl which is split up into two AX_CFLAGS_WARN_ALL and
dnl AX_CFLAGS_WARN_ALL_ANSI
dnl
dnl For the GNU CC compiler it will be -Wall (and -ansi -pedantic) The
dnl result is added to the shellvar being CFLAGS by default.
dnl
dnl Currently this macro knows about GCC, Solaris C compiler, Digital
dnl Unix C compiler, C for AIX Compiler, HP-UX C compiler, IRIX C
dnl compiler, NEC SX-5 (Super-UX 10) C compiler, and Cray J90 (Unicos
dnl 10.0.0.8) C compiler.
dnl
dnl  - $1 shell-variable-to-add-to : CFLAGS
dnl  - $2 add-value-if-not-found : nothing
dnl  - $3 action-if-found : add value to shellvariable
dnl  - $4 action-if-not-found : nothing
dnl
dnl @category C
dnl @author Guido Draheim <guidod@gmx.de>
dnl @version 2003-01-06
dnl @license GPLWithACException

AC_DEFUN([AX_CFLAGS_WARN_ALL_ANSI],[dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cflags_warn_all_ansi])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for maximum ANSI warnings],
VAR,[VAR="no, unknown"
 AC_LANG_PUSH([C])
 ac_save_[]FLAGS="$[]FLAGS"
# IRIX C compiler:
#      -use_readonly_const is the default for IRIX C,
#       puts them into .rodata, but they are copied later.
#       need to be "-G0 -rdatashared" for strictmode but
#       I am not sure what effect that has really.         - guidod
for ac_arg dnl
in "-pedantic  % -Wall -ansi -pedantic"       dnl   GCC
   "-xstrconst % -v -Xc"                      dnl Solaris C
   "-std1      % -verbose -w0 -warnprotos -std1" dnl Digital Unix
   " % -qlanglvl=ansi -qsrcmsg -qinfo=all:noppt:noppc:noobs:nocnd" dnl AIX
   " % -ansi -ansiE -fullwarn"                dnl IRIX
   "+ESlit     % +w1 -Aa"                     dnl HP-UX C
   "-Xc        % -pvctl[,]fullmsg -Xc"        dnl NEC SX-5 (Super-UX 10)
   "-h conform % -h msglevel 2 -h conform"    dnl Cray C (Unicos)
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_POP([C])
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4,[m4_ifval($2,[
        AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $2"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $2"])]) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])

dnl the only difference - the LANG selection... and the default FLAGS

AC_DEFUN([AX_CXXFLAGS_WARN_ALL_ANSI],[dnl
AS_VAR_PUSHDEF([FLAGS],[CXXFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cxxflags_warn_all_ansi])dnl
AC_CACHE_CHECK([m4_ifval($1,$1,FLAGS) for maximum ANSI warnings],
VAR,[VAR="no, unknown"
 AC_LANG_PUSH([C++])
 ac_save_[]FLAGS="$[]FLAGS"
# IRIX C compiler:
#      -use_readonly_const is the default for IRIX C,
#       puts them into .rodata, but they are copied later.
#       need to be "-G0 -rdatashared" for strictmode but
#       I am not sure what effect that has really.         - guidod
for ac_arg dnl
in "-pedantic  % -Wall -ansi -pedantic"       dnl   GCC
   "-xstrconst % -v -Xc"                      dnl Solaris C
   "-std1      % -verbose -w0 -warnprotos -std1" dnl Digital Unix
   " % -qlanglvl=ansi -qsrcmsg -qinfo=all:noppt:noppc:noobs:nocnd" dnl AIX
   " % -ansi -ansiE -fullwarn"                dnl IRIX
   "+ESlit     % +w1 -Aa"                     dnl HP-UX C
   "-Xc        % -pvctl[,]fullmsg -Xc"        dnl NEC SX-5 (Super-UX 10)
   "-h conform % -h msglevel 2 -h conform"    dnl Cray C (Unicos)
   #
do FLAGS="$ac_save_[]FLAGS "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
   [VAR=`echo $ac_arg | sed -e 's,.*% *,,'` ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_POP([C++])
])
case ".$VAR" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4,[m4_ifval($2,[
        AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $2"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $2"])]) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($1,$1,FLAGS) " | grep " $VAR " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($1,$1,FLAGS) does contain $VAR])
   else AC_RUN_LOG([: m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"])
                      m4_ifval($1,$1,FLAGS)="$m4_ifval($1,$1,FLAGS) $VAR"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])
