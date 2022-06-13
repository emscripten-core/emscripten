#include <emscripten.h>
#include <emscripten/html5_webgpu.h>

int main() {
  EM_ASM({
    Module['preinitializedWebGPUDevice'] = { this_is: 'a_dummy_object' };
  });
  emscripten_webgpu_get_device();
}
