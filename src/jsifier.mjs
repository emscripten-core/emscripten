/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

import assert from 'node:assert';
import * as fs from 'node:fs/promises';
import * as acorn from 'acorn';
import {
  ATMODULES,
  ATEXITS,
  ATINITS,
  ATPOSTCTORS,
  ATPRERUNS,
  ATMAINS,
  ATPOSTRUNS,
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
  debugLog,
  error,
  errorOccured,
  extraLibraryFuncs,
  isDecorator,
  isJsOnlySymbol,
  compileTimeContext,
  readFile,
  runInMacroContext,
  warn,
  warnOnce,
  warningOccured,
  localFile,
  timer,
} from './utility.mjs';
import {
  extraExports,
  LibraryManager,
  librarySymbols,
  nativeAliases,
  shouldExportLibrarySymbol,
} from './modules.mjs';

const addedLibraryItems = {};

// Debug-only mode (`EMCC_CHECK_DEPS=1` in the environment) that performs a full
// AST parse on JS library items and reports an error if any local binding
// shadows a library symbol (which could cause false positives in the
// token-based `calcImplicitDeps` scanner above).
const CHECK_DEPS = process.env.EMCC_CHECK_DEPS;

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
  return f[0] == '$' ? f.slice(1) : '_' + f;
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
  if (typeof obj === 'function') return obj.toString();
  if (obj === null || typeof obj !== 'object') return JSON.stringify(obj);
  if (Array.isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  }

  // preserve the type of the object if it is one of [Map, Set, WeakMap, WeakSet].
  const builtinContainers = runInMacroContext('[Map, Set, WeakMap, WeakSet]', {
    filename: '<internal>',
  });
  for (const container of builtinContainers) {
    if (obj instanceof container) {
      const className = container.name;
      assert(!obj.size, `cannot stringify ${className} with data`);
      return `new ${className}`;
    }
  }

  var rtn = '{\n';
  for (const [key, value] of Object.entries(obj)) {
    var str = stringifyWithFunctions(value);
    // Handle JS method syntax where the function property starts with its own
    // name. e.g.  `foo(a) {}` (or `async foo(a) {}`)
    if (typeof value === 'function' && (str.startsWith(key) || str.startsWith('async ' + key))) {
      rtn += str + ',\n';
    } else {
      rtn += `${escapeJSONKey(key)}:${str},\n`;
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
  if (MAIN_MODULE && symName.startsWith('invoke_')) {
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
      let directDeps = LibraryManager.library[sym + '__deps'] ?? [];
      directDeps = directDeps.filter((d) => typeof d === 'string');
      for (const dep of directDeps) {
        if (!transitiveDeps.has(dep)) {
          debugLog(`adding dependency ${symbol} -> ${dep}`);
        }
        transitiveDeps.add(dep);
        toVisit.push(dep);
      }
      seen.add(sym);
    }
  }
  return Array.from(transitiveDeps);
}

function shouldPreprocess(fileName) {
  var content = readFile(fileName).trim();
  return content.startsWith('#preprocess\n') || content.startsWith('#preprocess\r\n');
}

function getIncludeFile(fileName, alwaysPreprocess, shortName) {
  shortName ??= fileName;
  let result = `// include: ${shortName}\n`;
  const doPreprocess = alwaysPreprocess || shouldPreprocess(fileName);
  if (doPreprocess) {
    result += processMacros(preprocess(fileName), fileName);
  } else {
    result += readFile(fileName);
  }
  result += `// end include: ${shortName}\n`;
  return result;
}

function getSystemIncludeFile(fileName) {
  return getIncludeFile(localFile(fileName), /*alwaysPreprocess=*/ true, /*shortName=*/ fileName);
}

function preJS() {
  let result = '';
  for (const fileName of PRE_JS_FILES) {
    result += getIncludeFile(fileName);
  }
  return result;
}

// Check if tokens[i] is the name of an object/class method definition `foo(...) {`
function isMethodDef(tokens, i) {
  if (tokens[i + 1]?.type.label !== '(') {
    return false;
  }
  let depth = 1;
  for (let j = i + 2; j < tokens.length; j++) {
    const label = tokens[j].type.label;
    if (label === '(') {
      depth++;
    } else if (label === ')') {
      depth--;
      if (depth === 0) {
        return tokens[j + 1]?.type.label === '{';
      }
    }
  }
  return false;
}

