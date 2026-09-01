/*
 * Helper function used in browser tests to simulate HTML5 events
 */

// Derive `event.key` when not explicitly provided. For named keys (e.g.
// 'ArrowUp', 'Enter'), `key` matches `code`. For letter keys ('KeyA'..'KeyZ')
// or when `code` is omitted, derive `key` from `keyCode`.
function deriveKey(code, keyCode) {
  if (code && !code.startsWith('Key')) {
    return code;
  }
  return String.fromCharCode(keyCode);
}

function simulateKeyEvent(eventType, keyCode, code = undefined, key = undefined, target = undefined) {
  if (!key) key = deriveKey(code, keyCode);
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

function simulateKeyUp(keyCode, code = undefined, key = undefined, target = undefined) {
  simulateKeyEvent('keyup', keyCode, code, key, target);
}

function simulateKeyDownUp(keyCode, code = undefined, key = undefined, target = undefined) {
  simulateKeyDown(keyCode, code, key, target);
  simulateKeyUp(keyCode, code, key, target);
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
