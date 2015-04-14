//"use strict";

// Convert analyzed data to javascript. Everything has already been calculated
// before this stage, which just does the final conversion to JavaScript.

// Handy sets

var STRUCT_LIST = set('struct', 'list');

var addedLibraryItems = {};
var asmLibraryFunctions = [];

var SETJMP_LABEL = -1;

var INDENTATION = ' ';

var functionStubSigs = {};

// JSifier
function JSify(data, functionsOnly) {
  //B.start('jsifier');
  var mainPass = !functionsOnly;

  var itemsDict = { type: [], GlobalVariableStub: [], functionStub: [], function: [], GlobalVariable: [], GlobalVariablePostSet: [] };

  if (mainPass) {
    var shellFile = SHELL_FILE ? SHELL_FILE : (BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'shell_sharedlib.js' : 'shell.js');

    if (phase == 'glue') {
      // We will start to print out the data, but must do so carefully - we are
      // dealing with potentially *huge* strings. Convenient replacements and
      // manipulations may create in-memory copies, and we may OOM.
      //
      // Final shape that will be created:
      //    shell
      //      (body)
      //        preamble
      //          runtime
      //        generated code
      //        postamble
      //          global_vars
      //
      // First, we print out everything until the generated code. Then the
      // functions will print themselves out as they are parsed. Finally, we
      // will call finalCombiner in the main pass, to print out everything
      // else. This lets us not hold any strings in memory, we simply print
      // things out as they are ready.

      var shellParts = read(shellFile).split('{{BODY}}');
      print(processMacros(preprocess(shellParts[0])));
      var preFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'preamble_sharedlib.js' : 'preamble.js';
      var pre = processMacros(preprocess(read(preFile).replace('{{RUNTIME}}', getRuntime())));
      print(pre);
    }
  }

  if (mainPass) {
    // Add additional necessary items for the main pass. We can now do this since types are parsed (types can be used through
    // generateStructInfo in library.js)
    //B.start('jsifier-libload');
    LibraryManager.load();
    //B.stop('jsifier-libload');

    if (phase == 'glue') {
      var libFuncsToInclude;
      if (INCLUDE_FULL_LIBRARY) {
        assert(!(BUILD_AS_SHARED_LIB || SIDE_MODULE), 'Cannot have both INCLUDE_FULL_LIBRARY and BUILD_AS_SHARED_LIB/SIDE_MODULE set.')
        libFuncsToInclude = [];
        for (var key in LibraryManager.library) {
          if (!key.match(/__(deps|postset|inline|asm|sig)$/)) {
            libFuncsToInclude.push(key);
          }
        }
      } else {
        libFuncsToInclude = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE;
      }
      libFuncsToInclude.forEach(function(ident) {
        var finalName = '_' + ident;
        if (ident[0] === '$') {
          finalName = ident.substr(1);
        }
        data.functionStubs.push({
          intertype: 'functionStub',
          finalName: finalName,
          ident: '_' + ident
        });
      });
    }
  }

  // Actors

  function makeEmptyStruct(type) {
    var ret = [];
    var typeData = Types.types[type];
    assertTrue(typeData);
    for (var i = 0; i < typeData.flatSize; i++) {
      ret.push(0);
    }
    return ret;
  }

  function alignStruct(values, type) {
    var typeData = Types.types[type];
    assertTrue(typeData);
    var ret = new Array(typeData.flatSize);
    var index = 0;
    var i = 0;
    while (i < values.length) {
      // Pad until the right place
      var padded = typeData.flatFactor ? typeData.flatFactor*i : typeData.flatIndexes[i];
      while (index < padded) {
        ret[index++] = 0;
      }
      // Add current value(s)
      var currValue = values[i];
      if ((typeData.fields[i] == 'i64' || (typeData.flatFactor && typeData.fields[0] == 'i64'))) {
        // 'flatten' out the 64-bit value into two 32-bit halves
        var parts = parseI64Constant(currValue, true);
        ret[index++] = parts[0];
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = parts[1];
        ret[index++] = 0;
        ret[index++] = 0;
        ret[index++] = 0;
      } else {
        currValue = flatten(currValue);
        if (typeof currValue == 'object') {
          for (var j = 0; j < currValue.length; j++) {
            ret[index++] = currValue[j];
          }
        } else {
          ret[index++] = currValue;
        }
      }
      i += 1;
    }
    while (index < typeData.flatSize) {
      ret[index++] = 0;
    }
    return ret;
  }

  // Gets an entire constant expression
  function makeConst(value, type, ident) {
    // Gets an array of constant items, separated by ',' tokens
    function handleSegments(tokens) {
      // Handle a single segment (after comma separation)
      function handleSegment(segment) {
        var ret;
        if (segment.intertype === 'value') {
          ret = segment.ident;
        } else if (segment.intertype === 'emptystruct') {
          ret = makeEmptyStruct(segment.type);
        } else if (segment.intertype in PARSABLE_LLVM_FUNCTIONS) {
          ret = finalizeLLVMFunctionCall(segment);
        } else if (segment.intertype in STRUCT_LIST) {
          ret = alignStruct(handleSegments(segment.contents), segment.type);
        } else if (segment.intertype === 'string') {
          ret = parseLLVMString(segment.text); // + ' /* ' + text + '*/';
        } else if (segment.intertype === 'blockaddress') {
          ret = finalizeBlockAddress(segment);
        } else {
          throw 'Invalid segment: ' + dump(segment);
        }
        assert(segment.type, 'Missing type for constant segment!');
        return makeGlobalUse(indexizeFunctions(ret, segment.type));
      };
      return tokens.map(handleSegment)
    }

    //dprint('jsifier const: ' + JSON.stringify(value) + ',' + type + '\n');
    if (value.intertype in PARSABLE_LLVM_FUNCTIONS) {
      return [finalizeLLVMFunctionCall(value)];
    } else if (Compiletime.isNumberType(type) || pointingLevels(type) >= 1) {
      return [makeGlobalUse(indexizeFunctions(parseNumerical(value.value), type))];
    } else if (value.intertype === 'emptystruct') {
      return makeEmptyStruct(type);
    } else if (value.intertype === 'string') {
      return parseLLVMString(value.text);
    } else {
      return alignStruct(handleSegments(value.contents), type);
    }
  }

  function parseConst(value, type, ident) {
    var constant = makeConst(value, type);
    // Sadly, we've thrown away type information in makeConst, so we're not
    // passing correct type info to parseNumerical which works around this
    // lack.
    constant = flatten(constant).map(function(x) { return parseNumerical(x) })
    return constant;
  }

  // globalVariable
  function globalVariableHandler(item) {

    function needsPostSet(value) {
      if (typeof value !== 'string') return false;
      // (' is ok, as it is something we can indexize later into a concrete int: ('{{ FI_ ...
      return /^([(_][^']|CHECK_OVERFLOW|GLOBAL).*/.test(value);
    }

    item.intertype = 'GlobalVariableStub';
    itemsDict.GlobalVariableStub.push(item);
    assert(!item.lines); // FIXME remove this, after we are sure it isn't needed
    if (item.ident == '_llvm_global_ctors') {
      item.JS = '\n/* global initializers */ __ATINIT__.push(' +
                  item.ctors.map(function(ctor) { return '{ func: function() { ' + ctor + '() } }' }).join(',') +
                ');\n';
      return;
    }

    var constant = null;
    var allocator = (BUILD_AS_SHARED_LIB && !item.external) ? 'ALLOC_NORMAL' : 'ALLOC_STATIC';
    var index = null;
    if (item.external && BUILD_AS_SHARED_LIB) {
      // External variables in shared libraries should not be declared as
      // they would shadow similarly-named globals in the parent.
      item.JS = '';
    } else {
      item.JS = makeGlobalDef(item.ident);
    }

    if (!NAMED_GLOBALS && isIndexableGlobal(item.ident)) {
      index = makeGlobalUse(item.ident); // index !== null indicates we are indexing this
      allocator = 'ALLOC_NONE';
    }

    Variables.globals[item.ident].named = item.named;

    if (ASM_JS && (MAIN_MODULE || SIDE_MODULE) && !item.private_ && !NAMED_GLOBALS && isIndexableGlobal(item.ident)) {
      // We need this to be named (and it normally would not be), so that it can be linked to and used from other modules
      Variables.globals[item.ident].linkable = 1;
    }

    if (isBSS(item)) {
      // If using indexed globals, go ahead and early out (no need to explicitly
      // initialize).
      if (!NAMED_GLOBALS) return;

      // If using named globals, we can at least shorten the call to allocate by
      // passing an integer representing the size of memory to alloc instead of
      // an array of 0s of size length.
      constant = Runtime.alignMemory(calcAllocatedSize(item.type));
    } else {
      if (item.external) {
        if (LibraryManager.library[item.ident.slice(1)]) {
          constant = LibraryManager.library[item.ident.slice(1)];
        } else if (Compiletime.isNumberType(item.type) || isPointerType(item.type)) {
          constant = zeros(Runtime.getNativeFieldSize(item.type));
        } else {
          constant = makeEmptyStruct(item.type);
        }
      } else {
        constant = parseConst(item.value, item.type, item.ident);
      }

      // This is a flattened object. We need to find its idents, so they can be assigned to later
      if (typeof constant === 'object') {
        var structTypes = null;
        constant.forEach(function(value, i) {
          if (needsPostSet(value)) { // ident, or expression containing an ident
            if (!structTypes) structTypes = generateStructTypes(item.type);
            itemsDict.GlobalVariablePostSet.push({
              intertype: 'GlobalVariablePostSet',
              JS: makeSetValue(makeGlobalUse(item.ident), i, value, structTypes[i], false, true) + ';' // ignore=true, since e.g. rtti and statics cause lots of safe_heap errors
            });
            constant[i] = '0';
          } else {
            if (typeof value === 'string') constant[i] = deParenCarefully(value);
          }
        });
      }

      if (item.external) {
        // External variables in shared libraries should not be declared as
        // they would shadow similarly-named globals in the parent, so do nothing here.
        if (BUILD_AS_SHARED_LIB) return;
        if (SIDE_MODULE) {
          itemsDict.GlobalVariableStub.pop(); // remove this item
          return;
        }
        // Library items need us to emit something, but everything else requires nothing.
        if (!LibraryManager.library[item.ident.slice(1)]) return;
      }

      // ensure alignment
      if (typeof constant === 'object') {
        var extra = Runtime.alignMemory(constant.length) - constant.length;
        if (item.ident.substr(0, 5) == '__ZTV') extra += Runtime.alignMemory(QUANTUM_SIZE);
        while (extra-- > 0) constant.push(0);
      }
    }

    // NOTE: This is the only place that could potentially create static
    //       allocations in a shared library.
    if (typeof constant !== 'string') {
      constant = makePointer(constant, null, allocator, item.type, index);
    }

    var js = (index !== null ? '' : item.ident + '=') + constant;
    if (js) js += ';';

    if (BUILD_AS_SHARED_LIB == 2 && !item.private_) {
      // TODO: make the assert conditional on ASSERTIONS
      js += 'if (globalScope) { assert(!globalScope["' + item.ident + '"]); globalScope["' + item.ident + '"] = ' + item.ident + ' }';
    }
    if (item.external && !NAMED_GLOBALS) {
      js = 'var ' + item.ident + '=' + js; // force an explicit naming, even if unnamed globals, for asm forwarding
    }
    itemsDict.GlobalVariableStub.push({
      intertype: 'GlobalVariable',
      JS: js,
    });
  }

  function processLibraryFunction(snippet, ident, finalName) {
    snippet = snippet.toString();
    assert(snippet.indexOf('XXX missing C define') == -1,
           'Trying to include a library function with missing C defines: ' + finalName + ' | ' + snippet);

    // name the function; overwrite if it's already named
    snippet = snippet.replace(/function(?:\s+([^(]+))?\s*\(/, 'function ' + finalName + '(');
    if (LIBRARY_DEBUG && !LibraryManager.library[ident + '__asm']) {
      snippet = snippet.replace('{', '{ var ret = (function() { if (Runtime.debug) Module.printErr("[library call:' + finalName + ': " + Array.prototype.slice.call(arguments).map(Runtime.prettyPrint) + "]"); ');
      snippet = snippet.substr(0, snippet.length-1) + '}).apply(this, arguments); if (Runtime.debug && typeof ret !== "undefined") Module.printErr("  [     return:" + Runtime.prettyPrint(ret)); return ret; \n}';
    }
    return snippet;
  }

  // functionStub
  function functionStubHandler(item) {
    // note the signature
    if (item.returnType && item.params) {
      functionStubSigs[item.ident] = Functions.getSignature(item.returnType.text, item.params.map(function(arg) { return arg.type }), false);
    }

    function addFromLibrary(ident, notDep) {
      if (ident in addedLibraryItems) return '';
      addedLibraryItems[ident] = true;

      // dependencies can be JS functions, which we just run
      if (typeof ident == 'function') return ident();

      // $ident's are special, we do not prefix them with a '_'.
      if (ident[0] === '$') {
        var finalName = ident.substr(1);
      } else {
        var finalName = '_' + ident;
      }

      // Don't replace implemented functions with library ones (which can happen when we add dependencies).
      // Note: We don't return the dependencies here. Be careful not to end up where this matters
      if (finalName in Functions.implementedFunctions) return '';

      if (!LibraryManager.library.hasOwnProperty(ident) && !LibraryManager.library.hasOwnProperty(ident + '__inline')) {
        if (notDep) {
          if (VERBOSE || ident.substr(0, 11) !== 'emscripten_') { // avoid warning on emscripten_* functions which are for internal usage anyhow
            if (ERROR_ON_UNDEFINED_SYMBOLS) error('unresolved symbol: ' + ident);
            else if (VERBOSE || (WARN_ON_UNDEFINED_SYMBOLS && !LINKABLE)) warn('unresolved symbol: ' + ident);
          }
        }
        // emit a stub that will fail at runtime
        LibraryManager.library[shortident] = new Function("Module['printErr']('missing function: " + shortident + "'); abort(-1);");
      }

      var snippet = LibraryManager.library[ident];
      var redirectedIdent = null;
      var deps = LibraryManager.library[ident + '__deps'] || [];
      deps.forEach(function(dep) {
        if (typeof snippet === 'string' && !(dep in LibraryManager.library)) warn('missing library dependency ' + dep + ', make sure you are compiling with the right options (see #ifdefs in src/library*.js)');
      });
      var isFunction = false;

      if (typeof snippet === 'string') {
        var target = LibraryManager.library[snippet];
        if (target) {
          // Redirection for aliases. We include the parent, and at runtime make ourselves equal to it.
          // This avoid having duplicate functions with identical content.
          redirectedIdent = snippet;
          deps.push(snippet);
          snippet = '_' + snippet;
        }
        // In asm, we need to know about library functions. If there is a target, though, then no
        // need to consider this a library function - we will call directly to it anyhow
        if (ASM_JS && !redirectedIdent && (typeof target == 'function' || /Math_\w+/.exec(snippet))) {
          Functions.libraryFunctions[finalName] = 1;
        }
      } else if (typeof snippet === 'object') {
        snippet = stringifyWithFunctions(snippet);
      } else if (typeof snippet === 'function') {
        isFunction = true;
        snippet = processLibraryFunction(snippet, ident, finalName);
        if (ASM_JS) Functions.libraryFunctions[finalName] = 1;
      }

      var postsetId = ident + '__postset';
      var postset = LibraryManager.library[postsetId];
      if (postset && !addedLibraryItems[postsetId] && !SIDE_MODULE) {
        addedLibraryItems[postsetId] = true;
        itemsDict.GlobalVariablePostSet.push({
          intertype: 'GlobalVariablePostSet',
          JS: postset
        });
      }

      if (redirectedIdent) {
        deps = deps.concat(LibraryManager.library[redirectedIdent + '__deps'] || []);
      }
      if (ASM_JS) {
        // In asm, dependencies implemented in C might be needed by JS library functions.
        // We don't know yet if they are implemented in C or not. To be safe, export such
        // special cases.
        [LIBRARY_DEPS_TO_AUTOEXPORT].forEach(function(special) {
          deps.forEach(function(dep) {
            if (dep == special && !EXPORTED_FUNCTIONS[dep]) {
              EXPORTED_FUNCTIONS[dep] = 1;
            }
          });
        });
      }
      if (VERBOSE) printErr('adding ' + finalName + ' and deps ' + deps + ' : ' + (snippet + '').substr(0, 40));
      var depsText = (deps ? '\n' + deps.map(addFromLibrary).filter(function(x) { return x != '' }).join('\n') : '');
      var contentText = isFunction ? snippet : ('var ' + finalName + '=' + snippet + ';');
      if (ASM_JS) {
        var sig = LibraryManager.library[ident + '__sig'];
        if (isFunction && sig && LibraryManager.library[ident + '__asm']) {
          // asm library function, add it as generated code alongside the generated code
          Functions.implementedFunctions[finalName] = sig;
          asmLibraryFunctions.push(contentText);
          contentText = ' ';
          EXPORTED_FUNCTIONS[finalName] = 1;
          Functions.libraryFunctions[finalName] = 2;
        }
      }
      if (SIDE_MODULE) return ';'; // we import into the side module js library stuff from the outside parent 
      if ((phase == 'glue') &&
          (EXPORT_ALL || (finalName in EXPORTED_FUNCTIONS))) {
        contentText += '\nModule["' + finalName + '"] = ' + finalName + ';';
      }
      return depsText + contentText;
    }

    itemsDict.functionStub.push(item);
    var shortident = item.ident.substr(1);
    if (BUILD_AS_SHARED_LIB) {
      // Shared libraries reuse the runtime of their parents.
      item.JS = '';
    } else {
      // If this is not linkable, anything not in the library is definitely missing
      if (item.ident in DEAD_FUNCTIONS) {
        if (LibraryManager.library[shortident + '__asm']) {
          warn('cannot kill asm library function ' + item.ident);
        } else {
          LibraryManager.library[shortident] = new Function("Module['printErr']('dead function: " + shortident + "'); abort(-1);");
          delete LibraryManager.library[shortident + '__inline'];
          delete LibraryManager.library[shortident + '__deps'];
        }
      }
      item.JS = addFromLibrary(shortident, true);
    }
  }

  // function for filtering functions for label debugging
  if (LABEL_FUNCTION_FILTERS.length > 0) {
    var LABEL_FUNCTION_FILTER_SET = set(LABEL_FUNCTION_FILTERS);
    var finalNameFilterTest = function(ident) {
      return (ident in LABEL_FUNCTION_FILTER_SET);
    };
  } else {
    // no filters are specified, all function names are printed
    var finalNameFilterTest = function(ident) {
      return true;
    }
  }

  // Final combiner

  function finalCombiner() {
    var splitPostSets = splitter(itemsDict.GlobalVariablePostSet, function(x) { return x.ident && x.dependencies });
    itemsDict.GlobalVariablePostSet = splitPostSets.leftIn;
    var orderedPostSets = splitPostSets.splitOut;

    var limit = orderedPostSets.length * orderedPostSets.length;
    for (var i = 0; i < orderedPostSets.length; i++) {
      for (var j = i+1; j < orderedPostSets.length; j++) {
        if (orderedPostSets[j].ident in orderedPostSets[i].dependencies) {
          var temp = orderedPostSets[i];
          orderedPostSets[i] = orderedPostSets[j];
          orderedPostSets[j] = temp;
          i--;
          limit--;
          assert(limit > 0, 'Could not sort postsets!');
          break;
        }
      }
    }

    itemsDict.GlobalVariablePostSet = itemsDict.GlobalVariablePostSet.concat(orderedPostSets);

    //

    if (!mainPass) {
      if ((phase == 'glue') && !Variables.generatedGlobalBase && !BUILD_AS_SHARED_LIB) {
        Variables.generatedGlobalBase = true;
        // Globals are done, here is the rest of static memory
        if (!SIDE_MODULE) {
          print('STATIC_BASE = ' + Runtime.GLOBAL_BASE + ';\n');
          print('STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n');
        } else {
          print('H_BASE = parentModule["_malloc"](' + Runtime.alignMemory(Variables.nextIndexedOffset) + ' + Runtime.GLOBAL_BASE);\n');
          print('// STATICTOP = STATIC_BASE + ' + Runtime.alignMemory(Variables.nextIndexedOffset) + ';\n'); // comment as metadata only
        }
      }
      var generated = itemsDict.function.concat(itemsDict.type).concat(itemsDict.GlobalVariableStub).concat(itemsDict.GlobalVariable);
      print(generated.map(function(item) { return item.JS; }).join('\n'));

      if (phase == 'glue') {
        if (memoryInitialization.length > 0) {
          // apply postsets directly into the big memory initialization
          itemsDict.GlobalVariablePostSet = itemsDict.GlobalVariablePostSet.filter(function(item) {
            var m;
            if (m = /^HEAP([\dFU]+)\[([()>\d]+)\] *= *([()|\d{}\w_' ]+);?$/.exec(item.JS)) {
              var type = getTypeFromHeap(m[1]);
              var bytes = Runtime.getNativeTypeSize(type);
              var target = eval(m[2]) << log2(bytes);
              var value = m[3];
              try {
                value = eval(value);
              } catch(e) {
                // possibly function table {{{ FT_* }}} etc.
                if (value.indexOf('{{ ') < 0) return true;
              }
              writeInt8s(memoryInitialization, target - Runtime.GLOBAL_BASE, value, type);
              return false;
            }
            return true;
          });
          // write out the singleton big memory initialization value
          print('/* memory initializer */ ' + makePointer(memoryInitialization, null, 'ALLOC_NONE', 'i8', 'Runtime.GLOBAL_BASE' + (SIDE_MODULE ? '+H_BASE' : ''), true));
        } else if (phase !== 'glue') {
          print('/* no memory initializer */'); // test purposes
        }

        if (phase !== 'glue') {
          // Define postsets. These will be run in ATINIT, right before global initializers (which might need the postsets). We cannot
          // run them now because the memory initializer might not have been applied yet.
          print('function runPostSets() {\n');
          print(itemsDict.GlobalVariablePostSet.map(function(item) { return item.JS }).join('\n'));
          print('}\n');
        }

        if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
          print('var tempDoublePtr = Runtime.alignMemory(allocate(12, "i8", ALLOC_STATIC), 8);\n');
          print('assert(tempDoublePtr % 8 == 0);\n');
          print('function copyTempFloat(ptr) { // functions, because inlining this code increases code size too much\n');
          print('  HEAP8[tempDoublePtr] = HEAP8[ptr];\n');
          print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];\n');
          print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];\n');
          print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];\n');
          print('}\n');
          print('function copyTempDouble(ptr) {\n');
          print('  HEAP8[tempDoublePtr] = HEAP8[ptr];\n');
          print('  HEAP8[tempDoublePtr+1] = HEAP8[ptr+1];\n');
          print('  HEAP8[tempDoublePtr+2] = HEAP8[ptr+2];\n');
          print('  HEAP8[tempDoublePtr+3] = HEAP8[ptr+3];\n');
          print('  HEAP8[tempDoublePtr+4] = HEAP8[ptr+4];\n');
          print('  HEAP8[tempDoublePtr+5] = HEAP8[ptr+5];\n');
          print('  HEAP8[tempDoublePtr+6] = HEAP8[ptr+6];\n');
          print('  HEAP8[tempDoublePtr+7] = HEAP8[ptr+7];\n');
          print('}\n');
        }
      }

      return;
    }

    // Print out global variables and postsets TODO: batching
    if (phase == 'glue') {
      var legalizedI64sDefault = legalizedI64s;
      legalizedI64s = false;

      var globalsData = {globalVariables: {}, functionStubs: []}
      JSify(globalsData, true);
      globalsData = null;

      var generated = itemsDict.functionStub.concat(itemsDict.GlobalVariablePostSet);
      generated.forEach(function(item) { print(indentify(item.JS || '', 2)); });

      legalizedI64s = legalizedI64sDefault;

      if (!BUILD_AS_SHARED_LIB && !SIDE_MODULE) {
        print('STACK_BASE = STACKTOP = Runtime.alignMemory(STATICTOP);\n');
        print('staticSealed = true; // seal the static portion of memory\n');
        print('STACK_MAX = STACK_BASE + TOTAL_STACK;\n');
        print('DYNAMIC_BASE = DYNAMICTOP = Runtime.alignMemory(STACK_MAX);\n');
        print('assert(DYNAMIC_BASE < TOTAL_MEMORY, "TOTAL_MEMORY not big enough for stack");\n');
      }

      if (asmLibraryFunctions.length > 0) {
        print('// ASM_LIBRARY FUNCTIONS');
        function fix(f) { // fix indenting to not confuse js optimizer
          f = f.substr(f.indexOf('f')); // remove initial spaces before 'function'
          f = f.substr(0, f.lastIndexOf('\n')+1); // remove spaces and last }  XXX assumes function has multiple lines
          return f + '}'; // add unindented } to match function
        }
        print(asmLibraryFunctions.map(fix).join('\n'));
      }
    }

    if (abortExecution) throw 'Aborting compilation due to previous errors';

    // This is the main 'post' pass. Print out the generated code that we have here, together with the
    // rest of the output that we started to print out earlier (see comment on the
    // "Final shape that will be created").
    if (PRECISE_I64_MATH && Types.preciseI64MathUsed) {
      if (!INCLUDE_FULL_LIBRARY && !SIDE_MODULE && !BUILD_AS_SHARED_LIB) {
        // first row are utilities called from generated code, second are needed from fastLong
        ['i64Add', 'i64Subtract', 'bitshift64Shl', 'bitshift64Lshr', 'bitshift64Ashr',
         'llvm_cttz_i32'].forEach(function(ident) {
          var finalName = '_' + ident;
          if (!Functions.libraryFunctions[finalName] || (phase == 'glue' && ident[0] === 'l' && !addedLibraryItems[ident])) { // TODO: one-by-one in fastcomp glue mode
            print(processLibraryFunction(LibraryManager.library[ident], ident, finalName)); // must be first to be close to generated code
            Functions.implementedFunctions[finalName] = LibraryManager.library[ident + '__sig'];
            Functions.libraryFunctions[finalName] = phase == 'glue' ? 2 : 1; // XXX
            // limited dependency handling
            var deps = LibraryManager.library[ident + '__deps'];
            if (deps) {
              deps.forEach(function(dep) {
                assert(typeof dep == 'function');
                var text = dep();
                assert(text.indexOf('\n') < 0);
                text = text.replace('ALLOC_STATIC', 'ALLOC_DYNAMIC');
                print('/* PRE_ASM */ ' + text + '\n');
              });
            }
          }
        });
      }
      // these may be duplicated in side modules and the main module without issue
      print(read('fastLong.js'));
      print('// EMSCRIPTEN_END_FUNCS\n');
      print(read('long.js'));
    } else {
      print('// EMSCRIPTEN_END_FUNCS\n');
      print('// Warning: printing of i64 values may be slightly rounded! No deep i64 math used, so precise i64 code not included');
      print('var i64Math = null;');
    }

    if (CORRUPTION_CHECK) {
      assert(!ASM_JS, 'corruption checker is not compatible with asm.js');
      print(processMacros(read('corruptionCheck.js')));
    }
    if (HEADLESS) {
      print('if (!ENVIRONMENT_IS_WEB) {');
      print(read('headlessCanvas.js'));
      print('\n');
      print(read('headless.js').replace("'%s'", "'http://emscripten.org'").replace("'?%s'", "''").replace("'?%s'", "'/'").replace('%s,', 'null,').replace('%d', '0'));
      print('}');
    }
    if (PROXY_TO_WORKER) {
      print('if (ENVIRONMENT_IS_WORKER) {\n');
      print(read('webGLWorker.js'));
      print(read('proxyWorker.js'));
      print('}');
    }
    if (DETERMINISTIC) {
      print(read('deterministic.js'));
    }
    if (RUNTIME_TYPE_INFO) {
      Types.cleanForRuntime();
      print('Runtime.typeInfo = ' + JSON.stringify(Types.types));
      print('Runtime.structMetadata = ' + JSON.stringify(Types.structMetadata));
    }
    var postFile = BUILD_AS_SHARED_LIB || SIDE_MODULE ? 'postamble_sharedlib.js' : 'postamble.js';
    var postParts = processMacros(preprocess(read(postFile))).split('{{GLOBAL_VARS}}');
    print(postParts[0]);

    Functions.generateIndexing(); // done last, as it may rely on aliases set in postsets

    // Load runtime-linked libraries
    RUNTIME_LINKED_LIBS.forEach(function(lib) {
      print('eval(Module["read"]("' + lib + '"))(' + Functions.getTable('x') + '.length, this);');
    });

    print(postParts[1]);

    var shellParts = read(shellFile).split('{{BODY}}');
    print(processMacros(preprocess(shellParts[1])));
    // Print out some useful metadata
    if (RUNNING_JS_OPTS || PGO) {
      var generatedFunctions = JSON.stringify(keys(Functions.implementedFunctions));
      if (PGO) {
        print('PGOMonitor.allGenerated = ' + generatedFunctions + ';\nremoveRunDependency("pgo");\n');
      }
      if (RUNNING_JS_OPTS) {
        print('// EMSCRIPTEN_GENERATED_FUNCTIONS: ' + generatedFunctions + '\n');
      }
    }

    PassManager.serialize();
  }

  // Data

  if (mainPass) {
    data.functionStubs.forEach(functionStubHandler);
  } else {
    //B.start('jsifier-handle-gv');
    sortGlobals(data.globalVariables).forEach(globalVariableHandler);
    //B.stop('jsifier-handle-gv');
  }

  //B.start('jsifier-fc');
  finalCombiner();
  //B.stop('jsifier-fc');

  dprint('framework', 'Big picture: Finishing JSifier, main pass=' + mainPass);
  //B.stop('jsifier');
}