// Unlike JS library symbols, user code in `--pre-js`, `--post-js`, and
// `EM_JS`/`EM_ASM` snippets does not automatically pull in arbitrary JS
// library symbols (users must pass `-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE` or
// `-sEXPORTED_RUNTIME_METHODS`). Only these common runtime helpers and heap
// views are implicitly included when referenced in those snippets.
const implicitSnippetDeps = new Set([
  'getDynCaller',
  'getWasmTableEntry',
  'makeBigInt',
  'runtimeKeepalivePush',
  'runtimeKeepalivePop',
  'UTF8ToString',
  // TODO: Consider removing getValue and setValue if they are rarely used implicitly.
  'getValue',
  'setValue',
  'HEAP8',
  'HEAP16',
  'HEAPU8',
  'HEAPU16',
  'HEAP32',
  'HEAPU32',
  'HEAPF32',
  'HEAPF64',
  'HEAP64',
  'HEAPU64',
]);

function findLibrarySymbol(name, isUserSnippet = false) {
  let sym;
  if ('$' + name in LibraryManager.library) {
    sym = '$' + name;
  } else if (!isUserSnippet && name.startsWith('_') && name.slice(1) in LibraryManager.library) {
    sym = name.slice(1);
  }
  // $noExitRuntime cannot be referenced via __deps (it must be included via
  // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE or EXPORTED_RUNTIME_METHODS).
  if (sym && sym !== '$noExitRuntime') {
    return sym;
  }
}

// Scan a JS snippet for references to JS library symbols using `acorn.tokenizer`.
// Because this uses a flat token scan rather than full AST/scope analysis (for
// speed and to handle partial JS fragments in `--pre-js`/`--post-js`), it is
// conservative and can produce false positives if local variables or parameters
// shadow a library symbol name.  (We use a full AST walk in EMCC_CHECK_DEPS to
// detect such shadowing in system libraries).
function calcImplicitDeps(snippet, isUserSnippet = false) {
  const found = new Set();
  const typeofSyms = new Set();
  const tokens = Array.from(acorn.tokenizer(snippet, {ecmaVersion: 2022}));

  for (let i = 0; i < tokens.length; i++) {
    const tok = tokens[i];
    if (tok.type.label !== 'name') {
      continue;
    }
    const name = tok.value;
    const prevToken = tokens[i - 1];
    const nextToken = tokens[i + 1];

    // Ignore property accesses (e.g. `obj.foo` or `obj?.foo`)
    if (prevToken?.type.label === '.' || prevToken?.type.label === '?.') {
      continue;
    }
    // Ignore function or class declarations' own name (e.g. `function foo()` or `class Foo`)
    if (prevToken?.type.keyword === 'function' || prevToken?.type.keyword === 'class') {
      continue;
    }
    // Ignore object literal property keys (e.g. `{ foo: 123 }`)
    if (nextToken?.type.label === ':' && (prevToken?.type.label === '{' || prevToken?.type.label === ',')) {
      continue;
    }
    // Ignore object/class shorthand method definitions (e.g. `foo() { ... }`)
    if (isMethodDef(tokens, i)) {
      continue;
    }
    // Ignore `typeof foo` existence checks (e.g. `if (typeof SDL !== 'undefined') SDL.foo()`).
    if (prevToken?.type.keyword === 'typeof') {
      if (nextToken?.type.label !== '.') {
        typeofSyms.add(name);
      }
      continue;
    }
    if (typeofSyms.has(name)) {
      continue;
    }

    // If the snippet contains eval(), it may dynamically evaluate code loaded from memory at runtime
    // (for example, in emscripten_run_script where the snippet is eval(UTF8ToString(ptr))).
    // Because static analysis cannot inspect what strings are stored in memory or evaluated
    // at runtime, we must conservatively include all heap views whenever a snippet uses eval().
    if (name === 'eval' && nextToken?.type.label === '(') {
      found.add('$HEAP8');
      found.add('$HEAPU8');
      found.add('$HEAP16');
      found.add('$HEAPU16');
      found.add('$HEAP32');
      found.add('$HEAPU32');
      found.add('$HEAPF32');
      found.add('$HEAPF64');
      if (WASM_BIGINT || MEMORY64) {
        found.add('$HEAP64');
        found.add('$HEAPU64');
      }
      continue;
    }

    // When scanning `--pre-js`, `--post-js`, or `EM_JS`/`EM_ASM`,
    // only auto-include symbols from `implicitSnippetDeps`.
    if (isUserSnippet && !implicitSnippetDeps.has(name)) {
      continue;
    }

    const dep = findLibrarySymbol(name, isUserSnippet);
    if (dep) {
      found.add(dep);
    }
  }

  return found;
}

