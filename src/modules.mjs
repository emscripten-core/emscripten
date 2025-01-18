/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

import * as path from 'node:path';
import * as fs from 'node:fs';
import {fileURLToPath} from 'node:url';

import {
  isDecorator,
  assert,
  isJsOnlySymbol,
  error,
  read,
  warn,
  setCurrentFile,
  printErr,
  addToCompileTimeContext,
  runInMacroContext,
  mergeInto,
  localFile,
} from './utility.mjs';
import {preprocess, processMacros} from './parseTools.mjs';

// Various namespace-like modules

// List of symbols that were added from the library.
export const librarySymbols = [];

const srcDir = fileURLToPath(new URL('.', import.meta.url));
const systemLibdir = path.join(srcDir, 'lib');

function isBeneath(childPath, parentPath) {
  const relativePath = path.relative(parentPath, childPath);
  return !relativePath.startsWith('..') && !path.isAbsolute(relativePath);
}

export const LibraryManager = {
  library: {},
  // The JS and JS docs of each library definition indexed my mangled name.
  libraryDefinitions: {},
  structs: {},
  loaded: false,
  libraries: [],

  has(name) {
    if (!path.isAbsolute(name)) {
      // Our libraries used to be called `library_xxx.js` rather than
      // `lib_xx.js`.  In case we have external code using this function
      // we check for the old form too.
      if (name.startsWith('library_')) {
        name = name.replace('library_', 'lib');
      }
      name = path.join(systemLibdir, name);
    }
    return this.libraries.includes(name);
  },

  load() {
    assert(!this.loaded);
    this.loaded = true;

    // Core system libraries (always linked against)
    let libraries = [
      'libint53.js',
      'libcore.js',
      'libsigs.js',
      'libccall.js',
      'libaddfunction.js',
      'libformatString.js',
      'libgetvalue.js',
      'libmath.js',
      'libpath.js',
      'libstrings.js',
      'libhtml5.js',
      'libstack_trace.js',
      'libwasi.js',
      'libeventloop.js',
      'libpromise.js',
    ];

    if (LINK_AS_CXX) {
      if (DISABLE_EXCEPTION_THROWING && !WASM_EXCEPTIONS) {
        libraries.push('libexceptions_stub.js');
      } else {
        libraries.push('libexceptions.js');
      }
    }

    if (!MINIMAL_RUNTIME) {
      libraries.push('libbrowser.js');
      libraries.push('libwget.js');
    }

    if (!STANDALONE_WASM) {
      libraries.push('libtime.js');
    }

    if (EMSCRIPTEN_TRACING) {
      libraries.push('libmemoryprofiler.js');
    }

    if (AUTODEBUG) {
      libraries.push('libautodebug.js');
    }

    if (!WASMFS) {
      libraries.push('libsyscall.js');
    }

    if (RELOCATABLE) {
      libraries.push('libdylink.js');
    }

    if (FILESYSTEM) {
      libraries.push('libfs_shared.js');
      if (WASMFS) {
        libraries.push(
          'libwasmfs.js',
          'libwasmfs_js_file.js',
          'libwasmfs_jsimpl.js',
          'libwasmfs_fetch.js',
          'libwasmfs_node.js',
          'libwasmfs_opfs.js',
        );
      } else {
        // Core filesystem libraries (always linked against, unless -sFILESYSTEM=0 is specified)
        libraries.push(
          'libfs.js',
          'libmemfs.js',
          'libtty.js',
          'libpipefs.js', // ok to include it by default since it's only used if the syscall is used
          'libsockfs.js', // ok to include it by default since it's only used if the syscall is used
        );

        if (NODERAWFS) {
          // NODERAWFS requires NODEFS
          if (!JS_LIBRARIES.includes('libnodefs.js')) {
            libraries.push('libnodefs.js');
          }
          libraries.push('libnoderawfs.js');
          // NODERAWFS overwrites libpath.js
          libraries.push('libnodepath.js');
        }
      }
    }

    // Additional JS libraries (without AUTO_JS_LIBRARIES, link to these explicitly via -lxxx.js)
    if (AUTO_JS_LIBRARIES) {
      libraries.push(
        'libwebgl.js',
        'libhtml5_webgl.js',
        'libopenal.js',
        'libglut.js',
        'libxlib.js',
        'libegl.js',
        'libuuid.js',
        'libglew.js',
        'libidbstore.js',
        'libasync.js',
      );
      if (USE_SDL != 2) {
        libraries.push('libsdl.js');
      }
    } else {
      if (ASYNCIFY) {
        libraries.push('libasync.js');
      }
      if (USE_SDL == 1) {
        libraries.push('libsdl.js');
      }
      if (USE_SDL == 2) {
        libraries.push('libegl.js', 'libwebgl.js', 'libhtml5_webgl.js');
      }
    }

    if (USE_GLFW) {
      libraries.push('libglfw.js');
    }

    if (LZ4) {
      libraries.push('liblz4.js');
    }

    if (SHARED_MEMORY) {
      libraries.push('libatomic.js');
    }

    if (MAX_WEBGL_VERSION >= 2) {
      // libwebgl2.js must be included only after libwebgl.js, so if we are
      // about to include libwebgl2.js, first squeeze in libwebgl.js.
      libraries.push('libwebgl.js');
      libraries.push('libwebgl2.js');
    }

    if (GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING) {
      libraries.push('libc_preprocessor.js');
    }

    if (LEGACY_GL_EMULATION) {
      libraries.push('libglemu.js');
    }

    if (USE_WEBGPU) {
      libraries.push('libwebgpu.js');
      libraries.push('libhtml5_webgpu.js');
    }

    if (!STRICT) {
      libraries.push('liblegacy.js');
    }

    if (BOOTSTRAPPING_STRUCT_INFO) {
      libraries = ['libbootstrap.js', 'libformatString.js', 'libstrings.js', 'libint53.js'];
    }

    if (SUPPORT_BIG_ENDIAN) {
      libraries.push('liblittle_endian_heap.js');
    }

    // Add all user specified JS library files to the link.
    // These must be added last after all Emscripten-provided system libraries
    // above, so that users can override built-in JS library symbols in their
    // own code.
    libraries.push(...JS_LIBRARIES);

    // Resolve all filenames to absolute paths
    libraries = libraries.map((filename) => {
      if (!path.isAbsolute(filename) && fs.existsSync(path.join(systemLibdir, filename))) {
        filename = path.join(systemLibdir, filename);
      }
      return path.resolve(filename);
    });

    // Deduplicate libraries to avoid processing any library file multiple times
    libraries = libraries.filter((item, pos) => libraries.indexOf(item) == pos);

    // Save the list for has() queries later.
    this.libraries = libraries;

    for (let filename of libraries) {
      const isUserLibrary = !isBeneath(filename, systemLibdir);

      if (VERBOSE) {
        if (isUserLibrary) {
          printErr('processing user library: ' + filename);
        } else {
          printErr('processing system library: ' + filename);
        }
      }
      let origLibrary = undefined;
      let processed = undefined;
      // When we parse user libraries also set `__user` attribute
      // on each element so that we can distinguish them later.
      if (isUserLibrary) {
        origLibrary = this.library;
        this.library = new Proxy(this.library, {
          set(target, prop, value) {
            target[prop] = value;
            if (!isDecorator(prop)) {
              target[prop + '__user'] = true;
            }
            return true;
          },
        });
      }
      const oldFile = setCurrentFile(filename);
      try {
        processed = processMacros(preprocess(filename), filename);
        runInMacroContext(processed, {filename: filename.replace(/\.\w+$/, '.preprocessed$&')});
      } catch (e) {
        error(`failure to execute js library "${filename}":`);
        if (VERBOSE) {
          const orig = read(filename);
          if (processed) {
            error(
              `preprocessed source (you can run a js engine on this to get a clearer error message sometimes):\n=============\n${processed}\n=============`,
            );
          } else {
            error(`original source:\n=============\n${orig}\n=============`);
          }
        } else {
          error('use -sVERBOSE to see more details');
        }
        throw e;
      } finally {
        setCurrentFile(oldFile);
        if (origLibrary) {
          this.library = origLibrary;
        }
      }
    }
  },
};

