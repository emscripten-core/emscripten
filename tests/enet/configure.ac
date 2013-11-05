AC_INIT([libenet], [1.3.3])
AC_CONFIG_SRCDIR([include/enet/enet.h])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([enable])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_CHECK_FUNC(gethostbyaddr_r, [AC_DEFINE(HAS_GETHOSTBYADDR_R)])
AC_CHECK_FUNC(gethostbyname_r, [AC_DEFINE(HAS_GETHOSTBYNAME_R)])
AC_CHECK_FUNC(poll, [AC_DEFINE(HAS_POLL)])
AC_CHECK_FUNC(fcntl, [AC_DEFINE(HAS_FCNTL)])
AC_CHECK_FUNC(inet_pton, [AC_DEFINE(HAS_INET_PTON)])
AC_CHECK_FUNC(inet_ntop, [AC_DEFINE(HAS_INET_NTOP)])

AC_CHECK_MEMBER(struct msghdr.msg_flags, [AC_DEFINE(HAS_MSGHDR_FLAGS)], , [#include <sys/socket.h>])

AC_CHECK_TYPE(socklen_t, [AC_DEFINE(HAS_SOCKLEN_T)], , 
              #include <sys/types.h>
              #include <sys/socket.h>
)

AC_EGREP_HEADER(MSG_MAXIOVLEN, /usr/include/sys/socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))
AC_EGREP_HEADER(MSG_MAXIOVLEN, socket.h, AC_DEFINE(ENET_BUFFER_MAXIMUM, [MSG_MAXIOVLEN]))

AC_CONFIG_FILES([Makefile
	libenet.pc])
AC_OUTPUT
