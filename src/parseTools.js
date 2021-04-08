/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Various tools for parsing LLVM. Utilities of various sorts, that are
// specific to Emscripten (and hence not in utility.js).

let currentlyParsedFilename = '';

// Does simple 'macro' substitution, using Django-like syntax,
// {{{ code }}} will be replaced with |eval(code)|.
// NOTE: Be careful with that ret check. If ret is |0|, |ret ? ret.toString() : ''| would result in ''!
function processMacros(text) {
  return text.replace(/{{{([^}]|}(?!}))+}}}/g, (str) => {
    str = str.substr(3, str.length - 6);
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
// Param filenameHint can be passed as a description to identify the file that is being processed, used
// to locate errors for reporting and for html files to stop expansion between <style> and </style>.
function preprocess(text, filenameHint) {
  currentlyParsedFilename = filenameHint;
  try {
    const fileExt = (filenameHint) ? filenameHint.split('.').pop().toLowerCase() : '';
    const isHtml = (fileExt === 'html' || fileExt === 'htm') ? true : false;
    let inStyle = false;
    const lines = text.split('\n');
    let ret = '';
    const showStack = [];
    let emptyLine = false;
    for (let i = 0; i < lines.length; i++) {
      let line = lines[i];
      try {
        if (line[line.length - 1] === '\r') {
          line = line.substr(0, line.length - 1); // Windows will have '\r' left over from splitting over '\r\n'
        }
        if (isHtml && line.indexOf('<style') !== -1 && !inStyle) {
          inStyle = true;
        }
        if (isHtml && line.indexOf('</style') !== -1 && inStyle) {
          inStyle = false;
        }

        if (!inStyle) {
          const trimmed = line.trim();
          if (trimmed[0] === '#') {
            const first = trimmed.split(' ', 1)[0];
            if (first == '#if' || first == '#ifdef') {
              if (first == '#ifdef') {
                warn('warning: use of #ifdef in js library.  Use #if instead.');
              }
              const after = trimmed.substring(trimmed.indexOf(' '));
              const truthy = !!eval(after);
              showStack.push(truthy);
            } else if (first === '#include') {
              if (showStack.indexOf(false) === -1) {
                let filename = line.substr(line.indexOf(' ') + 1);
                if (filename.indexOf('"') === 0) {
                  filename = filename.substr(1, filename.length - 2);
                }
                const included = read(filename);
                const result = preprocess(included, filename);
                if (result) {
                  ret += `// include: ${filename}\n`;
                  ret += result;
                  ret += `// end include: ${filename}\n`;
                }
              }
            } else if (first === '#else') {
              assert(showStack.length > 0);
              showStack.push(!showStack.pop());
            } else if (first === '#endif') {
              assert(showStack.length > 0);
              showStack.pop();
            } else {
              throw new Error(`Unknown preprocessor directive on line ${i}: ``${line}```);
            }
          } else {
            if (showStack.indexOf(false) === -1) {
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
        } else { // !inStyle
          if (showStack.indexOf(false) === -1) {
            ret += line + '\n';
          }
        }
      } catch (e) {
        printErr('parseTools.js preprocessor error in ' + filenameHint + ':' + (i + 1) + ': \"' + line + '\"!');
        throw e;
      }
    }
    assert(showStack.length == 0, `preprocessing error in file ${filenameHint}, \
no matching #endif found (${showStack.length$}' unmatched preprocessing directives on stack)`);
    return ret;
  } finally {
    currentlyParsedFilename = null;
  }
}

function removePointing(type, num) {
  if (num === 0) return type;
  assert(type.substr(type.length - (num ? num : 1)).replace(/\*/g, '') === '');
  // , 'Error in removePointing with ' + [type, num, type.substr(type.length-(num ? num : 1))]);
  return type.substr(0, type.length - (num ? num : 1));
}

function pointingLevels(type) {
  if (!type) return 0;
  let ret = 0;
  const len1 = type.length - 1;
  while (type[len1 - ret] && type[len1 - ret] === '*') {
    ret++;
  }
  return ret;
}

function removeAllPointing(type) {
  return removePointing(type, pointingLevels(type));
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

function isStructPointerType(type) {
  // This test is necessary for clang - in llvm-gcc, we
  // could check for %struct. The downside is that %1 can
  // be either a variable or a structure, and we guess it is
  // a struct, which can lead to |call i32 %5()| having
  // |%5()| as a function call (like |i32 (i8*)| etc.). So
  // we must check later on, in call(), where we have more
  // context, to differentiate such cases.
  // A similar thing happens in isStructType()
  return !Compiletime.isNumberType(type) && type[0] == '%';
}

function isPointerType(type) {
  return type[type.length - 1] == '*';
}

function isArrayType(type) {
  return /^\[\d+\ x\ (.*)\]/.test(type);
}

function isStructType(type) {
  if (isPointerType(type)) return false;
  if (isArrayType(type)) return true;
  if (/<?\{ ?[^}]* ?\}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types
  // See comment in isStructPointerType()
  return type[0] == '%';
}

function isVectorType(type) {
  return type[type.length - 1] === '>';
}

function isStructuralType(type) {
  return /^\{ ?[^}]* ?\}$/.test(type); // { i32, i8 } etc. - anonymous struct types
}

function getStructuralTypeParts(type) { // split { i32, i8 } etc. into parts
  return type.replace(/[ {}]/g, '').split(',');
}

function getStructuralTypePartBits(part) {
  return Math.ceil((getBits(part) || 32) / 32) * 32; // simple 32-bit alignment. || 32 is for pointers
}

function isIntImplemented(type) {
  return type[0] == 'i' || isPointerType(type);
}

// Note: works for iX types and structure types, not pointers (even though they are implemented as ints)
function getBits(type, allowPointers) {
  if (allowPointers && isPointerType(type)) return 32;
  if (!type) return 0;
  if (type[0] == 'i') {
    const left = type.substr(1);
    if (!isNumber(left)) return 0;
    return parseInt(left);
  }
  if (isStructuralType(type)) {
    return sum(getStructuralTypeParts(type).map(getStructuralTypePartBits));
  }
  if (isStructType(type)) {
    const typeData = Types.types[type];
    if (typeData === undefined) return 0;
    return typeData.flatSize * 8;
  }
  return 0;
}

function isVoidType(type) {
  return type == 'void';
}

// Detects a function definition, ([...|type,[type,...]])
function isFunctionDef(token, out) {
  const text = token.text;
  const nonPointing = removeAllPointing(text);
  if (nonPointing[0] != '(' || nonPointing.substr(-1) != ')') {
    return false;
  }
  if (nonPointing === '()') return true;
  if (!token.tokens) return false;
  let fail = false;
  const segments = splitTokenList(token.tokens);
  segments.forEach((segment) => {
    const subtext = segment[0].text;
    fail = fail || segment.length > 1 || !(isType(subtext) || subtext == '...');
  });
  if (out) {
    out.segments = segments;
    out.numArgs = segments.length;
  }
  return !fail;
}

function isPossiblyFunctionType(type) {
  // A quick but unreliable way to see if something is a function type. Yes is just 'maybe', no is definite.
  const len = type.length;
  return type[len - 2] == ')' && type[len - 1] == '*';
}

function isFunctionType(type, out) {
  if (!isPossiblyFunctionType(type)) return false;
  type = type.substr(0, type.length - 1); // remove final '*'
  const firstOpen = type.indexOf('(');
  if (firstOpen <= 0) return false;
  type = type.replace(/"[^"]+"/g, '".."');
  const lastOpen = type.lastIndexOf('(');
  let returnType;
  if (firstOpen == lastOpen) {
    returnType = getReturnType(type);
    if (!isType(returnType)) return false;
  } else {
    returnType = 'i8*'; // some pointer type, no point in analyzing further
  }
  if (out) out.returnType = returnType;
  // find ( that starts the arguments
  let depth = 0;
  let i = type.length - 1;
  let argText = null;
  while (i >= 0) {
    const curr = type[i];
    if (curr == ')') depth++;
    else if (curr == '(') {
      depth--;
      if (depth == 0) {
        argText = type.substr(i);
        break;
      }
    }
    i--;
  }
  assert(argText);
  return isFunctionDef({text: argText, tokens: tokenize(argText.substr(1, argText.length - 2))}, out);
}

function getReturnType(type) {
  // the type of a call can be either the return value, or the entire function. ** or more means it is a return value
  if (pointingLevels(type) > 1) return '*';
  const lastOpen = type.lastIndexOf('(');
  if (lastOpen > 0) {
    // handle things like   void (i32)* (i32, void (i32)*)*
    const closeStar = type.indexOf(')*');
    if (closeStar > 0 && closeStar < type.length - 2) lastOpen = closeStar + 3;
    return type.substr(0, lastOpen - 1);
  }
  return type;
}

const isTypeCache = {}; // quite hot, optimize as much as possible

function isType(type) {
  if (type in isTypeCache) return isTypeCache[type];
  const ret = isPointerType(type) || isVoidType(type) || Compiletime.isNumberType(type) || isStructType(type) || isFunctionType(type);
  isTypeCache[type] = ret;
  return ret;
}

const SPLIT_TOKEN_LIST_SPLITTERS = set(',', 'to'); // 'to' can separate parameters as well...

// Splits a list of tokens separated by commas. For example, a list of arguments in a function call
function splitTokenList(tokens) {
  if (tokens.length == 0) return [];
  if (!tokens.slice) tokens = tokens.tokens;
  const ret = [];
  const seg = [];
  for (let i = 0; i < tokens.length; i++) {
    const token = tokens[i];
    if (token.text in SPLIT_TOKEN_LIST_SPLITTERS) {
      ret.push(seg);
      seg = [];
    } else if (token.text == ';') {
      ret.push(seg);
      return ret;
    } else {
      seg.push(token);
    }
  }
  if (seg.length) ret.push(seg);
  return ret;
}

function _IntToHex(x) {
  assert(x >= 0 && x <= 15);
  if (x <= 9) {
    return String.fromCharCode('0'.charCodeAt(0) + x);
  }
  return String.fromCharCode('A'.charCodeAt(0) + x - 10);
}

function IEEEUnHex(stringy) {
  stringy = stringy.substr(2); // leading '0x';
  if (stringy.replace(/0/g, '') === '') return 0;
  while (stringy.length < 16) stringy = '0' + stringy;
  assert(stringy.length === 16, 'Can only unhex 16-digit double numbers, nothing platform-specific'); // |long double| might cause this
  const top = eval('0x' + stringy[0]);
  const neg = !!(top & 8); // sign
  if (neg) {
    stringy = _IntToHex(top & ~8) + stringy.substr(1);
  }
  let a = eval('0x' + stringy.substr(0, 8)); // top half
  const b = eval('0x' + stringy.substr(8)); // bottom half
  let e = a >> ((52 - 32) & 0x7ff); // exponent
  a = a & 0xfffff;
  if (e === 0x7ff) {
    if (a == 0 && b == 0) {
      return neg ? '-Infinity' : 'Infinity';
    }
    return 'NaN';
  }
  e -= 1023; // offset
  const absolute = ((((a | 0x100000) * 1.0) / Math.pow(2, 52 - 32)) * Math.pow(2, e)) + (((b * 1.0) / Math.pow(2, 52)) * Math.pow(2, e));
  return (absolute * (neg ? -1 : 1)).toString();
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

// Makes a proper runtime value for a 64-bit value from low and high i32s. low and high are assumed to be unsigned.
function makeI64(low, high) {
  high = high || '0';
  return '[' + makeSignOp(low, 'i32', 'un', 1, 1) + ',' + makeSignOp(high, 'i32', 'un', 1, 1) + ']';
}

// XXX Make all i64 parts signed

// Splits a number (an integer in a double, possibly > 32 bits) into an i64 value, represented by a low and high i32 pair.
// Will suffer from rounding.
function splitI64(value, floatConversion) {
  // general idea:
  //
  //  $1$0 = ~~$d >>> 0;
  //  $1$1 = Math.abs($d) >= 1 ? (
  //     $d > 0 ? Math.min(Math.floor(($d)/ 4294967296.0), 4294967295.0)
  //            : Math.ceil(Math.min(-4294967296.0, $d - $1$0)/ 4294967296.0)
  //  ) : 0;
  //
  // We need to min on positive values here, since our input might be a double, and large values are rounded, so they can
  // be slightly higher than expected. And if we get 4294967296, that will turn into a 0 if put into a
  // HEAP32 or |0'd, etc.
  //
  // For negatives, we need to ensure a -1 if the value is overall negative, even if not significant negative component

  const lowInput = legalizedI64s ? value : 'VALUE';
  if (floatConversion) lowInput = asmFloatToInt(lowInput);
  const low = lowInput + '>>>0';
  const high = makeInlineCalculation(
      asmCoercion('Math.abs(VALUE)', 'double') + ' >= ' + asmEnsureFloat('1', 'double') + ' ? ' +
        '(VALUE > ' + asmEnsureFloat('0', 'double') + ' ? ' +
        asmCoercion('Math.min(' + asmCoercion('Math.floor((VALUE)/' +
        asmEnsureFloat(4294967296, 'double') + ')', 'double') + ', ' +
        asmEnsureFloat(4294967295, 'double') + ')', 'i32') + '>>>0' +
        ' : ' +
        asmFloatToInt(asmCoercion('Math.ceil((VALUE - +((' + asmFloatToInt('VALUE') + ')>>>0))/' +
        asmEnsureFloat(4294967296, 'double') + ')', 'double')) + '>>>0' +
        ')' +
      ' : 0',
      value,
      'tempDouble',
  );
  if (legalizedI64s) {
    return [low, high];
  }
  return makeI64(low, high);
}

// Misc

function indentify(text, indent) {
  // Don't try to indentify huge strings - we may run out of memory
  if (text.length > 1024 * 1024) return text;
  if (typeof indent === 'number') {
    const len = indent;
    indent = '';
    for (let i = 0; i < len; i++) {
      indent += ' ';
    }
  }
  return text.replace(/\n/g, '\n' + indent);
}

// Correction tools

function checkSafeHeap() {
  return SAFE_HEAP === 1;
}

function getHeapOffset(offset, type) {
  if (Runtime.getNativeFieldSize(type) > 4 && type == 'i64') {
    // we emulate 64-bit integer values as 32 in asmjs-unknown-emscripten, but not double
    type = 'i32';
  }

  const sz = Runtime.getNativeTypeSize(type);
  const shifts = Math.log(sz) / Math.LN2;
  return `((${offset})>>${shifts})`;
}

function ensureDot(value) {
  value = value.toString();
  // if already dotted, or Infinity or NaN, nothing to do here
  // if smaller than 1 and running js opts, we always need to force a coercion
  // (0.001 will turn into 1e-3, which has no .)
  if ((value.indexOf('.') >= 0 || /[IN]/.test(value))) return value;
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
  if (type in Compiletime.FLOAT_TYPES) {
    return ensureDot(value);
  }
  return value;
}

function asmCoercion(value, type, signedness) {
  if (type == 'void') {
    return value;
  } else if (type in Compiletime.FLOAT_TYPES) {
    if (isNumber(value)) {
      return asmEnsureFloat(value, type);
    } else {
      if (signedness) {
        if (signedness == 'u') {
          value = '(' + value + ')>>>0';
        } else {
          value = '(' + value + ')|0';
        }
      }
      if (type === 'float') {
        return 'Math.fround(' + value + ')';
      } else {
        return '(+(' + value + '))';
      }
    }
  } else {
    if (signedness == 'u') {
      return '((' + value + ')>>>0)';
    }
    return '((' + value + ')|0)';
  }
}

function asmFloatToInt(x) {
  return '(~~(' + x + '))';
}

function makeGetTempDouble(i, type, forSet) { // get an aliased part of the tempDouble temporary storage
  // Cannot use makeGetValue because it uses us
  // this is a unique case where we *can* use HEAPF64
  const heap = getHeapForType(type);
  const ptr = getFastValue('tempDoublePtr', '+', Runtime.getNativeTypeSize(type) * i);
  let offset;
  if (type == 'double') {
    offset = '(' + ptr + ')>>3';
  } else {
    offset = getHeapOffset(ptr, type);
  }
  let ret = heap + '[' + offset + ']';
  if (!forSet) ret = asmCoercion(ret, type);
  return ret;
}

function makeSetTempDouble(i, type, value) {
  return makeGetTempDouble(i, type, true) + '=' + asmEnsureFloat(value, type);
}

// See makeSetValue
function makeGetValue(ptr, pos, type, noNeedFirst, unsigned, ignore, align, noSafe, forceAsm) {
  assert(!forceAsm, 'forceAsm is no longer supported');
  if (isStructType(type)) {
    const typeData = Types.types[type];
    const ret = [];
    for (let i = 0; i < typeData.fields.length; i++) {
      ret.push('f' + i + ': ' + makeGetValue(ptr, pos + typeData.flatIndexes[i], typeData.fields[i], noNeedFirst, unsigned, 0, 0, noSafe));
    }
    return '{ ' + ret.join(', ') + ' }';
  }

  if (type == 'double' && (align < 8)) {
    const setdouble1 = makeSetTempDouble(0, 'i32', makeGetValue(ptr, pos, 'i32', noNeedFirst, unsigned, ignore, align, noSafe));
    const setdouble2 = makeSetTempDouble(1, 'i32', makeGetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'i32', noNeedFirst, unsigned, ignore, align, noSafe));
    return '(' + setdouble1 + ',' + setdouble2 + ',' + makeGetTempDouble(0, 'double') + ')';
  }

  if (align) {
    // Alignment is important here. May need to split this up
    const bytes = Runtime.getNativeTypeSize(type);
    if (bytes > align) {
      let ret = '(';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += makeGetValue(ptr, pos, 'i16', noNeedFirst, 2, ignore, 2, noSafe) + '|' +
                 '(' + makeGetValue(ptr, getFastValue(pos, '+', 2), 'i16', noNeedFirst, 2, ignore, 2, noSafe) + '<<16)';
        } else { // XXX we cannot truly handle > 4... (in x86)
          ret = '';
          for (let i = 0; i < bytes; i++) {
            ret += '(' + makeGetValue(ptr, getFastValue(pos, '+', i), 'i8', noNeedFirst, 1, ignore, 1, noSafe) + (i > 0 ? '<<' + (8 * i) : '') + ')';
            if (i < bytes - 1) ret += '|';
          }
          ret = '(' + makeSignOp(ret, type, unsigned ? 'un' : 're', true);
        }
      } else {
        if (type == 'float') {
          ret += 'copyTempFloat(' + asmCoercion(getFastValue(ptr, '+', pos), 'i32') + '),' + makeGetTempDouble(0, 'float');
        } else {
          ret += 'copyTempDouble(' + asmCoercion(getFastValue(ptr, '+', pos), 'i32') + '),' + makeGetTempDouble(0, 'double');
        }
      }
      ret += ')';
      return ret;
    }
  }

  const offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP && !noSafe) {
    if (!ignore) {
      return asmCoercion('SAFE_HEAP_LOAD' + ((type in Compiletime.FLOAT_TYPES) ? '_D' : '') + '(' + asmCoercion(offset, 'i32') + ', ' + Runtime.getNativeTypeSize(type) + ', ' + (!!unsigned + 0) + ')', type, unsigned ? 'u' : undefined);
    }
  }
  return getHeapForType(type, unsigned) + '[' + getHeapOffset(offset, type) + ']';
}

