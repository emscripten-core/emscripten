// Various namespace-like modules

var LLVM_STYLE = null;

var LLVM = {
  LINKAGES: set('private', 'linker_private', 'linker_private_weak', 'linker_private_weak_def_auto', 'internal',
                'available_externally', 'linkonce', 'common', 'weak', 'appending', 'extern_weak', 'linkonce_odr',
                'weak_odr', 'externally_visible', 'dllimport', 'dllexport', 'unnamed_addr'),
  VISIBILITIES: set('default', 'hidden', 'protected'),
  PARAM_ATTR: set('noalias', 'signext', 'zeroext', 'inreg', 'sret', 'nocapture', 'nest'),
  CALLING_CONVENTIONS: set('ccc', 'fastcc', 'coldcc', 'cc10', 'x86_fastcallcc', 'x86_stdcallcc'),
  ACCESS_OPTIONS: set('volatile', 'atomic'),
  INVOKE_MODIFIERS: set('alignstack', 'alwaysinline', 'inlinehint', 'naked', 'noimplicitfloat', 'noinline', 'alwaysinline attribute.', 'noredzone', 'noreturn', 'nounwind', 'optsize', 'readnone', 'readonly', 'ssp', 'sspreq'),
  SHIFTS: set('ashr', 'lshr', 'shl'),
  INTRINSICS_32: set('_llvm_memcpy_p0i8_p0i8_i64'), // intrinsics that need args converted to i32 in I64_MODE 1
};
LLVM.GLOBAL_MODIFIERS = set(keys(LLVM.LINKAGES).concat(['constant', 'global', 'hidden']));

