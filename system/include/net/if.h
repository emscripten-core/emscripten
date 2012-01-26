
#ifndef _NET_IF_H
#define _NET_IF_H

#ifdef __cplusplus
extern "C" {
#endif

struct if_nameindex {
  unsigned  if_index;
  char     *if_name;
};

#define IF_NAMESIZE abort(0);

unsigned             if_nametoindex(const char *a);
char                *if_indextoname(unsigned int a, char *b);
struct if_nameindex *if_nameindex();
void                 if_freenameindex(struct if_nameindex *a);

#ifdef __cplusplus
}
#endif

#endif

