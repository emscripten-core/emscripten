// Various tools for parsing LLVM. Utilities of various sorts, that are
// specific to Emscripten (and hence not in utility.js).

// Does simple 'macro' substitution, using Django-like syntax,
// {{{ code }}} will be replaced with |eval(code)|.
function processMacros(text) {
  return text.replace(/{{{[^}]+}}}/g, function(str) {
    str = str.substr(3, str.length-6);
    var ret = eval(str);
    if (ret !== undefined) ret = ret.toString();
    return ret;
  });
}

// Simple #if/else/endif preprocessing for a file. Checks if the
// ident checked is true in our global. Also replaces some constants.
function preprocess(text, constants) {
  for (var constant in constants) {
    text = text.replace(eval('/' + constant + '/g'), constants[constant]);
  }
  var lines = text.split('\n');
  var ret = '';
  var showStack = [];
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
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
    ret ++;
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
    return /^[\w$_ ()]+$/.test(ident);
  } else {
    return /^[\w$_]+$/.test(ident);
  }
}

function isStructPointerType(type) {
  // This test is necessary for clang - in llvm-gcc, we
  // could check for %struct. The downside is that %1 can
  // be either a variable or a structure, and we guess it is
  // a struct, which can lead to |call i32 %5()| having
  // |%5()| as a function call (like |i32 (i8*)| etc.). So
  // we must check later on, in call(), where we have more
  // context, to differentiate such cases.
  // A similar thing happns in isStructType()
  return !Runtime.isNumberType(type) && type[0] == '%';
}

function isStructType(type) {
  if (isPointerType(type)) return false;
  if (new RegExp(/^\[\d+\ x\ (.*)\]/g).test(type)) return true; // [15 x ?] blocks. Like structs
  if (new RegExp(/<?{ [^}]* }>?/g).test(type)) return true; // { i32, i8 } etc. - anonymous struct types
  // See comment in isStructPointerType()
  return !Runtime.isNumberType(type) && type[0] == '%';
}

function isPointerType(type) {
  return pointingLevels(type) > 0;
}

function isIntImplemented(type) {
  return type[0] == 'i' || isPointerType(type);
}

function isVoidType(type) {
  return type == 'void';
}

// Detects a function definition, ([...|type,[type,...]])
function isFunctionDef(token) {
  var text = token.text;
  var nonPointing = removeAllPointing(text);
  if (nonPointing[0] != '(' || nonPointing.substr(-1) != ')')
    return false;
  if (nonPointing === '()') return true;
  if (!token.item) return false;
  var fail = false;
  splitTokenList(token.item.tokens).forEach(function(segment) {
    var subtext = segment[0].text;
    fail = fail || segment.length > 1 || !(isType(subtext) || subtext == '...');
  });
  return !fail;
}

function isFunctionType(type) {
  type = type.replace(/"[^"]+"/g, '".."');
  var parts = type.split(' ');
  if (pointingLevels(type) !== 1) return false;
  var text = removeAllPointing(parts.slice(1).join(' '));
  if (!text) return false;
  return isType(parts[0]) && isFunctionDef({ text: text, item: tokenize(text.substr(1, text.length-2), true) });
}

function isType(type) { // TODO!
  return isVoidType(type) || Runtime.isNumberType(type) || isStructType(type) || isPointerType(type) || isFunctionType(type);
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
  for (var i = 0; i < item.tokens.length; i++) {
    if (item.tokens[i].text == text) return i;
  }
  return -1;
}

