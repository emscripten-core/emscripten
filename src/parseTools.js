/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Helpers and tools for use at compile time by JavaScript library files.
 *
 * Tests live in test/other/test_parseTools.js.
 */

global.FOUR_GB = 4 * 1024 * 1024 * 1024;
const FLOAT_TYPES = new Set(['float', 'double']);

// Does simple 'macro' substitution, using Django-like syntax,
// {{{ code }}} will be replaced with |eval(code)|.
// NOTE: Be careful with that ret check. If ret is |0|, |ret ? ret.toString() : ''| would result in ''!
function processMacros(text) {
  // The `?` here in makes the regex non-greedy so it matches with the closest
  // set of closing braces.
  // `[\s\S]` works like `.` but include newline.
  return text.replace(/{{{([\s\S]+?)}}}/g, (_, str) => {
    try {
      const ret = eval(str);
      return ret !== null ? ret.toString() : '';
    } catch (ex) {
      ex.stack = 'In the following macro:\n\n' + str + '\n\n' + ex.stack;
      throw ex;
    }
  });
}

// Simple #if/else/endif preprocessing for a file. Checks if the
// ident checked is true in our global.
// Also handles #include x.js (similar to C #include <file>)
function preprocess(filename) {
  let text = read(filename);
  if (EXPORT_ES6 && USE_ES6_IMPORT_META) {
    // `eval`, Terser and Closure don't support module syntax; to allow it,
    // we need to temporarily replace `import.meta` and `await import` usages
    // with placeholders during preprocess phase, and back after all the other ops.
    // See also: `phase_final_emitting` in emcc.py.
    text = text
      .replace(/\bimport\.meta\b/g, 'EMSCRIPTEN$IMPORT$META')
      .replace(/\bawait import\b/g, 'EMSCRIPTEN$AWAIT$IMPORT');
  }
  // Remove windows line endings, if any
  text = text.replace(/\r\n/g, '\n');

  const IGNORE = 0;
  const SHOW = 1;
  // This state is entered after we have shown one of the block of an if/elif/else sequence.
  // Once we enter this state we dont show any blocks or evaluate any
  // conditions until the sequence ends.
  const IGNORE_ALL = 2;
  const showStack = [];
  const showCurrentLine = () => showStack.every((x) => x == SHOW);

  const oldFilename = currentFile;
  currentFile = filename;
  const fileExt = filename.split('.').pop().toLowerCase();
  const isHtml = (fileExt === 'html' || fileExt === 'htm') ? true : false;
  let inStyle = false;
  const lines = text.split('\n');
  // text.split yields an extra empty element at the end if text itself ends with a newline.
  if (!lines[lines.length - 1]) {
    lines.pop();
  }

  let ret = '';
  let emptyLine = false;

  try {
    for (let [i, line] of lines.entries()) {
      if (isHtml) {
        if (line.includes('<style') && !inStyle) {
          inStyle = true;
        }
        if (line.includes('</style') && inStyle) {
          inStyle = false;
        }
        if (inStyle) {
          if (showCurrentLine()) {
            ret += line + '\n';
          }
          continue;
        }
      }

      const trimmed = line.trim();
      if (trimmed.startsWith('#')) {
        const first = trimmed.split(' ', 1)[0];
        if (first == '#if' || first == '#ifdef' || first == '#elif') {
          if (first == '#ifdef') {
            warn('use of #ifdef in js library.  Use #if instead.');
          }
          if (first == '#elif') {
            const curr = showStack.pop();
            if (curr == SHOW || curr == IGNORE_ALL) {
              // If we showed to previous block we enter the IGNORE_ALL state
              // and stay there until endif is seen
              showStack.push(IGNORE_ALL);
              continue;
            }
          }
          const after = trimmed.substring(trimmed.indexOf(' '));
          const truthy = !!vm.runInThisContext(after, { filename, lineOffset: i, columnOffset: line.indexOf(after) });
          showStack.push(truthy ? SHOW : IGNORE);
        } else if (first === '#include') {
          if (showCurrentLine()) {
            let filename = line.substr(line.indexOf(' ') + 1);
            if (filename.startsWith('"')) {
              filename = filename.substr(1, filename.length - 2);
            }
            const result = preprocess(filename);
            if (result) {
              ret += `// include: ${filename}\n`;
              ret += result;
              ret += `// end include: ${filename}\n`;
            }
          }
        } else if (first === '#else') {
          assert(showStack.length > 0);
          const curr = showStack.pop();
          if (curr == IGNORE) {
            showStack.push(SHOW);
          } else {
            showStack.push(IGNORE);
          }
        } else if (first === '#endif') {
          assert(showStack.length > 0);
          showStack.pop();
        } else if (first === '#warning') {
          if (showCurrentLine()) {
            printErr(`${filename}:${i + 1}: #warning ${trimmed.substring(trimmed.indexOf(' ')).trim()}`);
          }
        } else if (first === '#error') {
          if (showCurrentLine()) {
            error(`${filename}:${i + 1}: #error ${trimmed.substring(trimmed.indexOf(' ')).trim()}`);
          }
        } else {
          error(`${filename}:${i + 1}: Unknown preprocessor directive ${first}`);
        }
      } else {
        if (showCurrentLine()) {
          // Never emit more than one empty line at a time.
          if (emptyLine && !line) {
            continue;
          }
          ret += line + '\n';
          if (!line) {
            emptyLine = true;
          } else {
            emptyLine = false;
          }
        }
      }
    }
    assert(showStack.length == 0, `preprocessing error in file ${filename}, \
no matching #endif found (${showStack.length$}' unmatched preprocessing directives on stack)`);
    return ret;
  } finally {
    currentFile = oldFilename;
  }
}

