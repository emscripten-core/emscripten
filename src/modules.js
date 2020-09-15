/**
 * @license
 * Copyright 2011 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//"use strict";

// Various namespace-like modules

var STACK_ALIGN = 16;

var Variables = {
  globals: {},
  indexedGlobals: {}, // for indexed globals, ident ==> index
  // Used in calculation of indexed globals
  nextIndexedOffset: 0,

  resolveAliasToIdent: function(ident) {
    while (1) {
      var varData = Variables.globals[ident];
      if (!(varData && varData.targetIdent)) break;
      ident = varData.targetIdent; // might need to eval to turn (6) into 6
    }
    return ident;
  },
};

var Types = {
  types: {},
};

// Constructs an array ['a0', 'a1', 'a2', ..., 'a(n-1)']
function genArgSequence(n) {
  var args = [];
  for(var i = 0; i < n; ++i) {
    args.push('a'+i);
  }
  return args;
}

var Functions = {
  // All functions that will be implemented in this file. Maps id to signature
  implementedFunctions: {},
  // functions added from the library. value 2 means asmLibraryFunction
  libraryFunctions: {},
};

var LibraryManager = {
  library: null,
  structs: {},
  loaded: false,
  libraries: [],

  has: function(name) {
    return this.libraries.indexOf(name) >= 0;
  },

  load: function() {
    if (this.library) return;

    // Core system libraries (always linked against)
    var libraries = [
      'library.js',
      'library_stack.js',
      'library_formatString.js',
      'library_math.js',
      'library_path.js',
      'library_signals.js',
      'library_syscall.js',
      'library_html5.js',
      'library_stack_trace.js',
      'library_wasi.js',
      'library_int53.js',
      'library_dylink.js'
    ];

    if (!EXCEPTION_HANDLING) {
      if (!DISABLE_EXCEPTION_THROWING) {
        libraries.push('library_exceptions.js');
      } else {
        libraries.push('library_exceptions_stub.js');
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
        if (SYSTEM_JS_LIBRARIES.indexOf('library_nodefs.js') < 0) {
          libraries.push('library_nodefs.js');
        }
        libraries.push('library_noderawfs.js');
      }
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
        'library_async.js'
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

    // Add any explicitly specified system JS libraries to link to, add those to link.
    libraries = libraries.concat(SYSTEM_JS_LIBRARIES)

    if (LZ4) {
      libraries.push('library_lz4.js');
    }

    if (MAX_WEBGL_VERSION >= 2) {
      libraries.push('library_webgl2.js');
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
        'library_stack.js',
        'library_formatString.js',
        'library_stack_trace.js'
      ];
    }

    // Deduplicate libraries to avoid processing any library file multiple times
    libraries = libraries.filter(function(item, pos) {
      return libraries.indexOf(item) == pos;
    });

    // Save the list for has() queries later.
    this.libraries = libraries;

    for (var i = 0; i < libraries.length; i++) {
      var filename = libraries[i];
      var src = read(filename);
      var processed = undefined;
      try {
        processed = processMacros(preprocess(src, filename));
        eval(processed);
      } catch(e) {
        var details = [e, e.lineNumber ? 'line number: ' + e.lineNumber : ''];
        if (VERBOSE) {
          details.push((e.stack || "").toString().replace('Object.<anonymous>', filename));
        }
        if (processed) {
          error('failure to execute js library "' + filename + '": ' + details);
          if (VERBOSE) {
            error('preprocessed source (you can run a js engine on this to get a clearer error message sometimes):\n=============\n' + processed + '\n=============');
          } else {
            error('use -s VERBOSE to see more details')
          }
        } else {
          error('failure to process js library "' + filename + '": ' + details);
          if (VERBOSE) {
            error('original source:\n=============\n' + src + '\n=============');
          } else {
            error('use -s VERBOSE to see more details')
          }
        }
        throw e;
      }
    }

    // apply synonyms. these are typically not speed-sensitive, and doing it this way makes it possible to not include hacks in the compiler
    // (and makes it simpler to switch between SDL versions, fastcomp and non-fastcomp, etc.).
    var lib = LibraryManager.library;
    libloop: for (var x in lib) {
      if (x.lastIndexOf('__') > 0) continue; // ignore __deps, __*
      if (lib[x + '__asm']) continue; // ignore asm library functions, those need to be fully optimized
      if (typeof lib[x] === 'string') {
        var target = x;
        while (typeof lib[target] === 'string') {
          // ignore code and variable assignments, aliases are just simple names
          if (lib[target].search(/[=({; ]/) >= 0) continue libloop;
          // ignore trivial pass-throughs to Math.*
          if (lib[target].indexOf('Math_') == 0) continue libloop;
          target = lib[target];
        }
        if (!isNaN(target)) continue; // This is a number, and so cannot be an alias target.
        if (typeof lib[target] === 'undefined' || typeof lib[target] === 'function') {
          // When functions are aliased, the alias target must provide a signature for the function so that an efficient form of forwarding can be implemented.
          // Primarily read the signature on the alias, and secondarily on the target.
          function testStringType(sig) {
            if (typeof lib[sig] !== 'undefined' && typeof typeof lib[sig] !== 'string') {
              error(sig + ' should be a string! (was ' + typeof lib[sig]);
            }
          }
          testStringType(x + '__sig');
          testStringType(target + '__sig');
          if (typeof lib[x + '__sig'] === 'string' && typeof lib[target + '__sig'] === 'string' && lib[x + '__sig'] != lib[target + '__sig']) {
            error(x + '__sig (' + lib[x + '__sig'] + ')  differs from ' + target + '__sig (' + lib[target + '__sig'] + ')');
          }

          var sig = lib[x + '__sig'] || lib[target + '__sig'];
          if (typeof sig !== 'string') {
            error('Function ' + x + ' aliases to target function ' + target + ', but neither the alias or the target provide a signature. Please add a ' + target + "__sig: 'vifj...' annotation or a " + x + "__sig: 'vifj...' annotation to describe the type of function forwarding that is needed!");
          }

          var argCount = sig.length - 1;
          var ret = sig == 'v' ? '' : 'return ';
          var args = genArgSequence(argCount).join(',');
          lib[x] = new Function(args, ret + '_' + target + '(' + args + ');');

          if (!lib[x + '__deps']) lib[x + '__deps'] = [];
          lib[x + '__deps'].push(target);
        }
      }
    }

    // all asm.js methods should just be run in JS. We should optimize them eventually into wasm. TODO
    for (var x in lib) {
      if (lib[x + '__asm']) {
        lib[x + '__asm'] = undefined;
      }
    }

    /*
    // export code for CallHandlers.h
    printErr('============================');
    for (var x in this.library) {
      var y = this.library[x];
      if (typeof y === 'string' && x.indexOf('__sig') < 0 && x.indexOf('__postset') < 0 && y.indexOf(' ') < 0) {
        printErr('DEF_REDIRECT_HANDLER(' + x + ', ' + y + ');');
      }
    }
    printErr('============================');
    for (var x in this.library) {
      var y = this.library[x];
      if (typeof y === 'string' && x.indexOf('__sig') < 0 && x.indexOf('__postset') < 0 && y.indexOf(' ') < 0) {
        printErr('  SETUP_CALL_HANDLER(' + x + ');');
      }
    }
    printErr('============================');
    // end export code for CallHandlers.h
    */

    this.loaded = true;
  },

  // Given an ident, see if it is an alias for something, and so forth, returning
  // the earliest ancestor (the root)
  getRootIdent: function(ident) {
    if (!this.library) return null;
    var ret = LibraryManager.library[ident];
    if (!ret) return null;
    var last = ident;
    while (typeof ret === 'string') {
      last = ret;
      ret = LibraryManager.library[ret];
    }
    return last;
  },

  isStubFunction: function(ident) {
    var libCall = LibraryManager.library[ident.substr(1)];
    return typeof libCall === 'function' && libCall.toString().replace(/\s/g, '') === 'function(){}'
                                         && !(ident in Functions.implementedFunctions);
  }
};

