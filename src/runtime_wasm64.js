/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !MEMORY64
#error "should only be inclded in MEMORY64 mode"
#endif

// In memory64 mode wasm pointers are 64-bit.  In JS these show up as BigInts.
// For now, we keep JS as much the same as it always was, that is, stackAlloc()
// receives and returns a Number from the JS point of view - we translate
// BigInts automatically for that.
// TODO: support minified export names, so we can turn
// MINIFY_WASM_IMPORTS_AND_EXPORTS back on for MEMORY64.
// TODO: Remove this hacky mechanism and replace with something more like the
// `__sig` attributes we have in JS library code.
function instrumentWasmExportsForMemory64(exports) {
  var instExports = {};
  for (var name in exports) {
    (function(name) {
      var original = exports[name];
      var replacement = original;
      if (['sbrk', 'stackAlloc', 'emscripten_builtin_malloc', 'malloc', '__getTypeName'].includes(name)) {
        // get one i64, return an i64.
        replacement = (x) => {
          var r = Number(original(BigInt(x ? x : 0)));
          return r;
        };
      } else if (['setThrew', 'free', 'stackRestore', '__cxa_is_pointer_type'].includes(name)) {
        // get one i64
        replacement = (x) => {
          original(BigInt(x));
        };
      } else if (['stackSave', 'emscripten_stack_get_end',
                  'emscripten_stack_get_base', 'pthread_self',
                  'emscripten_stack_get_current',
                  '__errno_location'].includes(name)) {
        // return an i64
        replacement = () => {
          var r = Number(original());
          return r;
        };
      } else if (name === 'emscripten_builtin_memalign') {
        // get two i64, return an i64
        replacement = (x, y) => {
          var r = Number(original(BigInt(x), BigInt(y)));
          return r;
        };
      } else if (name === 'main') {
        // Special case for main.  Use `function` here rather than arrow
        // function to avoid implicit `strict`.
        replacement = function(x, y) {
          // Pass an extra 0 in case its a 3-argument form of main.  Sadly we
          // can't just omit that argument like we can for wasm32 because the
          // missing third argument will generate:
          // `TypeError: Cannot convert undefined to a BigInt`.
          // See https://github.com/WebAssembly/JS-BigInt-integration/issues/12
          return original(x, BigInt(y ? y : 0), BigInt(0));
        };
      } else if (['emscripten_stack_set_limits', '__set_stack_limits'].includes(name)) {
        // get 2 i64 arguments
        replacement = (x, y) => {
          var r = original(BigInt(x), BigInt(y));
          return r;
        };
      } else if (name === '__cxa_can_catch') {
        // get 3 i64 arguments
        replacement = (x, y, z) => {
          var r = original(BigInt(x), BigInt(y), BigInt(z));
          return r;
        };
      }
      instExports[name] = replacement;
    })(name);
  }
  return instExports;
}
