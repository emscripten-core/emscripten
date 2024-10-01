/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryStackTrace = {
  $jsStackTrace: function() {
    return new Error().stack.toString();
  },

  $getCallstack__deps: ['$jsStackTrace', '$warnOnce'],
  $getCallstack__docs: '/** @param {number=} flags */',
  $getCallstack: function(flags) {
    var callstack = jsStackTrace();

    // Find the symbols in the callstack that corresponds to the functions that
    // report callstack information, and remove everything up to these from the
    // output.
    var iThisFunc = callstack.lastIndexOf('_emscripten_log');
    var iThisFunc2 = callstack.lastIndexOf('_emscripten_get_callstack');
    var iNextLine = callstack.indexOf('\n', Math.max(iThisFunc, iThisFunc2))+1;
    callstack = callstack.slice(iNextLine);

    // If user requested to see the original source stack, but no source map
    // information is available, just fall back to showing the JS stack.
    if (flags & {{{ cDefs.EM_LOG_C_STACK }}} && typeof emscripten_source_map == 'undefined') {
      warnOnce('Source map information is not available, emscripten_log with EM_LOG_C_STACK will be ignored. Build with "--pre-js $EMSCRIPTEN/src/emscripten-source-map.min.js" linker flag to add source map loading to code.');
      flags ^= {{{ cDefs.EM_LOG_C_STACK }}};
      flags |= {{{ cDefs.EM_LOG_JS_STACK }}};
    }

    // Process all lines:
    var lines = callstack.split('\n');
    callstack = '';
    // New FF30 with column info: extract components of form:
    // '       Object._main@http://server.com:4324:12'
    var newFirefoxRe = new RegExp('\\s*(.*?)@(.*?):([0-9]+):([0-9]+)');
    // Old FF without column info: extract components of form:
    // '       Object._main@http://server.com:4324'
    var firefoxRe = new RegExp('\\s*(.*?)@(.*):(.*)(:(.*))?');
    // Extract components of form:
    // '    at Object._main (http://server.com/file.html:4324:12)'
    var chromeRe = new RegExp('\\s*at (.*?) \\\((.*):(.*):(.*)\\\)');

    for (var l in lines) {
      var line = lines[l];

      var symbolName = '';
      var file = '';
      var lineno = 0;
      var column = 0;

      var parts = chromeRe.exec(line);
      if (parts && parts.length == 5) {
        symbolName = parts[1];
        file = parts[2];
        lineno = parts[3];
        column = parts[4];
      } else {
        parts = newFirefoxRe.exec(line) || firefoxRe.exec(line);
        if (parts && parts.length >= 4) {
          symbolName = parts[1];
          file = parts[2];
          lineno = parts[3];
          // Old Firefox doesn't carry column information, but in new FF30, it
          // is present. See https://bugzilla.mozilla.org/show_bug.cgi?id=762556
          column = parts[4]|0;
        } else {
          // Was not able to extract this line for demangling/sourcemapping
          // purposes. Output it as-is.
          callstack += line + '\n';
          continue;
        }
      }

      var haveSourceMap = false;

      if (flags & {{{ cDefs.EM_LOG_C_STACK }}}) {
        var orig = emscripten_source_map.originalPositionFor({line: lineno, column: column});
        haveSourceMap = orig?.source;
        if (haveSourceMap) {
          if (flags & {{{ cDefs.EM_LOG_NO_PATHS }}}) {
            orig.source = orig.source.substring(orig.source.replace(/\\/g, "/").lastIndexOf('/')+1);
          }
          callstack += `    at ${symbolName} (${orig.source}:${orig.line}:${orig.column})\n`;
        }
      }
      if ((flags & {{{ cDefs.EM_LOG_JS_STACK }}}) || !haveSourceMap) {
        if (flags & {{{ cDefs.EM_LOG_NO_PATHS }}}) {
          file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
        }
        callstack += (haveSourceMap ? (`     = ${symbolName}`) : (`    at ${symbolName}`)) + ` (${file}:${lineno}:${column})\n`;
      }
    }
    // Trim extra whitespace at the end of the output.
    callstack = callstack.replace(/\s+$/, '');
    return callstack;
  },

  emscripten_get_callstack__deps: ['$getCallstack', '$lengthBytesUTF8', '$stringToUTF8'],
  emscripten_get_callstack: function(flags, str, maxbytes) {
    var callstack = getCallstack(flags);
    // User can query the required amount of bytes to hold the callstack.
    if (!str || maxbytes <= 0) {
      return lengthBytesUTF8(callstack)+1;
    }
    // Output callstack string as C string to HEAP.
    var bytesWrittenExcludingNull = stringToUTF8(callstack, str, maxbytes);

    // Return number of bytes written, including null.
    return bytesWrittenExcludingNull+1;
  },

  // Generates a representation of the program counter from a line of stack trace.
  // The exact return value depends in whether we are running WASM or JS, and whether
  // the engine supports offsets into WASM. See the function body for details.
  $convertFrameToPC__docs: '/** @returns {number} */',
  $convertFrameToPC__internal: true,
  $convertFrameToPC: (frame) => {
#if !USE_OFFSET_CONVERTER
    abort('Cannot use convertFrameToPC (needed by __builtin_return_address) without -sUSE_OFFSET_CONVERTER');
#else
#if ASSERTIONS
    assert(wasmOffsetConverter);
#endif
    var match;

    if (match = /\bwasm-function\[\d+\]:(0x[0-9a-f]+)/.exec(frame)) {
      // some engines give the binary offset directly, so we use that as return address
      return +match[1];
    } else if (match = /\bwasm-function\[(\d+)\]:(\d+)/.exec(frame)) {
      // other engines only give function index and offset in the function,
      // so we try using the offset converter. If that doesn't work,
      // we pack index and offset into a "return address"
      return wasmOffsetConverter.convert(+match[1], +match[2]);
    } else if (match = /:(\d+):\d+(?:\)|$)/.exec(frame)) {
      // If we are in js, we can use the js line number as the "return address".
      // This should work for wasm2js.  We tag the high bit to distinguish this
      // from wasm addresses.
      return 0x80000000 | +match[1];
    }
#endif
    // return 0 if we can't find any
    return 0;
  },

  // Returns a representation of a call site of the caller of this function, in a manner
  // similar to __builtin_return_address. If level is 0, we return the call site of the
  // caller of this function.
  emscripten_return_address__deps: ['$convertFrameToPC', '$jsStackTrace'],
  emscripten_return_address: (level) => {
    var callstack = jsStackTrace().split('\n');
    if (callstack[0] == 'Error') {
      callstack.shift();
    }
    // skip this function and the caller to get caller's return address
#if MEMORY64
    // MEMORY64 injects and extra wrapper within emscripten_return_address
    // to handle BigInt conversions.
    var caller = callstack[level + 4];
#else
    var caller = callstack[level + 3];
#endif
    return convertFrameToPC(caller);
  },

  $UNWIND_CACHE: {},

  // This function pulls the JavaScript stack trace and updates UNWIND_CACHE so
  // that our representation of the program counter is mapped to the line of the
  // stack trace for every line in the stack trace. This allows
  // emscripten_pc_get_* to lookup the line of the stack trace from the PC and
  // return meaningful information.
  //
  // Additionally, it saves a copy of the entire stack trace and the return
  // address of the caller. This is because there are two common forms of a
  // stack trace.  The first form starts the stack trace at the caller of the
  // function requesting a stack trace. In this case, the function can simply
  // walk down the stack from the return address using emscripten_return_address
  // with increasing values for level.  The second form starts the stack trace
  // at the current function. This requires a helper function to get the program
  // counter. This helper function will return the return address.  This is the
  // program counter at the call site. But there is a problem: when calling into
  // code that performs stack unwinding, the program counter has changed since
  // execution continued from calling the helper function. So we can't just walk
  // down the stack and expect to see the PC value we got. By caching the call
  // stack, we can call emscripten_stack_unwind with the PC value and use that
  // to unwind the cached stack. Naturally, the PC helper function will have to
  // call emscripten_stack_snapshot to cache the stack. We also return the
  // return address of the caller so the PC helper function does not need to
  // call emscripten_return_address, saving a lot of time.
  //
  // One might expect that a sensible solution is to call the stack unwinder and
  // explicitly tell it how many functions to skip from the stack. However,
  // existing libraries do not work this way.  For example, compiler-rt's
  // sanitizer_common library has macros GET_CALLER_PC_BP_SP and
  // GET_CURRENT_PC_BP_SP, which obtains the PC value for the two common cases
  // stated above, respectively. Then, it passes the PC, BP, SP values along
  // until some other function uses them to unwind. On standard machines, the
  // stack can be unwound by treating BP as a linked list.  This makes PC
  // unnecessary to walk the stack, since walking is done with BP, which remains
  // valid until the function returns. But on Emscripten, BP does not exist, at
  // least in JavaScript frames, so we have to rely on PC values. Therefore, we
  // must be able to unwind from a PC value that may no longer be on the
  // execution stack, and so we are forced to cache the entire call stack.
  emscripten_stack_snapshot__deps: ['$convertFrameToPC', '$UNWIND_CACHE', '$saveInUnwindCache', '$jsStackTrace'],
  emscripten_stack_snapshot: function() {
    var callstack = jsStackTrace().split('\n');
    if (callstack[0] == 'Error') {
      callstack.shift();
    }
    saveInUnwindCache(callstack);

    // Caches the stack snapshot so that emscripten_stack_unwind_buffer() can
    // unwind from this spot.
    UNWIND_CACHE.last_addr = convertFrameToPC(callstack[3]);
    UNWIND_CACHE.last_stack = callstack;
    return UNWIND_CACHE.last_addr;
  },

  $saveInUnwindCache__deps: ['$UNWIND_CACHE', '$convertFrameToPC'],
  $saveInUnwindCache__internal: true,
  $saveInUnwindCache: (callstack) => {
    callstack.forEach((frame) => {
      var pc = convertFrameToPC(frame);
      if (pc) {
        UNWIND_CACHE[pc] = frame;
      }
    });
  },

  // Unwinds the stack from a cached PC value. See emscripten_stack_snapshot for
  // how this is used.  addr must be the return address of the last call to
  // emscripten_stack_snapshot, or this function will instead use the current
  // call stack.
  emscripten_stack_unwind_buffer__deps: ['$UNWIND_CACHE', '$saveInUnwindCache', '$convertFrameToPC', '$jsStackTrace'],
  emscripten_stack_unwind_buffer: (addr, buffer, count) => {
    var stack;
    if (UNWIND_CACHE.last_addr == addr) {
      stack = UNWIND_CACHE.last_stack;
    } else {
      stack = jsStackTrace().split('\n');
      if (stack[0] == 'Error') {
        stack.shift();
      }
      saveInUnwindCache(stack);
    }

    var offset = 3;
    while (stack[offset] && convertFrameToPC(stack[offset]) != addr) {
      ++offset;
    }

    for (var i = 0; i < count && stack[i+offset]; ++i) {
      {{{ makeSetValue('buffer', 'i*4', 'convertFrameToPC(stack[i + offset])', 'i32') }}};
    }
    return i;
  },

  // Look up the function name from our stack frame cache with our PC representation.
