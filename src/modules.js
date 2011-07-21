// Various namespace-like modules

var LLVM_STYLE = null;

var LLVM = {
  LINKAGES: set('private', 'linker_private', 'linker_private_weak', 'linker_private_weak_def_auto', 'internal',
                'available_externally', 'linkonce', 'common', 'weak', 'appending', 'extern_weak', 'linkonce_odr',
                'weak_odr', 'externally_visible', 'dllimport', 'dllexport', 'unnamed_addr'),
  VISIBILITIES: set('default', 'hidden', 'protected'),
  PARAM_ATTR: set('noalias', 'signext', 'zeroext', 'inreg', 'sret', 'nocapture', 'nest'),
  CALLING_CONVENTIONS: set('ccc', 'fastcc', 'coldcc', 'cc10')
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
    var structToSize = {};
    var memberMetaToMembers = {};
    var metadataToMember = {};

    var form1 = new RegExp(/^  .*, !dbg !(\d+) *$/);
    var form2 = new RegExp(/^  .*, !dbg !(\d+) *; .*$/);
    var form3 = new RegExp(/^!(\d+) = metadata !{i32 (\d+), i32 \d+, metadata !(\d+), .*}$/);
    var form3a = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !\d+, i32 \d+, i32 \d+, metadata !(\d+), i32 \d+} ; \[ DW_TAG_lexical_block \]$/);
    var form3ab = new RegExp(/^!(\d+) = metadata !{i32 \d+, i32 \d+, metadata !(\d+), .*$/);
    var form3ac = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !\d+, metadata !"[^"]+", metadata !(\d+)[^\[]* ; \[ DW_TAG_.*$/);
    var form3b = new RegExp(/^!(\d+) = metadata !{i32 \d+, metadata !"([^"]+)", metadata !"([^"]+)", metadata !\d+} ; \[ DW_TAG_file_type \]$/);
    var form3c = new RegExp(/^!(\d+) = metadata !{\w+\d* !?(\d+)[^\d].*$/);
    var form4 = new RegExp(/^!llvm.dbg.[\w\.]+ = .*$/);
    var form5 = new RegExp(/^!(\d+) = metadata !{.*$/);
    var form6 = new RegExp(/^  (tail )?call void \@llvm.dbg.\w+\(metadata .*$/);
    var formStruct = /^!(\d+) = metadata !\{i32 \d+, metadata !\d+, metadata !"([^"]+)", metadata !\d+, i32 \d+, i64 (\d+), [^,]*, [^,]*, [^,]*, [^,]*, metadata !(\d+), .*} ; \[ DW_TAG_(?:structure|class)_type \]$/;
    var formStructMembers = /^!(\d+) = metadata !\{(.*)\}$/;
    var formMember = /^!(\d+) = metadata !\{i32 \d+, metadata !\d+, metadata !"([^"]+)", metadata !\d+, i32 \d+, i64 (\d+), i64 \d+, i64 (\d+), .+?, metadata !(\d+)} ; \[ DW_TAG_member \]$/;

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
        structMetaToStruct[calc[1]] = calc[2];
        structToSize[calc[2]] = calc[3];
        structToMemberMeta[calc[2]] = calc[4];
        memberMetaToStruct[calc[4]] = calc[1];
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
      calc = form3a.exec(line) || form3ab.exec(line) || form3ac.exec(line);
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
      //dprint('starting to recurse metadata for: ' + m);
      while (!metadataToFilename[m]) {
        //dprint('recursing metadata, at: ' + m);
        m = metadataToParentMetadata[m];
        assert(m, 'Confused as to parent metadata for llvm #' + l + ', metadata !' + m);
      }
      this.llvmLineToSourceFile[l] = metadataToFilename[m];
    }

    // Create base struct definitions.
    for (var structName in structToMemberMeta) {
      // TODO: Account for bitfields.
      Types.structDefinitions[structName] = {
        size: parseInt(structToSize[structName]) / 8,
        members: {}
      }
    }
    // Fill struct members.
    for (var structName in structToMemberMeta) {
      var struct = Types.structDefinitions[structName];
      var memberMetaId = structToMemberMeta[structName];
      var memberIds = memberMetaToMembers[memberMetaId];
      for (var i = 0; i < memberIds.length; i++) {
        if (memberIds[i] in metadataToMember) {
          var member = metadataToMember[memberIds[i]];
          var memberObj = {
            size: parseInt(member[1]) / 8,
            offset: parseInt(member[2]) / 8
          }
          if (member[3] in structMetaToStruct) {
            var subStruct = Types.structDefinitions[structMetaToStruct[member[3]]];
            memberObj.members = subStruct.members;
          }
          struct.members[member[0]] = memberObj;
        }
      }
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
    var sourceFile = this.llvmLineToSourceFile[lineNum];
    if (!sourceFile) return null;
    return sourceFile.split('/').slice(-1)[0] + ':' + this.llvmLineToSourceLine[lineNum];
  }
};

var Types = {
  types: {},
  fatTypes: {}, // With QUANTUM_SIZE=1, we store the full-size type data here
  flipTypes: function() {
    var temp = this.fatTypes;
    this.fatTypes = this.types;
    this.types = temp;
  },
  structDefinitions: {},

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
      return 'var FUNCTION_TABLE = [' + indices + '];';
    }
  }
};

var LibraryManager = {
  library: null,

  load: function() {
    assert(!this.library);

    for (suffix in set('', '_sdl', '_gl', '_browser')) {
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
  }
};

