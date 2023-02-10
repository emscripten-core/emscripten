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
  if (f === '__main_argc_argv') {
    f = 'main';
  }
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
function isJsOnlySymbol(symbol) {
  return symbol[0] == '$';
}

function escapeJSONKey(x) {
  if (/^[\d\w_]+$/.exec(x) || x[0] === '"' || x[0] === "'") return x;
  assert(!x.includes("'"), 'cannot have internal single quotes in keys: ' + x);
  return "'" + x + "'";
}

function stringifyWithFunctions(obj) {
  if (typeof obj == 'function') return obj.toString();
  if (obj === null || typeof obj != 'object') return JSON.stringify(obj);
  if (Array.isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  }
  return '{' + Object.keys(obj).map((key) => escapeJSONKey(key) + ':' + stringifyWithFunctions(obj[key])).join(',') + '}';
}

function isDefined(symName) {
  if (WASM_EXPORTS.has(symName) || SIDE_MODULE_EXPORTS.has(symName)) {
    return true;
  }
  if (symName == '__main_argc_argv' && SIDE_MODULE_EXPORTS.has('main')) {
    return true;
  }
  // 'invoke_' symbols are created at runtime in libary_dylink.py so can
  // always be considered as defined.
  if (RELOCATABLE && symName.startsWith('invoke_')) {
    return true;
  }
  return false;
}

