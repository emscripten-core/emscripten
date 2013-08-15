/* This file was produced by running the config_h.SH script, which
 * gets its values from uconfig.sh, which is generally produced by
 * running Configure.
 *
 * Feel free to modify any of this as the need arises.  Note, however,
 * that running config_h.SH again will wipe out any changes you've made.
 * For a more permanent change edit uconfig.sh and rerun config_h.SH.
 */

/* Package name      : perl5
 * Source directory  : .
 * Configuration time: Thu Jan  1 00:00:00 GMT 1970
 * Configured by     : root@localhost
 * Target system     : unknown
 */

#ifndef _config_h_
#define _config_h_

/* LOC_SED:
 *	This symbol holds the complete pathname to the sed program.
 */
#define LOC_SED 	""	/**/

/* HAS_ALARM:
 *	This symbol, if defined, indicates that the alarm routine is
 *	available.
 */
/*#define HAS_ALARM		/ **/

/* HAS_BCMP:
 *	This symbol is defined if the bcmp() routine is available to
 *	compare blocks of memory.
 */
/*#define HAS_BCMP	/ **/

/* HAS_BCOPY:
 *	This symbol is defined if the bcopy() routine is available to
 *	copy blocks of memory.
 */
/*#define HAS_BCOPY	/ **/

/* HAS_BZERO:
 *	This symbol is defined if the bzero() routine is available to
 *	set a memory block to 0.
 */
/*#define HAS_BZERO	/ **/

/* HAS_CHOWN:
 *	This symbol, if defined, indicates that the chown routine is
 *	available.
 */
/*#define HAS_CHOWN		/ **/

/* HAS_CHROOT:
 *	This symbol, if defined, indicates that the chroot routine is
 *	available.
 */
/*#define HAS_CHROOT		/ **/

/* HAS_CHSIZE:
 *	This symbol, if defined, indicates that the chsize routine is available
 *	to truncate files.  You might need a -lx to get this routine.
 */
/*#define	HAS_CHSIZE		/ **/

/* HAS_CRYPT:
 *	This symbol, if defined, indicates that the crypt routine is available
 *	to encrypt passwords and the like.
 */
/*#define HAS_CRYPT		/ **/

/* HAS_CTERMID:
 *	This symbol, if defined, indicates that the ctermid routine is
 *	available to generate filename for terminal.
 */
/*#define HAS_CTERMID		/ **/

/* HAS_CUSERID:
 *	This symbol, if defined, indicates that the cuserid routine is
 *	available to get character login names.
 */
/*#define HAS_CUSERID		/ **/

/* HAS_DBL_DIG:
 *	This symbol, if defined, indicates that this system's <float.h>
 *	or <limits.h> defines the symbol DBL_DIG, which is the number
 *	of significant digits in a double precision number.  If this
 *	symbol is not defined, a guess of 15 is usually pretty good.
 */
/*#define HAS_DBL_DIG 	/ **/

/* HAS_DIFFTIME:
 *	This symbol, if defined, indicates that the difftime routine is
 *	available.
 */
/*#define HAS_DIFFTIME		/ **/

/* HAS_DLERROR:
 *	This symbol, if defined, indicates that the dlerror routine is
 *	available to return a string describing the last error that
 *	occurred from a call to dlopen(), dlclose() or dlsym().
 */
/*#define HAS_DLERROR	/ **/

/* HAS_DUP2:
 *	This symbol, if defined, indicates that the dup2 routine is
 *	available to duplicate file descriptors.
 */
/*#define HAS_DUP2	/ **/

/* HAS_FCHMOD:
 *	This symbol, if defined, indicates that the fchmod routine is available
 *	to change mode of opened files.  If unavailable, use chmod().
 */
/*#define HAS_FCHMOD		/ **/

/* HAS_FCHOWN:
 *	This symbol, if defined, indicates that the fchown routine is available
 *	to change ownership of opened files.  If unavailable, use chown().
 */
/*#define HAS_FCHOWN		/ **/

/* HAS_FCNTL:
 *	This symbol, if defined, indicates to the C program that
 *	the fcntl() function exists.
 */
/*#define HAS_FCNTL		/ **/

/* HAS_FGETPOS:
 *	This symbol, if defined, indicates that the fgetpos routine is
 *	available to get the file position indicator, similar to ftell().
 */
/*#define HAS_FGETPOS	/ **/

/* HAS_FLOCK:
 *	This symbol, if defined, indicates that the flock routine is
 *	available to do file locking.
 */
/*#define HAS_FLOCK		/ **/

/* HAS_FORK:
 *	This symbol, if defined, indicates that the fork routine is
 *	available.
 */
#define HAS_FORK		/**/

/* HAS_FSETPOS:
 *	This symbol, if defined, indicates that the fsetpos routine is
 *	available to set the file position indicator, similar to fseek().
 */
/*#define HAS_FSETPOS	/ **/

/* HAS_GETTIMEOFDAY:
 *	This symbol, if defined, indicates that the gettimeofday() system
 *	call is available for a sub-second accuracy clock. Usually, the file
 *	<sys/resource.h> needs to be included (see I_SYS_RESOURCE).
 *	The type "Timeval" should be used to refer to "struct timeval".
 */
/*#define HAS_GETTIMEOFDAY	/ **/
#ifdef HAS_GETTIMEOFDAY
#define Timeval struct timeval	/* Structure used by gettimeofday() */
#endif

/* HAS_GETGROUPS:
 *	This symbol, if defined, indicates that the getgroups() routine is
 *	available to get the list of process groups.  If unavailable, multiple
 *	groups are probably not supported.
 */
/*#define HAS_GETGROUPS		/ **/

/* HAS_GETLOGIN:
 *	This symbol, if defined, indicates that the getlogin routine is
 *	available to get the login name.
 */
/*#define HAS_GETLOGIN		/ **/

/* HAS_GETPGID:
 *	This symbol, if defined, indicates to the C program that 
 *	the getpgid(pid) function is available to get the
 *	process group id.
 */
/*#define HAS_GETPGID		/ **/

/* HAS_GETPGRP2:
 *	This symbol, if defined, indicates that the getpgrp2() (as in DG/UX)
 *	routine is available to get the current process group.
 */
/*#define HAS_GETPGRP2		/ **/

/* HAS_GETPPID:
 *	This symbol, if defined, indicates that the getppid routine is
 *	available to get the parent process ID.
 */
/*#define HAS_GETPPID		/ **/

/* HAS_GETPRIORITY:
 *	This symbol, if defined, indicates that the getpriority routine is
 *	available to get a process's priority.
 */
/*#define HAS_GETPRIORITY		/ **/

/* HAS_INET_ATON:
 *	This symbol, if defined, indicates to the C program that the
 *	inet_aton() function is available to parse IP address "dotted-quad"
 *	strings.
 */
/*#define HAS_INET_ATON		/ **/

/* HAS_KILLPG:
 *	This symbol, if defined, indicates that the killpg routine is available
 *	to kill process groups.  If unavailable, you probably should use kill
 *	with a negative process number.
 */
/*#define HAS_KILLPG	/ **/

/* HAS_LINK:
 *	This symbol, if defined, indicates that the link routine is
 *	available to create hard links.
 */
/*#define HAS_LINK	/ **/

/* HAS_LOCALECONV:
 *	This symbol, if defined, indicates that the localeconv routine is
 *	available for numeric and monetary formatting conventions.
 */
/*#define HAS_LOCALECONV	/ **/

/* HAS_LOCKF:
 *	This symbol, if defined, indicates that the lockf routine is
 *	available to do file locking.
 */
/*#define HAS_LOCKF		/ **/

/* HAS_LSTAT:
 *	This symbol, if defined, indicates that the lstat routine is
 *	available to do file stats on symbolic links.
 */
/*#define HAS_LSTAT		/ **/

/* HAS_MBLEN:
 *	This symbol, if defined, indicates that the mblen routine is available
 *	to find the number of bytes in a multibye character.
 */
/*#define HAS_MBLEN		/ **/

/* HAS_MBSTOWCS:
 *	This symbol, if defined, indicates that the mbstowcs routine is
 *	available to covert a multibyte string into a wide character string.
 */
/*#define	HAS_MBSTOWCS		/ **/

/* HAS_MBTOWC:
 *	This symbol, if defined, indicates that the mbtowc routine is available
 *	to covert a multibyte to a wide character.
 */
/*#define HAS_MBTOWC		/ **/

/* HAS_MEMCMP:
 *	This symbol, if defined, indicates that the memcmp routine is available
 *	to compare blocks of memory.
 */
#define HAS_MEMCMP	/**/

/* HAS_MEMCPY:
 *	This symbol, if defined, indicates that the memcpy routine is available
 *	to copy blocks of memory.
 */
#define HAS_MEMCPY	/**/

/* HAS_MEMMOVE:
 *	This symbol, if defined, indicates that the memmove routine is available
 *	to copy potentially overlapping blocks of memory. This should be used
 *	only when HAS_SAFE_BCOPY is not defined. If neither is there, roll your
 *	own version.
 */
/*#define HAS_MEMMOVE	/ **/

/* HAS_MEMSET:
 *	This symbol, if defined, indicates that the memset routine is available
 *	to set blocks of memory.
 */
#define HAS_MEMSET	/**/

/* HAS_MKDIR:
 *	This symbol, if defined, indicates that the mkdir routine is available
 *	to create directories.  Otherwise you should fork off a new process to
 *	exec /bin/mkdir.
 */
/*#define HAS_MKDIR		/ **/

/* HAS_MKFIFO:
 *	This symbol, if defined, indicates that the mkfifo routine is
 *	available to create FIFOs. Otherwise, mknod should be able to
 *	do it for you. However, if mkfifo is there, mknod might require
 *	super-user privileges which mkfifo will not.
 */
/*#define HAS_MKFIFO		/ **/

/* HAS_MKTIME:
 *	This symbol, if defined, indicates that the mktime routine is
 *	available.
 */
/*#define HAS_MKTIME		/ **/

/* HAS_MSYNC:
 *	This symbol, if defined, indicates that the msync system call is
 *	available to synchronize a mapped file.
 */
/*#define HAS_MSYNC		/ **/

/* HAS_MUNMAP:
 *	This symbol, if defined, indicates that the munmap system call is
 *	available to unmap a region, usually mapped by mmap().
 */
/*#define HAS_MUNMAP		/ **/

/* HAS_NICE:
 *	This symbol, if defined, indicates that the nice routine is
 *	available.
 */
/*#define HAS_NICE		/ **/

/* HAS_PATHCONF:
 *	This symbol, if defined, indicates that pathconf() is available
 *	to determine file-system related limits and options associated
 *	with a given filename.
 */
/* HAS_FPATHCONF:
 *	This symbol, if defined, indicates that pathconf() is available
 *	to determine file-system related limits and options associated
 *	with a given open file descriptor.
 */
/*#define HAS_PATHCONF		/ **/
/*#define HAS_FPATHCONF		/ **/

/* HAS_PAUSE:
 *	This symbol, if defined, indicates that the pause routine is
 *	available to suspend a process until a signal is received.
 */
/*#define HAS_PAUSE		/ **/

/* HAS_PIPE:
 *	This symbol, if defined, indicates that the pipe routine is
 *	available to create an inter-process channel.
 */
/*#define HAS_PIPE		/ **/

/* HAS_POLL:
 *	This symbol, if defined, indicates that the poll routine is
 *	available to poll active file descriptors.  Please check I_POLL and
 *	I_SYS_POLL to know which header should be included as well.
 */
/*#define HAS_POLL		/ **/

/* HAS_READDIR:
 *	This symbol, if defined, indicates that the readdir routine is
 *	available to read directory entries. You may have to include
 *	<dirent.h>. See I_DIRENT.
 */
#define HAS_READDIR		/**/

/* HAS_SEEKDIR:
 *	This symbol, if defined, indicates that the seekdir routine is
 *	available. You may have to include <dirent.h>. See I_DIRENT.
 */
/*#define HAS_SEEKDIR		/ **/

/* HAS_TELLDIR:
 *	This symbol, if defined, indicates that the telldir routine is
 *	available. You may have to include <dirent.h>. See I_DIRENT.
 */
/*#define HAS_TELLDIR		/ **/

/* HAS_REWINDDIR:
 *	This symbol, if defined, indicates that the rewinddir routine is
 *	available. You may have to include <dirent.h>. See I_DIRENT.
 */
/*#define HAS_REWINDDIR		/ **/

/* HAS_READLINK:
 *	This symbol, if defined, indicates that the readlink routine is
 *	available to read the value of a symbolic link.
 */
/*#define HAS_READLINK		/ **/

/* HAS_RENAME:
 *	This symbol, if defined, indicates that the rename routine is available
 *	to rename files.  Otherwise you should do the unlink(), link(), unlink()
 *	trick.
 */
#define HAS_RENAME	/**/

/* HAS_RMDIR:
 *	This symbol, if defined, indicates that the rmdir routine is
 *	available to remove directories. Otherwise you should fork off a
 *	new process to exec /bin/rmdir.
 */
/*#define HAS_RMDIR		/ **/

/* HAS_SELECT:
 *	This symbol, if defined, indicates that the select routine is
 *	available to select active file descriptors. If the timeout field
 *	is used, <sys/time.h> may need to be included.
 */
/*#define HAS_SELECT	/ **/

/* HAS_SETEGID:
 *	This symbol, if defined, indicates that the setegid routine is available
 *	to change the effective gid of the current program.
 */
/*#define HAS_SETEGID		/ **/

/* HAS_SETEUID:
 *	This symbol, if defined, indicates that the seteuid routine is available
 *	to change the effective uid of the current program.
 */
/*#define HAS_SETEUID		/ **/

/* HAS_SETGROUPS:
 *	This symbol, if defined, indicates that the setgroups() routine is
 *	available to set the list of process groups.  If unavailable, multiple
 *	groups are probably not supported.
 */
/*#define HAS_SETGROUPS		/ **/

/* HAS_SETLINEBUF:
 *	This symbol, if defined, indicates that the setlinebuf routine is
 *	available to change stderr or stdout from block-buffered or unbuffered
 *	to a line-buffered mode.
 */
/*#define HAS_SETLINEBUF		/ **/

/* HAS_SETLOCALE:
 *	This symbol, if defined, indicates that the setlocale routine is
 *	available to handle locale-specific ctype implementations.
 */
/*#define HAS_SETLOCALE	/ **/

/* HAS_SETPGID:
 *	This symbol, if defined, indicates that the setpgid(pid, gpid)
 *	routine is available to set process group ID.
 */
/*#define HAS_SETPGID	/ **/

/* HAS_SETPGRP2:
 *	This symbol, if defined, indicates that the setpgrp2() (as in DG/UX)
 *	routine is available to set the current process group.
 */
/*#define HAS_SETPGRP2		/ **/

/* HAS_SETPRIORITY:
 *	This symbol, if defined, indicates that the setpriority routine is
 *	available to set a process's priority.
 */
/*#define HAS_SETPRIORITY		/ **/

/* HAS_SETREGID:
 *	This symbol, if defined, indicates that the setregid routine is
 *	available to change the real and effective gid of the current
 *	process.
 */
/* HAS_SETRESGID:
 *	This symbol, if defined, indicates that the setresgid routine is
 *	available to change the real, effective and saved gid of the current
 *	process.
 */
/*#define HAS_SETREGID		/ **/
/*#define HAS_SETRESGID		/ **/

/* HAS_SETREUID:
 *	This symbol, if defined, indicates that the setreuid routine is
 *	available to change the real and effective uid of the current
 *	process.
 */
/* HAS_SETRESUID:
 *	This symbol, if defined, indicates that the setresuid routine is
 *	available to change the real, effective and saved uid of the current
 *	process.
 */
/*#define HAS_SETREUID		/ **/
/*#define HAS_SETRESUID		/ **/

/* HAS_SETRGID:
 *	This symbol, if defined, indicates that the setrgid routine is available
 *	to change the real gid of the current program.
 */
/*#define HAS_SETRGID		/ **/

/* HAS_SETRUID:
 *	This symbol, if defined, indicates that the setruid routine is available
 *	to change the real uid of the current program.
 */
/*#define HAS_SETRUID		/ **/

/* HAS_SETSID:
 *	This symbol, if defined, indicates that the setsid routine is
 *	available to set the process group ID.
 */
/*#define HAS_SETSID	/ **/

/* HAS_STRCHR:
 *	This symbol is defined to indicate that the strchr()/strrchr()
 *	functions are available for string searching. If not, try the
 *	index()/rindex() pair.
 */
/* HAS_INDEX:
 *	This symbol is defined to indicate that the index()/rindex()
 *	functions are available for string searching.
 */
/*#define HAS_STRCHR	/ **/
/*#define HAS_INDEX	/ **/

/* HAS_STRCOLL:
 *	This symbol, if defined, indicates that the strcoll routine is
 *	available to compare strings using collating information.
 */
/*#define HAS_STRCOLL	/ **/

/* HAS_STRTOD:
 *	This symbol, if defined, indicates that the strtod routine is
 *	available to provide better numeric string conversion than atof().
 */
/*#define HAS_STRTOD	/ **/

/* HAS_STRTOL:
 *	This symbol, if defined, indicates that the strtol routine is available
 *	to provide better numeric string conversion than atoi() and friends.
 */
/*#define HAS_STRTOL	/ **/

/* HAS_STRXFRM:
 *	This symbol, if defined, indicates that the strxfrm() routine is
 *	available to transform strings.
 */
/*#define HAS_STRXFRM	/ **/

/* HAS_SYMLINK:
 *	This symbol, if defined, indicates that the symlink routine is available
 *	to create symbolic links.
 */
/*#define HAS_SYMLINK	/ **/

/* HAS_SYSCALL:
 *	This symbol, if defined, indicates that the syscall routine is
 *	available to call arbitrary system calls. If undefined, that's tough.
 */
/*#define HAS_SYSCALL	/ **/

/* HAS_SYSCONF:
 *	This symbol, if defined, indicates that sysconf() is available
 *	to determine system related limits and options.
 */
/*#define HAS_SYSCONF	/ **/

/* HAS_SYSTEM:
 *	This symbol, if defined, indicates that the system routine is
 *	available to issue a shell command.
 */
/*#define HAS_SYSTEM	/ **/

/* HAS_TCGETPGRP:
 *	This symbol, if defined, indicates that the tcgetpgrp routine is
 *	available to get foreground process group ID.
 */
/*#define HAS_TCGETPGRP		/ **/

/* HAS_TCSETPGRP:
 *	This symbol, if defined, indicates that the tcsetpgrp routine is
 *	available to set foreground process group ID.
 */
/*#define HAS_TCSETPGRP		/ **/

/* HAS_TRUNCATE:
 *	This symbol, if defined, indicates that the truncate routine is
 *	available to truncate files.
 */
/*#define HAS_TRUNCATE	/ **/

/* HAS_TZNAME:
 *	This symbol, if defined, indicates that the tzname[] array is
 *	available to access timezone names.
 */
/*#define HAS_TZNAME		/ **/

/* HAS_UMASK:
 *	This symbol, if defined, indicates that the umask routine is
 *	available to set and get the value of the file creation mask.
 */
/*#define HAS_UMASK		/ **/

/* HAS_USLEEP:
 *	This symbol, if defined, indicates that the usleep routine is
 *	available to let the process sleep on a sub-second accuracy.
 */
/*#define HAS_USLEEP		/ **/

/* HAS_WAIT4:
 *	This symbol, if defined, indicates that wait4() exists.
 */
/*#define HAS_WAIT4	/ **/

/* HAS_WAITPID:
 *	This symbol, if defined, indicates that the waitpid routine is
 *	available to wait for child process.
 */
/*#define HAS_WAITPID	/ **/

/* HAS_WCSTOMBS:
 *	This symbol, if defined, indicates that the wcstombs routine is
 *	available to convert wide character strings to multibyte strings.
 */
/*#define HAS_WCSTOMBS	/ **/

/* HAS_WCTOMB:
 *	This symbol, if defined, indicates that the wctomb routine is available
 *	to covert a wide character to a multibyte.
 */
/*#define HAS_WCTOMB		/ **/

/* Groups_t:
 *	This symbol holds the type used for the second argument to
 *	getgroups() and setgroups().  Usually, this is the same as
 *	gidtype (gid_t) , but sometimes it isn't.
 *	It can be int, ushort, gid_t, etc... 
 *	It may be necessary to include <sys/types.h> to get any 
 *	typedef'ed information.  This is only required if you have
 *	getgroups() or setgroups()..
 */
#if defined(HAS_GETGROUPS) || defined(HAS_SETGROUPS)
#define Groups_t int	/* Type for 2nd arg to [sg]etgroups() */
#endif

