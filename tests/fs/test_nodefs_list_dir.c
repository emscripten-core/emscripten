#include <assert.h>
#include <dirent.h>
#include <emscripten.h>
#include <stdio.h>


void list_dir(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);
    assert(dir);
    int n = 0;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n",entry->d_name);
        ++n;
    }
    assert(n);
    closedir(dir);
}

// Test that readdir works on a mount point
int main() {
  EM_ASM(
    FS.mkdir('working');
    FS.mount(NODEFS, { root: '.' }, 'working');
  );
  list_dir("working");
  puts("success");

  return 0;
}
