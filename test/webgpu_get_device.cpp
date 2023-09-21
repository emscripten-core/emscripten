#include <stdio.h>

#include <emscripten/em_asm.h>
#include <emscripten/html5_webgpu.h>

__attribute__((constructor)) void init() {
  EM_ASM({
    Module['preinitializedWebGPUDevice'] = { this_is: 'a_dummy_object' };
  });
}

int main() {
  WGPUDevice d = emscripten_webgpu_get_device();
  printf("emscripten_webgpu_get_device: %p\n", d);
  return 0;
}
