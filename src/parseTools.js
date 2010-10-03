// Various tools for parsing llvm

// Simple #if/else/endif preprocessing for a file. Checks if the
// ident checked is true in our global. Also replaces some constants
function preprocess(text, constants) {
  for (constant in constants) {
    text = text.replace(eval('/' + constant + '/g'), constants[constant]);
  }
  var lines = text.split('\n');
  var ret = '';
  var show = true;
  for (var i = 0; i < lines.length; i++) {
    var line = lines[i];
    if (line[0] != '#') {
      if (show) {
        ret += line + '\n';
      }
    } else {
      if (line[1] == 'i') { // if
        var ident = line.substr(4);
        show = !!this[ident] && this[ident] > 0;
      } else if (line[2] == 'l') { // else
        show = !show;
      } else if (line[2] == 'n') { // endif
        show = true;
      } else {
        throw "Unclear preprocessor command: " + line;
      }
    }
  }
  return ret;
}

function addPointing(type) { return type + '*' }
function removePointing(type, num) {
  if (num === 0) return type;
  return type.substr(0, type.length-(num ? num : 1))
}

function pointingLevels(type) {
  if (!type) return 0;
  var ret = 0;
  var len1 = type.length - 1;
  while (type[len1-ret] === '*') {
    ret ++;
  }
  return ret;
}

