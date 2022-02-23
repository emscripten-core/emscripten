#include <emscripten.h>
#include <stdio.h>

double result = 0.0;

EMSCRIPTEN_KEEPALIVE
void set_result() {
  result += 2.1;
}

EMSCRIPTEN_KEEPALIVE
double get_result() {
  return result;
}

EM_ASYNC_JS(double, foo, (int timeout), {
  await new Promise(resolve => setTimeout(
    () => {
      // Do a simple call back into compiled code synchronously. This is
      // allowed (an async one would not).
      _set_result();
      // Do the same with ccall.
      ccall("set_result");
      // Finish the async operation.
      resolve();
    },
    timeout
  ));
  // Do another simple call back into compiled code synchronously.
  return _get_result();
});

int main() {
  printf("foo returned: %f\n", foo(10));
  return 0;
}