/* I_ARPA_INET:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <arpa/inet.h> to get inet_addr and friends declarations.
 */
/*#define	I_ARPA_INET		/ **/

/* I_DBM:
 *	This symbol, if defined, indicates that <dbm.h> exists and should
 *	be included.
 */
/* I_RPCSVC_DBM:
 *	This symbol, if defined, indicates that <rpcsvc/dbm.h> exists and
 *	should be included.
 */
/*#define I_DBM	/ **/
/*#define I_RPCSVC_DBM	/ **/

/* I_DLFCN:
 *	This symbol, if defined, indicates that <dlfcn.h> exists and should
 *	be included.
 */
/*#define I_DLFCN		/ **/

/* I_FCNTL:
 *	This manifest constant tells the C program to include <fcntl.h>.
 */
/*#define I_FCNTL	/ **/

/* I_FLOAT:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <float.h> to get definition of symbols like DBL_MAX or
 *	DBL_MIN, i.e. machine dependent floating point values.
 */
/*#define I_FLOAT		/ **/

/* I_GDBM:
 *	This symbol, if defined, indicates that <gdbm.h> exists and should
 *	be included.
 */
/*#define I_GDBM	/ **/

/* I_LIMITS:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <limits.h> to get definition of symbols like WORD_BIT or
 *	LONG_MAX, i.e. machine dependant limitations.
 */
/*#define I_LIMITS		/ **/

/* I_LOCALE:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <locale.h>.
 */
/*#define	I_LOCALE		/ **/

/* I_MATH:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <math.h>.
 */
#define I_MATH		/**/

/* I_MEMORY:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <memory.h>.
 */
/*#define I_MEMORY		/ **/

/* I_NETINET_IN:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <netinet/in.h>. Otherwise, you may try <sys/in.h>.
 */
/*#define I_NETINET_IN	/ **/

/* I_SFIO:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sfio.h>.
 */
/*#define	I_SFIO		/ **/

/* I_STDDEF:
 *	This symbol, if defined, indicates that <stddef.h> exists and should
 *	be included.
 */
#define I_STDDEF	/**/

/* I_STDLIB:
 *	This symbol, if defined, indicates that <stdlib.h> exists and should
 *	be included.
 */
#define I_STDLIB		/**/

/* I_STRING:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <string.h> (USG systems) instead of <strings.h> (BSD systems).
 */
#define I_STRING		/**/

/* I_SYS_DIR:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/dir.h>.
 */
/*#define I_SYS_DIR		/ **/

/* I_SYS_FILE:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/file.h> to get definition of R_OK and friends.
 */
/*#define I_SYS_FILE		/ **/

/* I_SYS_IOCTL:
 *	This symbol, if defined, indicates that <sys/ioctl.h> exists and should
 *	be included. Otherwise, include <sgtty.h> or <termio.h>.
 */
/* I_SYS_SOCKIO:
 *	This symbol, if defined, indicates the <sys/sockio.h> should be included
 *	to get socket ioctl options, like SIOCATMARK.
 */
/*#define	I_SYS_IOCTL		/ **/
/*#define I_SYS_SOCKIO	/ **/

/* I_SYS_NDIR:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/ndir.h>.
 */
/*#define I_SYS_NDIR	/ **/

/* I_SYS_PARAM:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/param.h>.
 */
/*#define I_SYS_PARAM		/ **/

/* I_SYS_POLL:
 *	This symbol, if defined, indicates that the program may include
 *	<sys/poll.h>.  When I_POLL is also defined, it's probably safest
 *	to only include <poll.h>.
 */
/*#define I_SYS_POLL	/ **/

/* I_SYS_RESOURCE:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/resource.h>.
 */
/*#define I_SYS_RESOURCE		/ **/

/* I_SYS_SELECT:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/select.h> in order to get definition of struct timeval.
 */
/*#define I_SYS_SELECT	/ **/

/* I_SYS_STAT:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/stat.h>.
 */
#define	I_SYS_STAT		/**/

/* I_SYS_TIMES:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/times.h>.
 */
/*#define	I_SYS_TIMES		/ **/

/* I_SYS_TYPES:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/types.h>.
 */
/*#define	I_SYS_TYPES		/ **/

/* I_SYS_UN:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/un.h> to get UNIX domain socket definitions.
 */
/*#define I_SYS_UN		/ **/

/* I_SYS_WAIT:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/wait.h>.
 */
/*#define I_SYS_WAIT	/ **/

/* I_TERMIO:
 *	This symbol, if defined, indicates that the program should include
 *	<termio.h> rather than <sgtty.h>.  There are also differences in
 *	the ioctl() calls that depend on the value of this symbol.
 */
/* I_TERMIOS:
 *	This symbol, if defined, indicates that the program should include
 *	the POSIX termios.h rather than sgtty.h or termio.h.
 *	There are also differences in the ioctl() calls that depend on the
 *	value of this symbol.
 */
/* I_SGTTY:
 *	This symbol, if defined, indicates that the program should include
 *	<sgtty.h> rather than <termio.h>.  There are also differences in
 *	the ioctl() calls that depend on the value of this symbol.
 */
/*#define I_TERMIO		/ **/
/*#define I_TERMIOS		/ **/
/*#define I_SGTTY		/ **/

/* I_UNISTD:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <unistd.h>.
 */
/*#define I_UNISTD		/ **/

/* I_UTIME:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <utime.h>.
 */
/*#define I_UTIME		/ **/

/* I_VALUES:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <values.h> to get definition of symbols like MINFLOAT or
 *	MAXLONG, i.e. machine dependant limitations.  Probably, you
 *	should use <limits.h> instead, if it is available.
 */
/*#define I_VALUES		/ **/

/* I_VFORK:
 *	This symbol, if defined, indicates to the C program that it should
 *	include vfork.h.
 */
/*#define I_VFORK	/ **/

/* CAN_VAPROTO:
 *	This variable is defined on systems supporting prototype declaration
 *	of functions with a variable number of arguments.
 */
/* _V:
 *	This macro is used to declare function parameters in prototypes for
 *	functions with a variable number of parameters. Use double parentheses.
 *	For example:
 *
 *		int printf _V((char *fmt, ...));
 *
 *	Remember to use the plain simple _() macro when declaring a function
 *	with no variable number of arguments, since it might be possible to
 *	have a non-effect _V() macro and still get prototypes via _().
 */
/*#define CAN_VAPROTO	/ **/
#ifdef CAN_VAPROTO
#define	_V(args) args
#else
#define	_V(args) ()
#endif

/* OSNAME:
 *	This symbol contains the name of the operating system, as determined
 *	by Configure.  You shouldn't rely on it too much; the specific
 *	feature tests from Configure are generally more reliable.
 */
/* OSVERS:
 *	This symbol contains the version of the operating system, as determined
 *	by Configure.  You shouldn't rely on it too much; the specific
 *	feature tests from Configure are generally more reliable.
 */
#define OSNAME "unknown"		/**/
#define OSVERS "unknown"		/**/

/* USE_CROSS_COMPILE:
 *	This symbol, if defined, indicates that Perl is being cross-compiled.
 */
/* PERL_TARGETARCH:
 *	This symbol, if defined, indicates the target architecture
 *	Perl has been cross-compiled to.  Undefined if not a cross-compile.
 */
#ifndef USE_CROSS_COMPILE
/*#define	USE_CROSS_COMPILE	/ **/
#define	PERL_TARGETARCH	""	/**/
#endif

/* MULTIARCH:
 *	This symbol, if defined, signifies that the build
 *	process will produce some binary files that are going to be
 *	used in a cross-platform environment.  This is the case for
 *	example with the NeXT "fat" binaries that contain executables
 *	for several CPUs.
 */
/*#define MULTIARCH		/ **/

/* MEM_ALIGNBYTES:
 *	This symbol contains the number of bytes required to align a
 *	double, or a long double when applicable. Usual values are 2,
 *	4 and 8. The default is eight, for safety.
 */
#if defined(USE_CROSS_COMPILE) || defined(MULTIARCH)
#  define MEM_ALIGNBYTES 8
#else
#define MEM_ALIGNBYTES 4
#endif

/* ARCHLIB:
 *	This variable, if defined, holds the name of the directory in
 *	which the user wants to put architecture-dependent public
 *	library files for perl5.  It is most often a local directory
 *	such as /usr/local/lib.  Programs using this variable must be
 *	prepared to deal with filename expansion.  If ARCHLIB is the
 *	same as PRIVLIB, it is not defined, since presumably the
 *	program already searches PRIVLIB.
 */
/* ARCHLIB_EXP:
 *	This symbol contains the ~name expanded version of ARCHLIB, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
/*#define ARCHLIB "/usr/local/lib/perl5/5.16/unknown"		/ **/
/*#define ARCHLIB_EXP "/usr/local/lib/perl5/5.16/unknown"		/ **/

/* ARCHNAME:
 *	This symbol holds a string representing the architecture name.
 *	It may be used to construct an architecture-dependant pathname
 *	where library files may be held under a private library, for
 *	instance.
 */
#define ARCHNAME "unknown"		/**/

/* BIN:
 *	This symbol holds the path of the bin directory where the package will
 *	be installed. Program must be prepared to deal with ~name substitution.
 */
/* BIN_EXP:
 *	This symbol is the filename expanded version of the BIN symbol, for
 *	programs that do not want to deal with that at run-time.
 */
/* PERL_RELOCATABLE_INC:
 *	This symbol, if defined, indicates that we'd like to relocate entries
 *	in @INC at run time based on the location of the perl binary.
 */
#define BIN "/usr/local/bin"	/**/
#define BIN_EXP "/usr/local/bin"	/**/
/*#define PERL_RELOCATABLE_INC  		/ **/

/* INTSIZE:
 *	This symbol contains the value of sizeof(int) so that the C
 *	preprocessor can make decisions based on it.
 */
/* LONGSIZE:
 *	This symbol contains the value of sizeof(long) so that the C
 *	preprocessor can make decisions based on it.
 */
/* SHORTSIZE:
 *	This symbol contains the value of sizeof(short) so that the C
 *	preprocessor can make decisions based on it.
 */
#define INTSIZE 4		/**/
#define LONGSIZE 4		/**/
#define SHORTSIZE 2		/**/

/* BYTEORDER:
 *	This symbol holds the hexadecimal constant defined in byteorder,
 *	in a UV, i.e. 0x1234 or 0x4321 or 0x12345678, etc...
 *	If the compiler supports cross-compiling or multiple-architecture
 *	binaries (eg. on NeXT systems), use compiler-defined macros to
 *	determine the byte order.
 *	On NeXT 3.2 (and greater), you can build "Fat" Multiple Architecture
 *	Binaries (MAB) on either big endian or little endian machines.
 *	The endian-ness is available at compile-time.  This only matters
 *	for perl, where the config.h can be generated and installed on
 *	one system, and used by a different architecture to build an
 *	extension.  Older versions of NeXT that might not have
 *	defined either *_ENDIAN__ were all on Motorola 680x0 series,
 *	so the default case (for NeXT) is big endian to catch them.
 *	This might matter for NeXT 3.0.
 */
#if defined(USE_CROSS_COMPILE) || defined(MULTIARCH)
#  ifdef __LITTLE_ENDIAN__
#    if LONGSIZE == 4
#      define BYTEORDER 0x1234
#    else
#      if LONGSIZE == 8
#        define BYTEORDER 0x12345678
#      endif
#    endif
#  else
#    ifdef __BIG_ENDIAN__
#      if LONGSIZE == 4
#        define BYTEORDER 0x4321
#      else
#        if LONGSIZE == 8
#          define BYTEORDER 0x87654321
#        endif
#      endif
#    endif
#  endif
#  if !defined(BYTEORDER) && (defined(NeXT) || defined(__NeXT__))
#    define BYTEORDER 0x4321
#  endif
#else
#define BYTEORDER 0x1234	/* large digits for MSB */
#endif /* NeXT */

/* CHARBITS:
 *	This symbol contains the size of a char, so that the C preprocessor
 *	can make decisions based on it.
 */
#define CHARBITS 8		/**/

/* CAT2:
 *	This macro concatenates 2 tokens together.
 */
/* STRINGIFY:
 *	This macro surrounds its token with double quotes.
 */
#if 42 == 1
#define CAT2(a,b)	a/**/b
#define STRINGIFY(a)	"a"
#endif
#if 42 == 42
#define PeRl_CaTiFy(a, b)	a ## b
#define PeRl_StGiFy(a)	#a
#define CAT2(a,b)	PeRl_CaTiFy(a,b)
#define StGiFy(a)	PeRl_StGiFy(a)
#define STRINGIFY(a)	PeRl_StGiFy(a)
#endif
#if 42 != 1 && 42 != 42
#include "Bletch: How does this C preprocessor concatenate tokens?"
#endif

/* CPPSTDIN:
 *	This symbol contains the first part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.	 Typical value of "cc -E" or "/lib/cpp", but it can also
 *	call a wrapper. See CPPRUN.
 */
/* CPPMINUS:
 *	This symbol contains the second part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.  This symbol will have the value "-" if CPPSTDIN needs a minus
 *	to specify standard input, otherwise the value is "".
 */
/* CPPRUN:
 *	This symbol contains the string which will invoke a C preprocessor on
 *	the standard input and produce to standard output. It needs to end
 *	with CPPLAST, after all other preprocessor flags have been specified.
 *	The main difference with CPPSTDIN is that this program will never be a
 *	pointer to a shell wrapper, i.e. it will be empty if no preprocessor is
 *	available directly to the user. Note that it may well be different from
 *	the preprocessor used to compile the C program.
 */
/* CPPLAST:
 *	This symbol is intended to be used along with CPPRUN in the same manner
 *	symbol CPPMINUS is used with CPPSTDIN. It contains either "-" or "".
 */
#define CPPSTDIN "cc -E"
#define CPPMINUS "-"
#define CPPRUN "cc -E"
#define CPPLAST "-"

/* HAS_ACCESS:
 *	This manifest constant lets the C program know that the access()
 *	system call is available to check for accessibility using real UID/GID.
 *	(always present on UNIX.)
 */
/*#define HAS_ACCESS		/ **/

/* HAS_ACCESSX:
 *	This symbol, if defined, indicates that the accessx routine is
 *	available to do extended access checks.
 */
/*#define HAS_ACCESSX		/ **/

/* HAS_ASCTIME_R:
 *	This symbol, if defined, indicates that the asctime_r routine
 *	is available to asctime re-entrantly.
 */
/* ASCTIME_R_PROTO:
 *	This symbol encodes the prototype of asctime_r.
 *	It is zero if d_asctime_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_asctime_r
 *	is defined.
 */
/*#define HAS_ASCTIME_R	   / **/
#define ASCTIME_R_PROTO 0	   /**/

/* HASATTRIBUTE_FORMAT:
 *	Can we handle GCC attribute for checking printf-style formats
 */
/* PRINTF_FORMAT_NULL_OK:
 *	Allows __printf__ format to be null when checking printf-style
 */
/* HASATTRIBUTE_MALLOC:
 *	Can we handle GCC attribute for malloc-style functions.
 */
/* HASATTRIBUTE_NONNULL:
 *	Can we handle GCC attribute for nonnull function parms.
 */
/* HASATTRIBUTE_NORETURN:
 *	Can we handle GCC attribute for functions that do not return
 */
/* HASATTRIBUTE_PURE:
 *	Can we handle GCC attribute for pure functions
 */
/* HASATTRIBUTE_UNUSED:
 *	Can we handle GCC attribute for unused variables and arguments
 */
/* HASATTRIBUTE_DEPRECATED:
 *	Can we handle GCC attribute for marking deprecated APIs
 */
/* HASATTRIBUTE_WARN_UNUSED_RESULT:
 *	Can we handle GCC attribute for warning on unused results
 */
/*#define HASATTRIBUTE_DEPRECATED	/ **/
/*#define HASATTRIBUTE_FORMAT	/ **/
/*#define PRINTF_FORMAT_NULL_OK	/ **/
/*#define HASATTRIBUTE_NORETURN	/ **/
/*#define HASATTRIBUTE_MALLOC	/ **/
/*#define HASATTRIBUTE_NONNULL	/ **/
/*#define HASATTRIBUTE_PURE	/ **/
/*#define HASATTRIBUTE_UNUSED	/ **/
/*#define HASATTRIBUTE_WARN_UNUSED_RESULT	/ **/

/* CASTI32:
 *	This symbol is defined if the C compiler can cast negative
 *	or large floating point numbers to 32-bit ints.
 */
/*#define	CASTI32		/ **/

/* CASTNEGFLOAT:
 *	This symbol is defined if the C compiler can cast negative
 *	numbers to unsigned longs, ints and shorts.
 */
/* CASTFLAGS:
 *	This symbol contains flags that say what difficulties the compiler
 *	has casting odd floating values to unsigned long:
 *		0 = ok
 *		1 = couldn't cast < 0
 *		2 = couldn't cast >= 0x80000000
 *		4 = couldn't cast in argument expression list
 */
/*#define	CASTNEGFLOAT		/ **/
#define CASTFLAGS 0		/**/

/* VOID_CLOSEDIR:
 *	This symbol, if defined, indicates that the closedir() routine
 *	does not return a value.
 */
/*#define VOID_CLOSEDIR		/ **/

/* HASCONST:
 *	This symbol, if defined, indicates that this C compiler knows about
 *	the const type. There is no need to actually test for that symbol
 *	within your programs. The mere use of the "const" keyword will
 *	trigger the necessary tests.
 */
/*#define HASCONST	/ **/
#ifndef HASCONST
#define const
#endif

/* HAS_CRYPT_R:
 *	This symbol, if defined, indicates that the crypt_r routine
 *	is available to crypt re-entrantly.
 */
/* CRYPT_R_PROTO:
 *	This symbol encodes the prototype of crypt_r.
 *	It is zero if d_crypt_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_crypt_r
 *	is defined.
 */
/*#define HAS_CRYPT_R	   / **/
#define CRYPT_R_PROTO 0	   /**/

/* HAS_CSH:
 *	This symbol, if defined, indicates that the C-shell exists.
 */
/* CSH:
 *	This symbol, if defined, contains the full pathname of csh.
 */
/*#define HAS_CSH		/ **/
#ifdef HAS_CSH
#define CSH ""	/**/
#endif

/* HAS_CTERMID_R:
 *	This symbol, if defined, indicates that the ctermid_r routine
 *	is available to ctermid re-entrantly.
 */
/* CTERMID_R_PROTO:
 *	This symbol encodes the prototype of ctermid_r.
 *	It is zero if d_ctermid_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_ctermid_r
 *	is defined.
 */
/*#define HAS_CTERMID_R	   / **/
#define CTERMID_R_PROTO 0	   /**/

/* HAS_CTIME_R:
 *	This symbol, if defined, indicates that the ctime_r routine
 *	is available to ctime re-entrantly.
 */
/* CTIME_R_PROTO:
 *	This symbol encodes the prototype of ctime_r.
 *	It is zero if d_ctime_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_ctime_r
 *	is defined.
 */
/*#define HAS_CTIME_R	   / **/
#define CTIME_R_PROTO 0	   /**/

/* SETUID_SCRIPTS_ARE_SECURE_NOW:
 *	This symbol, if defined, indicates that the bug that prevents
 *	setuid scripts from being secure is not present in this kernel.
 */
/* DOSUID:
 *	This symbol, if defined, indicates that the C program should
 *	check the script that it is executing for setuid/setgid bits, and
 *	attempt to emulate setuid/setgid on systems that have disabled
 *	setuid #! scripts because the kernel can't do it securely.
 *	It is up to the package designer to make sure that this emulation
 *	is done securely.  Among other things, it should do an fstat on
 *	the script it just opened to make sure it really is a setuid/setgid
 *	script, it should make sure the arguments passed correspond exactly
 *	to the argument on the #! line, and it should not trust any
 *	subprocesses to which it must pass the filename rather than the
 *	file descriptor of the script to be executed.
 */
/*#define SETUID_SCRIPTS_ARE_SECURE_NOW	/ **/
/*#define DOSUID		/ **/

/* HAS_DRAND48_R:
 *	This symbol, if defined, indicates that the drand48_r routine
 *	is available to drand48 re-entrantly.
 */
/* DRAND48_R_PROTO:
 *	This symbol encodes the prototype of drand48_r.
 *	It is zero if d_drand48_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_drand48_r
 *	is defined.
 */
/*#define HAS_DRAND48_R	   / **/
#define DRAND48_R_PROTO 0	   /**/

/* HAS_DRAND48_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the drand48() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern double drand48(void);
 */
/*#define	HAS_DRAND48_PROTO	/ **/

/* HAS_EACCESS:
 *	This symbol, if defined, indicates that the eaccess routine is
 *	available to do extended access checks.
 */
