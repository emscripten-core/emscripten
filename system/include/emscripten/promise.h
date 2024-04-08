/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// EXPERIMENTAL AND SUBJECT TO CHANGE!

// An opaque handle to a JS Promise object.
typedef struct _em_promise* em_promise_t;

typedef enum em_promise_result_t {
  EM_PROMISE_FULFILL,
  EM_PROMISE_MATCH,
  EM_PROMISE_MATCH_RELEASE,
  EM_PROMISE_REJECT,
} em_promise_result_t;

// A callback passed to `emscripten_promise_then` to be invoked once a promise
// is fulfilled or rejected. `data` is arbitrary user-provided data provided
// when `emscripten_promise_then` is called to install the callback and `value`
// is the value the promise was fulfilled or rejected with.
//
// The callback can signal how to resolve the new promise returned from
// `emscripten_promise_then` via its return and by writing a new result to
// outparam `result`. The behavior depends on the returned `em_promise_result_t`
// value:
//
//  - `EM_PROMISE_FULFILL`: The new promise is fulfilled with the value written
//    to `result` or NULL if no value is written.
//
//  - `EM_PROMISE_MATCH` or `EM_PROMISE_MATCH_RELEASE`: The callback must write
//    a promise handle to `result` and the new promise is resolved to match the
//    eventual state of that promise. `EM_PROMISE_MATCH_RELEASE` will also cause
//    the written promise handle to be destroyed so that the user does not have
//    to arrange for it to be destroyed after the callback is executed.
//
//  - `EM_PROMISE_REJECT`: The new promise is rejected with the reason written
//    to `result` or NULL if no reason is written.
//
// If the callback throws a number (or bigint in the case of memory64), the new
// promise will be rejected with that number converted to a pointer as its
// rejection reason. If the callback throws any other value, the new promise
// will be rejected with a NULL rejection reason.
typedef em_promise_result_t (*em_promise_callback_t)(void** result,
                                                     void* data,
                                                     void* value);

// Create a new promise that can be explicitly resolved or rejected using
// `emscripten_promise_resolve`. The returned promise handle must eventually be
// freed with `emscripten_promise_destroy`.
__attribute__((warn_unused_result)) em_promise_t
emscripten_promise_create(void);

// Release the resources associated with this promise. This must be called on
// every promise handle created, whether by `emscripten_promise_create` or any
// other function that returns a fresh promise, such as
// `emscripten_promise_then`. It is fine to call `emscripten_promise_destroy` on
// a promise handle before the promise is resolved; the configured callbacks
// will still be called.
void emscripten_promise_destroy(em_promise_t promise);

// Explicitly resolve the `promise` created by `emscripten_promise_create`. If
// `result` is `EM_PROMISE_FULFILL`, then the promise is fulfilled with the
// given `value`. If `result` is `EM_PROMISE_MATCH`, then the promise is
// resolved to match the eventual state of `value` interpreted as a promise
// handle. Finally, if `result` is `EM_PROMISE_REJECT`, then the promise is
// rejected with the given value. Promises not created by
// `emscripten_promise_create` should not be passed to this function.
void emscripten_promise_resolve(em_promise_t promise,
                                em_promise_result_t result,
                                void* value);

// Install `on_fulfilled` and `on_rejected` callbacks on the given `promise`,
// creating and returning a handle to a new promise. See `em_promise_callback_t`
// for documentation on how the callbacks work. `data` is arbitrary user data
// that will be passed to the callbacks. The returned promise handle must
// eventually be freed with `emscripten_promise_destroy`.
__attribute__((warn_unused_result)) em_promise_t
emscripten_promise_then(em_promise_t promise,
                        em_promise_callback_t on_fulfilled,
                        em_promise_callback_t on_rejected,
                        void* data);

// Call Promise.all to create and return a new promise that is either fulfilled
// once the `num_promises` input promises passed in `promises` have been
// fulfilled or is rejected once any of the input promises has been rejected.
// When the returned promise is fulfilled, the values each of the input promises
// were resolved with will be written to the `results` array if it is non-null
// and the returned promise will be fulfilled with the address of that array as
// well.
__attribute__((warn_unused_result)) em_promise_t emscripten_promise_all(
  em_promise_t* promises, void** results, size_t num_promises);

typedef struct em_settled_result_t {
  em_promise_result_t result;
  void* value;
} em_settled_result_t;

// Call Promise.allSettled to create and return a new promise that is fulfilled
// once the `num_promises` input promises passed in `promises` have been
// settled. When the returned promise is fulfilled, the `results` buffer will be
// filled with the result comprising of either EM_PROMISE_FULFILL and the
// fulfilled value or EM_PROMISE_REJECT and the rejection reason for each of the
// input promises if `results` is non-null. The returned promise will be
// fulfilled with the value of `results` as well.
__attribute__((warn_unused_result)) em_promise_t emscripten_promise_all_settled(
  em_promise_t* promises, em_settled_result_t* results, size_t num_promises);

// Call Promise.any to create and return a new promise that is fulfilled once
// any of the `num_promises` input promises passed in `promises` has been
// fulfilled or is rejected once all of the input promises have been rejected.
// If the returned promise is fulfilled, it will be fulfilled with the same
// value as the first fulfilled input promise. Otherwise, if the returned
// promise is rejected, the rejection reasons for each input promise will be
// written to the `errors` buffer if it is non-null. The rejection reason for
// the returned promise will also be the address of the `errors` buffer.
__attribute__((warn_unused_result)) em_promise_t emscripten_promise_any(
  em_promise_t* promises, void** errors, size_t num_promises);

// Call Promise.race to create and return a new promise that settles once any of
// the `num_promises` input promises passed in `promises` has been settled. If
// the first input promise to settle is fulfilled, the resulting promise is
// fulfilled with the same value. Otherwise, if the first input promise to
// settle is rejected, the resulting promise is rejected with the same reason.
__attribute__((warn_unused_result)) em_promise_t
emscripten_promise_race(em_promise_t* promises, size_t num_promises);

// Suspend the current Wasm execution context until the given promise has been
// settled.
//
// Since the stack is not unwound while Wasm execution is suspended, it is
// safe to pass pointers to the stack to asynchronous work that is waited on
// with this function.
//
// This function can only be used in programs that were built with `-sASYNCIFY`.
__attribute__((warn_unused_result)) em_settled_result_t
emscripten_promise_await(em_promise_t promise);

#ifdef __cplusplus
}
#endif
