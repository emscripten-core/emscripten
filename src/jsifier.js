/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// "use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

global.addedLibraryItems = {};

// Some JS-implemented library functions are proxied to be called on the main browser thread, if the Emscripten runtime is executing in a Web Worker.
// Each such proxied function is identified via an ordinal number (this is not the same namespace as function pointers in general).
global.proxiedFunctionTable = ['null'/* Reserve index 0 for an undefined function*/];

// Mangles the given C/JS side function name to assembly level function name (adds an underscore)
function mangleCSymbolName(f) {
  return f[0] == '$' ? f.substr(1) : '_' + f;
}

// Splits out items that pass filter. Returns also the original sans the filtered
function splitter(array, filter) {
  const splitOut = array.filter(filter);
  const leftIn = array.filter((x) => !filter(x));
  return {leftIn: leftIn, splitOut: splitOut};
}

// Functions that start with '$' should not be exported to the wasm module.
// They are intended to be exclusive to JS code only.
function isJsOnlyIdentifier(ident) {
  return ident[0] == '$';
}

function escapeJSONKey(x) {
  if (/^[\d\w_]+$/.exec(x) || x[0] === '"' || x[0] === "'") return x;
  assert(!x.includes("'"), 'cannot have internal single quotes in keys: ' + x);
  return "'" + x + "'";
}

function stringifyWithFunctions(obj) {
  if (typeof obj === 'function') return obj.toString();
  if (obj === null || typeof obj !== 'object') return JSON.stringify(obj);
  if (Array.isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  } else {
    return '{' + Object.keys(obj).map((key) => escapeJSONKey(key) + ':' + stringifyWithFunctions(obj[key])).join(',') + '}';
  }
}

function isDefined(symName) {
  if (WASM_EXPORTS.has(symName) || SIDE_MODULE_EXPORTS.has(symName)) {
    return true;
  }
  // 'invoke_' symbols are created at runtime in libary_dylink.py so can
  // always be considered as defined.
  if (RELOCATABLE && symName.startsWith('invoke_')) {
    return true;
  }
  return false;
}