/*#define HAS_EACCESS		/ **/

/* HAS_ENDGRENT:
 *	This symbol, if defined, indicates that the getgrent routine is
 *	available for finalizing sequential access of the group database.
 */
/*#define HAS_ENDGRENT		/ **/

/* HAS_ENDGRENT_R:
 *	This symbol, if defined, indicates that the endgrent_r routine
 *	is available to endgrent re-entrantly.
 */
/* ENDGRENT_R_PROTO:
 *	This symbol encodes the prototype of endgrent_r.
 *	It is zero if d_endgrent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endgrent_r
 *	is defined.
 */
/*#define HAS_ENDGRENT_R	   / **/
#define ENDGRENT_R_PROTO 0	   /**/

/* HAS_ENDHOSTENT:
 *	This symbol, if defined, indicates that the endhostent() routine is
 *	available to close whatever was being used for host queries.
 */
/*#define HAS_ENDHOSTENT		/ **/

/* HAS_ENDHOSTENT_R:
 *	This symbol, if defined, indicates that the endhostent_r routine
 *	is available to endhostent re-entrantly.
 */
/* ENDHOSTENT_R_PROTO:
 *	This symbol encodes the prototype of endhostent_r.
 *	It is zero if d_endhostent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endhostent_r
 *	is defined.
 */
/*#define HAS_ENDHOSTENT_R	   / **/
#define ENDHOSTENT_R_PROTO 0	   /**/

/* HAS_ENDNETENT:
 *	This symbol, if defined, indicates that the endnetent() routine is
 *	available to close whatever was being used for network queries.
 */
/*#define HAS_ENDNETENT		/ **/

/* HAS_ENDNETENT_R:
 *	This symbol, if defined, indicates that the endnetent_r routine
 *	is available to endnetent re-entrantly.
 */
/* ENDNETENT_R_PROTO:
 *	This symbol encodes the prototype of endnetent_r.
 *	It is zero if d_endnetent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endnetent_r
 *	is defined.
 */
/*#define HAS_ENDNETENT_R	   / **/
#define ENDNETENT_R_PROTO 0	   /**/

/* HAS_ENDPROTOENT:
 *	This symbol, if defined, indicates that the endprotoent() routine is
 *	available to close whatever was being used for protocol queries.
 */
/*#define HAS_ENDPROTOENT		/ **/

/* HAS_ENDPROTOENT_R:
 *	This symbol, if defined, indicates that the endprotoent_r routine
 *	is available to endprotoent re-entrantly.
 */
/* ENDPROTOENT_R_PROTO:
 *	This symbol encodes the prototype of endprotoent_r.
 *	It is zero if d_endprotoent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endprotoent_r
 *	is defined.
 */
/*#define HAS_ENDPROTOENT_R	   / **/
#define ENDPROTOENT_R_PROTO 0	   /**/

/* HAS_ENDPWENT:
 *	This symbol, if defined, indicates that the getgrent routine is
 *	available for finalizing sequential access of the passwd database.
 */
/*#define HAS_ENDPWENT		/ **/

/* HAS_ENDPWENT_R:
 *	This symbol, if defined, indicates that the endpwent_r routine
 *	is available to endpwent re-entrantly.
 */
/* ENDPWENT_R_PROTO:
 *	This symbol encodes the prototype of endpwent_r.
 *	It is zero if d_endpwent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endpwent_r
 *	is defined.
 */
/*#define HAS_ENDPWENT_R	   / **/
#define ENDPWENT_R_PROTO 0	   /**/

/* HAS_ENDSERVENT:
 *	This symbol, if defined, indicates that the endservent() routine is
 *	available to close whatever was being used for service queries.
 */
/*#define HAS_ENDSERVENT		/ **/

/* HAS_ENDSERVENT_R:
 *	This symbol, if defined, indicates that the endservent_r routine
 *	is available to endservent re-entrantly.
 */
/* ENDSERVENT_R_PROTO:
 *	This symbol encodes the prototype of endservent_r.
 *	It is zero if d_endservent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_endservent_r
 *	is defined.
 */
/*#define HAS_ENDSERVENT_R	   / **/
#define ENDSERVENT_R_PROTO 0	   /**/

/* HAS_FD_SET:
 *	This symbol, when defined, indicates presence of the fd_set typedef
 *	in <sys/types.h>
 */
/*#define HAS_FD_SET	/ **/

/* FLEXFILENAMES:
 *	This symbol, if defined, indicates that the system supports filenames
 *	longer than 14 characters.
 */
/*#define	FLEXFILENAMES		/ **/

/* Gconvert:
 *	This preprocessor macro is defined to convert a floating point
 *	number to a string without a trailing decimal point.  This
 *	emulates the behavior of sprintf("%g"), but is sometimes much more
 *	efficient.  If gconvert() is not available, but gcvt() drops the
 *	trailing decimal point, then gcvt() is used.  If all else fails,
 *	a macro using sprintf("%g") is used. Arguments for the Gconvert
 *	macro are: value, number of digits, whether trailing zeros should
 *	be retained, and the output buffer.
 *	The usual values are:
 *		d_Gconvert='gconvert((x),(n),(t),(b))'
 *		d_Gconvert='gcvt((x),(n),(b))'
 *		d_Gconvert='sprintf((b),"%.*g",(n),(x))'
 *	The last two assume trailing zeros should not be kept.
 */
#define Gconvert(x,n,t,b) sprintf((b),"%.*g",(n),(x))

/* HAS_GETGRENT:
 *	This symbol, if defined, indicates that the getgrent routine is
 *	available for sequential access of the group database.
 */
/*#define HAS_GETGRENT		/ **/

/* HAS_GETGRENT_R:
 *	This symbol, if defined, indicates that the getgrent_r routine
 *	is available to getgrent re-entrantly.
 */
/* GETGRENT_R_PROTO:
 *	This symbol encodes the prototype of getgrent_r.
 *	It is zero if d_getgrent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getgrent_r
 *	is defined.
 */
/*#define HAS_GETGRENT_R	   / **/
#define GETGRENT_R_PROTO 0	   /**/

/* HAS_GETGRGID_R:
 *	This symbol, if defined, indicates that the getgrgid_r routine
 *	is available to getgrgid re-entrantly.
 */
/* GETGRGID_R_PROTO:
 *	This symbol encodes the prototype of getgrgid_r.
 *	It is zero if d_getgrgid_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getgrgid_r
 *	is defined.
 */
/*#define HAS_GETGRGID_R	   / **/
#define GETGRGID_R_PROTO 0	   /**/

/* HAS_GETGRNAM_R:
 *	This symbol, if defined, indicates that the getgrnam_r routine
 *	is available to getgrnam re-entrantly.
 */
/* GETGRNAM_R_PROTO:
 *	This symbol encodes the prototype of getgrnam_r.
 *	It is zero if d_getgrnam_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getgrnam_r
 *	is defined.
 */
/*#define HAS_GETGRNAM_R	   / **/
#define GETGRNAM_R_PROTO 0	   /**/

/* HAS_GETHOSTBYADDR:
 *	This symbol, if defined, indicates that the gethostbyaddr() routine is
 *	available to look up hosts by their IP addresses.
 */
/*#define HAS_GETHOSTBYADDR		/ **/

/* HAS_GETHOSTBYNAME:
 *	This symbol, if defined, indicates that the gethostbyname() routine is
 *	available to look up host names in some data base or other.
 */
/*#define HAS_GETHOSTBYNAME		/ **/

/* HAS_GETHOSTENT:
 *	This symbol, if defined, indicates that the gethostent() routine is
 *	available to look up host names in some data base or another.
 */
/*#define HAS_GETHOSTENT		/ **/

/* HAS_GETHOSTNAME:
 *	This symbol, if defined, indicates that the C program may use the
 *	gethostname() routine to derive the host name.  See also HAS_UNAME
 *	and PHOSTNAME.
 */
/* HAS_UNAME:
 *	This symbol, if defined, indicates that the C program may use the
 *	uname() routine to derive the host name.  See also HAS_GETHOSTNAME
 *	and PHOSTNAME.
 */
/* PHOSTNAME:
 *	This symbol, if defined, indicates the command to feed to the
 *	popen() routine to derive the host name.  See also HAS_GETHOSTNAME
 *	and HAS_UNAME.	Note that the command uses a fully qualified path,
 *	so that it is safe even if used by a process with super-user
 *	privileges.
 */
/* HAS_PHOSTNAME:
 *	This symbol, if defined, indicates that the C program may use the
 *	contents of PHOSTNAME as a command to feed to the popen() routine
 *	to derive the host name.
 */
/*#define HAS_GETHOSTNAME	/ **/
/*#define HAS_UNAME		/ **/
/*#define HAS_PHOSTNAME	/ **/
#ifdef HAS_PHOSTNAME
#define PHOSTNAME "/bin/hostname"	/* How to get the host name */
#endif

/* HAS_GETHOSTBYADDR_R:
 *	This symbol, if defined, indicates that the gethostbyaddr_r routine
 *	is available to gethostbyaddr re-entrantly.
 */
/* GETHOSTBYADDR_R_PROTO:
 *	This symbol encodes the prototype of gethostbyaddr_r.
 *	It is zero if d_gethostbyaddr_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_gethostbyaddr_r
 *	is defined.
 */
/*#define HAS_GETHOSTBYADDR_R	   / **/
#define GETHOSTBYADDR_R_PROTO 0	   /**/

/* HAS_GETHOSTBYNAME_R:
 *	This symbol, if defined, indicates that the gethostbyname_r routine
 *	is available to gethostbyname re-entrantly.
 */
/* GETHOSTBYNAME_R_PROTO:
 *	This symbol encodes the prototype of gethostbyname_r.
 *	It is zero if d_gethostbyname_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_gethostbyname_r
 *	is defined.
 */
/*#define HAS_GETHOSTBYNAME_R	   / **/
#define GETHOSTBYNAME_R_PROTO 0	   /**/

/* HAS_GETHOSTENT_R:
 *	This symbol, if defined, indicates that the gethostent_r routine
 *	is available to gethostent re-entrantly.
 */
/* GETHOSTENT_R_PROTO:
 *	This symbol encodes the prototype of gethostent_r.
 *	It is zero if d_gethostent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_gethostent_r
 *	is defined.
 */
/*#define HAS_GETHOSTENT_R	   / **/
#define GETHOSTENT_R_PROTO 0	   /**/

/* HAS_GETHOST_PROTOS:
 *	This symbol, if defined, indicates that <netdb.h> includes
 *	prototypes for gethostent(), gethostbyname(), and
 *	gethostbyaddr().  Otherwise, it is up to the program to guess
 *	them.  See netdbtype.U for probing for various Netdb_xxx_t types.
 */
/*#define	HAS_GETHOST_PROTOS	/ **/

/* HAS_GETLOGIN_R:
 *	This symbol, if defined, indicates that the getlogin_r routine
 *	is available to getlogin re-entrantly.
 */
/* GETLOGIN_R_PROTO:
 *	This symbol encodes the prototype of getlogin_r.
 *	It is zero if d_getlogin_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getlogin_r
 *	is defined.
 */
/*#define HAS_GETLOGIN_R	   / **/
#define GETLOGIN_R_PROTO 0	   /**/

/* HAS_GETNETBYADDR:
 *	This symbol, if defined, indicates that the getnetbyaddr() routine is
 *	available to look up networks by their IP addresses.
 */
/*#define HAS_GETNETBYADDR		/ **/

/* HAS_GETNETBYNAME:
 *	This symbol, if defined, indicates that the getnetbyname() routine is
 *	available to look up networks by their names.
 */
/*#define HAS_GETNETBYNAME		/ **/

/* HAS_GETNETENT:
 *	This symbol, if defined, indicates that the getnetent() routine is
 *	available to look up network names in some data base or another.
 */
/*#define HAS_GETNETENT		/ **/

/* HAS_GETNETBYADDR_R:
 *	This symbol, if defined, indicates that the getnetbyaddr_r routine
 *	is available to getnetbyaddr re-entrantly.
 */
/* GETNETBYADDR_R_PROTO:
 *	This symbol encodes the prototype of getnetbyaddr_r.
 *	It is zero if d_getnetbyaddr_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getnetbyaddr_r
 *	is defined.
 */
/*#define HAS_GETNETBYADDR_R	   / **/
#define GETNETBYADDR_R_PROTO 0	   /**/

/* HAS_GETNETBYNAME_R:
 *	This symbol, if defined, indicates that the getnetbyname_r routine
 *	is available to getnetbyname re-entrantly.
 */
/* GETNETBYNAME_R_PROTO:
 *	This symbol encodes the prototype of getnetbyname_r.
 *	It is zero if d_getnetbyname_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getnetbyname_r
 *	is defined.
 */
/*#define HAS_GETNETBYNAME_R	   / **/
#define GETNETBYNAME_R_PROTO 0	   /**/

/* HAS_GETNETENT_R:
 *	This symbol, if defined, indicates that the getnetent_r routine
 *	is available to getnetent re-entrantly.
 */
/* GETNETENT_R_PROTO:
 *	This symbol encodes the prototype of getnetent_r.
 *	It is zero if d_getnetent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getnetent_r
 *	is defined.
 */
/*#define HAS_GETNETENT_R	   / **/
#define GETNETENT_R_PROTO 0	   /**/

/* HAS_GETNET_PROTOS:
 *	This symbol, if defined, indicates that <netdb.h> includes
 *	prototypes for getnetent(), getnetbyname(), and
 *	getnetbyaddr().  Otherwise, it is up to the program to guess
 *	them.  See netdbtype.U for probing for various Netdb_xxx_t types.
 */
/*#define	HAS_GETNET_PROTOS	/ **/

/* HAS_GETPAGESIZE:
 *	This symbol, if defined, indicates that the getpagesize system call
 *	is available to get system page size, which is the granularity of
 *	many memory management calls.
 */
/*#define HAS_GETPAGESIZE		/ **/

/* HAS_GETPROTOENT:
 *	This symbol, if defined, indicates that the getprotoent() routine is
 *	available to look up protocols in some data base or another.
 */
/*#define HAS_GETPROTOENT		/ **/

/* HAS_GETPGRP:
 *	This symbol, if defined, indicates that the getpgrp routine is
 *	available to get the current process group.
 */
/* USE_BSD_GETPGRP:
 *	This symbol, if defined, indicates that getpgrp needs one
 *	arguments whereas USG one needs none.
 */
/*#define HAS_GETPGRP		/ **/
/*#define USE_BSD_GETPGRP	/ **/

/* HAS_GETPROTOBYNAME:
 *	This symbol, if defined, indicates that the getprotobyname()
 *	routine is available to look up protocols by their name.
 */
/* HAS_GETPROTOBYNUMBER:
 *	This symbol, if defined, indicates that the getprotobynumber()
 *	routine is available to look up protocols by their number.
 */
/*#define HAS_GETPROTOBYNAME		/ **/
/*#define HAS_GETPROTOBYNUMBER		/ **/

/* HAS_GETPROTOBYNAME_R:
 *	This symbol, if defined, indicates that the getprotobyname_r routine
 *	is available to getprotobyname re-entrantly.
 */
/* GETPROTOBYNAME_R_PROTO:
 *	This symbol encodes the prototype of getprotobyname_r.
 *	It is zero if d_getprotobyname_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getprotobyname_r
 *	is defined.
 */
/*#define HAS_GETPROTOBYNAME_R	   / **/
#define GETPROTOBYNAME_R_PROTO 0	   /**/

/* HAS_GETPROTOBYNUMBER_R:
 *	This symbol, if defined, indicates that the getprotobynumber_r routine
 *	is available to getprotobynumber re-entrantly.
 */
/* GETPROTOBYNUMBER_R_PROTO:
 *	This symbol encodes the prototype of getprotobynumber_r.
 *	It is zero if d_getprotobynumber_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getprotobynumber_r
 *	is defined.
 */
/*#define HAS_GETPROTOBYNUMBER_R	   / **/
#define GETPROTOBYNUMBER_R_PROTO 0	   /**/

/* HAS_GETPROTOENT_R:
 *	This symbol, if defined, indicates that the getprotoent_r routine
 *	is available to getprotoent re-entrantly.
 */
/* GETPROTOENT_R_PROTO:
 *	This symbol encodes the prototype of getprotoent_r.
 *	It is zero if d_getprotoent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getprotoent_r
 *	is defined.
 */
/*#define HAS_GETPROTOENT_R	   / **/
#define GETPROTOENT_R_PROTO 0	   /**/

/* HAS_GETPROTO_PROTOS:
 *	This symbol, if defined, indicates that <netdb.h> includes
 *	prototypes for getprotoent(), getprotobyname(), and
 *	getprotobyaddr().  Otherwise, it is up to the program to guess
 *	them.  See netdbtype.U for probing for various Netdb_xxx_t types.
 */
/*#define	HAS_GETPROTO_PROTOS	/ **/

/* HAS_GETPWENT:
 *	This symbol, if defined, indicates that the getpwent routine is
 *	available for sequential access of the passwd database.
 *	If this is not available, the older getpw() function may be available.
 */
/*#define HAS_GETPWENT		/ **/

/* HAS_GETPWENT_R:
 *	This symbol, if defined, indicates that the getpwent_r routine
 *	is available to getpwent re-entrantly.
 */
/* GETPWENT_R_PROTO:
 *	This symbol encodes the prototype of getpwent_r.
 *	It is zero if d_getpwent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getpwent_r
 *	is defined.
 */
/*#define HAS_GETPWENT_R	   / **/
#define GETPWENT_R_PROTO 0	   /**/

/* HAS_GETPWNAM_R:
 *	This symbol, if defined, indicates that the getpwnam_r routine
 *	is available to getpwnam re-entrantly.
 */
/* GETPWNAM_R_PROTO:
 *	This symbol encodes the prototype of getpwnam_r.
 *	It is zero if d_getpwnam_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getpwnam_r
 *	is defined.
 */
/*#define HAS_GETPWNAM_R	   / **/
#define GETPWNAM_R_PROTO 0	   /**/

/* HAS_GETPWUID_R:
 *	This symbol, if defined, indicates that the getpwuid_r routine
 *	is available to getpwuid re-entrantly.
 */
/* GETPWUID_R_PROTO:
 *	This symbol encodes the prototype of getpwuid_r.
 *	It is zero if d_getpwuid_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getpwuid_r
 *	is defined.
 */
/*#define HAS_GETPWUID_R	   / **/
#define GETPWUID_R_PROTO 0	   /**/

/* HAS_GETSERVENT:
 *	This symbol, if defined, indicates that the getservent() routine is
 *	available to look up network services in some data base or another.
 */
/*#define HAS_GETSERVENT		/ **/

/* HAS_GETSERVBYNAME_R:
 *	This symbol, if defined, indicates that the getservbyname_r routine
 *	is available to getservbyname re-entrantly.
 */
/* GETSERVBYNAME_R_PROTO:
 *	This symbol encodes the prototype of getservbyname_r.
 *	It is zero if d_getservbyname_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getservbyname_r
 *	is defined.
 */
/*#define HAS_GETSERVBYNAME_R	   / **/
#define GETSERVBYNAME_R_PROTO 0	   /**/

/* HAS_GETSERVBYPORT_R:
 *	This symbol, if defined, indicates that the getservbyport_r routine
 *	is available to getservbyport re-entrantly.
 */
/* GETSERVBYPORT_R_PROTO:
 *	This symbol encodes the prototype of getservbyport_r.
 *	It is zero if d_getservbyport_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getservbyport_r
 *	is defined.
 */
/*#define HAS_GETSERVBYPORT_R	   / **/
#define GETSERVBYPORT_R_PROTO 0	   /**/

/* HAS_GETSERVENT_R:
 *	This symbol, if defined, indicates that the getservent_r routine
 *	is available to getservent re-entrantly.
 */
/* GETSERVENT_R_PROTO:
 *	This symbol encodes the prototype of getservent_r.
 *	It is zero if d_getservent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getservent_r
 *	is defined.
 */
/*#define HAS_GETSERVENT_R	   / **/
#define GETSERVENT_R_PROTO 0	   /**/

/* HAS_GETSERV_PROTOS:
 *	This symbol, if defined, indicates that <netdb.h> includes
 *	prototypes for getservent(), getservbyname(), and
 *	getservbyaddr().  Otherwise, it is up to the program to guess
 *	them.  See netdbtype.U for probing for various Netdb_xxx_t types.
 */
/*#define	HAS_GETSERV_PROTOS	/ **/

/* HAS_GETSPNAM_R:
 *	This symbol, if defined, indicates that the getspnam_r routine
 *	is available to getspnam re-entrantly.
 */
