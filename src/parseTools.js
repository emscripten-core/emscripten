//"use strict";

// Various tools for parsing LLVM. Utilities of various sorts, that are
// specific to Emscripten (and hence not in utility.js).

// Does simple 'macro' substitution, using Django-like syntax,
// {{{ code }}} will be replaced with |eval(code)|.
// NOTE: Be careful with that ret check. If ret is |0|, |ret ? ret.toString() : ''| would result in ''!
function processMacros(text) {
  return text.replace(/{{{([^}]|}(?!}))+}}}/g, function(str) {
    str = str.substr(3, str.length-6);
    var ret = eval(str);
    return ret !== null ? ret.toString() : '';
  });
}

// Simple #if/else/endif preprocessing for a file. Checks if the
// ident checked is true in our global.
// Also handles #include x.js (similar to C #include <file>)
// Param filenameHint can be passed as a description to identify the file that is being processed, used
// to locate errors for reporting.
function preprocess(text, filenameHint) {
  var lines = text.split('\n');
  var ret = '';
  var showStack = [];
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    try {
      if (line[line.length-1] == '\r') {
        line = line.substr(0, line.length-1); // Windows will have '\r' left over from splitting over '\r\n'
      }
      if (!line[0] || line[0] != '#') {
        if (showStack.indexOf(false) == -1) {
          ret += line + '\n';
        }
      } else {
        if (line[1] == 'i') {
          if (line[2] == 'f') { // if
            var parts = line.split(' ');
            var ident = parts[1];
            var op = parts[2];
            var value = parts[3];
            if (op) {
              if (op === '==') {
                showStack.push(ident in this && this[ident] == value);
              } else if (op === '!=') {
                showStack.push(!(ident in this && this[ident] == value));
              } else if (op === '<') {
                showStack.push(ident in this && this[ident] < value);
              } else if (op === '>') {
                showStack.push(ident in this && this[ident] > value);
              } else {
                error('unsupported preprocessor op ' + op);
              }
            } else {
              if (ident[0] === '!') {
                showStack.push(!(this[ident.substr(1)] > 0));
              } else {
                showStack.push(ident in this && this[ident] > 0);
              }
            }
          } else if (line[2] == 'n') { // include
            var filename = line.substr(line.indexOf(' ')+1);
            if (filename.indexOf('"') === 0) {
              filename = filename.substr(1, filename.length - 2);
            }
            var included = read(filename);
            ret += '\n' + preprocess(included, filename) + '\n'
          }
        } else if (line[2] == 'l') { // else
          assert(showStack.length > 0);
          showStack.push(!showStack.pop());
        } else if (line[2] == 'n') { // endif
          assert(showStack.length > 0);
          showStack.pop();
        } else {
          throw "Unclear preprocessor command: " + line;
        }
      }
    } catch(e) {
      printErr('parseTools.js preprocessor error in ' + filenameHint + ':' + (i+1) + ': \"' + line + '\"!');
      throw e;
    }
  }
  assert(showStack.length == 0);
  return ret;
}

function removePointing(type, num) {
  if (num === 0) return type;
  assert(type.substr(type.length-(num ? num : 1)).replace(/\*/g, '') === ''); //, 'Error in removePointing with ' + [type, num, type.substr(type.length-(num ? num : 1))]);
  return type.substr(0, type.length-(num ? num : 1));
}

function pointingLevels(type) {
  if (!type) return 0;
  var ret = 0;
  var len1 = type.length - 1;
  while (type[len1-ret] && type[len1-ret] === '*') {
    ret++;
  }
  return ret;
}

function removeAllPointing(type) {
  return removePointing(type, pointingLevels(type));
}

