#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/em_asm.h>
#include <emscripten/eventloop.h>

int fired_id;

void never(void *arg) {
  printf("cleared callback ran\n");
  abort();
}

void fired(void *arg) {
  printf("fired\n");
  // Clearing an id that already fired must be a no-op.
  emscripten_clear_timeout(fired_id);
  emscripten_clear_timeout(fired_id);
}

void immediate_fired(void *arg) {
  emscripten_clear_immediate(fired_id);
  emscripten_clear_immediate(fired_id);
}

// Only runs if the runtime actually exits.
void at_exit() {
  printf("done\n");
}

#define N 1000

#define PENDING_TIMEOUTS() EM_ASM_INT({ return safeSetTimeout.pending.size; })
#define PENDING_IMMEDIATES() EM_ASM_INT({ return setImmediateWrapped.pending.size; })

int count;
int last_id;

// Create-fire-clear per step, like a caller releasing a timer after it
// completes. The pending table must not grow.
void timeout_step(void *arg) {
  emscripten_clear_timeout(last_id);
  assert(PENDING_TIMEOUTS() == 0);
  if (++count < N) {
    last_id = emscripten_set_timeout(timeout_step, 0, NULL);
    assert(PENDING_TIMEOUTS() == 1);
  }
}

void immediate_step(void *arg) {
  emscripten_clear_immediate(last_id);
  assert(PENDING_IMMEDIATES() == 0);
  if (++count < N) {
    last_id = emscripten_set_immediate(immediate_step, NULL);
    assert(PENDING_IMMEDIATES() == 1);
  }
}

int main() {
  atexit(at_exit);
#if MODE_CLEARED
  // Clearing a pending timeout releases its keepalive so the runtime exits
  // from main without waiting for it (a leak fails the check below; a timer
  // that was not actually cleared aborts when it fires).
  int id = emscripten_set_timeout(never, 1000, NULL);
  assert(emscripten_runtime_keepalive_check());
  emscripten_clear_timeout(id);
  assert(!emscripten_runtime_keepalive_check());
  return 42;
#elif MODE_IDEMPOTENT
  int id = emscripten_set_timeout(never, 1000, NULL);
  emscripten_clear_timeout(id);
  emscripten_clear_timeout(id);
  fired_id = emscripten_set_timeout(fired, 0, NULL);
  // Still-pending timer must keep the runtime alive until it fires.
  emscripten_set_timeout(fired, 50, NULL);
  return 0;
#elif MODE_IMMEDIATE
  int id = emscripten_set_immediate(never, NULL);
  emscripten_clear_immediate(id);
  emscripten_clear_immediate(id);
  fired_id = emscripten_set_immediate(immediate_fired, NULL);
  emscripten_set_timeout(fired, 50, NULL);
  return 0;
#elif MODE_BOUNDED
  last_id = emscripten_set_timeout(timeout_step, 0, NULL);
  return 0;
#elif MODE_BOUNDED_IMMEDIATE
  last_id = emscripten_set_immediate(immediate_step, NULL);
  return 0;
#else
  emscripten_set_timeout(fired, 50, NULL);
  return 0;
#endif
}
