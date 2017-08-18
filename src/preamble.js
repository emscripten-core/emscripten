// {{PREAMBLE_ADDITIONS}}

// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

//========================================
// Runtime code shared with compiler
//========================================

{{RUNTIME}}

#if RELOCATABLE
Runtime.GLOBAL_BASE = Runtime.alignMemory(Runtime.GLOBAL_BASE, {{{ MAX_GLOBAL_ALIGN || 1 }}});
#endif

{{{ maybeExport('Runtime') }}}
#if USE_CLOSURE_COMPILER
Runtime['addFunction'] = Runtime.addFunction;
Runtime['removeFunction'] = Runtime.removeFunction;
#endif

#if BENCHMARK
Module.realPrint = Module.print;
Module.print = Module.printErr = function(){};
#endif

#if SAFE_HEAP
function getSafeHeapType(bytes, isFloat) {
  switch (bytes) {
    case 1: return 'i8';
    case 2: return 'i16';
    case 4: return isFloat ? 'float' : 'i32';
    case 8: return 'double';
    default: assert(0);
  }
}

#if SAFE_HEAP_LOG
var SAFE_HEAP_COUNTER = 0;
#endif

function SAFE_HEAP_STORE(dest, value, bytes, isFloat) {
#if SAFE_HEAP_LOG
  Module.print('SAFE_HEAP store: ' + [dest, value, bytes, isFloat, SAFE_HEAP_COUNTER++]);
#endif
  if (dest <= 0) abort('segmentation fault storing ' + bytes + ' bytes to address ' + dest);
  if (dest % bytes !== 0) abort('alignment error storing to address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (staticSealed) {
    if (dest + bytes > HEAP32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when storing ' + bytes + ' bytes to address ' + dest + '. STATICTOP=' + STATICTOP + ', DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
    assert(DYNAMICTOP_PTR);
    assert(HEAP32[DYNAMICTOP_PTR>>2] <= TOTAL_MEMORY);
  } else {
    if (dest + bytes > STATICTOP) abort('segmentation fault, exceeded the top of the available static heap when storing ' + bytes + ' bytes to address ' + dest + '. STATICTOP=' + STATICTOP);
  }
  setValue(dest, value, getSafeHeapType(bytes, isFloat), 1);
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  SAFE_HEAP_STORE(dest, value, bytes, true);
}

function SAFE_HEAP_LOAD(dest, bytes, unsigned, isFloat) {
  if (dest <= 0) abort('segmentation fault loading ' + bytes + ' bytes from address ' + dest);
  if (dest % bytes !== 0) abort('alignment error loading from address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (staticSealed) {
    if (dest + bytes > HEAP32[DYNAMICTOP_PTR>>2]) abort('segmentation fault, exceeded the top of the available dynamic heap when loading ' + bytes + ' bytes from address ' + dest + '. STATICTOP=' + STATICTOP + ', DYNAMICTOP=' + HEAP32[DYNAMICTOP_PTR>>2]);
    assert(DYNAMICTOP_PTR);
    assert(HEAP32[DYNAMICTOP_PTR>>2] <= TOTAL_MEMORY);
  } else {
    if (dest + bytes > STATICTOP) abort('segmentation fault, exceeded the top of the available static heap when loading ' + bytes + ' bytes from address ' + dest + '. STATICTOP=' + STATICTOP);
  }
  var type = getSafeHeapType(bytes, isFloat);
  var ret = getValue(dest, type, 1);
  if (unsigned) ret = unSign(ret, parseInt(type.substr(1)), 1);
#if SAFE_HEAP_LOG
  Module.print('SAFE_HEAP load: ' + [dest, ret, bytes, isFloat, unsigned, SAFE_HEAP_COUNTER++]);
#endif
  return ret;
}
function SAFE_HEAP_LOAD_D(dest, bytes, unsigned) {
  return SAFE_HEAP_LOAD(dest, bytes, unsigned, true);
}

function SAFE_FT_MASK(value, mask) {
  var ret = value & mask;
  if (ret !== value) {
    abort('Function table mask error: function pointer is ' + value + ' which is masked by ' + mask + ', the likely cause of this is that the function pointer is being called by the wrong type.');
  }
  return ret;
}

function segfault() {
  abort('segmentation fault');
}
function alignfault() {
  abort('alignment fault');
}
function ftfault() {
  abort('Function table mask error');
}
#endif

//========================================
// Runtime essentials
//========================================

var ABORT = 0; // whether we are quitting the application. no code should run after this. set in exit() and abort()
var EXITSTATUS = 0;

/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

var globalScope = this;

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
  if (!func) {
    {{{ makeEval("try { func = eval('_' + ident); } catch(e) {}") }}}
  }
  assert(func, 'Cannot call unknown function ' + ident + ' (perhaps LLVM optimizations or closure removed it?)');
  return func;
}

var cwrap, ccall;
(function(){
  var JSfuncs = {
    // Helpers for cwrap -- it can't refer to Runtime directly because it might
    // be renamed by closure, instead it calls JSfuncs['stackSave'].body to find
    // out what the minified function name is.
    'stackSave': function() {
      Runtime.stackSave()
    },
    'stackRestore': function() {
      Runtime.stackRestore()
    },
    // type conversion from js to c
    'arrayToC' : function(arr) {
      var ret = Runtime.stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    },
    'stringToC' : function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = Runtime.stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    }
  };
  // For fast lookup of conversion functions
  var toC = {'string' : JSfuncs['stringToC'], 'array' : JSfuncs['arrayToC']};

  // C calling interface.
  ccall = function ccallFunc(ident, returnType, argTypes, args, opts) {
    var func = getCFunc(ident);
    var cArgs = [];
    var stack = 0;
#if ASSERTIONS
    assert(returnType !== 'array', 'Return type should not be "array".');
#endif
    if (args) {
      for (var i = 0; i < args.length; i++) {
        var converter = toC[argTypes[i]];
        if (converter) {
          if (stack === 0) stack = Runtime.stackSave();
          cArgs[i] = converter(args[i]);
        } else {
          cArgs[i] = args[i];
        }
      }
    }
    var ret = func.apply(null, cArgs);
#if ASSERTIONS
    if ((!opts || !opts.async) && typeof EmterpreterAsync === 'object') {
      assert(!EmterpreterAsync.state, 'cannot start async op with normal JS calling ccall');
    }
    if (opts && opts.async) assert(!returnType, 'async ccalls cannot return values');
#endif
    if (returnType === 'string') ret = Pointer_stringify(ret);
    if (stack !== 0) {
      if (opts && opts.async) {
        EmterpreterAsync.asyncFinalizers.push(function() {
          Runtime.stackRestore(stack);
        });
        return;
      }
      Runtime.stackRestore(stack);
    }
    return ret;
  }

#if NO_DYNAMIC_EXECUTION == 0
  var sourceRegex = /^function\s*[a-zA-Z$_0-9]*\s*\(([^)]*)\)\s*{\s*([^*]*?)[\s;]*(?:return\s*(.*?)[;\s]*)?}$/;
  function parseJSFunc(jsfunc) {
    // Match the body and the return value of a javascript function source
    var parsed = jsfunc.toString().match(sourceRegex).slice(1);
    return {arguments : parsed[0], body : parsed[1], returnValue: parsed[2]}
  }

  // sources of useful functions. we create this lazily as it can trigger a source decompression on this entire file
  var JSsource = null;
  function ensureJSsource() {
    if (!JSsource) {
      JSsource = {};
      for (var fun in JSfuncs) {
        if (JSfuncs.hasOwnProperty(fun)) {
          // Elements of toCsource are arrays of three items:
          // the code, and the return value
          JSsource[fun] = parseJSFunc(JSfuncs[fun]);
        }
      }
    }
  }

  cwrap = function cwrap(ident, returnType, argTypes) {
    argTypes = argTypes || [];
    var cfunc = getCFunc(ident);
    // When the function takes numbers and returns a number, we can just return
    // the original function
    var numericArgs = argTypes.every(function(type){ return type === 'number'});
    var numericRet = (returnType !== 'string');
    if ( numericRet && numericArgs) {
      return cfunc;
    }
    // Creation of the arguments list (["$1","$2",...,"$nargs"])
    var argNames = argTypes.map(function(x,i){return '$'+i});
    var funcstr = "(function(" + argNames.join(',') + ") {";
    var nargs = argTypes.length;
    if (!numericArgs) {
      // Generate the code needed to convert the arguments from javascript
      // values to pointers
      ensureJSsource();
      funcstr += 'var stack = ' + JSsource['stackSave'].body + ';';
      for (var i = 0; i < nargs; i++) {
        var arg = argNames[i], type = argTypes[i];
        if (type === 'number') continue;
        var convertCode = JSsource[type + 'ToC']; // [code, return]
        funcstr += 'var ' + convertCode.arguments + ' = ' + arg + ';';
        funcstr += convertCode.body + ';';
        funcstr += arg + '=(' + convertCode.returnValue + ');';
      }
    }

    // When the code is compressed, the name of cfunc is not literally 'cfunc' anymore
    var cfuncname = parseJSFunc(function(){return cfunc}).returnValue;
    // Call the function
    funcstr += 'var ret = ' + cfuncname + '(' + argNames.join(',') + ');';
    if (!numericRet) { // Return type can only by 'string' or 'number'
      // Convert the result to a string
      var strgfy = parseJSFunc(function(){return Pointer_stringify}).returnValue;
      funcstr += 'ret = ' + strgfy + '(ret);';
    }
#if ASSERTIONS
    funcstr += "if (typeof EmterpreterAsync === 'object') { assert(!EmterpreterAsync.state, 'cannot start async op with normal JS calling cwrap') }";
#endif
    if (!numericArgs) {
      // If we had a stack, restore it
      ensureJSsource();
      funcstr += JSsource['stackRestore'].body.replace('()', '(stack)') + ';';
    }
    funcstr += 'return ret})';
    return eval(funcstr);
  };
#else
  // NO_DYNAMIC_EXECUTION is on, so we can't use the fast version of cwrap.
  // Fall back to returning a bound version of ccall.
  cwrap = function cwrap(ident, returnType, argTypes) {
    return function() {
#if ASSERTIONS
      Runtime.warnOnce('NO_DYNAMIC_EXECUTION was set, '
                     + 'using slow cwrap implementation');
#endif
      return ccall(ident, returnType, argTypes, arguments);
    }
  }
#endif
})();
{{{ maybeExport("ccall") }}}
{{{ maybeExport("cwrap") }}}

