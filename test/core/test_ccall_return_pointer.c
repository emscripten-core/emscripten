#include <emscripten.h>
#include <stdio.h>

int array[8];

int EMSCRIPTEN_KEEPALIVE *get_pointer_to_array_index(int i) {
  return &array[i];
}

// Fills array[0] ... end_ptr with given value.
void fill_array_up_to(int *end_ptr, int val);

int main() {
  fill_array_up_to(get_pointer_to_array_index(4), 42);
  for(int i = 0; i < 8; ++i) {
    printf("%d: %d\n", i, *get_pointer_to_array_index(i));
  }
}
