#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE extern "C" void finish() {
  REPORT_RESULT(1);
}

int main() {
  EM_ASM({
    setTimeout(function() {
      function loadChildScript(name, then) {
        var js = document.createElement("script");
        if (then) js.onload = then;
        js.src = name;
        document.body.appendChild(js);
      }
      loadChildScript("more.js", function() {
        setTimeout(function() {
          Module['_finish']();
        }, 1000);
      });
    }, 1);
  });
}