/**
 * @param {nunber} ptr The pointer. Used to find both the slab and the offset in that slab. If the pointer
 *            is just an integer, then this is almost redundant, but in general the pointer type
 *            may in the future include information about which slab as well. So, for now it is
 *            possible to put |0| here, but if a pointer is available, that is more future-proof.
 * @param {nunber} pos The position in that slab - the offset. Added to any offset in the pointer itself.
 * @param {number} value The value to set.
 * @param {string} type A string defining the type. Used to find the slab (HEAPU8, HEAP16, HEAPU32, etc.).
 *             'null' means, in the context of SAFE_HEAP, that we should accept all types;
 *             which means we should write to all slabs, ignore type differences if any on reads, etc.
 * @param {bool} noNeedFirst Whether to ignore the offset in the pointer itself.
 * @param {bool} ignore: legacy, ignored.
 * @param {number} align: TODO
 * @param {bool} noSafe: TODO
 * @param {string} sep: TODO
 * @param {bool} forcedAlign: legacy, ignored.
 * @return {TODO}
 */
function makeSetValue(ptr, pos, value, type, noNeedFirst, ignore, align, noSafe, sep, forcedAlign) {
  assert(!forcedAlign, 'forcedAlign is no longer supported');
  sep = sep || ';';
  if (isStructType(type)) {
    const typeData = Types.types[type];
    const ret = [];
    // We can receive either an object - an object literal that was in the .ll - or a string,
    // which is the ident of an aggregate struct
    if (typeof value === 'string') {
      value = range(typeData.fields.length).map((i) => value + '.f' + i);
    }
    for (let i = 0; i < typeData.fields.length; i++) {
      ret.push(makeSetValue(ptr, getFastValue(pos, '+', typeData.flatIndexes[i]), value[i], typeData.fields[i], noNeedFirst, 0, 0, noSafe));
    }
    return ret.join('; ');
  }

  if (type == 'double' && (align < 8)) {
    return '(' + makeSetTempDouble(0, 'double', value) + ',' +
            makeSetValue(ptr, pos, makeGetTempDouble(0, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), makeGetTempDouble(1, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  } else if (type == 'i64') {
    return '(tempI64 = [' + splitI64(value) + '],' +
            makeSetValue(ptr, pos, 'tempI64[0]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'tempI64[1]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  }

  const bits = getBits(type);
  const needSplitting = bits > 0 && !isPowerOfTwo(bits); // an unnatural type like i24
  if (align || needSplitting) {
    // Alignment is important here, or we need to split this up for other reasons.
    const bytes = Runtime.getNativeTypeSize(type);
    if (bytes > align || needSplitting) {
      let ret = '';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += 'tempBigInt=' + value + sep;
          ret += makeSetValue(ptr, pos, 'tempBigInt&0xffff', 'i16', noNeedFirst, ignore, 2, noSafe) + sep;
          ret += makeSetValue(ptr, getFastValue(pos, '+', 2), 'tempBigInt>>16', 'i16', noNeedFirst, ignore, 2, noSafe);
        } else {
          ret += 'tempBigInt=' + value + sep;
          for (let i = 0; i < bytes; i++) {
            ret += makeSetValue(ptr, getFastValue(pos, '+', i), 'tempBigInt&0xff', 'i8', noNeedFirst, ignore, 1, noSafe);
            if (i < bytes - 1) ret += sep + 'tempBigInt = tempBigInt>>8' + sep;
          }
        }
      } else {
        ret += makeSetValue('tempDoublePtr', 0, value, type, noNeedFirst, ignore, 8, noSafe, null, true) + sep;
        ret += makeCopyValues(getFastValue(ptr, '+', pos), 'tempDoublePtr', Runtime.getNativeTypeSize(type), type, null, align, sep);
      }
      return ret;
    }
  }

  const offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP && !noSafe) {
    if (!ignore) {
      return 'SAFE_HEAP_STORE' + ((type in Compiletime.FLOAT_TYPES) ? '_D' : '') + '(' + asmCoercion(offset, 'i32') + ', ' + asmCoercion(value, type) + ', ' + Runtime.getNativeTypeSize(type) + ')';
    }
  }
  return getHeapForType(type) + '[' + getHeapOffset(offset, type) + '] = ' + value;
}