/** @type {function(number, number, string, boolean=)} */
function setValue(ptr, value, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1', undefined, undefined, undefined, '1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8', undefined, undefined, undefined, '1') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16', undefined, undefined, undefined, '1') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32', undefined, undefined, undefined, '1') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64', undefined, undefined, undefined, '1') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float', undefined, undefined, undefined, '1') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double', undefined, undefined, undefined, '1') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': {{{ makeSetValue('ptr', '0', 'value', 'i1') }}}; break;
      case 'i8': {{{ makeSetValue('ptr', '0', 'value', 'i8') }}}; break;
      case 'i16': {{{ makeSetValue('ptr', '0', 'value', 'i16') }}}; break;
      case 'i32': {{{ makeSetValue('ptr', '0', 'value', 'i32') }}}; break;
      case 'i64': {{{ makeSetValue('ptr', '0', 'value', 'i64') }}}; break;
      case 'float': {{{ makeSetValue('ptr', '0', 'value', 'float') }}}; break;
      case 'double': {{{ makeSetValue('ptr', '0', 'value', 'double') }}}; break;
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
}
{{{ maybeExport("setValue") }}}

/** @type {function(number, string, boolean=)} */
function getValue(ptr, type, noSafe) {
  type = type || 'i8';
  if (type.charAt(type.length-1) === '*') type = 'i32'; // pointers are 32-bit
#if SAFE_HEAP
  if (noSafe) {
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1', undefined, undefined, undefined, undefined, '1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8', undefined, undefined, undefined, undefined, '1') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16', undefined, undefined, undefined, undefined, '1') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32', undefined, undefined, undefined, undefined, '1') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64', undefined, undefined, undefined, undefined, '1') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float', undefined, undefined, undefined, undefined, '1') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double', undefined, undefined, undefined, undefined, '1') }}};
      default: abort('invalid type for setValue: ' + type);
    }
  } else {
#endif
    switch(type) {
      case 'i1': return {{{ makeGetValue('ptr', '0', 'i1') }}};
      case 'i8': return {{{ makeGetValue('ptr', '0', 'i8') }}};
      case 'i16': return {{{ makeGetValue('ptr', '0', 'i16') }}};
      case 'i32': return {{{ makeGetValue('ptr', '0', 'i32') }}};
      case 'i64': return {{{ makeGetValue('ptr', '0', 'i64') }}};
      case 'float': return {{{ makeGetValue('ptr', '0', 'float') }}};
      case 'double': return {{{ makeGetValue('ptr', '0', 'double') }}};
      default: abort('invalid type for setValue: ' + type);
    }
#if SAFE_HEAP
  }
#endif
  return null;
}
{{{ maybeExport("getValue") }}}

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_STATIC = 2; // Cannot be freed
var ALLOC_DYNAMIC = 3; // Cannot be freed except through sbrk
var ALLOC_NONE = 4; // Do not allocate
{{{ maybeExport('ALLOC_NORMAL') }}}
{{{ maybeExport('ALLOC_STACK') }}}
{{{ maybeExport('ALLOC_STATIC') }}}
{{{ maybeExport('ALLOC_DYNAMIC') }}}
{{{ maybeExport('ALLOC_NONE') }}}

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
/** @type {function((TypedArray|Array<number>|number), string, number, number=)} */
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [typeof _malloc === 'function' ? _malloc : Runtime.staticAlloc, Runtime.stackAlloc, Runtime.staticAlloc, Runtime.dynamicAlloc][allocator === undefined ? ALLOC_STATIC : allocator](Math.max(size, singleType ? 1 : types.length));
  }

  if (zeroinit) {
    var ptr = ret, stop;
    assert((ret & 3) == 0);
    stop = ret + (size & ~3);
    for (; ptr < stop; ptr += 4) {
      {{{ makeSetValue('ptr', '0', '0', 'i32', null, true) }}};
    }
    stop = ret + size;
    while (ptr < stop) {
      {{{ makeSetValue('ptr++', '0', '0', 'i8', null, true) }}};
    }
    return ret;
  }

  if (singleType === 'i8') {
    if (slab.subarray || slab.slice) {
      HEAPU8.set(/** @type {!Uint8Array} */ (slab), ret);
    } else {
      HEAPU8.set(new Uint8Array(slab), ret);
    }
    return ret;
  }

  var i = 0, type, typeSize, previousType;
  while (i < size) {
    var curr = slab[i];

    if (typeof curr === 'function') {
      curr = Runtime.getFunctionIndex(curr);
    }

    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }
#if ASSERTIONS
    assert(type, 'Must know what type to store in allocate!');
#endif

    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later

    setValue(ret+i, curr, type);

    // no need to look up size unless type changes, so cache it
    if (previousType !== type) {
      typeSize = Runtime.getNativeTypeSize(type);
      previousType = type;
    }
    i += typeSize;
  }

  return ret;
}
{{{ maybeExport('allocate') }}}

// Allocate memory during any stage of startup - static memory early on, dynamic memory later, malloc when ready
function getMemory(size) {
  if (!staticSealed) return Runtime.staticAlloc(size);
  if (!runtimeInitialized) return Runtime.dynamicAlloc(size);
  return _malloc(size);
}
{{{ maybeExport('getMemory') }}}

/** @type {function(number, number=)} */
function Pointer_stringify(ptr, length) {
  if (length === 0 || !ptr) return '';
  // TODO: use TextDecoder
  // Find the length, and check for UTF while doing so
  var hasUtf = 0;
  var t;
  var i = 0;
  while (1) {
#if ASSERTIONS
    assert(ptr + i < TOTAL_MEMORY);
#endif
    t = {{{ makeGetValue('ptr', 'i', 'i8', 0, 1) }}};
    hasUtf |= t;
    if (t == 0 && !length) break;
    i++;
    if (length && i == length) break;
  }
  if (!length) length = i;

  var ret = '';

  if (hasUtf < 128) {
    var MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
    var curr;
    while (length > 0) {
      curr = String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
      ret = ret ? ret + curr : curr;
      ptr += MAX_CHUNK;
      length -= MAX_CHUNK;
    }
    return ret;
  }
  return Module['UTF8ToString'](ptr);
}
{{{ maybeExport('Pointer_stringify') }}}

// Given a pointer 'ptr' to a null-terminated ASCII-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function AsciiToString(ptr) {
  var str = '';
  while (1) {
    var ch = {{{ makeGetValue('ptr++', 0, 'i8') }}};
    if (!ch) return str;
    str += String.fromCharCode(ch);
  }
}
{{{ maybeExport('AsciiToString') }}}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in ASCII form. The copy will require at most str.length+1 bytes of space in the HEAP.

function stringToAscii(str, outPtr) {
  return writeAsciiToMemory(str, outPtr, false);
}
{{{ maybeExport('stringToAscii') }}}

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the given array that contains uint8 values, returns
// a copy of that string as a Javascript String object.

#if TEXTDECODER
var UTF8Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf8') : undefined;
#endif
function UTF8ArrayToString(u8Array, idx) {
#if TEXTDECODER
  var endPtr = idx;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  while (u8Array[endPtr]) ++endPtr;

  if (endPtr - idx > 16 && u8Array.subarray && UTF8Decoder) {
    return UTF8Decoder.decode(u8Array.subarray(idx, endPtr));
  } else {
#endif
    var u0, u1, u2, u3, u4, u5;

    var str = '';
    while (1) {
      // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
      u0 = u8Array[idx++];
      if (!u0) return str;
      if (!(u0 & 0x80)) { str += String.fromCharCode(u0); continue; }
      u1 = u8Array[idx++] & 63;
      if ((u0 & 0xE0) == 0xC0) { str += String.fromCharCode(((u0 & 31) << 6) | u1); continue; }
      u2 = u8Array[idx++] & 63;
      if ((u0 & 0xF0) == 0xE0) {
        u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
      } else {
        u3 = u8Array[idx++] & 63;
        if ((u0 & 0xF8) == 0xF0) {
          u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | u3;
        } else {
          u4 = u8Array[idx++] & 63;
          if ((u0 & 0xFC) == 0xF8) {
            u0 = ((u0 & 3) << 24) | (u1 << 18) | (u2 << 12) | (u3 << 6) | u4;
          } else {
            u5 = u8Array[idx++] & 63;
            u0 = ((u0 & 1) << 30) | (u1 << 24) | (u2 << 18) | (u3 << 12) | (u4 << 6) | u5;
          }
        }
      }
      if (u0 < 0x10000) {
        str += String.fromCharCode(u0);
      } else {
        var ch = u0 - 0x10000;
        str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
      }
    }
#if TEXTDECODER
  }
#endif
}
{{{ maybeExport('UTF8ArrayToString') }}}

// Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

function UTF8ToString(ptr) {
  return UTF8ArrayToString({{{ heapAndOffset('HEAPU8', 'ptr') }}});
}
{{{ maybeExport('UTF8ToString') }}}

// Copies the given Javascript String object 'str' to the given byte array at address 'outIdx',
// encoded in UTF8 form and null-terminated. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outU8Array: the array to copy to. Each index in this array is assumed to be one 8-byte element.
//   outIdx: The starting offset in the array to begin the copying.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=1, only the null terminator will be written and nothing else.
//                    maxBytesToWrite=0 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8Array(str, outU8Array, outIdx, maxBytesToWrite) {
  if (!(maxBytesToWrite > 0)) // Parameter maxBytesToWrite is not optional. Negative values, 0, null, undefined and false each don't write out any bytes.
    return 0;

  var startIdx = outIdx;
  var endIdx = outIdx + maxBytesToWrite - 1; // -1 for string null terminator.
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    // For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) {
      if (outIdx >= endIdx) break;
      outU8Array[outIdx++] = u;
    } else if (u <= 0x7FF) {
      if (outIdx + 1 >= endIdx) break;
      outU8Array[outIdx++] = 0xC0 | (u >> 6);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0xFFFF) {
      if (outIdx + 2 >= endIdx) break;
      outU8Array[outIdx++] = 0xE0 | (u >> 12);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0x1FFFFF) {
      if (outIdx + 3 >= endIdx) break;
      outU8Array[outIdx++] = 0xF0 | (u >> 18);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else if (u <= 0x3FFFFFF) {
      if (outIdx + 4 >= endIdx) break;
      outU8Array[outIdx++] = 0xF8 | (u >> 24);
      outU8Array[outIdx++] = 0x80 | ((u >> 18) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    } else {
      if (outIdx + 5 >= endIdx) break;
      outU8Array[outIdx++] = 0xFC | (u >> 30);
      outU8Array[outIdx++] = 0x80 | ((u >> 24) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 18) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 12) & 63);
      outU8Array[outIdx++] = 0x80 | ((u >> 6) & 63);
      outU8Array[outIdx++] = 0x80 | (u & 63);
    }
  }
  // Null-terminate the pointer to the buffer.
  outU8Array[outIdx] = 0;
  return outIdx - startIdx;
}
{{{ maybeExport('stringToUTF8Array') }}}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF8 form. The copy will require at most str.length*4+1 bytes of space in the HEAP.
// Use the function lengthBytesUTF8 to compute the exact number of bytes (excluding null terminator) that this function will write.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF8(str, outPtr, maxBytesToWrite) {
#if ASSERTIONS
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF8(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
  return stringToUTF8Array(str, {{{ heapAndOffset('HEAPU8', 'outPtr') }}}, maxBytesToWrite);
}
{{{ maybeExport('stringToUTF8') }}}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF8 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF8(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! So decode UTF16->UTF32->UTF8.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var u = str.charCodeAt(i); // possibly a lead surrogate
    if (u >= 0xD800 && u <= 0xDFFF) u = 0x10000 + ((u & 0x3FF) << 10) | (str.charCodeAt(++i) & 0x3FF);
    if (u <= 0x7F) {
      ++len;
    } else if (u <= 0x7FF) {
      len += 2;
    } else if (u <= 0xFFFF) {
      len += 3;
    } else if (u <= 0x1FFFFF) {
      len += 4;
    } else if (u <= 0x3FFFFFF) {
      len += 5;
    } else {
      len += 6;
    }
  }
  return len;
}
{{{ maybeExport('lengthBytesUTF8') }}}

