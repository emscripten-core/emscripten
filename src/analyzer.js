//"use strict";

// Analyze intertype data. Calculates things that are necessary in order
// to do the final conversion into JavaScript later, for example,
// properties of variables, loop structures of functions, etc.

var VAR_NATIVE = 'native';
var VAR_NATIVIZED = 'nativized';
var VAR_EMULATED = 'emulated';

var ENTRY_IDENT = toNiceIdent('%0');

function recomputeLines(func) {
  func.lines = func.labels.map(function(label) { return label.lines }).reduce(concatenator, []);
}

// Handy sets

var BRANCH_INVOKE = set('branch', 'invoke');
var LABEL_ENDERS = set('branch', 'return', 'switch');
var SIDE_EFFECT_CAUSERS = set('call', 'invoke', 'atomic');
var UNUNFOLDABLE = set('value', 'structvalue', 'type', 'phiparam');
var SHADOW_FLIP = { i64: 'double', double: 'i64' }; //, i32: 'float', float: 'i32' };

// Analyzer

function analyzer(data, sidePass) {
  //B.start('analyzer');
  var mainPass = !sidePass;

  var item = { items: data };
  var data = item;

  var newTypes = {};

  // Gather
  // Single-liners
  ['globalVariable', 'functionStub', 'unparsedFunction', 'unparsedGlobals', 'unparsedTypes', 'alias'].forEach(function(intertype) {
    var temp = splitter(item.items, function(item) { return item.intertype == intertype });
    item.items = temp.leftIn;
    item[intertype + 's'] = temp.splitOut;
  });
  var temp = splitter(item.items, function(item) { return item.intertype == 'type' });
  item.items = temp.leftIn;
  temp.splitOut.forEach(function(type) {
    //dprint('types', 'adding defined type: ' + type.name_);
    Types.types[type.name_] = type;
    newTypes[type.name_] = 1;
    if (QUANTUM_SIZE === 1) {
      Types.fatTypes[type.name_] = copy(type);
    }
  });

  // Functions & labels
  item.functions = [];
  var currLabelFinished = false; // Sometimes LLVM puts a branch in the middle of a label. We need to ignore all lines after that.
  item.items.sort(function(a, b) { return a.lineNum - b.lineNum });
  for (var i = 0; i < item.items.length; i++) {
    var subItem = item.items[i];
    assert(subItem.lineNum);
    if (subItem.intertype == 'function') {
      item.functions.push(subItem);
      subItem.endLineNum = null;
      subItem.lines = []; // We will fill in the function lines after the legalizer, since it can modify them
      subItem.labels = [];
      subItem.forceEmulated = false;

      // no explicit 'entry' label in clang on LLVM 2.8 - most of the time, but not all the time! - so we add one if necessary
      if (item.items[i+1].intertype !== 'label') {
        item.items.splice(i+1, 0, {
          intertype: 'label',
          ident: ENTRY_IDENT,
          lineNum: subItem.lineNum + '.5'
        });
      }
    } else if (subItem.intertype == 'functionEnd') {
      item.functions.slice(-1)[0].endLineNum = subItem.lineNum;
    } else if (subItem.intertype == 'label') {
      item.functions.slice(-1)[0].labels.push(subItem);
      subItem.lines = [];
      currLabelFinished = false;
    } else if (item.functions.length > 0 && item.functions.slice(-1)[0].endLineNum === null) {
      // Internal line
      if (!currLabelFinished) {
        item.functions.slice(-1)[0].labels.slice(-1)[0].lines.push(subItem); // If this line fails, perhaps missing a label?
        if (subItem.intertype in LABEL_ENDERS) {
          currLabelFinished = true;
        }
      } else {
        print('// WARNING: content after a branch in a label, line: ' + subItem.lineNum);
      }
    } else {
      throw 'ERROR: what is this? ' + dump(subItem);
    }
  }
  delete item.items;

  // CastAway - try to remove bitcasts of double<-->i64, which LLVM sometimes generates unnecessarily
  // (load a double, convert to i64, use as i64).
  // We optimize this by checking if there are such bitcasts. If so we create a shadow
  // variable that is of the other type, and use that in the relevant places. (As SSA, this is valid, and
  // variable elimination later will remove the double load if it is no longer needed.)
  //
  // Note that aside from being an optimization, this is needed for correctness in some cases: If code
  // assumes it can bitcast a double to an i64 and back and forth without loss, that may be violated
  // due to NaN canonicalization.
  function castAway() {
    if (USE_TYPED_ARRAYS != 2) return;

    item.functions.forEach(function(func) {
      var has = false;
      func.labels.forEach(function(label) {
        var lines = label.lines;
        for (var i = 0; i < lines.length; i++) {
          var line = lines[i];
          if (line.intertype == 'bitcast' && line.type in SHADOW_FLIP) {
            has = true;
          }
        }
      });
      if (!has) return;
      // there are integer<->floating-point bitcasts, create shadows for everything
      var shadowed = {};
      func.labels.forEach(function(label) {
        var lines = label.lines;
        var i = 0;
        while (i < lines.length) {
        var lines = label.lines;
          var line = lines[i];
          if (line.intertype == 'load' && line.type in SHADOW_FLIP) {
            if (line.pointer.intertype != 'value') { i++; continue } // TODO
            shadowed[line.assignTo] = 1;
            var shadow = line.assignTo + '$$SHADOW';
            var flip = SHADOW_FLIP[line.type];
            lines.splice(i + 1, 0, { // if necessary this element will be legalized in the next phase
              tokens: null,
              indent: 2,
              lineNum: line.lineNum + 0.5,
              assignTo: shadow,
              intertype: 'load',
              pointerType: flip + '*',
              type: flip,
              valueType: flip,
              pointer: {
               intertype: 'value',
               ident: line.pointer.ident,
               type: flip + '*'
              },
              align: line.align,
              ident: line.ident
            });
            // note: no need to update func.lines, it is generated in a later pass
            i++;
          }
          i++;
        }
      });
      // use shadows where possible
      func.labels.forEach(function(label) {
        var lines = label.lines;
        for (var i = 0; i < lines.length; i++) {
          var line = lines[i];
          if (line.intertype == 'bitcast' && line.type in SHADOW_FLIP && line.ident in shadowed) {
            var shadow = line.ident + '$$SHADOW';
            line.params[0].ident = shadow;
            line.params[0].type = line.type;
            line.type2 = line.type;
          }
        }
      });
    });
  }

  // Legalize LLVM unrealistic types into realistic types.
  //
  // With full LLVM optimizations, it can generate types like i888 which do not exist in
  // any actual hardware implementation, but are useful during optimization. LLVM then
  // legalizes these types into real ones during code generation. Sadly, there is no LLVM
  // IR pass to legalize them, which would have been useful and nice from a design perspective.
  // The LLVM community is also not interested in receiving patches to implement that
  // functionality, since it would duplicate existing code from the code generation
  // component. Therefore, we implement legalization here in Emscripten.
  //
  // Currently we just legalize completely unrealistic types into bundles of i32s, and just
  // the most common instructions that can be involved with such types: load, store, shifts,
  // trunc and zext.
  function legalizer() {
    // Legalization
    if (USE_TYPED_ARRAYS == 2) {
      function getLegalVars(base, bits, allowLegal) {
        bits = bits || 32; // things like pointers are all i32, but show up as 0 bits from getBits
        if (allowLegal && bits <= 32) return [{ intertype: 'value', ident: base + ('i' + bits in Compiletime.INT_TYPES ? '' : '$0'), bits: bits, type: 'i' + bits }];
        if (isNumber(base)) return getLegalLiterals(base, bits);
        if (base[0] == '{') {
          warnOnce('seeing source of illegal data ' + base + ', likely an inline struct - assuming zeroinit');
          return getLegalLiterals('0', bits);
        }
        var ret = new Array(Math.ceil(bits/32));
        var i = 0;
        if (base == 'zeroinitializer' || base == 'undef') base = 0;
        while (bits > 0) {
          ret[i] = { intertype: 'value', ident: base ? base + '$' + i : '0', bits: Math.min(32, bits), type: 'i' + Math.min(32, bits) };
          bits -= 32;
          i++;
        }
        return ret;
      }
      function getLegalLiterals(text, bits) {
        var parsed = parseArbitraryInt(text, bits);
        var ret = new Array(Math.ceil(bits/32));
        var i = 0;
        while (bits > 0) {
          ret[i] = { intertype: 'value', ident: (parsed[i]|0).toString(), bits: Math.min(32, bits), type: 'i' + Math.min(32, bits) }; // resign all values
          bits -= 32;
          i++;
        }
        return ret;
      }
      function getLegalStructuralParts(value) {
        return value.params.slice(0);
      }
      function getLegalParams(params, bits) {
        return params.map(function(param) {
          var value = param.value || param;
          if (isNumber(value.ident)) {
            return getLegalLiterals(value.ident, bits);
          } else if (value.intertype == 'structvalue') {
            return getLegalStructuralParts(value).map(function(part) {
              part.bits = part.type.substr(1); // can be some nested IR, like LLVM calls
              return part;
            });
          } else {
            return getLegalVars(value.ident, bits);
          }
        });
      }
      // Uses the right factor to multiply line numbers by so that they fit in between
      // the line[i] and the line after it
      function interpLines(lines, i, toAdd) {
        var prev = i >= 0 ? lines[i].lineNum : -1;
        var next = (i < lines.length-1) ? lines[i+1].lineNum : (lines[i].lineNum + 0.5);
        var factor = (next - prev)/(4*toAdd.length+3);
        for (var k = 0; k < toAdd.length; k++) {
          toAdd[k].lineNum = prev + ((k+1)*factor);
          assert(k == 0 || toAdd[k].lineNum > toAdd[k-1].lineNum);
        }
      }
      function removeAndAdd(lines, i, toAdd) {
        var item = lines[i];
        interpLines(lines, i, toAdd);
        Array.prototype.splice.apply(lines, [i, 1].concat(toAdd));
        if (i > 0) assert(lines[i].lineNum > lines[i-1].lineNum);
        if (i + toAdd.length < lines.length) assert(lines[i + toAdd.length - 1].lineNum < lines[i + toAdd.length].lineNum);
        return toAdd.length;
      }
      function legalizeFunctionParameters(params) {
        var i = 0;
        while (i < params.length) {
          var param = params[i];
          if (param.intertype == 'value' && isIllegalType(param.type)) {
            var toAdd = getLegalVars(param.ident, getBits(param.type)).map(function(element) {
              return {
                intertype: 'value',
                type: 'i' + element.bits,
                ident: element.ident,
                byval: 0
              };
            });
            Array.prototype.splice.apply(params, [i, 1].concat(toAdd));
            i += toAdd.length;
            continue;
          } else if (param.intertype == 'structvalue') {
            // 'flatten' out the struct into scalars
            var toAdd = param.params;
            toAdd.forEach(function(param) {
              param.byval = 0;
            });
            Array.prototype.splice.apply(params, [i, 1].concat(toAdd));
            continue; // do not increment i; proceed to process the new params
          }
          i++;
        }
      }
      function fixUnfolded(item) {
        // Unfolded items may need some correction to work properly in the global scope
        if (item.intertype in MATHOPS) {
          item.op = item.intertype;
          item.intertype = 'mathop';
        }
      }
      data.functions.forEach(function(func) {
        // Legalize function params
        legalizeFunctionParameters(func.params);
        // Legalize lines in labels
        var tempId = 0;
        func.labels.forEach(function(label) {
          if (dcheck('legalizer')) dprint('zz legalizing: \n' + dump(label.lines));
          var i = 0, bits;
          while (i < label.lines.length) {
            var item = label.lines[i];
            var value = item;
            // Check if we need to legalize here, and do some trivial legalization along the way
            var isIllegal = false;
            walkInterdata(item, function(item) {
              if (item.intertype == 'getelementptr' || (item.intertype == 'call' && item.ident in LLVM.INTRINSICS_32)) {
                // Turn i64 args into i32
                for (var i = 0; i < item.params.length; i++) {
                  if (item.params[i].type == 'i64') item.params[i].type = 'i32';
                }
              } else if (item.intertype == 'inttoptr') {
                var input = item.params[0];
                if (input.type == 'i64') input.type = 'i32'; // inttoptr can only care about 32 bits anyhow since pointers are 32-bit
              }
              if (isIllegalType(item.valueType) || isIllegalType(item.type)) {
                isIllegal = true;
              } else if ((item.intertype == 'load' || item.intertype == 'store') && isStructType(item.valueType)) {
                isIllegal = true; // storing an entire structure is illegal
              } else if (item.intertype == 'mathop' && item.op == 'trunc' && isIllegalType(item.params[1].ident)) { // trunc stores target value in second ident
                isIllegal = true;
              }
            });
            if (!isIllegal) {
              //if (dcheck('legalizer')) dprint('no need to legalize \n' + dump(item));
              i++;
              continue;
            }
            // Unfold this line. If we unfolded, we need to return and process the lines we just
            // generated - they may need legalization too
            var unfolded = [];
            walkAndModifyInterdata(item, function(subItem) {
              // Unfold all non-value interitems that we can, and also unfold all numbers (doing the latter
              // makes it easier later since we can then assume illegal expressions are always variables
              // accessible through ident$x, and not constants we need to parse then and there)
              if (subItem != item && (!(subItem.intertype in UNUNFOLDABLE) ||
                                     (subItem.intertype == 'value' && isNumber(subItem.ident) && isIllegalType(subItem.type)))) {
                if (item.intertype == 'phi') {
                  assert(subItem.intertype == 'value' || subItem.intertype == 'structvalue' || subItem.intertype in PARSABLE_LLVM_FUNCTIONS, 'We can only unfold some expressions in phis');
                  // we must handle this in the phi itself, if we unfold normally it will not be pushed back with the phi
                } else {
                  var tempIdent = '$$etemp$' + (tempId++);
                  subItem.assignTo = tempIdent;
                  unfolded.unshift(subItem);
                  fixUnfolded(subItem);
                  return { intertype: 'value', ident: tempIdent, type: subItem.type };
                }
              } else if (subItem.intertype == 'switch' && isIllegalType(subItem.type)) {
                subItem.switchLabels.forEach(function(switchLabel) {
                  if (switchLabel.value[0] != '$') {
                    var tempIdent = '$$etemp$' + (tempId++);
                    unfolded.unshift({
                      assignTo: tempIdent,
                      intertype: 'value',
                      ident: switchLabel.value,
                      type: subItem.type
                    });
                    switchLabel.value = tempIdent;
                  }
                });
              }
            });
            if (unfolded.length > 0) {
              interpLines(label.lines, i-1, unfolded);
              Array.prototype.splice.apply(label.lines, [i, 0].concat(unfolded));
              continue; // remain at this index, to unfold newly generated lines
            }
            // This is an illegal-containing line, and it is unfolded. Legalize it now
            dprint('legalizer', 'Legalizing ' + item.intertype + ' at line ' + item.lineNum);
            var finalizer = null;
            switch (item.intertype) {
              case 'store': {
                var toAdd = [];
                bits = getBits(item.valueType);
                var elements = getLegalParams([item.value], bits)[0];
                var j = 0;
                elements.forEach(function(element) {
                  var tempVar = '$st$' + (tempId++) + '$' + j;
                  toAdd.push({
                    intertype: 'getelementptr',
                    assignTo: tempVar,
                    ident: item.pointer.ident,
                    type: '[0 x i32]*',
                    params: [
                      { intertype: 'value', ident: item.pointer.ident, type: '[0 x i32]*' }, // technically a bitcase is needed in llvm, but not for us
                      { intertype: 'value', ident: '0', type: 'i32' },
                      { intertype: 'value', ident: j.toString(), type: 'i32' }
                    ],
                  });
                  var actualSizeType = 'i' + element.bits; // The last one may be smaller than 32 bits
                  toAdd.push({
                    intertype: 'store',
                    valueType: actualSizeType,
                    value: { intertype: 'value', ident: element.ident, type: actualSizeType },
                    pointer: { intertype: 'value', ident: tempVar, type: actualSizeType + '*' },
                    ident: tempVar,
                    pointerType: actualSizeType + '*',
                    align: item.align,
                  });
                  j++;
                });
                Types.needAnalysis['[0 x i32]'] = 0;
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              // call, return: Return the first 32 bits, the rest are in temp
              case 'call': {
                var toAdd = [value];
                // legalize parameters
                legalizeFunctionParameters(value.params);
                // legalize return value, if any
                var returnType = getReturnType(item.type);
                if (value.assignTo && isIllegalType(returnType)) {
                  bits = getBits(returnType);
                  var elements = getLegalVars(item.assignTo, bits);
                  // legalize return value
                  value.assignTo = elements[0].ident;
                  for (var j = 1; j < elements.length; j++) {
                    var element = elements[j];
                    toAdd.push({
                      intertype: 'value',
                      assignTo: element.ident,
                      type: 'i' + element.bits,
                      ident: 'tempRet' + (j - 1)
                    });
                    assert(j<10); // TODO: dynamically create more than 10 tempRet-s
                  }
                }
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'landingpad': {
                // not much to legalize
                i++;
                continue;
              }
              case 'return': {
                bits = getBits(item.type);
                var elements = getLegalVars(item.value.ident, bits);
                item.value.ident = '(';
                for (var j = 1; j < elements.length; j++) {
                  item.value.ident += 'tempRet' + (j-1) + '=' + elements[j].ident + ',';
                }
                item.value.ident += elements[0].ident + ')';
                i++;
                continue;
              }
              case 'invoke': {
                legalizeFunctionParameters(value.params);
                // We can't add lines after this, since invoke already modifies control flow. So we handle the return in invoke
                i++;
                continue;
              }
              case 'value': {
                bits = getBits(value.type);
                var elements = getLegalVars(item.assignTo, bits);
                var values = getLegalLiterals(item.ident, bits);
                var j = 0;
                var toAdd = elements.map(function(element) {
                  return {
                    intertype: 'value',
                    assignTo: element.ident,
                    type: 'i' + bits,
                    ident: values[j++].ident
                  };
                });
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'structvalue': {
                bits = getBits(value.type);
                var elements = getLegalVars(item.assignTo, bits);
                var toAdd = [];
                for (var j = 0; j < item.params.length; j++) {
                  toAdd[j] = {
                    intertype: 'value',
                    assignTo: elements[j].ident,
                    type: 'i32',
                    ident: item.params[j].ident
                  };
                }
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'load': {
                bits = getBits(value.valueType);
                var elements = getLegalVars(item.assignTo, bits);
                var j = 0;
                var toAdd = [];
                elements.forEach(function(element) {
                  var tempVar = '$ld$' + (tempId++) + '$' + j;
                  toAdd.push({
                    intertype: 'getelementptr',
                    assignTo: tempVar,
                    ident: value.pointer.ident,
                    type: '[0 x i32]*',
                    params: [
                      { intertype: 'value', ident: value.pointer.ident, type: '[0 x i32]*' }, // technically bitcast is needed in llvm, but not for us
                      { intertype: 'value', ident: '0', type: 'i32' },
                      { intertype: 'value', ident: j.toString(), type: 'i32' }
                    ]
                  });
                  var newItem = {
                    intertype: 'load',
                    assignTo: element.ident,
                    pointerType: 'i32*',
                    valueType: 'i32',
                    type: 'i32',
                    pointer: { intertype: 'value', ident: tempVar, type: 'i32*' },
                    ident: tempVar,
                    align: value.align
                  };
                  var newItem2 = null;
                  // The last one may be smaller than 32 bits
                  if (element.bits < 32) {
                    newItem.assignTo += '$preadd$';
                    newItem2 = {
                      intertype: 'mathop',
                      op: 'and',
                      assignTo: element.ident,
                      type: 'i32',
                      params: [{
                        intertype: 'value',
                        type: 'i32',
                        ident: newItem.assignTo
                      }, {
                        intertype: 'value',
                        type: 'i32',
                        ident: (0xffffffff >>> (32 - element.bits)).toString()
                      }],
                    };
                  }
                  toAdd.push(newItem);
                  if (newItem2) toAdd.push(newItem2);
                  j++;
                });
                Types.needAnalysis['[0 x i32]'] = 0;
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'phi': {
                bits = getBits(value.type);
                var toAdd = [];
                var elements = getLegalVars(item.assignTo, bits);
                var j = 0;
                var values = getLegalParams(value.params, bits);
                elements.forEach(function(element) {
                  var k = 0;
                  toAdd.push({
                    intertype: 'phi',
                    assignTo: element.ident,
                    type: 'i' + element.bits,
                    params: value.params.map(function(param) {
                      return {
                        intertype: 'phiparam',
                        label: param.label,
                        value: values[k++][j]
                      };
                    })
                  });
                  j++;
                });
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'switch': {
                i++;
                continue; // special case, handled in makeComparison
              }
              case 'va_arg': {
                assert(value.type == 'i64');
                assert(value.value.type == 'i32*', value.value.type);
                i += removeAndAdd(label.lines, i, range(2).map(function(x) {
                  return {
                    intertype: 'va_arg',
                    assignTo: value.assignTo + '$' + x,
                    type: 'i32',
                    value: {
                      intertype: 'value',
                      ident: value.value.ident, // We read twice from the same i32* var, incrementing // + '$' + x,
                      type: 'i32*'
                    }
                  };
                }));
                continue;
              }
              case 'extractvalue': { // XXX we assume 32-bit alignment in extractvalue/insertvalue,
                                     // but in theory they can run on packed structs too (see use getStructuralTypePartBits)
                // potentially legalize the actual extracted value too if it is >32 bits, not just the extraction in general
                var index = item.indexes[0][0].text;
                var parts = getStructureTypeParts(item.type);
                var indexedType = parts[index];
                var targetBits = getBits(indexedType);
                var sourceBits = getBits(item.type);
                var elements = getLegalVars(item.assignTo, targetBits, true); // possibly illegal
                var sourceElements = getLegalVars(item.ident, sourceBits); // definitely illegal
                var toAdd = [];
                var sourceIndex = 0;
                for (var partIndex = 0; partIndex < parts.length; partIndex++) {
                  if (partIndex == index) {
                    for (var j = 0; j < elements.length; j++) {
                      toAdd.push({
                        intertype: 'value',
                        assignTo: elements[j].ident,
                        type: 'i' + elements[j].bits,
                        ident: sourceElements[sourceIndex+j].ident
                      });
                    }
                    break;
                  }
                  sourceIndex += getStructuralTypePartBits(parts[partIndex])/32;
                }
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'insertvalue': {
                var index = item.indexes[0][0].text; // the modified index
                var parts = getStructureTypeParts(item.type);
                var indexedType = parts[index];
                var indexBits = getBits(indexedType);
                var bits = getBits(item.type); // source and target
                bits = getBits(value.type);
                var toAdd = [];
                var elements = getLegalVars(item.assignTo, bits);
                var sourceElements = getLegalVars(item.ident, bits);
                var indexElements = getLegalVars(item.value.ident, indexBits, true); // possibly legal
                var sourceIndex = 0;
                for (var partIndex = 0; partIndex < parts.length; partIndex++) {
                  var currNum = getStructuralTypePartBits(parts[partIndex])/32;
                  for (var j = 0; j < currNum; j++) {
                    toAdd.push({
                      intertype: 'value',
                      assignTo: elements[sourceIndex+j].ident,
                      type: 'i' + elements[sourceIndex+j].bits,
                      ident: partIndex == index ? indexElements[j].ident : sourceElements[sourceIndex+j].ident
                    });
                  }
                  sourceIndex += currNum;
                }
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
              case 'bitcast': {
                var inType = item.type2;
                var outType = item.type;
                if ((inType in Compiletime.INT_TYPES && outType in Compiletime.FLOAT_TYPES) ||
                    (inType in Compiletime.FLOAT_TYPES && outType in Compiletime.INT_TYPES)) {
                  i++;
                  continue; // special case, handled in processMathop
                }
                // fall through
              }
              case 'inttoptr': case 'ptrtoint': case 'zext': case 'sext': case 'trunc': case 'ashr': case 'lshr': case 'shl': case 'or': case 'and': case 'xor': {
                value = {
                  op: item.intertype,
                  variant: item.variant,
                  type: item.type,
                  params: item.params
                };
                // fall through
              }
              case 'mathop': {
                var toAdd = [];
                var sourceBits = getBits(value.params[0].type);
                // All mathops can be parametrized by how many shifts we do, and how big the source is
                var shifts = 0;
                var targetBits = sourceBits;
                var processor = null;
                var signed = false;
                switch (value.op) {
                  case 'ashr': {
                    signed = true;
                    // fall through
                  }
                  case 'lshr': {
                    shifts = parseInt(value.params[1].ident);
                    break;
                  }
                  case 'shl': {
                    shifts = -parseInt(value.params[1].ident);
                    break;
                  }
                  case 'sext': {
                    signed = true;
                    // fall through
                  }
                  case 'trunc': case 'zext': case 'ptrtoint': {
                    targetBits = getBits(value.params[1] ? value.params[1].ident : value.type);
                    break;
                  }
                  case 'inttoptr': {
                    targetBits = 32;
                    break;
                  }
                  case 'bitcast': {
                    if (!sourceBits) {
                      // we can be asked to bitcast doubles or such to integers, handle that as best we can (if it's a double that
                      // was an x86_fp80, this code will likely break when called)
                      sourceBits = targetBits = Runtime.getNativeTypeSize(value.params[0].type);
                      warn('legalizing non-integer bitcast on ll #' + item.lineNum);
                    }
                    break;
                  }
                  case 'select': {
                    sourceBits = targetBits = getBits(value.params[1].type);
                    var params = getLegalParams(value.params.slice(1), sourceBits);
                    processor = function(result, j) {
                      return {
                        intertype: 'mathop',
                        op: 'select',
                        type: 'i' + params[0][j].bits,
                        params: [
                          value.params[0],
                          { intertype: 'value', ident: params[0][j].ident, type: 'i' + params[0][j].bits },
                          { intertype: 'value', ident: params[1][j].ident, type: 'i' + params[1][j].bits }
                        ]
                      };
                    };
                    break;
                  }
                  case 'or': case 'and': case 'xor': case 'icmp': {
                    var otherElements = getLegalVars(value.params[1].ident, sourceBits);
                    processor = function(result, j) {
                      return {
                        intertype: 'mathop',
                        op: value.op,
                        variant: value.variant,
                        type: 'i' + otherElements[j].bits,
                        params: [
                          result,
                          { intertype: 'value', ident: otherElements[j].ident, type: 'i' + otherElements[j].bits }
                        ]
                      };
                    };
                    if (value.op == 'icmp') {
                      if (sourceBits == 64) { // handle the i64 case in processMathOp, where we handle full i64 math
                        i++;
                        continue;
                      }
                      finalizer = function() {
                        var ident = '';
                        for (var i = 0; i < targetElements.length; i++) {
                          if (i > 0) {
                            switch(value.variant) {
                              case 'eq': ident += '&'; break;
                              case 'ne': ident += '|'; break;
                              default: throw 'unhandleable illegal icmp: ' + value.variant;
                            }
                          }
                          ident += targetElements[i].ident;
                        }
                        return {
                          intertype: 'value',
                          ident: ident,
                          type: 'rawJS',
                          assignTo: item.assignTo
                        };
                      }
                    }
                    break;
                  }
                  case 'add': case 'sub': case 'sdiv': case 'udiv': case 'mul': case 'urem': case 'srem': {
                    if (sourceBits < 32) {
                      // when we add illegal types like i24, we must work on the singleton chunks
                      item.assignTo += '$0';
                      item.params[0].ident += '$0';
                      item.params[1].ident += '$0';
                    }
                    // fall through
                  }
                  case 'uitofp': case 'sitofp': case 'fptosi': case 'fptoui': {
                    // We cannot do these in parallel chunks of 32-bit operations. We will handle these in processMathop
                    i++;
                    continue;
                  }
                  default: throw 'Invalid mathop for legalization: ' + [value.op, item.lineNum, dump(item)];
                }
                // Do the legalization
                var sourceElements = getLegalVars(value.params[0].ident, sourceBits, true);
                if (!isNumber(shifts)) {
                  // We can't statically legalize this, do the operation at runtime TODO: optimize
                  assert(sourceBits == 64, 'TODO: handle nonconstant shifts on != 64 bits');
                  assert(PRECISE_I64_MATH, 'Must have precise i64 math for non-constant 64-bit shifts');
                  Types.preciseI64MathUsed = 1;
                  value.intertype = 'value';
                  value.ident = makeVarDef(value.assignTo) + '$0=' +
                      asmCoercion('_bitshift64' + value.op[0].toUpperCase() + value.op.substr(1) + '(' + 
                        asmCoercion(sourceElements[0].ident, 'i32') + ',' +
                        asmCoercion(sourceElements[1].ident, 'i32') + ',' +
                        asmCoercion(value.params[1].ident + '$0', 'i32') + ')', 'i32'
                      ) + ';' +
                      makeVarDef(value.assignTo) + '$1=tempRet0;';
                  value.vars = [[value.assignTo + '$0', 'i32'], [value.assignTo + '$1', 'i32']];
                  value.assignTo = null;
                  i++;
                  continue;
                }
                var targetElements = getLegalVars(item.assignTo, targetBits);
                var sign = shifts >= 0 ? 1 : -1;
                var shiftOp = shifts >= 0 ? 'shl' : 'lshr';
                var shiftOpReverse = shifts >= 0 ? 'lshr' : 'shl';
                var whole = (shifts/32)|0; // Remove fractional part either for positive or negative number.
                var fraction = Math.abs(shifts % 32);
                if (signed) {
                  var signedFill = {
                    intertype: 'mathop',
                    op: 'select',
                    variant: 's',
                    type: 'i32',
                    params: [{
                      intertype: 'mathop',
                      op: 'icmp',
                      variant: 'slt',
                      type: 'i32',
                      params: [
                        { intertype: 'value', ident: sourceElements[sourceElements.length-1].ident, type: 'i' + Math.min(sourceBits, 32) },
                        { intertype: 'value', ident: '0', type: 'i32' }
                      ]
                    },
                      { intertype: 'value', ident: '-1', type: 'i32' },
                      { intertype: 'value', ident: '0', type: 'i32' },
                    ]
                  };
                }
                for (var j = 0; j < targetElements.length; j++) {
                  var inBounds = j + whole >= 0 && j + whole < sourceElements.length;
                  var result;
                  if (inBounds || !signed) {
                    result = {
                      intertype: 'value',
                      ident: inBounds ? sourceElements[j + whole].ident : '0',
                      type: 'i' + Math.min(sourceBits, 32),
                    };
                    if (j == 0 && sourceBits < 32) {
                      // zext sign correction
                      var result2 = {
                        intertype: 'mathop',
                        op: isUnsignedOp(value.op) ? 'zext' : 'sext',
                        params: [result, {
                          intertype: 'type',
                          ident: 'i32',
                          type: 'i' + sourceBits
                        }],
                        type: 'i32'
                      };
                      result = result2;
                    }
                  } else {
                    // out of bounds and signed
                    result = copy(signedFill);
                  }
                  if (fraction != 0) {
                    var other;
                    var otherInBounds = j + sign + whole >= 0 && j + sign + whole < sourceElements.length;
                    if (otherInBounds || !signed) {
                      other = {
                        intertype: 'value',
                        ident: otherInBounds ? sourceElements[j + sign + whole].ident : '0',
                        type: 'i32',
                      };
                    } else {
                      other = copy(signedFill);
                    }
                    other = {
                      intertype: 'mathop',
                      op: shiftOp,
                      type: 'i32',
                      params: [
                        other,
                        { intertype: 'value', ident: (32 - fraction).toString(), type: 'i32' }
                      ]
                    };
                    result = {
                      intertype: 'mathop',
                      // shifting in 1s from the top is a special case
                      op: (signed && shifts >= 0 && j + sign + whole >= sourceElements.length) ? 'ashr' : shiftOpReverse,
                      type: 'i32',
                      params: [
                        result,
                        { intertype: 'value', ident: fraction.toString(), type: 'i32' }
                      ]
                    };
                    result = {
                      intertype: 'mathop',
                      op: 'or',
                      type: 'i32',
                      params: [
                        result,
                        other
                      ]
                    }
                  }
                  if (targetElements[j].bits < 32 && shifts < 0) {
                    // truncate bits that fall off the end. This is not needed in most cases, can probably be optimized out
                    result = {
                      intertype: 'mathop',
                      op: 'and',
                      type: 'i32',
                      params: [
                        result,
                        { intertype: 'value', ident: (Math.pow(2, targetElements[j].bits)-1).toString(), type: 'i32' }
                      ]
                    }
                  }
                  if (processor) {
                    result = processor(result, j);
                  }
                  result.assignTo = targetElements[j].ident;
                  toAdd.push(result);
                }
                if (targetBits <= 32) {
                  // We are generating a normal legal type here
                  legalValue = { intertype: 'value', ident: targetElements[0].ident, type: 'i32' };
                  if (targetBits < 32) {
                    legalValue = {
                      intertype: 'mathop',
                      op: 'and',
                      type: 'i32',
                      params: [
                        legalValue,
                        { intertype: 'value', ident: (Math.pow(2, targetBits)-1).toString(), type: 'i32' }
                      ]
                    }
                  };
                  legalValue.assignTo = item.assignTo;
                  toAdd.push(legalValue);
                } else if (finalizer) {
                  toAdd.push(finalizer());
                }
                i += removeAndAdd(label.lines, i, toAdd);
                continue;
              }
            }
            assert(0, 'Could not legalize illegal line: ' + [item.lineNum, dump(item)]);
          }
          if (dcheck('legalizer')) dprint('zz legalized: \n' + dump(label.lines));
        });
      });
    }

    // Add function lines to func.lines, after our modifications to the label lines
    data.functions.forEach(function(func) {
      func.labels.forEach(function(label) {
        func.lines = func.lines.concat(label.lines);
      });
    });
  }

  function addTypeInternal(type) {
    if (type.length == 1) return;
    if (Types.types[type]) return;
    if (['internal', 'hidden', 'inbounds', 'void'].indexOf(type) != -1) return;
    if (Compiletime.isNumberType(type)) return;
    dprint('types', 'Adding type: ' + type);

    // 'blocks': [14 x %struct.X] etc. If this is a pointer, we need
    // to look at the underlying type - it was not defined explicitly
    // anywhere else.
    var nonPointing = removeAllPointing(type);
    if (Types.types[nonPointing]) return;
    var check = /^\[(\d+)\ x\ (.*)\]$/.exec(nonPointing);
    if (check) {
      var num = parseInt(check[1]);
      num = Math.max(num, 1); // [0 x something] is used not for allocations and such of course, but
                              // for indexing - for an |array of unknown length|, basically. So we
                              // define the 'type' as having a single field. TODO: Ensure as a sanity
                              // check that we never allocate with this (either as a child structure
                              // in the analyzer, or in calcSize in alloca).
      var subType = check[2];
      addTypeInternal(subType); // needed for anonymous structure definitions (see below)

      var fields = [subType, subType]; // Two, so we get the flatFactor right. We care about the flatFactor, not the size here. see calculateStructAlignment
      Types.types[nonPointing] = {
        name_: nonPointing,
        fields: fields,
        lineNum: '?'
      };
      newTypes[nonPointing] = 1;
      // Also add a |[0 x type]| type
      var zerod = '[0 x ' + subType + ']';
      if (!Types.types[zerod]) {
        Types.types[zerod] = {
          name_: zerod,
          fields: fields,
          lineNum: '?'
        };
        newTypes[zerod] = 1;
      }
      return;
    }

    // anonymous structure definition, for example |{ i32, i8*, void ()*, i32 }|
    if (type[0] == '{' || type[0] == '<') {
      type = nonPointing;
      var packed = type[0] == '<';
      var internal = type;
      if (packed) {
        if (type[1] !== '{') {
          // vector type, <4 x float> etc.
          var size = getVectorSize(type);
          Types.types[type] = {
            name_: type,
            fields: zeros(size).map(function() {
              return getVectorNativeType(type);
            }),
            packed: false,
            flatSize: 4*size,
            lineNum: '?'
          };
          return;
        }
        if (internal[internal.length-1] != '>') {
          warnOnce('ignoring type ' + internal);
          return; // function pointer or such
        }
        internal = internal.substr(1, internal.length-2);
      }
      assert(internal[0] == '{', internal);
      if (internal[internal.length-1] != '}') {
        warnOnce('ignoring type ' + internal);
        return; // function pointer or such
      }
      internal = internal.substr(2, internal.length-4);
      Types.types[type] = {
        name_: type,
        fields: splitTokenList(tokenize(internal)).map(function(segment) {
          return segment[0].text;
        }),
        packed: packed,
        lineNum: '?'
      };
      newTypes[type] = 1;
      return;
    }

    if (isPointerType(type)) return;
    if (['['].indexOf(type) != -1) return;
    Types.types[type] = {
      name_: type,
      fields: [ 'i' + (QUANTUM_SIZE*8) ], // a single quantum size
      flatSize: 1,
      lineNum: '?'
    };
    newTypes[type] = 1;
  }

  function addType(type) {
    addTypeInternal(type);
    if (QUANTUM_SIZE === 1) {
      Types.flipTypes();
      addTypeInternal(type);
      Types.flipTypes();
    }
  }

  // Typevestigator
  function typevestigator() {
    if (sidePass) { // Do not investigate in the main pass - it is only valid to start to do so in the first side pass,
                    // which handles type definitions, and later. Doing so before the first side pass will result in
                    // making bad guesses about types which are actually defined
      for (var type in Types.needAnalysis) {
        if (type) addType(type);
      }
      Types.needAnalysis = {};
    }
  }

  // Type analyzer
  function analyzeTypes(fatTypes) {
    var types = Types.types;

    // 'fields' is the raw list of LLVM fields. However, we embed
    // child structures into parent structures, basically like C.
    // So { int, { int, int }, int } would be represented as
    // an Array of 4 ints. getelementptr on the parent would take
    // values 0, 1, 2, where 2 is the entire middle structure.
    // We also need to be careful with getelementptr to child
    // structures - we return a pointer to the same slab, just
    // a different offset. Likewise, need to be careful for
    // getelementptr of 2 (the last int) - it's real index is 4.
    // The benefit of this approach is inheritance -
    //    { { ancestor } , etc. } = descendant
    // In this case it is easy to bitcast ancestor to descendant
    // pointers - nothing needs to be done. If the ancestor were
    // a new slab, it would need some pointer to the outer one
    // for casting in that direction.
    // TODO: bitcasts of non-inheritance cases of embedding (not at start)
    var more = true;
    while (more) {
      more = false;
      for (var typeName in newTypes) {
        var type = types[typeName];
        if (type.flatIndexes) continue;
        var ready = true;
        type.fields.forEach(function(field) {
          if (isStructType(field)) {
            if (!types[field]) {
              addType(field);
              ready = false;
            } else {
              if (!types[field].flatIndexes) {
                newTypes[field] = 1;
                ready = false;
              }
            }
          }
        });
        if (!ready) {
          more = true;
          continue;
        }

        Runtime.calculateStructAlignment(type);

        if (dcheck('types')) dprint('type (fat=' + !!fatTypes + '): ' + type.name_ + ' : ' + JSON.stringify(type.fields));
        if (dcheck('types')) dprint('                        has final size of ' + type.flatSize + ', flatting: ' + type.needsFlattening + ' ? ' + (type.flatFactor ? type.flatFactor : JSON.stringify(type.flatIndexes)));
      }
    }

    if (QUANTUM_SIZE === 1 && !fatTypes) {
      Types.flipTypes();
      // Fake a quantum size of 4 for fat types. TODO: Might want non-4 for some reason?
      var trueQuantumSize = QUANTUM_SIZE;
      Runtime.QUANTUM_SIZE = 4;
      analyzeTypes(true);
      Runtime.QUANTUM_SIZE = trueQuantumSize;
      Types.flipTypes();
    }

    newTypes = null;
  }
  
  // Variable analyzer
  function variableAnalyzer() {
    // Globals

    var old = item.globalVariables;
    item.globalVariables = {};
    old.forEach(function(variable) {
      variable.impl = 'emulated'; // All global variables are emulated, for now. Consider optimizing later if useful
      item.globalVariables[variable.ident] = variable;
    });

    // Function locals

    item.functions.forEach(function(func) {
      func.variables = {};

      // LLVM is SSA, so we always have a single assignment/write. We care about
      // the reads/other uses.

      // Function parameters
      func.params.forEach(function(param) {
        if (param.intertype !== 'varargs') {
          if (func.variables[param.ident]) warn('cannot have duplicate variable names: ' + param.ident); // toNiceIdent collisions?
          func.variables[param.ident] = {
            ident: param.ident,
            type: param.type,
            origin: 'funcparam',
            lineNum: func.lineNum,
            rawLinesIndex: -1
          };
        }
      });

      // Normal variables
      func.lines.forEach(function(item, i) {
        if (item.assignTo) {
          if (func.variables[item.assignTo]) warn('cannot have duplicate variable names: ' + item.assignTo); // toNiceIdent collisions?
          var variable = func.variables[item.assignTo] = {
            ident: item.assignTo,
            type: item.type,
            origin: item.intertype,
            lineNum: item.lineNum,
            rawLinesIndex: i
          };
          if (variable.origin === 'alloca') {
            variable.allocatedNum = item.ident;
          }
          if (variable.origin === 'call') {
            variable.type = getReturnType(variable.type);
          }
        }
      });

      if (QUANTUM_SIZE === 1) {
        // Second pass over variables - notice when types are crossed by bitcast

        func.lines.forEach(function(item) {
          if (item.assignTo && item.intertype === 'bitcast') {
            // bitcasts are unique in that they convert one pointer to another. We
            // sometimes need to know the original type of a pointer, so we save that.
            //
            // originalType is the type this variable is created from
            // derivedTypes are the types that this variable is cast into
            func.variables[item.assignTo].originalType = item.type2;

            if (!isNumber(item.assignTo)) {
              if (!func.variables[item.assignTo].derivedTypes) {
                func.variables[item.assignTo].derivedTypes = [];
              }
              func.variables[item.assignTo].derivedTypes.push(item.type);
            }
          }
        });
      }

      // Analyze variable uses

      function analyzeVariableUses() {
        dprint('vars', 'Analyzing variables for ' + func.ident + '\n');

        for (vname in func.variables) {
          var variable = func.variables[vname];

          // Whether the value itself is used. For an int, always yes. For a pointer,
          // we might never use the pointer's value - we might always just store to it /
          // read from it. If so, then we can optimize away the pointer.
          variable.hasValueTaken = false;

          variable.pointingLevels = pointingLevels(variable.type);

          variable.uses = 0;
        }

        // TODO: improve the analysis precision. bitcast, for example, means we take the value, but perhaps we only use it to load/store
        var inNoop = 0;
        func.lines.forEach(function(line) {
          walkInterdata(line, function(item) {
            if (item.intertype == 'noop') inNoop++;
            if (!inNoop) {
              if (item.ident in func.variables) {
                func.variables[item.ident].uses++;

                if (item.intertype != 'load' && item.intertype != 'store') {
                  func.variables[item.ident].hasValueTaken = true;
                }
              }
            }
          }, function(item) {
            if (item.intertype == 'noop') inNoop--;
          });
        });

        //if (dcheck('vars')) dprint('analyzed variables: ' + dump(func.variables));
      }

      analyzeVariableUses();

      // Decision time

      for (vname in func.variables) {
        var variable = func.variables[vname];
        var pointedType = pointingLevels(variable.type) > 0 ? removePointing(variable.type) : null;
        if (variable.origin == 'getelementptr') {
          // Use our implementation that emulates pointers etc.
          // TODO Can we perhaps nativize some of these? However to do so, we need to discover their
          //      true types; we have '?' for them now, as they cannot be discovered in the intertyper.
          variable.impl = VAR_EMULATED;
        } else if (variable.origin == 'funcparam') {
          variable.impl = VAR_EMULATED;
        } else if (variable.type == 'i64*' && USE_TYPED_ARRAYS == 2) {
          variable.impl = VAR_EMULATED;
        } else if (MICRO_OPTS && variable.pointingLevels === 0) {
          // A simple int value, can be implemented as a native variable
          variable.impl = VAR_NATIVE;
        } else if (MICRO_OPTS && variable.origin === 'alloca' && !variable.hasValueTaken &&
                   variable.allocatedNum === 1 &&
                   (Compiletime.isNumberType(pointedType) || isPointerType(pointedType))) {
          // A pointer to a value which is only accessible through this pointer. Basically
          // a local value on the stack, which nothing fancy is done on. So we can
          // optimize away the pointing altogether, and just have a native variable
          variable.impl = VAR_NATIVIZED;
        } else {
          variable.impl = VAR_EMULATED;
        }
        if (dcheck('vars')) dprint('// var ' + vname + ': ' + JSON.stringify(variable));
      }
    });
  }

  // Sign analyzer
  //
  // Analyze our variables and detect their signs. In USE_TYPED_ARRAYS == 2,
  // we can read signed or unsigned values and prevent the need for signing
  // corrections. If on the other hand we are doing corrections anyhow, then
  // we can skip this pass.
  //
  // For each variable that is the result of a Load, we look a little forward
  // to see where it is used. We only care about mathops, since only they
  // need signs.
  //
  function signalyzer() {
    if (USE_TYPED_ARRAYS != 2 || CORRECT_SIGNS == 1) return;

    function seekIdent(item, obj) {
      if (item.ident === obj.ident) {
        obj.found++;
      }
    }

    function seekMathop(item, obj) {
      if (item.intertype === 'mathop' && obj.found && !obj.decided) {
        if (isUnsignedOp(item.op, item.variant)) {
          obj.unsigned++;
        } else {
          obj.signed++;
        }
      }
    }

    item.functions.forEach(function(func) {
      func.lines.forEach(function(line, i) {
        if (line.intertype === 'load') {
          // Floats have no concept of signedness. Mark them as 'signed', which is the default, for which we do nothing
          if (line.type in Compiletime.FLOAT_TYPES) {
            line.unsigned = false;
            return;
          }
          // Booleans are always unsigned
          var data = func.variables[line.assignTo];
          if (data.type === 'i1') {
            line.unsigned = true;
            return;
          }

          var total = data.uses;
          if (total === 0) return;
          var obj = { ident: line.assignTo, found: 0, unsigned: 0, signed: 0, total: total };
          // in loops with phis, we can also be used *before* we are defined
          var j = i-1, k = i+1;
          while(1) {
            assert(j >= 0 || k < func.lines.length, 'Signalyzer ran out of space to look for sign indications for line ' + line.lineNum);
            if (j >= 0 && walkInterdata(func.lines[j], seekIdent, seekMathop, obj)) break;
            if (k < func.lines.length && walkInterdata(func.lines[k], seekIdent, seekMathop, obj)) break;
            if (obj.total && obj.found >= obj.total) break; // see comment below
            j -= 1;
            k += 1;
          }

          // unsigned+signed might be < total, since the same ident can appear multiple times in the same mathop.
          // found can actually be > total, since we currently have the same ident in a GEP (see cubescript test)
          // in the GEP item, and a child item (we have the ident copied onto the GEP item as a convenience).
          // probably not a bug-causer, but FIXME. see also a reference to this above
          // we also leave the loop above potentially early due to this. otherwise, though, we end up scanning the
          // entire function in some cases which is very slow
          assert(obj.found >= obj.total, 'Could not Signalyze line ' + line.lineNum);
          line.unsigned = obj.unsigned > 0;
          dprint('vars', 'Signalyzer: ' + line.assignTo + ' has unsigned == ' + line.unsigned + ' (line ' + line.lineNum + ')');
        }
      });
    });
  }

  // Quantum fixer
  //
  // See settings.js for the meaning of QUANTUM_SIZE. The issue we fix here is,
  // to correct the .ll assembly code so that things work with QUANTUM_SIZE=1.
  //
  function quantumFixer() {
    if (QUANTUM_SIZE !== 1) return;

    // ptrs: the indexes of parameters that are pointers, whose originalType is what we want
    // bytes: the index of the 'bytes' parameter
    // TODO: malloc, realloc?
    var FIXABLE_CALLS = {
      'memcpy': { ptrs: [0,1], bytes: 2 },
      'memmove': { ptrs: [0,1], bytes: 2 },
      'memset': { ptrs: [0], bytes: 2 },
      'qsort': { ptrs: [0], bytes: 2 }
    };

    function getSize(types, type, fat) {
      if (types[type]) return types[type].flatSize;
      if (fat) {
        Runtime.QUANTUM_SIZE = 4;
      }
      var ret = Runtime.getNativeTypeSize(type);
      if (fat) {
        Runtime.QUANTUM_SIZE = 1;
      }
      return ret;
    }

    function getFlatIndexes(types, type) {
      if (types[type]) return types[type].flatIndexes;
      return [0];
    }

    item.functions.forEach(function(func) {
      function getOriginalType(param) {
        function get() {
          if (param.intertype === 'value' && !isNumber(param.ident)) {
            if (func.variables[param.ident]) {
              return func.variables[param.ident].originalType || null;
            } else {
              return item.globalVariables[param.ident].originalType;
            }
          } else if (param.intertype === 'bitcast') {
            return param.params[0].type;
          } else if (param.intertype === 'getelementptr') {
            if (param.params[0].type[0] === '[') return param.params[0].type;
          }
          return null;
        }
        var ret = get();
        if (ret && ret[0] === '[') {
          var check = /^\[(\d+)\ x\ (.*)\]\*$/.exec(ret);
          assert(check);
          ret = check[2] + '*';
        }
        return ret;
      }

      func.lines.forEach(function(line) {
        // Call
        if (line.intertype === 'call') {
          var funcIdent = LibraryManager.getRootIdent(line.ident.substr(1));
          var fixData = FIXABLE_CALLS[funcIdent];
          if (!fixData) return;
          var ptrs = fixData.ptrs.map(function(ptr) { return line.params[ptr] });
          var bytes = line.params[fixData.bytes].ident;

          // Only consider original types. This assumes memcpy always has pointers bitcast to i8*
          var originalTypes = ptrs.map(getOriginalType);
          for (var i = 0; i < originalTypes.length; i++) {
            if (!originalTypes[i]) return;
          }
          originalTypes = originalTypes.map(function(type) { return removePointing(type) });
          var sizes = originalTypes.map(function(type) { return getSize(Types.types, type) });
          var fatSizes = originalTypes.map(function(type) { return getSize(Types.fatTypes, type, true) });
          // The sizes may not be identical, if we copy a descendant class into a parent class. We use
          // the smaller size in that case. However, this may also be a bug, it is hard to tell, hence a warning
          warn(dedup(sizes).length === 1, 'All sizes should probably be identical here: ' + dump(originalTypes) + ':' + dump(sizes) + ':' +
               line.lineNum);
          warn(dedup(fatSizes).length === 1, 'All fat sizes should probably be identical here: ' + dump(originalTypes) + ':' + dump(sizes) + ':' +
               line.lineNum);
          var size = Math.min.apply(null, sizes);
          var fatSize = Math.min.apply(null, fatSizes);
          if (isNumber(bytes)) {
            // Figure out how much to copy.
            var fixedBytes;
            if (bytes % fatSize === 0) {
              fixedBytes = size*(bytes/fatSize);
            } else if (fatSize % bytes === 0 && size % (fatSize/bytes) === 0) {
              // Assume this is a simple array. XXX We can be wrong though! See next TODO
              fixedBytes = size/(fatSize/bytes);
            } else {
              // Just part of a structure. Align them to see how many fields. Err on copying more.
              // TODO: properly generate a complete structure, including nesteds, and calculate on that
              var flatIndexes = getFlatIndexes(Types.types, originalTypes[0]).concat(size);
              var fatFlatIndexes = getFlatIndexes(Types.fatTypes, originalTypes[0]).concat(fatSize);
              var index = 0;
              var left = bytes;
              fixedBytes = 0;
              while (left > 0) {
                left -= fatFlatIndexes[index+1] - fatFlatIndexes[index]; // note: we copy the alignment bytes too, which is unneeded
                fixedBytes += flatIndexes[index+1] - flatIndexes[index];
              }
            }
            line.params[fixData.bytes].ident = fixedBytes;
          } else {
            line.params[fixData.bytes].intertype = 'jsvalue';
            // We have an assertion in library::memcpy() that this is round
            line.params[fixData.bytes].ident = size + '*(' + bytes + '/' + fatSize + ')';
          }
        }
      });
    });

    // 2nd part - fix hardcoded constant offsets in global constants
    values(item.globalVariables).forEach(function(variable) {
      function recurse(item) {
        if (item.contents) {
          item.contents.forEach(recurse);
        } else if (item.intertype === 'getelementptr' && item.params[0].intertype === 'bitcast' && item.params[0].type === 'i8*') {
          var originalType = removePointing(item.params[0].params[0].type);
          var fatSize = getSize(Types.fatTypes, originalType, true);
          var slimSize = getSize(Types.types, originalType, false);
          assert(fatSize % slimSize === 0);
          item.params.slice(1).forEach(function(param) {
            if (param.intertype === 'value' && isNumber(param.ident)) {
              var corrected = parseInt(param.ident)/(fatSize/slimSize);
              assert(corrected % 1 === 0);
              param.ident = corrected.toString();
            }
          });
        } else if (item.params) {
          item.params.forEach(recurse);
        }
      }
      if (!variable.external && variable.value) recurse(variable.value);
    });
  }

  function operateOnLabels(line, func) {
    function process(item, id) {
      ['label', 'labelTrue', 'labelFalse', 'toLabel', 'unwindLabel', 'defaultLabel'].forEach(function(id) {
        if (item[id]) {
          func(item, id);
        }
      });
    }
    if (line.intertype in BRANCH_INVOKE) {
      process(line);
    } else if (line.intertype == 'switch') {
      process(line);
      line.switchLabels.forEach(process);
    }
  }

  // Label analyzer
  function labelAnalyzer() {
    item.functions.forEach(function(func) {
      func.labelsDict = {};
      func.labelIds = {};
      func.labelIdsInverse = {};
      func.labelIdCounter = 1;
      func.labels.forEach(function(label) {
        if (!(label.ident in func.labelIds)) {
          func.labelIds[label.ident] = func.labelIdCounter++;
          func.labelIdsInverse[func.labelIdCounter-1] = label.ident;
        }
      });
      var entryIdent = func.labels[0].ident;

      // Minify label ids to numeric ids.
      func.labels.forEach(function(label) {
        label.ident = func.labelIds[label.ident];
        label.lines.forEach(function(line) {
          operateOnLabels(line, function(item, id) {
            item[id] = func.labelIds[item[id]].toString(); // strings, because we will append as we process
          });
        });
      });

      func.labels.forEach(function(label) {
        func.labelsDict[label.ident] = label;
      });

      // Correct phis
      func.labels.forEach(function(label) {
        label.lines.forEach(function(phi) {
          if (phi.intertype == 'phi') {
            for (var i = 0; i < phi.params.length; i++) {
              phi.params[i].label = func.labelIds[phi.params[i].label];
              if (VERBOSE && !phi.params[i].label) warn('phi refers to nonexistent label on line ' + phi.lineNum);
            }
          }
        });
      });

      func.lines.forEach(function(line) {
        if (line.intertype == 'indirectbr') {
          func.forceEmulated = true;
        }
      });

      function getActualLabelId(labelId) {
        if (func.labelsDict[labelId]) return labelId;
        // If not present, it must be a surprisingly-named entry (or undefined behavior, in which case, still ok to use the entry)
        labelId = func.labelIds[entryIdent];
        assert(func.labelsDict[labelId]);
        return labelId;
      }

      // Basic longjmp support, see library.js setjmp/longjmp
      var setjmp = toNiceIdent('@setjmp');
      func.setjmpTable = null;
      for (var i = 0; i < func.labels.length; i++) {
        var label = func.labels[i];
        for (var j = 0; j < label.lines.length; j++) {
          var line = label.lines[j];
          if ((line.intertype == 'call' || line.intertype == 'invoke') && line.ident == setjmp) {
            if (line.intertype == 'invoke') {
              // setjmp cannot trigger unwinding, so just reduce the invoke to a call + branch
              line.intertype = 'call';
              label.lines.push({
                intertype: 'branch',
                label: line.toLabel,
                lineNum: line.lineNum + 0.01, // XXX legalizing might confuse this
              });
              line.toLabel = line.unwindLabel = -2;
            }
            // split this label into up to the setjmp (including), then a new label for the rest. longjmp will reach the rest
            var oldLabel = label.ident;
            var newLabel = func.labelIdCounter++;
            if (!func.setjmpTable) func.setjmpTable = [];
            func.setjmpTable.push({ oldLabel: oldLabel, newLabel: newLabel, assignTo: line.assignTo });
            func.labels.splice(i+1, 0, {
              intertype: 'label',
              ident: newLabel,
              lineNum: label.lineNum + 0.5,
              lines: label.lines.slice(j+1)
            });
            func.labelsDict[newLabel] = func.labels[i+1];
            label.lines = label.lines.slice(0, j+1);
            label.lines.push({
              intertype: 'branch',
              label: toNiceIdent(newLabel),
              lineNum: line.lineNum + 0.01, // XXX legalizing might confuse this
            });
            // Correct phis
            func.labels.forEach(function(label) {
              label.lines.forEach(function(phi) {
                if (phi.intertype == 'phi') {
                  for (var i = 0; i < phi.params.length; i++) {
                    var sourceLabelId = getActualLabelId(phi.params[i].label);
                    if (sourceLabelId == oldLabel) {
                      phi.params[i].label = newLabel;
                    }
                  }
                }
              });
            });
          }
        }
      }
      if (func.setjmpTable) {
        func.forceEmulated = true;
        recomputeLines(func);
      }

      // Properly implement phis, by pushing them back into the branch
      // that leads to here. We will only have the |var| definition in this location.

      // First, push phis back
      func.labels.forEach(function(label) {
        label.lines.forEach(function(phi) {
          if (phi.intertype == 'phi') {
            for (var i = 0; i < phi.params.length; i++) {
              var param = phi.params[i];
              if (VERBOSE && !param.label) warn('phi refers to nonexistent label on line ' + phi.lineNum);
              var sourceLabelId = getActualLabelId(param.label);
              if (sourceLabelId) {
                var sourceLabel = func.labelsDict[sourceLabelId];
                var lastLine = sourceLabel.lines.slice(-1)[0];
                assert(lastLine.intertype in LLVM.PHI_REACHERS, 'Only some can lead to labels with phis:' + [func.ident, label.ident, lastLine.intertype]);
                if (!lastLine.phi) {
                  lastLine.phi = true;
                  assert(!lastLine.dependent);
                  lastLine.dependent = {
                    intertype: 'phiassigns',
                    params: []
                  };
                };
                lastLine.dependent.params.push({
                  intertype: 'phiassign',
                  ident: phi.assignTo,
                  value: param.value,
                  targetLabel: label.ident
                });
              }
            }
            // The assign to phi is now just a var
            phi.intertype = 'var';
            phi.ident = phi.assignTo;
            phi.assignTo = null;
          }
        });
      });

      if (func.ident in NECESSARY_BLOCKADDRS) {
        Functions.blockAddresses[func.ident] = {};
        for (var needed in NECESSARY_BLOCKADDRS[func.ident]) {
          assert(needed in func.labelIds);
          Functions.blockAddresses[func.ident][needed] = func.labelIds[needed];
        }
      }
    });
  }

  // Stack analyzer - calculate the base stack usage
  function stackAnalyzer() {
    data.functions.forEach(function(func) {
      var lines = func.labels[0].lines;
      var hasAlloca = false;
      for (var i = 0; i < lines.length; i++) {
        var item = lines[i];
        if (!item.assignTo || item.intertype != 'alloca' || !isNumber(item.ident)) break;
        item.allocatedSize = func.variables[item.assignTo].impl === VAR_EMULATED ?
          calcAllocatedSize(item.allocatedType)*item.ident: 0;
        hasAlloca = true;
        if (USE_TYPED_ARRAYS === 2) {
          // We need to keep the stack aligned
          item.allocatedSize = RuntimeGenerator.forceAlign(item.allocatedSize, Runtime.STACK_ALIGN);
        }
      }
      var index = 0;
      for (var i = 0; i < lines.length; i++) {
        var item = lines[i];
        if (!item.assignTo || item.intertype != 'alloca' || !isNumber(item.ident)) break;
        item.allocatedIndex = index;
        index += item.allocatedSize;
        delete item.allocatedSize;
      }
      func.initialStack = index;
      func.otherStackAllocations = false;
      if (func.initialStack === 0 && hasAlloca) func.otherStackAllocations = true; // a single alloca of zero still requires us to emit stack support code
      while (func.initialStack == 0) { // one-time loop with possible abort in the middle
        // If there is no obvious need for stack management, perhaps we don't need it
        // (we try to optimize that way with SKIP_STACK_IN_SMALL). However,
        // we need to note if stack allocations other than initial allocs can happen here
        // If so, we need to rewind the stack when we leave.

        // By-value params are causes of additional allocas (although we could in theory make them normal allocas too)
        func.params.forEach(function(param) {
          if (param.byVal) {
            func.otherStackAllocations = true;
          }
        });
        if (func.otherStackAllocations) break;

        // Allocas
        var finishedInitial = false;

        lines = func.lines; // We need to consider all the function lines now, not just the first label

        for (var i = 0; i < lines.length; i++) {
          var item = lines[i];
          if (!finishedInitial && (!item.assignTo || item.intertype != 'alloca' || !isNumber(item.ident))) {
            finishedInitial = true;
          }
          if (item.intertype == 'alloca' && finishedInitial) {
            func.otherStackAllocations = true;
            break;
          }
        }
        if (func.otherStackAllocations) break;

        // Varargs
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i];
          if (item.intertype == 'call' && isVarArgsFunctionType(item.type)) {
            func.otherStackAllocations = true;
            break;
          }
        }
        if (func.otherStackAllocations) break;

        break;
      }
    });
  }

  // ReLooper - reconstruct nice loops, as much as possible
  // This is now done in the jsify stage, using compiled relooper2
  function relooper() {
    function makeBlock(labels, entries, labelsDict, forceEmulated) {
      if (labels.length == 0) return null;
      dprint('relooping', 'prelooping: ' + entries + ',' + labels.length + ' labels');
      assert(entries && entries[0]); // need at least 1 entry

      var emulated = {
        type: 'emulated',
        id: 'B',
        labels: labels,
        entries: entries.slice(0)
      };
      return emulated;
    }
    item.functions.forEach(function(func) {
      dprint('relooping', "// relooping function: " + func.ident);
      func.block = makeBlock(func.labels, [func.labels[0].ident], func.labelsDict, func.forceEmulated);
    });
  }

  // main
  castAway();
  legalizer();
  typevestigator();
  analyzeTypes();
  variableAnalyzer();
  signalyzer();
  quantumFixer();
  labelAnalyzer();
  stackAnalyzer();
  relooper();

  //B.stop('analyzer');
  return item;
}

