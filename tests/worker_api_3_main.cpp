#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int w1;

bool sawCalls[] = { false, false, false, false };

void c1(char *data, int size, void *arg) {
  assert((int)arg == 97);
  assert(size >= sizeof(int));

  int *x = (int*)data;
  printf("c1: %d\n", x[0]);

  if (*x >= 0 && *x < 4) {
    // Calls should have happened in order.
    sawCalls[*x] = true;  // Note the call with current param was made
    for (int i = 0; i < *x - 1; ++i) {
      if (!sawCalls[i]) {
        // If we were called out of order, fail this and all following calls.
        sawCalls[*x] = false;
        break;
      }
    }
  } else {
    assert(*x == 4);
    // This is the last call.  All prior calls should have occurred.
    int result = 1;  // Final call occurred.
    for (int i = 0; i < 4; ++i)
      if (sawCalls[i]) result++;
    REPORT_RESULT();
  }
}

int main() {
  w1 = emscripten_create_worker("worker.js");

  int x[1] = { 0 };
  emscripten_call_worker(w1, "one", (char*)x, sizeof(x), c1, (void*)97);

  return 0;
}