// options is optional input object containing mergeInto params
// currently, it can contain
//
// key: noOverride, value: true
// if it is set, it prevents symbol redefinition and shows error
// in case of redefinition
//
// key: checkSig, value: true
// if it is set, __sig is checked for functions and error is reported
// if <function name>__sig is missing
function addToLibrary(obj, options = null) {
  mergeInto(LibraryManager.library, obj, options);
}

let structs = {};
let defines = {};

/**
 * Read JSON file containing struct and macro/define information
 * that can then be used in JavaScript via macros.
 */
function loadStructInfo(filename) {
  const temp = JSON.parse(read(filename));
  Object.assign(structs, temp.structs);
  Object.assign(defines, temp.defines);
}

if (!BOOTSTRAPPING_STRUCT_INFO) {
  // Load struct and define information.
  if (MEMORY64) {
    loadStructInfo(localFile('struct_info_generated_wasm64.json'));
  } else {
    loadStructInfo(localFile('struct_info_generated.json'));
  }
}

// Use proxy objects for C_DEFINES and C_STRUCTS so that we can give useful
// error messages.
const C_STRUCTS = new Proxy(structs, {
  get(target, prop, receiver) {
    if (!(prop in target)) {
      throw new Error(
        `Missing C struct ${prop}! If you just added it to struct_info.json, you need to run ./tools/maint/gen_struct_info.py (then run a second time with --wasm64)`,
      );
    }
    return target[prop];
  },
});

