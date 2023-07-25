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
  return { leftIn, splitOut };
}

function escapeJSONKey(x) {
  if (/^[\d\w_]+$/.exec(x) || x[0] === '"' || x[0] === "'") return x;
  assert(!x.includes("'"), 'cannot have internal single quotes in keys: ' + x);
  return "'" + x + "'";
}

// JSON.stringify will completely omit function objects.  This function is
// similar but preserves functions.
function stringifyWithFunctions(obj) {
  if (typeof obj == 'function') return obj.toString();
  if (obj === null || typeof obj != 'object') return JSON.stringify(obj);
  if (Array.isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  }
  var rtn = '{\n';
  for (const [key, value] of Object.entries(obj)) {
    var str = stringifyWithFunctions(value);
    // Handle JS method syntax where the function property starts with its own
    // name. e.g.  foo(a) {},
    if (typeof value === 'function' && str.startsWith(key)) {
      rtn += str + ',\n'
    } else {
      rtn += escapeJSONKey(key) + ':' + str + ',\n';
    }
  }
  return rtn + '}';
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

function getTransitiveDeps(symbol) {
  // TODO(sbc): Use some kind of cache to avoid quadratic behaviour here.
  const transitiveDeps = new Set();
  const seen = new Set();
  const toVisit = [symbol];
  while (toVisit.length) {
    const sym = toVisit.pop();
    if (!seen.has(sym)) {
      let directDeps = LibraryManager.library[sym + '__deps'] || [];
      directDeps = directDeps.filter((d) => typeof d === 'string');
      if (directDeps.length) {
        directDeps.forEach(transitiveDeps.add, transitiveDeps);
        toVisit.push(...directDeps);
      }
      seen.add(sym);
    }
  }
  return Array.from(transitiveDeps);
}

function runJSify() {
  const libraryItems = [];
  const symbolDeps = {};
  const asyncFuncs = [];
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
      if (!isDecorator(key)) {
        symbolsNeeded.push(key);
      }
    }
  }

  function handleI64Signatures(symbol, snippet, sig, i53abi) {
    // Handle i64 paramaters and return values.
    //
    // When WASM_BIGINT is enabled these arrive as BigInt values which we
    // convert to int53 JS numbers.  If necessary, we also convert the return
    // value back into a BigInt.
    //
    // When WASM_BIGINT is not enabled we receive i64 values as a pair of i32
    // numbers which is coverted to single int53 number.  In necessary, we also
    // split the return value into a pair of i32 numbers.
    return modifyJSFunction(snippet, (args, body, async_, oneliner) => {
      let argLines = args.split('\n');
      argLines = argLines.map((line) => line.split('//')[0]);
      const argNames = argLines.join(' ').split(',').map((name) => name.trim());
      const newArgs = [];
      let innerArgs = [];
      let argConvertions = '';
      for (let i = 1; i < sig.length; i++) {
        const name = argNames[i - 1];
        if (!name) {
          error(`handleI64Signatures: missing name for argument ${i} in ${symbol}`);
          return snippet;
        }
        if (WASM_BIGINT) {
          if (sig[i] == 'p' || (sig[i] == 'j' && i53abi)) {
            argConvertions += `  ${receiveI64ParamAsI53(name, undefined, false)}\n`;
          }
        } else {
          if (sig[i] == 'j' && i53abi) {
            argConvertions += `  ${receiveI64ParamAsI53(name, undefined, false)}\n`;
            newArgs.push(defineI64Param(name));
          } else if (sig[i] == 'p' && CAN_ADDRESS_2GB) {
            argConvertions += `  ${name} >>>= 0;\n`;
            newArgs.push(name);
          } else {
            newArgs.push(name);
          }
        }
      }

      var origArgs = args;
      if (!WASM_BIGINT) {
        args = newArgs.join(',');
      }

      if ((sig[0] == 'j' && i53abi) || (sig[0] == 'p' && WASM_BIGINT)) {
        // For functions that where we need to mutate the return value, we
        // also need to wrap the body in an inner function.
        if (oneliner) {
          if (argConvertions) {
            return `${async_}(${args}) => {
${argConvertions}
  return ${makeReturn64(body)};
}`
          }
          return `${async_}(${args}) => ${makeReturn64(body)};`
        }
        return `\
${async_}function(${args}) {
${argConvertions}
  var ret = (() => { ${body} })();
  return ${makeReturn64('ret')};
}`;
      }

      // Otherwise no inner function is needed and we covert the arguments
      // before executing the function body.
      if (oneliner) {
        body = `return ${body}`;
      }
      return `\
${async_}function(${args}) {
${argConvertions}
  ${body};
}`;
    });
  }

  function processLibraryFunction(snippet, symbol, mangled, deps, isStub) {
    // It is possible that when printing the function as a string on Windows,
    // the js interpreter we are in returns the string with Windows line endings
    // \r\n. This is undesirable, since line endings are managed in the form \n
    // in the output for binary file writes, so make sure the endings are
    // uniform.
    snippet = snippet.toString().replace(/\r\n/gm, '\n');

    if (isStub) {
      return snippet;
    }

    // apply LIBRARY_DEBUG if relevant
    if (LIBRARY_DEBUG && !isJsOnlySymbol(symbol)) {
      snippet = modifyJSFunction(snippet, (args, body, async) => `\
function(${args}) {
  var ret = (function() { if (runtimeDebug) err("[library call:${mangled}: " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]");
  ${body}
  }).apply(this, arguments);
  if (runtimeDebug && typeof ret != "undefined") err("  [     return:" + prettyPrint(ret));
  return ret;
}`);
    }

    const sig = LibraryManager.library[symbol + '__sig'];
    const i53abi = LibraryManager.library[symbol + '__i53abi'];
    if (sig &&
        ((i53abi && sig.includes('j')) || ((MEMORY64 || CAN_ADDRESS_2GB) && sig.includes('p')))) {
      snippet = handleI64Signatures(symbol, snippet, sig, i53abi);
      i53ConversionDeps.forEach((d) => deps.push(d))
    }

    if (SHARED_MEMORY) {
      const proxyingMode = LibraryManager.library[symbol + '__proxy'];
      if (proxyingMode) {
        if (proxyingMode !== 'sync' && proxyingMode !== 'async') {
          throw new Error(`Invalid proxyingMode ${symbol}__proxy: '${proxyingMode}' specified!`);
        }
        const sync = proxyingMode === 'sync';
        if (PTHREADS) {
          snippet = modifyJSFunction(snippet, (args, body) => `
function(${args}) {
if (ENVIRONMENT_IS_PTHREAD)
  return proxyToMainThread(${proxiedFunctionTable.length}, ${+sync}${args ? ', ' : ''}${args});
${body}
}\n`);
        } else if (WASM_WORKERS && ASSERTIONS) {
          // In ASSERTIONS builds add runtime checks that proxied functions are not attempted to be called in Wasm Workers
          // (since there is no automatic proxying architecture available)
          snippet = modifyJSFunction(snippet, (args, body) => `
function(${args}) {
  assert(!ENVIRONMENT_IS_WASM_WORKER, "Attempted to call proxied function '${mangled}' in a Wasm Worker, but in Wasm Worker enabled builds, proxied function architecture is not available!");
  ${body}
}\n`);
        }
        proxiedFunctionTable.push(mangled);
      }
    }

    return snippet;
  }

  function addImplicitDeps(snippet, deps) {
    // There are some common dependencies that we inject automatically by
    // conservatively scanning the input functions for their usage.
    // Specifically, these are dependencies that are very common and would be
    // burdensome to add manually to all functions.
    // The first four are deps that are automatically/conditionally added
    // by the {{{ makeDynCall }}}, and {{{ runtimeKeepalivePush/Pop }}} macros.
    const autoDeps = [
      'getDynCaller',
      'getWasmTableEntry',
      'runtimeKeepalivePush',
      'runtimeKeepalivePop',
      'UTF8ToString',
    ];
    for (const dep of autoDeps) {
      if (snippet.includes(dep + '(')) {
        deps.push('$' + dep);
      }
    }
  }

  function symbolHandler(symbol) {
    // In LLVM, exceptions generate a set of functions of form
    // __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.  where
    // the number specifies the number of arguments. In Emscripten, route all
    // these to a single function 'findMatchingCatch' that takes an array
    // of argument.
    if (!WASM_EXCEPTIONS && symbol.startsWith('__cxa_find_matching_catch_')) {
      if (DISABLE_EXCEPTION_THROWING) {
        error('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but exception catching code appears. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.');
        return;
      }
      if (!(symbol in LibraryManager.library)) {
        // Create a new __cxa_find_matching_catch variant on demand.
        const num = +symbol.split('_').slice(-1)[0];
        addCxaCatch(num);
      }
      // Continue, with the code below emitting the proper JavaScript based on
      // what we just added to the library.
    }

    function addFromLibrary(symbol, dependent, force = false) {
      // dependencies can be JS functions, which we just run
      if (typeof symbol == 'function') {
        return symbol();
      }

      // don't process any special identifiers. These are looked up when
      // processing the base name of the identifier.
      if (isDecorator(symbol)) {
        return;
      }

      if (!(symbol + '__deps' in LibraryManager.library)) {
        LibraryManager.library[symbol + '__deps'] = [];
      }

      deps = LibraryManager.library[symbol + '__deps'];
      let sig = LibraryManager.library[symbol + '__sig'];
      if (!WASM_BIGINT && sig && sig[0] == 'j') {
        // Without WASM_BIGINT functions that return i64 depend on setTempRet0
        // to return the upper 32-bits of the result.
        // See makeReturn64 in parseTools.py.
        deps.push('setTempRet0');
      }

      let isAsyncFunction = false;
      if (ASYNCIFY) {
        const original = LibraryManager.library[symbol];
        if (typeof original == 'function' ) {
          isAsyncFunction = LibraryManager.library[symbol + '__async'] ||
                            original.constructor.name == 'AsyncFunction'
        }
        if (isAsyncFunction) {
          asyncFuncs.push(symbol);
        }
      }

      if (symbolsOnly) {
        if (LibraryManager.library.hasOwnProperty(symbol)) {
          var value = LibraryManager.library[symbol];
          var resolvedSymbol = symbol;
          // Resolve aliases before looking up deps
          if (typeof value == 'string' && value[0] != '=' && LibraryManager.library.hasOwnProperty(value)) {
            resolvedSymbol = value;
          }
          var transtiveDeps = getTransitiveDeps(resolvedSymbol);
          symbolDeps[symbol] = transtiveDeps.filter((d) => !isJsOnlySymbol(d) && !(d in LibraryManager.library));
        }
        return;
      }

      // if the function was implemented in compiled code, there is no need to
      // include the js version
      if (WASM_EXPORTS.has(symbol) && !force) {
        return;
      }

      if (symbol in addedLibraryItems) {
        return;
      }
      addedLibraryItems[symbol] = true;

      // This gets set to true in the case of dynamic linking for symbols that
      // are undefined in the main module.  In this case we create a stub that
      // will resolve the correct symbol at runtime, or assert if its missing.
      let isStub = false;

      const mangled = mangleCSymbolName(symbol);

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
      let aliasTarget;

      if (typeof snippet == 'string') {
        if (snippet[0] != '=') {
          if (LibraryManager.library[snippet]) {
            // Redirection for aliases. We include the parent, and at runtime
            // make ourselves equal to it.  This avoid having duplicate
            // functions with identical content.
            aliasTarget = snippet;
            snippet = mangleCSymbolName(aliasTarget);
            deps.push(aliasTarget);
          }
        }
      } else if (typeof snippet == 'object') {
        snippet = stringifyWithFunctions(snippet);
        addImplicitDeps(snippet, deps);
      } else if (typeof snippet == 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, symbol, mangled, deps, isStub);
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
          postSets.push(postset + ';');
        }
      }

      if (VERBOSE) {
        printErr(`adding ${symbol} (referenced by ${dependent})`)
      }
      const deps_list = deps.join("','");
      const identDependents = symbol + `__deps: ['${deps_list}']`;
      function addDependency(dep) {
        return addFromLibrary(dep, `${symbol}, referenced by ${dependent}`, dep === aliasTarget);
      }
      let contentText;
      if (isFunction) {
        // Emit the body of a JS library function.
        if ((USE_ASAN || USE_LSAN || UBSAN_RUNTIME) && LibraryManager.library[symbol + '__noleakcheck']) {
          contentText = modifyJSFunction(snippet, (args, body) => `(${args}) => withBuiltinMalloc(() => {${body}})`);
          deps.push('$withBuiltinMalloc');
        } else {
          contentText = snippet; // Regular JS function that will be executed in the context of the calling thread.
        }
        // Give the function the correct (mangled) name. Overwrite it if it's
        // already named.  This must happen after the last call to
        // modifyJSFunction which could have changed or removed the name.
        if (contentText.match(/^\s*([^}]*)\s*=>/s)) {
          // Handle arrow functions
          contentText = `var ${mangled} = ` + contentText + ';';
        } else {
          // Handle regular (non-arrow) functions
          contentText = contentText.replace(/function(?:\s+([^(]+))?\s*\(/, `function ${mangled}(`);
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
      // asm module exports are done in emscripten.py, after the asm module is ready. Here
      // we also export library methods as necessary.
      if ((EXPORT_ALL || EXPORTED_FUNCTIONS.has(mangled)) && !isStub) {
        contentText += `\nModule['${mangled}'] = ${mangled};`;
      }
      // Relocatable code needs signatures to create proper wrappers. Stack
      // switching needs signatures so we can create a proper
      // WebAssembly.Function with the signature for the Promise API.
      // TODO: For asyncify we could only add the signatures we actually need,
      //       of async imports/exports.
      if (sig && (RELOCATABLE || ASYNCIFY == 2)) {
        if (!WASM_BIGINT) {
          sig = sig[0].replace('j', 'i') + sig.slice(1).replace(/j/g, 'ii');
        }
        contentText += `\n${mangled}.sig = '${sig}';`;
      }
      if (ASYNCIFY && isAsyncFunction) {
        contentText += `\n${mangled}.isAsync = true;`;
      }
      if (isStub) {
        contentText += `\n${mangled}.stub = true;`;
        if (ASYNCIFY) {
          contentText += `\nasyncifyStubs['${symbol}'] = undefined;`;
        }
      }

      let commentText = '';
      if (force) {
        commentText += '/** @suppress {duplicate } */\n'
      }
      if (LibraryManager.library[symbol + '__docs']) {
        commentText += LibraryManager.library[symbol + '__docs'] + '\n';
      }

      const depsText = (deps ? deps.map(addDependency).filter((x) => x != '').join('\n') + '\n' : '');
      return depsText + commentText + contentText;
    }

    const JS = addFromLibrary(symbol, 'root reference (e.g. compiled C/C++ code)');
    libraryItems.push(JS);
  }

  function includeFile(fileName) {
    print(`// include: ${fileName}`);
    print(processMacros(preprocess(fileName)));
    print(`// end include: ${fileName}`);
  }

  function includeFileRaw(fileName) {
    print(`// include: ${fileName}`);
    print(read(fileName));
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
      print(indentify(item || '', 2));
    }

    if (PTHREADS) {
      print('\n // proxiedFunctionTable specifies the list of functions that can be called either synchronously or asynchronously from other threads in postMessage()d or internally queued events. This way a pthread in a Worker can synchronously access e.g. the DOM on the main thread.');
      print('\nvar proxiedFunctionTable = [' + proxiedFunctionTable.join() + '];\n');
    }

    if (abortExecution) {
      throw Error('Aborting compilation due to previous errors');
    }

    // This is the main 'post' pass. Print out the generated code
    // that we have here, together with the rest of the output
    // that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    print('// EMSCRIPTEN_END_FUNCS\n');

    const postFile = MINIMAL_RUNTIME ? 'postamble_minimal.js' : 'postamble.js';
    includeFile(postFile);

    for (const fileName of POST_JS_FILES) {
      includeFileRaw(fileName);
    }

    print('//FORWARDED_DATA:' + JSON.stringify({
      librarySymbols,
      warnings,
      asyncFuncs,
      ATINITS: ATINITS.join('\n'),
      ATMAINS: STRICT ? '' : ATMAINS.join('\n'),
      ATEXITS: ATEXITS.join('\n'),
    }));
  }

  for (const sym of symbolsNeeded) {
    symbolHandler(sym);
  }

  if (symbolsOnly) {
    print(JSON.stringify({
      deps: symbolDeps,
      asyncFuncs
    }));
  } else {
    finalCombiner();
  }

  if (abortExecution) {
    throw Error('Aborting compilation due to previous errors');
  }
}