// Returns true if ident is a niceIdent (see toNiceIdent). Also allow () and spaces.
function isNiceIdent(ident, loose) {
  return /^\(?[$_]+[\w$_\d ]*\)?$/.test(ident);
}

// Simple variables or numbers, or things already quoted, do not need to be quoted
function needsQuoting(ident) {
  if (/^[-+]?[$_]?[\w$_\d]*$/.test(ident)) return false; // number or variable
  if (ident[0] === '(' && ident[ident.length - 1] === ')' && ident.indexOf('(', 1) < 0) return false; // already fully quoted
  return true;
}

global.POINTER_SIZE = MEMORY64 ? 8 : 4;
global.STACK_ALIGN = 16;
const POINTER_BITS = POINTER_SIZE * 8;
const POINTER_TYPE = 'u' + POINTER_BITS;
const POINTER_JS_TYPE = MEMORY64 ? "'bigint'" : "'number'";
const POINTER_SHIFT = MEMORY64 ? '3' : '2';
const POINTER_HEAP = MEMORY64 ? 'HEAP64' : 'HEAP32';

const SIZE_TYPE = POINTER_TYPE;


// Similar to POINTER_TYPE, but this is the actual wasm type that is
// used in practice, while POINTER_TYPE is the more refined internal
// type (that is unsigned, where as core wasm does not have unsigned
// types).
const POINTER_WASM_TYPE = 'i' + POINTER_BITS;

function isPointerType(type) {
  return type[type.length - 1] == '*';
}

// Given an expression like (VALUE=VALUE*2,VALUE<10?VALUE:t+1) , this will
// replace VALUE with value. If value is not a simple identifier of a variable,
// value will be replaced with tempVar.
function makeInlineCalculation(expression, value, tempVar) {
  if (!isNiceIdent(value)) {
    expression = tempVar + '=' + value + ',' + expression;
    value = tempVar;
  }
  return '(' + expression.replace(/VALUE/g, value) + ')';
}

// XXX Make all i64 parts signed

// Splits a number (an integer in a double, possibly > 32 bits) into an i64
// value, represented by a low and high i32 pair.
// Will suffer from rounding and truncation.
function splitI64(value) {
  if (WASM_BIGINT) {
    // Nothing to do: just make sure it is a BigInt (as it must be of that
    // type, to be sent into wasm).
    return `BigInt(${value})`;
  }

  // general idea:
  //
  //  $1$0 = ~~$d >>> 0;
  //  $1$1 = Math.abs($d) >= 1 ? (
  //     $d > 0 ? Math.floor(($d)/ 4294967296.0) >>> 0
  //            : Math.ceil(Math.min(-4294967296.0, $d - $1$0)/ 4294967296.0)
  //  ) : 0;
  //
  // We need to min on positive values here, since our input might be a double,
  // and large values are rounded, so they can be slightly higher than expected.
  // And if we get 4294967296, that will turn into a 0 if put into a HEAP32 or
  // |0'd, etc.
  //
  // For negatives, we need to ensure a -1 if the value is overall negative,
  // even if not significant negative component

  const low = value + '>>>0';
  const high = makeInlineCalculation(
      asmCoercion('Math.abs(VALUE)', 'double') + ' >= ' + asmEnsureFloat('1', 'double') + ' ? ' +
        '(VALUE > ' + asmEnsureFloat('0', 'double') + ' ? ' +
        asmCoercion('Math.floor((VALUE)/' +
        asmEnsureFloat(4294967296, 'double') + ')', 'double') + '>>>0' +
        ' : ' +
        asmFloatToInt(asmCoercion('Math.ceil((VALUE - +((' + asmFloatToInt('VALUE') + ')>>>0))/' +
        asmEnsureFloat(4294967296, 'double') + ')', 'double')) + '>>>0' +
        ')' +
      ' : 0',
      value,
      'tempDouble',
  );
  return [low, high];
}

