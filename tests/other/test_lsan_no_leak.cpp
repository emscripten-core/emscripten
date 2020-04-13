#include <stdlib.h>

void *global_ptr;

int main(int argc, char **argv) {
  void *local_ptr = new short[21];
  free(local_ptr);
  global_ptr = new char[1337];
  delete [] new long long[256];
}
