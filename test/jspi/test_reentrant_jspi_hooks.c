// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// The built-in stack handling as seen from user hooks and from JS entry
// paths: a hook on all events checks the fiber's frame as it sees it (live
// before the leave at SUSPEND, again at RESUME); a suspending import called
// outside any fiber gets no events and consumes nothing; ccall with a string
// argument and a promising dynCall run as concurrent fibers.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/eventloop.h>
#include <emscripten/jspi.h>
#include <emscripten/stack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRAME 1024

static const char* name(jspi_event ev) {
  switch (ev) {
    case JSPI_ENTER: return "ENTER";
    case JSPI_EXIT: return "EXIT";
    case JSPI_SUSPEND: return "SUSPEND";
    case JSPI_RESUME: return "RESUME";
  }
  return "?";
}
static volatile unsigned char* frames[8];
static int hook_events;
static int hook_failures;

static int verify(volatile unsigned char* buf, int n, unsigned char pattern) {
  for (int i = 0; i < n; i++) {
    if (buf[i] != (unsigned char)(pattern + i)) {
      return 0;
    }
  }
  return 1;
}

// The fiber about to suspend announces its slot here; the hook picks it up
// at that SUSPEND as its token for the fiber and keeps it from then on.
static unsigned suspending;

void* hook(jspi_event ev, void* token, int error) {
  hook_events++;
  unsigned id = (unsigned)(uintptr_t)token;
  if (ev == JSPI_SUSPEND && !id) {
    id = suspending;
    suspending = 0;
    token = (void*)(uintptr_t)id;
  }
  if ((ev == JSPI_SUSPEND || ev == JSPI_RESUME) && id && frames[id]) {
    // The frame is live at SUSPEND and at RESUME, and the hook runs below it
    // on the same stack.
    int intact = verify(frames[id], FRAME, id);
    int below = emscripten_stack_get_current() < (uintptr_t)frames[id];
    if (!intact || !below) {
      printf("%s#%u: intact=%d below=%d\n", name(ev), id, intact, below);
      hook_failures++;
    }
  }
  return token;
}

EMSCRIPTEN_KEEPALIVE int fiber(int id, int ms) {
  volatile unsigned char buf[FRAME];
  for (int i = 0; i < FRAME; i++) {
    buf[i] = (unsigned char)(id + i);
  }
  frames[id] = buf;
  suspending = id;
  emscripten_sleep(ms);
  frames[id] = NULL;
  return verify(buf, FRAME, id);
}

// Called from JS via ccall with a string argument; the string must survive the
// suspension and other fibers' resumes.
EMSCRIPTEN_KEEPALIVE int with_string(const char* s) {
  char copy[64];
  strcpy(copy, s);
  emscripten_sleep(5);
  return strcmp(copy, s) == 0;
}

// Called from a plain export: the suspension fails with a SuspendError.
EMSCRIPTEN_KEEPALIVE int plain_sleep(void) {
  emscripten_sleep(1);
  return 1;
}

EMSCRIPTEN_KEEPALIVE int fiber_ptr(void) { return (int)(uintptr_t)&fiber; }

EMSCRIPTEN_KEEPALIVE uintptr_t stack_pointer(void) {
  return emscripten_stack_get_current();
}

EMSCRIPTEN_KEEPALIVE void finish(int ok, int sp_balanced) {
  printf("hook events: %d, hook failures: %d\n", hook_events > 0, hook_failures);
  printf("sp balanced: %d\n", sp_balanced);
  if (ok && sp_balanced && hook_events && !hook_failures) {
    printf("done\n");
  }
  emscripten_runtime_keepalive_pop();
  exit(0);
}

EM_JS(void, start, (), {
  (async () => {
    var sp = Module['_stack_pointer']();
    var ok = 1;
    var a = Module['_fiber'](1, 1);
    var b = Module['_fiber'](2, 10);
    // Suspending outside a fiber: SuspendError, stack untouched.
    try {
      Module['_plain_sleep']();
      ok = 0;
    } catch (e) {
      console.log('plain export:', e.constructor.name);
    }
    ok &= Module['_stack_pointer']() == sp;
    // ccall string argument across a suspension, interleaved with a and b.
    var c = ccall('with_string', 'number', ['string'], ['hello fibers'], {async: true});
    // Function pointer made promising.
    var d = dynCall('iii', Module['_fiber_ptr'](), [3, 3], true);
    ok &= (await a) & (await b) & (await c) & (await d);
    callUserCallback(() => Module['_finish'](ok, Module['_stack_pointer']() == sp));
  })();
});

int main() {
  jspi_register(hook, JSPI_ALL);
  emscripten_runtime_keepalive_push();
  EM_ASM({ setTimeout(start, 0); });
  return 0;
}
