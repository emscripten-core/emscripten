#include <stdlib.h>
#include "locale_impl.h"
#include "libc.h"

int __loc_is_allocated(locale_t);

void freelocale(locale_t l)
{
	if (__loc_is_allocated(l)) free(l);
}

weak_alias(freelocale, __freelocale);
