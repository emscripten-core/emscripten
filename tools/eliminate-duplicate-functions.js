// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Duplicate Function Elimination.
//
// This is a Javascript file that is used to post-process an Emscripten
// transpiled JS file.  It will remove all the duplicate functions from the
// generated ASM. In its current form, the input JS file is expected to be a
// 'chunk' from an Emscripten generated ASM.JS file.
//
// An ASM JS chunk consists of a number of ASM.JS function definitions. It can
// also represent the ASM JS 'shell' which consists of the global variable
// declarations for the generated ASM JS.
//
// The file will remove all the generated functions that are deemed to be
// identical. Currently, the file will only run one pass of the algorithm. The
// caller of this JS file can run multiple passes to ensure that higher level
// functions which will become identical after a pass can be further eliminated.
//
// Usually, 4 or at most 5 passes will result in an optimal reduction - i.e., in
// a file that cannot be reduced any further.

var crypto = require('crypto');
var uglify = require('../tools/eliminator/node_modules/uglify-js');

var nodeFS = require('fs');
var nodePath = require('path');
var debug = false;
var debugFile = undefined;
var debugFileName = 'function_eliminator.log';
var genHashInfo = false;
var useHashInfo = false;
var useAsmAst = false;

// Variables that helps control verbosity of debug spew
// Set appropriate zones here (to 0 or 1) for debugging various
// parts of the algorithm.
var ZONE_IDENTIFY_DUPLICATE_FUNCS = 1;
var ZONE_REPLACE_FUNCTION_REFERENCES = 1;
var ZONE_REPLACE_DUPLICATE_FUNCS = 1;
var ZONE_EQUIVALENT_FUNCTION_HASH = 1;
var ZONE_TOP_LEVEL = 1;
var ZONE_DUMP_AST = 0;

if (!nodeFS.existsSync) {
  nodeFS.existsSync = function(path) {
    try {
      return !!nodeFS.readFileSync(path);
    } catch (e) {
      return false;
    }
  }
}

function srcToAst(src) {
  return uglify.parser.parse(src, false, false);
}

function astToSrc(ast, minifyWhitespace) {
  return uglify.uglify.gen_code(ast, {
    debug: debug,
    ascii_only: true,
    beautify: !minifyWhitespace,
    indent_level: 1
  });
}

// Traverses the children of a node. If the traverse function returns an object,
// replaces the child. If it returns true, stop the traversal and return true.
function traverseChildren(node, traverse, pre, post) {
  for (var i = 0; i < node.length; i++) {
    var subnode = node[i];
    if (Array.isArray(subnode)) {
      var subresult = traverse(subnode, pre, post);
      if (subresult === true) return true;
      if (subresult !== null && typeof subresult === 'object') node[i] = subresult;
    }
  }
}

print = function(x) {
  process['stdout'].write(x + '\n');
};

printErr = function(x) {
  process['stderr'].write(x + '\n');
};

function debugLog(zone, str) {
  if (debug && (zone !== 0)) {
    nodeFS.writeSync(debugFile, str + '\n');
  }
}

// Traverses a JavaScript syntax tree rooted at the given node calling the given
// callback for each node.
//   @arg node: The root of the AST.
//   @arg pre: The pre to call for each node. This will be called with
//     the node as the first argument and its type as the second. If true is
//     returned, the traversal is stopped. If an object is returned,
//     it replaces the passed node in the tree. If null is returned, we stop
//     traversing the subelements (but continue otherwise).
//   @arg post: A callback to call after traversing all children.
//   @returns: If the root node was replaced, the new root node. If the traversal
//     was stopped, true. Otherwise undefined.
function traverse(node, pre, post) {
  var type = node[0],
    result, len;
  var relevant = typeof type === 'string';
  if (relevant) {
    var result = pre(node, type);
    if (result === true) return true;
    if (result && result !== null) node = result; // Continue processing on this node
  }
  if (result !== null) {
    if (traverseChildren(node, traverse, pre, post) === true) return true;
  }
  if (relevant) {
    if (post) {
      var postResult = post(node, type);
      result = result || postResult;
    }
  }
  return result;
}

function dumpAst(ast) {
  debugLog(ZONE_DUMP_AST, JSON.stringify(ast, null, '  '));
}

