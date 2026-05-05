#include <unistd.h>
#include <stdio.h>
#include <errno.h>

size_t confstr(int name, char *buf, size_t len)
{
	const char *s = "";
	if (!name) {
#ifndef __EMSCRIPTEN__
		s = "/bin:/usr/bin";
#else
	// TODO(sbc): Can we just remove these custom values.
	// We have tests that check for them but its not clear
	// anything else does.
		s = "/";
	} else if (name == _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS) {
		s = "POSIX_V6_ILP32_OFF32\nPOSIX_V6_ILP32_OFFBIG";
	} else if (name == _CS_GNU_LIBPTHREAD_VERSION) {
		s = "";
	} else if (name == _CS_GNU_LIBC_VERSION) {
		s = "glibc 2.14";
	} else if (name == _CS_POSIX_V6_ILP32_OFF32_CFLAGS ||
						 name == _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS ||
						 name == _CS_POSIX_V6_ILP32_OFF32_LDFLAGS) {
		s = "-m32";
	} else if (name == _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS) {
		s = "-m32 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64";
#endif
	} else if ((name&~4U)!=1 && name-_CS_POSIX_V6_ILP32_OFF32_CFLAGS>35U) {
		errno = EINVAL;
		return 0;
	}
	// snprintf is overkill but avoid wasting code size to implement
	// this completely useless function and its truncation semantics
	return snprintf(buf, len, "%s", s) + 1;
}
