//"use strict";

// Various tools for parsing LLVM. Utilities of various sorts, that are
// specific to Emscripten (and hence not in utility.js).

// Does simple 'macro' substitution, using Django-like syntax,
// {{{ code }}} will be replaced with |eval(code)|.
function processMacros(text) {
  return text.replace(/{{{([^}]|}(?!}))+}}}/g, function(str) {
    str = str.substr(3, str.length-6);
    var ret = eval(str);
    return ret ? ret.toString() : '';
  });
}

// Simple #if/else/endif preprocessing for a file. Checks if the
// ident checked is true in our global.
function preprocess(text) {
  var lines = text.split('\n');
  var ret = '';
  var showStack = [];
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    if (line[line.length-1] == '\r') {
      line = line.substr(0, line.length-1); // Windows will have '\r' left over from splitting over '\r\n'
    }
    if (!line[0] || line[0] != '#') {
      if (showStack.indexOf(false) == -1) {
        ret += line + '\n';
      }
    } else {
      if (line[1] && line[1] == 'i') { // if
        var parts = line.split(' ');
        var ident = parts[1];
        var op = parts[2];
        var value = parts[3];
        if (op) {
          assert(op === '==')
          showStack.push(ident in this && this[ident] == value);
        } else {
          showStack.push(ident in this && this[ident] > 0);
        }
      } else if (line[2] && line[2] == 'l') { // else
        showStack.push(!showStack.pop());
      } else if (line[2] && line[2] == 'n') { // endif
        showStack.pop();
      } else {
        throw "Unclear preprocessor command: " + line;
      }
    }
  }
  assert(showStack.length == 0);
  return ret;
}

function addPointing(type) { return type + '*' }
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

function isJSVar(ident) {
  return /^\(?[$_]?[\w$_\d ]*\)+$/.test(ident);

}

function isLocalVar(ident) {
  return ident[0] == '$';
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
  return !Runtime.isNumberType(type) && type[0] == '%';
}

function isPointerType(type) {
  return type[type.length-1] == '*';
}

function isStructType(type) {
  if (isPointerType(type)) return false;
  if (/^\[\d+\ x\ (.*)\]/.test(type)) return true; // [15 x ?] blocks. Like structs
  if (/<?{ ?[^}]* ?}>?/.test(type)) return true; // { i32, i8 } etc. - anonymous struct types
  // See comment in isStructPointerType()
  return type[0] == '%';
}

function isStructuralType(type) {
  return /^{ ?[^}]* ?}$/.test(type); // { i32, i8 } etc. - anonymous struct types
}

function getStructuralTypeParts(type) { // split { i32, i8 } etc. into parts
  return type.replace(/[ {}]/g, '').split(',');
}

function getStructureTypeParts(type) {
  if (isStructuralType(type)) {
    return type.replace(/[ {}]/g, '').split(',');
  } else {
    var typeData = Types.types[type];
    assert(typeData, type);
    return typeData.fields;
  }
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
  var floats = +(type1 in Runtime.FLOAT_TYPES) + +(type2 in Runtime.FLOAT_TYPES);
  if (floats == 2) return true;
  if (floats == 1) return false;
  return getNumIntChunks(type1) == getNumIntChunks(type2);
}

function isIllegalType(type) {
  var bits = getBits(type);
  return bits > 0 && (bits >= 64 || !isPowerOfTwo(bits));
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
  if (!token.item) return false;
  var fail = false;
  var segments = splitTokenList(token.item.tokens);
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
  return isFunctionDef({ text: argText, item: tokenize(argText.substr(1, argText.length-2), true) }, out);
}

function getReturnType(type) {
  if (pointingLevels(type) > 1) return '*'; // the type of a call can be either the return value, or the entire function. ** or more means it is a return value
  var lastOpen = type.lastIndexOf('(');
  if (lastOpen > 0) {
    return type.substr(0, lastOpen-1);
  }
  return type;
}

var isTypeCache = {}; // quite hot, optimize as much as possible
function isType(type) {
  if (type in isTypeCache) return isTypeCache[type];
  var ret = isPointerType(type) || isVoidType(type) || Runtime.isNumberType(type) || isStructType(type) || isFunctionType(type);
  isTypeCache[type] = ret;
  return ret;
}

function isVarArgsFunctionType(type) {
  // assumes this is known to be a function type already
  var varArgsSuffix = '...)*';
  return type.substr(-varArgsSuffix.length) == varArgsSuffix;
}

function getNumVars(type) { // how many variables are needed to represent this type
  if (type in Runtime.FLOAT_TYPES) return 1;
  return Math.max(getNumIntChunks(type), 1);
}

function countNormalArgs(type, out) {
  out = out || {};
  if (!isFunctionType(type, out)) return -1;
  var ret = 0;
  if (out.segments) {
    for (var i = 0; i < out.segments.length; i++) {
      ret += getNumVars(out.segments[i][0].text);
    }
  }
  if (isVarArgsFunctionType(type)) ret--;
  return ret;
}

function addIdent(token) {
  token.ident = token.text;
  return token;
}

function combineTokens(tokens) {
  var ret = {
    lineNum: tokens[0].lineNum,
    text: '',
    tokens: []
  };
  tokens.forEach(function(token) {
    ret.text += token.text;
    ret.tokens.push(token);
  });
  return ret;
}

function compareTokens(a, b) {
  var aId = a.__uid__;
  var bId = b.__uid__;
  a.__uid__ = 0;
  b.__uid__ = 0;
  var ret = JSON.stringify(a) == JSON.stringify(b);
  a.__uid__ = aId;
  b.__uid__ = bId;
  return ret;
}

function getTokenIndexByText(tokens, text) {
  var i = 0;
  while (tokens[i] && tokens[i].text != text) i++;
  return i;
}

function findTokenText(item, text) {
  return findTokenTextAfter(item, text, 0);
}

function findTokenTextAfter(item, text, startAt) {
  for (var i = startAt; i < item.tokens.length; i++) {
    if (item.tokens[i].text == text) return i;
  }
  return -1;
}

var SPLIT_TOKEN_LIST_SPLITTERS = set(',', 'to'); // 'to' can separate parameters as well...

// Splits a list of tokens separated by commas. For example, a list of arguments in a function call
function splitTokenList(tokens) {
  if (tokens.length == 0) return [];
  if (!tokens.slice) tokens = tokens.tokens;
  if (tokens.slice(-1)[0].text != ',') tokens.push({text:','});
  var ret = [];
  var seg = [];
  for (var i = 0; i < tokens.length; i++) {
    var token = tokens[i];
    if (token.text in SPLIT_TOKEN_LIST_SPLITTERS) {
      ret.push(seg);
      seg = [];
    } else if (token.text == ';') {
      ret.push(seg);
      break;
    } else {
      seg.push(token);
    }
  }
  return ret;
}

function parseParamTokens(params) {
  if (params.length === 0) return [];
  var ret = [];
  if (params[params.length-1].text != ',') {
    params.push({ text: ',' });
  }
  var anonymousIndex = 0;
  while (params.length > 0) {
    var i = 0;
    while (params[i].text != ',') i++;
    var segment = params.slice(0, i);
    params = params.slice(i+1);
    segment = cleanSegment(segment);
    var byVal = 0;
    if (segment[1] && segment[1].text === 'byval') {
      // handle 'byval' and 'byval align X'. We store the alignment in 'byVal'
      byVal = QUANTUM_SIZE;
      segment.splice(1, 1);
      if (segment[1] && segment[1].text === 'align') {
        assert(isNumber(segment[2].text));
        byVal = parseInt(segment[2].text);
        segment.splice(1, 2);
      }
    }
    if (segment.length == 1) {
      if (segment[0].text == '...') {
        ret.push({
          intertype: 'varargs',
          type: 'i8*',
          ident: 'varrp' // the conventional name we have for this
        });
      } else {
        // Clang sometimes has a parameter with just a type,
        // no name... the name is implied to be %{the index}
        ret.push({
          intertype: 'value',
          type: segment[0].text,
          ident: toNiceIdent('%') + anonymousIndex
        });
        Types.needAnalysis[ret[ret.length-1].type] = 0;
        anonymousIndex ++;
      }
    } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
      ret.push(parseLLVMFunctionCall(segment));
    } else if (segment[1].text === 'blockaddress') {
      ret.push(parseBlockAddress(segment));
    } else {
      if (segment[2] && segment[2].text == 'to') { // part of bitcast params
        segment = segment.slice(0, 2);
      }
      while (segment.length > 2) {
        segment[0].text += segment[1].text;
        segment.splice(1, 1); // TODO: merge tokens nicely
      }
      ret.push({
        intertype: 'value',
        type: segment[0].text,
        ident: toNiceIdent(parseNumerical(segment[1].text, segment[0].text))
      });
      Types.needAnalysis[removeAllPointing(ret[ret.length-1].type)] = 0;
    }
    ret[ret.length-1].byVal = byVal;
  }
  return ret;
}

function hasVarArgs(params) {
  for (var i = 0; i < params.length; i++) {
    if (params[i].intertype == 'varargs') {
      return true;
    }
  }
  return false;
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

function makeGlobalDef(ident) {
  if (!NAMED_GLOBALS && isIndexableGlobal(ident)) return '';
  return 'var ' + ident + ';';
}

function makeGlobalUse(ident) {
  if (!NAMED_GLOBALS && isIndexableGlobal(ident)) {
    var index = Variables.indexedGlobals[ident];
    if (index === undefined) {
      // we are accessing this before we index globals, likely from the library. mark as unindexable
      UNINDEXABLE_GLOBALS[ident] = 1;
      return ident;
    }
    return (Runtime.GLOBAL_BASE + index).toString();
  }
  return ident;
}

function sortGlobals(globals) {
  var ks = keys(globals);
  ks.sort();
  var inv = invertArray(ks);
  return values(globals).sort(function(a, b) {
    return inv[b.ident] - inv[a.ident];
  });
}

function finalizeParam(param) {
  if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
    return finalizeLLVMFunctionCall(param);
  } else if (param.intertype === 'blockaddress') {
    return finalizeBlockAddress(param);
  } else if (param.intertype === 'jsvalue') {
    return param.ident;
  } else {
    if (param.type == 'i64' && USE_TYPED_ARRAYS == 2) {
      return parseI64Constant(param.ident);
    }
    var ret = toNiceIdent(param.ident);
    if (ret in Variables.globals) {
      ret = makeGlobalUse(ret);
    }
    return ret;
  }
}