function toNiceIdent(ident) {
  assert(ident);
  if (parseFloat(ident) == ident) return ident;
  if (ident == 'null') return '0'; // see parseNumerical
  return ident.replace(/[" \.@%:<>,\*]/g, '_');
}

INT_TYPES = searchable('i1', 'i8', 'i16', 'i32', 'i64');
FLOAT_TYPES = searchable('float', 'double');

function isNumberType(type) {
  return type in INT_TYPES || type in FLOAT_TYPES;
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
  return !isNumberType(type) && type[0] == '%';
}

function isStructType(type) {
  if (isPointerType(type)) return false;
  if (new RegExp(/^\[\d+\ x\ (.*)\]/g).test(type)) return true; // [15 x ?] blocks. Like structs
  // See comment in isStructPointerType()
  return !isNumberType(type) && type[0] == '%';
}

function isPointerType(type) { // TODO!
  return pointingLevels(type) > 0;
}

function isVoidType(type) {
  return type == 'void';
}

function isType(type) { // TODO!
  return isVoidType(type) || isNumberType(type) || isStructType(type) || isPointerType(type);
}

// Detects a function definition, ([...|type,[type,...]])
function isFunctionDef(token) {
  var text = token.text;
  var pointing = pointingLevels(text);
  var nonPointing = removePointing(text, pointing);
  if (nonPointing[0] != '(' || nonPointing.substr(-1) != ')')
    return false;
  if (nonPointing == '(...)') return true;
  if (!token.item) return false;
  var fail = false;
  splitTokenList(token.item[0].tokens).forEach(function(segment) {
    var subtoken = segment[0];
    fail = fail || !isType(subtoken.text) || segment.length > 1;
  });
  return !fail;
}

function addIdent(token) {
  token.ident = token.text;
  return token;
}

function combineTokens(tokens) {
  var ret = {
    lineNum: tokens[0].lineNum,
    text: '',
    tokens: [],
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
  while (tokens[i].text != ';') i++;
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
  if (tokens.slice(-1)[0].text != ',') tokens.push({text:','});
  var ret = [];
  var seg = [];
  var SPLITTERS = searchable(',', 'to'); // 'to' can separate parameters as well...
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
    child: child,
  };
}

function makeReintegrator(afterFunc) {
  // reintegration - find intermediate representation-parsed items and
  // place back in parents TODO: Optimize this code to optimal O(..)
  return {
    process: function(items) {
      var ret = [];
      for (var i = 0; i < items.length; i++) {
        var found = false;
        if (items[i] && items[i].parentSlot) {
          var child = items[i];
          for (var j = 0; j < items.length; j++) {
            if (items[j] && items[j].lineNum == items[i].parentLineNum) {
              var parent = items[j];
              // process the pair
              parent[child.parentSlot] = child;
              delete child.parentLineNum;
              afterFunc.call(this, parent, child);

              items[i] = null;
              items[j] = null;
              found = true;
              break;
            }
          }
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
  var absIndex = 0;
  while (params.length > 0) {
    var i = 0;
    while (params[i].text != ',') i++;
    var segment = params.slice(0, i);
    params = params.slice(i+1);
    segment = cleanSegment(segment);
    if (segment.length == 1) {
      if (segment[0].text == '...') {
        ret.push({
          intertype: 'varargs',
        });
      } else {
        // Clang sometimes has a parameter with just a type,
        // no name... the name is implied to be %{the index}
        ret.push({
          intertype: 'value',
          type: segment[0],
          value: null,
          ident: '_' + absIndex,
        });
      }
    } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
      ret.push(parseLLVMFunctionCall(segment));
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
        type: segment[0],
        value: segment[1],
        ident: parseNumerical(segment[1].text),
      });
      //          } else {
      //            throw "what is this params token? " + JSON.stringify(segment);
    }
    absIndex ++;
  }
  return ret;
}

// Segment ==> Parameter
function parseLLVMSegment(segment) {
  if (segment.length == 1) {
    return {
      intertype: 'value',
      ident: segment[0].text,
      type: '?',
    };
  } else if (segment[0].text in PARSABLE_LLVM_FUNCTIONS) {
    return parseLLVMFunctionCall([{text: '?'}].concat(segment));
  } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
    return parseLLVMFunctionCall(segment);
  } else {
    return {
      intertype: 'value',
      ident: segment[1].text,
      type: segment[0].text,
    };
  }
}

function cleanSegment(segment) {
  if (segment.length == 1) return segment;
  while (['noalias', 'sret', 'nocapture', 'nest', 'zeroext', 'signext'].indexOf(segment[1].text) != -1) {
    segment.splice(1, 1);
  }
  return segment;
}

PARSABLE_LLVM_FUNCTIONS = searchable('getelementptr', 'bitcast', 'inttoptr', 'ptrtoint');

// Parses a function call of form
//         TYPE functionname MODIFIERS (...)
// e.g.
//         i32* getelementptr inbounds (...)
function parseLLVMFunctionCall(segment) {
  segment = segment.slice(0);
  segment = cleanSegment(segment);
  // Remove additional modifiers
  if (!segment[2] || !segment[2].item) {
    segment.splice(2, 1);
  }
  assertTrue(['inreg', 'byval'].indexOf(segment[1].text) == -1);
  assert(segment[1].text in PARSABLE_LLVM_FUNCTIONS);
  var ret = {
    intertype: segment[1].text,
    type: segment[0],
    params: parseParamTokens(segment[2].item[0].tokens),
  };
  ret.ident = toNiceIdent(ret.params[0].ident);
  return ret;
}

function cleanOutTokens(filterOut, tokens, index) {
  while (filterOut.indexOf(tokens[index].text) != -1) {
    tokens.splice(index, 1);
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
      return 'Infinity';
    } else {
      return 'NaN';
    }
  }
  e -= 1023; // offset
  var absolute = ((((a | 0x100000) * 1.0) / Math.pow(2,52-32)) * Math.pow(2, e)) + (((b * 1.0) / Math.pow(2, 52)) * Math.pow(2, e));
  return (absolute * (neg ? -1 : 1)).toString();
}

function parseNumerical(value, type) {
  if ((!type || type == 'double' || type == 'float') && value.substr(0,2) == '0x') {
    // Hexadecimal double value, as the llvm docs say,
    // "The one non-intuitive notation for constants is the hexadecimal form of floating point constants."
    return IEEEUnHex(value);
  }
  if (value == 'null') {
    // NULL *is* 0, in C/C++. No JS null! (null == 0 is false, etc.)
    return '0';
  }
  return value;
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

//! Returns the size of a field, as C/C++ would have it (in 32-bit,
//! for now).
//! @param field The field type, by name
//! @param alone Whether this is inside a structure (so padding is
//!              used) or alone (line in char*, where no padding is done).
function getNativeFieldSize(field, alone) {
  var size;
  if (QUANTUM_SIZE > 1) {
    size = {
      'i1': alone ? 1 : 4, // inside a struct, aligned to 4,
      'i8': alone ? 1 : 4, // most likely...? XXX
      'i16': alone ? 2 : 4, // ditto
      'i32': 4,
      'i64': 8,
      'float': 4,
      'double':8,
    }[field]; // XXX 32/64 bit stuff
    if (!size) {
      size = 4; // Must be a pointer XXX 32/64
    }
  } else {
    size = 1;
  }
  return size;
}

