/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifdef __EMSCRIPTEN__
#include <limits.h>
#include <string.h>

#include "emscripten_internal.h"

char *__progname=0, *__progname_full=0;

weak_alias(__progname, program_invocation_short_name);
weak_alias(__progname_full, program_invocation_name);

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