function getFunctionBody(node) {
  // Remove the function <name> part of the source for the function
  var functionSrc = astToSrc(node, true);
  var functionNameRegex = /(function .*?)\(/;
  return functionSrc.replace(functionNameRegex, "(");
}

function traverseFunctions(ast, callback) {
  var topLevelList = useAsmAst ? ast : ast[1];

  for (var listIndex = 0; listIndex < topLevelList.length; ++listIndex) {
    var node = topLevelList[listIndex];

    if (node[0] === 'defun') {
      callback(node);
    }
  }
}

function identifyDuplicateFunctions(ast) {
  debugLog(ZONE_TOP_LEVEL, "identifyDuplicateFunctions");

  var functionHashToFunctionName = {};

  traverseFunctions(ast, function(node) {
    debugLog(ZONE_IDENTIFY_DUPLICATE_FUNCS, "Node: " + node);
    var functionBody = getFunctionBody(node);

    debugLog(ZONE_IDENTIFY_DUPLICATE_FUNCS, "Function Body: " + functionBody + "\n");
    var functionHash = crypto.createHash('sha256').update(functionBody).digest('hex');

    if (functionHashToFunctionName[functionHash] === undefined) {
      functionHashToFunctionName[functionHash] = [];
    }

    debugLog(ZONE_IDENTIFY_DUPLICATE_FUNCS, typeof node[1]);
    functionHashToFunctionName[functionHash].push(node[1]);
    debugLog(ZONE_IDENTIFY_DUPLICATE_FUNCS, functionHash + '->' + node[1]);
  });

  if (debug) {
    for (var key in functionHashToFunctionName) {
      debugLog(ZONE_IDENTIFY_DUPLICATE_FUNCS, key + "->" + functionHashToFunctionName[key]);
    }
  }

  return functionHashToFunctionName;
}

function getVariableNames(ast) {
  var variableNames = {};
  traverse(ast, function(node, type) {
    if (type === 'var') {

      var vars = node[1];

      if (Array.isArray(vars)) {
        for (var i = 0; i < vars.length; i++) {
          var ident = vars[i][0];

          variableNames[ident] = 1;
        }
      }
    }
  });

  return variableNames;
}

function replaceFunctionDefinitions(ast, equivalentFunctionHash) {
  debugLog(ZONE_TOP_LEVEL, 'replaceFunctionDefinitions');

  var topLevelList = useAsmAst ? ast : ast[1];
  var indicesToRemove = [];
  for (var listIndex = 0; listIndex < topLevelList.length; ++listIndex) {
    var node = topLevelList[listIndex];

    if (node[0] === 'defun' && equivalentFunctionHash[node[1]] !== undefined) {
      indicesToRemove.push(listIndex);
    }
  }

  if (indicesToRemove.length > 0) {
    for (var i = indicesToRemove.length - 1; i >= 0; --i) {
      debugLog(ZONE_REPLACE_DUPLICATE_FUNCS, "Removing " + topLevelList[indicesToRemove[i]][1]);
      topLevelList.splice(indicesToRemove[i], 1);
    }
  }
}

function replaceFunctionReferences(ast, equivalentFunctionHash) {
  debugLog(ZONE_TOP_LEVEL, 'replaceFunctionReferences');
  traverse(ast, function(node, type) {
    if (type === 'call') {
      var functionName = node[1][1];

      // Replace the call with a call to the equivalent function if there is one
      if (equivalentFunctionHash[functionName] !== undefined) {
        node[1][1] = equivalentFunctionHash[functionName];
      }
    } else if (type === 'var') {
      var vars = node[1];
      for (var i = 0; i < vars.length; i++) {
        debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, 'Variable: ' + vars[i]);
        var value = vars[i][1][1];
        debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, 'Variable value: ' + value);

        if (equivalentFunctionHash[value] !== undefined) {
          debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, 'Variable value replacement: ' + equivalentFunctionHash[value]);
          vars[i][1][1] = equivalentFunctionHash[value];
        }
      }
    } else if (type === 'assign') {
      if (node[3][0] === 'name' && equivalentFunctionHash[node[3][1]] !== undefined) {
        node[3][1] = equivalentFunctionHash[node[3][1]];
      }
    } else if (type === 'object') {
      var assignments = node[1];

      for (var i = 0; i < assignments.length; i++) {
        debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, 'Object Value Assignment: ' + assignments[i][1][1]);

        if (equivalentFunctionHash[assignments[i][1][1]] !== undefined) {
          assignments[i][1][1] = equivalentFunctionHash[assignments[i][1][1]];
        }
      }
    } else if (type === 'array') {
      var arrayVars = node[1];

      if (Array.isArray(arrayVars)) {
        for (var i = 0; i < arrayVars.length; i++) {
          debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, "Array: " + arrayVars[i][0] + ", " + arrayVars[i][1]);
          // First element contains type, 2nd contains value
          if (arrayVars[i][0] == 'name' && equivalentFunctionHash[arrayVars[i][1]] !== undefined) {
            debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, "Replacing array value " + arrayVars[i][1]);
            arrayVars[i][1] = equivalentFunctionHash[arrayVars[i][1]];
          }
        }
      } else {
        debugLog(ZONE_REPLACE_FUNCTION_REFERENCES, "ArrayVars (not an array): " + arrayVars + ", node: " + node);
      }
    }
  });
}