// Given a pointer 'ptr' to a null-terminated UTF16LE-encoded string in the emscripten HEAP, returns
// a copy of that string as a Javascript String object.

var UTF16Decoder = typeof TextDecoder !== 'undefined' ? new TextDecoder('utf-16le') : undefined;
function UTF16ToString(ptr) {
#if ASSERTIONS
  assert(ptr % 2 == 0, 'Pointer passed to UTF16ToString must be aligned to two bytes!');
#endif
#if TEXTDECODER
  var endPtr = ptr;
  // TextDecoder needs to know the byte length in advance, it doesn't stop on null terminator by itself.
  // Also, use the length info to avoid running tiny strings through TextDecoder, since .subarray() allocates garbage.
  var idx = endPtr >> 1;
  while (HEAP16[idx]) ++idx;
  endPtr = idx << 1;

  if (endPtr - ptr > 32 && UTF16Decoder) {
    return UTF16Decoder.decode(HEAPU8.subarray(ptr, endPtr));
  } else {
#endif
    var i = 0;

    var str = '';
    while (1) {
      var codeUnit = {{{ makeGetValue('ptr', 'i*2', 'i16') }}};
      if (codeUnit == 0) return str;
      ++i;
      // fromCharCode constructs a character from a UTF-16 code unit, so we can pass the UTF16 string right through.
      str += String.fromCharCode(codeUnit);
    }
#if TEXTDECODER
  }
#endif
}
{{{ maybeExport('UTF16ToString') }}}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF16 form. The copy will require at most str.length*4+2 bytes of space in the HEAP.
// Use the function lengthBytesUTF16() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=2, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<2 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF16(str, outPtr, maxBytesToWrite) {
#if ASSERTIONS
  assert(outPtr % 2 == 0, 'Pointer passed to stringToUTF16 must be aligned to two bytes!');
#endif
#if ASSERTIONS
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF16(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 2) return 0;
  maxBytesToWrite -= 2; // Null terminator.
  var startPtr = outPtr;
  var numCharsToWrite = (maxBytesToWrite < str.length*2) ? (maxBytesToWrite / 2) : str.length;
  for (var i = 0; i < numCharsToWrite; ++i) {
    // charCodeAt returns a UTF-16 encoded code unit, so it can be directly written to the HEAP.
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    {{{ makeSetValue('outPtr', 0, 'codeUnit', 'i16') }}};
    outPtr += 2;
  }
  // Null-terminate the pointer to the HEAP.
  {{{ makeSetValue('outPtr', 0, 0, 'i16') }}};
  return outPtr - startPtr;
}
{{{ maybeExport('stringToUTF16') }}}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF16(str) {
  return str.length*2;
}
{{{ maybeExport('lengthBytesUTF16') }}}

function UTF32ToString(ptr) {
#if ASSERTIONS
  assert(ptr % 4 == 0, 'Pointer passed to UTF32ToString must be aligned to four bytes!');
#endif
  var i = 0;

  var str = '';
  while (1) {
    var utf32 = {{{ makeGetValue('ptr', 'i*4', 'i32') }}};
    if (utf32 == 0)
      return str;
    ++i;
    // Gotcha: fromCharCode constructs a character from a UTF-16 encoded code (pair), not from a Unicode code point! So encode the code point to UTF-16 for constructing.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    if (utf32 >= 0x10000) {
      var ch = utf32 - 0x10000;
      str += String.fromCharCode(0xD800 | (ch >> 10), 0xDC00 | (ch & 0x3FF));
    } else {
      str += String.fromCharCode(utf32);
    }
  }
}
{{{ maybeExport('UTF32ToString') }}}

// Copies the given Javascript String object 'str' to the emscripten HEAP at address 'outPtr',
// null-terminated and encoded in UTF32 form. The copy will require at most str.length*4+4 bytes of space in the HEAP.
// Use the function lengthBytesUTF32() to compute the exact number of bytes (excluding null terminator) that this function will write.
// Parameters:
//   str: the Javascript string to copy.
//   outPtr: Byte address in Emscripten HEAP where to write the string to.
//   maxBytesToWrite: The maximum number of bytes this function can write to the array. This count should include the null
//                    terminator, i.e. if maxBytesToWrite=4, only the null terminator will be written and nothing else.
//                    maxBytesToWrite<4 does not write any bytes to the output, not even the null terminator.
// Returns the number of bytes written, EXCLUDING the null terminator.

function stringToUTF32(str, outPtr, maxBytesToWrite) {
#if ASSERTIONS
  assert(outPtr % 4 == 0, 'Pointer passed to stringToUTF32 must be aligned to four bytes!');
#endif
#if ASSERTIONS
  assert(typeof maxBytesToWrite == 'number', 'stringToUTF32(str, outPtr, maxBytesToWrite) is missing the third parameter that specifies the length of the output buffer!');
#endif
  // Backwards compatibility: if max bytes is not specified, assume unsafe unbounded write is allowed.
  if (maxBytesToWrite === undefined) {
    maxBytesToWrite = 0x7FFFFFFF;
  }
  if (maxBytesToWrite < 4) return 0;
  var startPtr = outPtr;
  var endPtr = startPtr + maxBytesToWrite - 4;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i); // possibly a lead surrogate
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) {
      var trailSurrogate = str.charCodeAt(++i);
      codeUnit = 0x10000 + ((codeUnit & 0x3FF) << 10) | (trailSurrogate & 0x3FF);
    }
    {{{ makeSetValue('outPtr', 0, 'codeUnit', 'i32') }}};
    outPtr += 4;
    if (outPtr + 4 > endPtr) break;
  }
  // Null-terminate the pointer to the HEAP.
  {{{ makeSetValue('outPtr', 0, 0, 'i32') }}};
  return outPtr - startPtr;
}
{{{ maybeExport('stringToUTF32') }}}

// Returns the number of bytes the given Javascript string takes if encoded as a UTF16 byte array, EXCLUDING the null terminator byte.

function lengthBytesUTF32(str) {
  var len = 0;
  for (var i = 0; i < str.length; ++i) {
    // Gotcha: charCodeAt returns a 16-bit word that is a UTF-16 encoded code unit, not a Unicode code point of the character! We must decode the string to UTF-32 to the heap.
    // See http://unicode.org/faq/utf_bom.html#utf16-3
    var codeUnit = str.charCodeAt(i);
    if (codeUnit >= 0xD800 && codeUnit <= 0xDFFF) ++i; // possibly a lead surrogate, so skip over the tail surrogate.
    len += 4;
  }

  return len;
}
{{{ maybeExport('lengthBytesUTF32') }}}

function demangle(func) {
  var __cxa_demangle_func = Module['___cxa_demangle'] || Module['__cxa_demangle'];
  if (__cxa_demangle_func) {
    try {
      var s =
#if WASM_BACKEND
        func;
#else
        func.substr(1);
#endif
      var len = lengthBytesUTF8(s)+1;
      var buf = _malloc(len);
      stringToUTF8(s, buf, len);
      var status = _malloc(4);
      var ret = __cxa_demangle_func(buf, 0, 0, status);
      if (getValue(status, 'i32') === 0 && ret) {
        return Pointer_stringify(ret);
      }
      // otherwise, libcxxabi failed
    } catch(e) {
      // ignore problems here
    } finally {
      if (buf) _free(buf);
      if (status) _free(status);
      if (ret) _free(ret);
    }
    // failure when using libcxxabi, don't demangle
    return func;
  }
  Runtime.warnOnce('warning: build with  -s DEMANGLE_SUPPORT=1  to link in libcxxabi demangling');
  return func;
}

function demangleAll(text) {
  var regex =
#if WASM_BACKEND
    /_Z[\w\d_]+/g;
#else
    /__Z[\w\d_]+/g;
#endif
  return text.replace(regex,
    function(x) {
      var y = demangle(x);
      return x === y ? x : (x + ' [' + y + ']');
    });
}

function jsStackTrace() {
  var err = new Error();
  if (!err.stack) {
    // IE10+ special cases: It does have callstack info, but it is only populated if an Error object is thrown,
    // so try that as a special-case.
    try {
      throw new Error(0);
    } catch(e) {
      err = e;
    }
    if (!err.stack) {
      return '(no stack trace available)';
    }
  }
  return err.stack.toString();
}

function stackTrace() {
  var js = jsStackTrace();
  if (Module['extraStackTrace']) js += '\n' + Module['extraStackTrace']();
  return demangleAll(js);
}
{{{ maybeExport('stackTrace') }}}

// Memory management

var PAGE_SIZE = 16384;
var WASM_PAGE_SIZE = 65536;
var ASMJS_PAGE_SIZE = 16777216;
var MIN_TOTAL_MEMORY = 16777216;

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

var HEAP,
/** @type {ArrayBuffer} */
  buffer,
/** @type {Int8Array} */
  HEAP8,
/** @type {Uint8Array} */
  HEAPU8,
/** @type {Int16Array} */
  HEAP16,
/** @type {Uint16Array} */
  HEAPU16,
/** @type {Int32Array} */
  HEAP32,
/** @type {Uint32Array} */
  HEAPU32,
/** @type {Float32Array} */
  HEAPF32,
/** @type {Float64Array} */
  HEAPF64;

function updateGlobalBuffer(buf) {
  Module['buffer'] = buffer = buf;
}

