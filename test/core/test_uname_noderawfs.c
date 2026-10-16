/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

EM_JS(void, get_node_os_field, (const char* field, char* out, int max_len), {
  var os = require('node:os');
  var val = os[UTF8ToString(Number(field))]();
  stringToUTF8(val, Number(out), max_len);
});

int main() {
  struct utsname u;
  int ret = uname(&u);
  assert(ret == 0);

  printf("sysname: %s\n", u.sysname);
  printf("nodename: %s\n", u.nodename);
  printf("release: %s\n", u.release);
  printf("version: %s\n", u.version);
  printf("machine: %s\n", u.machine);

  char expected[sizeof(u.sysname)];

  get_node_os_field("type", expected, sizeof(expected));
  assert(strcmp(u.sysname, expected) == 0);

  get_node_os_field("hostname", expected, sizeof(expected));
  assert(strcmp(u.nodename, expected) == 0);

  get_node_os_field("release", expected, sizeof(expected));
  assert(strcmp(u.release, expected) == 0);

  get_node_os_field("version", expected, sizeof(expected));
  assert(strcmp(u.version, expected) == 0);

  get_node_os_field("machine", expected, sizeof(expected));
  assert(strcmp(u.machine, expected) == 0);

  char hostname[sizeof(u.nodename)];
  assert(gethostname(hostname, sizeof(hostname)) == 0);
  assert(strcmp(hostname, u.nodename) == 0);

  printf("done\n");
  return 0;
}
