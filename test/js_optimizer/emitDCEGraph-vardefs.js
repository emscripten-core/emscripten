// Functions defined by assignment of an arrow function or function expression
// to a top-level variable are tracked like function declarations. This is the
// form in which JS library functions are emitted, so it is what allows
// breaking JS<->wasm dependency cycles (see issue #26038).

// The JS implementation of a wasm import references a wasm export, forming a
// JS<->wasm cycle. Nothing else uses either, so none of these may be rooted:
// metadce must be able to remove the entire cycle.
var __setitimer_js = (which, timeout) => {
  __emscripten_timeout(which, timeout);
};

// A function expression is tracked just like an arrow function.
function helped() {
}
var helper = function() {
  helped();
};

// A named function expression: the inner reference is to the expression
// itself, but we conservatively attribute it to the same-named top-level
// function, keeping it alive.
function recur() {
}
var namedFE = function recur() {
  recur();
};

// Reassigning a tracked name roots it: the assignment target is an identifier
// use, and the replacement body is treated as top-level code.
function usedByOriginal() {
}
function usedByReplacement() {
}
var reassigned = () => {
  usedByOriginal();
};
reassigned = () => {
  usedByReplacement();
};

// A variable whose name was already saved as a wasm export is not tracked
// (re-mapping the name would misattribute references to the export), so its
// contents are treated as top-level code, which roots them.
var _expD1 = wasmExports['expD1'];
function rootedByShadow() {
}
var _expD1 = () => {
  rootedByShadow();
};

// A variable function nested inside an untracked scope (an object method) is
// not tracked either; its contents are treated as top-level code.
function rootedFromMethod() {
}
var obj = {
  method() {
    var innerArrow = () => {
      rootedFromMethod();
    };
  },
};

// wasm exports received in the usual way.
var __emscripten_timeout = wasmExports['_emscripten_timeout'];

var wasmImports = {
  setitimer_js: __setitimer_js,
};
