#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <assert.h>
#include <stdexcept>

using namespace emscripten;

EM_JS(EM_VAL, promise_sleep_impl, (int ms, int result), {
  let promise = new Promise(resolve => setTimeout(resolve, ms, result));
  let handle = Emval.toHandle(promise);
  // FIXME. See https://github.com/emscripten-core/emscripten/issues/16975.
#if __wasm64__
  handle = BigInt(handle);
#endif
  return handle;
});

val promise_sleep(int ms, int result = 0) {
  return val::take_ownership(promise_sleep_impl(ms, result));
}

// Test that we can subclass and make custom awaitable types.
template <typename T>
class typed_promise: public val {
public:
  typed_promise(val&& promise): val(std::move(promise)) {}

  auto operator co_await() const {
    struct typed_awaiter: public val::awaiter {
      T await_resume() {
        return val::awaiter::await_resume().template as<T>();
      }
    };

    return typed_awaiter(*this);
  }
};

val asyncCoro() {
  // check that just sleeping works
  co_await promise_sleep(1);
  // check that sleeping and receiving value works
  val v = co_await promise_sleep(1, 12);
  assert(v.as<int>() == 12);
  // check that awaiting a subclassed promise works and returns the correct type
  int x = co_await typed_promise<int>(promise_sleep(1, 23));
  assert(x == 23);
  // check that returning value works (checked by JS in tests)
  co_return 34;
}

val throwingCoro() {
  throw std::runtime_error("bang from throwingCoro!");
  co_return 56;
}

EMSCRIPTEN_BINDINGS(test_val_coro) {
  function("asyncCoro", asyncCoro);
  function("throwingCoro", throwingCoro);
}