function updateGlobalBufferViews() {
  Module['HEAP8'] = HEAP8 = new Int8Array(buffer);
  Module['HEAP16'] = HEAP16 = new Int16Array(buffer);
  Module['HEAP32'] = HEAP32 = new Int32Array(buffer);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buffer);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buffer);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buffer);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buffer);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buffer);
}

var STATIC_BASE, STATICTOP, staticSealed; // static area
var STACK_BASE, STACKTOP, STACK_MAX; // stack area
var DYNAMIC_BASE, DYNAMICTOP_PTR; // dynamic area handled by sbrk

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) { // Pthreads have already initialized these variables in src/pthread-main.js, where they were passed to the thread worker at startup time
#endif
  STATIC_BASE = STATICTOP = STACK_BASE = STACKTOP = STACK_MAX = DYNAMIC_BASE = DYNAMICTOP_PTR = 0;
  staticSealed = false;
#if USE_PTHREADS
}
#endif

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
  staticSealed = true; // The static memory area has been initialized already in the main thread, pthreads skip this.
#if SEPARATE_ASM != 0
  importScripts('{{{ SEPARATE_ASM }}}'); // load the separated-out asm.js
#endif
}
#endif

#if STACK_OVERFLOW_CHECK
// Initializes the stack cookie. Called at the startup of main and at the startup of each thread in pthreads mode.
function writeStackCookie() {
  assert((STACK_MAX & 3) == 0);
  HEAPU32[(STACK_MAX >> 2)-1] = 0x02135467;
  HEAPU32[(STACK_MAX >> 2)-2] = 0x89BACDFE;
}

function checkStackCookie() {
  if (HEAPU32[(STACK_MAX >> 2)-1] != 0x02135467 || HEAPU32[(STACK_MAX >> 2)-2] != 0x89BACDFE) {
    abort('Stack overflow! Stack cookie has been overwritten, expected hex dwords 0x89BACDFE and 0x02135467, but received 0x' + HEAPU32[(STACK_MAX >> 2)-2].toString(16) + ' ' + HEAPU32[(STACK_MAX >> 2)-1].toString(16));
  }
#if !SAFE_SPLIT_MEMORY
  // Also test the global address 0 for integrity. This check is not compatible with SAFE_SPLIT_MEMORY though, since that mode already tests all address 0 accesses on its own.
  if (HEAP32[0] !== 0x63736d65 /* 'emsc' */) throw 'Runtime error: The application has corrupted its heap memory area (address zero)!';
#endif
}

function abortStackOverflow(allocSize) {
  abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - Module['asm'].stackSave() + allocSize) + ' bytes available!');
}
#endif

#if ABORTING_MALLOC
function abortOnCannotGrowMemory() {
#if BINARYEN
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
#else
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime but prevents some optimizations, (3) set Module.TOTAL_MEMORY to a higher value before the program runs, or (4) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
#endif
}
#endif

#if ALLOW_MEMORY_GROWTH
if (!Module['reallocBuffer']) Module['reallocBuffer'] = function(size) {
  var ret;
  try {
    if (ArrayBuffer.transfer) {
      ret = ArrayBuffer.transfer(buffer, size);
    } else {
      var oldHEAP8 = HEAP8;
      ret = new ArrayBuffer(size);
      var temp = new Int8Array(ret);
      temp.set(oldHEAP8);
    }
  } catch(e) {
    return false;
  }
  var success = _emscripten_replace_memory(ret);
  if (!success) return false;
  return ret;
};
#endif

function enlargeMemory() {
#if USE_PTHREADS
  abort('Cannot enlarge memory arrays, since compiling with pthreads support enabled (-s USE_PTHREADS=1).');
#else
#if ALLOW_MEMORY_GROWTH == 0
#if ABORTING_MALLOC
  abortOnCannotGrowMemory();
#else
  return false; // malloc will report failure
#endif
#else
  // TOTAL_MEMORY is the current size of the actual array, and DYNAMICTOP is the new top.
#if ASSERTIONS
  assert(HEAP32[DYNAMICTOP_PTR>>2] > TOTAL_MEMORY); // This function should only ever be called after the ceiling of the dynamic heap has already been bumped to exceed the current total size of the asm.js heap.
#endif

#if EMSCRIPTEN_TRACING
  // Report old layout one last time
  _emscripten_trace_report_memory_layout();
#endif

  var PAGE_MULTIPLE = Module["usingWasm"] ? WASM_PAGE_SIZE : ASMJS_PAGE_SIZE; // In wasm, heap size must be a multiple of 64KB. In asm.js, they need to be multiples of 16MB.
  var LIMIT = 2147483648 - PAGE_MULTIPLE; // We can do one page short of 2GB as theoretical maximum.

  if (HEAP32[DYNAMICTOP_PTR>>2] > LIMIT) {
#if ASSERTIONS
    Module.printErr('Cannot enlarge memory, asked to go up to ' + HEAP32[DYNAMICTOP_PTR>>2] + ' bytes, but the limit is ' + LIMIT + ' bytes!');
#endif
    return false;
  }

  var OLD_TOTAL_MEMORY = TOTAL_MEMORY;
  TOTAL_MEMORY = Math.max(TOTAL_MEMORY, MIN_TOTAL_MEMORY); // So the loop below will not be infinite, and minimum asm.js memory size is 16MB.

  while (TOTAL_MEMORY < HEAP32[DYNAMICTOP_PTR>>2]) { // Keep incrementing the heap size as long as it's less than what is requested.
    if (TOTAL_MEMORY <= 536870912) {
      TOTAL_MEMORY = alignUp(2 * TOTAL_MEMORY, PAGE_MULTIPLE); // Simple heuristic: double until 1GB...
    } else {
      TOTAL_MEMORY = Math.min(alignUp((3 * TOTAL_MEMORY + 2147483648) / 4, PAGE_MULTIPLE), LIMIT); // ..., but after that, add smaller increments towards 2GB, which we cannot reach
    }
  }

#if ASSERTIONS
  var start = Date.now();
#endif

  var replacement = Module['reallocBuffer'](TOTAL_MEMORY);
  if (!replacement || replacement.byteLength != TOTAL_MEMORY) {
#if ASSERTIONS
    Module.printErr('Failed to grow the heap from ' + OLD_TOTAL_MEMORY + ' bytes to ' + TOTAL_MEMORY + ' bytes, not enough memory!');
    if (replacement) {
      Module.printErr('Expected to get back a buffer of size ' + TOTAL_MEMORY + ' bytes, but instead got back a buffer of size ' + replacement.byteLength);
    }
#endif
    // restore the state to before this call, we failed
    TOTAL_MEMORY = OLD_TOTAL_MEMORY;
    return false;
  }

  // everything worked

  updateGlobalBuffer(replacement);
  updateGlobalBufferViews();

#if ASSERTIONS
  Module.printErr('enlarged memory arrays from ' + OLD_TOTAL_MEMORY + ' to ' + TOTAL_MEMORY + ', took ' + (Date.now() - start) + ' ms (has ArrayBuffer.transfer? ' + (!!ArrayBuffer.transfer) + ')');
#endif

#if ASSERTIONS
  if (!Module["usingWasm"]) {
    Module.printErr('Warning: Enlarging memory arrays, this is not fast! ' + [OLD_TOTAL_MEMORY, TOTAL_MEMORY]);
  }
#endif

#if EMSCRIPTEN_TRACING
  _emscripten_trace_js_log_message("Emscripten", "Enlarging memory arrays from " + OLD_TOTAL_MEMORY + " to " + TOTAL_MEMORY);
  // And now report the new layout
  _emscripten_trace_report_memory_layout();
#endif

  return true;
#endif // ALLOW_MEMORY_GROWTH
#endif // USE_PTHREADS
}

#if ALLOW_MEMORY_GROWTH
var byteLength;
try {
  byteLength = Function.prototype.call.bind(Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, 'byteLength').get);
  byteLength(new ArrayBuffer(4)); // can fail on older ie
} catch(e) { // can fail on older node/v8
  byteLength = function(buffer) { return buffer.byteLength; };
}
#endif

var TOTAL_STACK = Module['TOTAL_STACK'] || {{{ TOTAL_STACK }}};
var TOTAL_MEMORY = Module['TOTAL_MEMORY'] || {{{ TOTAL_MEMORY }}};
if (TOTAL_MEMORY < TOTAL_STACK) Module.printErr('TOTAL_MEMORY should be larger than TOTAL_STACK, was ' + TOTAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// Initialize the runtime's memory
#if ASSERTIONS
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined,
       'JS engine does not provide full typed array support');
#endif

#if IN_TEST_HARNESS

// Test runs in browsers should always be free from uncaught exceptions. If an uncaught exception is thrown, we fail browser test execution in the REPORT_RESULT() macro to output an error value.
if (ENVIRONMENT_IS_WEB) {
  window.addEventListener('error', function(e) {
    if (e === 'SimulateInfiniteLoop' || e.message.indexOf('SimulateInfiniteLoop') != -1) return;
    console.error('Page threw an exception ' + e);
    Module['pageThrewException'] = true;
  });
}

#if USE_PTHREADS == 1
if (typeof SharedArrayBuffer === 'undefined' || typeof Atomics === 'undefined') {
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8888/report_result?skipped:%20SharedArrayBuffer%20is%20not%20supported!');
  xhr.send();
  setTimeout(function() { window.close() }, 2000);
}
#endif
#endif

#if USE_PTHREADS
if (typeof SharedArrayBuffer !== 'undefined') {
  if (!ENVIRONMENT_IS_PTHREAD) buffer = new SharedArrayBuffer(TOTAL_MEMORY);
  // Currently SharedArrayBuffer does not have a slice() operation, so polyfill it in.
  // Adapted from https://github.com/ttaubert/node-arraybuffer-slice, (c) 2014 Tim Taubert <tim@timtaubert.de>
  // arraybuffer-slice may be freely distributed under the MIT license.
  (function (undefined) {
    "use strict";
    function clamp(val, length) {
      val = (val|0) || 0;
      if (val < 0) return Math.max(val + length, 0);
      return Math.min(val, length);
    }
    if (typeof SharedArrayBuffer !== 'undefined' && !SharedArrayBuffer.prototype.slice) {
      SharedArrayBuffer.prototype.slice = function (from, to) {
        var length = this.byteLength;
        var begin = clamp(from, length);
        var end = length;
        if (to !== undefined) end = clamp(to, length);
        if (begin > end) return new ArrayBuffer(0);
        var num = end - begin;
        var target = new ArrayBuffer(num);
        var targetArray = new Uint8Array(target);
        var sourceArray = new Uint8Array(this, begin, num);
        targetArray.set(sourceArray);
        return target;
      };
    }
  })();
} else {
  if (!ENVIRONMENT_IS_PTHREAD) buffer = new ArrayBuffer(TOTAL_MEMORY);
}
updateGlobalBufferViews();