// Misc

function indentify(text, indent) {
  // Don't try to indentify huge strings - we may run out of memory
  if (text.length > 1024 * 1024) return text;
  if (typeof indent == 'number') {
    const len = indent;
    indent = '';
    for (let i = 0; i < len; i++) {
      indent += ' ';
    }
  }
  return text.replace(/\n/g, '\n' + indent);
}

// Correction tools

function getNativeTypeSize(type) {
  switch (type) {
    case 'i1': case 'i8': case 'u8': return 1;
    case 'i16': case 'u16': return 2;
    case 'i32': case 'u32': return 4;
    case 'i64': case 'u64': return 8;
    case 'float': return 4;
    case 'double': return 8;
    default: {
      if (type[type.length - 1] === '*') {
        return POINTER_SIZE;
      }
      if (type[0] === 'i') {
        const bits = Number(type.substr(1));
        assert(bits % 8 === 0, 'getNativeTypeSize invalid bits ' + bits + ', type ' + type);
        return bits / 8;
      }
      return 0;
    }
  }
}

function getHeapOffset(offset, type) {
  const sz = getNativeTypeSize(type);
  const shifts = Math.log(sz) / Math.LN2;
  if (MEMORY64 == 1) {
    return `((${offset})/2**${shifts})`;
  } else {
    return `((${offset})>>${shifts})`;
  }
}

function ensureDot(value) {
  value = value.toString();
  // if already dotted, or Infinity or NaN, nothing to do here
  // if smaller than 1 and running js opts, we always need to force a coercion
  // (0.001 will turn into 1e-3, which has no .)
  if ((value.includes('.') || /[IN]/.test(value))) return value;
  const e = value.indexOf('e');
  if (e < 0) return value + '.0';
  return value.substr(0, e) + '.0' + value.substr(e);
}

// ensures that a float type has either 5.5 (clearly a float) or +5 (float due to asm coercion)
function asmEnsureFloat(value, type) {
  if (!isNumber(value)) return value;
  if (type === 'float') {
    // normally ok to just emit Math.fround(0), but if the constant is large we
    // may need a .0 (if it can't fit in an int)
    if (value == 0) return 'Math.fround(0)';
    value = ensureDot(value);
    return 'Math.fround(' + value + ')';
  }
  if (FLOAT_TYPES.has(type)) {
    return ensureDot(value);
  }
  return value;
}

function asmCoercion(value, type) {
  assert(arguments.length == 2, 'asmCoercion takes exactly two arguments');
  if (type == 'void') {
    return value;
  }
  if (FLOAT_TYPES.has(type)) {
    if (isNumber(value)) {
      return asmEnsureFloat(value, type);
    }
    if (type === 'float') {
      return 'Math.fround(' + value + ')';
    }
    return '(+(' + value + '))';
  }
  return '((' + value + ')|0)';
}

function asmFloatToInt(x) {
  return '(~~(' + x + '))';
}

// See makeSetValue
function makeGetValue(ptr, pos, type, noNeedFirst, unsigned, ignore, align) {
  assert(typeof align === 'undefined', 'makeGetValue no longer supports align parameter');
  assert(typeof noNeedFirst === 'undefined', 'makeGetValue no longer supports noNeedFirst parameter');
  if (typeof unsigned !== 'undefined') {
    // TODO(sbc): make this into an error at some point.
    printErr('makeGetValue: Please use u8/u16/u32/u64 unsigned types in favor of additional argument');
    if (unsigned && type.startsWith('i')) {
      type = 'u' + type.slice(1);
    }
  } else if (type.startsWith('u')) {
    // Set `unsigned` based on the type name.
    unsigned = true;
  }

  const offset = calcFastOffset(ptr, pos);
  if (type === 'i53' || type === 'u53') {
    return 'readI53From' + (unsigned ? 'U' : 'I') + '64(' + offset + ')';
  }

  const slab = getHeapForType(type);
  let ret = slab + '[' + getHeapOffset(offset, type) + ']';
  if (MEMORY64 && isPointerType(type)) {
    ret = `Number(${ret})`;
  }
  return ret;
}

