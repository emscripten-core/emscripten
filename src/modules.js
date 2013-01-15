//"use strict";

// Various namespace-like modules

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
  PHI_REACHERS: set('branch', 'switch', 'invoke'),
  EXTENDS: set('sext', 'zext'),
  COMPS: set('icmp', 'fcmp'),
  INTRINSICS_32: set('_llvm_memcpy_p0i8_p0i8_i64', '_llvm_memmove_p0i8_p0i8_i64', '_llvm_memset_p0i8_i64'), // intrinsics that need args converted to i32 in USE_TYPED_ARRAYS == 2
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
    var form3b = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !"([^"]+)", metadata !"([^"]*)", (metadata !\d+|null)}.*$/);
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
      calc = form3a.exec(line) || form3ab.exec(line) || form3ac.exec(line) || form3ad.exec(line);
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
      this.llvmLineToSourceFile[l] = metadataToFilename[m];
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
  nextIndexedOffset: 0
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

  // Set to true if we actually use precise i64 math: If PRECISE_I64_MATH is set, and also such math is actually
  // needed (+,-,*,/,% - we do not need it for bitops), or PRECISE_I64_MATH is 2 (forced)
  preciseI64MathUsed: (PRECISE_I64_MATH == 2)
};

var Functions = {
  // All functions that will be implemented in this file. Maps id to signature
  implementedFunctions: {},
  libraryFunctions: {}, // functions added from the library
  unimplementedFunctions: {}, // library etc. functions that we need to index, maps id to signature

  indexedFunctions: {},
  nextIndex: 2, // Start at a non-0 (even, see below) value

  blockAddresses: {}, // maps functions to a map of block labels to label ids

  getSignature: function(returnType, argTypes) {
    var sig = returnType == 'void' ? 'v' : (isIntImplemented(returnType) ? 'i' : 'f');
    for (var i = 0; i < argTypes.length; i++) {
      var type = argTypes[i];
      if (!type) break; // varargs
      sig += isIntImplemented(type) ? (getBits(type) == 64 ? 'ii' : 'i') : 'f'; // legalized i64s will be i32s
    }
    return sig;
  },

  // Mark a function as needing indexing. Python will coordinate them all
  getIndex: function(ident) {
    if (phase != 'post' && singlePhase) {
      this.indexedFunctions[ident] = 0; // tell python we need this indexized
      return '{{{ FI_' + ident + ' }}}'; // something python will replace later
    } else {
      var ret = this.indexedFunctions[ident];
      if (!ret) {
        ret = this.nextIndex;
        this.nextIndex += 2; // Need to have indexes be even numbers, see |polymorph| test
        this.indexedFunctions[ident] = ret;
      }
      return ret.toString();
    }
  },

  getTable: function(sig) {
    return ASM_JS ? 'FUNCTION_TABLE_' + sig : 'FUNCTION_TABLE';
  },

  // Generate code for function indexing
  generateIndexing: function() {
    var total = this.nextIndex;
    if (ASM_JS) total = ceilPowerOfTwo(total); // must be power of 2 for mask
    function emptyTable(sig) {
      return zeros(total);
    }
    var tables = {};
    if (ASM_JS) {
      ['v', 'vi', 'ii', 'iii'].forEach(function(sig) { // add some default signatures that are used in the library
        tables[sig] = emptyTable(sig); // TODO: make them compact
      });
    }
    for (var ident in this.indexedFunctions) {
      var sig = ASM_JS ? Functions.implementedFunctions[ident] || Functions.unimplementedFunctions[ident] : 'x';
      assert(sig, ident);
      if (!tables[sig]) tables[sig] = emptyTable(sig); // TODO: make them compact
      tables[sig][this.indexedFunctions[ident]] = ident;
    }
    var generated = false;
    for (var t in tables) {
      generated = true;
      var table = tables[t];
      for (var i = 0; i < table.length; i++) {
        // Resolve multi-level aliases all the way down
        while (1) {
          var varData = Variables.globals[table[i]];
          if (!(varData && varData.resolvedAlias)) break;
          table[i] = table[+varData.resolvedAlias || eval(varData.resolvedAlias)]; // might need to eval to turn (6) into 6
        }
        // Resolve library aliases
        if (table[i]) {
          var libName = LibraryManager.getRootIdent(table[i].substr(1));
          if (libName && typeof libName == 'string') {
            table[i] = (libName.indexOf('.') < 0 ? '_' : '') + libName;
          }
        }
      }
      var indices = table.toString().replace('"', '');
      if (BUILD_AS_SHARED_LIB) {
        // Shared libraries reuse the parent's function table.
        tables[t] = Functions.getTable(t) + '.push.apply(' + Functions.getTable(t) + ', [' + indices + ']);\n';
      } else {
        tables[t] = 'var ' + Functions.getTable(t) + ' = [' + indices + '];\n';
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
  loaded: false,

  load: function() {
    if (this.library) return;

    var libraries = ['library.js', 'library_browser.js', 'library_sdl.js', 'library_gl.js', 'library_glut.js', 'library_xlib.js', 'library_egl.js', 'library_gc.js', 'library_jansson.js'].concat(additionalLibraries);
    for (var i = 0; i < libraries.length; i++) {
      eval(processMacros(preprocess(read(libraries[i]))));
    }

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

// Safe way to access a C define. We check that we don't add library functions with missing defines.
function cDefine(key) {
  return key in C_DEFINES ? C_DEFINES[key] : ('0 /* XXX missing C define ' + key + ' */');
}

var PassManager = {
  serialize: function() {
    if (phase == 'pre') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Types: Types,
        Variables: Variables,
        Functions: Functions,
        EXPORTED_FUNCTIONS: EXPORTED_FUNCTIONS // needed for asm.js global constructors (ctors)
      }));
    } else if (phase == 'funcs') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Types: { preciseI64MathUsed: Types.preciseI64MathUsed },
        Functions: {
          blockAddresses: Functions.blockAddresses,
          indexedFunctions: Functions.indexedFunctions,
          implementedFunctions: ASM_JS ? Functions.implementedFunctions : [],
          unimplementedFunctions: Functions.unimplementedFunctions,
        }
      }));
    } else if (phase == 'post') {
      print('\n//FORWARDED_DATA:' + JSON.stringify({
        Functions: { tables: Functions.tables }
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
    /*
    print('\n//LOADED_DATA:' + phase + ':' + JSON.stringify({
      Types: Types,
      Variables: Variables,
      Functions: Functions
    }));
    */
  }
};

