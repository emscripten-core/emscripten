// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int main()
{
  int ret = mkdir("a", 0777); assert(ret == 0); assert(errno == 0);
  ret = mkdir("./././a/b", 0777); assert(ret == 0); assert(errno == 0);
  ret = mkdir("a/b/./../b/c", 0777); assert(ret == 0); assert(errno == 0);
  FILE *file = fopen("./a/./hello_file.txt", "wb");
  assert(file);
  fputs("test", file);
  ret = fclose(file); assert(ret == 0); assert(errno == 0);

  ret = chdir("./././a/./b/../b/c/./../c/."); assert(ret == 0); assert(errno == 0);
  file = fopen(".././../hello_file.txt", "rb");
  assert(file); assert(errno == 0);
  char str[6] = {};
  fread(str, 1, 5, file); assert(errno == 0);
  ret = fclose(file); assert(ret == 0); assert(errno == 0);
  assert(!strcmp(str, "test"));

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
