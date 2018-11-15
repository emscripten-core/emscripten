// Copyright 2010 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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

// Some JS-implemented library functions are proxied to be called on the main browser thread, if the Emscripten runtime is executing in a Web Worker.
// Each such proxied function is identified via an ordinal number (this is not the same namespace as function pointers in general).
var proxiedFunctionTable = ["null" /* Reserve index 0 for an undefined function*/];

// proxiedFunctionInvokers contains bodies of the functions that will perform the proxying. These
// are generated in a map to keep track which ones have already been emitted, to avoid outputting duplicates.
// map: pair(sig, syncOrAsync) -> function body
var proxiedFunctionInvokers = {};

// We include asm2wasm imports if we may interpret (where we call out to JS to do some math stuff)
// or if the trap mode is 'js' (where we do the same). However, we always need some of them (like
// the frem import because % is in asm.js but not in wasm). But we can avoid emitting all the others
// in many cases.
var NEED_ALL_ASM2WASM_IMPORTS = BINARYEN_METHOD != 'native-wasm' || BINARYEN_TRAP_MODE == 'js';

// used internally. set when there is a main() function.
// also set when in a linkable module, as the main() function might
// arrive from a dynamically-linked library, and not necessarily
// the current compilation unit.
var HAS_MAIN = ('_main' in IMPLEMENTED_FUNCTIONS) || MAIN_MODULE || SIDE_MODULE;

var WASM_BACKEND_WITH_RESERVED_FUNCTION_POINTERS =
  WASM_BACKEND && RESERVED_FUNCTION_POINTERS;