// Segment ==> Parameter
function parseLLVMSegment(segment) {
  var type;
  if (segment.length == 1) {
    if (isType(segment[0].text)) {
      Types.needAnalysis[segment[0].text] = 0;
      return {
        intertype: 'type',
        ident: toNiceIdent(segment[0].text),
        type: segment[0].text
      };
    } else {
      return {
        intertype: 'value',
        ident: toNiceIdent(segment[0].text),
        type: 'i32'
      };
    }
  } else if (segment[1].type && segment[1].type == '{') {
    type = segment[0].text;
    Types.needAnalysis[type] = 0;
    return {
      intertype: 'structvalue',
      params: splitTokenList(segment[1].tokens).map(parseLLVMSegment),
      type: type
    };
  } else if (segment[0].text in PARSABLE_LLVM_FUNCTIONS) {
    return parseLLVMFunctionCall([{text: '?'}].concat(segment));
  } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
    return parseLLVMFunctionCall(segment);
  } else if (segment[1].text === 'blockaddress') {
    return parseBlockAddress(segment);
  } else {
    type = segment[0].text;
    Types.needAnalysis[type] = 0;
    return {
      intertype: 'value',
      ident: toNiceIdent(segment[1].text),
      type: type
    };
  }
}

function cleanSegment(segment) {
  while (segment.length >= 2 && ['noalias', 'sret', 'nocapture', 'nest', 'zeroext', 'signext'].indexOf(segment[1].text) != -1) {
    segment.splice(1, 1);
  }
  return segment;
}

var MATHOPS = set(['add', 'sub', 'sdiv', 'udiv', 'mul', 'icmp', 'zext', 'urem', 'srem', 'fadd', 'fsub', 'fmul', 'fdiv', 'fcmp', 'frem', 'uitofp', 'sitofp', 'fpext', 'fptrunc', 'fptoui', 'fptosi', 'trunc', 'sext', 'select', 'shl', 'shr', 'ashl', 'ashr', 'lshr', 'lshl', 'xor', 'or', 'and', 'ptrtoint', 'inttoptr']);

var PARSABLE_LLVM_FUNCTIONS = set('getelementptr', 'bitcast');
mergeInto(PARSABLE_LLVM_FUNCTIONS, MATHOPS);

// Parses a function call of form
//         TYPE functionname MODIFIERS (...)
// e.g.
//         i32* getelementptr inbounds (...)
function parseLLVMFunctionCall(segment) {
  segment = segment.slice(0);
  segment = cleanSegment(segment);
  // Remove additional modifiers
  var variant = null;
  if (!segment[2] || !segment[2].item) {
    variant = segment.splice(2, 1)[0];
    if (variant && variant.text) variant = variant.text; // needed for mathops
  }
  assertTrue(['inreg', 'byval'].indexOf(segment[1].text) == -1);
  assert(segment[1].text in PARSABLE_LLVM_FUNCTIONS);
  while (!segment[2].item) {
    segment.splice(2, 1); // Remove modifiers
    if (!segment[2]) throw 'Invalid segment!';
  }
  var intertype = segment[1].text;
  var type = segment[0].text;
  if (type === '?') {
    if (intertype === 'getelementptr') {
      type = '*'; // a pointer, we can easily say, this is
    } else if (segment[2].item.tokens.slice(-2)[0].text === 'to') {
      type = segment[2].item.tokens.slice(-1)[0].text;
    }
  }
  var ret = {
    intertype: intertype,
    variant: variant,
    type: type,
    params: parseParamTokens(segment[2].item.tokens)
  };
  Types.needAnalysis[ret.type] = 0;
  ret.ident = toNiceIdent(ret.params[0].ident || 'NOIDENT');
  return ret;
}

// Gets an array of tokens, we parse out the first
// 'ident' - either a simple ident of one token, or
// an LLVM internal function that generates an ident.
// We shift out of the array list the tokens that
// we ate.
function eatLLVMIdent(tokens) {
  var ret;
  if (tokens[0].text in PARSABLE_LLVM_FUNCTIONS) {
    var item = parseLLVMFunctionCall([{text: '?'}].concat(tokens.slice(0,2))); // TODO: Handle more cases, return a full object, process it later
    if (item.intertype == 'bitcast') checkBitcast(item);
    ret = item.ident;
    tokens.shift();
    tokens.shift();
  } else {
    ret = tokens[0].text;
    tokens.shift();
  }
  return ret;
}

function cleanOutTokens(filterOut, tokens, indexes) {
  if (typeof indexes !== 'object') indexes = [indexes];
  for (var i = indexes.length-1; i >=0; i--) {
    var index = indexes[i];
    while (index < tokens.length && tokens[index].text in filterOut) {
      tokens.splice(index, 1);
    }
  }
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
  if (FAKE_X86_FP80 && stringy.length > 16) {
    stringy = stringy.substr(stringy.length-16, 16);
    assert(TARGET_X86, 'must only see >64 bit floats in x86, as fp80s');
    warnOnce('.ll contains floating-point values with more than 64 bits. Faking values for them. If they are used, this will almost certainly break horribly!');
  }
  assert(stringy.length === 16, 'Can only unhex 16-digit double numbers, nothing platform-specific'); // |long double| can cause x86_fp80 which causes this
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
  if (USE_TYPED_ARRAYS == 2) {
    return '[' + makeSignOp(low, 'i32', 'un', 1, 1) + ',' + makeSignOp(high, 'i32', 'un', 1, 1) + ']';
  } else {
    if (high) return RuntimeGenerator.makeBigInt(low, high);
    return low;
  }
}

// XXX Make all i64 parts signed

// Splits a number (an integer in a double, possibly > 32 bits) into an USE_TYPED_ARRAYS == 2 i64 value.
// Will suffer from rounding. mergeI64 does the opposite.
function splitI64(value, floatConversion) {
  // We need to min here, since our input might be a double, and large values are rounded, so they can
  // be slightly higher than expected. And if we get 4294967296, that will turn into a 0 if put into a
  // HEAP32 or |0'd, etc.
  var lowInput = legalizedI64s ? value : 'VALUE';
  if (floatConversion && ASM_JS) lowInput = asmFloatToInt(lowInput);
  if (legalizedI64s) {
    return [lowInput + '>>>0', 'Math.min(Math.floor((' + value + ')/' + asmEnsureFloat(4294967296, 'float') + '), ' + asmEnsureFloat(4294967295, 'float') + ')>>>0'];
  } else {
    return makeInlineCalculation(makeI64(lowInput + '>>>0', 'Math.min(Math.floor(VALUE/' + asmEnsureFloat(4294967296, 'float') + '), ' + asmEnsureFloat(4294967295, 'float') + ')>>>0'), value, 'tempBigIntP');
  }
}
function mergeI64(value, unsigned) {
  assert(USE_TYPED_ARRAYS == 2);
  if (legalizedI64s) {
    return RuntimeGenerator.makeBigInt(value + '$0', value + '$1', unsigned);
  } else {
    return makeInlineCalculation(RuntimeGenerator.makeBigInt('VALUE[0]', 'VALUE[1]', unsigned), value, 'tempI64');
  }
}

// Takes an i64 value and changes it into the [low, high] form used in i64 mode 1. In that
// mode, this is a no-op
function ensureI64_1(value) {
  if (USE_TYPED_ARRAYS == 2) return value;
  return splitI64(value, 1);
}

function makeCopyI64(value) {
  assert(USE_TYPED_ARRAYS == 2);
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
  if ((!type || type == 'double' || type == 'float') && (value.substr && value.substr(0,2) == '0x')) {
    // Hexadecimal double value, as the llvm docs say,
    // "The one non-intuitive notation for constants is the hexadecimal form of floating point constants."
    value = IEEEUnHex(value);
  } else if (USE_TYPED_ARRAYS == 2 && isIllegalType(type)) {
    return value; // do not parseFloat etc., that can lead to loss of precision
  } else if (value == 'null') {
    // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)
    value = '0';
  } else if (value === 'true') {
    return '1';
  } else if (value === 'false') {
    return '0';
  }
  if (isNumber(value)) {
    var ret = parseFloat(value); // will change e.g. 5.000000e+01 to 50
    if (type in Runtime.FLOAT_TYPES && value[0] == '-' && ret === 0) return '-0'; // fix negative 0, toString makes it 0
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
    var chr = str[i];
    if (chr != '\\') {
      ret.push(chr.charCodeAt(0));
      i++;
    } else {
      ret.push(eval('0x' + str[i+1]+str[i+2]));
      i += 3;
    }
  }
  return ret;
}

function getLabelIds(labels) {
  return labels.map(function(label) { return label.ident });
}

function cleanLabel(label) {
  if (label[0] == 'B') {
    return label.substr(5);
  } else {
    return label;
  }
}

function getOldLabel(label) {
  var parts = label.split('|');
  return parts[parts.length-1];
}

function calcAllocatedSize(type) {
  if (pointingLevels(type) == 0 && isStructType(type)) {
    return Types.types[type].flatSize; // makeEmptyStruct(item.allocatedType).length;
  } else {
    return Runtime.getNativeTypeSize(type); // We can really get away with '1', though, at least on the stack...
  }
}

