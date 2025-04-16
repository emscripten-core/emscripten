#include <assert.h>
#include <dirent.h>
#include <emscripten.h>
#include <stdio.h>

void list_dir(const char *path) {
  printf("listing contents of dir=%s\n", path);
  struct dirent* entry;
  DIR* dir = opendir(path);
  assert(dir);
  int n = 0;
  while ((entry = readdir(dir)) != NULL) {
    printf("%s\n", entry->d_name);
    ++n;
  }
  assert(n);
  closedir(dir);
}

int main(int argc, char * argv[]) {
  list_dir("/");

  // mount the current folder as a NODEFS instance
  // inside of emscripten and create folders nodefs/a
  // in mounted directory
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
  );

  list_dir("/working");
  puts("success");
}