if (typeof Atomics === 'undefined') {
  // Polyfill singlethreaded atomics ops from http://lars-t-hansen.github.io/ecmascript_sharedmem/shmem.html#Atomics.add
  // No thread-safety needed since we don't have multithreading support.
  Atomics = {};
  Atomics['add'] = function(t, i, v) { var w = t[i]; t[i] += v; return w; }
  Atomics['and'] = function(t, i, v) { var w = t[i]; t[i] &= v; return w; }
  Atomics['compareExchange'] = function(t, i, e, r) { var w = t[i]; if (w == e) t[i] = r; return w; }
  Atomics['exchange'] = function(t, i, v) { var w = t[i]; t[i] = v; return w; }
  Atomics['wait'] = function(t, i, v, o) { if (t[i] != v) return 'not-equal'; else return 'timed-out'; }
  Atomics['wake'] = function(t, i, c) { return 0; }
  Atomics['wakeOrRequeue'] = function(t, i1, c, i2, v) { return 0; }
  Atomics['isLockFree'] = function(s) { return true; }
  Atomics['load'] = function(t, i) { return t[i]; }
  Atomics['or'] = function(t, i, v) { var w = t[i]; t[i] |= v; return w; }
  Atomics['store'] = function(t, i, v) { t[i] = v; return v; }
  Atomics['sub'] = function(t, i, v) { var w = t[i]; t[i] -= v; return w; }
  Atomics['xor'] = function(t, i, v) { var w = t[i]; t[i] ^= v; return w; }
}

#else // USE_PTHREADS

#if SPLIT_MEMORY == 0
// Use a provided buffer, if there is one, or else allocate a new one
if (Module['buffer']) {
  buffer = Module['buffer'];
#if ASSERTIONS
  assert(buffer.byteLength === TOTAL_MEMORY, 'provided buffer should be ' + TOTAL_MEMORY + ' bytes, but it is ' + buffer.byteLength);
#endif
} else {
  // Use a WebAssembly memory where available
#if BINARYEN
  if (typeof WebAssembly === 'object' && typeof WebAssembly.Memory === 'function') {
#if ASSERTIONS
    assert(TOTAL_MEMORY % WASM_PAGE_SIZE === 0);
#endif
#if ALLOW_MEMORY_GROWTH
#if BINARYEN_MEM_MAX
#if ASSERTIONS
    assert({{{ BINARYEN_MEM_MAX }}} % WASM_PAGE_SIZE == 0);
#endif
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE, 'maximum': {{{ BINARYEN_MEM_MAX }}} / WASM_PAGE_SIZE });
#else
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE });
#endif
#else
    Module['wasmMemory'] = new WebAssembly.Memory({ 'initial': TOTAL_MEMORY / WASM_PAGE_SIZE, 'maximum': TOTAL_MEMORY / WASM_PAGE_SIZE });
#endif
    buffer = Module['wasmMemory'].buffer;
  } else
#endif
  {
    buffer = new ArrayBuffer(TOTAL_MEMORY);
  }
#if ASSERTIONS
  assert(buffer.byteLength === TOTAL_MEMORY);
#endif
}
updateGlobalBufferViews();
#else // SPLIT_MEMORY
// make sure total memory is a multiple of the split memory size
var SPLIT_MEMORY = {{{ SPLIT_MEMORY }}};
var SPLIT_MEMORY_MASK = SPLIT_MEMORY - 1;
var SPLIT_MEMORY_BITS = -1;
var ALLOW_MEMORY_GROWTH = {{{ ALLOW_MEMORY_GROWTH }}};
var ABORTING_MALLOC = {{{ ABORTING_MALLOC }}};

Module['SPLIT_MEMORY'] = SPLIT_MEMORY;

totalMemory = TOTAL_MEMORY;
if (totalMemory % SPLIT_MEMORY) {
  totalMemory += SPLIT_MEMORY - (totalMemory % SPLIT_MEMORY);
}
if (totalMemory === SPLIT_MEMORY) totalMemory *= 2;
if (totalMemory !== TOTAL_MEMORY) {
  TOTAL_MEMORY = totalMemory;
#if ASSERTIONS == 2
  Module.printErr('increasing TOTAL_MEMORY to ' + TOTAL_MEMORY + ' to be a multiple>1 of the split memory size ' + SPLIT_MEMORY + ')');
#endif
}

var buffers = [], HEAP8s = [], HEAP16s = [], HEAP32s = [], HEAPU8s = [], HEAPU16s = [], HEAPU32s = [], HEAPF32s = [], HEAPF64s = [];

// Allocates a split chunk, a range of memory of size SPLIT_MEMORY. Generally data is not provided, and a new
// buffer is allocated, this is what happens when malloc works. However, you can provide your own buffer,
// which then lets you access it at address [ i*SPLIT_MEMORY, (i+1)*SPLIT_MEMORY ).
// The function returns true if it succeeds. It can also throw an exception if no data is provided and
// the browser fails to allocate the buffer.
function allocateSplitChunk(i, data) {
  if (buffers[i]) return false; // already taken
  var curr = data ? data : new ArrayBuffer(SPLIT_MEMORY);
#if ASSERTIONS
  assert(curr instanceof ArrayBuffer);
#endif
  buffers[i] = curr;
  HEAP8s[i] = new Int8Array(curr);
  HEAP16s[i] = new Int16Array(curr);
  HEAP32s[i] = new Int32Array(curr);
  HEAPU8s[i] = new Uint8Array(curr);
  HEAPU16s[i] = new Uint16Array(curr);
  HEAPU32s[i] = new Uint32Array(curr);
  HEAPF32s[i] = new Float32Array(curr);
  HEAPF64s[i] = new Float64Array(curr);
  return true;
}
function freeSplitChunk(i) {
#if ASSERTIONS
  assert(buffers[i] && HEAP8s[i]);
  assert(i > 0); // cannot free the first chunk
#endif
  buffers[i] = HEAP8s[i] = HEAP16s[i] = HEAP32s[i] = HEAPU8s[i] = HEAPU16s[i] = HEAPU32s[i] = HEAPF32s[i] = HEAPF64s[i] = null;
}

(function() {
  for (var i = 0; i < TOTAL_MEMORY / SPLIT_MEMORY; i++) {
    buffers[i] = HEAP8s[i] = HEAP16s[i] = HEAP32s[i] = HEAPU8s[i] = HEAPU16s[i] = HEAPU32s[i] = HEAPF32s[i] = HEAPF64s[i] = null;
  }

  var temp = SPLIT_MEMORY;
  while (temp) {
    temp >>= 1;
    SPLIT_MEMORY_BITS++;
  }

  allocateSplitChunk(0); // first chunk is for core runtime, static, stack, etc., always must be initialized

  // support HEAP8.subarray etc.
  var SHIFT_TABLE = [0, 0, 1, 0, 2, 0, 0, 0, 3];
  function fake(real) {
    var bytes = real[0].BYTES_PER_ELEMENT;
    var shifts = SHIFT_TABLE[bytes];
#if ASSERTIONS
    assert(shifts > 0 || bytes == 1);
#endif
    var that = {
      BYTES_PER_ELEMENT: bytes,
      set: function(array, offset) {
        if (offset === undefined) offset = 0;
        // potentially split over multiple chunks
        while (array.length > 0) {
          var chunk = offset >> SPLIT_MEMORY_BITS;
          var relative = offset & SPLIT_MEMORY_MASK;
          if (relative + (array.length << shifts) < SPLIT_MEMORY) {
            real[chunk].set(array, relative); // all fits in this chunk
            break;
          } else {
            var currSize = SPLIT_MEMORY - relative;
#if ASSERTIONS
            assert(currSize % that.BYTES_PER_ELEMENT === 0);
#endif
            var lastIndex = currSize >> shifts;
            real[chunk].set(array.subarray(0, lastIndex), relative);
            // increments
            array = array.subarray(lastIndex);
            offset += currSize;
          }
        }
      },
      subarray: function(from, to) {
        from = from << shifts;
        var start = from >> SPLIT_MEMORY_BITS;
        if (to === undefined) {
          to = (start + 1) << SPLIT_MEMORY_BITS;
        } else {
          to = to << shifts;
        }
        to = Math.max(from, to); // if to is smaller, we'll get nothing anyway, same as to == from
        if (from < to) {
          var end = (to - 1) >> SPLIT_MEMORY_BITS; // -1, since we do not actually read the last address
#if ASSERTIONS
          assert(start === end, 'subarray cannot span split chunks');
#endif
        }
        if (to > from && (to & SPLIT_MEMORY_MASK) == 0) {
          // avoid the mask on the next line giving 0 for the end
          return real[start].subarray((from & SPLIT_MEMORY_MASK) >> shifts); // just return to the end of the chunk
        }
        return real[start].subarray((from & SPLIT_MEMORY_MASK) >> shifts, (to & SPLIT_MEMORY_MASK) >> shifts);
      },
      buffer: {
        slice: function(from, to) {
#if ASSERTIONS
          assert(to, 'TODO: this is an actual copy, so we could support a slice across multiple chunks');
#endif
          return new Uint8Array(HEAPU8.subarray(from, to)).buffer;
        },
      },
    };
    return that;
  }
  HEAP8 = fake(HEAP8s);
  HEAP16 = fake(HEAP16s);
  HEAP32 = fake(HEAP32s);
  HEAPU8 = fake(HEAPU8s);
  HEAPU16 = fake(HEAPU16s);
  HEAPU32 = fake(HEAPU32s);
  HEAPF32 = fake(HEAPF32s);
  HEAPF64 = fake(HEAPF64s);
})();

#if SAFE_SPLIT_MEMORY
function checkPtr(ptr, shifts) {
  if (ptr <= 0) abort('segmentation fault storing to address ' + ptr);
  if (ptr !== ((ptr >> shifts) << shifts)) abort('alignment error storing to address ' + ptr + ', which was expected to be aligned to a shift of ' + shifts);
  if ((ptr >> SPLIT_MEMORY_BITS) !== (ptr + Math.pow(2, shifts) - 1 >> SPLIT_MEMORY_BITS)) abort('segmentation fault, write spans split chunks ' + [ptr, shifts]);
}
#endif

function get8(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  return HEAP8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] | 0;
}
function get16(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  return HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function get32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] | 0;
}
function getU8(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  return HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] | 0;
}
function getU16(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  return HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] | 0;
}
function getU32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] >>> 0;
}
function getF32(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  return +HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2];
}
function getF64(ptr) {
  ptr = ptr | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 3);
