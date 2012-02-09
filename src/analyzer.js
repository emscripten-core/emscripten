//"use strict";

// Analyze intertype data. Calculates things that are necessary in order
// to do the final conversion into JavaScript later, for example,
// properties of variables, loop structures of functions, etc.

var VAR_NATIVE = 'native';
var VAR_NATIVIZED = 'nativized';
var VAR_EMULATED = 'emulated';

var ENTRY_IDENT = toNiceIdent('%0');
var ENTRY_IDENTS = set(toNiceIdent('%0'), toNiceIdent('%1'));

function cleanFunc(func) {
  func.lines = func.lines.filter(function(line) { return line.intertype !== null });
  func.labels.forEach(function(label) {
    label.lines = label.lines.filter(function(line) { return line.intertype !== null });
  });
}

// Handy sets

var BRANCH_INVOKE = set('branch', 'invoke');
var SIDE_EFFECT_CAUSERS = set('call', 'invoke', 'atomic');
var UNFOLDABLE = set('value', 'type', 'phiparam');

// Analyzer

function analyzer(data, sidePass) {
  var mainPass = !sidePass;

  // Substrate
  var substrate = new Substrate('Analyzer');

  // Sorter
  substrate.addActor('Sorter', {
    processItem: function(item) {
      item.items.sort(function (a, b) { return a.lineNum - b.lineNum });
      this.forwardItem(item, 'Gatherer');
    }
  });

  // Gatherer
  substrate.addActor('Gatherer', {
    processItem: function(item) {
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
        if (QUANTUM_SIZE === 1) {
          Types.fatTypes[type.name_] = copy(type);
        }
      });

      // Functions & labels
      item.functions = [];
      var currLabelFinished; // Sometimes LLVM puts a branch in the middle of a label. We need to ignore all lines after that.
      item.items.sort(function(a, b) { return a.lineNum - b.lineNum });
      for (var i = 0; i < item.items.length; i++) {
        var subItem = item.items[i];
        assert(subItem.lineNum);
        if (subItem.intertype == 'function') {
          item.functions.push(subItem);
          subItem.endLineNum = null;
          subItem.lines = []; // We will fill in the function lines after the legalizer, since it can modify them
          subItem.labels = [];

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
            if (subItem.intertype === 'branch') {
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
      this.forwardItem(item, 'Legalizer');
    }
  });

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
  //
  // TODO: Expand this also into legalization of i64 into i32,i32, which can then
  //       replace our i64 mode 1 implementation. Legalizing i64s is harder though
  //       as they can appear in function arguments and we would also need to implement
  //       an unfolder (to uninline inline LLVM function calls, so that each LLVM line
  //       has a single LLVM instruction).
  substrate.addActor('Legalizer', {
    processItem: function(data) {
      // Legalization
      if (USE_TYPED_ARRAYS == 2) {
        function isIllegalType(type) {
          var bits = getBits(type);
          return bits > 0 && (bits > 64 || !isPowerOfTwo(bits));
        }
        function getLegalVars(base, bits) {
          if (isNumber(base)) {
            return getLegalLiterals(base, bits);
          }
          var ret = new Array(Math.ceil(bits/32));
          var i = 0;
          while (bits > 0) {
            ret[i] = { ident: base + '$' + i, bits: Math.min(32, bits) };
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
            ret[i] = { ident: parsed[i].toString(), bits: Math.min(32, bits) };
            bits -= 32;
            i++;
          }
          return ret;
        }
        // Uses the right factor to multiply line numbers by so that they fit in between
        // the line[i] and the line after it
        function interpLines(lines, i, toAdd) {
          var prev = i >= 0 ? lines[i].lineNum : -1;
          var next = (i < lines.length-1) ? lines[i+1].lineNum : (lines[i].lineNum + 0.5);
          var factor = (next - prev)/(4*toAdd.length+3);
          for (var k = 0; k < toAdd.length; k++) {
            toAdd[k].lineNum = prev + ((k+1)*factor);
          }
        }
        function removeAndAdd(lines, i, toAdd) {
          var item = lines[i];
          interpLines(lines, i, toAdd);
          Array.prototype.splice.apply(lines, [i, 1].concat(toAdd));
          return toAdd.length;
        }
        data.functions.forEach(function(func) {
          var tempId = 0;
          func.labels.forEach(function(label) {
            var i = 0, bits;
            while (i < label.lines.length) {
              var item = label.lines[i];
              // Check if we need to legalize here
              var isIllegal = false;
              walkInterdata(item, function(item) {
                if (isIllegalType(item.valueType) || isIllegalType(item.type)) {
                  isIllegal = true;
                }
              });
              if (!isIllegal) {
                i++;
                continue;
              }
              // Unfold this line. If we unfolded, we need to return and process the lines we just
              // generated - they may need legalization too
              var unfolded = [];
              walkAndModifyInterdata(item, function(subItem) {
                if (subItem != item && !(subItem.intertype in UNFOLDABLE)) {
                  var tempIdent = '$$emscripten$temp$' + (tempId++);
                  subItem.assignTo = tempIdent;
                  unfolded.unshift(subItem);
                  return { intertype: 'value', ident: tempIdent, type: subItem.type };
                }
              });
              if (unfolded.length > 0) {
                interpLines(label.lines, i-1, unfolded);
                Array.prototype.splice.apply(label.lines, [i, 0].concat(unfolded));
                continue; // remain at this index, to unfold newly generated lines
              }
              // This is an illegal-containing line, and it is unfolded. Legalize it now
              if (item.intertype == 'store') {
                dprint('legalizer', 'Legalizing store at line ' + item.lineNum);
                var toAdd = [];
                bits = getBits(item.valueType);
                var elements;
                elements = getLegalVars(item.value.ident, bits);
                var j = 0;
                elements.forEach(function(element) {
                  var tempVar = '$st$' + i + '$' + j;
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
              } else if (item.assignTo) {
                var value = item;
                switch (value.intertype) {
                  case 'load': {
                    dprint('legalizer', 'Legalizing load at line ' + item.lineNum);
                    bits = getBits(value.valueType);
                    var elements = getLegalVars(item.assignTo, bits);
                    var j = 0;
                    var toAdd = [];
                    elements.forEach(function(element) {
                      var tempVar = '$st$' + i + '$' + j;
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
                      var actualSizeType = 'i' + element.bits; // The last one may be smaller than 32 bits
                      toAdd.push({
                        intertype: 'load',
                        assignTo: element.ident,
                        pointerType: actualSizeType + '*',
                        valueType: actualSizeType,
                        type: actualSizeType, // XXX why is this missing from intertyper?
                        pointer: { intertype: 'value', ident: tempVar, type: actualSizeType + '*' },
                        ident: tempVar,
                        pointerType: actualSizeType + '*',
                        align: value.align
                      });
                      j++;
                    });
                    Types.needAnalysis['[0 x i32]'] = 0;
                    i += removeAndAdd(label.lines, i, toAdd);
                    continue;
                  }
                  case 'phi': {
                    dprint('legalizer', 'Legalizing phi at line ' + item.lineNum);
                    bits = getBits(value.type);
                    var toAdd = [];
                    var elements = getLegalVars(item.assignTo, bits);
                    var j = 0;
                    elements.forEach(function(element) {
                      toAdd.push({
                        intertype: 'phi',
                        assignTo: element.ident,
                        type: 'i' + element.bits,
                        params: value.params.map(function(param) {
                          return {
                            intertype: 'phiparam',
                            label: param.label,
                            value: {
                             intertype: 'value',
                             ident: param.value.ident + '$' + j,
                             type: 'i' + element.bits,
                            }
                          };
                        })
                      });
                      j++;
                    });
                    i += removeAndAdd(label.lines, i, toAdd);
                    continue;
                  }
                  case 'bitcast': {
                    value = {
                      op: 'bitcast',
                      param1: item.params[0]
                    };
                    // fall through
                  }
                  case 'mathop': {
                    dprint('legalizer', 'Legalizing mathop at line ' + item.lineNum);
                    var toAdd = [];
                    var sourceBits = getBits(value.param1.type);
                    var sourceElements;
                    if (sourceBits <= 64) {
                      // The input is a legal type
                      if (sourceBits <= 32) {
                        sourceElements = [{ ident: value.param1.ident, bits: sourceBits }];
                      } else if (sourceBits == 64 && I64_MODE == 1) {
                        sourceElements = [{ ident: value.param1.ident + '[0]', bits: 32 },
                                          { ident: value.param1.ident + '[1]', bits: 32 }];
                        // Add the source element as a param so that it is not eliminated as unneeded (the idents are not a simple ident here)
                        toAdd.push({
                          intertype: 'value', ident: ';', type: 'rawJS',
                          params: [{ intertype: 'value', ident: value.param1.ident, type: 'i32' }]
                        });
                      } else {
                        throw 'Invalid legal type as source of legalization ' + sourceBits;
                      }
                    } else {
                      sourceElements = getLegalVars(value.param1.ident, sourceBits);
                    }
                    // All mathops can be parametrized by how many shifts we do, and how big the source is
                    var shifts = 0;
                    var targetBits;
                    var processor = null;
                    switch (value.op) {
                      case 'lshr': {
                        shifts = parseInt(value.param2.ident);
                        targetBits = sourceBits;
                        break;
                      }
                      case 'shl': {
                        shifts = -parseInt(value.param2.ident);
                        targetBits = sourceBits;
                        break;
                      }
                      case 'trunc': case 'zext': {
                        targetBits = getBits(value.param2.ident);
                        break;
                      }
                      case 'bitcast': {
                        targetBits = sourceBits;
                        break;
                      }
                      case 'or': case 'and': case 'xor': {
                        targetBits = sourceBits;
                        var otherElements = getLegalVars(value.param2.ident, sourceBits);
                        processor = function(result, j) {
                          return {
                            intertype: 'mathop',
                            op: value.op,
                            type: 'i' + otherElements[j].bits,
                            param1: result,
                            param2: { intertype: 'value', ident: otherElements[j].ident, type: 'i' + otherElements[j].bits }
                          };                            
                        };
                        break;
                      }
                      default: throw 'Invalid mathop for legalization: ' + [value.op, item.lineNum, dump(item)];
                    }
                    // Do the legalization
                    assert(isNumber(shifts), 'TODO: handle nonconstant shifts');
                    var targetElements = getLegalVars(item.assignTo, targetBits);
                    var sign = shifts >= 0 ? 1 : -1;
                    var shiftOp = shifts >= 0 ? 'shl' : 'lshr';
                    var shiftOpReverse = shifts >= 0 ? 'lshr' : 'shl';
                    var whole = shifts >= 0 ? Math.floor(shifts/32) : Math.ceil(shifts/32);
                    var fraction = Math.abs(shifts % 32);
                    for (var j = 0; j < targetElements.length; j++) {
                      var result = {
                        intertype: 'value',
                        ident: (j + whole >= 0 && j + whole < sourceElements.length) ? sourceElements[j + whole].ident : '0',
                        type: 'i32',
                      };
                      if (fraction != 0) {
                        var other = {
                          intertype: 'value',
                          ident: (j + sign + whole >= 0 && j + sign + whole < sourceElements.length) ? sourceElements[j + sign + whole].ident : '0',
                          type: 'i32',
                        };
                        other = {
                          intertype: 'mathop',
                          op: shiftOp,
                          type: 'i32',
                          param1: other,
                          param2: { intertype: 'value', ident: (32 - fraction).toString(), type: 'i32' }
                        };
                        result = {
                          intertype: 'mathop',
                          op: shiftOpReverse,
                          type: 'i32',
                          param1: result,
                          param2: { intertype: 'value', ident: fraction.toString(), type: 'i32' }
                        };
                        result = {
                          intertype: 'mathop',
                          op: 'or',
                          type: 'i32',
                          param1: result,
                          param2: other
                        }
                      }
                      if (targetElements[j].bits < 32 && shifts < 0) {
                        // truncate bits that fall off the end. This is not needed in most cases, can probably be optimized out
                        result = {
                          intertype: 'mathop',
                          op: 'and',
                          type: 'i32',
                          param1: result,
                          param2: { intertype: 'value', ident: (Math.pow(2, targetElements[j].bits)-1).toString(), type: 'i32' }
                        }
                      }
                      if (processor) {
                        result = processor(result, j);
                      }
                      result.assignTo = targetElements[j].ident;
                      toAdd.push(result);
                    }
                    if (targetBits <= 64) {
                      // We are generating a normal legal type here
                      var legalValue;
                      if (targetBits == 64 && I64_MODE == 1) {
                        // Generate an i64-1 [low,high]. This will be unnecessary when we legalize i64s
                        legalValue = {
                          intertype: 'value',
                          ident: '[' + targetElements[0].ident + ',' + targetElements[1].ident + ']',
                          type: 'rawJS',
                          // Add the target elements as params so that they are not eliminated as unneeded (the ident is not a simple ident here)
                          params: targetElements.map(function(element) {
                            return { intertype: 'value', ident: element.ident, type: 'i32' };
                          })
                        };
                      } else if (targetBits <= 32) {
                        legalValue = { intertype: 'value', ident: targetElements[0].ident, type: 'rawJS' };
                        // truncation to smaller than 32 bits has already been done, if necessary
                      } else {
                        throw 'Invalid legal type as target of legalization ' + targetBits;
                      }
                      legalValue.assignTo = item.assignTo;
                      toAdd.push(legalValue);
                    }
                    i += removeAndAdd(label.lines, i, toAdd);
                    continue;
                  }
                }
              }
              assert(0, 'Could not legalize illegal line: ' + [item.lineNum, dump(item)]);
            }
          });
        });
      }

      // Add function lines to func.lines, after our modifications to the label lines
      data.functions.forEach(function(func) {
        func.labels.forEach(function(label) {
          func.lines = func.lines.concat(label.lines);
        });
      });
      this.forwardItem(data, 'Typevestigator');
    }
  });

  function addTypeInternal(type, data) {
    if (type.length == 1) return;
    if (Types.types[type]) return;
    if (['internal', 'hidden', 'inbounds', 'void'].indexOf(type) != -1) return;
    if (Runtime.isNumberType(type)) return;
    dprint('types', 'Adding type: ' + type);

    // 'blocks': [14 x %struct.X] etc. If this is a pointer, we need
    // to look at the underlying type - it was not defined explicitly
    // anywhere else.
    var nonPointing = removeAllPointing(type);
    var check = /^\[(\d+)\ x\ (.*)\]$/.exec(nonPointing);
    if (check && !Types.types[nonPointing]) {
      var num = parseInt(check[1]);
      num = Math.max(num, 1); // [0 x something] is used not for allocations and such of course, but
                              // for indexing - for an |array of unknown length|, basically. So we
                              // define the 'type' as having a single field. TODO: Ensure as a sanity
                              // check that we never allocate with this (either as a child structure
                              // in the analyzer, or in calcSize in alloca).
      var subType = check[2];
      addTypeInternal(subType, data); // needed for anonymous structure definitions (see below)

      Types.types[nonPointing] = {
        name_: nonPointing,
        fields: range(num).map(function() { return subType }),
        lineNum: '?'
      };
      // Also add a |[0 x type]| type
      var zerod = '[0 x ' + subType + ']';
      if (!Types.types[zerod]) {
        Types.types[zerod] = {
          name_: zerod,
          fields: [subType, subType], // Two, so we get the flatFactor right. We care about the flatFactor, not the size here
          lineNum: '?'
        };
      }
      return;
    }

    // anonymous structure definition, for example |{ i32, i8*, void ()*, i32 }|
    if (type[0] == '{' || type[0] == '<') {
      type = nonPointing;
      var packed = type[0] == '<';
      Types.types[type] = {
        name_: type,
        fields: splitTokenList(tokenize(type.substr(2 + packed, type.length - 4 - 2*packed)).tokens).map(function(segment) {
          return segment[0].text;
        }),
        packed: packed,
        lineNum: '?'
      };
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
  }

  function addType(type, data) {
    addTypeInternal(type, data);
    if (QUANTUM_SIZE === 1) {
      Types.flipTypes();
      addTypeInternal(type, data);
      Types.flipTypes();
    }
  }

  // Typevestigator
  substrate.addActor('Typevestigator', {
    processItem: function(data) {
      if (sidePass) { // Do not investigate in the main pass - it is only valid to start to do so in the first side pass,
                      // which handles type definitions, and later. Doing so before the first side pass will result in
                      // making bad guesses about types which are actually defined
        for (var type in Types.needAnalysis) {
          if (type) addType(type, data);
        }
        Types.needAnalysis = {};
      }
      this.forwardItem(data, 'Typeanalyzer');
    }
  });

  // Type analyzer
  substrate.addActor('Typeanalyzer', {
    processItem: function analyzeTypes(item, fatTypes) {
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
        for (var typeName in types) {
          var type = types[typeName];
          if (type.flatIndexes) continue;
          var ready = true;
          type.fields.forEach(function(field) {
            if (isStructType(field)) {
              if (!types[field]) {
                addType(field, item);
                ready = false;
              } else {
                if (!types[field].flatIndexes) {
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
        analyzeTypes(item, true);
        Runtime.QUANTUM_SIZE = trueQuantumSize;
        Types.flipTypes();
      }

      if (!fatTypes) {
        this.forwardItem(item, 'VariableAnalyzer');
      }
    }
  });
  
  // Variable analyzer
  substrate.addActor('VariableAnalyzer', {
    processItem: function(item) {
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
            var variable = func.variables[item.assignTo] = {
              ident: item.assignTo,
              type: item.type,
              origin: item.intertype,
              lineNum: item.lineNum,
              rawLinesIndex: i
            };
            if (variable.origin === 'alloca') {
              variable.allocatedNum = item.allocatedNum;
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

        // Filter out no longer used variables, collapsing more as we go
        while (true) {
          analyzeVariableUses();

          var recalc = false;

          keys(func.variables).forEach(function(vname) {
            var variable = func.variables[vname];
            if (variable.uses == 0 && variable.origin != 'funcparam') {
              // Eliminate this variable if we can
              var sideEffects = false;
              walkInterdata(func.lines[variable.rawLinesIndex], function(item) {
                if (item.intertype in SIDE_EFFECT_CAUSERS) sideEffects = true;
              });
              if (!sideEffects) {
                dprint('vars', 'Eliminating ' + vname);
                func.lines[variable.rawLinesIndex].intertype = 'noop';
                func.lines[variable.rawLinesIndex].assignTo = null;
                // in theory we can also null out some fields here to save memory
                delete func.variables[vname];
                recalc = true;
              }
            }
          });

          if (!recalc) break;
        }

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
          } else if (variable.type == 'i64*' && I64_MODE == 1) {
            variable.impl = VAR_EMULATED;
          } else if (MICRO_OPTS && variable.pointingLevels === 0) {
            // A simple int value, can be implemented as a native variable
            variable.impl = VAR_NATIVE;
          } else if (MICRO_OPTS && variable.origin === 'alloca' && !variable.hasValueTaken &&
                     variable.allocatedNum === 1 &&
                     (Runtime.isNumberType(pointedType) || Runtime.isPointerType(pointedType))) {
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
      this.forwardItem(item, 'Signalyzer');
    }
  });

  // Sign analyzer
  //
  // Analyze our variables and detect their signs. In USE_TYPED_ARRAYS == 2,
  // we can read signed or unsigned values and prevent the need for signing
  // corrections.
  //
  // For each variable that is the result of a Load, we look a little forward
  // to see where it is used. We only care about mathops, since only they
  // need signs.
  //
  substrate.addActor('Signalyzer', {
    processItem: function(item) {
      this.forwardItem(item, 'QuantumFixer');
      if (USE_TYPED_ARRAYS !== 2) return;

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
            if (line.type in Runtime.FLOAT_TYPES) {
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
  });

  // Quantum fixer
  //
  // See settings.js for the meaning of QUANTUM_SIZE. The issue we fix here is,
  // to correct the .ll assembly code so that things work with QUANTUM_SIZE=1.
  //
  substrate.addActor('QuantumFixer', {
    processItem: function(item) {
      this.forwardItem(item, 'LabelAnalyzer');
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
  });

  // Label analyzer
  substrate.addActor('LabelAnalyzer', {
    processItem: function(item) {
      item.functions.forEach(function(func) {
        func.labelsDict = {};
        func.labelIds = {};
        func.labelIdCounter = 0;
        func.labels.forEach(function(label) {
          func.labelsDict[label.ident] = label;
          func.labelIds[label.ident] = func.labelIdCounter++;
        });
        func.labelIds[toNiceIdent('%0')] = -1; // entry is always -1

        func.hasIndirectBr = false;
        func.lines.forEach(function(line) {
          if (line.intertype == 'indirectbr') {
            func.hasIndirectBr = true;
          }
        });

        // The entry might not have an explicit label, and there is no consistent naming convention for it - it can be %0 or %1
        // So we need to handle that in a special way here.
        function getActualLabelId(labelId) {
          if (func.labelsDict[labelId]) return labelId;
          if (labelId in ENTRY_IDENTS) {
            assert(func.labelsDict[ENTRY_IDENT]);
            return ENTRY_IDENT;
          }
          return null;
        }

        if (!MICRO_OPTS) {
          // 'Emulate' phis, by doing an if where the phi appears in the .ll. For this
          // we need __lastLabel__.
          func.needsLastLabel = false;
          func.labels.forEach(function(label) {
            var phis = [];
            label.lines.forEach(function(phi) {
              if (phi.intertype == 'phi') {
                for (var i = 0; i < phi.params.length; i++) {
                  var sourceLabelId = getActualLabelId(phi.params[i].label);
                  if (sourceLabelId) {
                    var sourceLabel = func.labelsDict[sourceLabelId];
                    var lastLine = sourceLabel.lines.slice(-1)[0];
                    assert(lastLine.intertype in LLVM.PHI_REACHERS, 'Only some can lead to labels with phis:' + [func.ident, label.ident, lastLine.intertype]);
                    lastLine.currLabelId = sourceLabelId;
                  }
                }
                phis.push(phi);
                func.needsLastLabel = true;
              }
            });

            if (phis.length >= 2) {
              // Multiple phis have the semantics that they all occur 'in parallel', i.e., changes to
              // a variable that is the result of a phi should *not* affect the other results. We must
              // therefore be careful!
              phis[phis.length-1].postSet = '; /* post-phi: */';
              for (var i = 0; i < phis.length-1; i++) {
                var ident = phis[i].assignTo;
                var phid = ident+'$phi'
                phis[phis.length-1].postSet += ident + '=' + phid + ';';
                phis[i].assignTo = phid;
                func.variables[phid] = {
                  ident: phid,
                  type: func.variables[ident].type,
                  origin: func.variables[ident].origin,
                  lineNum: func.variables[ident].lineNum,
                  uses: 1,
                  impl: VAR_EMULATED
                };
              }
            }
          });
        } else {
          // MICRO_OPTS == 1: Properly implement phis, by pushing them back into the branch
          // that leads to here. We will only have the |var| definition in this location.

          // First, push phis back
          func.labels.forEach(function(label) {
            label.lines.forEach(function(phi) {
              if (phi.intertype == 'phi') {
                for (var i = 0; i < phi.params.length; i++) {
                  var param = phi.params[i];
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
        }
      });
      this.forwardItem(item, 'StackAnalyzer');
    }
  });

  // Stack analyzer - calculate the base stack usage
  substrate.addActor('StackAnalyzer', {
    processItem: function(data) {
      data.functions.forEach(function(func) {
        var lines = func.labels[0].lines;
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i];
          if (!item.assignTo || item.intertype != 'alloca') break;
          assert(isNumber(item.allocatedNum));
          item.allocatedSize = func.variables[item.assignTo].impl === VAR_EMULATED ?
            calcAllocatedSize(item.allocatedType)*item.allocatedNum: 0;
          if (USE_TYPED_ARRAYS === 2) {
            // We need to keep the stack aligned
            item.allocatedSize = Runtime.forceAlign(item.allocatedSize, QUANTUM_SIZE);
          }
        }
        var index = 0;
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i];
          if (!item.assignTo || item.intertype != 'alloca') break;
          item.allocatedIndex = index;
          index += item.allocatedSize;
          delete item.allocatedSize;
        }
        func.initialStack = index;
        func.otherStackAllocations = false;
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
          for (var i = 0; i < lines.length; i++) {
            var item = lines[i];
            if (!item.assignTo || item.intertype != 'alloca') {
              finishedInitial = true;
              continue;
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
      this.forwardItem(data, 'Relooper');
    }
  });

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

  //! @param toLabelId If false, just a dry run - useful to search for labels
  function replaceLabels(line, labelIds, toLabelId) {
    var ret = [];

    var value = keys(labelIds)[0];
    var wildcard = value.indexOf('*') >= 0;
    assert(!wildcard || values(labelIds).length == 1); // For now, just handle that case
    var wildcardParts = null;
    if (wildcard) {
      wildcardParts = value.split('|');
    }
    function wildcardCheck(s) {
      var parts = s.split('|');
      for (var i = 0; i < 3; i++) {
        if (wildcardParts[i] !== '*' && wildcardParts[i] != parts[i]) return false;
      }
      return true;
    }

    operateOnLabels(line, function process(item, id) {
      if (item[id] in labelIds || (wildcard && wildcardCheck(item[id]))) {
        ret.push(item[id]);
        if (dcheck('relooping')) dprint('zz ' + id + ' replace ' + item[id] + ' with ' + toLabelId);
        if (toLabelId) {
          // replace wildcards in new value with old parts
          var oldParts = item[id].split('|');
          var newParts = toLabelId.split('|');
          for (var i = 1; i < 3; i++) {
            if (newParts[i] === '*') newParts[i] = oldParts[i];
          }
          item[id] = newParts.join('|') + '|' + item[id];
        }
      }
    });
    return ret;
  }

  function replaceLabelLabels(labels, labelIds, toLabelId) {
    ret = [];
    labels.forEach(function(label) {
      ret = ret.concat(replaceLabels(label.lines[label.lines.length-1], labelIds, toLabelId));
    });
    return ret;
  }

  function isReachable(label, otherLabels, ignoreLabel) { // is label reachable by otherLabels, ignoring ignoreLabel in those otherLabels
    var reachable = false;
    otherLabels.forEach(function(otherLabel) {
      reachable = reachable || (otherLabel !== ignoreLabel && (label.ident == otherLabel.ident ||
                                                               label.ident in otherLabel.allOutLabels));
    });
    return reachable;
  }

  // ReLooper - reconstruct nice loops, as much as possible
  substrate.addActor('Relooper', {
    processItem: function(item) {
      var that = this;
      function finish() {
        that.forwardItem(item, 'LoopOptimizer');
      }

      // Tools

      function calcLabelBranchingData(labels, labelsDict) {
        labels.forEach(function(label) {
          label.outLabels = [];
          label.inLabels = [];
          label.hasReturn = false;
          label.hasBreak = false;
        });
        // Find direct branchings
        labels.forEach(function(label) {
          var line = label.lines[label.lines.length-1];
          operateOnLabels(line, function process(item, id) {
            if (item[id][0] == 'B') { // BREAK, BCONT, BNOPP, BJSET
              label.hasBreak = true;
            } else {
              label.outLabels.push(item[id]);
              labelsDict[item[id]].inLabels.push(label.ident);
            }
          });
          label.hasReturn |= line.intertype == 'return';
        });
        // Find all incoming and all outgoing - recursively
        labels.forEach(function(label) {
          label.allInLabels = [];
          label.allOutLabels = [];
        });

        // First, find allInLabels
        var more = true, nextModified, modified = set(getLabelIds(labels));
        while (more) {
          more = false;
          nextModified = {};
          for (var labelId in modified) {
            var label = labelsDict[labelId];
            var temp = label.inLabels;
            label.inLabels.forEach(function(label2Id) {
              temp = temp.concat(labelsDict[label2Id].allInLabels);
            });
            temp = dedup(temp);
            if (temp.length > label.allInLabels.length) {
              label.allInLabels = temp;
              for (var i = 0; i < label.outLabels.length; i++) {
                nextModified[label.outLabels[i]] = true;
              }
              more = true;
            }
          }
          modified = nextModified;
        }

        // Infer allOutLabels from allInLabels, they are mirror images
        labels.forEach(function(label) {
          label.allInLabels.forEach(function(inLabelId) {
            labelsDict[inLabelId].allOutLabels.push(label.ident);
          });
        });

        labels.forEach(function(label) {
          if (dcheck('relooping')) {
            dprint('// label: ' + label.ident + ' :out      : ' + JSON.stringify(label.outLabels));
            dprint('//        ' + label.ident + ' :in       : ' + JSON.stringify(label.inLabels));
            dprint('//        ' + label.ident + ' :ALL out  : ' + JSON.stringify(label.allOutLabels));
            dprint('//        ' + label.ident + ' :ALL in   : ' + JSON.stringify(label.allInLabels));
          }

          // Convert to set, for speed (we mainly do lookups here) and code clarity (x in Xlabels)
          // Also removes duplicates (which we can get in llvm switches)
          // TODO do we need all these?
          label.outLabels = set(label.outLabels);
          label.inLabels = set(label.inLabels);
          label.allOutLabels = set(label.allOutLabels);
          label.allInLabels = set(label.allInLabels);
        });
      }

      var idCounter = 0;
      function makeBlockId(entries) {
        idCounter++;
        return entries.join('$') + '$' + idCounter;
      }

      // There are X main kinds of blocks:
      //
      //----------------------------------------------------------------------------------------
      //
      //  'emulated': A soup of labels, implemented as a barbaric switch in a loop. Any
      //              label can get to any label. No block follows this.
      //
      //  'reloop': That is a block of the following shape:
      //
      //       loopX: while(1) {
      //         // internal labels, etc. Labels are internal to the current one, if
      //         // they can return to it.
      //         //
      //         // Such labels can either do |continue loopX| to get back to the entry label,
      //         // or set __label__ and do |break loopX| to get to any of the external entries
      //         // they need to get to. External labels, of course, are those that cannot
      //         // get to the entry
      //       }
      //       // external labels
      //
      //  'multiple': A block that branches into multiple subblocks, each independent,
      //              finally leading outside into another block afterwards
      //              For now, we do this in a loop, so we can break out of it easily to get
      //              to the labels afterwards. TODO: Optimize that out
      //
      function makeBlock(labels, entries, labelsDict, forceEmulated) {
        if (labels.length == 0) return null;
        dprint('relooping', 'prelooping: ' + entries + ',' + labels.length + ' labels');
        assert(entries && entries[0]); // need at least 1 entry

        var blockId = makeBlockId(entries);

        var emulated = {
          type: 'emulated',
          id: blockId,
          labels: labels,
          entries: entries.slice(0)
        };
        if (!RELOOP || forceEmulated) return emulated;

        calcLabelBranchingData(labels, labelsDict);

        var s_entries = set(entries);
        dprint('relooping', 'makeBlock: ' + entries + ',' + labels.length + ' labels');

        var entryLabels = entries.map(function(entry) { return labelsDict[entry] });
        assert(entryLabels[0]);

        var canReturn = false, mustReturn = true;
        entryLabels.forEach(function(entryLabel) {
          var curr = values(entryLabel.inLabels).length > 0;
          canReturn = canReturn || curr;
          mustReturn = mustReturn && curr;
        });

        // Remove unreachables
        allOutLabels = {};
        entryLabels.forEach(function(entryLabel) {
          mergeInto(allOutLabels, entryLabel.allOutLabels);
        });
        labels = labels.filter(function(label) { return label.ident in s_entries || label.ident in allOutLabels });

        // === (simple) 'emulated' ===

        if (entries.length == 1 && !canReturn) {
          var entry = entries[0];
          var entryLabel = entryLabels[0];
          var others = labels.filter(function(label) { return label.ident != entry });

          var nextEntries = keys(entryLabel.outLabels);
          dprint('relooping', '   Creating simple emulated, outlabels: ' + nextEntries);
          nextEntries.forEach(function(nextEntry) {
            replaceLabelLabels([entryLabel], set(nextEntry), 'BJSET|' + nextEntry); // Just SET __label__ - no break or continue or whatnot
          });
          return {
            type: 'emulated',
            id: blockId,
            labels: [entryLabel],
            entries: entries,
            next: makeBlock(others, keys(entryLabel.outLabels), labelsDict)
          };
        }

        // === 'reloop' away a loop, if we need to ===

        function makeLoop() {
          var ret = {
            type: 'reloop',
            id: blockId,
            needBlockId: true,
            entries: entries,
            labels: labels
          };

          // Find internal and external labels
          var split_ = splitter(labels, function(label) {
            // External labels are those that are (1) not an entry, and (2) cannot reach an entry. In other words,
            // the labels inside the loop are the entries and those that can return to the entries.
            return !(label.ident in s_entries) && values(setIntersect(s_entries, label.allOutLabels)).length == 0;
          });
          var externals = split_.splitOut;
          var internals = split_.leftIn;
          var externalsLabels = set(getLabelIds(externals));

          if (dcheck('relooping')) dprint('   Creating reloop: Inner: ' + dump(getLabelIds(internals)) + ', Exxer: ' + dump(externalsLabels));

          if (ASSERTIONS) {
            // Verify that no external can reach an internal
            var inLabels = set(getLabelIds(internals));
            externals.forEach(function(external) {
              if (values(setIntersect(external.outLabels, inLabels)).length > 0) {
                dprint('relooping', 'Found an external that wants to reach an internal, fallback to emulated?');
                throw "Spaghetti label flow";
              }
            });
          }

          // We will be in a loop, |continue| gets us back to the entry
          var pattern = 'BCONT|' + blockId;
          if (entries.length == 1) {
            // We are returning to a loop that has one entry, so we don't need to set __label__
            pattern = 'BCNOL|' + blockId;
          }
          entries.forEach(function(entry) {
            replaceLabelLabels(internals, set(entries), pattern);
          });

          // Find the entries of the external labels
          var externalsEntries = {};
          internals.forEach(function(internal) {
            mergeInto(externalsEntries, setIntersect(internal.outLabels, externalsLabels));
          });
          externalsEntries = keys(externalsEntries);

          // We also want to include additional labels inside the loop. If the loop has just one exit label,
          // then that is fine - keep the loop small by having the next code outside, and do not set __label__ in
          // that break. If there is more than one, though, we can avoid __label__ checks in a multiple outside
          // by hoisting labels into the loop.
          if (externalsEntries.length > 1) {
            (function() {
              // If an external entry would make the loop too big, don't hoist
              var maxHoist = Infinity; //sum(internals.map(function(internal) { return internal.lines.length }));
              var avoid = externalsEntries.map(function(l) { return labelsDict[l] });
              var totalNewEntries = {};
              for (var i = 0; i < externalsEntries.length; i++) {
                var exitLabel = labelsDict[externalsEntries[i]];
                // Check if hoisting this external entry is worthwhile. We first do a dry run, aborting on
                // loops (which we never hoist, to avoid over-nesting) or on seeing too many labels would be hoisted
                // (to avoid enlarging loops too much). If the dry run succeeded, it will stop when it reaches
                // places where we rejoin other external entries.
                var seen, newEntries;
                function prepare() {
                  seen = {};
                  newEntries = {};
                }
                function hoist(label, dryRun) { // returns false if aborting
                  if (seen[label.ident]) return true;
                  seen[label.ident] = true;
                  if (label.ident in label.allInLabels) return false; // loop, abort
                  if (isReachable(label, avoid, exitLabel)) {
                    // We rejoined, so this is a new external entry
                    newEntries[label.ident] = true;
                    return true;
                  }
                  // Hoistable.
                  if (!dryRun) {
                    dprint('relooping', 'Hoisting into loop: ' + label.ident);
                    internals.push(label);
                    externals = externals.filter(function(l) { return l != label }); // not very efficient at all TODO: optimize
                  }
                  for (var outLabelId in label.outLabels) {
                    var outLabel = labelsDict[outLabelId];
                    if (!hoist(outLabel, dryRun)) return false;
                  }
                  return true;
                }
                prepare();
                if (hoist(exitLabel, true)) {
                  var seenList = unset(seen);
                  var num = sum(seenList.map(function(seen) { return labelsDict[seen].lines.length }));
                  // Only hoist if the sizes make sense
                  if (seenList.length >= 1 && num <= maxHoist) { // && unset(newEntries).length <= 1) {
                    prepare();
                    hoist(exitLabel);
                    mergeInto(totalNewEntries, newEntries);
                    externalsEntries.splice(i, 1);
                    i--;
                  }
                }
              }
              externalsLabels = set(getLabelIds(externals));
              externalsEntries = keys(set(externalsEntries.concat(unset(totalNewEntries))));
              assert(externalsEntries.length > 0 || externals.length == 0);
            })();
          }

          // To get to any of our (not our parents') exit labels, we will break.
          if (dcheck('relooping')) dprint('for exit purposes, Replacing: ' + dump(externalsLabels));
          if (externals.length > 0) {
            assert(externalsEntries.length > 0);
            var pattern = 'BREAK|' + blockId;
            if (externalsEntries.length == 1) {
              // We are breaking out of a loop and have one entry after it, so we don't need to set __label__ 
              pattern = 'BRNOL|' + blockId;
            }
            replaceLabelLabels(internals, externalsLabels, pattern);
            if (dcheck('relooping')) dprint('externalsEntries: ' + dump(externalsEntries));
          }

          // inner
          ret.inner = makeBlock(internals, entries, labelsDict);

          if (externals.length > 0) {
            // outer
            ret.next = makeBlock(externals, externalsEntries, labelsDict);
          }

          return ret;
        }

        // XXX change this logic?
        if (entries.length === 1 && canReturn) return makeLoop();

        // === handle multiple branches from the entry with a 'multiple' ===
        //
        // For each entry, try to 'build it out' as much as possible. Add labels, until
        //    * hit a post label
        //    * hit a label reachable by another actual entry

        dprint('relooping', 'trying multiple...');

        var shouldNotReach = entryLabels;
        var handlingNow = [];
        var actualEntryLabels = [];
        var postEntryLabels = {};
        entryLabels.forEach(function(entryLabel) {
          entryLabel.blockChildren = [];
          var visited = {};
          function tryAdd(label) {
            if (label.ident in visited) return;
            visited[label.ident] = true;
            if (!isReachable(label, shouldNotReach, entryLabel)) {
              entryLabel.blockChildren.push(label);
              handlingNow.push(label);
              keys(label.outLabels).forEach(function(outLabelId) { tryAdd(labelsDict[outLabelId]) });
            } else {
              postEntryLabels[label.ident] = true; // This will be an entry in the next block
            }
          }
          tryAdd(entryLabel);
          if (entryLabel.blockChildren.length > 0) {
            dprint('relooping', '  Considering multiple, found a valid entry, ' + entryLabel.ident);
            actualEntryLabels.push(entryLabel);
          }
        });

        if (dcheck('relooping')) dprint('  Considering multiple, canHandle: ' + getLabelIds(handlingNow));

        if (handlingNow.length > 0) {
          // This is a 'multiple'

          var actualEntries = getLabelIds(actualEntryLabels);
          if (dcheck('relooping')) dprint('   Creating multiple, with entries: ' + actualEntries + ', post entries: ' + dump(postEntryLabels));
          actualEntryLabels.forEach(function(actualEntryLabel) {
            if (dcheck('relooping')) dprint('      creating sub-block in multiple for ' + actualEntryLabel.ident + ' : ' + getLabelIds(actualEntryLabel.blockChildren) + ' ::: ' + actualEntryLabel.blockChildren.length);

            var pattern = 'BREAK|' + blockId;
            if (keys(postEntryLabels).length == 1) {
              // We are breaking out of a multiple and have one entry after it, so we don't need to set __label__
              pattern = 'BRNOL|' + blockId;
            }
            keys(postEntryLabels).forEach(function(post) {
              replaceLabelLabels(actualEntryLabel.blockChildren, set(post), pattern);
            });

            // Create child block
            actualEntryLabel.block = makeBlock(actualEntryLabel.blockChildren, [actualEntryLabel.blockChildren[0].ident], labelsDict);
          });
          return {
            type: 'multiple',
            id: blockId,
            needBlockId: true,
            entries: actualEntries,
            entryLabels: actualEntryLabels,
            labels: handlingNow,
            next: makeBlock(labels.filter(function(label) { return handlingNow.indexOf(label) == -1 }), keys(postEntryLabels), labelsDict)
          };
        }

        assert(canReturn, 'If not a multiple, must be able to create a loop');

        return makeLoop();
      }

      // TODO: each of these can be run in parallel
      item.functions.forEach(function(func) {
        dprint('relooping', "// relooping function: " + func.ident);
        func.block = makeBlock(func.labels, [toNiceIdent(func.labels[0].ident)], func.labelsDict, func.hasIndirectBr);
      });

      return finish();
    }
  });

  // LoopOptimizer. The Relooper generates native loop structures, that are
  //       logically correct. The LoopOptimizer works on that, doing further optimizations
  //       like switching to BNOPP when possible, etc.

  substrate.addActor('LoopOptimizer', {
    processItem: function(item) {
      var that = this;
      function finish() {
        item.__finalResult__ = true;
        return [item];
      }
      if (!RELOOP) return finish();

      // Find where each block will 'naturally' get to, just by the flow of code
      function exploreBlockEndings(block, endOfTheWorld) { // endoftheworld - where we will get, if we have nothing else to get to - 'fall off the face of the earth'
        if (!block) return;

        function singular(block) {
          if (!block) return endOfTheWorld;
          if (block.type === 'multiple') return null;
          if (block.entries.length == 1) {
            return block.entries[0];
          } else {
            return null;
          }
        }

        dprint('relooping', "//    exploring block: " + block.type + ' : ' + block.entries);

        if (block.type == 'reloop') {
          exploreBlockEndings(block.inner, singular(block.inner));
        } else if (block.type == 'multiple') {
          block.entryLabels.forEach(function(entryLabel) { exploreBlockEndings(entryLabel.block, singular(block.next)) });
        }

        exploreBlockEndings(block.next, endOfTheWorld);

        if (block.next) {
          block.willGetTo = singular(block.next);
        } else {
          block.willGetTo = endOfTheWorld;
        }

        dprint('relooping', "//    explored block: " + block.type + ' : ' + block.entries + ' , willGetTo: ' + block.willGetTo);
      }

      // Remove unneeded label settings, if we set it to where we will get anyhow
      function optimizeBlockEndings(block) {
        if (!block) return;

        dprint('relooping', "//    optimizing block: " + block.type + ' : ' + block.entries);

        recurseBlock(block, optimizeBlockEndings);

        if (block.type === 'emulated' && block.willGetTo) {
          dprint('relooping', '//         removing (trying): ' + block.willGetTo);
          replaceLabelLabels(block.labels, set('BJSET|*|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BCONT|*|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BREAK|*|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BRNOL|*|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BCNOL|*|' + block.willGetTo), 'BNOPP');
        }
      }

      // TODO: Parallelize
      item.functions.forEach(function(func) {
        dprint('relooping', "// loopOptimizing function: " + func.ident);
        exploreBlockEndings(func.block);
        optimizeBlockEndings(func.block);
      });
      return finish();
    }
  });

  // Data
  substrate.addItem({
    items: data
  }, 'Sorter');

  // Solve it
  return substrate.solve();
}