if (!BOOTSTRAPPING_STRUCT_INFO) {
  // Load struct and define information.
  var temp = JSON.parse(read(STRUCT_INFO));
  C_STRUCTS = temp.structs;
  C_DEFINES = temp.defines;
} else {
  C_STRUCTS = {};
  C_DEFINES = {};
}

// Safe way to access a C define. We check that we don't add library functions with missing defines.
function cDefine(key) {
	if (key in C_DEFINES) return C_DEFINES[key];
	throw 'Missing C define ' + key + '! If you just added it to struct_info.json, you need to ./emcc --clear-cache';
}

var EXPORTED_RUNTIME_METHODS_SET = set(EXPORTED_RUNTIME_METHODS.concat(EXTRA_EXPORTED_RUNTIME_METHODS));
EXPORTED_RUNTIME_METHODS = unset(EXPORTED_RUNTIME_METHODS_SET);
EXTRA_EXPORTED_RUNTIME_METHODS = [];

function isFSPrefixed(name) {
  return name.length > 3 && name[0] === 'F' && name[1] === 'S' && name[2] === '_';
}

// forcing the filesystem exports a few things by default
function isExportedByForceFilesystem(name) {
  return name === 'FS_createFolder' ||
         name === 'FS_createPath' ||
         name === 'FS_createDataFile' ||
         name === 'FS_createPreloadedFile' ||
         name === 'FS_createLazyFile' ||
         name === 'FS_createLink' ||
         name === 'FS_createDevice' ||
         name === 'FS_unlink' ||
         name === 'addRunDependency' ||
         name === 'removeRunDependency';
}

