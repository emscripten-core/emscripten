/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var LibraryStackTrace = {
  $jsStackTrace: () => new Error().stack.toString(),

  $getCallstack__deps: ['$jsStackTrace',
#if ASSERTIONS
    '$warnOnce'
#endif
  ],
  $getCallstack__docs: '/** @param {number=} flags */',
  $getCallstack: (flags) => {
    var callstack = jsStackTrace();

#if ASSERTIONS
    if (flags & {{{ cDefs.EM_LOG_C_STACK }}}) {
      warnOnce('emscripten_log with EM_LOG_C_STACK no longer has any effect');
    }
#endif

    // Process all lines:
    var lines = callstack.split('\n');
    callstack = '';
    // Extract components of form:
    // '       Object._main@http://server.com:4324:12'
    var firefoxRe = new RegExp('\\s*(.*?)@(.*?):([0-9]+):([0-9]+)');
    // Extract components of form:
    // '    at Object._main (http://server.com/file.html:4324:12)'
    var chromeRe = new RegExp('\\s*at (.*?) \\\((.*):(.*):(.*)\\\)');

    for (var line of lines) {
      var symbolName = '';
      var file = '';
      var lineno = 0;
      var column = 0;

      var parts = chromeRe.exec(line);
      if (parts?.length == 5) {
        symbolName = parts[1];
        file = parts[2];
        lineno = parts[3];
        column = parts[4];
      } else {
        parts = firefoxRe.exec(line);
        if (parts?.length >= 4) {
          symbolName = parts[1];
          file = parts[2];
          lineno = parts[3];
          // Old Firefox doesn't carry column information, but in new FF30, it
          // is present. See https://bugzil.la/762556
          column = parts[4]|0;
        } else {
          // Was not able to extract this line for demangling/sourcemapping
          // purposes. Output it as-is.
          callstack += line + '\n';
          continue;
        }
      }

      // Find the symbols in the callstack that corresponds to the functions that
      // report callstack information, and remove everything up to these from the
      // output.
      if (symbolName == '_emscripten_log' || symbolName == '_emscripten_get_callstack') {
        callstack = '';
        continue;
      }

      if ((flags & {{{ cDefs.EM_LOG_C_STACK | cDefs.EM_LOG_JS_STACK }}})) {
        if (flags & {{{ cDefs.EM_LOG_NO_PATHS }}}) {
          file = file.substring(file.replace(/\\/g, "/").lastIndexOf('/')+1);
        }
        callstack += `    at ${symbolName} (${file}:${lineno}:${column})\n`;
      }
    }
    // Trim extra whitespace at the end of the output.
    callstack = callstack.replace(/\s+$/, '');
    return callstack;
  },

  emscripten_get_callstack__deps: ['$getCallstack', '$lengthBytesUTF8', '$stringToUTF8'],
  emscripten_get_callstack: (flags, str, maxbytes) => {
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
    var match;

    if (match = /\bwasm-function\[\d+\]:(0x[0-9a-f]+)/.exec(frame)) {
      // Wasm engines give the binary offset directly, so we use that as return address
      return +match[1];
#if ASSERTIONS
    } else if (match = /\bwasm-function\[(\d+)\]:(\d+)/.exec(frame)) {
      // Older versions of v8 (e.g node v10) give function index and offset in
      // the function.  That format is not supported since it does not provide
      // the information we need to map the frame to a global program counter.
      warnOnce('legacy backtrace format detected, this version of v8 is no longer supported by the emscripten backtrace mechanism')
#endif
    } else if (match = /:(\d+):\d+(?:\)|$)/.exec(frame)) {
      // If we are in js, we can use the js line number as the "return address".
      // This should work for wasm2js.  We tag the high bit to distinguish this
      // from wasm addresses.
      return 0x80000000 | +match[1];
    }
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
  emscripten_stack_snapshot: () => {
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
    for (var line of callstack) {
      var pc = convertFrameToPC(line);
      if (pc) {
        UNWIND_CACHE[pc] = line;
      }
    }
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
  emscripten_pc_get_function__deps: ['$UNWIND_CACHE', 'free', '$stringToNewUTF8', 'emscripten_stack_snapshot'],
  // Don't treat allocation of _emscripten_pc_get_function.ret as a leak
  emscripten_pc_get_function__noleakcheck: true,
  emscripten_pc_get_function: (pc) => {
    var frame = UNWIND_CACHE[pc];
    if (!frame) return 0;

    var name;
    var match;
    // First try to match foo.wasm.sym files explcitly. e.g.
    //
    //   at test_return_address.wasm.main (wasm://wasm/test_return_address.wasm-0012cc2a:wasm-function[26]:0x9f3
    //
    // Then match JS symbols which don't include that module name:
    //
    //   at invokeEntryPoint (.../test_return_address.js:1500:42)
    //
    // Finally match firefox format:
    //
    //   Object._main@http://server.com:4324:12'
    if (match = /^\s+at .*\.wasm\.(.*) \(.*\)$/.exec(frame)) {
      name = match[1];
    } else if (match = /^\s+at (.*) \(.*\)$/.exec(frame)) {
      name = match[1];
    } else if (match = /^(.+?)@/.exec(frame)) {
      name = match[1];
    } else {
      return 0;
    }

    _free(_emscripten_pc_get_function.ret ?? 0);
    _emscripten_pc_get_function.ret = stringToNewUTF8(name);
    return _emscripten_pc_get_function.ret;
  },

  $convertPCtoSourceLocation__deps: ['$UNWIND_CACHE'],
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

    _free(_emscripten_pc_get_file.ret ?? 0);
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
