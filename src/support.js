// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// {{PREAMBLE_ADDITIONS}}

var STACK_ALIGN = {{{ STACK_ALIGN }}};

#if ASSERTIONS
// stack management, and other functionality that is provided by the compiled code,
// should not be used before it is ready
stackSave = stackRestore = stackAlloc = function() {
  abort('cannot use the stack before compiled code is ready to run, and has provided stack access');
};

function staticAlloc(size) {
  abort('staticAlloc is no longer available at runtime; instead, perform static allocations at compile time (using makeStaticAlloc)');
}
#endif

function dynamicAlloc(size) {
#if ASSERTIONS
  assert(DYNAMICTOP_PTR);
#if USE_PTHREADS
  assert(!ENVIRONMENT_IS_PTHREAD); // this function is not thread-safe
#endif
#endif
  var ret = HEAP32[DYNAMICTOP_PTR>>2];
  var end = (ret + size + 15) & -16;
  if (end <= _emscripten_get_heap_size()) {
    HEAP32[DYNAMICTOP_PTR>>2] = end;
  } else {
#if ALLOW_MEMORY_GROWTH
    var success = _emscripten_resize_heap(end);
    if (!success) return 0;
#else
    return 0;
#endif
  }
  return ret;
}

{{{ alignMemory }}}

{{{ getNativeTypeSize }}}

function warnOnce(text) {
  if (!warnOnce.shown) warnOnce.shown = {};
  if (!warnOnce.shown[text]) {
    warnOnce.shown[text] = 1;
    err(text);
  }
}

var asm2wasmImports = { // special asm2wasm imports
    "f64-rem": function(x, y) {
        return x % y;
    },
    "debugger": function() {
        debugger;
    }
#if NEED_ALL_ASM2WASM_IMPORTS
    ,
    "f64-to-int": function(x) {
        return x | 0;
    },
    "i32s-div": function(x, y) {
        return ((x | 0) / (y | 0)) | 0;
    },
    "i32u-div": function(x, y) {
        return ((x >>> 0) / (y >>> 0)) >>> 0;
    },
    "i32s-rem": function(x, y) {
        return ((x | 0) % (y | 0)) | 0;
    },
    "i32u-rem": function(x, y) {
        return ((x >>> 0) % (y >>> 0)) >>> 0;
    }
#endif // NEED_ALL_ASM2WASM_IMPORTS
};

#if RELOCATABLE
// dynamic linker/loader (a-la ld.so on ELF systems)
var LDSO = {
  // next free handle to use for a loaded dso.
  // (handle=0 is avoided as it means "error" in dlopen)
  nextHandle: 1,

  loadedLibs: {         // handle -> dso [refcount, name, module, global]
    // program itself
    // XXX uglifyjs fails on "[-1]: {"
    '-1': {
      refcount: Infinity,   // = nodelete
      name:     '__self__',
      module:   Module,
      global:   true
    }
  },

  loadedLibNames: {     // name   -> handle
    // program itself
    '__self__': -1
  },
}

// fetchBinary fetches binaray data @ url. (async)
function fetchBinary(url) {
  return fetch(url, { credentials: 'same-origin' }).then(function(response) {
    if (!response['ok']) {
      throw "failed to load binary file at '" + url + "'";
    }
    return response['arrayBuffer']();
  }).then(function(buffer) {
    return new Uint8Array(buffer);
  });
}

