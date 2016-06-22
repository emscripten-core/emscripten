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
  if (dest + bytes > Math.max(DYNAMICTOP, STATICTOP)) abort('segmentation fault, exceeded the top of the available heap when storing ' + bytes + ' bytes to address ' + dest + '. STATICTOP=' + STATICTOP + ', DYNAMICTOP=' + DYNAMICTOP);
  assert(DYNAMICTOP <= TOTAL_MEMORY);
  setValue(dest, value, getSafeHeapType(bytes, isFloat), 1);
}
function SAFE_HEAP_STORE_D(dest, value, bytes) {
  SAFE_HEAP_STORE(dest, value, bytes, true);
}

function SAFE_HEAP_LOAD(dest, bytes, unsigned, isFloat) {
  if (dest <= 0) abort('segmentation fault loading ' + bytes + ' bytes from address ' + dest);
  if (dest % bytes !== 0) abort('alignment error loading from address ' + dest + ', which was expected to be aligned to a multiple of ' + bytes);
  if (dest + bytes > Math.max(DYNAMICTOP, STATICTOP)) abort('segmentation fault, exceeded the top of the available heap when loading ' + bytes + ' bytes from address ' + dest + '. STATICTOP=' + STATICTOP + ', DYNAMICTOP=' + DYNAMICTOP);
  assert(DYNAMICTOP <= TOTAL_MEMORY);
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

var ABORT = false; // whether we are quitting the application. no code should run after this. set in exit() and abort()
var EXITSTATUS = 0;

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
        ret = Runtime.stackAlloc((str.length << 2) + 1);
        writeStringToMemory(str, ret);
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
      HEAPU8.set(slab, ret);
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
  if ((typeof _sbrk !== 'undefined' && !_sbrk.called) || !runtimeInitialized) return Runtime.dynamicAlloc(size);
  return _malloc(size);
}
{{{ maybeExport('getMemory') }}}

