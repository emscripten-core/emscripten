#include <emscripten.h>
#include <stdio.h>

double result = 0.0;

EMSCRIPTEN_KEEPALIVE
void inc_result() {
  puts("inc result");
  // this will be called twice, leading to 4.2 as the final result.
  // (we use floating-point numbers here to make sure that the final result is
  // returned using the proper type, and not an int)
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
      _inc_result();
      // Do the same with ccall.
      ccall("inc_result");
      // Finish the async operation.
      out("resolving promise");
      resolve();
    },
    timeout
  ));
  // Do another simple call back into compiled code synchronously.
  return _get_result();
});

// Test out void return and no params.
EM_ASYNC_JS(void, bar, (), {
  return;
});

int main() {
  printf("foo returned: %f\n", foo(10));
  bar();
  return 0;
}
