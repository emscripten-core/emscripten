/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 *
 * Dynamic library loading
 */

#if !MAIN_MODULE && !RELOCATABLE
#error "library_dylink.js requires MAIN_MODULE or RELOCATABLE"
#endif

{{{
const UNDEFINED_ADDR = to64(-1);
}}}

var LibraryDylink = {
#if FILESYSTEM
  $registerWasmPlugin__deps: ['$preloadPlugins'],
  $registerWasmPlugin: () => {
    // Use string keys here for public methods to avoid minification since the
    // plugin consumer also uses string keys.
    var wasmPlugin = {
      promiseChainEnd: Promise.resolve(),
      'canHandle': (name) => {
        return !Module['noWasmDecoding'] && name.endsWith('.so')
      },
      'handle': async (byteArray, name) =>
        // loadWebAssemblyModule can not load modules out-of-order, so rather
        // than just running the promises in parallel, this makes a chain of
        // promises to run in series.
        wasmPlugin.promiseChainEnd = wasmPlugin.promiseChainEnd.then(async () => {
          try {
            var exports = await loadWebAssemblyModule(byteArray, {loadAsync: true, nodelete: true}, name, {});
          } catch (error) {
            throw new Error(`failed to instantiate wasm: ${name}: ${error}`);
          }
#if DYLINK_DEBUG
          dbg('registering preloadedWasm:', name);
#endif
          preloadedWasm[name] = exports;
          return byteArray;
        })
    };
    preloadPlugins.push(wasmPlugin);
  },

  $preloadedWasm__deps: ['$registerWasmPlugin'],
  $preloadedWasm__postset: `
    registerWasmPlugin();
    `,
  $preloadedWasm: {},

  $replaceORIGIN__deps: ['$PATH'],
  $replaceORIGIN: (parentLibName, rpath) => {
    if (rpath.startsWith('$ORIGIN')) {
      // TODO: what to do if we only know the relative path of the file? It will return "." here.
      var origin = PATH.dirname(parentLibName);
      return rpath.replace('$ORIGIN', origin);
    }

    return rpath;
  },
#endif // FILESYSTEM

  $isSymbolDefined: (symName) => {
    // Ignore 'stub' symbols that are auto-generated as part of the original
    // `wasmImports` used to instantiate the main module.
    var existing = wasmImports[symName];
    if (!existing || existing.stub) {
      return false;
    }
#if ASYNCIFY
    // Even if a symbol exists in wasmImports, and is not itself a stub, it
    // could be an ASYNCIFY wrapper function that wraps a stub function.
    if (symName in asyncifyStubs && !asyncifyStubs[symName]) {
      return false;
    }
#endif
    return true;
  },

  // Dynamic version of shared.py:make_invoke.  This is needed for invokes
  // that originate from side modules since these are not known at JS
  // generation time.
#if !DISABLE_EXCEPTION_CATCHING || SUPPORT_LONGJMP == 'emscripten'
  $createInvokeFunction__internal: true,
  $createInvokeFunction__deps: ['$dynCall', 'setThrew', '$stackSave', '$stackRestore'],
  $createInvokeFunction: (sig) => (ptr, ...args) => {
    var sp = stackSave();
    try {
      return dynCall(sig, ptr, args);
    } catch(e) {
      stackRestore(sp);
      // Create a try-catch guard that rethrows the Emscripten EH exception.
#if EXCEPTION_STACK_TRACES
      // Exceptions thrown from C++ and longjmps will be an instance of
      // EmscriptenEH.
      if (!(e instanceof EmscriptenEH)) throw e;
#else
      // Exceptions thrown from C++ will be a pointer (number) and longjmp
      // will throw the number Infinity. Use the compact and fast "e !== e+0"
      // test to check if e was not a Number.
      if (e !== e+0) throw e;
#endif
      _setThrew(1, 0);
#if WASM_BIGINT
      // In theory this if statement could be done on
      // creating the function, but I just added this to
      // save wasting code space as it only happens on exception.
      if (sig[0] == "j") return 0n;
#endif
    }
  },
#endif

  // Resolve a global symbol by name.  This is used during module loading to
  // resolve imports, and by `dlsym` when used with `RTLD_DEFAULT`.
  // Returns both the resolved symbol (i.e. a function or a global) along with
  // the canonical name of the symbol (in some cases is modify the symbol as
  // part of the loop process, so that actual symbol looked up has a different
  // name).
  $resolveGlobalSymbol__deps: ['$isSymbolDefined', '$createNamedFunction',
#if !DISABLE_EXCEPTION_CATCHING || SUPPORT_LONGJMP == 'emscripten'
    '$createInvokeFunction',
#endif
  ],
  $resolveGlobalSymbol__internal: true,
  $resolveGlobalSymbol: (symName, direct = false) => {
    var sym;
#if !WASM_BIGINT
    // First look for the orig$ symbol which is the symbol without i64
    // legalization performed.
    if (direct && ('orig$' + symName in wasmImports)) {
      symName = 'orig$' + symName;
    }
#endif
    if (isSymbolDefined(symName)) {
      sym = wasmImports[symName];
    }
#if !DISABLE_EXCEPTION_CATCHING || SUPPORT_LONGJMP == 'emscripten'
    // Asm.js-style exception handling: invoke wrapper generation
    else if (symName.startsWith('invoke_')) {
      // Create (and cache) new invoke_ functions on demand.
      sym = wasmImports[symName] = createNamedFunction(symName, createInvokeFunction(symName.split('_')[1]));
    }
#endif
#if !DISABLE_EXCEPTION_CATCHING
    else if (symName.startsWith('__cxa_find_matching_catch_')) {
      // When the main module is linked we create whichever variants of
      // `__cxa_find_matching_catch_` (see jsifier.js) that we know are needed,
      // but a side module loaded at runtime might need different/additional
      // variants so we create those dynamically.
      sym = wasmImports[symName] = createNamedFunction(symName, (...args) => {
#if MEMORY64
        args = args.map(Number);
#endif
        var rtn = findMatchingCatch(args);
        return {{{ to64('rtn') }}};
      });
    }
#endif
    return {sym, name: symName};
  },

  $GOT: {},

  // Proxy handler used for GOT.mem and GOT.func imports.  Each of these
  // imports is fullfilled dynamically via the `get` method of this proxy
  // handler.  We abuse the `target` of the Proxy in order to pass the set of
  // weak imports to the handler.
  $GOTHandler__internal: true,
  $GOTHandler__deps: ['$GOT'],
  $GOTHandler: {
    get(weakImports, symName) {
      var rtn = GOT[symName];
      if (!rtn) {
#if DYLINK_DEBUG == 2
        dbg(`new GOT entry: ${symName}`);
#endif
        rtn = GOT[symName] = new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': true}, {{{ UNDEFINED_ADDR }}});
      }
      if (!weakImports.has(symName)) {
        // Any non-weak reference to a symbol marks it as `required`, which
        // enabled `reportUndefinedSymbols` to report undefined symbol errors
        // correctly.
        rtn.required = true;
      }
      return rtn;
    }
  },

  $isInternalSym__internal: true,
  $isInternalSym: (symName) => {
    // TODO: find a way to mark these in the binary or avoid exporting them.
    return [
      'memory',
      '__memory_base',
      '__table_base',
      '__stack_pointer',
      '__indirect_function_table',
      '__cpp_exception',
      '__c_longjmp',
      '__wasm_apply_data_relocs',
      '__dso_handle',
      '__tls_size',
      '__tls_align',
      '__set_stack_limits',
      '_emscripten_tls_init',
      '__wasm_init_tls',
      '__wasm_call_ctors',
      '__start_em_asm',
      '__stop_em_asm',
      '__start_em_js',
      '__stop_em_js',
    ].includes(symName) || symName.startsWith('__em_js__')
#if SPLIT_MODULE
        // Exports synthesized by wasm-split should be prefixed with '%'
        || symName[0] == '%'
#endif
    ;
  },

  $updateGOT__internal: true,
  $updateGOT__deps: ['$GOT', '$isInternalSym', '$addFunction'],
  $updateGOT__docs: '/** @param {boolean=} replace */',
  $updateGOT: (exports, replace) => {
#if DYLINK_DEBUG
    dbg(`updateGOT: adding ${Object.keys(exports).length} symbols`);
#endif
    for (var symName in exports) {
      if (isInternalSym(symName)) {
        continue;
      }

      var value = exports[symName];
#if !WASM_BIGINT
      if (symName.startsWith('orig$')) {
        symName = symName.split('$')[1];
        replace = true;
      }
#endif

      var existingEntry = GOT[symName] && GOT[symName].value != {{{ UNDEFINED_ADDR }}};
      if (replace || !existingEntry) {
#if DYLINK_DEBUG == 2
        dbg(`updateGOT: before: ${symName} : ${GOT[symName]?.value}`);
#endif
        var newValue;
        if (typeof value == 'function') {
          newValue = {{{ to64('addFunction(value)') }}};
        } else if (typeof value.value == {{{ POINTER_JS_TYPE }}}) {
          newValue = value;
        } else {
          // The GOT can only contain addresses (i.e data addresses or function
          // addresses so we currently ignore other types export here.
#if DYLINK_DEBUG
          dbg(`updateGOT: ignoring ${symName} due to its type: ${typeof value}`);
#endif
          continue;
        }
#if DYLINK_DEBUG == 2
        dbg(`updateGOT:  after: ${symName} : ${newValue} (${value})`);
#endif
        GOT[symName] ??= new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}', 'mutable': true});
        GOT[symName].value = newValue;
      }
#if DYLINK_DEBUG
      else if (GOT[symName].value != value) {
        dbg(`updateGOT: EXISTING SYMBOL: ${symName} : ${GOT[symName].value} (${value})`);
      }
#endif
    }
#if DYLINK_DEBUG
    dbg("done updateGOT");
#endif
  },

  $isImmutableGlobal__internal: true,
  $isImmutableGlobal: (val) => {
    if (val instanceof WebAssembly.Global) {
      try {
        val.value = val.value;
      } catch {
        return true;
      }
    }
    return false;
  },

  // Applies relocations to exported things.
  $relocateExports__internal: true,
  $relocateExports__deps: ['$isImmutableGlobal'],
  $relocateExports: (exports, memoryBase = 0) => {
#if DYLINK_DEBUG
    dbg(`relocateExports memoryBase=${memoryBase} count=${Object.keys(exports).length}`);
#endif

    function relocateExport(name, value) {
#if SPLIT_MODULE
      // Do not modify exports synthesized by wasm-split
      if (name.startsWith('%')) {
        return value;
      }
#endif
      // Detect immuable wasm global exports. These represent data addresses
      // which are relative to `memoryBase`
      if (isImmutableGlobal(value)) {
        return new WebAssembly.Global({'value': '{{{ POINTER_WASM_TYPE }}}'}, value.value + {{{ to64('memoryBase') }}});
      }

      // Return unmodified value (no relocation required).
      return value;
    }

    var relocated = {};
    for (var e in exports) {
      relocated[e] = relocateExport(e, exports[e])
    }
    return relocated;
  },

  $reportUndefinedSymbols__internal: true,
  $reportUndefinedSymbols__deps: ['$GOT', '$resolveGlobalSymbol'],
  $reportUndefinedSymbols: () => {
#if DYLINK_DEBUG
    dbg('reportUndefinedSymbols');
#endif
    for (var [symName, entry] of Object.entries(GOT)) {
      if (entry.value == {{{ UNDEFINED_ADDR }}}) {
#if DYLINK_DEBUG
        dbg(`undef GOT entry: ${symName}`);
#endif
        var value = resolveGlobalSymbol(symName, true).sym;
        if (!value && !entry.required) {
          // Ignore undefined symbols that are imported as weak.
#if DYLINK_DEBUG
          dbg('ignoring undefined weak symbol:', symName);
#endif
          entry.value = {{{ to64(0) }}};
          continue;
        }
#if ASSERTIONS
        assert(value, `undefined symbol '${symName}'. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment`);
#endif
#if DYLINK_DEBUG == 2
        dbg(`assigning dynamic symbol from main module: ${symName} -> ${prettyPrint(value)}`);
#endif
        if (typeof value == 'function') {
          /** @suppress {checkTypes} */
          entry.value = {{{ to64('addFunction(value, value.sig)') }}};
#if DYLINK_DEBUG == 2
          dbg(`assigning table entry for : ${symName} -> ${entry.value}`);
#endif
        } else if (typeof value == 'number') {
          entry.value = {{{ to64('value') }}};
        } else if (typeof value.value == {{{ POINTER_JS_TYPE }}}) {
          entry.value = value;
        } else {
          throw new Error(`bad export type for '${symName}': ${typeof value} (${value})`);
        }
      }
    }
#if DYLINK_DEBUG
    dbg('done reportUndefinedSymbols');
#endif
  },

  // dynamic linker/loader (a-la ld.so on ELF systems)
  $LDSO__deps: ['$newDSO'],
  $LDSO: {
    // name -> dso [refcount, name, module, global]; Used by dlopen
    loadedLibsByName: {},
    // handle  -> dso; Used by dlsym
    loadedLibsByHandle: {},
    init() {
#if ASSERTIONS
      // This function needs to run after the initial wasmImports object
      // as been created.
      assert(wasmImports);
#endif
      newDSO('__main__', {{{ cDefs.RTLD_DEFAULT }}}, wasmImports);
    },
  },

  $dlSetError__internal: true,
  $dlSetError__deps: ['__dl_seterr', '$stringToUTF8OnStack', '$stackSave', '$stackRestore'],
  $dlSetError: (msg) => {
#if DYLINK_DEBUG
    dbg('dlSetError:', msg);
#endif
    var sp = stackSave();
    var cmsg = stringToUTF8OnStack(msg);
    ___dl_seterr(cmsg, 0);
    stackRestore(sp);
  },

  // We support some amount of allocation during startup in the case of
  // dynamic linking, which needs to allocate memory for dynamic libraries that
  // are loaded. That has to happen before the main program can start to run,
  // because the main program needs those linked in before it runs (so we can't
  // use normally malloc from the main program to do these allocations).
  //
  // Allocate memory even if malloc isn't ready yet.  The allocated memory here
  // must be zero initialized since its used for all static data, including bss.
  $getMemory__noleakcheck: true,
  $getMemory__deps: ['$GOT', 'emscripten_get_sbrk_ptr', '__heap_base', '$alignMemory', 'calloc'],
  $getMemory: (size) => {
    // After the runtime is initialized, we must only use sbrk() normally.
#if DYLINK_DEBUG
    dbg("getMemory: " + size + " runtimeInitialized=" + runtimeInitialized);
#endif
    if (runtimeInitialized) {
      // Currently we don't support freeing of static data when modules are
      // unloaded via dlclose.  This function is tagged as `noleakcheck` to
      // avoid having this reported as leak.
      return _calloc(size, 1);
    }
    var ret = ___heap_base;
    // Keep __heap_base stack aligned.
    var end = ret + alignMemory(size, {{{ STACK_ALIGN }}});
#if ASSERTIONS
    //dbg(ret);
    //dbg(HEAP8.length);
    assert(end <= HEAP8.length, 'failure to getMemory - memory growth etc. is not supported there, call malloc/sbrk directly or increase INITIAL_MEMORY');
#endif
    ___heap_base = end;

    // After allocating the memory from the start of the heap we need to ensure
    // that once the program starts it doesn't use this region.  In relocatable
    // mode we can just update the __heap_base symbol that we are exporting to
    // the main module.
    // When not relocatable `__heap_base` is fixed and exported by the main
    // module, but we can update the `sbrk_ptr` value instead.  We call
    // `_emscripten_get_sbrk_ptr` knowing that it is safe to call prior to
    // runtime initialization (unlike, the higher level sbrk function)
#if RELOCATABLE
    GOT['__heap_base'].value = {{{ to64('end') }}};
#else
#if PTHREADS
    if (!ENVIRONMENT_IS_PTHREAD) {
#endif
      var sbrk_ptr = _emscripten_get_sbrk_ptr();
      {{{ makeSetValue('sbrk_ptr', 0, 'end', '*') }}}
#if PTHREADS
    }
#endif
#endif
    return ret;
  },

  // returns the side module metadata as an object
  // { memorySize, memoryAlign, tableSize, tableAlign, neededDynlibs}
  $getDylinkMetadata__deps: ['$UTF8ArrayToString'],
  $getDylinkMetadata__internal: true,
  $getDylinkMetadata: (binary) => {
    var offset = 0;
    var end = 0;

    function getU8() {
      return binary[offset++];
    }

    function getLEB() {
      var ret = 0;
      var mul = 1;
      while (1) {
        var byte = binary[offset++];
        ret += ((byte & 0x7f) * mul);
        mul *= 0x80;
        if (!(byte & 0x80)) break;
      }
      return ret;
    }

    function getString() {
      var len = getLEB();
      offset += len;
      return UTF8ArrayToString(binary, offset - len, len);
    }

    function getStringList() {
      var count = getLEB();
      var rtn = []
      while (count--) rtn.push(getString());
      return rtn;
    }

    /** @param {string=} message */
    function failIf(condition, message) {
      if (condition) throw new Error(message);
    }

    if (binary instanceof WebAssembly.Module) {
      var dylinkSection = WebAssembly.Module.customSections(binary, 'dylink.0');
      failIf(dylinkSection.length === 0, 'need dylink section');
      binary = new Uint8Array(dylinkSection[0]);
      end = binary.length
    } else {
      var int32View = new Uint32Array(new Uint8Array(binary.subarray(0, 24)).buffer);
#if SUPPORT_BIG_ENDIAN
      var magicNumberFound = int32View[0] == 0x6d736100 || int32View[0] == 0x0061736d;
#else
      var magicNumberFound = int32View[0] == 0x6d736100;
#endif
      failIf(!magicNumberFound, 'need to see wasm magic number'); // \0asm
      // we should see the dylink custom section right after the magic number and wasm version
      failIf(binary[8] !== 0, 'need the dylink section to be first')
      offset = 9;
      var section_size = getLEB(); //section size
      end = offset + section_size;
      var name = getString();
      failIf(name !== 'dylink.0');
    }

    var customSection = { neededDynlibs: [], tlsExports: new Set(), weakImports: new Set(), runtimePaths: [] };
    var WASM_DYLINK_MEM_INFO = 0x1;
    var WASM_DYLINK_NEEDED = 0x2;
    var WASM_DYLINK_EXPORT_INFO = 0x3;
    var WASM_DYLINK_IMPORT_INFO = 0x4;
    var WASM_DYLINK_RUNTIME_PATH = 0x5;
    var WASM_SYMBOL_TLS = 0x100;
    var WASM_SYMBOL_BINDING_MASK = 0x3;
    var WASM_SYMBOL_BINDING_WEAK = 0x1;
    while (offset < end) {
      var subsectionType = getU8();
      var subsectionSize = getLEB();
      if (subsectionType === WASM_DYLINK_MEM_INFO) {
        customSection.memorySize = getLEB();
        customSection.memoryAlign = getLEB();
        customSection.tableSize = getLEB();
        customSection.tableAlign = getLEB();
      } else if (subsectionType === WASM_DYLINK_NEEDED) {
        customSection.neededDynlibs = getStringList();
      } else if (subsectionType === WASM_DYLINK_EXPORT_INFO) {
        var count = getLEB();
        while (count--) {
          var symname = getString();
          var flags = getLEB();
          if (flags & WASM_SYMBOL_TLS) {
            customSection.tlsExports.add(symname);
          }
        }
      } else if (subsectionType === WASM_DYLINK_IMPORT_INFO) {
        var count = getLEB();
        while (count--) {
          var modname = getString();
          var symname = getString();
          var flags = getLEB();
          if ((flags & WASM_SYMBOL_BINDING_MASK) == WASM_SYMBOL_BINDING_WEAK) {
            customSection.weakImports.add(symname);
          }
        }
      } else if (subsectionType === WASM_DYLINK_RUNTIME_PATH) {
        customSection.runtimePaths = getStringList();
      } else {
#if ASSERTIONS
        err('unknown dylink.0 subsection:', subsectionType)
#endif
        // unknown subsection
        offset += subsectionSize;
      }
    }

#if ASSERTIONS
    var tableAlign = Math.pow(2, customSection.tableAlign);
    assert(tableAlign === 1, `invalid tableAlign ${tableAlign}`);
    assert(offset == end);
#endif

#if DYLINK_DEBUG
    dbg('dylink needed:', customSection.neededDynlibs);
#endif

    return customSection;
  },

