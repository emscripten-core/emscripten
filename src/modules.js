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
  // Set to true if we actually use precise i64 math: If PRECISE_I64_MATH is set, and also such math is actually
  // needed (+,-,*,/,% - we do not need it for bitops), or PRECISE_I64_MATH is 2 (forced)
  preciseI64MathUsed: (PRECISE_I64_MATH == 2)
};

var firstTableIndex = RESERVED_FUNCTION_POINTERS + 1;

var Functions = {
  // All functions that will be implemented in this file. Maps id to signature
  implementedFunctions: {},
  libraryFunctions: {}, // functions added from the library. value 2 means asmLibraryFunction
  unimplementedFunctions: {}, // library etc. functions that we need to index, maps id to signature

  nextIndex: firstTableIndex, // Start at a non-0 (even, see below) value
  neededTables: set('v', 'vi', 'ii', 'iii'), // signatures that appeared (initialized with library stuff
                                             // we always use), and we will need a function table for

  blockAddresses: {}, // maps functions to a map of block labels to label ids

  aliases: {}, // in shared modules (MAIN_MODULE or SHARED_MODULE), a list of aliases for functions that have them

  getSignatureLetter: function(type) {
    switch(type) {
      case 'float': return 'f';
      case 'double': return 'd';
      case 'void': return 'v';
      default: return 'i';
    }
  },

  getSignatureType: function(letter) {
    switch(letter) {
      case 'v': return 'void';
      case 'i': return 'i32';
      case 'f': return 'float';
      case 'd': return 'double';
      default: throw 'what is this sig? ' + sig;
    }
  },

  getSignature: function(returnType, argTypes, hasVarArgs) {
    var sig = Functions.getSignatureLetter(returnType);
    for (var i = 0; i < argTypes.length; i++) {
      var type = argTypes[i];
      if (!type) break; // varargs
      if (type in Compiletime.FLOAT_TYPES) {
        sig += Functions.getSignatureLetter(type);
      } else {
        var chunks = getNumIntChunks(type);
        if (chunks > 0) {
          for (var j = 0; j < chunks; j++) sig += 'i';
        } else if (type !== '...') {
          // some special type like a SIMD vector (anything but varargs, which we handle below)
          sig += Functions.getSignatureLetter(type);
        }
      }
    }
    if (hasVarArgs) sig += 'i';
    return sig;
  },

  getTable: function(sig) {
    return 'FUNCTION_TABLE_' + sig
  }
};

var LibraryManager = {
  library: null,
  structs: {},
  loaded: false,

  load: function() {
    if (this.library) return;

    // Core system libraries (always linked against)
    var libraries = [
      'library.js',
      'library_browser.js',
      'library_formatString.js',
      'library_path.js',
      'library_signals.js',
      'library_syscall.js',
      'library_html5.js'
    ];

    if (FILESYSTEM) {
      // Core filesystem libraries (always linked against, unless -s FILESYSTEM=0 is specified)
      libraries = libraries.concat([
        'library_fs.js',
        'library_memfs.js',
        'library_tty.js',
        'library_pipefs.js',
      ]);

      // Additional filesystem libraries (in strict mode, link to these explicitly via -lxxx.js)
      if (!STRICT) {
        libraries = libraries.concat([
          'library_idbfs.js',
          'library_nodefs.js',
          'library_proxyfs.js',
          'library_sockfs.js',
          'library_workerfs.js',
          'library_lz4.js',
        ]);

        if (NODERAWFS) {
          libraries.push('library_noderawfs.js')
        }
      }
    }

    // Additional JS libraries (in strict mode, link to these explicitly via -lxxx.js)
    if (!STRICT) {
      libraries = libraries.concat([
        'library_sdl.js',
        'library_gl.js',
        'library_glut.js',
        'library_xlib.js',
        'library_egl.js',
        'library_openal.js',
        'library_glfw.js',
        'library_uuid.js',
        'library_glew.js',
        'library_idbstore.js',
        'library_async.js',
        'library_vr.js'
      ]);
    }

    // If there are any explicitly specified system JS libraries to link to, add those to link.
    if (SYSTEM_JS_LIBRARIES) {
      libraries = libraries.concat(SYSTEM_JS_LIBRARIES.split(','));
    }

    libraries = libraries.concat(additionalLibraries);

    // For each JS library library_xxx.js, add a preprocessor token __EMSCRIPTEN_HAS_xxx_js__ so that code can conditionally dead code eliminate out
    // if a particular feature is not being linked in.
    for (var i = 0; i < libraries.length; ++i) {
      global['__EMSCRIPTEN_HAS_' + libraries[i].replace('.', '_').replace('library_', '') + '__'] = 1
    }

    if (BOOTSTRAPPING_STRUCT_INFO) libraries = ['library_bootstrap_structInfo.js', 'library_formatString.js'];
    if (ONLY_MY_CODE) {
      libraries = [];
      LibraryManager.library = {};
    }

    for (var i = 0; i < libraries.length; i++) {
      var filename = libraries[i];
      var src = read(filename);
      var processed = undefined;
      try {
        processed = processMacros(preprocess(src, filename));
        eval(processed);
      } catch(e) {
        var details = [e, e.lineNumber ? 'line number: ' + e.lineNumber : '', (e.stack || "").toString().replace('Object.<anonymous>', filename)];
        if (processed) {
          error('failure to execute js library "' + filename + '": ' + details + '\npreprocessed source (you can run a js engine on this to get a clearer error message sometimes):\n=============\n' + processed + '\n=============\n');
        } else {
          error('failure to process js library "' + filename + '": ' + details + '\noriginal source:\n=============\n' + src + '\n=============\n');
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
          // ignore code, aliases are just simple names
          if (lib[target].search(/[({; ]/) >= 0) continue libloop;
          // ignore trivial pass-throughs to Math.*
          if (lib[target].indexOf('Math_') == 0) continue libloop;
          target = lib[target];
        }
        if (lib[target + '__asm']) continue; // This is an alias of an asm library function. Also needs to be fully optimized.
        if (typeof lib[target] === 'undefined' || typeof lib[target] === 'function') {
          lib[x] = new Function('return _' + target + '.apply(null, arguments)');
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
        return 'if (!Module["' + name + '"]) Module["' + name + '"] = function() { abort("\'' + name + '\' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)' + extra + '") };';
      } else {
        return 'if (!Module["' + name + '"]) Object.defineProperty(Module, "' + name + '", { get: function() { abort("\'' + name + '\' was not exported. add it to EXTRA_EXPORTED_RUNTIME_METHODS (see the FAQ)' + extra + '") } });';
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
    'Pointer_stringify',
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
    'staticAlloc',
    'dynamicAlloc',
    'warnOnce',
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
  ];
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
    'ALLOC_STATIC',
    'ALLOC_DYNAMIC',
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
      EXPORTED_FUNCTIONS: EXPORTED_FUNCTIONS
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
