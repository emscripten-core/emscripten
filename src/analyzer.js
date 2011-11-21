// Analyze intertype data. Calculates things that are necessary in order
// to do the final conversion into JavaScript later, for example,
// properties of variables, loop structures of functions, etc.

VAR_NATIVE = 'native';
VAR_NATIVIZED = 'nativized';
VAR_EMULATED = 'emulated';

function cleanFunc(func) {
  func.lines = func.lines.filter(function(line) { return line.intertype !== null });
  func.labels.forEach(function(label) {
    label.lines = label.lines.filter(function(line) { return line.intertype !== null });
  });
}

function analyzer(data) {
  // Substrate
  substrate = new Substrate('Analyzer');

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
      ['globalVariable', 'functionStub', 'unparsedFunction', 'alias'].forEach(function(intertype) {
        var temp = splitter(item.items, function(item) { return item.intertype == intertype });
        item.items = temp.leftIn;
        item[intertype + 's'] = temp.splitOut;
      });
      var temp = splitter(item.items, function(item) { return item.intertype == 'type' });
      item.items = temp.leftIn;
      temp.splitOut.forEach(function(type) {
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
          subItem.lines = [];
          subItem.labels = [];

          // no explicit 'entry' label in clang on LLVM 2.8 - most of the time, but not all the time! - so we add one if necessary
          if (LLVM_STYLE == 'new' && item.items[i+1].intertype !== 'label') {
            item.items.splice(i+1, 0, {
              intertype: 'label',
              ident: toNiceIdent('%0'),
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
            item.functions.slice(-1)[0].lines.push(subItem);
            item.functions.slice(-1)[0].labels.slice(-1)[0].lines.push(subItem); // If this line fails, perhaps missing a label? LLVM_STYLE related?
            if (subItem.intertype === 'branch') {
              currLabelFinished = true;
            }
          } else {
            print('// WARNING: content after a branch in a label, line: ' + subItem.lineNum + '::' + dump(subItem));
          }
        } else {
          throw "ERROR: what is this? " + JSON.stringify(subItem);
        }
      }
      delete item.items;
      this.forwardItem(item, 'Typevestigator');
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
    var check = new RegExp(/^\[(\d+)\ x\ (.*)\]$/g).exec(nonPointing);
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
      for (type in Types.needAnalysis) {
        if (type) addType(type, data);
      }
      Types.needAnalysis = [];
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
        values(types).forEach(function(type) {
          if (type.flatIndexes) return;
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
            return;
          }

          Runtime.calculateStructAlignment(type);

          dprint('types', 'type (fat=' + !!fatTypes + '): ' + type.name_ + ' : ' + JSON.stringify(type.fields));
          dprint('types', '                        has final size of ' + type.flatSize + ', flatting: ' + type.needsFlattening + ' ? ' + (type.flatFactor ? type.flatFactor : JSON.stringify(type.flatIndexes)));
        });
      }

      if (QUANTUM_SIZE === 1 && !fatTypes) {
        Types.flipTypes();
        // Fake a quantum size of 4 for fat types. TODO: Might want non-4 for some reason?
        var trueQuantumSize = QUANTUM_SIZE;
        QUANTUM_SIZE = 4;
        analyzeTypes(item, true);
        QUANTUM_SIZE = trueQuantumSize;
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
        dprint('vars', 'Analyzing variables in ' + func.ident);

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
              uses: null
            };
          }
        });

        // Normal variables
        func.lines.forEach(function(item) {
          if (item.intertype === 'assign') {
            if (!item.value.tokens.slice(-1)[0].item) throw 'Did you run llvm-dis with -show-annotations?';
            var commaIndex = getTokenIndexByText(item.value.tokens, ';');
            var variable = func.variables[item.ident] = {
              ident: item.ident,
              type: item.value.type,
              origin: item.value.intertype,
              lineNum: item.lineNum,
              uses: parseInt(item.value.tokens[commaIndex+1].item.tokens[0].text.split('=')[1])
            };
            assert(isNumber(variable.uses), 'Failed to find the # of uses of var: ' + item.ident);
            if (variable.origin === 'alloca') {
              variable.allocatedNum = item.value.allocatedNum;
            }
          }
        });

        if (QUANTUM_SIZE === 1) {
          // Second pass over variables - notice when types are crossed by bitcast

          func.lines.forEach(function(item) {
            if (item.intertype === 'assign' && item.value.intertype === 'bitcast') {
              // bitcasts are unique in that they convert one pointer to another. We
              // sometimes need to know the original type of a pointer, so we save that.
              //
              // originalType is the type this variable is created from
              // derivedTypes are the types that this variable is cast into
              func.variables[item.ident].originalType = item.value.type2;

              if (!isNumber(item.value.ident)) {
                if (!func.variables[item.value.ident].derivedTypes) {
                  func.variables[item.value.ident].derivedTypes = [];
                }
                func.variables[item.value.ident].derivedTypes.push(item.value.type);
              }
            }
          });
        }

        for (vname in func.variables) {
          var variable = func.variables[vname];

          // Whether the value itself is used. For an int, always yes. For a pointer,
          // we might never use the pointer's value - we might always just store to it /
          // read from it. If so, then we can optimize away the pointer.
          variable.hasValueTaken = false;
          // Whether our address was used. If not, then we do not need to bother with
          // implementing this variable in a way that other functions can access it.
          variable.hasAddrTaken = false;

          variable.pointingLevels = pointingLevels(variable.type);

          // Analysis!

          if (variable.pointingLevels > 0) {
            // Pointers
            variable.loads = 0;
            variable.stores = 0;

            func.lines.forEach(function(line) {
              //print(dump(line))
              if (line.intertype == 'store' && line.ident == vname) {
                variable.stores ++;
              } else if (line.intertype == 'assign' && line.value.intertype == 'load' && line.value.ident == vname) {
                variable.loads ++;
              }
            });

            variable.otherUses = variable.uses - variable.loads - variable.stores;
            if (variable.otherUses > 0)
              variable.hasValueTaken = true;
          }
 
          // Decision time

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
          } else if (MICRO_OPTS && variable.pointingLevels === 0 && !variable.hasAddrTaken) {
            // A simple int value, can be implemented as a native variable
            variable.impl = VAR_NATIVE;
          } else if (MICRO_OPTS && variable.origin === 'alloca' && !variable.hasAddrTaken && !variable.hasValueTaken &&
                     variable.allocatedNum === 1 &&
                     (Runtime.isNumberType(pointedType) || Runtime.isPointerType(pointedType))) {
            // A pointer to a value which is only accessible through this pointer. Basically
            // a local value on the stack, which nothing fancy is done on. So we can
            // optimize away the pointing altogether, and just have a native variable
            variable.impl = VAR_NATIVIZED;
          } else {
            variable.impl = VAR_EMULATED;
          }
          dprint('vars', '// var ' + vname + ': ' + JSON.stringify(variable));
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
          if (line.intertype === 'assign' && line.value.intertype === 'load') {
            var data = func.variables[line.ident]
            if (data.type === 'i1') {
              line.value.unsigned = true;
              return;
            }

            var total = data.uses;
            if (total === 0) return;
            var obj = { ident: line.ident, found: 0, unsigned: 0, signed: 0, total: total };
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
            line.value.unsigned = obj.unsigned > 0;
            dprint('vars', 'Signalyzer: ' + line.ident + ' has unsigned == ' + line.value.unsigned + ' (line ' + line.lineNum + ')');
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
          QUANTUM_SIZE = 4;
        }
        var ret = getNativeTypeSize(type);
        if (fat) {
          QUANTUM_SIZE = 1;
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
            var check = new RegExp(/^\[(\d+)\ x\ (.*)\]\*$/g).exec(ret);
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
                param.ident = param.value.text = corrected.toString();
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

        if (!MICRO_OPTS) {
          // 'Emulate' phis, by doing an if where the phi appears in the .ll. For this
          // we need __lastLabel__.
          func.needsLastLabel = false;
          func.labels.forEach(function(label) {
            var phis = [], phi;
            label.lines.forEach(function(line) {
              if ((phi = line.value) && phi.intertype == 'phi') {
                for (var i = 0; i < phi.params.length; i++) {
                  var sourceLabelId = phi.params[i].label;
                  var sourceLabel = func.labelsDict[sourceLabelId];
                  var lastLine = sourceLabel.lines.slice(-1)[0];
                  if (lastLine.intertype == 'assign') lastLine = lastLine.value;
                  assert(lastLine.intertype in LLVM.PHI_REACHERS, 'Only some can lead to labels with phis:' + [func.ident, label.ident, lastLine.intertype]);
                  lastLine.currLabelId = sourceLabelId;
                }
                phis.push(line);
                func.needsLastLabel = true;
              }
            });

            if (phis.length >= 2) {
              // Multiple phis have the semantics that they all occur 'in parallel', i.e., changes to
              // a variable that is the result of a phi should *not* affect the other results. We must
              // therefore be careful!
              phis[phis.length-1].value.postSet = '; /* post-phi: */';
              for (var i = 0; i < phis.length-1; i++) {
                var ident = phis[i].ident;
                var phid = ident+'$phi'
                phis[phis.length-1].value.postSet += ident + '=' + phid + ';';
                phis[i].ident = phid;
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
            label.lines.forEach(function(line) {
              var phi;
              if ((phi = line.value) && phi.intertype == 'phi') {
                for (var i = 0; i < phi.params.length; i++) {
                  var param = phi.params[i];
                  var sourceLabelId = param.label;
                  var sourceLabel = func.labelsDict[sourceLabelId];
                  var lastLine = sourceLabel.lines.slice(-1)[0];
                  if (lastLine.intertype == 'assign') lastLine = lastLine.value;
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
                    ident: line.ident,
                    value: param.value,
                    targetLabel: label.ident
                  });
                }
                // The assign to phi is now just a var
                line.intertype = 'var';
                line.value = null;
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
          var line = lines[i];
          var item = line.value;
          if (!item || item.intertype != 'alloca') break;
          assert(isNumber(item.allocatedNum));
          item.allocatedSize = func.variables[line.ident].impl === VAR_EMULATED ?
            calcAllocatedSize(item.allocatedType)*item.allocatedNum: 0;
        }
        var index = 0;
        for (var i = 0; i < lines.length; i++) {
          var item = lines[i].value;
          if (!item || item.intertype != 'alloca') break;
          if (USE_TYPED_ARRAYS === 2) index = Runtime.forceAlign(index, Math.min(item.allocatedSize, QUANTUM_SIZE));
          item.allocatedIndex = index;
          index += item.allocatedSize;
          delete item.allocatedSize;
        }
        func.initialStack = index;
      });
      this.forwardItem(data, 'Relooper');
    }
  });

  var BRANCH_INVOKE = set('branch', 'invoke');
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
    } else if (line.intertype == 'assign' && line.value.intertype == 'invoke') {
      process(line.value);
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
        dprint('relooping', 'zz ' + id + ' replace ' + item[id] + ' with ' + toLabelId);
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

  // ReLooper - reconstruct nice loops, as much as possible
  substrate.addActor('Relooper', {
    processItem: function(item) {
      var that = this;
      function finish() {
        that.forwardItem(item, 'LoopOptimizer');
      }

      // Tools

      function calcLabelBranchingData(labels, labelsDict) {
        item.functions.forEach(function(func) {
          labels.forEach(function(label) {
            label.outLabels = [];
            label.inLabels = [];
            label.hasReturn = false;
            label.hasBreak = false;
            if (!label.originalOutlabels) {
              label.originalOutLabels = [];
              label.needOriginalOutLabels = true;
            }
          });
        });
        // Find direct branchings
        labels.forEach(function(label) {
          [label.lines[label.lines.length-1]].forEach(function(line) {
            operateOnLabels(line, function process(item, id) {
              if (item[id][0] == 'B') { // BREAK, BCONT, BNOPP, BJSET
                label.hasBreak = true;
              } else {
                label.outLabels.push(item[id]);
                labelsDict[item[id]].inLabels.push(label.ident);
              }
              if (label.needOriginalOutLabels) {
                label.originalOutLabels.push(item[id]);
              }
            });
            label.needOriginalOutLabels = false;
            label.hasReturn |= line.intertype == 'return';
          });
        });
        // Find all incoming and all outgoing - recursively
        labels.forEach(function(label) {
          label.allInLabels = [];
          label.allOutLabels = [];
        });

        var worked = true;
        while (worked) {
          worked = false;
          labels.forEach(function(label) {
            function inout(s, l) {
              var temp = label[s].slice(0);
              label[s].forEach(function(label2Id) {
                temp = temp.concat(labelsDict[label2Id][l]);
              });
              temp = dedup(temp);
              temp.sort();
              if (JSON.stringify(label[l]) != JSON.stringify(temp)) {
                label[l] = temp;
                worked = true;
              }
            }
            inout('inLabels', 'allInLabels');
            inout('outLabels', 'allOutLabels');
          });
        }

        labels.forEach(function(label) {
          if (dcheck('relooping')) {
            dprint('// label: ' + label.ident + ' :out      : ' + JSON.stringify(label.outLabels));
            dprint('//        ' + label.ident + ' :in       : ' + JSON.stringify(label.inLabels));
            dprint('//        ' + label.ident + ' :ALL out  : ' + JSON.stringify(label.allOutLabels));
            dprint('//        ' + label.ident + ' :ALL in   : ' + JSON.stringify(label.allInLabels));
            dprint('//        ' + label.ident + ' :origOut  : ' + JSON.stringify(label.originalOutLabels));
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
            return !(label.ident in s_entries) && values(setIntersect(s_entries, label.allOutLabels)).length == 0;
          });
          var externals = split_.splitOut;
          var internals = split_.leftIn;
          var currExitLabels = set(getLabelIds(externals));

          dprint('relooping', function() { return '   Creating reloop: Inner: ' + dump(getLabelIds(internals)) + ', Exxer: ' + dump(currExitLabels) });

          // Verify that no external can reach an internal
          var inLabels = set(getLabelIds(internals));
          externals.forEach(function(external) {
            if (values(setIntersect(external.outLabels, inLabels)).length > 0) {
              dprint('relooping', 'Found an external that wants to reach an internal, fallback to emulated?');
              throw "Spaghetti label flow";
            }
          });

          // We will be in a loop, |continue| gets us back to the entry
          entries.forEach(function(entry) {
            replaceLabelLabels(internals, set(entries), 'BCONT|' + blockId);
          });

          // To get to any of our (not our parents') exit labels, we will break.
          dprint('relooping', 'for exit purposes, Replacing: ' + dump(currExitLabels));
          var enteredExitLabels = {};
          if (externals.length > 0) {
            entries.forEach(function(entry) {
              mergeInto(enteredExitLabels, set(replaceLabelLabels(internals, currExitLabels, 'BREAK|' + blockId)));
            });
            enteredExitLabels = keys(enteredExitLabels).map(cleanLabel);
            dprint('relooping', 'enteredExitLabels: ' + dump(enteredExitLabels));
            assert(enteredExitLabels.length > 0);
          }

          // inner
          ret.inner = makeBlock(internals, entries, labelsDict);

          if (externals.length > 0) {
            // outer
            ret.next = makeBlock(externals, enteredExitLabels, labelsDict);
          }

          return ret;
        }

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
            function isReachable(label, otherLabels, ignoreLabel) { // is label reachable by otherLabels, ignoring ignoreLabel in those otherLabels
              var reachable = false;
              otherLabels.forEach(function(otherLabel) {
                reachable = reachable || (otherLabel !== ignoreLabel && (label.ident == otherLabel.ident ||
                                                                         label.ident in otherLabel.allOutLabels));
              });
              return reachable;
            }

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

        dprint('relooping', '  Considering multiple, canHandle: ' + getLabelIds(handlingNow));

        if (handlingNow.length > 0) {
          // This is a 'multiple'

          var actualEntries = getLabelIds(actualEntryLabels);
          dprint('relooping', '   Creating multiple, with entries: ' + actualEntries + ', post entries: ' + dump(postEntryLabels));
          actualEntryLabels.forEach(function(actualEntryLabel) {
            dprint('relooping', '      creating sub-block in multiple for ' + actualEntryLabel.ident + ' : ' + getLabelIds(actualEntryLabel.blockChildren) + ' ::: ' + actualEntryLabel.blockChildren.length);

            keys(postEntryLabels).forEach(function(post) {
              replaceLabelLabels(actualEntryLabel.blockChildren, set(post), 'BREAK|' + blockId);
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
        } else if (block.type === 'emulated' && block.next && block.next.type === 'multiple') {
          assert(block.labels.length == 1);
          var lastLine = block.labels[0].lines.slice(-1)[0];
          if (lastLine.intertype == 'branch' && lastLine.ident) { // TODO: handle switch, and other non-branch2 things
            // 'Steal' the condition
            block.next.stolenCondition = lastLine;
            dprint('relooping', 'steal condition: ' + block.next.stolenCondition.ident);
            lastLine.stolen = true;
          }
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
          replaceLabelLabels(block.labels, set('BJSET|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BCONT|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
          replaceLabelLabels(block.labels, set('BREAK|*|' + block.willGetTo), 'BNOPP');
        } else if (block.type === 'multiple') {
          // Stolen conditions can be optimized further than the same branches in their original position
          var stolen = block.stolenCondition;
          if (stolen) {
            [stolen.labelTrue, stolen.labelFalse].forEach(function(entry) {
              entryLabel = block.entryLabels.filter(function(possible) { return possible.ident === getActualLabelId(entry) })[0];
              if (entryLabel) {
                replaceLabelLabels([{ lines: [stolen] }], set(entry), 'BNOPP');
              } else {
                replaceLabelLabels([{ lines: [stolen] }], set('BJSET|' + block.willGetTo + '|' + block.willGetTo), 'BNOPP');
              }
            });
          }

          // Check if the one-time loop (that allows breaking out) is actually needed
          if (replaceLabelLabels(block.labels, set('BREAK|' + block.id + '|*')).length === 0) {
            block.loopless = true;
          }
        }
      }

      // Checks whether we actually need labels. We return whether we have a loop nested inside us.
      function optimizeOutUnneededLabels(block) {
        if (!block) return false;

        dprint('relooping', "//    optimizing (2) block: " + block.type + ' : ' + block.entries);

        var containLoop = sum(recurseBlock(block, optimizeOutUnneededLabels)) > 0;

        if (block.type === 'emulated') {
          return containLoop;
        } else if (block.type === 'multiple') {
          // TODO: Apply the same optimization below for 'reloop', to looped multiples
          return containLoop || !block.loopless;
        } else if (block.type === 'reloop') {
          if (!containLoop) {
            block.needBlockId = false;

            replaceLabelLabels(block.labels, set('BCONT|' + block.id + '|*'), 'BCONT||*');
            replaceLabelLabels(block.labels, set('BREAK|' + block.id + '|*'), 'BREAK||*');
          }
          return true;
        }
        return assert(false);
      }

      // TODO: Parallelize
      item.functions.forEach(function(func) {
        dprint('relooping', "// loopOptimizing function: " + func.ident);
        exploreBlockEndings(func.block);
        optimizeBlockEndings(func.block);
        optimizeOutUnneededLabels(func.block);
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

