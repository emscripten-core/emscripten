//"use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Handy sets

var STRUCT_LIST = set('struct', 'list');
var UNDERSCORE_OPENPARENS = set('_', '(');

// JSifier
function JSify(data, functionsOnly, givenFunctions) {
  var mainPass = !functionsOnly;

  if (mainPass) {
    // We will start to print out the data, but must do so carefully - we are
    // dealing with potentially *huge* strings. Convenient replacements and
    // manipulations may create in-memory copies, and we may OOM.
    //
    // Final shape that will be created:
    //    shell
    //      (body)
    //        preamble
    //          runtime
    //        generated code
    //        postamble
    //          global_vars
    //
    // First, we print out everything until the generated code. Then the
    // functions will print themselves out as they are parsed. Finally, we
    // will call finalCombiner in the main pass, to print out everything
    // else. This lets us not hold any strings in memory, we simply print
    // things out as they are ready.

    var shellFile = BUILD_AS_SHARED_LIB ? 'shell_sharedlib.js' : 'shell.js';
    var shellParts = read(shellFile).split('{{BODY}}');
    print(shellParts[0]);
    var preFile = BUILD_AS_SHARED_LIB ? 'preamble_sharedlib.js' : 'preamble.js';
    var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime())));
    print(pre);

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

  var substrate = new Substrate('JSifyer');

  if (mainPass) {
    // Handle unparsed types TODO: Batch them
    analyzer(intertyper(data.unparsedTypess[0].lines, true), true);
    data.unparsedTypess = null;

    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)
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
      libFuncsToInclude = ['memcpy', 'memset', 'malloc', 'free'];
    }
    libFuncsToInclude.forEach(function(ident) {
      data.functionStubs.push({
        intertype: 'functionStub',
        ident: '_' + ident
      });
    });
  }

  // Functions

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

  var MAX_BATCH_FUNC_LINES = 1000;
  while (data.unparsedFunctions.length > 0) {
    var currFuncLines = [];
    var currBaseLineNums = [];
    while (currFuncLines.length == 0 ||
           (data.unparsedFunctions.length > 0 && currFuncLines.length + data.unparsedFunctions[0].lines.length <= MAX_BATCH_FUNC_LINES)) {
      currBaseLineNums.push([currFuncLines.length, data.unparsedFunctions[0].lineNum-1]);
      currFuncLines = currFuncLines.concat(data.unparsedFunctions[0].lines); // for first one, assign, do not concat?
      data.unparsedFunctions.shift();
    }
    dprint('unparsedFunctions','====================\n// Processing function batch of ' + currBaseLineNums.length +
                               ' functions, ' + currFuncLines.length + ' lines, functions left: ' + data.unparsedFunctions.length);
    if (DEBUG_MEMORY) MemoryDebugger.tick('pre-func');
    JSify(analyzer(intertyper(currFuncLines, true, currBaseLineNums), true), true, Functions);
    if (DEBUG_MEMORY) MemoryDebugger.tick('post-func');
  }
  currFuncLines = currBaseLineNums = null; // Do not hold on to anything from inside that loop (JS function scoping..)
  data.unparsedFunctions = null;

  // Actors

  // type
  // FIXME: This is no longer used, we do not actually need to JSify on types. TODO: Remove this and related code
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
    var ret = new Array(typeData.flatSize);
    var index = 0;
    var i = 0;
    while (i < values.length) {
      // Pad until the right place
      var padded = typeData.flatFactor ? typeData.flatFactor*i : typeData.flatIndexes[i];
      while (index < padded) {
        ret[index++] = 0;
      }
      // Add current value(s)
      var currValue = flatten(values[i]);
      if (I64_MODE == 1 && typeData.fields[i] == 'i64') {
        // 'flatten' out the 64-bit value into two 32-bit halves
        ret[index++] = currValue>>>0;
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = Math.floor(currValue/4294967296);
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
      } else if (typeof currValue == 'object') {
        for (var j = 0; j < currValue.length; j++) {
          ret[index++] = currValue[j];
        }
      } else {
        ret[index++] = currValue;
      }
      i += 1;
    }
    while (index < typeData.flatSize) {
      ret[index++] = 0;
    }
    return ret;
  }

  // Gets an entire constant expression
  function makeConst(value, type, ident) {
    // Gets an array of constant items, separated by ',' tokens
    function handleSegments(tokens) {
      // Handle a single segment (after comma separation)
      function handleSegment(segment) {
        var ret;
        if (segment.intertype === 'value') {
          ret = segment.ident;
        } else if (segment.intertype === 'emptystruct') {
          ret = makeEmptyStruct(segment.type);
        } else if (segment.intertype in PARSABLE_LLVM_FUNCTIONS) {
          ret = finalizeLLVMFunctionCall(segment);
        } else if (segment.intertype in STRUCT_LIST) {
          ret = alignStruct(handleSegments(segment.contents), segment.type);
        } else if (segment.intertype === 'string') {
          ret = parseLLVMString(segment.text); // + ' /* ' + text + '*/';
        } else if (segment.intertype === 'blockaddress') {
          ret = finalizeBlockAddress(segment);
        } else {
          throw 'Invalid segment: ' + dump(segment);
        }
        assert(segment.type, 'Missing type for constant segment!');
        return indexizeFunctions(ret, segment.type);
      };
      return tokens.map(handleSegment)
    }

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
      function needsPostSet(value) {
        return value[0] in UNDERSCORE_OPENPARENS || value.substr(0, 14) === 'CHECK_OVERFLOW'
            || value.substr(0, 13) === 'STRING_TABLE.';
      }

      item.intertype = 'GlobalVariableStub';
      assert(!item.lines); // FIXME remove this, after we are sure it isn't needed
      var ret = [item];
      if (item.ident == '_llvm_global_ctors') {
        item.JS = '\n__ATINIT__ = __ATINIT__.concat([\n' +
                    item.ctors.map(function(ctor) { return '  { func: ' + toNiceIdent(ctor) + ' }' }).join(',\n') +
                  '\n]);\n';
        return ret;
      } else {
        if (item.external && BUILD_AS_SHARED_LIB) {
          // External variables in shared libraries should not be declared as
          // they would shadow similarly-named globals in the parent.
          item.JS = '';
        } else {
          if (!(item.ident in Variables.globals) || !Variables.globals[item.ident].isString) {
          	item.JS = 'var ' + item.ident + ';';
          } 
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
              padding = [item.type].concat(zeros(Runtime.getNativeFieldSize(item.type)));
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
                  JS: makeSetValue(item.ident, i, value, 'i32', false, true) + ';' // ignore=true, since e.g. rtti and statics cause lots of safe_heap errors
                });
                constant[i] = '0';
              }
            });
            constant = '[' + constant.join(', ') + ']';
          }
          // NOTE: This is the only place that could potentially create static
          //       allocations in a shared library.
          constant = makePointer(constant, null, BUILD_AS_SHARED_LIB ? 'ALLOC_NORMAL' : 'ALLOC_STATIC', item.type);

          var js;

          // Strings are held in STRING_TABLE, to not clutter up the main namespace (in some cases we have
          // many many strings, possibly exceeding the js engine limit on global vars).
          if (Variables.globals[item.ident].isString) {
            js = 'STRING_TABLE.' + item.ident + '=' + constant + ';';
          } else {
          	js = item.ident + '=' + constant + ';';
          }

          // Special case: class vtables. We make sure they are null-terminated, to allow easy runtime operations
          if (item.ident.substr(0, 5) == '__ZTV') {
            js += '\n' + makePointer('[0]', null, BUILD_AS_SHARED_LIB ? 'ALLOC_NORMAL' : 'ALLOC_STATIC', ['void*']) + ';';
          }
          if (item.ident in EXPORTED_GLOBALS) {
            js += '\nModule["' + item.ident + '"] = ' + item.ident + ';';
          }
          if (BUILD_AS_SHARED_LIB == 2 && !item.private_) {
            // TODO: make the assert conditional on ASSERTIONS
            js += 'if (globalScope) { assert(!globalScope["' + item.ident + '"]); globalScope["' + item.ident + '"] = ' + item.ident + ' }';
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
      var value = Variables.globals[item.ident].resolvedAlias = finalizeLLVMParameter(item.value);
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
          snippet = stringifyWithFunctions(snippet);
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

      var ret = [item];
      if (IGNORED_FUNCTIONS.indexOf(item.ident) >= 0) return null;
      var shortident = item.ident.substr(1);
      if (BUILD_AS_SHARED_LIB) {
        // Shared libraries reuse the runtime of their parents.
        item.JS = '';
      } else if (LibraryManager.library.hasOwnProperty(shortident)) {
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

      func.JS = '\n';

      var paramIdents = func.params.map(function(param) {
          return (param.intertype == 'varargs') ? null : toNiceIdent(param.ident);
      }).filter(function(param) { return param != null; })

      if (CLOSURE_ANNOTATIONS) {
        func.JS += '/**\n';
        paramIdents.forEach(function(param) {
          func.JS += ' * @param {number} ' + param + '\n';
        });
        func.JS += ' * @return {number}\n'
        func.JS += ' */\n';
      }

      func.JS += 'function ' + func.ident + '(' + paramIdents.join(', ') + ') {\n';

      if (PROFILE) {
        func.JS += '  if (PROFILING) { '
                +      'var __parentProfilingNode__ = PROFILING_NODE; PROFILING_NODE = PROFILING_NODE.children["' + func.ident + '"]; '
                +      'if (!PROFILING_NODE) __parentProfilingNode__.children["' + func.ident + '"] = PROFILING_NODE = { time: 0, children: {}, calls: 0 };'
                +      'PROFILING_NODE.calls++; '
                +      'var __profilingStartTime__ = Date.now() '
                +    '}\n';
      }

      // Prepare the stack, if we need one. If we have other stack allocations, force the stack to be set up.
      func.JS += '  ' + RuntimeGenerator.stackEnter(func.initialStack, func.otherStackAllocations) + ';\n';

      // Make copies of by-value params
      // XXX It is not clear we actually need this. While without this we fail, it does look like
      //     Clang normally does the copy itself, in the calling function. We only need this code
      //     when Clang optimizes the code and passes the original, not the copy, to the other
      //     function. But Clang still copies, the copy is just unused! Need to figure out if that
      //     is caused by our running just some optimizations (the safe ones), or if its a bug
      //     in Clang, or a bug in our understanding of the IR.
      func.params.forEach(function(param) {
        if (param.byVal) {
          var type = removePointing(param.type);
          var typeInfo = Types.types[type];
          func.JS += '  var tempParam = ' + param.ident + '; ' + param.ident + ' = ' + RuntimeGenerator.stackAlloc(typeInfo.flatSize) + ';' +
                     makeCopyValues(param.ident, 'tempParam', typeInfo.flatSize, 'null', null, param.byVal) + ';\n';
        }
      });

      if (LABEL_DEBUG) func.JS += "  print(INDENT + ' Entering: " + func.ident + "'); INDENT += '  ';\n";

      if (true) { // TODO: optimize away when not needed
        if (CLOSURE_ANNOTATIONS) func.JS += '/** @type {number} */';
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
          ret += indent + block.id + ': while(1) { ' + (SHOW_LABELS ? ' /* ' + block.entries + + ' */' : '') + '\n';
          ret += walkBlock(block.inner, indent + '  ');
          ret += indent + '}\n';
        } else if (block.type == 'multiple') {
          var first = true;
          var multipleIdent = '';
          ret += indent + block.id + ': do { \n';
          multipleIdent = '  ';
          // TODO: Find out cases where the final if/case is not needed - where we know we must be in a specific label at that point
          var SWITCH_IN_MULTIPLE = 0; // This appears to never be worth it, for no amount of labels
          if (SWITCH_IN_MULTIPLE && block.entryLabels.length >= 2) {
            ret += indent + multipleIdent + 'switch(__label__) {\n';
            block.entryLabels.forEach(function(entryLabel) {
              ret += indent + multipleIdent + '  case ' + getLabelId(entryLabel.ident) + ': {\n';
              ret += walkBlock(entryLabel.block, indent + '    ' + multipleIdent);
              ret += indent + multipleIdent + '  } break;\n';
            });
            ret += indent + multipleIdent + '}\n';
          } else {
            block.entryLabels.forEach(function(entryLabel) {
              ret += indent + multipleIdent + (first ? '' : 'else ') + 'if (__label__ == ' + getLabelId(entryLabel.ident) + ') {\n';
              ret += walkBlock(entryLabel.block, indent + '  ' + multipleIdent);
              ret += indent + multipleIdent + '}\n';
              first = false;
            });
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
      // Add an unneeded return, needed for strict mode to not throw warnings in some cases.
      // If we are not relooping, then switches make it unimportant to have this (and, we lack hasReturn anyhow)
      if (RELOOP && func.lines.length > 0 && func.labels.filter(function(label) { return label.hasReturn }).length > 0) {
        func.JS += '  return' + (func.returnType !== 'void' ? ' null' : '') + ';\n';
      }
      func.JS += '}\n';
      if (func.ident in EXPORTED_FUNCTIONS) {
        func.JS += 'Module["' + func.ident + '"] = ' + func.ident + ';';
      }

      if (INLINING_LIMIT && func.lines.length >= INLINING_LIMIT) {
        func.JS += func.ident + '["X"]=1;';
      }

      if (BUILD_AS_SHARED_LIB == 2) {
        // TODO: make the assert conditional on ASSERTIONS
        func.JS += 'if (globalScope) { assert(!globalScope["' + func.ident + '"]); globalScope["' + func.ident + '"] = ' + func.ident + ' }';
      }

      return func;
    }
  });

  function getVarData(funcData, ident) {
    var local = funcData.variables[ident];
    if (local) return local;
    var global = Variables.globals[ident];
    return global || null;
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
        this.forwardItem(item, 'FunctionReconstructor'); // XXX not really needed
      } else if (item.JS) {
        this.forwardItem(item, 'FunctionReconstructor'); // XXX not really needed
      } else {
        this.forwardItem(item, 'Intertype:' + item.intertype);
      }
    }
  });

  // An interitem that has |assignTo| is an assign to that item. They call this function which
  // generates the actual assignment.
  function makeAssign(item) {
    var valueJS = item.JS;
    item.JS = '';
    if (CLOSURE_ANNOTATIONS) item.JS += '/** @type {number} */ ';
    item.JS += (item.overrideSSA ? '' : 'var ') + toNiceIdent(item.assignTo);

    var value = parseNumerical(valueJS);
    var impl = getVarImpl(item.funcData, item.assignTo);
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
    if (value && value != ';') {
      item.JS += '=' + value;
    }
    if (item.JS[item.JS.length-1] != ';') {
      item.JS += ';';
    }
  }

  // Function lines
  function makeFuncLineActor(intertype, func) {
    return substrate.addActor('Intertype:' + intertype, {
      processItem: function(item) {
        item.JS = func(item);
        if (!item.JS) throw "No JS generated for " + dump(item);
        if (item.assignTo) {
          makeAssign(item);
          if (!item.JS) throw "No assign JS generated for " + dump(item);
        }
        this.forwardItem(item, 'FunctionReconstructor');
      }
    });
  }
  makeFuncLineActor('value', function(item) {
    return item.ident;
  });
  makeFuncLineActor('noop', function(item) {
    return ';';
  });
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
        if (label[2] == 'N') { // BRNOL: break, no label setting
          labelSetting = '';
        }
        return pre + labelSetting + 'break ' + trueLabel + ';';
      } else if (label[1] == 'C') { // CONT
        if (label[2] == 'N') { // BCNOL: continue, no label setting
          labelSetting = '';
        }
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
        if (item.intertype == 'value' && item.ident in deps && labelSet.ident != item.ident) {
          deps[labelSet.ident][item.ident] = true;
        }
      });
    });
    var pre = '', post = '', idents;
    mainLoop: while ((idents = keys(deps)).length > 0) {
      var remove = function(ident) {
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
      pre = 'var ' + idents[0] + '$phi = ' + valueJSes[idents[0]] + ';' + pre;
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
    // TODO: Find a case where switch is important, and benchmark that. var SWITCH_IN_SWITCH = 1; 
    var phiSets = calcPhiSets(item);
    // Consolidate checks that go to the same label. This is important because it makes the
    // js optimizer hoistMultiples much easier to implement (we hoist into one place, not
    // many).
    var targetLabels = {}; // for each target label, the list of values going to it
    item.switchLabels.forEach(function(switchLabel) {
      if (!targetLabels[switchLabel.label]) {
        targetLabels[switchLabel.label] = [];
      }
      targetLabels[switchLabel.label].push(switchLabel.value);
    });
    var ret = '';
    var first = true;
    for (var targetLabel in targetLabels) {
      if (!first) {
        ret += 'else ';
      } else {
        first = false;
      }
      ret += 'if (' + targetLabels[targetLabel].map(function(value) { return item.ident + ' == ' + value }).join(' || ') + ') {\n';
      ret += '  ' + getPhiSetsForLabel(phiSets, targetLabel) + makeBranch(targetLabel, item.currLabelId || null) + '\n';
      ret += '}\n';
    }
    if (item.switchLabels.length > 0) ret += 'else {\n';
    ret += getPhiSetsForLabel(phiSets, item.defaultLabel) + makeBranch(item.defaultLabel, item.currLabelId) + '\n';
    if (item.switchLabels.length > 0) ret += '}\n';
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  });
  makeFuncLineActor('return', function(item) {
    var ret = RuntimeGenerator.stackExit(item.funcData.initialStack, item.funcData.otherStackAllocations) + ';\n';
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
    return (EXCEPTION_DEBUG ? 'print("Resuming exception");' : '') + 
    	'throw ' + makeGetValue('_llvm_eh_exception.buf', '0', 'void*') + ';';
  });
  makeFuncLineActor('invoke', function(item) {
    // Wrapping in a function lets us easily return values if we are
    // in an assignment
    var phiSets = calcPhiSets(item);
    var call_ = makeFunctionCall(item.ident, item.params, item.funcData, item.type);
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
  makeFuncLineActor('atomic', function(item) {
    var type = item.params[0].type;
    var param1 = finalizeLLVMParameter(item.params[0]);
    var param2 = finalizeLLVMParameter(item.params[1]);
    switch (item.op) {
      case 'add': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue+' + param2, type) + ',tempValue)';
      case 'xchg': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, param2, type) + ',tempValue)';
      case 'cmpxchg': {
        var param3 = finalizeLLVMParameter(item.params[2]);
        return '(tempValue=' + makeGetValue(param1, 0, type) + ',(' + makeGetValue(param1, 0, type) + '==' + param2 + ' && (' + makeSetValue(param1, 0, param3, type) + ')),tempValue)';
      }
      default: throw 'unhandled atomic op: ' + item.op;
    }
  });
  makeFuncLineActor('landingpad', function(item) {
    // Just a stub
    return '{ f0: ' + makeGetValue('_llvm_eh_exception.buf', '0', 'void*') +
	 	   ', f1:' + makeGetValue('_llvm_eh_exception.buf', QUANTUM_SIZE, 'void*') + ' }';
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
    return makeBranch(finalizeLLVMParameter(item.value), item.currLabelId, true);
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
    return processMathop({
      op: 'bitcast', variant: null, type: item.type,
      param1: item.params[0]
    });
  });

  function makeFunctionCall(ident, params, funcData, type) {
    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    var shortident = LibraryManager.getRootIdent(ident.slice(1)) || ident.slice(1); // ident may not be in library, if all there is is ident__inline
    var args = [];
    var argsTypes = [];
    var varargs = [];
    var varargsTypes = [];
    var ignoreFunctionIndexizing = [];
    var useJSArgs = (shortident + '__jsargs') in LibraryManager.library;
    var hasVarArgs = isVarArgsFunctionType(type);
    var normalArgs = (hasVarArgs && !useJSArgs) ? countNormalArgs(type) : -1;

    params.forEach(function(param, i) {
      var val = finalizeParam(param);
      if (!hasVarArgs || useJSArgs || i < normalArgs) {
        if (param.type == 'i64' && I64_MODE == 1) {
          val = makeCopyI64(val); // Must copy [low, high] i64s, so they don't end up modified in the caller
        }
        args.push(val);
        argsTypes.push(param.type);
      } else {
        if (!(param.type == 'i64' && I64_MODE == 1)) {
          varargs.push(val);
          varargs = varargs.concat(zeros(Runtime.getNativeFieldSize(param.type)-1));
          varargsTypes.push(param.type);
          varargsTypes = varargsTypes.concat(zeros(Runtime.getNativeFieldSize(param.type)-1));
        } else {
          // i64 mode 1. Write one i32 with type i64, and one i32 with type i32
          varargs.push(val + '[0]');
          varargs = varargs.concat(zeros(Runtime.getNativeFieldSize('i32')-1));
          ignoreFunctionIndexizing.push(varargs.length); // We will have a value there, but no type (the type is i64, but we write two i32s)
          varargs.push(val + '[1]');
          varargs = varargs.concat(zeros(Runtime.getNativeFieldSize('i32')-1));
          varargsTypes.push('i64');
          varargsTypes = varargsTypes.concat(zeros(Runtime.getNativeFieldSize('i32')-1));
          varargsTypes.push('i32');
          varargsTypes = varargsTypes.concat(zeros(Runtime.getNativeFieldSize('i32')-1));
        }
      }
    });

    args = args.map(function(arg, i) { return indexizeFunctions(arg, argsTypes[i]) });
    varargs = varargs.map(function(vararg, i) {
      if (ignoreFunctionIndexizing.indexOf(i) >= 0) return vararg;
      return vararg === 0 ? 0 : indexizeFunctions(vararg, varargsTypes[i])
    });

    if (hasVarArgs && !useJSArgs) {
      if (varargs.length === 0) {
        varargs = [0];
        varargsTypes = ['i32'];
      }
      var offset = 0;
      varargs = '(tempInt=' + RuntimeGenerator.stackAlloc(varargs.length, ',') + ',' +
                varargs.map(function(arg, i) {
                  var type = varargsTypes[i];
                  if (type == 0) return null;
                  if (I64_MODE == 1 && type == 'i64') type = 'i32'; // We have [i64, 0, 0, 0, i32, 0, 0, 0] in the layout at this point
                  var ret = makeSetValue(getFastValue('tempInt', '+', offset), 0, arg, type, null, null, QUANTUM_SIZE, null, ',');
                  offset += Runtime.getNativeFieldSize(type);
                  return ret;
                }).filter(function(arg) {
                  return arg !== null;
                }).join(',') + ',tempInt)';
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
    return makeFunctionCall(item.ident, item.params, item.funcData, item.type) + (item.standalone ? ';' : '');
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

    if (!mainPass) {
      var generated = itemsDict.function.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.GlobalVariable).concat(itemsDict.GlobalVariablePostSet);
      Functions.allIdents = Functions.allIdents.concat(itemsDict.function.map(function(func) {
        return func.ident;
      }).filter(function(func) {
        return IGNORED_FUNCTIONS.indexOf(func.ident) < 0;
      }));
      if (!DEBUG_MEMORY) print(generated.map(function(item) { return item.JS }).join('\n'));
      return;
    }

    // This is the main pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    var generated = itemsDict.functionStub.concat(itemsDict.GlobalVariablePostSet);
    generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });
    if (RUNTIME_TYPE_INFO) {
      Types.cleanForRuntime();
      print('Runtime.typeInfo = ' + JSON.stringify(Types.types));
      print('Runtime.structMetadata = ' + JSON.stringify(Types.structMetadata));
    }
    var postFile = BUILD_AS_SHARED_LIB ? 'postamble_sharedlib.js' : 'postamble.js';
    var postParts = processMacros(preprocess(read(postFile))).split('{{GLOBAL_VARS}}');
    print(postParts[0]);

    // Print out global variables and postsets TODO: batching
    legalizedI64s = false;
    JSify(analyzer(intertyper(data.unparsedGlobalss[0].lines, true), true), true, Functions);
    data.unparsedGlobalss = null;

    print(Functions.generateIndexing()); // done last, as it may rely on aliases set in postsets

    // Load runtime-linked libraries
    RUNTIME_LINKED_LIBS.forEach(function(lib) {
      print('eval(read("' + lib + '"))(FUNCTION_TABLE.length, this);');
    });

    print(postParts[1]);

    print(shellParts[1]);
    // Print out some useful metadata (for additional optimizations later, like the eliminator)
    print('// EMSCRIPTEN_GENERATED_FUNCTIONS: ' + JSON.stringify(Functions.allIdents) + '\n');

    return null;
  }

  // Data

  if (mainPass) {
    substrate.addItems(data.functionStubs, 'FunctionStub');
    assert(data.functions.length == 0);
  } else {
    substrate.addItems(values(data.globalVariables), 'GlobalVariable');
    substrate.addItems(data.aliass, 'Alias');
    substrate.addItems(data.functions, 'FunctionSplitter');
  }

  finalCombiner(substrate.solve());

  dprint('framework', 'Big picture: Finishing JSifier, main pass=' + mainPass);
}