#if DYNCALLS || !WASM_BIGINT
  $registerDynCallSymbols: (exports) => {
    for (var [sym, exp] of Object.entries(exports)) {
      if (sym.startsWith('dynCall_')) {
        var sig = sym.substring(8);
        if (!dynCalls.hasOwnProperty(sig)) {
          dynCalls[sig] = exp;
        }
      }
    }
  },
#endif

  // Module.symbols <- libModule.symbols (flags.global handler)
  $mergeLibSymbols__deps: ['$isSymbolDefined'],
  $mergeLibSymbols: (exports, libName) => {
#if DYNCALLS || !WASM_BIGINT
    registerDynCallSymbols(exports);
#endif
    // add symbols into global namespace TODO: weak linking etc.
    for (var [sym, exp] of Object.entries(exports)) {
#if ASSERTIONS == 2
      if (isSymbolDefined(sym)) {
        var curr = wasmImports[sym], next = exp;
        // don't warn on functions - might be odr, linkonce_odr, etc.
        if (!(typeof curr == 'function' && typeof next == 'function')) {
          err(`warning: symbol '${sym}' from '${libName}' already exists (duplicate symbol? or weak linking, which isn't supported yet?)`); // + [curr, ' vs ', next]);
        }
      }
#endif

      // When RTLD_GLOBAL is enabled, the symbols defined by this shared object
      // will be made available for symbol resolution of subsequently loaded
      // shared objects.
      //
      // We should copy the symbols (which include methods and variables) from
      // SIDE_MODULE to MAIN_MODULE.
      const setImport = (target) => {
#if ASYNCIFY
        if (target in asyncifyStubs) {
          asyncifyStubs[target] = exp;
        }
#endif
        if (!isSymbolDefined(target)) {
          wasmImports[target] = exp;
        }
      }
      setImport(sym);

#if !hasExportedSymbol('main')
      // Special case for handling of main symbol:  If a side module exports
      // `main` that also acts a definition for `__main_argc_argv` and vice
      // versa.
      const main_alias = '__main_argc_argv';
      if (sym == 'main') {
        setImport(main_alias)
      }
      if (sym == main_alias) {
        setImport('main')
      }
#endif
    }
  },

