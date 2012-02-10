//"use strict";

// Various namespace-like modules

var LLVM = {
  LINKAGES: set('private', 'linker_private', 'linker_private_weak', 'linker_private_weak_def_auto', 'internal',
                'available_externally', 'linkonce', 'common', 'weak', 'appending', 'extern_weak', 'linkonce_odr',
                'weak_odr', 'externally_visible', 'dllimport', 'dllexport', 'unnamed_addr'),
  VISIBILITIES: set('default', 'hidden', 'protected'),
  PARAM_ATTR: set('noalias', 'signext', 'zeroext', 'inreg', 'sret', 'nocapture', 'nest'),
  FUNC_ATTR: set('hidden', 'nounwind', 'define', 'inlinehint', '{'),
  CALLING_CONVENTIONS: set('ccc', 'fastcc', 'coldcc', 'cc10', 'x86_fastcallcc', 'x86_stdcallcc'),
  ACCESS_OPTIONS: set('volatile', 'atomic'),
  INVOKE_MODIFIERS: set('alignstack', 'alwaysinline', 'inlinehint', 'naked', 'noimplicitfloat', 'noinline', 'alwaysinline attribute.', 'noredzone', 'noreturn', 'nounwind', 'optsize', 'readnone', 'readonly', 'ssp', 'sspreq'),
  SHIFTS: set('ashr', 'lshr', 'shl'),
  PHI_REACHERS: set('branch', 'switch', 'invoke'),
  EXTENDS: set('sext', 'zext'),
  INTRINSICS_32: set('_llvm_memcpy_p0i8_p0i8_i64', '_llvm_memmove_p0i8_p0i8_i64', '_llvm_memset_p0i8_i64'), // intrinsics that need args converted to i32 in I64_MODE 1
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
  globals: {}
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

  needAnalysis: {} // Types noticed during parsing, that need analysis
};

var Functions = {
  // The list of function datas which are being processed in the jsifier, currently
  currFunctions: [],

  // All functions that will be implemented in this file
  implementedFunctions: null,

  // All the function idents seen so far
  allIdents: [],

  indexedFunctions: {},
  nextIndex: 2, // Start at a non-0 (even, see below) value

  // Mark a function as needing indexing, and returns the index
  getIndex: function(ident) {
    var ret = this.indexedFunctions[ident];
    if (!ret) {
      ret = this.nextIndex;
      this.nextIndex += 2; // Need to have indexes be even numbers, see |polymorph| test
      this.indexedFunctions[ident] = ret;
    }
    return ret.toString();
  },

  // Generate code for function indexing
  generateIndexing: function() {
    var vals = zeros(this.nextIndex);
    for (var ident in this.indexedFunctions) {
      vals[this.indexedFunctions[ident]] = ident;
    }

    // Resolve multi-level aliases all the way down
    for (var i = 0; i < vals.length; i++) {
      while (1) {
        var varData = Variables.globals[vals[i]];
        if (!(varData && varData.resolvedAlias)) break;
        vals[i] = vals[varData.resolvedAlias];
      }
    }

    var indices = vals.toString().replace('"', '');
    if (BUILD_AS_SHARED_LIB) {
      // Shared libraries reuse the parent's function table.
      return 'FUNCTION_TABLE = FUNCTION_TABLE.concat([' + indices + ']);';
    } else {
      return 'FUNCTION_TABLE = [' + indices + ']; Module["FUNCTION_TABLE"] = FUNCTION_TABLE;';
    }
  }
};

var LibraryManager = {
  library: null,

  load: function() {
    assert(!this.library);

    for (var suffix in set('', '_sdl', '_browser', '_gl')) {
      eval(processMacros(preprocess(read('library' + suffix + '.js'))));
    }
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

