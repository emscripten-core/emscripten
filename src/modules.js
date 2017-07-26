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

var firstTableIndex = FUNCTION_POINTER_ALIGNMENT * RESERVED_FUNCTION_POINTERS + 1;

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

    if (!NO_FILESYSTEM) {
      // Core filesystem libraries (always linked against, unless -s NO_FILESYSTEM=1 is specified)
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
      try {
        var processed = processMacros(preprocess(src, filename));
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
	throw 'XXX missing C define ' + key + '!';
}

var EXPORTED_RUNTIME_METHODS_SET = null;

function maybeExport(name) {
  if (!EXPORTED_RUNTIME_METHODS_SET) EXPORTED_RUNTIME_METHODS_SET = set(EXPORTED_RUNTIME_METHODS.concat(EXTRA_EXPORTED_RUNTIME_METHODS));
  if (name in EXPORTED_RUNTIME_METHODS_SET) {
    return 'Module["' + name + '"] = ' + name + ';';
  } else {
    return '';
  }
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