const UNROLL_LOOP_MAX = 8;

function makeCopyValues(dest, src, num, type, modifier, align, sep) {
  sep = sep || ';';
  function unroll(type, num, jump) {
    jump = jump || 1;
    return range(num).map((i) => {
      return makeSetValue(dest, i * jump, makeGetValue(src, i * jump, type), type);
    }).join(sep);
  }
  // If we don't know how to handle this at compile-time, or handling it is best
  // done in a large amount of code, call memcpy
  if (!isNumber(num)) num = stripCorrections(num);
  if (!isNumber(align)) align = stripCorrections(align);
  if (!isNumber(num) || (parseInt(num) / align >= UNROLL_LOOP_MAX)) {
    return '(_memcpy(' + dest + ', ' + src + ', ' + num + ')|0)';
  }
  num = parseInt(num);
  // remove corrections, since we will be correcting after we add anyhow,
  dest = stripCorrections(dest);
  src = stripCorrections(src);
  // and in the heap assignment expression
  const ret = [];
  [4, 2, 1].forEach((possibleAlign) => {
    if (num == 0) return;
    if (align >= possibleAlign) {
      ret.push(unroll('i' + (possibleAlign * 8), Math.floor(num / possibleAlign), possibleAlign));
      src = getFastValue(src, '+', Math.floor(num / possibleAlign) * possibleAlign);
      dest = getFastValue(dest, '+', Math.floor(num / possibleAlign) * possibleAlign);
      num %= possibleAlign;
    }
  });
  return ret.join(sep);
}

