// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Main function
function JSify(data, functionsOnly, givenFunctions, givenGlobalVariables) {
  var mainPass = !functionsOnly;

  // Add additional necessary items for the main pass
  if (mainPass) {
    LibraryManager.load();
    var libFuncsToInclude;
    if (INCLUDE_FULL_LIBRARY) {
      assert(!BUILD_AS_SHARED_LIB, 'Cannot have both INCLUDE_FULL_LIBRARY and BUILD_AS_SHARED_LIB set.')
      libFuncsToInclude = [];
      for (var key in LibraryManager.library) {
        if (!key.match(/__(deps|postset)$/)) {
          libFuncsToInclude.push(key);
        }
      }
    } else {
      libFuncsToInclude = ['memset', 'malloc', 'free'];
    }
    libFuncsToInclude.forEach(function(ident) {
      data.functionStubs.push({
        intertype: 'functionStub',
        ident: '_' + ident
      });
    });

    Functions.implementedFunctions = set(data.unparsedFunctions.map(function(func) { return func.ident }));
  }

  // Does simple 'macro' substitution, using Django-like syntax,
  // {{{ code }}} will be replaced with |eval(code)|.
  function processMacros(text) {
    return text.replace(/{{{[^}]+}}}/g, function(str) {
      str = str.substr(3, str.length-6);
      return eval(str).toString();
    });
  }

  substrate = new Substrate('JSifyer');

  var GLOBAL_VARIABLES = !mainPass ? givenGlobalVariables : data.globalVariables;
  Variables.globals = GLOBAL_VARIABLES;

  Functions.currFunctions = !mainPass ? givenFunctions.currFunctions : {};
  Functions.currExternalFunctions = !mainPass ? givenFunctions.currExternalFunctions : {};

  // Now that first-pass analysis has completed (so we have basic types, etc.), we can get around to handling unparsedFunctions
  (!mainPass ? data.functions : data.unparsedFunctions.concat(data.functions)).forEach(function(func) {
    // Save just what we need, to save memory
    Functions.currFunctions[func.ident] = {
      hasVarArgs: func.hasVarArgs,
      numParams: func.params.length,
      labelIds: func.labelIds // TODO: We need this for globals, but perhaps we can calculate them early and free this
   };
  });

  data.functionStubs.forEach(function(func) {
    // Don't overwrite stubs that have more info.
    if (!Functions.currExternalFunctions.hasOwnProperty(func.ident) ||
        !Functions.currExternalFunctions[func.ident].numParams === undefined) {
      Functions.currExternalFunctions[func.ident] = {
        hasVarArgs: func.hasVarArgs,
        numParams: func.params && func.params.length
      };
    }
  });

  for (var i = 0; i < data.unparsedFunctions.length; i++) {
    var func = data.unparsedFunctions[i];
    dprint('unparsedFunctions', '====================\n// Processing |' + func.ident + '|, ' + i + '/' + data.unparsedFunctions.length);
    //var t = Date.now();
    func.JS = JSify(analyzer(intertyper(func.lines, true, func.lineNum-1)), true, Functions, GLOBAL_VARIABLES);
    //t = (Date.now()-t)/1000;
    //dprint('unparsedFunctions', 'unparsedFunction took ' + t + ' seconds.');
    delete func.lines; // clean up memory as much as possible
  }

  if (data.unparsedFunctions.length > 0) {
    // We are now doing the final JS generation
    dprint('unparsedFunctions', '== Completed unparsedFunctions ==\n');

    // Save some memory, before the final heavy lifting
    //Functions.currFunctions = null;
    //Functions.currExternalFunctions = null;
    //Debugging.clear();
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
      return indexizeFunctions(parseNumerical(value.value), type);
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
          var ret;
          if (segment.intertype === 'value') {
            ret = segment.value.toString();
          } else if (segment.intertype === 'emptystruct') {
            ret = makeEmptyStruct(segment.type);
          } else if (segment.intertype in PARSABLE_LLVM_FUNCTIONS) {
            ret = finalizeLLVMFunctionCall(segment);
          } else if (segment.intertype in set('struct', 'list')) {
            ret = alignStruct(handleSegments(segment.contents), segment.type);
          } else if (segment.intertype === 'string') {
            ret = parseLLVMString(segment.text); // + ' /* ' + text + '*/';
          } else if (segment.intertype === 'blockaddress') {
            ret = finalizeBlockAddress(segment);
          } else {
            throw 'Invalid segment: ' + dump(segment);
          }
          assert(segment.type, 'Missing type for constant segment: ' + dump(segment));
          return indexizeFunctions(ret, segment.type);
        };
        return tokens.map(handleSegment)
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
        if (item.external && BUILD_AS_SHARED_LIB) {
          // External variables in shared libraries should not be declared as
          // they would shadow similarly-named globals in the parent.
          item.JS = '';
        } else {
          item.JS = 'var ' + item.ident + ';';
        }
        var constant = null;
        if (item.external) {
          // Import external global variables from the library if available.
          var shortident = item.ident.slice(1);
          if (LibraryManager.library[shortident] &&
              LibraryManager.library[shortident].length &&
              !BUILD_AS_SHARED_LIB) {
            var val = LibraryManager.library[shortident];
            var padding;
            if (Runtime.isNumberType(item.type) || isPointerType(item.type)) {
              padding = [item.type].concat(zeros(getNativeFieldSize(item.type)));
            } else {
              padding = makeEmptyStruct(item.type);
            }
            var padded = val.concat(padding.slice(val.length));
            var js = item.ident + '=' + makePointer(JSON.stringify(padded), null, 'ALLOC_STATIC', item.type) + ';'
            if (LibraryManager.library[shortident + '__postset']) {
              js += '\n' + LibraryManager.library[shortident + '__postset'];
            }
            ret.push({
              intertype: 'GlobalVariablePostSet',
              JS: js
            });
          }
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
          // NOTE: This is the only place that could potentially create static
          //       allocations in a shared library.
          constant = makePointer(constant, null, BUILD_AS_SHARED_LIB ? 'ALLOC_NORMAL' : 'ALLOC_STATIC', item.type);

          var js = item.ident + '=' + constant + ';';
          // Special case: class vtables. We make sure they are null-terminated, to allow easy runtime operations
          if (item.ident.substr(0, 5) == '__ZTV') {
            js += '\n' + makePointer('[0]', null, BUILD_AS_SHARED_LIB ? 'ALLOC_NORMAL' : 'ALLOC_STATIC', ['void*']) + ';';
          }
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

  // alias
  substrate.addActor('Alias', {
    processItem: function(item) {
      item.intertype = 'GlobalVariableStub';
      var ret = [item];
      item.JS = 'var ' + item.ident + ';';
      // Set the actual value in a postset, since it may be a global variable. We also order by dependencies there
      var value = finalizeLLVMParameter(item.value, true); // do *not* indexize functions here
      ret.push({
        intertype: 'GlobalVariablePostSet',
        ident: item.ident,
        dependencies: set([value]),
        JS: item.ident + ' = ' + value + ';'
      });
      return ret;
    }
  });

  var addedLibraryItems = {};

  // functionStub
  substrate.addActor('FunctionStub', {
    processItem: function(item) {
      var ret = [item];
      if (IGNORED_FUNCTIONS.indexOf(item.ident) >= 0) return null;
      var shortident = item.ident.substr(1);
      if (BUILD_AS_SHARED_LIB) {
        // Shared libraries reuse the runtime of their parents.
        item.JS = '';
      } else if (LibraryManager.library.hasOwnProperty(shortident)) {
        function addFromLibrary(ident) {
          if (ident in addedLibraryItems) return '';
          // Don't replace implemented functions with library ones (which can happen when we add dependencies).
          // Note: We don't return the dependencies here. Be careful not to end up where this matters
          if (('_' + ident) in Functions.implementedFunctions) return '';

          addedLibraryItems[ident] = true;
          var snippet = LibraryManager.library[ident];
          var redirectedIdent = null;
          var deps = LibraryManager.library[ident + '__deps'] || [];
          var isFunction = false;

          if (typeof snippet === 'string') {
            if (LibraryManager.library[snippet]) {
              // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
              // This avoid having duplicate functions with identical content.
              redirectedIdent = snippet;
              deps.push(snippet);
              snippet = '_' + snippet;
            }
          } else if (typeof snippet === 'object') {
            if (snippet === null) {
              snippet = 'null';
            } else {
              var members = [];
              for (var property in snippet) {
                if (typeof snippet[property] === 'function') {
                  members.push(property + ': ' + snippet[property].toString());
                } else {
                  members.push(property + ': ' + JSON.stringify(snippet[property]));
                }
              }
              snippet = '{' + members.join(', ') + ' }';
            }
          } else if (typeof snippet === 'function') {
            isFunction = true;
            snippet = snippet.toString();
            assert(snippet.indexOf('XXX missing C define') == -1,
                   'Trying to include a library function with missing C defines: ' + ident + ' | ' + snippet);

            // name the function; overwrite if it's already named
            snippet = snippet.replace(/function(?:\s+([^(]+))?\s*\(/, 'function _' + ident + '(');
            if (LIBRARY_DEBUG) {
              snippet = snippet.replace('{', '{ print("[library call:' + ident + ']"); ');
            }
          }

          var postsetId = ident + '__postset';
          var postset = LibraryManager.library[postsetId];
          if (postset && !addedLibraryItems[postsetId]) {
            addedLibraryItems[postsetId] = true;
            ret.push({
              intertype: 'GlobalVariablePostSet',
              JS: postset
            });
          }

          if (redirectedIdent) {
            deps = deps.concat(LibraryManager.library[redirectedIdent + '__deps'] || []);
          }
          // $ident's are special, we do not prefix them with a '_'.
          if (ident[0] === '$') {
            ident = ident.substr(1);
          } else {
            ident = '_' + ident;
          }
          var text = (deps ? '\n' + deps.map(addFromLibrary).join('\n') : '');
          text += isFunction ? snippet : 'var ' + ident + '=' + snippet + ';';
          if (ident in EXPORTED_FUNCTIONS) {
            text += '\nModule["' + ident + '"] = ' + ident + ';';
          }
          return text;
        }
        item.JS = addFromLibrary(shortident);
      } else {
        item.JS = 'var ' + item.ident + '; // stub for ' + item.ident;
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
          line.funcData = item; // TODO: remove all these, access it globally
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

      if (IGNORED_FUNCTIONS.indexOf(func.ident) >= 0) return null;

      func.JS = '\nfunction ' + func.ident + '(' + func.paramIdents.join(', ') + ') {\n';

      if (PROFILE) {
        func.JS += '  if (PROFILING) { '
                +      'var __parentProfilingNode__ = PROFILING_NODE; PROFILING_NODE = PROFILING_NODE.children["' + func.ident + '"]; '
                +      'if (!PROFILING_NODE) __parentProfilingNode__.children["' + func.ident + '"] = PROFILING_NODE = { time: 0, children: {}, calls: 0 };'
                +      'PROFILING_NODE.calls++; '
                +      'var __profilingStartTime__ = Date.now() '
                +    '}\n';
      }

      func.JS += '  ' + RuntimeGenerator.stackEnter(func.initialStack) + ';\n';

      if (LABEL_DEBUG) func.JS += "  print(INDENT + ' Entering: " + func.ident + "'); INDENT += '  ';\n";

      if (true) { // TODO: optimize away when not needed
        func.JS += '  var __label__;\n';
      }
      if (func.needsLastLabel) {
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

      if (func.lines.length >= CLOSURE_INLINE_PREVENTION_LINES) {
        func.JS += func.ident + '["X"]=1;';
      }

      return func;
    }
  });

  function getVarData(funcData, ident) {
    return funcData.variables[ident] || GLOBAL_VARIABLES[ident] || null;
  }

  function getVarImpl(funcData, ident) {
    if (ident === 'null' || isNumber(ident)) return VAR_NATIVIZED; // like nativized, in that we have the actual value right here
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
  makeFuncLineActor('var', function(item) { // assigns into phis become simple vars when MICRO_OPTS
    return 'var ' + item.ident + ';';
  });
  makeFuncLineActor('store', function(item) {
    var value = finalizeLLVMParameter(item.value);
    if (pointingLevels(item.pointerType) == 1) {
      value = parseNumerical(value, item.valueType);
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
          return makeSetValue(item.ident, 0, value, item.valueType, 0, 0, item.align) + ';';
        } else {
          return makeSetValue(0, finalizeLLVMParameter(item.pointer), value, item.valueType, 0, 0, item.align) + ';';
        }
        break;
      default:
        throw 'unknown [store] impl: ' + impl;
    }
    return null;
  });

  makeFuncLineActor('deleted', function(item) { return ';' });

  function getLabelId(label) {
    var funcData = Framework.currItem.funcData;
    var labelIds = funcData.labelIds;
    if (labelIds[label] !== undefined) return labelIds[label];
    return labelIds[label] = funcData.labelIdCounter++;
  }

  function makeBranch(label, lastLabel, labelIsVariable) {
    var pre = '';
    if (!MICRO_OPTS && lastLabel) {
      pre = '__lastLabel__ = ' + getLabelId(lastLabel) + '; ';
    }
    if (label[0] == 'B') {
      assert(!labelIsVariable, 'Cannot handle branches to variables with special branching options');
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
      if (!labelIsVariable) label = getLabelId(label);
      return pre + '__label__ = ' + label + ';' + (SHOW_LABELS ? ' /* to: ' + cleanLabel(label) + ' */' : '') + ' break;';
    }
  }

  function calcPhiSets(item) {
    if (!item.phi) return null;
    var phiSets = {};
    item.dependent.params.forEach(function(param) {
      if (!phiSets[param.targetLabel]) phiSets[param.targetLabel] = [];
      phiSets[param.targetLabel].push(param);
      param.valueJS = finalizeLLVMParameter(param.value);
    });
    return phiSets;
  }

  function getPhiSetsForLabel(phiSets, label) {
    if (!phiSets) return '';
    label = getOldLabel(label);
    if (!phiSets[label]) return '';
    var labelSets = phiSets[label];
    // FIXME: Many of the |var |s here are not needed, but without them we get slowdowns with closure compiler. TODO: remove this workaround.
    if (labelSets.length == 1) {
      return 'var ' + labelSets[0].ident + ' = ' + labelSets[0].valueJS + ';';
    }
    // TODO: eliminate unneeded sets (to undefined etc.)
    var deps = {}; // for each ident we will set, which others it depends on
    var valueJSes = {};
    labelSets.forEach(function(labelSet) {
      deps[labelSet.ident] = {};
      valueJSes[labelSet.ident] = labelSet.valueJS;
    });
    labelSets.forEach(function(labelSet) {
      walkInterdata(labelSet.value, function mark(item) {
        if (item.intertype == 'value' && item.ident in deps) {
          deps[labelSet.ident][item.ident] = true;
        }
      });
    });
    var pre = '', post = '', idents;
    mainLoop: while ((idents = keys(deps)).length > 0) {
      function remove(ident) {
        for (var i = 0; i < idents.length; i++) {
          delete deps[idents[i]][ident];
        }
        delete deps[ident];
      }
      for (var i = 0; i < idents.length; i++) {
        if (keys(deps[idents[i]]).length == 0) {
          pre = 'var ' + idents[i] + ' = ' + valueJSes[idents[i]] + ';' + pre;
          remove(idents[i]);
          continue mainLoop;
        }
      }
      // If we got here, we have circular dependencies, and must break at least one.
      pre = 'var ' + idents[0] + '$phi = ' + valueJSes[idents[i]] + ';' + pre;
      post += 'var ' + idents[0] + ' = ' + idents[0] + '$phi;';
      remove(idents[0]);
    }
    return pre + post;

    /* // Safe, unoptimized copying
    var ret = '';
    for (var i = 0; i < labelSets.length-1; i++) {
      ret += 'var ' + labelSets[i].ident + '$phi = ' + labelSets[i].valueJS + ';';
    }
    ret += labelSets[labelSets.length-1].ident + ' = ' + labelSets[labelSets.length-1].valueJS + ';';
    for (var i = 0; i < labelSets.length-1; i++) {
      ret += labelSets[i].ident + ' = ' + labelSets[i].ident + '$phi;';
    }
    return ret;
    */
  }

  makeFuncLineActor('branch', function(item) {
    var phiSets = calcPhiSets(item);
    if (item.stolen) return getPhiSetsForLabel(phiSets, item.label) || ';'; // We will appear where we were stolen to
    if (!item.value) {
      return getPhiSetsForLabel(phiSets, item.label) + makeBranch(item.label, item.currLabelId);
    } else {
      var condition = finalizeLLVMParameter(item.value);
      var labelTrue = getPhiSetsForLabel(phiSets, item.labelTrue) + makeBranch(item.labelTrue, item.currLabelId);
      var labelFalse = getPhiSetsForLabel(phiSets, item.labelFalse) + makeBranch(item.labelFalse, item.currLabelId);
      if (labelTrue == ';' && labelFalse == ';') return ';';
      var head = 'if (' + condition + ') { ';
      var head2 = 'if (!(' + condition + ')) { ';
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
    var phiSets = calcPhiSets(item);
    var ret = '';
    var first = true;
    item.switchLabels.forEach(function(switchLabel) {
      if (!first) {
        ret += 'else ';
      } else {
        first = false;
      }
      ret += 'if (' + item.ident + ' == ' + switchLabel.value + ') {\n';
      ret += '  ' + getPhiSetsForLabel(phiSets, switchLabel.label) + makeBranch(switchLabel.label, item.currLabelId || null) + '\n';
      ret += '}\n';
    });
    if (item.switchLabels.length > 0) ret += 'else {\n';
    ret += getPhiSetsForLabel(phiSets, item.defaultLabel) + makeBranch(item.defaultLabel, item.currLabelId) + '\n';
    if (item.switchLabels.length > 0) ret += '}\n';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  });
  makeFuncLineActor('return', function(item) {
    var ret = RuntimeGenerator.stackExit(item.funcData.initialStack) + ';\n';
    if (PROFILE) {
      ret += 'if (PROFILING) { '
          +    'PROFILING_NODE.time += Date.now() - __profilingStartTime__; '
          +    'PROFILING_NODE = __parentProfilingNode__ '
          +  '}\n';
    }
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
  makeFuncLineActor('resume', function(item) {
    return (EXCEPTION_DEBUG ? 'print("Resuming exception");' : '') + 'throw [0,0];';
  });
  makeFuncLineActor('invoke', function(item) {
    // Wrapping in a function lets us easily return values if we are
    // in an assignment
    var phiSets = calcPhiSets(item);
    var call_ = makeFunctionCall(item.ident, item.params, item.funcData);
    var ret = '(function() { try { __THREW__ = false; return '
            + call_ + ' '
            + '} catch(e) { '
            + 'if (typeof e != "number") throw e; '
            + 'if (ABORT) throw e; __THREW__ = true; '
            + (EXCEPTION_DEBUG ? 'print("Exception: " + e + ", currently at: " + (new Error().stack)); ' : '')
            + 'return null } })(); if (!__THREW__) { ' + getPhiSetsForLabel(phiSets, item.toLabel) + makeBranch(item.toLabel, item.currLabelId)
            + ' } else { ' + getPhiSetsForLabel(phiSets, item.unwindLabel)  + makeBranch(item.unwindLabel, item.currLabelId) + ' }';
    return ret;
  });
  makeFuncLineActor('landingpad', function(item) {
    // Just a stub
    return '{ f0: 0, f1: 0 }';
  });
  makeFuncLineActor('load', function(item) {
    var value = finalizeLLVMParameter(item.pointer);
    var impl = item.ident ? getVarImpl(item.funcData, item.ident) : VAR_EMULATED;
    switch (impl) {
      case VAR_NATIVIZED: {
        return value; // We have the actual value here
      }
      case VAR_EMULATED: return makeGetValue(value, 0, item.type, 0, item.unsigned, 0, item.align);
      default: throw "unknown [load] impl: " + impl;
    }
  });
  makeFuncLineActor('extractvalue', function(item) {
    assert(item.indexes.length == 1); // TODO: use getelementptr parsing stuff, for depth. For now, we assume that LLVM aggregates are flat,
                                      //       and we emulate them using simple JS objects { f1: , f2: , } etc., for speed
    return item.ident + '.f' + item.indexes[0][0].text;
  });
  makeFuncLineActor('insertvalue', function(item) {
    assert(item.indexes.length == 1); // TODO: see extractvalue
    var ret = '(', ident;
    if (item.ident === 'undef') {
      item.ident = 'tempValue';
      ret += item.ident + ' = [' + makeEmptyStruct(item.type) + '], ';
    }
    return ret + item.ident + '.f' + item.indexes[0][0].text + ' = ' + finalizeLLVMParameter(item.value) + ', ' + item.ident + ')';
  });
  makeFuncLineActor('indirectbr', function(item) {
    return makeBranch(finalizeLLVMParameter(item.pointer), item.currLabelId, true);
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
    assert(!MICRO_OPTS);
    function makeOne(i) {
      if (i === params.length-1) {
        return finalizeLLVMParameter(params[i].value);
      }
      return '__lastLabel__ == ' + getLabelId(params[i].label) + ' ? ' + 
                                   finalizeLLVMParameter(params[i].value) + ' : (' + makeOne(i+1) + ')';
    }
    var ret = makeOne(0);
    if (item.postSet) ret += item.postSet;
    return ret;
  });

  makeFuncLineActor('mathop', processMathop);

  makeFuncLineActor('bitcast', function(item) {
    return finalizeLLVMParameter(item.params[0]);
  });

  function makeFunctionCall(ident, params, funcData) {
    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    var shortident = LibraryManager.getRootIdent(ident.slice(1)) || ident.slice(1); // ident may not be in library, if all there is is ident__inline
    var func = Functions.currFunctions[ident] || Functions.currExternalFunctions[ident];
    var args = [];
    var argsTypes = [];
    var varargs = [];
    var varargsTypes = [];
    var ignoreFunctionIndexizing = [];
    var useJSArgs = (shortident + '__jsargs') in LibraryManager.library;

    if (I64_MODE == 1 && ident in LLVM.INTRINSICS_32) {
      // Some LLVM intrinsics use i64 where it is not needed, and would cause much overhead
      params.forEach(function(param) { if (param.type == 'i64') param.type = 'i32' });
    }

    params.forEach(function(param, i) {
      var val = finalizeParam(param);
      if (!func || !func.hasVarArgs || i < func.numParams-1 || useJSArgs) {
        if (param.type == 'i64' && I64_MODE == 1) {
          val = makeCopyI64(val); // Must copy [low, high] i64s, so they don't end up modified in the caller
        }
        args.push(val);
        argsTypes.push(param.type);
      } else {
        if (!(param.type == 'i64' && I64_MODE == 1)) {
          varargs.push(val);
          varargs = varargs.concat(zeros(getNativeFieldSize(param.type)-1));
          varargsTypes.push(param.type);
          varargsTypes = varargsTypes.concat(zeros(getNativeFieldSize(param.type)-1));
        } else {
          // i64 mode 1. Write one i32 with type i64, and one i32 with type i32
          varargs.push(val + '[0]');
          varargs = varargs.concat(zeros(getNativeFieldSize('i32')-1));
          ignoreFunctionIndexizing.push(varargs.length); // We will have a value there, but no type (the type is i64, but we write two i32s)
          varargs.push(val + '[1]');
          varargs = varargs.concat(zeros(getNativeFieldSize('i32')-1));
          varargsTypes.push('i64');
          varargsTypes = varargsTypes.concat(zeros(getNativeFieldSize('i32')-1));
          varargsTypes.push('i32');
          varargsTypes = varargsTypes.concat(zeros(getNativeFieldSize('i32')-1));
        }
      }
    });

    args = args.map(function(arg, i) { return indexizeFunctions(arg, argsTypes[i]) });
    varargs = varargs.map(function(vararg, i) {
      if (ignoreFunctionIndexizing.indexOf(i) >= 0) return vararg;
      return vararg === 0 ? 0 : indexizeFunctions(vararg, varargsTypes[i])
    });

    if (func && func.hasVarArgs && !useJSArgs) {
      if (varargs.length === 0) {
        varargs = [0];
        varargsTypes = ['i32'];
      }
      varargs = makePointer('[' + varargs + ']', 0, 'ALLOC_STACK', varargsTypes);
    }

    args = args.concat(varargs);
    var argsText = args.join(', ');

    // Inline if either we inline whenever we can (and we can), or if there is no noninlined version
    var inline = LibraryManager.library[shortident + '__inline'];
    var nonInlined = shortident in LibraryManager.library;
    if (inline && (INLINE_LIBRARY_FUNCS || !nonInlined)) {
      return inline.apply(null, args); // Warning: inlining does not prevent recalculation of the arguments. They should be simple identifiers
    }

    if (getVarData(funcData, ident)) {
      ident = 'FUNCTION_TABLE[' + ident + ']';
    }

    return ident + '(' + args.join(', ') + ')';
  }
  makeFuncLineActor('getelementptr', function(item) { return finalizeLLVMFunctionCall(item) });
  makeFuncLineActor('call', function(item) {
    if (item.standalone && LibraryManager.isStubFunction(item.ident)) return ';';
    return makeFunctionCall(item.ident, item.params, item.funcData) + (item.standalone ? ';' : '');
  });

  makeFuncLineActor('unreachable', function(item) { return 'throw "Reached an unreachable!"' }); // Original .ll line: ' + item.lineNum + '";' });

  // Final combiner

  function finalCombiner(items) {
    dprint('unparsedFunctions', 'Starting finalCombiner');
    var itemsDict = { type: [], GlobalVariableStub: [], functionStub: [], function: [], GlobalVariable: [], GlobalVariablePostSet: [] };
    items.forEach(function(item) {
      item.lines = null;
      var small = { intertype: item.intertype, JS: item.JS, ident: item.ident, dependencies: item.dependencies }; // Release memory
      itemsDict[small.intertype].push(small);
    });
    items = null;

    var splitPostSets = splitter(itemsDict.GlobalVariablePostSet, function(x) { return x.ident && x.dependencies });
    itemsDict.GlobalVariablePostSet = splitPostSets.leftIn;
    var orderedPostSets = splitPostSets.splitOut;

    var limit = orderedPostSets.length * orderedPostSets.length;
    for (var i = 0; i < orderedPostSets.length; i++) {
      for (var j = i+1; j < orderedPostSets.length; j++) {
        if (orderedPostSets[j].ident in orderedPostSets[i].dependencies) {
          var temp = orderedPostSets[i];
          orderedPostSets[i] = orderedPostSets[j];
          orderedPostSets[j] = temp;
          i--;
          limit--;
          assert(limit > 0, 'Could not sort postsets!');
          break;
        }
      }
    }

    itemsDict.GlobalVariablePostSet = itemsDict.GlobalVariablePostSet.concat(orderedPostSets);

    //

    var generated = [];
    if (mainPass) {
      generated = generated.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.functionStub);
    }
    generated = generated.concat(itemsDict.function).concat(data.unparsedFunctions);

    if (!mainPass) {
      Functions.allIdents = Functions.allIdents.concat(itemsDict.function.map(function(func) {
        return func.ident;
      }).filter(function(func) {
        return IGNORED_FUNCTIONS.indexOf(func.ident) < 0;
      }));
      return generated.map(function(item) { return item.JS }).join('\n');
    }

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
      print('Runtime.QUANTUM_SIZE = ' + QUANTUM_SIZE);
      if (RUNTIME_TYPE_INFO) {
        Types.cleanForRuntime();
        print('Runtime.typeInfo = ' + JSON.stringify(Types.types));
        print('Runtime.structMetadata = ' + JSON.stringify(Types.structMetadata));
      }
      generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });
      var postFile = BUILD_AS_SHARED_LIB ? 'postamble_sharedlib.js' : 'postamble.js';
      var postParts = processMacros(preprocess(read(postFile), CONSTANTS)).split('{{GLOBAL_VARS}}');
      print(postParts[0]);
        itemsDict.GlobalVariable.forEach(function(item) { print(indentify(item.JS, 4)); });
        itemsDict.GlobalVariablePostSet.forEach(function(item) { print(indentify(item.JS, 4)); });
        print(Functions.generateIndexing()); // done last, as it may rely on aliases set in postsets
      print(postParts[1]);
    print(shellParts[1]);
    // Print out some useful metadata (for additional optimizations later, like the eliminator)
    print('// EMSCRIPTEN_GENERATED_FUNCTIONS: ' + JSON.stringify(Functions.allIdents) + '\n');
    return null;
  }

  // Data

  if (mainPass) {
    substrate.addItems(values(Types.types).filter(function(type) { return type.lineNum != '?' }), 'Type');
    substrate.addItems(values(data.globalVariables), 'GlobalVariable');
    substrate.addItems(data.functionStubs, 'FunctionStub');
    substrate.addItems(data.aliass, 'Alias');
    assert(data.functions.length == 0);
  } else {
    substrate.addItems(data.functions, 'FunctionSplitter');
  }

  return finalCombiner(substrate.solve());
}