/* GETSPNAM_R_PROTO:
 *	This symbol encodes the prototype of getspnam_r.
 *	It is zero if d_getspnam_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_getspnam_r
 *	is defined.
 */
/*#define HAS_GETSPNAM_R	   / **/
#define GETSPNAM_R_PROTO 0	   /**/

/* HAS_GETSERVBYNAME:
 *	This symbol, if defined, indicates that the getservbyname()
 *	routine is available to look up services by their name.
 */
/* HAS_GETSERVBYPORT:
 *	This symbol, if defined, indicates that the getservbyport()
 *	routine is available to look up services by their port.
 */
/*#define HAS_GETSERVBYNAME		/ **/
/*#define HAS_GETSERVBYPORT		/ **/

/* HAS_GMTIME_R:
 *	This symbol, if defined, indicates that the gmtime_r routine
 *	is available to gmtime re-entrantly.
 */
/* GMTIME_R_PROTO:
 *	This symbol encodes the prototype of gmtime_r.
 *	It is zero if d_gmtime_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_gmtime_r
 *	is defined.
 */
/*#define HAS_GMTIME_R	   / **/
#define GMTIME_R_PROTO 0	   /**/

/* HAS_GNULIBC:
 *	This symbol, if defined, indicates to the C program that
 *	the GNU C library is being used.  A better check is to use
 *	the __GLIBC__ and __GLIBC_MINOR__ symbols supplied with glibc.
 */
/*#define HAS_GNULIBC  	/ **/
#if defined(HAS_GNULIBC) && !defined(_GNU_SOURCE)
#   define _GNU_SOURCE
#endif

/* HAS_HTONL:
 *	This symbol, if defined, indicates that the htonl() routine (and
 *	friends htons() ntohl() ntohs()) are available to do network
 *	order byte swapping.
 */
/* HAS_HTONS:
 *	This symbol, if defined, indicates that the htons() routine (and
 *	friends htonl() ntohl() ntohs()) are available to do network
 *	order byte swapping.
 */
/* HAS_NTOHL:
 *	This symbol, if defined, indicates that the ntohl() routine (and
 *	friends htonl() htons() ntohs()) are available to do network
 *	order byte swapping.
 */
/* HAS_NTOHS:
 *	This symbol, if defined, indicates that the ntohs() routine (and
 *	friends htonl() htons() ntohl()) are available to do network
 *	order byte swapping.
 */
/*#define HAS_HTONL		/ **/
/*#define HAS_HTONS		/ **/
/*#define HAS_NTOHL		/ **/
/*#define HAS_NTOHS		/ **/

/* HAS_ISASCII:
 *	This manifest constant lets the C program know that isascii
 *	is available.
 */
/*#define HAS_ISASCII		/ **/

/* HAS_LCHOWN:
 *	This symbol, if defined, indicates that the lchown routine is
 *	available to operate on a symbolic link (instead of following the
 *	link).
 */
/*#define HAS_LCHOWN		/ **/

/* HAS_LOCALTIME_R:
 *	This symbol, if defined, indicates that the localtime_r routine
 *	is available to localtime re-entrantly.
 */
/* LOCALTIME_R_NEEDS_TZSET:
 *	Many libc's localtime_r implementations do not call tzset,
 *	making them differ from localtime(), and making timezone
 *	changes using $ENV{TZ} without explicitly calling tzset
 *	impossible. This symbol makes us call tzset before localtime_r
 */
/*#define LOCALTIME_R_NEEDS_TZSET / **/
#ifdef LOCALTIME_R_NEEDS_TZSET
#define L_R_TZSET tzset(),
#else
#define L_R_TZSET
#endif

/* LOCALTIME_R_PROTO:
 *	This symbol encodes the prototype of localtime_r.
 *	It is zero if d_localtime_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_localtime_r
 *	is defined.
 */
/*#define HAS_LOCALTIME_R	   / **/
#define LOCALTIME_R_PROTO 0	   /**/

/* HAS_LONG_DOUBLE:
 *	This symbol will be defined if the C compiler supports long
 *	doubles.
 */
/* LONG_DOUBLESIZE:
 *	This symbol contains the size of a long double, so that the
 *	C preprocessor can make decisions based on it.  It is only
 *	defined if the system supports long doubles.
 */
/*#define HAS_LONG_DOUBLE		/ **/
#ifdef HAS_LONG_DOUBLE
#define LONG_DOUBLESIZE 8		/**/
#endif

/* HAS_LONG_LONG:
 *	This symbol will be defined if the C compiler supports long long.
 */
/* LONGLONGSIZE:
 *	This symbol contains the size of a long long, so that the
 *	C preprocessor can make decisions based on it.  It is only
 *	defined if the system supports long long.
 */
/*#define HAS_LONG_LONG		/ **/
#ifdef HAS_LONG_LONG
#define LONGLONGSIZE 8		/**/
#endif

/* HAS_LSEEK_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the lseek() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern off_t lseek(int, off_t, int);
 */
/*#define	HAS_LSEEK_PROTO	/ **/

/* HAS_MEMCHR:
 *	This symbol, if defined, indicates that the memchr routine is available
 *	to locate characters within a C string.
 */
#define HAS_MEMCHR	/**/

/* HAS_MKSTEMP:
 *	This symbol, if defined, indicates that the mkstemp routine is
 *	available to exclusively create and open a uniquely named
 *	temporary file.
 */
/*#define HAS_MKSTEMP		/ **/

/* HAS_MMAP:
 *	This symbol, if defined, indicates that the mmap system call is
 *	available to map a file into memory.
 */
/* Mmap_t:
 *	This symbol holds the return type of the mmap() system call
 *	(and simultaneously the type of the first argument).
 *	Usually set to 'void *' or 'caddr_t'.
 */
/*#define HAS_MMAP		/ **/
#define Mmap_t void *	/**/

/* HAS_MSG:
 *	This symbol, if defined, indicates that the entire msg*(2) library is
 *	supported (IPC mechanism based on message queues).
 */
/*#define HAS_MSG		/ **/

/* HAS_OPEN3:
 *	This manifest constant lets the C program know that the three
 *	argument form of open(2) is available.
 */
/*#define HAS_OPEN3		/ **/

/* OLD_PTHREAD_CREATE_JOINABLE:
 *	This symbol, if defined, indicates how to create pthread
 *	in joinable (aka undetached) state.  NOTE: not defined
 *	if pthread.h already has defined PTHREAD_CREATE_JOINABLE
 *	(the new version of the constant).
 *	If defined, known values are PTHREAD_CREATE_UNDETACHED
 *	and __UNDETACHED.
 */
/*#define OLD_PTHREAD_CREATE_JOINABLE  / **/

/* HAS_PTHREAD_ATFORK:
 *	This symbol, if defined, indicates that the pthread_atfork routine
 *	is available to setup fork handlers.
 */
/*#define HAS_PTHREAD_ATFORK		/ **/

/* HAS_PTHREAD_YIELD:
 *	This symbol, if defined, indicates that the pthread_yield
 *	routine is available to yield the execution of the current
 *	thread.	 sched_yield is preferable to pthread_yield.
 */
/* SCHED_YIELD:
 *	This symbol defines the way to yield the execution of
 *	the current thread.  Known ways are sched_yield,
 *	pthread_yield, and pthread_yield with NULL.
 */
/* HAS_SCHED_YIELD:
 *	This symbol, if defined, indicates that the sched_yield
 *	routine is available to yield the execution of the current
 *	thread.	 sched_yield is preferable to pthread_yield.
 */
/*#define HAS_PTHREAD_YIELD	/ **/
#define SCHED_YIELD	sched_yield()	/**/
/*#define HAS_SCHED_YIELD	/ **/

/* HAS_RANDOM_R:
 *	This symbol, if defined, indicates that the random_r routine
 *	is available to random re-entrantly.
 */
/* RANDOM_R_PROTO:
 *	This symbol encodes the prototype of random_r.
 *	It is zero if d_random_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_random_r
 *	is defined.
 */
/*#define HAS_RANDOM_R	   / **/
#define RANDOM_R_PROTO 0	   /**/

/* HAS_READDIR64_R:
 *	This symbol, if defined, indicates that the readdir64_r routine
 *	is available to readdir64 re-entrantly.
 */
/* READDIR64_R_PROTO:
 *	This symbol encodes the prototype of readdir64_r.
 *	It is zero if d_readdir64_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_readdir64_r
 *	is defined.
 */
/*#define HAS_READDIR64_R	   / **/
#define READDIR64_R_PROTO 0	   /**/

/* HAS_READDIR_R:
 *	This symbol, if defined, indicates that the readdir_r routine
 *	is available to readdir re-entrantly.
 */
/* READDIR_R_PROTO:
 *	This symbol encodes the prototype of readdir_r.
 *	It is zero if d_readdir_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_readdir_r
 *	is defined.
 */
/*#define HAS_READDIR_R	   / **/
#define READDIR_R_PROTO 0	   /**/

/* HAS_SAFE_BCOPY:
 *	This symbol, if defined, indicates that the bcopy routine is available
 *	to copy potentially overlapping memory blocks. Normally, you should
 *	probably use memmove() or memcpy(). If neither is defined, roll your
 *	own version.
 */
/*#define HAS_SAFE_BCOPY	/ **/

/* HAS_SAFE_MEMCPY:
 *	This symbol, if defined, indicates that the memcpy routine is available
 *	to copy potentially overlapping memory blocks.  If you need to
 *	copy overlapping memory blocks, you should check HAS_MEMMOVE and
 *	use memmove() instead, if available.
 */
/*#define HAS_SAFE_MEMCPY	/ **/

/* HAS_SANE_MEMCMP:
 *	This symbol, if defined, indicates that the memcmp routine is available
 *	and can be used to compare relative magnitudes of chars with their high
 *	bits set.  If it is not defined, roll your own version.
 */
/*#define HAS_SANE_MEMCMP	/ **/

/* HAS_SEM:
 *	This symbol, if defined, indicates that the entire sem*(2) library is
 *	supported.
 */
/*#define HAS_SEM		/ **/

/* HAS_SETGRENT:
 *	This symbol, if defined, indicates that the setgrent routine is
 *	available for initializing sequential access of the group database.
 */
/*#define HAS_SETGRENT		/ **/

/* HAS_SETGRENT_R:
 *	This symbol, if defined, indicates that the setgrent_r routine
 *	is available to setgrent re-entrantly.
 */
/* SETGRENT_R_PROTO:
 *	This symbol encodes the prototype of setgrent_r.
 *	It is zero if d_setgrent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setgrent_r
 *	is defined.
 */
/*#define HAS_SETGRENT_R	   / **/
#define SETGRENT_R_PROTO 0	   /**/

/* HAS_SETHOSTENT:
 *	This symbol, if defined, indicates that the sethostent() routine is
 *	available.
 */
/*#define HAS_SETHOSTENT		/ **/

/* HAS_SETHOSTENT_R:
 *	This symbol, if defined, indicates that the sethostent_r routine
 *	is available to sethostent re-entrantly.
 */
/* SETHOSTENT_R_PROTO:
 *	This symbol encodes the prototype of sethostent_r.
 *	It is zero if d_sethostent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_sethostent_r
 *	is defined.
 */
/*#define HAS_SETHOSTENT_R	   / **/
#define SETHOSTENT_R_PROTO 0	   /**/

/* HAS_SETLOCALE_R:
 *	This symbol, if defined, indicates that the setlocale_r routine
 *	is available to setlocale re-entrantly.
 */
/* SETLOCALE_R_PROTO:
 *	This symbol encodes the prototype of setlocale_r.
 *	It is zero if d_setlocale_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setlocale_r
 *	is defined.
 */
/*#define HAS_SETLOCALE_R	   / **/
#define SETLOCALE_R_PROTO 0	   /**/

/* HAS_SETNETENT:
 *	This symbol, if defined, indicates that the setnetent() routine is
 *	available.
 */
/*#define HAS_SETNETENT		/ **/

/* HAS_SETNETENT_R:
 *	This symbol, if defined, indicates that the setnetent_r routine
 *	is available to setnetent re-entrantly.
 */
/* SETNETENT_R_PROTO:
 *	This symbol encodes the prototype of setnetent_r.
 *	It is zero if d_setnetent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setnetent_r
 *	is defined.
 */
/*#define HAS_SETNETENT_R	   / **/
#define SETNETENT_R_PROTO 0	   /**/

/* HAS_SETPROTOENT:
 *	This symbol, if defined, indicates that the setprotoent() routine is
 *	available.
 */
/*#define HAS_SETPROTOENT		/ **/

/* HAS_SETPGRP:
 *	This symbol, if defined, indicates that the setpgrp routine is
 *	available to set the current process group.
 */
/* USE_BSD_SETPGRP:
 *	This symbol, if defined, indicates that setpgrp needs two
 *	arguments whereas USG one needs none.  See also HAS_SETPGID
 *	for a POSIX interface.
 */
/*#define HAS_SETPGRP		/ **/
/*#define USE_BSD_SETPGRP	/ **/

/* HAS_SETPROTOENT_R:
 *	This symbol, if defined, indicates that the setprotoent_r routine
 *	is available to setprotoent re-entrantly.
 */
/* SETPROTOENT_R_PROTO:
 *	This symbol encodes the prototype of setprotoent_r.
 *	It is zero if d_setprotoent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setprotoent_r
 *	is defined.
 */
/*#define HAS_SETPROTOENT_R	   / **/
#define SETPROTOENT_R_PROTO 0	   /**/

/* HAS_SETPWENT:
 *	This symbol, if defined, indicates that the setpwent routine is
 *	available for initializing sequential access of the passwd database.
 */
/*#define HAS_SETPWENT		/ **/

/* HAS_SETPWENT_R:
 *	This symbol, if defined, indicates that the setpwent_r routine
 *	is available to setpwent re-entrantly.
 */
/* SETPWENT_R_PROTO:
 *	This symbol encodes the prototype of setpwent_r.
 *	It is zero if d_setpwent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setpwent_r
 *	is defined.
 */
/*#define HAS_SETPWENT_R	   / **/
#define SETPWENT_R_PROTO 0	   /**/

/* HAS_SETSERVENT:
 *	This symbol, if defined, indicates that the setservent() routine is
 *	available.
 */
/*#define HAS_SETSERVENT		/ **/

/* HAS_SETSERVENT_R:
 *	This symbol, if defined, indicates that the setservent_r routine
 *	is available to setservent re-entrantly.
 */
/* SETSERVENT_R_PROTO:
 *	This symbol encodes the prototype of setservent_r.
 *	It is zero if d_setservent_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_setservent_r
 *	is defined.
 */
/*#define HAS_SETSERVENT_R	   / **/
#define SETSERVENT_R_PROTO 0	   /**/

/* HAS_SETVBUF:
 *	This symbol, if defined, indicates that the setvbuf routine is
 *	available to change buffering on an open stdio stream.
 *	to a line-buffered mode.
 */
/*#define HAS_SETVBUF		/ **/

/* HAS_SHM:
 *	This symbol, if defined, indicates that the entire shm*(2) library is
 *	supported.
 */
/*#define HAS_SHM		/ **/

/* Shmat_t:
 *	This symbol holds the return type of the shmat() system call.
 *	Usually set to 'void *' or 'char *'.
 */
/* HAS_SHMAT_PROTOTYPE:
 *	This symbol, if defined, indicates that the sys/shm.h includes
 *	a prototype for shmat().  Otherwise, it is up to the program to
 *	guess one.  Shmat_t shmat(int, Shmat_t, int) is a good guess,
 *	but not always right so it should be emitted by the program only
 *	when HAS_SHMAT_PROTOTYPE is not defined to avoid conflicting defs.
 */
#define Shmat_t void *	/**/
/*#define HAS_SHMAT_PROTOTYPE	/ **/

/* HAS_SIGACTION:
 *	This symbol, if defined, indicates that Vr4's sigaction() routine
 *	is available.
 */
/*#define HAS_SIGACTION	/ **/

/* HAS_SIGSETJMP:
 *	This variable indicates to the C program that the sigsetjmp()
 *	routine is available to save the calling process's registers
 *	and stack environment for later use by siglongjmp(), and
 *	to optionally save the process's signal mask.  See
 *	Sigjmp_buf, Sigsetjmp, and Siglongjmp.
 */
/* Sigjmp_buf:
 *	This is the buffer type to be used with Sigsetjmp and Siglongjmp.
 */
/* Sigsetjmp:
 *	This macro is used in the same way as sigsetjmp(), but will invoke
 *	traditional setjmp() if sigsetjmp isn't available.
 *	See HAS_SIGSETJMP.
 */
/* Siglongjmp:
 *	This macro is used in the same way as siglongjmp(), but will invoke
 *	traditional longjmp() if siglongjmp isn't available.
 *	See HAS_SIGSETJMP.
 */
/*#define HAS_SIGSETJMP	/ **/
#ifdef HAS_SIGSETJMP
#define Sigjmp_buf sigjmp_buf
#define Sigsetjmp(buf,save_mask) sigsetjmp((buf),(save_mask))
#define Siglongjmp(buf,retval) siglongjmp((buf),(retval))
#else
#define Sigjmp_buf jmp_buf
#define Sigsetjmp(buf,save_mask) setjmp((buf))
#define Siglongjmp(buf,retval) longjmp((buf),(retval))
#endif

/* HAS_SOCKET:
 *	This symbol, if defined, indicates that the BSD socket interface is
 *	supported.
 */
/* HAS_SOCKETPAIR:
 *	This symbol, if defined, indicates that the BSD socketpair() call is
 *	supported.
 */
/* HAS_MSG_CTRUNC:
 *	This symbol, if defined, indicates that the MSG_CTRUNC is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_MSG_DONTROUTE:
 *	This symbol, if defined, indicates that the MSG_DONTROUTE is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_MSG_OOB:
 *	This symbol, if defined, indicates that the MSG_OOB is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_MSG_PEEK:
 *	This symbol, if defined, indicates that the MSG_PEEK is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_MSG_PROXY:
 *	This symbol, if defined, indicates that the MSG_PROXY is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_SCM_RIGHTS:
 *	This symbol, if defined, indicates that the SCM_RIGHTS is supported.
 *	Checking just with #ifdef might not be enough because this symbol
 *	has been known to be an enum.
 */
/* HAS_SOCKADDR_SA_LEN:
 *	This symbol, if defined, indicates that the struct sockaddr
 *	structure has a member called sa_len, indicating the length of
 *	the structure.
 */
/* HAS_SOCKADDR_IN6:
 *	This symbol, if defined, indicates the availability of
 *	struct sockaddr_in6;
 */
/* HAS_SIN6_SCOPE_ID:
 *	This symbol, if defined, indicates that the struct sockaddr_in6
 *	structure has a member called sin6_scope_id.
 */
/* HAS_IPV6_MREQ:
 *	This symbol, if defined, indicates the availability of
 *	struct ipv6_mreq;
 */
/*#define	HAS_SOCKET		/ **/
/*#define	HAS_SOCKETPAIR	/ **/
/*#define	HAS_SOCKADDR_SA_LEN	/ **/
/*#define	HAS_MSG_CTRUNC	/ **/
/*#define	HAS_MSG_DONTROUTE	/ **/
/*#define	HAS_MSG_OOB	/ **/
/*#define	HAS_MSG_PEEK	/ **/
/*#define	HAS_MSG_PROXY	/ **/
/*#define	HAS_SCM_RIGHTS	/ **/
/*#define	HAS_SOCKADDR_IN6	/ **/
/*#define	HAS_SIN6_SCOPE_ID	/ **/
/*#define	HAS_IPV6_MREQ	/ **/

/* HAS_SRAND48_R:
 *	This symbol, if defined, indicates that the srand48_r routine
 *	is available to srand48 re-entrantly.
 */
/* SRAND48_R_PROTO:
 *	This symbol encodes the prototype of srand48_r.
 *	It is zero if d_srand48_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_srand48_r
 *	is defined.
 */
/*#define HAS_SRAND48_R	   / **/
#define SRAND48_R_PROTO 0	   /**/

/* HAS_SRANDOM_R:
 *	This symbol, if defined, indicates that the srandom_r routine
 *	is available to srandom re-entrantly.
 */
/* SRANDOM_R_PROTO:
 *	This symbol encodes the prototype of srandom_r.
 *	It is zero if d_srandom_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_srandom_r
 *	is defined.
 */
/*#define HAS_SRANDOM_R	   / **/
#define SRANDOM_R_PROTO 0	   /**/

/* USE_STAT_BLOCKS:
 *	This symbol is defined if this system has a stat structure declaring
 *	st_blksize and st_blocks.
 */
#ifndef USE_STAT_BLOCKS
/*#define USE_STAT_BLOCKS 	/ **/
#endif