function runJSify(symbolsOnly = false) {
  const libraryItems = [];
  let postSets = [];

  LibraryManager.load();

  const symbolsNeeded = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
  for (const sym of EXPORTED_RUNTIME_METHODS) {
    if ('$' + sym in LibraryManager.library) {
      symbolsNeeded.push('$' + sym);
    }
  }
  if (INCLUDE_FULL_LIBRARY) {
    for (const key of Object.keys(LibraryManager.library)) {
      if (!isJsLibraryConfigIdentifier(key)) {
        symbolsNeeded.push(key);
      }
    }
  }

  function convertPointerParams(snippet, sig) {
    // Automatically convert any incoming pointer arguments from BigInt
    // to double (this limits the range to int53).
    // And convert the return value if the function returns a pointer.
    return modifyFunction(snippet, (name, args, body) => {
      let argLines = args.split('\n');
      argLines = argLines.map((line) => line.split('//')[0]);
      const argNames = argLines.join(' ').split(',').map((name) => name.trim());
      let newArgs = [];
      let argConvertions = '';
      for (let i = 1; i < sig.length; i++) {
        const name = argNames[i - 1];
        if (sig[i] == 'p') {
          argConvertions += `  ${name} = Number(${name});\n`;
          newArgs.push(`Number(${name})`);
        } else {
          newArgs.push(name);
        }
      }

      if (sig[0] == 'p') {
        // For functions that return a pointer we need to convert
        // the return value too, which means we need to wrap the
        // body in an inner function.
        newArgs = newArgs.join(',');
        return `\
function ${name}(${args}) {
  var ret = ((${args}) => { ${body} })(${newArgs});
  return BigInt(ret);
}`;
      }

      // Otherwise no inner function is needed and we covert the arguments
      // before executing the function body.
      return `\
function ${name}(${args}) {
${argConvertions}
  ${body};
}`;
    });
  }

  function processLibraryFunction(snippet, symbol, mangled, deps) {
    // It is possible that when printing the function as a string on Windows,
    // the js interpreter we are in returns the string with Windows line endings
    // \r\n. This is undesirable, since line endings are managed in the form \n
    // in the output for binary file writes, so make sure the endings are
    // uniform.
    snippet = snippet.toString().replace(/\r\n/gm, '\n');

    // name the function; overwrite if it's already named
    snippet = snippet.replace(/function(?:\s+([^(]+))?\s*\(/, 'function ' + mangled + '(');

    // apply LIBRARY_DEBUG if relevant
    if (LIBRARY_DEBUG && !isJsOnlySymbol(symbol)) {
      snippet = modifyFunction(snippet, (name, args, body) => `\
function ${name}(${args}) {
  var ret = (function() { if (runtimeDebug) err("[library call:${mangled}: " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]");
  ${body}
  }).apply(this, arguments);
  if (runtimeDebug && typeof ret != "undefined") err("  [     return:" + prettyPrint(ret));
  return ret;
}`);
    }

    if (SHARED_MEMORY) {
      const proxyingMode = LibraryManager.library[symbol + '__proxy'];
      if (proxyingMode) {
        if (proxyingMode !== 'sync' && proxyingMode !== 'async') {
          throw new Error(`Invalid proxyingMode ${symbol}__proxy: '${proxyingMode}' specified!`);
        }
        const sync = proxyingMode === 'sync';
        if (USE_PTHREADS) {
          snippet = modifyFunction(snippet, (name, args, body) => `
function ${name}(${args}) {
if (ENVIRONMENT_IS_PTHREAD)
  return _emscripten_proxy_to_main_thread_js(${proxiedFunctionTable.length}, ${+sync}${args ? ', ' : ''}${args});
${body}
}\n`);
        } else if (WASM_WORKERS && ASSERTIONS) {
          // In ASSERTIONS builds add runtime checks that proxied functions are not attempted to be called in Wasm Workers
          // (since there is no automatic proxying architecture available)
          snippet = modifyFunction(snippet, (name, args, body) => `
function ${name}(${args}) {
  assert(!ENVIRONMENT_IS_WASM_WORKER, "Attempted to call proxied function '${name}' in a Wasm Worker, but in Wasm Worker enabled builds, proxied function architecture is not available!");
  ${body}
}\n`);
        }
        proxiedFunctionTable.push(mangled);
      }
    }

    if (MEMORY64) {
      const sig = LibraryManager.library[symbol + '__sig'];
      if (sig && sig.includes('p')) {
        snippet = convertPointerParams(snippet, sig);
      }
    }

    return snippet;
  }

  function addImplicitDeps(snippet, deps) {
    // There are some common dependencies that we inject automatically by
    // conservatively scanning the input functions for their usage.
    // Specifically, these are dependencies that are automatically generated by
    // the {{{ makeDynCall }}}, and {{{ runtimeKeepalivePush/Pop }}} macros.
    const autoDeps = [
      'getDynCaller',
      'getWasmTableEntry',
      'runtimeKeepalivePush',
      'runtimeKeepalivePop',
    ];
    for (const dep of autoDeps) {
      if (snippet.includes(dep + '(')) {
        deps.push('$' + dep);
      }
    }
  }

  function itemHandler(item) {
    // In LLVM, exceptions generate a set of functions of form
    // __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.  where
    // the number specifies the number of arguments. In Emscripten, route all
    // these to a single function '__cxa_find_matching_catch' that variadically
    // processes all of these functions using JS 'arguments' object.
    if (item.mangled.startsWith('___cxa_find_matching_catch_')) {
      if (DISABLE_EXCEPTION_THROWING) {
        error('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but exception catching code appears. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.');
        return;
      }
      const num = +item.mangled.split('_').slice(-1)[0];
      addCxaCatch(num);
      // Continue, with the code below emitting the proper JavaScript based on
      // what we just added to the library.
    }

    const TOP_LEVEL = 'top-level compiled C/C++ code';

    function addFromLibrary(item, dependent) {
      // dependencies can be JS functions, which we just run
      if (typeof item == 'function') {
        return item();
      }

      const symbol = item.symbol;
      const mangled = item.mangled;

      if (symbol in addedLibraryItems) {
        return;
      }
      addedLibraryItems[symbol] = true;

      // don't process any special identifiers. These are looked up when
      // processing the base name of the identifier.
      if (isJsLibraryConfigIdentifier(symbol)) {
        return;
      }

      if (symbolsOnly) {
        if (!isJsOnlySymbol(symbol) && LibraryManager.library.hasOwnProperty(symbol)) {
          librarySymbols.push(symbol);
        }
        return;
      }

      // if the function was implemented in compiled code, there is no need to
      // include the js version
      if (WASM_EXPORTS.has(symbol)) {
        return;
      }

      // This gets set to true in the case of dynamic linking for symbols that
      // are undefined in the main module.  In this case we create a stub that
      // will resolve the correct symbol at runtime, or assert if its missing.
      let isStub = false;

      if (!LibraryManager.library.hasOwnProperty(symbol)) {
        const isWeakImport = WEAK_IMPORTS.has(symbol);
        if (!isDefined(symbol) && !isWeakImport) {
          if (PROXY_TO_PTHREAD && !MAIN_MODULE && symbol == '__main_argc_argv') {
            error('PROXY_TO_PTHREAD proxies main() for you, but no main exists');
            return;
          }
          let undefinedSym = symbol;
          if (symbol === '__main_argc_argv') {
            undefinedSym = 'main/__main_argc_argv';
          }
          let msg = 'undefined symbol: ' + undefinedSym;
          if (dependent) msg += ` (referenced by ${dependent})`;
          if (ERROR_ON_UNDEFINED_SYMBOLS) {
            error(msg);
            if (dependent == TOP_LEVEL && !LLD_REPORT_UNDEFINED) {
              warnOnce('Link with `-sLLD_REPORT_UNDEFINED` to get more information on undefined symbols');
            }
            warnOnce('To disable errors for undefined symbols use `-sERROR_ON_UNDEFINED_SYMBOLS=0`');
            warnOnce(mangled + ' may need to be added to EXPORTED_FUNCTIONS if it arrives from a system library');
          } else if (VERBOSE || WARN_ON_UNDEFINED_SYMBOLS) {
            warn(msg);
          }
          if (symbol === '__main_argc_argv' && STANDALONE_WASM) {
            warn('To build in STANDALONE_WASM mode without a main(), use emcc --no-entry');
          }
        }
        if (!RELOCATABLE) {
          // emit a stub that will fail at runtime
          LibraryManager.library[symbol] = new Function(`err('missing function: ${symbol}'); abort(-1);`);
          // We have already warned/errored about this function, so for the purposes of Closure use, mute all type checks
          // regarding this function, marking ot a variadic function that can take in anything and return anything.
          // (not useful to warn/error multiple times)
          LibraryManager.library[symbol + '__docs'] = '/** @type {function(...*):?} */';
        } else {
          // Create a stub for this symbol which can later be replaced by the
          // dynamic linker.  If this stub is called before the symbol is
          // resolved assert in debug builds or trap in release builds.
          if (ASYNCIFY) {
            // See the definition of asyncifyStubs in preamble.js for why this
            // is needed.
            target = `asyncifyStubs['${symbol}']`;
          } else {
            target = `wasmImports['${symbol}']`;
          }
          let assertion = '';
          if (ASSERTIONS) {
            assertion += `if (!${target} || ${target}.stub) abort("external symbol '${symbol}' is missing. perhaps a side module was not linked in? if this function was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");\n`;
          }
          const functionBody = assertion + `return ${target}.apply(null, arguments);`;
          LibraryManager.library[symbol] = new Function(functionBody);
          isStub = true;
        }
      }

      librarySymbols.push(mangled);

      const original = LibraryManager.library[symbol];
      let snippet = original;
      const deps = LibraryManager.library[symbol + '__deps'] || [];
      if (!Array.isArray(deps)) {
        error(`JS library directive ${symbol}__deps=${deps.toString()} is of type ${typeof deps}, but it should be an array!`);
        return;
      }

      const isUserSymbol = LibraryManager.library[symbol + '__user'];
      deps.forEach((dep) => {
        if (typeof snippet == 'string' && !(dep in LibraryManager.library)) {
          warn(`missing library dependency ${dep}, make sure you are compiling with the right options (see #if in src/library*.js)`);
        }
        if (isUserSymbol && LibraryManager.library[dep + '__internal']) {
          warn(`user library symbol '${symbol}' depends on internal symbol '${dep}'`);
        }
      });
      let isFunction = false;

      if (typeof snippet == 'string') {
        if (snippet[0] != '=') {
          const target = LibraryManager.library[snippet];
          if (target) {
            // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
            // This avoid having duplicate functions with identical content.
            const redirectedTarget = snippet;
            deps.push(redirectedTarget);
            snippet = mangleCSymbolName(redirectedTarget);
          }
        }
      } else if (typeof snippet == 'object') {
        snippet = stringifyWithFunctions(snippet);
        addImplicitDeps(snippet, deps);
      } else if (typeof snippet == 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, symbol, mangled, deps);
        addImplicitDeps(snippet, deps);
      }

      const postsetId = symbol + '__postset';
      let postset = LibraryManager.library[postsetId];
      if (postset) {
        // A postset is either code to run right now, or some text we should emit.
        // If it's code, it may return some text to emit as well.
        if (typeof postset == 'function') {
          postset = postset();
        }
        if (postset && !addedLibraryItems[postsetId]) {
          addedLibraryItems[postsetId] = true;
          postSets.push({
            JS: postset + ';',
          });
        }
      }

      if (VERBOSE) {
        printErr(`adding ${mangled} and deps ${deps} : ` + (snippet + '').substr(0, 40));
      }
      const deps_list = deps.join("','");
      const identDependents = symbol + `__deps: ['${deps_list}']`;
      function addDependency(dep) {
        if (typeof dep != 'function') {
          dep = {symbol: dep, mangled: mangleCSymbolName(dep)};
        }
        return addFromLibrary(dep, `${identDependents}, referenced by ${dependent}`);
      }
      let contentText;
      if (isFunction) {
        // Emit the body of a JS library function.
        if ((USE_ASAN || USE_LSAN || UBSAN_RUNTIME) && LibraryManager.library[symbol + '__noleakcheck']) {
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
      } else if (typeof snippet == 'string' && snippet.startsWith(';')) {
        // In JS libraries
        //   foo: ';[code here verbatim]'
        //  emits
        //   'var foo;[code here verbatim];'
        contentText = 'var ' + mangled + snippet;
        if (snippet[snippet.length - 1] != ';' && snippet[snippet.length - 1] != '}') contentText += ';';
      } else {
        // In JS libraries
        //   foo: '=[value]'
        //  emits
        //   'var foo = [value];'
        if (typeof snippet == 'string' && snippet[0] == '=') {
          snippet = snippet.substr(1);
        }
        contentText = `var ${mangled} = ${snippet};`;
      }
      const sig = LibraryManager.library[symbol + '__sig'];
      // asm module exports are done in emscripten.py, after the asm module is ready. Here
      // we also export library methods as necessary.
      if ((EXPORT_ALL || EXPORTED_FUNCTIONS.has(mangled)) && !isStub) {
        contentText += `\nModule["${mangled}"] = ${mangled};`;
      }
      // Relocatable code needs signatures to create proper wrappers. Stack
      // switching needs signatures so we can create a proper
      // WebAssembly.Function with the signature for the Promise API.
      // TODO: For asyncify we could only add the signatures we actually need,
      //       of async imports/exports.
      if (sig && (RELOCATABLE || ASYNCIFY == 2)) {
        contentText += `\n${mangled}.sig = '${sig}';`;
      }
      if (isStub) {
        contentText += `\n${mangled}.stub = true;`;
        if (ASYNCIFY) {
          contentText += `\nasyncifyStubs['${symbol}'] = undefined;`;
        }
      }

      let commentText = '';
      if (LibraryManager.library[symbol + '__docs']) {
        commentText = LibraryManager.library[symbol + '__docs'] + '\n';
      }

      const depsText = (deps ? deps.map(addDependency).filter((x) => x != '').join('\n') + '\n' : '');
      return depsText + commentText + contentText;
    }

    libraryItems.push(item);
    item.JS = addFromLibrary(item, TOP_LEVEL);
  }

  function includeFile(fileName) {
    print(`// include: ${fileName}`);
    print(processMacros(preprocess(fileName)));
    print(`// end include: ${fileName}`);
  }

  function finalCombiner() {
    const splitPostSets = splitter(postSets, (x) => x.symbol && x.dependencies);
    postSets = splitPostSets.leftIn;
    const orderedPostSets = splitPostSets.splitOut;

    let limit = orderedPostSets.length * orderedPostSets.length;
    for (let i = 0; i < orderedPostSets.length; i++) {
      for (let j = i + 1; j < orderedPostSets.length; j++) {
        if (orderedPostSets[j].symbol in orderedPostSets[i].dependencies) {
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

    postSets = postSets.concat(orderedPostSets);

    const shellFile = MINIMAL_RUNTIME ? 'shell_minimal.js' : 'shell.js';
    includeFile(shellFile);

    const preFile = MINIMAL_RUNTIME ? 'preamble_minimal.js' : 'preamble.js';
    includeFile(preFile);

    for (const item of libraryItems.concat(postSets)) {
      print(indentify(item.JS || '', 2));
    }

    if (USE_PTHREADS) {
      print('\n // proxiedFunctionTable specifies the list of functions that can be called either synchronously or asynchronously from other threads in postMessage()d or internally queued events. This way a pthread in a Worker can synchronously access e.g. the DOM on the main thread.');
      print('\nvar proxiedFunctionTable = [' + proxiedFunctionTable.join() + '];\n');
    }

    if ((SUPPORT_BASE64_EMBEDDING || FORCE_FILESYSTEM) && !MINIMAL_RUNTIME) {
      includeFile('base64Utils.js');
    }

    if (abortExecution) throw Error('Aborting compilation due to previous errors');

    // This is the main 'post' pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    print('// EMSCRIPTEN_END_FUNCS\n');

    if (HEADLESS) {
      print('if (!ENVIRONMENT_IS_WEB) {');
      includeFile('headlessCanvas.js');
      includeFile('headless.js')
      print('}');
    }
    if (PROXY_TO_WORKER) {
      print('if (ENVIRONMENT_IS_WORKER) {\n');
      includeFile('webGLWorker.js');
      includeFile('proxyWorker.js');
      print('}');
    }
    if (DETERMINISTIC) {
      includeFile('deterministic.js');
    }

    const postFile = MINIMAL_RUNTIME ? 'postamble_minimal.js' : 'postamble.js';
    includeFile(postFile);

    for (const fileName of POST_JS_FILES) {
      includeFile(fileName);
    }

    print('//FORWARDED_DATA:' + JSON.stringify({
      librarySymbols: librarySymbols,
      warnings: warnings,
      ATINITS: ATINITS.join('\n'),
      ATMAINS: ATMAINS.join('\n'),
      ATEXITS: ATEXITS.join('\n'),
    }));
  }

  for (const sym of symbolsNeeded) {
    itemHandler({
      symbol: sym,
      mangled: mangleCSymbolName(sym),
    });
  }

  if (symbolsOnly) {
    print(JSON.stringify(librarySymbols));
    return;
  }

  finalCombiner();
}
