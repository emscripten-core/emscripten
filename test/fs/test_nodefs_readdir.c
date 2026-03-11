#include <assert.h>
#include <dirent.h>
#include <emscripten.h>
#include <stdio.h>
#include <string.h>

int compare_strings(const void* a, const void* b) {
  return strcmp(*(const char**)a, *(const char**)b);
}

void list_dir(const char *path) {
  char* entries[64];

  printf("listing contents of dir=%s\n", path);
  struct dirent* entry;
  DIR* dir = opendir(path);
  assert(dir);
  int n = 0;
  while ((entry = readdir(dir)) != NULL) {
    entries[n] = strdup(entry->d_name);
    ++n;
  }
  assert(n);

  // Sort the array of entries before printing them so that this
  // part of the test output is deterministic.
  printf("sorted contents of dir=%s:\n", path);
  qsort(entries, n, sizeof(char*), compare_strings);
  for (int i = 0; i < n; i++) {
    printf("- %s\n", entries[i]);
    free(entries[i]);
  }

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