/* HAS_STATIC_INLINE:
 *	This symbol, if defined, indicates that the C compiler supports
 *	C99-style static inline.  That is, the function can't be called
 *	from another translation unit.
 */
/* PERL_STATIC_INLINE:
 *	This symbol gives the best-guess incantation to use for static
 *	inline functions.  If HAS_STATIC_INLINE is defined, this will
 *	give C99-style inline.  If HAS_STATIC_INLINE is not defined,
 *	this will give a plain 'static'.  It will always be defined
 *	to something that gives static linkage.
 *	Possibilities include
 *		static inline       (c99)
 *		static __inline__   (gcc -ansi)
 *		static __inline     (MSVC)
 *		static _inline      (older MSVC)
 *		static              (c89 compilers)
 */
/*#define HAS_STATIC_INLINE				/ **/
#define PERL_STATIC_INLINE static	/**/

/* USE_STDIO_PTR:
 *	This symbol is defined if the _ptr and _cnt fields (or similar)
 *	of the stdio FILE structure can be used to access the stdio buffer
 *	for a file handle.  If this is defined, then the FILE_ptr(fp)
 *	and FILE_cnt(fp) macros will also be defined and should be used
 *	to access these fields.
 */
/* FILE_ptr:
 *	This macro is used to access the _ptr field (or equivalent) of the
 *	FILE structure pointed to by its argument. This macro will always be
 *	defined if USE_STDIO_PTR is defined.
 */
/* STDIO_PTR_LVALUE:
 *	This symbol is defined if the FILE_ptr macro can be used as an
 *	lvalue.
 */
/* FILE_cnt:
 *	This macro is used to access the _cnt field (or equivalent) of the
 *	FILE structure pointed to by its argument. This macro will always be
 *	defined if USE_STDIO_PTR is defined.
 */
/* STDIO_CNT_LVALUE:
 *	This symbol is defined if the FILE_cnt macro can be used as an
 *	lvalue.
 */
/* STDIO_PTR_LVAL_SETS_CNT:
 *	This symbol is defined if using the FILE_ptr macro as an lvalue
 *	to increase the pointer by n has the side effect of decreasing the
 *	value of File_cnt(fp) by n.
 */
/* STDIO_PTR_LVAL_NOCHANGE_CNT:
 *	This symbol is defined if using the FILE_ptr macro as an lvalue
 *	to increase the pointer by n leaves File_cnt(fp) unchanged.
 */
/*#define USE_STDIO_PTR 	/ **/
#ifdef USE_STDIO_PTR
#define FILE_ptr(fp)	((fp)->_IO_read_ptr)
/*#define STDIO_PTR_LVALUE 		/ **/
#define FILE_cnt(fp)	((fp)->_IO_read_end - (fp)->_IO_read_ptr)
/*#define STDIO_CNT_LVALUE 		/ **/
/*#define STDIO_PTR_LVAL_SETS_CNT	/ **/
/*#define STDIO_PTR_LVAL_NOCHANGE_CNT	/ **/
#endif

/* USE_STDIO_BASE:
 *	This symbol is defined if the _base field (or similar) of the
 *	stdio FILE structure can be used to access the stdio buffer for
 *	a file handle.  If this is defined, then the FILE_base(fp) macro
 *	will also be defined and should be used to access this field.
 *	Also, the FILE_bufsiz(fp) macro will be defined and should be used
 *	to determine the number of bytes in the buffer.  USE_STDIO_BASE
 *	will never be defined unless USE_STDIO_PTR is.
 */
/* FILE_base:
 *	This macro is used to access the _base field (or equivalent) of the
 *	FILE structure pointed to by its argument. This macro will always be
 *	defined if USE_STDIO_BASE is defined.
 */
/* FILE_bufsiz:
 *	This macro is used to determine the number of bytes in the I/O
 *	buffer pointed to by _base field (or equivalent) of the FILE
 *	structure pointed to its argument. This macro will always be defined
 *	if USE_STDIO_BASE is defined.
 */
/*#define USE_STDIO_BASE 	/ **/
#ifdef USE_STDIO_BASE
#define FILE_base(fp)	((fp)->_IO_read_base)
#define FILE_bufsiz(fp)	((fp)->_IO_read_end - (fp)->_IO_read_base)
#endif

/* USE_STRUCT_COPY:
 *	This symbol, if defined, indicates that this C compiler knows how
 *	to copy structures.  If undefined, you'll need to use a block copy
 *	routine of some sort instead.
 */
/*#define	USE_STRUCT_COPY	/ **/

/* HAS_STRERROR:
 *	This symbol, if defined, indicates that the strerror routine is
 *	available to translate error numbers to strings. See the writeup
 *	of Strerror() in this file before you try to define your own.
 */
/* HAS_SYS_ERRLIST:
 *	This symbol, if defined, indicates that the sys_errlist array is
 *	available to translate error numbers to strings. The extern int
 *	sys_nerr gives the size of that table.
 */
/* Strerror:
 *	This preprocessor symbol is defined as a macro if strerror() is
 *	not available to translate error numbers to strings but sys_errlist[]
 *	array is there.
 */
/*#define HAS_STRERROR		/ **/
/*#define HAS_SYS_ERRLIST	/ **/
#define Strerror(e) strerror(e)

/* HAS_STRERROR_R:
 *	This symbol, if defined, indicates that the strerror_r routine
 *	is available to strerror re-entrantly.
 */
/* STRERROR_R_PROTO:
 *	This symbol encodes the prototype of strerror_r.
 *	It is zero if d_strerror_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_strerror_r
 *	is defined.
 */
/*#define HAS_STRERROR_R	   / **/
#define STRERROR_R_PROTO 0	   /**/

/* HAS_STRTOUL:
 *	This symbol, if defined, indicates that the strtoul routine is
 *	available to provide conversion of strings to unsigned long.
 */
#define HAS_STRTOUL	/**/

/* HAS_TIME:
 *	This symbol, if defined, indicates that the time() routine exists.
 */
/* Time_t:
 *	This symbol holds the type returned by time(). It can be long,
 *	or time_t on BSD sites (in which case <sys/types.h> should be
 *	included).
 */
#define HAS_TIME		/**/
#define Time_t time_t		/* Time type */

/* HAS_TIMES:
 *	This symbol, if defined, indicates that the times() routine exists.
 *	Note that this became obsolete on some systems (SUNOS), which now
 * use getrusage(). It may be necessary to include <sys/times.h>.
 */
/*#define HAS_TIMES		/ **/

/* HAS_TMPNAM_R:
 *	This symbol, if defined, indicates that the tmpnam_r routine
 *	is available to tmpnam re-entrantly.
 */
/* TMPNAM_R_PROTO:
 *	This symbol encodes the prototype of tmpnam_r.
 *	It is zero if d_tmpnam_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_tmpnam_r
 *	is defined.
 */
/*#define HAS_TMPNAM_R	   / **/
#define TMPNAM_R_PROTO 0	   /**/

/* HAS_TTYNAME_R:
 *	This symbol, if defined, indicates that the ttyname_r routine
 *	is available to ttyname re-entrantly.
 */
/* TTYNAME_R_PROTO:
 *	This symbol encodes the prototype of ttyname_r.
 *	It is zero if d_ttyname_r is undef, and one of the
 *	REENTRANT_PROTO_T_ABC macros of reentr.h if d_ttyname_r
 *	is defined.
 */
/*#define HAS_TTYNAME_R	   / **/
#define TTYNAME_R_PROTO 0	   /**/

/* HAS_UNION_SEMUN:
 *	This symbol, if defined, indicates that the union semun is
 *	defined by including <sys/sem.h>.  If not, the user code
 *	probably needs to define it as:
 *	union semun {
 *	    int val;
 *	    struct semid_ds *buf;
 *	    unsigned short *array;
 *	}
 */
/* USE_SEMCTL_SEMUN:
 *	This symbol, if defined, indicates that union semun is
 *	used for semctl IPC_STAT.
 */
/* USE_SEMCTL_SEMID_DS:
 *	This symbol, if defined, indicates that struct semid_ds * is
 *	used for semctl IPC_STAT.
 */
/*#define HAS_UNION_SEMUN	/ **/
/*#define USE_SEMCTL_SEMUN	/ **/
/*#define USE_SEMCTL_SEMID_DS	/ **/

/* HAS_VFORK:
 *	This symbol, if defined, indicates that vfork() exists.
 */
/*#define HAS_VFORK	/ **/

/* HAS_PSEUDOFORK:
 *	This symbol, if defined, indicates that an emulation of the
 *	fork routine is available.
 */
/*#define HAS_PSEUDOFORK	/ **/

/* Signal_t:
 *	This symbol's value is either "void" or "int", corresponding to the
 *	appropriate return type of a signal handler.  Thus, you can declare
 *	a signal handler using "Signal_t (*handler)()", and define the
 *	handler using "Signal_t handler(sig)".
 */
#define Signal_t int	/* Signal handler's return type */

/* HASVOLATILE:
 *	This symbol, if defined, indicates that this C compiler knows about
 *	the volatile declaration.
 */
/*#define	HASVOLATILE	/ **/
#ifndef HASVOLATILE
#define volatile
#endif

/* HAS_VPRINTF:
 *	This symbol, if defined, indicates that the vprintf routine is available
 *	to printf with a pointer to an argument list.  If unavailable, you
 *	may need to write your own, probably in terms of _doprnt().
 */
/* USE_CHAR_VSPRINTF:
 *	This symbol is defined if this system has vsprintf() returning type
 *	(char*).  The trend seems to be to declare it as "int vsprintf()".  It
 *	is up to the package author to declare vsprintf correctly based on the
 *	symbol.
 */
#define HAS_VPRINTF	/**/
/*#define USE_CHAR_VSPRINTF 	/ **/

/* DOUBLESIZE:
 *	This symbol contains the size of a double, so that the C preprocessor
 *	can make decisions based on it.
 */
#define DOUBLESIZE 8		/**/

/* EBCDIC:
 *     This symbol, if defined, indicates that this system uses
 *	EBCDIC encoding.
 */
/*#define	EBCDIC 		/ **/

/* Fpos_t:
 *	This symbol holds the type used to declare file positions in libc.
 *	It can be fpos_t, long, uint, etc... It may be necessary to include
 *	<sys/types.h> to get any typedef'ed information.
 */
#define Fpos_t int		/* File position type */

/* Gid_t_f:
 *	This symbol defines the format string used for printing a Gid_t.
 */
#define	Gid_t_f		"lu"		/**/

/* Gid_t_sign:
 *	This symbol holds the signedess of a Gid_t.
 *	1 for unsigned, -1 for signed.
 */
#define Gid_t_sign	1		/* GID sign */

/* Gid_t_size:
 *	This symbol holds the size of a Gid_t in bytes.
 */
#define Gid_t_size 4		/* GID size */

/* Gid_t:
 *	This symbol holds the return type of getgid() and the type of
 *	argument to setrgid() and related functions.  Typically,
 *	it is the type of group ids in the kernel. It can be int, ushort,
 *	gid_t, etc... It may be necessary to include <sys/types.h> to get
 *	any typedef'ed information.
 */
#define Gid_t int		/* Type for getgid(), etc... */

/* I_DIRENT:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <dirent.h>. Using this symbol also triggers the definition
 *	of the Direntry_t define which ends up being 'struct dirent' or
 *	'struct direct' depending on the availability of <dirent.h>.
 */
/* DIRNAMLEN:
 *	This symbol, if defined, indicates to the C program that the length
 *	of directory entry names is provided by a d_namlen field.  Otherwise
 *	you need to do strlen() on the d_name field.
 */
/* Direntry_t:
 *	This symbol is set to 'struct direct' or 'struct dirent' depending on
 *	whether dirent is available or not. You should use this pseudo type to
 *	portably declare your directory entries.
 */
#define I_DIRENT		/**/
/*#define DIRNAMLEN	/ **/
#define Direntry_t struct dirent

/* I_GRP:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <grp.h>.
 */
/* GRPASSWD:
 *	This symbol, if defined, indicates to the C program that struct group
 *	in <grp.h> contains gr_passwd.
 */
/*#define I_GRP		/ **/
/*#define GRPASSWD	/ **/

/* I_MACH_CTHREADS:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <mach/cthreads.h>.
 */
/*#define   I_MACH_CTHREADS	/ **/

/* I_NDBM:
 *	This symbol, if defined, indicates that <ndbm.h> exists and should
 *	be included.
 */
/* I_GDBMNDBM:
 *	This symbol, if defined, indicates that <gdbm/ndbm.h> exists and should
 *	be included.  This was the location of the ndbm.h compatibility file
 *	in RedHat 7.1.
 */
/* I_GDBM_NDBM:
 *	This symbol, if defined, indicates that <gdbm-ndbm.h> exists and should
 *	be included.  This is the location of the ndbm.h compatibility file
 *	in Debian 4.0.
 */
/* NDBM_H_USES_PROTOTYPES:
 *	This symbol, if defined, indicates that <ndbm.h> uses real ANSI C
 *	prototypes instead of K&R style function declarations without any
 *	parameter information. While ANSI C prototypes are supported in C++,
 *	K&R style function declarations will yield errors.
 */
/* GDBMNDBM_H_USES_PROTOTYPES:
 *	This symbol, if defined, indicates that <gdbm/ndbm.h> uses real ANSI C
 *	prototypes instead of K&R style function declarations without any
 *	parameter information. While ANSI C prototypes are supported in C++,
 *	K&R style function declarations will yield errors.
 */
/* GDBM_NDBM_H_USES_PROTOTYPES:
 *	This symbol, if defined, indicates that <gdbm-ndbm.h> uses real ANSI C
 *	prototypes instead of K&R style function declarations without any
 *	parameter information. While ANSI C prototypes are supported in C++,
 *	K&R style function declarations will yield errors.
 */
/*#define I_NDBM	/ **/
/*#define I_GDBMNDBM	/ **/
/*#define I_GDBM_NDBM	/ **/
/*#define NDBM_H_USES_PROTOTYPES	/ **/
/*#define GDBMNDBM_H_USES_PROTOTYPES	/ **/
/*#define GDBM_NDBM_H_USES_PROTOTYPES	/ **/

/* I_NETDB:
 *	This symbol, if defined, indicates that <netdb.h> exists and
 *	should be included.
 */
/*#define I_NETDB		/ **/

/* I_NET_ERRNO:
 *	This symbol, if defined, indicates that <net/errno.h> exists and
 *	should be included.
 */
/*#define I_NET_ERRNO		/ **/

/* I_PTHREAD:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <pthread.h>.
 */
/*#define   I_PTHREAD	/ **/

/* I_PWD:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <pwd.h>.
 */
/* PWQUOTA:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_quota.
 */
/* PWAGE:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_age.
 */
/* PWCHANGE:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_change.
 */
/* PWCLASS:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_class.
 */
/* PWEXPIRE:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_expire.
 */
/* PWCOMMENT:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_comment.
 */
/* PWGECOS:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_gecos.
 */
/* PWPASSWD:
 *	This symbol, if defined, indicates to the C program that struct passwd
 *	contains pw_passwd.
 */
/*#define I_PWD		/ **/
/*#define PWQUOTA	/ **/
/*#define PWAGE	/ **/
/*#define PWCHANGE	/ **/
/*#define PWCLASS	/ **/
/*#define PWEXPIRE	/ **/
/*#define PWCOMMENT	/ **/
/*#define PWGECOS	/ **/
/*#define PWPASSWD	/ **/

/* I_SYS_ACCESS:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <sys/access.h>.
 */
/*#define   I_SYS_ACCESS                / **/

/* I_SYS_SECURITY:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <sys/security.h>.
 */
/*#define   I_SYS_SECURITY	/ **/

/* I_SYSUIO:
 *	This symbol, if defined, indicates that <sys/uio.h> exists and
 *	should be included.
 */
/*#define	I_SYSUIO		/ **/

/* I_TIME:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <time.h>.
 */
/* I_SYS_TIME:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/time.h>.
 */
/* I_SYS_TIME_KERNEL:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <sys/time.h> with KERNEL defined.
 */
/* HAS_TM_TM_ZONE:
 *	This symbol, if defined, indicates to the C program that
 *	the struct tm has a tm_zone field.
 */
/* HAS_TM_TM_GMTOFF:
 *	This symbol, if defined, indicates to the C program that
 *	the struct tm has a tm_gmtoff field.
 */
#define I_TIME		/**/
/*#define I_SYS_TIME		/ **/
/*#define I_SYS_TIME_KERNEL		/ **/
/*#define HAS_TM_TM_ZONE		/ **/
/*#define HAS_TM_TM_GMTOFF		/ **/

/* I_STDARG:
 *	This symbol, if defined, indicates that <stdarg.h> exists and should
 *	be included.
 */
/* I_VARARGS:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <varargs.h>.
 */
#define I_STDARG		/**/
/*#define I_VARARGS	/ **/

/* PERL_INC_VERSION_LIST:
 *	This variable specifies the list of subdirectories in over
 *	which perl.c:incpush() and lib/lib.pm will automatically
 *	search when adding directories to @INC, in a format suitable
 *	for a C initialization string.  See the inc_version_list entry
 *	in Porting/Glossary for more details.
 */
/*#define PERL_INC_VERSION_LIST NULL		/ **/

/* INSTALL_USR_BIN_PERL:
 *	This symbol, if defined, indicates that Perl is to be installed
 * 	also as /usr/bin/perl.
 */
/*#define INSTALL_USR_BIN_PERL	/ **/

/* Off_t:
 *	This symbol holds the type used to declare offsets in the kernel.
 *	It can be int, long, off_t, etc... It may be necessary to include
 *	<sys/types.h> to get any typedef'ed information.
 */
/* LSEEKSIZE:
 *	This symbol holds the number of bytes used by the Off_t.
 */
/* Off_t_size:
 *	This symbol holds the number of bytes used by the Off_t.
 */
#define Off_t int		/* <offset> type */
#define LSEEKSIZE 4		/* <offset> size */
#define Off_t_size 4	/* <offset> size */

/* Free_t:
 *	This variable contains the return type of free().  It is usually
 * void, but occasionally int.
 */
/* Malloc_t:
 *	This symbol is the type of pointer returned by malloc and realloc.
 */
#define Malloc_t void *			/**/
#define Free_t void			/**/

/* PERL_MALLOC_WRAP:
 *	This symbol, if defined, indicates that we'd like malloc wrap checks.
 */
/*#define PERL_MALLOC_WRAP		/ **/

/* MYMALLOC:
 *	This symbol, if defined, indicates that we're using our own malloc.
 */
/*#define MYMALLOC			/ **/

/* Mode_t:
 *	This symbol holds the type used to declare file modes
 *	for systems calls.  It is usually mode_t, but may be
 *	int or unsigned short.  It may be necessary to include <sys/types.h>
 *	to get any typedef'ed information.
 */
#define Mode_t int	 /* file mode parameter for system calls */

/* VAL_O_NONBLOCK:
 *	This symbol is to be used during open() or fcntl(F_SETFL) to turn on
 *	non-blocking I/O for the file descriptor. Note that there is no way
 *	back, i.e. you cannot turn it blocking again this way. If you wish to
 *	alternatively switch between blocking and non-blocking, use the
 *	ioctl(FIOSNBIO) call instead, but that is not supported by all devices.
 */
/* VAL_EAGAIN:
 *	This symbol holds the errno error code set by read() when no data was
 *	present on the non-blocking file descriptor.
 */
/* RD_NODATA:
 *	This symbol holds the return code from read() when no data is present
 *	on the non-blocking file descriptor. Be careful! If EOF_NONBLOCK is
 *	not defined, then you can't distinguish between no data and EOF by
 *	issuing a read(). You'll have to find another way to tell for sure!
 */
/* EOF_NONBLOCK:
 *	This symbol, if defined, indicates to the C program that a read() on
 *	a non-blocking file descriptor will return 0 on EOF, and not the value
 *	held in RD_NODATA (-1 usually, in that case!).
 */
#define VAL_O_NONBLOCK O_NONBLOCK
#define VAL_EAGAIN EAGAIN
#define RD_NODATA -1
#undef EOF_NONBLOCK

/* Netdb_host_t:
 *	This symbol holds the type used for the 1st argument
 *	to gethostbyaddr().
 */
/* Netdb_hlen_t:
 *	This symbol holds the type used for the 2nd argument
 *	to gethostbyaddr().
 */
/* Netdb_name_t:
 *	This symbol holds the type used for the argument to
 *	gethostbyname().
 */
/* Netdb_net_t:
 *	This symbol holds the type used for the 1st argument to
 *	getnetbyaddr().
 */
#define Netdb_host_t		const char * /**/
#define Netdb_hlen_t		int /**/
#define Netdb_name_t		const char * /**/
#define Netdb_net_t		unsigned long /**/