function checkShadowing(symbol, mangled, contentText) {
  function checkBoundName(name) {
    if (name === mangled) return;
    const shadowed = findLibrarySymbol(name);
    if (shadowed) {
      error(`${symbol}: local binding '${name}' shadows library symbol '${shadowed}'`);
    }
  }

  function walkPattern(node) {
    if (!node) return;
    switch (node.type) {
      case 'Identifier':
        checkBoundName(node.name);
        break;
      case 'AssignmentPattern':
        walkPattern(node.left);
        break;
      case 'ArrayPattern':
        node.elements.forEach(walkPattern);
        break;
      case 'ObjectPattern':
        node.properties.forEach((p) => walkPattern(p.value || p.argument));
        break;
      case 'RestElement':
        walkPattern(node.argument);
        break;
    }
  }

  function walk(node) {
    switch (node.type) {
      case 'VariableDeclarator':
        walkPattern(node.id);
        break;
      case 'FunctionDeclaration':
      case 'FunctionExpression':
      case 'ArrowFunctionExpression':
        if (node.id) checkBoundName(node.id.name);
        node.params.forEach(walkPattern);
        break;
      case 'ClassDeclaration':
      case 'ClassExpression':
        if (node.id) checkBoundName(node.id.name);
        break;
      case 'CatchClause':
        walkPattern(node.param);
        break;
    }
    for (const child of Object.values(node)) {
      if (typeof child?.type === 'string') {
        walk(child);
      } else if (Array.isArray(child)) {
        for (const elem of child) {
          if (typeof elem?.type === 'string') walk(elem);
        }
      }
    }
  }

  walk(acorn.parse(contentText, {ecmaVersion: 2022}));
}

function sigToArgs(sig) {
  const args = []
  for (var i = 1; i < sig.length; i++) {
    args.push(`a${i}`);
  }
  return args.join(',');
}

