// -*- Mode: javascript; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 ; js-indent-level : 2 ; js-curly-indent-offset: 0 -*-
// vim: set ts=2 et sw=2:

//==============================================================================
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
//==============================================================================

if (!Math.fround) {
  var froundBuffer = new Float32Array(1);
  Math.fround = function(x) { froundBuffer[0] = x; return froundBuffer[0] };
}

// *** Environment setup code ***
var arguments_ = [];
var debug = false;

var ENVIRONMENT_IS_NODE = typeof process === 'object';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  print = function(x) {
    process['stdout'].write(x + '\n');
  };
  printErr = function(x) {
    process['stderr'].write(x + '\n');
  };

  var nodeFS = require('fs');
  var nodePath = require('path');

  if (!nodeFS.existsSync) {
    nodeFS.existsSync = function(path) {
      try {
        return !!nodeFS.readFileSync(path);
      } catch(e) {
        return false;
      }
    }
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

  read = function(filename) {
    var absolute = find(filename);
    return nodeFS['readFileSync'](absolute).toString();
  };

  load = function(f) {
    globalEval(read(f));
  };

  arguments_ = process['argv'].slice(2);

} else if (ENVIRONMENT_IS_SHELL) {
  // Polyfill over SpiderMonkey/V8 differences
  if (!this['read']) {
    this['read'] = function(f) { snarf(f) };
  }

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

} else if (ENVIRONMENT_IS_WEB) {
  this['print'] = printErr = function(x) {
    console.log(x);
  };

  this['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (this['arguments']) {
    arguments_ = arguments;
  }
} else if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...

  this['load'] = importScripts;

} else {
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}

if (typeof load === 'undefined' && typeof read != 'undefined') {
  this['load'] = function(f) {
    globalEval(read(f));
  };
}

if (typeof printErr === 'undefined') {
  this['printErr'] = function(){};
}

if (typeof print === 'undefined') {
  this['print'] = printErr;
}
// *** Environment setup code ***

var uglify = require('../tools/eliminator/node_modules/uglify-js');
var fs = require('fs');
var path = require('path');

// Load some modules

