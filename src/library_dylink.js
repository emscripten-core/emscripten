// ==========================================================================
// Dynamic library loading
//
// ==========================================================================

var LibraryDylink = {
#if RELOCATABLE
  $resolveGlobalSymbol__internal: true,
  $resolveGlobalSymbol__deps: ['$asmjsMangle'],
  $resolveGlobalSymbol: function(symName, direct) {
    var sym;
#if !WASM_BIGINT
    if (direct) {
      // First look for the orig$ symbol which is the symbols without
      // any legalization performed.
      sym = asmLibraryArg['orig$' + symName];
    }
#endif
    if (!sym) {
      sym = asmLibraryArg[symName];
    }

    // Check for the symbol on the Module object.  This is the only
    // way to dynamically access JS library symbols that were not
    // referenced by the main module (and therefore not part of the
    // initial set of symbols included in asmLibraryArg when it
    // was declared.
    if (!sym) {
      sym = Module[asmjsMangle(symName)];
    }

    if (!sym && symName.startsWith('invoke_')) {
      sym = createInvokeFunction(symName.split('_')[1]);
    }

#if !DISABLE_EXCEPTION_CATCHING
    if (!sym && symName.startsWith("__cxa_find_matching_catch")) {
      sym = Module["___cxa_find_matching_catch"];
    }
#endif
    return sym;
  },

  $GOT: {},

  // Create globals to each imported symbol.  These are all initialized to zero
  // and get assigned later in `updateGOT`
  $GOTHandler__internal: true,
  $GOTHandler__deps: ['$GOT'],
  $GOTHandler: {
    'get': function(obj, symName) {
      if (!GOT[symName]) {
        GOT[symName] = new WebAssembly.Global({'value': '{{{ POINTER_TYPE }}}', 'mutable': true});
#if DYLINK_DEBUG
        err("new GOT entry: " + symName);
#endif
      }
      return GOT[symName]
    }
  },

  $isInternalSym__internal: true,
  $isInternalSym: function(symName) {
    // TODO: find a way to mark these in the binary or avoid exporting them.
    return [
      '__cpp_exception',
      '__c_longjmp',
      '__wasm_apply_data_relocs',
      '__dso_handle',
      '__tls_size',
      '__tls_align',
      '__set_stack_limits',
      'emscripten_tls_init',
      '__wasm_init_tls',
      '__wasm_call_ctors',
    ].includes(symName)
#if SPLIT_MODULE
        // Exports synthesized by wasm-split should be prefixed with '%'
        || symName[0] == '%'
#endif
    ;
  },

  $updateGOT__internal: true,
  $updateGOT__deps: ['$GOT', '$isInternalSym'],
  $updateGOT: function(exports, replace) {
#if DYLINK_DEBUG
    err("updateGOT: adding " + Object.keys(exports).length + " symbols");
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

      if (!GOT[symName]) {
        GOT[symName] = new WebAssembly.Global({'value': 'i32', 'mutable': true});
      }
      if (replace || GOT[symName].value == 0) {
#if DYLINK_DEBUG
        err("updateGOT: before: " + symName + ' : ' + GOT[symName].value);
#endif
        if (typeof value == 'function') {
          GOT[symName].value = addFunction(value);
#if DYLINK_DEBUG
          err("updateGOT: FUNC: " + symName + ' : ' + GOT[symName].value);
#endif
        } else if (typeof value == 'number') {
          GOT[symName].value = value;
        } else if (typeof value == 'bigint') {
          GOT[symName].value = Number(value);
        } else {
          err("unhandled export type for `" + symName + "`: " + (typeof value));
        }
#if DYLINK_DEBUG
        err("updateGOT:  after: " + symName + ' : ' + GOT[symName].value + ' (' + value + ')');
#endif
      }
#if DYLINK_DEBUG
      else if (GOT[symName].value != value) {
        err("updateGOT: EXISTING SYMBOL: " + symName + ' : ' + GOT[symName].value + ' (' + value + ')');
      }
#endif
    }
#if DYLINK_DEBUG
    err("done updateGOT");
#endif
  },

  // Applies relocations to exported things.
  $relocateExports__internal: true,
  $relocateExports__deps: ['$updateGOT'],
  $relocateExports__docs: '/** @param {boolean=} replace */',
  $relocateExports: function(exports, memoryBase, replace) {
    var relocated = {};

    for (var e in exports) {
      var value = exports[e];
#if SPLIT_MODULE
      // Do not modify exports synthesized by wasm-split
      if (e.startsWith('%')) {
        relocated[e] = value
        continue;
      }
#endif
      if (typeof value == 'object') {
        // a breaking change in the wasm spec, globals are now objects
        // https://github.com/WebAssembly/mutable-global/issues/1
        value = value.value;
      }
      if (typeof value == 'number') {
        value += memoryBase;
      }
      relocated[e] = value;
    }
    updateGOT(relocated, replace);
    return relocated;
  },

  $reportUndefinedSymbols__internal: true,
  $reportUndefinedSymbols__deps: ['$GOT', '$resolveGlobalSymbol'],
  $reportUndefinedSymbols: function() {
#if DYLINK_DEBUG
    err('reportUndefinedSymbols');
#endif
    for (var symName in GOT) {
      if (GOT[symName].value == 0) {
        var value = resolveGlobalSymbol(symName, true)
#if ASSERTIONS
        assert(value, 'undefined symbol `' + symName + '`. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment');
#endif
#if DYLINK_DEBUG
        err('assigning dynamic symbol from main module: ' + symName + ' -> ' + prettyPrint(value));
#endif
        if (typeof value == 'function') {
          /** @suppress {checkTypes} */
          GOT[symName].value = addFunction(value, value.sig);
#if DYLINK_DEBUG
          err('assigning table entry for : ' + symName + ' -> ' + GOT[symName].value);
#endif
        } else if (typeof value == 'number') {
          GOT[symName].value = value;
        } else {
          throw new Error('bad export type for `' + symName + '`: ' + (typeof value));
        }
      }
    }
#if DYLINK_DEBUG
    err('done reportUndefinedSymbols');
#endif
  },