function makeHEAPView(which, start, end) {
  const size = parseInt(which.replace('U', '').replace('F', '')) / 8;
  const mod = size == 1 ? '' : ('>>' + Math.log2(size));
  return `HEAP${which}.subarray((${start})${mod}, (${end})${mod})`;
}

// When dynamically linking, some things like dynCalls may not exist in one module and
// be provided by a linked module, so they must be accessed indirectly using Module
function exportedAsmFunc(func) {
  if (!MAIN_MODULE) {
    return func;
  }
  return `Module['${func}']`;
}

const TWO_TWENTY = Math.pow(2, 20);

// Given two values and an operation, returns the result of that operation.
// Tries to do as much as possible at compile time.
// Leaves overflows etc. unhandled, *except* for integer multiply, in order to be efficient with Math.imul
function getFastValue(a, op, b, type) {
  a = a === 'true' ? '1' : (a === 'false' ? '0' : a);
  b = b === 'true' ? '1' : (b === 'false' ? '0' : b);

  let aNumber = null;
  let bNumber = null;
  if (typeof a === 'number') {
    aNumber = a;
    a = a.toString();
  } else if (isNumber(a)) aNumber = parseFloat(a);
  if (typeof b === 'number') {
    bNumber = b;
    b = b.toString();
  } else if (isNumber(b)) bNumber = parseFloat(b);

  if (aNumber !== null && bNumber !== null) {
    switch (op) {
      case '+': return (aNumber + bNumber).toString();
      case '-': return (aNumber - bNumber).toString();
      case '*': return (aNumber * bNumber).toString();
      case '/': {
        if (type[0] === 'i') {
          return ((aNumber / bNumber) | 0).toString();
        } else {
          return (aNumber / bNumber).toString();
        }
      }
      case '%': return (aNumber % bNumber).toString();
      case '|': return (aNumber | bNumber).toString();
      case '>>>': return (aNumber >>> bNumber).toString();
      case '&': return (aNumber & bNumber).toString();
      case 'pow': return Math.pow(aNumber, bNumber).toString();
      default: assert(false, 'need to implement getFastValue pn ' + op);
    }
  }
  if (op === 'pow') {
    if (a === '2' && isIntImplemented(type)) {
      return `(1 << (${b}))`;
    }
    return `Math.pow(${a}, ${b})`;
  }
  if ((op === '+' || op === '*') && aNumber !== null) { // if one of them is a number, keep it last
    const c = b;
    b = a;
    a = c;
    const cNumber = bNumber;
    bNumber = aNumber;
    aNumber = cNumber;
  }
  if (op === '*') {
    // We can't eliminate where a or b are 0 as that would break things for creating
    // a negative 0.
    if ((aNumber === 0 || bNumber === 0) && !(type in Compiletime.FLOAT_TYPES)) {
      return '0';
    } else if (aNumber === 1) {
      return b;
    } else if (bNumber === 1) {
      return a;
    } else if (bNumber !== null && type && isIntImplemented(type) && Runtime.getNativeTypeSize(type) <= 32) {
      const shifts = Math.log(bNumber) / Math.LN2;
      if (shifts % 1 === 0) {
        return `(${a}<<${shifts})`;
      }
    }
    if (!(type in Compiletime.FLOAT_TYPES)) {
      // if guaranteed small enough to not overflow into a double, do a normal multiply
      // default is 32-bit multiply for things like getelementptr indexes
      const bits = getBits(type) || 32;
      // Note that we can emit simple multiple in non-asm.js mode, but asm.js
      // will not parse "16-bit" multiple, so must do imul there
      if ((aNumber !== null && Math.abs(a) < TWO_TWENTY) || (bNumber !== null && Math.abs(b) < TWO_TWENTY)) {
        // keep a non-eliminatable coercion directly on this
        return `(((${a})*(${b}))&${(Math.pow(2, bits) - 1) | 0})`;
      }
      return `(Math.imul(${a}, ${b})|0)`;
    }
  } else if (op === '/') {
    // careful on floats, since 0*NaN is not 0
    if (a === '0' && !(type in Compiletime.FLOAT_TYPES)) {
      return '0';
    } else if (b === 1) {
      return a;
    } // Doing shifts for division is problematic, as getting the rounding right on negatives is tricky
  } else if (op === '+' || op === '-') {
    if (b[0] === '-') {
      op = op === '+' ? '-' : '+';
      b = b.substr(1);
    }
    if (aNumber === 0) {
      return op === '+' ? b : `(-${b})`;
    } else if (bNumber === 0) {
      return a;
    }
  }
  return `(${a})${op}(${b})`;
}