/* PERL_OTHERLIBDIRS:
 *	This variable contains a colon-separated set of paths for the perl
 *	binary to search for additional library files or modules.
 *	These directories will be tacked to the end of @INC.
 *	Perl will automatically search below each path for version-
 *	and architecture-specific directories.  See PERL_INC_VERSION_LIST
 *	for more details.
 */
/*#define PERL_OTHERLIBDIRS " "		/ **/

/* Pid_t:
 *	This symbol holds the type used to declare process ids in the kernel.
 *	It can be int, uint, pid_t, etc... It may be necessary to include
 *	<sys/types.h> to get any typedef'ed information.
 */
#define Pid_t int		/* PID type */

/* PRIVLIB:
 *	This symbol contains the name of the private library for this package.
 *	The library is private in the sense that it needn't be in anyone's
 *	execution path, but it should be accessible by the world.  The program
 *	should be prepared to do ~ expansion.
 */
/* PRIVLIB_EXP:
 *	This symbol contains the ~name expanded version of PRIVLIB, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
#define PRIVLIB "/usr/local/lib/perl5/5.16"		/**/
#define PRIVLIB_EXP "/usr/local/lib/perl5/5.16"		/**/

/* CAN_PROTOTYPE:
 *	If defined, this macro indicates that the C compiler can handle
 *	function prototypes.
 */
/* _:
 *	This macro is used to declare function parameters for folks who want
 *	to make declarations with prototypes using a different style than
 *	the above macros.  Use double parentheses.  For example:
 *
 *		int main _((int argc, char *argv[]));
 */
/*#define	CAN_PROTOTYPE	/ **/
#ifdef CAN_PROTOTYPE
#define	_(args) args
#else
#define	_(args) ()
#endif

/* PTRSIZE:
 *	This symbol contains the size of a pointer, so that the C preprocessor
 *	can make decisions based on it.  It will be sizeof(void *) if
 *	the compiler supports (void *); otherwise it will be
 *	sizeof(char *).
 */
#define PTRSIZE 4		/**/

/* HAS_QUAD:
 *	This symbol, if defined, tells that there's a 64-bit integer type,
 *	Quad_t, and its unsigned counterpart, Uquad_t. QUADKIND will be one
 *	of QUAD_IS_INT, QUAD_IS_LONG, QUAD_IS_LONG_LONG, QUAD_IS_INT64_T,
 *	or QUAD_IS___INT64.
 */
/*#define HAS_QUAD	/ **/
#ifdef HAS_QUAD
#   define Quad_t int64_t	/**/
#   define Uquad_t uint64_t	/**/
#   define QUADKIND 4	/**/
#   define QUAD_IS_INT	1
#   define QUAD_IS_LONG	2
#   define QUAD_IS_LONG_LONG	3
#   define QUAD_IS_INT64_T	4
#   define QUAD_IS___INT64	5
#endif

/* Drand01:
 *	This macro is to be used to generate uniformly distributed
 *	random numbers over the range [0., 1.[.  You may have to supply
 *	an 'extern double drand48();' in your program since SunOS 4.1.3
 *	doesn't provide you with anything relevant in its headers.
 *	See HAS_DRAND48_PROTO.
 */
/* Rand_seed_t:
 *	This symbol defines the type of the argument of the
 *	random seed function.
 */
/* seedDrand01:
 *	This symbol defines the macro to be used in seeding the
 *	random number generator (see Drand01).
 */
/* RANDBITS:
 *	This symbol indicates how many bits are produced by the
 *	function used to generate normalized random numbers.
 *	Values include 15, 16, 31, and 48.
 */
#define Drand01()		((rand() & 0x7FFF) / (double) ((unsigned long)1 << 15))		/**/
#define Rand_seed_t		int		/**/
#define seedDrand01(x)	srand((Rand_seed_t)x)	/**/
#define RANDBITS		48		/**/

/* Select_fd_set_t:
 *	This symbol holds the type used for the 2nd, 3rd, and 4th
 *	arguments to select.  Usually, this is 'fd_set *', if HAS_FD_SET
 *	is defined, and 'int *' otherwise.  This is only useful if you
 *	have select(), of course.
 */
#define Select_fd_set_t 	int	/**/

/* SH_PATH:
 *	This symbol contains the full pathname to the shell used on this
 *	on this system to execute Bourne shell scripts.  Usually, this will be
 *	/bin/sh, though it's possible that some systems will have /bin/ksh,
 *	/bin/pdksh, /bin/ash, /bin/bash, or even something such as
 *	D:/bin/sh.exe.
 */
#define SH_PATH "/bin/sh"  /**/

/* SIG_NAME:
 *	This symbol contains a list of signal names in order of
 *	signal number. This is intended
 *	to be used as a static array initialization, like this:
 *		char *sig_name[] = { SIG_NAME };
 *	The signals in the list are separated with commas, and each signal
 *	is surrounded by double quotes. There is no leading SIG in the signal
 *	name, i.e. SIGQUIT is known as "QUIT".
 *	Gaps in the signal numbers (up to NSIG) are filled in with NUMnn,
 *	etc., where nn is the actual signal number (e.g. NUM37).
 *	The signal number for sig_name[i] is stored in sig_num[i].
 *	The last element is 0 to terminate the list with a NULL.  This
 *	corresponds to the 0 at the end of the sig_name_init list.
 *	Note that this variable is initialized from the sig_name_init,
 *	not from sig_name (which is unused).
 */
/* SIG_NUM:
 *	This symbol contains a list of signal numbers, in the same order as the
 *	SIG_NAME list. It is suitable for static array initialization, as in:
 *		int sig_num[] = { SIG_NUM };
 *	The signals in the list are separated with commas, and the indices
 *	within that list and the SIG_NAME list match, so it's easy to compute
 *	the signal name from a number or vice versa at the price of a small
 *	dynamic linear lookup.
 *	Duplicates are allowed, but are moved to the end of the list.
 *	The signal number corresponding to sig_name[i] is sig_number[i].
 *	if (i < NSIG) then sig_number[i] == i.
 *	The last element is 0, corresponding to the 0 at the end of
 *	the sig_name_init list.
 *	Note that this variable is initialized from the sig_num_init,
 *	not from sig_num (which is unused).
 */
/* SIG_SIZE:
 *	This variable contains the number of elements of the SIG_NAME
 *	and SIG_NUM arrays, excluding the final NULL entry.
 */
#define SIG_NAME 0		/**/
#define SIG_NUM  0		/**/
#define SIG_SIZE 1			/**/

/* SITEARCH:
 *	This symbol contains the name of the private library for this package.
 *	The library is private in the sense that it needn't be in anyone's
 *	execution path, but it should be accessible by the world.  The program
 *	should be prepared to do ~ expansion.
 *	The standard distribution will put nothing in this directory.
 *	After perl has been installed, users may install their own local
 *	architecture-dependent modules in this directory with
 *		MakeMaker Makefile.PL
 *	or equivalent.  See INSTALL for details.
 */
/* SITEARCH_EXP:
 *	This symbol contains the ~name expanded version of SITEARCH, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
/*#define SITEARCH "/usr/local/lib/perl5/5.16/unknown"		/ **/
/*#define SITEARCH_EXP "/usr/local/lib/perl5/5.16/unknown"		/ **/

/* SITELIB:
 *	This symbol contains the name of the private library for this package.
 *	The library is private in the sense that it needn't be in anyone's
 *	execution path, but it should be accessible by the world.  The program
 *	should be prepared to do ~ expansion.
 *	The standard distribution will put nothing in this directory.
 *	After perl has been installed, users may install their own local
 *	architecture-independent modules in this directory with
 *		MakeMaker Makefile.PL
 *	or equivalent.  See INSTALL for details.
 */
/* SITELIB_EXP:
 *	This symbol contains the ~name expanded version of SITELIB, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
/* SITELIB_STEM:
 *	This define is SITELIB_EXP with any trailing version-specific component
 *	removed.  The elements in inc_version_list (inc_version_list.U) can
 *	be tacked onto this variable to generate a list of directories to search.
 */
#define SITELIB "/usr/local/lib/perl5/5.16"		/**/
#define SITELIB_EXP "/usr/local/lib/perl5/5.16"		/**/
#define SITELIB_STEM "/usr/local/lib/perl5"		/**/

/* Size_t_size:
 *	This symbol holds the size of a Size_t in bytes.
 */
#define Size_t_size 4		/**/

/* Size_t:
 *	This symbol holds the type used to declare length parameters
 *	for string functions.  It is usually size_t, but may be
 *	unsigned long, int, etc.  It may be necessary to include
 *	<sys/types.h> to get any typedef'ed information.
 */
#define Size_t size_t	 /* length parameter for string functions */

/* Sock_size_t:
 *	This symbol holds the type used for the size argument of
 *	various socket calls (just the base type, not the pointer-to).
 */
#define Sock_size_t		int /**/

/* SSize_t:
 *	This symbol holds the type used by functions that return
 *	a count of bytes or an error condition.  It must be a signed type.
 *	It is usually ssize_t, but may be long or int, etc.
 *	It may be necessary to include <sys/types.h> or <unistd.h>
 *	to get any typedef'ed information.
 *	We will pick a type such that sizeof(SSize_t) == sizeof(Size_t).
 */
#define SSize_t int	 /* signed count of bytes */

/* STDCHAR:
 *	This symbol is defined to be the type of char used in stdio.h.
 *	It has the values "unsigned char" or "char".
 */
#define STDCHAR char	/**/

/* Uid_t_f:
 *	This symbol defines the format string used for printing a Uid_t.
 */
#define	Uid_t_f		"lu"		/**/

/* Uid_t_sign:
 *	This symbol holds the signedess of a Uid_t.
 *	1 for unsigned, -1 for signed.
 */
#define Uid_t_sign	1		/* UID sign */

/* Uid_t_size:
 *	This symbol holds the size of a Uid_t in bytes.
 */
#define Uid_t_size 4		/* UID size */

/* Uid_t:
 *	This symbol holds the type used to declare user ids in the kernel.
 *	It can be int, ushort, uid_t, etc... It may be necessary to include
 *	<sys/types.h> to get any typedef'ed information.
 */
#define Uid_t int		/* UID type */

/* USE_ITHREADS:
 *	This symbol, if defined, indicates that Perl should be built to
 *	use the interpreter-based threading implementation.
 */
/* USE_5005THREADS:
 *	This symbol, if defined, indicates that Perl should be built to
 *	use the 5.005-based threading implementation.
 *	Only valid up to 5.8.x.
 */
/* OLD_PTHREADS_API:
 *	This symbol, if defined, indicates that Perl should
 *	be built to use the old draft POSIX threads API.
 */
/* USE_REENTRANT_API:
 *	This symbol, if defined, indicates that Perl should
 *	try to use the various _r versions of library functions.
 *	This is extremely experimental.
 */
/*#define	USE_5005THREADS		/ **/
/*#define	USE_ITHREADS		/ **/
#if defined(USE_5005THREADS) && !defined(USE_ITHREADS)
#define		USE_THREADS		/* until src is revised*/
#endif
/*#define	OLD_PTHREADS_API		/ **/
/*#define	USE_REENTRANT_API	/ **/

/* PERL_VENDORARCH:
 *	If defined, this symbol contains the name of a private library.
 *	The library is private in the sense that it needn't be in anyone's
 *	execution path, but it should be accessible by the world.
 *	It may have a ~ on the front.
 *	The standard distribution will put nothing in this directory.
 *	Vendors who distribute perl may wish to place their own
 *	architecture-dependent modules and extensions in this directory with
 *		MakeMaker Makefile.PL INSTALLDIRS=vendor
 *	or equivalent.  See INSTALL for details.
 */
/* PERL_VENDORARCH_EXP:
 *	This symbol contains the ~name expanded version of PERL_VENDORARCH, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
/*#define PERL_VENDORARCH ""		/ **/
/*#define PERL_VENDORARCH_EXP ""		/ **/

/* PERL_VENDORLIB_EXP:
 *	This symbol contains the ~name expanded version of VENDORLIB, to be used
 *	in programs that are not prepared to deal with ~ expansion at run-time.
 */
/* PERL_VENDORLIB_STEM:
 *	This define is PERL_VENDORLIB_EXP with any trailing version-specific component
 *	removed.  The elements in inc_version_list (inc_version_list.U) can
 *	be tacked onto this variable to generate a list of directories to search.
 */
/*#define PERL_VENDORLIB_EXP ""		/ **/
/*#define PERL_VENDORLIB_STEM ""		/ **/

/* VOIDFLAGS:
 *	This symbol indicates how much support of the void type is given by this
 *	compiler.  What various bits mean:
 *
 *	    1 = supports declaration of void
 *	    2 = supports arrays of pointers to functions returning void
 *	    4 = supports comparisons between pointers to void functions and
 *		    addresses of void functions
 *	    8 = supports declaration of generic void pointers
 *
 *	The package designer should define VOIDUSED to indicate the requirements
 *	of the package.  This can be done either by #defining VOIDUSED before
 *	including config.h, or by defining defvoidused in Myinit.U.  If the
 *	latter approach is taken, only those flags will be tested.  If the
 *	level of void support necessary is not present, defines void to int.
 */
#ifndef VOIDUSED
#define VOIDUSED 1
#endif
#define VOIDFLAGS 1
#if (VOIDFLAGS & VOIDUSED) != VOIDUSED
#define void int		/* is void to be avoided? */
#define M_VOID			/* Xenix strikes again */
#endif

/* PERL_USE_DEVEL:
 *	This symbol, if defined, indicates that Perl was configured with
 *	-Dusedevel, to enable development features.  This should not be
 *	done for production builds.
 */
/*#define	PERL_USE_DEVEL		/ **/

/* HAS_ATOLF:
 *	This symbol, if defined, indicates that the atolf routine is
 *	available to convert strings into long doubles.
 */
/*#define HAS_ATOLF		/ **/

/* HAS_ATOLL:
 *	This symbol, if defined, indicates that the atoll routine is
 *	available to convert strings into long longs.
 */
/*#define HAS_ATOLL		/ **/

/* HAS__FWALK:
 *	This symbol, if defined, indicates that the _fwalk system call is
 *	available to apply a function to all the file handles.
 */
/*#define HAS__FWALK		/ **/

/* HAS_AINTL:
 *	This symbol, if defined, indicates that the aintl routine is
 *	available.  If copysignl is also present we can emulate modfl.
 */
/*#define HAS_AINTL		/ **/

/* HAS_BUILTIN_CHOOSE_EXPR:
 *	Can we handle GCC builtin for compile-time ternary-like expressions
 */
/* HAS_BUILTIN_EXPECT:
 *	Can we handle GCC builtin for telling that certain values are more
 *	likely
 */
/*#define HAS_BUILTIN_EXPECT	/ **/
/*#define HAS_BUILTIN_CHOOSE_EXPR	/ **/

/* HAS_C99_VARIADIC_MACROS:
 *	If defined, the compiler supports C99 variadic macros.
 */
/*#define	HAS_C99_VARIADIC_MACROS	/ **/

/* HAS_CLASS:
 *	This symbol, if defined, indicates that the class routine is
 *	available to classify doubles.  Available for example in AIX.
 *	The returned values are defined in <float.h> and are:
 *
 *	FP_PLUS_NORM	Positive normalized, nonzero
 *	FP_MINUS_NORM	Negative normalized, nonzero
 *	FP_PLUS_DENORM	Positive denormalized, nonzero
 *	FP_MINUS_DENORM	Negative denormalized, nonzero
 *	FP_PLUS_ZERO	+0.0
 *	FP_MINUS_ZERO	-0.0
 *	FP_PLUS_INF	+INF
 *	FP_MINUS_INF	-INF
 *	FP_NANS		Signaling Not a Number (NaNS)
 *	FP_NANQ		Quiet Not a Number (NaNQ)
 */
/*#define HAS_CLASS		/ **/

/* HAS_CLEARENV:
 *	This symbol, if defined, indicates that the clearenv () routine is
 *	available for use.
 */
/*#define HAS_CLEARENV		/ **/

/* HAS_STRUCT_CMSGHDR:
 *	This symbol, if defined, indicates that the struct cmsghdr
 *	is supported.
 */
/*#define HAS_STRUCT_CMSGHDR	/ **/

/* HAS_COPYSIGNL:
 *	This symbol, if defined, indicates that the copysignl routine is
 *	available.  If aintl is also present we can emulate modfl.
 */
/*#define HAS_COPYSIGNL		/ **/

/* USE_CPLUSPLUS:
 *	This symbol, if defined, indicates that a C++ compiler was
 *	used to compiled Perl and will be used to compile extensions.
 */
/*#define USE_CPLUSPLUS		/ **/

/* HAS_DBMINIT_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the dbminit() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern int dbminit(char *);
 */
/*#define	HAS_DBMINIT_PROTO	/ **/

/* HAS_DIR_DD_FD:
 *	This symbol, if defined, indicates that the the DIR* dirstream
 *	structure contains a member variable named dd_fd.
 */
/*#define HAS_DIR_DD_FD		/ **/

/* HAS_DIRFD:
 *	This manifest constant lets the C program know that dirfd
 *	is available.
 */
/*#define HAS_DIRFD		/ **/

/* DLSYM_NEEDS_UNDERSCORE:
 *	This symbol, if defined, indicates that we need to prepend an
 *	underscore to the symbol name before calling dlsym().  This only
 *	makes sense if you *have* dlsym, which we will presume is the
 *	case if you're using dl_dlopen.xs.
 */
/*#define 	DLSYM_NEEDS_UNDERSCORE 	/ **/

/* HAS_FAST_STDIO:
 *	This symbol, if defined, indicates that the "fast stdio"
 *	is available to manipulate the stdio buffers directly.
 */
/*#define HAS_FAST_STDIO		/ **/

/* HAS_FCHDIR:
 *	This symbol, if defined, indicates that the fchdir routine is
 *	available to change directory using a file descriptor.
 */
/*#define HAS_FCHDIR		/ **/

/* FCNTL_CAN_LOCK:
 *	This symbol, if defined, indicates that fcntl() can be used
 *	for file locking.  Normally on Unix systems this is defined.
 *	It may be undefined on VMS.
 */
/*#define FCNTL_CAN_LOCK		/ **/

/* HAS_FINITE:
 *	This symbol, if defined, indicates that the finite routine is
 *	available to check whether a double is finite (non-infinity non-NaN).
 */
/*#define HAS_FINITE		/ **/

/* HAS_FINITEL:
 *	This symbol, if defined, indicates that the finitel routine is
 *	available to check whether a long double is finite
 *	(non-infinity non-NaN).
 */
/*#define HAS_FINITEL		/ **/

/* HAS_FLOCK_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the flock() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern int flock(int, int);
 */
/*#define	HAS_FLOCK_PROTO	/ **/

/* HAS_FP_CLASS:
 *	This symbol, if defined, indicates that the fp_class routine is
 *	available to classify doubles.  Available for example in Digital UNIX.
 *	The returned values are defined in <math.h> and are:
 *
 *	FP_SNAN           Signaling NaN (Not-a-Number)
 *	FP_QNAN           Quiet NaN (Not-a-Number)
 *	FP_POS_INF        +infinity
 *	FP_NEG_INF        -infinity
 *	FP_POS_NORM       Positive normalized
 *	FP_NEG_NORM       Negative normalized
 *	FP_POS_DENORM     Positive denormalized
 *	FP_NEG_DENORM     Negative denormalized
 *	FP_POS_ZERO       +0.0 (positive zero)
 *	FP_NEG_ZERO       -0.0 (negative zero)
 */
/*#define HAS_FP_CLASS		/ **/

/* HAS_FPCLASS:
 *	This symbol, if defined, indicates that the fpclass routine is
 *	available to classify doubles.  Available for example in Solaris/SVR4.
 *	The returned values are defined in <ieeefp.h> and are:
 *
 *	FP_SNAN		signaling NaN
 *	FP_QNAN		quiet NaN
 *	FP_NINF		negative infinity
 *	FP_PINF		positive infinity
 *	FP_NDENORM	negative denormalized non-zero
 *	FP_PDENORM	positive denormalized non-zero
 *	FP_NZERO	negative zero
 *	FP_PZERO	positive zero
 *	FP_NNORM	negative normalized non-zero
 *	FP_PNORM	positive normalized non-zero
 */
/*#define HAS_FPCLASS		/ **/

/* HAS_FPCLASSIFY:
 *	This symbol, if defined, indicates that the fpclassify routine is
 *	available to classify doubles.  Available for example in HP-UX.
 *	The returned values are defined in <math.h> and are
 *
 *           FP_NORMAL     Normalized
 *           FP_ZERO       Zero
 *           FP_INFINITE   Infinity
 *           FP_SUBNORMAL  Denormalized
 *           FP_NAN        NaN
 *
 */