#endif

#if MAIN_MODULE == 0
  _dlopen_js: function(filename, flag) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  _emscripten_dlopen_js: function(filename, flags, user_data, onsuccess, onerror) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  _dlsym_js: function(handle, symbol) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
#else // MAIN_MODULE != 0
  // dynamic linker/loader (a-la ld.so on ELF systems)
  $LDSO: {
    // name -> dso [refcount, name, module, global]; Used by dlopen
    loadedLibsByName: {},
    // handle  -> dso; Used by dlsym
    loadedLibsByHandle: {},
  },

  $dlSetError__internal: true,
  $dlSetError__deps: ['__dl_seterr',
#if MINIMAL_RUNTIME
   '$intArrayFromString'
#endif
  ],
  $dlSetError: function(msg) {
    withStackSave(function() {
      var cmsg = allocateUTF8OnStack(msg);
      ___dl_seterr(cmsg);
    });
  },

  // Dynamic version of shared.py:make_invoke.  This is needed for invokes
  // that originate from side modules since these are not known at JS
  // generation time.
  $createInvokeFunction__internal: true,
  $createInvokeFunction__deps: ['$dynCall', 'setThrew'],
  $createInvokeFunction: function(sig) {
    return function() {
      var sp = stackSave();
      try {
        return dynCall(sig, arguments[0], Array.prototype.slice.call(arguments, 1));
      } catch(e) {
        stackRestore(sp);
        // Exceptions thrown from C++ exception will be integer numbers.
        // longjmp will throw the number Infinity. Re-throw other types of
        // exceptions using a compact and fast check.
        if (e !== e+0) throw e;
        _setThrew(1, 0);
      }
    }
  },

  // We support some amount of allocation during startup in the case of
  // dynamic linking, which needs to allocate memory for dynamic libraries that
  // are loaded. That has to happen before the main program can start to run,
  // because the main program needs those linked in before it runs (so we can't
  // use normally malloc from the main program to do these allocations).

  // Allocate memory even if malloc isn't ready yet.
  $getMemory__deps: ['$GOT', '__heap_base'],
  $getMemory: function(size) {
    // After the runtime is initialized, we must only use sbrk() normally.
#if DYLINK_DEBUG
    err("getMemory: " + size + " runtimeInitialized=" + runtimeInitialized);
#endif
    if (runtimeInitialized)
      return _malloc(size);
    var ret = ___heap_base;
    var end = (ret + size + 15) & -16;
#if ASSERTIONS
    assert(end <= HEAP8.length, 'failure to getMemory - memory growth etc. is not supported there, call malloc/sbrk directly or increase INITIAL_MEMORY');
#endif
    ___heap_base = end;
    GOT['__heap_base'].value = end;
    return ret;
  },

  // returns the side module metadata as an object
  // { memorySize, memoryAlign, tableSize, tableAlign, neededDynlibs}
  $getDylinkMetadata__internal: true,
  $getDylinkMetadata: function(binary) {
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

    /** @param {string=} message */
    function failIf(condition, message) {
      if (condition) throw new Error(message);
    }

    var name = 'dylink.0';
    if (binary instanceof WebAssembly.Module) {
      var dylinkSection = WebAssembly.Module.customSections(binary, name);
      if (dylinkSection.length === 0) {
        name = 'dylink'
        dylinkSection = WebAssembly.Module.customSections(binary, name);
      }
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
      name = getString();
    }

    var customSection = { neededDynlibs: [], tlsExports: {} };
    if (name == 'dylink') {
      customSection.memorySize = getLEB();
      customSection.memoryAlign = getLEB();
      customSection.tableSize = getLEB();
      customSection.tableAlign = getLEB();
      // shared libraries this module needs. We need to load them first, so that
      // current module could resolve its imports. (see tools/shared.py
      // WebAssembly.make_shared_library() for "dylink" section extension format)
      var neededDynlibsCount = getLEB();
      for (var i = 0; i < neededDynlibsCount; ++i) {
        var libname = getString();
        customSection.neededDynlibs.push(libname);
      }
    } else {
      failIf(name !== 'dylink.0');
      var WASM_DYLINK_MEM_INFO = 0x1;
      var WASM_DYLINK_NEEDED = 0x2;
      var WASM_DYLINK_EXPORT_INFO = 0x3;
      var WASM_SYMBOL_TLS = 0x100;
      while (offset < end) {
        var subsectionType = getU8();
        var subsectionSize = getLEB();
        if (subsectionType === WASM_DYLINK_MEM_INFO) {
          customSection.memorySize = getLEB();
          customSection.memoryAlign = getLEB();
          customSection.tableSize = getLEB();
          customSection.tableAlign = getLEB();
        } else if (subsectionType === WASM_DYLINK_NEEDED) {
          var neededDynlibsCount = getLEB();
          for (var i = 0; i < neededDynlibsCount; ++i) {
            libname = getString();
            customSection.neededDynlibs.push(libname);
          }
        } else if (subsectionType === WASM_DYLINK_EXPORT_INFO) {
          var count = getLEB();
          while (count--) {
            var symname = getString();
            var flags = getLEB();
            if (flags & WASM_SYMBOL_TLS) {
              customSection.tlsExports[symname] = 1;
            }
          }
        } else {
#if ASSERTIONS
          err('unknown dylink.0 subsection: ' + subsectionType)
#endif
          // unknown subsection
          offset += subsectionSize;
        }
      }
    }

#if ASSERTIONS
    var tableAlign = Math.pow(2, customSection.tableAlign);
    assert(tableAlign === 1, 'invalid tableAlign ' + tableAlign);
    assert(offset == end);
#endif

#if DYLINK_DEBUG
    err('dylink needed:' + customSection.neededDynlibs);
#endif

    return customSection;
  },

  // Module.symbols <- libModule.symbols (flags.global handler)
  $mergeLibSymbols__deps: ['$asmjsMangle'],
  $mergeLibSymbols: function(exports, libName) {
    // add symbols into global namespace TODO: weak linking etc.
    for (var sym in exports) {
      if (!exports.hasOwnProperty(sym)) {
        continue;
      }

      // When RTLD_GLOBAL is enable, the symbols defined by this shared object will be made
      // available for symbol resolution of subsequently loaded shared objects.
      //
      // We should copy the symbols (which include methods and variables) from SIDE_MODULE to MAIN_MODULE.

      if (!asmLibraryArg.hasOwnProperty(sym)) {
        asmLibraryArg[sym] = exports[sym];
      }
#if ASSERTIONS == 2
      else {
        var curr = asmLibraryArg[sym], next = exports[sym];
        // don't warn on functions - might be odr, linkonce_odr, etc.
        if (!(typeof curr == 'function' && typeof next == 'function')) {
          err("warning: symbol '" + sym + "' from '" + libName + "' already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
        }
      }
#endif

      // Export native export on the Module object.
      // TODO(sbc): Do all users want this?  Should we skip this by default?
      var module_sym = asmjsMangle(sym);
      if (!Module.hasOwnProperty(module_sym)) {
        Module[module_sym] = exports[sym];
      }
    }
  },

#if DYLINK_DEBUG
  $dumpTable: function() {
    for (var i = 0; i < wasmTable.length; i++)
      err('table: ' + i + ' : ' + wasmTable.get(i));
  },
#endif

  // Loads a side module from binary data or compiled Module. Returns the module's exports or a
  // promise that resolves to its exports if the loadAsync flag is set.
  $loadWebAssemblyModule__docs: '/** @param {number=} handle */',
  $loadWebAssemblyModule__deps: [
    '$loadDynamicLibrary', '$createInvokeFunction', '$getMemory',
    '$relocateExports', '$resolveGlobalSymbol', '$GOTHandler',
    '$getDylinkMetadata', '$alignMemory', '$zeroMemory',
  ],
  $loadWebAssemblyModule: function(binary, flags, handle) {
    var metadata = getDylinkMetadata(binary);
#if ASSERTIONS
    var originalTable = wasmTable;
#endif

    // loadModule loads the wasm module after all its dependencies have been loaded.
    // can be called both sync/async.
    function loadModule() {
      // The first thread to load a given module needs to allocate the static
      // table and memory regions.  Later threads re-use the same table region
      // and can ignore the memory region (since memory is shared between
      // threads already).
      var needsAllocation = !handle || !{{{ makeGetValue('handle', C_STRUCTS.dso.mem_allocated, 'i8') }}};
      if (needsAllocation) {
        // alignments are powers of 2
        var memAlign = Math.pow(2, metadata.memoryAlign);
        // finalize alignments and verify them
        memAlign = Math.max(memAlign, STACK_ALIGN); // we at least need stack alignment
        // prepare memory
        var memoryBase = metadata.memorySize ? alignMemory(getMemory(metadata.memorySize + memAlign), memAlign) : 0; // TODO: add to cleanups
        var tableBase = metadata.tableSize ? wasmTable.length : 0;
        if (handle) {
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_allocated, '1', 'i8') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_addr, 'memoryBase', '*') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.mem_size, 'metadata.memorySize', 'i32') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.table_addr, 'tableBase', '*') }}};
          {{{ makeSetValue('handle', C_STRUCTS.dso.table_size, 'metadata.tableSize', 'i32') }}};
        }
      } else {
        memoryBase = {{{ makeGetValue('handle', C_STRUCTS.dso.mem_addr, '*') }}};
        tableBase = {{{ makeGetValue('handle', C_STRUCTS.dso.table_addr, '*') }}};
      }

      var tableGrowthNeeded = tableBase + metadata.tableSize - wasmTable.length;
      if (tableGrowthNeeded > 0) {
#if DYLINK_DEBUG
        err("loadModule: growing table: " + tableGrowthNeeded);
#endif
        wasmTable.grow(tableGrowthNeeded);
      }
