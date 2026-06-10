#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

EM_JS(EM_VAL, promise_fail_impl, (), {
  let promise = new Promise((_, reject) => setTimeout(reject, 1, new Error("bang from JS promise!")));
  let handle = Emval.toHandle(promise);
  // FIXME. See https://github.com/emscripten-core/emscripten/issues/16975.
#if __wasm64__
  handle = BigInt(handle);
#endif
  return handle;
});

val promise_fail() {
  return val::take_ownership(promise_fail_impl());
}

template <size_t N>
val failingPromise() {
  co_await failingPromise<N - 1>();
  co_return 65;
}

template <>
val failingPromise<0>() {
  co_await promise_fail();
  co_return 65;
}

EMSCRIPTEN_BINDINGS(test_val_coro) {
  function("failingPromise", failingPromise<3>);
}
