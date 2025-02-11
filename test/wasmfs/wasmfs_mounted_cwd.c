#include <assert.h>
#include <emscripten/wasmfs.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  printf("WasmFS Mounted Directory CWD Test\n");

  backend_t backend = wasmfs_create_memory_backend();

  // Mount the backend at /tfs
  int r = wasmfs_create_directory("/tfs", 0777, backend);
  assert(r == 0);

  // Get initial working directory
  char initial_cwd[1024];
  char* result = getcwd(initial_cwd, sizeof(initial_cwd));
  assert(result != NULL);
  printf("Initial CWD: %s\n", initial_cwd);

  // Change to the mounted directory
  r = chdir("/tfs");
  assert(r == 0);

  // Get and verify the current working directory
  char buf[1024];
  result = getcwd(buf, sizeof(buf));
  assert(result != NULL);

  // This should be "/tfs" not "/"
  printf("CWD after chdir(\"/tfs\"): %s\n", buf);
  assert(strcmp(buf, "/tfs") == 0);

  // Test with a subdirectory
  // First create a subdirectory in the mounted directory
  errno = 0;
  assert(mkdir("/tfs/subdir", 0777) == 0); // Ensure mkdir succeeds

  // Change to the subdirectory
  assert(chdir("/tfs/subdir") == 0);

  // Get and verify the current working directory again
  result = getcwd(buf, sizeof(buf));
  assert(result != NULL);

  // This should be "/tfs/subdir" not "//subdir"
  printf("CWD after chdir(\"/tfs/subdir\"): %s\n", buf);
  assert(strcmp(buf, "/tfs/subdir") == 0);

  // Return to parent directory
  assert(chdir("..") == 0);

  result = getcwd(buf, sizeof(buf));
  assert(result != NULL);
  assert(strcmp(buf, "/tfs") == 0);

  // Test relative paths
  r = chdir("subdir");
  assert(r == 0);
  
  result = getcwd(buf, sizeof(buf));
  assert(result != NULL);
  assert(strcmp(buf, "/tfs/subdir") == 0);

  return 0;
}
