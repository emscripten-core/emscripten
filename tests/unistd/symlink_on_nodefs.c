#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <emscripten.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>

int main() {
  EM_ASM(
    fs.mkdirSync('./new-directory', '0777');
    fs.writeFileSync('./new-directory/test', 'Link it');
    fs.symlinkSync(fs.realpathSync('./new-directory'), './symlink');

    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');

    FS.mkdir('direct-link');
    FS.mount(NODEFS, { root: './symlink' }, 'direct-link');
  );

  {
    const char* path = "/working/symlink/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd == NULL) {
      printf("failed to open file %s\n", path);
    }
    else {
      char buffer[8];
      fread(buffer, 1, 7, fd);
      printf("buffer is %s\n", buffer);
      fclose(fd);
    }
  }

  printf("\n");

  {
    const char* path = "/direct-link/test";
    printf("reading %s\n", path);

    FILE* fd = fopen(path, "r");
    if (fd != NULL) {
      // This should not happen, it resolves to ../new-directory which is not mounted
      printf("opened file %s\n", path);
      fclose(fd);
    }
    else {
      printf("failed to open file %s\n", path);
    }
  }
  
  return 0;
}
