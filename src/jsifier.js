//"use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Handy sets

var STRUCT_LIST = set('struct', 'list');

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
    print(processMacros(preprocess(shellParts[0], shellFile)));
    var preFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'preamble_sharedlib.js' : 'preamble.js';
    var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime()), preFile));
    print(pre);
  }

  if (mainPass) {
    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)
    //B.start('jsifier-libload');
    LibraryManager.load();
    //B.stop('jsifier-libload');

    var libFuncsToInclude;
    if (INCLUDE_FULL_LIBRARY) {
      assert(!(BUILD_AS_SHARED_LIB || SIDE_MODULE), 'Cannot have both INCLUDE_FULL_LIBRARY and BUILD_AS_SHARED_LIB/SIDE_MODULE set.')
      libFuncsToInclude = (MAIN_MODULE || SIDE_MODULE) ? DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.slice(0) : [];
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

  function processLibraryFunction(snippet, ident, finalName) {
    // It is possible that when printing the function as a string on Windows, the js interpreter we are in returns the string with Windows
    // line endings \r\n. This is undesirable, since line endings are managed in the form \n in the output for binary file writes, so
    // make sure the endings are uniform.
    snippet = snippet.toString().replace(/\r\n/gm,"\n");
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
    // special logic
    if (item.ident.startsWith('___cxa_find_matching_catch_')) {
      var num = +item.ident.split('_').slice(-1)[0];
      LibraryManager.library[item.ident.substr(1)] = function() {
        return ___cxa_find_matching_catch.apply(null, arguments);
      };
    }

    // note the signature
    if (item.returnType && item.params) {
      functionStubSigs[item.ident] = Functions.getSignature(item.returnType.text, item.params.map(function(arg) { return arg.type }), false);
    }

    function addFromLibrary(ident) {
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

      var noExport = false;

      if ((!LibraryManager.library.hasOwnProperty(ident) && !LibraryManager.library.hasOwnProperty(ident + '__inline')) || SIDE_MODULE) {
        if (!(finalName in IMPLEMENTED_FUNCTIONS)) {
          if (VERBOSE || ident.substr(0, 11) !== 'emscripten_') { // avoid warning on emscripten_* functions which are for internal usage anyhow
            if (!LINKABLE) {
              if (ERROR_ON_UNDEFINED_SYMBOLS) error('unresolved symbol: ' + ident);
              else if (VERBOSE || WARN_ON_UNDEFINED_SYMBOLS) warn('unresolved symbol: ' + ident);
            }
          }
        }
        if (!(MAIN_MODULE || SIDE_MODULE)) {
          // emit a stub that will fail at runtime
          LibraryManager.library[shortident] = new Function("Module['printErr']('missing function: " + shortident + "'); abort(-1);");
        } else {
          var target = (MAIN_MODULE ? '' : 'parent') + "Module['_" + shortident + "']";
          var assertion = '';
          if (ASSERTIONS) assertion = 'if (!' + target + ') abort("external function \'' + shortident + '\' is missing. perhaps a side module was not linked in? if this function was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");';
          LibraryManager.library[shortident] = new Function(assertion + "return " + target + ".apply(null, arguments);");
          if (SIDE_MODULE) {
            // no dependencies, just emit the thunk
            Functions.libraryFunctions[finalName] = 1;
            return processLibraryFunction(LibraryManager.library[shortident], ident, finalName);
          }
          noExport = true;
        }
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
        if (!redirectedIdent && (typeof target == 'function' || /Math_\w+/.exec(snippet))) {
          Functions.libraryFunctions[finalName] = 1;
        }
      } else if (typeof snippet === 'object') {
        snippet = stringifyWithFunctions(snippet);
      } else if (typeof snippet === 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, ident, finalName);
        Functions.libraryFunctions[finalName] = 1;
      }

      var postsetId = ident + '__postset';
      var postset = LibraryManager.library[postsetId];
      if (postset && !addedLibraryItems[postsetId] && !SIDE_MODULE) {
        addedLibraryItems[postsetId] = true;
        itemsDict.GlobalVariablePostSet.push({
          intertype: 'GlobalVariablePostSet',
          JS: postset + ';'
        });
      }

      if (redirectedIdent) {
        deps = deps.concat(LibraryManager.library[redirectedIdent + '__deps'] || []);
      }
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
      if (VERBOSE) printErr('adding ' + finalName + ' and deps ' + deps + ' : ' + (snippet + '').substr(0, 40));
      var depsText = (deps ? '\n' + deps.map(addFromLibrary).filter(function(x) { return x != '' }).join('\n') : '');
      var contentText;
      if (isFunction) {
        contentText = snippet;
      } else if (typeof snippet === 'string' && snippet.indexOf(';') == 0) {
        contentText = 'var ' + finalName + snippet;
        if (snippet[snippet.length-1] != ';' && snippet[snippet.length-1] != '}') contentText += ';';
      } else {
        contentText = 'var ' + finalName + '=' + snippet + ';';
      }
      var sig = LibraryManager.library[ident + '__sig'];
      if (isFunction && sig && LibraryManager.library[ident + '__asm']) {
        // asm library function, add it as generated code alongside the generated code
        Functions.implementedFunctions[finalName] = sig;
        asmLibraryFunctions.push(contentText);
        contentText = ' ';
        EXPORTED_FUNCTIONS[finalName] = 1;
        Functions.libraryFunctions[finalName] = 2;
      }
      if ((EXPORT_ALL || (finalName in EXPORTED_FUNCTIONS)) && !noExport) {
        contentText += '\nModule["' + finalName + '"] = ' + finalName + ';';
      }
      return depsText + contentText;
    }

    itemsDict.functionStub.push(item);
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
      item.JS = addFromLibrary(shortident);
    }
  }

  // Final combiner

  function finalCombiner() {
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
      if (!Variables.generatedGlobalBase && !BUILD_AS_SHARED_LIB) {
        Variables.generatedGlobalBase = true;
        // Globals are done, here is the rest of static memory
        if (!SIDE_MODULE) {
          print('STATIC_BASE = ' + Runtime.GLOBAL_BASE + ';\n');
          print('STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n');
        } else {
          print('gb = Runtime.alignMemory(getMemory({{{ STATIC_BUMP }}}, ' + MAX_GLOBAL_ALIGN + ' || 1));\n');
          print('// STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n'); // comment as metadata only
        }
        if (BINARYEN) {
          print('var STATIC_BUMP = {{{ STATIC_BUMP }}};');
        }
      }
      var generated = itemsDict.function.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.GlobalVariable);
      print(generated.map(function(item) { return item.JS; }).join('\n'));

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
        if (USE_PTHREADS) {
          print('if (!ENVIRONMENT_IS_PTHREAD) {') // Pthreads should not initialize memory again, since it's shared with the main thread.
        }
        print('/* memory initializer */ ' + makePointer(memoryInitialization, null, 'ALLOC_NONE', 'i8', 'Runtime.GLOBAL_BASE' + (SIDE_MODULE ? '+H_BASE' : ''), true));
        if (USE_PTHREADS) {
          print('}')
        }
      } else {
        print('/* no memory initializer */'); // test purposes
      }

      if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
        if (USE_PTHREADS) {
          print('var tempDoublePtr;\n');
          print('if (!ENVIRONMENT_IS_PTHREAD) tempDoublePtr = Runtime.alignMemory(allocate(12, "i8", ALLOC_STATIC), 8);\n');
        } else {
          print('var tempDoublePtr = ' + makeStaticAlloc(8) + '\n');
        }
        if (ASSERTIONS) print('assert(tempDoublePtr % 8 == 0);\n');
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
      print('// {{PRE_LIBRARY}}\n'); // safe to put stuff here that statically allocates

      return;
    }

    // Print out global variables and postsets TODO: batching
    var legalizedI64sDefault = legalizedI64s;
    legalizedI64s = false;

    var globalsData = {functionStubs: []}
    JSify(globalsData, true);
    globalsData = null;

    var generated = itemsDict.functionStub.concat(itemsDict.GlobalVariablePostSet);
    generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });

    legalizedI64s = legalizedI64sDefault;

    if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
      print('STACK_BASE = STACKTOP = Runtime.alignMemory(STATICTOP);\n');
      print('staticSealed = true; // seal the static portion of memory\n');
      print('STACK_MAX = STACK_BASE + TOTAL_STACK;\n');
      print('DYNAMIC_BASE = DYNAMICTOP = Runtime.alignMemory(STACK_MAX);\n');
      if (ASSERTIONS) print('assert(DYNAMIC_BASE < TOTAL_MEMORY, "TOTAL_MEMORY not big enough for stack");\n');
    }
    if (SPLIT_MEMORY) {
      print('assert(STACK_MAX < SPLIT_MEMORY, "SPLIT_MEMORY size must be big enough so the entire static memory + stack can fit in one chunk, need " + STACK_MAX);\n');
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

    if (abortExecution) throw 'Aborting compilation due to previous errors';

    // This is the main 'post' pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    print('// EMSCRIPTEN_END_FUNCS\n');

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
      print(processMacros(preprocess(read('proxyWorker.js'), 'proxyWorker.js')));
      print('}');
    }
    if (DETERMINISTIC) {
      print(read('deterministic.js'));
    }
    var postFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'postamble_sharedlib.js' : 'postamble.js';
    var postParts = processMacros(preprocess(read(postFile), postFile)).split('{{GLOBAL_VARS}}');
    print(postParts[0]);

    print(postParts[1]);

    var shellParts = read(shellFile).split('{{BODY}}');
    print(processMacros(preprocess(shellParts[1], shellFile)));
    // Print out some useful metadata
    if (RUNNING_JS_OPTS || PGO) {
      var generatedFunctions = JSON.stringify(keys(Functions.implementedFunctions));
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
    data.functionStubs.forEach(functionStubHandler);
  }

  //B.start('jsifier-fc');
  finalCombiner();
  //B.stop('jsifier-fc');

  dprint('framework', 'Big picture: Finishing JSifier, main pass=' + mainPass);
  //B.stop('jsifier');
}