/**
 * @param {number} ptr The pointer. Used to find both the slab and the offset in that slab. If the pointer
 *            is just an integer, then this is almost redundant, but in general the pointer type
 *            may in the future include information about which slab as well. So, for now it is
 *            possible to put |0| here, but if a pointer is available, that is more future-proof.
 * @param {number} pos The position in that slab - the offset. Added to any offset in the pointer itself.
 * @param {number} value The value to set.
 * @param {string} type A string defining the type. Used to find the slab (HEAPU8, HEAP16, HEAPU32, etc.).
 *             which means we should write to all slabs, ignore type differences if any on reads, etc.
 * @return {string} JS code for performing the memory set operation
 */
function makeSetValue(ptr, pos, value, type) {
  var rtn = makeSetValueImpl(ptr, pos, value, type);
  if (ASSERTIONS == 2 && (type.startsWith('i') || type.startsWith('u'))) {
    const width = getBitWidth(type);
    const assertion = `checkInt${width}(${value})`;
    rtn += ';' + assertion
  }
  return rtn;
}

function makeSetValueImpl(ptr, pos, value, type) {
  if (type == 'i64' && !WASM_BIGINT) {
    // If we lack BigInt support we must fall back to an reading a pair of I32
    // values.
    return '(tempI64 = [' + splitI64(value) + '], ' +
            makeSetValueImpl(ptr, pos, 'tempI64[0]', 'i32') + ',' +
            makeSetValueImpl(ptr, getFastValue(pos, '+', getNativeTypeSize('i32')), 'tempI64[1]', 'i32') + ')';
  }

  const offset = calcFastOffset(ptr, pos);

  if (type === 'i53') {
    return `writeI53ToI64(${offset}, ${value})`;
  }

  const slab = getHeapForType(type);
  if (slab == 'HEAPU64' || slab == 'HEAP64') {
    value = `BigInt(${value})`;
  }
  return slab + '[' + getHeapOffset(offset, type) + '] = ' + value;
}

function makeHEAPView(which, start, end) {
  const size = parseInt(which.replace('U', '').replace('F', '')) / 8;
  const mod = size == 1 ? '' : ('>>' + Math.log2(size));
  return `HEAP${which}.subarray((${start})${mod}, (${end})${mod})`;
}

// Given two values and an operation, returns the result of that operation.
// Tries to do as much as possible at compile time.
function getFastValue(a, op, b) {
  // In the past we supported many operations, but today we only use addition.
  assert(op == '+');

  // Convert 'true' and 'false' to '1' and '0'.
  a = a === 'true' ? '1' : (a === 'false' ? '0' : a);
  b = b === 'true' ? '1' : (b === 'false' ? '0' : b);

  let aNumber = null;
  let bNumber = null;
  if (typeof a == 'number') {
    aNumber = a;
    a = a.toString();
  } else if (isNumber(a)) {
    aNumber = parseFloat(a);
  }
  if (typeof b == 'number') {
    bNumber = b;
    b = b.toString();
  } else if (isNumber(b)) {
    bNumber = parseFloat(b);
  }

  // First check if we can do the addition at compile time
  if (aNumber !== null && bNumber !== null) {
    return (aNumber + bNumber).toString();
  }

  // If one of them is a number, keep it last
  if (aNumber !== null) {
    const c = b;
    b = a;
    a = c;
    const cNumber = bNumber;
    bNumber = aNumber;
    aNumber = cNumber;
  }

  if (aNumber === 0) {
    return b;
  } else if (bNumber === 0) {
    return a;
  }

  if (b[0] === '-') {
    op = '-'
    b = b.substr(1);
  }

  return `(${a})${op}(${b})`;
}

function calcFastOffset(ptr, pos) {
  return getFastValue(ptr, '+', pos);
}

function getBitWidth(type) {
  if (type == 'i53' || type == 'u53')
    return 53;
  return getNativeTypeSize(type) * 8;
}

function getHeapForType(type) {
  assert(type);
  if (isPointerType(type)) {
    type = POINTER_TYPE;
  }
  if (WASM_BIGINT) {
    switch (type) {
      case 'i64': return 'HEAP64';
      case 'u64': return 'HEAPU64';
    }
  }
  switch (type) {
    case 'i1':     // fallthrough
    case 'i8':     return 'HEAP8';
    case 'u8':     return 'HEAPU8';
    case 'i16':    return 'HEAP16';
    case 'u16':    return 'HEAPU16';
    case 'i32':    return 'HEAP32';
    case 'u32':    return 'HEAPU32';
    case 'double': return 'HEAPF64';
    case 'float':  return 'HEAPF32';
    case 'i64':    // fallthrough
    case 'u64':    error('use i53/u53, or avoid i64/u64 without WASM_BIGINT');
  }
  assert(false, 'bad heap type: ' + type);
}