load('utility.js');

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
  for (var i = 0; i < node.length; i++) {
    var subnode = node[i];
    if (Array.isArray(subnode)) {
      var subresult = traverse(subnode, pre, post);
      if (subresult === true) return true;
      if (subresult !== null && typeof subresult === 'object') node[i] = subresult;
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

function overwrite(x, y) {
  for (var i = 0; i < y.length; i++) x[i] = y[i];
  x.length = y.length;
}

var STACK_ALIGN = 16;

function stackAlign(x) {
  if (x % STACK_ALIGN) x += STACK_ALIGN - (x % STACK_ALIGN);
  return x;
}

// Closure compiler, when inlining, will insert assignments to
// undefined for the shared variables. However, in compiled code
// - and in library/shell code too! - we should never rely on
// undefined being assigned. So we can simply remove those assignments.
//
// Note: An inlined function that kept a large value referenced, may
//       keep that references when inlined, if we remove the setting to
//       undefined. This is not dangerous in compiled code, but might be
//       in supporting code (for example, holding on to the HEAP when copying).
//
// This pass assumes that unGlobalize has been run, so undefined
// is now explicit.
function removeAssignsToUndefined(ast) {
  traverse(ast, function(node, type) {
    if (type === 'assign' && jsonCompare(node[3], ['unary-prefix', 'void', ['num', 0]])) {
      return emptyNode();
    } else if (type === 'var') {
      node[1] = node[1].map(function(varItem, j) {
        var ident = varItem[0];
        var value = varItem[1];
        if (jsonCompare(value, UNDEFINED_NODE)) return [ident];
        return [ident, value];
      });
    }
  });
  // cleanup (|x = y = void 0| leaves |x = ;| right now)
  var modified = true;
  while (modified) {
    modified = false;
    traverse(ast, function(node, type) {
      if (type === 'assign' && jsonCompare(node[3], emptyNode())) {
        modified = true;
        return emptyNode();
      } else if (type === 'var') {
        node[1] = node[1].map(function(varItem, j) {
          var ident = varItem[0];
          var value = varItem[1];
          if (value && jsonCompare(value, emptyNode())) return [ident];
          return [ident, value];
        });
      }
    });
  }
}

// XXX This is an invalid optimization
// We sometimes leave some settings to label that are not needed, if later in
// the relooper we realize that we have a single entry, so no checks on label
// are actually necessary. It's easy to clean those up now.
function removeUnneededLabelSettings(ast) {
  traverse(ast, function(node, type) {
    if (type === 'defun') { // all of our compiled code is in defun nodes
      // Find all checks
      var checked = {};
      traverse(node, function(node, type) {
        if (type === 'binary' && node[1] === '==' && node[2][0] === 'name' && node[2][1] === 'label') {
          assert(node[3][0] === 'num');
          checked[node[3][1]] = 1;
        }
      });
      // Remove unneeded sets
      traverse(node, function(node, type) {
        if (type === 'assign' && node[2][0] === 'name' && node[2][1] === 'label') {
          assert(node[3][0] === 'num');
          if (!(node[3][1] in checked)) return emptyNode();
        }
      });
    }
  });
}

// Various expression simplifications. Happens after elimination, which opens up many of these simplification opportunities.

function makeTempParseHeap() {
  return { unsigned: false, float: false, bits: 0, type: 0 };
}

var parseHeapTemp = makeTempParseHeap();

function parseHeap(name, out) { // XXX this uses parseHeapTemp by default, which is global! If between your call and your use, something else can call - that is bad
  out = out || parseHeapTemp;
  if (name.substr(0, 4) != 'HEAP') return false;
  out.unsigned = name[4] === 'U';
  out.float = name[4] === 'F';
  out.bits = parseInt(name.substr(out.unsigned || out.float ? 5 : 4));
  out.type = !out.float ? ASM_INT : (out.bits === 64 ? ASM_DOUBLE : ASM_FLOAT);
  return true;
}

var USEFUL_BINARY_OPS = set('<<', '>>', '|', '&', '^');
var COMPARE_OPS = set('<', '<=', '>', '>=', '==', '===', '!=', '!==');

function isFunctionTable(name) {
  return /^FUNCTION_TABLE.*/.test(name);
}

function simplifyExpressions(ast) {
  // Simplify common expressions used to perform integer conversion operations
  // in cases where no conversion is needed.
  function simplifyIntegerConversions(ast) {
    traverse(ast, function(node, type) {
      if (type === 'binary'       && node[1]    === '>>' && node[3][0] === 'num' &&
          node[2][0] === 'binary' && node[2][1] === '<<' && node[2][3][0] === 'num' && node[3][1] === node[2][3][1]) {
        // Transform (x&A)<<B>>B to X&A.
        var innerNode = node[2][2];
        var shifts = node[3][1];
        if (innerNode[0] === 'binary' && innerNode[1] === '&' && innerNode[3][0] === 'num') {
          var mask = innerNode[3][1];
          if (mask << shifts >> shifts === mask) {
            return innerNode;
          }
        }
      } else if (type === 'binary' && (node[1] in BITWISE)) {
        for (var i = 2; i <= 3; i++) {
          var subNode = node[i];
          if (subNode[0] === 'binary' && subNode[1] === '&' && subNode[3][0] === 'num' && subNode[3][1] == 1) {
            // Rewrite (X < Y) & 1 to X < Y , when it is going into a bitwise operator. We could
            // remove even more (just replace &1 with |0, then subsequent passes could remove the |0)
            // but v8 issue #2513 means the code would then run very slowly in chrome.
            var input = subNode[2];
            if (input[0] === 'binary' && (input[1] in COMPARE_OPS)) {
              node[i] = input;
            }
          }
        }
      }
    });
  }

  // When there is a bunch of math like (((8+5)|0)+12)|0, only the external |0 is needed, one correction is enough.
  // At each node, ((X|0)+Y)|0 can be transformed into (X+Y): The inner corrections are not needed
  // TODO: Is the same is true for 0xff, 0xffff?
  // Likewise, if we have |0 inside a block that will be >>'d, then the |0 is unnecessary because some
  // 'useful' mathops already |0 anyhow.

  function simplifyOps(ast) {
    var SAFE_BINARY_OPS;
    if (asm) {
      SAFE_BINARY_OPS = set('+', '-'); // division is unsafe as it creates non-ints in JS; mod is unsafe as signs matter so we can't remove |0's; mul does not nest with +,- in asm
    } else {
      SAFE_BINARY_OPS = set('+', '-', '*');
    }
    var COERCION_REQUIRING_OPS = set('sub', 'unary-prefix'); // ops that in asm must be coerced right away
    var COERCION_REQUIRING_BINARIES = set('*', '/', '%'); // binary ops that in asm must be coerced
    var ZERO = ['num', 0];

    function removeMultipleOrZero() {
      var rerun = true;
      while (rerun) {
        rerun = false;
        var stack = [];
        traverse(ast, function process(node, type) {
          if (type === 'binary' && node[1] === '|') {
            if (node[2][0] === 'num' && node[3][0] === 'num') {
              node[2][1] |= node[3][1];
              stack.push(0);
              return node[2];
            }
            var go = false;
            if (jsonCompare(node[2], ZERO)) {
              // canonicalize order
              var temp = node[3];
              node[3] = node[2];
              node[2] = temp;
              go = true;
            } else if (jsonCompare(node[3], ZERO)) {
              go = true;
            }
            if (!go) {
              stack.push(1);
              return;
            }
            // We might be able to remove this correction
            for (var i = stack.length-1; i >= 0; i--) {
              if (stack[i] >= 1) {
                if (asm) {
                  if (stack[stack.length-1] < 2 && node[2][0] === 'call') break; // we can only remove multiple |0s on these
                  if (stack[stack.length-1] < 1 && (node[2][0] in COERCION_REQUIRING_OPS ||
                                                    (node[2][0] === 'binary' && node[2][1] in COERCION_REQUIRING_BINARIES))) break; // we can remove |0 or >>2
                }
                // we will replace ourselves with the non-zero side. Recursively process that node.
                var result = jsonCompare(node[2], ZERO) ? node[3] : node[2], other;
                // replace node in-place
                node.length = result.length;
                for (var j = 0; j < result.length; j++) {
                  node[j] = result[j];
                }
                rerun = true;
                return process(result, result[0]);
              } else if (stack[i] === -1) {
                break; // Too bad, we can't
              }
            }
            stack.push(2); // From here on up, no need for this kind of correction, it's done at the top
                           // (Add this at the end, so it is only added if we did not remove it)
          } else if (type === 'binary' && node[1] in USEFUL_BINARY_OPS) {
            stack.push(1);
          } else if ((type === 'binary' && node[1] in SAFE_BINARY_OPS) || type === 'num' || type === 'name') {
            stack.push(0); // This node is safe in that it does not interfere with this optimization
          } else if (type === 'unary-prefix' && node[1] === '~') {
            stack.push(1);
          } else {
            stack.push(-1); // This node is dangerous! Give up if you see this before you see '1'
          }
        }, function() {
          stack.pop();
        });
      }
    }

    removeMultipleOrZero();

    // & and heap-related optimizations

    var hasTempDoublePtr = false, rerunOrZeroPass = false;

    traverse(ast, function(node, type) {
      // The "pre" visitor. Useful for detecting trees which should not
      // be simplified.
      if (type == 'sub' && node[1][0] == 'name' && isFunctionTable(node[1][1])) {
        return null; // do not traverse subchildren here, we should not collapse 55 & 126.
      }
    }, function(node, type) {
      // The "post" visitor. The simplifications are done in this visitor so
      // that we simplify a node's operands before the node itself. This allows
      // optimizations to cascade.
      if (type === 'name') {
        if (node[1] === 'tempDoublePtr') hasTempDoublePtr = true;
      } else if (type === 'binary' && node[1] === '&' && node[3][0] === 'num') {
        if (node[2][0] === 'num') return ['num', node[2][1] & node[3][1]];
        var input = node[2];
        var amount = node[3][1];
        if (input[0] === 'binary' && input[1] === '&' && input[3][0] === 'num') {
          // Collapse X & 255 & 1
          node[3][1] = amount & input[3][1];
          node[2] = input[2];
        } else if (input[0] === 'sub' && input[1][0] === 'name') {
          // HEAP8[..] & 255 => HEAPU8[..]
          var name = input[1][1];
          if (parseHeap(name)) {
            if (amount === Math.pow(2, parseHeapTemp.bits)-1) {
              if (!parseHeapTemp.unsigned) {
                input[1][1] = 'HEAPU' + parseHeapTemp.bits; // make unsigned
              }
              if (asm) {
                // we cannot return HEAPU8 without a coercion, but at least we do HEAP8 & 255 => HEAPU8 | 0
                node[1] = '|';
                node[3][1] = 0;
                return node;
              }
              return input;
            }
          }
        } else if (input[0] === 'binary' && input[1] === '>>' &&
                   input[2][0] === 'binary' && input[2][1] === '<<' &&
                   input[2][3][0] === 'num' && input[3][0] === 'num' &&
                   input[2][3][1] === input[3][1] &&
                   (~(-1 >>> input[3][1]) & amount) == 0) {
            // x << 24 >> 24 & 255 => x & 255
            return ['binary', '&', input[2][2], node[3]];
        }
      } else if (type === 'binary' && node[1] === '^') {
        // LLVM represents bitwise not as xor with -1. Translate it back to an actual bitwise not.
        if (node[3][0] === 'unary-prefix' && node[3][1] === '-' && node[3][2][0] === 'num' &&
            node[3][2][1] === 1 &&
            !(node[2][0] == 'unary-prefix' && node[2][1] == '~')) { // avoid creating ~~~ which is confusing for asm given the role of ~~
            return ['unary-prefix', '~', node[2]];
        }
      } else if (type       === 'binary' && node[1]    === '>>' && node[3][0]    === 'num' &&
                 node[2][0] === 'binary' && node[2][1] === '<<' && node[2][3][0] === 'num' &&
                 node[2][2][0] === 'sub' && node[2][2][1][0] === 'name') {
        // collapse HEAPU?8[..] << 24 >> 24 etc. into HEAP8[..] | 0
        var amount = node[3][1];
        var name = node[2][2][1][1];
        if (amount === node[2][3][1] && parseHeap(name)) {
          if (parseHeapTemp.bits === 32 - amount) {
            node[2][2][1][1] = 'HEAP' + parseHeapTemp.bits;
            node[1] = '|';
            node[2] = node[2][2];
            node[3][1] = 0;
            rerunOrZeroPass = true;
            return node;
          }
        }
      } else if (type === 'assign') {
        // optimizations for assigning into HEAP32 specifically
        if (node[1] === true && node[2][0] === 'sub' && node[2][1][0] === 'name') {
          if (node[2][1][1] === 'HEAP32') {
            // HEAP32[..] = x | 0 does not need the | 0 (unless it is a mandatory |0 of a call)
            if (node[3][0] === 'binary' && node[3][1] === '|') {
              if (node[3][2][0] === 'num' && node[3][2][1] === 0 && node[3][3][0] != 'call') {
                node[3] = node[3][3];
              } else if (node[3][3][0] === 'num' && node[3][3][1] === 0 && node[3][2][0] != 'call') {
                node[3] = node[3][2];
              }
            }
          } else if (node[2][1][1] === 'HEAP8') {
            // HEAP8[..] = x & 0xff does not need the & 0xff
            if (node[3][0] === 'binary' && node[3][1] === '&' && node[3][3][0] == 'num' && node[3][3][1] == 0xff) {
              node[3] = node[3][2];
            }
          } else if (node[2][1][1] === 'HEAP16') {
            // HEAP16[..] = x & 0xffff does not need the & 0xffff
            if (node[3][0] === 'binary' && node[3][1] === '&' && node[3][3][0] == 'num' && node[3][3][1] == 0xffff) {
              node[3] = node[3][2];
            }
          }
        }
        var value = node[3];
        if (value[0] === 'binary' && value[1] === '|') {
          // canonicalize order of |0 to end
          if (value[2][0] === 'num' && value[2][1] === 0) {
            var temp = value[2];
            value[2] = value[3];
            value[3] = temp;
          }
          // if a seq ends in an |0, remove an external |0
          // note that it is only safe to do this in assigns, like we are doing here (return (x, y|0); is not valid)
          if (value[2][0] === 'seq' && value[2][2][0] === 'binary' && value[2][2][1] in USEFUL_BINARY_OPS) {
            node[3] = value[2];
          }
        }
      } else if (type === 'binary' && node[1] === '>>' && node[2][0] === 'num' && node[3][0] === 'num') {
        // optimize num >> num, in asm we need this since we do not optimize shifts in asm.js
        node[0] = 'num';
        node[1] = node[2][1] >> node[3][1];
        node.length = 2;
        return node;
      } else if (type === 'binary' && node[1] === '+') {
        // The most common mathop is addition, e.g. in getelementptr done repeatedly. We can join all of those,
        // by doing (num+num) ==> newnum.
        if (node[2][0] === 'num' && node[3][0] === 'num') {
          node[2][1] += node[3][1];
          return node[2];
        }
      }
    });

    if (rerunOrZeroPass) removeMultipleOrZero();

    if (asm) {
      if (hasTempDoublePtr) {
        var asmData = normalizeAsm(ast);
        traverse(ast, function(node, type) {
          if (type === 'assign') {
            if (node[1] === true && node[2][0] === 'sub' && node[2][1][0] === 'name' && node[2][1][1] === 'HEAP32') {
              // remove bitcasts that are now obviously pointless, e.g.
              // HEAP32[$45 >> 2] = HEAPF32[tempDoublePtr >> 2] = ($14 < $28 ? $14 : $28) - $42, HEAP32[tempDoublePtr >> 2] | 0;
              var value = node[3];
              if (value[0] === 'seq' && value[1][0] === 'assign' && value[1][2][0] === 'sub' && value[1][2][1][0] === 'name' && value[1][2][1][1] === 'HEAPF32' &&
                  value[1][2][2][0] === 'binary' && value[1][2][2][2][0] === 'name' && value[1][2][2][2][1] === 'tempDoublePtr') {
                // transform to HEAPF32[$45 >> 2] = ($14 < $28 ? $14 : $28) - $42;
                node[2][1][1] = 'HEAPF32';
                node[3] = value[1][3];
              }
            }
          } else if (type === 'seq') {
            // (HEAP32[tempDoublePtr >> 2] = HEAP32[$37 >> 2], +HEAPF32[tempDoublePtr >> 2])
            //   ==>
            // +HEAPF32[$37 >> 2]
            if (node[0] === 'seq' && node[1][0] === 'assign' && node[1][2][0] === 'sub' && node[1][2][1][0] === 'name' &&
                (node[1][2][1][1] === 'HEAP32' || node[1][2][1][1] === 'HEAPF32') &&
                node[1][2][2][0] === 'binary' && node[1][2][2][2][0] === 'name' && node[1][2][2][2][1] === 'tempDoublePtr' &&
                node[1][3][0] === 'sub' && node[1][3][1][0] === 'name' && (node[1][3][1][1] === 'HEAP32' || node[1][3][1][1] === 'HEAPF32') &&
                node[2][0] !== 'seq') { // avoid (x, y, z) which can be used for tempDoublePtr on doubles for alignment fixes
              if (node[1][2][1][1] === 'HEAP32') {
                node[1][3][1][1] = 'HEAPF32';
                return makeAsmCoercion(node[1][3], detectType(node[2]));
              } else {
                node[1][3][1][1] = 'HEAP32';
                return ['binary', '|', node[1][3], ['num', 0]];
              }
            }
          }
        });

        // finally, wipe out remaining ones by finding cases where all assignments to X are bitcasts, and all uses are writes to
        // the other heap type, then eliminate the bitcast
        var bitcastVars = {};
        traverse(ast, function(node, type) {
          if (type === 'assign' && node[1] === true && node[2][0] === 'name') {
            var value = node[3];
            if (value[0] === 'seq' && value[1][0] === 'assign' && value[1][2][0] === 'sub' && value[1][2][1][0] === 'name' &&
                (value[1][2][1][1] === 'HEAP32' || value[1][2][1][1] === 'HEAPF32') &&
                value[1][2][2][0] === 'binary' && value[1][2][2][2][0] === 'name' && value[1][2][2][2][1] === 'tempDoublePtr') {
              var name = node[2][1];
              if (!bitcastVars[name]) bitcastVars[name] = {
                define_HEAP32: 0, define_HEAPF32: 0, use_HEAP32: 0, use_HEAPF32: 0, bad: false, namings: 0, defines: [], uses: []
              };
              bitcastVars[name]['define_' + value[1][2][1][1]]++;
              bitcastVars[name].defines.push(node);
            }
          }
        });
        traverse(ast, function(node, type) {
          if (type === 'name' && bitcastVars[node[1]]) {
            bitcastVars[node[1]].namings++;
          } else if (type === 'assign' && node[1] === true) {
            var value = node[3];
            if (value[0] === 'name') {
              var name = value[1];
              if (bitcastVars[name]) {
                var target = node[2];
                if (target[0] === 'sub' && target[1][0] === 'name' && (target[1][1] === 'HEAP32' || target[1][1] === 'HEAPF32')) {
                  bitcastVars[name]['use_' + target[1][1]]++;
                  bitcastVars[name].uses.push(node);
                }
              }
            }
          }
        });
        for (var v in bitcastVars) {
          var info = bitcastVars[v];
          // good variables define only one type, use only one type, have definitions and uses, and define as a different type than they use
          if (info.define_HEAP32*info.define_HEAPF32 === 0 && info.use_HEAP32*info.use_HEAPF32 === 0 &&
              info.define_HEAP32+info.define_HEAPF32 > 0  && info.use_HEAP32+info.use_HEAPF32 > 0 &&
              info.define_HEAP32*info.use_HEAP32 === 0 && info.define_HEAPF32*info.use_HEAPF32 === 0 &&
              v in asmData.vars && info.namings === info.define_HEAP32+info.define_HEAPF32+info.use_HEAP32+info.use_HEAPF32) {
            var correct = info.use_HEAP32 ? 'HEAPF32' : 'HEAP32';
            info.defines.forEach(function(define) {
              define[3] = define[3][1][3];
              if (correct === 'HEAP32') {
                define[3] = ['binary', '|', define[3], ['num', 0]];
              } else {
                define[3] = makeAsmCoercion(define[3], asmPreciseF32 ? ASM_FLOAT : ASM_DOUBLE);
              }
              // do we want a simplifybitops on the new values here?
            });
            info.uses.forEach(function(use) {
              use[2][1][1] = correct;
            });
            var correctType;
            switch(asmData.vars[v]) {
              case ASM_INT: correctType = asmPreciseF32 ? ASM_FLOAT : ASM_DOUBLE; break;
              case ASM_FLOAT: case ASM_DOUBLE: correctType = ASM_INT; break;
            }
            asmData.vars[v] = correctType;
          }
        }
        denormalizeAsm(ast, asmData);
      }
    }
  }

  function emitsBoolean(node) {
    if (node[0] === 'num') {
      return node[1] === 0 || node[1] === 1;
    }
    if (node[0] === 'binary') return node[1] in COMPARE_OPS;
    if (node[0] === 'unary-prefix') return node[1] === '!';
    if (node[0] === 'conditional') return emitsBoolean(node[2]) && emitsBoolean(node[3]);
    return false;
  }

  //   expensive | expensive can be turned into expensive ? 1 : expensive, and
  //   expensive | cheap     can be turned into cheap     ? 1 : expensive,
  // so that we can avoid the expensive computation, if it has no side effects.
  function conditionalize(ast) {
    var MIN_COST = 7;
    traverse(ast, function(node, type) {
      if (node[0] === 'binary' && (node[1] === '|' || node[1] === '&') && node[3][0] !== 'num' && node[2][0] !== 'num') {
        // logical operator on two non-numerical values
        var left = node[2];
        var right = node[3];
        if (!emitsBoolean(left) || !emitsBoolean(right)) return;
        var leftEffects = hasSideEffects(left);
        var rightEffects = hasSideEffects(right);
        if (leftEffects && rightEffects) return; // both must execute
        // canonicalize with side effects, if any, happening on the left
        if (rightEffects) {
          if (measureCost(left) < MIN_COST) return; // avoidable code is too cheap
          var temp = left;
          left = right;
          right = temp;
        } else if (leftEffects) {
          if (measureCost(right) < MIN_COST) return; // avoidable code is too cheap
        } else {
          // no side effects, reorder based on cost estimation
          var leftCost = measureCost(left);
          var rightCost = measureCost(right);
          if (Math.max(leftCost, rightCost) < MIN_COST) return; // avoidable code is too cheap
          // canonicalize with expensive code on the right
          if (leftCost > rightCost) {
            var temp = left;
            left = right;
            right = temp;
          }
        }
        // worth it, perform conditionalization
        var ret;
        if (node[1] === '|') {
          ret = ['conditional', left, ['num', 1], right];
        } else { // &
          ret = ['conditional', left, right, ['num', 0]];
        }
        if (left[0] === 'unary-prefix' && left[1] === '!') {
          ret[1] = flipCondition(left);
          var temp = ret[2];
          ret[2] = ret[3];
          ret[3] = temp;
        }
        return ret;
      }
    });
  }

  function simplifyNotZero(ast) {
    traverse(ast, function(node, type) {
      if (node[0] in BOOLEAN_RECEIVERS) {
        var boolean = node[1];
        if (boolean[0] === 'binary' && boolean[1] === '!=' && boolean[3][0] === 'num' && boolean[3][1] === 0) {
          node[1] = boolean[2];
        }
      }
    });
  }

  traverseGeneratedFunctions(ast, function(func) {
    simplifyIntegerConversions(func);
    simplifyOps(func);
    simplifyNotComps(func);
    conditionalize(func);
    simplifyNotZero(func);
  });
}

function localCSE(ast) {
  // very simple CSE/GVN type optimization, factor out common expressions in a single basic block
  assert(asm);
  var MIN_COST = 3;
  traverseGeneratedFunctions(ast, function(func) {
    var asmData = normalizeAsm(func);
    var counter = 0;
    var optimized = false;
    traverse(func, function(node, type) {
      var stats = getStatements(node);
      if (!stats) return;
      var exps = {}; // JSON'd expression => [i it first appears on, original node, replacement var, type, sign]
      var deps = {}; // dependency (local name, or 'memory' or 'global')
      function invalidate(what) {
        var list = deps[what];
        if (!list) return;
        for (var i = 0; i < list.length; i++) {
          delete exps[list[i]];
        }
        delete deps[what];
      }
      function doInvalidations(curr) {
        return traverse(curr, function(node, type) {
          if (type in CONTROL_FLOW) {
            exps = {};
            deps = {};
            return true; // abort everything
          }
          if (type === 'assign') {
            var target = node[2];
            if (target[0] === 'name') {
              var name = target[1];
              if (name in asmData.params || name in asmData.vars) {
                invalidate(name);
              } else {
                invalidate('<global>');
              }
            } else {
              assert(target[0] === 'sub');
              invalidate('<memory>');
            }
          }
          if (type === 'call') {
            invalidate('<global>');
            invalidate('<memory>');
          }
        });
      }
      for (var i = 0; i < stats.length; i++) {
        var curr = stats[i];
        // first, look at the entire line and invalidate what we need to
        if (doInvalidations(curr) === true) {
          continue; // we saw control flow
        }
        // next, process the line and try to find useful expressions
        var skips = [];
        traverse(curr, function seekExpressions(node, type) {
          if (type === 'sub' && node[1][0] === 'name' && node[2][0] === 'binary' && node[2][1] === '>>') {
            // skip over the shift, we can't cse that
            skips.push(node[2]);
            return;
          }
          if (type === 'binary' || type === 'unary-prefix') {
            if (type === 'binary' && skips.indexOf(node) >= 0) return;
            if (measureCost(node) < MIN_COST) return;
            if (detectType(node, asmData) === ASM_NONE) return; // if we can't figure it out locally, forget it
            var str = JSON.stringify(node);
            var lookup = exps[str];
            if (!lookup) {
              // add ourselves, and set up our deps
              exps[str] = [i, node, null];
              traverse(node, function(node, type) {
                var names = [];
                if (type === 'name') {
                  var name = node[1];
                  if (!(name in asmData.params || name in asmData.vars)) name = '<global>';
                  names.push(name);
                } else if (type === 'sub') {
                  names.push('<memory>');
                } else if (type === 'call') {
                  names.push('<memory>');
                  names.push('<global>');
                }
                names.forEach(function(name) {
                  if (!deps[name]) deps[name] = [];
                  deps[name].push(str);
                });
              });
            } else {
              //printErr('CSEing ' + str);
              optimized = true;
              var type, sign;
              // with the original node plus us, this is worth optimizing out
              if (lookup[2] === null) {
                // this is the first node after the first. generate the saved var, and optimize out the original
                lookup[2] = 'CSE$' + (counter++);
                // ensure an asm coercion
                type = lookup[3] = detectType(node, asmData);
                sign = detectSign(node);
                if (sign === ASM_FLEXIBLE) sign = ASM_SIGNED;
                lookup[4] = sign;
                asmData.vars[lookup[2]] = type;
                overwrite(lookup[1], makeSignedAsmCoercion(['name', lookup[2]], type, sign));
                stats.splice(lookup[0], 0, ['stat', ['assign', true, ['name', lookup[2]], makeSignedAsmCoercion(node, type, sign)]]);
                // adjust indexes after that splice
                i++; // i must be after lookup[0]
                for (var e in exps) {
                  var curr = exps[e];
                  if (curr[2] === null) {
                    if (curr[0] >= lookup[0]) curr[0]++;
                  }
                }
              } else {
                type = lookup[3];
                sign = lookup[4];
              }
              // optimize out ourselves
              return makeSignedAsmCoercion(['name', lookup[2]], type, sign);
            }
          }
        });
        // finally, repeat invalidation processing, to not be sensitive to inter-line control flow
        doInvalidations(curr);
      }
    });
    denormalizeAsm(func, asmData);
    if (optimized) {
      simplifyExpressions(func); // remove double coercions, etc.
    }
  });
}

function safeLabelSettingInternal(func, asmData) {
  if ('label' in asmData.vars) {
    var stats = getStatements(func);
    var seenVar = false;
    for (var i = 0; i < stats.length; i++) {
      var curr = stats[i];
      if (curr[0] === 'stat') curr = curr[1];
      if (curr[0] === 'var') {
        seenVar = true;
      } else if (seenVar && curr[0] !== 'var') {
        // first location after the vars
        stats.splice(i, 0, ['stat', ['assign', true, ['name', 'label'], ['num', 0]]]);
        break;
      }
    }
  }
}

function safeLabelSetting(ast) {
  // Add an assign to label, if it exists, so that even after we minify/registerize variable names, we can tell if any vars use the asm init value of 0 - none will, so it's easy to tell
  assert(asm);
  traverseGeneratedFunctions(ast, function(func) {
    var asmData = normalizeAsm(func);
    safeLabelSettingInternal(func, asmData);
    denormalizeAsm(func, asmData);
  });
}

function simplifyIfs(ast) {
  traverseGeneratedFunctions(ast, function(func) {
    var simplifiedAnElse = false;

    traverse(func, function(node, type) {
      // simplify   if (x) { if (y) { .. } }   to   if (x ? y : 0) { .. }
      if (type === 'if') {
        var body = node[2];
        // recurse to handle chains
        while (body[0] === 'block') {
          var stats = body[1];
          if (stats.length === 0) break;
          var other = stats[stats.length-1];
          if (other[0] !== 'if') {
            // our if block does not end with an if. perhaps if have an else we can flip
            if (node[3] && node[3][0] === 'block') {
              var stats = node[3][1];
              if (stats.length === 0) break;
              var other = stats[stats.length-1];
              if (other[0] === 'if') {
                // flip node
                node[1] = flipCondition(node[1]);
                node[2] = node[3];
                node[3] = body;
                body = node[2];
              } else break;
            } else break;
          }
          // we can handle elses, but must be fully identical
          if (node[3] || other[3]) {
            if (!node[3]) break;
            if (!astCompare(node[3], other[3])) {
              // the elses are different, but perhaps if we flipped a condition we can do better
              if (astCompare(node[3], other[2])) {
                // flip other. note that other may not have had an else! add one if so; we will eliminate such things later
                if (!other[3]) other[3] = ['block', []];
                other[1] = flipCondition(other[1]);
                var temp = other[2];
                other[2] = other[3];
                other[3] = temp;
              } else break;
            }
          }
          if (stats.length > 1) {
            // try to commaify - turn everything between the ifs into a comma operator inside the second if
            var ok = true;
            for (var i = 0; i < stats.length-1; i++) {
              var curr = stats[i];
              if (curr[0] === 'stat') curr = curr[1];
              if (!(curr[0] in COMMABLE)) ok = false;
            }
            if (!ok) break;
            for (var i = stats.length-2; i >= 0; i--) {
              var curr = stats[i];
              if (curr[0] === 'stat') curr = curr[1];
              other[1] = ['seq', curr, other[1]];
            }
            stats = body[1] = [other];
          }
          if (stats.length !== 1) break;
          if (node[3]) simplifiedAnElse = true;
          node[1] = ['conditional', node[1], other[1], ['num', 0]];
          body = node[2] = other[2];
        }
      }
    });

    if (simplifiedAnElse) {
      // there may be fusing opportunities

      // we can only fuse if we remove all uses of the label. if there are
      // other ones - if the label check can be reached from elsewhere -
      // we must leave it
      var abort = false;

      var labelAssigns = {};
      traverse(func, function(node, type) {
        if (type === 'assign' && node[2][0] === 'name' && node[2][1] === 'label') {
          if (node[3][0] === 'num') {
            var value = node[3][1];
            labelAssigns[value] = (labelAssigns[value] || 0) + 1;
          } else {
            // label is assigned a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      var labelChecks = {};
      traverse(func, function(node, type) {
        if (type === 'binary' && node[1] === '==' && node[2][0] === 'binary' && node[2][1] === '|' &&
            node[2][2][0] === 'name' && node[2][2][1] === 'label') {
          if (node[3][0] === 'num') {
            var value = node[3][1];
            labelChecks[value] = (labelChecks[value] || 0) + 1;
          } else {
            // label is checked vs a dynamic value (like from indirectbr), we cannot do anything
            abort = true;
          }
        }
      });
      if (abort) return;

      var inLoop = 0; // when in a loop, we do not emit   label = 0;   in the relooper as there is no need
      traverse(func, function(node, type) {
        if (type === 'while') inLoop++;
        var stats = getStatements(node);
        if (stats) {
          for (var i = 0; i < stats.length-1; i++) {
            var pre = stats[i];
            var post = stats[i+1];
            if (pre[0] === 'if' && pre[3] && post[0] === 'if' && !post[3]) {
              var postCond = post[1];
              if (postCond[0] === 'binary' && postCond[1] === '==' &&
                  postCond[2][0] === 'binary' && postCond[2][1] === '|' &&
                  postCond[2][2][0] === 'name' && postCond[2][2][1] === 'label' &&
                  postCond[2][3][0] === 'num' && postCond[2][3][1] === 0 &&
                  postCond[3][0] === 'num') {
                var postValue = postCond[3][1];
                var preElse = pre[3];
                if (labelAssigns[postValue] === 1 && labelChecks[postValue] === 1 && preElse[0] === 'block' && preElse[1] && preElse[1].length === 1) {
                  var preStat = preElse[1][0];
                  if (preStat[0] === 'stat' && preStat[1][0] === 'assign' &&
                      preStat[1][1] === true && preStat[1][2][0] === 'name' && preStat[1][2][1] === 'label' &&
                      preStat[1][3][0] === 'num' && preStat[1][3][1] === postValue) {
                    // Conditions match, just need to make sure the post clears label
                    if (post[2][0] === 'block' && post[2][1] && post[2][1].length > 0) {
                      var postStat = post[2][1][0];
                      var haveClear =
                        postStat[0] === 'stat' && postStat[1][0] === 'assign' &&
                        postStat[1][1] === true && postStat[1][2][0] === 'name' && postStat[1][2][1] === 'label' &&
                        postStat[1][3][0] === 'num' && postStat[1][3][1] === 0;
                      if (!inLoop || haveClear) {
                        // Everything lines up, do it
                        pre[3] = post[2];
                        if (haveClear) pre[3][1].splice(0, 1); // remove the label clearing
                        stats.splice(i+1, 1); // remove the post entirely
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }, function(node, type) {
        if (type === 'while') inLoop--;
      });
    }
  });
}

// We often have branchings that are simplified so one end vanishes, and
// we then get
//   if (!(x < 5))
// or such. Simplifying these saves space and time.
function simplifyNotCompsDirect(node) {
  if (node[0] === 'unary-prefix' && node[1] === '!') {
    // de-morgan's laws do not work on floats, due to nans >:(
    if (node[2][0] === 'binary' && (!asm || (detectType(node[2][2]) === ASM_INT && detectType(node[2][3]) === ASM_INT))) {
      switch(node[2][1]) {
        case '<': return ['binary', '>=', node[2][2], node[2][3]];
        case '>': return ['binary', '<=', node[2][2], node[2][3]];
        case '<=': return ['binary', '>', node[2][2], node[2][3]];
        case '>=': return ['binary', '<', node[2][2], node[2][3]];
        case '==': return ['binary', '!=', node[2][2], node[2][3]];
        case '!=': return ['binary', '==', node[2][2], node[2][3]];
        case '===': return ['binary', '!==', node[2][2], node[2][3]];
        case '!==': return ['binary', '===', node[2][2], node[2][3]];
      }
    } else if (node[2][0] === 'unary-prefix' && node[2][1] === '!') {
      return node[2][2];
    }
  }
  if (!simplifyNotCompsPass) return node;
}

var SAFE_TO_DROP_COERCION = set('unary-prefix', 'name', 'num');

function canDropCoercion(node) {
  if (node[0] in SAFE_TO_DROP_COERCION) return true;
  if (node[0] === 'binary') {
    switch (node[1]) {
      case '>>': case '>>>': case '<<': case '|': case '^': case '&': return true;
    }
  }
  return false;
}

function simplifyCondition(node) {
  node = simplifyNotCompsDirect(node);
  // on integers, if (x == 0) is the same as if (x), and if (x != 0) as if (!x)
  if (node[0] === 'binary' && (node[1] === '==' || node[1] === '!=')) {
    var target = null;
    if (detectType(node[2]) === ASM_INT && node[3][0] === 'num' && node[3][1] === 0) {
      target = node[2];
    } else if (detectType(node[3]) === ASM_INT && node[2][0] === 'num' && node[2][1] === 0) {
      target = node[3];
    }
    if (target) {
      if (target[0] === 'binary' && (target[1] === '|' || target[1] === '>>>') && target[3][0] === 'num' && target[3][1] === 0 &&
          canDropCoercion(target[2])) {
        target = target[2]; // drop the coercion, in a condition it is ok to do if (x)
      }
      if (node[1] === '==') {
        return ['unary-prefix', '!', target];
      } else {
        return target;
      }
    }
  }
  return node;
}

function flipCondition(cond) {
  return simplifyNotCompsDirect(['unary-prefix', '!', cond]);
}

var simplifyNotCompsPass = false;

function simplifyNotComps(ast) {
  simplifyNotCompsPass = true;
  traverse(ast, simplifyNotCompsDirect);
  simplifyNotCompsPass = false;
}

function isMathFunc(name) {
  return /^Math_/.test(name);
}

function callHasSideEffects(node) { // checks if the call itself (not the args) has side effects (or is not statically known)
  return !(node[1][0] === 'name' && isMathFunc(node[1][1]));
}

function hasSideEffects(node) { // this is 99% incomplete!
  switch (node[0]) {
    case 'num': case 'name': case 'string': return false;
    case 'unary-prefix': return hasSideEffects(node[2]);
    case 'binary': return hasSideEffects(node[2]) || hasSideEffects(node[3]);
    case 'sub': return hasSideEffects(node[1]) || hasSideEffects(node[2]);
    case 'call': {
      if (callHasSideEffects(node)) return true;
      // This is a statically known call, with no side effects. only args can side effect us
      var args = node[2];
      var num = args.length;
      for (var i = 0; i < num; i++) {
        if (hasSideEffects(args[i])) return true;
      }
      return false;
    }
    case 'conditional': return hasSideEffects(node[1]) || hasSideEffects(node[2]) || hasSideEffects(node[3]); 
    default: return true;
  }
}

// checks if a node has just basic operations, nothing with side effects nor that can notice side effects, which
// implies we can move it around in the code
function triviallySafeToMove(node, asmData) {
  var ok = true;
  traverse(node, function(node, type) {
    switch (type) {
      case 'stat': case 'binary': case 'unary-prefix': case 'assign': case 'num':
        break;
      case 'name':
        if (!(node[1] in asmData.vars) && !(node[1] in asmData.params)) ok = false;
        break;
      case 'call':
        if (callHasSideEffects(node)) ok = false;
        break;
      default:
        ok = false;
    }  
  });
  return ok;
}

// Clear out empty ifs and blocks, and redundant blocks/stats and so forth
// Operates on generated functions only
function vacuum(ast) {
  function isEmpty(node) {
    if (!node) return true;
    if (node[0] === 'toplevel' && (!node[1] || node[1].length === 0)) return true;
    if (node[0] === 'block' && (!node[1] || (typeof node[1] != 'object') || node[1].length === 0 || (node[1].length === 1 && isEmpty(node[1])))) return true;
    return false;
  }
  function simplifyList(node, si) {
    var changed = false;
    // Merge block items into this list, thus removing unneeded |{ .. }|'s
    var statements = node[si];
    var i = 0;
    while (i < statements.length) {
      var subNode = statements[i];
      if (subNode[0] === 'block') {
        statements.splice.apply(statements, [i, 1].concat(subNode[1] || []));
        changed = true;
      } else {
        i++;
      }
    }
    // Remove empty items
    var pre = node[si].length;
    node[si] = node[si].filter(function(node) { return !isEmpty(node) });
    if (node[si].length < pre) changed = true;
    if (changed) {
      return node;
    }
  }
  function vacuumInternal(node) {
    traverseChildren(node, vacuumInternal);
    var ret;
    switch(node[0]) {
      case 'block': {
        if (node[1] && node[1].length === 1 && node[1][0][0] === 'block') {
          return node[1][0];
        } else if (typeof node[1] === 'object') {
          ret = simplifyList(node, 1);
          if (ret) return ret;
        }
      } break;
      case 'stat': {
        if (node[1][0] === 'block') {
          return node[1];
        } else if (!hasSideEffects(node[1])) {
          return emptyNode();
        }
      } break;
      case 'defun': {
        if (node[3].length === 1 && node[3][0][0] === 'block') {
          node[3] = node[3][0][1];
          return node;
        } else {
          ret = simplifyList(node, 3);
          if (ret) return ret;
        }
      } break;
      case 'do': {
        if (node[1][0] === 'num' && node[2][0] === 'toplevel' && (!node[2][1] || node[2][1].length === 0)) {
          return emptyNode();
        } else if (isEmpty(node[2]) && !hasSideEffects(node[1])) {
          return emptyNode();
        }
      } break;
      case 'label': {
        if (node[2] && node[2][0] === 'toplevel' && (!node[2][1] || node[2][1].length === 0)) {
          return emptyNode();
        }
      } break;
      case 'if': {
        var empty2 = isEmpty(node[2]), empty3 = isEmpty(node[3]), has3 = node.length === 4;
        if (!empty2 && empty3 && has3) { // empty else clauses
          return node.slice(0, 3);
        } else if (empty2 && !empty3) { // empty if blocks
          return ['if', ['unary-prefix', '!', node[1]], node[3]];
        } else if (empty2 && empty3) {
          if (hasSideEffects(node[1])) {
            return ['stat', node[1]];
          } else {
            return emptyNode();
          }
        }
      } break;
    }
  }
  traverseGeneratedFunctions(ast, function(node) {
    vacuumInternal(node);
    simplifyNotComps(node);
    removeEmptySubNodes(node);
  });
}

function getStatements(node) {
  if (node[0] === 'defun') {
    return node[3];
  } else if (node[0] === 'block') {
    return node[1];
  } else {
    return null;
  }
}

// Multiple blocks from the relooper are, in general, implemented by
//   if (label === x) { } else if ..
// and branching into them by
//   if (condition) { label === x } else ..
// We can hoist the multiple block into the condition, thus removing code and one 'if' check
function hoistMultiples(ast) {
  traverseGeneratedFunctions(ast, function(node) {
    traverse(node, function(node, type) {
      var statements = getStatements(node);
      if (!statements) return;
      var modified = false;
      for (var i = 0; i < statements.length-1; i++) {
        var modifiedI = false;
        var pre = statements[i];
        if (pre[0] != 'if') continue;
        var post = statements[i+1];
        // Look into some block types. shell() will then recreate the shell that we looked into
        var postInner = post;
        var shellLabel = false, shellDo = false;
        while (true) {
          if (postInner[0] === 'label') {
            shellLabel = postInner[1];
            postInner = postInner[2];
          } else if (postInner[0] === 'do') {
            shellDo = postInner[1];
            postInner = postInner[2][1][0];
          } else {
            break; // give up
          }
        }
        if (postInner[0] != 'if') continue;
        // Look into this if, and its elseifs
        while (postInner && postInner[0] === 'if') {
          var cond = postInner[1];
          if (cond[0] === 'binary' && cond[1] === '==' && cond[2][0] === 'name' && cond[2][1] === 'label') {
            assert(cond[3][0] === 'num');
            // We have a valid Multiple check here. Try to hoist it, look for the source in |pre| and its else's
            var labelNum = cond[3][1];
            var labelBlock = postInner[2];
            assert(labelBlock[0] === 'block');
            var found = false;
            traverse(pre, function(preNode, preType) {
              if (!found && preType === 'assign' && preNode[2][0] === 'name' && preNode[2][1] === 'label') {
                assert(preNode[3][0] === 'num');
                if (preNode[3][1] === labelNum) {
                  // That's it! Hoist away. We can also throw away the label setting as its goal has already been achieved
                  found = true;
                  modifiedI = true;
                  postInner[2] = ['block', []];
                  return labelBlock;
                }
              }
            });
          }
          postInner = postInner[3]; // Proceed to look in the else clause
        }
        if (modifiedI) {
          if (shellDo) {
            statements[i] = ['do', shellDo, ['block', [statements[i]]]];
          }
          if (shellLabel) {
            statements[i] = ['label', shellLabel, statements[i]];
          }
        }
      }
      if (modified) return node;
    });

    // After hoisting in this function, it is safe to remove { label = x; } blocks, because
    // if they were leading to the next code right after them, they would be hoisted, and if they
    // are going to some other place entirely, they would break or continue. The only risky
    // situation is if the code after us is a multiple, in which case we might be checking for
    // this label inside it (or in a later multiple, even)
    function tryEliminate(node) {
      if (node[0] === 'if') {
        var replaced;
        if (replaced = tryEliminate(node[2])) node[2] = replaced;
        if (node[3] && (replaced = tryEliminate(node[3]))) node[3] = replaced;
      } else {
        if (node[0] === 'block' && node[1] && node[1].length > 0) {
          var subNode = node[1][node[1].length-1];
          if (subNode[0] === 'stat' && subNode[1][0] === 'assign' && subNode[1][2][0] === 'name' &&
              subNode[1][2][1] === 'label' && subNode[1][3][0] === 'num') {
            if (node[1].length === 1) {
              return emptyNode();
            } else {
              node[1].splice(node[1].length-1, 1);
              return node;
            }
          }
        }
      }
      return false;
    }
    function getActualStatement(node) { // find the actual active statement, ignoring a label and one-time do loop
      if (node[0] === 'label') node = node[2];
      if (node[0] === 'do') node = node[2];
      if (node[0] === 'block' && node[1].length === 1) node = node[1][0];
      return node;
    }
    vacuum(node);
    traverse(node, function(node, type) {
      var statements = getStatements(node);
      if (!statements) return;
      for (var i = 0; i < statements.length-1; i++) {
        var curr = getActualStatement(statements[i]);
        var next = statements[i+1];
        if (curr[0] === 'if' && next[0] != 'if' && next[0] != 'label' && next[0] != 'do' && next[0] != 'while') {
          tryEliminate(curr);
        }
      }
    });
  });

  vacuum(ast);

  // Afterpass: Reduce
  //    if (..) { .. break|continue } else { .. }
  // to
  //    if (..) { .. break|continue } ..
  traverseGenerated(ast, function(container, type) {
    var statements = getStatements(container);
    if (!statements) return;
    for (var i = 0; i < statements.length; i++) {
      var node = statements[i];
      if (node[0] === 'if' && node[2][0] === 'block' && node[3] && node[3][0] === 'block') {
        var stat1 = node[2][1], stat2 = node[3][1];
        // If break|continue in the latter and not the former, reverse them
        if (!(stat1[stat1.length-1][0] in LOOP_FLOW) && (stat2[stat2.length-1][0] in LOOP_FLOW)) {
          var temp = node[3];
          node[3] = node[2];
          node[2] = temp;
          node[1] = flipCondition(node[1]);
          stat1 = node[2][1];
          stat2 = node[3][1];
        }
        if (stat1[stat1.length-1][0] in LOOP_FLOW) {
          statements.splice.apply(statements, [i+1, 0].concat(stat2));
          node[3] = null;
        }
      }
    }
  });
}

// Simplifies loops
// WARNING: This assumes all loops and breaks/continues are labelled
function loopOptimizer(ast) {
  // Remove unneeded labels and one-time (do while(0)) loops. It is convenient to do these both at once.
  function passTwo(ast) {
    var neededDos = [];
    // Find unneeded labels
    traverseGenerated(ast, function(node, type, stack) {
      if (type === 'label' && node[2][0] in LOOP) {
        // this is a labelled loop. we don't know if it's needed yet. Mark its label for removal for now now.
        stack.push(node);
        node[1] = '+' + node[1];
      } else if (type in LOOP) {
        stack.push(node);
      } else if (type in LOOP_FLOW) {
        // Find topmost loop, and its label if there is one
        var lastLabel = null, lastLoop = null, i = stack.length-1;
        while (i >= 0 && !lastLoop) {
          if (stack[i][0] in LOOP) lastLoop = stack[i];
          i--;
        }
        assert(lastLoop, 'Cannot break/continue without a Label');
        while (i >= 0 && !lastLabel) {
          if (stack[i][0] in LOOP) break; // another loop in the middle - no label for lastLoop
          if (stack[i][0] === 'label') lastLabel = stack[i];
          i--;
        }
        var ident = node[1]; // there may not be a label ident if this is a simple break; or continue;
        var plus = '+' + ident;
        if (lastLabel && ident && (ident === lastLabel[1] || plus === lastLabel[1])) {
          // If this is a 'do' loop, this break means we actually need it.
          neededDos.push(lastLoop);
          // We don't need the control flow command to have a label - it's referring to the current loop
          return [node[0]];
        } else {
          if (!ident) {
            // No label on the break/continue, so keep the last loop alive (no need for its label though)
            neededDos.push(lastLoop);
          } else {
            // Find the label node that needs to stay alive
            stack.forEach(function(label) {
              if (!label) return;
              if (label[1] === plus) label[1] = label[1].substr(1); // Remove '+', marking it as needed
            });
          }
        }
      }
    }, null, []);
    // We return whether another pass is necessary
    var more = false;
    // Remove unneeded labels
    traverseGenerated(ast, function(node, type) {
      if (type === 'label' && node[1][0] === '+') {
        more = true;
        var ident = node[1].substr(1);
        // Remove label from loop flow commands
        traverse(node[2], function(node2, type) {
          if (type in LOOP_FLOW && node2[1] === ident) {
            return [node2[0]];
          }
        });
        return node[2]; // Remove the label itself on the loop
      }
    });
    // Remove unneeded one-time loops. We need such loops if (1) they have a label, or (2) they have a direct break so they are in neededDos.
    // First, add all labeled loops of this nature to neededDos
    traverseGenerated(ast, function(node, type) {
      if (type === 'label' && node[2][0] === 'do') {
        neededDos.push(node[2]);
      }
    });
    // Remove unneeded dos, we know who they are now
    traverseGenerated(ast, function(node, type) {
      if (type === 'do' && neededDos.indexOf(node) < 0) {
        assert(jsonCompare(node[1], ['num', 0]), 'Trying to remove a one-time do loop that is not one of our generated ones.;');
        more = true;
        return node[2];
      }
    });
    return more;
  }

  // Go

  // TODO: pass 1: Removal of unneeded continues, breaks if they get us to where we are already going. That will
  //               help the next pass.

  // Multiple pass two runs may be needed, as we remove one-time loops and so forth
  do {
    var more = passTwo(ast);
    vacuum(ast);
  } while (more);

  vacuum(ast);
}

function unVarify(vars, ret) { // transform var x=1, y=2 etc. into (x=1, y=2), i.e., the same assigns, but without a var definition
  ret = ret || [];
  ret[0] = 'stat';
  if (vars.length === 1) {
    ret[1] = ['assign', true, ['name', vars[0][0]], vars[0][1]];
  } else {
    ret[1] = [];
    var curr = ret[1];
    for (var i = 0; i < vars.length-1; i++) {
      curr[0] = 'seq';
      curr[1] = ['assign', true, ['name', vars[i][0]], vars[i][1]];
      if (i != vars.length-2) curr = curr[2] = [];
    }
    curr[2] = ['assign', true, ['name', vars[vars.length-1][0]], vars[vars.length-1][1]];
  }
  return ret;
}

// asm.js support code - normalize (convert asm.js code to 'normal' JS, without
// annotations, plus explicit metadata) and denormalize (vice versa)
var ASM_INT = 0;
var ASM_DOUBLE = 1;
var ASM_FLOAT = 2;
var ASM_FLOAT32X4 = 3;
var ASM_FLOAT64X2 = 4;
var ASM_INT8X16 = 5;
var ASM_INT16X8 = 6;
var ASM_INT32X4 = 7;
var ASM_BOOL8X16 = 8;
var ASM_BOOL16X8 = 9;
var ASM_BOOL32X4 = 10;
var ASM_BOOL64X2 = 11;
var ASM_NONE = 12;

var ASM_SIG = {
  0: 'i',
  1: 'd',
  2: 'f',
  3: 'F',
  4: 'D',
  5: 'B',
  6: 'S',
  7: 'I',
  8: 'Z', // For Bool SIMD.js types, arbitrarily use consecutive letters ZXCV.
  9: 'X',
  10: 'C',
  11: 'V',
  12: 'v'
};

var ASM_FLOAT_ZERO = null; // TODO: share the entire node?

function detectType(node, asmInfo, inVarDef) {
  // for params, +x vs x|0, for vars, 0.0 vs 0
  switch (node[0]) {
    case 'num': {
      if (node[1].toString().indexOf('.') >= 0) return ASM_DOUBLE;
      return ASM_INT;
    }
    case 'unary-prefix': {
      switch (node[1]) {
        case '+': return ASM_DOUBLE;
        case '-': return detectType(node[2], asmInfo, inVarDef);
        case '!': case '~': return ASM_INT;
      }
      break;
    }
    case 'call': {
      if (node[1][0] === 'name') {
        switch (node[1][1]) {
          case 'Math_fround':          return ASM_FLOAT;
          case 'SIMD_Float32x4':
          case 'SIMD_Float32x4_check': return ASM_FLOAT32X4;
          case 'SIMD_Float64x2':
          case 'SIMD_Float64x2_check': return ASM_FLOAT64X2;
          case 'SIMD_Int8x16':
          case 'SIMD_Int8x16_check':   return ASM_INT8X16;
          case 'SIMD_Int16x8':
          case 'SIMD_Int16x8_check':   return ASM_INT16X8;
          case 'SIMD_Int32x4':
          case 'SIMD_Int32x4_check':   return ASM_INT32X4;
          case 'SIMD_Bool8x16':
          case 'SIMD_Bool8x16_check':  return ASM_BOOL8X16;
          case 'SIMD_Bool16x8':
          case 'SIMD_Bool16x8_check':  return ASM_BOOL16X8;
          case 'SIMD_Bool32x4':
          case 'SIMD_Bool32x4_check':  return ASM_BOOL32X4;
          case 'SIMD_Bool64x2':
          case 'SIMD_Bool64x2_check':  return ASM_BOOL64X2;
          default: break;
        }
      }
      return ASM_NONE;
    }
    case 'name': {
      if (asmInfo) {
        var ret = getAsmType(node[1], asmInfo);
        if (ret !== ASM_NONE) return ret;
      }
      if (!inVarDef) {
        switch (node[1]) {
          case 'inf': case 'nan': return ASM_DOUBLE; // TODO: when minified
          case 'tempRet0': return ASM_INT;
        }
        return ASM_NONE;
      }
      // We are in a variable definition, where Math_fround(0) optimized into a global constant becomes f0 = Math_fround(0)
      if (!ASM_FLOAT_ZERO) ASM_FLOAT_ZERO = node[1];
      else assert(ASM_FLOAT_ZERO === node[1]);
      return ASM_FLOAT;
    }
    case 'binary': {
      switch (node[1]) {
        case '+': case '-':
        case '*': case '/': case '%': {
          var ret = detectType(node[2], asmInfo, inVarDef);
          if (ret !== ASM_NONE) return ret;
          return detectType(node[3], asmInfo, inVarDef)
        }
        case '|': case '&': case '^': case '<<': case '>>': case '>>>':
        case '==': case '!=': case '<': case '<=': case '>': case '>=': {
          return ASM_INT;
        }
      }
      break;
    }
    case 'conditional': case 'seq': {
      return detectType(node[2], asmInfo, inVarDef);
    }
    case 'sub': {
      assert(node[1][0] === 'name');
      if (!parseHeap(node[1][1])) return ASM_NONE;
      return parseHeapTemp.float ? ASM_DOUBLE : ASM_INT; // XXX ASM_FLOAT?
    }
  }
  assert(0 , 'horrible ' + JSON.stringify(node));
}

function isAsmCoercion(node) {
  if (node[0] === 'binary' && ((node[1] === '|' && node[3][0] === 'num' && node[3][1] === 0) ||
                               (node[1] === '>>>' && node[3][0] === 'num' && node[3][1] === 0))) return true;
  if (node[0] === 'unary-prefix' && (node[1] === '+' || (node[1] === '~' && node[2][0] === 'unary-prefix' && node[2][1] === '~'))) return true;
  return false;
}

function makeAsmCoercion(node, type) {
  switch (type) {
    case ASM_INT:       return ['binary', '|', node, ['num', 0]];
    case ASM_DOUBLE:    return ['unary-prefix', '+', node];
    case ASM_FLOAT:     return ['call', ['name', 'Math_fround'], [node]];
    case ASM_FLOAT32X4: return ['call', ['name', 'SIMD_Float32x4_check'], [node]];
    case ASM_FLOAT64X2: return ['call', ['name', 'SIMD_Float64x2_check'], [node]];
    case ASM_INT8X16:   return ['call', ['name', 'SIMD_Int8x16_check'],   [node]];
    case ASM_INT16X8:   return ['call', ['name', 'SIMD_Int16x8_check'],   [node]];
    case ASM_INT32X4:   return ['call', ['name', 'SIMD_Int32x4_check'],   [node]];
    case ASM_BOOL8X16:  return ['call', ['name', 'SIMD_Bool8x16_check'],  [node]];
    case ASM_BOOL16X8:  return ['call', ['name', 'SIMD_Bool16x8_check'],  [node]];
    case ASM_BOOL32X4:  return ['call', ['name', 'SIMD_Bool32x4_check'],  [node]];
    case ASM_BOOL64X2:  return ['call', ['name', 'SIMD_Bool64x2_check'],  [node]];
    case ASM_NONE:
    default: return node; // non-validating code, emit nothing XXX this is dangerous, we should only allow this when we know we are not validating
  }
}

function makeSignedAsmCoercion(node, type, sign) {
  if (type !== ASM_INT || sign === ASM_SIGNED) return makeAsmCoercion(node, type);
  if (sign === ASM_UNSIGNED) return ['binary', '>>>', node, ['num', 0]];
  assert(0);
}

function makeAsmCoercedZero(type) {
  switch (type) {
    case ASM_INT: return ['num', 0];
    case ASM_DOUBLE: return ['unary-prefix', '+', ['num', 0]];
    case ASM_FLOAT: {
      if (ASM_FLOAT_ZERO) {
        return ['name', ASM_FLOAT_ZERO];
      } else {
        return ['call', ['name', 'Math_fround'], [['num', 0]]];
      }
    }
    case ASM_FLOAT32X4: {
      return ['call', ['name', 'SIMD_Float32x4'], [['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_FLOAT64X2: {
      return ['call', ['name', 'SIMD_Float64x2'], [['num', 0], ['num', 0]]];
    }
    case ASM_INT8X16: {
      return ['call', ['name', 'SIMD_Int8x16'], [['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_INT16X8: {
      return ['call', ['name', 'SIMD_Int16x8'], [['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_INT32X4: {
      return ['call', ['name', 'SIMD_Int32x4'], [['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_BOOL8X16: {
      return ['call', ['name', 'SIMD_Bool8x16'], [['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_BOOL16X8: {
      return ['call', ['name', 'SIMD_Bool16x8'], [['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_BOOL32X4: {
      return ['call', ['name', 'SIMD_Bool32x4'], [['num', 0], ['num', 0], ['num', 0], ['num', 0]]];
    }
    case ASM_BOOL64X2: {
      return ['call', ['name', 'SIMD_Bool64x2'], [['num', 0], ['num', 0]]];
    }
    default: throw 'wha? ' + JSON.stringify(type) + new Error().stack;
  }
}

function makeAsmVarDef(v, type) {
  return [v, makeAsmCoercedZero(type)];
}

function getAsmType(name, asmInfo) {
  if (name in asmInfo.vars) return asmInfo.vars[name];
  if (name in asmInfo.params) return asmInfo.params[name];
  return ASM_NONE;
}

function getCombinedType(node1, node2, asmData, hint) {
  var type1 = detectType(node1, asmData);
  var type2 = detectType(node2, asmData);
  if (type1 === ASM_NONE && type2 === ASM_NONE) {
    assert(hint !== undefined);
    return hint;
  }
  if (type1 === ASM_NONE) {
    assert(type2 != ASM_NONE);
    return type2;
  } else if (type2 === ASM_NONE) {
    assert(type1 != ASM_NONE);
    return type1;
  }
  if (type1 !== type2) {
    if (type1 === hint || type2 === hint) return hint;
    assert(0, "can't figure it out " + JSON.stringify([node1, '....', node2, '     ', type1, type2, hint], null, '  '));
  }
  return type1;
}

var ASM_FLEXIBLE = 0; // small constants can be signed or unsigned, variables are also flexible
var ASM_SIGNED = 1;
var ASM_UNSIGNED = 2;
var ASM_NONSIGNED = 3;

function detectSign(node) {
  switch (node[0]) {
    case 'binary': {
      switch(node[1]) {
        case '|': case '&': case '^': case '<<': case '>>': return ASM_SIGNED;
        case '>>>': return ASM_UNSIGNED;
        case '+': case '-': return ASM_FLEXIBLE;
        case '*': {
          // a double, unless one is a small int and the other is an int, in
          // which case one is a num. that can't be a double, since then it
          // would be a +num.
          if (node[2][0] === 'num' || node[3][0] === 'num') return ASM_FLEXIBLE;
          return ASM_NONSIGNED;
        }
        case '/': return ASM_NONSIGNED; // without a coercion, this is double
        case '==': case '!=': case '<': case '<=': case '>': case '>=': return ASM_SIGNED;
        default: throw 'yikes ' + node[1];
      }
      break;
    }
    case 'unary-prefix': {
      switch(node[1]) {
        case '-': return ASM_FLEXIBLE;
        case '+': return ASM_NONSIGNED; // XXX double
        case '~': return ASM_SIGNED;
        default: throw 'yikes';
      }
      break;
    }
    case 'num': {
      var value = node[1];
      if (value < 0) return ASM_SIGNED;
      if (value > (-1>>>0) || value % 1 !== 0) return ASM_NONSIGNED;
      if (value === (value | 0)) return ASM_FLEXIBLE;
      return ASM_UNSIGNED;
    }
    case 'name': return ASM_FLEXIBLE;
    case 'conditional': case 'seq': {
      return detectSign(node[2]);
    }
    case 'call': {
      if (node[1][0] === 'name') {
        switch (node[1][1]) {
          case 'Math_fround': return ASM_NONSIGNED
          default: break;
        }
      }
    }
  }
  assert(0 , 'badd ' + JSON.stringify(node));
}

function getCombinedSign(node1, node2, hint) {
  var sign1 = detectSign(node1);
  var sign2 = detectSign(node2);
  if (sign1 === ASM_FLEXIBLE && sign2 === ASM_FLEXIBLE) {
    return ASM_FLEXIBLE;
  }
  if (sign1 === ASM_FLEXIBLE) {
    assert(sign2 != ASM_FLEXIBLE);
    return sign2;
  } else if (sign2 === ASM_FLEXIBLE) {
    assert(sign1 != ASM_FLEXIBLE);
    return sign1;
  }
  if (sign1 === sign2) return sign1;
  if (sign1 === hint || sign2 === hint) return hint;
  if ((sign1 === ASM_SIGNED && sign2 === ASM_UNSIGNED) ||
      (sign1 === ASM_UNSIGNED && sign2 === ASM_SIGNED)) {
    return ASM_FLEXIBLE;
  }
  assert(0, JSON.stringify([node1, '      ', node2, sign1, sign2, hint]));
}

function getSignature(func, asmData) {
  var ret = asmData.ret >= 0 ? ASM_SIG[asmData.ret] : 'v';
  for (var i = 0; i < func[2].length; i++) {
    ret += ASM_SIG[asmData.params[func[2][i]]];
  }
  return ret;
}

function normalizeAsm(func) {
  //printErr('pre-normalize \n\n' + astToSrc(func) + '\n\n');
  var data = {
    params: {}, // ident => ASM_* type
    vars: {}, // ident => ASM_* type
    inlines: [], // list of inline assembly copies
    ret: undefined,
  };
  // process initial params
  var stats = func[3];
  var i = 0;
  while (i < stats.length) {
    var node = stats[i];
    if (node[0] != 'stat' || node[1][0] != 'assign' || node[1][2][0] != 'name') break;
    node = node[1];
    var name = node[2][1];
    if (func[2] && func[2].indexOf(name) < 0) break; // not an assign into a parameter, but a global
    if (name in data.params) break; // already done that param, must be starting function body
    data.params[name] = detectType(node[3]);
    stats[i] = emptyNode();
    i++;
  }
  // process initial variable definitions
  outer:
  while (i < stats.length) {
    var node = stats[i];
    if (node[0] != 'var') break;
    for (var j = 0; j < node[1].length; j++) {
      var v = node[1][j];
      var name = v[0];
      var value = v[1];
      if (!(name in data.vars)) {
        data.vars[name] = detectType(value, null, true);
        v.length = 1; // make an un-assigning var
      } else {
        assert(j === 0, 'cannot break in the middle');
        break outer;
      }
    }
    i++;
  }
  // look for other var definitions and collect them
  while (i < stats.length) {
    traverse(stats[i], function(node, type) {
      if (type === 'var') {
        assert(0, 'should be no vars to fix! ' + func[1] + ' : ' + JSON.stringify(node));
      } else if (type === 'call' && node[1][0] === 'function') {
        assert(!node[1][1]); // anonymous functions only
        data.inlines.push(node[1]);
        node[1] = ['name', 'inlinejs']; // empty out body, leave arguments, so they are eliminated/minified properly
      }
    });
    i++;
  }
  // look for final 'return' statement to get return type.
  var retStmt = stats[stats.length - 1];
  if (retStmt && retStmt[0] === 'return' && retStmt[1]) {
    data.ret = detectType(retStmt[1]);
  }
  //printErr('normalized \n\n' + astToSrc(func) + '\n\nwith: ' + JSON.stringify(data));
  return data;
}

function denormalizeAsm(func, data) {
  //printErr('pre-denormalize \n\n' + astToSrc(func) + '\n\nwith: ' + JSON.stringify(data));
  var stats = func[3];
  // Remove var definitions, if any
  for (var i = 0; i < stats.length; i++) {
    if (stats[i][0] === 'var') {
      stats[i] = emptyNode();
    } else {
      if (!isEmptyNode(stats[i])) break;
    }
  }
  // calculate variable definitions
  var varDefs = [];
  for (var v in data.vars) {
    varDefs.push(makeAsmVarDef(v, data.vars[v]));
  }
  // each param needs a line; reuse emptyNodes as much as we can
  var numParams = 0;
  for (var i in data.params) numParams++;
  var emptyNodes = 0;
  while (emptyNodes < stats.length) {
    if (!isEmptyNode(stats[emptyNodes])) break;
    emptyNodes++;
  }
  var neededEmptyNodes = numParams + (varDefs.length ? 1 : 0); // params plus one big var if there are vars
  if (neededEmptyNodes > emptyNodes) {
    var args = [0, 0];
    for (var i = 0; i < neededEmptyNodes - emptyNodes; i++) args[i+2] = 0;
    stats.splice.apply(stats, args);
  } else if (neededEmptyNodes < emptyNodes) {
    stats.splice(0, emptyNodes - neededEmptyNodes);
  }
  // add param coercions
  var next = 0;
  func[2].forEach(function(param) {
    stats[next++] = ['stat', ['assign', true, ['name', param], makeAsmCoercion(['name', param], data.params[param])]];
  });
  if (varDefs.length) {
    stats[next] = ['var', varDefs];
  }
  if (data.inlines.length > 0) {
    var i = 0;
    traverse(func, function(node, type) {
      if (type === 'call' && node[1][0] === 'name' && node[1][1] === 'inlinejs') {
        node[1] = data.inlines[i++]; // swap back in the body
      }
    });
  }
  // ensure that there's a final 'return' statement if needed.
  if (data.ret !== undefined) {
    var retStmt = stats[stats.length - 1];
    if (!retStmt || retStmt[0] !== 'return') {
      stats.push(['return', makeAsmCoercedZero(data.ret)]);
    }
  }
  //printErr('denormalized \n\n' + astToSrc(func) + '\n\n');
}

function getFirstIndexInNormalized(func, data) {
  // In a normalized asm function, return the index of the first element that is not not defs or annotation
  var stats = func[3];
  var i = stats.length-1;
  while (i >= 0) {
    var stat = stats[i];
    if (stat[0] == 'var') break;
    i--;
  }
  return i+1;
}

function getStackBumpNode(ast) {
  var found = null;
  traverse(ast, function(node, type) {
    if (type === 'assign' && node[2][0] === 'name' && node[2][1] === 'STACKTOP') {
      var value = node[3];
      if (value[0] === 'name') return true;
      if (value[0] == 'binary' && value[1] == '&') return; // this is an alignment fix, ignore
      assert(value[0] == 'binary' && value[1] == '|' && value[2][0] == 'binary' && value[2][1] == '+' && value[2][2][0] == 'name' && value[2][2][1] == 'STACKTOP');
      if (value[2][3][0] !== 'num') return; // non-constant bump, ignore
      found = node;
      return true;
    }
  });
  return found;
}

function getStackBumpSize(ast) {
  var node = getStackBumpNode(ast);
  return node ? node[3][2][3][1] : 0;
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

// Very simple 'registerization', coalescing of variables into a smaller number.
//
// We do not optimize when there are switches, so this pass only makes sense with
// relooping.
// TODO: Consider how this fits in with the rest of the optimization toolchain. Do
//       we still need the eliminator? Closure? And in what order? Perhaps just
//       closure simple?
function registerize(ast) {
  traverseGeneratedFunctions(ast, function(fun) {
    if (asm) var asmData = normalizeAsm(fun);
    if (!asm) {
      var hasFunction = false;
      traverse(fun, function(node, type) {
        if (type === 'function') hasFunction = true;
      });
      if (hasFunction) {
        return; // inline assembly, and not asm (where we protect it in normalize/denormalize), so abort registerize pass
      }
    }
    // Add parameters as a first (fake) var (with assignment), so they get taken into consideration
    var params = {}; // note: params are special, they can never share a register between them (see later)
    if (fun[2] && fun[2].length) {
      var assign = ['num', 0];
      fun[3].unshift(['var', fun[2].map(function(param) {
        params[param] = 1;
        return [param, assign];
      })]);
    }
    if (asm) {
      // copy params into vars
      for (var p in asmData.params) asmData.vars[p] = asmData.params[p];
      //printErr('fake params: \n\n' + astToSrc(fun) + '\n\n');
    }
    // Replace all var definitions with assignments; we will add var definitions at the top after we registerize
    // We also mark local variables - i.e., having a var definition
    var localVars = {};
    var allVars = {};
    var hasSwitch = false; // we cannot optimize variables if there is a switch, unless in asm mode
    traverse(fun, function(node, type) {
      if (type === 'var') {
        node[1].forEach(function(defined) { localVars[defined[0]] = 1 });
        var vars = node[1].filter(function(varr) { return varr[1] });
        if (vars.length >= 1) {
          return unVarify(vars);
        } else {
          return emptyNode();
        }
      } else if (type === 'switch') {
        hasSwitch = true;
      } else if (type === 'name') {
        allVars[node[1]] = 1;
      }
    });
    vacuum(fun);
    var regTypes = {};
    function getNewRegName(num, name) {
      var ret;
      if (!asm) {
        ret = 'r' + num;
      } else {
        var type = asmData.vars[name];
        switch (type) {
          case ASM_INT:       ret = 'i'; break;
          case ASM_DOUBLE:    ret = 'd'; break;
          case ASM_FLOAT:     ret = 'f'; break;
          case ASM_FLOAT32X4: ret = 'F4'; break;
          case ASM_FLOAT64X2: ret = 'F2'; break;
          case ASM_INT8X16:   ret = 'I16'; break;
          case ASM_INT16X8:   ret = 'I8'; break;
          case ASM_INT32X4:   ret = 'I4'; break;
          case ASM_BOOL8X16:  ret = 'B16'; break;
          case ASM_BOOL16X8:  ret = 'B8'; break;
          case ASM_BOOL32X4:  ret = 'B4'; break;
          case ASM_BOOL64X2:  ret = 'B2'; break;
          case ASM_NONE:      ret = 'Z'; break;
          default: assert(false, 'type ' + type + ' doesn\'t have a name yet');
        }
        ret += num;
        regTypes[ret] = type;
      }
      if (ret in allVars) {
        assert(ret in localVars, 'register shadows non-local name');
      }
      return ret;
    }
    // Find the # of uses of each variable.
    // While doing so, check if all a variable's uses are dominated in a simple
    // way by a simple assign, if so, then we can assign its register to it
    // just for its definition to its last use, and not to the entire toplevel loop,
    // we call such variables "optimizable"
    var varUses = {};
    var level = 1;
    var levelDominations = {};
    var varLevels = {};
    var possibles = {};
    var unoptimizables = {};
    function purgeLevel() {
      // Invalidate all dominating on this level, further users make it unoptimizable
      for (var name in levelDominations[level]) {
        varLevels[name] = 0;
      }
      levelDominations[level] = null;
      level--;
    }
    traverse(fun, function possibilifier(node, type) {
      if (type === 'name') {
        var name = node[1];
        if (localVars[name]) {
          if (!varUses[name]) varUses[name] = 0;
          varUses[name]++;
          if (possibles[name] && !varLevels[name]) unoptimizables[name] = 1; // used outside of simple domination
        }
      } else if (type === 'assign' && typeof node[1] != 'string') {
        if (node[2] && node[2][0] === 'name') {
          var name = node[2][1];
          // if local and not yet used, this might be optimizable if we dominate
          // all other uses
          if (localVars[name] && !varUses[name] && !varLevels[name]) {
            possibles[name] = 1;
            varLevels[name] = level;
            if (!levelDominations[level]) levelDominations[level] = {};
            levelDominations[level][name] = 1;
          }
        }
      } else if (type in CONTROL_FLOW) {
        // recurse children, in the context of a loop
        switch(type) {
          case 'while': case 'do': {
            traverse(node[1], possibilifier);
            level++;
            traverse(node[2], possibilifier);
            purgeLevel();
            break;
          }
          case 'for': {
            traverse(node[1], possibilifier);
            for (var i = 2; i <= 4; i++) {
              level++;
              traverse(node[i], possibilifier);
              purgeLevel();
            }
            break;
          }
          case 'if': {
            traverse(node[1], possibilifier);
            level++;
            traverse(node[2], possibilifier);
            purgeLevel();
            if (node[3]) {
              level++;
              traverse(node[3], possibilifier);
              purgeLevel();
            }
            break;
          }
          case 'switch': {
            traverse(node[1], possibilifier);
            var cases = node[2];
            for (var i = 0; i < cases.length; i++) {
              level++;
              traverse(cases[i][1], possibilifier);
              purgeLevel();
            }
            break;
          }
          default: throw dumpAst(node);
        }
        return null; // prevent recursion into children, which we already did
      }
    });
    var optimizables = {};
    if (!hasSwitch || asm) {
      for (var possible in possibles) {
        if (!unoptimizables[possible]) optimizables[possible] = 1;
      }
    }

    //printErr('optimizables: ' + JSON.stringify(optimizables));
    //printErr('unoptimizables: ' + JSON.stringify(unoptimizables));

    // Go through the function's code, assigning 'registers'.
    // The only tricky bit is to keep variables locked on a register through loops,
    // since they can potentially be returned to. Optimizable variables lock onto
    // loops that they enter, unoptimizable variables lock in a conservative way
    // into the topmost loop.
    // Note that we cannot lock onto a variable in a loop if it was used and free'd
    // before! (then they could overwrite us in the early part of the loop). For now
    // we just use a fresh register to make sure we avoid this, but it could be
    // optimized to check for safe registers (free, and not used in this loop level).
    var varRegs = {}; // maps variables to the register they will use all their life
    var freeRegsClasses = asm ? [[], [], [], [], [], [], [], [], [], [], [], [], []] : []; // two classes for asm, one otherwise XXX - hardcoded length
    var nextReg = 1;
    var fullNames = {};
    var loopRegs = {}; // for each loop nesting level, the list of bound variables
    var loops = 0; // 0 is toplevel, 1 is first loop, etc
    var saved = 0;
    var activeOptimizables = {};
    var optimizableLoops = {};
    var paramRegs = {}; // true if the register is used by a parameter (and so needs no def at start of function; also cannot
                        // be shared with another param, each needs its own)
    function decUse(name) {
      if (!varUses[name]) return false; // no uses left, or not a relevant variable
      if (optimizables[name]) activeOptimizables[name] = 1;
      var reg = varRegs[name];
      if (asm) assert(name in asmData.vars, name);
      var freeRegs = asm ? freeRegsClasses[asmData.vars[name]] : freeRegsClasses;
      if (!reg) {
        // acquire register
        if (optimizables[name] && freeRegs.length > 0 &&
            !(params[name] && paramRegs[freeRegs[freeRegs.length-1]])) { // do not share registers between parameters
          reg = freeRegs.pop();
          saved++;
        } else {
          reg = nextReg++;
          fullNames[reg] = getNewRegName(reg, name);
          if (params[name]) paramRegs[reg] = 1;
        }
        varRegs[name] = reg;
      }
      varUses[name]--;
      assert(varUses[name] >= 0);
      if (varUses[name] === 0) {
        if (optimizables[name]) delete activeOptimizables[name];
        // If we are not in a loop, or we are optimizable and not bound to a loop
        // (we might have been in one but left it), we can free the register now.
        if (loops === 0 || (optimizables[name] && !optimizableLoops[name])) {
          // free register
          freeRegs.push(reg);
        } else {
          // when the relevant loop is exited, we will free the register
          var releventLoop = optimizables[name] ? (optimizableLoops[name] || 1) : 1;
          if (!loopRegs[releventLoop]) loopRegs[releventLoop] = [];
          loopRegs[releventLoop].push(reg);
        }
      }
      return true;
    }
    traverse(fun, function(node, type) { // XXX we rely on traversal order being the same as execution order here
      if (type === 'name') {
        var name = node[1];
        if (decUse(name)) {
          node[1] = fullNames[varRegs[name]];
        }
      } else if (type in LOOP) {
        loops++;
        // Active optimizables lock onto this loop, if not locked onto one that encloses this one
        for (var name in activeOptimizables) {
          if (!optimizableLoops[name]) {
            optimizableLoops[name] = loops;
          }
        }
      }
    }, function(node, type) {
      if (type in LOOP) {
        // Free registers that were locked to this loop
        if (loopRegs[loops]) {
          if (asm) {
            loopRegs[loops].forEach(function(loopReg) {
              freeRegsClasses[regTypes[fullNames[loopReg]]].push(loopReg);
            });
          } else {
            freeRegsClasses = freeRegsClasses.concat(loopRegs[loops]);
          }
          loopRegs[loops].length = 0;
        }
        loops--;
      }
    });
    if (fun[2] && fun[2].length) {
      fun[2].length = 0; // clear params, we will fill with registers
      fun[3].shift(); // remove fake initial var
    }
    //printErr('var regs: ' + JSON.stringify(varRegs) + '\n\nparam regs: ' + JSON.stringify(paramRegs));
    if (!asm) {
      if (nextReg > 1) {
        var vars = [];
        for (var i = 1; i < nextReg; i++) {
          var reg = fullNames[i];
          if (!paramRegs[i]) {
            vars.push([reg]);
          } else {
            fun[2].push(reg);
          }
        }
        if (vars.length > 0) getStatements(fun).unshift(['var', vars]);
      }
    } else {
      //printErr('unfake params: \n\n' + astToSrc(fun) + '\n\n');
      var finalAsmData = {
        params: {},
        vars: {},
        inlines: asmData.inlines,
        ret: asmData.ret,
      };
      for (var i = 1; i < nextReg; i++) {
        var reg = fullNames[i];
        var type = regTypes[reg];
        if (!paramRegs[i]) {
          finalAsmData.vars[reg] = type;
        } else {
          finalAsmData.params[reg] = type;
          fun[2].push(reg);
        }
      }
      denormalizeAsm(fun, finalAsmData);
    }
  });
}


// Assign variables to 'registers', coalescing them onto a smaller number of shared
// variables.
//
// This does the same job as 'registerize' above, but burns a lot more cycles trying
// to reduce the total number of register variables.  Key points about the operation:
//
//   * we decompose the AST into a flow graph and perform a full liveness
//     analysis, to determine which variables are live at each point.
//
//   * variables that are live concurrently are assigned to different registers.
//
//   * variables that are linked via 'x=y' style statements are assigned the same
//     register if possible, so that the redundant assignment can be removed.
//     (e.g. assignments used to pass state around through loops).
//
//   * any code that cannot be reached through the flow-graph is removed.
//     (e.g. redundant break statements like 'break L123; break;').
//
//   * any assignments that we can prove are not subsequently used are removed.
//     (e.g. unnecessary assignments to the 'label' variable).
//
function registerizeHarder(ast) {
  assert(asm);

  traverseGeneratedFunctions(ast, function(fun) {

    // Do not try to process non-validating methods, like the heap replacer
    var abort = false;
    traverse(fun, function(node, type) {
      if (type === 'new') abort = true;
    });
    if (abort) return;

    var asmData = normalizeAsm(fun);

    var localVars = asmData.vars;
    for (var name in asmData.params) {
      localVars[name] = asmData.params[name];
    }

    // Utilities for allocating register variables.
    // We need distinct register pools for each type of variable.

    var allRegsByType = [{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}]; // XXX - hardcoded length
    var regPrefixByType = ['i', 'd', 'f', 'F4', 'F2', 'I16', 'I8', 'I4', 'B16', 'B8', 'B4', 'B2', 'n'];
    var nextReg = 1;

    function createReg(forName) {
      // Create a new register of type suitable for the given variable name.
      var allRegs = allRegsByType[localVars[forName]];
      var reg = nextReg++;
      allRegs[reg] = regPrefixByType[localVars[forName]] + reg;
      return reg;
    }

    // Traverse the tree in execution order and synthesize a basic flow-graph.
    // It's convenient to build a kind of "dual" graph where the nodes identify
    // the junctions between blocks at which control-flow may branch, and each
    // basic block is an edge connecting two such junctions.
    // For each junction we store:
    //    * set of blocks that originate at the junction
    //    * set of blocks that terminate at the junction
    // For each block we store:
    //    * a single entry junction
    //    * a single exit junction
    //    * a 'use' and 'kill' set of names for the block
    //    * full sequence of 'name' and 'assign' nodes in the block
    //    * whether each such node appears as part of a larger expression
    //      (and therefore cannot be safely eliminated)
    //    * set of labels that can be used to jump to this block

    var junctions = [];
    var blocks = [];
    var currEntryJunction = null;
    var nextBasicBlock = null;
    var isInExpr = 0;
    var activeLabels = [{}];
    var nextLoopLabel = null;

    var ENTRY_JUNCTION = 0;
    var EXIT_JUNCTION = 1;
    var ENTRY_BLOCK = 0;

    function addJunction() {
      // Create a new junction, without inserting it into the graph.
      // This is useful for e.g. pre-allocating an exit node.
      var id = junctions.length;
      junctions[id] = {id: id, inblocks: {}, outblocks: {}};
      return id;
    }

    function markJunction(id) {
      // Mark current traversal location as a junction.
      // This makes a new basic block exiting at this position.
      if (id === undefined || id === null) {
        id = addJunction();
      }
      joinJunction(id, true);
      return id;
    }

    function setJunction(id, force) {
      // Set the next entry junction to the given id.
      // This can be used to enter at a previously-declared point.
      // You can't return to a junction with no incoming blocks
      // unless the 'force' parameter is specified.
      assert(nextBasicBlock.nodes.length === 0, 'refusing to abandon an in-progress basic block')
      if (force || setSize(junctions[id].inblocks) > 0) {
        currEntryJunction = id;
      } else {
        currEntryJunction = null;
      }
    }

    function joinJunction(id, force) {
      // Complete the pending basic block by exiting at this position.
      // This can be used to exit at a previously-declared point.
      if (currEntryJunction !== null) {
        nextBasicBlock.id = blocks.length;
        nextBasicBlock.entry = currEntryJunction;
        nextBasicBlock.exit = id;
        junctions[currEntryJunction].outblocks[nextBasicBlock.id] = 1;
        junctions[id].inblocks[nextBasicBlock.id] = 1;
        blocks.push(nextBasicBlock);
      } 
      nextBasicBlock = { id: null, entry: null, exit: null, labels: {}, nodes: [], isexpr: [], use: {}, kill: {} };
      setJunction(id, force);
      return id;
    }

    function pushActiveLabels(onContinue, onBreak) {
      // Push the target junctions for continuing/breaking a loop.
      // This should be called before traversing into a loop.
      var prevLabels = activeLabels[activeLabels.length-1];
      var newLabels = copy(prevLabels);
      newLabels[null] = [onContinue, onBreak];
      if (nextLoopLabel) {
        newLabels[nextLoopLabel] = [onContinue, onBreak];
        nextLoopLabel = null;
      }
      // An unlabelled 'continue' should jump to innermost loop,
      // ignoring any nested 'switch' statements.
      if (onContinue === null && prevLabels[null]) {
        newLabels[null][0] = prevLabels[null][0];
      }
      activeLabels.push(newLabels);
    }

    function popActiveLabels() {
      // Pop the target junctions for continuing/breaking a loop.
      // This should be called after traversing into a loop.
      activeLabels.pop();
    }

    function markNonLocalJump(type, label) {
      // Complete a block via 'return', 'break' or 'continue'.
      // This joins the targetted junction and then sets the current junction to null.
      // Any code traversed before we get back to an existing junction is dead code.
      if (type === 'return') {
        joinJunction(EXIT_JUNCTION);
      } else {
        label = label ? label : null;
        var targets = activeLabels[activeLabels.length-1][label];
        assert(targets, 'jump to unknown label');
        if (type === 'continue') {
          joinJunction(targets[0]);
        } else if (type === 'break') {
          joinJunction(targets[1]);
        } else {
          assert(false, 'unknown jump node type');
        }
      }
      currEntryJunction = null;
    }

    function addUseNode(node) {
      // Mark a use of the given name node in the current basic block.
      assert(node[0] === 'name', 'not a use node');
      var name = node[1];
      if (name in localVars) {
        nextBasicBlock.nodes.push(node);
        nextBasicBlock.isexpr.push(isInExpr);
        if (!nextBasicBlock.kill[name]) {
          nextBasicBlock.use[name] = 1;
        }
      }
    }

    function addKillNode(node) {
      // Mark an assignment to the given name node in the current basic block.
      assert(node[0] === 'assign', 'not a kill node');
      assert(node[1] === true, 'not a kill node');
      assert(node[2][0] === 'name', 'not a kill node');
      var name = node[2][1];
      if (name in localVars) {
        nextBasicBlock.nodes.push(node);
        nextBasicBlock.isexpr.push(isInExpr);
        nextBasicBlock.kill[name] = 1;
      }
    }

    function lookThroughCasts(node) {
      // Look through value-preserving casts, like "x | 0" => "x"
      if (node[0] === 'binary' && node[1] === '|') {
        if (node[3][0] === 'num' && node[3][1] === 0) {
          return lookThroughCasts(node[2]);
        }
      }
      return node;
    }

    function addBlockLabel(node) {
      assert(nextBasicBlock.nodes.length === 0, 'cant add label to an in-progress basic block')
      if (node[0] === 'num') {
        nextBasicBlock.labels[node[1]] = 1;
      }
    }

    function isTrueNode(node) {
      // Check if the given node is statically truthy.
      return (node[0] === 'num' && node[1] != 0);
    }

    function isFalseNode(node) {
      // Check if the given node is statically falsy.
      return (node[0] === 'num' && node[1] == 0);
    }

    function morphNode(node, newNode) {
      // In-place morph a node into some other type of node.
      var i = 0;
      while (i < node.length && i < newNode.length) {
        node[i] = newNode[i];
        i++;
      }
      while (i < newNode.length) {
        node.push(newNode[i]);
        i++;
      }
      if (node.length > newNode.length) {
        node.length = newNode.length;
      }
    }

    function buildFlowGraph(node) {
      // Recursive function to build up the flow-graph.
      // It walks the tree in execution order, calling the above state-management
      // functions at appropriate points in the traversal.
      var type = node[0];
  
      // Any code traversed without an active entry junction must be dead,
      // as the resulting block could never be entered. Let's remove it.
      if (currEntryJunction === null && junctions.length > 0) {
        morphNode(node, ['block', []]);
        return;
      }
 
      // Traverse each node type according to its particular control-flow semantics.
      switch (type) {
        case 'defun':
          var jEntry = markJunction();
          assert(jEntry === ENTRY_JUNCTION);
          var jExit = addJunction();
          assert(jExit === EXIT_JUNCTION);
          for (var i = 0; i < node[3].length; i++) {
            buildFlowGraph(node[3][i]);
          }
          joinJunction(jExit);
          break;
        case 'if':
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
          var jEnter = markJunction();
          var jExit = addJunction();
          if (node[2]) {
            // Detect and mark "if (label == N) { <labelled block> }".
            if (node[1][0] === 'binary' && node[1][1] === '==') {
              var lhs = lookThroughCasts(node[1][2]);
              if (lhs[0] === 'name' && lhs[1] === 'label') {
                addBlockLabel(lookThroughCasts(node[1][3]));
              }
            }
            buildFlowGraph(node[2]);
          }
          joinJunction(jExit);
          setJunction(jEnter);
          if (node[3]) {
            buildFlowGraph(node[3]);
          }
          joinJunction(jExit);
          break;
        case 'conditional':
          isInExpr++;
          // If the conditional has no side-effects, we can treat it as a single
          // block, which might open up opportunities to remove it entirely.
          if (!hasSideEffects(node)) {
            buildFlowGraph(node[1]);
            if (node[2]) {
              buildFlowGraph(node[2]);
            }
            if (node[3]) {
              buildFlowGraph(node[3]);
            }
          } else {
            buildFlowGraph(node[1]);
            var jEnter = markJunction();
            var jExit = addJunction();
            if (node[2]) {
              buildFlowGraph(node[2]);
            }
            joinJunction(jExit);
            setJunction(jEnter);
            if (node[3]) {
              buildFlowGraph(node[3]);
            }
            joinJunction(jExit);
          }
          isInExpr--;
          break;
        case 'while':
          // Special-case "while (1) {}" to use fewer junctions,
          // since emscripten generates a lot of these.
          if (isTrueNode(node[1])) {
            var jLoop = markJunction();
            var jExit = addJunction();
            pushActiveLabels(jLoop, jExit);
            buildFlowGraph(node[2]);
            popActiveLabels();
            joinJunction(jLoop);
            setJunction(jExit);
          } else {
            var jCond = markJunction();
            var jLoop = addJunction();
            var jExit = addJunction();
            isInExpr++;
            buildFlowGraph(node[1]);
            isInExpr--;
            joinJunction(jLoop);
            pushActiveLabels(jCond, jExit);
            buildFlowGraph(node[2]);
            popActiveLabels();
            joinJunction(jCond);
            // An empty basic-block linking condition exit to loop exit.
            setJunction(jLoop);
            joinJunction(jExit);
          }
          break;
        case 'do':
          // Special-case "do {} while (1)" and "do {} while (0)" to use
          // fewer junctions, since emscripten generates a lot of these.
          if (isFalseNode(node[1])) {
            var jExit = addJunction();
            pushActiveLabels(jExit, jExit);
            buildFlowGraph(node[2]);
            popActiveLabels();
            joinJunction(jExit);
          } else if (isTrueNode(node[1])) {
            var jLoop = markJunction();
            var jExit = addJunction();
            pushActiveLabels(jLoop, jExit);
            buildFlowGraph(node[2]);
            popActiveLabels();
            joinJunction(jLoop);
            setJunction(jExit);
          } else {
            var jLoop = markJunction();
            var jCond = addJunction();
            var jCondExit = addJunction();
            var jExit = addJunction();
            pushActiveLabels(jCond, jExit);
            buildFlowGraph(node[2]);
            popActiveLabels();
            joinJunction(jCond);
            isInExpr++;
            buildFlowGraph(node[1]);
            isInExpr--;
            joinJunction(jCondExit);
            joinJunction(jLoop);
            setJunction(jCondExit);
            joinJunction(jExit);
          }
          break;
        case 'for':
          var jTest = addJunction();
          var jBody = addJunction();
          var jStep = addJunction();
          var jExit = addJunction();
          buildFlowGraph(node[1]);
          joinJunction(jTest);
          isInExpr++;
          buildFlowGraph(node[2]);
          isInExpr--;
          joinJunction(jBody);
          pushActiveLabels(jStep, jExit);
          buildFlowGraph(node[4]);
          popActiveLabels();
          joinJunction(jStep);
          buildFlowGraph(node[3]);
          joinJunction(jTest);
          setJunction(jBody);
          joinJunction(jExit);
          break;
        case 'label':
          assert(node[2][0] in BREAK_CAPTURERS, 'label on non-loop, non-switch statement')
          nextLoopLabel = node[1];
          buildFlowGraph(node[2]);
          break;
        case 'switch':
          // Emscripten generates switch statements of a very limited
          // form: all case clauses are numeric literals, and all
          // case bodies end with a (maybe implicit) break.  So it's
          // basically equivalent to a multi-way 'if' statement.
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
          var condition = lookThroughCasts(node[1]);
          var jCheckExit = markJunction();
          var jExit = addJunction();
          pushActiveLabels(null, jExit);
          var hasDefault = false;
          for (var i=0; i<node[2].length; i++) {
            setJunction(jCheckExit);
            // All case clauses are either 'default' or a numeric literal.
            if (!node[2][i][0]) {
              hasDefault = true;
            } else {
              // Detect switches dispatching to labelled blocks.
              if (condition[0] === 'name' && condition[1] === 'label') {
                addBlockLabel(lookThroughCasts(node[2][i][0]));
              }
            }
            for (var j = 0; j < node[2][i][1].length; j++) {
              buildFlowGraph(node[2][i][1][j]);
            }
            // Control flow will never actually reach the end of the case body.
            // If there's live code here, assume it jumps to case exit.
            if (currEntryJunction !== null && nextBasicBlock.nodes.length > 0) {
              if (node[2][i][0]) {
                markNonLocalJump('return');
              } else {
                joinJunction(jExit);
              }
            }
          }
          // If there was no default case, we also need an empty block
          // linking straight from the test evaluation to the exit.
          if (!hasDefault) {
            setJunction(jCheckExit);
          }
          joinJunction(jExit);
          popActiveLabels();
          break;
        case 'return':
          if (node[1]) {
            isInExpr++;
            buildFlowGraph(node[1]);
            isInExpr--;
          }
          markNonLocalJump(type);
          break;
        case 'break':
        case 'continue':
          markNonLocalJump(type, node[1]);
          break;
        case 'assign':
          isInExpr++;
          buildFlowGraph(node[3]);
          isInExpr--;
          if (node[1] === true && node[2][0] === 'name') {
            addKillNode(node);
          } else {
            buildFlowGraph(node[2]);
          }
          break;
        case 'name':
          addUseNode(node);
          break;
        case 'block':
        case 'toplevel':
          if (node[1]) {
            for (var i = 0; i < node[1].length; i++) {
              buildFlowGraph(node[1][i]);
            }
          }
          break;
        case 'stat':
          buildFlowGraph(node[1]);
          break;
        case 'unary-prefix':
        case 'unary-postfix':
          isInExpr++;
          buildFlowGraph(node[2]);
          isInExpr--;
          break;
        case 'binary':
          isInExpr++;
          buildFlowGraph(node[2]);
          buildFlowGraph(node[3]);
          isInExpr--;
          break;
        case 'call':
          isInExpr++;
          buildFlowGraph(node[1]);
          if (node[2]) {
            for (var i = 0; i < node[2].length; i++) {
              buildFlowGraph(node[2][i]);
            }
          }
          isInExpr--;
          // If the call is statically known to throw,
          // treat it as a jump to function exit.
          if (!isInExpr && node[1][0] === 'name') {
            if (node[1][1] in FUNCTIONS_THAT_ALWAYS_THROW) {
              markNonLocalJump('return');
            }
          }
          break;
        case 'seq':
        case 'sub':
          isInExpr++;
          buildFlowGraph(node[1]);
          buildFlowGraph(node[2]);
          isInExpr--;
          break;
        case 'dot':
        case 'throw':
          isInExpr++;
          buildFlowGraph(node[1]);
          isInExpr--;
          break;
        case 'num':
        case 'string':
        case 'var':
          break;
        default:
          printErr(JSON.stringify(node));
          assert(false, 'unsupported node type: ' + type);
      }
    }
    buildFlowGraph(fun);

    assert(setSize(junctions[ENTRY_JUNCTION].inblocks) === 0, 'function entry must have no incoming blocks');
    assert(setSize(junctions[EXIT_JUNCTION].outblocks) === 0, 'function exit must have no outgoing blocks');
    assert(blocks[ENTRY_BLOCK].entry === ENTRY_JUNCTION, 'block zero must be the initial block');

    // Fix up implicit jumps done by assigning to the 'label' variable.
    // If a block ends with an assignment to 'label' and there's another block
    // with that value of 'label' as precondition, we tweak the flow graph so
    // that the former jumps straight to the later.

    var labelledBlocks = {};
    var labelledJumps = [];
    FINDLABELLEDBLOCKS:
    for (var i = 0; i < blocks.length; i++) {
      var block = blocks[i];
      // Does it have any labels as preconditions to its entry?
      for (var labelVal in block.labels) {
        // If there are multiple blocks with the same label, all bets are off.
        // This seems to happen sometimes for short blocks that end with a return.
        // TODO: it should be safe to merge the duplicates if they're identical.
        if (labelVal in labelledBlocks) {
          labelledBlocks = {};
          labelledJumps = [];
          break FINDLABELLEDBLOCKS;
        }
        labelledBlocks[labelVal] = block;
      }
      // Does it assign a specific label value at exit?
      if ('label' in block.kill) {
        var finalNode = block.nodes[block.nodes.length - 1];
        if (finalNode[0] === 'assign' && finalNode[2][1] === 'label') {
          // If labels are computed dynamically then all bets are off.
          // This can happen due to indirect branching in llvm output.
          if (finalNode[3][0] !== 'num') {
            labelledBlocks = {};
            labelledJumps = [];
            break FINDLABELLEDBLOCKS;
          }
          labelledJumps.push([finalNode[3][1], block]);
        } else { 
          // If label is assigned a non-zero value elsewhere in the block
          // then all bets are off.  This can happen e.g. due to outlining
          // saving/restoring label to the stack.
          for (var j = 0; j < block.nodes.length - 1; j++) {
            if (block.nodes[j][0] === 'assign' && block.nodes[j][2][1] === 'label') {
              if (block.nodes[j][3][0] !== 'num' || block.nodes[j][3][1] !== 0) {
                labelledBlocks = {};
                labelledJumps = [];
                break FINDLABELLEDBLOCKS;
              }
            }
          }
        }
      }
    }
    for (var labelVal in labelledBlocks) {
      var block = labelledBlocks[labelVal];
      // Disconnect it from the graph, and create a
      // new junction for jumps targetting this label.
      delete junctions[block.entry].outblocks[block.id];
      block.entry = addJunction();
      junctions[block.entry].outblocks[block.id] = 1;
      // Add a fake use of 'label' to keep it alive in predecessor.
      block.use['label'] = 1;
      block.nodes.unshift(['name', 'label']);
      block.isexpr.unshift(1);
    }
    for (var i = 0; i < labelledJumps.length; i++) {
      var labelVal = labelledJumps[i][0];
      var block = labelledJumps[i][1];
      var targetBlock = labelledBlocks[labelVal];
      if (targetBlock) {
        // Redirect its exit to entry of the target block.
        delete junctions[block.exit].inblocks[block.id];
        block.exit = targetBlock.entry;
        junctions[block.exit].inblocks[block.id] = 1;
      }
    }
    labelledBlocks = null;
    labelledJumps = null;

    // Do a backwards data-flow analysis to determine the set of live
    // variables at each junction, and to use this information to eliminate
    // any unused assignments.
    // We run two nested phases.  The inner phase builds the live set for each
    // junction.  The outer phase uses this to try to eliminate redundant
    // stores in each basic block, which might in turn affect liveness info.

    function analyzeJunction(junc) {
      // Update the live set for this junction.
      var live = {};
      for (var b in junc.outblocks) {
        var block = blocks[b];
        var liveSucc = junctions[block.exit].live || {};
        for (var name in liveSucc) {
          if (!(name in block.kill)) {
            live[name] = 1;
          }
        }
        for (var name in block.use) {
          live[name] = 1;
        }
      }
      junc.live = live;
    }

    function analyzeBlock(block) {
      // Update information about the behaviour of the block.
      // This includes the standard 'use' and 'kill' information,
      // plus a 'link' set naming values that flow through from entry
      // to exit, possibly changing names via simple 'x=y' assignments.
      // As we go, we eliminate assignments if the variable is not
      // subsequently used.
      var live = copy(junctions[block.exit].live);
      var use = {};
      var kill = {};
      var link = {};
      var lastUseLoc = {};
      var firstDeadLoc = {};
      var firstKillLoc = {};
      var lastKillLoc = {};
      for (var name in live) {
        link[name] = name;
        lastUseLoc[name] = block.nodes.length;
        firstDeadLoc[name] = block.nodes.length;
      }
      for (var j = block.nodes.length - 1; j >=0 ; j--) {
        var node = block.nodes[j];
        if (node[0] === 'name') {
          var name = node[1];
          live[name] = 1;
          use[name] = j;
          if (lastUseLoc[name] === undefined) {
            lastUseLoc[name] = j;
            firstDeadLoc[name] = j;
          }
        } else {
          var name = node[2][1];
          // We only keep assignments if they will be subsequently used.
          if (name in live) {
            kill[name] = 1;
            delete use[name];
            delete live[name];
            firstDeadLoc[name] = j;
            firstKillLoc[name] = j;
            if (lastUseLoc[name] === undefined) {
              lastUseLoc[name] = j;
            }
            if (lastKillLoc[name] === undefined) {
              lastKillLoc[name] = j;
            }
            // If it's an "x=y" and "y" is not live, then we can create a
            // flow-through link from "y" to "x".  If not then there's no
            // flow-through link for "x".
            var oldLink = link[name];
            if (oldLink) {
              delete link[name];
              if (node[3][0] === 'name') {
                if (node[3][1] in localVars) {
                  link[node[3][1]] = oldLink;
                }
              }
            }
          } else {
            // The result of this assignment is never used, so delete it.
            // We may need to keep the RHS for its value or its side-effects.
            function removeUnusedNodes(j, n) {
              for (var name in lastUseLoc) {
                lastUseLoc[name] -= n;
              }
              for (var name in firstKillLoc) {
                firstKillLoc[name] -= n;
              }
              for (var name in lastKillLoc) {
                lastKillLoc[name] -= n;
              }
              for (var name in firstDeadLoc) {
                firstDeadLoc[name] -= n;
              }
              block.nodes.splice(j, n);
              block.isexpr.splice(j, n);
            }
            if (block.isexpr[j] || hasSideEffects(node[3])) {
              morphNode(node, node[3]);
              removeUnusedNodes(j, 1);
            } else {
              var numUsesInExpr = 0;
              traverse(node[3], function(node, type) {
                if (type === 'name' && node[1] in localVars) {
                  numUsesInExpr++;
                }
              });
              morphNode(node, ['block', []]);
              j = j - numUsesInExpr;
              removeUnusedNodes(j, 1 + numUsesInExpr);
            }
          }
        }
      }
      block.use = use;
      block.kill = kill;
      block.link = link;
      block.lastUseLoc = lastUseLoc;
      block.firstDeadLoc = firstDeadLoc;
      block.firstKillLoc = firstKillLoc;
      block.lastKillLoc = lastKillLoc;
    }

    var jWorklistMap = { EXIT_JUNCTION: 1 };
    var jWorklist = [EXIT_JUNCTION];
    var bWorklistMap = {};
    var bWorklist = [];

    // Be sure to visit every junction at least once.
    // This avoids missing some vars because we disconnected them
    // when processing the labelled jumps.
    for (var i = junctions.length - 1; i >= EXIT_JUNCTION; i--) {
      jWorklistMap[i] = 1;
      jWorklist.push(i);
    }

    while (jWorklist.length > 0) {
      // Iterate on just the junctions until we get stable live sets.
      // The first run of this loop will grow the live sets to their maximal size.
      // Subsequent runs will shrink them based on eliminated in-block uses.
      while (jWorklist.length > 0) {
        var junc = junctions[jWorklist.pop()];
        delete jWorklistMap[junc.id];
        var oldLive = junc.live || null;
        analyzeJunction(junc);
        if (!sortedJsonCompare(oldLive, junc.live)) {
          // Live set changed, updated predecessor blocks and junctions.
          for (var b in junc.inblocks) {
            if (!(b in bWorklistMap)) {
              bWorklistMap[b] = 1;
              bWorklist.push(b);
            }
            var jPred = blocks[b].entry;
            if (!(jPred in jWorklistMap)) {
              jWorklistMap[jPred] = 1;
              jWorklist.push(jPred);
            }
          }
        }
      }
      // Now update the blocks based on the calculated live sets.
      while (bWorklist.length > 0) {
        var block = blocks[bWorklist.pop()];
        delete bWorklistMap[block.id];
        var oldUse = block.use;
        analyzeBlock(block);
        if (!sortedJsonCompare(oldUse, block.use)) {
          // The use set changed, re-process the entry junction.
          if (!(block.entry in jWorklistMap)) {
            jWorklistMap[block.entry] = 1;
            jWorklist.push(block.entry);
          }
        }
      }
    }

    // Insist that all function parameters are alive at function entry.
    // This ensures they will be assigned independent registers, even
    // if they happen to be unused.

    for (var name in asmData.params) {
      junctions[ENTRY_JUNCTION].live[name] = 1;
    }

    // For variables that are live at one or more junctions, we assign them
    // a consistent register for the entire scope of the function.  Find pairs
    // of variable that cannot use the same register (the "conflicts") as well
    // as pairs of variables that we'd like to have share the same register
    // (the "links").

    var junctionVariables = {};

    function initializeJunctionVariable(name) {
      junctionVariables[name] = { conf: {}, link: {}, excl: {}, reg: null };
    }

    for (var i = 0; i < junctions.length; i++) {
      var junc = junctions[i];
      for (var name in junc.live) {
        if (!junctionVariables[name]) initializeJunctionVariable(name);
        // It conflicts with all other names live at this junction.
        for (var otherName in junc.live) {
          if (otherName == name) continue;
          junctionVariables[name].conf[otherName] = 1;
        }
        for (var b in junc.outblocks) {
          // It conflicts with any output vars of successor blocks,
          // if they're assigned before it goes dead in that block.
          var block = blocks[b];
          var jSucc = junctions[block.exit];
          for (var otherName in jSucc.live) {
            if (junc.live[otherName]) continue;
            if (block.lastKillLoc[otherName] < block.firstDeadLoc[name]) {
              if (!junctionVariables[otherName]) initializeJunctionVariable(otherName);
              junctionVariables[name].conf[otherName] = 1;
              junctionVariables[otherName].conf[name] = 1;
            }
          }
          // It links with any linkages in the outgoing blocks.
          var linkName = block.link[name];
          if (linkName && linkName !== name) {
            if (!junctionVariables[linkName]) initializeJunctionVariable(linkName);
            junctionVariables[name].link[linkName] = 1;
            junctionVariables[linkName].link[name] = 1;
          }
        }
      }
    }

    // Attempt to sort the junction variables to heuristically reduce conflicts.
    // Simple starting point: handle the most-conflicted variables first.
    // This seems to work pretty well.

    var sortedJunctionVariables = keys(junctionVariables);
    sortedJunctionVariables.sort(function(name1, name2) {
      var jv1 = junctionVariables[name1];
      var jv2 = junctionVariables[name2];
      if (jv1.numConfs === undefined) {
        jv1.numConfs = setSize(jv1.conf);
      }
      if (jv2.numConfs === undefined) {
        jv2.numConfs = setSize(jv2.conf);
      }
      return jv2.numConfs - jv1.numConfs;
    });

    // We can now assign a register to each junction variable.
    // Process them in order, trying available registers until we find
    // one that works, and propagating the choice to linked/conflicted
    // variables as we go.

    function tryAssignRegister(name, reg) {
      // Try to assign the given register to the given variable,
      // and propagate that choice throughout the graph.
      // Returns true if successful, false if there was a conflict.
      var jv = junctionVariables[name];
      if (jv.reg !== null) {
        return jv.reg === reg;
      }
      if (jv.excl[reg]) {
        return false;
      }
      jv.reg = reg;
      // Exclude use of this register at all conflicting variables.
      for (var confName in jv.conf) {
        junctionVariables[confName].excl[reg] = 1;
      }
      // Try to propagate it into linked variables.
      // It's not an error if we can't.
      for (var linkName in jv.link) {
        tryAssignRegister(linkName, reg);
      }
      return true;
    }

    NEXTVARIABLE:
    for (var i = 0; i < sortedJunctionVariables.length; i++) {
      var name = sortedJunctionVariables[i];
      // It may already be assigned due to linked-variable propagation.
      if (junctionVariables[name].reg !== null) {
        continue NEXTVARIABLE;
      }
      // Try to use existing registers first.
      var allRegs = allRegsByType[localVars[name]];
      for (var reg in allRegs) {
        if (tryAssignRegister(name, reg)) {
          continue NEXTVARIABLE;
        }
      }
      // They're all taken, create a new one.
      tryAssignRegister(name, createReg(name));
    }

    // Each basic block can now be processed in turn.
    // There may be internal-use-only variables that still need a register
    // assigned, but they can be treated just for this block.  We know
    // that all inter-block variables are in a good state thanks to
    // junction variable consistency.

    for (var i = 0; i < blocks.length; i++) {
      var block = blocks[i];
      if (block.nodes.length === 0) continue;
      var jEnter = junctions[block.entry];
      var jExit = junctions[block.exit];
      // Mark the point at which each input reg becomes dead.
      // Variables alive before this point must not be assigned
      // to that register.
      var inputVars = {};
      var inputDeadLoc = {};
      var inputVarsByReg = {};
      for (var name in jExit.live) {
        if (!(name in block.kill)) {
          inputVars[name] = 1;
          var reg = junctionVariables[name].reg;
          assert(reg !== null, 'input variable doesnt have a register');
          inputDeadLoc[reg] = block.firstDeadLoc[name];
          inputVarsByReg[reg] = name;
        }
      }
      for (var name in block.use) {
        if (!(name in inputVars)) {
          inputVars[name] = 1;
          var reg = junctionVariables[name].reg;
          assert(reg !== null, 'input variable doesnt have a register');
          inputDeadLoc[reg] = block.firstDeadLoc[name];
          inputVarsByReg[reg] = name;
        }
      }
      assert(setSize(setSub(inputVars, jEnter.live)) == 0);
      // Scan through backwards, allocating registers on demand.
      // Be careful to avoid conflicts with the input registers.
      // We consume free registers in last-used order, which helps to
      // eliminate "x=y" assignments that are the last use of "y".
      var assignedRegs = {};
      var freeRegsByType = copy(allRegsByType);
      // Begin with all live vars assigned per the exit junction.
      for (var name in jExit.live) {
        var reg = junctionVariables[name].reg;
        assert(reg !== null, 'output variable doesnt have a register');
        assignedRegs[name] = reg;
        delete freeRegsByType[localVars[name]][reg];
      }
      for (var j = 0; j < freeRegsByType.length; j++) {
        freeRegsByType[j] = keys(freeRegsByType[j]);
      }
      // Scan through the nodes in sequence, modifying each node in-place
      // and grabbing/freeing registers as needed.
      var maybeRemoveNodes = [];
      for (var j = block.nodes.length - 1; j >= 0; j--) {
        var node = block.nodes[j];
        var name = node[0] === 'assign' ? node[2][1] : node[1];
        var allRegs = allRegsByType[localVars[name]];
        var freeRegs = freeRegsByType[localVars[name]];
        var reg = assignedRegs[name];
        if (node[0] === 'name') {
          // A use.  Grab a register if it doesn't have one.
          if (!reg) {
            if (name in inputVars && j <= block.firstDeadLoc[name]) {
              // Assignment to an input variable, must use pre-assigned reg.
              reg = junctionVariables[name].reg;
              assignedRegs[name] = reg;
              for (var k = freeRegs.length - 1; k >= 0; k--) {
                if (freeRegs[k] === reg) {
                  freeRegs.splice(k, 1);
                  break;
                }
              }
            } else {
              // Try to use one of the existing free registers.
              // It must not conflict with an input register.
              for (var k = freeRegs.length - 1; k >= 0; k--) {
                reg = freeRegs[k];
                // Check for conflict with input registers.
                if (block.firstKillLoc[name] <= inputDeadLoc[reg]) {
                  if (name !== inputVarsByReg[reg]) {
                    continue;
                  }
                }
                // Found one!
                assignedRegs[name] = reg;
                freeRegs.splice(k, 1);
                break;
              }
              // If we didn't find a suitable register, create a new one.
              if (!assignedRegs[name]) {
                reg = createReg(name);
                assignedRegs[name] = reg;
              }
            }
          }
          node[1] = allRegs[reg];
        } else {
          // A kill. This frees the assigned register.
          assert(reg, 'live variable doesnt have a reg?')
          node[2][1] = allRegs[reg];
          freeRegs.push(reg);
          delete assignedRegs[name];
          if (node[3][0] === 'name' && node[3][1] in localVars) {
            maybeRemoveNodes.push([j, node]);
          }
        }
      }
      // If we managed to create any "x=x" assignments, remove them.
      for (var j = 0; j < maybeRemoveNodes.length; j++) {
        var node = maybeRemoveNodes[j][1];
        if (node[2][1] === node[3][1]) {
          if (block.isexpr[maybeRemoveNodes[j][0]]) {
            morphNode(node, node[2]);
          } else {
            morphNode(node, ['block', []]);
          }
        }
      }
    }

    // Assign registers to function params based on entry junction

    var paramRegs = {};
    if (fun[2]) {
      for (var i = 0; i < fun[2].length; i++) {
        var allRegs = allRegsByType[localVars[fun[2][i]]];
        fun[2][i] = allRegs[junctionVariables[fun[2][i]].reg];
        paramRegs[fun[2][i]] = 1;
      }
    }

    // That's it!
    // Re-construct the function with appropriate variable definitions.
 
    var finalAsmData = {
      params: {},
      vars: {},
      inlines: asmData.inlines,
      ret: asmData.ret,
    };
    for (var i = 1; i < nextReg; i++) {
      var reg;
      for (var type=0; type<allRegsByType.length; type++) {
        reg = allRegsByType[type][i];
        if (reg) break;
      }
      if (!paramRegs[reg]) {
        finalAsmData.vars[reg] = type;
      } else {
        finalAsmData.params[reg] = type;
      }
    }
    denormalizeAsm(fun, finalAsmData);

    vacuum(fun);

  });
}


// Eliminator aka Expressionizer
//
// The goal of this pass is to eliminate unneeded variables (which represent one of the infinite registers in the LLVM
// model) and thus to generate complex expressions where possible, for example
//
//  var x = a(10);
//  var y = HEAP[20];
//  print(x+y);
//
// can be transformed into
//
//  print(a(10)+HEAP[20]);
//
// The basic principle is to scan along the code in the order of parsing/execution, and keep a list of tracked
// variables that are current contenders for elimination. We must untrack when we see something that we cannot
// cross, for example, a write to memory means we must invalidate variables that depend on reading from
// memory, since if we change the order then we do not preserve the computation.
//
// We rely on some assumptions about emscripten-generated code here, which means we can do a lot more than
// a general JS optimization can. For example, we assume that 'sub' nodes (indexing like HEAP[..]) are
// memory accesses or FUNCTION_TABLE accesses, and in both cases that the symbol cannot be replaced although
// the contents can. So we assume FUNCTION_TABLE might have its contents changed but not be pointed to
// a different object, which allows
//
//  var x = f();
//  FUNCTION_TABLE[x]();
//
// to be optimized (f could replace FUNCTION_TABLE, so in general JS eliminating x is not valid).
//
// In memSafe mode, we are more careful and assume functions can replace HEAP and FUNCTION_TABLE, which
// can happen in ALLOW_MEMORY_GROWTH mode

var ELIMINATION_SAFE_NODES = set('var', 'assign', 'call', 'if', 'toplevel', 'do', 'return', 'label', 'switch', 'binary', 'unary-prefix'); // do is checked carefully, however
var IGNORABLE_ELIMINATOR_SCAN_NODES = set('num', 'toplevel', 'string', 'break', 'continue', 'dot'); // dot can only be STRING_TABLE.*
var ABORTING_ELIMINATOR_SCAN_NODES = set('new', 'object', 'function', 'defun', 'for', 'while', 'array', 'throw'); // we could handle some of these, TODO, but nontrivial (e.g. for while, the condition is hit multiple times after the body)
var HEAP_NAMES = set('HEAP8', 'HEAP16', 'HEAP32', 'HEAPU8', 'HEAPU16', 'HEAPU32', 'HEAPF32', 'HEAPF64');

function isTempDoublePtrAccess(node) { // these are used in bitcasts; they are not really affecting memory, and should cause no invalidation
  assert(node[0] === 'sub');
  return (node[2][0] === 'name' && node[2][1] === 'tempDoublePtr') ||
         (node[2][0] === 'binary' && ((node[2][2][0] === 'name' && node[2][2][1] === 'tempDoublePtr') ||
                                     (node[2][3][0] === 'name' && node[2][3][1] === 'tempDoublePtr')));
}

function eliminate(ast, memSafe) {
  // Find variables that have a single use, and if they can be eliminated, do so
  traverseGeneratedFunctions(ast, function(func, type) {
    if (asm) var asmData = normalizeAsm(func);
    //printErr('eliminate in ' + func[1]);

    // First, find the potentially eliminatable functions: that have one definition and one use
    var definitions = {};
    var uses = {};
    var namings = {};
    var values = {};
    var locals = {};
    var varsToRemove = {}; // variables being removed, that we can eliminate all 'var x;' of (this refers to 'var' nodes we should remove)
                           // 1 means we should remove it, 2 means we successfully removed it
    var varsToTryToRemove = {}; // variables that have 0 uses, but have side effects - when we scan we can try to remove them
    // add arguments as locals
    if (func[2]) {
      for (var i = 0; i < func[2].length; i++) {
        locals[func[2][i]] = true;
      }
    }
    // examine body and note locals
    var hasSwitch = false;
    traverse(func, function(node, type) {
      if (type === 'var') {
        var node1 = node[1];
        for (var i = 0; i < node1.length; i++) {
          var node1i = node1[i];
          var name = node1i[0];
          var value = node1i[1];
          if (value) {
            if (!(name in definitions)) definitions[name] = 0;
            definitions[name]++;
            if (!values[name]) values[name] = value;
          }
          if (!uses[name]) uses[name] = 0;
          locals[name] = true;
        }
      } else if (type === 'name') {
        var name = node[1];
        if (!uses[name]) uses[name] = 0;
        uses[name]++;
      } else if (type === 'assign') {
        var target = node[2];
        if (target[0] === 'name') {
          var name = target[1];
          if (!(name in definitions)) definitions[name] = 0;
          definitions[name]++;
          if (!uses[name]) uses[name] = 0;
          if (!values[name]) values[name] = node[3];
          if (node[1] === true) { // not +=, -= etc., just =
            uses[name]--; // because the name node will show up by itself in the previous case
            if (!namings[name]) namings[name] = 0;
            namings[name]++; // offset it here, this tracks the total times we are named
          }
        }
      } else if (type === 'switch') {
        hasSwitch = true;
      }
    });

    for (var used in uses) {
      namings[used] = (namings[used] || 0) + uses[used];
    }

    // we cannot eliminate variables if there is a switch
    if (hasSwitch && !asm) return;

    var potentials = {}; // local variables with 1 definition and 1 use
    var sideEffectFree = {}; // whether a local variable has no side effects in its definition. Only relevant when there are no uses

    function unprocessVariable(name) {
      if (name in potentials) delete potentials[name];
      if (name in varsToRemove) delete varsToRemove[name];
      if (name in sideEffectFree) delete sideEffectFree[name];
      if (name in varsToTryToRemove) delete varsToTryToRemove[name];
    }
    function processVariable(name) {
      if (definitions[name] === 1 && uses[name] === 1) {
        potentials[name] = 1;
      } else if (uses[name] === 0 && (!definitions[name] || definitions[name] <= 1)) { // no uses, no def or 1 def (cannot operate on phis, and the llvm optimizer will remove unneeded phis anyhow) (no definition means it is a function parameter, or a local with just |var x;| but no defining assignment)
        var sideEffects = false;
        var value = values[name];
        if (value) {
          // TODO: merge with other side effect code
          // First, pattern-match
          //  (HEAP32[((tempDoublePtr)>>2)]=((HEAP32[(($_sroa_0_0__idx1)>>2)])|0),HEAP32[(((tempDoublePtr)+(4))>>2)]=((HEAP32[((($_sroa_0_0__idx1)+(4))>>2)])|0),(+(HEAPF64[(tempDoublePtr)>>3])))
          // which has no side effects and is the special form of converting double to i64.
          if (!(value[0] === 'seq' && value[1][0] === 'assign' && value[1][2][0] === 'sub' && value[1][2][2][0] === 'binary' && value[1][2][2][1] === '>>' &&
                value[1][2][2][2][0] === 'name' && value[1][2][2][2][1] === 'tempDoublePtr')) {
            // If not that, then traverse and scan normally.
            sideEffects = hasSideEffects(value);
          }
        }
        if (!sideEffects) {
          varsToRemove[name] = !definitions[name] ? 2 : 1; // remove it normally
          sideEffectFree[name] = true;
          // Each time we remove a variable with 0 uses, if its value has no
          // side effects and vanishes too, then we can remove a use from variables
          // appearing in it, and possibly eliminate again
          if (value) {
            traverse(value, function(node, type) {
              if (type === 'name') {
                var name = node[1];
                node[1] = ''; // we can remove this - it will never be shown, and should not be left to confuse us as we traverse
                if (name in locals) {
                  uses[name]--; // cannot be infinite recursion since we descend an energy function
                  assert(uses[name] >= 0);
                  unprocessVariable(name);
                  processVariable(name);
                }
              }
            });
          }
        } else {
          varsToTryToRemove[name] = 1; // try to remove it later during scanning
        }
      }
    }
    for (var name in locals) {
      processVariable(name);
    }

    //printErr('defs: ' + JSON.stringify(definitions));
    //printErr('uses: ' + JSON.stringify(uses));
    //printErr('values: ' + JSON.stringify(values));
    //printErr('locals: ' + JSON.stringify(locals));
    //printErr('varsToRemove: ' + JSON.stringify(varsToRemove));
    //printErr('varsToTryToRemove: ' + JSON.stringify(varsToTryToRemove));
    values = null;
    //printErr('potentials: ' + JSON.stringify(potentials));
    // We can now proceed through the function. In each list of statements, we try to eliminate
    var tracked = {};
    var globalsInvalidated = false; // do not repeat invalidations, until we track something new
    var memoryInvalidated = false;
    var callsInvalidated = false;
    function track(name, value, defNode) { // add a potential that has just been defined to the tracked list, we hope to eliminate it
      var usesGlobals = false, usesMemory = false, deps = {}, doesCall = false, hasDeps = false;
      var ignoreName = false; // one-time ignorings of names, as first op in sub and call
      traverse(value, function(node, type) {
        if (type === 'name') {
          if (!ignoreName) {
            var name = node[1];
            if (!(name in locals)) {
              usesGlobals = true;
            }
            if (!(name in potentials)) { // deps do not matter for potentials - they are defined once, so no complexity
              deps[name] = 1;
              hasDeps = true;
            }
          } else {
            ignoreName = false;
          }
        } else if (type === 'sub') {
          usesMemory = true;
          ignoreName = true;
        } else if (type === 'call') {
          usesGlobals = true;
          usesMemory = true;
          doesCall = true;        
          ignoreName = true;
        } else {
          ignoreName = false;
        }
      });
      tracked[name] = {
        usesGlobals: usesGlobals,
        usesMemory: usesMemory,
        defNode: defNode,
        deps: deps,
        hasDeps: hasDeps,
        doesCall: doesCall
      };
      globalsInvalidated = false;
      memoryInvalidated = false;
      callsInvalidated = false;
      //printErr('track ' + [name, JSON.stringify(tracked[name])]);
    }
    var temp = [];
    // TODO: invalidate using a sequence number for each type (if you were tracked before the last invalidation, you are cancelled). remove for.in loops
    function invalidateGlobals() {
      //printErr('invalidate globals');
      temp.length = 0;
      for (var name in tracked) {
        var info = tracked[name];
        if (info.usesGlobals) {
          temp.push(name);
        }
      }
      for (var i = 0; i < temp.length; i++) {
        delete tracked[temp[i]];
      }
    }
    function invalidateMemory() {
      //printErr('invalidate memory');
      temp.length = 0;
      for (var name in tracked) {
        var info = tracked[name];
        if (info.usesMemory) {
          temp.push(name);
        }
      }
      for (var i = 0; i < temp.length; i++) {
        delete tracked[temp[i]];
      }
    }
    function invalidateByDep(dep) {
      //printErr('invalidate by dep ' + dep);
      temp.length = 0;
      for (var name in tracked) {
        var info = tracked[name];
        if (info.deps[dep]) {
          temp.push(name);
        }
      }
      for (var i = 0; i < temp.length; i++) {
        delete tracked[temp[i]];
      }
    }
    function invalidateCalls() {
      //printErr('invalidate calls');
      temp.length = 0;
      for (var name in tracked) {
        var info = tracked[name];
        if (info.doesCall) {
          temp.push(name);
        }
      }
      for (var i = 0; i < temp.length; i++) {
        delete tracked[temp[i]];
      }
    }

    // Generate the sequence of execution. This determines what is executed before what, so we know what can be reordered. Using
    // that, performs invalidations and eliminations
    function scan(node) {
      //printErr('scan: ' + JSON.stringify(node).substr(0, 50) + ' : ' + keys(tracked));
      var abort = false;
      var allowTracking = true; // false inside an if; also prevents recursing in an if
      //var nesting = 1; // printErr-related
      function traverseInOrder(node, ignoreSub, ignoreName) {
        if (abort) return;
        //nesting++; // printErr-related
        //printErr(JSON.stringify(node).substr(0, 50) + ' : ' + keys(tracked) + ' : ' + [allowTracking, ignoreSub, ignoreName]);
        var type = node[0];
        if (type === 'assign') {
          var target = node[2];
          var value = node[3];
          var nameTarget = target[0] === 'name';
          traverseInOrder(target, true, nameTarget); // evaluate left
          traverseInOrder(value); // evaluate right
          // do the actual assignment
          if (nameTarget) {
            var name = target[1];
            if (!(name in potentials)) {
              if (!(name in varsToTryToRemove)) {
                // expensive check for invalidating specific tracked vars. This list is generally quite short though, because of
                // how we just eliminate in short spans and abort when control flow happens TODO: history numbers instead
                invalidateByDep(name); // can happen more than once per dep..
                if (!(name in locals) && !globalsInvalidated) {
                  invalidateGlobals();
                  globalsInvalidated = true;
                }
                // if we can track this name (that we assign into), and it has 0 uses and we want to remove its 'var'
                // definition - then remove it right now, there is no later chance
                if (allowTracking && (name in varsToRemove) && uses[name] === 0) {
                  track(name, node[3], node);
                  doEliminate(name, node);
                }
              } else {
                // replace it in-place
                node.length = value.length;
                for (var i = 0; i < value.length; i++) {
                  node[i] = value[i];
                }
                varsToRemove[name] = 2;
              }
            } else {
              if (allowTracking) track(name, node[3], node);
            }
          } else if (target[0] === 'sub') {
            if (isTempDoublePtrAccess(target)) {
              if (!globalsInvalidated) {
                invalidateGlobals();
                globalsInvalidated = true;
              }
            } else if (!memoryInvalidated) {
              invalidateMemory();
              memoryInvalidated = true;
            }
          }
        } else if (type === 'sub') {
          traverseInOrder(node[1], false, !memSafe); // evaluate inner
          traverseInOrder(node[2]); // evaluate outer
          // ignoreSub means we are a write (happening later), not a read
          if (!ignoreSub && !isTempDoublePtrAccess(node)) {
            // do the memory access
            if (!callsInvalidated) {
              invalidateCalls();
              callsInvalidated = true;
            }
          }
        } else if (type === 'var') {
          var vars = node[1];
          for (var i = 0; i < vars.length; i++) {
            var name = vars[i][0];
            var value = vars[i][1];
            if (value) {
              traverseInOrder(value);
              if (name in potentials && allowTracking) {
                track(name, value, node);
              } else {
                invalidateByDep(name);
              }
              if (vars.length === 1 && name in varsToTryToRemove && value) {
                // replace it in-place
                value = ['stat', value];
                node.length = value.length;
                for (var i = 0; i < value.length; i++) {
                  node[i] = value[i];
                }
                varsToRemove[name] = 2;
              }
            }
          }
        } else if (type === 'binary') {
          var flipped = false;
          if (node[1] in ASSOCIATIVE_BINARIES && !(node[2][0] in NAME_OR_NUM) && node[3][0] in NAME_OR_NUM) { // TODO recurse here?
            // associatives like + and * can be reordered in the simple case of one of the sides being a name, since we assume they are all just numbers
            var temp = node[2];
            node[2] = node[3];
            node[3] = temp;
            flipped = true;
          }
          traverseInOrder(node[2]);
          traverseInOrder(node[3]);
          if (flipped && node[2][0] in NAME_OR_NUM) { // dunno if we optimized, but safe to flip back - and keeps the code closer to the original and more readable
            var temp = node[2];
            node[2] = node[3];
            node[3] = temp;
          }
        } else if (type === 'name') {
          if (!ignoreName) { // ignoreName means we are the name of something like a call or a sub - irrelevant for us
            var name = node[1];
            if (name in tracked) {
              doEliminate(name, node);
            } else if (!(name in locals) && !callsInvalidated && (memSafe || !(name in HEAP_NAMES))) { // ignore HEAP8 etc when not memory safe, these are ok to
                                                                                                       // access, e.g. SIMD_Int32x4_load(HEAP8, ...)
              invalidateCalls();
              callsInvalidated = true;
            }
          }
        } else if (type === 'unary-prefix' || type === 'unary-postfix') {
          traverseInOrder(node[2]);
        } else if (type in IGNORABLE_ELIMINATOR_SCAN_NODES) {
        } else if (type === 'call') {
          traverseInOrder(node[1], false, true);
          var args = node[2];
          for (var i = 0; i < args.length; i++) {
            traverseInOrder(args[i]);
          }
          if (callHasSideEffects(node)) {
            // these two invalidations will also invalidate calls
            if (!globalsInvalidated) {
              invalidateGlobals();
              globalsInvalidated = true;
            }
            if (!memoryInvalidated) {
              invalidateMemory();
              memoryInvalidated = true;
            }
          }
        } else if (type === 'if') {
          if (allowTracking) {
            traverseInOrder(node[1]); // can eliminate into condition, but nowhere else
            if (!callsInvalidated) { // invalidate calls, since we cannot eliminate them into an if that may not execute!
              invalidateCalls();
              callsInvalidated = true;
            }

            allowTracking = false;
            traverseInOrder(node[2]); // 2 and 3 could be 'parallel', really..
            if (node[3]) traverseInOrder(node[3]);
            allowTracking = true;

          } else {
            tracked = {};
          }
        } else if (type === 'block') {
          var stats = node[1];
          if (stats) {
            for (var i = 0; i < stats.length; i++) {
              traverseInOrder(stats[i]);
            }
          }
        } else if (type === 'stat') {
          traverseInOrder(node[1]);
        } else if (type === 'label') {
          traverseInOrder(node[2]);
        } else if (type === 'seq') {
          traverseInOrder(node[1]);
          traverseInOrder(node[2]);
        } else if (type === 'do') {
          if (node[1][0] === 'num' && node[1][1] === 0) { // one-time loop
            traverseInOrder(node[2]);
          } else {
            tracked = {};
          }
        } else if (type === 'return') {
          if (node[1]) traverseInOrder(node[1]);
        } else if (type === 'conditional') {
          if (!callsInvalidated) { // invalidate calls, since we cannot eliminate them into a branch of an LLVM select/JS conditional that does not execute
            invalidateCalls();
            callsInvalidated = true;
          }
          traverseInOrder(node[1]);
          traverseInOrder(node[2]);
          traverseInOrder(node[3]);
        } else if (type === 'switch') {
          traverseInOrder(node[1]);
          var originalTracked = {};
          for (var o in tracked) originalTracked[o] = 1;
          var cases = node[2];
          for (var i = 0; i < cases.length; i++) {
            var c = cases[i];
            assert(c[0] === null || c[0][0] === 'num' || (c[0][0] === 'unary-prefix' && c[0][2][0] === 'num'));
            var stats = c[1];
            for (var j = 0; j < stats.length; j++) {
              traverseInOrder(stats[j]);
            }
            // We cannot track from one switch case into another if there are external dependencies, undo all new trackings
            // Otherwise we can track, e.g. a var used in a case before assignment in another case is UB in asm.js, so no need for the assignment
            // TODO: general framework here, use in if-else as well
            for (var t in tracked) {
              if (!(t in originalTracked)) {
                var info = tracked[t];
                if (info.usesGlobals || info.usesMemory || info.hasDeps) {
                  delete tracked[t];
                }
              }
            }
          }
          tracked = {}; // do not track from inside the switch to outside
        } else {
          if (!(type in ABORTING_ELIMINATOR_SCAN_NODES)) {
            printErr('unfamiliar eliminator scan node: ' + JSON.stringify(node));
          }
          tracked = {};
          abort = true;
        }
        //nesting--; // printErr-related
      }
      traverseInOrder(node);
    }
    //var eliminationLimit = 0; // used to debugging purposes
    function doEliminate(name, node) {
      //if (eliminationLimit === 0) return;
      //eliminationLimit--;
      //printErr('elim!!!!! ' + name);
      // yes, eliminate!
      varsToRemove[name] = 2; // both assign and var definitions can have other vars we must clean up
      var info = tracked[name];
      delete tracked[name];
      var defNode = info.defNode;
      var value;
      if (!sideEffectFree[name]) {
        if (defNode[0] === 'var') {
          defNode[1].forEach(function(pair) {
            if (pair[0] === name) {
              value = pair[1];
            }
          });
          assert(value);
        } else { // assign
          value = defNode[3];
          // wipe out the assign
          defNode[0] = 'toplevel';
          defNode[1] = [];
          defNode.length = 2;
        }
        // replace this node in-place
        node.length = 0;
        for (var i = 0; i < value.length; i++) {
          node[i] = value[i];
        }
      } else {
        // This has no side effects and no uses, empty it out in-place
        node.length = 0;
        node[0] = 'toplevel';
        node[1] = [];
      }
    }
    traverse(func, function(block) {
      // Look for statements, including while-switch pattern
      var stats = getStatements(block) || (block[0] === 'while' && block[2][0] === 'switch' ? [block[2]] : stats);
      if (!stats) return;
      //printErr('Stats: ' + JSON.stringify(stats).substr(0,100));
      tracked = {};
      //printErr('new StatBlock');
      for (var i = 0; i < stats.length; i++) {
        var node = stats[i];
        //printErr('StatBlock[' + i + '] => ' + JSON.stringify(node).substr(0,100));
        var type = node[0];
        if (type === 'stat') {
          node = node[1];
          type = node[0];
        } else if (type == 'return' && i < stats.length-1) {
          stats.length = i+1; // remove any code after a return
        }
        // Check for things that affect elimination
        if (type in ELIMINATION_SAFE_NODES) {
          scan(node);
        } else {
          tracked = {}; // not a var or assign, break all potential elimination so far
        }
      }
      //printErr('delete StatBlock');
    });

    var seenUses = {}, helperReplacements = {}; // for looper-helper optimization

    // clean up vars, and loop variable elimination
    traverse(func, function(node, type) {
      // pre
      if (type === 'var') {
        node[1] = node[1].filter(function(pair) { return !varsToRemove[pair[0]] });
        if (node[1].length === 0) {
          // wipe out an empty |var;|
          node[0] = 'toplevel';
          node[1] = [];
        }
      } else if (type === 'assign' && node[1] === true && node[2][0] === 'name' && node[3][0] === 'name' && node[2][1] === node[3][1]) {
        // elimination led to X = X, which we can just remove
        return emptyNode();
      }
    }, function(node, type) {
      // post
      if (type === 'name') {
        var name = node[1];
        if (name in helperReplacements) {
          node[1] = helperReplacements[name];
          return; // no need to track this anymore, we can't loop-optimize more than once
        }
        // track how many uses we saw. we need to know when a variable is no longer used (hence we run this in the post)
        if (!(name in seenUses)) {
          seenUses[name] = 1;
        } else {
          seenUses[name]++;
        }
      } else if (type === 'while') {
        if (!asm) return;
        // try to remove loop helper variables specifically
        var stats = node[2][1];
        var last = stats[stats.length-1];
        if (last && last[0] === 'if' && last[2][0] === 'block' && last[3] && last[3][0] === 'block') {
          var ifTrue = last[2];
          var ifFalse = last[3];
          clearEmptyNodes(ifTrue[1]);
          clearEmptyNodes(ifFalse[1]);
          var flip = false;
          if (ifFalse[1][0] && ifFalse[1][ifFalse[1].length-1][0] === 'break') { // canonicalize break in the if-true
            var temp = ifFalse;
            ifFalse = ifTrue;
            ifTrue = temp;
            flip = true;
          }
          if (ifTrue[1][0] && ifTrue[1][ifTrue[1].length-1][0] === 'break') {
            var assigns = ifFalse[1];
            clearEmptyNodes(assigns);
            var loopers = [], helpers = [];
            for (var i = 0; i < assigns.length; i++) {
              if (assigns[i][0] === 'stat' && assigns[i][1][0] === 'assign') {
                var assign = assigns[i][1];
                if (assign[1] === true && assign[2][0] === 'name' && assign[3][0] === 'name') {
                  var looper = assign[2][1];
                  var helper = assign[3][1];
                  if (definitions[helper] === 1 && seenUses[looper] === namings[looper] &&
                      !helperReplacements[helper] && !helperReplacements[looper]) {
                    loopers.push(looper);
                    helpers.push(helper);
                  }
                }
              }
            }
            // remove loop vars that are used in the rest of the else
            for (var i = 0; i < assigns.length; i++) {
              if (assigns[i][0] === 'stat' && assigns[i][1][0] === 'assign') {
                var assign = assigns[i][1];
                if (!(assign[1] === true && assign[2][0] === 'name' && assign[3][0] === 'name') || loopers.indexOf(assign[2][1]) < 0) {
                  // this is not one of the loop assigns
                  traverse(assign, function(node, type) {
                    if (type === 'name') {
                      var index = loopers.indexOf(node[1]);
                      if (index < 0) index = helpers.indexOf(node[1]);
                      if (index >= 0) {
                        loopers.splice(index, 1);
                        helpers.splice(index, 1);
                      }
                    }
                  });
                }
              }
            }
            // remove loop vars that are used in the if
            traverse(ifTrue, function(node, type) {
              if (type === 'name') {
                var index = loopers.indexOf(node[1]);
                if (index < 0) index = helpers.indexOf(node[1]);
                if (index >= 0) {
                  loopers.splice(index, 1);
                  helpers.splice(index, 1);
                }
              }
            });
            if (loopers.length === 0) return;
            for (var l = 0; l < loopers.length; l++) {
              var looper = loopers[l];
              var helper = helpers[l];
              // the remaining issue is whether loopers are used after the assignment to helper and before the last line (where we assign to it)
              var found = -1;
              for (var i = stats.length-2; i >= 0; i--) {
                var curr = stats[i];
                if (curr[0] === 'stat' && curr[1][0] === 'assign') {
                  var currAssign = curr[1];
                  if (currAssign[1] === true && currAssign[2][0] === 'name') {
                    var to = currAssign[2][1];
                    if (to === helper) {
                      found = i;
                      break;
                    }
                  }
                }
              }
              if (found < 0) return;
              // if a loop variable is used after we assigned to the helper, we must save its value and use that.
              // (note that this can happen due to elimination, if we eliminate an expression containing the
              // loop var far down, past the assignment!)
              // first, see if the looper and helpers overlap. Note that we check for this looper, compared to
              // *ALL* the helpers. Helpers will be replaced by loopers as we eliminate them, potentially
              // causing conflicts, so any helper is a concern.
              var firstLooperUsage = -1;
              var lastLooperUsage = -1;
              var firstHelperUsage = -1;
              for (var i = found+1; i < stats.length; i++) {
                var curr = i < stats.length-1 ? stats[i] : last[1]; // on the last line, just look in the condition
                traverse(curr, function(node, type) {
                  if (type === 'name') {
                    if (node[1] === looper) {
                      if (firstLooperUsage < 0) firstLooperUsage = i;
                      lastLooperUsage = i;
                    } else if (helpers.indexOf(node[1]) >= 0) {
                      if (firstHelperUsage < 0) firstHelperUsage = i;
                    }
                  }
                });
              }
              if (firstLooperUsage >= 0) {
                // the looper is used, we cannot simply merge the two variables
                if ((firstHelperUsage < 0 || firstHelperUsage > lastLooperUsage) && lastLooperUsage+1 < stats.length && triviallySafeToMove(stats[found], asmData) &&
                    seenUses[helper] === namings[helper]) {
                  // the helper is not used, or it is used after the last use of the looper, so they do not overlap,
                  // and the last looper usage is not on the last line (where we could not append after it), and the
                  // helper is not used outside of the loop.
                  // just move the looper definition to after the looper's last use
                  stats.splice(lastLooperUsage+1, 0, stats[found]);
                  stats.splice(found, 1);
                } else {
                  // they overlap, we can still proceed with the loop optimization, but we must introduce a
                  // loop temp helper variable
                  var temp = looper + '$looptemp';
                  assert(!(temp in asmData.vars)); 
                  for (var i = firstLooperUsage; i <= lastLooperUsage; i++) {
                    var curr = i < stats.length-1 ? stats[i] : last[1]; // on the last line, just look in the condition
                    traverse(curr, function looperToLooptemp(node, type) {
                      if (type === 'name') {
                        if (node[1] === looper) {
                          node[1] = temp;
                        }
                      } else if (type === 'assign' && node[2][0] === 'name') {
                        // do not traverse the assignment target, phi assignments to the loop variable must remain
                        traverse(node[3], looperToLooptemp);
                        return null;
                      }
                    });
                  }
                  asmData.vars[temp] = asmData.vars[looper];
                  stats.splice(found, 0, ['stat', ['assign', true, ['name', temp], ['name', looper]]]);
                }
              }
            }
            for (var l = 0; l < helpers.length; l++) {
              for (var k = 0; k < helpers.length; k++) {
                if (l != k && helpers[l] === helpers[k]) return; // it is complicated to handle a shared helper, abort
              }
            }
            // hurrah! this is safe to do
            //printErr("ELIM LOOP VAR " + JSON.stringify(loopers) + ' :: ' + JSON.stringify(helpers));
            for (var l = 0; l < loopers.length; l++) {
              var looper = loopers[l];
              var helper = helpers[l];
              varsToRemove[helper] = 2;
              traverse(node, function(node, type) { // replace all appearances of helper with looper
                if (type === 'name' && node[1] === helper) node[1] = looper;
              });
              helperReplacements[helper] = looper; // replace all future appearances of helper with looper
              helperReplacements[looper] = looper; // avoid any further attempts to optimize looper in this manner (seenUses is wrong anyhow, too)
            }
            // simplify the if. we remove the if branch, leaving only the else
            if (flip) {
              last[1] = simplifyNotCompsDirect(['unary-prefix', '!', last[1]]);
              var temp = last[2];
              last[2] = last[3];
              last[3] = temp;
            }
            if (loopers.length === assigns.length) {
              last.pop();
            } else {
              var elseStats = getStatements(last[3]);
              for (var i = 0; i < elseStats.length; i++) {
                var stat = elseStats[i];
                if (stat[0] === 'stat') stat = stat[1];
                if (stat[0] === 'assign' && stat[2][0] === 'name') {
                  if (loopers.indexOf(stat[2][1]) >= 0) {
                    elseStats[i] = emptyNode();
                  }
                }
              }
            }
          }
        }
      }
    });

    if (asm) {
      for (var v in varsToRemove) {
        if (varsToRemove[v] === 2) delete asmData.vars[v];
      }
      denormalizeAsm(func, asmData);
    }
  });

  if (!asm) { // TODO: deprecate in non-asm too
    // A class for optimizing expressions. We know that it is legitimate to collapse
    // 5+7 in the generated code, as it will always be numerical, for example. XXX do we need this? here?
    function ExpressionOptimizer(node) {
      this.node = node;

      this.run = function() {
        traverse(this.node, function(node, type) {
          if (type === 'binary' && node[1] === '+') {
            var names = [];
            var num = 0;
            var has_num = false;
            var fail = false;
            traverse(node, function(subNode, subType) {
              if (subType === 'binary') {
                if (subNode[1] !== '+') {
                  fail = true;
                  return false;
                }
              } else if (subType === 'name') {
                names.push(subNode[1]);
                return;
              } else if (subType === 'num') {
                num += subNode[1];
                has_num = true;
                return;
              } else {
                fail = true;
                return false;
              }
            });
            if (!fail && has_num) {
              var ret = ['num', num];
              for (var i = 0; i < names.length; i++) {
                ret = ['binary', '+', ['name', names[i]], ret];
              }
              return ret;
            }
          }
        });
      };
    }
    new ExpressionOptimizer(ast).run();
  }

  removeAllEmptySubNodes(ast);
}

function eliminateMemSafe(ast) {
  eliminate(ast, true);
}

function minifyGlobals(ast) {
  var minified = {};
  var next = 0;
  var first = true; // do not minify initial 'var asm ='
  // find the globals
  traverse(ast, function(node, type) {
    if (type === 'var' || type === 'const') {
      if (first) {
        first = false;
        return;
      }
      var vars = node[1];
      for (var i = 0; i < vars.length; i++) {
        var name = vars[i][0];
        ensureMinifiedNames(next);
        vars[i][0] = minified[name] = minifiedNames[next++];
      }
    } else if (type === 'defun') {
      var name = node[1];
      ensureMinifiedNames(next);
      node[1] = minified[name] = minifiedNames[next++];
    }
  });
  // add all globals in function chunks, i.e. not here but passed to us
  for (var i = 0; i < extraInfo.globals.length; i++) {
    name = extraInfo.globals[i];
    ensureMinifiedNames(next);
    minified[name] = minifiedNames[next++];
  }
  // apply minification
  traverse(ast, function(node, type) {
    if (type === 'name') {
      var name = node[1];
      if (name in minified) {
        node[1] = minified[name];
      }
    }
  });
  suffix = '// EXTRA_INFO:' + JSON.stringify(minified);
}


function minifyLocals(ast) {
  assert(asm);
  assert(extraInfo && extraInfo.globals);

  traverseGeneratedFunctions(ast, function(fun, type) {

    // Analyse the asmjs to figure out local variable names,
    // but operate on the original source tree so that we don't
    // miss any global names in e.g. variable initializers.
    var asmData = normalizeAsm(fun); denormalizeAsm(fun, asmData); // TODO: we can avoid modifying at all here - we just need a list of local vars+params
    var newNames = {};
    var usedNames = {};

    // Find all the globals that we need to minify using
    // pre-assigned names.  Don't actually minify them yet
    // as that might interfere with local variable names.
    function isLocalName(name) {
      return name in asmData.vars || name in asmData.params;
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
    if (fun[1] in extraInfo.globals) {
      fun[1] = extraInfo.globals[fun[1]];
      assert(fun[1]);
    }
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

// Relocation pass for a shared module (for the functions part of the module)
//
// 1. Replace function names with alternate names as defined (to avoid colliding with
// names in the main module we are being linked to)
// 2. Hardcode function table offsets from F_BASE+x to const+x if x is a variable, or
//    the constant sum of the base + offset
// 3. Hardcode heap offsets from H_BASE as well
function relocate(ast) {
  assert(asm); // we also assume we are normalized

  var replacements = extraInfo.replacements;
  var fBases = extraInfo.fBases;
  var hBase = extraInfo.hBase;
  var m;

  traverse(ast, function(node, type) {
    switch(type) {
      case 'name': case 'defun': {
        var rep = replacements[node[1]];
        if (rep) node[1] = rep;
        break;
      }
      case 'binary': {
        if (node[1] == '+' && node[2][0] == 'name') {
          var base = null;
          if (node[2][1] == 'H_BASE') {
            base = hBase;
          } else if (m = /^F_BASE_(\w+)$/.exec(node[2][1])) {
            base = fBases[m[1]] || 0; // 0 if the parent has no function table for this, but the child does. so no relocation needed
          }
          if (base !== null) {
            var other = node[3];
            if (base === 0) return other;
            if (other[0] == 'num') {
              other[1] = (other[1] + base)|0;
              return other;
            } else {
              node[2] = ['num', base];
            }
          }
        }
        break;
      }
      case 'var': {
        var vars = node[1];
        for (var i = 0; i < vars.length; i++) {
          var name = vars[i][0];
          assert(!(name in replacements)); // cannot shadow functions we are replacing TODO: fix that
        }
        break;
      }
    }
  });
}

// Break up very large functions

var NODES_WITHOUT_ELIMINATION_SENSITIVITY = set('name', 'num', 'binary', 'unary-prefix');
var FAST_ELIMINATION_BINARIES = setUnion(setUnion(USEFUL_BINARY_OPS, COMPARE_OPS), set('+'));

function measureSize(ast) {
  var size = 0;
  traverse(ast, function() {
    size++;
  });
  return size;
}

function measureCost(ast) {
  var size = 0;
  traverse(ast, function(node, type) {
    if (type === 'num' || type === 'unary-prefix') size--;
    else if (type === 'binary') {
      if (node[3][0] === 'num' && node[3][1] === 0) size--;
      else if (node[1] === '/' || node[1] === '%') size += 2;
    }
    else if (type === 'call' && !callHasSideEffects(node)) size -= 2;
    else if (type === 'sub') size++;
    size++;
  });
  return size;
}

function aggressiveVariableEliminationInternal(func, asmData) {
  // This removes as many variables as possible. This is often not the best thing because it increases
  // code size, but it is far preferable to the risk of split functions needing to do more spilling, so
  // we use it when outlining.
  // Specifically, this finds 'trivial' variables: ones with 1 definition, and that definition is not sensitive to any changes: it
  // only depends on constants and local variables that are themselves trivial. We can unquestionably eliminate
  // such variables in a trivial manner.

  var assignments = {};
  var appearances = {};
  var defs = {};
  var considered = {};

  traverse(func, function(node, type) {
    if (type == 'assign' && node[2][0] == 'name') {
      var name = node[2][1];
      if (name in asmData.vars) {
        assignments[name] = (assignments[name] || 0) + 1;
        appearances[name] = (appearances[name] || 0) - 1; // this appearance is a definition, offset the counting later
        defs[name] = node;
      } else {
        if (name in asmData.params) {
          assignments[name] = (assignments[name] || 1) + 1; // init to 1 for initial parameter assignment
          considered[name] = true; // this parameter is not ssa, it must be in a hand-optimized function, so it is not trivial
        }
      }
    } else if (type == 'name') {
      var name = node[1];
      if (name in asmData.vars) {
        appearances[name] = (appearances[name] || 0) + 1;
      }
    }
  });

  var allTrivials = {}; // key of a trivial var => size of its (expanded) value, at least 1

  // three levels of variables:
  // 1. trivial: 1 def (or less), uses nothing sensitive, can be eliminated
  // 2. safe: 1 def (or less), can be used in a trivial, but cannot itself be eliminated
  // 3. sensitive: uses a global or memory or something else that prevents trivial elimination.

  function assessTriviality(name) {
    // only care about vars with 0-1 assignments of (0 for parameters), and can ignore label (which is not explicitly initialized, but cannot be eliminated ever anyhow)
    if (assignments[name] > 1 || (!(name in asmData.vars) && !(name in asmData.params)) || name == 'label') return false;
    if (considered[name]) return allTrivials[name];
    considered[name] = true;
    var sensitive = false;
    var size = 0, originalSize = 0;
    var def = defs[name];
    if (def) {
      var value = def[3];
      originalSize = measureSize(value);
      if (value) {
        traverse(value, function recurseValue(node, type) {
          var one = node[1];
          if (!(type in NODES_WITHOUT_ELIMINATION_SENSITIVITY)) { // || (type == 'binary' && !(one in FAST_ELIMINATION_BINARIES))) {
            sensitive = true;
            return true;
          }
          if (type == 'name' && !assessTriviality(one)) {
            if (assignments[one] > 1 || (!(one in asmData.vars) && !(one in asmData.params))) {
              sensitive = true; // directly using something sensitive
              return true;
            } // otherwise, not trivial, but at least safe.
          }
          // if this is a name, it must be a trivial variable (or a safe one) and we know its size
          size += ((type == 'name') ? allTrivials[one] : 1) || 1;
        });
      }
    }
    if (!sensitive) {
      size = size || 1;
      originalSize = originalSize || 1;
      var factor = ((appearances[name] - 1) || 0) * (size - originalSize); // If no size change or just one appearance, always ok to trivially eliminate. otherwise, tradeoff
      if (factor <= 12) {
        allTrivials[name] = size; // trivial!
        return true;
      }
    }
    return false;
  }
  for (var name in asmData.vars) {
    assessTriviality(name);
  }
  var trivials = {};

  for (var name in allTrivials) { // from now on, ignore parameters
    if (name in asmData.vars) trivials[name] = true;
  }

  allTrivials = {};

  var values = {}, recursives = {};

  function evaluate(name) {
    var node = values[name];
    if (node) return node;
    values[name] = null; // prevent infinite recursion
    var def = defs[name];
    if (def) {
      node = def[3];
      if (node[0] == 'name') {
        var name2 = node[1];
        assert(name2 !== name);
        if (name2 in trivials) {
          node = evaluate(name2);
        }
      } else {
        traverse(node, function(node, type) {
          if (type == 'name') {
            var name2 = node[1];
            if (name2 === name) {
              recursives[name] = 1;
              return false;
            }
            if (name2 in trivials) {
              return evaluate(name2);
            }
          }
        });
      }
      values[name] = node;
    }
    return node;
  }

  for (var name in trivials) {
    evaluate(name);
  }
  for (var name in recursives) {
    delete trivials[name];
  }

  for (var name in trivials) {
    var def = defs[name];
    if (def) {
      def.length = 0;
      def[0] = 'toplevel';
      def[1] = [];
    }
    delete asmData.vars[name];
  }

  // Perform replacements TODO: save list of uses objects before, replace directly, avoid extra traverse
  traverse(func, function(node, type) {
    if (type == 'name') {
      var name = node[1];
      if (name in trivials) {
        var value = values[name];
        if (value) return copy(value); // must copy, or else the same object can be used multiple times
        else return emptyNode();
      }
    }
  });

  removeAllEmptySubNodes(func);
}

function aggressiveVariableElimination(ast) {
  assert(asm, 'need ASM_JS for aggressive variable elimination');
  traverseGeneratedFunctions(ast, function(func, type) {
    var asmData = normalizeAsm(func);
    aggressiveVariableEliminationInternal(func, asmData);
    denormalizeAsm(func, asmData);
  });
}

function outline(ast) {
  // Move from 'label' to another name. This will avoid optimizations for label later, but we modify control flow so much here, we just give up on them
  function deLabel(func, asmData) {
    if (!('label' in asmData.vars)) return;
    safeLabelSettingInternal(func, asmData);
    assert(!('fakeLabel' in asmData.vars));
    asmData.vars.fakeLabel = ASM_INT;
    delete asmData.vars.label;
    traverse(func, function(node, type) {
      if (type === 'name' && node[1] === 'label') node[1] = 'fakeLabel';
    });
  }
  // Try to flatten out code as much as possible, to make outlining more feasible.
  function flatten(func, asmData) {
    var minSize = extraInfo.sizeToOutline/4;
    var helperId = 0;
    function getHelper() {
      while (1) {
        var ret = 'helper$' + (helperId++);
        if (!(ret in asmData.vars) && !(ret in asmData.params)) {
          asmData.vars[ret] = ASM_INT;
          return ret;
        }
      }
    }
    var ignore = [];
    traverse(func, function(node) {
      if (node[0] === 'while' && node[2][0] !== 'block') {
        node[2] = ['block', [node[2]]]; // so we have a list of statements and can flatten  while(1) switch
      }
      var stats = getStatements(node);
      if (stats) {
        for (var i = 0; i < stats.length; i++) {
          var node = stats[i]; // step over param
          if (ignore.indexOf(node) >= 0) continue;
          if (node[0] == 'stat') node = node[1];
          if (ignore.indexOf(node) >= 0) continue;
          var type = node[0];
          if (measureSize(node) >= minSize) {
            if ((type === 'if' && node[3]) || type === 'switch') {
              var isIf = type === 'if';
              var reps = [];
              var helper = getHelper();
              // clear helper
              reps.push(['stat', ['assign', true, ['name', helper], ['num', 1]]]); // 1 means continue in ifs
              // gather parts
              var parts;
              if (isIf) {
                parts = [];
                var curr = node;
                while (1) {
                  if (!curr[3]) {
                    // we normally expect ..if (cond) { .. } else [if (nextCond) {] (in [] is what we hope to see)
                    // but are now seeing ..if (cond) { .. } with no else. This might be
                    //                    ..if (cond) if (nextCond) {
                    // which vacuum can generate from       if (cond) {} else if (nextCond), making it
                    //                                      if (!cond) if (nextCond)
                    // so we undo that, in hopes of making it more flattenable
                    curr[3] = curr[2];
                    curr[2] = ['block', []];
                    curr[1] = simplifyNotCompsDirect(['unary-prefix', '!', curr[1]]);
                  }
                  parts.push({ condition: curr[1], body: curr[2] });
                  curr = curr[3];
                  if (!curr) break;
                  if (curr[0] != 'if') {
                    parts.push({ condition: null, body: curr });
                    break;
                  }
                }
              } else { // switch
                var switchVar = getHelper(); // switch var could be an expression
                reps.push(['stat', ['assign', true, ['name', switchVar], node[1]]]);
                parts = node[2].map(function(case_) {
                  return { condition: case_[0], body: case_[1] };
                });
              }
              // chunkify. Each chunk is a chain of if-elses, with the new overhead just on entry and exit
              var chunks = [];
              var currSize = 0;
              var currChunk = [];
              var force = false; // when we hit a case X: that falls through, we force inclusion of everything until a full case
              parts.forEach(function(part) {
                var size = (part.condition ? measureSize(part.condition) : 0) + measureSize(part.body) + 5; // add constant for overhead of extra code
                assert(size > 0);
                if (size + currSize >= minSize && currSize && !force) {
                  chunks.push(currChunk);
                  currChunk = [];
                  currSize = 0;
                }
                currChunk.push(part);
                currSize += size;
                if (!isIf) {
                  var last = part.body;
                  last = last[last.length-1];
                  if (last && last[0] === 'block') last = last[1][last[1].length-1];
                  if (last && last[0] === 'stat') last = last[1];
                  force = !last || !(last[0] in ALTER_FLOW);
                }
              });
              assert(currSize);
              chunks.push(currChunk);
              // generate flattened code
              chunks.forEach(function(chunk) {
                var pre = ['stat', ['assign', true, ['name', helper], ['num', 0]]];
                if (isIf) {
                  var chain = null, tail = null;
                  chunk.forEach(function(part) {
                    // add to chain
                    var contents = makeIf(part.condition || ['num', 1], part.body[1]);
                    if (chain) {
                      tail[3] = contents;
                    } else {
                      chain = contents;
                      ignore.push(contents);
                    }
                    tail = contents;
                  });
                  // if none of the ifs were entered, in the final else note that we need to continue
                  tail[3] = ['block', [['stat', ['assign', true, ['name', helper], ['num', 1]]]]];
                  reps.push(makeIf(['name', helper], [pre, chain]));
                } else { // switch
                  var hasDefault;
                  var s = makeSwitch(['binary', '|', ['name', switchVar], ['num', 0]], chunk.map(function(part) {
                    hasDefault = hasDefault || part.condition === null;
                    return [part.condition, part.body];
                  }));
                  // if no default, add one where we note that we need to continue
                  if (!hasDefault) {
                    s[2].push([null, [['block', [['stat', ['assign', true, ['name', helper], ['num', 1]]]]]]]);
                  }
                  ignore.push(s);
                  reps.push(makeIf(['name', helper], [pre, s]));
                }
              });
              // replace code and update i
              stats.splice.apply(stats, [i, 1].concat(reps));
              i--; // negate loop increment
              i += reps.length;
              continue;
            }
          }
        }
      }
    });
  }

  var maxTotalOutlinings = Infinity; // debugging tool

  // Prepares information for spilling of local variables
  function analyzeFunction(func, asmData) {
    var stack = []; // list of variables, each gets 8 bytes
    for (var name in asmData.params) {
      stack.push(name);
    }
    for (var name in asmData.vars) {
      stack.push(name);
    }
    asmData.stackPos = {};
    var stackSize = getStackBumpSize(func);
    assert(stackSize % STACK_ALIGN === 0, 'bad stack! ' + stackSize);
    stackSize += STACK_ALIGN;
    for (var i = 0; i < stack.length; i++) {
      asmData.stackPos[stack[i]] = stackSize + i*8;
    }
    // Reserve an extra two spots per possible outlining: one for control flow var, the other for control flow data
    // The control variables are zeroed out when calling an outlined function, and after using
    // the value after they return.
    var size = measureSize(func);
    asmData.maxOutlinings = Math.min(Math.round(3*size/extraInfo.sizeToOutline), maxTotalOutlinings);
    asmData.maxAttemptedOutlinings = Infinity;
    if (extraInfo.sizeToOutline < 100) asmData.maxAttemptedOutlinings = Math.min(50, asmData.maxAttemptedOutlinings); // tiny sizes, be careful of too many attempts
    asmData.intendedPieces = Math.ceil(size/extraInfo.sizeToOutline);
    asmData.totalStackSize = stackAlign(stackSize + (stack.length + 2*asmData.maxOutlinings)*8);
    asmData.controlStackPos = function(i) { return stackSize + (stack.length + i)*8 };
    asmData.controlDataStackPos = function(i) { return stackSize + (stack.length + i)*8 + 4 };
    asmData.splitCounter = 0;
  }

  // Analyze uses - reads and writes - of variables in part of the AST of a function
  function analyzeCode(func, asmData, ast) {
    var labels = {}; // labels defined in this code
    var labelCounter = 1; // 0 means no label

    traverse(ast, function(node, type) {
      if (type == 'label' && !(node[1] in labels)) {
        labels[node[1]] = labelCounter++;
      }
    });

    var writes = {};
    var namings = {};
    var hasReturn = false, hasReturnType = {}, hasBreak = false, hasContinue = false;
    var breaks = {};    // set of labels we break or continue
    var continues = {}; // to (name -> id, just like labels)
    var breakCapturers = 0;
    var continueCapturers = 0;

    traverse(ast, function(node, type) {
      if (type == 'assign' && node[2][0] == 'name') {
        var name = node[2][1];
        if (name in asmData.vars || name in asmData.params) {
          writes[name] = (writes[name] || 0) + 1;
        }
      } else if (type == 'name') {
        var name = node[1];
        if (name in asmData.vars || name in asmData.params) {
          namings[name] = (namings[name] || 0) + 1;
        }
      } else if (type == 'return') {
        if (!node[1]) {
          hasReturn = true;
        } else {
          hasReturnType[detectType(node[1])] = true;
        }
      } else if (type == 'break') {
        var label = node[1] || 0;
        if (!label && breakCapturers > 0) return; // no label, and captured
        if (label && (label in labels)) return; // label, and defined in this code, so captured
        if (label) breaks[label] = labelCounter++;
        hasBreak = true;
      } else if (type == 'continue') {
        var label = node[1] || 0;
        if (!label && continueCapturers > 0) return; // no label, and captured
        if (label && (label in labels)) return; // label, and defined in this code, so captured
        if (label) continues[label] = labelCounter++;
        hasContinue = true;
      } else {
        if (type in BREAK_CAPTURERS) {
          breakCapturers++;
        }
        if (type in CONTINUE_CAPTURERS) {
          continueCapturers++;
        }
      }
    }, function(node, type) {
      if (type in BREAK_CAPTURERS) {
        breakCapturers--;
      }
      if (type in CONTINUE_CAPTURERS) {
        continueCapturers--;
      }
    });

    var reads = {};
    for (var v in namings) {
      var actualReads = namings[v] - (writes[v] || 0);
      if (actualReads > 0) reads[v] = actualReads;
    }

    return { writes: writes, reads: reads, hasReturn: hasReturn, hasReturnType: hasReturnType, hasBreak: hasBreak, hasContinue: hasContinue, breaks: breaks, continues: continues, labels: labels };
  }

  function makeAssign(dst, src) {
    return ['assign', true, dst, src];
  }
  function makeStackAccess(type, pos) { // TODO: float64, not 32
    return ['sub', ['name', type == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', pos]], ['num', 2]]];
  }
  function makeIf(cond, then, else_) {
    var ret = ['if', cond, ['block', then]];
    if (else_) ret.push(['block', else_]);
    return ret;
  }
  function makeComparison(left, comp, right) {
    return ['binary', comp, left, right];
  }
  function makeSwitch(value, cases) {
    return ['switch', value, cases];
  }

  var CONTROL_BREAK = 1, CONTROL_BREAK_LABEL = 2, CONTROL_CONTINUE = 3, CONTROL_CONTINUE_LABEL = 4, CONTROL_RETURN_VOID = 5, CONTROL_RETURN_INT = 6, CONTROL_RETURN_DOUBLE = 7, CONTROL_RETURN_FLOAT = 8;
  function controlFromAsmType(asmType) {
    return CONTROL_RETURN_INT + (asmType | 0); // assumes ASM_INT starts at 0, and order of these two is identical!
  }

  var sizeToOutline = null; // customized per function and as we make progress
  function calculateThreshold(func, asmData) {
    var size = measureSize(func);
    if (size <= extraInfo.sizeToOutline) {
      sizeToOutline = Infinity;
      //printErr('  no point in trying to reduce the size of ' + func[1] + ' which is ' + size + ' <= ' + extraInfo.sizeToOutline);
    } else {
      sizeToOutline = Math.round(size/Math.max(2, asmData.intendedPieces--));
      //printErr('trying to reduce the size of ' + func[1] + ' which is ' + size + ' (>=? ' + extraInfo.sizeToOutline + '), aim for ' + sizeToOutline);
    }
  }

  var level = 0, loops = 0;
  var outliningParents = {}; // function name => parent it was outlined from

  function doOutline(func, asmData, stats, start, end) {
    if (asmData.splitCounter === asmData.maxOutlinings) return [];
    if (!extraInfo.allowCostlyOutlines) var originalStats = copy(stats);
    var code = stats.slice(start, end+1);
    var originalCodeSize =  measureSize(code);
    var funcSize = measureSize(func);
    var outlineIndex = asmData.splitCounter++;
    var newIdent = func[1] + '$' + outlineIndex;
    // analyze variables, and find 'owned' variables - that only appear in the outlined code, and do not need any spill support
    var codeInfo = analyzeCode(func, asmData, code);
    var allCodeInfo = analyzeCode(func, asmData, func);
    var owned = { sp: 1 }; // sp is always owned, each has its own
    keys(setUnion(codeInfo.reads, codeInfo.writes)).forEach(function(v) {
      if (allCodeInfo.reads[v] === codeInfo.reads[v] && allCodeInfo.writes[v] === codeInfo.writes[v] && !(v in asmData.params)) {
        owned[v] = 1;
      }
    });
    var reps = [];
    // add spills
    function orderFunc(x, y) {
      return (asmData.stackPos[x] - asmData.stackPos[y]) || x.localeCompare(y);
    }
    var sortedReadsAndWrites = keys(setUnion(codeInfo.reads, codeInfo.writes)).sort(orderFunc);
    var sortedWrites = keys(codeInfo.writes).sort(orderFunc);
    sortedReadsAndWrites.forEach(function(v) {
      if (!(v in owned)) {
        reps.push(['stat', ['assign', true, ['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', 2]]], ['name', v]]]);
      }
    });
    // wipe out control variable
    reps.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', 0])]);
    reps.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlDataStackPos(outlineIndex)), ['num', 0])]); // XXX not really needed
    // do the call
    reps.push(['stat', ['call', ['name', newIdent], [['name', 'sp']]]]);
    // add unspills
    sortedWrites.forEach(function(v) {
      if (!(v in owned)) {
        reps.push(['stat', ['assign', true, ['name', v], makeAsmCoercion(['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', 2]]], getAsmType(v, asmData))]]);
      }
    });

    // Generate new function
    if (codeInfo.hasReturn || codeInfo.hasReturnType[ASM_INT] || codeInfo.hasReturnType[ASM_DOUBLE] || codeInfo.hasReturnType[ASM_FLOAT] || codeInfo.hasBreak || codeInfo.hasContinue) {
      // we need to capture all control flow using a top-level labeled one-time loop in the outlined function
      var breakCapturers = 0;
      var continueCapturers = 0;
      traverse(['block', code], function(node, type) { // traverse on dummy block, so we get the toplevel statements
        // replace all break/continue/returns with code to break out of the main one-time loop, and set the control data
        if (type in BREAK_CAPTURERS) {
          breakCapturers++;
        }
        if (type in CONTINUE_CAPTURERS) {
          continueCapturers++;
        }
        var stats = node === code ? node : getStatements(node);
        if (stats) {
          for (var i = 0; i < stats.length; i++) {
            var node = stats[i]; // step all over node and type here, for convenience
            if (node[0] == 'stat') node = node[1];
            var type = node[0];
            var ret = null;
            if (type == 'return') {
              ret = [];
              if (!node[1]) {
                ret.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', CONTROL_RETURN_VOID])]);
              } else {
                var type = detectType(node[1], asmData);
                ret.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', controlFromAsmType(type)])]);
                ret.push(['stat', makeAssign(makeStackAccess(type, asmData.controlDataStackPos(outlineIndex)), node[1])]);
              }
              ret.push(['stat', ['break', 'OL']]);
            } else if (type == 'break') {
              var label = node[1] || 0;
              if (label == 'OL') continue; // this was just added before us, it is new replacement code
              if (!label && breakCapturers > 0) continue; // no label, and captured
              if (label && (label in codeInfo.labels)) continue; // label, and defined in this code, so captured
              ret = [['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', label ? CONTROL_BREAK_LABEL : CONTROL_BREAK])]];
              if (label) {
                assert(label in codeInfo.breaks);
                ret.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlDataStackPos(outlineIndex)), ['num', codeInfo.breaks[label]])]);
              }
              ret.push(['stat', ['break', 'OL']]);
            } else if (type == 'continue') {
              var label = node[1] || 0;
              if (!label && continueCapturers > 0) continue; // no label, and captured
              if (label && (label in codeInfo.labels)) continue; // label, and defined in this code, so captured
              ret = [['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', label ? CONTROL_CONTINUE_LABEL : CONTROL_CONTINUE])]];
              if (label) {
                assert(label in codeInfo.continues);
                ret.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlDataStackPos(outlineIndex)), ['num', codeInfo.continues[label]])]);
              }
              ret.push(['stat', ['break', 'OL']]);
            }
            if (ret) {
              stats.splice.apply(stats, [i, 1].concat(ret));
              i += ret.length-1;
            }
          }
        }
      }, function(node, type) {
        if (type in BREAK_CAPTURERS) {
          breakCapturers--;
        }
        if (type in CONTINUE_CAPTURERS) {
          continueCapturers--;
        }
      });
      code = [['label', 'OL', ['do', ['num', 0], ['block', code]]]]; // do this after processing, to not confuse breakCapturers etc.
      // read the control data at the callsite to the outlined function, and clear the control values
      reps.push(['stat', makeAssign(
        ['name', 'tempValue'],
        makeAsmCoercion(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ASM_INT)
      )]);
      reps.push(['stat', makeAssign(
        ['name', 'tempInt'],
        makeAsmCoercion(makeStackAccess(ASM_INT, asmData.controlDataStackPos(outlineIndex)), ASM_INT)
      )]);
      reps.push(['stat', makeAssign(
        ['name', 'tempDouble'],
        makeAsmCoercion(makeStackAccess(ASM_DOUBLE, asmData.controlDataStackPos(outlineIndex)), ASM_DOUBLE)
      )]);
      reps.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlStackPos(outlineIndex)), ['num', 0])]);
      reps.push(['stat', makeAssign(makeStackAccess(ASM_INT, asmData.controlDataStackPos(outlineIndex)), ['num', 0])]); // XXX not really needed
      // use the control data information
      if (codeInfo.hasReturn) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_RETURN_VOID]),
          [['stat', ['return', null]]]
        ));
      }
      for (var returnType in codeInfo.hasReturnType) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', controlFromAsmType(returnType)]),
          [['stat', ['return', makeAsmCoercion(['name', returnType == ASM_INT ? 'tempInt' : 'tempDouble'], returnType | 0)]]]
        ));
      }
      if (codeInfo.hasBreak) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_BREAK]),
          [['stat', ['break', null]]]
        ));
        if (keys(codeInfo.breaks).length > 0) {
          reps.push(makeIf(
            makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_BREAK_LABEL]),
            [makeSwitch(makeAsmCoercion(['name', 'tempInt'], ASM_INT), keys(codeInfo.breaks).map(function(key) {
              var id = codeInfo.breaks[key];
              return [['num', id], [['block', [['stat', ['break', key]]]]]];
            }))]
          ));
        }
      }
      if (codeInfo.hasContinue) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_CONTINUE]),
          [['stat', ['continue', null]]]
        ));
        if (keys(codeInfo.continues).length > 0) {
          reps.push(makeIf(
            makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_CONTINUE_LABEL]),
            [makeSwitch(makeAsmCoercion(['name', 'tempInt'], ASM_INT), keys(codeInfo.continues).map(function(key) {
              var id = codeInfo.continues[key];
              return [['num', id], [['block', [['stat', ['continue', key]]]]]];
            }))]
          ));
        }
      }
    }
    // add spills and unspills in outlined code outside the OL loop
    sortedReadsAndWrites.reverse();
    sortedReadsAndWrites.forEach(function(v) {
      if (!(v in owned)) {
        code.unshift(['stat', ['assign', true, ['name', v], makeAsmCoercion(['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', 2]]], getAsmType(v, asmData))]]);
      }
    });
    sortedWrites.forEach(function(v) {
      if (!(v in owned)) {
        code.push(['stat', ['assign', true, ['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', 2]]], ['name', v]]]);
      }
    });
    // finalize
    var newFunc = ['defun', newIdent, ['sp'], code];
    var newAsmData = { params: { sp: ASM_INT }, vars: {}, inlines: asmData.inlines };
    for (var v in codeInfo.reads) {
      if (v != 'sp') newAsmData.vars[v] = getAsmType(v, asmData);
    }
    for (var v in codeInfo.writes) {
      assert(v != 'sp'); // we send sp as a read-only parameter, cannot be written to in outlined code
      newAsmData.vars[v] = getAsmType(v, asmData);
    }
    denormalizeAsm(newFunc, newAsmData);
    // add outline call markers (we cannot do later outlinings that cut through an outlining call)
    reps.unshift(['begin-outline-call', newIdent]);
    reps.push(['end-outline-call', newIdent]);
    // replace in stats
    stats.splice.apply(stats, [start, end-start+1].concat(reps));
    // final evaluation and processing
    if (!extraInfo.allowCostlyOutlines && (measureSize(func) >= funcSize || measureSize(newFunc) >= funcSize)) {
      //printErr('aborted outline attempt ' + [measureSize(func), measureSize(newFunc), ' one of which >= ', funcSize]);
      // abort, this was pointless
      stats.length = originalStats.length;
      for (var i = 0; i < stats.length; i++) stats[i] = originalStats[i];
      asmData.splitCounter--;
      return [];
    }
    maxTotalOutlinings--;
    for (var v in owned) {
      if (v != 'sp') delete asmData.vars[v]; // parent does not need these anymore
    }
    // if we just removed a final return from the original function, add one
    var last = getStatements(func)[getStatements(func).length-1];
    if (last[0] === 'stat') last = last[1];
    if (last[0] !== 'return') {
      for (var returnType in codeInfo.hasReturnType) {
        getStatements(func).push(['stat', ['return', makeAsmCoercion(['num', 0], returnType | 0)]]);
        break;
      }
    }
    outliningParents[newIdent] = func[1];
    //printErr('performed outline ' + [func[1], newIdent, 'pre size', originalCodeSize, 'resulting size', measureSize(code), 'overhead (w/r):', setSize(setSub(codeInfo.writes, owned)), setSize(setSub(codeInfo.reads, owned)), ' owned: ', setSize(owned), ' left: ', setSize(asmData.vars), setSize(asmData.params), ' loopsDepth: ', loops]);
    calculateThreshold(func, asmData);
    return [newFunc];
  }

  function outlineStatements(func, asmData, stats, maxSize) {
    asmData.maxAttemptedOutlinings--;
    level++;
    //printErr('outlineStatements: ' + [func[1], level, measureSize(func)]);
    var lastSize = measureSize(stats);
    if (lastSize < sizeToOutline) { level--; return }
    var ret = [];
    var sizeSeen = 0;
    var end = stats.length-1;
    var i = stats.length;
    var canRestart = false;
    var minIndex = 0;
    function calcMinIndex() {
      if (stats == getStatements(func)) {
        minIndex = getFirstIndexInNormalized(func, asmData);
        for (var i = minIndex; i < stats.length; i++) {
          var stat = stats[i];
          if (stat[0] == 'stat') stat = stat[1];
          if (stat[0] == 'assign' && stat[2][0] == 'name' && stat[2][1] == 'sp') {
            // cannot outline |sp = |
            minIndex = i+1;
            // When followed by a STACKTOP bump, preserve that too (we may need to replace it later)
            stat = stats[i+1];
            if (stat[0] == 'stat') stat = stat[1];
            if (stat && stat[0] == 'assign' && stat[2][0] == 'name' && stat[2][1] == 'STACKTOP') {
              minIndex = i+2;
            }
          }
        }
      }
    }
    function done() {
      return asmData.splitCounter >= asmData.maxOutlinings || measureSize(func) <= extraInfo.sizeToOutline || asmData.maxAttemptedOutlinings < 0;
    }
    while (1) {
      i--;
      calcMinIndex(); // TODO: optimize
      if (i < minIndex) {
        // we might be done. but, if we have just outlined, do a further attempt from the beginning.
        // (but only if the total costs are not extravagant)
        var currSize = measureSize(stats);
        var outlinedSize = measureSize(ret);
        if (canRestart && currSize > 1.2*sizeToOutline && lastSize - currSize >= 0.75*sizeToOutline) {
          //printErr('restarting ' + func[1] + ' since ' + [currSize, outlinedSize, lastSize] + ' in level ' + level);
          lastSize = currSize;
          i = stats.length;
          end = stats.length-1;
          sizeSeen = 0;
          canRestart = false;
          continue;
        } else {
          break;
        }
      }

      var stat = stats[i];
      while (stat[0] === 'end-outline-call') {
        // we cannot outline through an outline call, so include all of it
        while (stats[--i][0] !== 'begin-outline-call') {
          assert(i >= minIndex+1);
          assert(stats[i][0] !== 'end-outline-call');
        }
        stat = stats[i];
      }

      var size = measureSize(stat);
      //printErr(level + ' size          ' + [i, size]);
      if (size >= sizeToOutline) {
        // this by itself is big enough to inline, recurse into it and find statements to split on
        var subStatements = null;
        var pre = ret.length;
        traverse(stat, function(node, type) {
          if (type == 'block') {
            if (measureSize(node) >= sizeToOutline) {
              var subRet = outlineStatements(func, asmData, node[1], maxSize);
              if (subRet && subRet.length > 0) ret.push.apply(ret, subRet);
            }
            return null; // do not recurse into children, outlineStatements will do so if necessary
          } else if (type == 'while') {
            loops++;
          }
        }, function(node, type) {
          if (type == 'while') {
            loops--;
          }
        });
        if (ret.length > pre) {
          // we outlined recursively, reset our state here
          //printErr('successful outline in recursion ' + func[1] + ' due to recursive in level ' + level);
          if (done()) break;
          end = i-1;
          sizeSeen = 0;
          canRestart = true;
          continue;
        }
      }
      sizeSeen += size;
      // If this is big enough to outline, but not too big (if very close to the size of the full function,
      // outlining is pointless; remove stats from the end to try to achieve the good case), then outline.
      // Also, try to reduce the size if it is much larger than the hoped-for size
      while ((sizeSeen > maxSize || sizeSeen > 2*sizeToOutline) && end > i && stats[end][0] !== 'begin-outline-call' && stats[end][0] !== 'end-outline-call') {
        sizeSeen -= measureSize(stats[end]);
        if (sizeSeen >= sizeToOutline) {
          end--;
        } else {
          sizeSeen += measureSize(stats[end]); // abort, this took away too much
          break;
        }
      }
      // verify we are not outlining through an outline call
      var sum = 0;
      stats.slice(i, end+1).forEach(function(stat) {
        if (stat[0] == 'begin-outline-call') {
          assert(sum == 0);
          sum++;
        } else if (stat[0] == 'end-outline-call') {
          assert(sum == 1);
          sum--;
        }
      });
      assert(sum == 0);
      // final decision and action
      //printErr(' will try done working on sizeSeen due to ' + [(sizeSeen > maxSize || sizeSeen > 2*sizeToOutline), end > i , stats[end][0] !== 'begin-outline-call' , stats[end][0] !== 'end-outline-call'] + '  ...  ' + [sizeSeen, sizeToOutline, maxSize, sizeSeen >= sizeToOutline, sizeSeen <= maxSize]);
      if (sizeSeen >= sizeToOutline && sizeSeen <= maxSize) {
        assert(i >= minIndex);
        var newFuncs = doOutline(func, asmData, stats, i, end); // outline [i, .. ,end] inclusive
        if (newFuncs.length) {
          ret.push.apply(ret, newFuncs);
        }
        if (done()) break;
        sizeSeen = 0;
        end = i-1;
        canRestart = true;
        continue;
      }
    }
    level--;
    return ret;
  }

  //

  if (ast[0] !== 'toplevel') {
    assert(ast[0] == 'defun');
    ast = ['toplevel', [ast]];
  }

  var funcs = ast[1];

  var maxTotalFunctions = Infinity; // debugging tool

  //printErr('\n');

  var more = true;
  while (more) {
    more = false;

    var newFuncs = [];

    funcs.forEach(function(func) {
      vacuum(func); // clear out empty nodes that affect code size
      var asmData = normalizeAsm(func);
      var size = measureSize(func);
      if (size >= extraInfo.sizeToOutline && maxTotalFunctions > 0) {
        maxTotalFunctions--;
        aggressiveVariableEliminationInternal(func, asmData);
        deLabel(func, asmData);
        flatten(func, asmData);
        analyzeFunction(func, asmData);
        calculateThreshold(func, asmData);
        var stats = getStatements(func);
        var ret = outlineStatements(func, asmData, stats, 0.9*size);
        assert(level == 0);
        if (ret && ret.length > 0) {
          newFuncs.push.apply(newFuncs, ret);
          // We have outlined. Add stack support
          if ('sp' in asmData.vars) {
            // find stack bump (STACKTOP = STACKTOP + X | 0) and add the extra space
            var stackBumpNode = getStackBumpNode(stats);
            if (stackBumpNode) {
              stackBumpNode[3][2][3][1] = asmData.totalStackSize;
            } else {
              // sp exists, but no stack bump, so we need to add it
              var found = false;
              for (var i = 0; i < stats.length; i++) {
                var stat = stats[i];
                if (stat[0] === 'stat') stat = stat[1];
                if (stat[0] === 'assign' && stat[2][0] === 'name' && stat[2][1] === 'sp') {
                  var newNode = ['stat', makeAssign(['name', 'STACKTOP'], ['binary', '|', ['binary', '+', ['name', 'STACKTOP'], ['num', asmData.totalStackSize]], ['num', 0]])];
                  if (i+1 < stats.length) {
                    stats.splice(i+1, 0, newNode);
                  } else {
                    stats.push(newNode);
                  }
                  found = true;
                  break;
                }
              }
              assert(found);
            }
          } else if (!('sp' in asmData.params)) { // if sp is a param, then we are an outlined function, no need to add stack support for us
            // add sp variable and stack bump
            var index = getFirstIndexInNormalized(func, asmData);
            stats.splice(index, 0,
              ['stat', makeAssign(['name', 'sp'], ['name', 'STACKTOP'])],
              ['stat', makeAssign(['name', 'STACKTOP'], ['binary', '|', ['binary', '+', ['name', 'STACKTOP'], ['num', asmData.totalStackSize]], ['num', 0]])]
            );
            asmData.vars.sp = ASM_INT; // no need to add to vars, we are about to denormalize anyhow
            // we added sp, so we must add stack popping
            function makePop() {
              return ['stat', makeAssign(['name', 'STACKTOP'], ['name', 'sp'])];
            }
            traverse(func, function(node, type) {
              var stats = getStatements(node);
              if (!stats) return;
              for (var i = 0; i < stats.length; i++) {
                var subNode = stats[i];
                if (subNode[0] === 'stat') subNode = subNode[1];
                if (subNode[0] == 'return') {
                  stats.splice(i, 0, makePop());
                  i++;
                }
              }
            });
            // pop the stack at the end if there is not a return
            var last = stats[stats.length-1];
            if (last[0] === 'stat') last = last[1];
            if (last[0] !== 'return') {
              stats.push(makePop());
            }
          }
        }
        if (ret) {
          ret.push(func);
          //printErr('... resulting sizes of ' + func[1] + ' is ' + ret.map(measureSize) + '\n');
        }
      }
      denormalizeAsm(func, asmData);
    });

    funcs = null;

    // TODO: control flow: route returns and breaks. outlined code should have all breaks/continues/returns break into the outermost scope,
    //       after setting a state variable, etc.

    if (newFuncs.length > 0) {
      // add new functions to the toplevel, or create a toplevel if there isn't one
      ast[1].push.apply(ast[1], newFuncs);

      // TODO: check if in some cases we do need to outline new functions
      //funcs = newFuncs.filter(function(newFunc) {
      //  // recursively outline if we have a large new function that did not come at a high cost
      //  return measureSize(newFunc) > sizeToOutline && costs[newFunc[1]] < 0.1*sizeToOutline;
      //});
      //more = funcs.length > 0;
    }
  }

  // clear out markers
  traverse(ast, function(node, type) {
    if (type === 'begin-outline-call' || type === 'end-outline-call') return emptyNode();
  });
}

