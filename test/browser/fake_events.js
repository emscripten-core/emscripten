/*
 * Helper function used in browser tests to simulate HTML5 events
 */

function simulateKeyEvent(eventType, keyCode, code) {
  var props = { keyCode, charCode: keyCode, view: window, bubbles: true, cancelable: true };
  if (code) props['code'] = code;
  var event = new KeyboardEvent(eventType, props);
  return document.dispatchEvent(event);
}

function simulateKeyDown(keyCode, code = undefined) {
  var doDefault = simulateKeyEvent('keydown', keyCode, code);
  // As long as not handler called `preventDefault` we also send a keypress
  // event.
  if (doDefault) {
    simulateKeyEvent('keypress', keyCode, code);
  }
}

function simulateKeyUp(keyCode, code = undefined) {
  simulateKeyEvent('keyup', keyCode, code);
}

function simulateMouseEvent(x, y, button, absolute) {
  if (!absolute) {
    x += Module['canvas'].offsetLeft;
    y += Module['canvas'].offsetTop;
  }
  var event = document.createEvent("MouseEvents");
  if (button >= 0) {
    var event1 = document.createEvent("MouseEvents");
    event1.initMouseEvent('mousedown', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               button, null);
    Module['canvas'].dispatchEvent(event1);
    var event2 = document.createEvent("MouseEvents");
    event2.initMouseEvent('mouseup', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               button, null);
    Module['canvas'].dispatchEvent(event2);
  } else {
    var event1 = document.createEvent("MouseEvents");
    event1.initMouseEvent('mousemove', true, true, window,
               1, x, y, x, y,
               0, 0, 0, 0,
               0, null);
    Module['canvas'].dispatchEvent(event1);
  }
}
