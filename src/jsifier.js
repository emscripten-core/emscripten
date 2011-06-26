// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Main function
function JSify(data, functionsOnly, givenFunctions, givenGlobalVariables) {
  // Does simple 'macro' substitution, using Django-like syntax,
  // {{{ code }}} will be replaced with |eval(code)|.
  function processMacros(text) {
    return text.replace(/{{{[^}]+}}}/g, function(str) {
      str = str.substr(3, str.length-6);
      return eval(str).toString();
    });
  }

  substrate = new Substrate('JSifyer');

  var GLOBAL_VARIABLES = functionsOnly ? givenGlobalVariables : data.globalVariables;

  Functions.currFunctions = functionsOnly ? givenFunctions : {};
  // Now that analysis has completed, we can get around to handling unparsedFunctions
  (functionsOnly ? data.functions : data.unparsedFunctions.concat(data.functions)).forEach(function(func) {
    // Save just what we need, to save memory - whether there are varargs, and the # of parameters
    Functions.currFunctions[func.ident] = {
      hasVarArgs: func.hasVarArgs,
      numParams: func.params.length
   };
  });

  for (var i = 0; i < data.unparsedFunctions.length; i++) {
    var func = data.unparsedFunctions[i];
    dprint('unparsedFunctions', '====================\n// Processing |' + func.ident + '|, ' + i + '/' + data.unparsedFunctions.length);
    //var t = Date.now();
    func.JS = JSify(analyzer(intertyper(func.lines, true, func.lineNum-1)), true, Functions.currFunctions, GLOBAL_VARIABLES);
    //t = (Date.now()-t)/1000;
    //dprint('unparsedFunctions', 'unparsedFunction took ' + t + ' seconds.');
    delete func.lines; // clean up memory as much as possible
  }

  if (data.unparsedFunctions.length > 0) {
    // We are now doing the final JS generation
    dprint('unparsedFunctions', '== Completed unparsedFunctions ==\n');
    //Debugging.clear(); // Save some memory, before the final heavy lifting
  }

  // Actors

  // type
  substrate.addActor('Type', {
    processItem: function(item) {
      var type = Types.types[item.name_];
      var niceName = toNiceIdent(item.name_);
      // We might export all of Types.types, cleaner that way, but do not want slowdowns in accessing flatteners
      item.JS = 'var ' + niceName + '___SIZE = ' + Types.types[item.name_].flatSize + '; // ' + item.name_ + '\n';
      if (type.needsFlattening && !type.flatFactor) {
        item.JS += 'var ' + niceName + '___FLATTENER = ' + JSON.stringify(Types.types[item.name_].flatIndexes) + ';';
      }
      return [item];
    }
  });

  function makeEmptyStruct(type) {
    var ret = [];
    var typeData = Types.types[type];
    assertTrue(typeData);
    for (var i = 0; i < typeData.flatSize; i++) {
      ret.push(0);
    }
    return ret;
  }

  function alignStruct(values, type) {
    var typeData = Types.types[type];
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
    //dprint('jsifier const: ' + JSON.stringify(value) + ',' + type + '\n');
    if (value.intertype in PARSABLE_LLVM_FUNCTIONS) {
      return [finalizeLLVMFunctionCall(value)];
    } else if (Runtime.isNumberType(type) || pointingLevels(type) >= 1) {
      return indexizeFunctions(parseNumerical(value.value));
    } else if (value.intertype === 'emptystruct') {
      return makeEmptyStruct(type);
    } else if (value.intertype === 'string') {
      return JSON.stringify(parseLLVMString(value.text)) +
             ' /* ' + value.text.substr(0, 20).replace(/\*/g, '_') + ' */'; // make string safe for inclusion in comment
    } else {
      // Gets an array of constant items, separated by ',' tokens
      function handleSegments(tokens) {
        // Handle a single segment (after comma separation)
        function handleSegment(segment) {
          if (segment.intertype === 'value') {
            return segment.value.toString();
          } else if (segment.intertype === 'emptystruct') {
            return makeEmptyStruct(segment.type);
          } else if (segment.intertype in PARSABLE_LLVM_FUNCTIONS) {
            return finalizeLLVMFunctionCall(segment);
          } else if (segment.intertype in set('struct', 'list')) {
            return alignStruct(handleSegments(segment.contents), segment.type);
          } else if (segment.intertype === 'string') {
            return parseLLVMString(segment.text); // + ' /* ' + text + '*/';
          } else {
            throw 'Invalid segment: ' + dump(segment);
          }
        };
        return tokens.map(handleSegment).map(indexizeFunctions);
      }
      return alignStruct(handleSegments(value.contents), type);
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
      delete item.lines; // Save some memory
      var ret = [item];
      if (item.ident == '_llvm_global_ctors') {
        item.JS = '\n__globalConstructor__ = function() {\n' +
                    item.ctors.map(function(ctor) { return '  ' + toNiceIdent(ctor) + '();' }).join('\n') +
                  '\n}\n';
        return ret;
      } else {
        item.JS = 'var ' + item.ident + ';';
        var constant = null;
        if (item.external) {
          return ret;
        } else {
          function needsPostSet(value) {
            return value[0] in set('_', '(') || value.substr(0, 14) === 'CHECK_OVERFLOW';
          }

          constant = parseConst(item.value, item.type, item.ident);
          if (typeof constant === 'string' && constant[0] != '[') {
            constant = [constant]; // A single item. We may need a postset for it.
          }
          if (typeof constant === 'object') {
            // This is a flattened object. We need to find its idents, so they can be assigned to later
            constant.forEach(function(value, i) {
              if (needsPostSet(value)) { // ident, or expression containing an ident
                ret.push({
                  intertype: 'GlobalVariablePostSet',
                  JS: makeSetValue(item.ident, i, value, 'i32', false, true) // ignore=true, since e.g. rtti and statics cause lots of safe_heap errors
                });
                constant[i] = '0';
              }
            });
            constant = '[' + constant.join(', ') + ']';
          }
          constant = makePointer(constant, null, 'ALLOC_STATIC', item.type);

          var js = item.ident + '=' + constant + ';';
          if (item.ident in EXPORTED_GLOBALS) {
            js += '\nModule["' + item.ident + '"] = ' + item.ident + ';';
          }
          return ret.concat({
            intertype: 'GlobalVariable',
            JS: js,
          });
        }
      }
    }
  });

  var moduleFunctions = set(data.unparsedFunctions.map(function(func) { return func.ident }));

  var addedLibraryItems = {};

  // functionStub
  substrate.addActor('FunctionStub', {
    processItem: function(item) {
      var ret = [item];
      var shortident = item.ident.substr(1);
      if (BUILD_AS_SHARED_LIB) {
        // Shared libraries reuse the runtime of their parents.
        item.JS = '';
      } else if (Library.hasOwnProperty(shortident)) {
        function addFromLibrary(ident) {
          if (ident in addedLibraryItems) return '';
          // Don't replace implemented functions with library ones (which can happen when we add dependencies).
          // Note: We don't return the dependencies here. Be careful not to end up where this matters
          if (('_' + ident) in moduleFunctions) return '';

          addedLibraryItems[ident] = true;
          var snippet = Library[ident];
          var redirectedIdent = null;
          var deps = Library[ident + '__deps'] || [];

          if (typeof snippet === 'string') {
            if (Library[snippet]) {
              // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
              // This avoid having duplicate functions with identical content.
              redirectedIdent = snippet;
              deps.push(snippet);
              snippet = '_' + snippet;
            }
          } else if (typeof snippet === 'object') {
            // JSON.stringify removes functions, so we need to make sure they are added
            var funcs = [];
            var hasNonFunc = false;
            for (var x in snippet) {
              if (typeof snippet[x] === 'function') {
                funcs.push(x + ': ' + snippet[x].toString());
              } else {
                hasNonFunc = true;
              }
            }
            snippet = JSON.stringify(snippet);
            if (funcs.length > 0) {
              snippet = snippet.replace(/}$/, (hasNonFunc ? ', ' : '') + funcs.join(', ') + ' }');
            }
          } else if (typeof snippet === 'function') {
            snippet = snippet.toString();
            if (/function ?\(/.exec(snippet)) { // name the function, if not already named
              snippet = snippet.replace('function', 'function _' + ident);
            }
          }

          var postsetId = ident + '__postset';
          var postset = Library[postsetId];
          if (postset && !addedLibraryItems[postsetId]) {
            addedLibraryItems[postsetId] = true;
            ret.push({
              intertype: 'GlobalVariablePostSet',
              JS: postset
            });
          }

          if (redirectedIdent) {
            deps = deps.concat(Library[redirectedIdent + '__deps'] || []);
          }
          // $ident's are special, we do not prefix them with a '_'.
          if (ident[0] === '$') {
            ident = ident.substr(1);
          } else {
            ident = '_' + ident;
          }
          var text = (deps ? '\n' + deps.map(addFromLibrary).join('\n') : '') + 'var ' + ident + '=' + snippet + ';';
          if (ident in EXPORTED_FUNCTIONS) {
            text += '\nModule["' + ident + '"] = ' + ident + ';';
          }
          return text;
        }
        item.JS = addFromLibrary(shortident);
      } else {
        item.JS = '// stub for ' + item.ident;
      }
      return ret;
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
      if (this.seen[item.__uid__]) return null;
      if (item.intertype == 'function') {
        this.funcs[item.ident] = item;
        item.relines = {};
        this.seen[item.__uid__] = true;
        return null;
      }
      var line = item;
      var func = this.funcs[line.func];
      if (!func) return null;

      // Re-insert our line
      this.seen[item.__uid__] = true;
      var label = func.labels.filter(function(label) { return label.ident == line.parentLabel })[0];
      label.lines = label.lines.map(function(line2) {
        return (line2.lineNum !== line.lineNum) ? line2 : line;
      });
      func.splitItems --;
      // OLD    delete line.funcData; // clean up
      if (func.splitItems > 0) return null;

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
          return ret + label.lines.map(function(line) { return line.JS + (Debugging.on ? Debugging.getComment(line.lineNum) : '') })
                                  .join('\n')
                                  .split('\n') // some lines include line breaks
                                  .map(function(line) { return indent + line })
                                  .join('\n');
        }
        var ret = '';
        if (block.type == 'emulated') {
          if (block.labels.length > 1) {
            if (block.entries.length == 1) {
              ret += indent + '__label__ = ' + getLabelId(block.entries[0]) + '; ' + (SHOW_LABELS ? '/* ' + block.entries[0] + ' */' : '') + '\n';
            } // otherwise, should have been set before!
            ret += indent + 'while(1) switch(__label__) {\n';
            ret += block.labels.map(function(label) {
              return indent + '  case ' + getLabelId(label.ident) + ': // ' + label.ident + '\n'
                            + getLabelLines(label, indent + '    ');
            }).join('\n');
            ret += '\n' + indent + '  default: assert(0, "bad label: " + __label__);\n' + indent + '}';
          } else {
            ret += (SHOW_LABELS ? indent + '/* ' + block.entries[0] + ' */' : '') + '\n' + getLabelLines(block.labels[0], indent);
          }
          ret += '\n';
        } else if (block.type == 'reloop') {
          ret += indent + (block.needBlockId ? block.id + ': ' : '') + 'while(1) { ' + (SHOW_LABELS ? ' /* ' + block.entries + + ' */' : '') + '\n';
          ret += walkBlock(block.inner, indent + '  ');
          ret += indent + '}\n';
        } else if (block.type == 'multiple') {
          var first = true;
          var multipleIdent = '';
          if (!block.loopless) {
            ret += indent + (block.needBlockId ? block.id + ': ' : '') + 'do { \n';
            multipleIdent = '  ';
          }
          var stolen = block.stolenCondition;
          if (stolen) {
            var intendedTrueLabel = stolen.labelTrue;
            assert(block.entryLabels.length <= 2);
            [stolen.labelTrue, stolen.labelFalse].forEach(function(entry) {
              var branch = makeBranch(entry, stolen.currLabelId || null);
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
            // TODO: Find out cases where the final if is not needed - where we know we must be in a specific label at that point
            block.entryLabels.forEach(function(entryLabel) {
              ret += indent + multipleIdent + (first ? '' : 'else ') + 'if (__label__ == ' + getLabelId(entryLabel.ident) + ') {\n';
              ret += walkBlock(entryLabel.block, indent + '  ' + multipleIdent);
              ret += indent + multipleIdent + '}\n';
              first = false;
            });
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
      // Add an unneeded return, needed for strict mode to not throw warnings in some cases.
      // If we are not relooping, then switches make it unimportant to have this (and, we lack hasReturn anyhow)
      if (RELOOP && func.lines.length > 0 && func.labels.filter(function(label) { return label.hasReturn }).length > 0) {
        func.JS += '  return' + (func.returnType !== 'void' ? ' null' : '') + ';\n';
      }
      func.JS += '}\n';
      if (func.ident in EXPORTED_FUNCTIONS) {
        func.JS += 'Module["' + func.ident + '"] = ' + func.ident + ';';
      }

      return func;
    }
  });

  function getVarData(funcData, ident) {
    return funcData.variables[ident] || GLOBAL_VARIABLES[ident] || null;
  }

  function getVarImpl(funcData, ident) {
    if (ident === 'null') return VAR_NATIVIZED; // like nativized, in that we have the actual value right here
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
      item.JS += '=' + value;
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
        return item.ident + '=' + value + ';'; // We have the actual value here
        break;
      case VAR_EMULATED:
        if (item.pointer.intertype == 'value') {
          return makeSetValue(item.ident, 0, value, item.valueType);
        } else {
          return makeSetValue(0, indexizeFunctions(finalizeLLVMParameter(item.pointer)), value, item.valueType);
        }
        break;
      default:
        throw 'unknown [store] impl: ' + impl;
    }
    return null;
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
      var trueLabel = parts[1] || '';
      var oldLabel = parts[2] || '';
      var labelSetting = oldLabel ? '__label__ = ' + getLabelId(oldLabel) + ';' +
                         (SHOW_LABELS ? ' /* to: ' + cleanLabel(oldLabel) + ' */' : '') : ''; // TODO: optimize away the setting
      if (label[1] == 'R') {
        return pre + labelSetting + 'break ' + trueLabel + ';';
      } else if (label[1] == 'C') { // CONT
        return pre + labelSetting + 'continue ' + trueLabel + ';';
      } else if (label[1] == 'N') { // NOPP
        return pre + ';'; // Returning no text might confuse this parser
      } else if (label[1] == 'J') { // JSET
        return pre + labelSetting + ';';
      } else {
        throw 'Invalid B-op in branch: ' + trueLabel + ',' + oldLabel;
      }
    } else {
      return pre + '__label__ = ' + getLabelId(label) + ';' + (SHOW_LABELS ? ' /* to: ' + cleanLabel(label) + ' */' : '') + ' break;';
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
      ret += '  ' + makeBranch(switchLabel.label, item.currLabelId || null) + '\n';
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
            + (EXCEPTION_DEBUG ? 'print("Exception: " + e + " : " + e.stack + ", currently at: " + (new Error().stack)); ' : '')
            + 'return null } })(); if (!__THREW__) { ' + makeBranch(item.toLabel, item.currLabelId)
            + ' } else { ' + makeBranch(item.unwindLabel, item.currLabelId) + ' }';
    return ret;
  });
  makeFuncLineActor('load', function(item) {
    var value = finalizeLLVMParameter(item.pointer);
    var impl = item.ident ? getVarImpl(item.funcData, item.ident) : VAR_EMULATED;
    switch (impl) {
      case VAR_NATIVIZED: {
        return value; // We have the actual value here
      }
      case VAR_EMULATED: return makeGetValue(value, null, item.type, 0, item.unsigned);
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
      return RuntimeGenerator.stackAlloc(getFastValue(calcAllocatedSize(item.allocatedType), '*', item.allocatedNum));
    }
  });
  makeFuncLineActor('phi', function(item) {
    var params = item.params;
    function makeOne(i) {
      if (i === params.length-1) {
        return indexizeFunctions(finalizeLLVMParameter(params[i].value));
      }
      return '__lastLabel__ == ' + getLabelId(params[i].label) + ' ? ' + 
                                   indexizeFunctions(finalizeLLVMParameter(params[i].value)) + ' : (' + makeOne(i+1) + ')';
    }
    return makeOne(0);
  });

  makeFuncLineActor('mathop', processMathop);

  makeFuncLineActor('bitcast', function(item) {
    return item.ident;
  });

  function makeFunctionCall(ident, params, funcData) {
    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    // Special cases
    if (ident == '_llvm_va_start') {
      // varargs - we received a pointer to the varargs as a final 'extra' parameter
      var data = 'arguments[' + Framework.currItem.funcData.ident + '.length]';
      return makeSetValue(params[0].ident, 0, data, 'void*');
    } else if (ident == '_llvm_va_end') {
      return ';';
    }

    var func = Functions.currFunctions[ident];
    var args = [];
    var varargs = [];
    var varargsTypes = [];

    params.forEach(function(param, i) {
      var val = finalizeParam(param);
      if (!func || !func.hasVarArgs || i < func.numParams-1) { // unrecognized functions (like library ones) cannot have varargs
        args.push(val);
      } else {
        varargs.push(val);
        varargs = varargs.concat(zeros(getNativeFieldSize(param.type)-1));
        varargsTypes.push(param.type);
        varargsTypes = varargsTypes.concat(zeros(getNativeFieldSize(param.type)-1));
      }
    });

    args = args.map(indexizeFunctions);
    varargs = varargs.map(indexizeFunctions);
    if (func && func.hasVarArgs) {
      if (varargs.length === 0) {
        varargs = [0];
        varargsTypes = ['i32'];
      }
      varargs = makePointer('[' + varargs + ']', 0, 'ALLOC_STACK', varargsTypes);
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

  makeFuncLineActor('unreachable', function(item) { return 'throw "Reached an unreachable! Original .ll line: ' + item.lineNum + '";' });

  // Final combiner

  function finalCombiner(items) {
    dprint('unparsedFunctions', 'Starting finalCombiner');
    var itemsDict = { type: [], GlobalVariableStub: [], functionStub: [], function: [], GlobalVariable: [], GlobalVariablePostSet: [] };
    items.forEach(function(item) {
      item.lines = null;
      var small = { intertype: item.intertype, JS: item.JS }; // Release memory
      itemsDict[small.intertype].push(small);
    });
    items = null;

    var generated = [];
    if (!functionsOnly) {
      generated = generated.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.functionStub);
    }
    generated = generated.concat(itemsDict.function).concat(data.unparsedFunctions);

    if (functionsOnly) return generated.map(function(item) { return item.JS }).join('\n');

    // We are ready to print out the data, but must do so carefully - we are
    // dealing with potentially *huge* strings. Convenient replacements and
    // manipulations may create in-memory copies, and we may OOM.
    //
    // Final shape that we now create:
    //    shell
    //      (body)
    //        preamble
    //          runtime
    //        generated code
    //        postamble
    //          global_vars

    var shellFile = BUILD_AS_SHARED_LIB ? 'shell_sharedlib.js' : 'shell.js';
    var shellParts = read(shellFile).split('{{BODY}}');
    print(shellParts[0]);
      var preFile = BUILD_AS_SHARED_LIB ? 'preamble_sharedlib.js' : 'preamble.js';
      var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime()), CONSTANTS));
      print(pre);
      generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });
      print(Functions.generateIndexing());

      var postFile = BUILD_AS_SHARED_LIB ? 'postamble_sharedlib.js' : 'postamble.js';
      var postParts = processMacros(preprocess(read(postFile), CONSTANTS)).split('{{GLOBAL_VARS}}');
      print(postParts[0]);
        itemsDict.GlobalVariable.forEach(function(item) { print(indentify(item.JS, 4)); });
        itemsDict.GlobalVariablePostSet.forEach(function(item) { print(indentify(item.JS, 4)); });
      print(postParts[1]);
    print(shellParts[1]);
    return null;
  }

  // Data

  substrate.addItems(values(Types.types).filter(function(type) { return type.lineNum != '?' }), 'Type');
  substrate.addItems(values(data.globalVariables), 'GlobalVariable');
  substrate.addItems(data.functions, 'FunctionSplitter');
  substrate.addItems(data.functionStubs, 'FunctionStub');

  return finalCombiner(substrate.solve());
}

