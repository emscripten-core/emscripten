// Convert analyzed data to javascript

function JSify(data, functionsOnly, givenTypes, givenFunctions, givenGlobalVariables) {
  substrate = new Substrate('JSifyer');

  var TYPES = functionsOnly ? givenTypes : data.types;

  var GLOBAL_VARIABLES = functionsOnly ? givenGlobalVariables : data.globalVariables;

  var FUNCTIONS = functionsOnly ? givenFunctions : {};
  // Now that analysis has completed, we can get around to handling unparsedFunctions
  (functionsOnly ? data.functions : data.unparsedFunctions.concat(data.functions)).forEach(function(func) {
    // Save just what we need, to save memory - whether there are varargs, and the # of parameters
    FUNCTIONS[func.ident] = {
      hasVarArgs: func.hasVarArgs,
      numParams: func.params.length
   };
  });

  for (var i = 0; i < data.unparsedFunctions.length; i++) {
    var func = data.unparsedFunctions[i];
    dprint('unparsedFunctions', 'processing |' + func.ident + '|, ' + i + '/' + data.unparsedFunctions.length);
    func.JS = JSify(analyzer(intertyper(func.lines, true, func.lineNum-1), TYPES), true, TYPES, FUNCTIONS, GLOBAL_VARIABLES);
    delete func.lines; // clean up memory as much as possible
  }

  // type
  substrate.addActor('Type', {
    processItem: function(item) {
      var type = TYPES[item.name_];
      var niceName = toNiceIdent(item.name_);
      // We might export all of TYPES, cleaner that way, but do not want slowdowns in accessing flatteners
      item.JS = 'var ' + niceName + '___SIZE = ' + TYPES[item.name_].flatSize + '; // ' + item.name_ + '\n';
      if (type.needsFlattening && !type.flatFactor) {
        item.JS += 'var ' + niceName + '___FLATTENER = ' + JSON.stringify(TYPES[item.name_].flatIndexes) + ';';
      }
      return [item];
    }
  });

  function makePointer(slab, pos, allocator, type) { // type is FFU
    if (slab in set('HEAP', 'IHEAP', 'FHEAP')) return pos;
    if (slab[0] != '[') {
      slab = '[' + slab + ']';
    }
    return 'Pointer_make(' + slab + ', ' + (pos ? pos : 0) + (allocator ? ', ' + allocator : '') + ')';
  }

  function makeGetSlab(ptr, type) {
    assert(type);
    if (!USE_TYPED_ARRAYS) {
      return 'HEAP';
    } else {
      if (type in Runtime.FLOAT_TYPES || type === 'int64') {
        return 'FHEAP';
      } else if (type in Runtime.INT_TYPES || isPointerType(type)) {
        return 'IHEAP';
      } else {
        return 'HEAP';
      }
    }
  }

  function makeGetPos(ptr) {
    return ptr;
  }

  function calcFastOffset(ptr, pos, noNeedFirst) {
    var offset = noNeedFirst ? '0' : makeGetPos(ptr);
    return getFastValue(offset, '+', pos);
  }

  function makeGetValue(ptr, pos, noNeedFirst, type) {
    if (isStructType(type)) {
      var typeData = TYPES[type];
      var ret = [];
      for (var i = 0; i < typeData.fields.length; i++) {
        ret.push('f' + i + ': ' + makeGetValue(ptr, pos + typeData.flatIndexes[i], noNeedFirst, typeData.fields[i]));
      }
      return '{ ' + ret.join(', ') + ' }';
    }

    var offset = calcFastOffset(ptr, pos, noNeedFirst);
    if (SAFE_HEAP) {
      return 'SAFE_HEAP_LOAD(' + offset + ', "' + safeQuote(type) + '")';
    } else {
      return makeGetSlab(ptr, type) + '[' + offset + ']';
    }
  }

  function indexizeFunctions(value) { // TODO: Also check for other functions (externals, library, etc.)
    if (value in FUNCTIONS) {
      value = value + '.__index__'; // Store integer value
    }
    return value;
  }

  function makeSetValue(ptr, pos, value, noNeedFirst, type) {
    if (isStructType(type)) {
      var typeData = TYPES[type];
      var ret = [];
      for (var i = 0; i < typeData.fields.length; i++) {
        ret.push(makeSetValue(ptr, pos + typeData.flatIndexes[i], value[i], noNeedFirst, typeData.fields[i]));
      }
      return ret.join('; ');
    }

    value = indexizeFunctions(value);
    var offset = calcFastOffset(ptr, pos, noNeedFirst);
    if (SAFE_HEAP) {
      return 'SAFE_HEAP_STORE(' + offset + ', ' + value + ', "' + safeQuote(type) + '")';
    } else {
      return makeGetSlab(ptr, type) + '[' + offset + '] = ' + value;
    }
  }

  function makeEmptyStruct(type) {
    var ret = [];
    var typeData = TYPES[type];
    assertTrue(typeData);
    for (var i = 0; i < typeData.flatSize; i++) {
      ret.push(0);
    }
    return ret;
  }

  function alignStruct(values, type) {
    var typeData = TYPES[type];
    assertTrue(typeData);
    var ret = [];
    var i = 0, soFar = 0;
    while (i < values.length) {
      // Pad until the right place
      var padded = typeData.flatFactor ? typeData.flatFactor*i : typeData.flatIndexes[i];
      while (soFar < padded) {
        ret.push(0);
        soFar++;
      }
      // Add current value(s)
      var currValue = flatten(values[i]);
      ret.push(currValue);
      i += 1;
      soFar += typeof currValue === 'object' ? currValue.length : 1;
    }
    while (soFar < typeData.flatSize) {
      ret.push(0);
      soFar++;
    }
    return ret;
  }

  // Gets an entire constant expression
  function makeConst(value, type, ident) {
    //dprint('gconst', '//yyyyy ' + JSON.stringify(value) + ',' + type + '\n');
    if (value.intertype) {
      return [finalizeLLVMFunctionCall(value)];
    } else if (Runtime.isNumberType(type) || pointingLevels(type) >= 1) {
      return indexizeFunctions(parseNumerical(toNiceIdent(value.text)));
    } else if (value.text in set('zeroinitializer', 'undef')) { // undef doesn't really need initting, but why not
      return makeEmptyStruct(type);
    } else if (value.text && value.text[0] == '"') {
      value.text = value.text.substr(1, value.text.length-2);
      return JSON.stringify(parseLLVMString(value.text)) + ' /* ' + value.text + '*/';
    } else {
      // Gets an array of constant items, separated by ',' tokens
      function handleSegments(tokens) {
        //dprint('gconst', '// segggS: ' + JSON.stringify(tokens) + '\n' + '\n')
        // Handle a single segment (after comma separation)
        function handleSegment(segment) {
          //dprint('// seggg: ' + JSON.stringify(segment) + '\n' + '\n')
          if (segment[1].text == 'null') {
            return '0';
          } else if (segment[1].text == 'zeroinitializer') {
            return makeEmptyStruct(segment[0].text);
          } else if (segment[1].text in set('bitcast', 'inttoptr', 'ptrtoint')) { // TODO: Use parse/finalizeLLVMFunctionCall
            var type = segment[2].item.tokens.slice(-1)[0].text; // TODO: Use this?
            return handleSegment(segment[2].item.tokens.slice(0, -2));
          } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
            return finalizeLLVMFunctionCall(parseLLVMFunctionCall(segment));
          } else if (segment[1].text == 'add') {
            var subSegments = splitTokenList(segment[2].item.tokens);
            return '(' + handleSegment(subSegments[0]) + ' + ' + handleSegment(subSegments[1]) + ')';
          } else if (segment[1].type == '{') {
            // struct
            var type = segment[0].text;
            return alignStruct(handleSegments(segment[1].tokens), type);
          } else if (segment[1].type == '[') {
            var type = segment[0].text;
            return alignStruct(handleSegments(segment[1].item.tokens), type);
          } else if (segment.length == 2) {
            return toNiceIdent(segment[1].text);
          } else if (segment[1].text === 'c') {
            // string
            var text = segment[2].text;
            text = text.substr(1, text.length-2);
            return parseLLVMString(text); // + ' /* ' + text + '*/';
          } else {
            throw 'Invalid segment: ' + dump(segment);
          }
        };
        return splitTokenList(tokens).map(handleSegment).map(indexizeFunctions);
      }
      var contents;
      if (value.item) {
        // list of items
        contents = value.item.tokens;
      } else if (value.type == '{') {
        // struct
        contents = value.tokens;
      } else if (value[0]) {
        contents = value[0];
      } else {
        throw '// failzzzzzzzzzzzzzz ' + dump(value.item) + ' ::: ' + dump(value);
      }
      return alignStruct(handleSegments(contents), type);
    }
  }

  function parseConst(value, type, ident) {
    var constant = makeConst(value, type);
    if (typeof constant === 'object') {
      constant = flatten(constant).map(function(x) { return parseNumerical(x) })
    }
    return constant;
  }

  // globalVariable
  substrate.addActor('GlobalVariable', {
    processItem: function(item) {
      item.intertype = 'GlobalVariableStub';
      var ret = [item];
      if (item.ident == '_llvm_global_ctors') {
        item.JS = '\n__globalConstructor__ = function() {\n' +
                    item.ctors.map(function(ctor) { return '  ' + toNiceIdent(ctor) + '();' }).join('\n') +
                  '\n}\n';
        return ret;
      } else {
        item.JS = 'var ' + item.ident + ';';
        var constant = item.external ?
          makePointer(JSON.stringify(makeEmptyStruct(item.type)) + ' /* external value? */', null, 'ALLOC_STATIC')
          :
          parseConst(item.value, item.type, item.ident);
        if (typeof constant === 'object') {
          // This is a flattened object. We need to find its idents, so they can be assigned to later
          constant.forEach(function(value, i) {
            if (value[0] in set('_', '(')) { // ident, or expression containing an ident
              ret.push({
                intertype: 'GlobalVariablePostSet',
                JS: 'IHEAP[' + item.ident + '+' + i + '] = ' + value + ';',
              });
              constant[i] = '0';
            }
          });
          constant = '[' + constant.join(', ') + ']';
        }
        constant = makePointer(constant, null, 'ALLOC_STATIC', item.type);
        return ret.concat({
          intertype: 'GlobalVariable',
          JS: item.ident + ' = ' + constant + ';',
        });
      }
    }
  });

  // functionStub
  substrate.addActor('FunctionStub', {
    processItem: function(item) {
      var shortident = item.ident.substr(1);
      if (shortident in Library) {
        function addFromLibrary(ident) {
          var me = arguments.callee;
          if (!me.added) me.added = {};
          if (ident in me.added) return '';
          me.added[ident] = true;
          var snippet = Library[ident];
          if (typeof snippet === 'string') {
            if (Library[snippet]) {
              snippet = Library[snippet]; // redirection for aliases
            }
          }
          var deps = Library[ident + '__deps'];
          return '_' + ident + ' = ' + snippet.toString() + (deps ? '\n' + deps.map(addFromLibrary).join('\n') : '');
        }
        item.JS = addFromLibrary(shortident);
      } else {
        item.JS = '// stub for ' + item.ident;
      }
      return [item];
    }
  });

  // function splitter
  substrate.addActor('FunctionSplitter', {
    processItem: function(item) {
      var ret = [item];
      item.splitItems = 0;
      item.labels.forEach(function(label) {
        label.lines.forEach(function(line) {
          line.func = item.ident;
          line.funcData = item;
          line.parentLabel = label.ident;
          ret.push(line);
          item.splitItems ++;
        });
      });

      this.forwardItems(ret, 'FuncLineTriager');
    }
  });

  // function reconstructor & post-JS optimizer
  substrate.addActor('FunctionReconstructor', {
    funcs: {},
    seen: {},
    processItem: function(item) {
      if (this.seen[item.__uid__]) return;
      if (item.intertype == 'function') {
        this.funcs[item.ident] = item;
        item.relines = {};
        this.seen[item.__uid__] = true;
        return;
      }
      var line = item;
      var func = this.funcs[line.func];
      if (!func) return;

      // Re-insert our line
      this.seen[item.__uid__] = true;
      var label = func.labels.filter(function(label) { return label.ident == line.parentLabel })[0];
      label.lines = label.lines.map(function(line2) {
        return (line2.lineNum !== line.lineNum) ? line2 : line;
      });
      func.splitItems --;
      // OLD    delete line.funcData; // clean up
      if (func.splitItems > 0) return;

      // We have this function all reconstructed, go and finalize it's JS!

      func.JS = '\nfunction ' + func.ident + '(' + func.paramIdents.join(', ') + ') {\n';

      func.JS += '  ' + RuntimeGenerator.stackEnter(func.initialStack) + ';\n';

      if (LABEL_DEBUG) func.JS += "  print(INDENT + ' Entering: " + func.ident + "'); INDENT += '  ';\n";

      if (true) { // TODO: optimize away when not needed
        func.JS += '  var __label__;\n';
      }
      if (func.hasPhi) {
        func.JS += '  var __lastLabel__ = null;\n';
      }

      // Walk function blocks and generate JS
      function walkBlock(block, indent) {
        if (!block) return '';
        dprint('relooping', 'walking block: ' + block.type + ',' + block.entries + ' : ' + block.labels.length);
        function getLabelLines(label, indent) {
          if (!label) return '';
          var ret = '';
          if (LABEL_DEBUG) {
            ret += indent + "print(INDENT + '" + func.ident + ":" + label.ident + "');\n";
          }
          if (EXECUTION_TIMEOUT > 0) {
            ret += indent + 'if (Date.now() - START_TIME >= ' + (EXECUTION_TIMEOUT*1000) + ') throw "Timed out!" + (new Error().stack);\n';
          }
          // for special labels we care about (for phi), mark that we visited them
          return ret + label.lines.map(function(line) { return line.JS + (line.comment ? ' // ' + line.comment : '') })
                                  .join('\n')
                                  .split('\n') // some lines include line breaks
                                  .map(function(line) { return indent + line })
                                  .join('\n');
        }
        var ret = '';
        if (block.type == 'emulated') {
          if (block.labels.length > 1) {
            if (block.entries.length == 1) {
              ret += indent + '__label__ = ' + getLabelId(block.entries[0]) + '; /* ' + block.entries[0] + ' */\n';
            } // otherwise, should have been set before!
            ret += indent + 'while(1) switch(__label__) {\n';
            ret += block.labels.map(function(label) {
              return indent + '  case ' + getLabelId(label.ident) + ': // ' + label.ident + '\n'
                            + getLabelLines(label, indent + '    ');
            }).join('\n');
            ret += '\n' + indent + '  default: assert(0, "bad label: " + __label__);\n' + indent + '}';
          } else {
            ret += getLabelLines(block.labels[0], indent);
          }
          ret += '\n';
        } else if (block.type == 'reloop') {
          ret += indent + block.id + ': while(1) { // ' + block.entries + '\n';
          ret += walkBlock(block.inner, indent + '  ');
          ret += indent + '}\n';
        } else if (block.type == 'multiple') {
          var first = true;
          var multipleIdent = '';
          if (!block.loopless) {
            ret += indent + block.id + ':' + ' do { \n';
            multipleIdent = '  ';
          }
          var stolen = block.stolenCondition;
          if (stolen) {
            var intendedTrueLabel = stolen.labelTrue;
            assert(block.entryLabels.length <= 2);
            [stolen.labelTrue, stolen.labelFalse].forEach(function(entry) {
              var branch = makeBranch(entry, stolen.currLabelId);
              entryLabel = block.entryLabels.filter(function(possible) { return possible.ident === getActualLabelId(entry) })[0];
              if (branch.length < 5 && !entryLabel) return;
              //ret += indent + multipleIdent + (first ? '' : 'else ') +
              //       'if (' + (entry == intendedTrueLabel ? '' : '!') + stolen.ident + ')' + ' {\n';
              ret += indent + multipleIdent + (first ? 'if (' + (entry == intendedTrueLabel ? '' : '!') + stolen.ident + ')' : 'else') + ' {\n';
              ret += indent + multipleIdent + '  ' + branch + '\n';
              if (entryLabel) {
                ret += walkBlock(entryLabel.block, indent + '  ' + multipleIdent);
              }
              ret += indent + multipleIdent + '}\n';
              first = false;
            });
          } else {
            block.entryLabels.forEach(function(entryLabel) {
              ret += indent + multipleIdent + (first ? '' : 'else ') + 'if (__label__ == ' + getLabelId(entryLabel.ident) + ') {\n';
              ret += walkBlock(entryLabel.block, indent + '  ' + multipleIdent);
              ret += indent + multipleIdent + '}\n';
              first = false;
            });
          }
          if (GUARD_LABELS) {
            ret += indent + multipleIdent + 'else { throw "Bad multiple branching: " + __label__ + " : " + (new Error().stack); }\n';
          }
          if (!block.loopless) {
            ret += indent + '} while(0);\n';
          }
        } else {
          throw "Walked into an invalid block type: " + block.type;
        }
        return ret + walkBlock(block.next, indent);
      }
      func.JS += walkBlock(func.block, '  ');
      // Finalize function
      if (LABEL_DEBUG) func.JS += "  INDENT = INDENT.substr(0, INDENT.length-2);\n";
      func.JS += '}\n';
      func.JS += func.ident + '.__index__ = Runtime.getFunctionIndex(' + func.ident + ', "' + func.ident + '");\n';
      return func;
    }
  });

  function getVarData(funcData, ident) {
    return funcData.variables[ident] || GLOBAL_VARIABLES[ident];
  }

  function getVarImpl(funcData, ident) {
    var data = getVarData(funcData, ident);
    assert(data, 'What variable is this? |' + ident + '|');
    return data.impl;
  }

  substrate.addActor('FuncLineTriager', {
    processItem: function(item) {
      if (item.intertype == 'function') {
        this.forwardItem(item, 'FunctionReconstructor');
      } else if (item.JS) {
        if (item.parentLineNum) {
          this.forwardItem(item, 'AssignReintegrator');
        } else {
          this.forwardItem(item, 'FunctionReconstructor');
        }
      } else {
        this.forwardItem(item, 'Intertype:' + item.intertype);
      }
    }
  });

  // assignment
  substrate.addActor('Intertype:assign', {
    processItem: function(item) {
      var pair = splitItem(item, 'value', ['funcData']);
      this.forwardItem(pair.parent, 'AssignReintegrator');
      this.forwardItem(pair.child, 'FuncLineTriager');
    }
  });
  substrate.addActor('AssignReintegrator', makeReintegrator(function(item, child) {
    // 'var', since this is SSA - first assignment is the only assignment, and where it is defined
    item.JS = (item.overrideSSA ? '' : 'var ') + toNiceIdent(item.ident);

    var type = item.value.type;
    var value = parseNumerical(item.value.JS);
    var impl = getVarImpl(item.funcData, item.ident);
    switch (impl) {
      case VAR_NATIVE: {
        break;
      }
      case VAR_NATIVIZED: {
        // SSA, so this must be the alloca. No need for a value
        if (!item.overrideSSA) value = '';
        break;
      }
      case VAR_EMULATED: {
        break;
      }
      default: throw 'zz unknown impl: ' + impl;
    }
    if (value)
      item.JS += ' = ' + value;
    item.JS += ';';

    this.forwardItem(item, 'FunctionReconstructor');
  }));

  // Function lines
  function makeFuncLineActor(intertype, func) {
    return substrate.addActor('Intertype:' + intertype, {
      processItem: function(item) {
        item.JS = func(item);
        if (!item.JS) throw "No JS generated for " + dump(item);
        this.forwardItem(item, 'FuncLineTriager');
      }
    });
  }
  makeFuncLineActor('store', function(item) {
    var value = indexizeFunctions(finalizeLLVMParameter(item.value));
    if (pointingLevels(item.pointerType) == 1) {
      value = parseNumerical(value, removePointing(item.pointerType));
    }
    var impl = VAR_EMULATED;
    if (item.pointer.intertype == 'value') {
      impl = getVarImpl(item.funcData, item.ident);
    }
    switch (impl) {
      case VAR_NATIVIZED:
        return item.ident + ' = ' + value + ';'; // We have the actual value here
        break;
      case VAR_EMULATED:
        if (item.pointer.intertype == 'value') {
          return makeSetValue(item.ident, 0, value, null, item.valueType) + ';';
        } else {
          return makeSetValue(0, indexizeFunctions(finalizeLLVMParameter(item.pointer)), value, null, item.valueType) + ';';
        }
        break;
      default:
        throw 'unknown [store] impl: ' + impl;
    }
  });

  makeFuncLineActor('deleted', function(item) { return ';' });

  var LABEL_IDs = {};
  var LABEL_ID_COUNTER = 0;
  function getLabelId(label) {
    label = label.substr(1);
    if (label === 'entry') return '-1';
    if (label === parseInt(label)) return label; // clang
    //return '"' + label + '"'; // debugging
    label = toNiceIdent(label);
    if (label in LABEL_IDs) return LABEL_IDs[label];
    return LABEL_IDs[label] = LABEL_ID_COUNTER ++;
  }

  function makeBranch(label, lastLabel) {
    var pre = '';
    if (lastLabel) {
      pre = '__lastLabel__ = ' + getLabelId(lastLabel) + '; ';
    }
    if (label[0] == 'B') {
      var parts = label.split('|');
      var trueLabel = parts[1];
      var oldLabel = parts[2];
      var labelSetting = '__label__ = ' + getLabelId(oldLabel) + '; /* ' + cleanLabel(oldLabel) + ' */ '; // TODO: optimize away
      if (label[1] == 'R') {
        return pre + labelSetting + 'break ' + trueLabel + ';';
      } else if (label[1] == 'C') { // CONT
        return pre + labelSetting + 'continue ' + trueLabel + ';';
      } else if (label[1] == 'N') { // NOPP
        return pre + ';'; // Returning no text might confuse this parser
      } else if (label[1] == 'J') { // JSET
        return pre + labelSetting;
      } else {
        throw 'Invalid B-op in branch: ' + trueLabel + ',' + oldLabel;
      }
    } else {
      return pre + '__label__ = ' + getLabelId(label) + '; /* ' + cleanLabel(label) + ' */ break;';
    }
  }

  makeFuncLineActor('branch', function(item) {
    if (item.stolen) return ';'; // We will appear where we were stolen to
    if (!item.ident) {
      return makeBranch(item.label, item.currLabelId);
    } else {
      var labelTrue = makeBranch(item.labelTrue, item.currLabelId);
      var labelFalse = makeBranch(item.labelFalse, item.currLabelId);
      if (labelTrue == ';' && labelFalse == ';') return ';';
      var head = 'if (' + item.ident + ') { ';
      var head2 = 'if (!(' + item.ident + ')) { ';
      var else_ = ' } else { ';
      var tail = ' }';
      if (labelTrue == ';') {
        return head2 + labelFalse + tail;
      } else if (labelFalse == ';') {
        return head + labelTrue + tail;
      } else {
        return head + labelTrue + else_ + labelFalse + tail;
      }
    }
  });
  makeFuncLineActor('switch', function(item) {
    var ret = '';
    var first = true;
    item.switchLabels.forEach(function(switchLabel) {
      if (!first) {
        ret += 'else ';
      } else {
        first = false;
      }
      ret += 'if (' + item.ident + ' == ' + switchLabel.value + ') {\n';
      ret += '  ' + makeBranch(switchLabel.label, item.currLabelId) + '\n';
      ret += '}\n';
    });
    ret += 'else {\n';
    ret += makeBranch(item.defaultLabel, item.currLabelId) + '\n';
    ret += '}\n';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  });
  makeFuncLineActor('return', function(item) {
    var ret = RuntimeGenerator.stackExit(item.funcData.initialStack) + ';\n';
    if (LABEL_DEBUG) {
      ret += "print(INDENT + 'Exiting: " + item.funcData.ident + "');\n"
          +  "INDENT = INDENT.substr(0, INDENT.length-2);\n";
    }
    ret += 'return';
    if (item.value) {
      ret += ' ' + finalizeLLVMParameter(item.value);
    }
    return ret + ';';
  });
  makeFuncLineActor('invoke', function(item) {
    // Wrapping in a function lets us easily return values if we are
    // in an assignment
    var ret = '(function() { try { __THREW__ = false; return '
            + makeFunctionCall(item.ident, item.params, item.funcData) + ' '
            + '} catch(e) { '
            + 'if (ABORT) throw e; __THREW__ = true; '
            + (EXCEPTION_DEBUG ? 'print("Exception: " + e + " : " + (new Error().stack)); ' : '')
            + '} })(); if (!__THREW__) { ' + makeBranch(item.toLabel, item.currLabelId)
            + ' } else { ' + makeBranch(item.unwindLabel, item.currLabelId) + ' }';
    return ret;
  });
  makeFuncLineActor('load', function(item) {
    var ident = toNiceIdent(item.ident);
    var impl = getVarImpl(item.funcData, item.ident);
    switch (impl) {
      case VAR_NATIVIZED: {
        return ident; // We have the actual value here
      }
      case VAR_EMULATED: return makeGetValue(ident, null, null, item.type);
      default: throw "unknown [load] impl: " + impl;
    }
  });
  makeFuncLineActor('extractvalue', function(item) {
    assert(item.indexes.length == 1); // TODO: use getelementptr parsing stuff, for depth. For now, we assume that LLVM aggregates are flat,
                                      //       and we emulate them using simple JS objects { f1: , f2: , } etc., for speed
    return item.ident + '.f' + item.indexes[0][0].text;
  });
  makeFuncLineActor('alloca', function(item) {
    if (typeof item.allocatedIndex === 'number') {
      if (item.allocatedSize === 0) return ''; // This will not actually be shown - it's nativized
      return getFastValue('__stackBase__', '+', item.allocatedIndex.toString());
    } else {
      return RuntimeGenerator.stackAlloc(getFastValue(calcAllocatedSize(item.allocatedType, TYPES), '*', item.allocatedNum));
    }
  });
  makeFuncLineActor('phi', function(item) {
    var params = item.params;
    function makeOne(i) {
      if (i === params.length-1) {
        return finalizeLLVMParameter(params[i].value);
      }
      return '__lastLabel__ == ' + getLabelId(params[i].label) + ' ? ' + finalizeLLVMParameter(params[i].value) + ' : (' + makeOne(i+1) + ')';
    }
    return makeOne(0);
  });

  function makeUnSign(value, type) {
    if (type in Runtime.INT_TYPES) {
      return 'unSign(' + value + ', ' + type.substr(1) + ')';
    } else {
      return value;
    }
  }

  var mathop = makeFuncLineActor('mathop', function(item) { with(item) {
    for (var i = 1; i <= 4; i++) {
      if (item['param'+i]) {
        item['ident'+i] = indexizeFunctions(finalizeLLVMParameter(item['param'+i]));
      }
    }
    if (GUARD_SIGNS) {
      if (op[0] == 'u' || (variant && variant[0] == 'u')) {
        ident1 = makeUnSign(ident1, type);
        ident2 = makeUnSign(ident2, type);
      }
    }
    function checkOverflow(text) {
      if (!CHECK_OVERFLOWS) return text;
      if (item.type[0] !== 'i') return text;
      var bits = item.type.substr(1);
      return 'CHECK_OVERFLOW(' + text + ', ' + bits + ')';
    }
    switch (op) {
      case 'add': return checkOverflow(ident1 + ' + ' + ident2);
      case 'sub': return checkOverflow(ident1 + ' - ' + ident2);
      case 'sdiv': case 'udiv': return checkOverflow('Math.floor(' + ident1 + ' / ' + ident2 + ')');
      case 'mul': return checkOverflow(ident1 + ' * ' + ident2);
      case 'urem': case 'srem': return 'Math.floor(' + ident1 + ' % ' + ident2 + ')';
      case 'or': return ident1 + ' | ' + ident2; // TODO this forces into a 32-bit int - add overflow-style checks? also other bitops below us
      case 'and': return ident1 + ' & ' + ident2;
      case 'xor': return ident1 + ' ^ ' + ident2;
      case 'shl': return ident1 + ' << ' + ident2;
      case 'ashr': return ident1 + ' >> ' + ident2;
      case 'lshr': return ident1 + ' >>> ' + ident2;
      case 'fadd': return ident1 + ' + ' + ident2;
      case 'fsub': return ident1 + ' - ' + ident2;
      case 'fdiv': return ident1 + ' / ' + ident2;
      case 'fmul': return ident1 + ' * ' + ident2;
      case 'uitofp': case 'sitofp': return ident1;
      case 'fptoui': case 'fptosi': return 'Math.floor(' + ident1 + ')';
      case 'icmp': {
        switch (variant) {
          case 'uge': case 'sge': return ident1 + ' >= ' + ident2;
          case 'ule': case 'sle': return ident1 + ' <= ' + ident2;
          case 'ugt': case 'sgt': return ident1 + ' > ' + ident2;
          case 'ult': case 'slt': return ident1 + ' < ' + ident2;
          case 'ne': case 'une': return ident1 + ' != ' + ident2;
          case 'eq': return ident1 + ' == ' + ident2;
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
      case 'zext': case 'fpext': case 'sext': case 'fptrunc': return ident1;
      case 'trunc': {
        // Unlike extending, which we just 'do' (by doing nothing),
        // truncating can change the number, e.g. by truncating to an i1
        // in order to get the first bit
        assert(ident2[0] == 'i');
        var bitsLeft = ident2.substr(1);
        return '((' + ident1 + ') & ' + (Math.pow(2, bitsLeft)-1) + ')';
      }
      case 'select': return ident1 + ' ? ' + ident2 + ' : ' + ident3;
      case 'ptrtoint': return ident1;
      case 'inttoptr': return ident1;
      default: throw 'Unknown mathcmp op: ' + item.op;
    }
  } });

  // Given two values and an operation, returns the result of that operation.
  // Tries to do as much as possible at compile time.
  function getFastValue(a, op, b) {
    if (isNumber(a) && isNumber(b)) {
      return eval(a + op + b);
    }
    if (op == '*') {
      if (!a) a = 1;
      if (!b) b = 1;
      if (a == 0 || b == 0) {
        return 0;
      } else if (a == 1) {
        return b;
      } else if (b == 1) {
        return a;
      }
    } else if (op in set('+', '-')) {
      if (!a) a = 0;
      if (!b) b = 0;
      if (a == 0) {
        return b;
      } else if (b == 0) {
        return a;
      }
    }
    return a + op + b;
  }

  function getGetElementPtrIndexes(item) {
    var ident = item.ident;
    var type = item.params[0].type; // param 0 == type
    // struct pointer, struct*, and getting a ptr to an element in that struct. Param 1 is which struct, then we have items in that
    // struct, and possibly further substructures, all embedded
    // can also be to 'blocks': [8 x i32]*, not just structs
    type = removePointing(type);
    var indexes = [makeGetPos(ident)];
    var offset = toNiceIdent(item.params[1].ident);
    if (offset != 0) {
      if (isStructType(type)) {
        indexes.push(getFastValue(TYPES[type].flatSize, '*', offset));
      } else {
        indexes.push(getFastValue(getNativeFieldSize(type, true), '*', offset));
      }
    }
    item.params.slice(2, item.params.length).forEach(function(arg) {
      dprint('types', 'GEP type: ' + type);
      var curr = toNiceIdent(arg.ident);
      // TODO: If index is constant, optimize
      var typeData = TYPES[type];
      if (isStructType(type) && typeData.needsFlattening) {
        if (typeData.flatFactor) {
          indexes.push(getFastValue(curr, '*', typeData.flatFactor));
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
      type = typeData ? typeData.fields[curr] : '';
    });
    var ret = indexes[0];
    for (var i = 1; i < indexes.length; i++) {
      ret = getFastValue(ret, '+', indexes[i]);
    }
    return ret;
  }

  function finalizeLLVMFunctionCall(item) {
    switch(item.intertype) {
      case 'getelementptr': // TODO finalizeLLVMParameter on the ident and the indexes?
        return makePointer(makeGetSlab(item.ident, item.type), getGetElementPtrIndexes(item), null, item.type);
      case 'bitcast':
      case 'inttoptr':
      case 'ptrtoint':
        return finalizeLLVMParameter(item.params[0]);
      case 'icmp': case 'mul': case 'zext': // TODO: Other mathops
        var temp = {
          op: item.intertype,
          variant: item.variant,
          type: item.type
        };
        for (var i = 1; i <= 4; i++) {
          if (item.params[i-1]) {
            temp['param' + i] = finalizeLLVMParameter(item.params[i-1]);
          }
        }
        mathop.processItem(temp);
        return temp.JS;
      default:
        throw 'Invalid function to finalize: ' + dump(item.intertype);
    }
  }

  // From parseLLVMSegment
  function finalizeLLVMParameter(param) {
    if (isNumber(param) || typeof param === 'string') {
      return param;
    } else if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
      return finalizeLLVMFunctionCall(param);
    } else if (param.intertype == 'value') {
      return parseNumerical(param.ident);
    } else if (param.intertype == 'structvalue') {
      return param.values.map(finalizeLLVMParameter);
    } else {
      throw 'invalid llvm parameter: ' + param.intertype;
    }
  }

  makeFuncLineActor('bitcast', function(item) {
    var ident = toNiceIdent(item.ident);
    return ident;
  });

  function makeFunctionCall(ident, params, funcData) {
    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    // Special cases
    if (ident == '_llvm_va_start') {
      // varargs - we received a pointer to the varargs as a final 'extra' parameter
      var data = 'arguments[arguments.callee.length]';
      if (SAFE_HEAP) {
        return 'SAFE_HEAP_STORE(' + params[0].ident + ', ' + data + ', null)';
      } else {
        return 'IHEAP[' + params[0].ident + '] = ' + data;
      }
    } else if (ident == '_llvm_va_end') {
      return ';';
    }

    var func = FUNCTIONS[ident];
    var args = [];
    var varargs = [];

    params.forEach(function(param, i) {
      var val;
      if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
        val = finalizeLLVMFunctionCall(param);
      } else {
        val = toNiceIdent(param.ident);
      }
      if (!func || !func.hasVarArgs || i < func.numParams-1) { // unrecognized functions (like library ones) cannot have varargs
        args.push(val);
      } else {
        varargs.push(val);
        varargs = varargs.concat(zeros(getNativeFieldSize(param.type)-1));
      }
    });

    args = args.map(indexizeFunctions);
    varargs = varargs.map(indexizeFunctions);
    if (varargs.length > 0) {
      varargs = makePointer(varargs, 0, 'ALLOC_STACK');
    }

    if (getVarData(funcData, ident)) {
      ident = 'FUNCTION_TABLE[' + ident + ']';
    }

    return ident + '(' + args.concat(varargs).join(', ') + ')';
  }
  makeFuncLineActor('getelementptr', function(item) { return finalizeLLVMFunctionCall(item) });
  makeFuncLineActor('call', function(item) {
    return makeFunctionCall(item.ident, item.params, item.funcData) + (item.standalone ? ';' : '');
  });

  // Optimzed intertypes

  makeFuncLineActor('fastgetelementptrload', function(item) {
    return 'var ' + item.ident + ' = ' + makeGetValue(parseNumerical(item.value.ident), getGetElementPtrIndexes(item.value), true, item.value.valueType) + ';';
  });
  makeFuncLineActor('fastgetelementptrstore', function(item) {
    return makeSetValue(item.value.ident, getGetElementPtrIndexes(item.value), parseNumerical(item.ident), true, item.type) + ';';
  });

  makeFuncLineActor('unreachable', function(item) { return 'throw "Reached an unreachable! Original .ll line: ' + item.lineNum + '";' });

  // Final combiner

  function finalCombiner(items) {
    var ret = [];
    if (!functionsOnly) {
      ret = ret.concat(items.filter(function(item) { return item.intertype == 'type' }));
      ret = ret.concat(items.filter(function(item) { return item.intertype == 'GlobalVariableStub' }));
      ret.push('\n');
      ret = ret.concat(items.filter(function(item) { return item.intertype == 'functionStub' }));
      ret.push('\n');
    }
    ret = ret.concat(items.filter(function(item) { return item.intertype == 'function' }));
    ret = ret.concat(data.unparsedFunctions);

    ret = ret.map(function(item) { return item.JS }).join('\n');

    if (functionsOnly) return ret;

    var body = preprocess(read('preamble.js').replace('{{RUNTIME}}', getRuntime()) + ret + read('postamble.js'), CONSTANTS);
    var globalVars = items.filter(function(item) { return item.intertype == 'GlobalVariable' }).map(function(item) { return item.JS }).join('\n');
    var globalVarsPostSets = items.filter(function(item) { return item.intertype == 'GlobalVariablePostSet' }).map(function(item) { return item.JS }).join('\n');
    return read('shell.js').replace('{{BODY}}', indentify(body, 2))
                           .replace('{{GLOBAL_VARS}}', indentify(globalVars+'\n\n\n'+globalVarsPostSets, 4));
  }

  // Data

  substrate.addItems(values(data.types).filter(function(type) { return type.lineNum != '?' }), 'Type');
  substrate.addItems(values(data.globalVariables), 'GlobalVariable');
  substrate.addItems(data.functions, 'FunctionSplitter');
  substrate.addItems(data.functionStubs, 'FunctionStub');

  return finalCombiner(substrate.solve());
}