#if DYLINK_DEBUG
  $dumpTable__deps: ['$wasmTable'],
  $dumpTable: () => {
    var len = wasmTable.length;
    for (var i = {{{ toIndexType(0) }}} ; i < len; i++) {
      dbg(`table: ${i} : ${wasmTable.get(i)}`);
    }
  },
#endif

  // Loads a side module from binary data or compiled Module. Returns the module's exports or a
  // promise that resolves to its exports if the loadAsync flag is set.
  $loadWebAssemblyModule__docs: `
   /**
    * @param {string=} libName
    * @param {Object=} localScope
    * @param {number=} handle
    */`,
  $loadWebAssemblyModule__deps: [
    '$loadDynamicLibrary', '$getMemory', '$updateGOT',
    '$relocateExports', '$resolveGlobalSymbol', '$GOTHandler',
    '$getDylinkMetadata', '$alignMemory',
    '$updateTableMap',
    '$wasmTable',
    '$addOnPostCtor',
  ],
  $loadWebAssemblyModule: (binary, flags, libName, localScope, handle) => {
#if DYLINK_DEBUG
    dbg('loadWebAssemblyModule:', libName, handle);
#endif
    var metadata = getDylinkMetadata(binary);

    // loadModule loads the wasm module after all its dependencies have been loaded.
    // can be called both sync/async.
    function loadModule() {
#if ASSERTIONS
      var originalTable = wasmTable;
#endif
#if PTHREADS
      // The first thread to load a given module needs to allocate the static
      // table and memory regions.  Later threads re-use the same table region
      // and can ignore the memory region (since memory is shared between
      // threads already).
      // If `handle` is specified than it is assumed that the calling thread has
      // exclusive access to it for the duration of this function.  See the
      // locking in `dynlink.c`.
      var firstLoad = !handle || !{{{ makeGetValue('handle', C_STRUCTS.dso.mem_allocated, 'i8') }}};
#if DYLINK_DEBUG
      dbg('firstLoad:', firstLoad);
#endif
      if (firstLoad) {
#endif
        // alignments are powers of 2
        var memAlign = Math.pow(2, metadata.memoryAlign);
        // prepare memory
        var memoryBase = metadata.memorySize ? alignMemory(getMemory(metadata.memorySize + memAlign), memAlign) : 0; // TODO: add to cleanups
        var tableBase = metadata.tableSize ? {{{ from64Expr('wasmTable.length') }}} : 0;
        if (handle) {
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_allocated, '1', 'i8') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_addr, 'memoryBase', '*') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_size, 'metadata.memorySize', 'i32') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.table_addr, 'tableBase', '*') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.table_size, 'metadata.tableSize', 'i32') }}};
        }
