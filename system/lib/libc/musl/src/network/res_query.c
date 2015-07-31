#define _GNU_SOURCE
#include <resolv.h>
#include <netdb.h>
#include "__dns.h"
#include "libc.h"

int res_query(const char *name, int class, int type, unsigned char *dest, int len)
{
	if (class != 1 || len < 512)
		return -1;
	switch(__dns_doqueries(dest, name, &type, 1)) {
	case EAI_NONAME:
		h_errno = HOST_NOT_FOUND;
		return -1;
	case EAI_AGAIN:
		h_errno = TRY_AGAIN;
		return -1;
	case EAI_FAIL:
		h_errno = NO_RECOVERY;
		return -1;
	}
	return 512;
}

weak_alias(res_query, res_search);
