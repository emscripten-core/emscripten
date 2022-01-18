#ifdef __EMSCRIPTEN__
#include <limits.h>
#include <string.h>
#endif

#include "libc.h"

struct __libc __libc;

size_t __hwcap;
char *__progname=0, *__progname_full=0;

weak_alias(__progname, program_invocation_short_name);
weak_alias(__progname_full, program_invocation_name);

#ifdef __EMSCRIPTEN__
/* See src/library.js for the implementation. */
void _emscripten_get_progname(char*, int);

__attribute__((constructor))
static void __progname_ctor(void)
{
	static char full_path[PATH_MAX];
	char *basename;

	_emscripten_get_progname(full_path, sizeof(full_path));

	basename = strrchr(full_path, '/');
	if (basename == NULL) {
		basename = full_path;
	} else {
		basename++;
	}

	__progname_full = full_path;
	__progname = basename;
}
#endif