// JSifier
function JSify(data, functionsOnly) {
  var mainPass = !functionsOnly;

  var itemsDict = { type: [], GlobalVariableStub: [], functionStub: [], function: [], GlobalVariable: [], GlobalVariablePostSet: [] };

  if (mainPass) {
    var shellFile = SHELL_FILE ? SHELL_FILE : (SIDE_MODULE ? 'shell_sharedlib.js' : 'shell.js');

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
    var pre;
    if (SIDE_MODULE) {
      pre = processMacros(preprocess(read('preamble_sharedlib.js'), 'preamble_sharedlib.js'));
    } else {
      pre = processMacros(preprocess(read('support.js'), 'support.js')) +
            processMacros(preprocess(read('preamble.js'), 'preamble.js'));
    }
    print(pre);
  }

  if (mainPass) {
    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)

    LibraryManager.load();

    var libFuncsToInclude;
    if (INCLUDE_FULL_LIBRARY) {
      assert(!SIDE_MODULE, 'Cannot have both INCLUDE_FULL_LIBRARY and SIDE_MODULE set.')
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
      snippet = snippet.replace('{', '{ var ret = (function() { if (runtimeDebug) err("[library call:' + finalName + ': " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]"); ');
      snippet = snippet.substr(0, snippet.length-1) + '}).apply(this, arguments); if (runtimeDebug && typeof ret !== "undefined") err("  [     return:" + prettyPrint(ret)); return ret; \n}';
    }
    return snippet;
  }

  // Generates a function that invokes a proxied function call from the calling thread to the main browser thread.
  function generateProxiedCallInvoker(sig, sync /*async if false*/) {
    if (sig.length == 0) throw 'Function signature cannot be empty!';
    function argsList(num) { // ", p0, p1, p2, p3, p4"
      var s = '';
      for(var i = 0; i < num; ++i) s += ', p' + i;
      return s;
    }

    var func = "function _emscripten_" + (sync ? '' : 'a') + 'sync_run_in_browser_thread_' + sig + '(func' + argsList(sig.length-1) + ') {\n';
    if (sync) func += '  var waitAddress = stackSave();\n';

    function sizeofType(t) {
      switch(t) {
        case 'd': return 8;
//      case 'I': return 8; // int64 // TODO: For wasm, we'll have to have something like this
        case 'i': return 4;
        case 'f': return 4;
        case 'v': return 0;
        // TODO: Smaller sizes?
        default: throw 'unsupported type in signature: ' + t;
      }
    }

    var sizeofReturn = sizeofType(sig[0]);
    if (sync) {
      if (sizeofReturn == 4) func += '  var returnValue = waitAddress + 4;\n';
      else if (sizeofReturn == 8) func += '  var returnValue = waitAddress + 8;\n'; // Retain alignment of each type
    }

    if (sync) func += '  Atomics.store(HEAP32, waitAddress >> 2, 0);\n';

    function argsDict(num) { // ", p0: p0, p1: p1, p2: p2, p3: p3, p4: p4"
      var s = '';
      for(var i = 0; i < num; ++i) s += ', p' + i + ': p' + i;
      return s;
    }

    // This is ad-hoc numbering scheme to map signatures to IDs, and must agree with call handler in src/library_pthread.js.
    // Signatures with numbers 0-19 are for functions that return void, int or float, and ID numbers >= 32 are for functions that return a double.
    // Once proxied function calls no longer go through postMessage()s but instead in the heap, this will need to change, since int vs float will matter.
    const idForFunctionReturningDouble = 32;
    var functionCallOrdinal = sig.length + (sig[0] == 'd' ? idForFunctionReturningDouble : 0);
    if (sig.length > 10) throw 'Proxying functions with 10 or more function parameters is not supported!';

    // next line generates a form: "postMessage({ proxiedCall: 9, func: func, waitAddress: waitAddress, returnValue: returnValue, p0: p0, p1: p1, p2: p2, p3: p3, p4: p4, p5: p5, p6: p6, p7: p7 });"
    func += '  postMessage({ proxiedCall: ' + functionCallOrdinal + ', func: func' + (sync ? ', waitAddress: waitAddress' : '') + (sync && sizeofReturn > 0 ? ', returnValue: returnValue' : '') + argsDict(sig.length-1) + ' });\n';
    if (sync) {
      func += '  Atomics.wait(HEAP32, waitAddress >> 2, 0);\n';
      switch(sig[0]) {
        case 'i': func += '  return HEAP32[returnValue >> 2];\n'; break;
        case 'f': func += '  return HEAPF32[returnValue >> 2];\n'; break;
        case 'd': func += '  return HEAPF64[returnValue >> 3];\n'; break;
        //case 'I': func += '  return HEAP64[returnValue >> 3];\n'; // TODO: For wasm
        // TODO: Smaller sizes?
      }
    }
    func += '}';
    return func;
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

      // don't process any special identifiers. These are looked up when processing the base name of the identifier.
      if (ident.endsWith('__sig') || ident.endsWith('__proxy') || ident.endsWith('__asm') || ident.endsWith('__inline') || ident.endsWith('__deps') || ident.endsWith('__postset')) {
        return '';
      }

      // $ident's are special, we do not prefix them with a '_'.
      if (ident[0] === '$') {
        var finalName = ident.substr(1);
      } else {
        var finalName = '_' + ident;
      }

      // if the function was implemented in compiled code, we just need to export it so we can reach it from JS
      if (finalName in IMPLEMENTED_FUNCTIONS) {
        EXPORTED_FUNCTIONS[finalName] = 1;
        // stop here: we don't need to add anything from our js libraries, not even deps, compiled code is on it
        return '';
      }

      // Don't replace implemented functions with library ones (which can happen when we add dependencies).
      // Note: We don't return the dependencies here. Be careful not to end up where this matters
      if (finalName in Functions.implementedFunctions) return '';

      var noExport = false;

      if ((!LibraryManager.library.hasOwnProperty(ident) && !LibraryManager.library.hasOwnProperty(ident + '__inline')) || SIDE_MODULE) {
        if (!(finalName in IMPLEMENTED_FUNCTIONS)) {
          if (VERBOSE || ident.substr(0, 11) !== 'emscripten_') { // avoid warning on emscripten_* functions which are for internal usage anyhow
            if (!LINKABLE) {
              if (ERROR_ON_UNDEFINED_SYMBOLS) {
                error('undefined symbol: ' + ident);
                warnOnce('To disable errors for undefined symbols use `-s ERROR_ON_UNDEFINED_SYMBOLS=0`')
              } else if (VERBOSE || WARN_ON_UNDEFINED_SYMBOLS) {
                warn('undefined symbol: ' + ident);
              }
            }
          }
        }
        if (!(MAIN_MODULE || SIDE_MODULE)) {
          // emit a stub that will fail at runtime
          LibraryManager.library[shortident] = new Function("err('missing function: " + shortident + "'); abort(-1);");
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
        // Emit the body of a JS library function.
        var proxyingMode = LibraryManager.library[ident + '__proxy'];
        if (proxyingMode && proxyingMode !== 'sync' && proxyingMode !== 'async') {
          throw 'Invalid proxyingMode ' + ident + '__proxy: \'' + proxyingMode + '\' specified!';
        }

        if (USE_PTHREADS && proxyingMode) {
          var sig = LibraryManager.library[ident + '__sig'];
          if (!sig) throw 'Missing function signature field "' + ident + '__sig"! (Using proxying mode requires specifying the signature of the function)';
          sig = sig.replace(/f/g, 'i'); // TODO: Implement float signatures.
          var synchronousCall = (proxyingMode === 'sync');
          var invokerKey = sig + (synchronousCall ? '_sync' : '_async');
          if (!proxiedFunctionInvokers[invokerKey]) proxiedFunctionInvokers[invokerKey] = generateProxiedCallInvoker(sig, synchronousCall);
          var proxyingFunc = synchronousCall ? '_emscripten_sync_run_in_browser_thread_' : '_emscripten_async_run_in_browser_thread_';
          if (sig.length > 1) {
            // If the function takes parameters, forward those to the proxied function call
            var processedSnippet = snippet.replace(/function\s+(.*)?\s*\((.*?)\)\s*{/, 'function $1($2) {\nif (ENVIRONMENT_IS_PTHREAD) return ' + proxyingFunc + sig + '(' + proxiedFunctionTable.length + ', $2);');
          } else {
            // No parameters to the function
            var processedSnippet = snippet.replace(/function\s+(.*)?\s*\((.*?)\)\s*{/, 'function $1() {\nif (ENVIRONMENT_IS_PTHREAD) return ' + proxyingFunc + sig + '(' + proxiedFunctionTable.length + ');');
          }
          if (processedSnippet == snippet) throw 'Failed to regex parse function to add pthread proxying preamble to it! Function: ' + snippet;
          contentText = processedSnippet;
          proxiedFunctionTable.push(finalName);
        } else {
          contentText = snippet; // Regular JS function that will be executed in the context of the calling thread.
        }
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
        Functions.libraryFunctions[finalName] = 2;
        noExport = true; // if it needs to be exported, that will happen in emscripten.py
      }
      // asm module exports are done in emscripten.py, after the asm module is ready. Here
      // we also export library methods as necessary.
      if ((EXPORT_ALL || (finalName in EXPORTED_FUNCTIONS)) && !noExport) {
        contentText += '\nModule["' + finalName + '"] = ' + finalName + ';';
      }
      if (!LibraryManager.library[ident + '__asm']) {
        // If we are not an asm library func, and we have a dep that is, then we need to call
        // into the asm module to reach that dep. so it must be exported from the asm module.
        // We set EXPORTED_FUNCTIONS here to tell emscripten.py to do that.
        deps.forEach(function(dep) {
          if (LibraryManager.library[dep + '__asm']) {
            EXPORTED_FUNCTIONS['_' + dep] = 0;
          }
        });
      }
      return depsText + contentText;
    }

    itemsDict.functionStub.push(item);
    var shortident = item.ident.substr(1);
    // If this is not linkable, anything not in the library is definitely missing
    if (item.ident in DEAD_FUNCTIONS) {
      if (LibraryManager.library[shortident + '__asm']) {
        warn('cannot kill asm library function ' + item.ident);
      } else {
        LibraryManager.library[shortident] = new Function("err('dead function: " + shortident + "'); abort(-1);");
        delete LibraryManager.library[shortident + '__inline'];
        delete LibraryManager.library[shortident + '__deps'];
      }
    }
    item.JS = addFromLibrary(shortident);
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
      if (!Variables.generatedGlobalBase) {
        Variables.generatedGlobalBase = true;
        // Globals are done, here is the rest of static memory
        if (!SIDE_MODULE) {
          print('STATIC_BASE = GLOBAL_BASE;\n');
          print('STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n');
        } else {
          print('gb = alignMemory(getMemory({{{ STATIC_BUMP }}} + ' + MAX_GLOBAL_ALIGN + '), ' + MAX_GLOBAL_ALIGN + ' || 1);\n');
          // The static area consists of explicitly initialized data, followed by zero-initialized data.
          // The latter may need zeroing out if the MAIN_MODULE has already used this memory area before
          // dlopen'ing the SIDE_MODULE.  Since we don't know the size of the explicitly initialized data
          // here, we just zero the whole thing, which is suboptimal, but should at least resolve bugs
          // from uninitialized memory.
          print('for (var i = gb; i < gb + {{{ STATIC_BUMP }}}; ++i) HEAP8[i] = 0;\n');
          print('// STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n'); // comment as metadata only
        }
        if (WASM) {
          // export static base and bump, needed for linking in wasm binary's memory, dynamic linking, etc.
          print('var STATIC_BUMP = {{{ STATIC_BUMP }}};');
          print('Module["STATIC_BASE"] = STATIC_BASE;');
          print('Module["STATIC_BUMP"] = STATIC_BUMP;');
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
        print('/* memory initializer */ ' + makePointer(memoryInitialization, null, 'ALLOC_NONE', 'i8', 'GLOBAL_BASE' + (SIDE_MODULE ? '+H_BASE' : ''), true));
        if (USE_PTHREADS) {
          print('}')
        }
      } else {
        print('/* no memory initializer */'); // test purposes
      }

      if (!SIDE_MODULE) {
        if (USE_PTHREADS) {
          print('var tempDoublePtr;');
          print('if (!ENVIRONMENT_IS_PTHREAD) tempDoublePtr = alignMemory(allocate(12, "i8", ALLOC_STATIC), 8);');
        } else {
          print('var tempDoublePtr = ' + makeStaticAlloc(8) + '');
        }
        if (ASSERTIONS) print('assert(tempDoublePtr % 8 == 0);');
        print('\nfunction copyTempFloat(ptr) { // functions, because inlining this code increases code size too much');
        print('  HEAP8[tempDoublePtr] = HEAP8[ptr];');
        print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];');
        print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];');
        print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];');
        print('}\n');
        print('function copyTempDouble(ptr) {');
        print('  HEAP8[tempDoublePtr] = HEAP8[ptr];');
        print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];');
        print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];');
        print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];');
        print('  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];');
        print('  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];');
        print('  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];');
        print('  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];');
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

    if (!SIDE_MODULE) {
      if (USE_PTHREADS) {
        print('\n // proxiedFunctionTable specifies the list of functions that can be called either synchronously or asynchronously from other threads in postMessage()d or internally queued events. This way a pthread in a Worker can synchronously access e.g. the DOM on the main thread.')
        print('\nvar proxiedFunctionTable = [' + proxiedFunctionTable.join() + '];\n');
        // Generate worker->main thread proxy function invokers for MAIN_THREAD_EM_ASM() signatures.
        for (var i in PROXIED_FUNCTION_SIGNATURES) {
          var invokerKey = PROXIED_FUNCTION_SIGNATURES[i];
          if (!proxiedFunctionInvokers[invokerKey]) {
            var sig_sync = invokerKey.split('_');
            proxiedFunctionInvokers[invokerKey] = generateProxiedCallInvoker(sig_sync[0], sig_sync[1] === 'sync');
          }
        }
        for(i in proxiedFunctionInvokers) print(proxiedFunctionInvokers[i]+'\n');
        print('if (!ENVIRONMENT_IS_PTHREAD) {\n // Only main thread initializes these, pthreads copy them over at thread worker init time (in pthread-main.js)');
      }
      print('DYNAMICTOP_PTR = staticAlloc(4);\n');
      print('STACK_BASE = STACKTOP = alignMemory(STATICTOP);\n');
      if (STACK_START > 0) print('if (STACKTOP < ' + STACK_START + ') STACK_BASE = STACKTOP = alignMemory(' + STACK_START + ');\n');
      print('STACK_MAX = STACK_BASE + TOTAL_STACK;\n');
      print('DYNAMIC_BASE = alignMemory(STACK_MAX);\n');
      print('HEAP32[DYNAMICTOP_PTR>>2] = DYNAMIC_BASE;\n');
      print('staticSealed = true; // seal the static portion of memory\n');
      if (ASSERTIONS) print('assert(DYNAMIC_BASE < TOTAL_MEMORY, "TOTAL_MEMORY not big enough for stack");\n');
      if (USE_PTHREADS) print('}\n');
    }

    print('var ASSERTIONS = ' + !!ASSERTIONS + ';\n');

    print(preprocess(read('arrayUtils.js')));

    if (SUPPORT_BASE64_EMBEDDING) {
      print(preprocess(read('base64Utils.js')));
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

    if (abortExecution) throw Error('Aborting compilation due to previous errors');

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

    var postFile = SIDE_MODULE ? 'postamble_sharedlib.js' : 'postamble.js';
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