// JSifier
function runJSify(functionsOnly) {
  const mainPass = !functionsOnly;
  const functionStubs = [];

  const itemsDict = {type: [], functionStub: [], function: [], globalVariablePostSet: []};

  if (mainPass) {
    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)

    LibraryManager.load();

    const libFuncsToInclude = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
    if (INCLUDE_FULL_LIBRARY) {
      for (const key in LibraryManager.library) {
        if (!isJsLibraryConfigIdentifier(key)) {
          libFuncsToInclude.push(key);
        }
      }
    }
    libFuncsToInclude.forEach((ident) => {
      functionStubs.push({
        identOrig: ident,
        identMangled: mangleCSymbolName(ident),
      });
    });
  }

  function processLibraryFunction(snippet, ident, finalName) {
    // It is possible that when printing the function as a string on Windows, the js interpreter we are in returns the string with Windows
    // line endings \r\n. This is undesirable, since line endings are managed in the form \n in the output for binary file writes, so
    // make sure the endings are uniform.
    snippet = snippet.toString().replace(/\r\n/gm, '\n');

    // name the function; overwrite if it's already named
    snippet = snippet.replace(/function(?:\s+([^(]+))?\s*\(/, 'function ' + finalName + '(');

    // apply LIBRARY_DEBUG if relevant
    if (LIBRARY_DEBUG && !isJsOnlyIdentifier(ident)) {
      snippet = modifyFunction(snippet, (name, args, body) => `\
function ${name}(${args}) {
  var ret = (function() { if (runtimeDebug) err("[library call:${finalName}: " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]");
  ${body}
  }).apply(this, arguments);
  if (runtimeDebug && typeof ret !== "undefined") err("  [     return:" + prettyPrint(ret));
  return ret;
}`);
    }
    return snippet;
  }

  // functionStub
  function functionStubHandler(item) {
    // In LLVM, exceptions generate a set of functions of form __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.
    // where the number specifies the number of arguments. In Emscripten, route all these to a single function '__cxa_find_matching_catch'
    // that variadically processes all of these functions using JS 'arguments' object.
    if (item.identMangled.startsWith('___cxa_find_matching_catch_')) {
      if (DISABLE_EXCEPTION_THROWING) {
        error('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but exception catching code appears. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.');
        return;
      }
      const num = +item.identMangled.split('_').slice(-1)[0];
      addCxaCatch(num);
      // Continue, with the code below emitting the proper JavaScript based on
      // what we just added to the library.
    }

    const TOP_LEVEL = 'top-level compiled C/C++ code';

    function addFromLibrary(item, dependent) {
      // dependencies can be JS functions, which we just run
      if (typeof item == 'function') return item();

      const ident = item.identOrig;
      const finalName = item.identMangled;

      if (ident in addedLibraryItems) return '';
      addedLibraryItems[ident] = true;

      // don't process any special identifiers. These are looked up when processing the base name of the identifier.
      if (isJsLibraryConfigIdentifier(ident)) {
        return '';
      }

      // if the function was implemented in compiled code, there is no need to include the js version
      if (WASM_EXPORTS.has(ident)) {
        return '';
      }

      let noExport = false;

      if (!LibraryManager.library.hasOwnProperty(ident)) {
        if (ONLY_CALC_JS_SYMBOLS) {
          return;
        }
        if (!isDefined(ident)) {
          let msg = 'undefined symbol: ' + ident;
          if (dependent) msg += ` (referenced by ${dependent})`;
          if (ERROR_ON_UNDEFINED_SYMBOLS) {
            error(msg);
            if (dependent == TOP_LEVEL && !LLD_REPORT_UNDEFINED) {
              warnOnce('Link with `-s LLD_REPORT_UNDEFINED` to get more information on undefined symbols');
            }
            warnOnce('To disable errors for undefined symbols use `-s ERROR_ON_UNDEFINED_SYMBOLS=0`');
            warnOnce(finalName + ' may need to be added to EXPORTED_FUNCTIONS if it arrives from a system library');
          } else if (VERBOSE || WARN_ON_UNDEFINED_SYMBOLS) {
            warn(msg);
          }
          if (ident === 'main' && STANDALONE_WASM) {
            warn('To build in STANDALONE_WASM mode without a main(), use emcc --no-entry');
          }
        }
        if (!RELOCATABLE) {
          // emit a stub that will fail at runtime
          LibraryManager.library[ident] = new Function(`err('missing function: ${ident}'); abort(-1);`);
          // We have already warned/errored about this function, so for the purposes of Closure use, mute all type checks
          // regarding this function, marking ot a variadic function that can take in anything and return anything.
          // (not useful to warn/error multiple times)
          LibraryManager.library[ident + '__docs'] = '/** @type {function(...*):?} */';
        } else {
          const target = `Module['${finalName}']`;
          let assertion = '';
          if (ASSERTIONS) {
            assertion += `if (!${target}) abort("external symbol '${ident}' is missing. perhaps a side module was not linked in? if this function was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");\n`;
          }
          const functionBody = assertion + `return ${target}.apply(null, arguments);`;
          LibraryManager.library[ident] = new Function(functionBody);
          noExport = true;
        }
      }

      const original = LibraryManager.library[ident];
      let snippet = original;
      let redirectedIdent = null;
      const deps = LibraryManager.library[ident + '__deps'] || [];
      if (!Array.isArray(deps)) {
        error(`JS library directive ${ident}__deps=${deps.toString()} is of type ${typeof deps}, but it should be an array!`);
        return;
      }
      const isUserSymbol = LibraryManager.library[ident + '__user'];
      deps.forEach((dep) => {
        if (typeof snippet === 'string' && !(dep in LibraryManager.library)) {
          warn(`missing library dependency ${dep}, make sure you are compiling with the right options (see #if in src/library*.js)`);
        }
        if (isUserSymbol && LibraryManager.library[dep + '__internal']) {
          warn(`user library symbol '${ident}' depends on internal symbol '${dep}'`);
        }
      });
      let isFunction = false;

      if (typeof snippet === 'string') {
        if (snippet[0] != '=') {
          const target = LibraryManager.library[snippet];
          if (target) {
            // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
            // This avoid having duplicate functions with identical content.
            redirectedIdent = snippet;
            deps.push(snippet);
            snippet = mangleCSymbolName(snippet);
          }
          // In asm, we need to know about library functions. If there is a target, though, then no
          // need to consider this a library function - we will call directly to it anyhow
          if (!redirectedIdent && (typeof target == 'function')) {
            libraryFunctions.push(finalName);
          }
        }
      } else if (typeof snippet === 'object') {
        snippet = stringifyWithFunctions(snippet);
      } else if (typeof snippet === 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, ident, finalName);
        libraryFunctions.push(finalName);
      }

      // If a JS library item specifies xxx_import: true, then explicitly mark that symbol to be exported
      // to wasm module.
      if (LibraryManager.library[ident + '__import']) {
        libraryFunctions.push(finalName);
      }

      if (ONLY_CALC_JS_SYMBOLS) {
        return '';
      }

      const postsetId = ident + '__postset';
      let postset = LibraryManager.library[postsetId];
      if (postset) {
        // A postset is either code to run right now, or some text we should emit.
        // If it's code, it may return some text to emit as well.
        if (typeof postset === 'function') {
          postset = postset();
        }
        if (postset && !addedLibraryItems[postsetId]) {
          addedLibraryItems[postsetId] = true;
          itemsDict.globalVariablePostSet.push({
            JS: postset + ';',
          });
        }
      }

      if (redirectedIdent) {
        deps = deps.concat(LibraryManager.library[redirectedIdent + '__deps'] || []);
      }
      if (VERBOSE) {
        printErr(`adding ${finalName} and deps ${deps} : ` + (snippet + '').substr(0, 40));
      }
      const identDependents = ident + "__deps: ['" + deps.join("','") + "']";
      function addDependency(dep) {
        if (typeof dep !== 'function') {
          dep = {identOrig: dep, identMangled: mangleCSymbolName(dep)};
        }
        return addFromLibrary(dep, `${identDependents}, referenced by ${dependent}`);
      }
      let contentText;
      if (isFunction) {
        // Emit the body of a JS library function.
        const proxyingMode = LibraryManager.library[ident + '__proxy'];
        if (USE_PTHREADS && proxyingMode) {
          if (proxyingMode !== 'sync' && proxyingMode !== 'async') {
            throw new Error(`Invalid proxyingMode ${ident}__proxy: '${proxyingMode}' specified!`);
          }
          const sync = proxyingMode === 'sync';
          assert(typeof original === 'function');
          contentText = modifyFunction(snippet, (name, args, body) => `
function ${name}(${args}) {
  if (ENVIRONMENT_IS_PTHREAD)
    return _emscripten_proxy_to_main_thread_js(${proxiedFunctionTable.length}, ${+sync}${args ? ', ' : ''}${args});
  ${body}
}\n`);
          proxiedFunctionTable.push(finalName);
        } else if ((USE_ASAN || USE_LSAN || UBSAN_RUNTIME) && LibraryManager.library[ident + '__noleakcheck']) {
          contentText = modifyFunction(snippet, (name, args, body) => `
function ${name}(${args}) {
  return withBuiltinMalloc(function() {
    ${body}
  });
}\n`);
          deps.push('$withBuiltinMalloc');
        } else {
          contentText = snippet; // Regular JS function that will be executed in the context of the calling thread.
        }
      } else if (typeof snippet === 'string' && snippet.startsWith(';')) {
        // In JS libraries
        //   foo: ';[code here verbatim]'
        //  emits
        //   'var foo;[code here verbatim];'
        contentText = 'var ' + finalName + snippet;
        if (snippet[snippet.length - 1] != ';' && snippet[snippet.length - 1] != '}') contentText += ';';
      } else {
        // In JS libraries
        //   foo: '=[value]'
        //  emits
        //   'var foo = [value];'
        if (typeof snippet === 'string' && snippet[0] == '=') snippet = snippet.substr(1);
        contentText = `var ${finalName} = ${snippet};`;
      }
      const sig = LibraryManager.library[ident + '__sig'];
      // asm module exports are done in emscripten.py, after the asm module is ready. Here
      // we also export library methods as necessary.
      if ((EXPORT_ALL || EXPORTED_FUNCTIONS.has(finalName)) && !noExport) {
        contentText += `\nModule["${finalName}"] = ${finalName};`;
      }
      if (MAIN_MODULE && sig) {
        contentText += `\n${finalName}.sig = '${sig}';`;
      }

      let commentText = '';
      if (LibraryManager.library[ident + '__docs']) {
        commentText = LibraryManager.library[ident + '__docs'] + '\n';
      }

      const depsText = (deps ? deps.map(addDependency).filter((x) => x != '').join('\n') + '\n' : '');
      return depsText + commentText + contentText;
    }

    itemsDict.functionStub.push(item);
    item.JS = addFromLibrary(item, TOP_LEVEL);
  }

  // Final combiner

  function finalCombiner() {
    const splitPostSets = splitter(itemsDict.globalVariablePostSet, (x) => x.ident && x.dependencies);
    itemsDict.globalVariablePostSet = splitPostSets.leftIn;
    const orderedPostSets = splitPostSets.splitOut;

    let limit = orderedPostSets.length * orderedPostSets.length;
    for (let i = 0; i < orderedPostSets.length; i++) {
      for (let j = i + 1; j < orderedPostSets.length; j++) {
        if (orderedPostSets[j].ident in orderedPostSets[i].dependencies) {
          const temp = orderedPostSets[i];
          orderedPostSets[i] = orderedPostSets[j];
          orderedPostSets[j] = temp;
          i--;
          limit--;
          assert(limit > 0, 'Could not sort postsets!');
          break;
        }
      }
    }

    itemsDict.globalVariablePostSet = itemsDict.globalVariablePostSet.concat(orderedPostSets);

    //

    if (!mainPass) {
      const generated = itemsDict.function.concat(itemsDict.type);
      print(generated.map((item) => item.JS).join('\n'));
      return;
    }

    const shellFile = SHELL_FILE ? SHELL_FILE : (MINIMAL_RUNTIME ? 'shell_minimal.js' : 'shell.js');

    const shellParts = read(shellFile).split('{{BODY}}');
    print(processMacros(preprocess(shellParts[0], shellFile)));
    let pre;
    if (MINIMAL_RUNTIME) {
      pre = processMacros(preprocess(read('preamble_minimal.js'), 'preamble_minimal.js'));
    } else {
      pre = processMacros(preprocess(read('support.js'), 'support.js')) +
            processMacros(preprocess(read('preamble.js'), 'preamble.js'));
    }
    print(pre);

    // Print out global variables and postsets TODO: batching
    const legalizedI64sDefault = legalizedI64s;
    legalizedI64s = false;

    runJSify(true);

    const generated = itemsDict.functionStub.concat(itemsDict.globalVariablePostSet);
    generated.forEach((item) => print(indentify(item.JS || '', 2)));

    legalizedI64s = legalizedI64sDefault;

    if (USE_PTHREADS) {
      print('\n // proxiedFunctionTable specifies the list of functions that can be called either synchronously or asynchronously from other threads in postMessage()d or internally queued events. This way a pthread in a Worker can synchronously access e.g. the DOM on the main thread.');
      print('\nvar proxiedFunctionTable = [' + proxiedFunctionTable.join() + '];\n');
    }

    if (!MINIMAL_RUNTIME) {
      print('var ASSERTIONS = ' + !!ASSERTIONS + ';\n');

      print(preprocess(read('arrayUtils.js')));
    }

    if ((SUPPORT_BASE64_EMBEDDING || FORCE_FILESYSTEM) && !MINIMAL_RUNTIME) {
      print(preprocess(read('base64Utils.js')));
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

    const postFile = MINIMAL_RUNTIME ? 'postamble_minimal.js' : 'postamble.js';
    const post = processMacros(preprocess(read(postFile), postFile));
    print(post);

    print(processMacros(preprocess(shellParts[1], shellFile)));

    print('\n//FORWARDED_DATA:' + JSON.stringify({
      libraryFunctions: libraryFunctions,
      ATINITS: ATINITS.join('\n'),
      ATMAINS: ATMAINS.join('\n'),
      ATEXITS: ATEXITS.join('\n'),
    }));
  }

  // Data

  if (mainPass) {
    functionStubs.forEach(functionStubHandler);
  }

  finalCombiner();
}