function calcFastOffset(ptr, pos, noNeedFirst) {
  assert(!noNeedFirst);
  return getFastValue(ptr, '+', pos, 'i32');
}

function getHeapForType(type, unsigned) {
  assert(type);
  if (isPointerType(type)) {
    type = 'i32'; // Hardcoded 32-bit
  }
  switch (type) {
    case 'i1':
    case 'i8':
      return unsigned ? 'HEAPU8' : 'HEAP8';
    case 'i16':
      return unsigned ? 'HEAPU16' : 'HEAP16';
    case '<4 x i32>':
    case 'i32':
    case 'i64':
      return unsigned ? 'HEAPU32' : 'HEAP32';
    case 'double':
      return 'HEAPF64';
    case '<4 x float>':
    case 'float':
      return 'HEAPF32';
  }
  assert(false, 'bad heap type: ' + type);
}

function makeGetTempRet0() {
  return '(getTempRet0() | 0)';
}

function makeSetTempRet0(value) {
  return 'setTempRet0((' + value + ') | 0)';
}

// Takes a pair of return values, stashes one in tempRet0 and returns the other.
// Should probably be renamed to `makeReturn64` but keeping this old name in
// case external JS library code uses this name.
function makeStructuralReturn(values) {
  assert(values.length == 2);
  return makeSetTempRet0(values[1]) + '; return ' + asmCoercion(values[0], 'i32');
}

