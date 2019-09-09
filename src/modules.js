// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
      'library_formatString.js',
      'library_path.js',
      'library_signals.js',
      'library_syscall.js',
      'library_html5.js',
      'library_stack_trace.js'
    ];

    if (!DISABLE_EXCEPTION_THROWING) {
      libraries.push('library_exceptions.js');
    } else {
      libraries.push('library_exceptions_stub.js');
    }

    if (!MINIMAL_RUNTIME) {
      libraries.push('library_browser.js');
    }

    if (FILESYSTEM) {
      // Core filesystem libraries (always linked against, unless -s FILESYSTEM=0 is specified)
      libraries = libraries.concat([
        'library_fs.js',
        'library_memfs.js',
        'library_tty.js',
        'library_pipefs.js',
      ]);

      // Additional filesystem libraries (without AUTO_JS_LIBRARIES, link to these explicitly via -lxxx.js)
      if (AUTO_JS_LIBRARIES) {
        if (ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER) {
          libraries = libraries.concat([
            'library_idbfs.js',
            'library_proxyfs.js',
            'library_sockfs.js',
            'library_workerfs.js',
          ]);
        }
        if (ENVIRONMENT_MAY_BE_NODE) {
          libraries = libraries.concat([
            'library_nodefs.js',
          ]);
        }
        if (NODERAWFS) {
          libraries.push('library_noderawfs.js')
        }
      }
    }

    // Additional JS libraries (without AUTO_JS_LIBRARIES, link to these explicitly via -lxxx.js)
    if (AUTO_JS_LIBRARIES) {
      libraries = libraries.concat([
        'library_webgl.js',
        'library_openal.js',
        'library_vr.js',
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
      if (EMTERPRETIFY_ASYNC || ASYNCIFY) {
        libraries.push('library_async.js');
      }
      if (USE_SDL == 1) {
        libraries.push('library_sdl.js');
      }
      if (USE_SDL == 2) {
        libraries.push('library_egl.js', 'library_webgl.js');
      }
    }

    // If there are any explicitly specified system JS libraries to link to, add those to link.
    if (SYSTEM_JS_LIBRARIES) {
      libraries = libraries.concat(SYSTEM_JS_LIBRARIES.split(','));
    }

    if (LZ4) {
      libraries.push('library_lz4.js');
    }

    if (USE_WEBGL2) {
      libraries.push('library_webgl2.js');
    }

    if (LEGACY_GL_EMULATION) {
      libraries.push('library_glemu.js');
    }

    libraries = libraries.concat(additionalLibraries);

    if (BOOTSTRAPPING_STRUCT_INFO) libraries = ['library_bootstrap_structInfo.js', 'library_formatString.js'];
    if (ONLY_MY_CODE) {
      libraries.length = 0;
      LibraryManager.library = {};
    }

    // TODO: deduplicate libraries (not needed for correctness, but avoids unnecessary work)

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
        if (lib[target + '__asm']) continue; // This is an alias of an asm library function. Also needs to be fully optimized.
        if (!isNaN(target)) continue; // This is a number, and so cannot be an alias target.
        if (typeof lib[target] === 'undefined' || typeof lib[target] === 'function') {
          // If the alias provides a signature, then construct a specific 'function foo(a0, a1, a2) { [return] _target(a0, a1, a2); }' form of forwarding.
          // Otherwise construct a generic 'function foo() { return _target.apply(null, arguments); }' forwarding.
          // The benefit of the first form is that Closure is able to fully inline and reason about the function.
          // Note that the signature is checked on the alias function, not on the target function. That allows aliases to choose individually which form
          // to use. 
          if (lib[x + '__sig']) {
            var argCount = lib[x + '__sig'].length - 1;
            var ret = lib[x + '__sig'] == 'v' ? '' : 'return ';
            var args = genArgSequence(argCount).join(',');
            lib[x] = new Function(args, ret + '_' + target + '(' + args + ');');
          } else {
            lib[x] = new Function('return _' + target + '.apply(null, arguments)');
          }
          if (!lib[x + '__deps']) lib[x + '__deps'] = [];
          lib[x + '__deps'].push(target);
        }
      }
    }

    if (WASM_BACKEND) {
      // all asm.js methods should just be run in JS. We should optimize them eventually into wasm. TODO
      for (var x in lib) {
        if (lib[x + '__asm']) {
          lib[x + '__asm'] = undefined;
        }
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
    if (SIDE_MODULE == 1) return false; // cannot eliminate these, as may be implement in the main module and imported by us
    var libCall = LibraryManager.library[ident.substr(1)];
    return typeof libCall === 'function' && libCall.toString().replace(/\s/g, '') === 'function(){}'
                                         && !(ident in Functions.implementedFunctions);
  }
};

if (!BOOTSTRAPPING_STRUCT_INFO && !ONLY_MY_CODE) {
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
         name === 'getMemory' ||
         name === 'addRunDependency' ||
         name === 'removeRunDependency' ||
         name === 'calledRun';
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
        return 'if (!Object.getOwnPropertyDescriptor(Module, "' + name + '")) Object.defineProperty(Module, "' + name + '", { get: function() { abort("\'' + name + '\' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)' + extra + '") } });';
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
    'getMemory',
    'AsciiToString',
    'stringToAscii',
    'UTF8ArrayToString',
    'UTF8ToString',
    'stringToUTF8Array',
    'stringToUTF8',
    'lengthBytesUTF8',
    'UTF16ToString',
    'stringToUTF16',
    'lengthBytesUTF16',
    'UTF32ToString',
    'stringToUTF32',
    'lengthBytesUTF32',
    'allocateUTF8',
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
    'ENV',
    'FS',
    'FS_createFolder',
    'FS_createPath',
    'FS_createDataFile',
    'FS_createPreloadedFile',
    'FS_createLazyFile',
    'FS_createLink',
    'FS_createDevice',
    'FS_unlink',
    'GL',
    'dynamicAlloc',
    'loadDynamicLibrary',
    'loadWebAssemblyModule',
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
    'stackSave',
    'stackRestore',
    'stackAlloc',
    'establishStackSpace',
    'print',
    'printErr',
    'getTempRet0',
    'setTempRet0',
    'callMain',
  ];

  if (!MINIMAL_RUNTIME) {
    runtimeElements.push('Pointer_stringify');
    runtimeElements.push('warnOnce');
  }

  if (MODULARIZE) {
    // In MODULARIZE=1 mode, the following functions need to be exported out to Module for worker.js to access.
    if (STACK_OVERFLOW_CHECK) {
      runtimeElements.push('writeStackCookie');
      runtimeElements.push('checkStackCookie');
      runtimeElements.push('abortStackOverflow');
    }
    if (USE_PTHREADS) {
      runtimeElements.push('PThread');
      runtimeElements.push('ExitStatus');
    }
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
    'ALLOC_DYNAMIC',
    'ALLOC_NONE',
    'calledRun',
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
      STATIC_BUMP: STATIC_BUMP, // updated with info from JS
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