// loadDynamicLibrary loads dynamic library @ lib URL / path and returns handle for loaded DSO.
//
// Several flags affect the loading:
//
// - if flags.global=true, symbols from the loaded library are merged into global
//   process namespace. Flags.global is thus similar to RTLD_GLOBAL in ELF.
//
// - if flags.nodelete=true, the library will be never unloaded. Flags.nodelete
//   is thus similar to RTLD_NODELETE in ELF.
//
// - if flags.loadAsync=true, the loading is performed asynchronously and
//   loadDynamicLibrary returns corresponding promise.
//
// - if flags.fs is provided, it is used as FS-like interface to load library data.
//   By default, when flags.fs=undefined, native loading capabilities of the
//   environment are used.
//
// If a library was already loaded, it is not loaded a second time. However
// flags.global and flags.nodelete are handled every time a load request is made.
// Once a library becomes "global" or "nodelete", it cannot be removed or unloaded.
function loadDynamicLibrary(lib, flags) {
  // when loadDynamicLibrary did not have flags, libraries were loaded globally & permanently
  flags = flags || {global: true, nodelete: true}

  var handle = LDSO.loadedLibNames[lib];
  var dso;
  if (handle) {
    // the library is being loaded or has been loaded already.
    //
    // however it could be previously loaded only locally and if we get
    // load request with global=true we have to make it globally visible now.
    dso = LDSO.loadedLibs[handle];
    if (flags.global && !dso.global) {
      dso.global = true;
      if (dso.module !== 'loading') {
        // ^^^ if module is 'loading' - symbols merging will be eventually done by the loader.
        mergeLibSymbols(dso.module)
      }
    }
    // same for "nodelete"
    if (flags.nodelete && dso.refcount !== Infinity) {
      dso.refcount = Infinity;
    }
    dso.refcount++
    return flags.loadAsync ? Promise.resolve(handle) : handle;
  }

  // allocate new DSO & handle
  handle = LDSO.nextHandle++;
  dso = {
    refcount: flags.nodelete ? Infinity : 1,
    name:     lib,
    module:   'loading',
    global:   flags.global,
  };
  LDSO.loadedLibNames[lib] = handle;
  LDSO.loadedLibs[handle] = dso;

  // libData <- lib
  function loadLibData() {
#if WASM
    // for wasm, we can use fetch for async, but for fs mode we can only imitate it
    if (flags.fs) {
      var libData = flags.fs.readFile(lib, {encoding: 'binary'});
      if (!(libData instanceof Uint8Array)) {
        libData = new Uint8Array(lib_data);
      }
      return flags.loadAsync ? Promise.resolve(libData) : libData;
    }

    if (flags.loadAsync) {
      return fetchBinary(lib);
    }
    // load the binary synchronously
    return Module['readBinary'](lib);
#else
    // for js we only imitate async for both native & fs modes.
    var libData;
    if (flags.fs) {
      libData = flags.fs.readFile(lib, {encoding: 'utf8'});
    } else {
      libData = Module['read'](lib);
    }
    return flags.loadAsync ? Promise.resolve(libData) : libData;
#endif
  }

  // libModule <- libData
  function createLibModule(libData) {
#if WASM
    return loadWebAssemblyModule(libData, flags)
#else
    var libModule = eval(libData)(
      alignFunctionTables(),
      Module
    );
    // load dynamic libraries that this js lib depends on
    // (wasm loads needed libraries _before_ lib in its own codepath)
    if (libModule.dynamicLibraries) {
      if (flags.loadAsync) {
        return Promise.all(libModule.dynamicLibraries.map(function(dynNeeded) {
          return loadDynamicLibrary(dynNeeded, flags);
        })).then(function() {
          return libModule;
        });
      }

      libModule.dynamicLibraries.forEach(function(dynNeeded) {
        loadDynamicLibrary(dynNeeded, flags);
      });
    }
    return libModule;
#endif
  }

  // libModule <- lib
  function getLibModule() {
    // lookup preloaded cache first
    if (Module['preloadedWasm'] !== undefined &&
        Module['preloadedWasm'][lib] !== undefined) {
      var libModule = Module['preloadedWasm'][lib];
      return flags.loadAsync ? Promise.resolve(libModule) : libModule;
    }

    // module not preloaded - load lib data and create new module from it
    if (flags.loadAsync) {
      return loadLibData(lib).then(function(libData) {
        return createLibModule(libData);
      });
    }

    return createLibModule(loadLibData(lib));
  }

  // Module.symbols <- libModule.symbols (flags.global handler)
  function mergeLibSymbols(libModule) {
    // add symbols into global namespace TODO: weak linking etc.
    for (var sym in libModule) {
      if (!libModule.hasOwnProperty(sym)) {
        continue;
      }

      // When RTLD_GLOBAL is enable, the symbols defined by this shared object will be made
      // available for symbol resolution of subsequently loaded shared objects.
      //
      // We should copy the symbols (which include methods and variables) from SIDE_MODULE to MAIN_MODULE.
      //
      // Module of SIDE_MODULE has not only the symbols (which should be copied)
      // but also others (print*, asmGlobal*, FUNCTION_TABLE_**, NAMED_GLOBALS, and so on).
      //
      // When the symbol (which should be copied) is method, Module._* 's type becomes function.
      // When the symbol (which should be copied) is variable, Module._* 's type becomes number.
      //
      // Except for the symbol prefix (_), there is no difference in the symbols (which should be copied) and others.
      // So this just copies over compiled symbols (which start with _).
      if (sym[0] !== '_') {
        continue;
      }

      if (!Module.hasOwnProperty(sym)) {
        Module[sym] = libModule[sym];
      }
#if ASSERTIONS == 2
      else {
        var curr = Module[sym], next = libModule[sym];
        // don't warn on functions - might be odr, linkonce_odr, etc.
        if (!(typeof curr === 'function' && typeof next === 'function')) {
          err("warning: trying to dynamically load symbol '" + sym + "' (from '" + lib + "') that already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
        }
      }
#endif
    }
  }

  // module for lib is loaded - update the dso & global namespace
  function moduleLoaded(libModule) {
    if (dso.global) {
      mergeLibSymbols(libModule);
    }
    dso.module = libModule;
  }

  if (flags.loadAsync) {
    return getLibModule().then(function(libModule) {
      moduleLoaded(libModule);
      return handle;
    })
  }

  moduleLoaded(getLibModule());
  return handle;
}

#if WASM
// Loads a side module from binary data
function loadWebAssemblyModule(binary, flags) {
  var int32View = new Uint32Array(new Uint8Array(binary.subarray(0, 24)).buffer);
  assert(int32View[0] == 0x6d736100, 'need to see wasm magic number'); // \0asm
  // we should see the dylink section right after the magic number and wasm version
  assert(binary[8] === 0, 'need the dylink section to be first')
  var next = 9;
  function getLEB() {
    var ret = 0;
    var mul = 1;
    while (1) {
      var byte = binary[next++];
      ret += ((byte & 0x7f) * mul);
      mul *= 0x80;
      if (!(byte & 0x80)) break;
    }
    return ret;
  }
  var sectionSize = getLEB();
  assert(binary[next] === 6);                 next++; // size of "dylink" string
  assert(binary[next] === 'd'.charCodeAt(0)); next++;
  assert(binary[next] === 'y'.charCodeAt(0)); next++;
  assert(binary[next] === 'l'.charCodeAt(0)); next++;
  assert(binary[next] === 'i'.charCodeAt(0)); next++;
  assert(binary[next] === 'n'.charCodeAt(0)); next++;
  assert(binary[next] === 'k'.charCodeAt(0)); next++;
  var memorySize = getLEB();
  var memoryAlign = getLEB();
  var tableSize = getLEB();
  var tableAlign = getLEB();

  // shared libraries this module needs. We need to load them first, so that
  // current module could resolve its imports. (see tools/shared.py
  // WebAssembly.make_shared_library() for "dylink" section extension format)
  var neededDynlibsCount = getLEB();
  var neededDynlibs = [];
  for (var i = 0; i < neededDynlibsCount; ++i) {
    var nameLen = getLEB();
    var nameUTF8 = binary.subarray(next, next + nameLen);
    next += nameLen;
    var name = UTF8ArrayToString(nameUTF8, 0);
    neededDynlibs.push(name);
  }

  // loadModule loads the wasm module after all its dependencies have been loaded.
  // can be called both sync/async.
  function loadModule() {
    // alignments are powers of 2
    memoryAlign = Math.pow(2, memoryAlign);
    tableAlign = Math.pow(2, tableAlign);
    // finalize alignments and verify them
    memoryAlign = Math.max(memoryAlign, STACK_ALIGN); // we at least need stack alignment
#if ASSERTIONS
    assert(tableAlign === 1, 'invalid tableAlign ' + tableAlign);
#endif
    // prepare memory
    var memoryBase = alignMemory(getMemory(memorySize + memoryAlign), memoryAlign); // TODO: add to cleanups
    // The static area consists of explicitly initialized data, followed by zero-initialized data.
    // The latter may need zeroing out if the MAIN_MODULE has already used this memory area before
    // dlopen'ing the SIDE_MODULE.  Since we don't know the size of the explicitly initialized data
    // here, we just zero the whole thing, which is suboptimal, but should at least resolve bugs
    // from uninitialized memory.
    for (var i = memoryBase; i < memoryBase + memorySize; ++i) HEAP8[i] = 0;
    // prepare env imports
    var env = asmLibraryArg;
    // TODO: use only __memory_base and __table_base, need to update asm.js backend
    var table = wasmTable;
    var tableBase = table.length;
    var originalTable = table;
    table.grow(tableSize);
    assert(table === originalTable);
    // zero-initialize memory and table
    // TODO: in some cases we can tell it is already zero initialized
    for (var i = memoryBase; i < memoryBase + memorySize; i++) {
      HEAP8[i] = 0;
    }
    for (var i = tableBase; i < tableBase + tableSize; i++) {
      table.set(i, null);
    }
    // copy currently exported symbols so the new module can import them
    for (var x in Module) {
      if (!(x in env)) {
        env[x] = Module[x];
      }
    }
    // TODO kill ↓↓↓ (except "symbols local to this module", it will likely be
    // not needed if we require that if A wants symbols from B it has to link
    // to B explicitly: similarly to -Wl,--no-undefined)
    //
    // wasm dynamic libraries are pure wasm, so they cannot assist in
    // their own loading. When side module A wants to import something
    // provided by a side module B that is loaded later, we need to
    // add a layer of indirection, but worse, we can't even tell what
    // to add the indirection for, without inspecting what A's imports
    // are. To do that here, we use a JS proxy (another option would
    // be to inspect the binary directly).
    var proxyHandler = {
      'get': function(obj, prop) {
        // symbols that should be local to this module
        switch (prop) {
          case '__memory_base':
          case 'gb':
            return memoryBase;
          case '__table_base':
          case 'fb':
            return tableBase;
        }

        if (prop in obj) {
          return obj[prop]; // already present
        }
        if (prop.startsWith('g$')) {
          // a global. the g$ function returns the global address.
          var name = prop.substr(2); // without g$ prefix
          return env[prop] = function() {
#if ASSERTIONS
            assert(Module[name], 'missing linked global ' + name + '. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment');
#endif
            return Module[name];
          };
        }
        if (prop.startsWith('invoke_')) {
          // A missing invoke, i.e., an invoke for a function type
          // present in the dynamic library but not in the main JS,
          // and the dynamic library cannot provide JS for it. Use
          // the generic "X" invoke for it.
          return env[prop] = invoke_X;
        }
        // if not a global, then a function - call it indirectly
        return env[prop] = function() {
#if ASSERTIONS
          assert(Module[prop], 'missing linked function ' + prop + '. perhaps a side module was not linked in? if this function was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment');
#endif
          return Module[prop].apply(null, arguments);
        };
      }
    };
    var info = {
      global: {
        'NaN': NaN,
        'Infinity': Infinity,
      },
      'global.Math': Math,
      env: new Proxy(env, proxyHandler),
      'asm2wasm': asm2wasmImports
    };
#if ASSERTIONS
    var oldTable = [];
    for (var i = 0; i < tableBase; i++) {
      oldTable.push(table.get(i));
    }
#endif

    function postInstantiation(instance) {
      var exports = {};
#if ASSERTIONS
      // the table should be unchanged
      assert(table === originalTable);
      assert(table === wasmTable);
      if (instance.exports['table']) {
        assert(table === instance.exports['table']);
      }
      // the old part of the table should be unchanged
      for (var i = 0; i < tableBase; i++) {
        assert(table.get(i) === oldTable[i], 'old table entries must remain the same');
      }
      // verify that the new table region was filled in
      for (var i = 0; i < tableSize; i++) {
        assert(table.get(tableBase + i) !== undefined, 'table entry was not filled in');
      }
#endif
      for (var e in instance.exports) {
        var value = instance.exports[e];
        if (typeof value === 'object') {
          // a breaking change in the wasm spec, globals are now objects
          // https://github.com/WebAssembly/mutable-global/issues/1
          value = value.value;
        }
        if (typeof value === 'number') {
          // relocate it - modules export the absolute value, they can't relocate before they export
#if EMULATE_FUNCTION_POINTER_CASTS
          // it may be a function pointer
          if (e.substr(0, 3) == 'fp$' && typeof instance.exports[e.substr(3)] === 'function') {
            value = value + tableBase;
          } else {
#endif
            value = value + memoryBase;
#if EMULATE_FUNCTION_POINTER_CASTS
          }
#endif
        }
        exports[e] = value;
      }
      // initialize the module
      var init = exports['__post_instantiate'];
      if (init) {
        if (runtimeInitialized) {
          init();
        } else {
          // we aren't ready to run compiled code yet
          __ATINIT__.push(init);
        }
      }
      return exports;
    }

    if (flags.loadAsync) {
      return WebAssembly.instantiate(binary, info).then(function(result) {
        return postInstantiation(result.instance);
      });
    } else {
      var instance = new WebAssembly.Instance(new WebAssembly.Module(binary), info);
      return postInstantiation(instance);
    }
  }

  // now load needed libraries and the module itself.
  if (flags.loadAsync) {
    return Promise.all(neededDynlibs.map(function(dynNeeded) {
      return loadDynamicLibrary(dynNeeded, flags);
    })).then(function() {
      return loadModule();
    });
  }

  neededDynlibs.forEach(function(dynNeeded) {
    loadDynamicLibrary(dynNeeded, flags);
  });
  return loadModule();
}
Module['loadWebAssemblyModule'] = loadWebAssemblyModule;

#endif // WASM
#endif // RELOCATABLE

#if EMULATED_FUNCTION_POINTERS
#if WASM == 0
function getFunctionTables(module) {
  if (!module) module = Module;
  var tables = {};
  for (var t in module) {
    if (/^FUNCTION_TABLE_.*/.test(t)) {
      var table = module[t];
      if (typeof table === 'object') tables[t.substr('FUNCTION_TABLE_'.length)] = table;
    }
  }
  return tables;
}

function alignFunctionTables(module) {
  var tables = getFunctionTables(module);
  var maxx = 0;
  for (var sig in tables) {
    maxx = Math.max(maxx, tables[sig].length);
  }
  assert(maxx >= 0);
  for (var sig in tables) {
    var table = tables[sig];
    while (table.length < maxx) table.push(0);
  }
  return maxx;
}
#endif // WASM == 0

#if RELOCATABLE
// register functions from a new module being loaded
function registerFunctions(sigs, newModule) {
  sigs.forEach(function(sig) {
    if (!Module['FUNCTION_TABLE_' + sig]) {
      Module['FUNCTION_TABLE_' + sig] = [];
    }
  });
  var oldMaxx = alignFunctionTables(); // align the new tables we may have just added
  var newMaxx = alignFunctionTables(newModule);
  var maxx = oldMaxx + newMaxx;
  sigs.forEach(function(sig) {
    var newTable = newModule['FUNCTION_TABLE_' + sig];
    var oldTable = Module['FUNCTION_TABLE_' + sig];
    assert(newTable !== oldTable);
    assert(oldTable.length === oldMaxx);
    for (var i = 0; i < newTable.length; i++) {
      oldTable.push(newTable[i]);
    }
    assert(oldTable.length === maxx);
  });
  assert(maxx === alignFunctionTables()); // align the ones we didn't touch
}
// export this so side modules can use it
Module['registerFunctions'] = registerFunctions;
#endif // RELOCATABLE
#endif // EMULATED_FUNCTION_POINTERS

#if !WASM_BACKEND && EMULATED_FUNCTION_POINTERS == 0
var jsCallStartIndex = 1;
var functionPointers = new Array({{{ RESERVED_FUNCTION_POINTERS }}});
#endif // !WASM_BACKEND && EMULATED_FUNCTION_POINTERS == 0

#if WASM
// Wraps a JS function as a wasm function with a given signature.
// In the future, we may get a WebAssembly.Function constructor. Until then,
// we create a wasm module that takes the JS function as an import with a given
// signature, and re-exports that as a wasm function.
function convertJsFunctionToWasm(func, sig) {
  // The module is static, with the exception of the type section, which is
  // generated based on the signature passed in.
  var typeSection = [
    0x01, // id: section,
    0x00, // length: 0 (placeholder)
    0x01, // count: 1
    0x60, // form: func
  ];
  var sigRet = sig.slice(0, 1);
  var sigParam = sig.slice(1);
  var typeCodes = {
    'i': 0x7f, // i32
    'j': 0x7e, // i64
    'f': 0x7d, // f32
    'd': 0x7c, // f64
  };

  // Parameters, length + signatures
  typeSection.push(sigParam.length);
  for (var i = 0; i < sigParam.length; ++i) {
    typeSection.push(typeCodes[sigParam[i]]);
  }

  // Return values, length + signatures
  // With no multi-return in MVP, either 0 (void) or 1 (anything else)
  if (sigRet == 'v') {
    typeSection.push(0x00);
  } else {
    typeSection = typeSection.concat([0x01, typeCodes[sigRet]]);
  }

  // Write the overall length of the type section back into the section header
  // (excepting the 2 bytes for the section id and length)
  typeSection[1] = typeSection.length - 2;

  // Rest of the module is static
  var bytes = new Uint8Array([
    0x00, 0x61, 0x73, 0x6d, // magic ("\0asm")
    0x01, 0x00, 0x00, 0x00, // version: 1
  ].concat(typeSection, [
    0x02, 0x07, // import section
      // (import "e" "f" (func 0 (type 0)))
      0x01, 0x01, 0x65, 0x01, 0x66, 0x00, 0x00,
    0x07, 0x05, // export section
      // (export "f" (func 0 (type 0)))
      0x01, 0x01, 0x66, 0x00, 0x00,
  ]));

   // We can compile this wasm module synchronously because it is very small.
  // This accepts an import (at "e.f"), that it reroutes to an export (at "f")
  var module = new WebAssembly.Module(bytes);
  var instance = new WebAssembly.Instance(module, {
    e: {
      f: func
    }
  });
  var wrappedFunc = instance.exports.f;
  return wrappedFunc;
}

// Add a wasm function to the table.
function addFunctionWasm(func, sig) {
  var table = wasmTable;
  var ret = table.length;

  // Grow the table
  try {
    table.grow(1);
  } catch (err) {
    if (!err instanceof RangeError) {
      throw err;
    }
    throw 'Unable to grow wasm table. Use a higher value for RESERVED_FUNCTION_POINTERS or set ALLOW_TABLE_GROWTH.';
  }

  // Insert new element
  try {
    // Attempting to call this with JS function will cause of table.set() to fail
    table.set(ret, func);
  } catch (err) {
    if (!err instanceof TypeError) {
      throw err;
    }
    assert(typeof sig !== 'undefined', 'Missing signature argument to addFunction');
    var wrapped = convertJsFunctionToWasm(func, sig);
    table.set(ret, wrapped);
  }

  return ret;
}

function removeFunctionWasm(index) {
  // TODO(sbc): Look into implementing this to allow re-using of table slots
}
#endif

// 'sig' parameter is required for the llvm backend but only when func is not
// already a WebAssembly function.
function addFunction(func, sig) {
#if ASSERTIONS == 2
  if (typeof sig === 'undefined') {
    err('warning: addFunction(): You should provide a wasm function signature string as a second argument. This is not necessary for asm.js and asm2wasm, but can be required for the LLVM wasm backend, so it is recommended for full portability.');
  }
#endif // ASSERTIONS

#if WASM_BACKEND
  return addFunctionWasm(func, sig);
#else

#if EMULATED_FUNCTION_POINTERS == 0
  var base = 0;
  for (var i = base; i < base + {{{ RESERVED_FUNCTION_POINTERS }}}; i++) {
    if (!functionPointers[i]) {
      functionPointers[i] = func;
      return jsCallStartIndex + i;
    }
  }
  throw 'Finished up all reserved function pointers. Use a higher value for RESERVED_FUNCTION_POINTERS.';

#else // EMULATED_FUNCTION_POINTERS == 0

#if WASM
  return addFunctionWasm(func, sig);
#else
  alignFunctionTables(); // TODO: we should rely on this being an invariant
  var tables = getFunctionTables();
  var ret = -1;
  for (var sig in tables) {
    var table = tables[sig];
    if (ret < 0) ret = table.length;
    else assert(ret === table.length);
    table.push(func);
  }
  return ret;
#endif // WASM

#endif // EMULATED_FUNCTION_POINTERS == 0
#endif // WASM_BACKEND
}

function removeFunction(index) {
#if WASM_BACKEND
  removeFunctionWasm(index);
#else

#if EMULATED_FUNCTION_POINTERS == 0
  functionPointers[index-jsCallStartIndex] = null;
#else
#if WASM
  removeFunctionWasm(index);
#else
  alignFunctionTables(); // XXX we should rely on this being an invariant
  var tables = getFunctionTables();
  for (var sig in tables) {
    tables[sig][index] = null;
  }
#endif // WASM

#endif // EMULATE_FUNCTION_POINTER_CASTS == 0
#endif // WASM_BACKEND
}

var funcWrappers = {};

function getFuncWrapper(func, sig) {
  if (!func) return; // on null pointer, return undefined
  assert(sig);
  if (!funcWrappers[sig]) {
    funcWrappers[sig] = {};
  }
  var sigCache = funcWrappers[sig];
  if (!sigCache[func]) {
    // optimize away arguments usage in common cases
    if (sig.length === 1) {
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func);
      };
    } else if (sig.length === 2) {
      sigCache[func] = function dynCall_wrapper(arg) {
        return dynCall(sig, func, [arg]);
      };
    } else {
      // general case
      sigCache[func] = function dynCall_wrapper() {
        return dynCall(sig, func, Array.prototype.slice.call(arguments));
      };
    }
  }
  return sigCache[func];
}

