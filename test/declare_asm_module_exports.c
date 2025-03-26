#include <assert.h>
#include <emscripten.h>
#include <stdio.h>

int EMSCRIPTEN_KEEPALIVE cFunction(void) {
  return 1;
}

EM_JS(int, jsFunction, (), {
  return _cFunction();
});

// Intentional use of __main_argc_argv
int main(int argc, char* argv[]) {
  printf("cFunction: %d\n", cFunction());
  printf("jsFunction: %d\n", jsFunction());
  assert(jsFunction() == 1);
  return 0;
}