function fixPtr(ptr, heap) {
  switch (heap) {
    case 'HEAP8':   case 'HEAPU8': break;
    case 'HEAP16':  case 'HEAPU16': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 1) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 2]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAP32':  case 'HEAPU32': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 2) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 4]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAPF32': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 2) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 4]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAPF64': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 3) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 8]]; // was unshifted, convert to absolute address
      }
      break;
    }
    default: {
      return ptr; // unchanged
    }
  }
  ptr = ['binary', '|', ptr, ['num', 0]];
  return ptr;
}

function safeHeap(ast) {
  var SAFE_HEAP_FUNCS = set('SAFE_HEAP_LOAD', 'SAFE_HEAP_LOAD_D', 'SAFE_HEAP_STORE', 'SAFE_HEAP_STORE_D', 'SAFE_FT_MASK');
  traverseGeneratedFunctions(ast, function(func) {
    if (func[1] in SAFE_HEAP_FUNCS) return null;
    traverseGenerated(func, function(node, type) {
      if (type === 'assign') {
        if (node[1] === true && node[2][0] === 'sub') {
          var heap = node[2][1][1];
          var ptr = fixPtr(node[2][2], heap);
          var value = node[3];
          // SAFE_HEAP_STORE(ptr, value, bytes, isFloat) 
          switch (heap) {
            case 'HEAP8':   case 'HEAPU8': {
              return ['call', ['name', 'SAFE_HEAP_STORE'], [ptr, makeAsmCoercion(value, ASM_INT), ['num', 1]]];
            }
            case 'HEAP16':  case 'HEAPU16': {
              return ['call', ['name', 'SAFE_HEAP_STORE'], [ptr, makeAsmCoercion(value, ASM_INT), ['num', 2]]];
            }
            case 'HEAP32':  case 'HEAPU32': {
              return ['call', ['name', 'SAFE_HEAP_STORE'], [ptr, makeAsmCoercion(value, ASM_INT), ['num', 4]]];
            }
            case 'HEAPF32': {
              return ['call', ['name', 'SAFE_HEAP_STORE_D'], [ptr, makeAsmCoercion(value, ASM_DOUBLE), ['num', 4]]];
            }
            case 'HEAPF64': {
              return ['call', ['name', 'SAFE_HEAP_STORE_D'], [ptr, makeAsmCoercion(value, ASM_DOUBLE), ['num', 8]]];
            }
            default: throw 'bad heap ' + heap;
          }
        }
      } else if (type === 'sub') {
        var target = node[1][1];
        if (target[0] === 'H') {
          // heap access
          var heap = target;
          var ptr = fixPtr(node[2], heap);
          // SAFE_HEAP_LOAD(ptr, bytes, isFloat) 
          switch (heap) {
            case 'HEAP8': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 1], ['num', 0]]], ASM_INT);
            }
            case 'HEAPU8': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 1], ['num', 1]]], ASM_INT);
            }
            case 'HEAP16': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 2], ['num', 0]]], ASM_INT);
            }
            case 'HEAPU16': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 2], ['num', 1]]], ASM_INT);
            }
            case 'HEAP32': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 4], ['num', 0]]], ASM_INT);
            }
            case 'HEAPU32': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD'], [ptr, ['num', 4], ['num', 1]]], ASM_INT);
            }
            case 'HEAPF32': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD_D'], [ptr, ['num', 4]]], ASM_DOUBLE);
            }
            case 'HEAPF64': {
              return makeAsmCoercion(['call', ['name', 'SAFE_HEAP_LOAD_D'], [ptr, ['num', 8]]], ASM_DOUBLE);
            }
            default: throw 'bad heap ' + heap;
          }
        } else {
          assert(target[0] == 'F');
          // function table indexing mask
          assert(node[2][0] === 'binary' && node[2][1] === '&');
          node[2][2] = makeAsmCoercion(['call', ['name', 'SAFE_FT_MASK'], [makeAsmCoercion(node[2][2], ASM_INT), makeAsmCoercion(node[2][3], ASM_INT)]], ASM_INT);
        }
      }
    });
  });
}

