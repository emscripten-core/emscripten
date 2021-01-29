/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/html5.h>

// The event handler functions can return 1 to suppress the event and disable the default action. That calls event.preventDefault();
// Returning 0 signals that the event was not consumed by the code, and will allow the event to pass on and bubble up normally.
EM_BOOL click_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
  return 1;
}

int main(int argc, char **argv)
{
  // Suppress the event's default action early via a capture handler.
  emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, click_callback);

  EM_ASM({
    // Check if the earlier prevention was successful.
    window.addEventListener("click", function(e) {
      var result = e.defaultPrevented;
      if (result) {
        console.log("Test passed!")
      } else {
        console.log("Test failed! Event default must be prevented in earlier handler.")
      }
      reportResultToServer(Number(result));
    }, false);

    function sendEvent(type, data) {
      var event = document.createEvent('Event');
      event.initEvent(type, true, true);
      for(var d in data) event[d] = data[d];
      window.dispatchEvent(event);
    }
    sendEvent('click', { screenX: 123, screenY: 456, clientX: 123, clientY: 456, button: 0, buttons: 1 });
  });

  return 0;
}
