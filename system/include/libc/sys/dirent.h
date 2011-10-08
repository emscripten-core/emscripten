/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifdef __cplusplus
extern "C" {
#endif

/* XXX Emscripten */
struct dirent {
  ino_t d_ino;
  char  d_name[MAXNAMLEN];
  int   d_off;
  int   d_reclen;
  char  d_type;
};

#define DIR struct dirent

DIR  *opendir(const char *);
void  seekdir(DIR *, long);
long  telldir(DIR *);
DIR  *readdir(DIR *);
int   closedir(DIR *dirp);
void  rewinddir(DIR *dirp);

#ifdef __cplusplus
}
#endif
