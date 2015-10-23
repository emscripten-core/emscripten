#include <assert.h>
#include <stdio.h>
#include <string.h>
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

  // mount the current folder as a NODEFS instance
  // inside of emscripten
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
  );

  // read and validate the contents of the file
  file = fopen("/working/foobar.txt", "r");
  assert(file);
  res = fread(buffer, sizeof(char), 6, file);
  assert(res == 6);
  fclose(file);

  assert(!strcmp(buffer, "yeehaw"));

  // write out something new
  file = fopen("/working/foobar.txt", "w");
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

  puts("success");

  return 0;
}