function makeReturn64(value) {
  if (WASM_BIGINT) {
    return `BigInt(${value})`;
  }
  const pair = splitI64(value);
  // `return (a, b, c)` in JavaScript will execute `a`, and `b` and return the final
  // element `c`
  return `(setTempRet0(${pair[1]}), ${pair[0]})`;
}

function makeThrow(excPtr) {
  if (ASSERTIONS && DISABLE_EXCEPTION_CATCHING) {
    var assertInfo = 'Exception thrown, but exception catching is not enabled. Compile with -sNO_DISABLE_EXCEPTION_CATCHING or -sEXCEPTION_CATCHING_ALLOWED=[..] to catch.';
    if (MAIN_MODULE) {
      assertInfo += ' (note: in dynamic linking, if a side module wants exceptions, the main module must be built with that support)';
    }
    return `assert(false, '${assertInfo}');`;
  }
  return `throw ${excPtr};`;
}

function storeException(varName, excPtr) {
  var exceptionToStore = EXCEPTION_STACK_TRACES ? `new CppException(${excPtr})` : `${excPtr}`;
  return `${varName} = ${exceptionToStore};`;
}

function charCode(char) {
  return char.charCodeAt(0);
}

function ensureValidFFIType(type) {
  return type === 'float' ? 'double' : type; // ffi does not tolerate float XXX
}

// FFI return values must arrive as doubles, and we can force them to floats afterwards
function asmFFICoercion(value, type) {
  value = asmCoercion(value, ensureValidFFIType(type));
  if (type === 'float') value = asmCoercion(value, 'float');
  return value;
}

function makeDynCall(sig, funcPtr) {
  assert(!sig.includes('j'), 'Cannot specify 64-bit signatures ("j" in signature string) with makeDynCall!');

  let args = [];
  for (let i = 1; i < sig.length; ++i) {
    args.push(`a${i}`);
  }
  args = args.join(', ');

  const needArgConversion = MEMORY64 && sig.includes('p');
  let callArgs = args;
  if (needArgConversion) {
    callArgs = [];
    for (let i = 1; i < sig.length; ++i) {
      if (sig[i] == 'p') {
        callArgs.push(`BigInt(a${i})`);
      } else {
        callArgs.push(`a${i}`);
      }
    }
    callArgs = callArgs.join(', ');
  }

  // Normalize any 'p' characters to either 'j' (wasm64) or 'i' (wasm32)
  if (sig.includes('p')) {
    let normalizedSig = '';
    for (let sigChr of sig) {
      if (sigChr == 'p') {
        sigChr = MEMORY64 ? 'j' : 'i';
      }
      normalizedSig += sigChr;
    }
    sig = normalizedSig;
  }


  if (funcPtr === undefined) {
    warn(`
Legacy use of {{{ makeDynCall("${sig}") }}}(funcPtr, arg1, arg2, ...). \
Starting from Emscripten 2.0.2 (Aug 31st 2020), syntax for makeDynCall has changed. \
New syntax is {{{ makeDynCall("${sig}", "funcPtr") }}}(arg1, arg2, ...). \
Please update to new syntax.`);

    if (DYNCALLS) {
      if (!hasExportedSymbol(`dynCall_${sig}`)) {
        if (ASSERTIONS) {
          return `((${args}) => { throw 'Internal Error! Attempted to invoke wasm function pointer with signature "${sig}", but no such functions have gotten exported!' })`;
        } else {
          return `((${args}) => {} /* a dynamic function call to signature ${sig}, but there are no exported function pointers with that signature, so this path should never be taken. Build with ASSERTIONS enabled to validate. */)`;
        }
      }
      return `((cb, ${args}) => getDynCaller("${sig}", cb)(${callArgs}))`;
    } else {
      return `((cb, ${args}) => getWasmTableEntry(cb)(${callArgs}))`;
    }
  }

  if (DYNCALLS) {
    if (!hasExportedSymbol(`dynCall_${sig}`)) {
      if (ASSERTIONS) {
        return `((${args}) => { throw 'Internal Error! Attempted to invoke wasm function pointer with signature "${sig}", but no such functions have gotten exported!' })`;
      } else {
        return `((${args}) => {} /* a dynamic function call to signature ${sig}, but there are no exported function pointers with that signature, so this path should never be taken. Build with ASSERTIONS enabled to validate. */)`;
      }
    }

    const dyncall = `dynCall_${sig}`;
    if (sig.length > 1) {
      return `((${args}) => ${dyncall}.apply(null, [${funcPtr}, ${callArgs}]))`;
    }
    return `(() => ${dyncall}.call(null, ${funcPtr}))`;
  }

  if (needArgConversion) {
    return `((${args}) => getWasmTableEntry(${funcPtr}).call(null, ${callArgs}))`;
  }
  return `getWasmTableEntry(${funcPtr})`;
}