#if PTHREADS
      } else {
        // Read the values for tableBase and memoryBase from shared memory. The
        // thread that first loaded the DLL already set these values.
        memoryBase = {{{ makeGetValue('handle', C_STRUCTS.dso.mem_addr, '*') }}};
        tableBase = {{{ makeGetValue('handle', C_STRUCTS.dso.table_addr, '*') }}};
      }
#endif

      if (metadata.tableSize) {
#if ASSERTIONS
        assert({{{ from64Expr('wasmTable.length') }}} == tableBase, `unexpected table size while loading ${libName}: ${wasmTable.length}`);
#endif
#if DYLINK_DEBUG
        dbg("loadModule: growing table by: " + metadata.tableSize);
#endif
        wasmTable.grow({{{ toIndexType('metadata.tableSize') }}});
      }
#if DYLINK_DEBUG
      dbg(`loadModule: memory[${memoryBase}:${memoryBase + metadata.memorySize}]` +
                     ` table[${tableBase}:${tableBase + metadata.tableSize}]`);
#endif

      // This is the export map that we ultimately return.  We declare it here
      // so it can be used within resolveSymbol.  We resolve symbols against
      // this local symbol map in the case there they are not present on the
      // global Module object.  We need this fallback because Modules sometime
      // need to import their own symbols
      var moduleExports;

      function resolveSymbol(sym) {
        var resolved = resolveGlobalSymbol(sym).sym;
        if (!resolved && localScope) {
          resolved = localScope[sym];
        }
        if (!resolved) {
          resolved = moduleExports[sym];
        }
#if ASSERTIONS
        assert(resolved, `undefined symbol '${sym}'. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment`);
#endif
        return resolved;
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
        get(stubs, prop) {
          // symbols that should be local to this module
          switch (prop) {
            case '__memory_base':
              return {{{ to64('memoryBase') }}};
            case '__table_base':
              return {{{ to64('tableBase') }}};
#if MEMORY64
#if MEMORY64 == 2
            case '__memory_base32':
              return memoryBase;
#endif
            case '__table_base32':
              return tableBase;
#endif
          }
          if (prop in wasmImports && !wasmImports[prop].stub) {
            // No stub needed, symbol already exists in symbol table
            var res = wasmImports[prop];
#if ASYNCIFY
            // Asyncify wraps exports, and we need to look through those wrappers.
            if (res.orig) {
              res = res.orig;
            }
#endif
            return res;
          }
          // Return a stub function that will resolve the symbol
          // when first called.
          if (!(prop in stubs)) {
            var resolved;
            stubs[prop] = (...args) => {
              resolved ||= resolveSymbol(prop);
              return resolved(...args);
            };
          }
          return stubs[prop];
        }
      };
      var proxy = new Proxy({}, proxyHandler);
      var GOTProxy = new Proxy(metadata.weakImports, GOTHandler);
      var info = {
        'GOT.mem': GOTProxy,
        'GOT.func': GOTProxy,
        'env': proxy,
        '{{{ WASI_MODULE_NAME }}}': proxy,
      };

      function postInstantiation(module, instance) {
#if ASSERTIONS
        // the table should be unchanged
        assert(wasmTable === originalTable);
#endif
#if PTHREADS
        if (!ENVIRONMENT_IS_PTHREAD && libName) {
#if DYLINK_DEBUG
          dbg('registering sharedModules:', libName)
#endif
          // cache all loaded modules in `sharedModules`, which gets passed
          // to new workers when they are created.
          sharedModules[libName] = module;
        }
#endif
        // add new entries to functionsInTableMap
        updateTableMap(tableBase, metadata.tableSize);
        moduleExports = relocateExports(instance.exports, memoryBase);
        updateGOT(moduleExports);
#if ASYNCIFY
        moduleExports = Asyncify.instrumentWasmExports(moduleExports);
#endif
        if (!flags.allowUndefined) {
          reportUndefinedSymbols();
        }
#if STACK_OVERFLOW_CHECK >= 2
        // If the runtime has already been initialized we set the stack limits
        // now.  Otherwise this is delayed until `setDylinkStackLimits` is
        // called after initialization.
        if (moduleExports['__set_stack_limits'] && runtimeInitialized) {
          moduleExports['__set_stack_limits']({{{ to64('_emscripten_stack_get_base()') }}}, {{{ to64('_emscripten_stack_get_end()') }}});
        }
#endif

#if MAIN_MODULE
        function addEmAsm(addr, body) {
          var args = [];
          for (var arity = 0; ; arity++) {
            var argName = '$' + arity;
            if (!body.includes(argName)) break;
            args.push(argName);
          }
          args = args.join(',');
          var func = `(${args}) => { ${body} };`;
#if DYLINK_DEBUG
          dbg('adding new EM_ASM constant at:', ptrToString(start));
#endif
          {{{ makeEval('ASM_CONSTS[start] = eval(func)') }}};
        }

        // Add any EM_ASM function that exist in the side module
        if ('__start_em_asm' in moduleExports) {
          var start = moduleExports['__start_em_asm'].value;
          var stop = moduleExports['__stop_em_asm'].value;
#if CAN_ADDRESS_2GB
          start >>>= 0;
          stop >>>= 0;
#else
          {{{ from64('start') }}}
          {{{ from64('stop') }}}
#endif
          while (start < stop) {
            var jsString = UTF8ToString(start);
            addEmAsm(start, jsString);
            start = HEAPU8.indexOf(0, start) + 1;
          }
        }

        function addEmJs(name, cSig, body) {
          // The signature here is a C signature (e.g. "(int foo, char* bar)").
          // See `create_em_js` in emcc.py` for the build-time version of this
          // code.
          var jsArgs = [];
          cSig = cSig.slice(1, -1)
          if (cSig != 'void') {
            cSig = cSig.split(',');
            for (var arg of cSig) {
              var jsArg = arg.split(' ').pop();
              jsArgs.push(jsArg.replaceAll('*', ''));
            }
          }
          var func = `(${jsArgs}) => ${body};`;
#if DYLINK_DEBUG
          dbg(`adding new EM_JS function: ${jsArgs} = ${func}`);
#endif
          {{{ makeEval('moduleExports[name] = eval(func)') }}};
        }

        for (var name in moduleExports) {
          if (name.startsWith('__em_js__')) {
            var start = moduleExports[name].value
            var jsString = UTF8ToString({{{ from64Expr('start') }}});
            // EM_JS strings are stored in the data section in the form
            // SIG<::>BODY.
            var [sig, body] = jsString.split('<::>');
            addEmJs(name.replace('__em_js__', ''), sig, body);
            delete moduleExports[name];
          }
        }
#endif

        // initialize the module
#if PTHREADS
        // Only one thread should call __wasm_call_ctors, but all threads need
        // to call _emscripten_tls_init
        registerTLSInit(moduleExports['_emscripten_tls_init'], instance.exports, metadata)
        if (firstLoad) {
#endif
          var applyRelocs = moduleExports['__wasm_apply_data_relocs'];
          if (applyRelocs) {
            if (runtimeInitialized) {
#if DYLINK_DEBUG
              dbg('running __wasm_apply_data_relocs');
#endif
              applyRelocs();
            } else {
#if DYLINK_DEBUG
              dbg('delaying __wasm_apply_data_relocs');
#endif
              __RELOC_FUNCS__.push(applyRelocs);
            }
          }
          var init = moduleExports['__wasm_call_ctors'];
          if (init) {
            if (runtimeInitialized) {
#if DYLINK_DEBUG
              dbg('running __wasm_call_ctors');
#endif
              init();
            } else {
#if DYLINK_DEBUG
              dbg('delaying __wasm_call_ctors');
#endif
              // we aren't ready to run compiled code yet
              addOnPostCtor(init);
            }
          }
#if PTHREADS
        }
#endif
        return moduleExports;
      }

      if (flags.loadAsync) {
        return (async () => {
          var instance;
          if (binary instanceof WebAssembly.Module) {
            instance = new WebAssembly.Instance(binary, info);
          } else {
            // Destructuring assignment without declaration has to be wrapped
            // with parens or parser will treat the l-value as an object
            // literal instead.
            ({ module: binary, instance } = await WebAssembly.instantiate(binary, info));
          }
          return postInstantiation(binary, instance);
        })();
      }

      var module = binary instanceof WebAssembly.Module ? binary : new WebAssembly.Module(binary);
      var instance = new WebAssembly.Instance(module, info);
      return postInstantiation(module, instance);
    }

    // We need to set rpath in flags based on the current library's rpath.
    // We can't mutate flags or else if a depends on b and c and b depends on d,
    // then c will be loaded with b's rpath instead of a's.
    flags = {...flags, rpath: { parentLibPath: libName, paths: metadata.runtimePaths }}
    // now load needed libraries and the module itself.
    if (flags.loadAsync) {
      return metadata.neededDynlibs
        .reduce((chain, needed) => chain.then(() => {
#if FILESYSTEM
          needed = findLibraryFS(needed, flags.rpath) ?? needed;
#endif
          return loadDynamicLibrary(needed, flags, localScope);
        }), Promise.resolve())
        .then(loadModule);
    }

    for (var needed of metadata.neededDynlibs) {
#if FILESYSTEM
      needed = findLibraryFS(needed, flags.rpath) ?? needed;
#endif
      loadDynamicLibrary(needed, flags, localScope)
    }
    return loadModule();
  },