function makeThrow(what) {
  if (ASSERTIONS && DISABLE_EXCEPTION_CATCHING) {
    what += ' + " - Exception catching is disabled, this exception cannot be caught. Compile with -s NO_DISABLE_EXCEPTION_CATCHING or -s EXCEPTION_CATCHING_ALLOWED=[..] to catch."';
    if (MAIN_MODULE) {
      what += ' + " (note: in dynamic linking, if a side module wants exceptions, the main module must be built with that support)"';
    }
  }
  return `throw ${what};`;
}

function makeSignOp(value, type, op, force, ignore) {
  if (type == 'i64') {
    return value; // these are always assumed to be two 32-bit unsigneds.
  }
  if (isPointerType(type)) type = 'i32'; // Pointers are treated as 32-bit ints
  if (!value) return value;
  let bits;
  let full;
  if (type[0] === 'i') {
    bits = parseInt(type.substr(1));
    full = op + 'Sign(' + value + ', ' + bits + ', ' + Math.floor(ignore) + ')';
    // Always sign/unsign constants at compile time, regardless of CHECK/CORRECT
    if (isNumber(value)) {
      return eval(full).toString();
    }
  }
  if ((ignore) && !force) return value;
  if (type[0] === 'i') {
    // this is an integer, but not a number (or we would have already handled it)
    // shortcuts
    if (ignore) {
      if (value === 'true') {
        value = '1';
      } else if (value === 'false') {
        value = '0';
      } else if (needsQuoting(value)) value = '(' + value + ')';
      if (bits === 32) {
        if (op === 're') {
          return '(' + value + '|0)';
        } else {
          return '(' + value + '>>>0)';
        }
      } else if (bits < 32) {
        if (op === 're') {
          return '((' + value + '<<' + (32 - bits) + ')>>' + (32 - bits) + ')';
        } else {
          return '(' + value + '&' + (Math.pow(2, bits) - 1) + ')';
        }
      } else { // bits > 32
        if (op === 're') {
          return makeInlineCalculation('VALUE >= ' + Math.pow(2, bits - 1) + ' ? VALUE-' + Math.pow(2, bits) + ' : VALUE', value, 'tempBigIntS');
        } else {
          return makeInlineCalculation('VALUE >= 0 ? VALUE : ' + Math.pow(2, bits) + '+VALUE', value, 'tempBigIntS');
        }
      }
    }
    return full;
  }
  return value;
}