function heapAndOffset(heap, ptr) { // given   HEAP8, ptr   , we return    splitChunk, relptr
  return heap + ',' + ptr;
}

function makeEval(code) {
  if (DYNAMIC_EXECUTION == 0) {
    // Treat eval as error.
    return "abort('DYNAMIC_EXECUTION=0 was set, cannot eval');";
  }
  let ret = '';
  if (DYNAMIC_EXECUTION == 2) {
    // Warn on evals, but proceed.
    ret += "err('Warning: DYNAMIC_EXECUTION=2 was set, but calling eval in the following location:');\n";
    ret += 'err(stackTrace());\n';
  }
  ret += code;
  return ret;
}

global.ATINITS = [];

function addAtInit(code) {
  ATINITS.push(code);
}

global.ATEXITS = [];

function addAtExit(code) {
  if (EXIT_RUNTIME) {
    ATEXITS.push(code);
  }
}

function makeRetainedCompilerSettings() {
  const ignore = new Set();
  if (STRICT) {
    for (const setting of LEGACY_SETTINGS) {
      ignore.add(setting);
    }
  }

  const ret = {};
  for (const x in global) {
    if (!ignore.has(x) && x[0] !== '_' && x == x.toUpperCase()) {
      const value = global[x];
      if (typeof value == 'number' || typeof value == 'boolean' || typeof value == 'string' || Array.isArray(x)) {
        ret[x] = value;
      }
    }
  }
  return ret;
}

// In wasm, the heap size must be a multiple of 64KiB.
const WASM_PAGE_SIZE = 65536;

// Receives a function as text, and a function that constructs a modified
// function, to which we pass the parsed-out arguments, body, and possible
// "async" prefix of the input function. Returns the output of that function.
function modifyJSFunction(text, func) {
  // Match a function with a name.
  let async_;
  let args;
  let rest;
  let match = text.match(/^\s*(async\s+)?function\s+([^(]*)?\s*\(([^)]*)\)/);
  if (match) {
    async_ = match[1] || '';
    args = match[3];
    rest = text.substr(match[0].length);
  } else {
    // Match an arrow function
    let match = text.match(/^\s*(var (\w+) = )?(async\s+)?\(([^)]*)\)\s+=>\s+/);
    if (match) {
      async_ = match[3] || '';
      args = match[4];
      rest = text.substr(match[0].length);
    } else {
      // Match a function without a name (we could probably use a single regex
      // for both, but it would be more complex).
      match = text.match(/^\s*(async\s+)?function\(([^)]*)\)/);
      assert(match, `could not match function:\n${text}\n`);
      async_ = match[1] || '';
      args = match[2];
      rest = text.substr(match[0].length);
    }
  }
  let body = rest;
  const bodyStart = rest.indexOf('{');
  let oneliner = bodyStart < 0;
  if (!oneliner) {
    const bodyEnd = rest.lastIndexOf('}');
    assert(bodyEnd > 0);
    body = rest.substring(bodyStart + 1, bodyEnd);
  }
  return func(args, body, async_, oneliner);
}

function runIfMainThread(text) {
  if (WASM_WORKERS && PTHREADS) {
    return 'if (!ENVIRONMENT_IS_WASM_WORKER && !ENVIRONMENT_IS_PTHREAD) { ' + text + ' }';
  } else if (WASM_WORKERS) {
    return 'if (!ENVIRONMENT_IS_WASM_WORKER) { ' + text + ' }';
  } else if (PTHREADS) {
    return 'if (!ENVIRONMENT_IS_PTHREAD) { ' + text + ' }';
  } else {
    return text;
  }
}

// Legacy name for runIfMainThread.
// TODO(remove).
const runOnMainThread = runIfMainThread;

function expectToReceiveOnModule(name) {
  return INCOMING_MODULE_JS_API.has(name);
}

function makeRemovedModuleAPIAssert(moduleName, localName) {
  if (!ASSERTIONS) return '';
  if (!localName) localName = moduleName;
  return `legacyModuleProp('${moduleName}', '${localName}');`;
}

function checkReceiving(name) {
  // ALL_INCOMING_MODULE_JS_API contains all valid incoming module API symbols
  // so calling makeModuleReceive* with a symbol not in this list is an error
  assert(ALL_INCOMING_MODULE_JS_API.has(name), `${name} is not part of INCOMING_MODULE_JS_API`);
}