const C_DEFINES = new Proxy(defines, {
  get(target, prop, receiver) {
    if (!(prop in target)) {
      throw new Error(
        `Missing C define ${prop}! If you just added it to struct_info.json, you need to run ./tools/maint/gen_struct_info.py (then run a second time with --wasm64)`,
      );
    }
    return target[prop];
  },
});

// shorter alias for C_DEFINES
const cDefs = C_DEFINES;

// Legacy function that existed solely to give error message.  These are now
// provided by the cDefs proxy object above.
function cDefine(key) {
  return cDefs[key];
}

function isInternalSymbol(ident) {
  return ident + '__internal' in LibraryManager.library;
}

function getUnusedLibrarySymbols() {
  const librarySymbolSet = new Set(librarySymbols);
  const missingSyms = new Set();
  for (const [ident, value] of Object.entries(LibraryManager.library)) {
    if (typeof value === 'function' || typeof value === 'number') {
      if (isJsOnlySymbol(ident) && !isDecorator(ident) && !isInternalSymbol(ident)) {
        const name = ident.substr(1);
        if (!librarySymbolSet.has(name)) {
          missingSyms.add(name);
        }
      }
    }
  }
  return missingSyms;
}

// When running with ASSERTIONS enabled we create stubs for each library
// function that that was not included in the build.  This gives useful errors
// when library dependencies are missing from `__deps` or depended on without
// being added to DEFAULT_LIBRARY_FUNCS_TO_INCLUDE
// TODO(sbc): These errors could potentially be generated at build time via
// some kind of acorn pass that searched for uses of these missing symbols.
function addMissingLibraryStubs(unusedLibSymbols) {
  let rtn = '';
  rtn += 'var missingLibrarySymbols = [\n';
  for (const sym of unusedLibSymbols) {
    rtn += `  '${sym}',\n`;
  }
  rtn += '];\n';
  rtn += 'missingLibrarySymbols.forEach(missingLibrarySymbol)\n';
  return rtn;
}