// We do not legalize globals, but do legalize function lines. This will be true in the latter case
// eslint-disable-next-line prefer-const
global.legalizedI64s = true;

function stripCorrections(param) {
  let m;
  while (true) {
    if (m = /^\((.*)\)$/.exec(param)) {
      param = m[1];
      continue;
    }
    if (m = /^\(([$_\w]+)\)&\d+$/.exec(param)) {
      param = m[1];
      continue;
    }
    if (m = /^\(([$_\w()]+)\)\|0$/.exec(param)) {
      param = m[1];
      continue;
    }
    if (m = /^\(([$_\w()]+)\)\>>>0$/.exec(param)) {
      param = m[1];
      continue;
    }
    if (m = /CHECK_OVERFLOW\(([^,)]*),.*/.exec(param)) {
      param = m[1];
      continue;
    }
    break;
  }
  return param;
}

function charCode(char) {
  return char.charCodeAt(0);
}

function getTypeFromHeap(suffix) {
  switch (suffix) {
    case '8': return 'i8';
    case '16': return 'i16';
    case '32': return 'i32';
    case 'F32': return 'float';
    case 'F64': return 'double';
  }
  assert(false, 'bad type suffix: ' + suffix);
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
  assert(sig.indexOf('j') == -1, 'Cannot specify 64-bit signatures ("j" in signature string) with makeDynCall!');

  const returnExpr = (sig[0] == 'v') ? '' : 'return';

  let args = [];
  for (let i = 1; i < sig.length; ++i) {
    args.push(`a${i}`);
  }
  args = args.join(', ');

  if (funcPtr === undefined) {
    printErr(`warning: ${currentlyParsedFilename}: \
Legacy use of {{{ makeDynCall("${sig}") }}}(funcPtr, arg1, arg2, ...). \
Starting from Emscripten 2.0.2 (Aug 31st 2020), syntax for makeDynCall has changed. \
New syntax is {{{ makeDynCall("${sig}", "funcPtr") }}}(arg1, arg2, ...). \
Please update to new syntax.`);

    if (DYNCALLS) {
      return `(function(cb, ${args}) { ${returnExpr} getDynCaller("${sig}", cb)(${args}) })`;
    } else {
      return `(function(cb, ${args}) { ${returnExpr} wasmTable.get(cb)(${args}) })`;
    }
  }

  if (DYNCALLS) {
    const dyncall = exportedAsmFunc(`dynCall_${sig}`);
    if (sig.length > 1) {
      return `(function(${args}) { ${returnExpr} ${dyncall}.apply(null, [${funcPtr}, ${args}]); })`;
    } else {
      return `(function() { ${returnExpr} ${dyncall}.call(null, ${funcPtr}); })`;
    }
  } else {
    return `wasmTable.get(${funcPtr})`;
  }
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

global.ATMAINS = [];

function addAtMain(code) {
  ATMAINS.push(code);
}

global.ATEXITS = [];

function addAtExit(code) {
  if (EXIT_RUNTIME) {
    ATEXITS.push(code);
  }
}

function makeRetainedCompilerSettings() {
  const ignore = set('STRUCT_INFO');
  if (STRICT) {
    for (const setting of LEGACY_SETTINGS) {
      const name = setting[0];
      ignore[name] = 0;
    }
  }

  const ret = {};
  for (const x in global) {
    if (!(x in ignore) && x[0] !== '_' && x == x.toUpperCase()) {
      try {
        if (typeof global[x] === 'number' || typeof global[x] === 'string' || this.isArray()) {
          ret[x] = global[x];
        }
      } catch (e) {}
    }
  }
  return ret;
}

// In wasm, the heap size must be a multiple of 64KiB.
const WASM_PAGE_SIZE = 65536;

// Page size reported by some POSIX calls, mostly filesystem. This does not
// depend on the memory page size which differs between wasm and asm.js, and
// makes us report a consistent value despite the compile target. However,
// perhaps we should unify all the page sizes (especially after fastcomp is
// gone TODO).
const POSIX_PAGE_SIZE = 16384;

// Receives a function as text, and a function that constructs a modified
// function, to which we pass the parsed-out name, arguments, and body of the
// function. Returns the output of that function.
function modifyFunction(text, func) {
  // Match a function with a name.
  let match = text.match(/^\s*function\s+([^(]*)?\s*\(([^)]*)\)/);
  let names;
  let args;
  let rest;
  if (match) {
    name = match[1];
    args = match[2];
    rest = text.substr(match[0].length);
  } else {
    // Match a function without a name (we could probably use a single regex
    // for both, but it would be more complex).
    match = text.match(/^\s*function\(([^)]*)\)/);
    assert(match, 'could not match function ' + text + '.');
    name = '';
    args = match[1];
    rest = text.substr(match[0].length);
  }
  const bodyStart = rest.indexOf('{');
  assert(bodyStart >= 0);
  const bodyEnd = rest.lastIndexOf('}');
  assert(bodyEnd > 0);
  return func(name, args, rest.substring(bodyStart + 1, bodyEnd));
}