// Generates the type signature for a structure, for each byte, the type that is there.
// i32, 0, 0, 0 - for example, an int32 is here, then nothing to do for the 3 next bytes, naturally
function generateStructTypes(type) {
  if (isArray(type)) return type; // already in the form of [type, type,...]
  if (Runtime.isNumberType(type) || isPointerType(type)) {
    if (USE_TYPED_ARRAYS == 2 && type == 'i64') {
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
    var start = index;
    for (var i = 0; i < typeData.fields.length; i++) {
      var type = typeData.fields[i];
      if (!SAFE_HEAP && isPointerType(type)) type = '*'; // do not include unneeded type names without safe heap
      if (Runtime.isNumberType(type) || isPointerType(type)) {
        if (USE_TYPED_ARRAYS == 2 && type == 'i64') {
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
        add(Types.types[type]);
      }
      var more = (i+1 < typeData.fields.length ? typeData.flatIndexes[i+1] : typeData.flatSize) - (index - start);
      for (var j = 0; j < more; j++) {
        ret[index++] = 0;
      }
    }
  }
  add(typeData);
  assert(index == size);
  return ret;
}

// Flow blocks

function recurseBlock(block, func) {
  var ret = [];
  if (block.type == 'reloop') {
    ret.push(func(block.inner));
  } else if (block.type == 'multiple') {
    block.entryLabels.forEach(function(entryLabel) { ret.push(func(entryLabel.block)) });
  }
  ret.push(func(block.next));
  return ret;
}

function getActualLabelId(labelId) {
  return labelId.split('|').slice(-1)[0];
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

function correctSpecificSign() {
  if (!Framework.currItem) return false;
  if (Framework.currItem.funcData.ident.indexOf('emscripten_autodebug') >= 0) return 1; // always correct in the autodebugger code!
  return (CORRECT_SIGNS === 2 && Debugging.getIdentifier() in CORRECT_SIGNS_LINES) ||
         (CORRECT_SIGNS === 3 && !(Debugging.getIdentifier() in CORRECT_SIGNS_LINES));
}
function correctSigns() {
  return CORRECT_SIGNS === 1 || correctSpecificSign();
}

function correctSpecificOverflow() {
  if (!Framework.currItem) return false;
  return (CORRECT_OVERFLOWS === 2 && Debugging.getIdentifier() in CORRECT_OVERFLOWS_LINES) ||
         (CORRECT_OVERFLOWS === 3 && !(Debugging.getIdentifier() in CORRECT_OVERFLOWS_LINES));
}
function correctOverflows() {
  return CORRECT_OVERFLOWS === 1 || correctSpecificOverflow();
}

function correctSpecificRounding() {
  if (!Framework.currItem) return false;
  return (CORRECT_ROUNDINGS === 2 && Debugging.getIdentifier() in CORRECT_ROUNDINGS_LINES) ||
         (CORRECT_ROUNDINGS === 3 && !(Debugging.getIdentifier() in CORRECT_ROUNDINGS_LINES));
}
function correctRoundings() {
  return CORRECT_ROUNDINGS === 1 || correctSpecificRounding();
}

function checkSpecificSafeHeap() {
  if (!Framework.currItem) return false;
  return (SAFE_HEAP === 2 && Debugging.getIdentifier() in SAFE_HEAP_LINES) ||
         (SAFE_HEAP === 3 && !(Debugging.getIdentifier() in SAFE_HEAP_LINES));
}
function checkSafeHeap() {
  return SAFE_HEAP === 1 || checkSpecificSafeHeap();
}

function getHeapOffset(offset, type, forceAsm) {
  if (USE_TYPED_ARRAYS !== 2) {
    return offset;
  }

  if (Runtime.getNativeFieldSize(type) > 4) {
    if (type == 'i64' || TARGET_X86) {
      type = 'i32'; // XXX we emulate 64-bit values as 32 in x86, and also in le32 but only i64, not double
    }
  }

  var sz = Runtime.getNativeTypeSize(type);
  var shifts = Math.log(sz)/Math.LN2;
  offset = '(' + offset + ')';
  if (shifts != 0) {
    if (CHECK_HEAP_ALIGN) {
      return '(CHECK_ALIGN_' + sz + '(' + offset + '|0)>>' + shifts + ')';
    } else {
      return '(' + offset + '>>' + shifts + ')';
    }
  } else {
    // we need to guard against overflows here, HEAP[U]8 expects a guaranteed int
    return isJSVar(offset) ? offset : '(' + offset + '|0)';
  }
}

function makeVarDef(js) {
  if (!ASM_JS) js = 'var ' + js;
  return js;
}

function asmEnsureFloat(value, type) { // ensures that a float type has either 5.5 (clearly a float) or +5 (float due to asm coercion)
  if (!ASM_JS) return value;
  // coerce if missing a '.', or if smaller than 1, so could be 1e-5 which has no .
  if (type in Runtime.FLOAT_TYPES && isNumber(value) && (value.toString().indexOf('.') < 0 || Math.abs(value) < 1)) {
    return '(+(' + value + '))';
  } else {
    return value;
  }
}

function asmInitializer(type, impl) {
  if (type in Runtime.FLOAT_TYPES) {
    return '+0';
  } else {
    return '0';
  }
}

function asmCoercion(value, type, signedness) {
  if (!ASM_JS) return value;
  if (type == 'void') {
    return value;
  } else if (type in Runtime.FLOAT_TYPES) {
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
      return '(+(' + value + '))';
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
      ret.push('f' + i + ': ' + makeGetValue(ptr, pos + typeData.flatIndexes[i], typeData.fields[i], noNeedFirst, unsigned));
    }
    return '{ ' + ret.join(', ') + ' }';
  }

  // In double mode 1, in x86 we always assume unaligned because we can't trust that; otherwise in le32
  // we need this code path if we are not fully aligned.
  if (DOUBLE_MODE == 1 && USE_TYPED_ARRAYS == 2 && type == 'double' && (TARGET_X86 || align < 8)) {
    return '(' + makeSetTempDouble(0, 'i32', makeGetValue(ptr, pos, 'i32', noNeedFirst, unsigned, ignore, align)) + ',' +
                 makeSetTempDouble(1, 'i32', makeGetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'i32', noNeedFirst, unsigned, ignore, align)) + ',' +
            makeGetTempDouble(0, 'double') + ')';
  }

  if (USE_TYPED_ARRAYS == 2 && align) {
    // Alignment is important here. May need to split this up
    var bytes = Runtime.getNativeTypeSize(type);
    if (DOUBLE_MODE == 0 && type == 'double') bytes = 4; // we will really only read 4 bytes here
    if (bytes > align) {
      var ret = '(';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += makeGetValue(ptr, pos, 'i16', noNeedFirst, 2, ignore) + '|' +
                 '(' + makeGetValue(ptr, getFastValue(pos, '+', 2), 'i16', noNeedFirst, 2, ignore) + '<<16)';
        } else { // XXX we cannot truly handle > 4... (in x86)
          ret = '';
          for (var i = 0; i < bytes; i++) {
            ret += '(' + makeGetValue(ptr, getFastValue(pos, '+', i), 'i8', noNeedFirst, 1, ignore) + (i > 0 ? '<<' + (8*i) : '') + ')';
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
    return asmCoercion('SAFE_HEAP_LOAD(' + asmCoercion(offset, 'i32') + ', ' + (ASM_JS ? 0 : printType) + ', ' + (!!unsigned+0) + ', ' + ((!checkSafeHeap() || ignore)|0) + ')', type);
  } else {
    var ret = makeGetSlabs(ptr, type, false, unsigned)[0] + '[' + getHeapOffset(offset, type, forceAsm) + ']';
    if (ASM_JS && (phase == 'funcs' || forceAsm)) {
      ret = asmCoercion(ret, type);
    }
    if (ASM_HEAP_LOG) {
      ret = makeInlineCalculation('(asmPrint' + (type in Runtime.FLOAT_TYPES ? 'Float' : 'Int') + '(' + (asmPrintCounter++) + ',' + asmCoercion('VALUE', type) + '), VALUE)', ret,
                                  'temp' + (type in Runtime.FLOAT_TYPES ? 'Double' : 'Int'));
    }
    return ret;
  }
}

function makeGetValueAsm(ptr, pos, type, unsigned) {
  return makeGetValue(ptr, pos, type, null, unsigned, null, null, null, true);
}

function indexizeFunctions(value, type) {
  assert((type && type !== '?') || (typeof value === 'string' && value.substr(0, 6) === 'CHECK_'), 'No type given for function indexizing');
  assert(value !== type, 'Type set to value');
  var out = {};
  if (type && isFunctionType(type, out) && value[0] === '_') { // checking for _ differentiates from $ (local vars)
    // add signature to library functions that we now know need indexing
    if (!(value in Functions.implementedFunctions) && !(value in Functions.unimplementedFunctions)) {
      Functions.unimplementedFunctions[value] = Functions.getSignature(out.returnType, out.segments ? out.segments.map(function(segment) { return segment[0].text }) : []);
    }

    if (BUILD_AS_SHARED_LIB) {
      return '(FUNCTION_TABLE_OFFSET + ' + Functions.getIndex(value) + ')';
    } else {
      return Functions.getIndex(value);
    }
  }
  return value;
}

//! @param ptr The pointer. Used to find both the slab and the offset in that slab. If the pointer
//!            is just an integer, then this is almost redundant, but in general the pointer type
//!            may in the future include information about which slab as well. So, for now it is
//!            possible to put |0| here, but if a pointer is available, that is more future-proof.
//! @param pos The position in that slab - the offset. Added to any offset in the pointer itself.
//! @param value The value to set.
//! @param type A string defining the type. Used to find the slab (IHEAP, FHEAP, etc.).
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
      ret.push(makeSetValue(ptr, getFastValue(pos, '+', typeData.flatIndexes[i]), value[i], typeData.fields[i], noNeedFirst));
    }
    return ret.join('; ');
  }

  if (DOUBLE_MODE == 1 && USE_TYPED_ARRAYS == 2 && type == 'double' && (TARGET_X86 || align < 8)) {
    return '(' + makeSetTempDouble(0, 'double', value) + ',' +
            makeSetValue(ptr, pos, makeGetTempDouble(0, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), makeGetTempDouble(1, 'i32'), 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  } else if (USE_TYPED_ARRAYS == 2 && type == 'i64') {
    return '(tempI64 = [' + splitI64(value) + '],' +
            makeSetValue(ptr, pos, 'tempI64[0]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ',' +
            makeSetValue(ptr, getFastValue(pos, '+', Runtime.getNativeTypeSize('i32')), 'tempI64[1]', 'i32', noNeedFirst, ignore, align, noSafe, ',') + ')';
  }

  var bits = getBits(type);
  var needSplitting = bits > 0 && !isPowerOfTwo(bits); // an unnatural type like i24
  if (USE_TYPED_ARRAYS == 2 && (align || needSplitting)) {
    // Alignment is important here, or we need to split this up for other reasons.
    var bytes = Runtime.getNativeTypeSize(type);
    if (DOUBLE_MODE == 0 && type == 'double') bytes = 4; // we will really only read 4 bytes here
    if (bytes > align || needSplitting) {
      var ret = '';
      if (isIntImplemented(type)) {
        if (bytes == 4 && align == 2) {
          // Special case that we can optimize
          ret += 'tempBigInt=' + value + sep;
          ret += makeSetValue(ptr, pos, 'tempBigInt&0xffff', 'i16', noNeedFirst, ignore, 2) + sep;
          ret += makeSetValue(ptr, getFastValue(pos, '+', 2), 'tempBigInt>>16', 'i16', noNeedFirst, ignore, 2);
        } else {
          ret += 'tempBigInt=' + value + sep;
          for (var i = 0; i < bytes; i++) {
            ret += makeSetValue(ptr, getFastValue(pos, '+', i), 'tempBigInt&0xff', 'i8', noNeedFirst, ignore, 1);
            if (i < bytes-1) ret += sep + 'tempBigInt = tempBigInt>>8' + sep;
          }
        }
      } else {
        ret += makeSetValue('tempDoublePtr', 0, value, type, noNeedFirst, ignore, 8, null, null, true) + sep;
        ret += makeCopyValues(getFastValue(ptr, '+', pos), 'tempDoublePtr', Runtime.getNativeTypeSize(type), type, null, align, sep);
      }
      return ret;
    }
  }

  value = indexizeFunctions(value, type);
  var offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP && !noSafe) {
    var printType = type;
    if (printType !== 'null' && printType[0] !== '#') printType = '"' + safeQuote(printType) + '"';
    if (printType[0] === '#') printType = printType.substr(1);
    return 'SAFE_HEAP_STORE(' + asmCoercion(offset, 'i32') + ', ' + asmCoercion(value, type) + ', ' + (ASM_JS ? 0 : printType) + ', ' + ((!checkSafeHeap() || ignore)|0) + ')';
  } else {
    return makeGetSlabs(ptr, type, true).map(function(slab) { return slab + '[' + getHeapOffset(offset, type, forceAsm) + ']=' + value }).join(sep);
  }
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
  if (USE_TYPED_ARRAYS <= 1) {
    if (isNumber(num) && parseInt(num) <= UNROLL_LOOP_MAX) {
      return unroll(type, num);
    }
    return 'for (var $$dest = ' + getFastValue(ptr, '+', pos) + ', $$stop = $$dest + ' + num + '; $$dest < $$stop; $$dest++) {\n' +
      makeSetValue('$$dest', '0', value, type) + '\n}';
  } else { // USE_TYPED_ARRAYS == 2
    // If we don't know how to handle this at compile-time, or handling it is best done in a large amount of code, call memset
    // TODO: optimize the case of numeric num but non-numeric value
    if (!isNumber(num) || !isNumber(value) || (parseInt(num)/align >= UNROLL_LOOP_MAX)) {
      return '_memset(' + asmCoercion(getFastValue(ptr, '+', pos), 'i32') + ', ' + asmCoercion(value, 'i32') + ', ' + asmCoercion(num, 'i32') + ')';
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
}

var TYPED_ARRAY_SET_MIN = Infinity; // .set() as memcpy seems to just slow us down

function makeCopyValues(dest, src, num, type, modifier, align, sep) {
  sep = sep || ';';
  function unroll(type, num, jump) {
    jump = jump || 1;
    return range(num).map(function(i) {
      if (USE_TYPED_ARRAYS <= 1 && type === 'null') {
        // Null is special-cased: We copy over all heaps
        return makeGetSlabs(dest, 'null', true).map(function(slab) {
          return slab + '[' + getFastValue(dest, '+', i) + ']=' + slab + '[' + getFastValue(src, '+', i) + ']';
        }).join(sep) + (SAFE_HEAP ? sep + 'SAFE_HEAP_COPY_HISTORY(' + getFastValue(dest, '+', i) + ', ' +  getFastValue(src, '+', i) + ')' : '');
      } else {
        return makeSetValue(dest, i*jump, makeGetValue(src, i*jump, type), type);
      }
    }).join(sep);
  }
  if (USE_TYPED_ARRAYS <= 1) {
    if (isNumber(num) && parseInt(num) <= UNROLL_LOOP_MAX) {
      return unroll(type, num);
    }
    var oldDest = dest, oldSrc = src;
    dest = '$$dest';
    src = '$$src';
    return 'for (var $$src = ' + oldSrc + ', $$dest = ' + oldDest + ', $$stop = $$src + ' + num + '; $$src < $$stop; $$src++, $$dest++) {\n' +
            unroll(type, 1) + ' }';
  } else { // USE_TYPED_ARRAYS == 2
    // If we don't know how to handle this at compile-time, or handling it is best done in a large amount of code, call memset
    if (!isNumber(num)) num = stripCorrections(num);
    if (!isNumber(align)) align = stripCorrections(align);
    if (!isNumber(num) || (parseInt(num)/align >= UNROLL_LOOP_MAX)) {
      return '_memcpy(' + dest + ', ' + src + ', ' + num + ')';
    }
    num = parseInt(num);
    if (ASM_JS) {
      dest = stripCorrections(dest); // remove corrections, since we will be correcting after we add anyhow,
      src = stripCorrections(src);   // and in the heap assignment expression
    }
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
}

function makeHEAPView(which, start, end) {
  // Assumes USE_TYPED_ARRAYS == 2
  var size = parseInt(which.replace('U', '').replace('F', ''))/8;
  var mod = size == 1 ? '' : ('>>' + log2(size));
  return 'HEAP' + which + '.subarray((' + start + ')' + mod + ',(' + end + ')' + mod + ')';
}

var PLUS_MUL = set('+', '*');
var MUL_DIV = set('*', '/');
var PLUS_MINUS = set('+', '-');
var TWO_TWENTY = Math.pow(2, 20);

// Given two values and an operation, returns the result of that operation.
// Tries to do as much as possible at compile time.
// Leaves overflows etc. unhandled, *except* for integer multiply, in order to be efficient with Math.imul
function getFastValue(a, op, b, type) {
  a = a.toString();
  b = b.toString();
  a = a == 'true' ? '1' : (a == 'false' ? '0' : a);
  b = b == 'true' ? '1' : (b == 'false' ? '0' : b);
  if (isNumber(a) && isNumber(b)) {
    if (op == 'pow') {
      return Math.pow(a, b).toString();
    } else {
      var value = eval(a + op + '(' + b + ')'); // parens protect us from "5 - -12" being seen as "5--12" which is "(5--)12"
      if (op == '/' && type in Runtime.INT_TYPES) value = value|0; // avoid emitting floats
      return value.toString();
    }
  }
  if (op == 'pow') {
    if (a == '2' && isIntImplemented(type)) {
      return '(1 << (' + b + '))';
    }
    return 'Math.pow(' + a + ', ' + b + ')';
  }
  if (op in PLUS_MUL && isNumber(a)) { // if one of them is a number, keep it last
    var c = b;
    b = a;
    a = c;
  }
  if (op in MUL_DIV) {
    if (op == '*') {
      if (a == 0 || b == 0) {
        return '0';
      } else if (a == 1) {
        return b;
      } else if (b == 1) {
        return a;
      } else if (isNumber(b) && type && isIntImplemented(type) && Runtime.getNativeTypeSize(type) <= 32) {
        var shifts = Math.log(parseFloat(b))/Math.LN2;
        if (shifts % 1 == 0) {
          return '(' + a + '<<' + shifts + ')';
        }
      }
      if (!(type in Runtime.FLOAT_TYPES)) {
        // if guaranteed small enough to not overflow into a double, do a normal multiply
        var bits = getBits(type) || 32; // default is 32-bit multiply for things like getelementptr indexes
        // Note that we can emit simple multiple in non-asm.js mode, but asm.js will not parse "16-bit" multiple, so must do imul there
        if ((isNumber(a) && Math.abs(a) < TWO_TWENTY) || (isNumber(b) && Math.abs(b) < TWO_TWENTY) || (bits < 32 && !ASM_JS)) {
          return '(((' + a + ')*(' + b + '))&' + ((Math.pow(2, bits)-1)|0) + ')'; // keep a non-eliminatable coercion directly on this
        }
        return 'Math.imul(' + a + ',' + b + ')';
      }
    } else {
      if (a == '0') {
        return '0';
      } else if (b == 1) {
        return a;
      } // Doing shifts for division is problematic, as getting the rounding right on negatives is tricky
    }
  } else if (op in PLUS_MINUS) {
    if (b[0] == '-') {
      op = op == '+' ? '-' : '+';
      b = b.substr(1);
    }
    if (a == 0) {
      return op == '+' ? b : '(-' + b + ')';
    } else if (b == 0) {
      return a;
    }
  }
  return '(' + a + ')' + op + '(' + b + ')';
}

function getFastValues(list, op, type) {
  assert(op == '+');
  var changed = true;
  while (changed) {
    changed = false;
    for (var i = 0; i < list.length-1; i++) {
      var fast = getFastValue(list[i], op, list[i+1], type);
      var raw = list[i] + op + list[i+1];
      if (fast.length < raw.length || fast.indexOf(op) < 0) {
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
  var offset = noNeedFirst ? '0' : makeGetPos(ptr);
  return getFastValue(offset, '+', pos, 'i32');
}

function makeGetPos(ptr) {
  return ptr;
}

var IHEAP_FHEAP = set('IHEAP', 'IHEAPU', 'FHEAP');

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
  if (typeof slab == 'string' && (slab.substr(0, 4) === 'HEAP' || (USE_TYPED_ARRAYS == 1 && slab in IHEAP_FHEAP))) return pos;
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
  if (USE_TYPED_ARRAYS != 2) {
    var de;
    try {
      // compress all-zeros into a number (which will become zeros(..)).
      // note that we cannot always eval the slab, e.g., if it contains ident,0,0 etc. In that case, no compression TODO: ensure we get arrays here, not str
      var evaled = typeof slab === 'string' ? eval(slab) : slab;
      de = dedup(evaled);
      if (de.length === 1 && de[0] == 0) {
        slab = types.length;
      }
      // TODO: if not all zeros, at least filter out items with type === 0. requires cleverness to know how to skip at runtime though. also
      //       be careful of structure padding
    } catch(e){}
    de = dedup(types);
    if (de.length === 1) {
      types = de[0];
    } else if (de.length === 2 && typeof slab === 'number') {
      // If slab is all zeros, we can compress types even if we have i32,0,0,0,i32,0,0,0 etc. - we do not need the zeros
      de = de.filter(function(x) { return x !== 0 });
      if (de.length === 1) {
        types = de[0];
      }
    }
  } else { // USE_TYPED_ARRAYS == 2
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
  }
  if (allocator == 'ALLOC_NONE' && USE_TYPED_ARRAYS == 2) {
    if (!finalMemoryInitialization) {
      // writing out into memory, without a normal allocation. We put all of these into a single big chunk.
      assert(typeof slab == 'object');
      assert(slab.length % QUANTUM_SIZE == 0, slab.length); // must be aligned already
      var offset = ptr - Runtime.GLOBAL_BASE;
      for (var i = 0; i < slab.length; i++) {
        memoryInitialization[offset + i] = slab[i];
      }
      return '';
    }
    // This is the final memory initialization
    types = 'i8';
  }

  // JS engines sometimes say array initializers are too large. Work around that by chunking and calling concat to combine at runtime
  var chunkSize = JS_CHUNK_SIZE;
  function chunkify(array) {
    // break very large slabs into parts
    var ret = '';
    var index = 0;
    while (index < array.length) {
      ret = (ret ? ret + '.concat(' : '') + '[' + array.slice(index, index + chunkSize).map(JSON.stringify) + ']' + (ret ? ')\n' : '');
      index += chunkSize;
    }
    return ret;
  }
  if (typeof slab == 'object' && slab.length > chunkSize) {
    slab = chunkify(slab);
  }
  if (typeof types == 'object') {
    while (types.length < slab.length) types.push(0);
  }
  if (typeof types != 'string' && types.length > chunkSize) {
    types = chunkify(types);
  } else {
    types = JSON.stringify(types);
  }
  if (typeof slab == 'object') slab = '[' + slab.join(',') + ']';
  return 'allocate(' + slab + ', ' + types + (allocator ? ', ' + allocator : '') + (allocator == 'ALLOC_NONE' ? ', ' + ptr : '') + ')';
}

function makeGetSlabs(ptr, type, allowMultiple, unsigned) {
  assert(type);
  if (!USE_TYPED_ARRAYS) {
    return ['HEAP'];
  } else if (USE_TYPED_ARRAYS == 1) {
    if (type in Runtime.FLOAT_TYPES || type === 'int64') { // XXX should be i64, no?
      return ['FHEAP']; // If USE_FHEAP is false, will fail at runtime. At compiletime we do need it for library stuff.
    } else if (type in Runtime.INT_TYPES || isPointerType(type)) {
      return [unsigned ? 'IHEAPU' : 'IHEAP'];
    } else {
      assert(allowMultiple, 'Unknown slab type and !allowMultiple: ' + type);
      if (USE_FHEAP) {
        return ['IHEAP', 'FHEAP']; // unknown, so assign to both typed arrays
      } else {
        return ['IHEAP'];
      }
    }
  } else { // USE_TYPED_ARRAYS == 2)
    if (isPointerType(type)) type = 'i32'; // Hardcoded 32-bit
    switch(type) {
      case 'i1': case 'i8': return [unsigned ? 'HEAPU8' : 'HEAP8']; break;
      case 'i16': return [unsigned ? 'HEAPU16' : 'HEAP16']; break;
      case 'i32': case 'i64': return [unsigned ? 'HEAPU32' : 'HEAP32']; break;
      case 'double': {
        if (TARGET_LE32) return ['HEAPF64']; // in le32, we do have the ability to assume 64-bit alignment
        // otherwise, fall through to float
      }
      case 'float': return ['HEAPF32'];
      default: {
        throw 'what, exactly, can we do for unknown types in TA2?! ' + new Error().stack;
      }
    }
  }
  return [];
}

function checkBitcast(item) {
  // Warn about some types of casts, then fall through to the handling code below
  var oldType = item.params[0].type;
  var newType = item.type;
  if (isPossiblyFunctionType(oldType) && isPossiblyFunctionType(newType)) {
    var oldInfo = {}, newInfo = {};
    var oldCount = countNormalArgs(oldType, oldInfo);
    var newCount = countNormalArgs(newType, newInfo);
    var warned = false;
    function showWarning() {
      if (warned) return;
      warned = true;
      if (VERBOSE) {
        warnOnce('Casting potentially incompatible function pointer ' + oldType + ' to ' + newType + ', for ' + item.params[0].ident.slice(1));
      } else {
        warnOnce('Casting a function pointer type to a potentially incompatible one (use -s VERBOSE=1 to see more)');
      }
      warnOnce('See https://github.com/kripken/emscripten/wiki/CodeGuidlinesAndLimitations#function-pointer-issues for more information on dangerous function pointer casts');
      if (ASM_JS) warnOnce('Incompatible function pointer casts are very dangerous with ASM_JS=1, you should investigate and correct these');
    }
    if (oldCount != newCount && oldCount && newCount) showWarning();
    if (ASM_JS) {
      if (oldCount != newCount) showWarning();
      else if (!isIdenticallyImplemented(oldInfo.returnType, newInfo.returnType)) {
        showWarning();
      } else {
        for (var i = 0; i < oldCount; i++) {
          if (!isIdenticallyImplemented(oldInfo.segments[i][0].text, newInfo.segments[i][0].text)) {
            showWarning();
            break;
          }
        }
      }
    }
  }
}

function finalizeLLVMFunctionCall(item, noIndexizeFunctions) {
  if (item.intertype == 'getelementptr') { // TODO finalizeLLVMParameter on the ident and the indexes?
    return makePointer(makeGetSlabs(item.ident, item.type)[0], getGetElementPtrIndexes(item), null, item.type);
  }
  if (item.intertype == 'bitcast') checkBitcast(item);
  var temp = {
    op: item.intertype,
    variant: item.variant,
    type: item.type,
    params: item.params.slice(0) // XXX slice?
  };
  return processMathop(temp);
}

function getGetElementPtrIndexes(item) {
  var type = item.params[0].type;
  if (USE_TYPED_ARRAYS == 2) {
    // GEP indexes are marked as i64s, but they are just numbers to us
    item.params.forEach(function(param) { param.type = 'i32' });
  }
  item.params = item.params.map(finalizeLLVMParameter);
  var ident = item.params[0];

  // struct pointer, struct*, and getting a ptr to an element in that struct. Param 1 is which struct, then we have items in that
  // struct, and possibly further substructures, all embedded
  // can also be to 'blocks': [8 x i32]*, not just structs
  type = removePointing(type);
  var indexes = [makeGetPos(ident)];
  var offset = item.params[1];
  if (offset != 0) {
    if (isStructType(type)) {
      indexes.push(getFastValue(Types.types[type].flatSize, '*', offset, 'i32'));
    } else {
      indexes.push(getFastValue(Runtime.getNativeTypeSize(type), '*', offset, 'i32'));
    }
  }
  item.params.slice(2, item.params.length).forEach(function(arg) {
    var curr = arg;
    // TODO: If index is constant, optimize
    var typeData = Types.types[type];
    if (isStructType(type) && typeData.needsFlattening) {
      if (typeData.flatFactor) {
        indexes.push(getFastValue(curr, '*', typeData.flatFactor, 'i32'));
      } else {
        if (isNumber(curr)) {
          indexes.push(typeData.flatIndexes[curr]);
        } else {
          indexes.push(toNiceIdent(type) + '___FLATTENER[' + curr + ']'); // TODO: If curr is constant, optimize out the flattener struct
        }
      }
    } else {
      if (curr != 0) {
        indexes.push(curr);
      }
    }
    if (!isNumber(curr) || parseInt(curr) < 0) {
      // We have a *variable* to index with, or a negative number. In both
      // cases, in theory we might need to do something dynamic here. FIXME?
      // But, most likely all the possible types are the same, so do that case here now...
      for (var i = 1; i < typeData.fields.length; i++) {
        assert(typeData.fields[0] === typeData.fields[i]);
      }
      curr = 0;
    }
    type = typeData && typeData.fields[curr] ? typeData.fields[curr] : '';
  });

  var ret = getFastValues(indexes, '+', 'i32');

  ret = handleOverflow(ret, 32); // XXX - we assume a 32-bit arch here. If you fail on this, change to 64

  return ret;
}

function handleOverflow(text, bits) {
  // TODO: handle overflows of i64s
  if (!bits) return text;
  var correct = correctOverflows();
  warnOnce(!correct || bits <= 32, 'Cannot correct overflows of this many bits: ' + bits);
  if (CHECK_OVERFLOWS) return 'CHECK_OVERFLOW(' + text + ', ' + bits + ', ' + Math.floor(correctSpecificOverflow()) + ')';
  if (!correct) return text;
  if (bits == 32) {
    return '((' + text + ')|0)';
  } else if (bits < 32) {
    return '((' + text + ')&' + (Math.pow(2, bits) - 1) + ')';
  } else {
    return text; // We warned about this earlier
  }
}

function makeLLVMStruct(values) {
  if (USE_TYPED_ARRAYS == 2) {
    return 'DEPRECATED' + (new Error().stack) + 'XXX';
  } else {
    return '{ ' + values.map(function(value, i) { return 'f' + i + ': ' + value }).join(', ') + ' }'
  }
}

function makeStructuralReturn(values, inAsm) {
  if (USE_TYPED_ARRAYS == 2) {
    var i = -1;
    return 'return ' + asmCoercion(values.slice(1).map(function(value) {
      i++;
      return ASM_JS ? (inAsm ? 'tempRet' + i + ' = ' + value : 'asm["setTempRet' + i + '"](' + value + ')')
                    : 'tempRet' + i + ' = ' + value;
    }).concat([values[0]]).join(','), 'i32');
  } else {
    var i = 0;
    return 'return { ' + values.map(function(value) {
      return 'f' + (i++) + ': ' + value;
    }).join(', ') + ' }';
  }
}

function makeStructuralAccess(ident, i) {
  if (USE_TYPED_ARRAYS == 2) {
    return ident + '$' + i;
  } else {
    return ident + '.f' + i;
  }
}

function makeThrow(what) {
  return 'throw ' + what + (DISABLE_EXCEPTION_CATCHING == 1 ? ' + " - Exception catching is disabled, this exception cannot be caught. Compile with -s DISABLE_EXCEPTION_CATCHING=0 or DISABLE_EXCEPTION_CATCHING=2 to catch."' : '') + ';';
}

// From parseLLVMSegment
function finalizeLLVMParameter(param, noIndexizeFunctions) {
  var ret;
  if (isNumber(param)) {
    return param;
  } else if (typeof param === 'string') {
    return toNiceIdentCarefully(param);
  } else if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
    ret = finalizeLLVMFunctionCall(param, noIndexizeFunctions);
  } else if (param.ident == 'zeroinitializer') {
    if (isStructType(param.type)) {
      return makeLLVMStruct(zeros(Types.types[param.type].fields.length));
    } else {
      return '0';
    }
  } else if (param.intertype == 'value') {
    ret = param.ident;
    if (ret in Variables.globals) {
      ret = makeGlobalUse(ret);
    }
    if (param.type == 'i64' && USE_TYPED_ARRAYS == 2) {
      ret = parseI64Constant(ret);
    }
    ret = parseNumerical(ret, param.type);
    ret = asmEnsureFloat(ret, param.type);
  } else if (param.intertype == 'structvalue') {
    ret = makeLLVMStruct(param.params.map(function(value) { return finalizeLLVMParameter(value, noIndexizeFunctions) }));
  } else if (param.intertype === 'blockaddress') {
    return finalizeBlockAddress(param);
  } else if (param.intertype === 'type') {
    return param.ident; // we don't really want the type here
  } else if (param.intertype == 'mathop') {
    return processMathop(param);
  } else {
    throw 'invalid llvm parameter: ' + param.intertype;
  }
  assert(param.type || (typeof param === 'string' && param.substr(0, 6) === 'CHECK_'), 'Missing type for param!');
  if (!noIndexizeFunctions) ret = indexizeFunctions(ret, param.type);
  return ret;
}

function makeComparison(a, op, b, type) {
  assert(type);
  if (!isIllegalType(type)) {
    return asmCoercion(a, type) + op + asmCoercion(b, type);
  } else {
    assert(type == 'i64');
    return asmCoercion(a + '$0', 'i32') + op + asmCoercion(b + '$0', 'i32') + ' & ' +
           asmCoercion(a + '$1', 'i32') + op + asmCoercion(b + '$1', 'i32');
  }
}

function makeSignOp(value, type, op, force, ignore) {
  if (USE_TYPED_ARRAYS == 2 && type == 'i64') {
    return value; // these are always assumed to be two 32-bit unsigneds.
  }

  if (isPointerType(type)) type = 'i32'; // Pointers are treated as 32-bit ints
  if (!value) return value;
  var bits, full;
  if (type in Runtime.INT_TYPES) {
    bits = parseInt(type.substr(1));
    full = op + 'Sign(' + value + ', ' + bits + ', ' + Math.floor(ignore || (correctSpecificSign())) + ')';
    // Always sign/unsign constants at compile time, regardless of CHECK/CORRECT
    if (isNumber(value)) {
      return eval(full).toString();
    }
  }
  if ((ignore || !correctSigns()) && !CHECK_SIGNS && !force) return value;
  if (type in Runtime.INT_TYPES) {
    // shortcuts
    if (!CHECK_SIGNS || ignore) {
      if (bits === 32) {
        if (op === 're') {
          return '(' + getFastValue(value, '|', '0') + ')';
        } else {

          return '(' + getFastValue(value, '>>>', '0') + ')';
          // Alternatively, we can consider the lengthier
          //    return makeInlineCalculation('VALUE >= 0 ? VALUE : ' + Math.pow(2, bits) + ' + VALUE', value, 'tempBigInt');
          // which does not always turn us into a 32-bit *un*signed value
        }
      } else if (bits < 32) {
        if (op === 're') {
          return makeInlineCalculation('(VALUE << ' + (32-bits) + ') >> ' + (32-bits), value, 'tempInt');
        } else {
          return '(' + getFastValue(value, '&', Math.pow(2, bits)-1) + ')';
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

// @param floatConversion Means that we are receiving a float and rounding it to
//                        an integer. We must be careful here, the input has *not*
//                        already been converted to a signed/unsigned value (that
//                        would already do rounding, before us!)
function makeRounding(value, bits, signed, floatConversion) {
  // TODO: handle roundings of i64s
  assert(bits);
  if (!ASM_JS) {
    // C rounds to 0 (-5.5 to -5, +5.5 to 5), while JS has no direct way to do that.
    if (bits <= 32 && signed) return '((' + value + ')&-1)'; // This is fast and even correct, for all cases. Note that it is the same
                                                             // as |0, but &-1 hints to the js optimizer that this is a rounding correction
    // Do Math.floor, which is reasonably fast, if we either don't care, or if we can be sure
    // the value is non-negative
    if (!correctRoundings() || (!signed && !floatConversion)) return 'Math.floor(' + value + ')';
    // We are left with >32 bits signed, or a float conversion. Check and correct inline
    // Note that if converting a float, we may have the wrong sign at this point! But, we have
    // been rounded properly regardless, and we will be sign-corrected later when actually used, if
    // necessary.
    return makeInlineCalculation(makeComparison('VALUE', '>=', '0', 'float') + ' ? Math.floor(VALUE) : Math.ceil(VALUE)', value, 'tempBigIntR');
  } else {
    // asm.js mode, cleaner refactoring of this function as well. TODO: use in non-asm case, most of this
    if (floatConversion && bits <= 32) {
      return '(~~(' + value + '))'; // explicit float-to-int conversion
    }

    if (bits <= 32) {
      if (signed) {
        return '((' + value + ')&-1)'; // &-1 (instead of |0) hints to the js optimizer that this is a rounding correction
      } else {
        return '((' + value + ')>>>0)';
      }
    }
    // Math.floor is reasonably fast if we don't care about corrections (and even correct if unsigned)
    if (!correctRoundings() || !signed) return 'Math.floor(' + value + ')';
    // We are left with >32 bits
    return makeInlineCalculation(makeComparison('VALUE', '>=', '0', 'float') + ' ? Math.floor(VALUE) : Math.ceil(VALUE)', value, 'tempBigIntR');
  }
}

function makeIsNaN(value) {
  if (ASM_JS) return makeInlineCalculation('((VALUE) != (VALUE))', value, 'tempDouble');
  return 'isNaN(' + value + ')';
}

// fptoui and fptosi are not in these, because we need to be careful about what we do there. We can't
// just sign/unsign the input first.
var UNSIGNED_OP = set('udiv', 'urem', 'uitofp', 'zext', 'lshr');
var SIGNED_OP = set('sdiv', 'srem', 'sitofp', 'sext', 'ashr');

function isUnsignedOp(op, variant) {
  return op in UNSIGNED_OP || (variant && variant[0] == 'u');
}
function isSignedOp(op, variant) {
  return op in SIGNED_OP || (variant && variant[0] == 's');
}

var legalizedI64s = USE_TYPED_ARRAYS == 2; // We do not legalize globals, but do legalize function lines. This will be true in the latter case

function processMathop(item) {
  var op = item.op;
  var variant = item.variant;
  var type = item.type;
  var paramTypes = ['', '', '', ''];
  var idents = [];
  for (var i = 0; i < 3; i++) {
    if (item.params[i]) {
      paramTypes[i] = item.params[i].type || type;
      idents[i] = finalizeLLVMParameter(item.params[i]);
      if (!isNumber(idents[i]) && !isNiceIdent(idents[i])) {
        idents[i] = '(' + idents[i] + ')'; // we may have nested expressions. So enforce the order of operations we want
      }
    } else {
      idents[i] = null; // just so it exists for purposes of reading idents[1] etc. later on, and no exception is thrown
    }
  }
  var originalIdents = idents.slice(0);
  if (isUnsignedOp(op, variant)) {
    idents[0] = makeSignOp(idents[0], paramTypes[0], 'un');
    idents[1] = makeSignOp(idents[1], paramTypes[1], 'un');
  } else if (isSignedOp(op, variant)) {
    idents[0] = makeSignOp(idents[0], paramTypes[0], 're');
    idents[1] = makeSignOp(idents[1], paramTypes[1], 're');
  }
  var bits = null;
  if (item.type[0] === 'i') {
    bits = parseInt(item.type.substr(1));
  }
  var bitsBefore = parseInt((item.params[0] ? item.params[0].type : item.type).substr(1)); // remove i to leave the number of bits left after this
  var bitsLeft = parseInt(((item.params[1] && item.params[1].ident) ? item.params[1].ident : item.type).substr(1)); // remove i to leave the number of bits left after this operation

  function integerizeBignum(value) {
    return makeInlineCalculation('VALUE-VALUE%1', value, 'tempBigIntI');
  }

  if ((type == 'i64' || paramTypes[0] == 'i64' || paramTypes[1] == 'i64' || idents[1] == '(i64)') && USE_TYPED_ARRAYS == 2) {
    var warnI64_1 = function() {
      warnOnce('Arithmetic on 64-bit integers in mode 1 is rounded and flaky, like mode 0!');
    };
    // In ops that can be either legalized or not, we need to differentiate how we access low and high parts
    var low1 = idents[0] + (legalizedI64s ? '$0' : '[0]');
    var high1 = idents[0] + (legalizedI64s ? '$1' : '[1]');
    var low2 = idents[1] + (legalizedI64s ? '$0' : '[0]');
    var high2 = idents[1] + (legalizedI64s ? '$1' : '[1]');
    function finish(result) {
      // If this is in legalization mode, steal the assign and assign into two vars
      if (legalizedI64s) {
        assert(item.assignTo);
        var ret = 'var ' + item.assignTo + '$0 = ' + result[0] + '; var ' + item.assignTo + '$1 = ' + result[1] + ';';
        item.assignTo = null;
        return ret;
      } else {
        return result;
      }
    }
    function i64PreciseOp(type, lastArg) {
      Types.preciseI64MathUsed = true;
      return finish(['(i64Math' + (ASM_JS ? '_' : '.') + type + '(' + asmCoercion(low1, 'i32') + ',' + asmCoercion(high1, 'i32') + ',' + asmCoercion(low2, 'i32') + ',' + asmCoercion(high2, 'i32') +
                     (lastArg ? ',' + asmCoercion(+lastArg, 'i32') : '') + '),' + makeGetValue('tempDoublePtr', 0, 'i32') + ')', makeGetValue('tempDoublePtr', Runtime.getNativeTypeSize('i32'), 'i32')]);
    }
    function preciseCall(name) {
      Types.preciseI64MathUsed = true;
      return finish([asmCoercion(name + '(' + low1 + ',' + high1 + ',' + low2 + ',' + high2 + ')', 'i32'), 'tempRet0']);
    }
    function i64PreciseLib(type) {
      return preciseCall('_i64' + type[0].toUpperCase() + type.substr(1));
    }
    switch (op) {
      // basic integer ops
      case 'or': {
        return '[' + idents[0] + '[0] | ' + idents[1] + '[0], ' + idents[0] + '[1] | ' + idents[1] + '[1]]';
      }
      case 'and': {
        return '[' + idents[0] + '[0] & ' + idents[1] + '[0], ' + idents[0] + '[1] & ' + idents[1] + '[1]]';
      }
      case 'xor': {
        return '[' + idents[0] + '[0] ^ ' + idents[1] + '[0], ' + idents[0] + '[1] ^ ' + idents[1] + '[1]]';
      }
      case 'shl':
      case 'ashr':
      case 'lshr': {
				throw 'shifts should have been legalized!';
      }
      case 'uitofp': case 'sitofp': return RuntimeGenerator.makeBigInt(low1, high1, op[0] == 'u');
      case 'fptoui': case 'fptosi': return finish(splitI64(idents[0], true));
      case 'icmp': {
        switch (variant) {
          case 'uge': return '((' + high1 + '>>>0) >= (' + high2 + '>>>0)) & ((((' + high1 + '>>>0) >  ('  + high2 + '>>>0)) | ' +
                                                                        '(' + low1 + '>>>0) >= ('  + low2 + '>>>0)))';
          case 'sge': return '((' + high1 + '|0) >= (' + high2 + '|0)) & ((((' + high1 + '|0) >  ('  + high2 + '|0)) | ' +
                                                                        '(' + low1 + '>>>0) >= ('  + low2 + '>>>0)))';
          case 'ule': return '((' + high1 + '>>>0) <= (' + high2 + '>>>0)) & ((((' + high1 + '>>>0) <  (' + high2 + '>>>0)) | ' +
                                                                        '(' + low1 + '>>>0) <= (' + low2 + '>>>0)))';
          case 'sle': return '((' + high1 + '|0) <= (' + high2 + '|0)) & ((((' + high1 + '|0) <  (' + high2 + '|0)) | ' +
                                                                        '(' + low1 + '>>>0) <= (' + low2 + '>>>0)))';
          case 'ugt': return '((' + high1 + '>>>0) > (' + high2 + '>>>0)) | ((((' + high1 + '>>>0) == (' + high2 + '>>>0) & ' +
                                                                       '(' + low1 + '>>>0) >  (' + low2 + '>>>0))))';
          case 'sgt': return '((' + high1 + '|0) > (' + high2 + '|0)) | ((((' + high1 + '|0) == (' + high2 + '|0) & ' +
                                                                       '(' + low1 + '>>>0) >  (' + low2 + '>>>0))))';
          case 'ult': return '((' + high1 + '>>>0) < (' + high2 + '>>>0)) | ((((' + high1 + '>>>0) == (' + high2 + '>>>0) & ' +
                                                                       '(' + low1 + '>>>0) <  (' + low2 + '>>>0))))';
          case 'slt': return '((' + high1 + '|0) < (' + high2 + '|0)) | ((((' + high1 + '|0) == (' + high2 + '|0) & ' +
                                                                       '(' + low1 + '>>>0) <  (' + low2 + '>>>0))))';
          case 'ne':  return '((' + low1 + '|0) != (' + low2 + '|0)) | ((' + high1 + '|0) != (' + high2 + '|0))';
          case 'eq':  return '((' + low1 + '|0) == (' + low2 + '|0)) & ((' + high1 + '|0) == (' + high2 + '|0))';
          default: throw 'Unknown icmp variant: ' + variant;
        }
      }
      case 'zext': return makeI64(idents[0], 0);
      case 'sext': return makeInlineCalculation(makeI64('VALUE', 'VALUE<0 ? 4294967295 : 0'), idents[0], 'tempBigIntD');
      case 'trunc': {
        return '((' + idents[0] + '[0]) & ' + (Math.pow(2, bitsLeft)-1) + ')';
      }
      case 'select': return idents[0] + ' ? ' + makeCopyI64(idents[1]) + ' : ' + makeCopyI64(idents[2]);
      case 'ptrtoint': return makeI64(idents[0], 0);
      case 'inttoptr': return '(' + idents[0] + '[0])'; // just directly truncate the i64 to a 'pointer', which is an i32
      // Dangerous, rounded operations. TODO: Fully emulate
      case 'add': {
        if (PRECISE_I64_MATH) {
          return i64PreciseLib('add');
        } else {
          warnI64_1();
          return finish(splitI64(mergeI64(idents[0]) + '+' + mergeI64(idents[1]), true));
        }
      }
      case 'sub': {
        if (PRECISE_I64_MATH) {
          return i64PreciseLib('subtract');
        } else {
          warnI64_1();
          return finish(splitI64(mergeI64(idents[0]) + '-' + mergeI64(idents[1]), true));
        }
      }
      case 'sdiv': case 'udiv': {
        if (PRECISE_I64_MATH) {
          return preciseCall(op[0] === 'u' ? '___udivdi3' : '___divdi3');
        } else {
          warnI64_1();
          return finish(splitI64(makeRounding(mergeI64(idents[0], op[0] === 'u') + '/' + mergeI64(idents[1], op[0] === 'u'), bits, op[0] === 's'), true));
        }
      }
      case 'mul': {
        if (PRECISE_I64_MATH) {
          return preciseCall('___muldi3');
        } else {
          warnI64_1();
          return finish(splitI64(mergeI64(idents[0], op[0] === 'u') + '*' + mergeI64(idents[1], op[0] === 'u'), true));
        }
      }
      case 'urem': case 'srem': {
        if (PRECISE_I64_MATH) {
          return preciseCall(op[0] === 'u' ? '___uremdi3' : '___remdi3');
        } else {
          warnI64_1();
          return finish(splitI64(mergeI64(idents[0], op[0] === 'u') + '%' + mergeI64(idents[1], op[0] === 'u'), true));
        }
      }
      case 'bitcast': {
        // Pointers are not 64-bit, so there is really only one possible type of bitcast here, int to float or vice versa
        assert(USE_TYPED_ARRAYS == 2, 'Can only bitcast ints <-> floats with typed arrays mode 2');
        var inType = item.params[0].type;
        var outType = item.type;
        if (inType in Runtime.INT_TYPES && outType in Runtime.FLOAT_TYPES) {
          if (legalizedI64s) {
            return '(' + makeSetTempDouble(0, 'i32', idents[0] + '$0') + ', ' + makeSetTempDouble(1, 'i32', idents[0] + '$1') + ', ' + makeGetTempDouble(0, 'double') + ')';
          } else {
            return makeInlineCalculation(makeSetTempDouble(0, 'i32', 'VALUE[0]') + ',' + makeSetTempDouble(1, 'i32', 'VALUE[1]') + ',' + makeGetTempDouble(0, 'double'), idents[0], 'tempI64');
          }
        } else if (inType in Runtime.FLOAT_TYPES && outType in Runtime.INT_TYPES) {
          if (legalizedI64s) {
            return makeSetTempDouble(0, 'double', idents[0]) + '; ' + finish([makeGetTempDouble(0, 'i32'), makeGetTempDouble(1, 'i32')]);
          } else {
            return '(' + makeSetTempDouble(0, 'double', idents[0]) + ',[' + makeGetTempDouble(0, 'i32') + ',' + makeGetTempDouble(1, 'i32') + '])';
          }
        } else {
          throw 'Invalid USE_TYPED_ARRAYS == 2 bitcast: ' + dump(item) + ' : ' + item.params[0].type;
        }
      }
      default: throw 'Unsupported i64 mode 1 op: ' + item.op + ' : ' + dump(item);
    }
  }

  switch (op) {
    // basic integer ops
    case 'add': return handleOverflow(getFastValue(idents[0], '+', idents[1], item.type), bits);
    case 'sub': return handleOverflow(getFastValue(idents[0], '-', idents[1], item.type), bits);
    case 'sdiv': case 'udiv': return makeRounding(getFastValue(idents[0], '/', idents[1], item.type), bits, op[0] === 's');
    case 'mul': return getFastValue(idents[0], '*', idents[1], item.type); // overflow handling is already done in getFastValue for '*'
    case 'urem': case 'srem': return getFastValue(idents[0], '%', idents[1], item.type);
    case 'or': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for or: ' + bits);
        dprint('Warning: 64 bit OR - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.or64(' + idents[0] + ', ' + idents[1] + ')';
      }
      return idents[0] + ' | ' + idents[1];
    }
    case 'and': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for and: ' + bits);
        dprint('Warning: 64 bit AND - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.and64(' + idents[0] + ', ' + idents[1] + ')';
      }
      return idents[0] + ' & ' + idents[1];
    }
    case 'xor': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for xor: ' + bits);
        dprint('Warning: 64 bit XOR - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.xor64(' + idents[0] + ', ' + idents[1] + ')';
      }
      return idents[0] + ' ^ ' + idents[1];
    }
    case 'shl': {
      if (bits > 32) return idents[0] + '*' + getFastValue(2, 'pow', idents[1]);
      return idents[0] + ' << ' + idents[1];
    }
    case 'ashr': {
      if (bits > 32) return integerizeBignum(idents[0] + '/' + getFastValue(2, 'pow', idents[1]));
      if (bits === 32) return originalIdents[0] + ' >> ' + idents[1]; // No need to reSign in this case
      return idents[0] + ' >> ' + idents[1];
    }
    case 'lshr': {
      if (bits > 32) return integerizeBignum(idents[0] + '/' + getFastValue(2, 'pow', idents[1]));
      if (bits === 32) return originalIdents[0] + ' >>> ' + idents[1]; // No need to unSign in this case
      return idents[0] + ' >>> ' + idents[1];
    }
    // basic float ops
    case 'fadd': return getFastValue(idents[0], '+', idents[1], item.type);
    case 'fsub': return getFastValue(idents[0], '-', idents[1], item.type);
    case 'fdiv': return getFastValue(idents[0], '/', idents[1], item.type);
    case 'fmul': return getFastValue(idents[0], '*', idents[1], item.type);
    case 'frem': return getFastValue(idents[0], '%', idents[1], item.type);
    case 'uitofp': case 'sitofp': return asmCoercion(idents[0], 'double', op[0]);
    case 'fptoui': case 'fptosi': return makeRounding(idents[0], bitsLeft, op === 'fptosi', true);

    // TODO: We sometimes generate false instead of 0, etc., in the *cmps. It seemed slightly faster before, but worth rechecking
    //       Note that with typed arrays, these become 0 when written. So that is a potential difference with non-typed array runs.
    case 'icmp': {
      switch (variant) {
        case 'uge': case 'sge': return idents[0] + ' >= ' + idents[1];
        case 'ule': case 'sle': return idents[0] + ' <= ' + idents[1];
        case 'ugt': case 'sgt': return idents[0] + ' > ' + idents[1];
        case 'ult': case 'slt': return idents[0] + ' < ' + idents[1];
        // We use loose comparisons, which allows false == 0 to be true, etc. Ditto in fcmp
        case 'ne': case 'eq': {
          // We must sign them, so we do not compare -1 to 255 (could have unsigned them both too)
          // since LLVM tells us if <=, >= etc. comparisons are signed, but not == and !=.
          idents[0] = makeSignOp(idents[0], paramTypes[0], 're');
          idents[1] = makeSignOp(idents[1], paramTypes[1], 're');
          return idents[0] + (variant === 'eq' ? '==' : '!=') + idents[1];
        }
        default: throw 'Unknown icmp variant: ' + variant;
      }
    }
    case 'fcmp': {
      switch (variant) {
        // TODO 'o' ones should be 'ordered (no NaN) and',
        //      'u' ones should be 'unordered or'.
        case 'uge': case 'oge': return idents[0] + ' >= ' + idents[1];
        case 'ule': case 'ole': return idents[0] + ' <= ' + idents[1];
        case 'ugt': case 'ogt': return idents[0] + ' > ' + idents[1];
        case 'ult': case 'olt': return idents[0] + ' < ' + idents[1];
        case 'une': case 'one': return idents[0] + ' != ' + idents[1];
        case 'ueq': case 'oeq': return idents[0] + ' == ' + idents[1];
        case 'ord': return '!' + makeIsNaN(idents[0]) + ' & !' + makeIsNaN(idents[1]);
        case 'uno': return makeIsNaN(idents[0]) + ' | ' + makeIsNaN(idents[1]);
        case 'true': return '1';
        default: throw 'Unknown fcmp variant: ' + variant;
      }
    }
    // Note that zext has sign checking, see above. We must guard against -33 in i8 turning into -33 in i32
    // then unsigning that i32... which would give something huge.
    case 'zext': {
      if (EXPLICIT_ZEXT && bitsBefore == 1 && bitsLeft > 1) {
        return '(' + originalIdents[0] + '?1:0)'; // explicit bool-to-int conversion, work around v8 issue 2513
        break;
      }
      // otherwise, fall through
    }
    case 'fpext': case 'sext': return idents[0];
    case 'fptrunc': return idents[0];
    case 'select': return idents[0] + ' ? ' + asmEnsureFloat(idents[1], item.type) + ' : ' + asmEnsureFloat(idents[2], item.type);
    case 'ptrtoint': case 'inttoptr': {
      var ret = '';
      if (QUANTUM_SIZE == 1) {
        warnOnce('.ll contains ptrtoint and/or inttoptr. These may be dangerous in QUANTUM == 1. ' +
                 'The safest thing is to investigate every appearance, and modify the source code to avoid this. ' +
                 'Emscripten will print a list of the .ll lines, and also annotate the .js.');
        dprint('  ' + op + ' on .ll line ' + item.lineNum);
        idents[0] += ' /* Warning: ' + op + ', .ll line ' + item.lineNum + ' */';
      }
      if (op == 'inttoptr' || bitsLeft >= 32) return idents[0];
      // For ptrtoint and <32 bits, fall through into trunc since we need to truncate here
    }
    case 'trunc': {
      // Unlike extending, which we just 'do' (by doing nothing),
      // truncating can change the number, e.g. by truncating to an i1
      // in order to get the first bit
      assert(bitsLeft <= 32, 'Cannot truncate to more than 32 bits, since we use a native & op');
      return '((' + idents[0] + ') & ' + (Math.pow(2, bitsLeft)-1) + ')';
    }
    case 'bitcast': {
      // Most bitcasts are no-ops for us. However, the exception is int to float and float to int
      var inType = item.params[0].type;
      var outType = item.type;
      if ((inType in Runtime.INT_TYPES && outType in Runtime.FLOAT_TYPES) ||
          (inType in Runtime.FLOAT_TYPES && outType in Runtime.INT_TYPES)) {
        assert(USE_TYPED_ARRAYS == 2, 'Can only bitcast ints <-> floats with typed arrays mode 2');
        if (inType in Runtime.INT_TYPES) {
          return '(' + makeSetTempDouble(0, 'i32', idents[0]) + ',' + makeGetTempDouble(0, 'float') + ')';
        } else {
          return '(' + makeSetTempDouble(0, 'float', idents[0]) + ',' + makeGetTempDouble(0, 'i32') + ')';
        }
      }
      return idents[0];
    }
    default: throw 'Unknown mathcmp op: ' + item.op;
  }
}

// Walks through some intertype data, calling a function at every item. If
// the function returns true, will stop the walk.
// TODO: Use this more in analyzer, possibly also in jsifier
function walkInterdata(item, pre, post, obj) {
  if (!item || !item.intertype) return false;
  if (pre && pre(item, obj)) return true;
  var originalObj = obj;
  if (obj && obj.replaceWith) obj = obj.replaceWith; // allow pre to replace the object we pass to all its children
  if (item.value && walkInterdata(item.value, pre, post,  obj)) return true;
  // TODO if (item.pointer && walkInterdata(item.pointer, pre, post,  obj)) return true;
  if (item.dependent && walkInterdata(item.dependent, pre, post,  obj)) return true;
  var i;
  if (item.params) {
    for (i = 0; i <= item.params.length; i++) {
      if (walkInterdata(item.params[i], pre, post,  obj)) return true;
    }
  }
  if (item.possibleVars) { // other attributes that might contain interesting data; here, variables
    var box = { intertype: 'value', ident: '' };
    for (i = 0; i <= item.possibleVars.length; i++) {
      box.ident = item[item.possibleVars[i]];
      if (walkInterdata(box, pre, post,  obj)) return true;
    }
  }
  return post && post(item, originalObj, obj);
}

// Separate from walkInterdata so that the former is as fast as possible
// If the callback returns a value, we replace the current item with that
// value, and do *not* walk the children.
function walkAndModifyInterdata(item, pre) {
  if (!item || !item.intertype) return false;
  var ret = pre(item);
  if (ret) return ret;
  var repl;
  if (item.value && (repl = walkAndModifyInterdata(item.value, pre))) item.value = repl;
  if (item.pointer && (repl = walkAndModifyInterdata(item.pointer, pre))) item.pointer = repl;
  if (item.dependent && (repl = walkAndModifyInterdata(item.dependent, pre))) item.dependent = repl;
  if (item.params) {
    for (var i = 0; i <= item.params.length; i++) {
      if (repl = walkAndModifyInterdata(item.params[i], pre)) item.params[i] = repl;
    }
  }
  // Ignore possibleVars because we can't replace them anyhow
}

function parseBlockAddress(segment) {
  return { intertype: 'blockaddress', func: toNiceIdent(segment[2].item.tokens[0].text), label: toNiceIdent(segment[2].item.tokens[2].text), type: 'i32' };
}

function finalizeBlockAddress(param) {
  return '{{{ BA_' + param.func + '|' + param.label + ' }}}'; // something python will replace later
}

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

// Generates code that prints without printf(), but just putchar (so can be directly inline in asm.js)
function makePrintChars(s, sep) {
  sep = sep || ';';
  var ret = '';
  for (var i = 0; i < s.length; i++) {
    ret += '_putchar(' + s.charCodeAt(i) + ')' + sep;
  }
  ret += '_putchar(10)';
  return ret;
}

