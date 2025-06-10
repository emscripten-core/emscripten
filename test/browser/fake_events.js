/*
 * Helper function used in browser tests to simulate HTML5 events
 */

function simulateKeyEvent(eventType, keyCode, code, key, target) {
  var props = { keyCode, charCode: keyCode, view: window, bubbles: true, cancelable: true };
  if (code) props['code'] = code;
  if (key) props['key'] = key;
  var event = new KeyboardEvent(eventType, props);
  if (!target) target = document;
  return target.dispatchEvent(event);
}

function simulateKeyDown(keyCode, code = undefined, key = undefined, target = undefined) {
  var doDefault = simulateKeyEvent('keydown', keyCode, code, key, target);
  // As long as not handler called `preventDefault` we also send a keypress
  // event.
  if (doDefault) {
    simulateKeyEvent('keypress', keyCode, code, key, target);
  }
}

function simulateKeyUp(keyCode, code = undefined, target = undefined) {
  simulateKeyEvent('keyup', keyCode, code, target);
}

function simulateKeyDownUp(keyCode, code = undefined, target = undefined) {
  simulateKeyDown(keyCode, code, target);
  simulateKeyUp(keyCode, code, target);
}

function simulateMouseEvent(eventType, x, y, button, absolute) {
  if (!absolute) {
    x += Module['canvas'].offsetLeft;
    y += Module['canvas'].offsetTop;
  }
  var event = document.createEvent("MouseEvents");
  event.initMouseEvent(eventType, true, true, window,
             1, x, y, x, y,
             0, 0, 0, 0,
             button, null);
  Module['canvas'].dispatchEvent(event);
}

function simulateMouseDown(x, y, button, absolute) {
  simulateMouseEvent('mousedown', x, y, button, absolute);
}

function simulateMouseUp(x, y, button, absolute) {
  simulateMouseEvent('mouseup', x, y, button, absolute);
}

function simulateMouseMove(x, y, absolute) {
  simulateMouseEvent('mousemove', x, y, 0, absolute);
}

function simulateMouseClick(x, y, button, absolute) {
  simulateMouseDown(x, y, button, absolute);
  simulateMouseUp(x, y, button, absolute);
}