function fixPtrSlim(ptr, heap, shell) {
  switch (heap) {
    case 'HEAP8':   case 'HEAPU8': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 0) {
        ptr = ['binary', '|', ptr[2], ['num', 0]]; // smaller
      }
      break;
    }
    case 'HEAP16':  case 'HEAPU16': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 1) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 2]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAP32':  case 'HEAPU32': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 2) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 4]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAPF32': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 2) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 4]]; // was unshifted, convert to absolute address
      }
      break;
    }
    case 'HEAPF64': {
      if (ptr[0] === 'binary' && ptr[1] === '>>' && ptr[3][0] === 'num' && ptr[3][1] === 3) {
        ptr = ptr[2]; // skip the shift
      } else {
        ptr = ['binary', '*', ptr, ['num', 8]]; // was unshifted, convert to absolute address
      }
      break;
    }
    default: {
      if (!shell) throw 'bad heap ' + heap;
      return ptr; // unchanged
    }
  }
  return ptr;
}

function splitMemory(ast, shell) {
  traverse(ast, function(node, type) {
    if (type === 'assign') {
      if (node[2][0] === 'sub' && node[2][1][0] === 'name') {
        var heap = node[2][1][1];
        if (parseHeap(heap)) {
          if (node[1] !== true) assert(0, 'bad assign, split memory cannot handle ' + JSON.stringify(node) + '= to a HEAP');
          var ptr = fixPtrSlim(node[2][2], heap, shell);
          var value = node[3];
          switch (heap) {
            case 'HEAP8': return ['call', ['name', 'set8'], [ptr, value]];
            case 'HEAP16': return ['call', ['name', 'set16'], [ptr, value]];
            case 'HEAP32': return ['call', ['name', 'set32'], [ptr, value]];
            case 'HEAPU8': return ['call', ['name', 'setU8'], [ptr, value]];
            case 'HEAPU16': return ['call', ['name', 'setU16'], [ptr, value]];
            case 'HEAPU32': return ['call', ['name', 'setU32'], [ptr, value]];
            case 'HEAPF32': return ['call', ['name', 'setF32'], [ptr, value]];
            case 'HEAPF64': return ['call', ['name', 'setF64'], [ptr, value]];
            default: if (!shell) throw 'bad heap ' + heap;
          }
        }
      }
    } else if (type === 'sub') {
      var target = node[1][1];
      if (target[0] === 'H') {
        // heap access
        var heap = target;
        var ptr = fixPtrSlim(node[2], heap, shell);
        switch (heap) {
          case 'HEAP8': return ['call', ['name', 'get8'], [ptr]];
          case 'HEAP16': return ['call', ['name', 'get16'], [ptr]];
          case 'HEAP32': return ['call', ['name', 'get32'], [ptr]];
          case 'HEAPU8': return ['call', ['name', 'getU8'], [ptr]];
          case 'HEAPU16': return ['call', ['name', 'getU16'], [ptr]];
          case 'HEAPU32': return ['call', ['name', 'getU32'], [ptr]];
          case 'HEAPF32': return ['call', ['name', 'getF32'], [ptr]];
          case 'HEAPF64': return ['call', ['name', 'getF64'], [ptr]];
          default: if (!shell) throw 'bad heap ' + heap;
        }
      }
    }
  });
  var SPLIT_GETS = set('get8', 'get16', 'get32', 'getU8', 'getU16', 'getU32', 'getF32', 'getF64');
  traverse(ast, function(node, type) {
    if (type === 'binary' && node[1] === '|' && node[2][0] === 'call' && node[2][1][0] === 'name' && node[2][1][1] in SPLIT_GETS && node[3][0] === 'num' && node[3][1] === 0) {
      return node[2];
    } else if (type === 'unary-prefix' && node[1] === '+' && node[2][0] === 'call' && node[2][1][0] === 'name' && node[2][1][1] in SPLIT_GETS) {
      return node[2];
    }
  });
}

