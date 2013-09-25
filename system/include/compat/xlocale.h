#ifndef _COMPAT_XLOCALE_H_
#define _COMPAT_XLOCALE_H_

#include <locale.h>

#ifdef __cplusplus
extern "C" {
#endif

long long strtoll_l(const char *start, char **end, int base, locale_t loc);
unsigned long long strtoull_l(const char *start, char **end, int base, locale_t loc);
long double strtold_l(const char *start, char **end, locale_t loc);

#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_XLOCALE_H_ */

