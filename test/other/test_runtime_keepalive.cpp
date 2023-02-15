#include <emscripten.h>
#include <stdio.h>

EM_JS_DEPS(main, "$runtimeKeepalivePush,$runtimeKeepalivePop,$callUserCallback");

int main() {
  EM_ASM({
    Module["onExit"] = () => { out("onExit"); };
    runtimeKeepalivePush();
    out("runtimeKeepalivePush done");
    counter = 0;
    function timerCallback() {
      if (counter < 5) {
        runtimeKeepalivePush();
        out("runtimeKeepalivePush done");
      } else {
        runtimeKeepalivePop();
        out("runtimeKeepalivePop done");
      }
      counter += 1;
      callUserCallback(() => {
        out("in user callback: " + counter);
      }, 0);
      if (!runtimeExited) {
        setTimeout(timerCallback, 0);
      }
    }
    setTimeout(timerCallback, 0);
  });
  puts("returning from main");
  return 0;
}