function replaceDuplicateFuncs(ast, equivalentFunctionHash) {
  debugLog(ZONE_TOP_LEVEL, "replaceDuplicateFuncs");

  // Replace references to all functions with their equivalent function
  replaceFunctionReferences(ast, equivalentFunctionHash);

  // Now lets replace the function definitions
  replaceFunctionDefinitions(ast, equivalentFunctionHash);
}

function logEquivalentFunctionHash(equivalentFunctionHash) {
  if (debug && ZONE_EQUIVALENT_FUNCTION_HASH != 0) {
    debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, "Equivalent Function Hash:");
    for (var fn in equivalentFunctionHash) {
      debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, fn + "->" + equivalentFunctionHash[fn]);
    }
  }
}

function generateEquivalentFunctionHash(functionHashToFunctionName, variableNames) {
  var equivalentFunctionHash = {};

  debugLog(ZONE_TOP_LEVEL, "generateEquivalentFunctionHash");

  if (debug && ZONE_EQUIVALENT_FUNCTION_HASH != 0) {
    debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, "Equivalent Functions:");

    for (var fnHash in functionHashToFunctionName) {
      if (functionHashToFunctionName[fnHash].length > 1) {
        debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, JSON.stringify(functionHashToFunctionName[fnHash], null, '  '));
      }
    }
  }

  for (var fnHash in functionHashToFunctionName) {
    var equivalentFunctions = functionHashToFunctionName[fnHash];
    var shortestFunction = undefined;
    var equivalentFn = undefined;

    // From each list of equivalent functions, pick the
    // shortest one that is not also a variable name
    for (var index in equivalentFunctions) {
      equivalentFn = equivalentFunctions[index];

      // If one of the variables is not the same name as the equivalent function,
      // and the equivalent function is shorter than the shortest function.
      if ((variableNames[equivalentFn] === undefined) &&
        (shortestFunction === undefined || equivalentFn.length < shortestFunction.length)) {
        shortestFunction = equivalentFn;
      }

      if (debug && variableNames[equivalentFn] !== undefined) {
        debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, equivalentFn + " is a variable");
      }
    }

    if (shortestFunction !== undefined) {
      // Populate the equivalent function hash with this info
      for (var index in equivalentFunctions) {
        equivalentFn = equivalentFunctions[index];

        // If we're not the shortest function, and
        // we are not a variable name
        if ((equivalentFn !== shortestFunction) && variableNames[equivalentFn] === undefined) {
          equivalentFunctionHash[equivalentFn] = shortestFunction;
          debugLog(ZONE_EQUIVALENT_FUNCTION_HASH, equivalentFn + "->" + shortestFunction);
        }
      }
    }
  }

  return equivalentFunctionHash;
}

function getBodyForFunction(ast, functionName) {
  var functionBody = undefined;
  var topLevelList = ast[1];

  for (var listIndex = 0; listIndex < topLevelList.length; ++listIndex) {
    var node = topLevelList[listIndex];

    if (node[0] === 'defun' && node[1] === functionName) {
      functionBody = getFunctionBody(node);
      break;
    }
  }

  return functionBody;
}

function checkForHashCollisions(ast, functionHashToFunctionName) {
  var functionHashToFunctionBody = {};

  for (var functionHash in functionHashToFunctionName) {
    var equivalentFunctions = functionHashToFunctionName[functionHash];
    var functionBody = getBodyForFunction(ast, equivalentFunctions[0]);

    functionHashToFunctionBody[functionHash] = functionBody;

    // If we have more than one equivalent function, make sure
    // that the bodies are the same from the hash values
    if (equivalentFunctions.length > 1) {
      for (var functionIndex = 1; functionIndex < equivalentFunctions.length; ++functionIndex) {
        var curFunctionBody = getBodyForFunction(ast, equivalentFunctions[functionIndex]);

        if (curFunctionBody !== functionBody) {
          printErr("ERROR!!! Function bodies for two hash-equivalent functions differ!!! Candidates: "
                  + equivalentFunctions[0] + ", " + equivalentFunctions[functionIndex]);
          process.exit(1);
        }
      }
    }
  }

  return functionHashToFunctionBody;
}