#endif
  return +HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3];
}
function set8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  HEAP8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] = value;
}
function set16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  HEAP16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function set32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAP32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setU8(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 0);
#endif
  HEAPU8s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 0] = value;
}
function setU16(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 1);
#endif
  HEAPU16s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 1] = value;
}
function setU32(ptr, value) {
  ptr = ptr | 0;
  value = value | 0;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAPU32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF32(ptr, value) {
  ptr = ptr | 0;
  value = +value;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 2);
#endif
  HEAPF32s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 2] = value;
}
function setF64(ptr, value) {
  ptr = ptr | 0;
  value = +value;
#if SAFE_SPLIT_MEMORY
  checkPtr(ptr, 3);
#endif
  HEAPF64s[ptr >> SPLIT_MEMORY_BITS][(ptr & SPLIT_MEMORY_MASK) >> 3] = value;
}
#endif // SPLIT_MEMORY

#endif // USE_PTHREADS

function getTotalMemory() {
  return TOTAL_MEMORY;
}

// Endianness check (note: assumes compiler arch was little-endian)
#if SAFE_SPLIT_MEMORY == 0
#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
#endif
  HEAP32[0] = 0x63736d65; /* 'emsc' */
#if USE_PTHREADS
} else {
  if (HEAP32[0] !== 0x63736d65) throw 'Runtime error: The application has corrupted its heap memory area (address zero)!';
}
#endif
HEAP16[1] = 0x6373;
if (HEAPU8[2] !== 0x73 || HEAPU8[3] !== 0x63) throw 'Runtime error: expected the system to be little-endian!';
#endif

Module['HEAP'] = HEAP;
Module['buffer'] = buffer;
Module['HEAP8'] = HEAP8;
Module['HEAP16'] = HEAP16;
Module['HEAP32'] = HEAP32;
Module['HEAPU8'] = HEAPU8;
Module['HEAPU16'] = HEAPU16;
Module['HEAPU32'] = HEAPU32;
Module['HEAPF32'] = HEAPF32;
Module['HEAPF64'] = HEAPF64;

function callRuntimeCallbacks(callbacks) {
  while(callbacks.length > 0) {
    var callback = callbacks.shift();
    if (typeof callback == 'function') {
      callback();
      continue;
    }
    var func = callback.func;
    if (typeof func === 'number') {
      if (callback.arg === undefined) {
        Module['dynCall_v'](func);
      } else {
        Module['dynCall_vi'](func, callback.arg);
      }
    } else {
      func(callback.arg === undefined ? null : callback.arg);
    }
  }
}

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the runtime has exited

var runtimeInitialized = false;
var runtimeExited = false;

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) runtimeInitialized = true; // The runtime is hosted in the main thread, and bits shared to pthreads via SharedArrayBuffer. No need to init again in pthread.
#endif

function preRun() {
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  // compatibility - merge in anything from Module['preRun'] at this time
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPRERUN__);
}

function ensureInitRuntime() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  if (runtimeInitialized) return;
  runtimeInitialized = true;
#if USE_PTHREADS
  // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
  __register_pthread_ptr(PThread.mainThreadBlock, /*isMainBrowserThread=*/!ENVIRONMENT_IS_WORKER, /*isMainRuntimeThread=*/1);
#endif
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  callRuntimeCallbacks(__ATEXIT__);
  runtimeExited = true;
}

function postRun() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  // compatibility - merge in anything from Module['postRun'] at this time
  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}
{{{ maybeExport('addOnPreRun') }}}

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}
{{{ maybeExport('addOnInit') }}}

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}
{{{ maybeExport('addOnPreMain') }}}

function addOnExit(cb) {
  __ATEXIT__.unshift(cb);
}
{{{ maybeExport('addOnExit') }}}

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}
{{{ maybeExport('addOnPostRun') }}}

// Tools

/** @type {function(string, boolean=, number=)} */
function intArrayFromString(stringy, dontAddNull, length) {
  var len = length > 0 ? length : lengthBytesUTF8(stringy)+1;
  var u8array = new Array(len);
  var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
  if (dontAddNull) u8array.length = numBytesWritten;
  return u8array;
}
{{{ maybeExport('intArrayFromString') }}}

function intArrayToString(array) {
  var ret = [];
  for (var i = 0; i < array.length; i++) {
    var chr = array[i];
    if (chr > 0xFF) {
#if ASSERTIONS
      assert(false, 'Character code ' + chr + ' (' + String.fromCharCode(chr) + ')  at offset ' + i + ' not in 0x00-0xFF.');
#endif
      chr &= 0xFF;
    }
    ret.push(String.fromCharCode(chr));
  }
  return ret.join('');
}
{{{ maybeExport('intArrayToString') }}}

// Deprecated: This function should not be called because it is unsafe and does not provide
// a maximum length limit of how many bytes it is allowed to write. Prefer calling the
// function stringToUTF8Array() instead, which takes in a maximum length that can be used
// to be secure from out of bounds writes.
/** @deprecated */
function writeStringToMemory(string, buffer, dontAddNull) {
  Runtime.warnOnce('writeStringToMemory is deprecated and should not be called! Use stringToUTF8() instead!');

  var /** @type {number} */ lastChar, /** @type {number} */ end;
  if (dontAddNull) {
    // stringToUTF8Array always appends null. If we don't want to do that, remember the
    // character that existed at the location where the null will be placed, and restore
    // that after the write (below).
    end = buffer + lengthBytesUTF8(string);
    lastChar = HEAP8[end];
  }
  stringToUTF8(string, buffer, Infinity);
  if (dontAddNull) HEAP8[end] = lastChar; // Restore the value under the null character.
}
{{{ maybeExport('writeStringToMemory') }}}

function writeArrayToMemory(array, buffer) {
#if ASSERTIONS
  assert(array.length >= 0, 'writeArrayToMemory array must have a length (should be an array or typed array)')
#endif
  HEAP8.set(array, buffer);
}
{{{ maybeExport('writeArrayToMemory') }}}

function writeAsciiToMemory(str, buffer, dontAddNull) {
  for (var i = 0; i < str.length; ++i) {
#if ASSERTIONS
    assert(str.charCodeAt(i) === str.charCodeAt(i)&0xff);
#endif
    {{{ makeSetValue('buffer++', 0, 'str.charCodeAt(i)', 'i8') }}};
  }
  // Null-terminate the pointer to the HEAP.
  if (!dontAddNull) {{{ makeSetValue('buffer', 0, 0, 'i8') }}};
}
{{{ maybeExport('writeAsciiToMemory') }}}

{{{ unSign }}}
{{{ reSign }}}

// check for imul support, and also for correctness ( https://bugs.webkit.org/show_bug.cgi?id=126345 )
if (!Math['imul'] || Math['imul'](0xffffffff, 5) !== -5) Math['imul'] = function imul(a, b) {
  var ah  = a >>> 16;
  var al = a & 0xffff;
  var bh  = b >>> 16;
  var bl = b & 0xffff;
  return (al*bl + ((ah*bl + al*bh) << 16))|0;
};
Math.imul = Math['imul'];

#if PRECISE_F32
#if PRECISE_F32 == 1
if (!Math['fround']) {
  var froundBuffer = new Float32Array(1);
  Math['fround'] = function(x) { froundBuffer[0] = x; return froundBuffer[0] };
}
#else // 2
if (!Math['fround']) Math['fround'] = function(x) { return x };
#endif
Math.fround = Math['fround'];
#else
#if SIMD
if (!Math['fround']) Math['fround'] = function(x) { return x };
#endif
#endif

if (!Math['clz32']) Math['clz32'] = function(x) {
  x = x >>> 0;
  for (var i = 0; i < 32; i++) {
    if (x & (1 << (31 - i))) return i;
  }
  return 32;
};
Math.clz32 = Math['clz32']

if (!Math['trunc']) Math['trunc'] = function(x) {
  return x < 0 ? Math.ceil(x) : Math.floor(x);
};
Math.trunc = Math['trunc'];

var Math_abs = Math.abs;
var Math_cos = Math.cos;
var Math_sin = Math.sin;
var Math_tan = Math.tan;
var Math_acos = Math.acos;
var Math_asin = Math.asin;
var Math_atan = Math.atan;
var Math_atan2 = Math.atan2;
var Math_exp = Math.exp;
var Math_log = Math.log;
var Math_sqrt = Math.sqrt;
var Math_ceil = Math.ceil;
var Math_floor = Math.floor;
var Math_pow = Math.pow;
var Math_imul = Math.imul;
var Math_fround = Math.fround;
var Math_round = Math.round;
var Math_min = Math.min;
var Math_clz32 = Math.clz32;
var Math_trunc = Math.trunc;

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// PRE_RUN_ADDITIONS (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
#if ASSERTIONS
var runDependencyTracking = {};
#endif

function getUniqueRunDependency(id) {
#if ASSERTIONS
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
#endif
  return id;
}

function addRunDependency(id) {
#if USE_PTHREADS
  // We should never get here in pthreads (could no-op this out if called in pthreads, but that might indicate a bug in caller side,
  // so good to be very explicit)
  assert(!ENVIRONMENT_IS_PTHREAD);
#endif
  runDependencies++;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#if ASSERTIONS
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            Module.printErr('still waiting on run dependencies:');
          }
          Module.printErr('dependency: ' + dep);
        }
        if (shown) {
          Module.printErr('(end of list)');
        }
      }, 10000);
    }
  } else {
    Module.printErr('warning: run dependency added without ID');
  }
#endif
}
{{{ maybeExport('addRunDependency') }}}

function removeRunDependency(id) {
  runDependencies--;
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#if ASSERTIONS
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    Module.printErr('warning: run dependency removed without ID');
  }
#endif
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}
{{{ maybeExport('removeRunDependency') }}}

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data

#if PGO
var PGOMonitor = {
  called: {},
  dump: function() {
    var dead = [];
    for (var i = 0; i < this.allGenerated.length; i++) {
      var func = this.allGenerated[i];
      if (!this.called[func]) dead.push(func);
    }
    Module.print('-s DEAD_FUNCTIONS=\'' + JSON.stringify(dead) + '\'\n');
  }
};
Module['PGOMonitor'] = PGOMonitor;
__ATEXIT__.push(function() { PGOMonitor.dump() });
addOnPreRun(function() { addRunDependency('pgo') });
#endif