/*#define HAS_FPCLASSIFY		/ **/

/* HAS_FPCLASSL:
 *	This symbol, if defined, indicates that the fpclassl routine is
 *	available to classify long doubles.  Available for example in IRIX.
 *	The returned values are defined in <ieeefp.h> and are:
 *
 *	FP_SNAN		signaling NaN
 *	FP_QNAN		quiet NaN
 *	FP_NINF		negative infinity
 *	FP_PINF		positive infinity
 *	FP_NDENORM	negative denormalized non-zero
 *	FP_PDENORM	positive denormalized non-zero
 *	FP_NZERO	negative zero
 *	FP_PZERO	positive zero
 *	FP_NNORM	negative normalized non-zero
 *	FP_PNORM	positive normalized non-zero
 */
/*#define HAS_FPCLASSL		/ **/

/* HAS_FPOS64_T:
 *	This symbol will be defined if the C compiler supports fpos64_t.
 */
/*#define	HAS_FPOS64_T    	/ **/

/* HAS_FREXPL:
 *	This symbol, if defined, indicates that the frexpl routine is
 *	available to break a long double floating-point number into
 *	a normalized fraction and an integral power of 2.
 */
/*#define HAS_FREXPL		/ **/

/* HAS_STRUCT_FS_DATA:
 *	This symbol, if defined, indicates that the struct fs_data
 *	to do statfs() is supported.
 */
/*#define HAS_STRUCT_FS_DATA	/ **/

/* HAS_FSEEKO:
 *	This symbol, if defined, indicates that the fseeko routine is
 *	available to fseek beyond 32 bits (useful for ILP32 hosts).
 */
/*#define HAS_FSEEKO		/ **/

/* HAS_FSTATFS:
 *	This symbol, if defined, indicates that the fstatfs routine is
 *	available to stat filesystems by file descriptors.
 */
/*#define HAS_FSTATFS		/ **/

/* HAS_FSYNC:
 *	This symbol, if defined, indicates that the fsync routine is
 *	available to write a file's modified data and attributes to
 *	permanent storage.
 */
/*#define HAS_FSYNC		/ **/

/* HAS_FTELLO:
 *	This symbol, if defined, indicates that the ftello routine is
 *	available to ftell beyond 32 bits (useful for ILP32 hosts).
 */
/*#define HAS_FTELLO		/ **/

/* HAS_FUTIMES:
 *	This symbol, if defined, indicates that the futimes routine is
 *	available to change file descriptor time stamps with struct timevals.
 */
/*#define HAS_FUTIMES		/ **/

/* HAS_GETADDRINFO:
 *	This symbol, if defined, indicates that the getaddrinfo() function
 *	is available for use.
 */
/*#define HAS_GETADDRINFO		/ **/

/* HAS_GETCWD:
 *	This symbol, if defined, indicates that the getcwd routine is
 *	available to get the current working directory.
 */
/*#define HAS_GETCWD		/ **/

/* HAS_GETESPWNAM:
 *	This symbol, if defined, indicates that the getespwnam system call is
 *	available to retrieve enhanced (shadow) password entries by name.
 */
/*#define HAS_GETESPWNAM		/ **/

/* HAS_GETFSSTAT:
 *	This symbol, if defined, indicates that the getfsstat routine is
 *	available to stat filesystems in bulk.
 */
/*#define HAS_GETFSSTAT		/ **/

/* HAS_GETITIMER:
 *	This symbol, if defined, indicates that the getitimer routine is
 *	available to return interval timers.
 */
/*#define HAS_GETITIMER		/ **/

/* HAS_GETMNT:
 *	This symbol, if defined, indicates that the getmnt routine is
 *	available to get filesystem mount info by filename.
 */
/*#define HAS_GETMNT		/ **/

/* HAS_GETMNTENT:
 *	This symbol, if defined, indicates that the getmntent routine is
 *	available to iterate through mounted file systems to get their info.
 */
/*#define HAS_GETMNTENT		/ **/

/* HAS_GETNAMEINFO:
 *	This symbol, if defined, indicates that the getnameinfo() function
 *	is available for use.
 */
/*#define HAS_GETNAMEINFO		/ **/

/* HAS_GETPRPWNAM:
 *	This symbol, if defined, indicates that the getprpwnam system call is
 *	available to retrieve protected (shadow) password entries by name.
 */
/*#define HAS_GETPRPWNAM		/ **/

/* HAS_GETSPNAM:
 *	This symbol, if defined, indicates that the getspnam system call is
 *	available to retrieve SysV shadow password entries by name.
 */
/*#define HAS_GETSPNAM		/ **/

/* HAS_HASMNTOPT:
 *	This symbol, if defined, indicates that the hasmntopt routine is
 *	available to query the mount options of file systems.
 */
/*#define HAS_HASMNTOPT		/ **/

/* HAS_ILOGBL:
 *	This symbol, if defined, indicates that the ilogbl routine is
 *	available.  If scalbnl is also present we can emulate frexpl.
 */
/*#define HAS_ILOGBL		/ **/

/* HAS_INETNTOP:
 *	This symbol, if defined, indicates that the inet_ntop() function
 *	is available to parse IPv4 and IPv6 strings.
 */
/*#define HAS_INETNTOP		/ **/

/* HAS_INETPTON:
 *	This symbol, if defined, indicates that the inet_pton() function
 *	is available to parse IPv4 and IPv6 strings.
 */
/*#define HAS_INETPTON		/ **/

/* HAS_INT64_T:
 *     This symbol will defined if the C compiler supports int64_t.
 *     Usually the <inttypes.h> needs to be included, but sometimes
 *	<sys/types.h> is enough.
 */
/*#define     HAS_INT64_T               / **/

/* HAS_ISBLANK:
 *	This manifest constant lets the C program know that isblank 
 *	is available.
 */
/*#define HAS_ISBLANK		/ **/

/* HAS_ISFINITE:
 *	This symbol, if defined, indicates that the isfinite routine is
 *	available to check whether a double is finite (non-infinity non-NaN).
 */
/*#define HAS_ISFINITE		/ **/

/* HAS_ISINF:
 *	This symbol, if defined, indicates that the isinf routine is
 *	available to check whether a double is an infinity.
 */
/*#define HAS_ISINF		/ **/

/* HAS_ISNAN:
 *	This symbol, if defined, indicates that the isnan routine is
 *	available to check whether a double is a NaN.
 */
/*#define HAS_ISNAN		/ **/

/* HAS_ISNANL:
 *	This symbol, if defined, indicates that the isnanl routine is
 *	available to check whether a long double is a NaN.
 */
/*#define HAS_ISNANL		/ **/

/* HAS_LDBL_DIG:
 *	This symbol, if defined, indicates that this system's <float.h>
 *	or <limits.h> defines the symbol LDBL_DIG, which is the number
 *	of significant digits in a long double precision number. Unlike
 *	for DBL_DIG, there's no good guess for LDBL_DIG if it is undefined.
 */
/*#define HAS_LDBL_DIG 	/ * */

/* LIBM_LIB_VERSION:
 *	This symbol, if defined, indicates that libm exports _LIB_VERSION
 *	and that math.h defines the enum to manipulate it.
 */
/*#define LIBM_LIB_VERSION		/ **/

/* HAS_MADVISE:
 *	This symbol, if defined, indicates that the madvise system call is
 *	available to map a file into memory.
 */
/*#define HAS_MADVISE		/ **/

/* HAS_MALLOC_SIZE:
 *	This symbol, if defined, indicates that the malloc_size
 *	routine is available for use.
 */
/*#define HAS_MALLOC_SIZE		/ **/

/* HAS_MALLOC_GOOD_SIZE:
 *	This symbol, if defined, indicates that the malloc_good_size
 *	routine is available for use.
 */
/*#define HAS_MALLOC_GOOD_SIZE	/ **/

/* HAS_MKDTEMP:
 *	This symbol, if defined, indicates that the mkdtemp routine is
 *	available to exclusively create a uniquely named temporary directory.
 */
/*#define HAS_MKDTEMP		/ **/

/* HAS_MKSTEMPS:
 *	This symbol, if defined, indicates that the mkstemps routine is
 *	available to exclusively create and open a uniquely named
 *	(with a suffix) temporary file.
 */
/*#define HAS_MKSTEMPS		/ **/

/* HAS_MODFL:
 *	This symbol, if defined, indicates that the modfl routine is
 *	available to split a long double x into a fractional part f and
 *	an integer part i such that |f| < 1.0 and (f + i) = x.
 */
/* HAS_MODFL_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the modfl() function.  Otherwise, it is up
 *	to the program to supply one.
 */
/* HAS_MODFL_POW32_BUG:
 *	This symbol, if defined, indicates that the modfl routine is
 *	broken for long doubles >= pow(2, 32).
 *	For example from 4294967303.150000 one would get 4294967302.000000
 *	and 1.150000.  The bug has been seen in certain versions of glibc,
 *	release 2.2.2 is known to be okay.
 */
/*#define HAS_MODFL		/ **/
/*#define HAS_MODFL_PROTO		/ **/
/*#define HAS_MODFL_POW32_BUG		/ **/

/* HAS_MPROTECT:
 *	This symbol, if defined, indicates that the mprotect system call is
 *	available to modify the access protection of a memory mapped file.
 */
/*#define HAS_MPROTECT		/ **/

/* HAS_STRUCT_MSGHDR:
 *	This symbol, if defined, indicates that the struct msghdr
 *	is supported.
 */
/*#define HAS_STRUCT_MSGHDR	/ **/

/* HAS_NL_LANGINFO:
 *	This symbol, if defined, indicates that the nl_langinfo routine is
 *	available to return local data.  You will also need <langinfo.h>
 *	and therefore I_LANGINFO.
 */
/*#define HAS_NL_LANGINFO		/ **/

/* HAS_OFF64_T:
 *	This symbol will be defined if the C compiler supports off64_t.
 */
/*#define	HAS_OFF64_T    		/ **/

/* HAS_PRCTL:
 *	This symbol, if defined, indicates that the prctl routine is
 *	available to set process title.
 */
/* HAS_PRCTL_SET_NAME:
 *	This symbol, if defined, indicates that the prctl routine is
 *	available to set process title and supports PR_SET_NAME.
 */
/*#define HAS_PRCTL		/ **/
/*#define HAS_PRCTL_SET_NAME		/ **/

/* HAS_PROCSELFEXE:
 *	This symbol is defined if PROCSELFEXE_PATH is a symlink
 *	to the absolute pathname of the executing program.
 */
/* PROCSELFEXE_PATH:
 *	If HAS_PROCSELFEXE is defined this symbol is the filename
 *	of the symbolic link pointing to the absolute pathname of
 *	the executing program.
 */
/*#define HAS_PROCSELFEXE	/ **/
#if defined(HAS_PROCSELFEXE) && !defined(PROCSELFEXE_PATH)
#define PROCSELFEXE_PATH		/**/
#endif

/* HAS_PTHREAD_ATTR_SETSCOPE:
 *	This symbol, if defined, indicates that the pthread_attr_setscope
 *	system call is available to set the contention scope attribute of
 *	a thread attribute object.
 */
/*#define HAS_PTHREAD_ATTR_SETSCOPE		/ **/

/* HAS_READV:
 *	This symbol, if defined, indicates that the readv routine is
 *	available to do gather reads.  You will also need <sys/uio.h>
 *	and there I_SYSUIO.
 */
/*#define HAS_READV		/ **/

/* HAS_RECVMSG:
 *	This symbol, if defined, indicates that the recvmsg routine is
 *	available to send structured socket messages.
 */
/*#define HAS_RECVMSG		/ **/

/* HAS_SBRK_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the sbrk() function.  Otherwise, it is up
 *	to the program to supply one.  Good guesses are
 *		extern void* sbrk(int);
 *		extern void* sbrk(size_t);
 */
/*#define	HAS_SBRK_PROTO	/ **/

/* HAS_SCALBNL:
 *	This symbol, if defined, indicates that the scalbnl routine is
 *	available.  If ilogbl is also present we can emulate frexpl.
 */
/*#define HAS_SCALBNL		/ **/

/* HAS_SENDMSG:
 *	This symbol, if defined, indicates that the sendmsg routine is
 *	available to send structured socket messages.
 */
/*#define HAS_SENDMSG		/ **/

/* HAS_SETITIMER:
 *	This symbol, if defined, indicates that the setitimer routine is
 *	available to set interval timers.
 */
/*#define HAS_SETITIMER		/ **/

/* HAS_SETPROCTITLE:
 *	This symbol, if defined, indicates that the setproctitle routine is
 *	available to set process title.
 */
/*#define HAS_SETPROCTITLE		/ **/

/* USE_SFIO:
 *	This symbol, if defined, indicates that sfio should
 *	be used.
 */
/*#define	USE_SFIO		/ **/

/* HAS_SIGNBIT:
 *	This symbol, if defined, indicates that the signbit routine is
 *	available to check if the given number has the sign bit set.
 *	This should include correct testing of -0.0.  This will only be set
 *	if the signbit() routine is safe to use with the NV type used internally
 *	in perl.  Users should call Perl_signbit(), which will be #defined to
 *	the system's signbit() function or macro if this symbol is defined.
 */
/*#define HAS_SIGNBIT		/ **/

/* HAS_SIGPROCMASK:
 *	This symbol, if defined, indicates that the sigprocmask
 *	system call is available to examine or change the signal mask
 *	of the calling process.
 */
/*#define HAS_SIGPROCMASK		/ **/

/* USE_SITECUSTOMIZE:
 *	This symbol, if defined, indicates that sitecustomize should
 *	be used.
 */
#ifndef USE_SITECUSTOMIZE
/*#define	USE_SITECUSTOMIZE		/ **/
#endif

/* HAS_SNPRINTF:
 *	This symbol, if defined, indicates that the snprintf () library
 *	function is available for use.
 */
/* HAS_VSNPRINTF:
 *	This symbol, if defined, indicates that the vsnprintf () library
 *	function is available for use.
 */
/*#define HAS_SNPRINTF	/ **/
/*#define HAS_VSNPRINTF	/ **/

/* HAS_SOCKATMARK:
 *	This symbol, if defined, indicates that the sockatmark routine is
 *	available to test whether a socket is at the out-of-band mark.
 */
/*#define HAS_SOCKATMARK		/ **/

/* HAS_SOCKATMARK_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the sockatmark() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern int sockatmark(int);
 */
/*#define	HAS_SOCKATMARK_PROTO	/ **/

/* HAS_SOCKS5_INIT:
 *	This symbol, if defined, indicates that the socks5_init routine is
 *	available to initialize SOCKS 5.
 */
/*#define HAS_SOCKS5_INIT		/ **/

/* SPRINTF_RETURNS_STRLEN:
 *	This variable defines whether sprintf returns the length of the string
 *	(as per the ANSI spec). Some C libraries retain compatibility with
 *	pre-ANSI C and return a pointer to the passed in buffer; for these
 *	this variable will be undef.
 */
/*#define SPRINTF_RETURNS_STRLEN	/ **/

/* HAS_SQRTL:
 *	This symbol, if defined, indicates that the sqrtl routine is
 *	available to do long double square roots.
 */
/*#define HAS_SQRTL		/ **/

/* HAS_SETRESGID_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the setresgid() function.  Otherwise, it is up
 *	to the program to supply one.  Good guesses are
 *		extern int setresgid(uid_t ruid, uid_t euid, uid_t suid);
 */
/*#define	HAS_SETRESGID_PROTO	/ **/

/* HAS_SETRESUID_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the setresuid() function.  Otherwise, it is up
 *	to the program to supply one.  Good guesses are
 *		extern int setresuid(uid_t ruid, uid_t euid, uid_t suid);
 */
/*#define	HAS_SETRESUID_PROTO	/ **/

/* HAS_STRUCT_STATFS_F_FLAGS:
 *	This symbol, if defined, indicates that the struct statfs
 *	does have the f_flags member containing the mount flags of
 *	the filesystem containing the file.
 *	This kind of struct statfs is coming from <sys/mount.h> (BSD 4.3),
 *	not from <sys/statfs.h> (SYSV).  Older BSDs (like Ultrix) do not
 *	have statfs() and struct statfs, they have ustat() and getmnt()
 *	with struct ustat and struct fs_data.
 */
/*#define HAS_STRUCT_STATFS_F_FLAGS		/ **/

/* HAS_STRUCT_STATFS:
 *	This symbol, if defined, indicates that the struct statfs
 *	to do statfs() is supported.
 */
/*#define HAS_STRUCT_STATFS	/ **/

/* HAS_FSTATVFS:
 *	This symbol, if defined, indicates that the fstatvfs routine is
 *	available to stat filesystems by file descriptors.
 */
/*#define HAS_FSTATVFS		/ **/

/* HAS_STRFTIME:
 *	This symbol, if defined, indicates that the strftime routine is
 *	available to do time formatting.
 */
/*#define HAS_STRFTIME		/ **/

/* HAS_STRLCAT:
 *	This symbol, if defined, indicates that the strlcat () routine is
 *	available to do string concatenation.
 */
/*#define HAS_STRLCAT		/ **/

/* HAS_STRLCPY:
 *	This symbol, if defined, indicates that the strlcpy () routine is
 *	available to do string copying.
 */
/*#define HAS_STRLCPY		/ **/

/* HAS_STRTOLD:
 *	This symbol, if defined, indicates that the strtold routine is
 *	available to convert strings to long doubles.
 */
/*#define HAS_STRTOLD		/ **/

/* HAS_STRTOLL:
 *	This symbol, if defined, indicates that the strtoll routine is
 *	available to convert strings to long longs.
 */
/*#define HAS_STRTOLL		/ **/

/* HAS_STRTOQ:
 *	This symbol, if defined, indicates that the strtoq routine is
 *	available to convert strings to long longs (quads).
 */
/*#define HAS_STRTOQ		/ **/

/* HAS_STRTOULL:
 *	This symbol, if defined, indicates that the strtoull routine is
 *	available to convert strings to unsigned long longs.
 */
/*#define HAS_STRTOULL		/ **/

/* HAS_STRTOUQ:
 *	This symbol, if defined, indicates that the strtouq routine is
 *	available to convert strings to unsigned long longs (quads).
 */
/*#define HAS_STRTOUQ		/ **/

/* HAS_SYSCALL_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the syscall() function.  Otherwise, it is up
 *	to the program to supply one.  Good guesses are
 *		extern int syscall(int,  ...);
 *		extern int syscall(long, ...);
 */
/*#define	HAS_SYSCALL_PROTO	/ **/

/* HAS_TELLDIR_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the telldir() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern long telldir(DIR*);
 */
/*#define	HAS_TELLDIR_PROTO	/ **/

/* HAS_CTIME64:
 *	This symbol, if defined, indicates that the ctime64 () routine is
 *	available to do the 64bit variant of ctime ()
 */
/* HAS_LOCALTIME64:
 *	This symbol, if defined, indicates that the localtime64 () routine is
 *	available to do the 64bit variant of localtime ()
 */
/* HAS_GMTIME64:
 *	This symbol, if defined, indicates that the gmtime64 () routine is
 *	available to do the 64bit variant of gmtime ()
 */
/* HAS_MKTIME64:
 *	This symbol, if defined, indicates that the mktime64 () routine is
 *	available to do the 64bit variant of mktime ()
 */
/* HAS_DIFFTIME64:
 *	This symbol, if defined, indicates that the difftime64 () routine is
 *	available to do the 64bit variant of difftime ()
 */
/* HAS_ASCTIME64:
 *	This symbol, if defined, indicates that the asctime64 () routine is
 *	available to do the 64bit variant of asctime ()
 */
/*#define	HAS_CTIME64		/ **/
/*#define	HAS_LOCALTIME64		/ **/
/*#define	HAS_GMTIME64		/ **/
/*#define	HAS_MKTIME64		/ **/
/*#define	HAS_DIFFTIME64		/ **/
/*#define	HAS_ASCTIME64		/ **/

/* HAS_TIMEGM:
 *	This symbol, if defined, indicates that the timegm routine is
 *	available to do the opposite of gmtime ()
 */
/*#define HAS_TIMEGM		/ **/

/* U32_ALIGNMENT_REQUIRED:
 *	This symbol, if defined, indicates that you must access
 *	character data through U32-aligned pointers.
 */
#ifndef U32_ALIGNMENT_REQUIRED
#define U32_ALIGNMENT_REQUIRED	/**/
#endif

/* HAS_UALARM:
 *	This symbol, if defined, indicates that the ualarm routine is
 *	available to do alarms with microsecond granularity.
 */
/*#define HAS_UALARM		/ **/