#if STACK_OVERFLOW_CHECK >= 2
  // Sometimes we load libraries before runtime initialization.  In this case
  // we delay calling __set_stack_limits (which must be called for each
  // module).
  $setDylinkStackLimits: (stackTop, stackMax) => {
    for (var name in LDSO.loadedLibsByName) {
#if DYLINK_DEBUG
      dbg(`setDylinkStackLimits for '${name}'`);
#endif
      var lib = LDSO.loadedLibsByName[name];
      lib.exports['__set_stack_limits']?.({{{ to64("stackTop") }}}, {{{ to64("stackMax") }}});
    }
  },
#endif

  $newDSO: (name, handle, syms) => {
    var dso = {
      refcount: Infinity,
      name,
      exports: syms,
      global: true,
    };
    LDSO.loadedLibsByName[name] = dso;
    if (handle != undefined) {
      LDSO.loadedLibsByHandle[handle] = dso;
    }
    return dso;
  },

#if FILESYSTEM
  $findLibraryFS__deps: [
    '$replaceORIGIN',
    '_emscripten_find_dylib',
    '$withStackSave',
    '$stackAlloc',
    '$lengthBytesUTF8',
    '$stringToUTF8OnStack',
    '$stringToUTF8',
    '$FS',
    '$PATH',
#if WASMFS
    '_wasmfs_identify',
    '_wasmfs_read_file',
#endif
  ],
  $findLibraryFS: (libName, rpath) => {
    // If we're preloading a dynamic library, the runtime is not ready to call
    // __wasmfs_identify or __emscripten_find_dylib. So just quit out.
    //
    // This means that DT_NEEDED for the main module and transitive dependencies
    // of it won't work with this code path. Similarly, it means that calling
    // loadDynamicLibrary in a preRun hook can't use this code path.
    if (!runtimeInitialized) {
      return undefined;
    }
    if (PATH.isAbs(libName)) {
#if WASMFS
      var result = withStackSave(() => __wasmfs_identify(stringToUTF8OnStack(libName)));
      return result === {{{ cDefs.EEXIST }}} ? libName : undefined;
#else
      try {
        FS.lookupPath(libName);
        return libName;
      } catch (e) {
        return undefined;
      }
#endif
    }
    var rpathResolved = (rpath?.paths || []).map((p) => replaceORIGIN(rpath?.parentLibPath, p));
    return withStackSave(() => {
      // In dylink.c we use: `char buf[2*NAME_MAX+2];` and NAME_MAX is 255.
      // So we use the same size here.
      var bufSize = 2*255 + 2;
      var buf = stackAlloc(bufSize);
      var rpathC = stringToUTF8OnStack(rpathResolved.join(':'));
      var libNameC = stringToUTF8OnStack(libName);
      var resLibNameC = __emscripten_find_dylib(buf, rpathC, libNameC, bufSize);
      return resLibNameC ? UTF8ToString(resLibNameC) : undefined;
    });
  },
