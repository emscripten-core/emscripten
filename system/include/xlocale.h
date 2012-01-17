
#ifndef _XLOCALE_H_
#define _XLOCALE_H_

#include <locale.h>

long long strtoll_l(const char *start, char **end, int base, locale_t loc);
unsigned long long strtoull_l(const char *start, char **end, int base, locale_t loc);

double strtold_l(const char *start, char **end, locale_t loc);

int isxdigit_l(int c, locale_t locale);
int isdigit_l(int c, locale_t locale);

#endif /* _LOCALE_H_ */