#if DYLINK_DEBUG
      err("loadModule: memory[" + memoryBase + ":" + (memoryBase + metadata.memorySize) + "]" +
                     " table[" + tableBase + ":" + (tableBase + metadata.tableSize) + "]");
#endif

      // This is the export map that we ultimately return.  We declare it here
      // so it can be used within resolveSymbol.  We resolve symbols against
      // this local symbol map in the case there they are not present on the
      // global Module object.  We need this fallback because:
      // a) Modules sometime need to import their own symbols
      // b) Symbols from side modules are not always added to the global namespace.
      var moduleExports;

      function resolveSymbol(sym) {
        var resolved = resolveGlobalSymbol(sym, false);
        if (!resolved) {
          resolved = moduleExports[sym];
        }
#if ASSERTIONS
        assert(resolved, 'undefined symbol `' + sym + '`. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment');
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
        'get': function(stubs, prop) {
          // symbols that should be local to this module
          switch (prop) {
            case '__memory_base':
              return memoryBase;
            case '__table_base':
              return tableBase;
          }
          if (prop in asmLibraryArg) {
            // No stub needed, symbol already exists in symbol table
            return asmLibraryArg[prop];
          }
          // Return a stub function that will resolve the symbol
          // when first called.
          if (!(prop in stubs)) {
            var resolved;
            stubs[prop] = function() {
              if (!resolved) resolved = resolveSymbol(prop);
              return resolved.apply(null, arguments);
            };
          }
          return stubs[prop];
        }
      };
      var proxy = new Proxy({}, proxyHandler);
      var info = {
        'GOT.mem': new Proxy({}, GOTHandler),
        'GOT.func': new Proxy({}, GOTHandler),
        'env': proxy,
        {{{ WASI_MODULE_NAME }}}: proxy,
      };

      function postInstantiation(instance) {
#if ASSERTIONS
        // the table should be unchanged
        assert(wasmTable === originalTable);
#endif
        // add new entries to functionsInTableMap
        updateTableMap(tableBase, metadata.tableSize);
        moduleExports = relocateExports(instance.exports, memoryBase);
        if (!flags.allowUndefined) {
          reportUndefinedSymbols();
        }
#if STACK_OVERFLOW_CHECK >= 2
        moduleExports['__set_stack_limits'](_emscripten_stack_get_base(), _emscripten_stack_get_end())
#endif

        // initialize the module
#if USE_PTHREADS
        // Only one thread (currently The main thread) should call
        // __wasm_call_ctors, but all threads need to call emscripten_tls_init
        registerTlsInit(moduleExports['emscripten_tls_init'], instance.exports, metadata)
        if (!ENVIRONMENT_IS_PTHREAD) {
#endif
          var init = moduleExports['__wasm_call_ctors'];
          if (init) {
            if (runtimeInitialized) {
              init();
            } else {
              // we aren't ready to run compiled code yet
              __ATINIT__.push(init);
            }
          }
#if USE_PTHREADS
        }
#endif
        return moduleExports;
      }

      if (flags.loadAsync) {
        if (binary instanceof WebAssembly.Module) {
          var instance = new WebAssembly.Instance(binary, info);
          return Promise.resolve(postInstantiation(instance));
        }
        return WebAssembly.instantiate(binary, info).then(function(result) {
          return postInstantiation(result.instance);
        });
      }

      var module = binary instanceof WebAssembly.Module ? binary : new WebAssembly.Module(binary);
      var instance = new WebAssembly.Instance(module, info);
      return postInstantiation(instance);
    }

    // now load needed libraries and the module itself.
    if (flags.loadAsync) {
      return metadata.neededDynlibs.reduce(function(chain, dynNeeded) {
        return chain.then(function() {
          return loadDynamicLibrary(dynNeeded, flags);
        });
      }, Promise.resolve()).then(function() {
        return loadModule();
      });
    }

    metadata.neededDynlibs.forEach(function(dynNeeded) {
      loadDynamicLibrary(dynNeeded, flags);
    });
    return loadModule();
  },