#endif // FILESYSTEM

  // loadDynamicLibrary loads dynamic library @ lib URL / path and returns
  // handle for loaded DSO.
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
  // If a library was already loaded, it is not loaded a second time. However
  // flags.global and flags.nodelete are handled every time a load request is made.
  // Once a library becomes "global" or "nodelete", it cannot be removed or unloaded.
  $loadDynamicLibrary__deps: ['$LDSO', '$loadWebAssemblyModule',
                              '$mergeLibSymbols', '$newDSO',
                              '$asyncLoad',
#if FILESYSTEM
                              '$preloadedWasm',
                              '$findLibraryFS',
#endif
#if DYNCALLS || !WASM_BIGINT
                              '$registerDynCallSymbols',
#endif
  ],
  $loadDynamicLibrary__docs: `
    /**
     * @param {number=} handle
     * @param {Object=} localScope
     */`,
  $loadDynamicLibrary: function(libName, flags = {global: true, nodelete: true}, localScope, handle) {
#if DYLINK_DEBUG
    dbg(`loadDynamicLibrary: ${libName} handle: ${handle}`);
    dbg('existing:', Object.keys(LDSO.loadedLibsByName));
#endif
    // when loadDynamicLibrary did not have flags, libraries were loaded
    // globally & permanently

    var dso = LDSO.loadedLibsByName[libName];
    if (dso) {
      // the library is being loaded or has been loaded already.
#if ASSERTIONS
      assert(dso.exports !== 'loading', `Attempt to load '${libName}' twice before the first load completed`);
#endif
      if (!flags.global) {
        if (localScope) {
          Object.assign(localScope, dso.exports);
        }
#if DYNCALLS || !WASM_BIGINT
        registerDynCallSymbols(dso.exports);
#endif
      } else if (!dso.global) {
        // The library was previously loaded only locally but not
        // we have a request with global=true.
        dso.global = true;
        mergeLibSymbols(dso.exports, libName)
      }
      // same for "nodelete"
      if (flags.nodelete && dso.refcount !== Infinity) {
        dso.refcount = Infinity;
      }
      dso.refcount++
      if (handle) {
        LDSO.loadedLibsByHandle[handle] = dso;
      }
      return flags.loadAsync ? Promise.resolve(true) : true;
    }

    // allocate new DSO
    dso = newDSO(libName, handle, 'loading');
    dso.refcount = flags.nodelete ? Infinity : 1;
    dso.global = flags.global;

    // libName -> libData
    function loadLibData() {
#if PTHREADS
      var sharedMod = sharedModules[libName];
#if DYLINK_DEBUG
      dbg(`checking sharedModules: ${libName}: ${sharedMod ? 'found' : 'not found'}`);
#endif
      if (sharedMod) {
        return flags.loadAsync ? Promise.resolve(sharedMod) : sharedMod;
      }
#endif

      // for wasm, we can use fetch for async, but for fs mode we can only imitate it
      if (handle) {
        var data = {{{ makeGetValue('handle', C_STRUCTS.dso.file_data, '*') }}};
        var dataSize = {{{ makeGetValue('handle', C_STRUCTS.dso.file_data_size, '*') }}};
        if (data && dataSize) {
          var libData = HEAP8.slice(data, data + dataSize);
          return flags.loadAsync ? Promise.resolve(libData) : libData;
        }
      }

#if FILESYSTEM
      var f = findLibraryFS(libName, flags.rpath);
#if DYLINK_DEBUG
      dbg(`checking filesystem: ${libName}: ${f ? 'found' : 'not found'}`);
#endif
      if (f) {
        var libData = FS.readFile(f, {encoding: 'binary'});
        return flags.loadAsync ? Promise.resolve(libData) : libData;
      }
#endif

      var libFile = locateFile(libName);
      if (flags.loadAsync) {
        return asyncLoad(libFile);
      }

      // load the binary synchronously
      if (!readBinary) {
        throw new Error(`${libFile}: file not found, and synchronous loading of external files is not available`);
      }
      return readBinary(libFile);
    }

    // libName -> exports
    function getExports() {
#if FILESYSTEM
      // lookup preloaded cache first
      var preloaded = preloadedWasm[libName];
#if DYLINK_DEBUG
      dbg(`checking preloadedWasm: ${libName}: ${preloaded ? 'found' : 'not found'}`);
#endif
      if (preloaded) {
        return flags.loadAsync ? Promise.resolve(preloaded) : preloaded;
      }
#endif

      // module not preloaded - load lib data and create new module from it
      if (flags.loadAsync) {
        return loadLibData().then((libData) => loadWebAssemblyModule(libData, flags, libName, localScope, handle));
      }

      return loadWebAssemblyModule(loadLibData(), flags, libName, localScope, handle);
    }

    // module for lib is loaded - update the dso & global namespace
    function moduleLoaded(exports) {
      if (dso.global) {
        mergeLibSymbols(exports, libName);
      } else if (localScope) {
        Object.assign(localScope, exports);
#if DYNCALLS || !WASM_BIGINT
        registerDynCallSymbols(exports);
#endif
      }
      dso.exports = exports;
    }

    if (flags.loadAsync) {
#if DYLINK_DEBUG
      dbg("loadDynamicLibrary: done (async)");
#endif
      return getExports().then((exports) => {
        moduleLoaded(exports);
        return true;
      });
    }

    moduleLoaded(getExports());
#if DYLINK_DEBUG
    dbg("loadDynamicLibrary: done");
#endif
    return true;
  },

  $loadDylibs__internal: true,
  $loadDylibs__deps: ['$loadDynamicLibrary', '$reportUndefinedSymbols', '$addRunDependency', '$removeRunDependency'],
  $loadDylibs: async () => {
    if (!dynamicLibraries.length) {
#if DYLINK_DEBUG
      dbg('loadDylibs: no libraries to preload');
#endif
      reportUndefinedSymbols();
      return;
    }

#if DYLINK_DEBUG
    dbg('loadDylibs:', dynamicLibraries);
#endif
    addRunDependency('loadDylibs');

    // Load binaries asynchronously
    for (var lib of dynamicLibraries) {
      await loadDynamicLibrary(lib, {loadAsync: true, global: true, nodelete: true, allowUndefined: true})
    }
    // we got them all, wonderful
    reportUndefinedSymbols();

#if DYLINK_DEBUG
    dbg('loadDylibs done!');
#endif
    removeRunDependency('loadDylibs');
  },

  // void* dlopen(const char* filename, int flags);
  $dlopenInternal__deps: ['$dlSetError', '$PATH'],
  $dlopenInternal: (handle, jsflags) => {
    // void *dlopen(const char *file, int mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlopen.html
    var filename = UTF8ToString(handle + {{{ C_STRUCTS.dso.name }}});
    var flags = {{{ makeGetValue('handle', C_STRUCTS.dso.flags, 'i32') }}};
#if DYLINK_DEBUG
    dbg('dlopenInternal:', filename);
#endif
    filename = PATH.normalize(filename);
    var searchpaths = [];

    var global = Boolean(flags & {{{ cDefs.RTLD_GLOBAL }}});
    var localScope = global ? null : {};

    // We don't care about RTLD_NOW and RTLD_LAZY.
    var combinedFlags = {
      global,
      nodelete:  Boolean(flags & {{{ cDefs.RTLD_NODELETE }}}),
      loadAsync: jsflags.loadAsync,
    }

    if (jsflags.loadAsync) {
      return loadDynamicLibrary(filename, combinedFlags, localScope, handle);
    }

    try {
      return loadDynamicLibrary(filename, combinedFlags, localScope, handle)
    } catch (e) {
#if ASSERTIONS
      err(`error loading dynamic library ${filename}: ${e}`);
#endif
      dlSetError(`could not load dynamic lib: ${filename}\n${e}`);
      return 0;
    }
  },

  _dlopen_js__deps: ['$dlopenInternal'],