function handleI64Signatures(symbol, snippet, sig, i53abi, isAsyncFunction) {
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
      error(`handleI64Signatures: signature '${sig}' too long for ${symbol}(${argNames.join(', ')})`);
      return snippet;
    }
    for (const [i, name] of argNames.entries()) {
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

      // If the inner function is marked as `__async` then we need to `await`
      // the result before casting it to BigInt.  Note that we use the `__async`
      // attribute here rather than the presence of the `async` JS keyword
      // because this is what tells us that the function is going to return
      // a promise.  i.e. we support async functions that return promises but
      // are not marked with the `async` keyword (the latter is only necessary
      // if the function uses the `await` keyword))
      const await_ = isAsyncFunction ? 'await ' : '';
      const orig_async_ = async_;
      async_ = isAsyncFunction ? 'async ' : async_;
      // An 'auto' body may return a Promise even without ASYNCIFY (on a
      // pthread's behalf, which awaits it), which must not be cast but
      // resolve to the cast value.
      const maybePromise = !isAsyncFunction && ['auto', 'proxy'].includes(LibraryManager.library[symbol + '__async']);
      const convert = (v) => maybePromise ?
        `(${v} instanceof Promise ? ${v}.then((v) => ${makeReturn64('v')}) : ${makeReturn64(v)})` :
        makeReturn64(await_ + v);
      if (oneliner) {
        // Special case for abort(), this a noreturn function and but closure
        // compiler doesn't have a way to express that, so it complains if we
        // do `BigInt(abort(..))`.
        if (body.startsWith('abort(')) {
          return snippet;
        }
        if (argConversions || maybePromise) {
          return `${async_}(${args}) => {
${argConversions}
var ret = ${body};
return ${convert('ret')};
}`;
        }
        return `${async_}(${args}) => ${makeReturn64(await_ + body)};`;
      }
      return `\
${async_}function(${args}) {
${argConversions}
var ret = (${orig_async_}() => { ${body} })();
return ${convert('ret')};
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

// An `__async: 'auto'` or `'proxy'` body returns a value or a Promise, as it
// can. A body whose last parameter is named `sync` receives there whether it
// must complete synchronously on this call, as a returned Promise could not be
// waited for. That is never the case for an 'auto' function under
// ASYNCIFY/JSPI, nor on behalf of a sync-proxied pthread caller, which awaits
// it; otherwise it is.
function handleSyncArg(symbol, snippet, sig, proxyingMode, canWait) {
  const sync = canWait ? 'false' :
    PTHREADS && proxyingMode == 'sync' ? '!PThread.currentProxiedOperationCallerThread' : 'true';
  return modifyJSFunction(snippet, (args, body, async_, oneliner) => {
    const params = args.split(',').map((a) => a.trim()).filter((a) => a);
    if (params.at(-1) != 'sync') return snippet;
    if (sig && params.length != sig.length) {
      error(`JS library error: '${symbol}' must take 'sync' as parameter ${sig.length}, after the ${sig.length - 1} of its '__sig'`);
      return snippet;
    }
    if (!oneliner) body = `{\n${body}\n}`;
    const outer = params.slice(0, -1).join(', ');
    return `\
function(${outer}) {
  return (${async_}(${args}) => ${body})(${outer}${outer ? ', ' : ''}${sync});
}\n`;
  });
}

function handleAsyncFunction(snippet, sig, proxied) {
  const return64 = sig && (MEMORY64 && sig.startsWith('p') || sig.startsWith('j'))
  let handleAsync = 'Asyncify.handleAsync(innerFunc)'
  if (return64 && ASYNCIFY == 1) {
    handleAsync = makeReturn64(handleAsync);
  }
  // When dispatching on behalf of a proxied caller (PROXY_SYNC_ASYNC), the
  // caller awaits the returned promise, so return it directly rather than
  // suspending the main thread.
  let proxiedDispatch = '';
  if (ASYNCIFY == 1 && PTHREADS && proxied) {
    proxiedDispatch = 'if (PThread.currentProxiedOperationCallerThread) return innerFunc();\n  ';
  }
  return modifyJSFunction(snippet, (args, body, async_, oneliner) => {
    if (!oneliner) {
      body = `{\n${body}\n}`;
    }
    return `\
function(${args}) {
  let innerFunc = ${async_} () => ${body};
  ${proxiedDispatch}return ${handleAsync};
}\n`;
  });
}

// The three different inter-thread proxying methods.
// See system/lib/pthread/proxying.c
const PROXY_ASYNC = 0;
const PROXY_SYNC = 1;
const PROXY_SYNC_ASYNC = 2;

export async function runJSify(outputFile, symbolsOnly) {
  const libraryItems = [];
  const libraryAliases = [];
  const asyncFuncs = [];
  let postSets = [];

  LibraryManager.load();

  let outputHandle = process.stdout;
  if (outputFile) {
    outputHandle = await fs.open(outputFile, 'w');
  }

  async function writeOutput(str) {
    // Unmangle previously mangled `import.meta` references.
    // See also: `mangleUnsupportedSyntax` in parseTools.mjs.
    if (EXPORT_ES6) {
      str = str.replaceAll('EMSCRIPTEN$IMPORT$META', 'import.meta');
    }

    await outputHandle.write(str + '\n');
  }

  const symbolsNeeded = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
  symbolsNeeded.push(...extraLibraryFuncs);

  if (!symbolsOnly) {
    for (const fileName of [...PRE_JS_FILES, ...POST_JS_FILES]) {
      const content = getIncludeFile(fileName);
      if (!errorOccured()) {
        symbolsNeeded.push(...calcImplicitDeps(content, /*isUserSnippet=*/ true));
      }
    }
    for (const snippet of EM_JS_SNIPPETS) {
      symbolsNeeded.push(...calcImplicitDeps(snippet, /*isUserSnippet=*/ true));
    }
  }
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
      snippet = modifyJSFunction(snippet, (args, body, _async, oneliner) => {
        var run_func;
        if (oneliner) {
          run_func = `var ret = ${body}`;
        } else {
          run_func = `var ret = (() => { ${body} })();`;
        }
        return `\
function(${args}) {
  dbg("[library call:${mangled}: " + Array.prototype.slice.call(arguments).map(prettyPrint) + "]");
  ${run_func}
  dbg("  [     return:" + prettyPrint(ret));
  return ret;
}`;
      });
    }

    const sig = LibraryManager.library[symbol + '__sig'];
    const asyncMode = LibraryManager.library[symbol + '__async'];
    // A 'proxy' function never suspends: a pthread's proxied call is its only
    // asynchronous path, so it stays a plain import under ASYNCIFY/JSPI.
    const isAsyncFunction = ASYNCIFY && asyncMode != 'proxy' && asyncMode;

    const i53abi = LibraryManager.library[symbol + '__i53abi'];
    if (i53abi) {
      if (!sig) {
        error(`JS library error: '__i53abi' decorator requires '__sig' decorator: '${symbol}'`);
      }
      if (!sig.includes('j')) {
        error(`JS library error: '__i53abi' only makes sense when '__sig' includes 'j' (int64): '${symbol}'`);
      }
    }
    if (
      sig &&
      ((i53abi && sig.includes('j')) || ((MEMORY64 || CAN_ADDRESS_2GB) && sig.includes('p')))
    ) {
      snippet = handleI64Signatures(symbol, snippet, sig, i53abi, isAsyncFunction);
    }

    const proxyingMode = LibraryManager.library[symbol + '__proxy'];

    if (asyncMode == 'proxy' && proxyingMode != 'sync') {
      error(`JS library error: '${symbol}' is '__async: 'proxy'' and requires '__proxy: 'sync''`);
    }
    if (asyncMode == 'auto' || asyncMode == 'proxy') {
      snippet = handleSyncArg(symbol, snippet, sig, proxyingMode, isAsyncFunction);
    }

    if (ASYNCIFY && isAsyncFunction == 'auto') {
      snippet = handleAsyncFunction(snippet, sig, proxyingMode == 'sync');
    }

    if (proxyingMode) {
      if (!['sync', 'async', 'none'].includes(proxyingMode)) {
        error(`JS library error: invalid proxying mode '${symbol}__proxy: ${proxyingMode}' specified`);
      }
      if (SHARED_MEMORY && proxyingMode != 'none') {
        if (PTHREADS) {
          snippet = modifyJSFunction(snippet, (args, body, async_, oneliner) => {
            if (oneliner) {
              body = `return ${body}`;
            }
            let proxyMode = PROXY_ASYNC;
            if (proxyingMode === 'sync') {
              const isAsyncFunction = LibraryManager.library[symbol + '__async'];
              if (isAsyncFunction) {
                proxyMode = PROXY_SYNC_ASYNC;
              } else {
                proxyMode = PROXY_SYNC;
              }
            }
            const rtnType = sig?.[0];
            const proxyFunc =
              MEMORY64 && rtnType == 'p' ? 'proxyToMainThreadPtr' : 'proxyToMainThread';
            return `
${async_}function(${args}) {
if (ENVIRONMENT_IS_PTHREAD)
  return ${proxyFunc}(${proxiedFunctionTable.length}, 0, ${proxyMode}${args ? ', ' : ''}${args});
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
  assert(!ENVIRONMENT_IS_WASM_WORKER, "attempt to call proxied function '${mangled}' from a Wasm Worker (where proxying is not possible)");
  ${body}
}\n`,
          );
        }
        proxiedFunctionTable.push(mangled);
      }
    }

    return snippet;
  }

  function symbolHandler(symbol) {
    // In LLVM, exceptions generate a set of functions of form
    // __cxa_find_matching_catch_1(), __cxa_find_matching_catch_2(), etc.  where
    // the number specifies the number of arguments. In Emscripten, route all
    // these to a single function 'findMatchingCatch' that takes an array
    // of argument.
    if (LINK_AS_CXX && !WASM_EXCEPTIONS && symbol.startsWith('__cxa_find_matching_catch_')) {
      if (DISABLE_EXCEPTION_THROWING) {
        error('DISABLE_EXCEPTION_THROWING was set (likely due to -fno-exceptions), which means no C++ exception throwing support code is linked in, but exception catching code appears. Either do not set DISABLE_EXCEPTION_THROWING (if you do want exception throwing) or compile all source files with -fno-exceptions (so that no exceptions support code is required); also make sure DISABLE_EXCEPTION_CATCHING is set to the right value - if you want exceptions, it should be off, and vice versa.');
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

    function addFromLibrary(symbol, dependent) {
      // don't process any special identifiers. These are looked up when
      // processing the base name of the identifier.
      if (isDecorator(symbol)) {
        return;
      }

      // if the function was implemented in compiled code, there is no need to
      // include the js version
      if (WASM_EXPORTS.has(symbol)) {
        return;
      }

      if (symbol in addedLibraryItems) {
        return;
      }
      addedLibraryItems[symbol] = true;

      const deps = LibraryManager.library[symbol + '__deps'] ??= [];
      const explicitDeps = new Set(deps);
      let sig = LibraryManager.library[symbol + '__sig'];
      if (!WASM_BIGINT && sig && sig[0] == 'j') {
        // Without WASM_BIGINT functions that return i64 depend on setTempRet0
        // to return the upper 32-bits of the result.
        // See makeReturn64 in parseTools.py.
        deps.push('setTempRet0');
      }

      const asyncMode = LibraryManager.library[symbol + '__async'];
      const isAsyncFunction = asyncMode && asyncMode != 'proxy';
      if (ASYNCIFY && isAsyncFunction) {
        asyncFuncs.push(symbol);
      }

      const isUserSymbol = LibraryManager.library[symbol + '__user'];

      function addImplicitDeps(snippet) {
        for (const dep of calcImplicitDeps(snippet)) {
          if (dep === symbol) continue;
          if (explicitDeps.has(dep)) {
            if (!symbolsOnly && (STRICT || !isUserSymbol) && LibraryManager.library[symbol] !== dep) {
              warn(`${symbol}: redundant __deps entry: ${dep}`);
              explicitDeps.delete(dep);
            }
          } else if (!deps.includes(dep)) {
            if (!symbolsOnly) {
              debugLog(`auto-detected dependency: ${symbol} -> ${dep}`);
            }
            deps.push(dep);
          }
        }
      }

      const postsetId = symbol + '__postset';
      const postset = LibraryManager.library[postsetId];

      if (symbolsOnly) {
        if (LibraryManager.library.hasOwnProperty(symbol)) {
          let snippet = LibraryManager.library[symbol];
          if (!LibraryManager.isAlias(snippet)) {
            if (typeof snippet === 'object') {
              snippet = stringifyWithFunctions(snippet);
            } else if (typeof snippet === 'function') {
              snippet = snippet.toString();
            } else if (typeof snippet === 'string') {
              snippet = snippet.replace(/^[=;]/, '');
            }
            if (typeof snippet === 'string') {
              addImplicitDeps(snippet);
            }
          }
          if (typeof postset === 'string') {
            addImplicitDeps(postset);
          }
        }
        return;
      }

      // This gets set to true in the case of dynamic linking for symbols that
      // are undefined in the main module.  In this case we create a stub that
      // will resolve the correct symbol at runtime, or assert if its missing.
      let isStub = false;

      const mangled = mangleCSymbolName(symbol);
      let snippet = LibraryManager.library[symbol];

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
          } else if (WARN_ON_UNDEFINED_SYMBOLS) {
            warn(msg);
          } else {
            debugLog(msg);
          }
          if (symbol === '__main_argc_argv' && STANDALONE_WASM) {
            warn('To build in STANDALONE_WASM mode without a main(), use emcc --no-entry');
          }
        }

        // emit a stub that will fail at runtime
        var stubFunctionBody = `abort('missing function: ${symbol}');`
        if (MAIN_MODULE) {
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
          stubFunctionBody = assertion + `return ${target}(...args);`;
        }
        isStub = true;
        snippet = new Function('...args', stubFunctionBody);
      }

      librarySymbols.push(mangled);

      if (!isStub && LibraryManager.library[symbol + '__export']) {
        extraExports.add(mangled);
      }

      let isFunction = typeof snippet === 'function';
      let isNativeAlias = false;

      if (postset) {
        // A postset is either code to run right now, or some text we should emit.
        // If it's code, it may return some text to emit as well.
        const postsetString = typeof postset === 'function' ? postset() : postset;
        if (postsetString && !addedLibraryItems[postsetId]) {
          addedLibraryItems[postsetId] = true;
          postSets.push(postsetString + ';');
          if (typeof postset === 'string') {
            addImplicitDeps(postsetString);
          }
        }
      }

      let aliasTarget;
      if (LibraryManager.isAlias(snippet)) {
        // Redirection for aliases. We include the parent, and at runtime
        // make ourselves equal to it.  This avoid having duplicate
        // functions with identical content.
        aliasTarget = snippet;
        if (WASM_EXPORTS.has(aliasTarget)) {
          debugLog(`native alias: ${mangled} -> ${aliasTarget}`);
          nativeAliases[mangled] = aliasTarget;
          snippet = undefined;
          isNativeAlias = true;
        } else {
          debugLog(`js alias: ${mangled} -> ${aliasTarget}`);
          snippet = mangleCSymbolName(aliasTarget);
          // When we have an alias for another JS function we can normally
          // point them at the same function.  However, in some cases (where
          // signatures are relevant and they differ between and alais and
          // it's target) we need to construct a forwarding function from
          // one to the other.
          const isSigRelevant = MAIN_MODULE || MEMORY64 || CAN_ADDRESS_2GB || sig?.includes('j');
          const targetSig = LibraryManager.library[aliasTarget + '__sig'];
          if (isSigRelevant && sig && targetSig && sig != targetSig) {
            debugLog(`${symbol}: Alias target (${aliasTarget}) has different signature (${sig} vs ${targetSig})`)
            isFunction = true;
            snippet = `(${sigToArgs(sig)}) => ${snippet}(${sigToArgs(targetSig)})`;
          }
        }
      } else if (typeof snippet === 'object') {
        snippet = stringifyWithFunctions(snippet);
        addImplicitDeps(snippet);
      } else if (typeof snippet === 'string') {
        if (snippet.match(/^\s*\([^}]*\)\s*=>/) || snippet.match(/^function\b/)) {
          // Support functions that are already "stringified"
          isFunction = true;
        } else {
          addImplicitDeps(snippet.replace(/^[=;]/, ''));
        }
      }

      if (isFunction) {
        snippet = processLibraryFunction(snippet, symbol, mangled, deps, isStub);
        if (!isStub) {
          addImplicitDeps(snippet);
        }
      }

      // Check for dependencies on `__internal` symbols from user libraries.
      if (isUserSymbol) {
        for (const dep of deps) {
          if (LibraryManager.library[dep + '__internal']) {
            warn(`user library symbol '${symbol}' depends on internal symbol '${dep}'`);
          }
        }
      }

      debugLog(`adding ${symbol} (referenced by ${dependent})`);
      function addDependency(dep) {
        // dependencies can be JS functions, which we just run
        if (typeof dep === 'function') {
          return dep();
        }
        // $noExitRuntime is special since there are conditional usages of it
        // in libcore.js and libpthread.js.  These happen before deps are
        // processed so depending on it via `__deps` doesn't work.
        if (dep === '$noExitRuntime') {
          error('noExitRuntime cannot be referenced via __deps mechanism.  Use DEFAULT_LIBRARY_FUNCS_TO_INCLUDE or EXPORTED_RUNTIME_METHODS');
        }
        return addFromLibrary(dep, `${symbol}, referenced by ${dependent}`);
      }
      let contentText;
      if (isFunction) {
        // Emit the body of a JS library function.
        if ((USE_ASAN || USE_LSAN) && LibraryManager.library[symbol + '__noleakcheck']) {
          contentText = modifyJSFunction(
            snippet,
            (args, body) => `(${args}) => noLeakCheck(() => {${body}})`,
          );
          deps.push('$noLeakCheck');
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
          // Handle class declarations (which also have typeof == 'function'.)
          contentText = contentText.replace(/^class(?:\s+(?!extends\b)[^{\s]+)?/, `class ${mangled}`);
        } else {
          // Handle regular (non-arrow) functions
          contentText = contentText.replace(/function(?:\s+([^(]+))?\s*\(/, `function ${mangled}(`);
        }
      } else if (typeof snippet === 'string' && snippet.startsWith(';')) {
        // In JS libraries
        //   foo: ';[code here verbatim]'
        //  emits
        //   'var foo;[code here verbatim];'
        contentText = 'var ' + mangled + snippet;
        if (snippet[snippet.length - 1] != ';' && snippet[snippet.length - 1] != '}') {
          contentText += ';';
        }
      } else if (typeof snippet === 'undefined') {
        // For JS library functions that are simply aliases of native symbols,
        // we don't need to generate anything here.  Instead these get included
        // and exported alongside native symbols.
        // See `create_receiving` in `tools/emscripten.py`.
        if (isNativeAlias) {
          contentText = '';
        } else {
          contentText = `var ${mangled};`;
        }
      } else {
        // In JS libraries
        //   foo: '=[value]'
        //  emits
        //   'var foo = [value];'
        if (typeof snippet === 'string' && snippet[0] == '=') {
          snippet = snippet.slice(1);
        }
        contentText = `var ${mangled} = ${snippet};`;
      }

      if (CHECK_DEPS && (STRICT || !isUserSymbol) && !isStub && contentText) {
        checkShadowing(symbol, mangled, contentText);
      }

      if (contentText && MODULARIZE == 'instance' && shouldExportLibrarySymbol(mangled) && !isStub) {
        // In MODULARIZE=instance mode mark JS library symbols are exported at
        // the point of declaration.
        contentText = 'export ' + contentText;
      }

      // Dynamic linking needs signatures to create proper wrappers.
      if (sig && MAIN_MODULE && !isNativeAlias) {
        if (!WASM_BIGINT) {
          sig = sig[0].replace('j', 'i') + sig.slice(1).replace(/j/g, 'ii');
        }
        contentText += `\n${mangled}.sig = '${sig}';`;
      }
      if (ASYNCIFY && isAsyncFunction) {
        assert(isFunction);
        contentText += `\n${mangled}.isAsync = true;`;
      }
      if (isStub) {
        contentText += `\n${mangled}.stub = true;`;
        if (ASYNCIFY && MAIN_MODULE) {
          contentText += `\nasyncifyStubs['${symbol}'] = undefined;`;
        }
      }

      // Add the docs if they exist and if we are actually emitting a declaration.
      // See the TODO about wasmTable above.
      let docs = LibraryManager.library[symbol + '__docs'];
      const commentText = contentText && docs ? docs : '';

      if (EMIT_TSD) {
        LibraryManager.libraryDefinitions[mangled] = {
          docs: docs ?? null,
          snippet: snippet ?? null,
        };
      }

      const joinChunks = (chunks) => chunks.filter(Boolean).join('\n');

      // Sort string dependencies alphabetically for deterministic output order
      // regardless of how __deps are declared or discovered. Keep function
      // callbacks after string dependencies in their original order.
      const stringDeps = deps.filter((d) => typeof d !== 'function').sort();
      const funcDeps = deps.filter((d) => typeof d === 'function');
      const depsText = joinChunks([...stringDeps, ...funcDeps].map(addDependency));

      // Defer direct JS value aliases (`var _alias = _target;`) to `libraryAliases`
      // so the target symbol is guaranteed to be defined first. Skip native
      // aliases (which emit no code here) and signature-mismatch aliases (where
      // `isFunction` is set because they are emitted as forwarding wrapper
      // functions rather than immediate value assignments).
      if (aliasTarget && !isNativeAlias && !isFunction) {
        libraryAliases.push(joinChunks([commentText, contentText]));
        return depsText;
      }
      return joinChunks([depsText, commentText, contentText]);
    }

    const JS = addFromLibrary(symbol, 'root reference (e.g. compiled C/C++ code)');
    libraryItems.push(JS);
  }

  function includeSystemFile(fileName) {
    writeOutput(getSystemIncludeFile(fileName));
  }

  function includeFile(fileName) {
    writeOutput(getIncludeFile(fileName));
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
          assert(limit > 0, 'could not sort postsets');
          break;
        }
      }
    }

    postSets.push(...orderedPostSets);

    const shellFile = MINIMAL_RUNTIME ? 'shell_minimal.js' : 'shell.js';
    includeSystemFile(shellFile);

    const preFile = MINIMAL_RUNTIME ? 'preamble_minimal.js' : 'preamble.js';
    includeSystemFile(preFile);

    writeOutput('// Begin JS library code\n');
    for (const item of [...libraryItems, ...libraryAliases, ...postSets]) {
      writeOutput(indentify(item ?? '', 2));
    }
    writeOutput('// End JS library code\n');

    if (!MINIMAL_RUNTIME) {
      includeSystemFile('postlibrary.js');
    }

    if (PTHREADS) {
      writeOutput(`
// proxiedFunctionTable specifies the list of functions that can be called
// either synchronously or asynchronously from other threads in postMessage()d
// or internally queued events. This way a pthread in a Worker can synchronously
// access e.g. the DOM on the main thread.
var proxiedFunctionTable = [
  ${proxiedFunctionTable.join(',\n  ')}
];
`);
    }

    // This is the main 'post' pass. Print out the generated code
    // that we have here, together with the rest of the output
    // that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    writeOutput('// EMSCRIPTEN_END_FUNCS\n');

    const postFile = MINIMAL_RUNTIME ? 'postamble_minimal.js' : 'postamble.js';
    includeSystemFile(postFile);

    for (const fileName of POST_JS_FILES) {
      includeFile(fileName);
    }

    if (MODULARIZE && MODULARIZE != 'instance') {
      includeSystemFile('postamble_modularize.js');
    }

    if (errorOccured()) {
      throw Error('Aborting compilation due to previous errors');
    }

    writeOutput(
      '//FORWARDED_DATA:' +
        JSON.stringify({
          librarySymbols,
          extraExports: Array.from(extraExports),
          nativeAliases,
          warnings: warningOccured(),
          asyncFuncs,
          libraryDefinitions: LibraryManager.libraryDefinitions,
          ATPRERUNS: ATPRERUNS.join('\n'),
          ATMODULES: ATMODULES.join('\n'),
          ATINITS: ATINITS.join('\n'),
          ATPOSTCTORS: ATPOSTCTORS.join('\n'),
          ATMAINS: ATMAINS.join('\n'),
          ATPOSTRUNS: ATPOSTRUNS.join('\n'),
          ATEXITS: ATEXITS.join('\n'),
        }),
    );
  }

  timer.start('symbolHandler');
  for (const sym of symbolsNeeded) {
    symbolHandler(sym);
  }
  timer.stop('symbolHandler');

  if (symbolsOnly) {
    const symbolDeps = {};
    for (const sym of Object.keys(LibraryManager.library)) {
      if (!isDecorator(sym)) {
        const transitiveDeps = getTransitiveDeps(sym);
        symbolDeps[sym] = transitiveDeps.filter(
          (d) => !isJsOnlySymbol(d) && !(d in LibraryManager.library),
        );
      }
    }
    writeOutput(
      JSON.stringify({
        deps: symbolDeps,
        asyncFuncs,
        extraLibraryFuncs,
      }),
    );
  } else {
    timer.start('finalCombiner')
    finalCombiner();
    timer.stop('finalCombiner')
  }

  if (errorOccured()) {
    throw Error('Aborting compilation due to previous errors');
  }

  if (outputFile) await outputHandle.close();
}

addToCompileTimeContext({
  extraLibraryFuncs,
  addedLibraryItems,
  preJS,
});