#if RUNTIME_DEBUG
var runtimeDebug = true; // Switch to false at runtime to disable logging at the right times

var printObjectList = [];

function prettyPrint(arg) {
  if (typeof arg == 'undefined') return '!UNDEFINED!';
  if (typeof arg == 'boolean') arg = arg + 0;
  if (!arg) return arg;
  var index = printObjectList.indexOf(arg);
  if (index >= 0) return '<' + arg + '|' + index + '>';
  if (arg.toString() == '[object HTMLImageElement]') {
    return arg + '\n\n';
  }
  if (arg.byteLength) {
    return '{' + Array.prototype.slice.call(arg, 0, Math.min(arg.length, 400)) + '}'; // Useful for correct arrays, less so for compiled arrays, see the code below for that
    var buf = new ArrayBuffer(32);
    var i8buf = new Int8Array(buf);
    var i16buf = new Int16Array(buf);
    var f32buf = new Float32Array(buf);
    switch(arg.toString()) {
      case '[object Uint8Array]':
        i8buf.set(arg.subarray(0, 32));
        break;
      case '[object Float32Array]':
        f32buf.set(arg.subarray(0, 5));
        break;
      case '[object Uint16Array]':
        i16buf.set(arg.subarray(0, 16));
        break;
      default:
        alert('unknown array for debugging: ' + arg);
        throw 'see alert';
    }
    var ret = '{' + arg.byteLength + ':\n';
    var arr = Array.prototype.slice.call(i8buf);
    ret += 'i8:' + arr.toString().replace(/,/g, ',') + '\n';
    arr = Array.prototype.slice.call(f32buf, 0, 8);
    ret += 'f32:' + arr.toString().replace(/,/g, ',') + '}';
    return ret;
  }
  if (typeof arg == 'object') {
    printObjectList.push(arg);
    return '<' + arg + '|' + (printObjectList.length-1) + '>';
  }
  if (typeof arg == 'number') {
    if (arg > 0) return '0x' + arg.toString(16) + ' (' + arg + ')';
  }
  return arg;
}
#endif

