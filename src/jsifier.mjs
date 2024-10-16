/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

import {
  ATEXITS,
  ATINITS,
  ATMAINS,
  defineI64Param,
  indentify,
  makeReturn64,
  modifyJSFunction,
  preprocess,
  processMacros,
  receiveI64ParamAsI53,
} from './parseTools.mjs';
import {
  addToCompileTimeContext,
  assert,
  error,
  errorOccured,
  isDecorator,
  isJsOnlySymbol,
  compileTimeContext,
  print,
  printErr,
  read,
  warn,
  warnOnce,
  warningOccured,
} from './utility.mjs';
import {LibraryManager, librarySymbols} from './modules.mjs';

const addedLibraryItems = {};

const extraLibraryFuncs = [];

// Some JS-implemented library functions are proxied to be called on the main
// browser thread, if the Emscripten runtime is executing in a Web Worker.
// Each such proxied function is identified via an ordinal number (this is not
// the same namespace as function pointers in general).
const proxiedFunctionTable = [];

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
  return {leftIn, splitOut};
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
      rtn += str + ',\n';
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
  // 'invoke_' symbols are created at runtime in library_dylink.py so can
  // always be considered as defined.
  if (RELOCATABLE && symName.startsWith('invoke_')) {
    return true;
  }
  return false;
}

function resolveAlias(symbol) {
  var value = LibraryManager.library[symbol];
  if (typeof value == 'string' && value[0] != '=' && LibraryManager.library.hasOwnProperty(value)) {
    return value;
  }
  return symbol;
}

function getTransitiveDeps(symbol, debug) {
  // TODO(sbc): Use some kind of cache to avoid quadratic behaviour here.
  const transitiveDeps = new Set();
  const seen = new Set();
  const toVisit = [symbol];
  while (toVisit.length) {
    const sym = toVisit.pop();
    if (!seen.has(sym)) {
      let directDeps = LibraryManager.library[sym + '__deps'] || [];
      directDeps = directDeps.filter((d) => typeof d === 'string');
      for (const dep of directDeps) {
        const resolved = resolveAlias(dep);
        if (VERBOSE && !transitiveDeps.has(dep)) {
          printErr(`adding dependency ${symbol} -> ${dep}`);
        }
        transitiveDeps.add(resolved);
        toVisit.push(resolved);
      }
      seen.add(sym);
    }
  }
  return Array.from(transitiveDeps);
}

function shouldPreprocess(fileName) {
  var content = read(fileName).trim();
  return content.startsWith('#preprocess\n') || content.startsWith('#preprocess\r\n');
}

function getIncludeFile(fileName, needsPreprocess) {
  let result = `// include: ${fileName}\n`;
  if (needsPreprocess) {
    result += processMacros(preprocess(fileName), fileName);
  } else {
    result += read(fileName);
  }
  result += `// end include: ${fileName}\n`;
  return result;
}

function preJS() {
  let result = '';
  for (const fileName of PRE_JS_FILES) {
    result += getIncludeFile(fileName, shouldPreprocess(fileName));
  }
  return result;
}

