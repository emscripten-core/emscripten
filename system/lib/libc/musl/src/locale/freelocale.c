#include <stdlib.h>
#include "locale_impl.h"
#include "libc.h"

void freelocale(locale_t l)
{
	free(l);
}

weak_alias(freelocale, __freelocale);
