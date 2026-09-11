// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/jspi.h>
#include <stdio.h>
#include <stdlib.h>

static const char* name(jspi_event ev) {
  switch (ev) {
    case JSPI_ENTER: return "ENTER";
    case JSPI_EXIT: return "EXIT";
    case JSPI_SUSPEND: return "SUSPEND";
    case JSPI_RESUME: return "RESUME";
  }
  return "?";
}

// All output goes through printf: under PROXY_TO_PTHREAD console.log from the
// worker is not ordered with the proxied stdout writes.
EMSCRIPTEN_KEEPALIVE void js_log(char* line) {
  printf("%s\n", line);
  free(line);
}

// Each hook mints its own per-fiber token at ENTER (a counter) and gets it
// back at every later event of that fiber, where it may replace it. Hook A
// keeps its token; hook B increments it at every event.
static int next_a, next_b;

void* hook_a(jspi_event ev, void* token, int error) {
  if (ev == JSPI_ENTER) {
    assert(token == NULL);
    token = (void*)(uintptr_t)++next_a;
  }
  printf("A %s#%u%s\n", name(ev), (unsigned)(uintptr_t)token, error ? " err" : "");
  return token;
}

void* hook_b(jspi_event ev, void* token, int error) {
  if (ev == JSPI_ENTER) {
    assert(token == NULL);
    token = (void*)(uintptr_t)(++next_b * 100);
  }
  printf("B %s#%u%s\n", name(ev), (unsigned)(uintptr_t)token, error ? " err" : "");
  return (void*)((uintptr_t)token + 1);
}

// Only registered for SUSPEND/RESUME: never sees ENTER, so its first token for
// a fiber is NULL; it counts that fiber's suspensions.
void* suspend_counter(jspi_event ev, void* token, int error) {
  assert(ev == JSPI_SUSPEND || ev == JSPI_RESUME);
  if (ev == JSPI_SUSPEND) {
    token = (void*)((uintptr_t)token + 1);
  }
  printf("C %s suspensions=%u\n", name(ev), (unsigned)(uintptr_t)token);
  return token;
}

// A suspending import that rejects.
EM_ASYNC_JS(void, reject_after_tick, (), {
  await new Promise((resolve) => setTimeout(resolve, 0));
  throw new Error('boom');
});

// A suspending import that suspends and then re-enters wasm synchronously
// from JS through a promising export, before resolving.
EM_ASYNC_JS(int, nested_entry, (), {
  await Promise.resolve();
  const p = Module['_nested'](7);
  log('  nested returned promise:', p instanceof Promise);
  return await p;
});

EMSCRIPTEN_KEEPALIVE int nested(int x) {
  printf("nested body\n");
  emscripten_sleep(0);
  printf("nested after sleep\n");
  return x * 2;
}

// Called synchronously from a fiber or from JS: a plain export, no events of
// its own.
EMSCRIPTEN_KEEPALIVE int plain(int x) { return x + 1; }

// The rejection of the import propagates out of the promising export.
EMSCRIPTEN_KEEPALIVE int rejects(void) {
  emscripten_sleep(0);
  reject_after_tick();
  return 1;
}

EM_JS(void, run_js_tests, (), {
  Module['jsTests'] = (async () => {
    // Rejected promising export: RESUME and EXIT see the error, and the
    // exception object reaches the caller unchanged.
    const boom = await Module['_rejects']().then(
      () => 'resolved', (e) => e instanceof Error ? e.message : 'not an Error');
    log('rejects ->', boom);
    // Nested promising entry from inside a fiber's import.
    log('nested_entry ->', await Module['_run_nested']());
    // Plain export called synchronously from outside: no events.
    log('plain outside ->', Module['_plain'](100));
    // Function pointer made promising via dynCall.
    log('dyncall ->', await dynCall('ii', Module['_nested_ptr'](), [21], true));
  })();
});

EMSCRIPTEN_KEEPALIVE int run_nested(void) { return nested_entry(); }

EMSCRIPTEN_KEEPALIVE int nested_ptr(void) {
  return (int)(uintptr_t)&nested;
}

void* noop(jspi_event ev, void* token, int error) { return token; }

int main() {
  // Hooks run in registration order; main's own fiber was entered before
  // they existed, so they first see it at SUSPEND with a NULL token.
  assert(jspi_register(hook_a, JSPI_ALL) == 0);
  assert(jspi_register(hook_b, JSPI_ALL) == 0);
  assert(jspi_register(suspend_counter,
                       JSPI_SUSPEND | JSPI_RESUME) == 0);

  emscripten_sleep(0);
  printf("plain from fiber -> %d\n", plain(100));

  // Table capacity: three registrations so far.
  int ok = 0;
  while (jspi_register(noop, JSPI_SUSPEND) == 0) ok++;
  printf("registered %d more hooks, then -2\n", ok);
  assert(ok == JSPI_MAX_HOOKS - 3);
  assert(jspi_register(noop, JSPI_ALL) == -2);

  // The JS tests continue after main returns; exit from finish().
  emscripten_runtime_keepalive_push();
  EM_ASM({
    globalThis.log = (...args) => Module['_js_log'](stringToNewUTF8(args.join(' ')));
  });
  run_js_tests();
  EM_ASM({ Module['jsTests'].then(() => callUserCallback(Module['_finish'])); });
  return 0;
}

EMSCRIPTEN_KEEPALIVE void finish(void) {
  printf("done\n");
  emscripten_runtime_keepalive_pop();
  exit(0);
}
