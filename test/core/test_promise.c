#include <assert.h>
#include <emscripten/console.h>
#include <emscripten/emscripten.h>
#include <emscripten/eventloop.h>
#include <emscripten/promise.h>
#include <emscripten/stack.h>
#include <stdint.h>
#include <stdlib.h>

static void do_exit(void* _) { emscripten_force_exit(0); }

// Report an error and asynchronously schedule a runtime exit.
static em_promise_result_t fail(void** result, void* data, void* value) {
  emscripten_console_logf("error! data: %ld", (uintptr_t)data);
  emscripten_async_call(do_exit, NULL, 0);
  return EM_PROMISE_REJECT;
}

// Used as a fulfillment handler. Report the success.
static em_promise_result_t
expect_success(void** result, void* data, void* value) {
  emscripten_console_logf("expected success: %ld", (uintptr_t)value);
  return EM_PROMISE_FULFILL;
}

// Used as an error handler. Report and drop the error.
static em_promise_result_t
expect_error(void** result, void* data, void* value) {
  emscripten_console_logf("expected error: %ld", (uintptr_t)value);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t test_create(void** result, void* data, void* value) {
  emscripten_console_log("test_create");
  assert(data == (void*)1);

  em_promise_t fulfilled = emscripten_promise_create();
  emscripten_promise_resolve(fulfilled, EM_PROMISE_FULFILL, (void*)1);
  emscripten_promise_destroy(
    emscripten_promise_then(fulfilled, expect_success, fail, NULL));

  em_promise_t rejected = emscripten_promise_create();
  emscripten_promise_resolve(rejected, EM_PROMISE_REJECT, (void*)2);
  emscripten_promise_destroy(
    emscripten_promise_then(rejected, fail, expect_error, NULL));

  em_promise_t match_fulfilled = emscripten_promise_create();
  emscripten_promise_resolve(match_fulfilled, EM_PROMISE_MATCH, fulfilled);
  em_promise_t finish1 =
    emscripten_promise_then(match_fulfilled, expect_success, fail, NULL);

  em_promise_t match_rejected = emscripten_promise_create();
  emscripten_promise_resolve(match_rejected, EM_PROMISE_MATCH, rejected);
  em_promise_t finish2 =
    emscripten_promise_then(match_rejected, fail, expect_error, NULL);

  emscripten_promise_destroy(fulfilled);
  emscripten_promise_destroy(rejected);
  emscripten_promise_destroy(match_fulfilled);
  emscripten_promise_destroy(match_rejected);

  em_promise_t to_finish[2] = {finish1, finish2};
  em_promise_t finish_test_create = emscripten_promise_all(to_finish, NULL, 2);

  emscripten_promise_destroy(finish1);
  emscripten_promise_destroy(finish2);

  *result = finish_test_create;
  return EM_PROMISE_MATCH_RELEASE;
}

static void called_async(void* data) {
  emscripten_console_log("Hello from a callback");
  em_promise_t promise = data;
  emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(promise);
}

static em_promise_result_t
test_promisify(void** result, void* data, void* value) {
  emscripten_console_log("test_promisify");
  assert(data == (void*)2);

  // Create a promise for an API based on callbacks.
  em_promise_t promise = emscripten_promise_create();
  emscripten_async_call(called_async, promise, 0);

  *result = promise;
  return EM_PROMISE_MATCH;
}

static em_promise_result_t
throw_string(void** result, void* data, void* value) {
  // The stack pointer should not be corrupted even though we don't make it to
  // the function epilogue.
  volatile int big_frame[128];
  EM_ASM({ throw "bang!"; });
  emscripten_console_log("unexpected success!");
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t throw_ptr(void** result, void* data, void* value) {
  // The stack pointer should not be corrupted even though we don't make it to
  // the function epilogue.
  volatile int big_frame[128];
  if (sizeof(void*) == 4) {
    EM_ASM({ throw 1234; });
  } else if (sizeof(void*) == 8) {
    EM_ASM({ throw 1234n; });
  }
  emscripten_console_log("unexpected success!");
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t
test_rejection(void** result, void* data, void* value) {
  emscripten_console_log("test_rejection");
  assert(data == (void*)3);

  // Reject by throwing in a success handler.
  em_promise_t start_fulfilled = emscripten_promise_create();
  em_promise_t rejected1 =
    emscripten_promise_then(start_fulfilled, throw_string, fail, NULL);
  em_promise_t recovered1 =
    emscripten_promise_then(rejected1, fail, expect_error, NULL);

  // Reject by throwing in a rejection handler.
  em_promise_t start_rejected = emscripten_promise_create();
  em_promise_t rejected2 =
    emscripten_promise_then(start_rejected, fail, throw_ptr, NULL);
  em_promise_t recovered2 =
    emscripten_promise_then(rejected2, fail, expect_error, NULL);

  em_promise_t to_finish[2] = {recovered1, recovered2};
  em_promise_t finish_test_rejection =
    emscripten_promise_all(to_finish, NULL, 2);

  emscripten_promise_resolve(start_fulfilled, EM_PROMISE_FULFILL, NULL);
  emscripten_promise_resolve(start_rejected, EM_PROMISE_REJECT, NULL);

  emscripten_promise_destroy(start_fulfilled);
  emscripten_promise_destroy(rejected1);
  emscripten_promise_destroy(recovered1);

  emscripten_promise_destroy(start_rejected);
  emscripten_promise_destroy(rejected2);
  emscripten_promise_destroy(recovered2);

  *result = finish_test_rejection;
  return EM_PROMISE_MATCH_RELEASE;
}

typedef struct promise_all_state {
  size_t size;
  em_promise_t in[3];
  void* out[3];
  void* expected[3];
  void* expected_err;
} promise_all_state;

static em_promise_result_t
check_promise_all_results(void** result, void* data, void* value) {
  promise_all_state* state = (promise_all_state*)data;
  assert(value == state->out);
  emscripten_console_log("promise_all results:");
  for (size_t i = 0; i < state->size; ++i) {
    emscripten_console_logf("%ld", (uintptr_t)state->out[i]);
    assert(state->out[i] == state->expected[i]);
  }
  free(state);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t
check_promise_all_error(void** result, void* data, void* value) {
  promise_all_state* state = (promise_all_state*)data;
  emscripten_console_logf("promise_all error: %ld", (uintptr_t)value);
  assert(value == state->expected_err);
  free(state);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t test_all(void** result, void* data, void* value) {
  emscripten_console_log("test_all");
  assert(data == (void*)4);

  // No input should result in success
  promise_all_state* state = malloc(sizeof(promise_all_state));
  *state = (promise_all_state){
    .size = 0, .in = {}, .out = {}, .expected = {}, .expected_err = NULL};
  em_promise_t empty =
    emscripten_promise_all(state->in, state->out, state->size);
  em_promise_t empty_checked =
    emscripten_promise_then(empty, check_promise_all_results, fail, state);
  emscripten_promise_destroy(empty);

  // Three fulfilled inputs should result in success.
  state = malloc(sizeof(promise_all_state));
  *state = (promise_all_state){.size = 3,
                               .in = {emscripten_promise_create(),
                                      emscripten_promise_create(),
                                      emscripten_promise_create()},
                               .out = {},
                               .expected = {(void*)42, (void*)1337, NULL},
                               .expected_err = NULL};
  em_promise_t full =
    emscripten_promise_all(state->in, state->out, state->size);
  em_promise_t full_checked =
    emscripten_promise_then(full, check_promise_all_results, fail, state);
  emscripten_promise_destroy(full);

  emscripten_promise_resolve(state->in[0], EM_PROMISE_FULFILL, (void*)42);
  emscripten_promise_resolve(state->in[1], EM_PROMISE_FULFILL, (void*)1337);
  emscripten_promise_resolve(state->in[2], EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(state->in[0]);
  emscripten_promise_destroy(state->in[1]);
  emscripten_promise_destroy(state->in[2]);

  // Similar, but now one of the inputs is rejected.
  state = malloc(sizeof(promise_all_state));
  *state = (promise_all_state){.size = 3,
                               .in = {emscripten_promise_create(),
                                      emscripten_promise_create(),
                                      emscripten_promise_create()},
                               .out = {},
                               .expected = {},
                               .expected_err = (void*)1337};
  em_promise_t err = emscripten_promise_all(state->in, state->out, state->size);
  em_promise_t err_checked =
    emscripten_promise_then(err, fail, check_promise_all_error, state);
  emscripten_promise_destroy(err);

  emscripten_promise_resolve(state->in[0], EM_PROMISE_FULFILL, (void*)42);
  emscripten_promise_resolve(state->in[1], EM_PROMISE_REJECT, (void*)1337);
  emscripten_promise_resolve(state->in[2], EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(state->in[0]);
  emscripten_promise_destroy(state->in[1]);
  emscripten_promise_destroy(state->in[2]);

  em_promise_t to_finish[3] = {empty_checked, full_checked, err_checked};
  em_promise_t finish_test_all = emscripten_promise_all(to_finish, NULL, 3);

  emscripten_promise_destroy(empty_checked);
  emscripten_promise_destroy(full_checked);
  emscripten_promise_destroy(err_checked);

  *result = finish_test_all;
  return EM_PROMISE_MATCH_RELEASE;
}

typedef struct promise_all_settled_state {
  size_t size;
  em_promise_t in[3];
  em_settled_result_t out[3];
  em_settled_result_t expected[3];
} promise_all_settled_state;

static em_promise_result_t
check_promise_all_settled_results(void** result, void* data, void* value) {
  promise_all_settled_state* state = (promise_all_settled_state*)data;
  assert(value == state->out);
  emscripten_console_log("promise_all_settled results:");
  for (size_t i = 0; i < state->size; ++i) {
    emscripten_console_logf(
      "%s %ld",
      state->out[i].result == EM_PROMISE_FULFILL ? "fulfill" : "reject",
      (uintptr_t)state->out[i].value);
    assert(state->out[i].result == state->expected[i].result);
    assert(state->out[i].value == state->expected[i].value);
  }
  free(state);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t check_null(void** result, void* data, void* value) {
  assert(value == NULL);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t
test_all_settled(void** result, void* data, void* value) {
  emscripten_console_log("test_all_settled");
  assert(data == (void*)5);

  // No input should be handled ok.
  promise_all_settled_state* state = malloc(sizeof(promise_all_settled_state));
  *state =
    (promise_all_settled_state){.size = 0, .in = {}, .out = {}, .expected = {}};
  em_promise_t empty =
    emscripten_promise_all_settled(state->in, state->out, state->size);
  em_promise_t empty_checked = emscripten_promise_then(
    empty, check_promise_all_settled_results, fail, state);
  emscripten_promise_destroy(empty);

  // Fulfilled and rejected inputs should be reported.
  state = malloc(sizeof(promise_all_settled_state));
  *state = (promise_all_settled_state){.size = 3,
                                       .in = {emscripten_promise_create(),
                                              emscripten_promise_create(),
                                              emscripten_promise_create()},
                                       .out = {},
                                       .expected = {
                                         {EM_PROMISE_FULFILL, (void*)42},
                                         {EM_PROMISE_REJECT, (void*)43},
                                         {EM_PROMISE_FULFILL, (void*)44},
                                       }};
  em_promise_t full =
    emscripten_promise_all_settled(state->in, state->out, state->size);
  em_promise_t full_checked = emscripten_promise_then(
    full, check_promise_all_settled_results, fail, state);
  emscripten_promise_destroy(full);
  emscripten_promise_resolve(state->in[0], EM_PROMISE_FULFILL, (void*)42);
  emscripten_promise_resolve(state->in[1], EM_PROMISE_REJECT, (void*)43);
  emscripten_promise_resolve(state->in[2], EM_PROMISE_FULFILL, (void*)44);
  emscripten_promise_destroy(state->in[0]);
  emscripten_promise_destroy(state->in[1]);
  emscripten_promise_destroy(state->in[2]);

  // Null buffer should be ok.
  em_promise_t null_in[3] = {
    emscripten_promise_create(),
    emscripten_promise_create(),
    emscripten_promise_create(),
  };
  em_promise_t null = emscripten_promise_all_settled(null_in, NULL, 3);
  em_promise_t null_checked =
    emscripten_promise_then(null, check_null, fail, NULL);
  emscripten_promise_destroy(null);
  emscripten_promise_resolve(null_in[0], EM_PROMISE_REJECT, (void*)42);
  emscripten_promise_resolve(null_in[1], EM_PROMISE_FULFILL, (void*)43);
  emscripten_promise_resolve(null_in[2], EM_PROMISE_REJECT, (void*)44);
  emscripten_promise_destroy(null_in[0]);
  emscripten_promise_destroy(null_in[1]);
  emscripten_promise_destroy(null_in[2]);

  em_promise_t to_finish[3] = {empty_checked, full_checked, null_checked};
  em_promise_t finish_test_all_settled =
    emscripten_promise_all(to_finish, NULL, 3);

  emscripten_promise_destroy(empty_checked);
  emscripten_promise_destroy(full_checked);
  emscripten_promise_destroy(null_checked);

  *result = finish_test_all_settled;
  return EM_PROMISE_MATCH_RELEASE;
}

typedef struct promise_any_state {
  size_t size;
  em_promise_t in[3];
  void* expected;
  void* err[3];
  void* expected_err[3];
} promise_any_state;

static em_promise_result_t
check_promise_any_result(void** result, void* data, void* value) {
  promise_any_state* state = (promise_any_state*)data;
  emscripten_console_logf("promise_any result: %ld", (uintptr_t)value);
  assert(value == state->expected);
  free(state);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t
check_promise_any_err(void** result, void* data, void* value) {
  promise_any_state* state = (promise_any_state*)data;
  assert(value == state->err);
  emscripten_console_log("promise_any reasons:");
  for (size_t i = 0; i < state->size; ++i) {
    emscripten_console_logf("%ld", (uintptr_t)state->err[i]);
    assert(state->err[i] == state->expected_err[i]);
  }
  free(state);
  return EM_PROMISE_FULFILL;
}

static em_promise_result_t test_any(void** result, void* data, void* value) {
  emscripten_console_log("test_any");
  assert(data == (void*)6);

  // No input should be handled ok and be rejected.
  promise_any_state* state = malloc(sizeof(promise_any_state));
  *state = (promise_any_state){
    .size = 0, .in = {}, .expected = NULL, .err = {}, .expected_err = {}};
  em_promise_t empty =
    emscripten_promise_any(state->in, state->err, state->size);
  em_promise_t empty_checked =
    emscripten_promise_then(empty, fail, check_promise_any_err, state);
  emscripten_promise_destroy(empty);

  // The first fulfilled promise should be propagated.
  state = malloc(sizeof(promise_any_state));
  *state = (promise_any_state){.size = 3,
                               .in = {emscripten_promise_create(),
                                      emscripten_promise_create(),
                                      emscripten_promise_create()},
                               .expected = (void*)42,
                               .err = {},
                               .expected_err = {}};
  em_promise_t full =
    emscripten_promise_any(state->in, state->err, state->size);
  em_promise_t full_checked =
    emscripten_promise_then(full, check_promise_any_result, fail, state);
  emscripten_promise_destroy(full);
  emscripten_promise_resolve(state->in[0], EM_PROMISE_REJECT, (void*)41);
  emscripten_promise_resolve(state->in[1], EM_PROMISE_FULFILL, (void*)42);
  emscripten_promise_resolve(state->in[2], EM_PROMISE_FULFILL, (void*)43);
  emscripten_promise_destroy(state->in[0]);
  emscripten_promise_destroy(state->in[1]);
  emscripten_promise_destroy(state->in[2]);

  // If all promises are rejected, the result will be rejected with the reasons.
  state = malloc(sizeof(promise_any_state));
  *state =
    (promise_any_state){.size = 3,
                        .in = {emscripten_promise_create(),
                               emscripten_promise_create(),
                               emscripten_promise_create()},
                        .expected = NULL,
                        .err = {},
                        .expected_err = {(void*)42, (void*)43, (void*)44}};
  em_promise_t rejected =
    emscripten_promise_any(state->in, state->err, state->size);
  em_promise_t rejected_checked =
    emscripten_promise_then(rejected, fail, check_promise_any_err, state);
  emscripten_promise_destroy(rejected);
  emscripten_promise_resolve(state->in[0], EM_PROMISE_REJECT, (void*)42);
  emscripten_promise_resolve(state->in[1], EM_PROMISE_REJECT, (void*)43);
  emscripten_promise_resolve(state->in[2], EM_PROMISE_REJECT, (void*)44);
  emscripten_promise_destroy(state->in[0]);
  emscripten_promise_destroy(state->in[1]);
  emscripten_promise_destroy(state->in[2]);

  // Same, but now the error reason buffer is null.
  em_promise_t null_in[3] = {
    emscripten_promise_create(),
    emscripten_promise_create(),
    emscripten_promise_create(),
  };
  em_promise_t null = emscripten_promise_any(null_in, NULL, 3);
  em_promise_t null_checked =
    emscripten_promise_then(null, fail, check_null, NULL);
  emscripten_promise_destroy(null);
  emscripten_promise_resolve(null_in[0], EM_PROMISE_REJECT, (void*)42);
  emscripten_promise_resolve(null_in[1], EM_PROMISE_REJECT, (void*)43);
  emscripten_promise_resolve(null_in[2], EM_PROMISE_REJECT, (void*)44);
  emscripten_promise_destroy(null_in[0]);
  emscripten_promise_destroy(null_in[1]);
  emscripten_promise_destroy(null_in[2]);

  em_promise_t to_finish[4] = {
    empty_checked, full_checked, rejected_checked, null_checked};
  em_promise_t finish_test_any = emscripten_promise_all(to_finish, NULL, 4);

  emscripten_promise_destroy(empty_checked);
  emscripten_promise_destroy(full_checked);
  emscripten_promise_destroy(rejected_checked);
  emscripten_promise_destroy(null_checked);

  *result = finish_test_any;
  return EM_PROMISE_MATCH_RELEASE;
}

static em_promise_result_t test_race(void** result, void* data, void* value) {
  emscripten_console_log("test_race");
  assert(data == (void*)7);

  em_promise_t in[2] = {emscripten_promise_create(),
                        emscripten_promise_create()};
  em_promise_t fulfill = emscripten_promise_race(in, 2);
  em_promise_t fulfill_checked =
    emscripten_promise_then(fulfill, expect_success, fail, NULL);
  emscripten_promise_destroy(fulfill);
  emscripten_promise_resolve(in[1], EM_PROMISE_FULFILL, (void*)42);
  emscripten_promise_resolve(in[0], EM_PROMISE_REJECT, NULL);
  emscripten_promise_destroy(in[0]);
  emscripten_promise_destroy(in[1]);

  in[0] = emscripten_promise_create();
  in[1] = emscripten_promise_create();
  em_promise_t reject = emscripten_promise_race(in, 2);
  em_promise_t reject_checked =
    emscripten_promise_then(reject, fail, expect_error, NULL);
  emscripten_promise_destroy(reject);
  emscripten_promise_resolve(in[0], EM_PROMISE_REJECT, (void*)42);
  emscripten_promise_resolve(in[1], EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(in[0]);
  emscripten_promise_destroy(in[1]);

  em_promise_t to_finish[2] = {fulfill_checked, reject_checked};
  em_promise_t finish_test_race = emscripten_promise_all(to_finish, NULL, 0);

  emscripten_promise_destroy(fulfill_checked);
  emscripten_promise_destroy(reject_checked);

  *result = finish_test_race;
  return EM_PROMISE_MATCH_RELEASE;
}

static em_promise_result_t finish(void** result, void* data, void* value) {
  emscripten_console_logf("finish");

  // We should not have leaked any handles.
  EM_ASM({
    promiseMap.allocated.forEach((p) => assert(typeof p === "undefined", "non-destroyed handle"));
  });

  // Cannot exit directly in a promise callback, since it would end up rejecting
  // the new promise. Instead, schedule a future shutdown outside of the promise
  // chain.
  emscripten_async_call(do_exit, NULL, 0);

  return EM_PROMISE_FULFILL;
}

static em_promise_result_t check_stack(void** result, void* data, void* value) {
  // Make sure the stack pointer is the same every time this is called.
  static uintptr_t expected_stack = 0;
  uintptr_t curr_stack = emscripten_stack_get_current();
  if (!expected_stack) {
    expected_stack = curr_stack;
    return EM_PROMISE_FULFILL;
  }
  if (curr_stack != expected_stack) {
    emscripten_console_logf("stack pointer at %p, expected %p",
                            (void*)curr_stack,
                            (void*)expected_stack);
    return EM_PROMISE_REJECT;
  }
  return EM_PROMISE_FULFILL;
}

int main() {
  em_promise_t start = emscripten_promise_create();
  em_promise_t measure_stack =
    emscripten_promise_then(start, check_stack, fail, NULL);
  em_promise_t test1 =
    emscripten_promise_then(measure_stack, test_create, fail, (void*)1);
  em_promise_t test2 =
    emscripten_promise_then(test1, test_promisify, fail, (void*)2);
  em_promise_t test3 =
    emscripten_promise_then(test2, test_rejection, fail, (void*)3);
  em_promise_t test4 = emscripten_promise_then(test3, test_all, fail, (void*)4);
  em_promise_t test5 =
    emscripten_promise_then(test4, test_all_settled, fail, (void*)5);
  em_promise_t test6 = emscripten_promise_then(test5, test_any, fail, (void*)6);
  em_promise_t test7 =
    emscripten_promise_then(test6, test_race, fail, (void*)7);
  em_promise_t assert_stack =
    emscripten_promise_then(test7, check_stack, fail, NULL);
  em_promise_t end = emscripten_promise_then(assert_stack, finish, fail, NULL);

  emscripten_promise_resolve(start, EM_PROMISE_FULFILL, NULL);

  emscripten_promise_destroy(start);
  emscripten_promise_destroy(measure_stack);
  emscripten_promise_destroy(test1);
  emscripten_promise_destroy(test2);
  emscripten_promise_destroy(test3);
  emscripten_promise_destroy(test4);
  emscripten_promise_destroy(test5);
  emscripten_promise_destroy(test6);
  emscripten_promise_destroy(test7);
  emscripten_promise_destroy(assert_stack);
  emscripten_promise_destroy(end);

  emscripten_runtime_keepalive_push();
}
