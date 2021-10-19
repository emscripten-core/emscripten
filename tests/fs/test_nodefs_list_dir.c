#include <assert.h>
#include <dirent.h>
#include <emscripten.h>
#include <stdio.h>



void list_dir(const char *path) {
    printf("listing contents of dir=%s\n",path);
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

int main(int argc, char * argv[]) {
  // mount the current folder as a NODEFS instance
  // inside of emscripten and create folders nodefs/a
  // in mounted directory
  EM_ASM(
    FS.mkdir('/working');
    FS.mount(NODEFS, { root: '.' }, '/working');
    FS.mkdir('/working/nodefs');
    FS.mkdir('/working/nodefs/a');
  );
  list_dir("/working");
  // query folders created through NODEFS
  list_dir("/working/nodefs");
  // query existing folders within mount point
  list_dir("/working/existing");
  puts("success");
}
