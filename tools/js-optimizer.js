#!/usr/bin/env node
// Copyright 2011 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Optimizer tool. This is meant to be run after the emscripten compiler has
// finished generating code. These optimizations are done on the generated
// code to further improve it.
//
// Be aware that this is *not* a general JS optimizer. It assumes that the
// input is valid asm.js and makes strong assumptions based on this. It may do
// anything from crashing to optimizing incorrectly if the input is not valid!
//
// TODO: Optimize traverse to modify a node we want to replace, in-place,
//       instead of returning it to the previous call frame where we check?
// TODO: Share EMPTY_NODE instead of emptyNode that constructs?

var uglify = require('../third_party/uglify-js');
var fs = require('fs');
var path = require('path');
var fs = require('fs');

var arguments_ = process['argv'].slice(2);
var debug = false;

function printErr(x) {
  process.stderr.write(x + '\n');
}

function print(x) {
  process.stdout.write(x + '\n');
}

function assert(x, msg) {
  if (!x) throw 'assertion failed (' + msg + ') : ' + new Error().stack;
}

function read(filename) {
  return fs.readFileSync(filename).toString();
}

function load(f) {
  f = path.join(__dirname, f)
  eval.call(null, read(f));
};

load('../src/utility.js');

// Utilities

var LOOP = set('do', 'while', 'for');
var LOOP_FLOW = set('break', 'continue');
var CONTROL_FLOW = set('do', 'while', 'for', 'if', 'switch');
var NAME_OR_NUM = set('name', 'num');
var ASSOCIATIVE_BINARIES = set('+', '*', '|', '&', '^');
var ALTER_FLOW = set('break', 'continue', 'return');
var BITWISE = set('|', '&', '^');

var BREAK_CAPTURERS = set('do', 'while', 'for', 'switch');
var CONTINUE_CAPTURERS = LOOP;

var COMMABLE = set('assign', 'binary', 'unary-prefix', 'unary-postfix', 'name', 'num', 'call', 'seq', 'conditional', 'sub');

var CONDITION_CHECKERS = set('if', 'do', 'while', 'switch');
var BOOLEAN_RECEIVERS = set('if', 'do', 'while', 'conditional');

var FUNCTIONS_THAT_ALWAYS_THROW = set('abort', '___resumeException', '___cxa_throw', '___cxa_rethrow');

var UNDEFINED_NODE = ['unary-prefix', 'void', ['num', 0]];

var GENERATED_FUNCTIONS_MARKER = '// EMSCRIPTEN_GENERATED_FUNCTIONS';
var generatedFunctions = false; // whether we have received only generated functions

var extraInfo = null;

function srcToAst(src) {
  return uglify.parser.parse(src, false, debug);
}

function astToSrc(ast, minifyWhitespace) {
  return uglify.uglify.gen_code(ast, {
    debug: debug,
    ascii_only: true,
    beautify: !minifyWhitespace,
    indent_level: 1
  });
}

function srcToStat(src) {
  return srcToAst(src)[1][0]; // look into toplevel
}

function srcToExp(src) {
  return srcToStat(src)[1];
}