// Make code to receive a value on the incoming Module object.
function makeModuleReceive(localName, moduleName) {
  if (!moduleName) moduleName = localName;
  checkReceiving(moduleName);
  let ret = '';
  if (expectToReceiveOnModule(moduleName)) {
    // Usually the local we use is the same as the Module property name,
    // but sometimes they must differ.
    ret = `\nif (Module['${moduleName}']) ${localName} = Module['${moduleName}'];`;
  }
  ret += makeRemovedModuleAPIAssert(moduleName, localName);
  return ret;
}

function makeModuleReceiveExpr(name, defaultValue) {
  checkReceiving(name);
  if (expectToReceiveOnModule(name)) {
    return `Module['${name}'] || ${defaultValue}`;
  } else {
    return `${defaultValue}`;
  }
}

function makeModuleReceiveWithVar(localName, moduleName, defaultValue, noAssert) {
  if (!moduleName) moduleName = localName;
  checkReceiving(moduleName);
  let ret = 'var ' + localName;
  if (!expectToReceiveOnModule(moduleName)) {
    if (defaultValue) {
      ret += ' = ' + defaultValue;
    }
    ret += ';';
  } else {
    if (defaultValue) {
      ret += ` = Module['${moduleName}'] || ${defaultValue};`;
    } else {
      ret += ';' + makeModuleReceive(localName, moduleName);
      return ret;
    }
  }
  if (!noAssert) {
    ret += makeRemovedModuleAPIAssert(moduleName, localName);
  }
  return ret;
}

function makeRemovedFSAssert(fsName) {
  assert(ASSERTIONS);
  const lower = fsName.toLowerCase();
  if (JS_LIBRARIES.includes('library_' + lower + '.js')) return '';
  return `var ${fsName} = '${fsName} is no longer included by default; build with -l${lower}.js';`;
}

// Given an array of elements [elem1,elem2,elem3], returns a string "['elem1','elem2','elem3']"
function buildStringArray(array) {
  if (array.length > 0) {
    return "['" + array.join("','") + "']";
  } else {
    return '[]';
  }
}

// Generates access to a JS imports scope variable in pthreads worker.js. In MODULARIZE mode these flow into the imports object for the Module.
// In non-MODULARIZE mode, we can directly access the variables in global scope.
function makeAsmImportsAccessInPthread(variable) {
  if (!MINIMAL_RUNTIME) {
    // Regular runtime uses the name "Module" for both imports and exports.
    return `Module['${variable}']`;
  }
  if (MODULARIZE) {
    // MINIMAL_RUNTIME uses 'imports' as the name for the imports object in MODULARIZE builds.
    return `imports['${variable}']`;
  }
  // In non-MODULARIZE builds, can access the imports from global scope.
  return `self.${variable}`;
}

function _asmjsDemangle(symbol) {
  if (symbol in WASM_SYSTEM_EXPORTS) {
    return symbol;
  }
  if (symbol.startsWith('dynCall_')) {
    return symbol;
  }
  // Strip leading "_"
  assert(symbol.startsWith('_'), 'expected mangled symbol: ' + symbol);
  return symbol.substr(1);
}

// TODO(sbc): Remove this function along with _asmjsDemangle.
function hasExportedFunction(func) {
  warnOnce('hasExportedFunction has been replaced with hasExportedSymbol, which takes and unmangled (no leading underscore) symbol name');
  return WASM_EXPORTS.has(_asmjsDemangle(func));
}

function hasExportedSymbol(sym) {
  return WASM_EXPORTS.has(sym);
}

// Called when global runtime symbols such as wasmMemory, wasmExports and
// wasmTable are set. In this case we maybe need to re-export them on the
// Module object.
function receivedSymbol(sym) {
  if (EXPORTED_RUNTIME_METHODS.includes(sym)) {
    return `Module['${sym}'] = ${sym};`
  }
  return '';
}

// JS API I64 param handling: if we have BigInt support, the ABI is simple,
// it is a BigInt. Otherwise, we legalize into pairs of i32s.
function defineI64Param(name) {
  if (WASM_BIGINT) {
    return name;
  }
  return `${name}_low, ${name}_high`;
}


function receiveI64ParamAsI53(name, onError, handleErrors = true) {
  var errorHandler = handleErrors ? `if (isNaN(${name})) return ${onError}` : '';
  if (WASM_BIGINT) {
    // Just convert the bigint into a double.
    return `${name} = bigintToI53Checked(${name});${errorHandler};`;
  }
  // Convert the high/low pair to a Number, checking for
  // overflow of the I53 range and returning onError in that case.
  return `var ${name} = convertI32PairToI53Checked(${name}_low, ${name}_high);${errorHandler};`;
}

