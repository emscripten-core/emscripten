// ==========================================================================
// Dynamic library loading
//
// ==========================================================================

var LibraryDylink = {
#if MAIN_MODULE == 0
  dlopen: function(filename, flag) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  dlclose: function(handle) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  dlsym: function(handle, symbol) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  dlerror: function() {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
  dladdr: function(address, info) {
    abort("To use dlopen, you need to use Emscripten's linking support, see https://github.com/emscripten-core/emscripten/wiki/Linking");
  },
#else // MAIN_MODULE != 0
  $DLFCN: {
    error: null,
    errorMsg: null,
  },

  // dynamic linker/loader (a-la ld.so on ELF systems)
  $LDSO: {
    // next free handle to use for a loaded dso.
    // (handle=0 is avoided as it means "error" in dlopen)
    nextHandle: 1,
    // handle -> dso [refcount, name, module, global]
    loadedLibs: {},
    // name   -> handle
    loadedLibNames: {},
  },

  // Dynmamic version of shared.py:make_invoke.  This is needed for invokes
  // that originate from side modules since these are not known at JS
  // generation time.
  $createInvokeFunction: function(sig) {
    return function() {
      var sp = stackSave();
      try {
        return dynCall(sig, arguments[0], Array.prototype.slice.call(arguments, 1));
      } catch(e) {
        stackRestore(sp);
        if (e !== e+0 && e !== 'longjmp') throw e;
        _setThrew(1, 0);
      }
    }
  },

  // We support some amount of allocation during startup in the case of
  // dynamic linking, which needs to allocate memory for dynamic libraries that
  // are loaded. That has to happen before the main program can start to run,
  // because the main program needs those linked in before it runs (so we can't
  // use normally malloc from the main program to do these allocations).

  // Allocate memory no even if malloc isn't ready yet.
  $getMemory: function(size) {
    // After the runtime is initialized, we must only use sbrk() normally.
    if (runtimeInitialized)
      return _malloc(size);
    var ret = Module['___heap_base'];
    var end = (ret + size + 15) & -16;
#if ASSERTIONS
    assert(end <= HEAP8.length, 'failure to getMemory - memory growth etc. is not supported there, call malloc/sbrk directly or increase INITIAL_MEMORY');
#endif
    Module['___heap_base'] = end;
    return ret;
  },

  // Loads a side module from binary data
  $loadWebAssemblyModule__deps: ['$createInvokeFunction', '$getMemory'],
  $loadWebAssemblyModule: function(binary, flags) {
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
      // prepare env imports
      var env = asmLibraryArg;
      // TODO: use only __memory_base and __table_base, need to update asm.js backend
      var table = wasmTable;
      var tableBase = table.length;
      var originalTable = table;
      table.grow(tableSize);
      assert(table === originalTable);
      // zero-initialize memory and table
      // The static area consists of explicitly initialized data, followed by zero-initialized data.
      // The latter may need zeroing out if the MAIN_MODULE has already used this memory area before
      // dlopen'ing the SIDE_MODULE.  Since we don't know the size of the explicitly initialized data
      // here, we just zero the whole thing, which is suboptimal, but should at least resolve bugs
      // from uninitialized memory.
      for (var i = memoryBase; i < memoryBase + memorySize; i++) {
        HEAP8[i] = 0;
      }
      for (var i = tableBase; i < tableBase + tableSize; i++) {
        table.set(i, null);
      }

      // We resolve symbols against the global Module but failing that also
      // against the local symbols exported a side module.  This is because
      // a) Module sometime need to import their own symbols
      // b) Symbols from loaded modules are not always added to the global Module.
      var moduleLocal = {};

      var resolveSymbol = function(sym, type, legalized) {
#if WASM_BIGINT
        assert(!legalized);
#else
        if (legalized) {
          sym = 'orig$' + sym;
        }
#endif

        var resolved = Module["asm"][sym];
        if (!resolved) {
          var mangled = asmjsMangle(sym);
          resolved = Module[mangled];
          if (!resolved) {
            resolved = moduleLocal[mangled];
          }
          if (!resolved && sym.startsWith('invoke_')) {
            resolved = createInvokeFunction(sym.split('_')[1]);
          }
#if ASSERTIONS
          assert(resolved, 'missing linked ' + type + ' `' + sym + '`. perhaps a side module was not linked in? if this global was expected to arrive from a system library, try to build the MAIN_MODULE with EMCC_FORCE_STDLIBS=1 in the environment');
#endif
        }
        return resolved;
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
            return obj[prop] = function() {
              return resolveSymbol(name, 'global');
            };
          }
          if (prop.startsWith('fp$')) {
            // the fp$ function returns the address (table index) of the function
            var parts = prop.split('$');
            assert(parts.length == 3)
            var name = parts[1];
            var sig = parts[2];
#if WASM_BIGINT
            var legalized = false;
#else
            var legalized = sig.indexOf('j') >= 0; // check for i64s
#endif
            var fp = 0;
            return obj[prop] = function() {
              if (!fp) {
                var f = resolveSymbol(name, 'function', legalized);
                fp = addFunction(f, sig);
              }
              return fp;
            };
          }
          // otherwise this is regular function import - call it indirectly
          return obj[prop] = function() {
            return resolveSymbol(prop, 'function').apply(null, arguments);
          };
        }
      };
      var proxy = new Proxy(env, proxyHandler);
      var info = {
        global: {
          'NaN': NaN,
          'Infinity': Infinity,
        },
        'global.Math': Math,
        env: proxy,
        {{{ WASI_MODULE_NAME }}}: proxy,
      };
#if ASSERTIONS
      var oldTable = [];
      for (var i = 0; i < tableBase; i++) {
        oldTable.push(table.get(i));
      }
#endif

      function postInstantiation(instance, moduleLocal) {
#if ASSERTIONS
        // the table should be unchanged
        assert(table === originalTable);
        assert(table === wasmTable);
        // the old part of the table should be unchanged
        for (var i = 0; i < tableBase; i++) {
          assert(table.get(i) === oldTable[i], 'old table entries must remain the same');
        }
        // verify that the new table region was filled in
        for (var i = 0; i < tableSize; i++) {
          assert(table.get(tableBase + i) !== undefined, 'table entry was not filled in');
        }
#endif
        var exports = relocateExports(instance.exports, memoryBase, tableBase, moduleLocal);
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
          return postInstantiation(result.instance, moduleLocal);
        });
      } else {
        var instance = new WebAssembly.Instance(new WebAssembly.Module(binary), info);
        return postInstantiation(instance, moduleLocal);
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
  },

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
  $loadDynamicLibrary__deps: ['$LDSO', '$loadWebAssemblyModule'],
  $loadDynamicLibrary: function(lib, flags) {
    if (lib == '__self__' && !LDSO.loadedLibNames[lib]) {
      LDSO.loadedLibs[-1] = {
        refcount: Infinity,   // = nodelete
        name:     '__self__',
        module:   Module['asm'],
        global:   true
      };
      LDSO.loadedLibNames['__self__'] = -1;
    }

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

    // libData <- libFile
    function loadLibData(libFile) {
      // for wasm, we can use fetch for async, but for fs mode we can only imitate it
      if (flags.fs) {
        var libData = flags.fs.readFile(libFile, {encoding: 'binary'});
        if (!(libData instanceof Uint8Array)) {
          libData = new Uint8Array(lib_data);
        }
        return flags.loadAsync ? Promise.resolve(libData) : libData;
      }

      if (flags.loadAsync) {
        return fetchBinary(libFile);
      }
      // load the binary synchronously
      return readBinary(libFile);
    }

    // libModule <- libData
    function createLibModule(libData) {
      return loadWebAssemblyModule(libData, flags)
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

        var module_sym = asmjsMangle(sym);

        if (!Module.hasOwnProperty(module_sym)) {
          Module[module_sym] = libModule[sym];
        }
#if ASSERTIONS == 2
        else {
          var curr = Module[sym], next = libModule[sym];
          // don't warn on functions - might be odr, linkonce_odr, etc.
          if (!(typeof curr === 'function' && typeof next === 'function')) {
            err("warning: symbol '" + sym + "' from '" + lib + "' already exists (duplicate symbol? or weak linking, which isn't supported yet?)"); // + [curr, ' vs ', next]);
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
  },

  $preloadDylibs__deps: ['$loadDynamicLibrary'],
  $preloadDylibs: function() {
    var libs = {{{ JSON.stringify(RUNTIME_LINKED_LIBS) }}};
    if (Module['dynamicLibraries']) {
      libs = libs.concat(Module['dynamicLibraries'])
    }
    if (!libs.length) {
      return;
    }
    // if we can load dynamic libraries synchronously, do so, otherwise, preload
#if WASM
    if (!readBinary) {
      // we can't read binary data synchronously, so preload
      addRunDependency('preloadDylibs');
      Promise.all(libs.map(function(lib) {
        return loadDynamicLibrary(lib, {loadAsync: true, global: true, nodelete: true});
      })).then(function() {
        // we got them all, wonderful
        removeRunDependency('preloadDylibs');
      });
      return;
    }
#endif
    libs.forEach(function(lib) {
      // libraries linked to main never go away
      loadDynamicLibrary(lib, {global: true, nodelete: true});
    });
  },

  // void* dlopen(const char* filename, int flag);
  dlopen__deps: ['$DLFCN', '$FS', '$ENV'],
  dlopen__proxy: 'sync',
  dlopen__sig: 'iii',
  dlopen: function(filenameAddr, flag) {
    // void *dlopen(const char *file, int mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlopen.html
    var searchpaths = [];
    var filename;
    if (filenameAddr === 0) {
      filename = '__self__';
    } else {
      filename = UTF8ToString(filenameAddr);

      var isValidFile = function (filename) {
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
    }

    // We don't care about RTLD_NOW and RTLD_LAZY.
    var flags = {
      global:   Boolean(flag & 256),  // RTLD_GLOBAL
      nodelete: Boolean(flag & 4096), // RTLD_NODELETE

      fs: FS, // load libraries from provided filesystem
    }

    try {
      return loadDynamicLibrary(filename, flags)
    } catch (e) {
#if ASSERTIONS
      err('Error in loading dynamic library ' + filename + ": " + e);
#endif
      DLFCN.errorMsg = 'Could not load dynamic lib: ' + filename + '\n' + e;
      return 0;
    }
  },

  // int dlclose(void* handle);
  dlclose__deps: ['$DLFCN'],
  dlclose__proxy: 'sync',
  dlclose__sig: 'ii',
  dlclose: function(handle) {
    // int dlclose(void *handle);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlclose.html
    var lib = LDSO.loadedLibs[handle];
    if (!lib) {
      DLFCN.errorMsg = 'Tried to dlclose() unopened handle: ' + handle;
      return 1;
    }
    if (--lib.refcount == 0) {
      delete LDSO.loadedLibNames[lib.name];
      delete LDSO.loadedLibs[handle];
    }
    return 0;
  },

  // void* dlsym(void* handle, const char* symbol);
  dlsym__deps: ['$DLFCN'],
  dlsym__proxy: 'sync',
  dlsym__sig: 'iii',
  dlsym: function(handle, symbol) {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = UTF8ToString(symbol);

    var lib = LDSO.loadedLibs[handle];
    if (!lib) {
      DLFCN.errorMsg = 'Tried to dlsym() from an unopened handle: ' + handle;
      return 0;
    }

    if (!lib.module.hasOwnProperty(symbol)) {
      DLFCN.errorMsg = ('Tried to lookup unknown symbol "' + symbol +
                             '" in dynamic lib: ' + lib.name);
      return 0;
    }

    var result = lib.module[symbol];
    if (typeof result !== 'function') {
      return result;
    }

#if WASM && EMULATE_FUNCTION_POINTER_CASTS
    // for wasm with emulated function pointers, the i64 ABI is used for all
    // function calls, so we can't just call addFunction on something JS
    // can call (which does not use that ABI), as the function pointer would
    // not be usable from wasm. instead, the wasm has exported function pointers
    // for everything we need, with prefix fp$, use those
    result = lib.module['fp$' + symbol];
    if (typeof result === 'object') {
      // a breaking change in the wasm spec, globals are now objects
      // https://github.com/WebAssembly/mutable-global/issues/1
      result = result.value;
    }
#if ASSERTIONS
    assert(typeof result === 'number', 'could not find function pointer for ' + symbol);
#endif // ASSERTIONS
    return result;
#else // WASM && EMULATE_FUNCTION_POINTER_CASTS

#if WASM
    // Insert the function into the wasm table.  Since we know the function
    // comes directly from the loaded wasm module we can insert it directly
    // into the table, avoiding any JS interaction.
    return addFunctionWasm(result);
#else
    // convert the exported function into a function pointer using our generic
    // JS mechanism.
    return addFunction(result);
#endif // WASM
#endif // WASM && EMULATE_FUNCTION_POINTER_CASTS
  },

  // char* dlerror(void);
  dlerror__deps: ['$DLFCN', '$stringToNewUTF8'],
  dlerror__proxy: 'sync',
  dlerror__sig: 'i',
  dlerror: function() {
    // char *dlerror(void);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlerror.html
    if (DLFCN.errorMsg === null) {
      return 0;
    }
    if (DLFCN.error) _free(DLFCN.error);
    DLFCN.error = stringToNewUTF8(DLFCN.errorMsg);
    DLFCN.errorMsg = null;
    return DLFCN.error;
  },

  dladdr__deps: ['$stringToNewUTF8', '$getExecutableName'],
  dladdr__proxy: 'sync',
  dladdr__sig: 'iii',
  dladdr: function(addr, info) {
    // report all function pointers as coming from this program itself XXX not really correct in any way
    var fname = stringToNewUTF8(getExecutableName()); // XXX leak
    {{{ makeSetValue('info', 0, 'fname', 'i32') }}};
    {{{ makeSetValue('info', Runtime.QUANTUM_SIZE, '0', 'i32') }}};
    {{{ makeSetValue('info', Runtime.QUANTUM_SIZE*2, '0', 'i32') }}};
    {{{ makeSetValue('info', Runtime.QUANTUM_SIZE*3, '0', 'i32') }}};
    return 1;
  },
#endif // MAIN_MODULE != 0
};

mergeInto(LibraryManager.library, LibraryDylink);
