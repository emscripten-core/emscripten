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

  function makePointer(slab, pos) {
    // XXX hardcoded ptr impl
    if (slab == 'HEAP') return pos;
    if (slab[0] != '[') {
      slab = '[' + slab + ']';
    }
    return 'Pointer_make(' + slab + ', ' + (pos ? pos : 0) + ')';
    //    return '{ slab: ' + slab + ', pos: ' + (pos ? pos : 0) + ' }';
    //    return '[' + slab + ', ' + (pos ? pos : 0) + ']';
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
      return makePointer(parseNumerical(value.text));
    } else if (value.text == 'zeroinitializer') {
      return makePointer(JSON.stringify(makeEmptyStruct(type)));
    } else if (value.text && value.text[0] == '"') {
      value.text = value.text.substr(1, value.text.length-2);
      return makePointer(JSON.stringify(parseLLVMString(value.text)) + ' /* ' + value.text + '*/');
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
          } else if (segment[1].text == 'getelementptr') {
            return finalizeGetElementPtr(parseGetElementPtr(segment));
          } else if (segment[1].text in searchable('bitcast', 'inttoptr', 'ptrtoint')) {
            var type = segment[2].item[0].tokens.slice(-1)[0].text; // TODO: Use this?
            return handleSegment(segment[2].item[0].tokens.slice(0, -2));
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
        return makePointer('[ ' + alignStruct(handleSegments(value.item[0].tokens), type).join(', ') + ' ]');
      } else if (value.type == '{') {
        // struct
        return makePointer('[ ' + alignStruct(handleSegments(value.tokens), type).join(', ') + ' ]');
      } else if (value[0]) {
        return makePointer('[ ' + alignStruct(handleSegments(value[0].tokens), type).join(', ') + ' ]');
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
      if (LABEL_DEBUG) func.JS += "  print(INDENT + ' Entering: " + func.ident + "'); INDENT += '  ';\n";

      if (hasVarArgs) {
        func.JS += '  __numArgs__ = ' + params.length + ';\n';
      }
      if (func.hasPhi) {
        func.JS += '  __lastLabel__ = null;\n';
      }

      // Walk function blocks and generate JS
      function walkBlock(block, indent) {
        if (!block) return '';
        function getLabelLines(label, indent) {
          var ret = '';
          if (LABEL_DEBUG) {
            ret += indent + "print(INDENT + '" + func.ident + ":" + label.ident + "');\n";
          }
          if (EXECUTION_TIMEOUT > 0) {
            ret += indent + 'if (Date.now() - START_TIME >= ' + (EXECUTION_TIMEOUT*1000) + ') throw "Timed out!" + (new Error().stack);\n';
          }
          // for special labels we care about (for phi), mark that we visited them
          if (func.remarkableLabels.indexOf(label.ident) >= 0) {
            ret += '      __lastLabel__ = ' + getLabelId(label.ident) + ';\n';
          }
          return ret + label.lines.map(function(line) { return indent + line.JS + (line.comment ? ' // ' + line.comment : '') }).join('\n');
        }
        var ret = '';
        if (block.type == 'emulated' || block.type == 'simple') {
          if (block.labels.length > 1) {
            ret += indent + 'var __label__ = ' + getLabelId(block.entry) + '; /* ' + block.entry + ' */\n';
            ret += indent + 'while(1) switch(__label__) {\n';
            ret += block.labels.map(function(label) {
              return indent + '  case ' + getLabelId(label.ident) + ': // ' + label.ident + '\n'
                            + getLabelLines(label, indent + '    ');
            }).join('\n');
            ret += '\n' + indent + '}';
          } else {
            ret += getLabelLines(block.labels[0], indent);
          }
          ret += '\n';
        } else if (block.type == 'loop') {
//            if (mustGetTo(first.outLabels[0], [first.ident, first.outLabels[1]])) { /* left branch must return here, or go to right branch */ 
          ret += indent + block.entry + ': while(1) {\n';
          ret += walkBlock(block.inc, indent + '  ');
          ret += walkBlock(block.rest, indent + '  ');
          ret += indent + '}\n';
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
        } else {
          ret = 'XXXXXXXXX!';
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
    //print('// zzqqzz ' + dump(item.value) + ' :::: ' + dump(item.pointer) + ' :::: ');
    var ident = toNiceIdent(item.ident);
    var value;
    if (item.value.intertype == 'getelementptr') {
      value = finalizeGetElementPtr(item.value);
    } else {
      value = toNiceIdent(item.value.ident);
    }
    if (pointingLevels(item.pointerType.text) == 1) {
      value = parseNumerical(value, removePointing(item.pointerType.text));
    }
    //print("// zz seek " + ident + ',' + dump(item));
    var impl = getVarData(item.funcData, item.ident);
    var ret;
    switch (impl) {
      case VAR_NATIVIZED: ret = ident + ' = ' + value + ';'; break; // We have the actual value here
      case VAR_EMULATED: ret = makeSetValue(ident, 0, value) + ';'; break;
      default: print('zz unknown [store] impl: ' + impl);
    }
/*
    if (LINEDEBUG && value) {
      ret += '\nprint(INDENT + "' + ident + ' == " + JSON.stringify(' + ident + '));';
      ret += '\nprint(INDENT + "' + ident + ' == " + (' + ident + ' && ' + ident + '.toString ? ' + ident + '.toString() : ' + ident + '));';
    }
*/
    return ret;
  });

  var LABEL_IDs = {};
  var LABEL_ID_COUNTER = 0;
  function getLabelId(label) {
    //print('needs id: ' + label + ' : ' + JSON.stringify(LABEL_IDs));
    label = toNiceIdent(label);
    if (label in LABEL_IDs) return LABEL_IDs[label];
    return LABEL_IDs[label] = LABEL_ID_COUNTER ++;
  }

  function makeBranch(label) {
    if (label[0] == 'B') {
      if (label[1] == 'R') {
        return 'break ' + label.substr(5) + ';';
      } else if (label[1] == 'C') {
        return 'continue ' + label.substr(5) + ';';
      } else { // NOPP
        return ';'; // Returning no text might confuse this parser
      }
    } else {
      return '__label__ = ' + getLabelId(label) + '; break;';
    }
  }

  makeFuncLineZyme('branch', function(item) {
    //print('branch: ' + dump(item));
    if (!item.ident) {
      return makeBranch(item.label);
    } else {
      var labelTrue = makeBranch(item.labelTrue);
      var labelFalse = makeBranch(item.labelFalse);
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
      ret += '  ' + makeBranch(switchLabel.label) + '\n';
      ret += '}\n';
    });
    ret += 'else {\n';
    ret += makeBranch(item.defaultLabel) + '\n';
    ret += '}\n';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  });
  makeFuncLineZyme('return', function(item) {
    var ret = '';
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
            + '} })(); if (!__THREW__) { ' + makeBranch(item.toLabel) + ' } else { ' + makeBranch(item.unwindLabel) + ' }';
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
      // TODO: allocate on a stack, not on the heap (we currently leak all this)
      return makePointer(JSON.stringify(makeEmptyStruct(item.allocatedType.text)));
    } else {
      return makePointer('[0]');
    }
  });
  makeFuncLineZyme('phi', function(item) {
    dprint('phi', dump(item));
    return '__lastLabel__ == ' + getLabelId(item.label1) + ' ? ' + toNiceIdent(item.value1) + ' : ' + toNiceIdent(item.value2);
  });
  makeFuncLineZyme('mathop', function(item) { with(item) {
    dprint('mathop', 'mathop: ' + dump(item));
    ident = parseNumerical(ident);
    ident2 = parseNumerical(ident2);
    switch (item.op) {
      case 'add': return ident + ' + ' + ident2;
      case 'sub': return ident + ' - ' + ident2;
      case 'sdiv': case 'udiv': return 'Math.floor(' + ident + ' / ' + ident2 + ')';
      case 'mul': return ident + ' * ' + ident2;
      case 'urem': case 'srem': return 'Math.floor(' + ident + ' % ' + ident2 + ')';
      case 'or': return ident + ' | ' + ident2;
      case 'and': return ident + ' & ' + ident2;
      case 'xor': return ident + ' ^ ' + ident2;
      case 'shl': case 'ashl': case 'lshl': return ident + ' << ' + ident2;
      case 'shr': case 'ashr': case 'lshr': return ident + ' >> ' + ident2;
      case 'fadd': return ident + ' + ' + ident2;
      case 'fsub': return ident + ' - ' + ident2;
      case 'fdiv': return ident + ' / ' + ident2;
      case 'fmul': return ident + ' * ' + ident2;
      case 'uitofp': case 'sitofp': return ident;
      case 'fptoui': case 'fptosi': return 'Math.floor(' + ident + ')';
      case 'icmp': {
        switch (variant) {
          case 'uge': case 'sge': return '0+(' + ident + ' >= ' + ident2 + ')';
          case 'ule': case 'sle': return '0+(' + ident + ' <= ' + ident2 + ')';
          case 'ugt': case 'sgt': return '0+(' + ident + ' > ' + ident2 + ')';
          case 'ult': case 'slt': return '0+(' + ident + ' < ' + ident2 + ')';
          case 'ne': case 'une': return '0+(' + ident + ' != ' + ident2 + ')';
          case 'eq': return '0+(' + ident + ' == ' + ident2 + ')';
          default: throw 'Unknown icmp variant: ' + variant
        }
      }
      case 'fcmp': {
        switch (variant) {
          case 'uge': case 'oge': return '0+(' + ident + ' >= ' + ident2 + ')';
          case 'ule': case 'ole': return '0+(' + ident + ' <= ' + ident2 + ')';
          case 'ugt': case 'ogt': return '0+(' + ident + ' > ' + ident2 + ')';
          case 'ult': case 'olt': return '0+(' + ident + ' < ' + ident2 + ')';
          case 'une': case 'one': return '0+(' + ident + ' != ' + ident2 + ')';
          case 'ueq': case 'oeq': return '0+(' + ident + ' == ' + ident2 + ')';
          default: throw 'Unknown fcmp variant: ' + variant
        }
      }
      case 'zext': case 'fpext': case 'trunc': case 'sext': case 'fptrunc': return ident;
      case 'select': return '(' + ident + ' ? ' + ident3 + ' : ' + ident4 + ')';
      case 'ptrtoint': {
        if (type.text != 'i8*') print('// XXX Warning: Risky ptrtoint operation on line ' + lineNum);
        return ident;
      }
      case 'inttoptr': {
        print('// XXX Warning: inttoptr operation on line ' + lineNum);
        return ident;
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

  function finalizeGetElementPtr(item) {
    // TODO: statically combine indexes here if consts
    return makePointer(makeGetSlab(item.ident), getGetElementPtrIndexes(item));
  }

  function finalizeBitcast(item) {
    //print('//zz finalizeBC: ' + dump(item));
    return item.ident;
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
      if (param.intertype === 'getelementptr') {
        return finalizeGetElementPtr(param);
      } else if (param.intertype === 'bitcast') {
        return finalizeBitcast(param);
      } else {
        return toNiceIdent(param.ident); //.value;//'??arg ' + param.intertype + '??';
      }
    });
    return ident + '(' + params.join(', ') + ')';
  }
  makeFuncLineZyme('getelementptr', function(item) { return finalizeGetElementPtr(item) });
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
  return preprocess(read('preamble.js'), params) + finalCombiner(substrate.solve()) + preprocess(read('postamble.js'), params);
//  return finalCombiner(substrate.solve());
}

