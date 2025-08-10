// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char* lastLongString;

char* makeLongStr(const char* str, int strCount) {
  char* result = lastLongString = realloc(lastLongString, (strCount * strlen(str)) + 1);;
  result[0] = '\0';

  for (int i = 0; i < strCount; ++i) {
    strcat(result, str);
  }

  return result;
}

void doPositiveTest(const char* dirName) {
  printf("positive test case: %s\n", dirName);

  char fullDirName[4096];
  sprintf(fullDirName, "/%s", dirName);
  int ret = mkdir(fullDirName, 0777);
  assert(ret == 0);
  ret = chdir(fullDirName);
  assert(ret == 0);

  char buf[4096];
  char* cwd = getcwd(buf, sizeof(buf));
  assert(cwd != NULL);
  printf("getcwd -> %s\n", cwd);
}

void doNegativeTest(const char* dirName) {
  printf("negative test case: %s\n", dirName);

  int ret = mkdir(dirName, 0777);
  assert(ret == 0);
  ret = chdir(dirName);
  assert(ret == 0);

  unsigned char buf[4096];
  unsigned char filler = 0xFE;
  memset(buf, filler, sizeof(buf));

  size_t allowedBufferSize = strlen(dirName) / 2;
  char* cwd = getcwd((char*)buf, allowedBufferSize);
  assert(cwd == 0);
  assert(errno == ERANGE);

  for (size_t i = allowedBufferSize; i < sizeof(buf); i++) {
    assert(buf[i] == filler);
  }
}

int main() {
  // Short non-ascii name
  doPositiveTest(u8"абвгд");

  // Long non-ascii name
  char longStr[4096] = "abcde";
  strcat(longStr, makeLongStr(u8"абвгд", 25));
  assert(strlen(longStr) == 255);
  doPositiveTest(longStr);

  // The negative test passes a half-sized buffer to getcwd and makes sure that it fails.
  doNegativeTest(makeLongStr(u8"abcde", 10));

  free(lastLongString);
}