function splitMemoryShell(ast) {
  splitMemory(ast, true);
}

function optimizeFrounds(ast) {
  // collapse fround(fround(..)), which can happen due to elimination
  // also emit f0 instead of fround(0) (except in returns)
  var inReturn = false;
  function fix(node) {
    if (node[0] === 'return') inReturn = true;
    traverseChildren(node, fix);
    if (node[0] === 'return') inReturn = false;
    if (node[0] === 'call' && node[1][0] === 'name' && node[1][1] === 'Math_fround') {
      var arg = node[2][0];
      if (arg[0] === 'num') {
        if (!inReturn && arg[1] === 0) return ['name', 'f0'];
      } else if (arg[0] === 'call' && arg[1][0] === 'name' && arg[1][1] === 'Math_fround') {
        return arg;
      }
    }
  }
  traverseChildren(ast, fix);
}

// Ensures that if label exists, it is assigned an initial value (to not assume the asm declaration has an effect, which we normally do not)
function ensureLabelSet(ast) {
  assert(asm);
  traverseGeneratedFunctions(ast, function(func) {
    var asmData = normalizeAsm(func);
    if ('label' in asmData.vars) {
      var stats = getStatements(func);
      for (var i = 0; i < stats.length; i++) {
        var node = stats[i];
        if (node[0] === 'stat') node = node[1];
        if (node[0] === 'assign' && node[2][0] === 'name' && node[2][1] === 'label') {
          break; // all good
        }
        if (node[0] === 'label' || (node[0] in CONTROL_FLOW)) {
          // we haven't seen an assign, and we hit control flow. add an assign
          stats.splice(i, 0, ['stat', ['assign', true, ['name', 'label'], ['num', 0]]]);
          break;
        }
      }
    }
    denormalizeAsm(func, asmData);
  });
}