// Splits a list of tokens separated by commas. For example, a list of arguments in a function call
function splitTokenList(tokens) {
  if (tokens.length == 0) return [];
  if (!tokens.slice) tokens = tokens.tokens;
  if (tokens.slice(-1)[0].text != ',') tokens.push({text:','});
  var ret = [];
  var seg = [];
  var SPLITTERS = set(',', 'to'); // 'to' can separate parameters as well...
  for (var i = 0; i < tokens.length; i++) {
    var token = tokens[i];
    if (token.text in SPLITTERS) {
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

// Splits an item, with the intent of later reintegration
function splitItem(parent, childSlot, copySlots) {
  if (!copySlots) copySlots = [];
  if (!parent[childSlot]) parent[childSlot] = {};
  var child = parent[childSlot];
  parent[childSlot] = null;
  child.parentUid = parent.__uid__;
  child.parentSlot = childSlot;
  child.parentLineNum = child.lineNum = parent.lineNum;
  copySlots.forEach(function(slot) { child[slot] = parent[slot] });
  return {
    parent: parent,
    child: child
  };
}

function makeReintegrator(afterFunc) {
  // Reintegration - find intermediate representation-parsed items and
  // place back in parents
  return {
    process: function(items) {
      var ret = [];
      var lineDict = {};
      for (var i = 0; i < items.length; i++) {
        var item = items[i];
        if (!item.parentSlot) {
          assert(!lineDict[item.lineNum]);
          lineDict[item.lineNum] = i;
        }
      }
      for (var i = 0; i < items.length; i++) {
        var child = items[i];
        var j = lineDict[child.parentLineNum];
        if (typeof j === 'number') {
          var parent = items[j];
          // process the pair
          parent[child.parentSlot] = child;
          delete child.parentLineNum;
          afterFunc.call(this, parent, child);

          items[i] = null;
          items[j] = null;
          lineDict[child.parentLineNum] = null;
        }
      }
      this.forwardItems(items.filter(function(item) { return !!item }), this.name_); // next time hopefully
      return ret;
    }
  };
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
    if (segment[1] && segment[1].text === 'byval') {
      // handle 'byval' and 'byval align X'
      segment.splice(1, 1);
      if (segment[1] && segment[1].text === 'align') {
        segment.splice(1, 2);
      }
    }
    if (segment.length == 1) {
      if (segment[0].text == '...') {
        ret.push({
          intertype: 'varargs'
        });
      } else {
        // Clang sometimes has a parameter with just a type,
        // no name... the name is implied to be %{the index}
        ret.push({
          intertype: 'value',
          type: segment[0].text,
          value: null,
          ident: toNiceIdent('%') + anonymousIndex
        });
        Types.needAnalysis[ret.type] = 0;
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
        value: segment[1],
        ident: toNiceIdent(parseNumerical(segment[1].text))
      });
      Types.needAnalysis[ret.type] = 0;
      //          } else {
      //            throw "what is this params token? " + JSON.stringify(segment);
    }
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

function finalizeParam(param) {
  if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
    return finalizeLLVMFunctionCall(param);
  } else if (param.intertype === 'blockaddress') {
    return finalizeBlockAddress(param);
  } else if (param.intertype === 'jsvalue') {
    return param.ident;
  } else {
    if (param.type == 'i64' && I64_MODE == 1) {
      return parseI64Constant(param.ident);
    }
    return toNiceIdent(param.ident);
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
      values: splitTokenList(segment[1].tokens).map(parseLLVMSegment),
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
      type: segment[0].text
    };
  }
}

function cleanSegment(segment) {
  while (segment.length >= 2 && ['noalias', 'sret', 'nocapture', 'nest', 'zeroext', 'signext'].indexOf(segment[1].text) != -1) {
    segment.splice(1, 1);
  }
  return segment;
}

PARSABLE_LLVM_FUNCTIONS = set('getelementptr', 'bitcast', 'inttoptr', 'ptrtoint', 'mul', 'icmp', 'zext', 'sub', 'add', 'div');

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
    } else if (intertype === 'bitcast') {
      assert(segment[2].item.tokens.slice(-2)[0].text === 'to');
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
    ret = parseLLVMFunctionCall([{text: 'i0'}].concat(tokens.slice(0,2))).ident; // TODO: Handle more cases, return a full object, process it later
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
    while (tokens[index].text in filterOut) {
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
    warnOnce('.ll contains floating-point values with more than 64 bits. Faking values for them. If they are used, this will almost certainly fail!');
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

// Makes a proper runtime value for a 64-bit value from low and high i32s.
function makeI64(low, high) {
  if (I64_MODE == 1) {
    return '[' + low + ',' + (high || '0') + ']';
    // FIXME with this?       return '[unSign(' + low + ',32),' + (high ? ('unSign(' + high + ',32)') : '0') + ']';
  } else {
    var ret = low;
    if (high) {
      ret = '(' + low + '+(4294967296*' + high + '))';
    }
    return ret;
  }
}

// Splits a number (an integer in a double, possibly > 32 bits) into an I64_MODE 1 i64 value.
// Will suffer from rounding. margeI64 does the opposite.
function splitI64(value) {
  assert(I64_MODE == 1);
  return '(tempInt=' + value + ',' + makeI64('tempInt>>>0', 'Math.floor(tempInt/4294967296)') + ')';
}
function mergeI64(value) {
  assert(I64_MODE == 1);
  return '(tempI64=' + value + ',tempI64[0]+tempI64[1]*4294967296)';
}

// Takes an i64 value and changes it into the [low, high] form used in i64 mode 1. In that
// mode, this is a no-op
function ensureI64_1(value) {
  if (I64_MODE == 1) return value;
  return '(tempInt=' + value + ',[tempInt>>>0, Math.floor(tempInt/4294967296)])';
}

function makeCopyI64(value) {
  assert(I64_MODE == 1);

  return value + '.slice(0)';
}

function parseI64Constant(str) {
  assert(I64_MODE == 1);

  if (!isNumber(str)) {
    // This is a variable. Copy it, so we do not modify the original
    return makeCopyI64(str);
  }

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
    v = str2vec('18446744073709551616'); // 2^64
    subtract(v, str2vec(str));
  } else {
    v = str2vec(str);
  }

  var bits = [];
  while (!isZero(v)) {
    bits.push((v[v.length-1] % 2 != 0)+0);
    v[v.length-1] = v[v.length-1] & 0xfe;
    divide2(v);
  }

  var low = 0, high = 0;
  for (var i = 0; i < bits.length; i++) {
    if (i <= 31) {
      low += bits[i]*Math.pow(2, i);
    } else {
      high += bits[i]*Math.pow(2, i-32);
    }
  }

  return '[' + low + ',' + high + ']';
}

function parseNumerical(value, type) {
  if ((!type || type == 'double' || type == 'float') && (value.substr && value.substr(0,2) == '0x')) {
    // Hexadecimal double value, as the llvm docs say,
    // "The one non-intuitive notation for constants is the hexadecimal form of floating point constants."
    value = IEEEUnHex(value);
  } else if (type == 'i64' && I64_MODE == 1) {
    value = parseI64Constant(value);
  } else if (value == 'null') {
    // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)
    value = '0';
  } else if (value === 'true') {
    return '1';
  } else if (value === 'false') {
    return '0';
  }
  if (isNumber(value)) {
    return eval(value).toString(); // will change e.g. 5.000000e+01 to 50
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

//! Returns the size of a type, as C/C++ would have it (in 32-bit, for now), in bytes.
//! @param type The type, by name.
function getNativeTypeSize(type) {
  if (QUANTUM_SIZE == 1) return 1;
  var size = {
    '_i1': 1,
    '_i8': 1,
    '_i16': 2,
    '_i32': 4,
    '_i64': 8,
    "_float": 4,
    "_double": 8
  }['_'+type]; // add '_' since float&double confuse Closure compiler as keys.
  if (!size && type[type.length-1] == '*') {
    size = QUANTUM_SIZE; // A pointer
  }
  return size;
}

//! Returns the size of a structure field, as C/C++ would have it (in 32-bit,
//! for now).
//! @param type The type, by name.
function getNativeFieldSize(type) {
  return Math.max(Runtime.getNativeTypeSize(type), QUANTUM_SIZE);
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
    return getNativeTypeSize(type); // We can really get away with '1', though, at least on the stack...
  }
}

// Generates the type signature for a structure, for each byte, the type that is there.
// i32, 0, 0, 0 - for example, an int32 is here, then nothing to do for the 3 next bytes, naturally
function generateStructTypes(type) {
  if (isArray(type)) return type; // already in the form of [type, type,...]
  if (Runtime.isNumberType(type) || isPointerType(type)) {
    return [type].concat(zeros(getNativeFieldSize(type)));
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
      if (Runtime.isNumberType(type) || isPointerType(type)) {
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
  assert(!(CORRECT_SIGNS >= 2 && !Debugging.on), 'Need debugging for line-specific corrections');
  if (!Framework.currItem) return false;
  if (Framework.currItem.funcData.ident.indexOf('emscripten_autodebug') >= 0) return 1; // always correct in the autodebugger code!
  return (CORRECT_SIGNS === 2 && Debugging.getIdentifier() in CORRECT_SIGNS_LINES) ||
         (CORRECT_SIGNS === 3 && !(Debugging.getIdentifier() in CORRECT_SIGNS_LINES));
}
function correctSigns() {
  return CORRECT_SIGNS === 1 || correctSpecificSign();
}

function correctSpecificOverflow() {
  assert(!(CORRECT_OVERFLOWS >= 2 && !Debugging.on), 'Need debugging for line-specific corrections');
  if (!Framework.currItem) return false;
  return (CORRECT_OVERFLOWS === 2 && Debugging.getIdentifier() in CORRECT_OVERFLOWS_LINES) ||
         (CORRECT_OVERFLOWS === 3 && !(Debugging.getIdentifier() in CORRECT_OVERFLOWS_LINES));
}
function correctOverflows() {
  return CORRECT_OVERFLOWS === 1 || correctSpecificOverflow();
}

function correctSpecificRounding() {
  assert(!(CORRECT_ROUNDINGS >= 2 && !Debugging.on), 'Need debugging for line-specific corrections');
  if (!Framework.currItem) return false;
  return (CORRECT_ROUNDINGS === 2 && Debugging.getIdentifier() in CORRECT_ROUNDINGS_LINES) ||
         (CORRECT_ROUNDINGS === 3 && !(Debugging.getIdentifier() in CORRECT_ROUNDINGS_LINES));
}
function correctRoundings() {
  return CORRECT_ROUNDINGS === 1 || correctSpecificRounding();
}

function checkSpecificSafeHeap() {
  assert(!(SAFE_HEAP >= 2 && !Debugging.on), 'Need debugging for line-specific checks');
  if (!Framework.currItem) return false;
  return (SAFE_HEAP === 2 && Debugging.getIdentifier() in SAFE_HEAP_LINES) ||
         (SAFE_HEAP === 3 && !(Debugging.getIdentifier() in SAFE_HEAP_LINES));
}
function checkSafeHeap() {
  return SAFE_HEAP === 1 || checkSpecificSafeHeap();
}

function getHeapOffset(offset, type) {
  if (USE_TYPED_ARRAYS !== 2) {
    return offset;
  } else {
    if (getNativeFieldSize(type) > 4) {
      type = 'i32'; // XXX we emulate 64-bit values as 32
    }
    var shifts = Math.log(getNativeTypeSize(type))/Math.LN2;
    if (shifts != 0) {
      return '((' + offset + ')>>' + (shifts) + ')';
    } else {
      return '(' + offset + ')';
    }
  }
}

// See makeSetValue
function makeGetValue(ptr, pos, type, noNeedFirst, unsigned, ignore, align) {
  if (isStructType(type)) {
    var typeData = Types.types[type];
    var ret = [];
    for (var i = 0; i < typeData.fields.length; i++) {
      ret.push('f' + i + ': ' + makeGetValue(ptr, pos + typeData.flatIndexes[i], typeData.fields[i], noNeedFirst, unsigned));
    }
    return '{ ' + ret.join(', ') + ' }';
  }

  if (EMULATE_UNALIGNED_ACCESSES && USE_TYPED_ARRAYS == 2 && align && isIntImplemented(type)) { // TODO: support unaligned doubles and floats
    // Alignment is important here. May need to split this up
    var bytes = getNativeTypeSize(type);
    if (bytes > align) {
      var ret = '/* unaligned */(';
      if (bytes <= 4) {
        for (var i = 0; i < bytes; i++) {
          ret += 'tempInt' + (i == 0 ? '=' : (i < bytes-1 ? '+=((' : '+(('));
          ret += makeSignOp(makeGetValue(ptr, getFastValue(pos, '+', i), 'i8', noNeedFirst, unsigned, ignore), 'i8', 'un', true);
          if (i > 0) ret += ')<<' + (8*i) + ')';
          if (i < bytes-1) ret += ',';
        }
      } else {
        assert(bytes == 8);
        ret += 'tempBigInt=' + makeGetValue(ptr, pos, 'i32', noNeedFirst, true, ignore, align) + ',';
        ret += 'tempBigInt2=' + makeGetValue(ptr, getFastValue(pos, '+', getNativeTypeSize('i32')), 'i32', noNeedFirst, true, ignore, align) + ',';
        ret += makeI64('tempBigInt', 'tempBigInt2');
      }
      ret += ')';
      return ret;
    }
  }

  if (type == 'i64' && I64_MODE == 1) {
    return '[' + makeGetValue(ptr, pos, 'i32', noNeedFirst, unsigned, ignore) + ','
               + makeGetValue(ptr, getFastValue(pos, '+', getNativeTypeSize('i32')), 'i32', noNeedFirst, unsigned, ignore) + ']';
  }

  var offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP) {
    if (type !== 'null' && type[0] !== '#') type = '"' + safeQuote(type) + '"';
    if (type[0] === '#') type = type.substr(1);
    return 'SAFE_HEAP_LOAD(' + offset + ', ' + type + ', ' + (!!unsigned+0) + ', ' + ((!checkSafeHeap() || ignore)|0) + ')';
  } else {
    return makeGetSlabs(ptr, type, false, unsigned)[0] + '[' + getHeapOffset(offset, type) + ']';
  }
}

function indexizeFunctions(value, type) {
  assert((type && type !== '?') || (typeof value === 'string' && value.substr(0, 6) === 'CHECK_'), 'No type given for function indexizing: ' + [value, type]);
  assert(value !== type, 'Type set to value: ' + [value, type]);
  if (type && isFunctionType(type) && value[0] === '_') { // checking for _ differentiates from $ (local vars)
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
function makeSetValue(ptr, pos, value, type, noNeedFirst, ignore, align) {
  if (isStructType(type)) {
    var typeData = Types.types[type];
    var ret = [];
    // We can receive either an object - an object literal that was in the .ll - or a string,
    // which is the ident of an aggregate struct
    if (typeof value === 'string') {
      value = range(typeData.fields.length).map(function(i) { return value + '.f' + i });
    }
    for (var i = 0; i < typeData.fields.length; i++) {
      ret.push(makeSetValue(ptr, pos + typeData.flatIndexes[i], value[i], typeData.fields[i], noNeedFirst));
    }
    return ret.join('; ');
  }

  if (EMULATE_UNALIGNED_ACCESSES && USE_TYPED_ARRAYS == 2 && align && isIntImplemented(type)) { // TODO: support unaligned doubles and floats
    // Alignment is important here. May need to split this up
    var bytes = getNativeTypeSize(type);
    if (bytes > align) {
      var ret = '/* unaligned */';
      if (bytes <= 4) {
        ret += 'tempInt=' + value + ';';
        for (var i = 0; i < bytes; i++) {
          ret += makeSetValue(ptr, getFastValue(pos, '+', i), 'tempInt&0xff', 'i8', noNeedFirst, ignore) + ';';
          if (i < bytes-1) ret += 'tempInt>>=8;';
        }
      } else {
        assert(bytes == 8);
        ret += 'tempPair=' + ensureI64_1(value) + ';';
        ret += makeSetValue(ptr, pos, 'tempPair[0]', 'i32', noNeedFirst, ignore, align) + ';';
        ret += makeSetValue(ptr, getFastValue(pos, '+', getNativeTypeSize('i32')), 'tempPair[1]', 'i32', noNeedFirst, ignore, align) + ';';
      }
      return ret;
    }
  }

  if (type == 'i64' && I64_MODE == 1) {
    return '(' + makeSetValue(ptr, pos, value + '[0]', 'i32', noNeedFirst, ignore) + ','
               + makeSetValue(ptr, getFastValue(pos, '+', getNativeTypeSize('i32')), value + '[1]', 'i32', noNeedFirst, ignore) + ')';
  }

  value = indexizeFunctions(value, type);
  var offset = calcFastOffset(ptr, pos, noNeedFirst);
  if (SAFE_HEAP) {
    if (type !== 'null' && type[0] !== '#') type = '"' + safeQuote(type) + '"';
    if (type[0] === '#') type = type.substr(1);
    return 'SAFE_HEAP_STORE(' + offset + ', ' + value + ', ' + type + ', ' + ((!checkSafeHeap() || ignore)|0) + ')';
  } else {
    return makeGetSlabs(ptr, type, true).map(function(slab) { return slab + '[' + getHeapOffset(offset, type) + ']=' + value }).join('; ');
  }
}

var UNROLL_LOOP_MAX = 5;

function makeSetValues(ptr, pos, value, type, num) {
  function safety(where) {
    where = where || getFastValue(ptr, '+', pos) + '+mspi';
    return ';' + (SAFE_HEAP ? 'SAFE_HEAP_ACCESS(' + where + ', ' + type + ', 1)' : '');
  }
  if (USE_TYPED_ARRAYS in set(0, 1)) {
    if (isNumber(num)) {
      if (parseInt(num) <= UNROLL_LOOP_MAX) {
        return range(num).map(function(i) {
          return makeSetValue(ptr, getFastValue(pos, '+', i), value, type);
        }).join('; ');
      }
    }
    return 'for (var mspi = 0; mspi < ' + num + '; mspi++) {\n' +
      makeSetValue(ptr, getFastValue(pos, '+', 'mspi'), value, type) + '\n}';
  } else { // USE_TYPED_ARRAYS == 2
/*
    return 'for (var mspi = 0; mspi < ' + num + '; mspi++) {\n' +
           '  HEAP8[' + getFastValue(ptr, '+', pos) + '+mspi] = ' + value + safety() + '\n}';
*/
    return '' +
      'var dest, stop, stop4, fast, value;\n' +
      'dest = ' + getFastValue(ptr, '+', pos) + ';\n' +
      'stop = dest + ' + num + ';\n' +
      'value = ' + value + ';\n' +
      'if (value < 0) value += 256;\n' +
      'value = value + (value<<8) + (value<<16) + (value*16777216);\n' + 
      'while (dest%4 !== 0 && dest < stop) {\n' +
      '  ' + safety('dest') + '; HEAP8[dest++] = ' + value + ';\n' +
      '}\n' +
      'dest >>= 2;\n' +
      'stop4 = stop >> 2;\n' +
      'while (dest < stop4) {\n' +
         safety('(dest<<2)+0', '(src<<2)+0') + ';' + safety('(dest<<2)+1', '(src<<2)+1') + ';' + 
         safety('(dest<<2)+2', '(src<<2)+2') + ';' + safety('(dest<<2)+3', '(src<<2)+3') + (SAFE_HEAP ? ';\n' : '') +
      '  HEAP32[dest++] = value;\n' + // this is the fast inner loop we try hard to stay in
      '}\n' +
      'dest <<= 2;\n' +
      'while (dest < stop) {\n' +
      '  ' + safety('dest') + '; HEAP8[dest++] = ' + value + ';\n' +
      '}'
  }
}

var TYPED_ARRAY_SET_MIN = Infinity; // .set() as memcpy seems to just slow us down

function makeCopyValues(dest, src, num, type, modifier) {
  function safety(to, from) {
    to = to || (dest + '+' + 'mcpi');
    from = from || (src + '+' + 'mcpi');
    return (SAFE_HEAP ? 'SAFE_HEAP_COPY_HISTORY(' + to + ', ' + from + ')' : '');
  }
  if (USE_TYPED_ARRAYS <= 1) {
    if (isNumber(num)) {
      if (parseInt(num) <= UNROLL_LOOP_MAX) {
        return range(num).map(function(i) {
          return type !== 'null' ? makeSetValue(dest, i, makeGetValue(src, i, type) + (modifier || ''), type)
                           : // Null is special-cased: We copy over all heaps
                            makeGetSlabs(dest, 'null', true).map(function(slab) {
                              return slab + '[' + getFastValue(dest, '+', i) + ']=' + slab + '[' + getFastValue(src, '+', i) + ']';
                            }).join('; ') + '; ' + safety(dest + '+' + i, src + '+' + i)
        }).join('; ');
      }
    }
    if (SAFE_HEAP) {
      return 'for (var mcpi = 0; mcpi < ' + num + '; mcpi++) {\n' +
        (type !== 'null' ? makeSetValue(dest, 'mcpi', makeGetValue(src, 'mcpi', type) + (modifier || ''), type)
                         : // Null is special-cased: We copy over all heaps
                          makeGetSlabs(dest, 'null', true).map(function(slab) {
                            return slab + '[' + dest + '+mcpi]=' + slab + '[' + src + '+mcpi]'
                          }).join('; ') + '; ' + safety()
        ) + '\n' + '}';
    }
    if (USE_TYPED_ARRAYS == 0) {
      return 'for (var mcpi_s=' + src + ',mcpi_e=' + src + '+' + num + ',mcpi_d=' + dest + '; mcpi_s<mcpi_e; mcpi_s++, mcpi_d++) {\n' +
             '  HEAP[mcpi_d] = HEAP[mcpi_s];\n' +
             '}';
    } else { // USE_TYPED_ARRAYS == 1
      if (isNumber(num) && parseInt(num) >= TYPED_ARRAY_SET_MIN) {
        return 'IHEAP.set(IHEAP.subarray(' + src + ',' + src + '+' + num + '), ' + dest + '); ' +
               'FHEAP.set(FHEAP.subarray(' + src + ',' + src + '+' + num + '), ' + dest + ')';
      }
      return 'for (var mcpi_s=' + src + ',mcpi_e=' + src + '+' + num + ',mcpi_d=' + dest + '; mcpi_s<mcpi_e; mcpi_s++, mcpi_d++) {\n' +
             '  IHEAP[mcpi_d] = IHEAP[mcpi_s];' + (USE_FHEAP ? ' FHEAP[mcpi_d] = FHEAP[mcpi_s];' : '') + '\n' +
             '}';
    }
  } else { // USE_TYPED_ARRAYS == 2
    var ret = '' +
      'var src, dest, stop, stop4;\n' +
      'src = ' + src + ';\n' +
      'dest = ' + dest + ';\n' +
      'stop = src + ' + num + ';\n' +
      'if ((dest%4) == (src%4) && ' + num + ' > 8) {\n' +
      '  while (src%4 !== 0 && src < stop) {\n' +
      '    ' + safety('dest', 'src') + '; HEAP8[dest++] = HEAP8[src++];\n' +
      '  }\n';
    if (SAFE_HEAP || !(isNumber(num) && parseInt(num) >= TYPED_ARRAY_SET_MIN)) {
      ret += '  src >>= 2;\n' +
             '  dest >>= 2;\n' +
             '  stop4 = stop >> 2;\n' +
             '  while (src < stop4) {\n' +
                  safety('(dest<<2)+0', '(src<<2)+0') + ';' + safety('(dest<<2)+1', '(src<<2)+1') + ';' + 
                  safety('(dest<<2)+2', '(src<<2)+2') + ';' + safety('(dest<<2)+3', '(src<<2)+3') + (SAFE_HEAP ? ';\n' : '') +
             '    HEAP32[dest++] = HEAP32[src++];\n' +
             '  }\n' +
             '  src <<= 2;\n' +
             '  dest <<= 2;\n';
    } else {
      ret += '  var src4 = src >> 2, stop4 = stop >> 2, num4 = (stop4 - src4) << 2;\n' +
             '  HEAP32.set(HEAP32.subarray(src4, stop4), dest >> 2);\n' +
             '  src += num4; dest += num4;\n';
    }
    ret += '}' +
      'while (src < stop) {\n' +
      '  ' + safety('dest', 'src') + '; HEAP8[dest++] = HEAP8[src++];\n' +
      '}';
    return ret;
  }
  return null;
}

// Given two values and an operation, returns the result of that operation.
// Tries to do as much as possible at compile time.
function getFastValue(a, op, b, type) {
  a = a.toString();
  b = b.toString();
  if (isNumber(a) && isNumber(b)) {
    return eval(a + op + b).toString();
  }
  if (op in set('+', '*') && isNumber(a)) { // if one of them is a number, keep it last
    var c = b;
    b = a;
    a = c;
  }
  if (op in set('*', '/')) {
    if (op == '*') {
      if (a == 0 || b == 0) {
        return '0';
      } else if (a == 1) {
        return b;
      } else if (b == 1) {
        return a;
      } else if (isNumber(b) && type && isIntImplemented(type) && getNativeTypeSize(type) <= 32) {
        var shifts = Math.log(parseFloat(b))/Math.LN2;
        if (shifts % 1 == 0) {
          return '(' + a + '<<' + shifts + ')';
        }
      }
    } else {
      if (a == '0') {
        return '0';
      } else if (b == 1) {
        return a;
      } // Doing shifts for division is problematic, as getting the rounding right on negatives is tricky
    }
  } else if (op in set('+', '-')) {
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
  return a + op + b;
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

function makePointer(slab, pos, allocator, type) {
  assert(type, 'makePointer requires type info');
  if (slab.substr(0, 4) === 'HEAP' || (USE_TYPED_ARRAYS == 1 && slab in set('IHEAP', 'FHEAP'))) return pos;
  var types = generateStructTypes(type);
  // compress type info and data if possible
  var de;
  try {
    // compress all-zeros into a number (which will become zeros(..)).
    // note that we cannot always eval the slab, e.g., if it contains ident,0,0 etc. In that case, no compression TODO: ensure we get arrays here, not str
    var evaled = typeof slab === 'string' ? eval(slab) : slab;
    de = dedup(evaled);
    if (de.length === 1 && de[0] === 0) {
      slab = evaled.length;
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
  return 'allocate(' + slab + ', ' + JSON.stringify(types) + (allocator ? ', ' + allocator : '') + ')';
}

function makeGetSlabs(ptr, type, allowMultiple, unsigned) {
  assert(type);
  if (!USE_TYPED_ARRAYS) {
    return ['HEAP'];
  } else if (USE_TYPED_ARRAYS == 1) {
    if (type in Runtime.FLOAT_TYPES || type === 'int64') { // XXX should be i64, no?
      return ['FHEAP']; // If USE_FHEAP is false, will fail at runtime. At compiletime we do need it for library stuff.
    } else if (type in Runtime.INT_TYPES || isPointerType(type)) {
      return ['IHEAP'];
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
    function warn64() {
      warnOnce('.ll contains i64 or double values. These 64-bit values are dangerous in USE_TYPED_ARRAYS == 2. ' +
               'We store i64 as i32, and double as float. This can cause serious problems!');
    }
    switch(type) {
      case 'i1': case 'i8': return [unsigned ? 'HEAPU8' : 'HEAP8']; break;
      case 'i16': return [unsigned ? 'HEAPU16' : 'HEAP16']; break;
      case 'i64': warn64();
      case 'i32': return [unsigned ? 'HEAPU32' : 'HEAP32']; break;
      case 'float': return ['HEAPF32']; break;
      case 'double': warn64(); return ['HEAPF32']; break;
      default: {
        throw 'what, exactly, can we do for unknown types in TA2?! ' + new Error().stack;
      }
    }
  }
  return [];
}

function finalizeLLVMFunctionCall(item, noIndexizeFunctions) {
  switch(item.intertype) {
    case 'getelementptr': // TODO finalizeLLVMParameter on the ident and the indexes?
      return makePointer(makeGetSlabs(item.ident, item.type)[0], getGetElementPtrIndexes(item), null, item.type);
    case 'bitcast':
    case 'inttoptr':
    case 'ptrtoint':
      return finalizeLLVMParameter(item.params[0], noIndexizeFunctions);
    case 'icmp': case 'mul': case 'zext': case 'add': case 'sub': case 'div':
      var temp = {
        op: item.intertype,
        variant: item.variant,
        type: item.type
      };
      for (var i = 1; i <= 4; i++) {
        if (item.params[i-1]) {
          temp['param' + i] = item.params[i-1];
        }
      }
      return processMathop(temp);
    default:
      throw 'Invalid function to finalize: ' + dump(item.intertype);
  }
}

function getGetElementPtrIndexes(item) {
  var type = item.params[0].type;
  if (I64_MODE == 1) {
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
      indexes.push(getFastValue(getNativeTypeSize(type), '*', offset, 'i32'));
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
  warn(!correct || bits <= 32, 'Cannot correct overflows of this many bits: ' + bits + ' at line ' + Framework.currItem.lineNum);
  if (CHECK_OVERFLOWS) return 'CHECK_OVERFLOW(' + text + ', ' + bits + ', ' + Math.floor(correctSpecificOverflow() && !PGO) + (
    PGO ? ', "' + Debugging.getIdentifier() + '"' : ''
  ) + ')';
  if (!correct) return text;
  if (bits <= 32) {
    return '((' + text + ')&' + (Math.pow(2, bits) - 1) + ')';
  } else {
    return text; // We warned about this earlier
  }
}

function makeLLVMStruct(values) { // TODO: Use this everywhere
  return '{ ' + values.map(function(value, i) { return 'f' + i + ': ' + value }).join(', ') + ' }'
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
    if (param.type == 'i64' && I64_MODE == 1) {
      ret = parseI64Constant(ret);
    }
    ret = parseNumerical(ret);
  } else if (param.intertype == 'structvalue') {
    ret = makeLLVMStruct(param.values.map(function(value) { return finalizeLLVMParameter(value, noIndexizeFunctions) }));
  } else if (param.intertype === 'blockaddress') {
    return finalizeBlockAddress(param);
  } else if (param.intertype === 'type') {
    return param.ident; // we don't really want the type here
  } else {
    throw 'invalid llvm parameter: ' + param.intertype;
  }
  assert(param.type || (typeof param === 'string' && param.substr(0, 6) === 'CHECK_'), 'Missing type for param: ' + dump(param));
  if (!noIndexizeFunctions) ret = indexizeFunctions(ret, param.type);
  return ret;
}

function makeSignOp(value, type, op, force) {
  if (I64_MODE == 1 && type == 'i64') {
    return '(tempPair=' + value + ',[' + makeSignOp('tempPair[0]', 'i32', op, force) + ',' + makeSignOp('tempPair[1]', 'i32', op, force) + '])';
  }

  if (isPointerType(type)) type = 'i32'; // Pointers are treated as 32-bit ints
  if (!value) return value;
  var bits, full;
  if (type in Runtime.INT_TYPES) {
    bits = parseInt(type.substr(1));
    full = op + 'Sign(' + value + ', ' + bits + ', ' + Math.floor(correctSpecificSign() && !PGO) + (
      PGO ? ', "' + Debugging.getIdentifier() + '"' : ''
    ) + ')';
    // Always sign/unsign constants at compile time, regardless of CHECK/CORRECT
    if (isNumber(value)) {
      return eval(full).toString();
    }
  }
  if (!correctSigns() && !CHECK_SIGNS && !force) return value;
  if (type in Runtime.INT_TYPES) {
    // shortcuts
    if (!CHECK_SIGNS) {
      if (bits === 32) {
        if (op === 're') {
          return '((' + value + ')|0)';
        } else {
          return '((' + value + ')>>>0)';
        }
      } else if (bits < 32) {
        if (op === 'un') {
          return '((' + value + ')&' + (Math.pow(2, bits)-1) + ')';
        } else {
          return '(tempInt=(' + value + '),(tempInt>=' + Math.pow(2, bits-1) + '?tempInt-' + Math.pow(2, bits) + ':tempInt))';
        }
      }
    }
    return full;
  }
  return value;
}

function makeRounding(value, bits, signed) {
  // TODO: handle roundings of i64s

  // C rounds to 0 (-5.5 to -5, +5.5 to 5), while JS has no direct way to do that.
  // With 32 bits and less, and a signed value, |0 will round it like C does.
  if (bits && bits <= 32 && signed) return '(('+value+')|0)';
  // If the value may be negative, and we care about proper rounding, then use a slow but correct function
  if (signed && correctRoundings()) return 'cRound(' + value + ')';
  // Either this must be positive, so Math.Floor is correct, or we don't care
  return 'Math.floor(' + value + ')';
}

function isUnsignedOp(op, variant) {
  return op in set('udiv', 'urem', 'uitofp', 'zext', 'lshr') || (variant && variant[0] == 'u');
}

function isSignedOp(op, variant) {
  return op in set('sdiv', 'srem', 'sitofp', 'sext', 'ashr') || (variant && variant[0] == 's');
}

function processMathop(item) { with(item) {
  var paramTypes = ['', '', '', ''];
  for (var i = 1; i <= 4; i++) {
    if (item['param'+i]) {
      paramTypes[i-1] = item['param'+i].type || type;
      item['ident'+i] = finalizeLLVMParameter(item['param'+i]);
      if (!isNumber(item['ident'+i])) {
        item['ident'+i] = '(' + item['ident'+i] + ')'; // we may have nested expressions. So enforce the order of operations we want
      }
    } else {
      item['ident'+i] = null; // just so it exists for purposes of reading ident2 etc. later on, and no exception is thrown
    }
  }
  var originalIdent1 = ident1;
  var originalIdent2 = ident2;
  if (isUnsignedOp(op, variant)) {
    ident1 = makeSignOp(ident1, paramTypes[0], 'un');
    ident2 = makeSignOp(ident2, paramTypes[1], 'un');
  } else if (isSignedOp(op, variant)) {
    ident1 = makeSignOp(ident1, paramTypes[0], 're');
    ident2 = makeSignOp(ident2, paramTypes[1], 're');
  }
  var bits = null;
  if (item.type[0] === 'i') {
    bits = parseInt(item.type.substr(1));
  }
  var bitsLeft = ident2 ? ident2.substr(2, ident2.length-3) : null; // remove (i and ), to leave number. This value is important in float ops

  function integerizeBignum(value) {
    return '(tempBigInt=(' + value + '), tempBigInt-tempBigInt%1)';
  }

  if ((type == 'i64' || paramTypes[0] == 'i64' || paramTypes[1] == 'i64') && I64_MODE == 1) {
    function warnI64_1() {
      warnOnce('Arithmetic on 64-bit integers in mode 1 is rounded and flaky, like mode 0, but much slower!');
    }
    switch (op) {
      // basic integer ops
      case 'or': {
        return '[' + ident1 + '[0] | ' + ident2 + '[0], ' + ident1 + '[1] | ' + ident2 + '[1]]';
      }
      case 'and': {
        return '[' + ident1 + '[0] & ' + ident2 + '[0], ' + ident1 + '[1] & ' + ident2 + '[1]]';
      }
      case 'xor': {
        return '[' + ident1 + '[0] ^ ' + ident2 + '[0], ' + ident1 + '[1] ^ ' + ident2 + '[1]]';
      }
      case 'shl': {
        return '[' + ident1 + '[0] << ' + ident2 + ', ' +
                 '('+ident1 + '[1] << ' + ident2 + ') | ((' + ident1 + '[0]&((Math.pow(2, ' + ident2 + ')-1)<<(32-' + ident2 + '))) >> (32-' + ident2 + '))]';
      }
      case 'ashr': {
        return '[('+ident1 + '[0] >> ' + ident2 + ') | ((' + ident1 + '[1]&((Math.pow(2, ' + ident2 + ')-1)<<(32-' + ident2 + '))) >> (32-' + ident2 + ')),' +
                    ident1 + '[1] >> ' + ident2 + ']';
      }
      case 'lshr': {
        return '[('+ident1 + '[0] >>> ' + ident2 + ') | ((' + ident1 + '[1]&((Math.pow(2, ' + ident2 + ')-1)<<(32-' + ident2 + '))) >> (32-' + ident2 + ')),' +
                    ident1 + '[1] >>> ' + ident2 + ']';
      }
      case 'uitofp': case 'sitofp': return ident1 + '[0] + ' + ident1 + '[1]*4294967296';
      case 'icmp': {
        switch (variant) {
          case 'uge': case 'sge': return ident1 + '[1] >= ' + ident2 + '[1] && (' + ident1 + '[1] > '  + ident2 + '[1] || ' +
                                                                                    ident1 + '[0] >= ' + ident2 + '[0])';
          case 'ule': case 'sle': return ident1 + '[1] <= ' + ident2 + '[1] && (' + ident1 + '[1] < '  + ident2 + '[1] || ' +
                                                                                    ident1 + '[0] <= ' + ident2 + '[0])';
          case 'ugt': case 'sgt': return ident1 + '[1] > ' + ident2 + '[1] || (' + ident1 + '[1] == '  + ident2 + '[1] && ' +
                                                                                   ident1 + '[0] > ' + ident2 + '[0])';
          case 'ult': case 'slt': return ident1 + '[1] < ' + ident2 + '[1] || (' + ident1 + '[1] == '  + ident2 + '[1] && ' +
                                                                                   ident1 + '[0] < ' + ident2 + '[0])';
          case 'ne': case 'eq': {
            // We must sign them, so we do not compare -1 to 255 (could have unsigned them both too)
            // since LLVM tells us if <=, >= etc. comparisons are signed, but not == and !=.
            ident1 = makeSignOp(ident1, type, 're');
            ident2 = makeSignOp(ident2, type, 're');
            if (variant === 'eq') {
              return ident1 + '[0] == ' + ident2 + '[0] && ' + ident1 + '[0] == ' + ident2 + '[0]';
            } else {
              return ident1 + '[0] != ' + ident2 + '[0] || ' + ident1 + '[0] != ' + ident2 + '[0]';
            }
          }
          default: throw 'Unknown icmp variant: ' + variant;
        }
      }
      case 'zext': return makeI64(ident1, 0);
      case 'sext': return '(tempInt=' + ident1 + ',' + makeI64('tempInt', 'tempInt<0 ? 4294967295 : 0') + ')';
      case 'trunc': {
        return '((' + ident1 + '[0]) & ' + (Math.pow(2, bitsLeft)-1) + ')';
      }
      case 'select': return ident1 + ' ? ' + makeCopyI64(ident2) + ' : ' + makeCopyI64(ident3);
      case 'ptrtoint': case 'inttoptr': throw 'Pointers cannot be 64-bit!';
      // Dangerous, rounded operations. TODO: Fully emulate
      case 'add': warnI64_1(); return handleOverflow(splitI64(mergeI64(ident1) + '+' + mergeI64(ident2)), bits);
      case 'sub': warnI64_1(); return handleOverflow(splitI64(mergeI64(ident1) + '-' + mergeI64(ident2)), bits);
      case 'sdiv': case 'udiv': warnI64_1(); return splitI64(makeRounding(mergeI64(ident1) + '/' + mergeI64(ident2), bits, op[0] === 's'));
      case 'mul': warnI64_1(); return handleOverflow(splitI64(mergeI64(ident1) + '*' + mergeI64(ident2)), bits);
      case 'urem': warnI64_1(); return splitI64(mergeI64(ident1) + '%' + mergeI64(ident2));
      default: throw 'Unsupported i64 mode 1 op: ' + item.op;
    }
  }

  switch (op) {
    // basic integer ops
    case 'add': return handleOverflow(getFastValue(ident1, '+', ident2, item.type), bits);
    case 'sub': return handleOverflow(getFastValue(ident1, '-', ident2, item.type), bits);
    case 'sdiv': case 'udiv': return makeRounding(getFastValue(ident1, '/', ident2, item.type), bits, op[0] === 's');
    case 'mul': return handleOverflow(getFastValue(ident1, '*', ident2, item.type), bits);
    case 'urem': case 'srem': return getFastValue(ident1, '%', ident2, item.type);
    case 'or': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for or: ' + bits);
        dprint('Warning: 64 bit OR - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.or64(' + ident1 + ', ' + ident2 + ')';
      }
      return ident1 + ' | ' + ident2;
    }
    case 'and': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for and: ' + bits);
        dprint('Warning: 64 bit AND - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.and64(' + ident1 + ', ' + ident2 + ')';
      }
      return ident1 + ' & ' + ident2;
    }
    case 'xor': {
      if (bits > 32) {
        assert(bits === 64, 'Too many bits for xor: ' + bits);
        dprint('Warning: 64 bit XOR - precision limit may be hit on llvm line ' + item.lineNum);
        return 'Runtime.xor64(' + ident1 + ', ' + ident2 + ')';
      }
      return ident1 + ' ^ ' + ident2;
    }
    case 'shl': {
      // Note: Increases in size may reach the 32-bit limit... where our sign can flip. But this may be expected by the code...
      /*
      if (bits >= 32) {
        if (CHECK_SIGNS && !CORRECT_SIGNS) return 'shlSignCheck(' + ident1 + ', ' + ident2 + ')';
        if (CORRECT_SIGNS) {
          var mul = 'Math.pow(2, ' + ident2 + ')';
          if (isNumber(ident2)) mul = eval(mul);
          return ident1 + ' * ' + mul;
        }
      }
      */
      if (bits > 32) return ident1 + '*Math.pow(2,' + ident2 + ')'; // TODO: calculate Math.pow at runtime for consts, and below too
      return ident1 + ' << ' + ident2;
    }
    case 'ashr': {
      if (bits > 32) return integerizeBignum(ident1 + '/Math.pow(2,' + ident2 + ')');
      if (bits === 32) return originalIdent1 + ' >> ' + ident2; // No need to reSign in this case
      return ident1 + ' >> ' + ident2;
    }
    case 'lshr': {
      if (bits > 32) return integerizeBignum(ident1 + '/Math.pow(2,' + ident2 + ')');
      if (bits === 32) return originalIdent1 + ' >>> ' + ident2; // No need to unSign in this case
      return ident1 + ' >>> ' + ident2;
    }
    // basic float ops
    case 'fadd': return getFastValue(ident1, '+', ident2, item.type);
    case 'fsub': return getFastValue(ident1, '-', ident2, item.type);
    case 'fdiv': return getFastValue(ident1, '/', ident2, item.type);
    case 'fmul': return getFastValue(ident1, '*', ident2, item.type);
    case 'uitofp': case 'sitofp': return ident1;
    case 'fptoui': case 'fptosi': return makeRounding(ident1, bitsLeft, op === 'fptosi');

    // TODO: We sometimes generate false instead of 0, etc., in the *cmps. It seemed slightly faster before, but worth rechecking
    //       Note that with typed arrays, these become 0 when written. So that is a potential difference with non-typed array runs.
    case 'icmp': {
      switch (variant) {
        case 'uge': case 'sge': return ident1 + ' >= ' + ident2;
        case 'ule': case 'sle': return ident1 + ' <= ' + ident2;
        case 'ugt': case 'sgt': return ident1 + ' > ' + ident2;
        case 'ult': case 'slt': return ident1 + ' < ' + ident2;
        // We use loose comparisons, which allows false == 0 to be true, etc. Ditto in fcmp
        case 'ne': case 'eq': {
          // We must sign them, so we do not compare -1 to 255 (could have unsigned them both too)
          // since LLVM tells us if <=, >= etc. comparisons are signed, but not == and !=.
          ident1 = makeSignOp(ident1, type, 're');
          ident2 = makeSignOp(ident2, type, 're');
          return ident1 + (variant === 'eq' ? '==' : '!=') + ident2;
        }
        default: throw 'Unknown icmp variant: ' + variant;
      }
    }
    case 'fcmp': {
      switch (variant) {
        // TODO 'o' ones should be 'ordered (no NaN) and',
        //      'u' ones should be 'unordered or'.
        case 'uge': case 'oge': return ident1 + ' >= ' + ident2;
        case 'ule': case 'ole': return ident1 + ' <= ' + ident2;
        case 'ugt': case 'ogt': return ident1 + ' > ' + ident2;
        case 'ult': case 'olt': return ident1 + ' < ' + ident2;
        case 'une': case 'one': return ident1 + ' != ' + ident2;
        case 'ueq': case 'oeq': return ident1 + ' == ' + ident2;
        case 'ord': return '!isNaN(' + ident1 + ') && !isNaN(' + ident2 + ')';
        case 'uno': return 'isNaN(' + ident1 + ') || isNaN(' + ident2 + ')';
        case 'true': return '1';
        default: throw 'Unknown fcmp variant: ' + variant;
      }
    }
    // Note that zext has sign checking, see above. We must guard against -33 in i8 turning into -33 in i32
    // then unsigning that i32... which would give something huge.
    case 'zext': case 'fpext': case 'sext': return ident1;
    case 'fptrunc': return ident1;
    case 'trunc': {
      // Unlike extending, which we just 'do' (by doing nothing),
      // truncating can change the number, e.g. by truncating to an i1
      // in order to get the first bit
      assert(ident2[1] == 'i');
      assert(bitsLeft <= 32, 'Cannot truncate to more than 32 bits, since we use a native & op');
      return '((' + ident1 + ') & ' + (Math.pow(2, bitsLeft)-1) + ')';
    }
    case 'select': return ident1 + ' ? ' + ident2 + ' : ' + ident3;
    case 'ptrtoint': case 'inttoptr': {
      var ret = '';
      if (QUANTUM_SIZE == 1) {
        warnOnce('.ll contains ptrtoint and/or inttoptr. These may be dangerous in QUANTUM == 1. ' +
                 'The safest thing is to investigate every appearance, and modify the source code to avoid this. ' +
                 'Emscripten will print a list of the .ll lines, and also annotate the .js.');
        dprint('  ' + op + ' on .ll line ' + item.lineNum);
        ret = ' /* Warning: ' + op + ', .ll line ' + item.lineNum + ' */';
      }
      return ident1 + ret;
    }
    default: throw 'Unknown mathcmp op: ' + item.op;
  }
} }

// Walks through some intertype data, calling a function at every item. If
// the function returns true, will stop the walk.
// TODO: Use this in analyzer, possibly also in jsifier
function walkInterdata(item, pre, post, obj) {
  if (!item || !item.intertype) return false;
  if (pre && pre(item, obj)) return true;
  var originalObj = obj;
  if (obj && obj.replaceWith) obj = obj.replaceWith; // allow pre to replace the object we pass to all its children
  if (item.value && walkInterdata(item.value, pre, post,  obj)) return true;
  if (item.dependent && walkInterdata(item.dependent, pre, post,  obj)) return true;
  var i;
  for (i = 1; i <= 4; i++) {
    if (item['param'+i] && walkInterdata(item['param'+i], pre, post,  obj)) return true;
  }
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

function parseBlockAddress(segment) {
  return { intertype: 'blockaddress', func: toNiceIdent(segment[2].item.tokens[0].text), label: toNiceIdent(segment[2].item.tokens[2].text), type: 'i32' };
}

function finalizeBlockAddress(param) {
  return Functions.currFunctions[param.func].labelIds[param.label]; // XXX We rely on currFunctions here...?
}

function stripCorrections(param) {
  var m;
  if (m = /^\((.*)\)$/.exec(param)) {
    param = m[1];
  }
  if (m = /^\((\w+)\)&\d+$/.exec(param)) {
    param = m[1];
  }
  if (m = /CHECK_OVERFLOW\(([^,)]*),.*/.exec(param)) {
    param = m[1];
  }
  return param;
}