#if RELOCATABLE
{{{
(function() {
  // add in RUNTIME_LINKED_LIBS, if provided
  if (RUNTIME_LINKED_LIBS.length > 0) {
    return "if (!Module['dynamicLibraries']) Module['dynamicLibraries'] = [];\n" +
           "Module['dynamicLibraries'] = " + JSON.stringify(RUNTIME_LINKED_LIBS) + ".concat(Module['dynamicLibraries']);\n";
  }
  return '';
})()
}}}

addOnPreRun(function() {
  function loadDynamicLibraries(libs) {
    if (libs) {
      libs.forEach(function(lib) {
        Runtime.loadDynamicLibrary(lib);
      });
    }
    if (Module['asm']['runPostSets']) {
      Module['asm']['runPostSets']();
    }
  }
  // if we can load dynamic libraries synchronously, do so, otherwise, preload
#if BINARYEN
  if (Module['dynamicLibraries'] && Module['dynamicLibraries'].length > 0 && !Module['readBinary']) {
    // we can't read binary data synchronously, so preload
    addRunDependency('preload_dynamicLibraries');
    var binaries = [];
    Module['dynamicLibraries'].forEach(function(lib) {
      fetch(lib, { credentials: 'same-origin' }).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + lib + "'";
        }
        return response['arrayBuffer']();
      }).then(function(buffer) {
        var binary = new Uint8Array(buffer);
        binaries.push(binary);
        if (binaries.length === Module['dynamicLibraries'].length) {
          // we got them all, wonderful
          loadDynamicLibraries(binaries);
          removeRunDependency('preload_dynamicLibraries');
        }
      });
    });
    return;
  }
#endif
  loadDynamicLibraries(Module['dynamicLibraries']);
});

#if ASSERTIONS
function lookupSymbol(ptr) { // for a pointer, print out all symbols that resolve to it
  var ret = [];
  for (var i in Module) {
    if (Module[i] === ptr) ret.push(i);
  }
  print(ptr + ' is ' + ret);
}
#endif
#endif

var memoryInitializer = null;

#if USE_PTHREADS
#if PTHREAD_HINT_NUM_CORES < 0
if (!ENVIRONMENT_IS_PTHREAD) addOnPreRun(function() {
  addRunDependency('pthreads_querycores');

  var bg = document.createElement('div');
  bg.style = "position: absolute; top: 0%; left: 0%; width: 100%; height: 100%; background-color: black; z-index:1001; -moz-opacity: 0.8; opacity:.80; filter: alpha(opacity=80);";
  var div = document.createElement('div');
  var default_num_cores = navigator.hardwareConcurrency || 4;
  var hwConcurrency = navigator.hardwareConcurrency ? ("says " + navigator.hardwareConcurrency) : "is not available";
  var html = '<div style="width: 100%; text-align:center;"> Thread setup</div> <br /> Number of logical cores: <input type="number" style="width: 50px;" value="'
    + default_num_cores + '" min="1" max="32" id="thread_setup_num_logical_cores"></input> <br /><span style="font-size: 75%;">(<span style="font-family: monospace;">navigator.hardwareConcurrency</span> '
    + hwConcurrency + ')</span> <br />';
#if PTHREAD_POOL_SIZE < 0
  html += 'PThread pool size: <input type="number" style="width: 50px;" value="'
    + default_num_cores + '" min="1" max="32" id="thread_setup_pthread_pool_size"></input> <br />';
#endif
  html += ' <br /> <input type="button" id="thread_setup_button_go" value="Go"></input>';
  div.innerHTML = html;
  div.style = 'position: absolute; top: 35%; left: 35%; width: 30%; height: 150px; padding: 16px; border: 16px solid gray; background-color: white; z-index:1002; overflow: auto;';
  document.body.appendChild(bg);
  document.body.appendChild(div);
  var goButton = document.getElementById('thread_setup_button_go');
  goButton.onclick = function() {
    var num_logical_cores = parseInt(document.getElementById('thread_setup_num_logical_cores').value);
    _emscripten_force_num_logical_cores(num_logical_cores);
#if PTHREAD_POOL_SIZE < 0
    var pthread_pool_size = parseInt(document.getElementById('thread_setup_pthread_pool_size').value);
    PThread.allocateUnusedWorkers(pthread_pool_size, function() { removeRunDependency('pthreads_querycores'); });
#else
    removeRunDependency('pthreads_querycores');
#endif
    document.body.removeChild(bg);
    document.body.removeChild(div);
  }
});
#endif
#endif

#if PTHREAD_POOL_SIZE > 0
// To work around https://bugzilla.mozilla.org/show_bug.cgi?id=1049079, warm up a worker pool before starting up the application.
if (!ENVIRONMENT_IS_PTHREAD) addOnPreRun(function() { if (typeof SharedArrayBuffer !== 'undefined') { addRunDependency('pthreads'); PThread.allocateUnusedWorkers({{{PTHREAD_POOL_SIZE}}}, function() { removeRunDependency('pthreads'); }); }});
#endif

#if ASSERTIONS
#if NO_FILESYSTEM
var /* show errors on likely calls to FS when it was not included */ FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1');
  },
  init: function() { FS.error() },
  createDataFile: function() { FS.error() },
  createPreloadedFile: function() { FS.error() },
  createLazyFile: function() { FS.error() },
  open: function() { FS.error() },
  mkdev: function() { FS.error() },
  registerDevice: function() { FS.error() },
  analyzePath: function() { FS.error() },
  loadFilesFromDB: function() { FS.error() },

  ErrnoError: function ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
#endif
#endif

#if CYBERDWARF
var cyberDWARFFile = '{{{ BUNDLED_CD_DEBUG_FILE }}}';
#endif