function Pointer_stringify(ptr, /* optional */ length) {
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

function UTF8ArrayToString(u8Array, idx) {
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
// Use the function lengthBytesUTF8() to compute the exact number of bytes (excluding null terminator) that this function will write.
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
// Use the function lengthBytesUTF8() to compute the exact number of bytes (excluding null terminator) that this function will write.
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

function UTF16ToString(ptr) {
  var i = 0;

  var str = '';
  while (1) {
    var codeUnit = {{{ makeGetValue('ptr', 'i*2', 'i16') }}};
    if (codeUnit == 0)
      return str;
    ++i;
    // fromCharCode constructs a character from a UTF-16 code unit, so we can pass the UTF16 string right through.
    str += String.fromCharCode(codeUnit);
  }
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
  var hasLibcxxabi = !!Module['___cxa_demangle'];
  if (hasLibcxxabi) {
    try {
      var buf = _malloc(func.length);
      writeStringToMemory(func.substr(1), buf);
      var status = _malloc(4);
      var ret = Module['___cxa_demangle'](buf, 0, 0, status);
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
  return text.replace(/__Z[\w\d_]+/g, function(x) { var y = demangle(x); return x === y ? x : (x + ' [' + y + ']') });
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

var PAGE_SIZE = 4096;

function alignMemoryPage(x) {
  if (x % 4096 > 0) {
    x += (4096 - (x % 4096));
  }
  return x;
}

var HEAP;
var buffer;
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

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

var STATIC_BASE = 0, STATICTOP = 0, staticSealed = false; // static area
var STACK_BASE = 0, STACKTOP = 0, STACK_MAX = 0; // stack area
var DYNAMIC_BASE = 0, DYNAMICTOP = 0; // dynamic area handled by sbrk

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
  abort('Stack overflow! Attempted to allocate ' + allocSize + ' bytes on the stack, but stack has only ' + (STACK_MAX - asm.stackSave() + allocSize) + ' bytes available!');
}
#endif

#if ALLOW_MEMORY_GROWTH == 0
function abortOnCannotGrowMemory() {
  abort('Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value ' + TOTAL_MEMORY + ', (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which adjusts the size at runtime but prevents some optimizations, (3) set Module.TOTAL_MEMORY to a higher value before the program runs, or if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ');
}
#else // ALLOW_MEMORY_GROWTH
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
  assert(DYNAMICTOP >= TOTAL_MEMORY);
  assert(TOTAL_MEMORY > 4); // So the loop below will not be infinite
#endif

  var OLD_TOTAL_MEMORY = TOTAL_MEMORY;

#if EMSCRIPTEN_TRACING
  // Report old layout one last time
  _emscripten_trace_report_memory_layout();
#endif

  var LIMIT = Math.pow(2, 31); // 2GB is a practical maximum, as we use signed ints as pointers
                               // and JS engines seem unhappy to give us 2GB arrays currently
  if (DYNAMICTOP >= LIMIT) return false;

  while (TOTAL_MEMORY <= DYNAMICTOP) { // Simple heuristic.
    if (TOTAL_MEMORY < LIMIT/2) {
      TOTAL_MEMORY = alignMemoryPage(2*TOTAL_MEMORY); // double until 1GB
    } else {
      var last = TOTAL_MEMORY;
      TOTAL_MEMORY = alignMemoryPage((3*TOTAL_MEMORY + LIMIT)/4); // add smaller increments towards 2GB, which we cannot reach
      if (TOTAL_MEMORY <= last) return false;
    }
  }

  TOTAL_MEMORY = Math.max(TOTAL_MEMORY, 16*1024*1024);

  if (TOTAL_MEMORY >= LIMIT) return false;

#if ASSERTIONS
  Module.printErr('Warning: Enlarging memory arrays, this is not fast! ' + [OLD_TOTAL_MEMORY, TOTAL_MEMORY]);
#endif

#if EMSCRIPTEN_TRACING
  _emscripten_trace_js_log_message("Emscripten", "Enlarging memory arrays from " + OLD_TOTAL_MEMORY + " to " + TOTAL_MEMORY);
  // And now report the new layout
  _emscripten_trace_report_memory_layout();
#endif

#if ASSERTIONS
  var start = Date.now();
#endif

  var replacement = Module['reallocBuffer'](TOTAL_MEMORY);
  if (!replacement) return false;

  // everything worked

  updateGlobalBuffer(replacement);
  updateGlobalBufferViews();

#if ASSERTIONS
  Module.printErr('enlarged memory arrays from ' + OLD_TOTAL_MEMORY + ' to ' + TOTAL_MEMORY + ', took ' + (Date.now() - start) + ' ms (has ArrayBuffer.transfer? ' + (!!ArrayBuffer.transfer) + ')');
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

var totalMemory = 64*1024;
while (totalMemory < TOTAL_MEMORY || totalMemory < 2*TOTAL_STACK) {
  if (totalMemory < 16*1024*1024) {
    totalMemory *= 2;
  } else {
    totalMemory += 16*1024*1024
  }
}
#if ALLOW_MEMORY_GROWTH
totalMemory = Math.max(totalMemory, 16*1024*1024);
#endif
if (totalMemory !== TOTAL_MEMORY) {
#if ASSERTIONS
  Module.printErr('increasing TOTAL_MEMORY to ' + totalMemory + ' to be compliant with the asm.js spec (and given that TOTAL_STACK=' + TOTAL_STACK + ')');
#endif
  TOTAL_MEMORY = totalMemory;
}

// Initialize the runtime's memory
#if ASSERTIONS
// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && !!(new Int32Array(1)['subarray']) && !!(new Int32Array(1)['set']),
       'JS engine does not provide full typed array support');
#endif

#if IN_TEST_HARNESS
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
  Atomics['wait'] = function(t, i, v, o) { if (t[i] != v) abort('Multithreading is not supported, cannot sleep to wait for futex!'); }
  Atomics['wake'] = function(t, i, c) {}
  Atomics['wakeOrRequeue'] = function(t, i1, c, i2, v) {}
  Atomics['isLockFree'] = function(s) { return true; }
  Atomics['load'] = function(t, i) { return t[i]; }
  Atomics['or'] = function(t, i, v) { var w = t[i]; t[i] |= v; return w; }
  Atomics['store'] = function(t, i, v) { t[i] = v; return v; }
  Atomics['sub'] = function(t, i, v) { var w = t[i]; t[i] -= v; return w; }
  Atomics['xor'] = function(t, i, v) { var w = t[i]; t[i] ^= v; return w; }
}

// In old Atomics spec, Atomics.OK/.TIMEDOUT/.NOTEQUAL were integers. In new spec, Atomics functions return strings, so if we are in the new spec,
// assign the strings to the place where the integers would have been to keep implementation of emscripten_futex_wait straightforward without dynamic checks for both.
// See https://github.com/tc39/ecmascript_sharedmem/issues/69 for details.
if (typeof Atomics['OK'] === 'undefined') {
  Atomics['OK'] = 'ok';
  Atomics['TIMEDOUT'] = 'timed-out';
  Atomics['NOTEQUAL'] = 'not-equal';
}

// If running browser with old API names, account for function renames. See https://bugzilla.mozilla.org/show_bug.cgi?id=1260910.
if (typeof Atomics['wait'] === 'undefined') {
  Atomics['wait'] = Atomics['futexWait'];
  Atomics['wake'] = Atomics['futexWake'];
  Atomics['wakeOrRequeue'] = Atomics['futexWakeOrRequeue'];
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
  buffer = new ArrayBuffer(TOTAL_MEMORY);
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
        Runtime.dynCall('v', func);
      } else {
        Runtime.dynCall('vi', func, [callback.arg]);
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


function intArrayFromString(stringy, dontAddNull, length /* optional */) {
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

function writeStringToMemory(string, buffer, dontAddNull) {
  var array = intArrayFromString(string, dontAddNull);
  var i = 0;
  while (i < array.length) {
    var chr = array[i];
    {{{ makeSetValue('buffer', 'i', 'chr', 'i8') }}};
    i = i + 1;
  }
}
{{{ maybeExport('writeStringToMemory') }}}

function writeArrayToMemory(array, buffer) {
  for (var i = 0; i < array.length; i++) {
    {{{ makeSetValue('buffer++', 0, 'array[i]', 'i8') }}};
  }
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

#if USE_PTHREADS
// Atomics.exchange is not yet implemented in the spec, so polyfill that in via compareExchange in the meanwhile.
// TODO: Keep an eye out for the opportunity to remove this once Atomics.exchange is available.
if (typeof Atomics !== 'undefined' && !Atomics['exchange']) {
  Atomics['exchange'] = function(heap, index, val) {
    var oldVal, oldVal2;
    do {
      oldVal = Atomics['load'](heap, index);
      oldVal2 = Atomics['compareExchange'](heap, index, oldVal, val);
    } while(oldVal != oldVal2);
    return oldVal;
  }
}
#endif

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
  if (Module['dynamicLibraries']) {
    Module['dynamicLibraries'].forEach(function(lib) {
      Runtime.loadDynamicLibrary(lib);
    });
  }
  asm['runPostSets']();
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

// === Body ===
