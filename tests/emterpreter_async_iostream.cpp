#include <stdio.h>
#include <iostream>
#include <emscripten.h>
#include <html5.h>

using namespace std;

int seen = 0;

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void* userData) {
  cout << "mouse_callback+" << endl;
  seen++;
  return 0;
}

void mainLoop() {
  emscripten_sleep_with_yield(100);
  static int counter = 0;
  counter++;
  EM_ASM(
    function sendEvent(type, data) {
      setTimeout(function() {
        var event = document.createEvent('Event');
        event.initEvent(type, true, true);
        for(var d in data) event[d] = data[d];
        Module['canvas'].dispatchEvent(event);
      }, Math.random()*100);
    }
    sendEvent('mousedown', { screenX: 1, screenY: 1, clientX: 1, clientY: 1, button: 0, buttons: 1 });
  );
  cout << "sent event " << counter << "\n";
  if (seen >= 10) {
    emscripten_cancel_main_loop();
    cout << "Success.\n";
    int result = 1;
    REPORT_RESULT();
    return;
  }
  if (counter >= 100) {
    emscripten_cancel_main_loop();
    cout << "FAIL\n";
    int result = 9999;
    REPORT_RESULT();
    return;
  }
}

int main() {
  cout << "HelloWorld" << endl;
  emscripten_set_mousedown_callback("#canvas", 0, 1, mouse_callback);
  emscripten_set_main_loop(mainLoop, 0, 0);
  return 0;
}

