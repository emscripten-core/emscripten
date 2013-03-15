#ifndef _DLFCN_H_INCLUDED
#define _DLFCN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define RTLD_DEFAULT 0
#define RTLD_LAZY    1
#define RTLD_NOW     2
#define RTLD_GLOBAL  4
#define RTLD_LOCAL   8

typedef struct {
  const char *dli_fname;
  void       *dli_fbase;
  const char *dli_sname;
  void       *dli_saddr;
} Dl_info;

void  *dlopen(const char *, int);
void  *dlsym(void *, const char *);
int    dlclose(void *);
char  *dlerror(void);
int    dladdr(void *addr, Dl_info *info);

#ifdef __cplusplus
}
#endif

#endif
