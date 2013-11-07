
#include <stdlib.h>

// force malloc&free to be included in from libc
struct Force {
  Force() {
    void *x = malloc(10);
    free(x);
  }
};

static Force f;