// export parts of the JS runtime that the user asked for
function exportRuntime() {
  // optionally export something.
  function maybeExport(name) {
    // If requested to be exported, export it.  HEAP objects are exported
    // separately in updateMemoryViews
    if (EXPORTED_RUNTIME_METHODS.has(name) && !name.startsWith('HEAP')) {
      if (MODULARIZE === 'instance') {
        return `__exp_${name} = ${name};`;
      }
      return `Module['${name}'] = ${name};`;
    }
  }

  // All possible runtime elements that can be exported
  let runtimeElements = [
    'run',
    'addOnPreRun',
    'addOnInit',
    'addOnPreMain',
    'addOnExit',
    'addOnPostRun',
    'addRunDependency',
    'removeRunDependency',
    'out',
    'err',
    'callMain',
    'abort',
    'wasmMemory',
    'wasmExports',
    'HEAPF32',
    'HEAPF64',
    'HEAP_DATA_VIEW',
    'HEAP8',
    'HEAPU8',
    'HEAP16',
    'HEAPU16',
    'HEAP32',
    'HEAPU32',
    'HEAP64',
    'HEAPU64',
  ];

  if (PTHREADS && ALLOW_MEMORY_GROWTH) {
    runtimeElements.push(
      'GROWABLE_HEAP_I8',
      'GROWABLE_HEAP_U8',
      'GROWABLE_HEAP_I16',
      'GROWABLE_HEAP_U16',
      'GROWABLE_HEAP_I32',
      'GROWABLE_HEAP_U32',
      'GROWABLE_HEAP_F32',
      'GROWABLE_HEAP_F64',
    );
  }
  if (USE_OFFSET_CONVERTER) {
    runtimeElements.push('WasmOffsetConverter');
  }

  if (LOAD_SOURCE_MAP) {
    runtimeElements.push('WasmSourceMap');
  }

  if (STACK_OVERFLOW_CHECK) {
    runtimeElements.push('writeStackCookie');
    runtimeElements.push('checkStackCookie');
  }

  if (RETAIN_COMPILER_SETTINGS) {
    runtimeElements.push('getCompilerSetting');
  }

  if (RUNTIME_DEBUG) {
    runtimeElements.push('prettyPrint');
  }

  // dynCall_* methods are not hardcoded here, as they
  // depend on the file being compiled. check for them
  // and add them.
  for (const name of EXPORTED_RUNTIME_METHODS) {
    if (/^dynCall_/.test(name)) {
      // a specific dynCall; add to the list
      runtimeElements.push(name);
    }
  }

  // Add JS library elements such as FS, GL, ENV, etc. These are prefixed with
  // '$ which indicates they are JS methods.
  let runtimeElementsSet = new Set(runtimeElements);
  for (const ident of Object.keys(LibraryManager.library)) {
    if (isJsOnlySymbol(ident) && !isDecorator(ident) && !isInternalSymbol(ident)) {
      const jsname = ident.substr(1);
      // Note that this assertion may be hit when a function is moved into the
      // JS library. In that case the function should be removed from the list
      // of runtime elements above.
      assert(!runtimeElementsSet.has(jsname), 'runtimeElements contains library symbol: ' + ident);
      runtimeElements.push(jsname);
    }
  }

  // check all exported things exist, warn about typos
  runtimeElementsSet = new Set(runtimeElements);
  for (const name of EXPORTED_RUNTIME_METHODS) {
    if (!runtimeElementsSet.has(name)) {
      warn(`invalid item in EXPORTED_RUNTIME_METHODS: ${name}`);
    }
  }

  const exports = runtimeElements.map(maybeExport);
  const results = exports.filter((name) => name);

  if (ASSERTIONS && !EXPORT_ALL) {
    // in ASSERTIONS mode we show a useful error if it is used without being
    // exported.  See `unexportedRuntimeSymbol` in runtime_debug.js.
    const unusedLibSymbols = getUnusedLibrarySymbols();
    if (unusedLibSymbols.size) {
      results.push(addMissingLibraryStubs(unusedLibSymbols));
    }

    const unexported = [];
    for (const name of runtimeElements) {
      if (!EXPORTED_RUNTIME_METHODS.has(name) && !unusedLibSymbols.has(name)) {
        unexported.push(name);
      }
    }

    if (unexported.length || unusedLibSymbols.size) {
      let unexportedStubs = 'var unexportedSymbols = [\n';
      for (const sym of unexported) {
        unexportedStubs += `  '${sym}',\n`;
      }
      unexportedStubs += '];\n';
      unexportedStubs += 'unexportedSymbols.forEach(unexportedRuntimeSymbol);\n';
      results.push(unexportedStubs);
    }
  }

  return results.join('\n') + '\n';
}

addToCompileTimeContext({
  exportRuntime,
  loadStructInfo,
  LibraryManager,
  librarySymbols,
  addToLibrary,
  cDefs,
  cDefine,
  C_STRUCTS,
  C_DEFINES,
});
