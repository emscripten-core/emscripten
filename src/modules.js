/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// "use strict";

// Various namespace-like modules

// Constructs an array ['a0', 'a1', 'a2', ..., 'a(n-1)']
function genArgSequence(n) {
  const args = [];
  for (let i = 0; i < n; ++i) {
    args.push('a' + i);
  }
  return args;
}

// List of functions that were added from the library.
global.libraryFunctions = [];

global.LibraryManager = {
  library: null,
  structs: {},
  loaded: false,
  libraries: [],

  has: function(name) {
    return this.libraries.includes(name);
  },

  load: function() {
    if (this.library) return;

    // Core system libraries (always linked against)
    let libraries = [
      'library.js',
      'library_formatString.js',
      'library_math.js',
      'library_path.js',
      'library_syscall.js',
      'library_html5.js',
      'library_stack_trace.js',
      'library_wasi.js',
      'library_int53.js',
      'library_dylink.js',
      'library_eventloop.js',
    ];

    if (LINK_AS_CXX && !EXCEPTION_HANDLING) {
      if (DISABLE_EXCEPTION_THROWING) {
        libraries.push('library_exceptions_stub.js');
      } else {
        libraries.push('library_exceptions.js');
      }
    }

    if (MINIMAL_RUNTIME) {
      // Classic runtime brings in string-related functions in the runtime preamble, by including
      // runtime_strings_extra.js that contain the same contents as library_strings.js does. In
      // MINIMAL_RUNTIME those string functions are available as JS library functions instead from
      // library_strings.js, to avoid unconditionally bringing in extra code to the build.
      libraries.push('library_strings.js');
    } else {
      libraries.push('library_browser.js');
      libraries.push('library_wget.js');
    }

    if (USE_PTHREADS) { // TODO: Currently WebGL proxying makes pthreads library depend on WebGL.
      libraries.push('library_webgl.js');
      libraries.push('library_html5_webgl.js');
    }

    if (FILESYSTEM) {
      // Core filesystem libraries (always linked against, unless -s FILESYSTEM=0 is specified)
      libraries = libraries.concat([
        'library_fs.js',
        'library_memfs.js',
        'library_tty.js',
        'library_pipefs.js', // ok to include it by default since it's only used if the syscall is used
        'library_sockfs.js', // ok to include it by default since it's only used if the syscall is used
      ]);

      if (NODERAWFS) {
        // NODERAWFS requires NODEFS
        if (!JS_LIBRARIES.includes('library_nodefs.js')) {
          libraries.push('library_nodefs.js');
        }
        libraries.push('library_noderawfs.js');
      }
    } else if (WASMFS) {
      libraries.push('library_wasmfs.js');
    }

    // Additional JS libraries (without AUTO_JS_LIBRARIES, link to these explicitly via -lxxx.js)
    if (AUTO_JS_LIBRARIES) {
      libraries = libraries.concat([
        'library_webgl.js',
        'library_html5_webgl.js',
        'library_openal.js',
        'library_sdl.js',
        'library_glut.js',
        'library_xlib.js',
        'library_egl.js',
        'library_glfw.js',
        'library_uuid.js',
        'library_glew.js',
        'library_idbstore.js',
        'library_async.js',
      ]);
    } else {
      if (ASYNCIFY) {
        libraries.push('library_async.js');
      }
      if (USE_SDL == 1) {
        libraries.push('library_sdl.js');
      }
      if (USE_SDL == 2) {
        libraries.push('library_egl.js', 'library_webgl.js', 'library_html5_webgl.js');
      }
    }

    if (LZ4) {
      libraries.push('library_lz4.js');
    }

    if (MAX_WEBGL_VERSION >= 2) {
      // library_webgl2.js must be included only after library_webgl.js, so if we are
      // about to include library_webgl2.js, first squeeze in library_webgl.js.
      libraries.push('library_webgl.js');
      libraries.push('library_webgl2.js');
    }

    if (GL_EXPLICIT_UNIFORM_LOCATION || GL_EXPLICIT_UNIFORM_BINDING) {
      libraries.push('library_c_preprocessor.js');
    }

    if (LEGACY_GL_EMULATION) {
      libraries.push('library_glemu.js');
    }

    if (USE_WEBGPU) {
      libraries.push('library_webgpu.js');
      libraries.push('library_html5_webgpu.js');
    }

    if (BOOTSTRAPPING_STRUCT_INFO) {
      libraries = [
        'library_bootstrap.js',
        'library_formatString.js',
        'library_int53.js',
      ];
    }

    if (SUPPORT_BIG_ENDIAN) {
      libraries.push('library_little_endian_heap.js');
    }

    // Add all user specified --js-library files to the link.
    // These must be added last after all Emscripten-provided system libraries
    // above, so that users can override built-in JS library symbols in their
    // own code.
    libraries = libraries.concat(JS_LIBRARIES);

    // Deduplicate libraries to avoid processing any library file multiple times
    libraries = libraries.filter((item, pos) => libraries.indexOf(item) == pos);

    // Save the list for has() queries later.
    this.libraries = libraries;

    for (const filename of libraries) {
      const isUserLibrary = nodePath.isAbsolute(filename);
      if (VERBOSE) {
        if (isUserLibrary) {
          printErr('processing user library: ' + filename);
        } else {
          printErr('processing system library: ' + filename);
        }
      }
      const src = read(filename);
      let origLibrary = undefined;
      let processed = undefined;
      // When we parse user libraries also set `__user` attribute
      // on each element so that we can distinguish them later.
      if (isUserLibrary) {
        origLibrary = this.library;
        this.library = new Proxy(this.library, {
          set: (target, prop, value) => {
            target[prop] = value;
            if (!isJsLibraryConfigIdentifier(prop)) {
              target[prop + '__user'] = true;
            }
          },
        });
      }
      try {
        processed = processMacros(preprocess(src, filename));
        eval(processed);
      } catch (e) {
        const details = [e, e.lineNumber ? `line number: ${e.lineNumber}` : ''];
        if (VERBOSE) {
          details.push((e.stack || '').toString().replace('Object.<anonymous>', filename));
        }
        if (processed) {
          error(`failure to execute js library "${filename}": ${details}`);
          if (VERBOSE) {
            error(`preprocessed source (you can run a js engine on this to get a clearer error message sometimes):\n=============\n${processed}\n=============`);
          } else {
            error('use -s VERBOSE to see more details');
          }
        } else {
          error(`failure to process js library "${filename}": ${details}`);
          if (VERBOSE) {
            error(`original source:\n=============\n${src}\n=============`);
          } else {
            error('use -s VERBOSE to see more details');
          }
        }
        throw e;
      } finally {
        if (origLibrary) {
          this.library = origLibrary;
        }
      }
    }

    // apply synonyms. these are typically not speed-sensitive, and doing it
    // this way makes it possible to not include hacks in the compiler
    // (and makes it simpler to switch between SDL versions, fastcomp and non-fastcomp, etc.).
    const lib = this.library;
    libloop: for (const x in lib) {
      if (!Object.prototype.hasOwnProperty.call(lib, x)) {
        continue;
      }
      if (isJsLibraryConfigIdentifier(x)) {
        const index = x.lastIndexOf('__');
        const basename = x.slice(0, index);
        if (!(basename in lib)) {
          error(`Missing library element '${basename}' for library config '${x}'`);
        }
        continue;
      }
      if (typeof lib[x] === 'string') {
        let target = x;
        while (typeof lib[target] === 'string') {
          // ignore code and variable assignments, aliases are just simple names
          if (lib[target].search(/[=({; ]/) >= 0) continue libloop;
          target = lib[target];
        }
        if (!isNaN(target)) continue; // This is a number, and so cannot be an alias target.
        if (typeof lib[target] === 'undefined' || typeof lib[target] === 'function') {
          // When functions are aliased, a signature for the function must be
          // provided so that an efficient form of forwarding can be
          // implemented.
          function testStringType(sig) {
            if (typeof lib[sig] !== 'undefined' && typeof typeof lib[sig] !== 'string') {
              error(`${sig} should be a string! (was ${typeof lib[sig]})`);
            }
          }
          const aliasSig = x + '__sig';
          const targetSig = target + '__sig';
          testStringType(aliasSig);
          testStringType(targetSig);
          if (typeof lib[aliasSig] === 'string' && typeof lib[targetSig] === 'string' && lib[aliasSig] != lib[targetSig]) {
            error(`${aliasSig} (${lib[aliasSig]}) differs from ${targetSig} (${lib[targetSig]})`);
          }

          const sig = lib[aliasSig] || lib[targetSig];
          if (typeof sig !== 'string') {
            error(`Function ${x} aliases to target function ${target}, but neither the alias or the target provide a signature. Please add a ${targetSig}: 'vifj...' annotation or a ${aliasSig}: 'vifj...' annotation to describe the type of function forwarding that is needed!`);
          }

          // If only one of the target or the alias specifies a sig then copy
          // this signature to the other.
          if (!lib[aliasSig]) {
            lib[aliasSig] = lib[targetSig];
          } else if (!lib[targetSig]) {
            lib[targetSig] = lib[aliasSig];
          }

          if (typeof lib[target] !== 'function') {
            error(`no alias found for ${x}`);
          }

          const argCount = sig.length - 1;
          if (argCount !== lib[target].length) {
            error(`incorrect number of arguments in signature of ${x} (declared: ${argCount}, expected: ${lib[target].length})`);
          }
          const ret = sig == 'v' ? '' : 'return ';
          const args = genArgSequence(argCount).join(',');
          lib[x] = new Function(args, `${ret}_${target}(${args});`);

          if (!lib[x + '__deps']) lib[x + '__deps'] = [];
          lib[x + '__deps'].push(target);
        }
      }
    }

    this.loaded = true;
  },
};

if (!BOOTSTRAPPING_STRUCT_INFO) {
  // Load struct and define information.
  const temp = JSON.parse(read(STRUCT_INFO));
  C_STRUCTS = temp.structs;
  C_DEFINES = temp.defines;
} else {
  C_STRUCTS = {};
  C_DEFINES = {};
}

// Safe way to access a C define. We check that we don't add library functions with missing defines.
function cDefine(key) {
  if (key in C_DEFINES) return C_DEFINES[key];
  throw new Error(`Missing C define ${key}! If you just added it to struct_info.json, you need to ./emcc --clear-cache`);
}

function isFSPrefixed(name) {
  return name.length > 3 && name[0] === 'F' && name[1] === 'S' && name[2] === '_';
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_createLazyFile' ||
         name === 'FS_createDevice' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         name === 'removeRunDependency';
}

// export parts of the JS runtime that the user asked for
function exportRuntime() {
  const EXPORTED_RUNTIME_METHODS_SET = new Set(EXPORTED_RUNTIME_METHODS);

  // optionally export something.
  // in ASSERTIONS mode we show a useful error if it is used without
  // being exported. how we show the message depends on whether it's
  // a function (almost all of them) or a number.
  function maybeExport(name, isNumber) {
    // if requested to be exported, export it
    if (EXPORTED_RUNTIME_METHODS_SET.has(name)) {
      let exported = name;
      // the exported name may differ from the internal name
      if (isFSPrefixed(exported)) {
        // this is a filesystem value, FS.x exported as FS_x
        exported = 'FS.' + exported.substr(3);
      } else if (exported === 'print') {
        exported = 'out';
      } else if (exported === 'printErr') {
        exported = 'err';
      }
      return `Module["${name}"] = ${exported};`;
    }
    // do not export it. but if ASSERTIONS, emit a
    // stub with an error, so the user gets a message
    // if it is used, that they should export it
    if (ASSERTIONS) {
      // check if it already exists, to support EXPORT_ALL and other cases
      // (we could optimize this, but in ASSERTIONS mode code size doesn't
      // matter anyhow)
      let extra = '';
      if (isExportedByForceFilesystem(name)) {
        extra = '. Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you';
      }
      if (!isNumber) {
        return `if (!Object.getOwnPropertyDescriptor(Module, "${name}")) Module["${name}"] = function() { abort("'${name}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)${extra}") };`;
      } else {
        return `if (!Object.getOwnPropertyDescriptor(Module, "${name}")) Object.defineProperty(Module, "${name}", { configurable: true, get: function() { abort("'${name}' was not exported. add it to EXPORTED_RUNTIME_METHODS (see the FAQ)${extra}") } });`;
      }
    }
    return '';
  }

  function maybeExportNumber(name) {
    return maybeExport(name, true);
  }

  // All possible runtime elements that can be exported
  let runtimeElements = [
    'intArrayFromString',
    'intArrayToString',
    'ccall',
    'cwrap',
    'setValue',
    'getValue',
    'allocate',
    'UTF8ArrayToString',
    'UTF8ToString',
    'stringToUTF8Array',
    'stringToUTF8',
    'lengthBytesUTF8',
    'stackTrace',
    'addOnPreRun',
    'addOnInit',
    'addOnPreMain',
    'addOnExit',
    'addOnPostRun',
    'writeStringToMemory',
    'writeArrayToMemory',
    'writeAsciiToMemory',
    'addRunDependency',
    'removeRunDependency',
    'FS_createFolder',
    'FS_createPath',
    'FS_createDataFile',
    'FS_createPreloadedFile',
    'FS_createLazyFile',
    'FS_createLink',
    'FS_createDevice',
    'FS_unlink',
    'getLEB',
    'getFunctionTables',
    'alignFunctionTables',
    'registerFunctions',
    'addFunction',
    'removeFunction',
    'getFuncWrapper',
    'prettyPrint',
    'dynCall',
    'getCompilerSetting',
    'print',
    'printErr',
    'getTempRet0',
    'setTempRet0',
    'callMain',
    'abort',
    'keepRuntimeAlive',
  ];

  if (USE_OFFSET_CONVERTER) {
    runtimeElements.push('WasmOffsetConverter');
  }

  if (LOAD_SOURCE_MAP) {
    runtimeElements.push('WasmSourceMap');
  }

  // Add JS library elements such as FS, GL, ENV, etc. These are prefixed with
  // '$ which indicates they are JS methods.
  for (const ident in LibraryManager.library) {
    if (ident[0] === '$' && !isJsLibraryConfigIdentifier(ident)) {
      runtimeElements.push(ident.substr(1));
    }
  }

  if (!MINIMAL_RUNTIME) {
    // MINIMAL_RUNTIME has moved these functions to library_strings.js
    runtimeElements = runtimeElements.concat([
      'warnOnce',
      'stackSave',
      'stackRestore',
      'stackAlloc',
      'AsciiToString',
      'stringToAscii',
      'UTF16ToString',
      'stringToUTF16',
      'lengthBytesUTF16',
      'UTF32ToString',
      'stringToUTF32',
      'lengthBytesUTF32',
      'allocateUTF8',
      'allocateUTF8OnStack',
    ]);
  }

  if (STACK_OVERFLOW_CHECK) {
    runtimeElements.push('writeStackCookie');
    runtimeElements.push('checkStackCookie');
  }

  if (USE_PTHREADS) {
    // In pthreads mode, the following functions always need to be exported to
    // Module for closure compiler, and also for MODULARIZE (so worker.js can
    // access them).
    const threadExports = ['PThread', 'wasmMemory'];
    if (!MINIMAL_RUNTIME) {
      threadExports.push('ExitStatus');
    }

    threadExports.forEach((x) => {
      EXPORTED_RUNTIME_METHODS_SET.add(x);
      runtimeElements.push(x);
    });
  }

  if (SUPPORT_BASE64_EMBEDDING) {
    runtimeElements.push('intArrayFromBase64');
    runtimeElements.push('tryParseAsDataURI');
  }
  // dynCall_* methods are not hardcoded here, as they
  // depend on the file being compiled. check for them
  // and add them.
  for (const name of EXPORTED_RUNTIME_METHODS_SET) {
    if (/^dynCall_/.test(name)) {
      // a specific dynCall; add to the list
      runtimeElements.push(name);
    }
  }
  const runtimeNumbers = [
    'ALLOC_NORMAL',
    'ALLOC_STACK',
  ];
  if (ASSERTIONS) {
    // check all exported things exist, warn about typos
    for (const name of EXPORTED_RUNTIME_METHODS_SET) {
      if (!runtimeElements.includes(name) && !runtimeNumbers.includes(name)) {
        printErr(`warning: invalid item (maybe a typo?) in EXPORTED_RUNTIME_METHODS: ${name}`);
      }
    }
  }
  let exports = runtimeElements.map((name) => maybeExport(name));
  exports = exports.concat(runtimeNumbers.map((name) => maybeExportNumber(name)));
  exports = exports.filter((name) => name != '');
  return exports.join('\n');
}