// finds vars that may not be assigned to. misses out on vars that are assigned to in all branches of an if etc. TODO: optimize
function findUninitializedVars(func, asmData) {
  var bad = {};
  function definitely(node, uninitialized) {
    if (!node) return;
    if (node[0] === 'assign' && node[2][0] === 'name') {
      var name = node[2][1];
      if (name in uninitialized) {
        delete uninitialized[name]; // this one is now ok
      }
    } else if (node[0] === 'name') {
      var name = node[1];
      if (name in uninitialized) {
        bad[name] = 1;
        delete uninitialized[name]; // this one is now bad, ignore it
      }
    }
    traverseChildrenInExecutionOrder(node, definitely, maybe, uninitialized);
  }
  function maybe(node, uninitialized) {
    uninitialized = copy(uninitialized); // copy it; changes here will not propagate
    traverseChildrenInExecutionOrder(node, definitely, maybe, uninitialized);
  }
  traverseChildrenInExecutionOrder(func, definitely, maybe, copy(asmData.vars));
  return bad;
}

function trample(x, y) { // x = y, by trampling it
  for (var i = 0; i < y.length; i++) {
    x[i] = y[i];
  }
  x.length = y.length;
}

function ilog2(x) {
  x = Math.round(x);
  if (x === 1) return 0;
  if (x === 2) return 1;
  if (x === 4) return 2;
  if (x === 8) return 3;
  throw 'ilog2 is not smart enough for ' + x;
}

