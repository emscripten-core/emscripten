#include <assert.h>
#include <emscripten.h>

extern "C" {

// Respond with 0, 1, 2, 3 each with finalResponse=false, and 4 with
// finalResponse=true.
void one(char *data, int size) {
  int *x = (int*)data;

  if (*x == 0) {
    // Respond 0, 1, 2, 3
    for (int i = 0; i < 4; ++i) {
      *x = i;
      emscripten_worker_respond_provisionally(data, size);
    }
  }

  // Respond 4
  *x = 4;
  emscripten_worker_respond(data, size);
}

}

