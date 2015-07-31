#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "libc.h"

struct passwd *__getpwent_a(FILE *f, struct passwd *pw, char **line, size_t *size);
struct group *__getgrent_a(FILE *f, struct group *gr, char **line, size_t *size, char ***mem, size_t *nmem);
int __parsespent(char *s, struct spwd *sp);