// Converts functions into binary format to be run by an emterpreter
function emterpretify(ast) {
  emitAst = false;

  var EMTERPRETED_FUNCS = set(extraInfo.emterpretedFuncs);
  var EXTERNAL_EMTERPRETED_FUNCS = set(extraInfo.externalEmterpretedFuncs);
  var OPCODES = extraInfo.opcodes;
  var ROPCODES = extraInfo.ropcodes;
  var ASYNC = extraInfo.ASYNC;
  var PROFILING = extraInfo.PROFILING;
  var ASSERTIONS = extraInfo.ASSERTIONS;

  var RELATIVE_BRANCHES = set('BR', 'BRT', 'BRF');
  var ABSOLUTE_BRANCHES = set('BRA', 'BRTA', 'BRFA');
  var BRANCHES = {};
  mergeInto(BRANCHES, RELATIVE_BRANCHES);
  mergeInto(BRANCHES, ABSOLUTE_BRANCHES);
  var UNCONDITIONAL_BRANCHES = set('BR', 'BRA');
  var CONDITION_BRFS = set('LNOTBRF', 'EQBRF', 'NEBRF', 'SLTBRF', 'ULTBRF', 'SLEBRF', 'ULEBRF');

  var COMPARISONS = set('LNOT', 'EQ', 'NE', 'SLT', 'ULT', 'SLE', 'ULE');

  var FAST_LOCALS = 200; // any local over this will be copied to a fast local first. hopefully,
                         // fast local + temp variables end up less than 256, because that's all we can do!

  var tempBuffer = new ArrayBuffer(8);
  var tempFloat64 = new Float64Array(tempBuffer);
  var tempFloat32 = new Float32Array(tempBuffer);
  var tempUint8 = new Uint8Array(tempBuffer);
  function flattenFloat32(value) {
    tempFloat32[0] = value;
    return Array.prototype.slice.call(tempUint8, 0, 4);
  }
  function flattenFloat64(value) {
    tempFloat64[0] = value;
    return Array.prototype.slice.call(tempUint8, 0, 8);
  }

  // functions which are ok to run while async, even if not emterpreted
  var OK_TO_CALL_WHILE_ASYNC = set('stackSave', 'stackRestore', 'stackAlloc', 'setThrew', '_memset', '_memcpy', '_memmove', '_strlen', '_strncpy', '_strcpy', '_strcat', 'SAFE_HEAP_LOAD', 'SAFE_HEAP_STORE', 'SAFE_FT_MASK');
  function okToCallWhileAsync(name) {
    // dynCall *can* be on the stack, they are just bridges; what matters is where they go
    if (/^dynCall_/.test(name)) return true;
    if (name in OK_TO_CALL_WHILE_ASYNC) return true;
    return false;
  }

  function verifyCode(code, stat) {
    if (code.length % 4 !== 0) assert(0, JSON.stringify(code));
    var len = code.length;
    for (var i = 0; i < len; i++) {
      if (typeof code[i] !== 'string' && typeof code[i] !== 'number' && !(typeof code[i] === 'object' && code[i].what)) {
        assert(0, i + ' : ' + JSON.stringify(code) + ' from ' + JSON.stringify(stat));
      }
    }
  }

  function walkFunction(func) {
    if (func[1] === 'emterpret') {
      // we will replace the stand-in, do not emit anything for it here
      func[0] = 'toplevel';
      func[1] = [];
      return;
    }

    var freeLocals = [];
    var maxLocal = 0;

    // gets a 'free' local. this notices the maximum local used, which is then the size of out stack
    // 'free' locals are ones above the set of actual local vars in the asm.js method.
    // you *must* free that local by calling releaseFree on it, which implies you must call
    // releaseIfFree on anything returned by getReg
    // if possible1 or possible2 are passed in, and they are free, they will be reused. this should
    // be done when they are free variables in use right now, but will become free in time to become
    // the free variable we need here (e.g. y = y + z, no need for a new x to assign into). this can
    // then be passed as a second argument to releaseFree.
    function getFree(possible1, possible2) {
      if (possible1 >= numLocals) return possible1; // (undefined >= any number is false)
      if (possible2 >= numLocals) return possible2;
      assert(freeLocals.length > 0);
      var ret = freeLocals.pop();
      maxLocal = Math.max(maxLocal, ret);
      assert(ret >= numLocals);
      //printErr('get free  ' + ret);
      return ret;
    }
    // if possible is passed in, and is identical to l, then it means l was reused, and we must not free it
    function releaseFree(l, possible) {
      if (l === possible) return;
      //printErr('free free ' + l);
      assert(freeLocals.indexOf(l) < 0);
      assert(l >= numLocals && l <= maxLocal);
      freeLocals.push(l);
      return l;
    }
    function unreleaseFree(l) {
      assert(l >= numLocals && l <= maxLocal);
      var i = freeLocals.indexOf(l);
      assert(l >= 0);
      freeLocals.splice(i, 1);
    }

    function isFree(l) {
      return l >= numLocals;
    }
    function getFreeOrAssignTo(assignTo) {
      return assignTo >= 0 ? assignTo : getFree();
    }

    var markerId = 0;
    function getMarker(name) {
      return { what: 'marker', name: name, relativeUses: 0, absoluteUses: 0, id: markerId++, i: 0 };
    }

    var absoluteTargets = {};

    var breakStack = [];
    var continueStack = [];
    var breakLabels = {};
    var continueLabels = {};

    // returns [l, bytecode] where l is a local register, and bytecode is bytecode to generate it.
    // if dropIt is provided, then the output of this can just be dropped.
    // if assignTo is provided, then it is where we can assign to, instead of allocating a free reg for that purpose.
    // you *must* call releaseIfFree on the l that is returned; if it is a free local, that will free it.
    function getReg(node, dropIt, typeHint, signHint, assignTo) {
      //printErr('getReg ' + JSON.stringify(node) + ' : ' + astToSrc(node) + ' : ' + [dropIt, typeHint, signHint, assignTo]);
      switch(node[0]) {
        case 'name': {
          var name = node[1];
          if (name in locals) {
            var l = locals[name];
            if (l < FAST_LOCALS) {
              return [l, []];
            } else {
              assert(l < 65535);
              var t = getFree();
              var type = getAsmType(name, asmData);
              var op;
              switch (type) {
                case ASM_INT:    op = 'FSLOW'; break;
                case ASM_DOUBLE: op = 'FSLOWD'; break;
                default: throw 'bad';
              }
              return [t, [op, t, l & 255, l >>> 8]];
            }
          }
          // this is a global
          switch(name) {
            case 'STACKTOP': {
              var x = getFree();
              return [x, ['GETST', x, 0, 0]];
            }
            case 'tempDoublePtr': {
              var x = getFree();
              return [x, ['GETTDP', x, 0, 0]];
            }
            case 'tempRet0': {
              var x = getFree();
              return [x, ['GETTR0', x, 0, 0]];
            }
            case 'inf': return makeNum(Infinity, ASM_DOUBLE);
            case 'nan': return makeNum(NaN, ASM_DOUBLE);
            case 'debugger': return [-1, []]; // nothing to do here (should we?)
            default: {
              var x = getFree();
              // We actually do not know the type here, and even hints won't help for global1 = global2. We swap GETGLBI to D in emterpretify.py as needed.
              return [x, ['GETGLBI', x, name, 0]];
            }
          }
        }
        case 'num': {
          return makeNum(node[1], ASM_INT, assignTo);
        }
        case 'var':
        case 'toplevel': {
          assert(dropIt || isEmptyNode(node));
          return [-1, []]; // empty node
        }
        case 'stat': return getReg(node[1], dropIt);
        case 'assign': {
          assert(node[1] === true);
          var target = node[2];
          var value = node[3];
          if (target[0] === 'name') {
            // assign to a local or a global
            var name = target[1];
            if (name in locals) {
              // local
              var l = locals[name];
              var type = getAsmType(name, asmData);
              if (l < FAST_LOCALS) {
                var reg = getReg(value, undefined, type, undefined, l);
                // TODO: detect when the last operation in reg[1] assigns in its arg x, in which case we can avoid the SET and make it assign to us
                reg[1] = reg[1].concat(makeSet(l, releaseIfFree(reg[0]), type));
                return [l, reg[1]];
              } else {
                assert(l < 65535);
                var t = getFree();
                var type = getAsmType(name, asmData);
                var op;
                switch (type) {
                  case ASM_INT:    op = 'TSLOW'; break;
                  case ASM_DOUBLE: op = 'TSLOWD'; break;
                  default: throw 'bad';
                }
                var reg = getReg(value, undefined, type, undefined, t);
                reg[1] = reg[1].concat(makeSet(t, releaseIfFree(reg[0], t), type))
                               .concat([op, t, l & 255, l >>> 8]);
                return [t, reg[1]];
              }
            } else {
              var reg = getReg(value, undefined, undefined, undefined, assignTo);
              var opcode;
              switch(name) {
                case 'STACKTOP': opcode = 'SETST'; break;
                case 'tempRet0': opcode = 'SETTR0'; break;
                default: {
                  var type = detectType(value, asmData);
                  assert(type === ASM_INT || type === ASM_DOUBLE);
                  reg[1].push(type === ASM_INT ? 'SETGLBI' : 'SETGLBD', name, 0, reg[0]);
                  return reg; // caller will free reg[0] if necessary
                }
              }
              reg[1].push(opcode, reg[0], 0, 0); // caller will free reg[0] if necessary
              return reg;
            }
          } else if (target[0] === 'sub') {
            assert(dropIt);
            // assign to memory
            var heap = target[1][1];
            var temp = makeTempParseHeap();
            assert(parseHeap(heap, temp));
            // coerced heap access => a load
            var opcode = 'STORE' + (temp.float ? 'F' : '') + temp.bits;
            if (target[2][0] === 'binary' && target[2][1] === '>>' && target[2][3][0] === 'num') {
              var shifts = target[2][3][1];
              assert(shifts >= 0 && shifts <= 3);
              var bits = Math.pow(2, shifts)*8;
              assert(bits === temp.bits);
              var pointer = target[2][2];
              var x = getReg(pointer, false, ASM_INT, ASM_SIGNED);
              var xLast = x[1][x[1].length-4];
              if (xLast === 'ADD' || xLast === 'ADDV') {
                var v = xLast === 'ADDV';
                // optimized store + add
                var curr = x[1].slice(x[1].length-4);
                x[1].splice(x[1].length-4, 4);
                if (curr[2] !== x[0]) unreleaseIfFree(curr[2]); // make sure these are kept alive during z's operations, we are
                if (!v && curr[3] !== x[0]) unreleaseIfFree(curr[3]); // putting code in between their definition and use
                var z = getReg(value);
                if (x[0] !== curr[2] && (v || x[0] !== curr[3])) releaseIfFree(x[0]);
                curr = [opcode + 'A' + (v ? 'V' : ''), releaseIfFree(curr[2]), !v ? releaseIfFree(curr[3]) : curr[3], releaseIfFree(z[0])];
                var ret = x[1].concat(z[1]).concat(curr);
                return [-1, ret];
              } else if (value[0] === 'sub' && value[1][0] === 'name' && value[1][1] === heap && value[2][0] === 'binary' && value[2][1] === '>>') {
                // a copy: store the result of a load, identical heap, identical shifts
                assert(value[2][3][0] === 'num' && value[2][3][1] === shifts);
                opcode += 'C';
                var y = getReg(value[2][2]);
                var ret = x[1].concat(y[1]);
                ret.push(opcode, releaseIfFree(x[0]), releaseIfFree(y[0]), 0);
                return [-1, ret];
              } else {
                var y = getReg(value); // generate y now, not earlier, to not trample x's output reg, which might be a temp
                var ret = x[1].concat(y[1]);
                ret.push(opcode, releaseIfFree(x[0]), releaseIfFree(y[0]), 0);
                return [-1, ret];
              }
            } else {
              assert(target[2][0] === 'num'); // HEAP32[8] or such
              var address = target[2][1];
              var shifts = ilog2(temp.bits/8);
              assert(address === ((address << shifts) >> shifts));
              var x = makeNum(address << shifts, ASM_INT);
              var y = getReg(value);
              y[1].push(opcode, releaseIfFree(x[0]), releaseIfFree(y[0]), 0);
              return [-1, x[1].concat(y[1])];
            }
          } else throw 'assign wha? ' + target[0];
        }
        case 'binary': {
          if (node[1] === '|' && node[3][0] === 'num' && node[3][1] === 0) {
            // signed-coerced operation
            return getReg(node[2], dropIt, ASM_INT, ASM_SIGNED, assignTo);
          } else if (node[1] === '>>>' && node[3][0] === 'num' && node[3][1] === 0) {
            // unsigned-coerced operation
            return getReg(node[2], dropIt, ASM_INT, ASM_UNSIGNED, assignTo);
          }

          // not a simple coercion

          if (dropIt) {
            // a pointless thing we can drop entirely
            var ret = [-1, []];
            if (hasSideEffects(node)) {
              // something here has side effects, emit it but drop the result
              if (hasSideEffects(node[2])) {
                var left = getReg(node[2]);
                releaseIfFree(left[0]);
                ret[1] = left[1];
              }
              if (hasSideEffects(node[3])) {
                var right = getReg(node[3]);
                releaseIfFree(right[0]);
                ret[1] = ret[1].concat(right[1]);
              }
            }
            return ret;
          }

          switch (node[1]) {
            case '&': case '|': case '^': case '<<': case '>>': case '>>>': return makeBinary(node, ASM_INT, ASM_SIGNED);
            case '>=': case '>':
            case '+': case '-': case '*': case '/': case '%': case '<': case '<=': case '==': case '!=': {
              var type = getCombinedType(node[2], node[3], asmData, typeHint);
              var sign = getCombinedSign(node[2], node[3], signHint);
              if (node[1] === '>=' || node[1] === '>') {
                if (type === ASM_INT) { // float/double comparisons are not antisymmetrical due to NaNs
                  var temp = node[2];
                  node[2] = node[3];
                  node[3] = temp;
                  node[1] = node[1] === '>=' ? '<=' : '<';
                }
              }
              return makeBinary(node, type, sign, assignTo);
            }
            default: throw 'ehh';
          }
          throw 'todo';
        }
        case 'unary-prefix': {
          if (node[1] === '+') {
            // double operation
            var appliedCoercion = false;
            var ret = (function() {
              var inner = node[2];
              switch (inner[0]) {
                case 'call': case 'sub': {
                  // the coercion is part of the syntax of these
                  appliedCoercion = true;
                  return getReg(inner, dropIt, ASM_DOUBLE, ASM_NONSIGNED, assignTo);
                }
                case 'num': {
                  appliedCoercion = true;
                  return makeNum(inner[1], ASM_DOUBLE, assignTo);
                }
                case 'unary-prefix': {
                  if (inner[1] === '-' && inner[2][0] === 'num') {
                    appliedCoercion = true;
                    return makeNum(-inner[2][1], ASM_DOUBLE, assignTo);
                  }
                  // otherwise fall through
                }
                default: {
                  return getReg(inner, dropIt, ASM_DOUBLE, ASM_NONSIGNED, assignTo);
                }
              }
            })();
            // add a coercion on the value, if needed
            if (!appliedCoercion) {
              var innerType = detectType(node[2], asmData);
              if (innerType !== ASM_DOUBLE) {
                if (innerType === ASM_INT) {
                  var sign = detectSign(node[2]);
                  var opcode = sign === ASM_SIGNED ? 'SI2D' : 'UI2D';
                  if (isFree(ret[0])) {
                    ret[1].push(opcode, ret[0], ret[0], 0);
                  } else {
                    // we can't trample this reg
                    var l = getFree();
                    ret[1].push(opcode, l, ret[0], 0);                   
                    ret[0] = l;
                  }
                } else {
                  throw 'whoops';
                }
              }
            }
            return ret;
          } else if (node[1] === '~' && node[2][0] === 'unary-prefix' && node[2][1] === '~') {
            return makeUnary(['unary-prefix', 'D2I', node[2][2]], ASM_INT, ASM_SIGNED, assignTo);
          }

          // not a simple coercion

          if (dropIt) {
            // a pointless thing we can drop entirely
            var ret = [-1, []];
            if (hasSideEffects(node)) {
              // emit it but drop the result
              var child = getReg(node[2]);
              releaseIfFree(child[0]);
              ret[1] = child[1];
            }
            return ret;
          }

          switch (node[1]) {
            case '-': {
              if (node[2][0] === 'num') {
                return makeNum(-node[2][1], ASM_INT, assignTo);
              }
              // otherwise fall through
            }
            case '~': {
              var type = detectType(node[2], asmData);
              return makeUnary(node, type, ASM_SIGNED, assignTo);
            }
            case '!': return makeUnary(node, ASM_INT, ASM_SIGNED, assignTo);
            default: throw 'ehh';
          }
          throw 'todo';
        }
        case 'call': {
          var type;
          var ret;
          if (dropIt && (typeHint === undefined || typeHint === ASM_NONE)) {
            type = ASM_NONE;
            ret = getFreeOrAssignTo(assignTo); // get a register that is ok to write to, even if no return value, to simplify ABI XXX
          } else {
            assert(typeHint !== ASM_NONE);
            type = typeHint;
            ret = getFreeOrAssignTo(assignTo);
          }
          return [ret, makeCall(ret, node, type)];
        }
        case 'return': {
          assert(dropIt);
          var value = node[1];
          var reg;
          if (value) reg = getReg(value);
          else reg = [-1, []];
          reg[1].push('RET', value ? releaseIfFree(reg[0]) : 0, 0, 0);
          return [-1, reg[1]];
        }
        case 'do': {
          return makeDo(node);
        }
        case 'while': {
          return makeWhile(node);
        }
        case 'label': {
          var name = node[1];
          var inner = node[2];
          assert(name);
          if (inner[0] === 'do') {
            return makeDo(inner, name);
          } else if (inner[0] === 'while') {
            return makeWhile(inner, name);
          } else if (inner[0] === 'switch') {
            return makeSwitch(inner, name);
          } else if (inner[0] === 'block') {
            return makeDo(['do', ['num', 0], inner], name);
          }
          throw 'sigh ' + inner[0];
        }
        case 'break': {
          var label = node[1];
          if (!label) {
            assert(breakStack.length > 0);
            return [-1, ['BR', 0, breakStack[breakStack.length-1], 0]];
          }
          assert(label in breakLabels);
          return [-1, ['BR', 0, breakLabels[label], 0]];
        }
        case 'continue': {
          var label = node[1];
          if (!label) {
            assert(continueStack.length > 0);
            return [-1, ['BR', 0, continueStack[continueStack.length-1], 0]];
          }
          assert(label in continueLabels);
          return [-1, ['BR', 0, continueLabels[label], 0]];
        }
        case 'if': {
          var exit = getMarker('if-exit');
          var ret;
          if (!node[3]) {
            ret = makeBranchIfFalse(node[1], exit).concat(walkStatements(node[2]));
          } else {
            var otherwise = getMarker('if-else');
            ret = makeBranchIfFalse(node[1], otherwise).concat(walkStatements(node[2]));
            ret.push('BR', 0, exit, 0, 'marker', otherwise, 0, 0);
            ret = ret.concat(walkStatements(node[3]));
          }
          ret.push('marker', exit, 0, 0);
          return [-1, ret];
        }
        case 'conditional': {
          // TODO: handle dropIt
          var type = detectType(node[2], asmData);
          if ((node[2][0] === 'name' || getNum(node[2]) !== null) && 
              (node[3][0] === 'name' || getNum(node[3]) !== null)) {
            // this is a simple choice between concrete values, no need for control flow here
            var out = assignTo >= 0 ? assignTo : getFree();
            var condition = getReg(node[1]);
            var ifTrue = getReg(node[2]);
            var ifFalse = getReg(node[3]);
            return [out, condition[1].concat(ifTrue[1]).concat(ifFalse[1]).concat(
              [type === ASM_INT ? 'COND' : 'CONDD', out, releaseIfFree(condition[0]), releaseIfFree(ifTrue[0]), releaseIfFree(ifFalse[0]), 0, 0, 0]
            )];
          }
          var otherwise = getMarker('cond-else'), exit = getMarker('cond-exit');
          var temp = getFree();
          assert(type !== ASM_NONE);
          var ret = makeBranchIfFalse(node[1], otherwise);
          var first = getReg(node[2]);
          ret = ret.concat(first[1]).concat(makeSet(temp, releaseIfFree(first[0]), type)); 
          ret.push('BR', 0, exit, 0);
          var second = getReg(node[3]);
          ret.push('marker', otherwise, 0, 0);
          ret = ret.concat(second[1]).concat(makeSet(temp, releaseIfFree(second[0]), type));
          ret.push('marker', exit, 0, 0);
          return [temp, ret];
        }
        case 'seq': {
          var first = getReg(node[1], true); // first output is always dropped
          releaseIfFree(first[0]);
          var second = getReg(node[2], dropIt, undefined, undefined, assignTo); // second output might be dropped
          return [second[0], first[1].concat(second[1])];
        }
        case 'sub': {
          assert(node[1][0] === 'name');
          var heap = node[1][1];
          var temp = makeTempParseHeap();
          assert(parseHeap(heap, temp));
          // coerced heap access => a load
          var opcode = 'LOAD' + (temp.float ? 'F' : (temp.bits < 32 && temp.unsigned ? 'U' : '')) + temp.bits;
          if (node[2][0] === 'binary' && node[2][1] === '>>' && node[2][3][0] === 'num') {
            var shifts = node[2][3][1];
            assert(shifts >= 0 && shifts <= 3);
            var bits = Math.pow(2, shifts)*8;
            assert(bits === temp.bits);
            var pointer = node[2][2];
            var y = getReg(pointer, false, ASM_INT, ASM_SIGNED);
            var yLast = y[1][y[1].length-4];
            if (yLast === 'ADD' || yLast === 'ADDV') {
              // optimized load + add
              y[1][y[1].length-4] = opcode + 'A' + (yLast === 'ADDV' ? 'V' : '');
              if (assignTo >= 0) {
                releaseIfFree(y[0]);
                y[1][y[1].length-3] = assignTo;
                y[0] = assignTo;
              }
              return y;
            } else {
              var x = assignTo >= 0 ? assignTo : getFree(y[0]);
              y[1].push(opcode, x, releaseIfFree(y[0], x), 0);
              y[0] = x;
              return y;
            }
          } else {
            assert(node[2][0] === 'num'); // HEAP32[8] or such
            var address = node[2][1];
            var shifts = ilog2(temp.bits/8);
            assert(address === ((address << shifts) >> shifts));
            var ret = makeNum(address << shifts, ASM_INT);
            var out = assignTo >= 0 ? assignTo : getFree(ret[0]);
            ret[1].push(opcode, out, releaseIfFree(ret[0], out), 0);
            ret[0] = out;
            return ret;
          }
        }
        case 'block': {
          return [-1, walkStatements(node[1])];
        }
        case 'switch': {
          return makeSwitch(node);
        }
        default: throw 'getReg wha? ' + node[0] + new Error().stack;
      }
    }
    function releaseIfFree(l, possible) {
      if (l >= numLocals) releaseFree(l, possible);
      return l;
    }
    function unreleaseIfFree(l) {
      if (l >= numLocals) unreleaseFree(l);
    }

    function makeSet(dst, src, type) {
      assert(dst < 256 && src < 256);
      if (dst === src) return [];
      var opcode;
      if (type === ASM_INT) {
        opcode = 'SET';
      } else if (type === ASM_DOUBLE) {
        opcode = 'SETD';
      } else assert(0, 'ick ' + type);
      return [opcode, dst, src, 0];
    }

    var hoistedNums = {};

    function makeNum(value, type, l) {
      if (type === ASM_INT && l === undefined && value in hoistedNums) return [hoistedNums[value], []];
      if (l === undefined) l = getFree();
      var opcode;
      if (((value << 16) >> 16) === (value | 0) && ((value === (value | 0)) || (type === ASM_INT && value === (value >>> 0))) &&
          (value !== 0 || 1/value > 0)) {
        // a small 16-bit integer value, and not negative zero
        // note that for ints, we don't care if it is signed or not; for floating-point, we need this to be signed
        if (type === ASM_INT) {
          opcode = 'SETVI';
        } else if (type === ASM_DOUBLE) {
          opcode = 'SETVD';
        } else throw 'yuck';
        return [l, [opcode, l, value & 255, (value >> 8) & 255]];
      } else {
        if (type === ASM_INT) {
          return [l, ['SETVIB', l, 0, 0, value & 255, (value >> 8) & 255, (value >> 16) & 255, (value >> 24) & 255]];
        } else if (type === ASM_DOUBLE) {
          if (value === (value | 0) && (value !== 0 || 1/value > 0)) {
            return [l, ['SETVDI', l, 0, 0, value & 255, (value >> 8) & 255, (value >> 16) & 255, (value >> 24) & 255]];
          } else if (value === Math.fround(value)) {
            return [l, ['SETVDF', l, 0, 0].concat(flattenFloat32(value))];
          } else {
            return [l, ['SETVDD', l, 0, 0].concat(flattenFloat64(value))];
          }
          throw 'fff ' + value;
        } else throw 'aw';
      }
    }

    function getNum(node) {
      if (node[0] === 'num') return node[1];
      if (node[0] === 'unary-prefix' && node[1] === '-' && node[2][0] === 'num') return -node[2][1];
      return null;
    }

    function makeBinary(node, type, sign, assignTo) {
      var opcode;
      var numValue = null; // if one operand is a number, we can emit an optimized op
      var otherValue = null;
      var numValueUnsigned = false;
      function tryNumSymmetrical(unsigned) { // flip operands to find a num
        numValue = getNum(node[3]);
        if (numValue !== null) {
          otherValue = node[2];
          numValueUnsigned = unsigned;
        } else {
          numValue = getNum(node[2]);
          if (numValue !== null) {
            otherValue = node[3];
            numValueUnsigned = unsigned;
          }
        }
      }
      function tryNumAsymmetrical(unsigned) { // only try on the last operand (still common, e.g. x % 5, y >> 2, z > 0
        numValue = getNum(node[3]);
        if (numValue !== null) {
          otherValue = node[2];
          numValueUnsigned = unsigned;
        }
      }
      switch(node[1]) {
        case '+': {
          if (node[3][0] === 'unary-prefix' && node[3][1] === '-') {
            // optimize x + (-y) into x - y
            node[1] = '-';
            node[3] = node[3][2];
            // fall through into '-'
          } else {
            if (type === ASM_INT) {
              opcode = 'ADD';
              tryNumSymmetrical();
            } else if (type === ASM_DOUBLE) opcode = 'ADDD';
            break;
          }
        }
        case '-': {
          if (type === ASM_INT) {
            opcode = 'SUB';
            tryNumAsymmetrical();
          } else if (type === ASM_DOUBLE) opcode = 'SUBD';
          break;
        }
        case '*': {
          if (type === ASM_INT) {
            opcode = 'MUL';
            tryNumSymmetrical();
          } else if (type === ASM_DOUBLE) opcode = 'MULD';
          break;
        }
        case '/': {
          if (type === ASM_INT) {
            if (sign === ASM_FLEXIBLE) sign = ASM_SIGNED;
            if (sign === ASM_SIGNED) opcode = 'SDIV';
            else opcode = 'UDIV';
            tryNumAsymmetrical(sign === ASM_UNSIGNED);
          }
          else if (type === ASM_DOUBLE) opcode = 'DIVD';
          break;
        }
        case '%': {
          if (type === ASM_INT) {
            if (sign === ASM_FLEXIBLE) sign = ASM_SIGNED;
            if (sign === ASM_SIGNED) opcode = 'SMOD';
            else opcode = 'UMOD';
            tryNumAsymmetrical(sign === ASM_UNSIGNED);
          }
          else if (type === ASM_DOUBLE) opcode = 'MODD';
          break;
        }
        case '<': {
          if (type === ASM_INT) {
            if (sign === ASM_FLEXIBLE) sign = ASM_SIGNED; // e.g. two numbers
            if (sign === ASM_SIGNED) opcode = 'SLT';
            else opcode = 'ULT';
            tryNumAsymmetrical(sign === ASM_UNSIGNED);
          }
          else if (type === ASM_DOUBLE) opcode = 'LTD';
          break;
        }
        case '<=': {
          if (type === ASM_INT) {
            if (sign === ASM_FLEXIBLE) sign = ASM_SIGNED; // e.g. two numbers
            if (sign === ASM_SIGNED) opcode = 'SLE';
            else opcode = 'ULE';
            tryNumAsymmetrical(sign === ASM_UNSIGNED);
          }
          else if (type === ASM_DOUBLE) opcode = 'LED';
          break;
        }
        case '>': {
          assert(type === ASM_DOUBLE);
          opcode = 'GTD';
          break;
        }
        case '>=': {
          assert(type === ASM_DOUBLE);
          opcode = 'GED';
          break;
        }
        case '==': {
          if (type === ASM_INT) {
            opcode = 'EQ';
            tryNumSymmetrical();
          } else if (type === ASM_DOUBLE) opcode = 'EQD';
          break;
        }
        case '!=': {
          if (type === ASM_INT) {
            opcode = 'NE';
            tryNumSymmetrical();
          } else if (type === ASM_DOUBLE) opcode = 'NED';
          break;
        }
        case '&': opcode = 'AND'; tryNumSymmetrical(); break;
        case '|': opcode = 'OR'; tryNumSymmetrical(); break;
        case '^': opcode = 'XOR'; tryNumSymmetrical(); break;
        case '<<': opcode = 'SHL'; tryNumAsymmetrical(true); break;
        case '>>': opcode = 'ASHR'; tryNumAsymmetrical(true); break;
        case '>>>': opcode = 'LSHR'; tryNumAsymmetrical(true); break;
        default: throw 'bad ' + node[1];
      }
      if (!opcode) assert(0, JSON.stringify([node, type, sign]));
      var x, y, z;
      var usingNumValue = numValue !== null && ((!numValueUnsigned && ((numValue << 24 >> 24) === numValue)) ||
                                                ( numValueUnsigned && ((numValue & 255) === numValue)));
      if (!usingNumValue) {
        y = getReg(node[2], undefined, type, sign);
        z = getReg(node[3], undefined, type, sign);
        x = assignTo >= 0 ? assignTo : getFree(y[0], z[0]);
        y[1] = y[1].concat(z[1]);
        y[1].push(opcode, x, releaseIfFree(y[0], x), releaseIfFree(z[0], x));
        return [x, y[1]];
      } else {
        // one operand is a small 8-bit signed number, emit an optimized instruction
        opcode += 'V';
        y = getReg(otherValue, undefined, type, sign);
        x = assignTo >= 0 ? assignTo : getFree(y[0]);
        y[1].push(opcode, x, releaseIfFree(y[0], x), numValue & 255);
        return [x, y[1]];
      }
    }

    function makeUnary(node, type, sign, assignTo) {
      var opcode;
      switch(node[1]) {
        case '-': {
          if (type === ASM_INT) opcode = 'NEG';
          else if (type === ASM_DOUBLE) opcode = 'NEGD';
          else throw 'x ' + type;
          break;
        }
        case '!': assert(type === ASM_INT); opcode = 'LNOT'; break;
        case '~': assert(type === ASM_INT); opcode = 'BNOT'; break;
        case 'I2D': case 'D2I': opcode = node[1]; break;
        default: throw 'bad';
      }
      var y = getReg(node[2]);
      var x = assignTo >= 0 ? assignTo : getFree(y[0]);
      y[1].push(opcode, x, releaseIfFree(y[0], x), 0);
      return [x, y[1]];
    }

    function makeBranchIfFalse(node, where) {
      // optimization: x = !y, BRF x => BRT y
      var opcode = 'BRF';
      if (node[0] === 'unary-prefix' && node[1] === '!') {
        node = node[2];
        opcode = 'BRT';
      }
      var condition = getReg(node);
      if (isFree(condition[0]) && condition[1][condition[1].length-4] in COMPARISONS) {
        // emit an optimized compare+branch: avoid storing to the free condition[0], and just load the jump address right after us
        condition[1][condition[1].length-4] += opcode;
        condition[1].push('absolute-value', where, 0, 0);
        releaseIfFree(condition[0]);
        return condition[1];
      }
      condition[1].push(opcode, releaseIfFree(condition[0]), where, 0);
      return condition[1];
    }

    function makeBranchIfTrue(node, where) {
      if (node[0] === 'unary-prefix' && node[1] === '!') {
        return makeBranchIfFalse(node[2], where);
      }
      return makeBranchIfFalse(['unary-prefix', '!', node], where);
    }

    function makeDo(node, label) {
      var oneTime = node[1][0] === 'num' && node[1][1] === 0; // trivial one-time loops do {..} while(0) do not need condition handling
      // TODO: more testing assert(!oneTime);
      var exit = getMarker('do-exit');
      var top, cond;
      if (!oneTime) {
        top = getMarker('do-top');
        cond = getMarker('do-cond');
      } else {
        top = -1; // no need to even mark the top
        cond = exit; // when we reach the condition, we just exit
      }
      breakStack.push(exit);
      continueStack.push(cond);
      if (label) {
        assert(!(label in breakLabels));
        breakLabels[label] = exit;
        assert(!(label in continueLabels));
        continueLabels[label] = cond;
      }
      var body = walkStatements(node[2]);
      breakStack.pop();
      continueStack.pop();
      if (label) {
        delete breakLabels[label];
        delete continueLabels[label];
      }
      var condition;
      if (!oneTime) {
        condition = makeBranchIfTrue(node[1], top);
      }
      var ret = [];
      if (!oneTime) {
        ret.push('marker', top, 0, 0);
      }
      ret = ret.concat(body);
      if (!oneTime) {
        ret.push('marker', cond, 0, 0);
        ret = ret.concat(condition);
      }
      ret.push('marker', exit, 0, 0);
      return [-1, ret];
    }

    function makeWhile(node, label) {
      var infinite = node[1][0] === 'num' && node[1][1] === 1; // trivial infinite loops while(1) {..} do not need condition handling
      var top = getMarker('while-top'), exit = getMarker('while-exit');
      var cond = !infinite ? getMarker('while-cond') : top;
      breakStack.push(exit);
      continueStack.push(cond);
      if (label) {
        assert(!(label in breakLabels));
        breakLabels[label] = exit;
        assert(!(label in continueLabels));
        continueLabels[label] = cond;
      }
      var ret = [];
      if (!infinite) {
        ret.push('marker', cond, 0, 0);
        ret = ret.concat(makeBranchIfFalse(node[1], exit));
      }
      ret = ret.concat(['marker', top, 0, 0]).concat(walkStatements(node[2]));
      ret.push('BR', 0, cond, 0);
      breakStack.pop();
      continueStack.pop();
      if (label) {
        delete breakLabels[label];
        delete continueLabels[label];
      }
      ret.push('marker', exit, 0, 0);
      return [-1, ret];
    }

    function makeSwitch(node, label) {
      var cases = node[2];
      // there must be one non-default case, otherwise we add a fake one
      var normals = 0;
      for (var i = 0; i < cases.length; i++) {
        var c = cases[i];
        var id = getId(c[0]);
        if (id !== 'default') normals++;
      }
      if (normals === 0) {
        // ignore the condition, we must always reach the default
        node[1] = ['seq', node[1], ['num', 0]]; // always 0
        cases.unshift([['num', 1], ['block', []]]); // a block for 1, which is never hit
      }
      // now the switch is normalized and has one non-default case, proceed
      var condition = getReg(node[1]);
      var exit = getMarker('switch-exit');
      // parse cases and emit code
      breakStack.push(exit);
      if (label) {
        assert(!(label in breakLabels));
        breakLabels[label] = exit;
      }
      var data = {};
      var minn = Infinity, maxx = -Infinity;
      function getId(raw) {
        if (raw === null) return 'default';
        else if (raw[0] === 'num') return raw[1];
        else if (raw[0] === 'unary-prefix' && raw[2][0] === 'num') return -raw[2][1];
        else throw 'bad case';
      }
      for (var i = 0; i < cases.length; i++) {
        var c = cases[i];
        var id = getId(c[0]);
        if (id === 'default') assert(i === cases.length-1, 'if there is a default, it must be last');
        data[id] = {
          i: i, // original index
          id: id,
          code: walkStatements(c[1]),
          marker: getMarker('switch-case'),
          next: -1 // will be the fall through target
        };
        if (typeof id === 'number') {
          minn = Math.min(id, minn);
          maxx = Math.max(id, maxx);
        }
      }
      breakStack.pop();
      if (label) {
        delete breakLabels[label];
      }
      // finalize nexts
      for (var id in data) {
        var info = data[id];
        var i = info.i + 1;
        while (1) {
          if (i >= cases.length) {
            info.next = exit;
            break;
          }
          var nextId = getId(cases[i][0]);
          assert(nextId in data);
          info.next = data[nextId].marker;
          break;
          // TODO: optimize all this, we don't need a fallthrough branch if we branch anyhow; recurse multiple fallthroughs; etc.
        }
      }
      // calculate values
      var range = maxx - minn + 1;
      assert(minn === (minn | 0) && range === (range | 0));
      var defaultMarker = data['default'] ? data['default'].marker : getMarker('switch-default');
      // emit the switch instruction itself
      var tempMin = getFree(), tempRange = getFree();
      var ret = condition[1].concat(makeNum(minn, ASM_INT, tempMin)[1]).concat(makeNum(range, ASM_INT, tempRange)[1]);
      ret.push('SWITCH', condition[0], tempMin, tempRange);
      releaseFree(tempRange);
      releaseFree(tempMin);
      releaseIfFree(condition[0]);
      // emit the jump table
      for (var i = 0; i < range; i++) {
        var j = minn + i;
        var info = data[j];
        if (info) {
          ret.push('absolute-value', info.marker, 0, 0);
        } else {
          ret.push('absolute-value', defaultMarker, 0, 0);
        }
      }
      // emit the default TODO: optimize when there is no default
      ret.push('marker', defaultMarker, 0, 0);
      if (data['default']) {
        ret = ret.concat(data['default'].code);
      }
      ret.push('BR', 0, exit, 0);
      // emit the jump table targets
      for (var i = 0; i < range; i++) {
        var j = minn + i;
        var info = data[j];
        if (info) {
          ret.push('marker', info.marker, 0, 0);
          ret = ret.concat(info.code);
          ret.push('BR', 0, info.next, 0);
        }
      }
      ret.push('marker', exit, 0, 0);
      return [-1, ret];
    }

    function makeCall(lx, node, type) {
      // TODO: specialize calls like imul
      assert(node[0] === 'call');
      var ret = [];
      var target;
      var functionPointer = null;
      var internal = false;
      if (node[1][0] === 'name') {
        // normal direct call
        target = node[1][1];
        // special-case some call targets
        switch (target) {
          case 'Math_imul': {
            assert(node[2].length === 2);
            var mul = makeBinary(['binary', '*', node[2][0], node[2][1]], ASM_INT, ASM_SIGNED, lx);
            assert(mul[0] === lx);
            return mul[1];
          }
          case 'Math_fround': {
            assert(node[2].length === 1);
            var child = getReg(node[2][0], undefined, ASM_DOUBLE, ASM_NONSIGNED, lx);
            child[1].push('FROUND', lx, child[0], 0);
            releaseIfFree(child[0], lx);
            return child[1];
          }
        }
        if ((target in EMTERPRETED_FUNCS) && !PROFILING) internal = true;
      } else {
        // function pointer call through function table
        assert(node[1][0] === 'sub' && node[1][1][0] === 'name');
        target = node[1][1][1];
        assert(isFunctionTable(target));
        functionPointer = getReg(node[1][2]);
        ret = ret.concat(functionPointer[1]);
      }
      var actuals = [];
      var sig = ASM_SIG[type];
      node[2].forEach(function(param) {
        var reg = getReg(param);
        ret = ret.concat(reg[1]);
        actuals.push(reg[0]);
        var curr = ASM_SIG[detectType(param, asmData)];
        if (curr === 'v') curr = 'i'; // if we can't detect it, it must be an asm global. the only possibilities are int. TODO: add globals to detectType
        sig += curr;
      });
      ret.push(internal ? 'INTCALL' : 'EXTCALL');
      ret.push(lx);
      if (!internal) {
        ret.push(target);
        assert(sig.indexOf('u') < 0); // no undefined
        ret.push(sig);
      } else {
        ret.push(0, 0);
      }
      actuals.forEach(function(actual) { releaseIfFree(actual) });
      if (functionPointer) {
        ret.push(releaseIfFree(functionPointer[0]));
      }
      if (internal) {
        ret.push('absolute-funcaddr', target, 0, 0);
      }
      ret = ret.concat(actuals);
      while (ret.length % 4 !== 0) ret.push(0);
      return ret;
    }

    function walkStatements(stats) {
      if (!stats) return [];
      if (stats[0] === 'block') return walkStatements(stats[1]);
      if (typeof stats[0] === 'string') stats = [stats];
      var ret = [];
      stats.forEach(function(stat) {
        var before = freeLocals.length;
        var raw = getReg(stat, true);
        //printErr('raw: ' + JSON.stringify(stat));
        releaseIfFree(raw[0]);
        if (freeLocals.length !== before) assert(0, [before, freeLocals.length] + ' due to ' + astToSrc(stat)); // the statement is done - nothing should still be held on to
        var curr = raw[1];
        //printErr('stat: ' + JSON.stringify(curr));
        verifyCode(curr, stat);
        ret = ret.concat(curr);
      });
      return ret;
    }

    function finalizeJumps(code) {
      function getI(obj) { // the target of a marker
        var i = 0;
        while (i < code.length) { // find the definition, ignoring uses
          i = code.indexOf(obj, i);
          if (i < 0) return -1;
          if (code[i-1] === 'marker') return i-1;
          i++;
        }
        return -1;
      }
      function sanityCheck() {
        var seenMarkers = {}; // every absolute value must have a valid target
        for (var i = 0; i < code.length; i += 4) {
          if (code[i] === 'marker') {
            seenMarkers[code[i+1].id] = 1;
          }
        }
        for (var i = 0; i < code.length; i += 4) {
          if (code[i] === 'absolute-value') {
            assert(code[i+1].id in seenMarkers);
          }
        }
      }
      sanityCheck();
      // first pass, collect markers and absolute targets, and their uses
      for (var i = 0; i < code.length; i += 4) {
        assert(!(code[i] in ABSOLUTE_BRANCHES));
        if (code[i] in RELATIVE_BRANCHES) {
          code[i+2].relativeUses++;
        } else if (code[i] === 'absolute-value') {
          code[i+1].absoluteUses++;
        }
      }
      sanityCheck();
      // optimization pass, skip over multiple jumps
      function skipNOPs(i) {
        while (code[i] === 'marker') i += 4; // jump over all NOPs here
        return i;
      }
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] in RELATIVE_BRANCHES) {
          while (1) {
            var j = getI(code[i+2]);
            assert(code[j] === 'marker');
            j = skipNOPs(j);
            if (code[j] === 'BR' && code[i+2] !== code[j+2]) {
              code[i+2].relativeUses--;
              assert(code[i+2].relativeUses >= 0);
              code[j+2].relativeUses++;
              code[i+2] = code[j+2];
            } else {
              break;
            }
          }
        }
      }
      sanityCheck();
      // optimization pass, remove unreachable code
      function deleteCode(i, num) {
        // drop uses for code we are removing
        for (var j = i; j < i + num; j += 4) {
          assert(!(code[j] in ABSOLUTE_BRANCHES));
          if (code[j] in RELATIVE_BRANCHES) {
            code[j+2].relativeUses--;
            assert(code[j+2].relativeUses >= 0);
          } else if (code[j] === 'absolute-value') {
            code[j+1].absoluteUses--;
            assert(code[j+1].absoluteUses >= 0);
          }
        }
        code.splice(i, num);
      }
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] in UNCONDITIONAL_BRANCHES) {
          var j = i + 4; // normal forward control flow cannot reach this position
          while (j < code.length) {
            if (code[j] === 'marker') {
              var relativeUses = code[j+1].relativeUses;
              assert(relativeUses >= 0);
              if (relativeUses > 0) break; // this is reachable
              var absoluteUses = code[j+1].absoluteUses;
              assert(absoluteUses >= 0);
              if (absoluteUses > 0) break; // this is reachable
            }
            j += 4; // this instr is unreachable
          }
          if (j > i + 4) {
            deleteCode(i + 4, j - i - 4);
          }
        }
      }
      sanityCheck();
      // second pass, find out which relative branches must be converted to absolutes, because they are too big
      // and convert them
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] in RELATIVE_BRANCHES) {
          var obj = code[i+2];
          var target = getI(obj);
          var offset = target - i; // overestimate, since there are still 'marker's that will be cleaned up
          var storedOffset = offset >> 2; // offsets are divisible by 4, so we ignore the lower bits
          var maxOffset = storedOffset * 2; // when we convert relative to absolute, we double the size of a branch.
                                            // so worst case, we may double offsets (TODO this could be optimized)
          if ((maxOffset << 16 >> 16) !== maxOffset) {
            code[i] += 'A'; // convert branch to absolute
            code[i+2] = 0; // id is no longer needed, 4 extra bytes in the inst will contain the absolute value
            code.splice(i+4, 0, 'absolute-value', obj, 0, 0); // add absolute value after first part of branch inst
            obj.relativeUses--;
            assert(obj.relativeUses >= 0);
            obj.absoluteUses++;
          }
        }
      }
      sanityCheck();
      // optimizations that can only reduce code size
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] === 'BRF' && code[i+4] === 'BR') {
          // if we have a branch-if-false and then a branch, and the BRF jumps to right after the branch, then we can just emit
          // a BRT. this can happen at the end of while loops, if there is a condition that checks if we should break out
          if (skipNOPs(i+8) === skipNOPs(getI(code[i+2]))) {
            code[i] = 'BRT';
            code[i+2].relativeUses--;
            code[i+2] = code[i+6];
            code.splice(i+4, 4);
          }
        } else if (code[i] in CONDITION_BRFS && code[i+8] === 'BR') {
          // similar optimization, with condition+BRF => condition+BRT
          if (skipNOPs(i+12) === skipNOPs(getI(code[i+5]))) {
            code[i] = code[i].substr(0, code[i].length-1) + 'T'; // same condition, swap BRF to BRT
            code[i+5].absoluteUses--;
            assert(code[i+5].absoluteUses >= 0);
            code[i+5] = code[i+10];
            code[i+5].relativeUses--;
            assert(code[i+5].relativeUses >= 0);
            code[i+5].absoluteUses++;
            code.splice(i+8, 4);
          }
        }
      }
      // remove relative and absolute placeholders, after which every instruction is now in its absolute location, and we can write out absolutes 
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] === 'marker') {
          var obj = code[i+1];
          obj.i = i;
          assert(i == getI(obj));
          if (obj.absoluteUses > 0) {
            absoluteTargets[obj.id] = i;
          }
          code.splice(i, 4);
          i -= 4;
        } else if (code[i] === 'absolute-value') {
          var obj = code[i+1];
          assert(obj.absoluteUses > 0);
          code[i+1] = obj.id;
        }
      }
      // final pass, finalize relative jumps TODO: optimize jump->jump->x to jump->x
      for (var i = 0; i < code.length; i += 4) {
        if (code[i] in RELATIVE_BRANCHES) {
          var obj = code[i+2];
          var target = obj.i;
          assert(target >= 0);
          var offset = target - i;
          assert(offset % 4 === 0);
          offset >>= 2;
          assert(Math.abs(offset) < 32768);
          code[i+2] = offset & 255;
          code[i+3] = (offset >> 8) & 255;
        }
      }
    }

    function hoistConstants(stats) {
      // find constants that appear a lot or appear in loops, and hoist them to the top
      var nums = {};
      var depth = 0;
      traverse(stats, function(node, type) {
        if (type in LOOP && !(node[1] === 'num' && node[2] === 0)) depth++;
        else if (type === 'unary-prefix' && node[1] === '+' && getNum(node[2]) !== null) return null; // we only care about ints
        var num = getNum(node);
        if (num === null) return;
        nums[num] = (nums[num] || 0) + Math.pow(5, depth);
        return null; // do not traverse into this node
      }, function(node, type) {
        if (type in LOOP && !(node[1] === 'num' && node[2] === 0)) depth--;
      });
      var ks = keys(nums);
      ks.sort(function(x, y) { return nums[y] - nums[x] });
      var ret = [];
      var limit = Math.max(numLocals + 3, Math.min(1.1*numLocals, 120));
      for (var i = 0; i < ks.length && numLocals < limit; i++) {
        var n = ks[i];
        if (n >= -127 && n < 128) continue; // constant like these are typically folded into ops anyhow
        var weight = nums[n];
        if (weight < 10) continue; // not heavy enough
        // hoist this num
        var reg = makeNum(n, ASM_INT);
        //printErr('hoist ' + [n, weight, reg[0]]);
        hoistedNums[n] = reg[0];
        ret = ret.concat(reg[1]);
        assert(reg[0] === numLocals);
        numLocals++; // this will never be freed
      }
      return ret;
    }

    // walkFunction main

    var ignore = !(func[1] in EMTERPRETED_FUNCS);

    if (ignore) {
      // we are not emterpreting this function
      if (ASYNC && ASSERTIONS && !okToCallWhileAsync(func[1])) {
        // we need to be careful to never enter non-emterpreted code while doing an async save/restore,
        // which is what happens if non-emterpreted code is on the stack while we attempt to save.
        // add asserts right after each call
        var stack = [];
        traverse(func, function(node, type) {
          stack.push(node);
        }, function(node, type) { // post-traversal
          stack.pop();
          if (type !== 'call') return;
          if (node[1][0] === 'name' && isMathFunc(node[1][1])) return;
          var callType = ASM_NONE;
          var parent = stack[stack.length-1];
          if (parent) {
            var temp = null;
            if (parent[0] === 'binary' && parent[1] === '|' && parent[3][0] === 'num' && parent[3][1] === 0 &&
                parent[2] === node) {
              // int-coerced call
              callType = ASM_INT;
              temp = 'tempInt';
            } else if (parent[0] === 'unary-prefix' && parent[1] === '+' && parent[2] === node) {
              // double-coerced call
              callType = ASM_DOUBLE;
              temp = 'tempDouble';
            }
            // XXX fails on other coercions of odd types, like float32, simd, etc!
            if (temp) {
              // assign to temp, assert, return proper value:     temp = call() , (asyncState ? abort() : temp)
              trample(node, ['seq',
                ['assign', null, ['name', temp], makeAsmCoercion(copy(node), callType)],
                ['conditional', ['name', 'asyncState'], makeAsmCoercion(['call', ['name', 'abort'], [['num', '-12']]], callType), ['name', temp]]
              ]);
              return;
            }
          }
          // no important parent
          trample(node, ['seq',
            copy(node),
            ['conditional', ['name', 'asyncState'], makeAsmCoercion(['call', ['name', 'abort'], [['num', '-12']]], ASM_INT), ['num', 0]]
          ]);
        });
        // add an assert in the prelude of the function
        var stats = getStatements(func);
        for (var i = 0; i < stats.length; i++) {
          var node = stats[i];
          if (node[0] == 'stat') node = node[1];
          if (node[0] !== 'var' && node[0] !== 'assign') {
            stats.splice(i, 0, ['stat', 
              ['conditional', ['name', 'asyncState'], makeAsmCoercion(['call', ['name', 'abort'], [['num', '-12']]], ASM_INT), ['num', 0]]
            ]);
            break;
          }
        }
        // perhaps also add at loop headers? TODO
      }
      print(astToSrc(func));
    }

    var asmData = normalizeAsm(func);
    print('// return type: [' + func[1] + ',' + getSignature(func, asmData) + ']');

    if (ignore) {
      return;
    }

    //printErr('emterpretifying ' + func[1]);

    // we implement floats as doubles, and just decrease precision when fround is called. flip floats to doubles, but we
    // must restore this at the end when we emit the trampolines
    var trueParams = asmData.params;
    asmData.params = {};
    for (var t in trueParams) {
      if (trueParams[t] === ASM_FLOAT) {
        asmData.params[t] = ASM_DOUBLE;
      } else {
        asmData.params[t] = trueParams[t];
      }
    }
    var trueVars = asmData.vars;
    asmData.vars = {};
    for (var t in trueVars) {
      if (trueVars[t] === ASM_FLOAT) {
        asmData.vars[t] = ASM_DOUBLE;
      } else {
        asmData.vars[t] = trueVars[t];
      }
    }
    traverse(func, function() {} , function(node, type) {
      // Math_fround(x) => +Math_fround(+x), so that see no float types on temp values; types are double or int, and fround is just a function we emit
      if (type === 'call' && node[1][0] === 'name' && node[1][1] === 'Math_fround') {
        assert(node[2].length === 1);
        old = ['call', node[1], [['unary-prefix', '+', node[2][0]]]];
        node[0] = 'unary-prefix';
        node[1] = '+';
        node[2] = old;
      }
    });

    // consider locals

    var locals = {};
    var numLocals = 0; // ignores slow locals, they are over 255 and not directly accessible
    var numVars = 0;

    function countLocals() {
      numLocals = 0;
      for (var i in asmData.params) {
        numLocals++;
      }
      for (var i in asmData.vars) {
        numLocals++;
        numVars++;
      }
    }

    countLocals();
    if (numLocals >= FAST_LOCALS) {
      //printErr('warning: ' + numLocals + ' locals in ' + func[1] + ', which is very high, trying to reduce');
      aggressiveVariableEliminationInternal(func, asmData);
      countLocals();
      //printErr('...number of locals is now ' + numLocals);
    }

    // put the variables that need a zero-init at the beginning
    locals = {};
    numLocals = 0;
    for (var i in asmData.params) {
      locals[i] = numLocals++;
    }
    assert(numLocals < FAST_LOCALS, 'way too many params!');
    assert(FAST_LOCALS < 256);
    for (var i in asmData.vars) {
      locals[i] = numLocals++; // TODO: sort by frequency of appearance, so common ones are fast, rare are slow
      if (numLocals === FAST_LOCALS) numLocals = 256; // jump over the temps, remaining locals are slow locals
    }
    var withSlowLocals = numLocals;
    numLocals = Math.min(numLocals, FAST_LOCALS); // ignore the slow locals
    for (var i = 255; i >= numLocals; i--) {
      freeLocals.push(i);
    }

    var stats = getStatements(func);
    func[3] = [];

    // do some pre-calculation and optimization
    var constants = hoistConstants(stats);
    assert(numLocals < 225); // leave plenty of room for temps

    // walk all the function to emit bytecode, and add a final ret
    var code = walkStatements(stats);
    assert(code.length % 4 === 0);
    if (code.length < 4 || code[code.length-4] != 'RET') {
      code.push('RET', 0, 0, 0); // final ret for the function
    }
    // calculate final count of local variables, and emit func header
    var finalLocals = Math.max(numLocals, maxLocal+1, withSlowLocals);
    assert(finalLocals < 65535, 'too many locals ' + [maxLocal, numLocals, withSlowLocals]);
    code = ['FUNC', func[2].length, finalLocals & 255, finalLocals >>> 8, 0, 0, 0, 0].concat(constants).concat(code);
    verifyCode(code);

    finalizeJumps(code);

    // check if this is a leaf method (does no calls to other emterpreted code)
    var DEFINITE_LEAVES = set('_memcpy', '_memmove', '_memset', '_strlen', '_strncpy', '_strcpy', '_strcat', '_printf', '_puts', '_malloc', '_free'); // things we know for sure are leaves XXX hackish, dangerous. To get more stuff in here, it must be either in a JS library, or blacklisted from the emterpreter, and be known to never reach emterpreted code.

    var leaf = true;
    for (var i = 0; i < code.length; i += 4) {
      // if this is a call that can conceivably reach other emterpreted code, it isn't a leaf
      if (code[i] === 'INTCALL' || (code[i] === 'EXTCALL' && !isMathFunc(code[i+2]) && !(code[i+2] in DEFINITE_LEAVES))) {
        leaf = false;
        //printErr('                   NOT leaf ' + func[1] + ' since ' + code[i+2]);
        break;
      }
    }
    //if (leaf) printErr(func[1]);

    var zero = false; // leaf; // TODO: heuristics
    var onlyLeavesAreZero = true; // if only leaves are zero, then we do not need to save and restore the stack XXX if this is not true, then setjmp and exceptions can fail, as cleanup is skipped!

    if (zero) code[3] = 1;

    if ((func[1] in EXTERNAL_EMTERPRETED_FUNCS) || PROFILING) {
      // this is reachable from outside emterpreter code, set up a trampoline
      asmData.params = trueParams; // restore them, we altered float=>double
      asmData.vars = {};
      if (zero && !onlyLeavesAreZero) {
        // emterpreters run using the stack starting at 0. we must copy it so we can restore it later
        asmData.vars['sp'] = ASM_INT;
        func[3].push(srcToStat('sp = EMTSTACKTOP;'));
        var stackBytes = finalLocals*8;
        func[3].push(srcToStat('EMTSTACKTOP = EMTSTACKTOP + ' + stackBytes + ' | 0;'));
        func[3].push(srcToStat('assert(((EMTSTACKTOP|0) <= (EMT_STACK_MAX|0))|0);'));
        asmData.vars['x'] = ASM_INT;
        func[3].push(srcToStat('while ((x | 0) < ' + stackBytes + ') { HEAP32[sp + x >> 2] = HEAP32[x >> 2] | 0; x = x + 4 | 0; }'));
      }
      // copy our arguments to our stack frame
      var bump = ASYNC ? 8 : 0; // we will assert in the emterpreter itself that we did not overflow the emtstack
      var argStats = [];
      func[2].forEach(function(arg) {
        var code;
        switch (asmData.params[arg]) {
          case ASM_INT:    code = 'HEAP32[' + (zero ? (bump >> 2) : ('EMTSTACKTOP + ' + bump + ' >> 2')) + '] = ' + arg + ';'; break;
          case ASM_FLOAT:
          case ASM_DOUBLE: code = 'HEAPF64[' + (zero ? (bump >> 3) : ('EMTSTACKTOP + ' + bump + ' >> 3')) + '] = ' + arg + ';'; break;
          default: throw 'bad';
        }
        argStats.push(srcToStat(code));
        bump += 8; // each local is a 64-bit value
      });
      if (ASYNC) {
        argStats.push(['if', srcToExp('(asyncState|0) == 1'), srcToStat('asyncState = 3;')]); // we know we are during a sleep, mark the state
        argStats = [['if', srcToExp('(asyncState|0) != 2'), ['block', argStats]]]; // 2 means restore, so do not trample the stack
      }
      func[3] = func[3].concat(argStats);
      // prepare the call into the emterpreter
      var theName = ['name', 'emterpret'];
      var theCall = ['call', theName, [['name', 'EMTERPRETER_' + func[1]]]]; // EMTERPRETER_* will be replaced with the absolute bytecode offset later
      // add the call
      func[3].push(['stat', theCall]);
      if (zero) {
        theName[1] += '_z';
        if (!onlyLeavesAreZero) {
          // restore the stack
          func[3].push(srcToStat('x = 0;'));
          func[3].push(srcToStat('while ((x | 0) < ' + stackBytes + ') { HEAP32[x >> 2] = HEAP32[sp + x >> 2] | 0; x = x + 4 | 0; }'));
          func[3].push(srcToStat('EMTSTACKTOP = sp;'));
        }
      }
      // add the return, if necessary
      if (asmData.ret !== undefined) {
        var ret;
        switch (asmData.ret) {
          case ASM_INT: ret = srcToExp('HEAP32[EMTSTACKTOP >> 2]'); break;
          case ASM_FLOAT:
          case ASM_DOUBLE: ret = srcToExp('HEAPF64[EMTSTACKTOP >> 3]'); break;
          default: throw 'bad';
        }
        func[3].push(['return', makeAsmCoercion(ret, asmData.ret)]);
      }
      // emit trampoline and bytecode
      denormalizeAsm(func, asmData);
      print(astToSrc(func));
    }
    print('// EMTERPRET_INFO ' + JSON.stringify([func[1], code, absoluteTargets]));
  }
  traverseGeneratedFunctions(ast, walkFunction);
}

