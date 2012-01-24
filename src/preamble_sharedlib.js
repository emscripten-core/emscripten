// === Auto-generated preamble library stuff ===

//========================================
// Runtime essentials
//========================================

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.pop();
    var func = callback.func;
    if (typeof func === 'number') {
      func = FUNCTION_TABLE[func];
    }
    func(callback.arg === undefined ? null : callback.arg);
  }
}

var __ATINIT__ = []; // functions called during startup
var STRING_TABLE = [];

function initRuntime() {
  callRuntimeCallbacks(__ATINIT__);
}

// === Body ===

