/* This file is only used if enabled in the build system, in which case it is
 * included automatically via command line options. It is not included
 * explicitly by any source files or other headers. Its purpose is to
 * override default visibilities to reduce the size and performance costs
 * of position-independent code. */

#if !defined(CRT) && !defined(__ASSEMBLER__)

/* Conceptually, all symbols should be protected, but some toolchains
 * fail to support copy relocations for protected data, so exclude all
 * exported data symbols. */

__attribute__((__visibility__("default")))
extern struct _IO_FILE *const stdin, *const stdout, *const stderr;

__attribute__((__visibility__("default")))
extern int optind, opterr, optopt, optreset, __optreset, getdate_err, h_errno, daylight, __daylight, signgam, __signgam;

__attribute__((__visibility__("default")))
extern long timezone, __timezone;

__attribute__((__visibility__("default")))
extern char *optarg, **environ, **__environ, *tzname[2], *__tzname[2], *__progname, *__progname_full;

#pragma GCC visibility push(protected)

#endif