function toNiceIdent(ident) {
  assert(ident);
  if (parseFloat(ident) == ident) return ident;
  if (ident == 'null') return '0'; // see parseNumerical
  if (ident == 'undef') return '0';
  return ident.replace('%', '$').replace(/["&\\ \.@:<>,\*\[\]\(\)-]/g, '_');
}

// Kind of a hack. In some cases we have strings that we do not want
// to |toNiceIdent|, as they are the output of previous processing. We
// should refactor everything into an object, with an explicit flag
// saying what has been |toNiceIdent|ed. Until then, this will detect
// simple idents that are in need of |toNiceIdent|ation. Or, we should
// ensure that processed strings never start with %,@, e.g. by always
// enclosing them in ().
function toNiceIdentCarefully(ident) {
  if (ident[0] == '%' || ident[0] == '@') ident = toNiceIdent(ident);
  return ident;
}

// Returns true if ident is a niceIdent (see toNiceIdent). If loose
// is true, then also allow () and spaces.
function isNiceIdent(ident, loose) {
  if (loose) {
    return /^\(?[$_]+[\w$_\d ]*\)?$/.test(ident);
  } else {
    return /^[$_]+[\w$_\d]*$/.test(ident);
  }
}

// Simple variables or numbers, or things already quoted, do not need to be quoted
function needsQuoting(ident) {
  if (/^[-+]?[$_]?[\w$_\d]*$/.test(ident)) return false; // number or variable
  if (ident[0] === '(' && ident[ident.length-1] === ')' && ident.indexOf('(', 1) < 0) return false; // already fully quoted
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
  return type[type.length-1] == '*';
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
  return type[type.length-1] === '>';
}

function isStructuralType(type) {
  return /^\{ ?[^}]* ?\}$/.test(type); // { i32, i8 } etc. - anonymous struct types
}

function getStructuralTypeParts(type) { // split { i32, i8 } etc. into parts
  return type.replace(/[ {}]/g, '').split(',');
}

function getStructuralTypePartBits(part) {
  return Math.ceil((getBits(part) || 32)/32)*32; // simple 32-bit alignment. || 32 is for pointers
}

function isIntImplemented(type) {
  return type[0] == 'i' || isPointerType(type);
}

// Note: works for iX types and structure types, not pointers (even though they are implemented as ints)
function getBits(type, allowPointers) {
  if (allowPointers && isPointerType(type)) return 32;
  if (!type) return 0;
  if (type[0] == 'i') {
    var left = type.substr(1);
    if (!isNumber(left)) return 0;
    return parseInt(left);
  }
  if (isStructuralType(type)) {
    return sum(getStructuralTypeParts(type).map(getStructuralTypePartBits));
  }
  if (isStructType(type)) {
    var typeData = Types.types[type];
    if (typeData === undefined) return 0;
    return typeData.flatSize*8;
  }
  return 0;
}

function getNumIntChunks(type) {
  return Math.ceil(getBits(type, true)/32);
}

function isIdenticallyImplemented(type1, type2) {
  var floats = +(type1 in Compiletime.FLOAT_TYPES) + +(type2 in Compiletime.FLOAT_TYPES);
  if (floats == 2) return true;
  if (floats == 1) return false;
  return getNumIntChunks(type1) == getNumIntChunks(type2);
}

function isIllegalType(type) {
  switch (type) {
    case 'i1':
    case 'i8':
    case 'i16':
    case 'i32':
    case 'float':
    case 'double':
    case 'rawJS':
    case '<2 x float>':
    case '<4 x float>':
    case '<2 x i32>':
    case '<4 x i32>':
    case 'void': return false;
  }
  if (!type || type[type.length-1] === '*') return false;
  return true;
}

function isVoidType(type) {
  return type == 'void';
}

// Detects a function definition, ([...|type,[type,...]])
function isFunctionDef(token, out) {
  var text = token.text;
  var nonPointing = removeAllPointing(text);
  if (nonPointing[0] != '(' || nonPointing.substr(-1) != ')')
    return false;
  if (nonPointing === '()') return true;
  if (!token.tokens) return false;
  var fail = false;
  var segments = splitTokenList(token.tokens);
  segments.forEach(function(segment) {
    var subtext = segment[0].text;
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
  var len = type.length;
  return type[len-2] == ')' && type[len-1] == '*';
}

function isFunctionType(type, out) {
  if (!isPossiblyFunctionType(type)) return false;
  type = type.substr(0, type.length-1); // remove final '*'
  var firstOpen = type.indexOf('(');
  if (firstOpen <= 0) return false;
  type = type.replace(/"[^"]+"/g, '".."');
  var lastOpen = type.lastIndexOf('(');
  var returnType;
  if (firstOpen == lastOpen) {
    returnType = getReturnType(type);
    if (!isType(returnType)) return false;
  } else {
    returnType = 'i8*'; // some pointer type, no point in analyzing further
  }
  if (out) out.returnType = returnType;
  // find ( that starts the arguments
  var depth = 0, i = type.length-1, argText = null;
  while (i >= 0) {
    var curr = type[i];
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
  return isFunctionDef({ text: argText, tokens: tokenize(argText.substr(1, argText.length-2)) }, out);
}

function getReturnType(type) {
  if (pointingLevels(type) > 1) return '*'; // the type of a call can be either the return value, or the entire function. ** or more means it is a return value
  var lastOpen = type.lastIndexOf('(');
  if (lastOpen > 0) {
    // handle things like   void (i32)* (i32, void (i32)*)*
    var closeStar = type.indexOf(')*');
    if (closeStar > 0 && closeStar < type.length-2) lastOpen = closeStar+3;
    return type.substr(0, lastOpen-1);
  }
  return type;
}

var isTypeCache = {}; // quite hot, optimize as much as possible
function isType(type) {
  if (type in isTypeCache) return isTypeCache[type];
  var ret = isPointerType(type) || isVoidType(type) || Compiletime.isNumberType(type) || isStructType(type) || isFunctionType(type);
  isTypeCache[type] = ret;
  return ret;
}

var SPLIT_TOKEN_LIST_SPLITTERS = set(',', 'to'); // 'to' can separate parameters as well...

// Splits a list of tokens separated by commas. For example, a list of arguments in a function call
function splitTokenList(tokens) {
  if (tokens.length == 0) return [];
  if (!tokens.slice) tokens = tokens.tokens;
  var ret = [];
  var seg = [];
  for (var i = 0; i < tokens.length; i++) {
    var token = tokens[i];
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

var UNINDEXABLE_GLOBALS = set(
  '_llvm_global_ctors' // special-cased
);

function isIndexableGlobal(ident) {
  if (!(ident in Variables.globals)) return false;
  if (ident in UNINDEXABLE_GLOBALS) {
    Variables.globals[ident].unIndexable = true;
    return false;
  }
  var data = Variables.globals[ident];
  return !data.alias && !data.external;
}

function makeGlobalUse(ident) {
  if (isIndexableGlobal(ident)) {
    var index = Variables.indexedGlobals[ident];
    if (index === undefined) {
      // we are accessing this before we index globals, likely from the library. mark as unindexable
      UNINDEXABLE_GLOBALS[ident] = 1;
      return ident;
    }
    var ret = (Runtime.GLOBAL_BASE + index).toString();
    if (SIDE_MODULE) ret = '(H_BASE+' + ret + ')';
    return ret;
  }
  return ident;
}

function _IntToHex(x) {
  assert(x >= 0 && x <= 15);
  if (x <= 9) {
    return String.fromCharCode('0'.charCodeAt(0) + x);
  } else {
    return String.fromCharCode('A'.charCodeAt(0) + x - 10);
  }
}

function IEEEUnHex(stringy) {
  stringy = stringy.substr(2); // leading '0x';
  if (stringy.replace(/0/g, '') === '') return 0;
  while (stringy.length < 16) stringy = '0' + stringy;
  assert(stringy.length === 16, 'Can only unhex 16-digit double numbers, nothing platform-specific'); // |long double| might cause this
  var top = eval('0x' + stringy[0]);
  var neg = !!(top & 8); // sign
  if (neg) {
    stringy = _IntToHex(top & ~8) + stringy.substr(1);
  }
  var a = eval('0x' + stringy.substr(0, 8)); // top half
  var b = eval('0x' + stringy.substr(8)); // bottom half
  var e = a >> ((52 - 32) & 0x7ff); // exponent
  a = a & 0xfffff;
  if (e === 0x7ff) {
    if (a == 0 && b == 0) {
      return neg ? '-Infinity' : 'Infinity';
    } else {
      return 'NaN';
    }
  }
  e -= 1023; // offset
  var absolute = ((((a | 0x100000) * 1.0) / Math.pow(2,52-32)) * Math.pow(2, e)) + (((b * 1.0) / Math.pow(2, 52)) * Math.pow(2, e));
  return (absolute * (neg ? -1 : 1)).toString();
}

// Given an expression like (VALUE=VALUE*2,VALUE<10?VALUE:t+1) , this will
// replace VALUE with value. If value is not a simple identifier of a variable,
// value will be replaced with tempVar.
function makeInlineCalculation(expression, value, tempVar) {
  if (!isNiceIdent(value, true)) {
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
// Will suffer from rounding. mergeI64 does the opposite.
function splitI64(value, floatConversion) {
  // general idea:
  //
  //  $1$0 = ~~$d >>> 0;
  //  $1$1 = Math_abs($d) >= 1 ? (
  //     $d > 0 ? Math.min(Math_floor(($d)/ 4294967296.0), 4294967295.0)
  //            : Math_ceil(Math.min(-4294967296.0, $d - $1$0)/ 4294967296.0)
  //  ) : 0;
  //
  // We need to min on positive values here, since our input might be a double, and large values are rounded, so they can
  // be slightly higher than expected. And if we get 4294967296, that will turn into a 0 if put into a
  // HEAP32 or |0'd, etc.
  //
  // For negatives, we need to ensure a -1 if the value is overall negative, even if not significant negative component

  var lowInput = legalizedI64s ? value : 'VALUE';
  if (floatConversion) lowInput = asmFloatToInt(lowInput);
  var low = lowInput + '>>>0';
  var high = makeInlineCalculation(
    asmCoercion('Math_abs(VALUE)', 'double') + ' >= ' + asmEnsureFloat('1', 'double') + ' ? ' +
      '(VALUE > ' + asmEnsureFloat('0', 'double') + ' ? ' +
               asmCoercion('Math_min(' + asmCoercion('Math_floor((VALUE)/' + asmEnsureFloat(4294967296, 'double') + ')', 'double') + ', ' + asmEnsureFloat(4294967295, 'double') + ')', 'i32') + '>>>0' +
               ' : ' + asmFloatToInt(asmCoercion('Math_ceil((VALUE - +((' + asmFloatToInt('VALUE') + ')>>>0))/' + asmEnsureFloat(4294967296, 'double') + ')', 'double')) + '>>>0' + 
      ')' +
    ' : 0',
    value,
    'tempDouble'
  );
  if (legalizedI64s) {
    return [low, high];
  } else {
    return makeI64(low, high);
  }
}
function mergeI64(value, unsigned) {
  if (legalizedI64s) {
    return RuntimeGenerator.makeBigInt(value + '$0', value + '$1', unsigned);
  } else {
    return makeInlineCalculation(RuntimeGenerator.makeBigInt('VALUE[0]', 'VALUE[1]', unsigned), value, 'tempI64');
  }
}

// Takes an i64 value and changes it into the [low, high] form used in i64 mode 1. In that
// mode, this is a no-op
function ensureI64_1(value) {
  return value;
}

function makeCopyI64(value) {
  return value + '.slice(0)';
}

// Given a string representation of an integer of arbitrary size, return it
// split up into 32-bit chunks
function parseArbitraryInt(str, bits) {
  // We parse the string into a vector of digits, base 10. This is convenient to work on.

  assert(bits > 0); // NB: we don't check that the value in str can fit in this amount of bits

  function str2vec(s) { // index 0 is the highest value
    var ret = [];
    for (var i = 0; i < s.length; i++) {
      ret.push(s.charCodeAt(i) - '0'.charCodeAt(0));
    }
    return ret;
  }

  function divide2(v) { // v /= 2
    for (var i = v.length-1; i >= 0; i--) {
      var d = v[i];
      var r = d % 2;
      d = Math.floor(d/2);
      v[i] = d;
      if (r) {
        assert(i+1 < v.length);
        var d2 = v[i+1];
        d2 += 5;
        if (d2 >= 10) {
          v[i] = d+1;
          d2 -= 10;
        }
        v[i+1] = d2;
      }
    }
  }

  function mul2(v) { // v *= 2
    for (var i = v.length-1; i >= 0; i--) {
      var d = v[i]*2;
      r = d >= 10;
      v[i] = d%10;
      var j = i-1;
      if (r) {
        if (j < 0) {
          v.unshift(1);
          break;
        }
        v[j] += 0.5; // will be multiplied
      }
    }
  }

  function subtract(v, w) { // v -= w. we assume v >= w
    while (v.length > w.length) w.splice(0, 0, 0);
    for (var i = 0; i < v.length; i++) {
      v[i] -= w[i];
      if (v[i] < 0) {
        v[i] += 10;
        // find something to take from
        var j = i-1;
        while (v[j] == 0) {
          v[j] = 9;
          j--;
          assert(j >= 0);
        }
        v[j]--;
      }
    }
  }

  function isZero(v) {
    for (var i = 0; i < v.length; i++) {
      if (v[i] > 0) return false;
    }
    return true;
  }

  var v;

  if (str[0] == '-') {
    // twos-complement is needed
    str = str.substr(1);
    v = str2vec('1');
    for (var i = 0; i < bits; i++) {
      mul2(v);
    }
    subtract(v, str2vec(str));
  } else {
    v = str2vec(str);
  }

  var bitsv = [];
  while (!isZero(v)) {
    bitsv.push((v[v.length-1] % 2 != 0)+0);
    v[v.length-1] = v[v.length-1] & 0xfe;
    divide2(v);
  }

  var ret = zeros(Math.ceil(bits/32));
  for (var i = 0; i < bitsv.length; i++) {
    ret[Math.floor(i/32)] += bitsv[i]*Math.pow(2, i % 32);
  }
  return ret;
}

function parseI64Constant(str, legalized) {
  if (!isNumber(str)) {
    // This is a variable. Copy it, so we do not modify the original
    return legalizedI64s ? str : makeCopyI64(str);
  }

  var parsed = parseArbitraryInt(str, 64);
  if (legalizedI64s || legalized) return parsed;
  return '[' + parsed[0] + ',' + parsed[1] + ']';
}

function parseNumerical(value, type) {
  if ((!type || type === 'double' || type === 'float') && /^0x/.test(value)) {
    // Hexadecimal double value, as the llvm docs say,
    // "The one non-intuitive notation for constants is the hexadecimal form of floating point constants."
    value = IEEEUnHex(value);
  } else if (isIllegalType(type)) {
    return value; // do not parseFloat etc., that can lead to loss of precision
  } else if (value === 'null') {
    // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)
    value = '0';
  } else if (value === 'true') {
    return '1';
  } else if (value === 'false') {
    return '0';
  }
  if (isNumber(value)) {
    var ret = parseFloat(value); // will change e.g. 5.000000e+01 to 50
    // type may be undefined here, like when this is called from makeConst with a single argument.
    // but if it is a number, then we can safely assume that this should handle negative zeros
    // correctly.
    if (type === undefined || type === 'double' || type === 'float') {
      if (value[0] === '-' && ret === 0) { return '-.0'; } // fix negative 0, toString makes it 0
    }
    if (type === 'double' || type === 'float') {
      if (!RUNNING_JS_OPTS) ret = asmEnsureFloat(ret, type);
    }
    return ret.toString();
  } else {
    return value;
  }
}

// \0Dsometext is really '\r', then sometext
// This function returns an array of int values
function parseLLVMString(str) {
  var ret = [];
  var i = 0;
  while (i < str.length) {
    var chr = str.charCodeAt(i);
    if (chr !== 92) { // 92 === '//'.charCodeAt(0)
      ret.push(chr);
      i++;
    } else {
      ret.push(parseInt(str[i+1]+str[i+2], '16'));
      i += 3;
    }
  }
  return ret;
}

// Generates the type signature for a structure, for each byte, the type that is there.
// i32, 0, 0, 0 - for example, an int32 is here, then nothing to do for the 3 next bytes, naturally
function generateStructTypes(type) {
  if (isArray(type)) return type; // already in the form of [type, type,...]
  if (Compiletime.isNumberType(type) || isPointerType(type)) {
    if (type == 'i64') {
      return ['i64', 0, 0, 0, 'i32', 0, 0, 0];
    }
    return [type].concat(zeros(Runtime.getNativeFieldSize(type)-1));
  }

  // Avoid multiple concats by finding the size first. This is much faster
  var typeData = Types.types[type];
  var size = typeData.flatSize;
  var ret = new Array(size);
  var index = 0;
  function add(typeData) {
    var array = typeData.name_[0] === '['; // arrays just have 2 elements in their fields, see calculateStructAlignment
    var num = array ? parseInt(typeData.name_.substr(1)) : typeData.fields.length;
    var start = index;
    for (var i = 0; i < num; i++) {
      var type = array ? typeData.fields[0] : typeData.fields[i];
      if (!SAFE_HEAP && isPointerType(type)) type = '*'; // do not include unneeded type names without safe heap
      if (Compiletime.isNumberType(type) || isPointerType(type)) {
        if (type == 'i64') {
          ret[index++] = 'i64';
          ret[index++] = 0;
          ret[index++] = 0;
          ret[index++] = 0;
          ret[index++] = 'i32';
          ret[index++] = 0;
          ret[index++] = 0;
          ret[index++] = 0;
          continue;
        }
        ret[index++] = type;
      } else {
        if (isStructType(type) && type[1] === '0') {
          // this is [0 x something], which does nothing
          // XXX this happens in java_nbody... assert(i === typeData.fields.length-1);
          continue;
        }
        add(Types.types[type]);
      }
      var more = array ? (i+1)*typeData.flatSize/num : (
        (i+1 < typeData.fields.length ? typeData.flatIndexes[i+1] : typeData.flatSize)
      );
      more -= index - start;
      for (var j = 0; j < more; j++) {
        ret[index++] = 0;
      }
    }
  }
  add(typeData);
  assert(index == size);
  return ret;
}

// Misc

function indentify(text, indent) {
  if (text.length > 1024*1024) return text; // Don't try to indentify huge strings - we may run out of memory
  if (typeof indent === 'number') {
    var len = indent;
    indent = '';
    for (var i = 0; i < len; i++) indent += ' ';
  }
  return text.replace(/\n/g, '\n' + indent);
}

// Correction tools

function checkSafeHeap() {
  return SAFE_HEAP === 1;
}

function getHeapOffset(offset, type) {
  if (Runtime.getNativeFieldSize(type) > 4) {
    if (type == 'i64') {
      type = 'i32'; // we emulate 64-bit integer values as 32 in asmjs-unknown-emscripten, but not double
    }
  }

  var sz = Runtime.getNativeTypeSize(type);
  var shifts = Math.log(sz)/Math.LN2;
  offset = '(' + offset + ')';
  return '(' + offset + '>>' + shifts + ')';
}

function ensureDot(value) {
  value = value.toString();
  // if already dotted, or Infinity or NaN, nothing to do here
  // if smaller than 1 and running js opts, we always need to force a coercion (0.001 will turn into 1e-3, which has no .)
  if ((value.indexOf('.') >= 0 || /[IN]/.test(value)) && (!RUNNING_JS_OPTS || Math.abs(value) >= 1)) return value;
  if (RUNNING_JS_OPTS) return '(+' + value + ')'; // JS optimizer will run, we must do +x, and it will be corrected later
  var e = value.indexOf('e');
  if (e < 0) return value + '.0';
  return value.substr(0, e) + '.0' + value.substr(e);
}

function asmEnsureFloat(value, type) { // ensures that a float type has either 5.5 (clearly a float) or +5 (float due to asm coercion)
  if (!isNumber(value)) return value;
  if (PRECISE_F32 && type === 'float') {
    // normally ok to just emit Math_fround(0), but if the constant is large we may need a .0 (if it can't fit in an int)
    if (value == 0) return 'Math_fround(0)';
    value = ensureDot(value);
    return 'Math_fround(' + value + ')';
  }
  if (type in Compiletime.FLOAT_TYPES) {
    return ensureDot(value);
  } else {
    return value;
  }
}

function asmInitializer(type) {
  if (type in Compiletime.FLOAT_TYPES) {
    if (PRECISE_F32 && type === 'float') return 'Math_fround(0)';
    return RUNNING_JS_OPTS ? '+0' : '.0';
  } else {
    return '0';
  }
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
      if (PRECISE_F32 && type === 'float') {
        return 'Math_fround(' + value + ')';
      } else {
        return '(+(' + value + '))';
      }
    }
  } else {
    return '((' + value + ')|0)';
  }
}

function asmFloatToInt(x) {
  return '(~~(' + x + '))';
}

function makeGetTempDouble(i, type, forSet) { // get an aliased part of the tempDouble temporary storage
  // Cannot use makeGetValue because it uses us
  // this is a unique case where we *can* use HEAPF64
  var slab = type == 'double' ? 'HEAPF64' : makeGetSlabs(null, type)[0];
  var ptr = getFastValue('tempDoublePtr', '+', Runtime.getNativeTypeSize(type)*i);
  var offset;
  if (type == 'double') {
    offset = '(' + ptr + ')>>3';
  } else {
    offset = getHeapOffset(ptr, type);
  }
  var ret = slab + '[' + offset + ']';
  if (!forSet) ret = asmCoercion(ret, type);
  return ret;
}

function makeSetTempDouble(i, type, value) {
  return makeGetTempDouble(i, type, true) + '=' + asmEnsureFloat(value, type);
}

var asmPrintCounter = 0;

// See makeSetValue
function makeGetValue(ptr, pos, type, noNeedFirst, unsigned, ignore, align, noSafe, forceAsm) {
  if (UNALIGNED_MEMORY) align = 1;
  else if (FORCE_ALIGNED_MEMORY && !isIllegalType(type)) align = 8;

  if (isStructType(type)) {
    var typeData = Types.types[type];
    var ret = [];
    for (var i = 0; i < typeData.fields.length; i++) {
      ret.push('f' + i + ': ' + makeGetValue(ptr, pos + typeData.flatIndexes[i], typeData.fields[i], noNeedFirst, unsigned, 0, 0, noSafe));
    }
    return '{ ' + ret.join(', ') + ' }';
  }

  // In double mode 1, in asmjs-unknown-emscripten we need this code path if we are not fully aligned.
  if (DOUBLE_MODE == 1 && type == 'double' && (align < 8)) {
    return '(' + makeSetTempDouble(0, 'i32', makeGetValue(ptr, pos, 'i32', noNeedFirst, unsigned, ignore, align, noSafe)) + ',' +
                 makeSetTempDouble(1, 'i32', makeGetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'i32', noNeedFirst, unsigned, ignore, align, noSafe)) + ',' +
            makeGetTempDouble(0, 'double') + ')';
  }

  if (align) {
    // Alignment is important here. May need to split this up
    var bytes = Runtime.getNativeTypeSize(type);
    if (DOUBLE_MODE == 0 && type == 'double') bytes = 4; // we will really only read 4 bytes here
    if (bytes > align) {
      var ret = '(';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += makeGetValue(ptr, pos, 'i16', noNeedFirst, 2, ignore, 2, noSafe) + '|' +
                 '(' + makeGetValue(ptr, getFastValue(pos, '+', 2), 'i16', noNeedFirst, 2, ignore, 2, noSafe) + '<<16)';
        } else { // XXX we cannot truly handle > 4... (in x86)
          ret = '';
          for (var i = 0; i < bytes; i++) {
            ret += '(' + makeGetValue(ptr, getFastValue(pos, '+', i), 'i8', noNeedFirst, 1, ignore, 1, noSafe) + (i > 0 ? '<<' + (8*i) : '') + ')';
            if (i < bytes-1) ret += '|';
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

  var offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP && !noSafe) {
    var printType = type;
    if (printType !== 'null' && printType[0] !== '#') printType = '"' + safeQuote(printType) + '"';
    if (printType[0] === '#') printType = printType.substr(1);
    if (!ignore) {
      return asmCoercion('SAFE_HEAP_LOAD' + ((type in Compiletime.FLOAT_TYPES) ? '_D' : '') + '(' + asmCoercion(offset, 'i32') + ', ' + Runtime.getNativeTypeSize(type) + ', ' + (!!unsigned+0) + ')', type);
    }
  }
  var ret = makeGetSlabs(ptr, type, false, unsigned)[0] + '[' + getHeapOffset(offset, type) + ']';
  if (forceAsm) {
    ret = asmCoercion(ret, type);
  }
  return ret;
}

function makeGetValueAsm(ptr, pos, type, unsigned) {
  return makeGetValue(ptr, pos, type, null, unsigned, null, null, null, true);
}

//! @param ptr The pointer. Used to find both the slab and the offset in that slab. If the pointer
//!            is just an integer, then this is almost redundant, but in general the pointer type
//!            may in the future include information about which slab as well. So, for now it is
//!            possible to put |0| here, but if a pointer is available, that is more future-proof.
//! @param pos The position in that slab - the offset. Added to any offset in the pointer itself.
//! @param value The value to set.
//! @param type A string defining the type. Used to find the slab (HEAPU8, HEAP16, HEAPU32, etc.).
//!             'null' means, in the context of SAFE_HEAP, that we should accept all types;
//!             which means we should write to all slabs, ignore type differences if any on reads, etc.
//! @param noNeedFirst Whether to ignore the offset in the pointer itself.
function makeSetValue(ptr, pos, value, type, noNeedFirst, ignore, align, noSafe, sep, forcedAlign, forceAsm) {
  if (UNALIGNED_MEMORY && !forcedAlign) align = 1;
  else if (FORCE_ALIGNED_MEMORY && !isIllegalType(type)) align = 8;

  sep = sep || ';';
  if (isStructType(type)) {
    var typeData = Types.types[type];
    var ret = [];
    // We can receive either an object - an object literal that was in the .ll - or a string,
    // which is the ident of an aggregate struct
    if (typeof value === 'string') {
      value = range(typeData.fields.length).map(function(i) { return value + '.f' + i });
    }
    for (var i = 0; i < typeData.fields.length; i++) {
      ret.push(makeSetValue(ptr, getFastValue(pos, '+', typeData.flatIndexes[i]), value[i], typeData.fields[i], noNeedFirst, 0, 0, noSafe));
    }
    return ret.join('; ');
  }

  if (DOUBLE_MODE == 1 && type == 'double' && (align < 8)) {
    return '(' + makeSetTempDouble(0, 'double', value) + ',' +
            makeSetValue(ptr, pos, makeGetTempDouble(0, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), makeGetTempDouble(1, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  } else if (type == 'i64') {
    return '(tempI64 = [' + splitI64(value) + '],' +
            makeSetValue(ptr, pos, 'tempI64[0]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'tempI64[1]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  }

  var bits = getBits(type);
  var needSplitting = bits > 0 && !isPowerOfTwo(bits); // an unnatural type like i24
  if (align || needSplitting) {
    // Alignment is important here, or we need to split this up for other reasons.
    var bytes = Runtime.getNativeTypeSize(type);
    if (DOUBLE_MODE == 0 && type == 'double') bytes = 4; // we will really only read 4 bytes here
    if (bytes > align || needSplitting) {
      var ret = '';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += 'tempBigInt=' + value + sep;
          ret += makeSetValue(ptr, pos, 'tempBigInt&0xffff', 'i16', noNeedFirst, ignore, 2, noSafe) + sep;
          ret += makeSetValue(ptr, getFastValue(pos, '+', 2), 'tempBigInt>>16', 'i16', noNeedFirst, ignore, 2, noSafe);
        } else {
          ret += 'tempBigInt=' + value + sep;
          for (var i = 0; i < bytes; i++) {
            ret += makeSetValue(ptr, getFastValue(pos, '+', i), 'tempBigInt&0xff', 'i8', noNeedFirst, ignore, 1, noSafe);
            if (i < bytes-1) ret += sep + 'tempBigInt = tempBigInt>>8' + sep;
          }
        }
      } else {
        ret += makeSetValue('tempDoublePtr', 0, value, type, noNeedFirst, ignore, 8, noSafe, null, true) + sep;
        ret += makeCopyValues(getFastValue(ptr, '+', pos), 'tempDoublePtr', Runtime.getNativeTypeSize(type), type, null, align, sep);
      }
      return ret;
    }
  }

  var offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP && !noSafe) {
    var printType = type;
    if (printType !== 'null' && printType[0] !== '#') printType = '"' + safeQuote(printType) + '"';
    if (printType[0] === '#') printType = printType.substr(1);
    if (!ignore) {
      return 'SAFE_HEAP_STORE' + ((type in Compiletime.FLOAT_TYPES) ? '_D' : '') + '(' + asmCoercion(offset, 'i32') + ', ' + asmCoercion(value, type) + ', ' + Runtime.getNativeTypeSize(type) + ')';
    }
  }
  return makeGetSlabs(ptr, type, true).map(function(slab) { return slab + '[' + getHeapOffset(offset, type) + ']=' + value }).join(sep);
}

function makeSetValueAsm(ptr, pos, value, type, noNeedFirst, ignore, align, noSafe, sep, forcedAlign) {
  return makeSetValue(ptr, pos, value, type, noNeedFirst, ignore, align, noSafe, sep, forcedAlign, true);
}

var UNROLL_LOOP_MAX = 8;

function makeSetValues(ptr, pos, value, type, num, align) {
  function unroll(type, num, jump, value$) {
    jump = jump || 1;
    value$ = value$ || value;
    return range(num).map(function(i) {
      return makeSetValue(ptr, getFastValue(pos, '+', i*jump), value$, type);
    }).join('; ');
  }
  // If we don't know how to handle this at compile-time, or handling it is best done in a large amount of code, call memset
  // TODO: optimize the case of numeric num but non-numeric value
  if (!isNumber(num) || !isNumber(value) || (parseInt(num)/align >= UNROLL_LOOP_MAX)) {
    return '_memset(' + asmCoercion(getFastValue(ptr, '+', pos), 'i32') + ', ' + asmCoercion(value, 'i32') + ', ' + asmCoercion(num, 'i32') + ')|0';
  }
  num = parseInt(num);
  value = parseInt(value);
  if (value < 0) value += 256; // make it unsigned
  var values = {
    1: value,
    2: value | (value << 8), 
    4: value | (value << 8) | (value << 16) | (value << 24)
  };
  var ret = [];
  [4, 2, 1].forEach(function(possibleAlign) {
    if (num == 0) return;
    if (align >= possibleAlign) {
      ret.push(unroll('i' + (possibleAlign*8), Math.floor(num/possibleAlign), possibleAlign, values[possibleAlign]));
      pos = getFastValue(pos, '+', Math.floor(num/possibleAlign)*possibleAlign);
      num %= possibleAlign;
    }
  });
  return ret.join('; ');
}

var TYPED_ARRAY_SET_MIN = Infinity; // .set() as memcpy seems to just slow us down

function makeCopyValues(dest, src, num, type, modifier, align, sep) {
  sep = sep || ';';
  function unroll(type, num, jump) {
    jump = jump || 1;
    return range(num).map(function(i) {
      return makeSetValue(dest, i*jump, makeGetValue(src, i*jump, type), type);
    }).join(sep);
  }
  // If we don't know how to handle this at compile-time, or handling it is best done in a large amount of code, call memcpy
  if (!isNumber(num)) num = stripCorrections(num);
  if (!isNumber(align)) align = stripCorrections(align);
  if (!isNumber(num) || (parseInt(num)/align >= UNROLL_LOOP_MAX)) {
    return '(_memcpy(' + dest + ', ' + src + ', ' + num + ')|0)';
  }
  num = parseInt(num);
  dest = stripCorrections(dest); // remove corrections, since we will be correcting after we add anyhow,
  src = stripCorrections(src);   // and in the heap assignment expression
  var ret = [];
  [4, 2, 1].forEach(function(possibleAlign) {
    if (num == 0) return;
    if (align >= possibleAlign) {
      ret.push(unroll('i' + (possibleAlign*8), Math.floor(num/possibleAlign), possibleAlign));
      src = getFastValue(src, '+', Math.floor(num/possibleAlign)*possibleAlign);
      dest = getFastValue(dest, '+', Math.floor(num/possibleAlign)*possibleAlign);
      num %= possibleAlign;
    }
  });
  return ret.join(sep);
}

function makeHEAPView(which, start, end) {
  var size = parseInt(which.replace('U', '').replace('F', ''))/8;
  var mod = size == 1 ? '' : ('>>' + log2(size));
  return 'HEAP' + which + '.subarray((' + start + ')' + mod + ',(' + end + ')' + mod + ')';
}

var TWO_TWENTY = Math.pow(2, 20);

// Given two values and an operation, returns the result of that operation.
// Tries to do as much as possible at compile time.
// Leaves overflows etc. unhandled, *except* for integer multiply, in order to be efficient with Math.imul
function getFastValue(a, op, b, type) {
  a = a === 'true' ? '1' : (a === 'false' ? '0' : a);
  b = b === 'true' ? '1' : (b === 'false' ? '0' : b);

  var aNumber = null, bNumber = null;
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
          return ((aNumber / bNumber)|0).toString();
        } else {
          return (aNumber / bNumber).toString();
        }
      }
      case '%': return (aNumber % bNumber).toString();
      case '|': return (aNumber | bNumber).toString();
      case '>>>': return (aNumber >>> bNumber).toString();
      case '&': return (aNumber & bNumber).toString();
      case 'pow': return Math.pow(aNumber, bNumber).toString();
      default: throw 'need to implement getFastValue pn ' + op;
    }
  }
  if (op === 'pow') {
    if (a === '2' && isIntImplemented(type)) {
      return '(1 << (' + b + '))';
    }
    return 'Math_pow(' + a + ', ' + b + ')';
  }
  if ((op === '+' || op === '*') && aNumber !== null) { // if one of them is a number, keep it last
    var c = b;
    b = a;
    a = c;
    var cNumber = bNumber;
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
      var shifts = Math.log(bNumber)/Math.LN2;
      if (shifts % 1 === 0) {
        return '(' + a + '<<' + shifts + ')';
      }
    }
    if (!(type in Compiletime.FLOAT_TYPES)) {
      // if guaranteed small enough to not overflow into a double, do a normal multiply
      var bits = getBits(type) || 32; // default is 32-bit multiply for things like getelementptr indexes
      // Note that we can emit simple multiple in non-asm.js mode, but asm.js will not parse "16-bit" multiple, so must do imul there
      if ((aNumber !== null && Math.abs(a) < TWO_TWENTY) || (bNumber !== null && Math.abs(b) < TWO_TWENTY) || (bits < 32 && !ASM_JS)) {
        return '(((' + a + ')*(' + b + '))&' + ((Math.pow(2, bits)-1)|0) + ')'; // keep a non-eliminatable coercion directly on this
      }
      return '(Math_imul(' + a + ',' + b + ')|0)';
    }
  } else if (op === '/') {
    if (a === '0' && !(type in Compiletime.FLOAT_TYPES)) { // careful on floats, since 0*NaN is not 0
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
      return op === '+' ? b : '(-' + b + ')';
    } else if (bNumber === 0) {
      return a;
    }
  }
  return '(' + a + ')' + op + '(' + b + ')';
}

function getFastValues(list, op, type) {
  assert(op === '+' && type === 'i32');
  for (var i = 0; i < list.length; i++) {
    if (isNumber(list[i])) list[i] = (list[i]|0) + '';
  }
  var changed = true;
  while (changed) {
    changed = false;
    for (var i = 0; i < list.length-1; i++) {
      var fast = getFastValue(list[i], op, list[i+1], type);
      var raw = list[i] + op + list[i+1];
      if (fast.length < raw.length || fast.indexOf(op) < 0) {
        if (isNumber(fast)) fast = (fast|0) + '';
        list[i] = fast;
        list.splice(i+1, 1);
        i--;
        changed = true;
        break;
      }
    }
  }
  if (list.length == 1) return list[0];
  return list.reduce(function(a, b) { return a + op + b });
}

function calcFastOffset(ptr, pos, noNeedFirst) {
  assert(!noNeedFirst);
  return getFastValue(ptr, '+', pos, 'i32');
}

var temp64f = new Float64Array(1);
var temp32f = new Float32Array(temp64f.buffer);
var temp32 = new Uint32Array(temp64f.buffer);
var temp16 = new Uint16Array(temp64f.buffer);
var temp8 = new Uint8Array(temp64f.buffer);
var memoryInitialization = [];

function writeInt8s(slab, i, value, type) {
  var currSize;
  switch (type) {
    case 'i1':
    case 'i8': temp8[0] = value;       currSize = 1; break;
    case 'i16': temp16[0] = value;     currSize = 2; break;
    case 'float': temp32f[0] = value;  currSize = 4; break;
    case 'double': temp64f[0] = value; currSize = 8; break;
    case 'i64': // fall through, i64 is two i32 chunks
    case 'i32': // fall through, i32 can be a pointer
    default: {
      if (type == 'i32' || type == 'i64' || type[type.length-1] == '*') {
        if (!isNumber(value)) { // function table stuff, etc.
          slab[i] = value;
          slab[i+1] = slab[i+2] = slab[i+3] = 0;
          return 4;
        }
        temp32[0] = value;
        currSize = 4;
      } else {
        throw 'what? ' + types[i];
      }
    }
  }
  for (var j = 0; j < currSize; j++) {
    slab[i+j] = temp8[j];
  }
  return currSize;
}

function makePointer(slab, pos, allocator, type, ptr, finalMemoryInitialization) {
  assert(type, 'makePointer requires type info');
  if (typeof slab == 'string' && (slab.substr(0, 4) === 'HEAP')) return pos;
  var types = generateStructTypes(type);
  if (typeof slab == 'object') {
    for (var i = 0; i < slab.length; i++) {
      var curr = slab[i];
      if (isNumber(curr)) {
        slab[i] = parseFloat(curr); // fix "5" to 5 etc.
      } else if (curr == 'undef') {
        slab[i] = 0;
      }
    }
  }
  // compress type info and data if possible
  if (!finalMemoryInitialization) {
    // XXX This heavily assumes the target endianness is the same as our current endianness! XXX
    var i = 0;
    while (i < slab.length) {
      var currType = types[i];
      if (!currType) { i++; continue }
      i += writeInt8s(slab, i, slab[i], currType);
    }
    types = 'i8';
  }
  if (allocator == 'ALLOC_NONE') {
    if (!finalMemoryInitialization) {
      // writing out into memory, without a normal allocation. We put all of these into a single big chunk.
      assert(typeof slab == 'object');
      assert(slab.length % QUANTUM_SIZE == 0, slab.length); // must be aligned already
      if (SIDE_MODULE && typeof ptr == 'string') {
        ptr = parseInt(ptr.substring(ptr.indexOf('+'), ptr.length-1)); // parse into (H_BASE+X)
      }
      var offset = ptr - Runtime.GLOBAL_BASE;
      for (var i = 0; i < slab.length; i++) {
        memoryInitialization[offset + i] = slab[i];
      }
      return '';
    }
    // This is the final memory initialization
    types = 'i8';
  }

  if (typeof types == 'object') {
    while (types.length < slab.length) types.push(0);
  }
  types = JSON.stringify(types);
  if (typeof slab == 'object') slab = '[' + slab.join(',') + ']';
  return 'allocate(' + slab + ', ' + types + (allocator ? ', ' + allocator : '') + (allocator == 'ALLOC_NONE' ? ', ' + ptr : '') + ');';
}

function makeGetSlabs(ptr, type, allowMultiple, unsigned) {
  assert(type);
  if (isPointerType(type)) type = 'i32'; // Hardcoded 32-bit
  switch(type) {
    case 'i1': case 'i8': return [unsigned ? 'HEAPU8' : 'HEAP8']; break;
    case 'i16': return [unsigned ? 'HEAPU16' : 'HEAP16']; break;
    case '<4 x i32>':
    case 'i32': case 'i64': return [unsigned ? 'HEAPU32' : 'HEAP32']; break;
    case 'double': return ['HEAPF64'];
    case '<4 x float>':
    case 'float': return ['HEAPF32'];
    default: {
      throw 'what, exactly, can we do for unknown types in TA2?! ' + [new Error().stack, ptr, type, allowMultiple, unsigned];
    }
  }
  return [];
}

function makeGetTempRet0() {
  return RELOCATABLE ? "(getTempRet0() | 0)" : "tempRet0";
}

function makeSetTempRet0(value) {
  return RELOCATABLE ? "setTempRet0((" + value + ") | 0)" : ("tempRet0 = " + value);
}

function makeStructuralReturn(values, inAsm) {
  var i = -1;
  return 'return ' + asmCoercion(values.slice(1).map(function(value) {
    i++;
    if (!inAsm) {
      if (!RELOCATABLE) {
        return 'asm["setTempRet' + i + '"](' + value + ')';
      } else {
        return 'Runtime.setTempRet' + i + '(' + value + ')';
      }
    }
    if (i === 0) {
      return makeSetTempRet0(value)
    } else {
      return 'tempRet' + i + ' = ' + value;
    }
  }).concat([values[0]]).join(','), 'i32');
}

function makeThrow(what) {
  return 'throw ' + what + (DISABLE_EXCEPTION_CATCHING == 1 ? ' + " - Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0 or DISABLE_EXCEPTION_CATCHING=2 to catch."' : '') + ';';
}

function makeSignOp(value, type, op, force, ignore) {
  if (type == 'i64') {
    return value; // these are always assumed to be two 32-bit unsigneds.
  }
  if (isPointerType(type)) type = 'i32'; // Pointers are treated as 32-bit ints
  if (!value) return value;
  var bits, full;
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
          return '(' + value +  '>>>0)';
        }
      } else if (bits < 32) {
        if (op === 're') {
          return '((' + value + '<<' + (32-bits) + ')>>' + (32-bits) + ')';
        } else {
          return '(' + value + '&' + (Math.pow(2, bits)-1) + ')';
        }
      } else { // bits > 32
        if (op === 're') {
          return makeInlineCalculation('VALUE >= ' + Math.pow(2, bits-1) + ' ? VALUE-' + Math.pow(2, bits) + ' : VALUE', value, 'tempBigIntS');
        } else {
          return makeInlineCalculation('VALUE >= 0 ? VALUE : ' + Math.pow(2, bits) + '+VALUE', value, 'tempBigIntS');
        }
      }
    }
    return full;
  }
  return value;
}

