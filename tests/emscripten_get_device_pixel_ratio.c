#include <assert.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

int main() {
  double devicePixelRatio = emscripten_get_device_pixel_ratio();
  printf("window.devicePixelRatio = %f.\n", devicePixelRatio);
  assert(devicePixelRatio > 0);
  return 0;
}