// emits which functions are directly reachable from, except for some blacklist
function findReachable(ast) {
  var BLACKLIST = set(extraInfo.blacklist);
  var reachable = {};
  traverseGeneratedFunctions(ast, function(func) {
    if (func[1] in BLACKLIST) return;
    traverse(func, function(node, type) {
      if (type === 'call' && node[1][0] === 'name') {
        reachable[node[1][1]] = 1;
      }
    });
  });
  print('// REACHABLE ' + JSON.stringify(keys(reachable)));
}

// emits call graph information
function dumpCallGraph(ast) {
  traverseGeneratedFunctions(ast, function(func) {
    var reachable = {};
    traverse(func, function(node, type) {
      if (type === 'call') {
        if (node[1][0] === 'name') {
          reachable[node[1][1]] = 1;
        } else {
          // (FUNCTION_TABLE[..])(..)
          assert(node[1][0] === 'sub')
          assert(node[1][1][0] === 'name');
          reachable[node[1][1][1]] = 1;
        }
      }
    });
    print('// REACHABLE ' + JSON.stringify([func[1], ' => ', keys(reachable)]));
  });
}

// Last pass utilities

// Change +5 to DOT$ZERO(5). We then textually change 5 to 5.0 (uglify's ast cannot differentiate between 5 and 5.0 directly)
function prepDotZero(ast) {
  traverse(ast, function(node, type) {
    if (type === 'unary-prefix' && node[1] === '+') {
      if (node[2][0] === 'num' ||
          (node[2][0] === 'unary-prefix' && node[2][1] === '-' && node[2][2][0] === 'num')) {
        return ['call', ['name', 'DOT$ZERO'], [node[2]]];
      }
    }
  });
}
function fixDotZero(js) {
  return js.replace(/-DOT\$ZERO\(-/g, '- DOT$ZERO(-') // avoid x - (-y.0) turning into x--y.0 when minified
           .replace(/DOT\$ZERO\(([-+]?(0x)?[0-9a-f]*\.?[0-9]+([eE][-+]?[0-9]+)?)\)/g, function(m, num) {
    if (num.substr(0, 2) === '0x' || num.substr(0, 3) === '-0x') {
      var ret = eval(num).toString();
      if (ret.indexOf('.') < 0) return ret + '.0';
      return ret;
    }
    if (num.indexOf('.') >= 0) return num;
    var e = num.indexOf('e');
    if (e < 0) return num + '.0';
    return num.substr(0, e) + '.0' + num.substr(e);
  });
}

function asmLastOpts(ast) {
  var statsStack = [];
  traverseGeneratedFunctions(ast, function(fun) {
    traverse(fun, function(node, type) {
      var stats = getStatements(node);
      if (stats) statsStack.push(stats);
      if (type in CONDITION_CHECKERS) {
        node[1] = simplifyCondition(node[1]);
      }
      if (type === 'while' && node[1][0] === 'num' && node[1][1] === 1 && node[2][0] === 'block' && node[2].length == 2) {
        // This is at the end of the pipeline, we can assume all other optimizations are done, and we modify loops
        // into shapes that might confuse other passes

        // while (1) { .. if (..) { break } } ==> do { .. } while(..)
        var stats = node[2][1];
        var last = stats[stats.length-1];
        if (last && last[0] === 'if' && !last[3] && last[2][0] === 'block' && last[2][1][0]) {
          var lastStats = last[2][1];
          var lastNum = lastStats.length;
          var lastLast = lastStats[lastNum-1];
          if (!(lastLast[0] === 'break' && !lastLast[1])) return;// if not a simple break, dangerous
          for (var i = 0; i < lastNum; i++) {
            if (lastStats[i][0] !== 'stat' && lastStats[i][0] !== 'break') return; // something dangerous
          }
          // ok, a bunch of statements ending in a break
          var abort = false;
          var stack = 0;
          var breaks = 0;
          traverse(stats, function(node, type) {
            if (type === 'continue') {
              if (stack === 0 || node[1]) { // abort if labeled (we do not analyze labels here yet), or a continue directly on us
                abort = true;
                return true;
              }
            } else if (type === 'break') {
              if (stack === 0 || node[1]) { // relevant if labeled (we do not analyze labels here yet), or a break directly on us
                breaks++;
              }
            } else if (type in LOOP) {
              stack++;
            }
          }, function(node, type) {
            if (type in LOOP) {
              stack--;
            }
          });
          if (abort) return;
          assert(breaks > 0);
          if (lastStats.length > 1 && breaks !== 1) return; // if we have code aside from the break, we can only move it out if there is just one break
          // start to optimize
          if (lastStats.length > 1) {
            var parent = statsStack[statsStack.length-1];
            var me = parent.indexOf(node);
            if (me < 0) return; // not always directly on a stats, could be in a label for example
            parent.splice.apply(parent, [me+1, 0].concat(lastStats.slice(0, lastStats.length-1)));
          }
          var conditionToBreak = last[1];
          stats.pop();
          node[0] = 'do';
          node[1] = simplifyNotCompsDirect(['unary-prefix', '!', conditionToBreak]);
          return node;
        }
      } else if (type === 'binary') {
        if (node[1] === '&') {
          if (node[3][0] === 'unary-prefix' && node[3][1] === '-' && node[3][2][0] === 'num' && node[3][2][1] === 1) {
            // Change &-1 into |0, at this point the hint is no longer needed
            node[1] = '|';
            node[3] = node[3][2];
            node[3][1] = 0;
          }
        } else if (node[1] === '-' && node[3][0] === 'unary-prefix') {
          // avoid X - (-Y) because some minifiers buggily emit X--Y which is invalid as -- can be a unary. Transform to
          //        X + Y
          if (node[3][1] === '-') { // integer
            node[1] = '+';
            node[3] = node[3][2];
          } else if (node[3][1] === '+') { // float
            if (node[3][2][0] === 'unary-prefix' && node[3][2][1] === '-') {
              node[1] = '+';
              node[3][2] = node[3][2][2];
            }
          }
        }
      }
    }, function(node, type) {
      var stats = getStatements(node);
      if (stats) statsStack.pop();
    });
    if (!debug) { // dangerous in debug mode, as without braces things can end up on the same line, together with comments
      // convert  { singleton }  into  singleton
      traverse(fun, function(node, type) {
        if (type === 'block' && node[1] && node[1].length === 1) {
          return node[1][0];
        }
      });
    }
  });
}

// Contrary to the name this does not eliminate actual dead functions, only
// those marked as such with DEAD_FUNCTIONS
function eliminateDeadFuncs(ast) {
  assert(asm);
  assert(extraInfo && extraInfo.dead_functions);
  var deadFunctions = set(extraInfo.dead_functions);
  traverseGeneratedFunctions(ast, function (fun, type) {
    if (!(fun[1] in deadFunctions)) {
      return;
    }
    var asmData = normalizeAsm(fun);
    fun[3] = [['stat', ['call', ['name', 'abort'], [['num', -1]]]]];
    asmData.vars = {};
    denormalizeAsm(fun, asmData);
  });
}

// Cleans up globals in an asm.js module that are not used. Assumes it
// receives a full asm.js module, as from the side file in --separate-asm
function eliminateDeadGlobals(ast) {
  traverse(ast, function(func, type) {
    if (type !== 'function') return;
    // find all symbols used by name that are not locals, so they must be globals
    var stats = func[3];
    var used = {};
    for (var i = 0; i < stats.length; i++) {
      var asmFunc = stats[i];
      if (asmFunc[0] === 'defun') {
        var asmData = normalizeAsm(asmFunc);
        traverse(asmFunc, function(node, type) {
          if (type == 'name') {
            var name = node[1];
            if (!(name in asmData.params || name in asmData.vars)) {
              used[name] = 1;
            }
          }
        });
        denormalizeAsm(asmFunc, asmData);
      } else {
        traverse(asmFunc, function(node, type) {
          if (type == 'name') {
            var name = node[1];
            used[name] = 1;
          }
        });
      }
    }
    for (var i = 0; i < stats.length; i++) {
      var node = stats[i];
      if (node[0] === 'var') {
        for (var j = 0; j < node[1].length; j++) {
          var v = node[1][j];
          var name = v[0];
          var value = v[1];
          if (!(name in used)) {
            node[1].splice(j, 1);
            j--;
            if (node[1].length == 0) {
              // remove the whole var
              stats[i] = emptyNode();
            }
          }
        }
      } else if (node[0] === 'defun') {
        if (!(node[1] in used)) {
          stats[i] = emptyNode();
        }
      }
    }
    removeEmptySubNodes(func);
  });
}

// Removes obviously-unused code. Similar to closure compiler in its rules -
// export e.g. by Module['..'] = theThing; , or use it somewhere, otherwise
// it goes away.
function JSDCE(ast) {
  var scopes = [{}]; // begin with empty toplevel scope
  function DUMP() {
    printErr('vvvvvvvvvvvvvv');
    for (var i = 0; i < scopes.length; i++) {
      printErr(i + ' : ' + JSON.stringify(scopes[i]));
    }
    printErr('^^^^^^^^^^^^^^');
  }
  function ensureData(scope, name) {
    if (scope[name]) return scope[name];
    scope[name] = {
      def: 0,
      use: 0,
      param: 0 // true for function params, which cannot be eliminated
    };
    return scope[name];
  }
  function cleanUp(ast, names) {
    traverse(ast, function(node, type) {
      if (type === 'defun' && node[1] in names) return emptyNode();
      if (type === 'defun' || type === 'function') return null; // do not enter other scopes
      if (type === 'var') {
        node[1] = node[1].filter(function(varItem, j) {
          var curr = varItem[0];
          var value = varItem[1];
          return !(curr in names) || (value && hasSideEffects(value));
        });
        if (node[1].length === 0) return emptyNode();
      }
    });
    return ast;
  }
  traverse(ast, function(node, type) {
    if (type === 'var') {
      node[1].forEach(function(varItem, j) {
        var name = varItem[0];
        ensureData(scopes[scopes.length-1], name).def = 1;
      });
      return;
    }
    if (type === 'defun' || type === 'function') {
      if (node[1]) ensureData(scopes[scopes.length-1], node[1]).def = 1;
      var scope = {};
      node[2].forEach(function(param) {
        ensureData(scope, param).def = 1;
        scope[param].param = 1;
      });
      scopes.push(scope);
      return;
    }
    if (type === 'name') {
      ensureData(scopes[scopes.length-1], node[1]).use = 1;
    }
  }, function(node, type) {
    if (type === 'defun' || type === 'function') {
      var scope = scopes.pop();
      var names = set();
      for (name in scope) {
        var data = scope[name];
        if (data.use && !data.def) {
          // this is used from a higher scope, propagate the use down 
          ensureData(scopes[scopes.length-1], name).use = 1;
          continue;
        }
        if (data.def && !data.use && !data.param) {
          // this is eliminateable!
          names[name] = 0;
        }
      }
      cleanUp(node[3], names);
    }
  });
  // toplevel
  var scope = scopes.pop();
  assert(scopes.length === 0);

  var names = set();
  for (var name in scope) {
    var data = scope[name];
    if (data.def && !data.use) {
      assert(!data.param); // can't be
      // this is eliminateable!
      names[name] = 0;
    }
  }
  cleanUp(ast, names);
}

function removeFuncs(ast) {
  assert(ast[0] === 'toplevel');
  var keep = set(extraInfo.keep);
  ast[1] = ast[1].filter(function(node) {
    assert(node[0] === 'defun');
    return node[1] in keep;
  });
}

// Passes table

var minifyWhitespace = false, printMetadata = true, asm = false, asmPreciseF32 = false, emitJSON = false, last = false;

var passes = {
  // passes
  dumpAst: dumpAst,
  dumpSrc: dumpSrc,
  removeAssignsToUndefined: removeAssignsToUndefined,
  //removeUnneededLabelSettings: removeUnneededLabelSettings,
  simplifyExpressions: simplifyExpressions,
  localCSE: localCSE,
  safeLabelSetting: safeLabelSetting,
  simplifyIfs: simplifyIfs,
  hoistMultiples: hoistMultiples,
  loopOptimizer: loopOptimizer,
  registerize: registerize,
  registerizeHarder: registerizeHarder,
  eliminateDeadFuncs: eliminateDeadFuncs,
  eliminateDeadGlobals: eliminateDeadGlobals,
  eliminate: eliminate,
  eliminateMemSafe: eliminateMemSafe,
  aggressiveVariableElimination: aggressiveVariableElimination,
  minifyGlobals: minifyGlobals,
  minifyLocals: minifyLocals,
  relocate: relocate,
  outline: outline,
  safeHeap: safeHeap,
  splitMemory: splitMemory,
  splitMemoryShell: splitMemoryShell,
  optimizeFrounds: optimizeFrounds,
  ensureLabelSet: ensureLabelSet,
  emterpretify: emterpretify,
  findReachable: findReachable,
  dumpCallGraph: dumpCallGraph,
  asmLastOpts: asmLastOpts,
  JSDCE: JSDCE,
  removeFuncs: removeFuncs,
  noop: function() {},

  // flags
  minifyWhitespace: function() { minifyWhitespace = true },
  noPrintMetadata: function() { printMetadata = false },
  asm: function() { asm = true },
  asmPreciseF32: function() { asmPreciseF32 = true },
  emitJSON: function() { emitJSON = true },
  receiveJSON: function() { }, // handled in a special way, before passes are run
  last: function() { last = true },
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

var emitAst = true;

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
} else {
  //print('/* not printing ast */');
}