export function runJSify(symbolsOnly) {
  const libraryItems = [];
  const symbolDeps = {};
  const asyncFuncs = [];
  let postSets = [];

  LibraryManager.load();

  const symbolsNeeded = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
  symbolsNeeded.push(...extraLibraryFuncs);
  for (const sym of EXPORTED_RUNTIME_METHODS) {
    if ('$' + sym in LibraryManager.library) {
      symbolsNeeded.push('$' + sym);
    }
  }

  for (const key of Object.keys(LibraryManager.library)) {
    if (!isDecorator(key)) {
      if (INCLUDE_FULL_LIBRARY || EXPORTED_FUNCTIONS.has(mangleCSymbolName(key))) {
        symbolsNeeded.push(key);
      }
    }
  }

  function handleI64Signatures(symbol, snippet, sig, i53abi) {
    // Handle i64 parameters and return values.
    //
    // When WASM_BIGINT is enabled these arrive as BigInt values which we
    // convert to int53 JS numbers.  If necessary, we also convert the return
    // value back into a BigInt.
    //
    // When WASM_BIGINT is not enabled we receive i64 values as a pair of i32
    // numbers which is converted to single int53 number.  In necessary, we also
    // split the return value into a pair of i32 numbers.
    return modifyJSFunction(snippet, (args, body, async_, oneliner) => {
      let argLines = args.split('\n');
      argLines = argLines.map((line) => line.split('//')[0]);
      const argNames = argLines
        .join(' ')
        .split(',')
        .map((name) => name.trim());
      const newArgs = [];
      let argConversions = '';
      if (sig.length > argNames.length + 1) {
        error(`handleI64Signatures: signature too long for ${symbol}`);
        return snippet;
      }
      for (let i = 0; i < argNames.length; i++) {
        const name = argNames[i];
        // If sig is shorter than argNames list then argType will be undefined
        // here, which will result in the default case below.
        const argType = sig[i + 1];
        if (WASM_BIGINT && ((MEMORY64 && argType == 'p') || (i53abi && argType == 'j'))) {
          argConversions += `  ${receiveI64ParamAsI53(name, undefined, false)}\n`;
        } else {
          if (argType == 'j' && i53abi) {
            argConversions += `  ${receiveI64ParamAsI53(name, undefined, false)}\n`;
            newArgs.push(defineI64Param(name));
          } else if (argType == 'p' && CAN_ADDRESS_2GB) {
            argConversions += `  ${name} >>>= 0;\n`;
            newArgs.push(name);
          } else {
            newArgs.push(name);
          }
        }
      }

      if (!WASM_BIGINT) {
        args = newArgs.join(',');
      }

      if ((sig[0] == 'j' && i53abi) || (sig[0] == 'p' && MEMORY64)) {
        // For functions that where we need to mutate the return value, we
        // also need to wrap the body in an inner function.
        if (oneliner) {
          if (argConversions) {
            return `${async_}(${args}) => {
${argConversions}
  return ${makeReturn64(body)};
}`;
          }
          return `${async_}(${args}) => ${makeReturn64(body)};`;
        }
        return `\
${async_}function(${args}) {
${argConversions}
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
${argConversions}
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

    // Is this a shorthand `foo() {}` method syntax?
    // If so, prepend a function keyword so that it's valid syntax when extracted.
    if (snippet.startsWith(symbol)) {
      snippet = 'function ' + snippet;
    }

    if (isStub) {
      return snippet;
    }

    // apply LIBRARY_DEBUG if relevant
    if (LIBRARY_DEBUG && !isJsOnlySymbol(symbol)) {
      snippet = modifyJSFunction(snippet, (args, body, async, oneliner) => {
        var run_func;
        if (oneliner) {
          run_func = `var ret = ${body}`;
        } else {
          run_func = `var ret = (() => { ${body} })();`;
        }
        return `\
function(${args}) {
  if (runtimeDebug) err("[library call:${mangled}: " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]");
  ${run_func}
  if (runtimeDebug) err("  [     return:" + prettyPrint(ret));
  return ret;
}`;
      });
    }

    const sig = LibraryManager.library[symbol + '__sig'];
    const i53abi = LibraryManager.library[symbol + '__i53abi'];
    if (i53abi) {
      if (!sig) {
        error(`JS library error: '__i53abi' decorator requires '__sig' decorator: '${symbol}'`);
      }
      if (!sig.includes('j')) {
        error(
          `JS library error: '__i53abi' only makes sense when '__sig' includes 'j' (int64): '${symbol}'`,
        );
      }
    }
    if (
      sig &&
      ((i53abi && sig.includes('j')) || ((MEMORY64 || CAN_ADDRESS_2GB) && sig.includes('p')))
    ) {
      snippet = handleI64Signatures(symbol, snippet, sig, i53abi);
      compileTimeContext.i53ConversionDeps.forEach((d) => deps.push(d));
    }

    const proxyingMode = LibraryManager.library[symbol + '__proxy'];
    if (proxyingMode) {
      if (proxyingMode !== 'sync' && proxyingMode !== 'async' && proxyingMode !== 'none') {
        throw new Error(`Invalid proxyingMode ${symbol}__proxy: '${proxyingMode}' specified!`);
      }
      if (SHARED_MEMORY) {
        const sync = proxyingMode === 'sync';
        if (PTHREADS) {
          snippet = modifyJSFunction(snippet, (args, body, async_, oneliner) => {
            if (oneliner) {
              body = `return ${body}`;
            }
            const rtnType = sig && sig.length ? sig[0] : null;
            const proxyFunc =
              MEMORY64 && rtnType == 'p' ? 'proxyToMainThreadPtr' : 'proxyToMainThread';
            deps.push('$' + proxyFunc);
            return `
function(${args}) {
if (ENVIRONMENT_IS_PTHREAD)
  return ${proxyFunc}(${proxiedFunctionTable.length}, 0, ${+sync}${args ? ', ' : ''}${args});
${body}
}\n`;
          });
        } else if (WASM_WORKERS && ASSERTIONS) {
          // In ASSERTIONS builds add runtime checks that proxied functions are not attempted to be called in Wasm Workers
          // (since there is no automatic proxying architecture available)
          snippet = modifyJSFunction(
            snippet,
            (args, body) => `
function(${args}) {
  assert(!ENVIRONMENT_IS_WASM_WORKER, "Attempted to call proxied function '${mangled}' in a Wasm Worker, but in Wasm Worker enabled builds, proxied function architecture is not available!");
  ${body}
}\n`,
          );
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
    if (LINK_AS_CXX && !WASM_EXCEPTIONS && symbol.startsWith('__cxa_find_matching_catch_')) {
      if (DISABLE_EXCEPTION_THROWING) {
        error(
          'DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but exception catching code appears. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-except (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.',
        );
        return;
      }
      if (!(symbol in LibraryManager.library)) {
        // Create a new __cxa_find_matching_catch variant on demand.
        const num = +symbol.split('_').slice(-1)[0];
        compileTimeContext.addCxaCatch(num);
      }
      // Continue, with the code below emitting the proper JavaScript based on
      // what we just added to the library.
    }

    function addFromLibrary(symbol, dependent, force = false) {
      // don't process any special identifiers. These are looked up when
      // processing the base name of the identifier.
      if (isDecorator(symbol)) {
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

      if (!(symbol + '__deps' in LibraryManager.library)) {
        LibraryManager.library[symbol + '__deps'] = [];
      }

      const deps = LibraryManager.library[symbol + '__deps'];
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
        if (typeof original == 'function') {
          isAsyncFunction =
            LibraryManager.library[symbol + '__async'] ||
            original.constructor.name == 'AsyncFunction';
        }
        if (isAsyncFunction) {
          asyncFuncs.push(symbol);
        }
      }

      if (symbolsOnly) {
        if (LibraryManager.library.hasOwnProperty(symbol)) {
          // Resolve aliases before looking up deps
          var resolvedSymbol = resolveAlias(symbol);
          var transtiveDeps = getTransitiveDeps(resolvedSymbol);
          symbolDeps[symbol] = transtiveDeps.filter(
            (d) => !isJsOnlySymbol(d) && !(d in LibraryManager.library),
          );
        }
        return;
      }

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
            warnOnce(
              'To disable errors for undefined symbols use `-sERROR_ON_UNDEFINED_SYMBOLS=0`',
            );
            warnOnce(
              mangled +
                ' may need to be added to EXPORTED_FUNCTIONS if it arrives from a system library',
            );
          } else if (VERBOSE || WARN_ON_UNDEFINED_SYMBOLS) {
            warn(msg);
          }
          if (symbol === '__main_argc_argv' && STANDALONE_WASM) {
            warn('To build in STANDALONE_WASM mode without a main(), use emcc --no-entry');
          }
        }
        if (!RELOCATABLE) {
          // emit a stub that will fail at runtime
          LibraryManager.library[symbol] = new Function(`abort('missing function: ${symbol}');`);
          // We have already warned/errored about this function, so for the purposes of Closure use, mute all type checks
          // regarding this function, marking ot a variadic function that can take in anything and return anything.
          // (not useful to warn/error multiple times)
          LibraryManager.library[symbol + '__docs'] = '/** @type {function(...*):?} */';
          isStub = true;
        } else {
          // Create a stub for this symbol which can later be replaced by the
          // dynamic linker.  If this stub is called before the symbol is
          // resolved assert in debug builds or trap in release builds.
          let target = `wasmImports['${symbol}']`;
          if (ASYNCIFY) {
            // See the definition of asyncifyStubs in preamble.js for why this
            // is needed.
            target = `asyncifyStubs['${symbol}']`;
          }
          let assertion = '';
          if (ASSERTIONS) {
            assertion += `if (!${target} || ${target}.stub) abort("external symbol '${symbol}' is missing. perhaps a side module was not linked in? if this function was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment");\n`;
          }
          const functionBody = assertion + `return ${target}(...args);`;
          LibraryManager.library[symbol] = new Function('...args', functionBody);
          isStub = true;
        }
      }

      librarySymbols.push(mangled);

      const original = LibraryManager.library[symbol];
      let snippet = original;

      // Check for dependencies on `__internal` symbols from user libraries.
      const isUserSymbol = LibraryManager.library[symbol + '__user'];
      deps.forEach((dep) => {
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
        printErr(`adding ${symbol} (referenced by ${dependent})`);
      }
      function addDependency(dep) {
        // dependencies can be JS functions, which we just run
        if (typeof dep == 'function') {
          return dep();
        }
        // $noExitRuntime is special since there are conditional usages of it
        // in library.js and library_pthread.js.  These happen before deps are
        // processed so depending on it via `__deps` doesn't work.
        if (dep === '$noExitRuntime') {
          error(
            'noExitRuntime cannot be referenced via __deps mechanism.  Use DEFAULT_LIBRARY_FUNCS_TO_INCLUDE or EXPORTED_RUNTIME_METHODS',
          );
        }
        return addFromLibrary(dep, `${symbol}, referenced by ${dependent}`, dep === aliasTarget);
      }
      let contentText;
      if (isFunction) {
        // Emit the body of a JS library function.
        if (
          (USE_ASAN || USE_LSAN || UBSAN_RUNTIME) &&
          LibraryManager.library[symbol + '__noleakcheck']
        ) {
          contentText = modifyJSFunction(
            snippet,
            (args, body) => `(${args}) => withBuiltinMalloc(() => {${body}})`,
          );
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
        } else if (contentText.startsWith('class ')) {
          contentText = contentText.replace(/^class /, `class ${mangled} `);
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
        if (snippet[snippet.length - 1] != ';' && snippet[snippet.length - 1] != '}')
          contentText += ';';
      } else if (typeof snippet == 'undefined') {
        contentText = `var ${mangled};`;
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
      // Relocatable code needs signatures to create proper wrappers.
      if (sig && RELOCATABLE) {
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
        if (ASYNCIFY && MAIN_MODULE) {
          contentText += `\nasyncifyStubs['${symbol}'] = undefined;`;
        }
      }

      let commentText = '';
      if (force) {
        commentText += '/** @suppress {duplicate } */\n';
      }

      let docs = LibraryManager.library[symbol + '__docs'];
      if (docs) {
        commentText += docs + '\n';
      }

      if (EMIT_TSD) {
        LibraryManager.libraryDefinitions[mangled] = {
          docs: docs ?? null,
          snippet: snippet ?? null,
        };
      }

      const depsText = deps
        ? deps
            .map(addDependency)
            .filter((x) => x != '')
            .join('\n') + '\n'
        : '';
      return depsText + commentText + contentText;
    }

    const JS = addFromLibrary(symbol, 'root reference (e.g. compiled C/C++ code)');
    libraryItems.push(JS);
  }

  function includeFile(fileName, needsPreprocess = true) {
    print(getIncludeFile(fileName, needsPreprocess));
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

    postSets.push(...orderedPostSets);

    const shellFile = MINIMAL_RUNTIME ? 'shell_minimal.js' : 'shell.js';
    includeFile(shellFile);

    const preFile = MINIMAL_RUNTIME ? 'preamble_minimal.js' : 'preamble.js';
    includeFile(preFile);

    for (const item of libraryItems.concat(postSets)) {
      print(indentify(item || '', 2));
    }

    if (PTHREADS) {
      print(`
// proxiedFunctionTable specifies the list of functions that can be called
// either synchronously or asynchronously from other threads in postMessage()d
// or internally queued events. This way a pthread in a Worker can synchronously
// access e.g. the DOM on the main thread.
var proxiedFunctionTable = [
  ${proxiedFunctionTable.join(',\n  ')}
];
`);
    }

    if (errorOccured()) {
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
      includeFile(fileName, shouldPreprocess(fileName));
    }

    if (MODULARIZE) {
      includeFile('postamble_modularize.js');
    }

    print(
      '//FORWARDED_DATA:' +
        JSON.stringify({
          librarySymbols,
          warnings: warningOccured(),
          asyncFuncs,
          libraryDefinitions: LibraryManager.libraryDefinitions,
          ATINITS: ATINITS.join('\n'),
          ATMAINS: STRICT ? '' : ATMAINS.join('\n'),
          ATEXITS: ATEXITS.join('\n'),
        }),
    );
  }

  for (const sym of symbolsNeeded) {
    symbolHandler(sym);
  }

  if (symbolsOnly) {
    print(
      JSON.stringify({
        deps: symbolDeps,
        asyncFuncs,
        extraLibraryFuncs,
      }),
    );
  } else {
    finalCombiner();
  }

  if (errorOccured()) {
    throw Error('Aborting compilation due to previous errors');
  }
}

addToCompileTimeContext({
  extraLibraryFuncs,
  addedLibraryItems,
  preJS,
});
