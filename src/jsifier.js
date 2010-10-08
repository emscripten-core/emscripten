// Convert analyzed data to javascript

function JSify(data) {
  substrate = new Substrate('JSifyer');

  var TYPES = data.types;

  // type
  substrate.addZyme('Type', {
    processItem: function(item) {
      var type = TYPES[item.name_];
      if (type.needsFlattening) {
        item.JS = 'var ' + toNiceIdent(item.name_) + '___FLATTENER = ' + JSON.stringify(TYPES[item.name_].flatIndexes) + ';';
      } else {
        item.JS = '// type: ' + item.name_;
      }
      item.__result__ = true;
      return [item];
    },
  });

  function makePointer(slab, pos, allocator) {
    // XXX hardcoded ptr impl
    if (slab == 'HEAP') return pos;
    if (slab[0] != '[') {
      slab = '[' + slab + ']';
    }
    return 'Pointer_make(' + slab + ', ' + (pos ? pos : 0) + (allocator ? ', ' + allocator : '') + ')';
  }

  function makeGetSlab(ptr) {
    // XXX hardcoded ptr impl
    //    return ptr + '.slab';
    return 'HEAP';
  }

  function makeGetPos(ptr) {
    // XXX hardcoded ptr impl
    //    return ptr + '.pos';
    return ptr;
  }

  function makeGetValue(ptr, pos, noNeedFirst) {
    return makeGetSlab(ptr) + '[' + (noNeedFirst ? '0' : makeGetPos(ptr)) + (pos ? ' + ' + pos : '') + ']';
  }

  function makeSetValue(ptr, pos, value, noNeedFirst) {
    if (SAFE_HEAP) {
      return 'SAFE_HEAP_STORE(' + (noNeedFirst ? '0' : makeGetPos(ptr)) + (pos ? ' + ' + pos : '') + ', ' + value + ')';
    } else {
      return makeGetSlab(ptr) + '[' + (noNeedFirst ? '0' : makeGetPos(ptr)) + (pos ? ' + ' + pos : '') + '] = ' + value;
    }
  }

  function makeEmptyStruct(type) {
    dprint('types', '??makeemptystruct?? ' + dump(type));
    // XXX hardcoded ptr impl
    var ret = [];
    var typeData = TYPES[type];
    assertTrue(typeData);
    for (var i = 0; i < typeData.flatSize; i++) {
      ret.push(0);
    }
    return ret;
  }

  function alignStruct(values, type) {
    dprint('types', 'alignStruct: ' + dump(type));
    // XXX hardcoded ptr impl
    var ret = [];
    var typeData = TYPES[type];
    assertTrue(typeData);
    var i = 0;
    while (i < values.length) {
      var currField = typeData.fields[i];
      var currValue = values[i];
      if (isStructType[currField]) {
        var fieldTypeData = TYPES[currField];
        assertTrue(fieldTypeData);
        ret = ret.concat(alignStruct(values.slice(i, fieldTypeData.fields.length), currField));
        i += fieldTypeData.fields.length;
      } else {
        ret.push(currValue);
        // pad to align, unless it's a structure and already aligned
        if (currValue[0] != '[') {
          ret = ret.concat(zeros(getNativeFieldSize(currField)-1));
        }
        i += 1;
      }
    }
    return ret;
  }

  // Gets an entire constant expression
  function parseConst(value, type) {
    dprint('gconst', '//yyyyy ' + JSON.stringify(value) + ',' + type + '\n');
    if (isNumberType(type) || pointingLevels(type) == 1) {
      return makePointer(parseNumerical(value.text), null, 'ALLOC_STATIC');
    } else if (value.text == 'zeroinitializer') {
      return makePointer(JSON.stringify(makeEmptyStruct(type)), null, 'ALLOC_STATIC');
    } else if (value.text && value.text[0] == '"') {
      value.text = value.text.substr(1, value.text.length-2);
      return makePointer(JSON.stringify(parseLLVMString(value.text)) + ' /* ' + value.text + '*/', null, 'ALLOC_STATIC');
    } else {
      // Gets an array of constant items, separated by ',' tokens
      function handleSegments(tokens) {
        // Handle a single segment (after comma separation)
        function handleSegment(segment) {
          dprint('gconst', '// seggg: ' + JSON.stringify(segment) + '\n' + '\n')
          if (segment[1].text == 'null') {
            return '0';
          } else if (segment[1].text == 'zeroinitializer') {
            return JSON.stringify(makeEmptyStruct(segment[0].text));
          } else if (segment[1].text in searchable('bitcast', 'inttoptr', 'ptrtoint')) { // TODO: Use parse/finalizeLLVMFunctionCall
            var type = segment[2].item[0].tokens.slice(-1)[0].text; // TODO: Use this?
            return handleSegment(segment[2].item[0].tokens.slice(0, -2));
          } else if (segment[1].text in PARSABLE_LLVM_FUNCTIONS) {
            return finalizeLLVMFunctionCall(parseLLVMFunctionCall(segment));
          } else if (segment[1].text == 'add') {
            var subSegments = splitTokenList(segment[2].item[0].tokens);
            return '(' + handleSegment(subSegments[0]) + ' + ' + handleSegment(subSegments[1]) + ')';
          } else if (segment[1].type == '{') {
            // struct
            var type = segment[0].text;
            return '[' + alignStruct(handleSegments(segment[1].tokens), type).join(', ') + ']';
          } else if (segment[1].type == '[') {
            var type = segment[0].text;
            return '[' + alignStruct(handleSegments(segment[1].item[0].tokens), type).join(', ') + ']';
          } else if (segment.length == 2) {
            return parseNumerical(toNiceIdent(segment[1].text));
          } else {
            throw 'Invalid segment: ' + dump(segment);
          }
        };
        return splitTokenList(tokens).map(handleSegment).map(parseNumerical);
      }
      if (value.item) {
        // list of items
        return makePointer('[ ' + alignStruct(handleSegments(value.item[0].tokens), type).join(', ') + ' ]', null, 'ALLOC_STATIC');
      } else if (value.type == '{') {
        // struct
        return makePointer('[ ' + alignStruct(handleSegments(value.tokens), type).join(', ') + ' ]', null, 'ALLOC_STATIC');
      } else if (value[0]) {
        return makePointer('[ ' + alignStruct(handleSegments(value[0].tokens), type).join(', ') + ' ]', null, 'ALLOC_STATIC');
      } else {
        throw '// failzzzzzzzzzzzzzz ' + dump(value.item) + ' ::: ' + dump(value);
      }
    }
  }

  // globalVariablw
  substrate.addZyme('GlobalVariable', {
    processItem: function(item) {
      dprint('gconst', '// zz global Cons: ' + dump(item) + ' :: ' + dump(item.value));
      if (item.ident == '_llvm_global_ctors') {
        item.JS = '\n__globalConstructor__ = function() {\n' +
                    item.ctors.map(function(ctor) { return '  ' + toNiceIdent(ctor) + '();' }).join('\n') +
                  '\n}\n';
      } else if (item.type.text == 'external') {
        item.JS = 'var ' + item.ident + ' = ' + '0; /* external value? */';
      } else {
        // GETTER - lazy loading, fixes issues with ordering.
        item.JS = 'this.__defineGetter__("' + item.ident + '", function() { delete ' + item.ident + '; ' + item.ident + ' = ' + parseConst(item.value, item.type.text) + '; return ' + item.ident + ' });';
      }
      item.__result__ = true;
      return [item];
    },
  });

  // functionStub
  substrate.addZyme('FunctionStub', {
    processItem: function(item) {
      var shortident = item.ident.substr(1);
      if (shortident in Library) {
        var snippet = Library[shortident];
        if (typeof snippet === 'string') {
          snippet = Library[snippet]; // redirection for aliases
        }
        item.JS = item.ident + ' = ' + snippet.toString();
      } else {
        item.JS = '// stub for ' + item.ident;
      }
      item.__result__ = true;
      return [item];
    },
  });

  // function splitter
  substrate.addZyme('FunctionSplitter', {
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
    },
  });
  // function reconstructor & post-JS optimizer
  substrate.addZyme('FunctionReconstructor', {
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

      var hasVarArgs = false;
      var params = parseParamTokens(func.params.item[0].tokens).map(function(param) {
        if (param.intertype == 'varargs') {
          hasVarArgs = true;
          return null;
        }
        return toNiceIdent(param.ident);
      }).filter(function(param) { return param != null });;

      func.JS = '\nfunction ' + func.ident + '(' + params.join(', ') + ') {\n';
      func.JS += '  ' + RuntimeGenerator.stackEnter() + '\n';
      if (LABEL_DEBUG) func.JS += "  print(INDENT + ' Entering: " + func.ident + "'); INDENT += '  ';\n";

      if (true) { // TODO: optimize away when not needed
        func.JS += '  var __label__;\n';
      }
      if (hasVarArgs) {
        func.JS += '  __numArgs__ = ' + params.length + ';\n';
      }
      if (func.hasPhi) {
        func.JS += '  __lastLabel__ = null;\n';
      }

      var usedLabels = {}; // We can get a loop and inside it a multiple, which will try to use the same
                           // label for their loops (until we remove loops from multiples! TODO). So, just
                           // do not use the label twice, that will prevent that
      // Walk function blocks and generate JS
      function walkBlock(block, indent) {
        if (!block) return '';
        block.entry = block.entries[0]; // convention: first entry is the representative
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
          if (func.remarkableLabels.indexOf(label.ident) >= 0) {
            ret += indent + '__lastLabel__ = ' + getLabelId(label.ident) + ';\n';
          }
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
              ret += indent + '__label__ = ' + getLabelId(block.entry) + '; /* ' + block.entry + ' */\n';
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
          usedLabels[block.entry] = 1;
          ret += indent + block.entry + ': while(1) { // ' + block.entry + '\n';
          ret += walkBlock(block.inner, indent + '  ');
          ret += indent + '}\n';
          ret += walkBlock(block.outer, indent);
        } else if (block.type == 'breakingif') {
          ret += walkBlock(block.check, indent);
          ret += indent + block.entry + ': do { if (' + block.ifVar + ') {\n';
          ret += walkBlock(block.ifTrue, indent + '  ');
          ret += indent + '} } while(0);\n';
        } else if (block.type == 'if') {
          ret += walkBlock(block.check, indent);
          ret += indent + 'if (' + block.ifVar + ') {\n';
          ret += walkBlock(block.ifTrue, indent + '  ');
          ret += indent + '}\n';
        } else if (block.type == 'multiple') {
          // TODO: Remove the loop here
          var first = true;
          ret += indent + ((block.entry in usedLabels) ? '' : (block.entry+':')) + ' do { \n';
          block.entryLabels.forEach(function(entryLabel) {
            ret += indent + (first ? '' : '  else ') + '  if (__label__ == ' + getLabelId(entryLabel.ident) + ') {\n';
            ret += walkBlock(entryLabel.block, indent + '    ');
            ret += indent + '  }\n';
            first = false;
          });
          if (GUARD_LABELS) {
            ret += indent + '  else { throw "Bad multiple branching: " + __label__ + " : " + (new Error().stack); }\n';
          }
          ret += indent + '} while(0);\n';
        } else {
          throw "Walked into an invalid block type: " + block.type;
        }
        return ret + walkBlock(block.next, indent);
      }
      func.JS += walkBlock(func.block, '  ');
      // Finalize function
      if (LABEL_DEBUG) func.JS += "  INDENT = INDENT.substr(0, INDENT.length-2);\n";
      func.JS += '}\n';
      func.__result__ = true;
      return func;
    },
  });

  function getVarData(funcData, ident) {
    if (funcData.variables[ident]) {
      return funcData.variables[ident].impl;
    } else {
      return 'emulated'; // All global are emulated
    }
  }

  substrate.addZyme('FuncLineTriager', {
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
    },
  });

  // assignment
  substrate.addZyme('Intertype:assign', {
    processItem: function(item) {
      var pair = splitItem(item, 'value', ['funcData']);
      this.forwardItem(pair.parent, 'AssignReintegrator');
      this.forwardItem(pair.child, 'FuncLineTriager');
    },
  });
  substrate.addZyme('AssignReintegrator', makeReintegrator(function(item, child) {
    // 'var', since this is SSA - first assignment is the only assignment, and where it is defined
    item.JS = (item.overrideSSA ? '' : 'var ') + toNiceIdent(item.ident);

    var type = item.value.type.text;
    var value = parseNumerical(item.value.JS);
    //print("zz var: " + item.JS);
    var impl = getVarData(item.funcData, item.ident);
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
      default: print('zz unknown impl: ' + impl);
    }
    if (value)
      item.JS += ' = ' + value;
    item.JS += ';';

    this.forwardItem(item, 'FunctionReconstructor');
  }));

  // Function lines
  function makeFuncLineZyme(intertype, func) {
    substrate.addZyme('Intertype:' + intertype, {
      processItem: function(item) {
        item.JS = func(item);
        if (!item.JS) throw "XXX - no JS generated for " + dump(item);
        this.forwardItem(item, 'FuncLineTriager');
      },
    });
  }
  makeFuncLineZyme('store', function(item) {
    var value = finalizeLLVMParameter(item.value);
    if (pointingLevels(item.pointerType.text) == 1) {
      value = parseNumerical(value, removePointing(item.pointerType.text));
    }
    var impl = VAR_EMULATED;
    if (item.pointer.intertype == 'value') {
      impl = getVarData(item.funcData, item.ident);
    }
    switch (impl) {
      case VAR_NATIVIZED:
        return item.ident + ' = ' + value + ';'; // We have the actual value here
        break;
      case VAR_EMULATED:
        if (item.pointer.intertype == 'value') {
          return makeSetValue(item.ident, 0, value) + ';';
        } else {
          return makeSetValue(0, getGetElementPtrIndexes(item.pointer), value) + ';';
        }
        break;
      default:
        throw 'unknown [store] impl: ' + impl;
    }
  });

  var LABEL_IDs = {};
  var LABEL_ID_COUNTER = 0;
  function getLabelId(label) {
    label = label.substr(1);
    if (label === 'entry') return '-1';
    if (label === parseInt(label)) return label; // clang
    label = toNiceIdent(label);
    if (label in LABEL_IDs) return LABEL_IDs[label];
    return LABEL_IDs[label] = LABEL_ID_COUNTER ++;
  }

  // TODO: remove 'oldLabel', store that inside the label: BXXX|Labeltobreak|Labeltogetto
  function makeBranch(label, oldLabel) {
    if (label[0] == 'B') {
      var labelSetting = '__label__ = ' + getLabelId(oldLabel) + '; /* ' + cleanLabel(oldLabel) + ' */ '; // TODO: optimize away
      var trueLabel = label.substr(5);
      assert(oldLabel);
      if (label[1] == 'R') {
        return labelSetting + 'break ' + trueLabel + ';';
      } else if (label[1] == 'C') { // CONT
        return labelSetting + 'continue ' + trueLabel + ';';
      } else if (label[1] == 'N') { // NOPP
        return ';'; // Returning no text might confuse this parser
      } else if (label[1] == 'J') { // JSET
        return labelSetting;
      } else {
        throw 'Invalid B-op in branch: ' + trueLabel + ',' + oldLabel;
      }
    } else {
      return '__label__ = ' + getLabelId(label) + '; /* ' + cleanLabel(label) + ' */ break;';
    }
  }

  makeFuncLineZyme('branch', function(item) {
    //print('branch: ' + dump(item));
    if (!item.ident) {
      return makeBranch(item.label, item.old_label);
    } else {
      var labelTrue = makeBranch(item.labelTrue, item.old_labelTrue);
      var labelFalse = makeBranch(item.labelFalse, item.old_labelFalse);
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
  makeFuncLineZyme('switch', function(item) {
    var ret = '';
    var first = true;
    item.switchLabels.forEach(function(switchLabel) {
      if (!first) {
        ret += 'else ';
      } else {
        first = false;
      }
      ret += 'if (' + item.ident + ' == ' + switchLabel.value + ') {\n';
      ret += '  ' + makeBranch(switchLabel.label, switchLabel.old_label) + '\n';
      ret += '}\n';
    });
    ret += 'else {\n';
    ret += makeBranch(item.defaultLabel, item.old_defaultLabel) + '\n';
    ret += '}\n';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  });
  makeFuncLineZyme('return', function(item) {
    var ret = RuntimeGenerator.stackExit() + '\n';
    if (LABEL_DEBUG) ret += "INDENT = INDENT.substr(0, INDENT.length-2);\n";
    ret += 'return';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret + ';';
  });
  makeFuncLineZyme('invoke', function(item) {
    // Wrapping in a function lets us easily return values if we are
    // in an assignment
    var ret = '(function() { try { return '
            + makeFunctionCall(item.ident, item.params) + '; '
            + '__THREW__ = false } catch(e) { '
            + '__THREW__ = true; '
            + (EXCEPTION_DEBUG ? 'print("Exception: " + e + " : " + (new Error().stack)); ' : '')
            + '} })(); if (!__THREW__) { ' + makeBranch(item.toLabel, item.old_toLabel) + ' } else { ' + makeBranch(item.unwindLabel, item.old_unwindLabel) + ' }';
    return ret;
  });
  makeFuncLineZyme('load', function(item) {
    //print('// zz LOAD ' + dump(item) + ' :: ' + dump(item.tokens));
    var ident = toNiceIdent(item.ident);
    var impl = getVarData(item.funcData, item.ident);
    switch (impl) {
      case VAR_NATIVIZED: {
        return ident; // We have the actual value here
      }
      case VAR_EMULATED: return makeGetValue(ident);
      default: return "unknown [load] impl: " + impl;
    }
  });
  makeFuncLineZyme('alloca', function(item) {
    dprint('alloca', dump(item));
    if (pointingLevels(item.allocatedType.text) == 0 && isStructType(item.allocatedType.text)) {
      return RuntimeGenerator.stackAlloc(makeEmptyStruct(item.allocatedType.text).length);
    } else {
      return RuntimeGenerator.stackAlloc(1);
    }
  });
  makeFuncLineZyme('phi', function(item) {
    dprint('phi', dump(item));
    return '__lastLabel__ == ' + getLabelId(item.label1) + ' ? ' + toNiceIdent(item.value1) + ' : ' + toNiceIdent(item.value2);
  });

  function makeUnSign(value, type) {
    if (type in INT_TYPES) {
      return 'unSign(' + value + ', ' + type.substr(1) + ')';
    } else {
      return value;
    }
  }

  makeFuncLineZyme('mathop', function(item) { with(item) {
    dprint('mathop', 'mathop: ' + dump(item));
    for (var i = 1; i <= 4; i++) {
      if (item['param'+i]) {
        item['ident'+i] = finalizeLLVMParameter(item['param'+i]);
      }
    }
    if (GUARD_SIGNS) {
      if (op[0] == 'u' || (variant && variant[0] == 'u')) {
        ident1 = makeUnSign(ident1, type.text);
        ident2 = makeUnSign(ident2, type.text);
      }
    }
    switch (op) {
      case 'add': return ident1 + ' + ' + ident2;
      case 'sub': return ident1 + ' - ' + ident2;
      case 'sdiv': case 'udiv': return 'Math.floor(' + ident1 + ' / ' + ident2 + ')';
      case 'mul': return ident1 + ' * ' + ident2;
      case 'urem': case 'srem': return 'Math.floor(' + ident1 + ' % ' + ident2 + ')';
      case 'or': return ident1 + ' | ' + ident2;
      case 'and': return ident1 + ' & ' + ident2;
      case 'xor': return ident1 + ' ^ ' + ident2;
      case 'shl': case 'ashl': case 'lshl': return ident1 + ' << ' + ident2;
      case 'shr': case 'ashr': case 'lshr': return ident1 + ' >> ' + ident2;
      case 'fadd': return ident1 + ' + ' + ident2;
      case 'fsub': return ident1 + ' - ' + ident2;
      case 'fdiv': return ident1 + ' / ' + ident2;
      case 'fmul': return ident1 + ' * ' + ident2;
      case 'uitofp': case 'sitofp': return ident1;
      case 'fptoui': case 'fptosi': return 'Math.floor(' + ident1 + ')';
      case 'icmp': {
        switch (variant) {
          case 'uge': case 'sge': return '0+(' + ident1 + ' >= ' + ident2 + ')';
          case 'ule': case 'sle': return '0+(' + ident1 + ' <= ' + ident2 + ')';
          case 'ugt': case 'sgt': return '0+(' + ident1 + ' > ' + ident2 + ')';
          case 'ult': case 'slt': return '0+(' + ident1 + ' < ' + ident2 + ')';
          case 'ne': case 'une': return '0+(' + ident1 + ' != ' + ident2 + ')';
          case 'eq': return '0+(' + ident1 + ' == ' + ident2 + ')';
          default: throw 'Unknown icmp variant: ' + variant
        }
      }
      case 'fcmp': {
        switch (variant) {
          case 'uge': case 'oge': return '0+(' + ident1 + ' >= ' + ident2 + ')';
          case 'ule': case 'ole': return '0+(' + ident1 + ' <= ' + ident2 + ')';
          case 'ugt': case 'ogt': return '0+(' + ident1 + ' > ' + ident2 + ')';
          case 'ult': case 'olt': return '0+(' + ident1 + ' < ' + ident2 + ')';
          case 'une': case 'one': return '0+(' + ident1 + ' != ' + ident2 + ')';
          case 'ueq': case 'oeq': return '0+(' + ident1 + ' == ' + ident2 + ')';
          default: throw 'Unknown fcmp variant: ' + variant
        }
      }
      case 'zext': case 'fpext': case 'trunc': case 'sext': case 'fptrunc': return ident1;
      case 'select': return '(' + ident1 + ' ? ' + ident2 + ' : ' + ident3 + ')';
      case 'ptrtoint': {
        if (type.text != 'i8*') print('// XXX Warning: Risky ptrtoint operation on line ' + lineNum);
        return ident1;
      }
      case 'inttoptr': {
        print('// XXX Warning: inttoptr operation on line ' + lineNum);
        return ident1;
      }
      default: throw 'Unknown mathcmp op: ' + item.op
    }
  } });
/*
      return [{
        __result__: true,
        intertype: 'mathop',
        op: op,
        variant: variant,
        type: item.tokens[1].text,
        ident: item.tokens[2].text,
        value: item.tokens[4].text,
        lineNum: item.lineNum,
      }];
*/

  function getGetElementPtrIndexes(item) {
    var ident = item.ident;
    var type = item.params[0].type.text; // param 0 == type
    // struct pointer, struct*, and getting a ptr to an element in that struct. Param 1 is which struct, then we have items in that
    // struct, and possibly further substructures, all embedded
    // can also be to 'blocks': [8 x i32]*, not just structs
    type = removePointing(type);
    var indexes = [makeGetPos(ident)];
    var offset = toNiceIdent(item.params[1].ident);
    if (offset != 0) {
      if (isStructType(type)) {
        indexes.push((TYPES[type].flatSize != 1 ? TYPES[type].flatSize + '*' : '') + offset);
      } else {
        indexes.push(getNativeFieldSize(type, true) + '*' + offset);
      }
    }
    item.params.slice(2, item.params.length).forEach(function(arg) {
      var curr = toNiceIdent(arg.ident);
      // TODO: If index is constant, optimize
      var typeData = TYPES[type];
      if (isStructType(type) && typeData.needsFlattening) {
        if (typeData.flatFactor) {
          indexes.push(curr + '*' + typeData.flatFactor);
        } else {
          indexes.push(toNiceIdent(type) + '___FLATTENER[' + curr + ']');
        }
      } else {
        if (curr != 0) {
          indexes.push(curr); // XXX QUANTUM_SIZE?
        }
      }
      type = TYPES[type] ? TYPES[type].fields[curr] : '';
    });
    return indexes.join('+');
  }

  function finalizeLLVMFunctionCall(item) {
    switch(item.intertype) {
      case 'getelementptr':
        return makePointer(makeGetSlab(item.ident), getGetElementPtrIndexes(item));
      case 'bitcast':
      case 'inttoptr':
      case 'ptrtoint':
        return item.ident;
      default:
        throw 'Invalid function to finalize: ' + dump(item);
    }
  }

  // From parseLLVMSegment
  function finalizeLLVMParameter(param) {
    if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
      return finalizeLLVMFunctionCall(param);
    } else {
      return parseNumerical(param.ident);
    }
  }

  makeFuncLineZyme('bitcast', function(item) {
    // XXX Don't we need to copy ptr - i.e. create new ones (at least if uses > just the next line)?
    // XXX hardcoded ptr impl - as ptrs are ints, we don't need to copy
    var ident = toNiceIdent(item.ident);
    return ident;
  });

  function makeFunctionCall(ident, params) {
    // Special cases
    if (ident == '_llvm_va_start') {
      var args = 'Array.prototype.slice.call(arguments, __numArgs__)';
      var data = 'Pointer_make([' + args + '.length].concat(' + args + '), 0)';
      if (SAFE_HEAP) {
        return 'SAFE_HEAP_STORE(' + params[0].ident + ', ' + data + ', 0)';
      } else {
        return 'HEAP[' + params[0].ident + '] = ' + data;
      }
    } else if (ident == '_llvm_va_end') {
      return ';'
    }

    var params = params.map(function(param) {
      if (param.intertype in PARSABLE_LLVM_FUNCTIONS) {
        return finalizeLLVMFunctionCall(param);
      } else {
        return toNiceIdent(param.ident);
      }
    });
    return ident + '(' + params.join(', ') + ')';
  }
  makeFuncLineZyme('getelementptr', function(item) { return finalizeLLVMFunctionCall(item) });
  makeFuncLineZyme('call', function(item) {
    return makeFunctionCall(item.ident, item.params) + (item.standalone ? ';' : '');
  });

  // Optimzed intertypes

  makeFuncLineZyme('fastgetelementptrload', function(item) {
    return 'var ' + item.ident + ' = ' + makeGetValue(parseNumerical(item.value.ident), getGetElementPtrIndexes(item.value), true) + ';';
  });
  makeFuncLineZyme('fastgetelementptrstore', function(item) {
    return makeSetValue(item.value.ident, getGetElementPtrIndexes(item.value), parseNumerical(item.ident), true) + ';';
  });

  makeFuncLineZyme('unreachable', function(item) { return '// unreachable' });

  // Final combiner

  function finalCombiner(items) {
    var ret = items.filter(function(item) { return item.intertype == 'type' });
    ret = ret.concat(items.filter(function(item) { return item.intertype == 'globalVariable' }));
    ret.push('\n');
    ret = ret.concat(items.filter(function(item) { return item.intertype == 'functionStub' }));
    ret.push('\n');
    ret = ret.concat(items.filter(function(item) { return item.intertype == 'function' }));
    return ret.map(function(item) { return item.JS }).join('\n');
  }

  // Data

  substrate.addItems(values(data.types).filter(function(type) { return type.lineNum != '?' }), 'Type');
  substrate.addItems(data.globalVariables, 'GlobalVariable');
  substrate.addItems(data.functions, 'FunctionSplitter');
  substrate.addItems(data.functionStubs, 'FunctionStub');

  var params = { 'QUANTUM_SIZE': QUANTUM_SIZE };
  return preprocess(read('preamble.js') + getRuntime() + finalCombiner(substrate.solve()) + read('postamble.js'), params);
}

