//"use strict";

// Various namespace-like modules

var STACK_ALIGN = TARGET_X86 ? 4 : (RUNNING_FASTCOMP ? 16 : 8);

var LLVM = {
  LINKAGES: set('private', 'linker_private', 'linker_private_weak', 'linker_private_weak_def_auto', 'internal',
                'available_externally', 'linkonce', 'common', 'weak', 'appending', 'extern_weak', 'linkonce_odr',
                'weak_odr', 'externally_visible', 'dllimport', 'dllexport', 'unnamed_addr', 'thread_local'),
  VISIBILITIES: set('default', 'hidden', 'protected'),
  PARAM_ATTR: set('noalias', 'signext', 'zeroext', 'inreg', 'sret', 'nocapture', 'nest'),
  FUNC_ATTR: set('hidden', 'nounwind', 'define', 'inlinehint', '{'),
  CALLING_CONVENTIONS: set('ccc', 'fastcc', 'coldcc', 'cc10', 'x86_fastcallcc', 'x86_stdcallcc', 'cc11'),
  ACCESS_OPTIONS: set('volatile', 'atomic'),
  INVOKE_MODIFIERS: set('alignstack', 'alwaysinline', 'inlinehint', 'naked', 'noimplicitfloat', 'noinline', 'alwaysinline attribute.', 'noredzone', 'noreturn', 'nounwind', 'optsize', 'readnone', 'readonly', 'ssp', 'sspreq'),
  SHIFTS: set('ashr', 'lshr', 'shl'),
  PHI_REACHERS: set('branch', 'switch', 'invoke', 'indirectbr'),
  EXTENDS: set('sext', 'zext'),
  COMPS: set('icmp', 'fcmp'),
  CONVERSIONS: set('inttoptr', 'ptrtoint', 'uitofp', 'sitofp', 'fptosi', 'fptoui', 'fpext', 'fptrunc'),
  INTRINSICS_32: set('_llvm_memcpy_p0i8_p0i8_i64', '_llvm_memmove_p0i8_p0i8_i64', '_llvm_memset_p0i8_i64'), // intrinsics that need args converted to i32 in USE_TYPED_ARRAYS == 2
  MATHOP_IGNORABLES: set('exact', 'nnan', 'ninf', 'nsz', 'arcp', 'fast'),
  PARAM_IGNORABLES: set('nocapture', 'readonly', 'readnone'),
};
LLVM.GLOBAL_MODIFIERS = set(keys(LLVM.LINKAGES).concat(['constant', 'global', 'hidden']));

