// Various namespace-like modules

var LLVM_STYLE = null;

var LLVM = {
  LINKAGES: set('private', 'linker_private', 'linker_private_weak', 'linker_private_weak_def_auto', 'internal',
                'available_externally', 'linkonce', 'common', 'weak', 'appending', 'extern_weak', 'linkonce_odr',
                'weak_odr', 'externally_visible', 'dllimport', 'dllexport', 'unnamed_addr'),
  PARAM_ATTR: set('noalias', 'signext', 'zeroext', 'inreg', 'byval', 'sret', 'nocapture', 'nest'),
  CALLING_CONVENTIONS: set('ccc', 'fastcc', 'coldcc', 'cc10')
};
LLVM.GLOBAL_MODIFIERS = set(keys(LLVM.LINKAGES).concat(['constant', 'global', 'hidden']));

var Debugging = {
  processMetadata: function(lines) {
    var llvmLineToMetadata = {};
    var metadataToSourceLine = {};
    var metadataToParentMetadata = {};
    var metadataToFilename = {};

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

    var debugComment = new RegExp(/; +\[debug line = \d+:\d+\]/);

    var ret = lines.map(function(line, i) {
      line = line.replace(debugComment, '');

      if (form6.exec(line)) return ';';

      var calc = form1.exec(line) || form2.exec(line);
      if (calc) {
        llvmLineToMetadata[i+1] = calc[1];
        return line.replace(', !dbg !' + calc[1], '');
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
      return line;
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
    return 'var FUNCTION_TABLE = [' + this.indexedFunctions.toString().replace('"', '') + '];';
  }
};