function makeBigInt(low, high, unsigned) {
  return unsigned ? ((+((low>>>0)))+((+((high>>>0)))*4294967296.0)) : ((+((low>>>0)))+((+((high|0)))*4294967296.0));
}

function dynCall(sig, ptr, args) {
  if (args && args.length) {
#if ASSERTIONS
    assert(args.length == sig.length-1);
#endif
#if ASSERTIONS
    assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
#endif
    return Module['dynCall_' + sig].apply(null, [ptr].concat(args));
  } else {
#if ASSERTIONS
    assert(sig.length == 1);
#endif
#if ASSERTIONS
    assert(('dynCall_' + sig) in Module, 'bad function pointer type - no table for sig \'' + sig + '\'');
#endif
    return Module['dynCall_' + sig].call(null, ptr);
  }
}

var tempRet0 = 0;

var setTempRet0 = function(value) {
  tempRet0 = value;
}

var getTempRet0 = function() {
  return tempRet0;
}

#if RETAIN_COMPILER_SETTINGS
var compilerSettings = {{{ JSON.stringify(makeRetainedCompilerSettings()) }}} ;

function getCompilerSetting(name) {
  if (!(name in compilerSettings)) return 'invalid compiler setting: ' + name;
  return compilerSettings[name];
}
#else // RETAIN_COMPILER_SETTINGS
#if ASSERTIONS
function getCompilerSetting(name) {
  throw 'You must build with -s RETAIN_COMPILER_SETTINGS=1 for getCompilerSetting or emscripten_get_compiler_setting to work';
}
#endif // ASSERTIONS
#endif // RETAIN_COMPILER_SETTINGS

var Runtime = {
#if ASSERTIONS
  // helpful errors
  getTempRet0: function() { abort('getTempRet0() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
  staticAlloc: function() { abort('staticAlloc() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
  stackAlloc: function() { abort('stackAlloc() is now a top-level function, after removing the Runtime object. Remove "Runtime."') },
#endif
};

// The address globals begin at. Very low in memory, for code size and optimization opportunities.
// Above 0 is static memory, starting with globals.
// Then the stack.
// Then 'dynamic' memory for sbrk.
var GLOBAL_BASE = {{{ GLOBAL_BASE }}};

#if RELOCATABLE
GLOBAL_BASE = alignMemory(GLOBAL_BASE, {{{ MAX_GLOBAL_ALIGN || 1 }}});
#endif

#if WASM_BACKEND && USE_PTHREADS
// The wasm backend path does not have a way to set the stack max, so we can
// just implement this function in a trivial way
function establishStackSpace(base, max) {
  stackRestore(base);
}

// JS library code refers to Atomics in the manner used from asm.js, provide
// the same API here.
var Atomics_load = Atomics.load;
var Atomics_store = Atomics.store;
var Atomics_compareExchange = Atomics.compareExchange;
#endif

