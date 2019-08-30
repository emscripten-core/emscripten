#include <time.h>

int __days_in_month(int, int);
int __month_to_secs(int, int);
long long __year_to_secs(long long, int *);
long long __tm_to_secs(const struct tm *);
int __secs_to_tm(long long, struct tm *);
void __secs_to_zone(long long, int, int *, long *, long *, const char **);
const unsigned char *__map_file(const char *, size_t *);
