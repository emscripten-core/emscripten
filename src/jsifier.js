//"use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Handy sets

var STRUCT_LIST = set('struct', 'list');
var RELOOP_IGNORED_LASTS = set('return', 'unreachable', 'resume');

var addedLibraryItems = {};
var asmLibraryFunctions = [];

var SETJMP_LABEL = -1;

var INDENTATION = ' ';

var functionStubSigs = {};

// JSifier
function JSify(data, functionsOnly) {
  //B.start('jsifier');
  var mainPass = !functionsOnly;

  var itemsDict = { type: [], GlobalVariableStub: [], functionStub: [], function: [], GlobalVariable: [], GlobalVariablePostSet: [] };

  if (mainPass) {
    var shellFile = SHELL_FILE ? SHELL_FILE : (BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'shell_sharedlib.js' : 'shell.js');

    if (phase == 'pre' || phase == 'glue') {
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
      print(processMacros(preprocess(shellParts[0])));
      var preFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'preamble_sharedlib.js' : 'preamble.js';
      var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime())));
      print(pre);

      // Populate implementedFunctions. Note that this is before types, and will be updated later.
      data.unparsedFunctions.forEach(function(func) {
        Functions.implementedFunctions[func.ident] = Functions.getSignature(func.returnType, func.params.map(function(param) { return param.type }));
      });
    }
  }

  if (mainPass) {
    // Handle unparsed types TODO: Batch them
    analyzer(intertyper(data.unparsedTypess[0].lines, true), true);
    data.unparsedTypess = null;

    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)
    //B.start('jsifier-libload');
    LibraryManager.load();
    //B.stop('jsifier-libload');

    if (phase == 'pre' || phase == 'glue') {
      var libFuncsToInclude;
      if (INCLUDE_FULL_LIBRARY) {
        assert(!(BUILD_AS_SHARED_LIB || SIDE_MODULE), 'Cannot have both INCLUDE_FULL_LIBRARY and BUILD_AS_SHARED_LIB/SIDE_MODULE set.')
        libFuncsToInclude = [];
        for (var key in LibraryManager.library) {
          if (!key.match(/__(deps|postset|inline|asm|sig)$/)) {
            libFuncsToInclude.push(key);
          }
        }
      } else {
        libFuncsToInclude = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
      }
      libFuncsToInclude.forEach(function(ident) {
        var finalName = '_' + ident;
        if (ident[0] === '$') {
          finalName = ident.substr(1);
        }
        data.functionStubs.push({
          intertype: 'functionStub',
          finalName: finalName,
          ident: '_' + ident
        });
      });
    }
  }

  // Functions

  if (phase == 'funcs') { // || phase == 'pre') { // pre has function shells, just to defined implementedFunctions
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
      JSify(analyzer(intertyper(currFuncLines, true, currBaseLineNums), true), true);
      if (DEBUG_MEMORY) MemoryDebugger.tick('post-func');
    }
    currFuncLines = currBaseLineNums = null; // Do not hold on to anything from inside that loop (JS function scoping..)
    data.unparsedFunctions = null;
  }

  // Actors

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
      if (USE_TYPED_ARRAYS == 2 && (typeData.fields[i] == 'i64' || (typeData.flatFactor && typeData.fields[0] == 'i64'))) {
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
    } else if (Compiletime.isNumberType(type) || pointingLevels(type) >= 1) {
      return [makeGlobalUse(indexizeFunctions(parseNumerical(value.value), type))];
    } else if (value.intertype === 'emptystruct') {
      return makeEmptyStruct(type);
    } else if (value.intertype === 'string') {
      return parseLLVMString(value.text);
    } else {
      return alignStruct(handleSegments(value.contents), type);
    }
  }

  function parseConst(value, type, ident) {
    var constant = makeConst(value, type);
    // Sadly, we've thrown away type information in makeConst, so we're not
    // passing correct type info to parseNumerical which works around this
    // lack.
    constant = flatten(constant).map(function(x) { return parseNumerical(x) })
    return constant;
  }

  // globalVariable
  function globalVariableHandler(item) {

    function needsPostSet(value) {
      if (typeof value !== 'string') return false;
      // (' is ok, as it is something we can indexize later into a concrete int: ('{{ FI_ ...
      return /^([(_][^']|CHECK_OVERFLOW|GLOBAL).*/.test(value);
    }

    item.intertype = 'GlobalVariableStub';
    itemsDict.GlobalVariableStub.push(item);
    assert(!item.lines); // FIXME remove this, after we are sure it isn't needed
    if (item.ident == '_llvm_global_ctors') {
      item.JS = '\n/* global initializers */ __ATINIT__.push(' +
                  item.ctors.map(function(ctor) { return '{ func: function() { ' + ctor + '() } }' }).join(',') +
                ');\n';
      return;
    }

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

    if (!NAMED_GLOBALS && isIndexableGlobal(item.ident)) {
      index = makeGlobalUse(item.ident); // index !== null indicates we are indexing this
      allocator = 'ALLOC_NONE';
    }

    Variables.globals[item.ident].named = item.named;

    if (ASM_JS && (MAIN_MODULE || SIDE_MODULE) && !item.private_ && !NAMED_GLOBALS && isIndexableGlobal(item.ident)) {
      // We need this to be named (and it normally would not be), so that it can be linked to and used from other modules
      Variables.globals[item.ident].linkable = 1;
    }

    if (isBSS(item)) {
      // If using indexed globals, go ahead and early out (no need to explicitly
      // initialize).
      if (!NAMED_GLOBALS) return;

      // If using named globals, we can at least shorten the call to allocate by
      // passing an integer representing the size of memory to alloc instead of
      // an array of 0s of size length.
      constant = Runtime.alignMemory(calcAllocatedSize(item.type));
    } else {
      if (item.external) {
        if (LibraryManager.library[item.ident.slice(1)]) {
          constant = LibraryManager.library[item.ident.slice(1)];
        } else if (Compiletime.isNumberType(item.type) || isPointerType(item.type)) {
          constant = zeros(Runtime.getNativeFieldSize(item.type));
        } else {
          constant = makeEmptyStruct(item.type);
        }
      } else {
        constant = parseConst(item.value, item.type, item.ident);
      }

      // This is a flattened object. We need to find its idents, so they can be assigned to later
      if (typeof constant === 'object') {
        var structTypes = null;
        constant.forEach(function(value, i) {
          if (needsPostSet(value)) { // ident, or expression containing an ident
            if (!structTypes) structTypes = generateStructTypes(item.type);
            itemsDict.GlobalVariablePostSet.push({
              intertype: 'GlobalVariablePostSet',
              JS: makeSetValue(makeGlobalUse(item.ident), i, value, structTypes[i], false, true) + ';' // ignore=true, since e.g. rtti and statics cause lots of safe_heap errors
            });
            constant[i] = '0';
          } else {
            if (typeof value === 'string') constant[i] = deParenCarefully(value);
          }
        });
      }

      if (item.external) {
        // External variables in shared libraries should not be declared as
        // they would shadow similarly-named globals in the parent, so do nothing here.
        if (BUILD_AS_SHARED_LIB) return;
        if (SIDE_MODULE) {
          itemsDict.GlobalVariableStub.pop(); // remove this item
          return;
        }
        // Library items need us to emit something, but everything else requires nothing.
        if (!LibraryManager.library[item.ident.slice(1)]) return;
      }

      // ensure alignment
      if (typeof constant === 'object') {
        var extra = Runtime.alignMemory(constant.length) - constant.length;
        if (item.ident.substr(0, 5) == '__ZTV') extra += Runtime.alignMemory(QUANTUM_SIZE);
        while (extra-- > 0) constant.push(0);
      }
    }

    // NOTE: This is the only place that could potentially create static
    //       allocations in a shared library.
    if (typeof constant !== 'string') {
      constant = makePointer(constant, null, allocator, item.type, index);
    }

    var js = (index !== null ? '' : item.ident + '=') + constant;
    if (js) js += ';';

    if (!ASM_JS && NAMED_GLOBALS && (EXPORT_ALL || (item.ident in EXPORTED_GLOBALS))) {
      js += '\nModule["' + item.ident + '"] = ' + item.ident + ';';
    }
    if (BUILD_AS_SHARED_LIB == 2 && !item.private_) {
      // TODO: make the assert conditional on ASSERTIONS
      js += 'if (globalScope) { assert(!globalScope["' + item.ident + '"]); globalScope["' + item.ident + '"] = ' + item.ident + ' }';
    }
    if (item.external && !NAMED_GLOBALS) {
      js = 'var ' + item.ident + '=' + js; // force an explicit naming, even if unnamed globals, for asm forwarding
    }
    itemsDict.GlobalVariableStub.push({
      intertype: 'GlobalVariable',
      JS: js,
    });
  }

  // alias
  function aliasHandler(item) {
    item.intertype = 'GlobalVariableStub';
    itemsDict.GlobalVariableStub.push(item);
    item.JS = 'var ' + item.ident + ';';
    // Set the actual value in a postset, since it may be a global variable. We also order by dependencies there
    Variables.globals[item.ident].targetIdent = item.value.ident;
    var value = Variables.globals[item.ident].resolvedAlias = finalizeLLVMParameter(item.value);
    if ((MAIN_MODULE || SIDE_MODULE) && isFunctionType(item.type)) {
      var target = item.value.ident;
      if (!Functions.aliases[target]) Functions.aliases[target] = [];
      Functions.aliases[target].push(item.ident);
    }
  }

  function processLibraryFunction(snippet, ident, finalName) {
    snippet = snippet.toString();
    assert(snippet.indexOf('XXX missing C define') == -1,
           'Trying to include a library function with missing C defines: ' + finalName + ' | ' + snippet);

    // name the function; overwrite if it's already named
    snippet = snippet.replace(/function(?:\s+([^(]+))?\s*\(/, 'function ' + finalName + '(');
    if (LIBRARY_DEBUG && !LibraryManager.library[ident + '__asm']) {
      snippet = snippet.replace('{', '{ var ret = (function() { if (Runtime.debug) Module.printErr("[library call:' + finalName + ': " + Array.prototype.slice.call(arguments).map(Runtime.prettyPrint) + "]"); ');
      snippet = snippet.substr(0, snippet.length-1) + '}).apply(this, arguments); if (Runtime.debug && typeof ret !== "undefined") Module.printErr("  [     return:" + Runtime.prettyPrint(ret)); return ret; \n}';
    }
    return snippet;
  }

  // functionStub
  function functionStubHandler(item) {
    // note the signature
    if (item.returnType && item.params) {
      functionStubSigs[item.ident] = Functions.getSignature(item.returnType.text, item.params.map(function(arg) { return arg.type }), false);
    }

    function addFromLibrary(ident, notDep) {
      if (ident in addedLibraryItems) return '';
      addedLibraryItems[ident] = true;

      // dependencies can be JS functions, which we just run
      if (typeof ident == 'function') return ident();

      // $ident's are special, we do not prefix them with a '_'.
      if (ident[0] === '$') {
        var finalName = ident.substr(1);
      } else {
        var finalName = '_' + ident;
      }

      // Don't replace implemented functions with library ones (which can happen when we add dependencies).
      // Note: We don't return the dependencies here. Be careful not to end up where this matters
      if (finalName in Functions.implementedFunctions) return '';

      if (!LibraryManager.library.hasOwnProperty(ident) && !LibraryManager.library.hasOwnProperty(ident + '__inline')) {
        if (notDep) {
          if (VERBOSE || ident.substr(0, 11) !== 'emscripten_') { // avoid warning on emscripten_* functions which are for internal usage anyhow
            if (ERROR_ON_UNDEFINED_SYMBOLS) error('unresolved symbol: ' + ident);
            else if (VERBOSE || (WARN_ON_UNDEFINED_SYMBOLS && !LINKABLE)) warn('unresolved symbol: ' + ident);
          }
        }
        // emit a stub that will fail at runtime
        LibraryManager.library[shortident] = new Function("Module['printErr']('missing function: " + shortident + "'); abort(-1);");
      }

      var snippet = LibraryManager.library[ident];
      var redirectedIdent = null;
      var deps = LibraryManager.library[ident + '__deps'] || [];
      deps.forEach(function(dep) {
        if (typeof snippet === 'string' && !(dep in LibraryManager.library)) warn('missing library dependency ' + dep + ', make sure you are compiling with the right options (see #ifdefs in src/library*.js)');
      });
      var isFunction = false;

      if (typeof snippet === 'string') {
        var target = LibraryManager.library[snippet];
        if (target) {
          // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
          // This avoid having duplicate functions with identical content.
          redirectedIdent = snippet;
          deps.push(snippet);
          snippet = '_' + snippet;
        }
        // In asm, we need to know about library functions. If there is a target, though, then no
        // need to consider this a library function - we will call directly to it anyhow
        if (ASM_JS && !redirectedIdent && (typeof target == 'function' || /Math_\w+/.exec(snippet))) {
          Functions.libraryFunctions[finalName] = 1;
        }
      } else if (typeof snippet === 'object') {
        snippet = stringifyWithFunctions(snippet);
      } else if (typeof snippet === 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, ident, finalName);
        if (ASM_JS) Functions.libraryFunctions[finalName] = 1;
      }

      var postsetId = ident + '__postset';
      var postset = LibraryManager.library[postsetId];
      if (postset && !addedLibraryItems[postsetId] && !SIDE_MODULE) {
        addedLibraryItems[postsetId] = true;
        itemsDict.GlobalVariablePostSet.push({
          intertype: 'GlobalVariablePostSet',
          JS: postset
        });
      }

      if (redirectedIdent) {
        deps = deps.concat(LibraryManager.library[redirectedIdent + '__deps'] || []);
      }
      if (ASM_JS) {
        // In asm, dependencies implemented in C might be needed by JS library functions.
        // We don't know yet if they are implemented in C or not. To be safe, export such
        // special cases.
        [LIBRARY_DEPS_TO_AUTOEXPORT].forEach(function(special) {
          deps.forEach(function(dep) {
            if (dep == special && !EXPORTED_FUNCTIONS[dep]) {
              EXPORTED_FUNCTIONS[dep] = 1;
            }
          });
        });
      }
      if (VERBOSE) printErr('adding ' + finalName + ' and deps ' + deps + ' : ' + (snippet + '').substr(0, 40));
      var depsText = (deps ? '\n' + deps.map(addFromLibrary).filter(function(x) { return x != '' }).join('\n') : '');
      var contentText = isFunction ? snippet : ('var ' + finalName + '=' + snippet + ';');
      if (ASM_JS) {
        var sig = LibraryManager.library[ident + '__sig'];
        if (isFunction && sig && LibraryManager.library[ident + '__asm']) {
          // asm library function, add it as generated code alongside the generated code
          Functions.implementedFunctions[finalName] = sig;
          asmLibraryFunctions.push(contentText);
          contentText = ' ';
          EXPORTED_FUNCTIONS[finalName] = 1;
          Functions.libraryFunctions[finalName] = 2;
        }
      }
      if (SIDE_MODULE) return ';'; // we import into the side module js library stuff from the outside parent 
      if ((!ASM_JS || phase == 'pre' || phase == 'glue') &&
          (EXPORT_ALL || (finalName in EXPORTED_FUNCTIONS))) {
        contentText += '\nModule["' + finalName + '"] = ' + finalName + ';';
      }
      return depsText + contentText;
    }

    itemsDict.functionStub.push(item);
    if (IGNORED_FUNCTIONS.indexOf(item.ident) >= 0) return;
    var shortident = item.ident.substr(1);
    if (BUILD_AS_SHARED_LIB) {
      // Shared libraries reuse the runtime of their parents.
      item.JS = '';
    } else {
      // If this is not linkable, anything not in the library is definitely missing
      if (item.ident in DEAD_FUNCTIONS) {
        if (LibraryManager.library[shortident + '__asm']) {
          warn('cannot kill asm library function ' + item.ident);
        } else {
          LibraryManager.library[shortident] = new Function("Module['printErr']('dead function: " + shortident + "'); abort(-1);");
          delete LibraryManager.library[shortident + '__inline'];
          delete LibraryManager.library[shortident + '__deps'];
        }
      }
      item.JS = addFromLibrary(shortident, true);
    }
  }

  // function splitter
  function functionSplitter(item) {
    item.lines.forEach(function(line) {
      //B.start('jsifier-handle-' + line.intertype);
      Framework.currItem = line;
      line.funcData = item; // TODO: remove all these, access it globally
      switch (line.intertype) {
        case 'value': line.JS = valueHandler(line); break;
        case 'noop': line.JS = noopHandler(line); break;
        case 'var': line.JS = varHandler(line); break;
        case 'store': line.JS = storeHandler(line); break;
        case 'deleted': line.JS = deletedHandler(line); break;
        case 'branch': line.JS = branchHandler(line); break;
        case 'switch': line.JS = switchHandler(line); break;
        case 'return': line.JS = returnHandler(line); break;
        case 'resume': line.JS = resumeHandler(line); break;
        case 'invoke': line.JS = invokeHandler(line); break;
        case 'atomic': line.JS = atomicHandler(line); break;
        case 'landingpad': line.JS = landingpadHandler(line); break;
        case 'load': line.JS = loadHandler(line); break;
        case 'extractvalue': line.JS = extractvalueHandler(line); break;
        case 'insertvalue': line.JS = insertvalueHandler(line); break;
        case 'insertelement': line.JS = insertelementHandler(line); break;
        case 'extracttelement': line.JS = extractelementHandler(line); break;
        case 'shufflevector': line.JS = shufflevectorHandler(line); break;
        case 'indirectbr': line.JS = indirectbrHandler(line); break;
        case 'alloca': line.JS = allocaHandler(line); break;
        case 'va_arg': line.JS = va_argHandler(line); break;
        case 'mathop': line.JS = mathopHandler(line); break;
        case 'bitcast': line.JS = bitcastHandler(line); break;
        case 'getelementptr': line.JS = getelementptrHandler(line); break;
        case 'call': line.JS = callHandler(line); break;
        case 'unreachable': line.JS = unreachableHandler(line); break;
        default: throw 'what is this line? ' + dump(line);
      }
      //if (ASM_JS) assert(line.JS.indexOf('var ') < 0, dump(line));
      if (line.assignTo) makeAssign(line);
      Framework.currItem = null;
      //B.stop('jsifier-handle-' + line.intertype);
    });
    //B.start('jsifier-frec');
    functionReconstructor(item);
    //B.stop('jsifier-frec');
  }

  // function for filtering functions for label debugging
  if (LABEL_FUNCTION_FILTERS.length > 0) {
    var LABEL_FUNCTION_FILTER_SET = set(LABEL_FUNCTION_FILTERS);
    var finalNameFilterTest = function(ident) {
      return (ident in LABEL_FUNCTION_FILTER_SET);
    };
  } else {
    // no filters are specified, all function names are printed
    var finalNameFilterTest = function(ident) {
      return true;
    }
  }

  // function reconstructor & post-JS optimizer
  function functionReconstructor(func) {
    // We have this function all reconstructed, go and finalize it's JS!

    if (IGNORED_FUNCTIONS.indexOf(func.ident) >= 0) return null;

    func.JS = '\n';

    var paramIdents = func.params.map(function(param) {
        return toNiceIdent(param.ident);
    });

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
    
    if (DLOPEN_SUPPORT) Functions.getIndex(func.ident);

    func.JS += 'function ' + func.ident + '(' + paramIdents.join(',') + '){\n';

    if (PGO) {
      func.JS += INDENTATION + 'PGOMonitor.called["' + func.ident + '"] = 1;\n';
    }

    if (ASM_JS) {
      // spell out argument types
      func.params.forEach(function(param) {
        func.JS += INDENTATION + param.ident + '=' + deParen(asmCoercion(param.ident, param.type)) + ';\n';
      });

      addVariable('label', 'i32', func);

      if (func.setjmpTable) {
        addVariable('setjmpLabel', 'i32', func);
        addVariable('setjmpTable', 'i32', func);
      }

      // spell out local variables
      var vars = values(func.variables).filter(function(v) {
        return v.origin !== 'funcparam' &&
               (!isIllegalType(getImplementationType(v)) || v.ident.indexOf('$', 1) > 0); // not illegal, or a broken up illegal (we have illegal chunks explicitly anyhow)
      });
      if (vars.length > 0) {
        var chunkSize = 20;
        var chunks = [];
        var i = 0;
        while (i < vars.length) {
          chunks.push(vars.slice(i, i+chunkSize));
          i += chunkSize;
        }
        for (i = 0; i < chunks.length; i++) {
          func.JS += INDENTATION + 'var ' + chunks[i].map(function(v) {
            return v.ident + '=' + asmInitializer(getImplementationType(v)); //, func.variables[v.ident].impl);
          }).join(',') + ';\n';
        }
      }
    }

    if (!ASM_JS) {
      if (CLOSURE_ANNOTATIONS) func.JS += '/** @type {number} */';
      func.JS += INDENTATION + 'var label=0;\n';
    }

    if (ASM_JS) {
      var hasByVal = false;
      func.params.forEach(function(param) {
        hasByVal = hasByVal || param.byVal;
      });
      if (hasByVal) {
        func.JS += INDENTATION + 'var tempParam=0;\n';
      }
    }

    if (func.hasVarArgsCall) {
      func.JS += INDENTATION + 'var tempVarArgs=0;\n';
    }

    // Prepare the stack, if we need one. If we have other stack allocations, force the stack to be set up.
    func.JS += INDENTATION + RuntimeGenerator.stackEnter(func.initialStack, func.otherStackAllocations) + ';\n';

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
        func.JS += INDENTATION + (ASM_JS ? '' : 'var ') + 'tempParam = ' + param.ident + '; ' + param.ident + '=' + RuntimeGenerator.stackAlloc(typeInfo.flatSize) + ';' +
                   makeCopyValues(param.ident, 'tempParam', typeInfo.flatSize, 'null', null, param.byVal) + ';\n';
      }
    });

    if (LABEL_DEBUG && finalNameFilterTest(func.ident)) func.JS += "  Module.print(INDENT + ' Entering: " + func.ident + ": ' + Array.prototype.slice.call(arguments)); INDENT += '  ';\n";

    // Walk function blocks and generate JS
    function walkBlock(block, indent) {
      if (!block) return '';
      dprint('relooping', 'walking block: ' + block.type + ',' + block.entries + ' : ' + block.labels.length);
      function getLabelLines(label, relooping) {
        if (!label) return '';
        var ret = '';
        if ((LABEL_DEBUG >= 2) && finalNameFilterTest(func.ident)) {
          ret += INDENTATION + "Module.print(INDENT + '" + func.ident + ":" + label.ident + "');\n";
        }
        if (EXECUTION_TIMEOUT > 0) {
          ret += INDENTATION + 'if (Date.now() - START_TIME >= ' + (EXECUTION_TIMEOUT*1000) + ') throw "Timed out!" + (new Error().stack);\n';
        }
        
        if (PRINT_SPLIT_FILE_MARKER && Debugging.on && Debugging.getAssociatedSourceFile(label.lines[label.lines.length-1].lineNum)) {
          // Overwrite the associated source file for every line. The last line should contain the source file associated to
          // the return value/address of outer most block (the marked function).
          associatedSourceFile = Debugging.getAssociatedSourceFile(label.lines[label.lines.length-1].lineNum);
        }
        
        // for special labels we care about (for phi), mark that we visited them
        var i = 0;
        return ret + label.lines.map(function(line) {
          var JS = line.JS;
          if (!relooping) JS = INDENTATION + JS;
          if (relooping && i == label.lines.length-1) {
            if (line.intertype == 'branch' || line.intertype == 'switch') {
              JS = ''; // just branching operations - done in the relooper, so nothing need be done here
            } else if (line.intertype == 'invoke') {
              JS = line.reloopingJS; // invokes have code that is not rendered in the relooper (the call inside a try-catch)
            }
          }
          i++;
          // invoke instructions span two lines, and the debug info is located
          // on the second line, hence the +1
          if (Debugging.on) JS += Debugging.getComment(line.lineNum + (line.intertype === 'invoke' ? 1 : 0));
          //assert(JS.indexOf('\n') < 0, JS);
          return JS;
        }).join('\n');
      }
      var ret = '';
      if (!RELOOP || func.forceEmulated) { // TODO: also if just 1 label?
        if (block.labels.length > 1) {
          if (block.entries.length == 1) {
            ret += indent + 'label = ' + getLabelId(block.entries[0]) + '; ' + (SHOW_LABELS ? '/* ' + getOriginalLabelId(block.entries[0]) + ' */' : '') + '\n';
          } // otherwise, should have been set before!
          if (func.setjmpTable) {
            if (!ASM_JS) {
              var setjmpTable = {};
              ret += indent + 'var mySetjmpIds = {};\n';
              ret += indent + 'var setjmpTable = {';
              func.setjmpTable.forEach(function(triple) { // original label, label we created for right after the setjmp, variable setjmp result goes into
                ret += '"' + getLabelId(triple.oldLabel) + '": ' + 'function(value) { label = ' + getLabelId(triple.newLabel) + '; ' + triple.assignTo + ' = value },';
              });
              ret += 'dummy: 0';
              ret += '};\n';
            } else {
              ret += makeVarDef('setjmpLabel') + '=0;\n';
              ret += makeVarDef('setjmpTable') + '=' + RuntimeGenerator.stackAlloc(4 * (MAX_SETJMPS + 1) * 2) + ';\n';
              ret += makeSetValue('setjmpTable', '0', '0', 'i32') + ';'; // initialize first entry to 0
            }
          }
          ret += indent + 'while(1)';
          if (func.setjmpTable && !ASM_JS) {
            ret += 'try { ';
          }
          ret += 'switch(' + asmCoercion('label', 'i32') + '){\n';
          ret += block.labels.map(function(label) {
            return INDENTATION + 'case ' + getLabelId(label.ident) + ': ' + (SHOW_LABELS ? '// ' + getOriginalLabelId(label.ident) : '') + '\n'
                          + getLabelLines(label);
          }).join('\n') + '\n';
          if (func.setjmpTable && ASM_JS) {
            // emit a label in which we write to the proper local variable, before jumping to the actual label
            ret += INDENTATION + 'case ' + SETJMP_LABEL + ': ';
            ret += func.setjmpTable.map(function(triple) { // original label, label we created for right after the setjmp, variable setjmp result goes into
              return 'if ((setjmpLabel|0) == ' + getLabelId(triple.oldLabel) + ') { ' + triple.assignTo + ' = threwValue; label = ' + triple.newLabel + ' }\n';
            }).join(' else ');
            if (ASSERTIONS) ret += 'else abort(-3);\n';
            ret += '__THREW__ = threwValue = 0;\n';
            ret += 'break;\n';
          }
          if (ASSERTIONS) ret += indent + INDENTATION + 'default: assert(0' + (ASM_JS ? '' : ', "bad label: " + label') + ');\n';
          ret += indent + '}\n';
          if (func.setjmpTable && !ASM_JS) {
            ret += ' } catch(e) { if (!e.longjmp || !(e.id in mySetjmpIds)) throw(e); setjmpTable[setjmpLabels[e.id]](e.value) }';
          }
          if (ASM_JS && func.returnType !== 'void') ret += '  return ' + asmInitializer(func.returnType) + ';\n'; // Add a return
        } else {
          ret += (SHOW_LABELS ? indent + '/* ' + block.entries[0] + ' */' : '') + '\n' + getLabelLines(block.labels[0]);
        }
        ret += '\n';
      } else {
        // Reloop multiple blocks using the compiled relooper
        //B.start('jsifier-reloop');

        //Relooper.setDebug(1);
        Relooper.init();

        if (ASM_JS) Relooper.setAsmJSMode(1);

        var blockMap = {};
        // add blocks
        for (var i = 0; i < block.labels.length; i++) {
          var label = block.labels[i];
          var content = getLabelLines(label, true);
          //printErr(func.ident + ' : ' + label.ident + ' : ' + content + '\n');
          var last = label.lines[label.lines.length-1];
          if (!last.signedIdent) {
            blockMap[label.ident] = Relooper.addBlock(content);
          } else {
            assert(last.intertype == 'switch');
            blockMap[label.ident] = Relooper.addBlock(content, last.signedIdent);
          }
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
        Relooper.cleanup();
        //B.stop('jsifier-reloop');
      }
      return ret;
    }
    func.JS += walkBlock(func.block, INDENTATION);
    // Finalize function
    if (LABEL_DEBUG && finalNameFilterTest(func.ident)) func.JS += "  INDENT = INDENT.substr(0, INDENT.length-2);\n";
    // Ensure a return in a function with a type that returns, even if it lacks a return (e.g., if it aborts())
    if (RELOOP && ASM_JS && func.lines.length > 0 && func.returnType != 'void') {
      var lastCurly = func.JS.lastIndexOf('}');
      var lastReturn = func.JS.lastIndexOf('return ');
      if ((lastCurly < 0 && lastReturn < 0) || // no control flow, no return
          (lastCurly >= 0 && lastReturn < lastCurly)) { // control flow, no return past last join
        func.JS += '  return ' + asmInitializer(func.returnType) + ';\n';
      }
    }
    func.JS += '}\n';
    
    if (PRINT_SPLIT_FILE_MARKER) {
        func.JS += '\n//FUNCTION_END_MARKER_OF_SOURCE_FILE_' + associatedSourceFile + '\n';
    }

    if (!ASM_JS && (EXPORT_ALL || (func.ident in EXPORTED_FUNCTIONS))) {
      func.JS += 'Module["' + func.ident + '"] = ' + func.ident + ';';
    }

    if (!ASM_JS && INLINING_LIMIT && func.lines.length >= INLINING_LIMIT) {
      func.JS += func.ident + '["X"]=1;';
    }

    if (BUILD_AS_SHARED_LIB == 2) {
      // TODO: make the assert conditional on ASSERTIONS
      func.JS += 'if (globalScope) { assert(!globalScope["' + func.finalName + '"]); globalScope["' + func.finalName + '"] = ' + func.finalName + ' }';
    }

    func.JS = func.JS.replace(/\n *;/g, '\n'); // remove unneeded lines

    if (MAIN_MODULE || SIDE_MODULE) {
      // Clone the function for each of its aliases. We do not know which name it will be used by in another module,
      // and we do not have a heavyweight metadata system to resolve aliases during linking
      var aliases = Functions.aliases[func.ident];
      if (aliases) {
        var body = func.JS.substr(func.JS.indexOf('('));
        aliases.forEach(function(alias) {
          func.JS += '\n' + 'function ' + alias + body;
        });
      }
    }
    itemsDict.function.push(func);
  }

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

  // An interitem that has |assignTo| is an assign to that item. They call this function which
  // generates the actual assignment.
  function makeAssign(item) {
    var valueJS = item.JS;
    item.JS = '';
    if (!ASM_JS || item.intertype != 'alloca' || item.funcData.variables[item.assignTo].impl == VAR_EMULATED) { // asm only needs non-allocas
      if (CLOSURE_ANNOTATIONS) item.JS += '/** @type {number} */ ';
      item.JS += ((ASM_JS || item.overrideSSA) ? '' : 'var ') + toNiceIdent(item.assignTo);
    }
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
  function valueHandler(item) {
    if (item.vars) {
      item.vars.forEach(function(v) {
        addVariable(v[0], v[1]);
      });
    }
    return item.ident;
  }
  function noopHandler(item) {
    return ';';
  }
  function varHandler(item) { // assigns into phis become simple vars
    return ASM_JS ? ';' : ('var ' + item.ident + ';');
  }
  function storeHandler(item) {
    var value = finalizeLLVMParameter(item.value);
    if (pointingLevels(item.pointerType) == 1) {
      value = parseNumerical(value, item.valueType);
    }
    var impl = VAR_EMULATED;
    if (item.pointer.intertype == 'value') {
      impl = getVarImpl(item.funcData, item.ident);
    }
    if (item.valueType[item.valueType.length-1] === '>') {
      // vector store TODO: move to makeSetValue?
      var native = getVectorNativeType(item.valueType);
      var base = getSIMDName(native);
      return '(' + makeSetValue(item.ident,  0, value + '.x', native, 0, 0, item.align) + ',' +
                   makeSetValue(item.ident,  4, value + '.y', native, 0, 0, item.align) + ',' +
                   makeSetValue(item.ident,  8, value + '.z', native, 0, 0, item.align) + ',' +
                   makeSetValue(item.ident, 12, value + '.w', native, 0, 0, item.align) + ');';
    }
    switch (impl) {
      case VAR_NATIVIZED:
        if (isNumber(item.ident)) {
          // Direct write to a memory address; this may be an intentional segfault, if not, it is a bug in the source
          if (ASM_JS) {
            return 'abort(' + item.ident + ')';
          } else {
            return 'throw "fault on write to ' + item.ident + '";';
          }
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
  }

  function deletedHandler(item) { return ';' }

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
      var labelSetting = oldLabel ? 'label=' + getLabelId(oldLabel) + ';' +
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
      return pre + 'label=' + label + ';' + (SHOW_LABELS ? ' /* to: ' + getOriginalLabelId(cleanLabel(label)) + ' */' : '') + 'break;';
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
      return (ASM_JS ? '' : 'var ') + labelSets[0].ident + '=' + labelSets[0].valueJS + ';';
    }
    // TODO: eliminate unneeded sets (to undefined etc.)
    var deps = {}; // for each ident we will set, which others it depends on
    var map = {};
    labelSets.forEach(function(labelSet) {
      deps[labelSet.ident] = {};
      map[labelSet.ident] = labelSet;
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
          post = idents[i] + '=' + map[idents[i]].valueJS + ';' + post;
          if (!ASM_JS) post = 'var ' + post;
          else addVariable(idents[i], map[idents[i]].value.type);
          remove(idents[i]);
          continue mainLoop;
        }
      }
      // If we got here, we have circular dependencies, and must break at least one.
      pre += makeVarDef(idents[0]) + '$phi=' + map[idents[0]].valueJS + ';';
      post += makeVarDef(idents[0]) + '=' + idents[0] + '$phi;';
      addVariable(idents[0] + '$phi', map[idents[0]].value.type);
      addVariable(idents[0], map[idents[0]].value.type);
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

  function branchHandler(item) {
    var phiSets = calcPhiSets(item);
    if (!item.value) {
      return (item.labelJS = getPhiSetsForLabel(phiSets, item.label)) + makeBranch(item.label, item.currLabelId);
    } else {
      var condition = item.valueJS = finalizeLLVMParameter(item.value);
      var labelTrue = (item.labelTrueJS = getPhiSetsForLabel(phiSets, item.labelTrue)) + makeBranch(item.labelTrue, item.currLabelId);
      var labelFalse = (item.labelFalseJS = getPhiSetsForLabel(phiSets, item.labelFalse)) + makeBranch(item.labelFalse, item.currLabelId);
      if (labelTrue == ';' && labelFalse == ';') return ';';
      var head = 'if(' + condition + '){';
      var head2 = 'if(!(' + condition + ')){';
      var else_ = '}else{';
      var tail = '}';
      if (labelTrue == ';') {
        return head2 + labelFalse + tail;
      } else if (labelFalse == ';') {
        return head + labelTrue + tail;
      } else {
        return head + labelTrue + else_ + labelFalse + tail;
      }
    }
  }
  function switchHandler(item) {
    // use a switch if the range is not too big or sparse
    var minn = Infinity, maxx = -Infinity;
    item.switchLabels.forEach(function(switchLabel) {
      var curr = Math.abs(parseInt(switchLabel.value));
      minn = Math.min(minn, curr);
      maxx = Math.max(maxx, curr);
    });
    var range = maxx - minn;
    var useIfs = (item.switchLabels.length+1) < 6 || range > 10*1024 || (range/item.switchLabels.length) > 1024; // heuristics
    if (VERBOSE && useIfs && item.switchLabels.length >= 6) {
      warn('not optimizing llvm switch into js switch because range of values is ' + range + ', density is ' + range/item.switchLabels.length);
    }
    if (!useIfs && isIllegalType(item.type)) {
      useIfs = true;
      if (VERBOSE) warn('not optimizing llvm switch because illegal type ' + item.type);
    }

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
    signedIdent = makeSignOp(item.ident, item.type, 're'); // we need to standardize for purpose of comparison
    if (!useIfs) item.signedIdent = signedIdent;
    if (RELOOP) {
      item.groupedLabels = [];
    }
    if (!useIfs) {
      ret += 'switch(' + signedIdent + '){';
    }
    // process target labels, sorting them so output is consistently ordered
    keys(targetLabels).sort().forEach(function(targetLabel) {
      if (!first && useIfs) {
        ret += 'else ';
      } else {
        first = false;
      }
      var value;
      if (useIfs) {
        value = targetLabels[targetLabel].map(function(value) {
          return makeComparison(signedIdent, '==', makeSignOp(value, item.type, 're'), item.type)
        }).join('|');
        ret += 'if(' + value + '){';
      } else {
        value = targetLabels[targetLabel].map(function(value) {
          return 'case ' + makeSignOp(value, item.type, 're') + ':';
        }).join('');
        ret += value + '{';
      }
      var phiSet = getPhiSetsForLabel(phiSets, targetLabel);
      ret += INDENTATION + '' + phiSet + makeBranch(targetLabel, item.currLabelId || null);
      ret += '}';
      if (RELOOP) {
        item.groupedLabels.push({
          label: targetLabel,
          value: value,
          labelJS: phiSet
        });
      }
    });
    var phiSet = item.defaultLabelJS = getPhiSetsForLabel(phiSets, item.defaultLabel);
    if (useIfs) {
      if (item.switchLabels.length > 0) ret += 'else{';
      ret += phiSet + makeBranch(item.defaultLabel, item.currLabelId) + '';
      if (item.switchLabels.length > 0) ret += '}';
    } else {
      ret += 'default:{';
      ret += phiSet + makeBranch(item.defaultLabel, item.currLabelId) + '';
      ret += '}';

      ret += '}break;'; // finish switch and break, to move control flow properly (breaks from makeBranch just broke out of the switch)
    }
    if (item.value) {
      ret += ' ' + toNiceIdent(item.value);
    }
    return ret;
  }
  function returnHandler(item) {
    var ret = RuntimeGenerator.stackExit(item.funcData.initialStack, item.funcData.otherStackAllocations);
    if (ret.length > 0) ret += ';';
    if (LABEL_DEBUG && finalNameFilterTest(item.funcData.ident)) {
      ret += "Module.print(INDENT + 'Exiting: " + item.funcData.ident + "');"
          +  "INDENT = INDENT.substr(0, INDENT.length-2);";
    }
    ret += 'return';
    var value = item.value ? finalizeLLVMParameter(item.value) : null;
    if (!value && item.funcData.returnType != 'void') value = '0'; // no-value returns must become value returns if function returns
    if (value) {
      ret += ' ' + asmCoercion(value, item.type);
    }
    return ret + ';';
  }
  function resumeHandler(item) {
    if (DISABLE_EXCEPTION_CATCHING && !(item.funcData.ident in EXCEPTION_CATCHING_WHITELIST)) return 'abort()';
    if (item.ident == 0) {
      // No exception to resume, so we can just bail.
      // This is related to issue #917 and http://llvm.org/PR15518
      return (EXCEPTION_DEBUG ? 'Module.print("no exception to resume")' : '') + ';';
    }
    // If there is no current exception, set this one as it (during a resume, the current exception can be wiped out)
    var ptr = makeStructuralAccess(item.ident, 0);
    return '___resumeException(' + asmCoercion(ptr, 'i32') + ')';
  }
  function invokeHandler(item) {
    // Wrapping in a function lets us easily return values if we are
    // in an assignment
    var disabled = DISABLE_EXCEPTION_CATCHING == 2  && !(item.funcData.ident in EXCEPTION_CATCHING_WHITELIST); 
    var phiSets = calcPhiSets(item);
    var call_ = makeFunctionCall(item, item.params, item.funcData, item.type, ASM_JS && !disabled, !!item.assignTo || !item.standalone, !disabled);

    var ret;

    if (disabled) {
      ret = call_ + ';';
    } else if (ASM_JS) {
      if (item.type != 'void') call_ = asmCoercion(call_, item.type); // ensure coercion to ffi in comma operator
      call_ = call_.replace('; return', ''); // we auto-add returns when aborting, but do not need them here
      if (item.type == 'void') {
        ret = '__THREW__ = 0;' +  call_ + ';';
      } else {
        ret = '(__THREW__ = 0,' +  call_ + ');';
      }
    } else {
      ret = '(function() { try { __THREW__ = 0; return '
          + call_ + ' '
          + '} catch(e) { '
          + 'if (typeof e != "number") throw e; '
          + 'if (ABORT) throw e; __THREW__ = 1; '
          + (EXCEPTION_DEBUG ? 'Module.print("Exception: " + e + ", currently at: " + (new Error().stack)); ' : '')
          + 'return null } })();';
    }
    ret = makeVarArgsCleanup(ret);

    if (item.assignTo) {
      var illegal = USE_TYPED_ARRAYS == 2 && isIllegalType(item.type);
      var assignTo = illegal ? item.assignTo + '$r' : item.assignTo;
      ret = makeVarDef(assignTo) + '=' + ret;
      if (ASM_JS) addVariable(assignTo, item.type);
      if (illegal) {
        var bits = getBits(item.type);
        for (var i = 0; i < bits/32; i++) {
          var v = item.assignTo + '$' + i;
          ret += makeVarDef(v) + '=' + (i == 0 ? assignTo : 'tempRet' + (i-1)) + ';'
          if (ASM_JS) addVariable(v, 'i32');
        }
      }
      item.assignTo = null;
    }
    item.reloopingJS = ret; // everything but the actual branching (which the relooper will do for us)
    item.toLabelJS = getPhiSetsForLabel(phiSets, item.toLabel);
    item.unwindLabelJS = (ASM_JS ? '__THREW__ = 0;' : '') + getPhiSetsForLabel(phiSets, item.unwindLabel);
    ret += 'if (!__THREW__) { ' + item.toLabelJS + makeBranch(item.toLabel, item.currLabelId)
            + ' } else { ' + item.unwindLabelJS + makeBranch(item.unwindLabel, item.currLabelId) + ' }';
    return ret;
  }
  function atomicHandler(item) {
    var type = item.params[0].type;
    var param1 = finalizeLLVMParameter(item.params[0]);
    var param2 = finalizeLLVMParameter(item.params[1]);
    switch (item.op) {
      case 'add': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, asmCoercion('tempValue+' + param2, type), type, null, null, null, null, ',') + ',tempValue)';
      case 'sub': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, asmCoercion('tempValue-' + param2, type), type, null, null, null, null, ',') + ',tempValue)';
      case 'or': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue|' + param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'and': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue&' + param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'xor': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, 'tempValue^' + param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'xchg': return '(tempValue=' + makeGetValue(param1, 0, type) + ',' + makeSetValue(param1, 0, param2, type, null, null, null, null, ',') + ',tempValue)';
      case 'cmpxchg': {
        var param3 = finalizeLLVMParameter(item.params[2]);
        return '(tempValue=' + makeGetValue(param1, 0, type) + ',(' + makeGetValue(param1, 0, type) + '==(' + param2 + '|0) ? ' + asmCoercion(makeSetValue(param1, 0, param3, type, null, null, null, null, ','), 'i32') + ' : 0),tempValue)';
      }
      default: throw 'unhandled atomic op: ' + item.op;
    }
  }
  function landingpadHandler(item) {
    if (ASM_JS) {
      addVariable(item.assignTo + '$0', 'i32');
      addVariable(item.assignTo + '$1', 'i32');
    }
    if (DISABLE_EXCEPTION_CATCHING && !(item.funcData.ident in EXCEPTION_CATCHING_WHITELIST) && USE_TYPED_ARRAYS == 2) {
      ret = makeVarDef(item.assignTo) + '$0 = 0; ' + makeVarDef(item.assignTo) + '$1 = 0;';
      item.assignTo = null;
      if (VERBOSE) warnOnce('landingpad, but exceptions are disabled!');
      return ret;
    }
    var catchTypeArray = item.catchables.map(finalizeLLVMParameter).map(function(element) { return asmCoercion(element, 'i32') }).join(',');
    var ret = asmCoercion('___cxa_find_matching_catch(' + catchTypeArray +')', 'i32');
    if (USE_TYPED_ARRAYS == 2) {
      ret = makeVarDef(item.assignTo) + '$0 = ' + ret + '; ' + makeVarDef(item.assignTo) + '$1 = tempRet0;';
      item.assignTo = null;
    }
    return ret;
  }
  function loadHandler(item) {
    var value = finalizeLLVMParameter(item.pointer);
    if (item.valueType[item.valueType.length-1] === '>') {
      // vector load
      var native = getVectorNativeType(item.valueType);
      var base = getSIMDName(native);
      return 'SIMD.' + base + '32x4(' + makeGetValue(value,  0, native, 0, item.unsigned, 0, item.align) + ',' +
                                        makeGetValue(value,  4, native, 0, item.unsigned, 0, item.align) + ',' +
                                        makeGetValue(value,  8, native, 0, item.unsigned, 0, item.align) + ',' +
                                        makeGetValue(value, 12, native, 0, item.unsigned, 0, item.align) + ');';
    }
    var impl = item.ident ? getVarImpl(item.funcData, item.ident) : VAR_EMULATED;
    switch (impl) {
      case VAR_NATIVIZED: {
        if (isNumber(item.ident)) {
          // Direct read from a memory address; this may be an intentional segfault, if not, it is a bug in the source
          if (ASM_JS) {
            return asmFFICoercion('abort(' + item.ident + ')', item.type);
          } else {
            item.assignTo = null;
            return 'throw "fault on read from ' + item.ident + '";';
          }
        }
        return value; // We have the actual value here
      }
      case VAR_EMULATED: return makeGetValue(value, 0, item.type, 0, item.unsigned, 0, item.align);
      default: throw "unknown [load] impl: " + impl;
    }
  }
  function extractvalueHandler(item) {
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
  }
  function insertvalueHandler(item) {
    assert(item.indexes.length == 1); // TODO: see extractvalue
    var ret = '(', ident;
    if (item.ident === '0') {
      item.ident = 'tempValue';
      ret += item.ident + ' = [' + makeEmptyStruct(item.type) + '], ';
    }
    return ret + item.ident + '.f' + item.indexes[0][0].text + '=' + finalizeLLVMParameter(item.value) + ', ' + item.ident + ')';
  }
  function insertelementHandler(item) {
    var base = getVectorBaseType(item.type);
    var ident = ensureVector(item.ident, base);
    var laneOp = ((base == 'float') ? 'SIMD.float32x4.with' : 'SIMD.int32x4.with');
    //return ident + '.with' + SIMDLane[finalizeLLVMParameter(item.index)] + '(' + finalizeLLVMParameter(item.value) + ')';
    return laneOp + SIMDLane[finalizeLLVMParameter(item.index)] + '(' + ident + ',' + finalizeLLVMParameter(item.value) + ')';
  }
  function extractelementHandler(item) {
    var base = getVectorBaseType(item.type);
    var ident = ensureVector(item.ident, base);
    var index = finalizeLLVMParameter(item.value);
    assert(isNumber(index));
    return ident + '.' + simdLane[index];
  }
  function shufflevectorHandler(item) {
    var base = getVectorBaseType(item.type);
    var first = ensureVector(item.ident, base);
    var second = ensureVector(finalizeLLVMParameter(item.value), base);
    var mask;
    if (item.mask.intertype === 'value') {
      assert(item.mask.ident === 'zeroinitializer');
      mask = [0, 0, 0, 0];
    } else {
      assert(item.mask.intertype === 'vector');
      mask = item.mask.idents;
    }
    for (var i = 0; i < 4; i++) assert(mask[0] == 0 || mask == 1);
    i = 0;
    return 'SIMD.' + base + '32x4(' + mask.map(function(m) {
      return (m == 1 ? second : first) + '.' + simdLane[i++];
    }).join(',') + ')';
  }
  function indirectbrHandler(item) {
    var phiSets = calcPhiSets(item);
    var js = makeVarDef('ibr') + '=' + finalizeLLVMParameter(item.value) + ';';
    if (ASM_JS) addVariable('ibr', 'i32');
    for (var targetLabel in phiSets) {
      js += 'if(' + makeComparison('ibr', '==', targetLabel, 'i32') + '){' + getPhiSetsForLabel(phiSets, targetLabel) + '}';
    }
    return js + makeBranch('ibr', item.currLabelId, true);
  }
  function allocaHandler(item) {
    if (typeof item.allocatedIndex === 'number') {
      if (item.allocatedSize === 0) return ''; // This will not actually be shown - it's nativized
      return asmCoercion(getFastValue('sp', '+', item.allocatedIndex.toString()), 'i32');
    } else {
      return RuntimeGenerator.stackAlloc(getFastValue(calcAllocatedSize(item.allocatedType), '*', item.ident));
    }
  }
  function va_argHandler(item) {
    assert(TARGET_ASMJS_UNKNOWN_EMSCRIPTEN);
    var ident = item.value.ident;
    var move = Runtime.STACK_ALIGN;
    
    // store current list offset in tempInt, advance list offset by STACK_ALIGN, return list entry stored at tempInt
    return '(tempInt=' + makeGetValue(ident, Runtime.QUANTUM_SIZE, '*') + ',' +
                         makeSetValue(ident, Runtime.QUANTUM_SIZE, asmCoercion('tempInt + ' + move, 'i32'), '*', null, null, null, null, ',') + ',' +
                         makeGetValue(makeGetValue(ident, 0, '*'), 'tempInt', item.type) + ')';
  }

  var mathopHandler = processMathop;

  function bitcastHandler(item) {
    var temp = {
      op: 'bitcast', variant: null, type: item.type,
      assignTo: item.assignTo,
      params: [item.params[0]] // XXX
    };
    var ret = processMathop(temp);
    if (!temp.assignTo) item.assignTo = null; // If the assign was stolen, propagate that
    return ret;
  }

  function makeFunctionCall(item, params, funcData, type, forceByPointer, hasReturn, invoke) {
    var ident = item.ident;

    // We cannot compile assembly. See comment in intertyper.js:'Call'
    assert(ident != 'asm', 'Inline assembly cannot be compiled to JavaScript!');

    var extCall = false;

    if (ASM_JS && funcData.setjmpTable) forceByPointer = true; // in asm.js mode, we must do an invoke for each call
    if (ASM_JS && DLOPEN_SUPPORT && !invoke && !funcData.setjmpTable) extCall = true; // go out, to be able to access other modules TODO: optimize

    ident = Variables.resolveAliasToIdent(ident);
    var shortident = ident.slice(1);
    var simpleIdent = shortident;
    if (isLocalVar(ident)) {
      var callIdent = ident;
    } else {
      // Not a local var, check if in library
      var callIdent = LibraryManager.getRootIdent(simpleIdent);
      if (callIdent) {
        simpleIdent = callIdent; // ident may not be in library, if all there is is ident__inline, but in this case it is
        if ((callIdent.indexOf('Math_') !== 0) && (callIdent[0] != '$')) {
          callIdent = '_' + callIdent; // Not Math.*, so add the normal prefix
        }
      } else {
        callIdent = ident;
      }
      if (callIdent == '0') return 'abort(-2)';
    }

    var args = [];
    var argsTypes = [];
    var varargs = [];
    var varargsTypes = [];
    var varargsByVals = {};
    var ignoreFunctionIndexizing = [];
    var useJSArgs = (simpleIdent + '__jsargs') in LibraryManager.library;
    var hasVarArgs = isVarArgsFunctionType(type);
    var normalArgs = (hasVarArgs && !useJSArgs) ? countNormalArgs(type, null, true) : -1;
    var byPointer = getVarData(funcData, ident);
    var byPointerForced = false;

    if (forceByPointer && !byPointer) {
      byPointer = byPointerForced = true;
    }

    params.forEach(function(param, i) {
      var val = finalizeLLVMParameter(param);
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
        size = Runtime.alignMemory(size, Runtime.STACK_ALIGN);
        varargs.push(val);
        varargs = varargs.concat(zeros(size-1));
        // TODO: replace concats like this with push
        varargsTypes.push(param.type);
        varargsTypes = varargsTypes.concat(zeros(size-1));
      }
    });

    args = args.map(function(arg, i) { return indexizeFunctions(arg, argsTypes[i]) });
    if (ASM_JS) {
      var ffiCall = (shortident in Functions.libraryFunctions || simpleIdent in Functions.libraryFunctions || ident in Functions.libraryFunctions || byPointerForced || invoke || extCall || funcData.setjmpTable) &&
                    !(simpleIdent in JS_MATH_BUILTINS);
      if (ffiCall) {
        args = args.map(function(arg, i) { return asmCoercion(arg, ensureValidFFIType(argsTypes[i])) });
      } else {
        args = args.map(function(arg, i) { return asmEnsureFloat(arg, argsTypes[i]) });
      }
    }

    varargs = varargs.map(function(vararg, i) {
      if (ignoreFunctionIndexizing.indexOf(i) >= 0) return vararg;
      return vararg === 0 ? 0 : indexizeFunctions(vararg, varargsTypes[i])
    });

    if (hasVarArgs && !useJSArgs) {
      funcData.hasVarArgsCall = true;
      if (varargs.length === 0) {
        varargs = [0];
        varargsTypes = ['i32'];
      }
      var offset = 0;
      varargs = '(tempVarArgs=' + RuntimeGenerator.stackAlloc(varargs.length, ',') + ',' +
                varargs.map(function(arg, i) {
                  var type = varargsTypes[i];
                  if (type == 0) return null;
                  arg = asmEnsureFloat(arg, type);
                  var ret;
                  assert(offset % Runtime.STACK_ALIGN == 0); // varargs must be aligned
                  if (!varargsByVals[i]) {
                    ret = makeSetValue(getFastValue('tempVarArgs', '+', offset), 0, arg, type, null, null, Runtime.STACK_ALIGN, null, ',');
                    offset += Runtime.alignMemory(Runtime.getNativeFieldSize(type), Runtime.STACK_ALIGN);
                  } else {
                    var size = calcAllocatedSize(removeAllPointing(type));
                    ret = makeCopyValues(getFastValue('tempVarArgs', '+', offset), arg, size, null, null, varargsByVals[i], ',');
                    offset += RuntimeGenerator.forceAlign(size, Runtime.STACK_ALIGN);
                  }
                  return ret;
                }).filter(function(arg) {
                  return arg !== null;
                }).join(',') + ',tempVarArgs)';
      varargs = asmCoercion(varargs, 'i32');
    }

    args = args.concat(varargs);

    // Inline if either we inline whenever we can (and we can), or if there is no noninlined version
    var inline = LibraryManager.library[simpleIdent + '__inline'];
    var nonInlined = simpleIdent in LibraryManager.library;
    if (inline && (INLINE_LIBRARY_FUNCS || !nonInlined)) {
      return inline.apply(null, args); // Warning: inlining does not prevent recalculation of the arguments. They should be simple identifiers
    }

    if (ASM_JS && ident.indexOf('llvm_') >= 0) {
      // remove unneeded arguments in llvm intrinsic functions, which the asm sig can show us. this lets us alias memset with llvm.memset, we just
      // drop the final 2 args so things validate properly in asm
      var libsig = LibraryManager.library[simpleIdent + '__sig'];
      if (libsig) {
        assert(!hasVarArgs);
        while (libsig.length - 1 < args.length) {
          args.pop();
          argsTypes.pop();
        }
      }
    }

    if (callIdent in Functions.implementedFunctions) {
      // LLVM sometimes bitcasts for no reason. We must call using the exact same type as the actual function is generated as.
      var numArgs = Functions.implementedFunctions[callIdent].length - 1;
      if (numArgs !== args.length) {
        if (VERBOSE) warnOnce('Fixing function call arguments based on signature, on ' + [callIdent, args.length, numArgs]);
        while (args.length > numArgs) { args.pop(); argsTypes.pop() }
        while (args.length < numArgs) { args.push('0'); argsTypes.push('i32') }
      }
    }

    var returnType = 'void';
    if ((byPointer || ASM_JS) && hasReturn) {
      returnType = getReturnType(type);
      if (callIdent in Functions.implementedFunctions) {
        // LLVM sometimes bitcasts for no reason. We must call using the exact same type as the actual function is generated as
        var trueType = Functions.getSignatureType(Functions.implementedFunctions[callIdent][0]);
        if (trueType !== returnType && !isIdenticallyImplemented(trueType, returnType)) {
          if (VERBOSE) warnOnce('Fixing function call based on return type from signature, on ' + [callIdent, returnType, trueType]);
          returnType = trueType;
          if (trueType === 'void') {
            item.assignTo = null;
          }
        }
      }
    }

    // we alias llvm memset and such to normal memset. The target has a return value, while the original
    // does not, so we need to fix that for the actual call target
    if (ASM_JS) {
      var sig = LibraryManager.library[simpleIdent + '__sig'];
      if (sig && sig[0] !== 'v') {
        returnType = Functions.getSignatureType(sig[0]);
      }
    }

    if (byPointer) {
      var sig = Functions.getSignature(returnType, argsTypes, hasVarArgs);
      if (ASM_JS) {
        assert(returnType.search(/\("'\[,/) == -1); // XXX need isFunctionType(type, out)
        Functions.neededTables[sig] = 1;
        var functionTableCall = !byPointerForced && !funcData.setjmpTable && !invoke && !extCall;
        if (functionTableCall) {
          // normal asm function pointer call
          callIdent = '(' + callIdent + ')&{{{ FTM_' + sig + ' }}}'; // the function table mask is set in emscripten.py
        } else {
          // This is a call through an invoke_* or extCall, either a forced one, or a setjmp-required one
          // note: no need to update argsTypes at this point
          if (byPointerForced) Functions.unimplementedFunctions[callIdent] = sig;
          args.unshift(byPointerForced ? Functions.getIndex(callIdent, sig) : asmCoercion(callIdent, 'i32'));
          callIdent = (extCall ? 'extCall' : 'invoke') + '_' + sig;
        }
      } else if (SAFE_DYNCALLS) {
        assert(!ASM_JS, 'cannot emit safe dyncalls in asm');
        callIdent = '(tempInt=' + callIdent + ',tempInt < 0 || tempInt >= FUNCTION_TABLE.length-1 || !FUNCTION_TABLE[tempInt] ? abort("dyncall error: ' + sig + ' " + FUNCTION_TABLE_NAMES[tempInt]) : tempInt)';
      }
      if (!ASM_JS || functionTableCall) callIdent = Functions.getTable(sig) + '[' + callIdent + ']';
    }

    var ret = callIdent + '(' + args.join(',') + ')';
    if (ASM_JS) { // TODO: do only when needed (library functions and Math.*?) XXX && simpleIdent in Functions.libraryFunctions) {
      if (ffiCall) {
        ret = asmFFICoercion(ret, returnType);
      } else {
        ret = asmCoercion(ret, returnType);
      }
      if (simpleIdent == 'abort' && funcData.returnType != 'void') {
        ret += '; return ' + asmCoercion('0', funcData.returnType); // special case: abort() can happen without return, breaking the return type of asm functions. ensure a return
      }
    }

    if (ASM_JS && funcData.setjmpTable) {
      // check if a longjmp was done. If a setjmp happened, check if ours. If ours, go to a special label to handle it.
      // otherwise, just return - the call to us must also have been an invoke, so the setjmp propagates that way
      ret += '; if (((__THREW__|0) != 0) & ((threwValue|0) != 0)) { setjmpLabel = ' + asmCoercion('_testSetjmp(' + makeGetValue('__THREW__', 0, 'i32') + ', setjmpTable)', 'i32') + '; if ((setjmpLabel|0) > 0) { label = ' + SETJMP_LABEL + '; break } else return ' + (funcData.returnType != 'void' ? asmCoercion('0', funcData.returnType) : '') + ' } __THREW__ = threwValue = 0;';
    }

    return ret;
  }

  function makeVarArgsCleanup(js) {
    if (js.indexOf('(tempVarArgs=') >= 0) {
      if (js[js.length-1] == ';') {
        return js + ' STACKTOP=tempVarArgs;';
      } else {
        assert(js.indexOf(';') < 0);
        return '((' + js + '), STACKTOP=tempVarArgs)';
      }
    }
    return js;
  }

  function getelementptrHandler(item) { return finalizeLLVMFunctionCall(item) }
  function callHandler(item) {
    if (item.standalone && LibraryManager.isStubFunction(item.ident)) return ';';
    var ret = makeFunctionCall(item, item.params, item.funcData, item.type, false, !!item.assignTo || !item.standalone) + (item.standalone ? ';' : '');
    return makeVarArgsCleanup(ret);
  }

  function unreachableHandler(item) {
    var ret = '';
    if (ASM_JS && item.funcData.returnType != 'void') ret = 'return ' + asmCoercion('0', item.funcData.returnType) + ';';
    if (ASSERTIONS) {
      ret = (ASM_JS ? 'abort()' : 'throw "Reached an unreachable!"') + ';' + ret;
    }
    return ret || ';';
  }

  // Final combiner

  function finalCombiner() {
    dprint('unparsedFunctions', 'Starting finalCombiner');

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
      if ((phase == 'pre' || phase == 'glue') && !Variables.generatedGlobalBase && !BUILD_AS_SHARED_LIB) {
        Variables.generatedGlobalBase = true;
        // Globals are done, here is the rest of static memory
        assert(TARGET_ASMJS_UNKNOWN_EMSCRIPTEN || (TARGET_X86 && Runtime.GLOBAL_BASE == 4)); // this is assumed in e.g. relocations for linkable modules
        if (!SIDE_MODULE) {
          print('STATIC_BASE = ' + Runtime.GLOBAL_BASE + ';\n');
          print('STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n');
        } else {
          print('H_BASE = parentModule["_malloc"](' + Runtime.alignMemory(Variables.nextIndexedOffset) + ' + Runtime.GLOBAL_BASE);\n');
          print('// STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n'); // comment as metadata only
        }
      }
      var generated = itemsDict.function.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.GlobalVariable);
      print(generated.map(function(item) { return item.JS; }).join('\n'));

      if (phase == 'pre' || phase == 'glue') {
        if (memoryInitialization.length > 0) {
          // apply postsets directly into the big memory initialization
          itemsDict.GlobalVariablePostSet = itemsDict.GlobalVariablePostSet.filter(function(item) {
            var m;
            if (m = /^HEAP([\dFU]+)\[([()>\d]+)\] *= *([()|\d{}\w_' ]+);?$/.exec(item.JS)) {
              var type = getTypeFromHeap(m[1]);
              var bytes = Runtime.getNativeTypeSize(type);
              var target = eval(m[2]) << log2(bytes);
              var value = m[3];
              try {
                value = eval(value);
              } catch(e) {
                // possibly function table {{{ FT_* }}} etc.
                if (value.indexOf('{{ ') < 0) return true;
              }
              writeInt8s(memoryInitialization, target - Runtime.GLOBAL_BASE, value, type);
              return false;
            }
            return true;
          });
          // write out the singleton big memory initialization value
          print('/* memory initializer */ ' + makePointer(memoryInitialization, null, 'ALLOC_NONE', 'i8', 'Runtime.GLOBAL_BASE' + (SIDE_MODULE ? '+H_BASE' : ''), true));
        } else if (phase !== 'glue') {
          print('/* no memory initializer */'); // test purposes
        }

        if (phase !== 'glue') {
          // Define postsets. These will be run in ATINIT, right before global initializers (which might need the postsets). We cannot
          // run them now because the memory initializer might not have been applied yet.
          print('function runPostSets() {\n');
          print(itemsDict.GlobalVariablePostSet.map(function(item) { return item.JS }).join('\n'));
          print('}\n');
        }

        if (USE_TYPED_ARRAYS == 2) {
          if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
            print('var tempDoublePtr = Runtime.alignMemory(allocate(12, "i8", ALLOC_STATIC), 8);\n');
            print('assert(tempDoublePtr % 8 == 0);\n');
            print('function copyTempFloat(ptr) { // functions, because inlining this code increases code size too much\n');
            print('  HEAP8[tempDoublePtr] = HEAP8[ptr];\n');
            print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];\n');
            print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];\n');
            print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];\n');
            print('}\n');
            print('function copyTempDouble(ptr) {\n');
            print('  HEAP8[tempDoublePtr] = HEAP8[ptr];\n');
            print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];\n');
            print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];\n');
            print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];\n');
            print('  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];\n');
            print('  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];\n');
            print('  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];\n');
            print('  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];\n');
            print('}\n');
          }
        }
      }

      return;
    }

    // Print out global variables and postsets TODO: batching
    if (phase == 'pre' || phase == 'glue') {
      var legalizedI64sDefault = legalizedI64s;
      legalizedI64s = false;

      var globalsData = analyzer(intertyper(data.unparsedGlobalss[0].lines, true), true);

      if (!NAMED_GLOBALS) {
        sortGlobals(globalsData.globalVariables).forEach(function(g) {
          var ident = g.ident;
          if (!isIndexableGlobal(ident)) return;
          assert(Variables.nextIndexedOffset % Runtime.STACK_ALIGN == 0);
          Variables.indexedGlobals[ident] = Variables.nextIndexedOffset;
          Variables.nextIndexedOffset += Runtime.alignMemory(calcAllocatedSize(Variables.globals[ident].type));
          if (ident.substr(0, 5) == '__ZTV') { // leave room for null-terminating the vtable
            Variables.nextIndexedOffset += Runtime.alignMemory(QUANTUM_SIZE);
          }
        });
      }
      JSify(globalsData, true);
      globalsData = null;
      data.unparsedGlobalss = null;

      var generated = itemsDict.functionStub.concat(itemsDict.GlobalVariablePostSet);
      generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });

      legalizedI64s = legalizedI64sDefault;

      if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
        print('STACK_BASE = STACKTOP = Runtime.alignMemory(STATICTOP);\n');
        print('staticSealed = true; // seal the static portion of memory\n');
        print('STACK_MAX = STACK_BASE + TOTAL_STACK;\n');
        print('DYNAMIC_BASE = DYNAMICTOP = Runtime.alignMemory(STACK_MAX);\n');
        print('assert(DYNAMIC_BASE < TOTAL_MEMORY, "TOTAL_MEMORY not big enough for stack");\n');
      }

      if (asmLibraryFunctions.length > 0) {
        print('// ASM_LIBRARY FUNCTIONS');
        function fix(f) { // fix indenting to not confuse js optimizer
          f = f.substr(f.indexOf('f')); // remove initial spaces before 'function'
          f = f.substr(0, f.lastIndexOf('\n')+1); // remove spaces and last }  XXX assumes function has multiple lines
          return f + '}'; // add unindented } to match function
        }
        print(asmLibraryFunctions.map(fix).join('\n'));
      }

    } else {
      if (singlePhase) {
        assert(data.unparsedGlobalss[0].lines.length == 0, dump([phase, data.unparsedGlobalss]));
        assert(itemsDict.functionStub.length == 0, dump([phase, itemsDict.functionStub]));
      }
    }

    if (abortExecution) throw 'Aborting compilation due to previous errors';

    if (phase == 'pre' || phase == 'funcs') {
      PassManager.serialize();
      return;
    }

    // This is the main 'post' pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    if (PRECISE_I64_MATH && Types.preciseI64MathUsed) {
      if (!INCLUDE_FULL_LIBRARY && !SIDE_MODULE && !BUILD_AS_SHARED_LIB) {
        // first row are utilities called from generated code, second are needed from fastLong
        ['i64Add', 'i64Subtract', 'bitshift64Shl', 'bitshift64Lshr', 'bitshift64Ashr',
         'llvm_cttz_i32'].forEach(function(ident) {
          var finalName = '_' + ident;
          if (!Functions.libraryFunctions[finalName] || (phase == 'glue' && ident[0] === 'l' && !addedLibraryItems[ident])) { // TODO: one-by-one in fastcomp glue mode
            print(processLibraryFunction(LibraryManager.library[ident], ident, finalName)); // must be first to be close to generated code
            Functions.implementedFunctions[finalName] = LibraryManager.library[ident + '__sig'];
            Functions.libraryFunctions[finalName] = phase == 'glue' ? 2 : 1; // XXX
            // limited dependency handling
            var deps = LibraryManager.library[ident + '__deps'];
            if (deps) {
              deps.forEach(function(dep) {
                assert(typeof dep == 'function');
                var text = dep();
                assert(text.indexOf('\n') < 0);
                text = text.replace('ALLOC_STATIC', 'ALLOC_DYNAMIC');
                print('/* PRE_ASM */ ' + text + '\n');
              });
            }
          }
        });
      }
      // these may be duplicated in side modules and the main module without issue
      print(read('fastLong.js'));
      print('// EMSCRIPTEN_END_FUNCS\n');
      print(read('long.js'));
    } else {
      print('// EMSCRIPTEN_END_FUNCS\n');
      print('// Warning: printing of i64 values may be slightly rounded! No deep i64 math used, so precise i64 code not included');
      print('var i64Math = null;');
    }

    if (CORRUPTION_CHECK) {
      assert(!ASM_JS, 'corruption checker is not compatible with asm.js');
      print(processMacros(read('corruptionCheck.js')));
    }
    if (HEADLESS) {
      print('if (!ENVIRONMENT_IS_WEB) {');
      print(read('headlessCanvas.js'));
      print('\n');
      print(read('headless.js').replace("'%s'", "'http://emscripten.org'").replace("'?%s'", "''").replace("'?%s'", "'/'").replace('%s,', 'null,').replace('%d', '0'));
      print('}');
    }
    if (PROXY_TO_WORKER) {
      print('if (ENVIRONMENT_IS_WORKER) {\n');
      print(read('webGLWorker.js'));
      print(read('proxyWorker.js'));
      print('}');
    }
    if (DETERMINISTIC) {
      print(read('deterministic.js'));
    }
    if (RUNTIME_TYPE_INFO) {
      Types.cleanForRuntime();
      print('Runtime.typeInfo = ' + JSON.stringify(Types.types));
      print('Runtime.structMetadata = ' + JSON.stringify(Types.structMetadata));
    }
    var postFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'postamble_sharedlib.js' : 'postamble.js';
    var postParts = processMacros(preprocess(read(postFile))).split('{{GLOBAL_VARS}}');
    print(postParts[0]);

    Functions.generateIndexing(); // done last, as it may rely on aliases set in postsets

    // Load runtime-linked libraries
    RUNTIME_LINKED_LIBS.forEach(function(lib) {
      print('eval(Module["read"]("' + lib + '"))(' + Functions.getTable('x') + '.length, this);');
    });

    print(postParts[1]);

    var shellParts = read(shellFile).split('{{BODY}}');
    print(processMacros(preprocess(shellParts[1])));
    // Print out some useful metadata
    if (RUNNING_JS_OPTS || PGO) {
      var generatedFunctions = JSON.stringify(keys(Functions.implementedFunctions).filter(function(func) {
        return IGNORED_FUNCTIONS.indexOf(func.ident) < 0;
      }));
      if (PGO) {
        print('PGOMonitor.allGenerated = ' + generatedFunctions + ';\nremoveRunDependency("pgo");\n');
      }
      if (RUNNING_JS_OPTS) {
        print('// EMSCRIPTEN_GENERATED_FUNCTIONS: ' + generatedFunctions + '\n');
      }
    }

    PassManager.serialize();
  }

  // Data

  if (mainPass) {
    if (phase == 'pre') {
      // types have been parsed, so we can figure out function signatures (which can use types)
      data.unparsedFunctions.forEach(function(func) {
        Functions.implementedFunctions[func.ident] = Functions.getSignature(func.returnType, func.params.map(function(param) { return param.type }));
      });
    }
    data.functionStubs.forEach(functionStubHandler);
    assert(data.functions.length == 0);
  } else {
    if (phase == 'pre') {
      // ensure there is a global ctors, for runPostSets
      if ('_llvm_global_ctors' in data.globalVariables) {
        data.globalVariables._llvm_global_ctors.ctors.unshift('runPostSets'); // run postsets right before global initializers
        hasCtors = true;
      } else {
        globalVariableHandler({
          intertype: 'GlobalVariableStub',
          ident: '_llvm_global_ctors',
          type: '[1 x { i32, void ()* }]',
          ctors: ["runPostSets"],
        });
      }
    }

    //B.start('jsifier-handle-gv');
    sortGlobals(data.globalVariables).forEach(globalVariableHandler);
    //B.stop('jsifier-handle-gv');
    data.aliass.forEach(aliasHandler);
    data.functions.forEach(functionSplitter);
  }

  //B.start('jsifier-fc');
  finalCombiner();
  //B.stop('jsifier-fc');

  dprint('framework', 'Big picture: Finishing JSifier, main pass=' + mainPass);
  //B.stop('jsifier');
}

