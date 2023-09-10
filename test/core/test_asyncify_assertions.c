#include <stdio.h>
#include <emscripten.h>

EM_JS(void, suspend, (), {
  Asyncify.handleSleep((wakeUp) => {
    Module.resume = wakeUp;
    setTimeout(function() {
      Module._resume_from_inside_c();
    }, 1);
  });
});

EM_JS(void, resume, (), {
  Module.resume();
});

EMSCRIPTEN_KEEPALIVE
void resume_from_inside_c() {
  // A C function that then resumes. This C code will be on the stack as we
  // try to resume, but that is invalid - we need to start a resume from the
  // *outside* of the wasm module. An assertion should catch this.
  resume();
}

int main() {
  printf("suspend\n");
  suspend();
  suspend();
  suspend();
  suspend();
  suspend();
  printf("finish\n");
}