/* HAS_UNORDERED:
 *	This symbol, if defined, indicates that the unordered routine is
 *	available to check whether two doubles are unordered
 *	(effectively: whether either of them is NaN)
 */
/*#define HAS_UNORDERED		/ **/

/* HAS_UNSETENV:
 *	This symbol, if defined, indicates that the unsetenv () routine is
 *	available for use.
 */
/*#define HAS_UNSETENV		/ **/

/* HAS_USLEEP_PROTO:
 *	This symbol, if defined, indicates that the system provides
 *	a prototype for the usleep() function.  Otherwise, it is up
 *	to the program to supply one.  A good guess is
 *		extern int usleep(useconds_t);
 */
/*#define	HAS_USLEEP_PROTO	/ **/

/* HAS_USTAT:
 *	This symbol, if defined, indicates that the ustat system call is
 *	available to query file system statistics by dev_t.
 */
/*#define HAS_USTAT		/ **/

/* HAS_WRITEV:
 *	This symbol, if defined, indicates that the writev routine is
 *	available to do scatter writes.
 */
/*#define HAS_WRITEV		/ **/

/* USE_DYNAMIC_LOADING:
 *	This symbol, if defined, indicates that dynamic loading of
 *	some sort is available.
 */
/*#define USE_DYNAMIC_LOADING		/ **/

/* FFLUSH_NULL:
 *	This symbol, if defined, tells that fflush(NULL) does flush
 *	all pending stdio output.
 */
/* FFLUSH_ALL:
 *	This symbol, if defined, tells that to flush
 *	all pending stdio output one must loop through all
 *	the stdio file handles stored in an array and fflush them.
 *	Note that if fflushNULL is defined, fflushall will not
 *	even be probed for and will be left undefined.
 */
/*#define	FFLUSH_NULL 		/ **/
/*#define	FFLUSH_ALL 		/ **/

/* I_ASSERT:
 *	This symbol, if defined, indicates that <assert.h> exists and
 *	could be included by the C program to get the assert() macro.
 */
#define	I_ASSERT		/**/

/* I_CRYPT:
 *	This symbol, if defined, indicates that <crypt.h> exists and
 *	should be included.
 */
/*#define	I_CRYPT		/ **/

/* DB_Prefix_t:
 *	This symbol contains the type of the prefix structure element
 *	in the <db.h> header file.  In older versions of DB, it was
 *	int, while in newer ones it is u_int32_t.
 */
/* DB_Hash_t:
 *	This symbol contains the type of the prefix structure element
 *	in the <db.h> header file.  In older versions of DB, it was
 *	int, while in newer ones it is size_t.
 */
/* DB_VERSION_MAJOR_CFG:
 *	This symbol, if defined, defines the major version number of
 *	Berkeley DB found in the <db.h> header when Perl was configured.
 */
/* DB_VERSION_MINOR_CFG:
 *	This symbol, if defined, defines the minor version number of
 *	Berkeley DB found in the <db.h> header when Perl was configured.
 *	For DB version 1 this is always 0.
 */
/* DB_VERSION_PATCH_CFG:
 *	This symbol, if defined, defines the patch version number of
 *	Berkeley DB found in the <db.h> header when Perl was configured.
 *	For DB version 1 this is always 0.
 */
#define DB_Hash_t	u_int32_t		/**/
#define DB_Prefix_t	size_t  	/**/
#define DB_VERSION_MAJOR_CFG	0  	/**/
#define DB_VERSION_MINOR_CFG	0  	/**/
#define DB_VERSION_PATCH_CFG	0  	/**/

/* I_FP:
 *	This symbol, if defined, indicates that <fp.h> exists and
 *	should be included.
 */
/*#define	I_FP		/ **/

/* I_FP_CLASS:
 *	This symbol, if defined, indicates that <fp_class.h> exists and
 *	should be included.
 */
/*#define	I_FP_CLASS		/ **/

/* I_IEEEFP:
 *	This symbol, if defined, indicates that <ieeefp.h> exists and
 *	should be included.
 */
/*#define	I_IEEEFP		/ **/

/* I_INTTYPES:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <inttypes.h>.
 */
/*#define   I_INTTYPES                / **/

/* I_LANGINFO:
 *	This symbol, if defined, indicates that <langinfo.h> exists and
 *	should be included.
 */
/*#define	I_LANGINFO		/ **/

/* I_LIBUTIL:
 *	This symbol, if defined, indicates that <libutil.h> exists and
 *	should be included.
 */
/*#define	I_LIBUTIL		/ **/

/* I_MALLOCMALLOC:
 *	This symbol, if defined, indicates to the C program that it should
 *	include <malloc/malloc.h>.
 */
/*#define I_MALLOCMALLOC		/ **/

/* I_MNTENT:
 *	This symbol, if defined, indicates that <mntent.h> exists and
 *	should be included.
 */
/*#define	I_MNTENT		/ **/

/* I_NETINET_TCP:
 *     This symbol, if defined, indicates to the C program that it should
 *     include <netinet/tcp.h>.
 */
/*#define   I_NETINET_TCP                / **/

/* I_POLL:
 *	This symbol, if defined, indicates that <poll.h> exists and
 *	should be included. (see also HAS_POLL)
 */
/*#define	I_POLL		/ **/

/* I_PROT:
 *	This symbol, if defined, indicates that <prot.h> exists and
 *	should be included.
 */
/*#define	I_PROT		/ **/

/* I_SHADOW:
 *	This symbol, if defined, indicates that <shadow.h> exists and
 *	should be included.
 */
/*#define	I_SHADOW		/ **/

/* I_SOCKS:
 *	This symbol, if defined, indicates that <socks.h> exists and
 *	should be included.
 */
/*#define	I_SOCKS		/ **/

/* I_STDBOOL:
 *	This symbol, if defined, indicates that <stdbool.h> exists and
 *	can be included.
 */
/*#define	I_STDBOOL		/ **/

/* I_SUNMATH:
 *	This symbol, if defined, indicates that <sunmath.h> exists and
 *	should be included.
 */
/*#define	I_SUNMATH		/ **/

/* I_SYSLOG:
 *	This symbol, if defined, indicates that <syslog.h> exists and
 *	should be included.
 */
/*#define	I_SYSLOG		/ **/

/* I_SYSMODE:
 *	This symbol, if defined, indicates that <sys/mode.h> exists and
 *	should be included.
 */
/*#define	I_SYSMODE		/ **/

/* I_SYS_MOUNT:
 *	This symbol, if defined, indicates that <sys/mount.h> exists and
 *	should be included.
 */
/*#define	I_SYS_MOUNT		/ **/

/* I_SYS_STATFS:
 *	This symbol, if defined, indicates that <sys/statfs.h> exists.
 */
/*#define	I_SYS_STATFS		/ **/

/* I_SYS_STATVFS:
 *	This symbol, if defined, indicates that <sys/statvfs.h> exists and
 *	should be included.
 */
/*#define	I_SYS_STATVFS		/ **/

/* I_SYSUTSNAME:
 *	This symbol, if defined, indicates that <sys/utsname.h> exists and
 *	should be included.
 */
/*#define	I_SYSUTSNAME		/ **/

/* I_SYS_VFS:
 *	This symbol, if defined, indicates that <sys/vfs.h> exists and
 *	should be included.
 */
/*#define	I_SYS_VFS		/ **/

/* I_USTAT:
 *	This symbol, if defined, indicates that <ustat.h> exists and
 *	should be included.
 */
/*#define	I_USTAT		/ **/

/* PERL_PRIfldbl:
 *	This symbol, if defined, contains the string used by stdio to
 *	format long doubles (format 'f') for output.
 */
/* PERL_PRIgldbl:
 *	This symbol, if defined, contains the string used by stdio to
 *	format long doubles (format 'g') for output.
 */
/* PERL_PRIeldbl:
 *	This symbol, if defined, contains the string used by stdio to
 *	format long doubles (format 'e') for output.
 */
/* PERL_SCNfldbl:
 *	This symbol, if defined, contains the string used by stdio to
 *	format long doubles (format 'f') for input.
 */
/*#define PERL_PRIfldbl	"llf"	/ **/
/*#define PERL_PRIgldbl	"llg"	/ **/
/*#define PERL_PRIeldbl	"lle"	/ **/
/*#define PERL_SCNfldbl	"llf"	/ **/

/* PERL_MAD:
 *	This symbol, if defined, indicates that the Misc Attribution
 *	Declaration code should be conditionally compiled.
 */
/*#define	PERL_MAD		/ **/

/* NEED_VA_COPY:
 *	This symbol, if defined, indicates that the system stores
 *	the variable argument list datatype, va_list, in a format
 *	that cannot be copied by simple assignment, so that some
 *	other means must be used when copying is required.
 *	As such systems vary in their provision (or non-provision)
 *	of copying mechanisms, handy.h defines a platform-
 *	independent macro, Perl_va_copy(src, dst), to do the job.
 */
#define	NEED_VA_COPY		/**/

/* IVTYPE:
 *	This symbol defines the C type used for Perl's IV.
 */
/* UVTYPE:
 *	This symbol defines the C type used for Perl's UV.
 */
/* I8TYPE:
 *	This symbol defines the C type used for Perl's I8.
 */
/* U8TYPE:
 *	This symbol defines the C type used for Perl's U8.
 */
/* I16TYPE:
 *	This symbol defines the C type used for Perl's I16.
 */
/* U16TYPE:
 *	This symbol defines the C type used for Perl's U16.
 */
/* I32TYPE:
 *	This symbol defines the C type used for Perl's I32.
 */
/* U32TYPE:
 *	This symbol defines the C type used for Perl's U32.
 */
/* I64TYPE:
 *	This symbol defines the C type used for Perl's I64.
 */
/* U64TYPE:
 *	This symbol defines the C type used for Perl's U64.
 */
/* NVTYPE:
 *	This symbol defines the C type used for Perl's NV.
 */
/* IVSIZE:
 *	This symbol contains the sizeof(IV).
 */
/* UVSIZE:
 *	This symbol contains the sizeof(UV).
 */
/* I8SIZE:
 *	This symbol contains the sizeof(I8).
 */
/* U8SIZE:
 *	This symbol contains the sizeof(U8).
 */
/* I16SIZE:
 *	This symbol contains the sizeof(I16).
 */
/* U16SIZE:
 *	This symbol contains the sizeof(U16).
 */
/* I32SIZE:
 *	This symbol contains the sizeof(I32).
 */
/* U32SIZE:
 *	This symbol contains the sizeof(U32).
 */
/* I64SIZE:
 *	This symbol contains the sizeof(I64).
 */
/* U64SIZE:
 *	This symbol contains the sizeof(U64).
 */
/* NVSIZE:
 *	This symbol contains the sizeof(NV).
 */
/* NV_PRESERVES_UV:
 *	This symbol, if defined, indicates that a variable of type NVTYPE
 *	can preserve all the bits of a variable of type UVTYPE.
 */
/* NV_PRESERVES_UV_BITS:
 *	This symbol contains the number of bits a variable of type NVTYPE
 *	can preserve of a variable of type UVTYPE.
 */
/* NV_OVERFLOWS_INTEGERS_AT:
 *	This symbol gives the largest integer value that NVs can hold. This
 *	value + 1.0 cannot be stored accurately. It is expressed as constant
 *	floating point expression to reduce the chance of decimal/binary
 *	conversion issues. If it can not be determined, the value 0 is given.
 */
/* NV_ZERO_IS_ALLBITS_ZERO:
 *	This symbol, if defined, indicates that a variable of type NVTYPE
 *	stores 0.0 in memory as all bits zero.
 */
#define	IVTYPE		long		/**/
#define	UVTYPE		unsigned long		/**/
#define	I8TYPE		signed char		/**/
#define	U8TYPE		unsigned char		/**/
#define	I16TYPE		short	/**/
#define	U16TYPE		unsigned short	/**/
#define	I32TYPE		long	/**/
#define	U32TYPE		unsigned long	/**/
#ifdef HAS_QUAD
#define	I64TYPE		int64_t	/**/
#define	U64TYPE		uint64_t	/**/
#endif
#define	NVTYPE		double		/**/
#define	IVSIZE		4		/**/
#define	UVSIZE		4		/**/
#define	I8SIZE		1		/**/
#define	U8SIZE		1		/**/
#define	I16SIZE		2	/**/
#define	U16SIZE		2	/**/
#define	I32SIZE		4	/**/
#define	U32SIZE		4	/**/
#ifdef HAS_QUAD
#define	I64SIZE		8	/**/
#define	U64SIZE		8	/**/
#endif
#define	NVSIZE		8		/**/
#undef	NV_PRESERVES_UV
#define	NV_PRESERVES_UV_BITS	0
#define	NV_OVERFLOWS_INTEGERS_AT	256.0*256.0*256.0*256.0*256.0*256.0*2.0*2.0*2.0*2.0*2.0
#undef	NV_ZERO_IS_ALLBITS_ZERO
#if UVSIZE == 8
#   ifdef BYTEORDER
#       if BYTEORDER == 0x1234
#           undef BYTEORDER
#           define BYTEORDER 0x12345678
#       else
#           if BYTEORDER == 0x4321
#               undef BYTEORDER
#               define BYTEORDER 0x87654321
#           endif
#       endif
#   endif
#endif

/* IVdf:
 *	This symbol defines the format string used for printing a Perl IV
 *	as a signed decimal integer.
 */
/* UVuf:
 *	This symbol defines the format string used for printing a Perl UV
 *	as an unsigned decimal integer.
 */
/* UVof:
 *	This symbol defines the format string used for printing a Perl UV
 *	as an unsigned octal integer.
 */
/* UVxf:
 *	This symbol defines the format string used for printing a Perl UV
 *	as an unsigned hexadecimal integer in lowercase abcdef.
 */
/* UVXf:
 *	This symbol defines the format string used for printing a Perl UV
 *	as an unsigned hexadecimal integer in uppercase ABCDEF.
 */
/* NVef:
 *	This symbol defines the format string used for printing a Perl NV
 *	using %e-ish floating point format.
 */
/* NVff:
 *	This symbol defines the format string used for printing a Perl NV
 *	using %f-ish floating point format.
 */
/* NVgf:
 *	This symbol defines the format string used for printing a Perl NV
 *	using %g-ish floating point format.
 */
#define	IVdf		"ld"		/**/
#define	UVuf		"lu"		/**/
#define	UVof		"lo"		/**/
#define	UVxf		"lx"		/**/
#define	UVXf		"lX"		/**/
#define	NVef		"e"		/**/
#define	NVff		"f"		/**/
#define	NVgf		"g"		/**/

/* SELECT_MIN_BITS:
 *	This symbol holds the minimum number of bits operated by select.
 *	That is, if you do select(n, ...), how many bits at least will be
 *	cleared in the masks if some activity is detected.  Usually this
 *	is either n or 32*ceil(n/32), especially many little-endians do
 *	the latter.  This is only useful if you have select(), naturally.
 */
#define SELECT_MIN_BITS 	32	/**/

/* ST_INO_SIZE:
 *	This variable contains the size of struct stat's st_ino in bytes.
 */
/* ST_INO_SIGN:
 *	This symbol holds the signedess of struct stat's st_ino.
 *	1 for unsigned, -1 for signed.
 */
#define ST_INO_SIGN 1	/* st_ino sign */
#define ST_INO_SIZE 4	/* st_ino size */

/* STARTPERL:
 *	This variable contains the string to put in front of a perl
 *	script to make sure (one hopes) that it runs with perl and not
 *	some shell.
 */
#define STARTPERL "#!perl"		/**/

/* HAS_STDIO_STREAM_ARRAY:
 *	This symbol, if defined, tells that there is an array
 *	holding the stdio streams.
 */
/* STDIO_STREAM_ARRAY:
 *	This symbol tells the name of the array holding the stdio streams.
 *	Usual values include _iob, __iob, and __sF.
 */
/*#define	HAS_STDIO_STREAM_ARRAY	/ **/
#ifdef HAS_STDIO_STREAM_ARRAY
#define STDIO_STREAM_ARRAY	
#endif

/* GMTIME_MAX:
 *	This symbol contains the maximum value for the time_t offset that
 *	the system function gmtime () accepts, and defaults to 0
 */
/* GMTIME_MIN:
 *	This symbol contains the minimum value for the time_t offset that
 *	the system function gmtime () accepts, and defaults to 0
 */
/* LOCALTIME_MAX:
 *	This symbol contains the maximum value for the time_t offset that
 *	the system function localtime () accepts, and defaults to 0
 */
/* LOCALTIME_MIN:
 *	This symbol contains the minimum value for the time_t offset that
 *	the system function localtime () accepts, and defaults to 0
 */
#define GMTIME_MAX		2147483647	/**/
#define GMTIME_MIN		0	/**/
#define LOCALTIME_MAX	2147483647	/**/
#define LOCALTIME_MIN	0	/**/

/* USE_64_BIT_INT:
 *	This symbol, if defined, indicates that 64-bit integers should
 *	be used when available.  If not defined, the native integers
 *	will be employed (be they 32 or 64 bits).  The minimal possible
 *	64-bitness is used, just enough to get 64-bit integers into Perl.
 *	This may mean using for example "long longs", while your memory
 *	may still be limited to 2 gigabytes.
 */
/* USE_64_BIT_ALL:
 *	This symbol, if defined, indicates that 64-bit integers should
 *	be used when available.  If not defined, the native integers
 *	will be used (be they 32 or 64 bits).  The maximal possible
 *	64-bitness is employed: LP64 or ILP64, meaning that you will
 *	be able to use more than 2 gigabytes of memory.  This mode is
 *	even more binary incompatible than USE_64_BIT_INT. You may not
 *	be able to run the resulting executable in a 32-bit CPU at all or
 *	you may need at least to reboot your OS to 64-bit mode.
 */
#ifndef USE_64_BIT_INT
/*#define	USE_64_BIT_INT		/ **/
#endif
#ifndef USE_64_BIT_ALL
/*#define	USE_64_BIT_ALL		/ **/
#endif

/* USE_DTRACE:
 *	This symbol, if defined, indicates that Perl should
 *	be built with support for DTrace.
 */
/*#define USE_DTRACE		/ **/

/* USE_FAST_STDIO:
 *	This symbol, if defined, indicates that Perl should
 *	be built to use 'fast stdio'.
 *	Defaults to define in Perls 5.8 and earlier, to undef later.
 */
#ifndef USE_FAST_STDIO
/*#define	USE_FAST_STDIO		/ **/
#endif

/* USE_KERN_PROC_PATHNAME:
 *	This symbol, if defined, indicates that we can use sysctl with
 *	KERN_PROC_PATHNAME to get a full path for the executable, and hence
 * 	convert $^X to an absolute path.
 */
/*#define USE_KERN_PROC_PATHNAME	/ **/

/* USE_LARGE_FILES:
 *	This symbol, if defined, indicates that large file support
 *	should be used when available.
 */
#ifndef USE_LARGE_FILES
/*#define	USE_LARGE_FILES		/ **/
#endif

/* USE_LONG_DOUBLE:
 *	This symbol, if defined, indicates that long doubles should
 *	be used when available.
 */
#ifndef USE_LONG_DOUBLE
/*#define	USE_LONG_DOUBLE		/ **/
#endif

/* USE_MORE_BITS:
 *	This symbol, if defined, indicates that 64-bit interfaces and
 *	long doubles should be used when available.
 */
#ifndef USE_MORE_BITS
/*#define	USE_MORE_BITS		/ **/
#endif

/* MULTIPLICITY:
 *	This symbol, if defined, indicates that Perl should
 *	be built to use multiplicity.
 */
#ifndef MULTIPLICITY
/*#define	MULTIPLICITY		/ **/
#endif

/* USE_NSGETEXECUTABLEPATH:
 *	This symbol, if defined, indicates that we can use _NSGetExecutablePath
 *	and realpath to get a full path for the executable, and hence convert
 *	$^X to an absolute path.
 */
/*#define USE_NSGETEXECUTABLEPATH	/ **/

/* USE_PERLIO:
 *	This symbol, if defined, indicates that the PerlIO abstraction should
 *	be used throughout.  If not defined, stdio should be
 *	used in a fully backward compatible manner.
 */
#ifndef USE_PERLIO
/*#define	USE_PERLIO		/ **/
#endif

/* USE_SOCKS:
 *	This symbol, if defined, indicates that Perl should
 *	be built to use socks.
 */
#ifndef USE_SOCKS
/*#define	USE_SOCKS		/ **/
#endif

#endif

/* Generated from:
 * d9b6a68b192c72c0695b560fae6e4916b381f55df5fdf7911b0ef434840f092e config_h.SH
 * 2678333a6d6676f8aa39b7011bf5133dce5be453df10a8d04cb3187103a10caf uconfig.sh
 * ex: set ro: */
