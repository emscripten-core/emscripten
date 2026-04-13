#include "locale_impl.h"
#include "pthread_impl.h"
#include "libc.h"

#ifdef __EMSCRIPTEN__
_Thread_local locale_t __tls_locale = &libc.global_locale;
#endif

locale_t __uselocale(locale_t new)
{
#ifdef __EMSCRIPTEN__
	locale_t old = __tls_locale;
	locale_t global = &libc.global_locale;

	if (new) __tls_locale = new == LC_GLOBAL_LOCALE ? global : new;
#else
	pthread_t self = __pthread_self();
	locale_t old = self->locale;
	locale_t global = &libc.global_locale;

	if (new) self->locale = new == LC_GLOBAL_LOCALE ? global : new;
#endif

	return old == global ? LC_GLOBAL_LOCALE : old;
}

weak_alias(__uselocale, uselocale);
