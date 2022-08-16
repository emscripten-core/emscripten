#include <emscripten/html5_webgl.h>

// This test just verifies the libgl can be linked with MAIN_MODULE=1.
// All we do here is reference a symbol libgl to cause it be added to the link
// line.

int main(int argc, char* argv[]) {
  emscripten_webgl_get_proc_address("foo");
  return 0;
}
