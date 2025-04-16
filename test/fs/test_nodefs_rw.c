/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <emscripten.h>

int main() {
  FILE *file;
  int res;
  char buffer[512];

  // write something locally with node
  EM_ASM(
    var fs = require('fs');
    fs.writeFileSync('foobar.txt', 'yeehaw');
  );

  // read and validate the contents of the file
  file = fopen("foobar.txt", "r");
  assert(file);
  res = fread(buffer, sizeof(char), 6, file);
  assert(res == 6);
  buffer[6] = '\0';
  fclose(file);

  printf("fread -> '%s'\n", buffer);
  assert(!strcmp(buffer, "yeehaw"));

  // write out something new
  file = fopen("foobar.txt", "w");
  assert(file);
  res = fwrite("cheez", sizeof(char), 5, file);
  assert(res == 5);
  fclose(file);

  // validate the changes were persisted to the underlying fs
  EM_ASM(
    var fs = require('fs');
    var contents = fs.readFileSync('foobar.txt', { encoding: 'utf8' });
    assert(contents === 'cheez');
  );

  file = fopen("csfsq", "r");
  assert(file == NULL);
  assert(errno == ENOENT);

  puts("success");

  return 0;
}