function eliminateDuplicateFuncs(ast) {
  debugLog(ZONE_TOP_LEVEL, "eliminateDuplicateFuncs");

  // Phase 1 - identify duplicate functions
  var functionHashToFunctionName = identifyDuplicateFunctions(ast);

  // Phase 1.1 - Check for hash collisions
  checkForHashCollisions(ast, functionHashToFunctionName);

  // Phase 2 - identify variables that conflict with function names
  var variableNames = getVariableNames(ast);

  // Phase 3 - generate the equivalent function hash
  var equivalentFunctionHash = generateEquivalentFunctionHash(functionHashToFunctionName, variableNames);

  // Phase 4 - for each set of equivalent functions, pick one and
  // use it to replace the other equivalent functions.
  replaceDuplicateFuncs(ast, equivalentFunctionHash);

  return;
}

function find(filename) {
  var prefixes = [nodePath.join(__dirname, '..', 'src'), process.cwd()];
  for (var i = 0; i < prefixes.length; ++i) {
    var combined = nodePath.join(prefixes[i], filename);
    if (nodeFS.existsSync(combined)) {
      return combined;
    }
  }
  return filename;
}

function findAsmAst(ast) {
  var asmNode = undefined;
  traverse(ast, function(node, type) {
    if (type === 'var') {

      var vars = node[1];
      for (var i = 0; i < vars.length; i++) {
        var ident = vars[i][0];

        if (ident === 'asm') {
          asmNode = vars[i][1][1][3]; // asm->call->toplevel-ast
        }
      }
    }
  });

  return asmNode;
}

function printHashInfo(ast) {
  debugLog(ZONE_TOP_LEVEL, "printHashInfo");

  var infoHash = {};
  infoHash['variable_names'] = getVariableNames(ast);
  infoHash['fn_hash_to_fn_name'] = identifyDuplicateFunctions(ast);
  infoHash['fn_hash_to_fn_body'] = checkForHashCollisions(ast, infoHash['fn_hash_to_fn_name']);

  print(JSON.stringify(infoHash));
}

read = function(filename) {
  var absolute = find(filename);
  return nodeFS['readFileSync'](absolute).toString();
};

// Main
var arguments_ = process['argv'].slice(2);
var noMinimizeWhitespace = false; // Eliminate whitespace by default
var functionName = undefined;
var src = undefined; 

for (var argIndex = 0; argIndex < arguments_.length; ++argIndex) {
  var arg = arguments_[argIndex];
  if (arg === '--debug') {
    debug = true;
    debugFile = nodeFS.openSync(debugFileName, 'w');
  } else if (arg === '--no-minimize-whitespace') {
    noMinimizeWhitespace = true;
  } else if (arg === '--gen-hash-info') {
    genHashInfo = true;
  } else if (arg === '--use-hash-info') {
    useHashInfo = true;
  } else if (arg === '--use-asm-ast') {
    useAsmAst = true;
  } else if (arg === '--get-function-body') {
    if (argIndex === arguments_.length_ - 1) {
      throw new Error('Please specify valid arguments!');
    }

    functionName = arguments_[argIndex+1];
    argIndex += 1;
  } else if (/^--/.test(arg)) {
    throw new Error('Please specify valid arguments!');
  } else if (src === undefined) {
    src = read(arg);
  } else {
    throw new Error('Please specify valid arguments!');
  }
}

var ast = srcToAst(src);
var asmAst = ast;

if (useAsmAst) {
  asmAst = findAsmAst(ast);
}

if (debug) {
  dumpAst(ast);
}

if (functionName !== undefined) {
  var functionBody = getBodyForFunction(ast, functionName);

  if (functionBody === undefined) {
    throw new Error('Could not find body for function ' + functionName + '!!!');
  }

  print(functionBody);
} else if (genHashInfo) {
  printHashInfo(asmAst);
} else {
  equivalentFunctionHash = {};

  if (useHashInfo) {
    // The last line has the required info
    infoHashJsonStart = src.lastIndexOf("//") + 2 // 2 for going past the //

    if (infoHashJsonStart == -1) {
      throw new Error('--use-hash-info specified but no JSON found at the end of the file!');
    }

    equivalentFunctionHash = JSON.parse(src.substring(infoHashJsonStart));

    logEquivalentFunctionHash(equivalentFunctionHash);
    replaceDuplicateFuncs(asmAst, equivalentFunctionHash);
  } else {
    eliminateDuplicateFuncs(asmAst);
  }

  var minimizeWhitespace = (debug || noMinimizeWhitespace) ? false : true;
  var js = astToSrc(ast, minimizeWhitespace);

  print(js);
}

if (debug && debugFile !== undefined) {
  printErr('Wrote debug log to ' + debugFileName);
  nodeFS.close(debugFile);
}