#if USE_OFFSET_CONVERTER
  emscripten_pc_get_function__deps: ['$UNWIND_CACHE', 'free', '$stringToNewUTF8'],
  // Don't treat allocation of _emscripten_pc_get_function.ret as a leak
  emscripten_pc_get_function__noleakcheck: true,
#endif
  emscripten_pc_get_function: (pc) => {
#if !USE_OFFSET_CONVERTER
    abort('Cannot use emscripten_pc_get_function without -sUSE_OFFSET_CONVERTER');
    return 0;
#else
    var name;
    if (pc & 0x80000000) {
      // If this is a JavaScript function, try looking it up in the unwind cache.
      var frame = UNWIND_CACHE[pc];
      if (!frame) return 0;

      var match;
      if (match = /^\s+at (.*) \(.*\)$/.exec(frame)) {
        name = match[1];
      } else if (match = /^(.+?)@/.exec(frame)) {
        name = match[1];
      } else {
        return 0;
      }
    } else {
      name = wasmOffsetConverter.getName(pc);
    }
    if (_emscripten_pc_get_function.ret) _free(_emscripten_pc_get_function.ret);
    _emscripten_pc_get_function.ret = stringToNewUTF8(name);
    return _emscripten_pc_get_function.ret;
#endif
  },

  $convertPCtoSourceLocation__deps: ['$UNWIND_CACHE', '$convertFrameToPC'],
  $convertPCtoSourceLocation: (pc) => {
    if (UNWIND_CACHE.last_get_source_pc == pc) return UNWIND_CACHE.last_source;

    var match;
    var source;
#if LOAD_SOURCE_MAP
    if (wasmSourceMap) {
      source = wasmSourceMap.lookup(pc);
    }
#endif

    if (!source) {
      var frame = UNWIND_CACHE[pc];
      if (!frame) return null;
      // Example: at callMain (a.out.js:6335:22)
      if (match = /\((.*):(\d+):(\d+)\)$/.exec(frame)) {
        source = {file: match[1], line: match[2], column: match[3]};
      // Example: main@a.out.js:1337:42
      } else if (match = /@(.*):(\d+):(\d+)/.exec(frame)) {
        source = {file: match[1], line: match[2], column: match[3]};
      }
    }
    UNWIND_CACHE.last_get_source_pc = pc;
    UNWIND_CACHE.last_source = source;
    return source;
  },

  // Look up the file name from our stack frame cache with our PC representation.
  emscripten_pc_get_file__deps: ['$convertPCtoSourceLocation', 'free', '$stringToNewUTF8'],
  // Don't treat allocation of _emscripten_pc_get_file.ret as a leak
  emscripten_pc_get_file__noleakcheck: true,
  emscripten_pc_get_file: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    if (!result) return 0;

    if (_emscripten_pc_get_file.ret) _free(_emscripten_pc_get_file.ret);
    _emscripten_pc_get_file.ret = stringToNewUTF8(result.file);
    return _emscripten_pc_get_file.ret;
  },

  // Look up the line number from our stack frame cache with our PC representation.
  emscripten_pc_get_line__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_line: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.line : 0;
  },

  // Look up the column number from our stack frame cache with our PC representation.
  emscripten_pc_get_column__deps: ['$convertPCtoSourceLocation'],
  emscripten_pc_get_column: (pc) => {
    var result = convertPCtoSourceLocation(pc);
    return result ? result.column || 0 : 0;
  },
}

addToLibrary(LibraryStackTrace);