#if ASYNCIFY
  _dlopen_js__async: true,
#endif
  _dlopen_js: {{{ asyncIf(ASYNCIFY == 2) }}} (handle) =>
#if ASYNCIFY
    Asyncify.handleSleep((wakeUp) =>
      dlopenInternal(handle, { loadAsync: true })
      .then(wakeUp)
      // Note: this currently relies on being able to catch errors even from `wakeUp` callback itself.
      // That's why we can't refactor it to `handleAsync` at the moment.
      .catch(() => wakeUp(0))
    )
#else
    dlopenInternal(handle, { loadAsync: false })
#endif
    ,

  // Async version of dlopen.
  _emscripten_dlopen_js__deps: ['$dlopenInternal', '$callUserCallback', '$dlSetError'],
  _emscripten_dlopen_js: (handle, onsuccess, onerror, user_data) => {
    /** @param {Object=} e */
    function errorCallback(e) {
      var filename = UTF8ToString(handle + {{{ C_STRUCTS.dso.name }}});
      dlSetError(`'Could not load dynamic lib: ${filename}\n${e}`);
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {{{ makeDynCall('vpp', 'onerror') }}}(handle, user_data));
    }
    function successCallback() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(() => {{{ makeDynCall('vpp', 'onsuccess') }}}(handle, user_data));
    }

    {{{ runtimeKeepalivePush() }}}
    var promise = dlopenInternal(handle, { loadAsync: true });
    if (promise) {
      promise.then(successCallback, errorCallback);
    } else {
      errorCallback();
    }
  },

  _dlsym_catchup_js: (handle, symbolIndex) => {
#if DYLINK_DEBUG
    dbg("_dlsym_catchup: handle=" + ptrToString(handle) + " symbolIndex=" + symbolIndex);
#endif
    var lib = LDSO.loadedLibsByHandle[handle];
    var symDict = lib.exports;
    var symName = Object.keys(symDict)[symbolIndex];
    var sym = symDict[symName];
    var result = addFunction(sym, sym.sig);
#if DYLINK_DEBUG
    dbg(`_dlsym_catchup: result=${result}`);
#endif
    return result;
  },

  // void* dlsym(void* handle, const char* symbol);
  _dlsym_js__deps: ['$dlSetError', '$getFunctionAddress', '$addFunction'],
  _dlsym_js: (handle, symbol, symbolIndex) => {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = UTF8ToString(symbol);
#if DYLINK_DEBUG
    dbg('dlsym_js:', symbol);
#endif
    var result;
    var newSymIndex;

    var lib = LDSO.loadedLibsByHandle[handle];
#if ASSERTIONS
    assert(lib, `Tried to dlsym() from an unopened handle: ${handle}`);
#endif
    newSymIndex = Object.keys(lib.exports).indexOf(symbol);
    if (newSymIndex == -1 || lib.exports[symbol].stub) {
      dlSetError(`Tried to lookup unknown symbol "${symbol}" in dynamic lib: ${lib.name}`)
      return 0;
    }
#if !WASM_BIGINT
    var origSym = 'orig$' + symbol;
    result = lib.exports[origSym];
    if (result) {
      newSymIndex = Object.keys(lib.exports).indexOf(origSym);
    }
    else
#endif
    result = lib.exports[symbol];

    if (typeof result == 'function') {
#if DYLINK_DEBUG
      dbg(`dlsym_js: ${symbol} getting table slot for: ${result}`);
#endif

#if ASYNCIFY
      // Asyncify wraps exports, and we need to look through those wrappers.
      if (result.orig) {
        result = result.orig;
      }
#endif
      var addr = getFunctionAddress(result);
      if (addr) {
#if DYLINK_DEBUG
        dbg('symbol already exists in table:', symbol);
#endif
        result = addr;
      } else {
        // Insert the function into the wasm table.  If its a direct wasm
        // function the second argument will not be needed.  If its a JS
        // function we rely on the `sig` attribute being set based on the
        // `<func>__sig` specified in library JS file.
        result = addFunction(result, result.sig);
#if DYLINK_DEBUG
        dbg('adding symbol to table:',  symbol);
#endif
        {{{ makeSetValue('symbolIndex', 0, 'newSymIndex', '*') }}};
      }
    }
#if DYLINK_DEBUG
    dbg(`dlsym_js: ${symbol} -> ${result}`);
#endif
    return result;
  },
};

addToLibrary(LibraryDylink);