var legalizedI64s = true; // We do not legalize globals, but do legalize function lines. This will be true in the latter case

function stripCorrections(param) {
  var m;
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

function getImplementationType(varInfo) {
  if (varInfo.impl == 'nativized') {
    return removePointing(varInfo.type);
  }
  return varInfo.type;
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
    default: throw 'getTypeFromHeap? ' + suffix;
  }
}

function ensureValidFFIType(type) {
  return type === 'float' ? 'double' : type; // ffi does not tolerate float XXX
}

// FFI return values must arrive as doubles, and we can force them to floats afterwards
function asmFFICoercion(value, type) {
  value = asmCoercion(value, ensureValidFFIType(type));
  if (PRECISE_F32 && type === 'float') value = asmCoercion(value, 'float');
  return value;
}

function makeDynCall(sig) {
  if (!EMULATED_FUNCTION_POINTERS) {
    return 'dynCall_' + sig;
  } else {
    return 'ftCall_' + sig;
  }
}

function heapAndOffset(heap, ptr) { // given   HEAP8, ptr   , we return    splitChunk, relptr
  if (!SPLIT_MEMORY) return heap + ',' + ptr;
  return heap + 's[(' + ptr + ') >> SPLIT_MEMORY_BITS], (' + ptr + ') & SPLIT_MEMORY_MASK'; 
}

function makeEval(code) {
  if (NO_DYNAMIC_EXECUTION == 1) {
    // Treat eval as error.
    return "abort('NO_DYNAMIC_EXECUTION=1 was set, cannot eval');";
  }
  var ret = '';
  if (NO_DYNAMIC_EXECUTION == 2) {
    // Warn on evals, but proceed.
    ret += "Module.printErr('Warning: NO_DYNAMIC_EXECUTION=2 was set, but calling eval in the following location:');\n";
    ret += "Module.printErr(stackTrace());\n";
  }
  ret += code;
  return ret;
}

function makeStaticAlloc(size) {
  size = (size + (STACK_ALIGN-1)) & -STACK_ALIGN;
  return 'STATICTOP; STATICTOP += ' + size + ';';
}

