// Emscripten-specific version of musl/src/network/lookup_name.c

#include <assert.h>
#include <string.h>
#include "musl/src/network/lookup.h"
#include "emscripten_internal.h"

int __lookup_name(struct address buf[static MAXADDRS], char canon[static 256], const char *name, int family, int flags)
{
	/* We currently only support the callsite in gethostbyname2_r which
	 * passes AI_CANONNAME.  Remove this assertion when if we ever expand
	 * this support. */
	assert(flags == AI_CANONNAME);

	if (family != AF_INET) {
		return EAI_SYSTEM;
	}

	/* This hunk is a duplicated from musl/src/network/lookup_name.c */
	*canon = 0;
	if (name) {
		/* reject empty name and check len so it fits into temp bufs */
		size_t l = strnlen(name, 255);
		if (l-1 >= 254)
			return EAI_NONAME;
		memcpy(canon, name, l+1);
	}

	/* We only support a single address */
	uint32_t addr = _emscripten_lookup_name(name);
	memset(&buf[0], 0, sizeof(buf[0]));
	memcpy(&buf[0].addr, &addr, sizeof(addr));
	return 1;
}
