#include <errno.h>
#include <stdio.h>

int main() {
  const char *filename = "test.dat";

  // Create a file
  FILE *f = fopen(filename, "wb");
  if (f == NULL) {
    return 1;
  }
  // Write to it
  if (fwrite("XXXX", 1, 4, f) != 4) {
    return 1;
  }
  // Close it
  if (fclose(f)) {
    return 1;
  }
  // This write should fail
  if (fwrite("YYYY", 1, 4, f) != 0) {
    return 1;
  }
  // The error number is EBADF
  if (errno != EBADF) {
    return 1;
  }
  return 0;
}