var Debugging = {
  processMetadata: function(lines) {
    var llvmLineToMetadata = {};
    var metadataToSourceLine = {};
    var metadataToParentMetadata = {};
    var metadataToFilename = {};

    var structToMemberMeta = {};
    var memberMetaToStruct = {};
    var structMetaToStruct = {};
    var memberMetaToMembers = {};
    var metadataToMember = {};

    var form1 = new RegExp(/^  .*, !dbg !(\d+) *$/);
    var form2 = new RegExp(/^  .*, !dbg !(\d+) *; .*$/);
    var form3 = new RegExp(/^!(\d+) = metadata !{i32 (\d+), (?:i32 \d+|null), metadata !(\d+), .*}$/);
    var form3a = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:i32 \d+|metadata !\d+), (?:i32 \d+|null), (?:i32 \d+|null), metadata !(\d+), (?:i32 \d+|null)}.*/);
    var form3ab = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:i32 \d+|null), metadata !(\d+), .*$/);
    var form3ac = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:metadata !\d+|null), metadata !"[^"]+", metadata !(\d+)[^\[]*.*$/);
    var form3ad = new RegExp(/^!(\d+) = metadata !{i32 \d+, (?:i32 \d+|null), (?:i32 \d+|null), metadata !"[^"]*", metadata !"[^"]*", metadata !"[^"]*", metadata !(\d+),.*$/);
    var form3b = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !"([^"]+)", metadata !"([^"]+)", (metadata !\d+|null)}.*$/);
    var form3c = new RegExp(/^!(\d+) = metadata !{\w+\d* !?(\d+)[^\d].*$/);
    var form4 = new RegExp(/^!llvm.dbg.[\w\.]+ = .*$/);
    var form5 = new RegExp(/^!(\d+) = metadata !{.*$/);
    var form6 = new RegExp(/^  (tail )?call void \@llvm.dbg.\w+\(metadata .*$/);
    var formStruct = /^!(\d+) = metadata !\{i32 \d+, (metadata !\d+|null), metadata !"([^"]+)", metadata !(\d+), (?:i32 \d+|null), i64 \d+, [^,]*, [^,]*, [^,]*, [^,]*, metadata !(\d+), .*}.*$/;
    var formStructMembers = /^!(\d+) = metadata !\{(.*)\}$/;
    var formMember = /^!(\d+) = metadata !\{i32 \d+, metadata !\d+, metadata !"([^"]+)", metadata !\d+, (?:i32 \d+|null), i64 \d+, i64 \d+, i64 \d+, .+?, metadata !(\d+)}.*$/;

    var debugComment = new RegExp(/; +\[debug line = \d+:\d+\]/);

    var ret = lines.map(function(line, i) {
      line = line.replace(debugComment, '');
      var skipLine = false;

      if (form6.exec(line)) return ';';

      var calc = form1.exec(line) || form2.exec(line);
      if (calc) {
        llvmLineToMetadata[i+1] = calc[1];
        return line.replace(', !dbg !' + calc[1], '');
      }
      calc = formStruct.exec(line);
      if (calc) {
        metadataToParentMetadata[calc[1]] = calc[4];
        if (!(calc[3] in structToMemberMeta)) {
          structMetaToStruct[calc[1]] = calc[3];
          structToMemberMeta[calc[3]] = calc[5];
          memberMetaToStruct[calc[5]] = calc[1];
        }
        skipLine = true;
      }
      calc = formStructMembers.exec(line);
      if (calc) {
        var children = calc[2].match(/!\d+/g) || [];
        memberMetaToMembers[calc[1]] = children.map(function(i) {
          return i.slice(1);
        });
        skipLine = true;
      }
      calc = formMember.exec(line);
      if (calc) {
        metadataToMember[calc[1]] = calc.slice(2);
        skipLine = true;
      }
      calc = form3.exec(line);
      if (calc) {
        metadataToSourceLine[calc[1]] = calc[2];
        metadataToParentMetadata[calc[1]] = calc[3];
        return ';'; // return an empty line, to keep line numbers of subsequent lines the same
      }
      calc = form3a.exec(line) || form3ab.exec(line) || form3ac.exec(line) || form3ad.exec(line);
      if (calc) {
        metadataToParentMetadata[calc[1]] = calc[2];
        return ';';
      }
      calc = form3b.exec(line);
      if (calc) {
        metadataToFilename[calc[1]] = /* LLVM 2.8<= : calc[3] + '/' + */ calc[2];
        return ';';
      }
      calc = form3c.exec(line) || form4.exec(line) || form5.exec(line);
      if (calc) return ';';
      return skipLine ? ';' : line;
    }, this);

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

    // Create struct definitions.
    // TODO: Account for bitfields.
    function generateStructDefinition(name) {
      if (!Types.structMetadata.hasOwnProperty(name)) {
        var struct = [];
        var membersMetaId = structToMemberMeta[name];
        var memberIds = memberMetaToMembers[membersMetaId];
        for (var i = 0; i < memberIds.length; i++) {
          var memberId = memberIds[i];
          if (memberId in metadataToMember) {
            var member = metadataToMember[memberId];
            if (member[1] in structMetaToStruct) {
              var def = generateStructDefinition(structMetaToStruct[member[1]]);
              var record = {};
              record[member[0]] = def;
              struct.push(record);
            } else {
              struct.push(member[0]);
            }
          }
        }
        Types.structMetadata[name] = struct;
      }
      return Types.structMetadata[name];
    }
    for (var name in structToMemberMeta) {
      generateStructDefinition(name);
    }

    this.on = true;

    return ret;
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

var Variables = {
  globals: null
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

  indexedFunctions: [0, 0], // Start at a non-0 (even, see below) value

  // Mark a function as needing indexing, and returns the index
  getIndex: function(ident) {
    var key = this.indexedFunctions.indexOf(ident);
    if (key < 0) {
      key = this.indexedFunctions.length;
      this.indexedFunctions[key] = ident;
      this.indexedFunctions[key+1] = 0; // Need to have keys be even numbers, see |polymorph| test
    }
    return key.toString();
  },

  // Generate code for function indexing
  generateIndexing: function() {
    var indices = this.indexedFunctions.toString().replace('"', '');
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

    for (var suffix in set('', '_sdl', '_browser')) {
      eval(processMacros(preprocess(read('library' + suffix + '.js'), CONSTANTS)));
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