// export parts of the JS runtime that the user asked for
function exportRuntime() {
  // optionally export something.
  // in ASSERTIONS mode we show a useful error if it is used without
  // being exported. how we show the message depends on whether it's
  // a function (almost all of them) or a number.
  function maybeExport(name, isNumber) {
    // if requested to be exported, export it
    if (name in EXPORTED_RUNTIME_METHODS_SET) {
      var exported = name;
      // the exported name may differ from the internal name
      if (isFSPrefixed(exported)) {
        // this is a filesystem value, FS.x exported as FS_x
        exported = 'FS.' + exported.substr(3);
      } else if (exported === 'print') {
        exported = 'out';
      } else if (exported === 'printErr') {
        exported = 'err';
      }
      return 'Module["' + name + '"] = ' + exported + ';';
    }
    // do not export it. but if ASSERTIONS, emit a
    // stub with an error, so the user gets a message
    // if it is used, that they should export it
    if (ASSERTIONS) {
      // check if it already exists, to support EXPORT_ALL and other cases
      // (we could optimize this, but in ASSERTIONS mode code size doesn't
      // matter anyhow)
      var extra = '';
      if (isExportedByForceFilesystem(name)) {
        extra = '. Alternatively, forcing filesystem support (-s FORCE_FILESYSTEM=1) can export this for you';
      }
      if (!isNumber) {
        return 'if (!Object.getOwnPropertyDescriptor(Module, "' + name + '")) Module["' + name + '"] = function() { abort("\'' + name + '\' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)' + extra + '") };';
      } else {
        return 'if (!Object.getOwnPropertyDescriptor(Module, "' + name + '")) Object.defineProperty(Module, "' + name + '", { configurable: true, get: function() { abort("\'' + name + '\' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)' + extra + '") } });';
      }
    }
    return '';
  }

  function maybeExportNumber(name) {
    return maybeExport(name, true);
  }

  // All possible runtime elements to export
  var runtimeElements = [
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
    'makeBigInt',
    'dynCall',
    'getCompilerSetting',
    'print',
    'printErr',
    'getTempRet0',
    'setTempRet0',
    'callMain',
    'abort',
  ];

  function isJsLibraryConfigIdentifier(ident) {
    return ident.endsWith('__sig') || ident.endsWith('__proxy') || ident.endsWith('__asm') || ident.endsWith('__inline')
     || ident.endsWith('__deps') || ident.endsWith('__postset') || ident.endsWith('__docs') || ident.endsWith('__import');
  }

  // Add JS library elements such as FS, GL, ENV, etc. These are prefixed with
  // '$ which indicates they are JS methods.
  for (var ident in LibraryManager.library) {
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
      'allocateUTF8OnStack'
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
    var threadExports = ['PThread', '_pthread_self'];
    if (WASM) {
      threadExports.push('wasmMemory');
    }
    if (!MINIMAL_RUNTIME) {
      threadExports.push('ExitStatus');
    }

    threadExports.forEach(function(x) {
      EXPORTED_RUNTIME_METHODS_SET[x] = 1;
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
  for (var name in EXPORTED_RUNTIME_METHODS_SET) {
    if (/^dynCall_/.test(name)) {
      // a specific dynCall; add to the list
      runtimeElements.push(name);
    }
  }
  var runtimeNumbers = [
    'ALLOC_NORMAL',
    'ALLOC_STACK',
    'ALLOC_NONE',
  ];
  if (ASSERTIONS) {
    // check all exported things exist, warn about typos
    for (var name in EXPORTED_RUNTIME_METHODS_SET) {
      if (runtimeElements.indexOf(name) < 0 &&
          runtimeNumbers.indexOf(name) < 0) {
        printErr('warning: invalid item (maybe a typo?) in EXPORTED_RUNTIME_METHODS: ' + name);
      }
    }
  }
  return runtimeElements.map(function(name) {
    return maybeExport(name);
  }).join('\n') + runtimeNumbers.map(function(name) {
    return maybeExportNumber(name);
  }).join('\n');
}

var PassManager = {
  serialize: function() {
    print('\n//FORWARDED_DATA:' + JSON.stringify({
      Functions: Functions,
      EXPORTED_FUNCTIONS: EXPORTED_FUNCTIONS,
      ATINITS: ATINITS.join('\n'),
      ATMAINS: ATMAINS.join('\n'),
      ATEXITS: ATEXITS.join('\n'),
    }));
  },
  load: function(json) {
    var data = JSON.parse(json);
    for (var i in data.Types) {
      Types[i] = data.Types[i];
    }
    for (var i in data.Variables) {
      Variables[i] = data.Variables[i];
    }
    for (var i in data.Functions) {
      Functions[i] = data.Functions[i];
    }
    EXPORTED_FUNCTIONS = data.EXPORTED_FUNCTIONS;
    /*
    print('\n//LOADED_DATA:' + JSON.stringify({
      Types: Types,
      Variables: Variables,
      Functions: Functions
    }));
    */
  }
};