#if STACK_OVERFLOW_CHECK >= 2
  $setDylinkStackLimits: function(stackTop, stackMax) {
    for (var name in LDSO.loadedLibsByName) {
#if DYLINK_DEBUG
      err('setDylinkStackLimits[' + name + ']');
#endif
      var lib = LDSO.loadedLibsByName[name];
      lib.module['__set_stack_limits'](stackTop, stackMax);
    }
  },
#endif

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
  // - if flags.fs is provided, it is used as FS-like interface to load library data.
  //   By default, when flags.fs=undefined, native loading capabilities of the
  //   environment are used.
  //
  // If a library was already loaded, it is not loaded a second time. However
  // flags.global and flags.nodelete are handled every time a load request is made.
  // Once a library becomes "global" or "nodelete", it cannot be removed or unloaded.
  $loadDynamicLibrary__deps: ['$LDSO', '$loadWebAssemblyModule', '$asmjsMangle', '$isInternalSym', '$mergeLibSymbols'],
  $loadDynamicLibrary__docs: '/** @param {number=} handle */',
  $loadDynamicLibrary: function(lib, flags, handle) {
#if DYLINK_DEBUG
    err('loadDynamicLibrary: ' + lib + ' handle:' + handle);
    err('existing: ' + Object.keys(LDSO.loadedLibsByName));
#endif
    if (lib == '__main__' && !LDSO.loadedLibsByName[lib]) {
      LDSO.loadedLibsByName[lib] = {
        refcount: Infinity,   // = nodelete
        name:     '__main__',
        module:   Module['asm'],
        global:   true
      };
    }

    // when loadDynamicLibrary did not have flags, libraries were loaded
    // globally & permanently
    flags = flags || {global: true, nodelete: true}

    var dso = LDSO.loadedLibsByName[lib];
    if (dso) {
      // the library is being loaded or has been loaded already.
      //
      // however it could be previously loaded only locally and if we get
      // load request with global=true we have to make it globally visible now.
      if (flags.global && !dso.global) {
        dso.global = true;
        if (dso.module !== 'loading') {
          // ^^^ if module is 'loading' - symbols merging will be eventually done by the loader.
          mergeLibSymbols(dso.module, lib)
        }
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
    dso = {
      refcount: flags.nodelete ? Infinity : 1,
      name:     lib,
      module:   'loading',
      global:   flags.global,
    };
    LDSO.loadedLibsByName[lib] = dso;
    if (handle) {
      LDSO.loadedLibsByHandle[handle] = dso;
    }

    // libData <- libFile
    function loadLibData(libFile) {
      // for wasm, we can use fetch for async, but for fs mode we can only imitate it
      if (flags.fs && flags.fs.findObject(libFile)) {
        var libData = flags.fs.readFile(libFile, {encoding: 'binary'});
        if (!(libData instanceof Uint8Array)) {
          libData = new Uint8Array(libData);
        }
        return flags.loadAsync ? Promise.resolve(libData) : libData;
      }

      if (flags.loadAsync) {
        return new Promise(function(resolve, reject) {
          readAsync(libFile, function(data) { resolve(new Uint8Array(data)); }, reject);
        });
      }

      // load the binary synchronously
      if (!readBinary) {
        throw new Error(libFile + ': file not found, and synchronous loading of external files is not available');
      }
      return readBinary(libFile);
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
          return loadWebAssemblyModule(libData, flags, handle);
        });
      }

      return loadWebAssemblyModule(loadLibData(lib), flags, handle);
    }

    // module for lib is loaded - update the dso & global namespace
    function moduleLoaded(libModule) {
      if (dso.global) {
        mergeLibSymbols(libModule, lib);
      }
      dso.module = libModule;
    }

    if (flags.loadAsync) {
#if DYLINK_DEBUG
      err("loadDynamicLibrary: done (async)");
#endif
      return getLibModule().then(function(libModule) {
        moduleLoaded(libModule);
        return true;
      });
    }

    moduleLoaded(getLibModule());