function receiveI64ParamAsI53Unchecked(name) {
  if (WASM_BIGINT) {
    return `${name} = Number(${name});`;
  }
  return `var ${name} = convertI32PairToI53(${name}_low, ${name}_high);`;
}

// Any function called from wasm64 may have bigint args, this function takes
// a list of variable names to convert to number.
function from64(x) {
  if (!MEMORY64) {
    return '';
  }
  if (Array.isArray(x)) {
    let ret = '';
    for (e of x) ret += from64(e);
    return ret;
  }
  return `${x} = Number(${x});`;
}

function to64(x) {
  if (!MEMORY64) return x;
  return `BigInt(${x})`;
}

// Add assertions to catch common errors when using the Promise object we
// create on Module.ready() and return from MODULARIZE Module() invocations.
function addReadyPromiseAssertions(promise) {
  // Warn on someone doing
  //
  //  var instance = Module();
  //  ...
  //  instance._main();
  const properties = Array.from(EXPORTED_FUNCTIONS.values());
  // Also warn on onRuntimeInitialized which might be a common pattern with
  // older MODULARIZE-using codebases.
  properties.push('onRuntimeInitialized');
  const warningEnding = ' on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js';
  const res = JSON.stringify(properties);
  return res + `.forEach((prop) => {
  if (!Object.getOwnPropertyDescriptor(${promise}, prop)) {
    Object.defineProperty(${promise}, prop, {
      get: () => abort('You are getting ' + prop + '${warningEnding}'),
      set: () => abort('You are setting ' + prop + '${warningEnding}'),
    });
  }
});`;
}

function asyncIf(condition) {
  return condition ? 'async' : '';
}

function awaitIf(condition) {
  return condition ? 'await' : '';
}

// Adds a call to runtimeKeepalivePush, if needed by the current build
// configuration.
// We skip this completely in MINIMAL_RUNTIME and also in builds that
// don't ever need to exit the runtime.
function runtimeKeepalivePush() {
  if (MINIMAL_RUNTIME || (EXIT_RUNTIME == 0 && PTHREADS == 0)) return '';
  return 'runtimeKeepalivePush();';
}

// Adds a call to runtimeKeepalivePush, if needed by the current build
// configuration.
// We skip this completely in MINIMAL_RUNTIME and also in builds that
// don't ever need to exit the runtime.
function runtimeKeepalivePop() {
  if (MINIMAL_RUNTIME || (EXIT_RUNTIME == 0 && PTHREADS == 0)) return '';
  return 'runtimeKeepalivePop();';
}

// Some web functions like TextDecoder.decode() may not work with a view of a
// SharedArrayBuffer, see https://github.com/whatwg/encoding/issues/172
// To avoid that, this function allows obtaining an unshared copy of an
// ArrayBuffer.
function getUnsharedTextDecoderView(heap, start, end) {
  const shared = `${heap}.slice(${start}, ${end})`;
  const unshared = `${heap}.subarray(${start}, ${end})`;

  // No need to worry about this in non-shared memory builds
  if (!SHARED_MEMORY) return unshared;

  // If asked to get an unshared view to what we know will be a shared view, or if in -Oz,
  // then unconditionally do a .slice() for smallest code size.
  if (SHRINK_LEVEL == 2 || heap == 'HEAPU8') return shared;

  // Otherwise, generate a runtime type check: must do a .slice() if looking at a SAB,
  // or can use .subarray() otherwise.
  return `${heap}.buffer instanceof SharedArrayBuffer ? ${shared} : ${unshared}`;
}

function getEntryFunction() {
  var entryFunction = 'main';
  if (STANDALONE_WASM) {
    if (EXPECT_MAIN) {
      entryFunction = '_start';
    } else {
      entryFunction = '_initialize';
    }
  } else if (PROXY_TO_PTHREAD) {
    // User requested the PROXY_TO_PTHREAD option, so call a stub main which pthread_create()s a new thread
    // that will call the user's real main() for the application.
    entryFunction = '_emscripten_proxy_main';
  }
  if (MAIN_MODULE) {
    return `resolveGlobalSymbol('${entryFunction}').sym;`
  }
  return '_' + entryFunction;
}

function preJS() {
  let result = '';
  for (const fileName of PRE_JS_FILES) {
    result += read(fileName);
  }
  return result;
}

function formattedMinNodeVersion() {
  var major = MIN_NODE_VERSION / 10000
  var minor = (MIN_NODE_VERSION / 100) % 100
  var rev = MIN_NODE_VERSION % 100
  return `v${major}.${minor}.${rev}`;
}

function getPerformanceNow() {
  if (DETERMINISTIC) {
    return 'deterministicNow';
  } else {
    return 'performance.now';
  }
}
