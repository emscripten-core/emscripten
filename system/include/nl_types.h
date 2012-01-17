
#ifndef _NL_TYPES_H_
#define _NL_TYPES_H_

typedef struct _nl_catd {
  void *data;
  int   size;
} *nl_catd;
typedef int nl_item;

#define NL_SETD 100
#define NL_CAT_LOCALE 200

nl_catd  catopen(const char *, int);
int      catclose(nl_catd);
char    *catgets(nl_catd, int, int, const char *);

#endif