// Traverses the children of a node. If the traverse function returns an object,
// replaces the child. If it returns true, stop the traversal and return true.
function traverseChildren(node, traverse, pre, post) {
  if (node[0] === 'var') {
    // don't traverse the names, just the values
    var children = node[1];
    if (!Array.isArray(children)) return;
    for (var i = 0; i < children.length; i++) {
      var subnode = children[i];
      if (subnode.length === 2) {
        var value = subnode[1];
        if (Array.isArray(value)) {
          var subresult = traverse(value, pre, post);
          if (subresult === true) return true;
          if (subresult !== null && typeof subresult === 'object') subnode[1] = subresult;
        }
      }
    }
  } else if (node[0] === 'object') {
    // don't traverse the names, just the values
    var children = node[1];
    if (!Array.isArray(children)) return;
    for (var i = 0; i < children.length; i++) {
      var subnode = children[i];
      var value = subnode[1];
      if (Array.isArray(value)) {
        var subresult = traverse(value, pre, post);
        if (subresult === true) return true;
        if (subresult !== null && typeof subresult === 'object') subnode[1] = subresult;
      }
    }
  } else {
    // generic traversal
    for (var i = 0; i < node.length; i++) {
      var subnode = node[i];
      if (Array.isArray(subnode)) {
        var subresult = traverse(subnode, pre, post);
        if (subresult === true) return true;
        if (subresult !== null && typeof subresult === 'object') node[i] = subresult;
      }
    }
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
  var type = node[0], result, len;
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

// Only walk through the generated functions
function traverseGeneratedFunctions(ast, callback) {
  assert(generatedFunctions);
  if (ast[0] === 'toplevel') {
    var stats = ast[1];
    for (var i = 0; i < stats.length; i++) {
      var curr = stats[i];
      if (curr[0] === 'defun') callback(curr);
    }
  } else if (ast[0] === 'defun') {
    callback(ast);
  }
}

function traverseGenerated(ast, pre, post) {
  traverseGeneratedFunctions(ast, function(func) {
    traverse(func, pre, post);
  });
}

function deStat(node) {
  if (node[0] === 'stat') return node[1];
  return node;
}

function emptyNode() { // XXX do we need to create new nodes here? can't we reuse?
  return ['toplevel', []]
}

function isEmptyNode(node) {
  return node.length === 2 && node[0] === 'toplevel' && node[1].length === 0;
}

function clearEmptyNodes(list) {
  for (var i = 0; i < list.length;) {
    if (isEmptyNode(list[i]) || (list[i][0] === 'stat' && isEmptyNode(list[i][1]))) {
      list.splice(i, 1);
    } else {
      i++;
    }
  }
}

function filterEmptyNodes(list) { // creates a copy and returns it
  return list.filter(function(node) {
    return !(isEmptyNode(node) || (node[0] === 'stat' && isEmptyNode(node[1])));
  });
}

function removeEmptySubNodes(node) {
  if (node[0] === 'defun') {
    node[3] = filterEmptyNodes(node[3]);
  } else if (node[0] === 'block' && node[1]) {
    node[1] = filterEmptyNodes(node[1]);
  } else if (node[0] === 'seq' && isEmptyNode(node[1])) {
    return node[2];
  }
/*
  var stats = getStatements(node);
  if (stats) clearEmptyNodes(stats);
*/
}

function removeAllEmptySubNodes(ast) {
  traverse(ast, removeEmptySubNodes);
}

function astCompare(x, y) {
  if (!Array.isArray(x)) return x === y;
  if (!Array.isArray(y)) return false;
  if (x.length !== y.length) return false;
  for (var i = 0; i < x.length; i++) {
    if (!astCompare(x[i], y[i])) return false;
  }
  return true;
}

// calls definitely on child nodes that will be called, in order, and calls maybe on nodes that might be called.
// a(); if (b) c(); d(); will call definitely(a, arg), maybe(c, arg), definitely(d, arg)
function traverseChildrenInExecutionOrder(node, definitely, maybe, arg) {
  switch (node[0]) {
    default: throw '!' + node[0];
    case 'num': case 'var': case 'name': case 'toplevel': case 'string':
    case 'break': case 'continue': break; // nodes with no interesting children; they themselves have already been definitely or maybe'd
    case 'assign': case 'binary': {
      definitely(node[2], arg);
      definitely(node[3], arg);
      break;
    }
    case 'sub': case 'seq': {
      definitely(node[1], arg);
      definitely(node[2], arg);
      break;
    }
    case 'while': {
      definitely(node[1], arg);
      maybe(node[2], arg); // may never enter the loop
      maybe(node[1], arg); // may enter the loop a second time
      maybe(node[2], arg);
      break;
    }
    case 'do': {
      definitely(node[2], arg);
      maybe(node[1], arg); // may never reach the condition if we break
      maybe(node[2], arg);
      maybe(node[1], arg);
      break;
    }
    case 'binary': {
      definitely(node[2], arg);
      definitely(node[3], arg);
      break;
    }
    case 'dot': {
      definitely(node[1], arg);
      break;
    }
    case 'unary-prefix': case 'label': {
      definitely(node[2], arg);
      break;
    }
    case 'call': {
      definitely(node[1], arg);
      var args = node[2];
      for (var i = 0; i < args.length; i++) {
        definitely(args[i], arg);
      }
      break;
    }
    case 'if': case 'conditional': {
      definitely(node[1], arg);
      maybe(node[2], arg);
      if (node[3]) { maybe(node[3], arg); }
      break;
    }
    case 'defun': case 'func': case 'block': {
      var stats = getStatements(node);
      if (!stats || stats.length === 0) break;
      for (var i = 0; i < stats.length; i++) {
        definitely(stats[i], arg);
        // check if we might break, if we have more to do
        if (i === stats.length - 1) break;
        var labels = {};
        var breakCaptured = 0;
        var mightBreak = false;
        traverse(stats[i], function(node, type) {
          if (type === 'label') labels[node[1]] = true;
          else if (type in BREAK_CAPTURERS) {
            breakCaptured++;
          } else if (type === 'break') {
            if (node[1]) {
              // labeled break
              if (!(node[1] in labels)) mightBreak = true;
            } else {
              if (!breakCaptured) mightBreak = true;
            }
          }
        }, function(node, type) {
          if (type === 'label') delete labels[node[1]];
          else if (type in BREAK_CAPTURERS) {
            breakCaptured--;
          }
        });
        if (mightBreak) {
          // all the rest are in one big maybe
          return maybe(['block', stats.slice(i+1)], arg);
        }
      }
      break;
    }
    case 'stat': case 'return': {
      definitely(node[1], arg);
      break;
    }
    case 'switch': {
      definitely(node[1], arg);
      var cases = node[2];
      for (var i = 0; i < cases.length; i++) {
        var c = cases[i];
        var stats = c[1];
        var temp = ['block', stats];
        maybe(temp, arg);
      }
    }
  }
}

// Passes

// Dump the AST. Useful for debugging. For example,
//  node tools/js-optimizer.js ABSOLUTE_PATH_TO_FILE dumpAst
function dumpAst(ast) {
  printErr(JSON.stringify(ast, null, '  '));
}

function dumpSrc(ast) {
  printErr(astToSrc(ast));
}

function minifyGlobals(ast) {
  // The input is in form
  //
  //   function instantiate(asmLibraryArg, wasmMemory, wasmTable) {
  //      var helper..
  //      function asmFunc(global, env, buffer) {
  //        var memory = env.memory;
  //        var HEAP8 = new global.Int8Array(buffer);
  //
  // We want to minify the interior instantiate, basically everything but
  // the name instantiate itself, which is used externally to call it.
  //
  // This is *not* a complete minification algorithm. It does not have a full
  // understanding of nested scopes. Instead it assumes the code is fairly
  // simple - as wasm2js output is - and looks at all the minifiable names as
  // a whole. A possible bug here is something like
  //
  //   function instantiate(asmLibraryArg, wasmMemory, wasmTable) {
  //      var x = foo;
  //      function asmFunc(global, env, buffer) {
  //        var foo = 10;
  //
  // Here foo is declared in an inner scope, and the outer use of foo looks
  // to the global scope. The analysis here only thinks something is from the
  // global scope if it is not in any var or function declaration. In practice,
  // the globals used from wasm2js output are things like Int8Array that we
  // don't declare as locals, but we should probably have a fully scope-aware
  // analysis here. FIXME
  assert(ast[0] === 'toplevel');
  var instantiateFunc = ast[1][0];
  assert(instantiateFunc[0] === 'defun');
  assert(instantiateFunc[1] === 'instantiate');

  var minified = new Map();
  var next = 0;
  function getMinified(name) {
    assert(name);
    if (minified.has(name)) return minified.get(name);
    ensureMinifiedNames(next);
    var m = minifiedNames[next++];
    minified.set(name, m);
    return m;
  }

  // name nodes, ['name', name]
  var allNames = [];
  // functions, whose element func[1] is the name
  var allNamedFunctions = [];
  // name arrays, as in func[2], [name1, name2] which are the arguments
  var allNameArrays = [];
  // var arrays, as in [[name, init], ..] in a var or const
  var allVarArrays = [];

  // Add instantiate's parameters, but *not* its own name, which is used
  // externally.
  allNameArrays.push(instantiateFunc[2]);

  // First, find all the other things to minify.
  instantiateFunc[3].forEach(function(ast) {
    traverse(ast, function(node, type) {
      if (type === 'defun') {
        allNamedFunctions.push(node);
        allNameArrays.push(node[2]);
      } else if (type === 'function') {
        allNameArrays.push(node[2]);
      } else if (type === 'var' || type === 'const') {
        allVarArrays.push(node[1]);
      } else if (type === 'name') {
        allNames.push(node);
      }
    });
  });

  // All the things that are valid to minify: names declared in vars, or
  // params, etc.
  var validNames = new Set();

  // TODO: sort to find the optimal minification
  allVarArrays.forEach(function(array) {
    for (var i = 0; i < array.length; i++) {
      var name = array[i][0];
      validNames.add(name);
      array[i][0] = getMinified(name);
    }
  });
  // add all globals in function chunks, i.e. not here but passed to us
  for (var i = 0; i < extraInfo.globals.length; i++) {
    var name = extraInfo.globals[i];
    validNames.add(name);
    getMinified(name);
  }
  allNamedFunctions.forEach(function(node) {
    var name = node[1];
    validNames.add(name);
    node[1] = getMinified(name);
  });
  allNameArrays.forEach(function(array) {
    for (var i = 0; i < array.length; i++) {
      var name = array[i];
      validNames.add(name);
      array[i] = getMinified(name);
    }
  });
  allNames.forEach(function(node) {
    var name = node[1];
    // A name may refer to a true global like Int8Array, which is not a valid
    // name to minify, as we didn't see it declared.
    if (validNames.has(name)) {
      node[1] = getMinified(name);
    }
  });
  var json = {};
  for (var x of minified.entries()) json[x[0]] = x[1];
  suffix = '// EXTRA_INFO:' + JSON.stringify(json);
}

// Name minification

var RESERVED = set('do', 'if', 'in', 'for', 'new', 'try', 'var', 'env', 'let', 'case', 'else', 'enum', 'void', 'this', 'void', 'with');
var VALID_MIN_INITS = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$';
var VALID_MIN_LATERS = VALID_MIN_INITS + '0123456789';

var minifiedNames = [];
var minifiedState = [0];

function ensureMinifiedNames(n) { // make sure the nth index in minifiedNames exists. done 100% deterministically
  while (minifiedNames.length < n+1) {
    // generate the current name
    var name = VALID_MIN_INITS[minifiedState[0]];
    for (var i = 1; i < minifiedState.length; i++) {
      name += VALID_MIN_LATERS[minifiedState[i]];
    }
    if (!(name in RESERVED)) minifiedNames.push(name);
    // increment the state
    var i = 0;
    while (1) {
      minifiedState[i]++;
      if (minifiedState[i] < (i === 0 ? VALID_MIN_INITS : VALID_MIN_LATERS).length) break;
      // overflow
      minifiedState[i] = 0;
      i++;
      if (i === minifiedState.length) minifiedState.push(-1); // will become 0 after increment in next loop head
    }
  }
}

function minifyLocals(ast) {
  assert(extraInfo && extraInfo.globals);

  traverseGeneratedFunctions(ast, function(fun, type) {
    // Find the list of local names, including params.
    if (asm) {
      // TODO: we can avoid modifying at all here - we just need a list of local vars+params
      var asmData = normalizeAsm(fun);
      denormalizeAsm(fun, asmData);
    } else {
      // non-asm.js code - scan the whole function, which is inefficient
      var localNames = {};
      for (var param of fun[2]) {
        localNames[param] = 1;
      }
      traverse(fun, function(node, type) {
        if (type === 'var') {
          node[1].forEach(function(defn) {
            var name = defn[0];
            localNames[name] = 1;
          });
        }
      });
    }

    var newNames = {};
    var usedNames = {};

    // Find all the globals that we need to minify using
    // pre-assigned names.  Don't actually minify them yet
    // as that might interfere with local variable names.
    function isLocalName(name) {
      if (asm) {
        return name in asmData.vars || name in asmData.params;
      } else {
        return Object.prototype.hasOwnProperty.call(localNames, name);
      }
    }
    traverse(fun, function(node, type) {
      if (type === 'name') {
        var name = node[1];
        if (!isLocalName(name)) {
          var minified = extraInfo.globals[name];
          if (minified){
            newNames[name] = minified;
            usedNames[minified] = 1;
          }
        }
      } else if (type === 'call') {
        // We should never call a local name, as in asm.js-style code our
        // locals are just numbers, not functions; functions are all declared
        // in the outer scope. If a local is called, that is a bug.
        if (node[1][0] === 'name') {
          var name = node[1][1];
          assert(!isLocalName(name), 'cannot call a local');
        }
      }
    });

    // The first time we encounter a local name, we assign it a
    // minified name that's not currently in use.  Allocating on
    // demand means they're processed in a predictable order,
    // which is very handy for testing/debugging purposes.
    var nextMinifiedName = 0;
    function getNextMinifiedName() {
      var minified;
      while (1) {
        ensureMinifiedNames(nextMinifiedName);
        minified = minifiedNames[nextMinifiedName++];
        // TODO: we can probably remove !isLocalName here
        if (!usedNames[minified] && !isLocalName(minified)) {
          return minified;
        }
      }
    }

    // We can also minify loop labels, using a separate namespace
    // to the variable declarations.
    var newLabels = {};
    var nextMinifiedLabel = 0;
    function getNextMinifiedLabel() {
      ensureMinifiedNames(nextMinifiedLabel);
      return minifiedNames[nextMinifiedLabel++];
    }

    // Traverse and minify all names.
    assert(extraInfo.globals.hasOwnProperty(fun[1]));
    fun[1] = extraInfo.globals[fun[1]];
    assert(fun[1] && typeof fun[1] === 'string');
    if (fun[2]) {
      for (var i = 0; i < fun[2].length; i++) {
        var minified = getNextMinifiedName();
        newNames[fun[2][i]] = minified;
        fun[2][i] = minified;
      }
    }
    traverse(fun[3], function(node, type) {
      if (type === 'name') {
        var name = node[1];
        var minified = newNames[name];
        if (minified) {
          node[1] = minified;
        } else if (isLocalName(name)) {
          minified = getNextMinifiedName();
          newNames[name] = minified;
          node[1] = minified;
        }
      } else if (type === 'var') {
        node[1].forEach(function(defn) {
          var name = defn[0];
          if (!(name in newNames)) {
            newNames[name] = getNextMinifiedName();
          }
          defn[0] = newNames[name];
        });
      } else if (type === 'label') {
        if (!newLabels[node[1]]) {
          newLabels[node[1]] = getNextMinifiedLabel();
        }
        node[1] = newLabels[node[1]];
      } else if (type === 'break' || type === 'continue') {
        if (node[1]) {
          node[1] = newLabels[node[1]];
        }
      }
    });
  });
}

// Passes table

var minifyWhitespace = false, printMetadata = true, asm = false,
    emitJSON = false, last = false,
    emitAst = true;

var passes = {
  // passes
  dumpAst: dumpAst,
  dumpSrc: dumpSrc,
  minifyGlobals: minifyGlobals,
  minifyLocals: minifyLocals,
  noop: function() {},

  // flags
  minifyWhitespace: function() { minifyWhitespace = true },
  noPrintMetadata: function() { printMetadata = false },
  asm: function() { asm = true },
  emitJSON: function() { emitJSON = true },
  receiveJSON: function() { }, // handled in a special way, before passes are run
  last: function() { last = true },
  noEmitAst: function() { emitAst = false },
};

// Main

var suffix = '';

arguments_ = arguments_.filter(function (arg) {
  if (!/^--/.test(arg)) return true;

  if (arg === '--debug') debug = true;
  else throw new Error('Unrecognized flag: ' + arg);
});


var src = read(arguments_[0]);
generatedFunctions = src.lastIndexOf(GENERATED_FUNCTIONS_MARKER) >= 0;
var extraInfoStart = src.lastIndexOf('// EXTRA_INFO:')
if (extraInfoStart > 0) extraInfo = JSON.parse(src.substr(extraInfoStart + 14));
//printErr(JSON.stringify(extraInfo));

var ast;
if (arguments_.indexOf('receiveJSON') < 0) {
  ast = srcToAst(src);
} else {
  var commentStart = src.indexOf('//');
  if (commentStart >= 0) {
    src = src.substr(0, commentStart); // JSON.parse will error on a trailing comment
  }
  ast = JSON.parse(src);
}
//printErr('ast: ' + JSON.stringify(ast));

arguments_.slice(1).forEach(function(arg) {
  passes[arg](ast);
});
if (asm && last) {
  prepDotZero(ast);
}

if (emitAst) {
  if (!emitJSON) {
    var js = astToSrc(ast, minifyWhitespace), old;
    if (asm && last) {
      js = fixDotZero(js);
    }
    // remove unneeded newlines+spaces, and print
    do {
      old = js;
      js = js.replace(/\n *\n/g, '\n');
    } while (js != old);
    print(js);
    print('\n');
    print(suffix);
  } else {
    print(JSON.stringify(ast));
  }
}
