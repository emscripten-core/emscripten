
#ifndef _MNTENT_H
#define _MNTENT_H

#include <stdio.h>

struct mntent {
  char *mnt_fsname;
  char *mnt_dir;
  char *mnt_type;
  char *mnt_opts;
  int   mnt_freq;
  int   mnt_passno;
};

struct mntent *getmntent(FILE *f);
FILE *setmntent(const char *filename, const char *type);
int   addmntent(FILE *f, const struct mntent *m);
int   endmntent(FILE *f);
char *hasmntopt(const struct mntent *m, const char *opt);

#endif

