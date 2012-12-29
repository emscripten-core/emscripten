//"use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Handy sets

var STRUCT_LIST = set('struct', 'list');
var UNDERSCORE_OPENPARENS = set('_', '(');
var RELOOP_IGNORED_LASTS = set('return', 'unreachable', 'resume');

// JSifier
function JSify(data, functionsOnly, givenFunctions) {
  var mainPass = !functionsOnly;

  if (mainPass) {
    var shellFile = SHELL_FILE ? SHELL_FILE : (BUILD_AS_SHARED_LIB ? 'shell_sharedlib.js' : 'shell.js');

    if (phase == 'pre') {
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

      var shellParts = read(shellFile).split('{{BODY}}');
      print(shellParts[0]);
      var preFile = BUILD_AS_SHARED_LIB ? 'preamble_sharedlib.js' : 'preamble.js';
      var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime())));
      print(pre);

      Functions.implementedFunctions = set(data.unparsedFunctions.map(function(func) { return func.ident }));
    }
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

    if (phase == 'pre') {
      var libFuncsToInclude;
      if (INCLUDE_FULL_LIBRARY) {
        assert(!BUILD_AS_SHARED_LIB, 'Cannot have both INCLUDE_FULL_LIBRARY and BUILD_AS_SHARED_LIB set.')
        libFuncsToInclude = [];
        for (var key in LibraryManager.library) {
          if (!key.match(/__(deps|postset|inline)$/)) {
            libFuncsToInclude.push(key);
          }
        }
      } else {
        libFuncsToInclude = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
      }
      libFuncsToInclude.forEach(function(ident) {
        data.functionStubs.push({
          intertype: 'functionStub',
          ident: '_' + ident
        });
      });
    }
  }

  // Functions

  Functions.currExternalFunctions = !mainPass ? givenFunctions.currExternalFunctions : {};

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

  if (phase == 'funcs') { // pre has function shells, just to defined implementedFunctions
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
  }

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
      var currValue = values[i];
      if (USE_TYPED_ARRAYS == 2 && typeData.fields[i] == 'i64') {
        // 'flatten' out the 64-bit value into two 32-bit halves
        var parts = parseI64Constant(currValue, true);
        ret[index++] = parts[0];
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = parts[1];
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
      } else {
        currValue = flatten(currValue);
        if (typeof currValue == 'object') {
          for (var j = 0; j < currValue.length; j++) {
            ret[index++] = currValue[j];
          }
        } else {
          ret[index++] = currValue;
        }
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
        return makeGlobalUse(indexizeFunctions(ret, segment.type));
      };
      return tokens.map(handleSegment)
    }

    //dprint('jsifier const: ' + JSON.stringify(value) + ',' + type + '\n');
    if (value.intertype in PARSABLE_LLVM_FUNCTIONS) {
      return [finalizeLLVMFunctionCall(value)];
    } else if (Runtime.isNumberType(type) || pointingLevels(type) >= 1) {
      return makeGlobalUse(indexizeFunctions(parseNumerical(value.value), type));
    } else if (value.intertype === 'emptystruct') {
      return makeEmptyStruct(type);
    } else if (value.intertype === 'string') {
      return JSON.stringify(parseLLVMString(value.text)) +
             ' /* ' + value.text.substr(0, 20).replace(/[*<>]/g, '_') + ' */'; // make string safe for inclusion in comment
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
                                                 || value.substr(0, 6) === 'GLOBAL';
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
        var constant = null;
        var allocator = (BUILD_AS_SHARED_LIB && !item.external) ? 'ALLOC_NORMAL' : 'ALLOC_STATIC';
        var index = null;
        if (item.external && BUILD_AS_SHARED_LIB) {
          // External variables in shared libraries should not be declared as
          // they would shadow similarly-named globals in the parent.
          item.JS = '';
        } else {
        	item.JS = makeGlobalDef(item.ident);
        }

        if (item.external) {
          // Import external global variables from the library if available.
          var shortident = item.ident.slice(1);
          if (LibraryManager.library[shortident] &&
              LibraryManager.library[shortident].length &&
              !BUILD_AS_SHARED_LIB) {
            var val = LibraryManager.library[shortident];
            var padding;
            if (Runtime.isNumberType(item.type) || isPointerType(item.type)) {
              padding = [item.type].concat(zeros(Runtime.getNativeFieldSize(item.type)-1));
            } else {
              padding = makeEmptyStruct(item.type);
            }
            var padded = val.concat(padding.slice(val.length));
            var js = item.ident + '=' + makePointer(JSON.stringify(padded), null, allocator, item.type, index) + ';'
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
          if (!NAMED_GLOBALS && isIndexableGlobal(item.ident)) {
            index = makeGlobalUse(item.ident); // index !== null indicates we are indexing this
            allocator = 'ALLOC_NONE';
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
                  JS: makeSetValue(makeGlobalUse(item.ident), i, value, 'i32', false, true) + ';' // ignore=true, since e.g. rtti and statics cause lots of safe_heap errors
                });
                constant[i] = '0';
              }
            });
            constant = '[' + constant.join(', ') + ']';
          }
          // NOTE: This is the only place that could potentially create static
          //       allocations in a shared library.
          constant = makePointer(constant, null, allocator, item.type, index);
          var js;

        	js = (index !== null ? '' : item.ident + '=') + constant + ';'; // \n Module.print("' + item.ident + ' :" + ' + makeGlobalUse(item.ident) + ');';

          // Special case: class vtables. We make sure they are null-terminated, to allow easy runtime operations
          if (item.ident.substr(0, 5) == '__ZTV') {
            if (index !== null) {
              index = getFastValue(index, '+', Runtime.alignMemory(calcAllocatedSize(Variables.globals[item.ident].type)));
            }
            js += '\n' + makePointer('[0]', null, allocator, ['void*'], index) + ';';
          }
          if (EXPORT_ALL || (item.ident in EXPORTED_GLOBALS)) {
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
      var fix = '';
      if (BUILD_AS_SHARED_LIB == 2 && !item.private_) {
        var target = item.ident;
        if (isFunctionType(item.type)) {
          target = item.value.ident; // the other side does not know this is an alias/function table index. So make it the alias target.
          var varData = Variables.globals[target];
          assert(!varData, 'multi-level aliasing does not work yet in shared lib 2 exports');
        }
        fix = '\nif (globalScope) { assert(!globalScope["' + item.ident + '"]); globalScope["' + item.ident + '"] = ' + target + ' }'
      }
      ret.push({
        intertype: 'GlobalVariablePostSet',
        ident: item.ident,
        dependencies: set([value]),
        JS: item.ident + ' = ' + value + ';' + fix
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
        addedLibraryItems[ident] = true;

        // dependencies can be JS functions, which we just run
        if (typeof ident == 'function') return ident();

        // Don't replace implemented functions with library ones (which can happen when we add dependencies).
        // Note: We don't return the dependencies here. Be careful not to end up where this matters
        if (('_' + ident) in Functions.implementedFunctions) return '';

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
            snippet = snippet.replace('{', '{ var ret = (function() { if (Runtime.debug) Module.print("[library call:' + ident + ': " + Array.prototype.slice.call(arguments).map(Runtime.prettyPrint) + "]"); ');
            snippet = snippet.substr(0, snippet.length-1) + '}).apply(this, arguments); if (Runtime.debug && typeof ret !== "undefined") Module.print("  [     return:" + Runtime.prettyPrint(ret)); return ret; }';
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
        var text = (deps ? '\n' + deps.map(addFromLibrary).filter(function(x) { return x != '' }).join('\n') : '');
        text += isFunction ? snippet : 'var ' + ident + '=' + snippet + ';';
        if (EXPORT_ALL || (ident in EXPORTED_FUNCTIONS)) {
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
        if (WARN_ON_UNDEFINED_SYMBOLS) {
          warn('Unresolved symbol: ' + item.ident);
        }
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

  // function for filtering functions for label debugging
  if (LABEL_FUNCTION_FILTERS.length > 0) {
    var LABEL_FUNCTION_FILTER_SET = set(LABEL_FUNCTION_FILTERS);
    var functionNameFilterTest = function(ident) {
      return (ident in LABEL_FUNCTION_FILTER_SET);
    };
  } else {
    // no filters are specified, all function names are printed
    var functionNameFilterTest = function(ident) {
      return true;
    }
  }

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

      if (PRINT_SPLIT_FILE_MARKER) {
          func.JS += '\n//FUNCTION_BEGIN_MARKER\n'
          var associatedSourceFile = "NO_SOURCE";
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

      if (LABEL_DEBUG && functionNameFilterTest(func.ident)) func.JS += "  Module.print(INDENT + ' Entering: " + func.ident + ": ' + Array.prototype.slice.call(arguments)); INDENT += '  ';\n";

      if (true) { // TODO: optimize away when not needed
        if (CLOSURE_ANNOTATIONS) func.JS += '/** @type {number} */';
        func.JS += '  var label;\n';
      }

      // Walk function blocks and generate JS
      function walkBlock(block, indent) {
        if (!block) return '';
        dprint('relooping', 'walking block: ' + block.type + ',' + block.entries + ' : ' + block.labels.length);
        function getLabelLines(label, indent, relooping) {
          if (!label) return '';
          var ret = '';
          if ((LABEL_DEBUG >= 2) && functionNameFilterTest(func.ident)) {
            ret += indent + "Module.print(INDENT + '" + func.ident + ":" + label.ident + "');\n";
          }
          if (EXECUTION_TIMEOUT > 0) {
            ret += indent + 'if (Date.now() - START_TIME >= ' + (EXECUTION_TIMEOUT*1000) + ') throw "Timed out!" + (new Error().stack);\n';
          }
          
          if (PRINT_SPLIT_FILE_MARKER && Debugging.on && Debugging.getAssociatedSourceFile(line.lineNum)) {
            // Overwrite the associated source file for every line. The last line should contain the source file associated to
            // the return value/address of outer most block (the marked function).
            associatedSourceFile = Debugging.getAssociatedSourceFile(line.lineNum);
          }
          
          // for special labels we care about (for phi), mark that we visited them
          var i = 0;
          return ret + label.lines.map(function(line) {
            var JS = line.JS;
            if (relooping && i == label.lines.length-1) {
              if (line.intertype == 'branch' || line.intertype == 'switch') {
                JS = ''; // just branching operations - done in the relooper, so nothing need be done here
              } else if (line.intertype == 'invoke') {
                JS = line.reloopingJS; // invokes have code that is not rendered in the relooper (the call inside a try-catch)
              }
            }
            i++;
            return JS + (Debugging.on ? Debugging.getComment(line.lineNum) : '');
          })
                                  .join('\n')
                                  .split('\n') // some lines include line breaks
                                  .map(function(line) { return indent + line })
                                  .join('\n');
        }
        var ret = '';
        if (!RELOOP || func.forceEmulated) { // TODO: also if just 1 label?
          if (block.labels.length > 1) {
            if (block.entries.length == 1) {
              ret += indent + 'label = ' + getLabelId(block.entries[0]) + '; ' + (SHOW_LABELS ? '/* ' + getOriginalLabelId(block.entries[0]) + ' */' : '') + '\n';
            } // otherwise, should have been set before!
            if (func.setjmpTable) {
              var setjmpTable = {};
              ret += indent + 'var mySetjmpIds = {};\n';
              ret += indent + 'var setjmpTable = {';
              func.setjmpTable.forEach(function(triple) { // original label, label we created for right after the setjmp, variable setjmp result goes into
                ret += '"' + getLabelId(triple[0]) + '": ' + 'function(value) { label = ' + getLabelId(triple[1]) + '; ' + triple[2] + ' = value },';
              });
              ret += 'dummy: 0';
              ret += '};\n';
            }
            ret += indent + 'while(1) ';
            if (func.setjmpTable) {
              ret += 'try { ';
            }
            ret += 'switch(label) {\n';
            ret += block.labels.map(function(label) {
              return indent + '  case ' + getLabelId(label.ident) + ': ' + (SHOW_LABELS ? '// ' + getOriginalLabelId(label.ident) : '') + '\n'
                            + getLabelLines(label, indent + '    ');
            }).join('\n');
            ret += '\n' + indent + '  default: assert(0, "bad label: " + label);\n' + indent + '}';
            if (func.setjmpTable) {
              ret += ' } catch(e) { if (!e.longjmp || !(e.id in mySetjmpIds)) throw(e); setjmpTable[setjmpLabels[e.id]](e.value) }';
            }
          } else {
            ret += (SHOW_LABELS ? indent + '/* ' + block.entries[0] + ' */' : '') + '\n' + getLabelLines(block.labels[0], indent);
          }
          ret += '\n';
        } else {
          // Reloop multiple blocks using the compiled relooper

          //Relooper.setDebug(1);
          Relooper.init();

          var blockMap = {};
          // add blocks
          for (var i = 0; i < block.labels.length; i++) {
            var label = block.labels[i];
            var content = getLabelLines(label, '', true);
            //printErr(func.ident + ' : ' + label.ident + ' : ' + content + '\n');
            blockMap[label.ident] = Relooper.addBlock(content);
          }
          // add branchings
          function relevant(x) { return x && x.length > 2 ? x : 0 } // ignores ';' which valueJS and label*JS can be if empty
          for (var i = 0; i < block.labels.length; i++) {
            var label = block.labels[i];
            var ident = label.ident;
            var last = label.lines[label.lines.length-1];
            //printErr('zz last ' + dump(last));
            if (last.intertype == 'branch') {
              if (last.label) { // 1 target
                Relooper.addBranch(blockMap[ident], blockMap[last.label], 0, relevant(last.labelJS));
              } else { // 2 targets
                Relooper.addBranch(blockMap[ident], blockMap[last.labelTrue], last.valueJS, relevant(last.labelTrueJS));
                Relooper.addBranch(blockMap[ident], blockMap[last.labelFalse], 0, relevant(last.labelFalseJS));
              }
            } else if (last.intertype == 'switch') {
              last.groupedLabels.forEach(function(switchLabel) {
                Relooper.addBranch(blockMap[ident], blockMap[switchLabel.label], switchLabel.value, relevant(switchLabel.labelJS));
              });
              Relooper.addBranch(blockMap[ident], blockMap[last.defaultLabel], 0, relevant(last.defaultLabelJS));
            } else if (last.intertype == 'invoke') {
              Relooper.addBranch(blockMap[ident], blockMap[last.toLabel], '!__THREW__', relevant(last.toLabelJS));
              Relooper.addBranch(blockMap[ident], blockMap[last.unwindLabel], 0, relevant(last.unwindLabelJS));
            } else if (last.intertype in RELOOP_IGNORED_LASTS) {
            } else {
              throw 'unknown reloop last line: ' + last.intertype;
            }
          }
          ret += Relooper.render(blockMap[block.entries[0]]);
        }
        return ret;
      }
      func.JS += walkBlock(func.block, '  ');
      // Finalize function
      if (LABEL_DEBUG && functionNameFilterTest(func.ident)) func.JS += "  INDENT = INDENT.substr(0, INDENT.length-2);\n";
      // Add an unneeded return, needed for strict mode to not throw warnings in some cases.
      // If we are not relooping, then switches make it unimportant to have this (and, we lack hasReturn anyhow)
      if (RELOOP && func.lines.length > 0 && func.labels.filter(function(label) { return label.hasReturn }).length > 0) {
        func.JS += '  return' + (func.returnType !== 'void' ? ' null' : '') + ';\n';
      }
      func.JS += '}\n';
      
      if (PRINT_SPLIT_FILE_MARKER) {
          func.JS += '\n//FUNCTION_END_MARKER_OF_SOURCE_FILE_' + associatedSourceFile + '\n';
      }
      
      if (EXPORT_ALL || (func.ident in EXPORTED_FUNCTIONS)) {
        func.JS += 'Module["' + func.ident + '"] = ' + func.ident + ';';
      }

      if (INLINING_LIMIT && func.lines.length >= INLINING_LIMIT) {
        func.JS += func.ident + '["X"]=1;';
      }

      if (BUILD_AS_SHARED_LIB == 2) {
        // TODO: make the assert conditional on ASSERTIONS
        func.JS += 'if (globalScope) { assert(!globalScope["' + func.ident + '"]); globalScope["' + func.ident + '"] = ' + func.ident + ' }';
      }

      func.JS = func.JS.replace(/\n *;/g, '\n'); // remove unneeded lines
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
  makeFuncLineActor('var', function(item) { // assigns into phis become simple vars
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
        if (isNumber(item.ident)) {
          // Direct write to a memory address; this may be an intentional segfault, if not, it is a bug in the source
          return 'throw "fault on write to ' + item.ident + '";';
        }
        return item.ident + '=' + value + ';'; // We have the actual value here
        break;
      case VAR_EMULATED:
        if (item.pointer.intertype == 'value') {
          return makeSetValue(makeGlobalUse(item.ident), 0, value, item.valueType, 0, 0, item.align) + ';';
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

  function getOriginalLabelId(label) {
    var funcData = Framework.currItem.funcData;
    var labelIdsInverse = funcData.labelIdsInverse;
    return labelIdsInverse[label];
  }

  function getLabelId(label) {
    return label;
  }

  function makeBranch(label, lastLabel, labelIsVariable) { // lastLabel is deprecated
    var pre = '';
    if (label[0] == 'B') {
      assert(!labelIsVariable, 'Cannot handle branches to variables with special branching options');
      var parts = label.split('|');
      var trueLabel = parts[1] || '';
      var oldLabel = parts[2] || '';
      var labelSetting = oldLabel ? 'label = ' + getLabelId(oldLabel) + ';' +
                         (SHOW_LABELS ? ' /* to: ' + getOriginalLabelId(cleanLabel(oldLabel)) + ' */' : '') : ''; // TODO: optimize away the setting
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
      return pre + 'label = ' + label + ';' + (SHOW_LABELS ? ' /* to: ' + getOriginalLabelId(cleanLabel(label)) + ' */' : '') + ' break;';
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
      return (item.labelJS = getPhiSetsForLabel(phiSets, item.label)) + makeBranch(item.label, item.currLabelId);
    } else {
      var condition = item.valueJS = finalizeLLVMParameter(item.value);
      var labelTrue = (item.labelTrueJS = getPhiSetsForLabel(phiSets, item.labelTrue)) + makeBranch(item.labelTrue, item.currLabelId);
      var labelFalse = (item.labelFalseJS = getPhiSetsForLabel(phiSets, item.labelFalse)) + makeBranch(item.labelFalse, item.currLabelId);
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
    // Consolidate checks that go to the same label. This is important because it makes the relooper simpler and faster.
    var targetLabels = {}; // for each target label, the list of values going to it
    var switchLabelMap = {};
    item.switchLabels.forEach(function(switchLabel) {
      switchLabelMap[switchLabel.label] = switchLabel;
      if (!targetLabels[switchLabel.label]) {
        targetLabels[switchLabel.label] = [];
      }
      targetLabels[switchLabel.label].push(switchLabel.value);
    });
    var ret = '';
    var first = true;
    var signedIdent = makeSignOp(item.ident, item.type, 're'); // we need to standardize for purpose of comparison
    if (RELOOP) {
      item.groupedLabels = [];
    }
    for (var targetLabel in targetLabels) {
      if (!first) {
        ret += 'else ';
      } else {
        first = false;
      }
      var value = targetLabels[targetLabel].map(function(value) {
        return makeComparison(signedIdent, makeSignOp(value, item.type, 're'), item.type)
      }).join(' || ');
      ret += 'if (' + value + ') {\n';
      var phiSet = getPhiSetsForLabel(phiSets, targetLabel);
      ret += '  ' + phiSet + makeBranch(targetLabel, item.currLabelId || null) + '\n';
      ret += '}\n';
      if (RELOOP) {
        item.groupedLabels.push({
          label: targetLabel,
          value: value,
          labelJS: phiSet
        });
      }
    }
    if (item.switchLabels.length > 0) ret += 'else {\n';
    var phiSet = item.defaultLabelJS = getPhiSetsForLabel(phiSets, item.defaultLabel);
    ret += phiSet + makeBranch(item.defaultLabel, item.currLabelId) + '\n';
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
    if (LABEL_DEBUG && functionNameFilterTest(item.funcData.ident)) {
      ret += "Module.print(INDENT + 'Exiting: " + item.funcData.ident + "');\n"
          +  "INDENT = INDENT.substr(0, INDENT.length-2);\n";
    }
    ret += 'return';
    if (item.value) {
      ret += ' ' + finalizeLLVMParameter(item.value);
    }
    return ret + ';';
  });
  makeFuncLineActor('resume', function(item) {
    // If there is no current exception, set this one as it (during a resume, the current exception can be wiped out)
    return (EXCEPTION_DEBUG ? 'Module.print("Resuming exception");' : '') + 
      'if (' + makeGetValue('_llvm_eh_exception.buf', 0, 'void*') + ' == 0) { ' + makeSetValue('_llvm_eh_exception.buf', 0, item.ident + '.f0', 'void*') + ' } ' + 
      'throw ' + item.ident + '.f0;';
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
            + (EXCEPTION_DEBUG ? 'Module.print("Exception: " + e + ", currently at: " + (new Error().stack)); ' : '')
            + 'return null } })();';
    if (item.assignTo) {
      ret = 'var ' + item.assignTo + ' = ' + ret;
      if (isIllegalType(item.type)) {
        assert(item.type == 'i64', 'Can only handle i64 invoke among illegal invokes');
        ret += 'var ' + item.assignTo + '$0 = ' + item.assignTo + '[0], ' + item.assignTo + '$1 = ' + item.assignTo + '[1];';
      }
      item.assignTo = null;
    }
    item.reloopingJS = ret; // everything but the actual branching (which the relooper will do for us)
    item.toLabelJS = getPhiSetsForLabel(phiSets, item.toLabel);
    item.unwindLabelJS = getPhiSetsForLabel(phiSets, item.unwindLabel);
    ret += 'if (!__THREW__) { ' + item.toLabelJS + makeBranch(item.toLabel, item.currLabelId)
            + ' } else { ' + item.unwindLabelJS + makeBranch(item.unwindLabel, item.currLabelId) + ' }';
    return ret;
  });
  makeFuncLineActor('atomic', function(item) {
    var type = item.params[0].type;
    var param1 = finalizeLLVMParameter(item.params[0]);
    var param2 = finalizeLLVMParameter(item.params[1]);
    switch (item.op) {
      case 'add': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue+' + param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'sub': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue-' + param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'xchg': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'cmpxchg': {
        var param3 = finalizeLLVMParameter(item.params[2]);
        return '(tempValue=' + makeGetValue(param1, 0, type) + ',(' + makeGetValue(param1, 0, type) + '==' + param2 + ' && (' + makeSetValue(param1, 0, param3, type, null, null, null, null, ',') + ')),tempValue)';
      }
      default: throw 'unhandled atomic op: ' + item.op;
    }
  });
  makeFuncLineActor('landingpad', function(item) {
    var catchTypeArray = item.catchables.map(finalizeLLVMParameter).join(',');
    return '___cxa_find_matching_catch('+ makeGetValue('_llvm_eh_exception.buf', '0', 'void*') +',' + makeGetValue('_llvm_eh_exception.buf', QUANTUM_SIZE, 'void*') + ',[' + catchTypeArray +'])';
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
    var index = item.indexes[0][0].text;
    var valueType = Types.types[item.type].fields[index];
    if (USE_TYPED_ARRAYS != 2 || valueType != 'i64') {
      return item.ident + '.f' + index;
    } else {
      var assignTo = item.assignTo;
      item.assignTo = null;
      return 'var ' + assignTo + '$0 = ' + item.ident + '.f' + index + '[0];' +
             'var ' + assignTo + '$1 = ' + item.ident + '.f' + index + '[1];';
    }
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

  makeFuncLineActor('mathop', processMathop);

  makeFuncLineActor('bitcast', function(item) {
    var temp = {
      op: 'bitcast', variant: null, type: item.type,
      assignTo: item.assignTo,
      params: [item.params[0]] // XXX
    };
    var ret = processMathop(temp);
    if (!temp.assignTo) item.assignTo = null; // If the assign was stolen, propagate that
    return ret;
  });

  function makeFunctionCall(ident, params, funcData, type) {
    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    var shortident = LibraryManager.getRootIdent(ident.slice(1)) || ident.slice(1); // ident may not be in library, if all there is is ident__inline
    var args = [];
    var argsTypes = [];
    var varargs = [];
    var varargsTypes = [];
    var varargsByVals = {};
    var ignoreFunctionIndexizing = [];
    var useJSArgs = (shortident + '__jsargs') in LibraryManager.library;
    var hasVarArgs = isVarArgsFunctionType(type);
    var normalArgs = (hasVarArgs && !useJSArgs) ? countNormalArgs(type) : -1;

    params.forEach(function(param, i) {
      var val = finalizeParam(param);
      if (!hasVarArgs || useJSArgs || i < normalArgs) {
        args.push(val);
        argsTypes.push(param.type);
      } else {
        var size;
        if (param.byVal) {
          varargsByVals[varargs.length] = param.byVal;
          size = calcAllocatedSize(removeAllPointing(param.type));
        } else {
          size = Runtime.getNativeFieldSize(param.type);
        }
        varargs.push(val);
        varargs = varargs.concat(zeros(size-1));
        // TODO: replace concats like this with push
        varargsTypes.push(param.type);
        varargsTypes = varargsTypes.concat(zeros(size-1));
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
                  var ret;
                  if (!varargsByVals[i]) {
                    ret = makeSetValue(getFastValue('tempInt', '+', offset), 0, arg, type, null, null, QUANTUM_SIZE, null, ',');
                    offset += Runtime.getNativeFieldSize(type);
                  } else {
                    assert(offset % 4 == 0); // varargs must be aligned
                    var size = calcAllocatedSize(removeAllPointing(type));
                    assert(size % 4 == 0); // varargs must stay aligned
                    ret = makeCopyValues(getFastValue('tempInt', '+', offset), arg, size, null, null, varargsByVals[i], ',');
                    offset += size;
                  }
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

  makeFuncLineActor('unreachable', function(item) {
    if (ASSERTIONS) {
      return 'throw "Reached an unreachable!"';
    } else {
      return ';';
    }
  });

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
      if (phase == 'pre' && !Variables.generatedGlobalBase) {
        Variables.generatedGlobalBase = true;
        if (Variables.nextIndexedOffset > 0) {
          // Variables have been calculated, print out the base generation before we print them
          print('var GLOBAL_BASE = STATICTOP;\n');
          print('STATICTOP += ' + Variables.nextIndexedOffset + ';\n');
          print('assert(STATICTOP < TOTAL_MEMORY);\n');
        }
      }
      var generated = itemsDict.function.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.GlobalVariable).concat(itemsDict.GlobalVariablePostSet);
      if (!DEBUG_MEMORY) print(generated.map(function(item) { return item.JS }).join('\n'));
      return;
    }

    // Print out global variables and postsets TODO: batching
    if (phase == 'pre') {
      var legalizedI64sDefault = legalizedI64s;
      legalizedI64s = false;

      var globalsData = analyzer(intertyper(data.unparsedGlobalss[0].lines, true), true);

      if (!NAMED_GLOBALS) {
        sortGlobals(globalsData.globalVariables).forEach(function(g) {
          var ident = g.ident;
          if (!isIndexableGlobal(ident)) return;
          Variables.indexedGlobals[ident] = Variables.nextIndexedOffset;
          Variables.nextIndexedOffset += Runtime.alignMemory(calcAllocatedSize(Variables.globals[ident].type));
          if (ident.substr(0, 5) == '__ZTV') { // leave room for null-terminating the vtable
            Variables.nextIndexedOffset += Runtime.getNativeTypeSize('i32');
          }
        });
      }

      JSify(globalsData, true, Functions);
      globalsData = null;
      data.unparsedGlobalss = null;

      var generated = itemsDict.functionStub.concat(itemsDict.GlobalVariablePostSet);
      generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });

      legalizedI64s = legalizedI64sDefault;
    } else {
      if (singlePhase) {
        assert(data.unparsedGlobalss[0].lines.length == 0, dump([phase, data.unparsedGlobalss]));
        assert(itemsDict.functionStub.length == 0, dump([phase, itemsDict.functionStub]));
      }
    }

    if (phase == 'pre' || phase == 'funcs') {
      // serialize out the data that later passes need
      PassManager.serialize(); // XXX for funcs pass, do not serialize it all. I think we just need which were indexized.
      return;
    }

    // This is the main 'post' pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    if (PRECISE_I64_MATH && Types.preciseI64MathUsed) {
      print(read('long.js'));
    } else {
      print('// Warning: printing of i64 values may be slightly rounded! No deep i64 math used, so precise i64 code not included');
      print('var i64Math = null;');
    }
    if (HEADLESS) {
      print('if (!ENVIRONMENT_IS_WEB) {');
      print(read('headless.js').replace("'%s'", "'http://emscripten.org'").replace("'?%s'", "''").replace('%s,', 'null,').replace('%d', '0'));
      print('}');
    }
    if (RUNTIME_TYPE_INFO) {
      Types.cleanForRuntime();
      print('Runtime.typeInfo = ' + JSON.stringify(Types.types));
      print('Runtime.structMetadata = ' + JSON.stringify(Types.structMetadata));
    }
    var postFile = BUILD_AS_SHARED_LIB ? 'postamble_sharedlib.js' : 'postamble.js';
    var postParts = processMacros(preprocess(read(postFile))).split('{{GLOBAL_VARS}}');
    print(postParts[0]);

    print(Functions.generateIndexing()); // done last, as it may rely on aliases set in postsets

    // Load runtime-linked libraries
    RUNTIME_LINKED_LIBS.forEach(function(lib) {
      print('eval(Module["read"]("' + lib + '"))(FUNCTION_TABLE.length, this);');
    });

    print(postParts[1]);

    var shellParts = read(shellFile).split('{{BODY}}');
    print(shellParts[1]);
    // Print out some useful metadata (for additional optimizations later, like the eliminator)
    print('// EMSCRIPTEN_GENERATED_FUNCTIONS: ' + JSON.stringify(keys(Functions.implementedFunctions).filter(function(func) {
      return IGNORED_FUNCTIONS.indexOf(func.ident) < 0;
    })) + '\n');

    return null;
  }

  // Data

  if (mainPass) {
    substrate.addItems(data.functionStubs, 'FunctionStub');
    assert(data.functions.length == 0);
  } else {
    substrate.addItems(sortGlobals(data.globalVariables), 'GlobalVariable');
    substrate.addItems(data.aliass, 'Alias');
    substrate.addItems(data.functions, 'FunctionSplitter');
  }

  finalCombiner(substrate.solve());

  dprint('framework', 'Big picture: Finishing JSifier, main pass=' + mainPass);
}