var Debugging = {
  handleMetadata: function(lines) {
    for (var i = lines.length-1; i >= 0; i--) {
      if (/^!\d+ = metadata .*/.exec(lines[i])) {
        Debugging.processMetadata(lines);
        break;
      }
    }
  },

  processMetadata: function(lines) {
    var llvmLineToMetadata = {};
    var metadataToSourceLine = {};
    var metadataToParentMetadata = {};
    var metadataToFilename = {};

    var form1 = new RegExp(/^  .*, !dbg !(\d+) *$/);
    var form2 = new RegExp(/^  .*, !dbg !(\d+) *; .*$/);
    var form3 = new RegExp(/^!(\d+) = metadata !{i32 (\d+), (?:i32 \d+|null), metadata !(\d+), .*}$/);
    var form3a = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:i32 \d+|metadata !\d+), (?:i32 \d+|null), (?:i32 \d+|null), metadata !(\d+), (?:i32 \d+|null)}.*/);
    var form3ab = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:metadata !\d+|i32 \d+|null), metadata !(\d+).*$/);
    var form3ac = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:metadata !\d+|null), metadata !"[^"]*", metadata !(\d+)[^\[]*.*$/);
    var form3ad = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:i32 \d+|null), (?:i32 \d+|null), metadata !"[^"]*", metadata !"[^"]*", metadata !"[^"]*", metadata !(\d+),.*$/);
    var form3ae = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !(\d+).*$/);
    // LLVM 3.3 drops the first and last parameters.
    var form3b = new RegExp(/^!(\d+) = metadata !{(?:i32 \d+, )?metadata !"([^"]+)", metadata !"([^"]*)"(?:, (metadata !\d+|null))?}.*$/);
    var form3c = new RegExp(/^!(\d+) = metadata !{\w+\d* !?(\d+)[^\d].*$/);
    var form4 = new RegExp(/^!llvm.dbg.[\w\.]+ = .*$/);
    var form5 = new RegExp(/^!(\d+) = metadata !{.*$/);
    var form6 = new RegExp(/^  (tail )?call void \@llvm.dbg.\w+\(metadata .*$/);

    for (var i = 0; i < lines.length; i++) {
      var line = lines[i];
      line = line.replace(/; +\[debug line = \d+:\d+\]/, '');
      var skipLine = false;

      if (form6.exec(line)) {
        lines[i] = ';';
        continue;
      }

      var calc = form1.exec(line) || form2.exec(line);
      if (calc) {
        llvmLineToMetadata[i+1] = calc[1];
        lines[i] = line.replace(/, !dbg !\d+/, '');
        continue;
      }
      calc = form3.exec(line);
      if (calc) {
        metadataToSourceLine[calc[1]] = calc[2];
        metadataToParentMetadata[calc[1]] = calc[3];
        lines[i] = ';'; // return an empty line, to keep line numbers of subsequent lines the same
        continue;
      }
      calc = form3a.exec(line) || form3ab.exec(line) || form3ac.exec(line) || form3ad.exec(line) || form3ae.exec(line);
      if (calc) {
        metadataToParentMetadata[calc[1]] = calc[2];
        lines[i] = ';';
        continue;
      }
      calc = form3b.exec(line);
      if (calc) {
        metadataToFilename[calc[1]] = /* LLVM 2.8<= : calc[3] + '/' + */ calc[2];
        lines[i] = ';';
        continue;
      }
      calc = form3c.exec(line) || form4.exec(line) || form5.exec(line);
      if (calc) {
        lines[i] = ';';
        continue;
      }
      if (line[0] == '!') skipLine = true;
      lines[i] = skipLine ? ';' : line;
    }

    /*
    dprint("ll ==> meta: " + JSON.stringify(llvmLineToMetadata));
    dprint("meta ==> sline: " + JSON.stringify(metadataToSourceLine));
    dprint("meta ==> pmeta: " + JSON.stringify(metadataToParentMetadata));
    dprint("meta ==> fname: " + JSON.stringify(metadataToFilename));
    */

    this.llvmLineToSourceLine = {};
    this.llvmLineToSourceFile = {};
    for (var l in llvmLineToMetadata) {
      var m = llvmLineToMetadata[l];
      this.llvmLineToSourceLine[l] = metadataToSourceLine[m];
      dprint('metadata', 'starting to recurse metadata for: ' + m);
      while (!metadataToFilename[m]) {
        dprint('metadata', 'recursing metadata, at: ' + m);
        m = metadataToParentMetadata[m];
        assert(m, 'Confused as to parent metadata for llvm #' + l + ', metadata !' + m);
      }
      // Normalize Windows path slashes coming from LLVM metadata, so that forward slashes can be assumed as path delimiters.
      this.llvmLineToSourceFile[l] = metadataToFilename[m].replace(/\\5C/g, '/');
    }

    this.on = true;
  },

  clear: function() {
    this.llvmLineToSourceLine = {};
    this.llvmLineToSourceFile = {};
    this.on = false;
  },

  getComment: function(lineNum) {
    if (!this.on) return null;
    return lineNum in this.llvmLineToSourceLine ? ' //@line ' + this.llvmLineToSourceLine[lineNum] + ' "' +
                                                                this.llvmLineToSourceFile[lineNum] + '"' : '';
  },

  getAssociatedSourceFile: function(lineNum) {
    if (!this.on) return null;
    return lineNum in this.llvmLineToSourceLine ? this.llvmLineToSourceFile[lineNum] : null;
  },
  
  getIdentifier: function(lineNum) {
    if (!this.on) return null;
    if (lineNum === undefined) {
      lineNum = Framework.currItem.lineNum;
      assert(lineNum !== undefined);
    }
    var approx = false;
    var sourceFile;
    while (lineNum >= 0) {
      var sourceFile = this.llvmLineToSourceFile[lineNum];
      if (sourceFile) break;
      lineNum--;
      approx = true;
    }
    if (!sourceFile) return 'UNKNOWN';
    return sourceFile.split('/').slice(-1)[0] + ':' + (approx ? '~' : '') + this.llvmLineToSourceLine[lineNum];
  }
};

var PreProcessor = {
  eliminateUnneededIntrinsics: function(lines) {
    // LLVM sometimes aggresively adds lifetime annotations, for example
    //
    //    %0 = bitcast %"class.std::__1::__tree"** %this.addr.i to i8* ; [#uses=1 type=i8*]
    //    call void @llvm.lifetime.start(i64 -1, i8* %0) nounwind
    //    [..]
    //    %6 = bitcast float* %__x.addr.i to i8*          ; [#uses=1 type=i8*]
    //    call void @llvm.lifetime.end(i64 -1, i8* %6) nounwind
    //
    // This greatly hurts us if we do not eliminate it ahead of time, because while we
    // will correctly do nothing for the lifetime intrinsic itself, the bitcast of the
    // parameter to it will prevent nativization of the variable being cast (!)
    for (var i = 0; i < lines.length; i++) {
      var line = lines[i];
      if (/call void @llvm.lifetime.(start|end)\(i\d+ -1,.*/.exec(line)) {
        lines[i] = ';';
      }
    }
  }
};

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
  fatTypes: {}, // With QUANTUM_SIZE=1, we store the full-size type data here
  flipTypes: function() {
    var temp = this.fatTypes;
    this.fatTypes = this.types;
    this.types = temp;
  },
  structMetadata: {},

  // Remove all data not needed during runtime (like line numbers, JS, etc.)
  cleanForRuntime: function() {
    values(this.types).forEach(function(type) {
      delete type.intertype;
      delete type.name_;
      delete type.lineNum;
      delete type.lines;
      delete type.needsFlattening;
      delete type.JS;
    });
    keys(this.types).forEach(function(longer) {
      var shorter = longer.replace('%struct.', '').replace('%class.');
      if (shorter === longer) return;
      if (shorter in this.types) return;
      this.types[shorter] = this.types[longer];
    }, this);
  },

  needAnalysis: {}, // Types noticed during parsing, that need analysis

  hasInlineJS: false, // whether the program has inline JS anywhere

  usesSIMD: false,

  // Set to true if we actually use precise i64 math: If PRECISE_I64_MATH is set, and also such math is actually
  // needed (+,-,*,/,% - we do not need it for bitops), or PRECISE_I64_MATH is 2 (forced)
  preciseI64MathUsed: (PRECISE_I64_MATH == 2)
};

var firstTableIndex = FUNCTION_POINTER_ALIGNMENT * ((ASM_JS ? RESERVED_FUNCTION_POINTERS : 0) + 1);

var Functions = {
  // All functions that will be implemented in this file. Maps id to signature
  implementedFunctions: {},
  libraryFunctions: {}, // functions added from the library. value 2 means asmLibraryFunction
  unimplementedFunctions: {}, // library etc. functions that we need to index, maps id to signature

  indexedFunctions: {},
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

  // Mark a function as needing indexing. Python will coordinate them all
  getIndex: function(ident, sig) {
    var ret;
    if (phase != 'post' && singlePhase) {
      ret = "'{{ FI_" + toNiceIdent(ident) + " }}'"; // something python will replace later
      this.indexedFunctions[ident] = 0;
    } else {
      if (!singlePhase) return 'NO_INDEX'; // Should not index functions in post
      ret = this.indexedFunctions[ident];
      assert(ret);
      ret = ret.toString();
    }
    if (SIDE_MODULE && sig) { // sig can be undefined for the GL library functions
      ret = '((F_BASE_' + sig + ' + ' + ret + ')|0)';
    } else if (BUILD_AS_SHARED_LIB) {
      ret = '(FUNCTION_TABLE_OFFSET + ' + ret + ')';
    }
    return ret;
  },

  getTable: function(sig) {
    return ASM_JS ? 'FUNCTION_TABLE_' + sig : 'FUNCTION_TABLE';
  },

  // Generate code for function indexing
  generateIndexing: function() {
    var tables = { pre: '' };
    if (ASM_JS) {
      keys(Functions.neededTables).forEach(function(sig) { // add some default signatures that are used in the library
        tables[sig] = zeros(firstTableIndex);
      });
    }
    for (var ident in this.indexedFunctions) {
      var sig = ASM_JS ? Functions.implementedFunctions[ident] || Functions.unimplementedFunctions[ident] || LibraryManager.library[ident.substr(1) + '__sig'] : 'x';
      assert(sig, ident);
      if (!tables[sig]) tables[sig] = zeros(firstTableIndex);
      var index = this.indexedFunctions[ident];
      for (var i = tables[sig].length; i < index; i++) {
        tables[sig][i] = 0; // keep flat
      }
      tables[sig][index] = ident;
    }
    var generated = false;
    var wrapped = {}; // whether we wrapped a lib func
    var maxTable = 0;
    for (var t in tables) {
      if (t == 'pre') continue;
      generated = true;
      var table = tables[t];
      for (var i = 0; i < table.length; i++) {
        // Resolve multi-level aliases all the way down
        while (1) {
          var varData = Variables.globals[table[i]];
          if (!(varData && varData.resolvedAlias && !/(FUNCTION_TABLE_OFFSET|F_BASE_)/.test(varData.resolvedAlias))) break;
          table[i] = table[+varData.resolvedAlias || eval(varData.resolvedAlias)]; // might need to eval to turn (6) into 6
        }
        // Resolve library aliases
        if (table[i]) {
          var libName = LibraryManager.getRootIdent(table[i].substr(1));
          if (libName && typeof libName == 'string') {
            table[i] = (libName.indexOf('Math_') < 0 ? '_' : '') + libName;
          }
        }
        if (ASM_JS) {
          var curr = table[i];
          if (curr && curr != '0' && !Functions.implementedFunctions[curr]) {
            var short = toNiceIdent(curr); // fix Math.* to Math_*
            curr = t + '_' + short; // libfuncs can alias with different sigs, wrap each separately
            // This is a library function, we can't just put it in the function table, need a wrapper
            if (!wrapped[curr]) {
              var args = '', arg_coercions = '', call = short + '(', retPre = '', retPost = '';
              if (t[0] != 'v') {
                var temp = asmFFICoercion('X', Functions.getSignatureType(t[0])).split('X');
                retPre = 'return ' + temp[0];
                retPost = temp[1];
              }
              for (var j = 1; j < t.length; j++) {
                args += (j > 1 ? ',' : '') + 'a' + j;
                var type = Functions.getSignatureType(t[j]);
                arg_coercions += 'a' + j + '=' + asmCoercion('a' + j, type) + ';';
                call += (j > 1 ? ',' : '') + asmCoercion('a' + j, type === 'float' ? 'double' : type); // ffi arguments must be doubles if they are floats
              }
              call += ')';
              if (short == '_setjmp') printErr('WARNING: setjmp used via a function pointer. If this is for libc setjmp (not something of your own with the same name), it will break things');
              tables.pre += 'function ' + curr + '__wrapper(' + args + ') { ' + arg_coercions + ' ; ' + retPre + call + retPost + ' }\n';
              wrapped[curr] = 1;
            }
            table[i] = curr + '__wrapper';
          }
        }
      }
      maxTable = Math.max(maxTable, table.length);
    }
    if (ASM_JS) maxTable = ceilPowerOfTwo(maxTable);
    for (var t in tables) {
      if (t == 'pre') continue;
      var table = tables[t];
      // asm function table mask must be power of two, and non-asm must be aligned
      // if nonaliasing, then standardize function table size, to avoid aliasing pointers through the &M mask (in a small table using a big index)
      var fullSize = ASM_JS ? (ALIASING_FUNCTION_POINTERS ? ceilPowerOfTwo(table.length) : maxTable) : ((table.length+FUNCTION_POINTER_ALIGNMENT-1)&-FUNCTION_POINTER_ALIGNMENT);
      for (var i = table.length; i < fullSize; i++) {
        table[i] = 0;
      }
      // finalize table
      var indices = table.toString().replace('"', '');
      if (BUILD_AS_SHARED_LIB) {
        // Shared libraries reuse the parent's function table.
        tables[t] = Functions.getTable(t) + '.push.apply(' + Functions.getTable(t) + ', [' + indices + ']);\n';
      } else {
        tables[t] = 'var ' + Functions.getTable(t) + ' = [' + indices + '];\n';
        if (SAFE_DYNCALLS) {
          tables[t] += 'var FUNCTION_TABLE_NAMES = ' + JSON.stringify(table).replace(/\n/g, '').replace(/,0/g, ',0\n') + ';\n';
        }
      }
    }
    if (!generated && !ASM_JS) {
      tables['x'] = 'var FUNCTION_TABLE = [0, 0];\n'; // default empty table
    }
    Functions.tables = tables;
  }
};

var LibraryManager = {
  library: null,
  structs: {},
  loaded: false,

  load: function() {
    if (this.library) return;

    var libraries = [
      'library.js',
      'library_formatString.js'
    ];
    if (!NO_FILESYSTEM) {
      libraries = libraries.concat([
        'library_path.js',
        'library_fs.js',
        'library_idbfs.js',
        'library_memfs.js',
        'library_nodefs.js',
        'library_sockfs.js',
        'library_tty.js'
      ]);
    }
    if (!NO_BROWSER) {
      libraries = libraries.concat([
        'library_browser.js'
      ]);
    }
    libraries = libraries.concat([
      'library_sdl.js',
      'library_gl.js',
      'library_glut.js',
      'library_xlib.js',
      'library_egl.js',
      'library_gc.js',
      'library_jansson.js',
      'library_openal.js',
      'library_glfw.js',
      'library_uuid.js',
      'library_glew.js',
      'library_html5.js',
      'library_signals.js',
      'library_idbstore.js',
      'library_async.js',
      'library_vr.js'
    ]).concat(additionalLibraries);

    if (BOOTSTRAPPING_STRUCT_INFO) libraries = ['library_bootstrap_structInfo.js', 'library_formatString.js'];

    for (var i = 0; i < libraries.length; i++) {
      var filename = libraries[i];
      var src = read(filename);
      try {
        var processed = processMacros(preprocess(src));
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
          if (lib[target].indexOf('(') >= 0) continue libloop;
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

var PassManager = {
  serialize: function() {
    if (phase == 'pre') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Types: Types,
        Variables: Variables,
        Functions: Functions,
        EXPORTED_FUNCTIONS: EXPORTED_FUNCTIONS, // needed for asm.js global constructors (ctors)
        Runtime: { GLOBAL_BASE: Runtime.GLOBAL_BASE }
      }));
    } else if (phase == 'funcs') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Types: {
          hasInlineJS: Types.hasInlineJS,
          usesSIMD: Types.usesSIMD,
          preciseI64MathUsed: Types.preciseI64MathUsed
        },
        Functions: {
          blockAddresses: Functions.blockAddresses,
          indexedFunctions: Functions.indexedFunctions,
          implementedFunctions: ASM_JS ? Functions.implementedFunctions : [],
          unimplementedFunctions: Functions.unimplementedFunctions,
          neededTables: Functions.neededTables
        }
      }));
    } else if (phase == 'post') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Functions: { tables: Functions.tables }
      }));
    } else if (phase == 'glue') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Functions: Functions,
        EXPORTED_FUNCTIONS: EXPORTED_FUNCTIONS
      }));
    }
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
    print('\n//LOADED_DATA:' + phase + ':' + JSON.stringify({
      Types: Types,
      Variables: Variables,
      Functions: Functions
    }));
    */
  }
};

var Framework = {
  currItem: null
};

