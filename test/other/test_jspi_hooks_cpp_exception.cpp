// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <emscripten/em_js.h>
#include <emscripten/jspi.h>
#include <stdexcept>
#include <stdio.h>

static const char* name(jspi_event ev) {
  switch (ev) {
    case JSPI_ENTER: return "ENTER";
    case JSPI_EXIT: return "EXIT";
    case JSPI_SUSPEND: return "SUSPEND";
    case JSPI_RESUME: return "RESUME";
  }
  return "?";
}

static unsigned next_id;

void* hook(jspi_event ev, void* token, int error) {
  if (ev == JSPI_ENTER) {
    token = (void*)(uintptr_t)++next_id;
  }
  printf("%s#%u%s\n", name(ev), (unsigned)(uintptr_t)token, error ? " err" : "");
  return token;
}

EM_ASYNC_JS(void, tick, (), { await Promise.resolve(); });

extern "C" {

EMSCRIPTEN_KEEPALIVE void throws_after_suspend() {
  tick();
  throw std::runtime_error("cpp");
}

EMSCRIPTEN_KEEPALIVE void caught_inside() {
  try {
    tick();
    throw std::runtime_error("cpp");
  } catch (const std::exception& e) {
    printf("caught %s in wasm\n", e.what());
  }
}

}

EM_JS(void, run_tests, (), {
  Module['done'] = (async () => {
    try {
      await Module['_throws_after_suspend']();
    } catch (e) {
      console.log('rejected with', e instanceof WebAssembly.Exception ? 'WebAssembly.Exception' : e);
    }
    await Module['_caught_inside']();
    console.log('done');
  })();
});

int main() {
  jspi_register(hook, JSPI_ALL);
  run_tests();
}
