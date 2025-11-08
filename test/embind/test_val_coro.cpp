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

EM_JS(EM_VAL, promise_fail_impl, (), {
  let promise = new Promise((_, reject) => setTimeout(reject, 1, new Error("bang from JS promise!")));
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

val promise_fail() {
  return val::take_ownership(promise_fail_impl());
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

template <size_t N>
val asyncCoro() {
  co_return co_await asyncCoro<N - 1>();
}

template <>
val asyncCoro<0>() {
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

template <size_t N>
val throwingCoro() {
  co_await throwingCoro<N - 1>();
  co_return 56;
}

template <>
val throwingCoro<0>() {
  throw std::runtime_error("bang from throwingCoro!");
  co_return 56;
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
  function("asyncCoro", asyncCoro<3>);
  function("throwingCoro", throwingCoro<3>);
  function("failingPromise", failingPromise<3>);
}