function runOnMainThread(text) {
  if (USE_PTHREADS) {
    return 'if (!ENVIRONMENT_IS_PTHREAD) { ' + text + ' }';
  } else {
    return text;
  }
}

function expectToReceiveOnModule(name) {
  return name in INCOMING_MODULE_JS_API;
}

function makeRemovedModuleAPIAssert(moduleName, localName) {
  if (!ASSERTIONS) return '';
  if (!localName) localName = moduleName;
  return `
if (!Object.getOwnPropertyDescriptor(Module, '${moduleName}')) {
  Object.defineProperty(Module, '${moduleName}', {
    configurable: true,
    get: function() {
      abort('Module.${moduleName} has been replaced with plain ${localName}\
 (the initial value can be provided on Module,\
 but after startup the value is only looked for on a local variable of that name)')
    }
  });
}`;
}

// Make code to receive a value on the incoming Module object.
function makeModuleReceive(localName, moduleName) {
  if (!moduleName) moduleName = localName;
  let ret = '';
  if (expectToReceiveOnModule(moduleName)) {
    // Usually the local we use is the same as the Module property name,
    // but sometimes they must differ.
    ret = `\nif (Module['${moduleName}']) ${localName} = Module['${moduleName}'];`;
  }
  ret += makeRemovedModuleAPIAssert(moduleName, localName);
  return ret;
}

function makeModuleReceiveWithVar(localName, moduleName, defaultValue, noAssert) {
  if (!moduleName) moduleName = localName;
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
  if (!ASSERTIONS) return;
  const lower = fsName.toLowerCase();
  if (SYSTEM_JS_LIBRARIES.indexOf('library_' + lower + '.js') >= 0) return '';
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
  return variable;
}

function hasExportedFunction(func) {
  return Object.keys(EXPORTED_FUNCTIONS).indexOf(func) != -1;
}

// JS API I64 param handling: if we have BigInt support, the ABI is simple,
// it is a BigInt. Otherwise, we legalize into pairs of i32s.
function defineI64Param(name) {
  if (WASM_BIGINT) {
    return name + '_bigint';
  }
  return name + '_low, ' + name + '_high';
}

function receiveI64ParamAsI32s(name) {
  if (WASM_BIGINT) {
    // TODO: use Xn notation when JS parsers support it (as of April 6 2020,
    //  * closure compiler is missing support
    //    https://github.com/google/closure-compiler/issues/3167
    //  * acorn needs to be upgraded, and to set ecmascript version >= 11
    //  * terser needs to be upgraded
    return `var ${name}_low = Number(${name}_bigint & BigInt(0xffffffff)) | 0, ${name}_high = Number(${name}_bigint >> BigInt(32)) | 0;`;
  }
  return '';
}

function sendI64Argument(low, high) {
  if (WASM_BIGINT) {
    return 'BigInt(low) | (BigInt(high) << BigInt(32))';
  }
  return low + ', ' + high;
}

// Add assertions to catch common errors when using the Promise object we
// create on Module.ready() and return from MODULARIZE Module() invocations.
function addReadyPromiseAssertions(promise) {
  // Warn on someone doing
  //
  //  var instance = Module();
  //  ...
  //  instance._main();
  const properties = keys(EXPORTED_FUNCTIONS);
  // Also warn on onRuntimeInitialized which might be a common pattern with
  // older MODULARIZE-using codebases.
  properties.push('onRuntimeInitialized');
  return properties.map((property) => {
    const warningEnding = `${property} on the Promise object, instead of the instance. Use .then() to get called back with the instance, see the MODULARIZE docs in src/settings.js`;
    return `
      if (!Object.getOwnPropertyDescriptor(${promise}, '${property}')) {
        Object.defineProperty(${promise}, '${property}', { configurable: true, get: function() { abort('You are getting ${warningEnding}') } });
        Object.defineProperty(${promise}, '${property}', { configurable: true, set: function() { abort('You are setting ${warningEnding}') } });
      }
    `;
  }).join('\n');
}

function makeMalloc(source, param) {
  if ('_malloc' in IMPLEMENTED_FUNCTIONS) {
    return `_malloc(${param})`;
  }
  // It should be impossible to call some functions without malloc being
  // included, unless we have a deps_info.json bug. To let closure not error
  // on `_malloc` not being present, they don't call malloc and instead abort
  // with an error at runtime.
  // TODO: A more comprehensive deps system could catch this at compile time.
  if (!ASSERTIONS) {
    return 'abort();';
  }
  return `abort('malloc was not included, but is needed in ${source}. Adding "_malloc" to EXPORTED_FUNCTIONS should fix that. This may be a bug in the compiler, please file an issue.');`;
}

// Adds a call to runtimeKeepalivePush, if needed by the current build
// configuration.
// We skip this completely in MINIMAL_RUNTIME and also in builds that
// don't ever need to exit the runtime.
function runtimeKeepalivePush() {
  if (MINIMAL_RUNTIME || (EXIT_RUNTIME == 0 && USE_PTHREADS == 0)) return '';
  return 'runtimeKeepalivePush();';
}

// Adds a call to runtimeKeepalivePush, if needed by the current build
// configuration.
// We skip this completely in MINIMAL_RUNTIME and also in builds that
// don't ever need to exit the runtime.
function runtimeKeepalivePop() {
  if (MINIMAL_RUNTIME || (EXIT_RUNTIME == 0 && USE_PTHREADS == 0)) return '';
  return 'runtimeKeepalivePop();';
}
