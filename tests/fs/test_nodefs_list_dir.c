#include <dirent.h>
#include <emscripten.h>
#include <stdio.h>


void list_dir(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n",entry->d_name);
    }
    closedir(dir);
}


int main() {
  EM_ASM(
    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');
  );
  list_dir("working");
  puts("success");

  return 0;
}