#if BINARYEN
function integrateWasmJS(Module) {
  // wasm.js has several methods for creating the compiled code module here:
  //  * 'native-wasm' : use native WebAssembly support in the browser
  //  * 'interpret-s-expr': load s-expression code from a .wast and interpret
  //  * 'interpret-binary': load binary wasm and interpret
  //  * 'interpret-asm2wasm': load asm.js code, translate to wasm, and interpret
  //  * 'asmjs': no wasm, just load the asm.js code and use that (good for testing)
  // The method can be set at compile time (BINARYEN_METHOD), or runtime by setting Module['wasmJSMethod'].
  // The method can be a comma-separated list, in which case, we will try the
  // options one by one. Some of them can fail gracefully, and then we can try
  // the next.

  // inputs

  var method = Module['wasmJSMethod'] || '{{{ BINARYEN_METHOD }}}';
  Module['wasmJSMethod'] = method;

  var wasmTextFile = Module['wasmTextFile'] || '{{{ WASM_TEXT_FILE }}}';
  var wasmBinaryFile = Module['wasmBinaryFile'] || '{{{ WASM_BINARY_FILE }}}';
  var asmjsCodeFile = Module['asmjsCodeFile'] || '{{{ ASMJS_CODE_FILE }}}';

  if (typeof Module['locateFile'] === 'function') {
    wasmTextFile = Module['locateFile'](wasmTextFile);
    wasmBinaryFile = Module['locateFile'](wasmBinaryFile);
    asmjsCodeFile = Module['locateFile'](asmjsCodeFile);
  }

  // utilities

  var wasmPageSize = 64*1024;

  var asm2wasmImports = { // special asm2wasm imports
    "f64-rem": function(x, y) {
      return x % y;
    },
    "f64-to-int": function(x) {
      return x | 0;
    },
    "i32s-div": function(x, y) {
      return ((x | 0) / (y | 0)) | 0;
    },
    "i32u-div": function(x, y) {
      return ((x >>> 0) / (y >>> 0)) >>> 0;
    },
    "i32s-rem": function(x, y) {
      return ((x | 0) % (y | 0)) | 0;
    },
    "i32u-rem": function(x, y) {
      return ((x >>> 0) % (y >>> 0)) >>> 0;
    },
    "debugger": function() {
      debugger;
    },
  };

  var info = {
    'global': null,
    'env': null,
    'asm2wasm': asm2wasmImports,
    'parent': Module // Module inside wasm-js.cpp refers to wasm-js.cpp; this allows access to the outside program.
  };

  var exports = null;

  function lookupImport(mod, base) {
    var lookup = info;
    if (mod.indexOf('.') < 0) {
      lookup = (lookup || {})[mod];
    } else {
      var parts = mod.split('.');
      lookup = (lookup || {})[parts[0]];
      lookup = (lookup || {})[parts[1]];
    }
    if (base) {
      lookup = (lookup || {})[base];
    }
    if (lookup === undefined) {
      abort('bad lookupImport to (' + mod + ').' + base);
    }
    return lookup;
  }

  function mergeMemory(newBuffer) {
    // The wasm instance creates its memory. But static init code might have written to
    // buffer already, including the mem init file, and we must copy it over in a proper merge.
    // TODO: avoid this copy, by avoiding such static init writes
    // TODO: in shorter term, just copy up to the last static init write
    var oldBuffer = Module['buffer'];
    if (newBuffer.byteLength < oldBuffer.byteLength) {
      Module['printErr']('the new buffer in mergeMemory is smaller than the previous one. in native wasm, we should grow memory here');
    }
    var oldView = new Int8Array(oldBuffer);
    var newView = new Int8Array(newBuffer);

    // If we have a mem init file, do not trample it
    if (!memoryInitializer) {
      oldView.set(newView.subarray(Module['STATIC_BASE'], Module['STATIC_BASE'] + Module['STATIC_BUMP']), Module['STATIC_BASE']);
    }

    newView.set(oldView);
    updateGlobalBuffer(newBuffer);
    updateGlobalBufferViews();
  }

  var WasmTypes = {
    none: 0,
    i32: 1,
    i64: 2,
    f32: 3,
    f64: 4
  };

  function fixImports(imports) {
    if (!{{{ WASM_BACKEND }}}) return imports;
    var ret = {};
    for (var i in imports) {
      var fixed = i;
      if (fixed[0] == '_') fixed = fixed.substr(1);
      ret[fixed] = imports[i];
    }
    return ret;
  }

  function getBinary() {
    try {
      var binary;
      if (Module['wasmBinary']) {
        binary = Module['wasmBinary'];
        binary = new Uint8Array(binary);
      } else if (Module['readBinary']) {
        binary = Module['readBinary'](wasmBinaryFile);
      } else {
        throw "on the web, we need the wasm binary to be preloaded and set on Module['wasmBinary']. emcc.py will do that for you when generating HTML (but not JS)";
      }
      return binary;
    }
    catch (err) {
      abort(err);
    }
  }

  function getBinaryPromise() {
    // if we don't have the binary yet, and have the Fetch api, use that
    if (!Module['wasmBinary'] && typeof fetch === 'function') {
      return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
        }
        return response['arrayBuffer']();
      });
    }
    // Otherwise, getBinary should be able to get it synchronously
    return new Promise(function(resolve, reject) {
      resolve(getBinary());
    });
  }

  // do-method functions

  function doJustAsm(global, env, providedBuffer) {
    // if no Module.asm, or it's the method handler helper (see below), then apply
    // the asmjs
    if (typeof Module['asm'] !== 'function' || Module['asm'] === methodHandler) {
      if (!Module['asmPreload']) {
        // you can load the .asm.js file before this, to avoid this sync xhr and eval
        {{{ makeEval("eval(Module['read'](asmjsCodeFile));") }}} // set Module.asm
      } else {
        Module['asm'] = Module['asmPreload'];
      }
    }
    if (typeof Module['asm'] !== 'function') {
      Module['printErr']('asm evalling did not set the module properly');
      return false;
    }
    return Module['asm'](global, env, providedBuffer);
  }

  function doNativeWasm(global, env, providedBuffer) {
    if (typeof WebAssembly !== 'object') {
      Module['printErr']('no native wasm support detected');
      return false;
    }
    // prepare memory import
    if (!(Module['wasmMemory'] instanceof WebAssembly.Memory)) {
      Module['printErr']('no native wasm Memory in use');
      return false;
    }
    env['memory'] = Module['wasmMemory'];
    // Load the wasm module and create an instance of using native support in the JS engine.
    info['global'] = {
      'NaN': NaN,
      'Infinity': Infinity
    };
    info['global.Math'] = global.Math;
    info['env'] = env;
    // handle a generated wasm instance, receiving its exports and
    // performing other necessary setup
    function receiveInstance(instance) {
      exports = instance.exports;
      if (exports.memory) mergeMemory(exports.memory);
      Module['asm'] = exports;
      Module["usingWasm"] = true;
      removeRunDependency('wasm-instantiate');
    }

    addRunDependency('wasm-instantiate'); // we can't run yet

    // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
    // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
    // to any other async startup actions they are performing.
    if (Module['instantiateWasm']) {
      try {
        return Module['instantiateWasm'](info, receiveInstance);
      } catch(e) {
        Module['printErr']('Module.instantiateWasm callback failed with error: ' + e);
        return false;
      }
    }

#if BINARYEN_ASYNC_COMPILATION
#if RUNTIME_LOGGING
    Module['printErr']('asynchronously preparing wasm');
#endif
    getBinaryPromise().then(function(binary) {
      return WebAssembly.instantiate(binary, info)
    }).then(function(output) {
      // receiveInstance() will swap in the exports (to Module.asm) so they can be called
      receiveInstance(output['instance']);
    }).catch(function(reason) {
      Module['printErr']('failed to asynchronously prepare wasm: ' + reason);
      abort(reason);
    });
    return {}; // no exports yet; we'll fill them in later
#else
    var instance;
    try {
      instance = new WebAssembly.Instance(new WebAssembly.Module(getBinary()), info)
    } catch (e) {
      Module['printErr']('failed to compile wasm module: ' + e);
      if (e.toString().indexOf('imported Memory with incompatible size') >= 0) {
        Module['printErr']('Memory size incompatibility issues may be due to changing TOTAL_MEMORY at runtime to something too large. Use ALLOW_MEMORY_GROWTH to allow any size memory (and also make sure not to set TOTAL_MEMORY at runtime to something smaller than it was at compile time).');
      }
      return false;
    }
    receiveInstance(instance);
    return exports;
#endif
  }

  function doWasmPolyfill(global, env, providedBuffer, method) {
    if (typeof WasmJS !== 'function') {
      Module['printErr']('WasmJS not detected - polyfill not bundled?');
      return false;
    }

    // Use wasm.js to polyfill and execute code in a wasm interpreter.
    var wasmJS = WasmJS({});

    // XXX don't be confused. Module here is in the outside program. wasmJS is the inner wasm-js.cpp.
    wasmJS['outside'] = Module; // Inside wasm-js.cpp, Module['outside'] reaches the outside module.

    // Information for the instance of the module.
    wasmJS['info'] = info;

    wasmJS['lookupImport'] = lookupImport;

    assert(providedBuffer === Module['buffer']); // we should not even need to pass it as a 3rd arg for wasm, but that's the asm.js way.

    info.global = global;
    info.env = env;

    // polyfill interpreter expects an ArrayBuffer
    assert(providedBuffer === Module['buffer']);
    env['memory'] = providedBuffer;
    assert(env['memory'] instanceof ArrayBuffer);

    wasmJS['providedTotalMemory'] = Module['buffer'].byteLength;

    // Prepare to generate wasm, using either asm2wasm or s-exprs
    var code;
    if (method === 'interpret-binary') {
      code = getBinary();
    } else {
      code = Module['read'](method == 'interpret-asm2wasm' ? asmjsCodeFile : wasmTextFile);
    }
    var temp;
    if (method == 'interpret-asm2wasm') {
      temp = wasmJS['_malloc'](code.length + 1);
      wasmJS['writeAsciiToMemory'](code, temp);
      wasmJS['_load_asm2wasm'](temp);
    } else if (method === 'interpret-s-expr') {
      temp = wasmJS['_malloc'](code.length + 1);
      wasmJS['writeAsciiToMemory'](code, temp);
      wasmJS['_load_s_expr2wasm'](temp);
    } else if (method === 'interpret-binary') {
      temp = wasmJS['_malloc'](code.length);
      wasmJS['HEAPU8'].set(code, temp);
      wasmJS['_load_binary2wasm'](temp, code.length);
    } else {
      throw 'what? ' + method;
    }
    wasmJS['_free'](temp);

    wasmJS['_instantiate'](temp);

    if (Module['newBuffer']) {
      mergeMemory(Module['newBuffer']);
      Module['newBuffer'] = null;
    }

    exports = wasmJS['asmExports'];

    return exports;
  }

  // We may have a preloaded value in Module.asm, save it
  Module['asmPreload'] = Module['asm'];

  // Memory growth integration code

  var asmjsReallocBuffer = Module['reallocBuffer'];

  var wasmReallocBuffer = function(size) {
    var PAGE_MULTIPLE = Module["usingWasm"] ? WASM_PAGE_SIZE : ASMJS_PAGE_SIZE; // In wasm, heap size must be a multiple of 64KB. In asm.js, they need to be multiples of 16MB.
    size = alignUp(size, PAGE_MULTIPLE); // round up to wasm page size
    var old = Module['buffer'];
    var oldSize = old.byteLength;
    if (Module["usingWasm"]) {
      // native wasm support
      try {
        var result = Module['wasmMemory'].grow((size - oldSize) / wasmPageSize); // .grow() takes a delta compared to the previous size
        if (result !== (-1 | 0)) {
          // success in native wasm memory growth, get the buffer from the memory
          return Module['buffer'] = Module['wasmMemory'].buffer;
        } else {
          return null;
        }
      } catch(e) {
#if ASSERTIONS
        console.error('Module.reallocBuffer: Attempted to grow from ' + oldSize  + ' bytes to ' + size + ' bytes, but got error: ' + e);
#endif
        return null;
      }
    } else {
      // wasm interpreter support
      exports['__growWasmMemory']((size - oldSize) / wasmPageSize); // tiny wasm method that just does grow_memory
      // in interpreter, we replace Module.buffer if we allocate
      return Module['buffer'] !== old ? Module['buffer'] : null; // if it was reallocated, it changed
    }
  };

  Module['reallocBuffer'] = function(size) {
    if (finalMethod === 'asmjs') {
      return asmjsReallocBuffer(size);
    } else {
      return wasmReallocBuffer(size);
    }
  };

  // we may try more than one; this is the final one, that worked and we are using
  var finalMethod = '';

  // Provide an "asm.js function" for the application, called to "link" the asm.js module. We instantiate
  // the wasm module at that time, and it receives imports and provides exports and so forth, the app
  // doesn't need to care that it is wasm or olyfilled wasm or asm.js.

  Module['asm'] = function(global, env, providedBuffer) {
    global = fixImports(global);
    env = fixImports(env);

    // import table
    if (!env['table']) {
      var TABLE_SIZE = Module['wasmTableSize'];
      if (TABLE_SIZE === undefined) TABLE_SIZE = 1024; // works in binaryen interpreter at least
      var MAX_TABLE_SIZE = Module['wasmMaxTableSize'];
      if (typeof WebAssembly === 'object' && typeof WebAssembly.Table === 'function') {
        if (MAX_TABLE_SIZE !== undefined) {
          env['table'] = new WebAssembly.Table({ 'initial': TABLE_SIZE, 'maximum': MAX_TABLE_SIZE, 'element': 'anyfunc' });
        } else {
          env['table'] = new WebAssembly.Table({ 'initial': TABLE_SIZE, element: 'anyfunc' });
        }
      } else {
        env['table'] = new Array(TABLE_SIZE); // works in binaryen interpreter at least
      }
      Module['wasmTable'] = env['table'];
    }

    if (!env['memoryBase']) {
      env['memoryBase'] = Module['STATIC_BASE']; // tell the memory segments where to place themselves
    }
    if (!env['tableBase']) {
      env['tableBase'] = 0; // table starts at 0 by default, in dynamic linking this will change
    }

    // try the methods. each should return the exports if it succeeded

    var exports;
#if BINARYEN_METHOD == 'native-wasm'
    exports = doNativeWasm(global, env, providedBuffer);
#else
#if BINARYEN_METHOD == 'asmjs'
    exports = doJustAsm(global, env, providedBuffer);
#else
    var methods = method.split(',');

    for (var i = 0; i < methods.length; i++) {
      var curr = methods[i];

#if RUNTIME_LOGGING
      Module['printErr']('trying binaryen method: ' + curr);
#endif

      finalMethod = curr;

      if (curr === 'native-wasm') {
        if (exports = doNativeWasm(global, env, providedBuffer)) break;
      } else if (curr === 'asmjs') {
        if (exports = doJustAsm(global, env, providedBuffer)) break;
      } else if (curr === 'interpret-asm2wasm' || curr === 'interpret-s-expr' || curr === 'interpret-binary') {
        if (exports = doWasmPolyfill(global, env, providedBuffer, curr)) break;
      } else {
        abort('bad method: ' + curr);
      }
    }

    if (!exports) throw 'no binaryen method succeeded. consider enabling more options, like interpreting, if you want that: https://github.com/kripken/emscripten/wiki/WebAssembly#binaryen-methods';
#endif
#endif

#if RUNTIME_LOGGING
    Module['printErr']('binaryen method succeeded.');
#endif

    return exports;
  };

  var methodHandler = Module['asm']; // note our method handler, as we may modify Module['asm'] later
}

integrateWasmJS(Module);
#endif

// === Body ===