#if DYLINK_DEBUG
    err("loadDynamicLibrary: done");
#endif
    return true;
  },

  $preloadDylibs__internal: true,
  $preloadDylibs__deps: ['$loadDynamicLibrary', '$reportUndefinedSymbols'],
  $preloadDylibs: function() {
#if DYLINK_DEBUG
    err('preloadDylibs');
#endif
    if (!dynamicLibraries.length) {
#if DYLINK_DEBUG
      err('preloadDylibs: no libraries to preload');
#endif
      reportUndefinedSymbols();
      return;
    }

    // Load binaries asynchronously
    addRunDependency('preloadDylibs');
    dynamicLibraries.reduce(function(chain, lib) {
      return chain.then(function() {
        return loadDynamicLibrary(lib, {loadAsync: true, global: true, nodelete: true, allowUndefined: true});
      });
    }, Promise.resolve()).then(function() {
      // we got them all, wonderful
      reportUndefinedSymbols();
      removeRunDependency('preloadDylibs');
#if DYLINK_DEBUG
    err('preloadDylibs done!');
#endif
    });
  },

  // void* dlopen(const char* filename, int flags);
  $dlopenInternal__deps: ['$FS', '$ENV', '$dlSetError', '$PATH'],
  $dlopenInternal: function(handle, jsflags) {
    // void *dlopen(const char *file, int mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlopen.html
    var filename = UTF8ToString(handle + {{{ C_STRUCTS.dso.name }}});
    var flags = {{{ makeGetValue('handle', C_STRUCTS.dso.flags, 'i32') }}};
#if DYLINK_DEBUG
    err('dlopenInternal: ' + filename);
#endif
    filename = PATH.normalize(filename);
    var searchpaths = [];

    var isValidFile = (filename) => {
      var target = FS.findObject(filename);
      return target && !target.isFolder && !target.isDevice;
    };

    if (!isValidFile(filename)) {
      if (ENV['LD_LIBRARY_PATH']) {
        searchpaths = ENV['LD_LIBRARY_PATH'].split(':');
      }

      for (var ident in searchpaths) {
        var searchfile = PATH.join2(searchpaths[ident], filename);
        if (isValidFile(searchfile)) {
          filename = searchfile;
          break;
        }
      }
    }

    // We don't care about RTLD_NOW and RTLD_LAZY.
    var combinedFlags = {
      global:    Boolean(flags & {{{ cDefine('RTLD_GLOBAL') }}}),
      nodelete:  Boolean(flags & {{{ cDefine('RTLD_NODELETE') }}}),
      loadAsync: jsflags.loadAsync,
      fs:        jsflags.fs,
    }

    if (jsflags.loadAsync) {
      return loadDynamicLibrary(filename, combinedFlags, handle);
    }

    try {
      return loadDynamicLibrary(filename, combinedFlags, handle)
    } catch (e) {
#if ASSERTIONS
      err('Error in loading dynamic library ' + filename + ": " + e);
#endif
      dlSetError('Could not load dynamic lib: ' + filename + '\n' + e);
      return 0;
    }
  },

  _dlopen_js__deps: ['$dlopenInternal'],
  _dlopen_js__sig: 'iiii',
  _dlopen_js: function(handle) {
#if ASYNCIFY
    return Asyncify.handleSleep(function(wakeUp) {
      var jsflags = {
        loadAsync: true,
        fs: FS, // load libraries from provided filesystem
      }
      var promise = dlopenInternal(handle, jsflags);
      promise.then(wakeUp).catch(function() { wakeUp(0) });
    });
#else
    var jsflags = {
      loadAsync: false,
      fs: FS, // load libraries from provided filesystem
    }
    return dlopenInternal(handle, jsflags);
#endif
  },

  // Async version of dlopen.
  _emscripten_dlopen_js__deps: ['$dlopenInternal', '$callUserCallback', '$dlSetError',
#if !MINIMAL_RUNTIME
    '$runtimeKeepalivePush',
    '$runtimeKeepalivePop',
#endif
  ],
  _emscripten_dlopen_js__sig: 'viiiii',
  _emscripten_dlopen_js: function(handle, onsuccess, onerror) {
    /** @param {Object=} e */
    function errorCallback(e) {
      var filename = UTF8ToString({{{ makeGetValue('handle', C_STRUCTS.dso.name, '*') }}});
      dlSetError('Could not load dynamic lib: ' + filename + '\n' + e);
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function () { {{{ makeDynCall('vi', 'onerror') }}}(handle); });
    }
    function successCallback() {
      {{{ runtimeKeepalivePop() }}}
      callUserCallback(function () { {{{ makeDynCall('vii', 'onsuccess') }}}(handle); });
    }

    {{{ runtimeKeepalivePush() }}}
    var promise = dlopenInternal(handle, { loadAsync: true });
    if (promise) {
      promise.then(successCallback, errorCallback);
    } else {
      errorCallback();
    }
  },

  // void* dlsym(void* handle, const char* symbol);
  _dlsym_js__deps: ['$dlSetError'],
  _dlsym_js__sig: 'iii',
  _dlsym_js: function(handle, symbol) {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = UTF8ToString(symbol);
    var result;

    if (handle == {{{ cDefine('RTLD_DEFAULT') }}}) {
      result = resolveGlobalSymbol(symbol, true);
      if (!result) {
        dlSetError('Tried to lookup unknown symbol "' + symbol + '" in dynamic lib: RTLD_DEFAULT');
        return 0;
      }
    } else {
      var lib = LDSO.loadedLibsByHandle[handle];
#if ASSERTIONS
      assert(lib, 'Tried to dlsym() from an unopened handle: ' + handle);
#endif
      if (!lib.module.hasOwnProperty(symbol)) {
        dlSetError('Tried to lookup unknown symbol "' + symbol + '" in dynamic lib: ' + lib.name)
        return 0;
      }
#if !WASM_BIGINT
      result = lib.module['orig$' + symbol];
      if (!result)
#endif
      result = lib.module[symbol];
    }

    if (typeof result == 'function') {
#if DYLINK_DEBUG
      err('dlsym: ' + symbol + ' getting table slot for: ' + result);
#endif
      // Insert the function into the wasm table.  If its a direct wasm function
      // the second argument will not be needed.  If its a JS function we rely
      // on the `sig` attribute being set based on the `<func>__sig` specified
      // in library JS file.
      result = addFunction(result, result.sig);
    }
#if DYLINK_DEBUG
    err('dlsym: ' + symbol + ' -> ' + result);
#endif
    return result;
  },
#endif // MAIN_MODULE != 0
};

mergeInto(LibraryManager.library, LibraryDylink);
