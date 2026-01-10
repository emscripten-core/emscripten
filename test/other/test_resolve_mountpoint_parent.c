#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <emscripten.h>
#include "assert.h"

int main(int argc, char **argv) {
  EM_ASM({
    FS.mkdir('/working');
    FS.mkdir('/other');
    FS.mount(NODEFS, { root: '.' }, '/working');
  });
  struct stat statBuf;
  assert(stat("/working/../other", &statBuf) == 0);
}
