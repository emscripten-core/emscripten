//==============================================================================
// Optimizer tool. This is meant to be run after the emscripten compiler has
// finished generating code. These optimizations are done on the generated
// code to further improve it. Some of the modifications also work in
// conjunction with closure compiler.
//
// TODO: Optimize traverse to modify a node we want to replace, in-place,
//       instead of returning it to the previous call frame where we check?
// TODO: Share EMPTY_NODE instead of emptyNode that constructs?
//==============================================================================

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

var FUNCTION = set('defun', 'function');
var LOOP = set('do', 'while', 'for');
var LOOP_FLOW = set('break', 'continue');
var ASSIGN_OR_ALTER = set('assign', 'unary-postfix', 'unary-prefix');
var CONTROL_FLOW = set('do', 'while', 'for', 'if', 'switch');
var NAME_OR_NUM = set('name', 'num');
var ASSOCIATIVE_BINARIES = set('+', '*', '|', '&', '^');
var ALTER_FLOW = set('break', 'continue', 'return');

var BREAK_CAPTURERS = set('do', 'while', 'for', 'switch');
var CONTINUE_CAPTURERS = LOOP;

var NULL_NODE = ['name', 'null'];
var UNDEFINED_NODE = ['unary-prefix', 'void', ['num', 0]];
var TRUE_NODE = ['unary-prefix', '!', ['num', 0]];
var FALSE_NODE = ['unary-prefix', '!', ['num', 1]];

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

// Traverses the children of a node. If the traverse function returns an object,
// replaces the child. If it returns true, stop the traversal and return true.
function traverseChildren(node, traverse, pre, post, stack) {
  for (var i = 0; i < node.length; i++) {
    var subnode = node[i];
    if (Array.isArray(subnode)) {
      var subresult = traverse(subnode, pre, post, stack);
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
//   @arg stack: If true, a stack will be implemented: If pre does not push on
//               the stack, we push a 0. We pop when we leave the node. The
//               stack is passed as a third parameter to the callbacks.
//   @returns: If the root node was replaced, the new root node. If the traversal
//     was stopped, true. Otherwise undefined.
function traverse(node, pre, post, stack) {
  var type = node[0], result, len;
  var relevant = typeof type === 'string';
  if (relevant) {
    if (stack) len = stack.length;
    var result = pre(node, type, stack);
    if (result === true) return true;
    if (result && result !== null) node = result; // Continue processing on this node
    if (stack && len === stack.length) stack.push(0);
  }
  if (result !== null) {
    if (traverseChildren(node, traverse, pre, post, stack) === true) return true;
  }
  if (relevant) {
    if (post) {
      var postResult = post(node, type, stack);
      result = result || postResult;
    }
    if (stack) stack.pop();
  }
  return result;
}

// Only walk through the generated functions
function traverseGenerated(ast, pre, post, stack) {
  assert(generatedFunctions);
  traverse(ast, function(node) {
    if (node[0] === 'defun') {
      traverse(node, pre, post, stack);
      return null;
    }
  });
}

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

// Walk the ast in a simple way, with an understanding of which JS variables are defined)
function traverseWithVariables(ast, callback) {
  traverse(ast, function(node, type, stack) {
    if (type in FUNCTION) {
      stack.push({ type: 'function', vars: node[2] });
    } else if (type === 'var') {
      // Find our function, add our vars
      var func = stack[stack.length-1];
      if (func) {
        func.vars = func.vars.concat(node[1].map(function(varItem) { return varItem[0] }));
      }
    }
  }, function(node, type, stack) {
    if (type === 'toplevel' || type in FUNCTION) {
      // We know all of the variables that are seen here, proceed to do relevant replacements
      var allVars = stack.map(function(item) { return item ? item.vars : [] }).reduce(concatenator, []); // FIXME dictionary for speed?
      traverse(node, function(node2, type2, stack2) {
        // Be careful not to look into our inner functions. They have already been processed.
        if (sum(stack2) > 1 || (type === 'toplevel' && sum(stack2) === 1)) return;
        if (type2 in FUNCTION) stack2.push(1);
        return callback(node2, type2, allVars);
      }, null, []);
    }
  }, []);
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

// Passes

// Dump the AST. Useful for debugging. For example,
//  node tools/js-optimizer.js ABSOLUTE_PATH_TO_FILE dumpAst
function dumpAst(ast) {
  printErr(JSON.stringify(ast, null, '  '));
}

function dumpSrc(ast) {
  printErr(astToSrc(ast));
}

// Undos closure's creation of global variables with values true, false,
// undefined, null. These cut down on size, but do not affect gzip size
// and make JS engine's lives slightly harder (?)
function unGlobalize(ast) {

  throw 'this is deprecated!'; // and does not work with parallel compilation

  assert(ast[0] === 'toplevel');
  var values = {};
  // Find global renamings of the relevant values
  ast[1].forEach(function(node, i) {
    if (node[0] != 'var') return;
    node[1] = node[1].filter(function(varItem, j) {
      var ident = varItem[0];
      var value = varItem[1];
      if (!value) return true;
      var possible = false;
      if (jsonCompare(value, NULL_NODE) ||
          jsonCompare(value, UNDEFINED_NODE) ||
          jsonCompare(value, TRUE_NODE) ||
          jsonCompare(value, FALSE_NODE)) {
        possible = true;
      }
      if (!possible) return true;
      // Make sure there are no assignments to this variable. (This isn't fast, we traverse many times..)
      ast[1][i][1][j] = emptyNode();
      var assigned = false;
      traverseWithVariables(ast, function(node, type, allVars) {
        if (type === 'assign' && node[2][0] === 'name' && node[2][1] === ident) assigned = true;
      });
      ast[1][i][1][j] = [ident, value];
      if (!assigned) {
        values[ident] = value;
        return false;
      }
      return true;
    });

    if (node[1].length === 0) {
      ast[1][i] = emptyNode();
    }
  });
  traverseWithVariables(ast, function(node, type, allVars) {
    if (type === 'name') {
      var ident = node[1];
      if (ident in values && allVars.indexOf(ident) < 0) {
        return copy(values[ident]);
      }
    }
  });
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

// Various expression simplifications. Pre run before closure (where we still have metadata), Post run after.

var USEFUL_BINARY_OPS = set('<<', '>>', '|', '&', '^');
var COMPARE_OPS = set('<', '<=', '>', '>=', '==', '===', '!=', '!==');

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
      } else if (type === 'binary' && node[1] === '&' && node[3][0] === 'num') {
        // Rewrite (X < Y) & 1 to (X < Y)|0. (Subsequent passes will eliminate
        // the |0 if possible.)
        var input = node[2];
        var amount = node[3][1];
        if (input[0] === 'binary' && (input[1] in COMPARE_OPS) && amount == 1) {
          node[1] = '|';
          node[3][1] = 0;
        }
      }
    });
  }

  // When there is a bunch of math like (((8+5)|0)+12)|0, only the external |0 is needed, one correction is enough.
  // At each node, ((X|0)+Y)|0 can be transformed into (X+Y): The inner corrections are not needed
  // TODO: Is the same is true for 0xff, 0xffff?
  // Likewise, if we have |0 inside a block that will be >>'d, then the |0 is unnecessary because some
  // 'useful' mathops already |0 anyhow.

  function simplifyBitops(ast) {
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
        traverse(ast, function process(node, type, stack) {
          if (type === 'binary' && node[1] === '|') {
            if (node[2][0] === 'num' && node[3][0] === 'num') {
              node[2][1] |= node[3][1];
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
                return process(result, result[0], stack);
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
        }, null, []);
      }
    }

    removeMultipleOrZero();

    // & and heap-related optimizations

    var heapBits, heapUnsigned;
    function parseHeap(name) {
      if (name.substr(0, 4) != 'HEAP') return false;
      heapUnsigned = name[4] === 'U';
      heapBits = parseInt(name.substr(heapUnsigned ? 5 : 4));
      return true;
    }

    var hasTempDoublePtr = false, rerunOrZeroPass = false;

    traverse(ast, function(node, type) {
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
            if (amount === Math.pow(2, heapBits)-1) {
              if (!heapUnsigned) {
                input[1][1] = 'HEAPU' + heapBits; // make unsigned
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
          if (heapBits === 32 - amount) {
            node[2][2][1][1] = 'HEAP' + heapBits;
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
      } else if (type == 'sub' && node[1][0] == 'name' && /^FUNCTION_TABLE.*/.exec(node[1][1])) {
        return null; // do not traverse subchildren here, we should not collapse 55 & 126. TODO: optimize this into a nonvirtual call (also because we lose some other opts here)!
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
                return makeAsmCoercion(node[1][3], detectAsmCoercion(node[2]));
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
                define[3] = ['unary-prefix', '+', define[3]];
              }
              // do we want a simplifybitops on the new values here?
            });
            info.uses.forEach(function(use) {
              use[2][1][1] = correct;
            });
            asmData.vars[v] = 1 - asmData.vars[v];
          }
        }
        denormalizeAsm(ast, asmData);
      }

      // optimize num >> num, in asm we need this here since we do not run optimizeShifts
      traverse(ast, function(node, type) {
        if (type === 'binary' && node[1] === '>>' && node[2][0] === 'num' && node[3][0] === 'num') {
          node[0] = 'num';
          node[1] = node[2][1] >> node[3][1];
          node.length = 2;
        }
      });
    }
  }

  // The most common mathop is addition, e.g. in getelementptr done repeatedly. We can join all of those,
  // by doing (num+num) ==> newnum, and (name+num)+num = name+newnum
  function joinAdditions(ast) {
    var rerun = true;
    while (rerun) {
      rerun = false;
      traverse(ast, function(node, type) {
        if (type === 'binary' && node[1] === '+') {
          if (node[2][0] === 'num' && node[3][0] === 'num') {
            rerun = true;
            node[2][1] += node[3][1];
            return node[2];
          }
          for (var i = 2; i <= 3; i++) {
            var ii = 5-i;
            for (var j = 2; j <= 3; j++) {
              if (node[i][0] === 'num' && node[ii][0] === 'binary' && node[ii][1] === '+' && node[ii][j][0] === 'num') {
                rerun = true;
                node[ii][j][1] += node[i][1];
                return node[ii];
              }
            }
          }
        }
      });
    }
  }

  // if (x === 0) can be if (!x), etc.
  function simplifyZeroComp(ast) {
    traverse(ast, function(node, type) {
      var binary;
      if (type === 'if' && (binary = node[1])[0] === 'binary') {
        if ((binary[1] === '!=' || binary[1] === '!==') && binary[3][0] === 'num' && binary[3][1] === 0) {
          node[1] = binary[2];
          return node;
        } else if ((binary[1] === '==' || binary[1] === '===') && binary[3][0] === 'num' && binary[3][1] === 0) {
          node[1] = ['unary-prefix', '!', binary[2]];
          return node;
        }
      }
    });
  }

  traverseGeneratedFunctions(ast, function(func) {
    simplifyIntegerConversions(func);
    simplifyBitops(func);
    joinAdditions(func);
    // simplifyZeroComp(func); TODO: investigate performance
    simplifyNotComps(func);
  });
}

// In typed arrays mode 2, we can have
//  HEAP[x >> 2]
// very often. We can in some cases do the shift on the variable itself when it is set,
// to greatly reduce the number of shift operations.
// XXX this optimization is deprecated and currently invalid: does not handle overflows
//     or non-aligned (round numbers, x >> 2 is a multiple of 4). Both are ok to assume
//     for pointers (undefined behavior otherwise), but invalid in general, and we do
//     no sufficiently-well distinguish the cases.
function optimizeShiftsInternal(ast, conservative) {
  var MAX_SHIFTS = 3;
  traverseGeneratedFunctions(ast, function(fun) {
    var funMore = true;
    var funFinished = {};
    while (funMore) {
      funMore = false;
      // Recognize variables and parameters
      var vars = {};
      function newVar(name, param, addUse) {
        if (!vars[name]) {
          vars[name] = {
            param: param,
            defs: addUse ? 1 : 0,
            uses: 0,
            timesShifted: [0, 0, 0, 0], // zero shifts of size 0, 1, 2, 3
            benefit: 0,
            primaryShift: -1
          };
        }
      }
      // params
      if (fun[2]) {
        fun[2].forEach(function(arg) {
          newVar(arg, true, true);
        });
      }
      // vars
      // XXX if var has >>=, ignore it here? That means a previous pass already optimized it
      var hasSwitch = traverse(fun, function(node, type) {
        if (type === 'var') {
          node[1].forEach(function(arg) {
            newVar(arg[0], false, arg[1]);
          });
        } else if (type === 'switch') {
          // The relooper can't always optimize functions, and we currently don't work with
          // switch statements when optimizing shifts. Bail.
          return true;
        }
      });
      if (hasSwitch) {
        break;
      }
      // uses and defs TODO: weight uses by being inside a loop (powers). without that, we
      // optimize for code size, not speed.
      traverse(fun, function(node, type, stack) {
        stack.push(node);
        if (type === 'name' && vars[node[1]] && stack[stack.length-2][0] != 'assign') {
          vars[node[1]].uses++;
        } else if (type === 'assign' && node[2][0] === 'name' && vars[node[2][1]]) {
          vars[node[2][1]].defs++;
        }
      }, null, []);
      // First, break up elements inside a shift. This lets us see clearly what to do next.
      traverse(fun, function(node, type) {
        if (type === 'binary' && node[1] === '>>' && node[3][0] === 'num') {
          var shifts = node[3][1];
          if (shifts <= MAX_SHIFTS) {
            // Push the >> inside the value elements
            function addShift(subNode) {
              if (subNode[0] === 'binary' && subNode[1] === '+') {
                subNode[2] = addShift(subNode[2]);
                subNode[3] = addShift(subNode[3]);
                return subNode;
              }
              if (subNode[0] === 'name' && !subNode[2]) { // names are returned with a shift, but we also note their being shifted
                var name = subNode[1];
                if (vars[name]) {
                  vars[name].timesShifted[shifts]++;
                  subNode[2] = true;
                }
              }
              return ['binary', '>>', subNode, ['num', shifts]];
            }
            return addShift(node[2]);
          }
        }
      });
      traverse(fun, function(node, type) {
        if (node[0] === 'name' && node[2]) {
          return node.slice(0, 2); // clean up our notes
        }
      });
      // At this point, shifted expressions are split up, and we know who the vars are and their info, so we can decide
      // TODO: vars that depend on other vars
      for (var name in vars) {
        var data = vars[name];
        var totalTimesShifted = sum(data.timesShifted);
        if (totalTimesShifted === 0) {
          continue;
        }
        if (totalTimesShifted != Math.max.apply(null, data.timesShifted)) {
          // TODO: Handle multiple different shifts
          continue;
        }
        if (funFinished[name]) continue;
        // We have one shift size (and possible unshifted uses). Consider replacing this variable with a shifted clone. If
        // the estimated benefit is >0, we will do it
        if (data.defs === 1) {
          data.benefit = totalTimesShifted - 2*(data.defs + (data.param ? 1 : 0));
        }
        if (conservative) data.benefit = 0;
        if (data.benefit > 0) {
          funMore = true; // We will reprocess this function
          for (var i = 0; i < 4; i++) {
            if (data.timesShifted[i]) {
              data.primaryShift = i;
            }
          }
        }
      }
      //printErr(JSON.stringify(vars));
      function cleanNotes() { // We need to mark 'name' nodes as 'processed' in some passes here; this cleans the notes up
        traverse(fun, function(node, type) {
          if (node[0] === 'name' && node[2]) {
            return node.slice(0, 2);
          }
        });
      }
      cleanNotes();
      // Apply changes
      function needsShift(name) {
        return vars[name] && vars[name].primaryShift >= 0;
      }
      for (var name in vars) { // add shifts for params and var's for all new variables
        var data = vars[name];
        if (needsShift(name)) {
          if (data.param) {
            fun[3].unshift(['var', [[name + '$s' + data.primaryShift, ['binary', '>>', ['name', name], ['num', data.primaryShift]]]]]);
          } else {
            fun[3].unshift(['var', [[name + '$s' + data.primaryShift]]]);
          }
        }
      }
      traverse(fun, function(node, type, stack) { // add shift to assignments
        stack.push(node);
        if (node[0] === 'assign' && node[1] === true && node[2][0] === 'name' && needsShift(node[2][1]) && !node[2][2]) {
          var name = node[2][1];
          var data = vars[name];
          var parent = stack[stack.length-3];
          var statements = getStatements(parent);
          assert(statements, 'Invalid parent for assign-shift: ' + dump(parent));
          var i = statements.indexOf(stack[stack.length-2]);
          statements.splice(i+1, 0, ['stat', ['assign', true, ['name', name + '$s' + data.primaryShift], ['binary', '>>', ['name', name, true], ['num', data.primaryShift]]]]);
        } else if (node[0] === 'var') {
          var args = node[1];
          for (var i = 0; i < args.length; i++) {
            var arg = args[i];
            var name = arg[0];
            var data = vars[name];
            if (arg[1] && needsShift(name)) {
              args.splice(i+1, 0, [name + '$s' + data.primaryShift, ['binary', '>>', ['name', name, true], ['num', data.primaryShift]]]);
            }
          }
          return node;
        }
      }, null, []);
      cleanNotes();
      traverse(fun, function(node, type, stack) { // replace shifted name with new variable
        stack.push(node);
        if (node[0] === 'binary' && node[1] === '>>' && node[2][0] === 'name' && needsShift(node[2][1]) && node[3][0] === 'num') {
          var name = node[2][1];
          var data = vars[name];
          var parent = stack[stack.length-2];
          // Don't modify in |x$sN = x >> 2|, in normal assigns and in var assigns
          if (parent[0] === 'assign' && parent[2][0] === 'name' && parent[2][1] === name + '$s' + data.primaryShift) return;
          if (parent[0] === name + '$s' + data.primaryShift) return;
          if (node[3][1] === data.primaryShift) {
            return ['name', name + '$s' + data.primaryShift];
          }
        }
      }, null, []);
      cleanNotes();
      var SIMPLE_SHIFTS = set('<<', '>>');
      var more = true;
      while (more) { // combine shifts in the same direction as an optimization
        more = false;
        traverse(fun, function(node, type) {
          if (node[0] === 'binary' && node[1] in SIMPLE_SHIFTS && node[2][0] === 'binary' && node[2][1] === node[1] &&
              node[3][0] === 'num' && node[2][3][0] === 'num') { // do not turn a << b << c into a << b + c; while logically identical, it is slower
            more = true;
            return ['binary', node[1], node[2][2], ['num', node[3][1] + node[2][3][1]]];
          }
        });
      }
      // Before recombining, do some additional optimizations
      traverse(fun, function(node, type) {
        // Apply constant shifts onto constants
        if (type === 'binary' && node[1] === '>>' && node[2][0] === 'num' && node[3][0] === 'num' && node[3][1] <= MAX_SHIFTS) {
          var subNode = node[2];
          var shifts = node[3][1];
          var result = subNode[1] / Math.pow(2, shifts);
          if (result % 1 === 0) {
            subNode[1] = result;
            return subNode;
          }
        }
        // Optimize the case of ($a*80)>>2 into ($a*20)|0
        if (type === 'binary' && node[1] in SIMPLE_SHIFTS &&
            node[2][0] === 'binary' && node[2][1] === '*') {
          var mulNode = node[2];
          if (mulNode[2][0] === 'num') {
            var temp = mulNode[2];
            mulNode[2] = mulNode[3];
            mulNode[3] = temp;
          }
          if (mulNode[3][0] === 'num') {
            if (node[1] === '<<') {
              mulNode[3][1] *= Math.pow(2, node[3][1]);
              node[1] = '|';
              node[3][1] = 0;
              return node;
            } else {
              if (mulNode[3][1] % Math.pow(2, node[3][1]) === 0) {
                mulNode[3][1] /= Math.pow(2, node[3][1]);
                node[1] = '|';
                node[3][1] = 0;
                return node;
              }
            }
          }
        }
        /* XXX - theoretically useful optimization(s), but commented out as not helpful in practice
        // Transform (x << 2) >> 2 into x & mask or something even simpler
        if (type === 'binary'       && node[1]    === '>>' && node[3][0] === 'num' &&
            node[2][0] === 'binary' && node[2][1] === '<<' && node[2][3][0] === 'num' && node[3][1] === node[2][3][1]) {
          var subNode = node[2];
          var shifts = node[3][1];
          var mask = ((0xffffffff << shifts) >>> shifts) | 0;
          return ['binary', '&', subNode[2], ['num', mask]];
          //return ['binary', '|', subNode[2], ['num', 0]];
          //return subNode[2];
        }
        */
      });
      // Re-combine remaining shifts, to undo the breaking up we did before. may require reordering inside +'s
      traverse(fun, function(node, type, stack) {
        stack.push(node);
        if (type === 'binary' && node[1] === '+' && (stack[stack.length-2][0] != 'binary' || stack[stack.length-2][1] !== '+')) {
          // 'Flatten' added items
          var addedItems = [];
          function flatten(node) {
            if (node[0] === 'binary' && node[1] === '+') {
              flatten(node[2]);
              flatten(node[3]);
            } else {
              addedItems.push(node);
            }
          }
          flatten(node);
          var originalOrder = addedItems.slice();
          function key(node) { // a unique value for all relevant shifts for recombining, non-unique for stuff we don't need to bother with
            function originalOrderKey(item) {
              return -originalOrder.indexOf(item);
            }
            if (node[0] === 'binary' && node[1] in SIMPLE_SHIFTS) {
              if (node[3][0] === 'num' && node[3][1] <= MAX_SHIFTS) return 2*node[3][1] + (node[1] === '>>' ? 100 : 0); // 0-106
              return (node[1] === '>>' ? 20000 : 10000) + originalOrderKey(node);
            }
            if (node[0] === 'num') return -20000 + node[1];
            return -10000 + originalOrderKey(node); // Don't modify the original order if we don't modify anything
          }
          for (var i = 0; i < addedItems.length; i++) {
            if (addedItems[i][0] === 'string') return; // this node is not relevant for us
          }
          addedItems.sort(function(node1, node2) {
            return key(node1) - key(node2);
          });
          // Regenerate items, now sorted
          var i = 0;
          while (i < addedItems.length-1) { // re-combine inside addedItems
            var k = key(addedItems[i]), k1 = key(addedItems[i+1]);
            if (k === k1 && k >= 0 && k1 <= 106) {
              addedItems[i] = ['binary', addedItems[i][1], ['binary', '+', addedItems[i][2], addedItems[i+1][2]], addedItems[i][3]];
              addedItems.splice(i+1, 1);
            } else {
              i++;
            }
          }
          var num = 0;
          for (i = 0; i < addedItems.length; i++) { // combine all numbers into one
            if (addedItems[i][0] === 'num') {
              num += addedItems[i][1];
              addedItems.splice(i, 1);
              i--;
            }
          }
          if (num != 0) { // add the numbers into an existing shift, we 
                          // prefer (x+5)>>7 over (x>>7)+5 , since >>'s result is known to be 32-bit and is more easily optimized.
                          // Also, in the former we can avoid the parentheses, which saves a little space (the number will be bigger,
                          // so it might take more space, but normally at most one more digit).
            var added = false;
            for (i = 0; i < addedItems.length; i++) {
              if (addedItems[i][0] === 'binary' && addedItems[i][1] === '>>' && addedItems[i][3][0] === 'num' && addedItems[i][3][1] <= MAX_SHIFTS) {
                addedItems[i] = ['binary', '>>', ['binary', '+', addedItems[i][2], ['num', num << addedItems[i][3][1]]], addedItems[i][3]];
                added = true;
              }
            }
            if (!added) {
              addedItems.unshift(['num', num]);
            }
          }
          var ret = addedItems.pop();
          while (addedItems.length > 0) { // re-create AST from addedItems
            ret = ['binary', '+', ret, addedItems.pop()];
          }
          return ret;
        }
      }, null, []);
      // Note finished variables
      for (var name in vars) {
        funFinished[name] = true;
      }
    }
  });
}

function optimizeShiftsConservative(ast) {
  optimizeShiftsInternal(ast, true);
}

function optimizeShiftsAggressive(ast) {
  optimizeShiftsInternal(ast, false);
}

// We often have branchings that are simplified so one end vanishes, and
// we then get
//   if (!(x < 5))
// or such. Simplifying these saves space and time.
function simplifyNotCompsDirect(node) {
  if (node[0] === 'unary-prefix' && node[1] === '!') {
    if (node[2][0] === 'binary') {
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

var simplifyNotCompsPass = false;

function simplifyNotComps(ast) {
  simplifyNotCompsPass = true;
  traverse(ast, simplifyNotCompsDirect);
  simplifyNotCompsPass = false;
}

function callHasSideEffects(node) { // checks if the call itself (not the args) has side effects (or is not statically known)
  return !(node[1][0] === 'name' && /^Math_/.test(node[1][1]));
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
    default: return true;
  }
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
          node[1] = simplifyNotCompsDirect(['unary-prefix', '!', node[1]]);
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

function detectAsmCoercion(node, asmInfo) {
  // for params, +x vs x|0, for vars, 0.0 vs 0
  if (node[0] === 'num' && node[1].toString().indexOf('.') >= 0) return ASM_DOUBLE;
  if (node[0] === 'unary-prefix') return ASM_DOUBLE;
  if (node[0] === 'call' && node[1][0] === 'name' && node[1][1] === 'Math_fround') return ASM_FLOAT;
  if (asmInfo && node[0] == 'name') return getAsmType(node[1], asmInfo);
  return ASM_INT;
}

function makeAsmCoercion(node, type) {
  switch (type) {
    case ASM_INT: return ['binary', '|', node, ['num', 0]];
    case ASM_DOUBLE: return ['unary-prefix', '+', node];
    case ASM_FLOAT: return ['call', ['name', 'Math_fround'], [node]];
    default: throw 'wha? ' + JSON.stringify([node, type]) + new Error().stack;
  }
}

function makeAsmVarDef(v, type) {
  switch (type) {
    case ASM_INT: return [v, ['num', 0]];
    case ASM_DOUBLE: return [v, ['unary-prefix', '+', ['num', 0]]];
    case ASM_FLOAT: return [v, ['call', ['name', 'Math_fround'], [['num', 0]]]];
    default: throw 'wha?';
  }
}

function getAsmType(name, asmInfo) {
  if (name in asmInfo.vars) return asmInfo.vars[name];
  if (name in asmInfo.params) return asmInfo.params[name];
  assert(false, 'unknown var ' + name);
}

function normalizeAsm(func) {
  //printErr('pre-normalize \n\n' + astToSrc(func) + '\n\n');
  var data = {
    params: {}, // ident => ASM_* type
    vars: {}, // ident => ASM_* type
    inlines: [], // list of inline assembly copies
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
    data.params[name] = detectAsmCoercion(node[3]);
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
        assert(value[0] === 'num' || (value[0] === 'unary-prefix' && value[2][0] === 'num') // must be valid coercion no-op
                                  || (value[0] === 'call' && value[1][0] === 'name' && value[1][1] === 'Math_fround'));
        data.vars[name] = detectAsmCoercion(value);
        v.length = 1; // make an un-assigning var
      } else {
        assert(j === 0, 'cannot break in the middle');
        break outer;
      }
    }
    i++;
  }
  // finally, look for other var definitions and collect them
  while (i < stats.length) {
    traverse(stats[i], function(node, type) {
      if (type === 'var') {
        assert(0, 'should be no vars to fix! ' + func[1] + ' : ' + JSON.stringify(node));
        /*
        for (var j = 0; j < node[1].length; j++) {
          var v = node[1][j];
          var name = v[0];
          var value = v[1];
          if (!(name in data.vars)) {
            if (value[0] != 'name') {
              data.vars[name] = detectAsmCoercion(value); // detect by coercion
            } else {
              var origin = value[1];
              data.vars[name] = data.vars[origin] || ASM_INT; // detect by origin variable, or assume int for non-locals
            }
          }
        }
        unVarify(node[1], node);
        */
      } else if (type === 'call' && node[1][0] === 'function') {
        assert(!node[1][1]); // anonymous functions only
        data.inlines.push(node[1]);
        node[1] = ['name', 'inlinejs']; // empty out body, leave arguments, so they are eliminated/minified properly
      }
    });
    i++;
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
  // each param needs a line; reuse emptyNodes as much as we can
  var numParams = 0;
  for (var i in data.params) numParams++;
  var emptyNodes = 0;
  while (emptyNodes < stats.length) {
    if (!isEmptyNode(stats[emptyNodes])) break;
    emptyNodes++;
  }
  var neededEmptyNodes = numParams + 1; // params plus one big var
  if (neededEmptyNodes > emptyNodes) {
    var args = [0, 0];
    for (var i = 0; i < neededEmptyNodes - emptyNodes; i++) args[i+2] = 0;
    stats.splice.apply(stats, args);
  }
  // add param coercions
  var next = 0;
  func[2].forEach(function(param) {
    stats[next++] = ['stat', ['assign', true, ['name', param], makeAsmCoercion(['name', param], data.params[param])]];
  });
  // add variable definitions
  var varDefs = [];
  for (var v in data.vars) {
    varDefs.push(makeAsmVarDef(v, data.vars[v]));
  }
  if (varDefs.length) {
    stats[next] = ['var', varDefs];
  } else {
    stats[next] = emptyNode();
  }
  if (data.inlines.length > 0) {
    var i = 0;
    traverse(func, function(node, type) {
      if (type === 'call' && node[1][0] === 'name' && node[1][1] === 'inlinejs') {
        node[1] = data.inlines[i++]; // swap back in the body
      }
    });
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
      assert(value[0] == 'binary' && value[1] == '|' && value[2][0] == 'binary' && value[2][1] == '+' && value[2][2][0] == 'name' && value[2][2][1] == 'STACKTOP' && value[2][3][0] == 'num');
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

// Very simple 'registerization', coalescing of variables into a smaller number,
// as part of minification. Globals-level minification began in a previous pass,
// we receive extraInfo which tells us how to rename globals. (Only in asm.js.)
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
      }
    });
    vacuum(fun);
    if (extraInfo && extraInfo.globals) {
      assert(asm);
      var usedGlobals = {};
      var nextLocal = 0;
      // Minify globals using the mapping we were given
      traverse(fun, function(node, type) {
        if (type === 'name') {
          var name = node[1];
          var minified = extraInfo.globals[name];
          if (minified) {
            assert(!localVars[name], name); // locals must not shadow globals, or else we don't know which is which
            if (localVars[minified]) {
              // trying to minify a global into a name used locally. rename all the locals
              var newName = '$_newLocal_' + (nextLocal++);
              assert(!localVars[newName]);
              if (params[minified]) {
                params[newName] = 1;
                delete params[minified];
              }
              localVars[newName] = 1;
              delete localVars[minified];
              asmData.vars[newName] = asmData.vars[minified];
              delete asmData.vars[minified];
              asmData.params[newName] = asmData.params[minified];
              delete asmData.params[minified];
              traverse(fun, function(node, type) {
                if (type === 'name' && node[1] === minified) {
                  node[1] = newName;
                }
              });
              if (fun[2]) {
                for (var i = 0; i < fun[2].length; i++) {
                  if (fun[2][i] === minified) fun[2][i] = newName;
                }
              }
            }
            node[1] = minified;
            usedGlobals[minified] = 1;
          }
        }
      });
      if (fun[1] in extraInfo.globals) { // if fun was created by a previous optimization pass, it will not be here
        fun[1] = extraInfo.globals[fun[1]];
        assert(fun[1]);
      }
      var nextRegName = 0;
    }
    var regTypes = {};
    function getNewRegName(num, name) {
      if (!asm) return 'r' + num;
      var type = asmData.vars[name];
      if (!extraInfo || !extraInfo.globals) {
        var ret = (type ? 'd' : 'i') + num;
        regTypes[ret] = type;
        return ret;
      }
      // find the next free minified name that is not used by a global that shows up in this function
      while (nextRegName < extraInfo.names.length) {
        var ret = extraInfo.names[nextRegName++];
        if (!usedGlobals[ret]) {
          regTypes[ret] = type;
          return ret;
        }
      }
      assert('ran out of names');
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
    var freeRegsClasses = asm ? [[], [], []] : []; // two classes for asm, one otherwise XXX - hardcoded length
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
      if (extraInfo && extraInfo.globals) {
        // minify in asm var definitions, that denormalizeAsm just generated
        function minify(value) {
          if (value && value[0] === 'call' && value[1][0] === 'name') {
            var name = value[1][1];
            var minified = extraInfo.globals[name];
            if (minified) {
              value[1][1] = minified;
            }
          }
        }
        var stats = fun[3];
        for (var i = 0; i < stats.length; i++) {
          var line = stats[i];
          if (i >= fun[2].length && line[0] !== 'var') break; // when we pass the arg and var coercions, break
          if (line[0] === 'stat') {
            assert(line[1][0] === 'assign');
            minify(line[1][3]);
          } else {
            assert(line[0] === 'var');
            var pairs = line[1];
            for (var j = 0; j < pairs.length; j++) {
              minify(pairs[j][1]);
            }
          }
        }
      }
    }
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
      var usesGlobals = false, usesMemory = false, deps = {}, doesCall = false;
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
        //printErr(spaces(2*(nesting+1)) + 'trav: ' + JSON.stringify(node).substr(0, 50) + ' : ' + keys(tracked) + ' : ' + [allowTracking, ignoreSub, ignoreName]);
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
            } else if (!(name in locals) && !callsInvalidated) {
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
          var cases = node[2];
          for (var i = 0; i < cases.length; i++) {
            var c = cases[i];
            assert(c[0] === null || c[0][0] === 'num' || (c[0][0] === 'unary-prefix' && c[0][2][0] === 'num'));
            var stats = c[1];
            for (var j = 0; j < stats.length; j++) {
              traverseInOrder(stats[j]);
            }
          }
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
        // try to remove loop helper variables specifically
        var stats = node[2][1];
        var last = stats[stats.length-1];
        if (last && last[0] === 'if' && last[2][0] === 'block' && last[3] && last[3][0] === 'block') {
          var ifTrue = last[2];
          var ifFalse = last[3];
          var flip = false;
          if (ifFalse[1][0] && ifFalse[1][0][0] === 'break') { // canonicalize break in the if
            var temp = ifFalse;
            ifFalse = ifTrue;
            ifTrue = temp;
            flip = true;
          }
          if (ifTrue[1][0] && ifTrue[1][0][0] === 'break') {
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
            if (loopers.length < assigns.length) return; // TODO: handle the case where can can just eliminate one. (we can't optimize the break, but we can remove the var at least)
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
              var looperUsed = false;
              for (var i = found+1; i < stats.length && !looperUsed; i++) {
                var curr = i < stats.length-1 ? stats[i] : last[1]; // on the last line, just look in the condition
                traverse(curr, function(node, type) {
                  if (type === 'name' && node[1] === looper) {
                    looperUsed = true;
                    return true;
                  }
                });
              }
              if (looperUsed) return;
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
              last[2] = last[3];
            }
            last.pop();
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
    if (type === 'var') {
      if (first) {
        first = false;
        return;
      }
      var vars = node[1];
      for (var i = 0; i < vars.length; i++) {
        var name = vars[i][0];
        assert(next < extraInfo.names.length);
        vars[i][0] = minified[name] = extraInfo.names[next++];
      }
    }
  });
  // add all globals in function chunks, i.e. not here but passed to us
  for (var i = 0; i < extraInfo.globals.length; i++) {
    name = extraInfo.globals[i];
    assert(next < extraInfo.names.length);
    minified[name] = extraInfo.names[next++];
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

function outline(ast) {
  function measureSize(ast) {
    var size = 0;
    traverse(ast, function() {
      size++;
    });
    return size;
  }

  function aggressiveVariableElimination(func, asmData) {
    // This removes as many variables as possible. This is often not the best thing because it increases
    // code size, but it is far preferable to the risk of split functions needing to do more spilling. Specifically,
    // it finds 'trivial' variables: ones with 1 definition, and that definition is not sensitive to any changes: it
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

    var values = {};

    function evaluate(name) {
      var node = values[name];
      if (node) return node;
      values[node] = null; // prevent infinite recursion
      var def = defs[name];
      if (def) {
        node = def[3];
        if (node[0] == 'name') {
          var name2 = node[1];
          if (name2 in trivials) {
            node = evaluate(name2);
          }
        } else {
          traverse(node, function(node, type) {
            if (type == 'name') {
              var name2 = node[1];
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
          if (!value) throw 'missing value: ' + [func[1], name, values[name]] + ' - faulty reliance on asm zero-init?';
          return copy(value); // must copy, or else the same object can be used multiple times
        }
      }
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
    if (stackSize % 8 === 0) stackSize += 8 - (stackSize % 8);
    for (var i = 0; i < stack.length; i++) {
      asmData.stackPos[stack[i]] = stackSize + i*8;
    }
    // Reserve an extra two spots per possible outlining: one for control flow var, the other for control flow data
    // The control variables are zeroed out when calling an outlined function, and after using
    // the value after they return.
    var size = measureSize(func);
    asmData.maxOutlinings = Math.min(Math.round(3*size/extraInfo.sizeToOutline), maxTotalOutlinings);
    asmData.intendedPieces = Math.ceil(size/extraInfo.sizeToOutline);
    asmData.totalStackSize = stackSize + (stack.length + 2*asmData.maxOutlinings)*8;
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
          hasReturnType[detectAsmCoercion(node[1])] = true;
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
    return ['sub', ['name', type == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', pos]], ['num', '2']]];
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
      printErr('  no point in trying to reduce the size of ' + func[1] + ' which is ' + size + ' <= ' + extraInfo.sizeToOutline);
    } else {
      sizeToOutline = Math.round(size/Math.max(2, asmData.intendedPieces--));
      printErr('trying to reduce the size of ' + func[1] + ' which is ' + size + ' (>=? ' + extraInfo.sizeToOutline + '), aim for ' + sizeToOutline);
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
        reps.push(['stat', ['assign', true, ['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', '2']]], ['name', v]]]);
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
        reps.push(['stat', ['assign', true, ['name', v], makeAsmCoercion(['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', '2']]], getAsmType(v, asmData))]]);
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
                var type = detectAsmCoercion(node[1], asmData);
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
          [['stat', ['return']]]
        ));
      }
      for (var returnType in codeInfo.hasReturnType) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', controlFromAsmType(returnType)]),
          [['stat', ['return', makeAsmCoercion(['name', 'tempInt'], returnType | 0)]]]
        ));
      }
      if (codeInfo.hasBreak) {
        reps.push(makeIf(
          makeComparison(makeAsmCoercion(['name', 'tempValue'], ASM_INT), '==', ['num', CONTROL_BREAK]),
          [['stat', ['break']]]
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
          [['stat', ['continue']]]
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
        code.unshift(['stat', ['assign', true, ['name', v], makeAsmCoercion(['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', '2']]], getAsmType(v, asmData))]]);
      }
    });
    sortedWrites.forEach(function(v) {
      if (!(v in owned)) {
        code.push(['stat', ['assign', true, ['sub', ['name', getAsmType(v, asmData) == ASM_INT ? 'HEAP32' : 'HEAPF32'], ['binary', '>>', ['binary', '+', ['name', 'sp'], ['num', asmData.stackPos[v]]], ['num', '2']]], ['name', v]]]);
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
    printErr('performed outline ' + [func[1], newIdent, 'pre size', originalCodeSize, 'resulting size', measureSize(code), 'overhead (w/r):', setSize(setSub(codeInfo.writes, owned)), setSize(setSub(codeInfo.reads, owned)), ' owned: ', setSize(owned), ' left: ', setSize(asmData.vars), setSize(asmData.params), ' loopsDepth: ', loops]);
    calculateThreshold(func, asmData);
    return [newFunc];
  }

  function outlineStatements(func, asmData, stats, maxSize) {
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
          if (stat[0] == 'assign' && stat[2][0] == 'name' && stat[2][1] == 'sp') minIndex = i+1; // cannot outline |sp = |
        }
      }
    }
    function done() {
      return asmData.splitCounter >= asmData.maxOutlinings || measureSize(func) <= extraInfo.sizeToOutline;
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

  printErr('\n');

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
        aggressiveVariableElimination(func, asmData);
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
            if (stackBumpNode) stackBumpNode[3][2][3][1] = asmData.totalStackSize;
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
          printErr('... resulting sizes of ' + func[1] + ' is ' + ret.map(measureSize) + '\n');
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
  return js.replace(/DOT\$ZERO\(([-+]?(0x)?[0-9a-f]*\.?[0-9]+([eE][-+]?[0-9]+)?)\)/g, function(m, num) {
    if (num.substr(0, 2) === '0x' || num.substr(0, 3) === '-0x') {
      return eval(num) + '.0';
    }
    if (num.indexOf('.') >= 0) return num;
    var e = num.indexOf('e');
    if (e < 0) return num + '.0';
    return num.substr(0, e) + '.0' + num.substr(e);
  });
}

function asmLastOpts(ast) {
  traverseGeneratedFunctions(ast, function(fun) {
    traverse(fun, function(node, type) {
      if (type === 'while' && node[1][0] === 'num' && node[1][1] === 1 && node[2][0] === 'block') {
        // This is at the end of the pipeline, we can assume all other optimizations are done, and we modify loops
        // into shapes that might confuse other passes

        // while (1) { .. if (..) { break } } ==> do { .. } while(..)
        var stats = node[2][1];
        var last = stats[stats.length-1];
        if (last && last[0] === 'if' && !last[3] && last[2][0] === 'block' && last[2][1][0] && last[2][1][0][0] === 'break' && !last[2][1][0][1]) {
          var abort = false;
          var stack = 0;
          traverse(stats, function(node, type) {
            if (type == 'continue') {
              if (stack == 0 || node[1]) { // abort if labeled (we do not analyze labels here yet), or a continue directly on us
                abort = true;
                return true;
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
          var conditionToBreak = last[1];
          stats.pop();
          node[0] = 'do';
          node[1] = simplifyNotCompsDirect(['unary-prefix', '!', conditionToBreak]);
          return node;
        }
      } else if (type == 'binary') {
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
    });
  });
}

// Passes table

var minifyWhitespace = false, printMetadata = true, asm = false, last = false;

var passes = {
  dumpAst: dumpAst,
  dumpSrc: dumpSrc,
  unGlobalize: unGlobalize,
  removeAssignsToUndefined: removeAssignsToUndefined,
  //removeUnneededLabelSettings: removeUnneededLabelSettings,
  simplifyExpressions: simplifyExpressions,
  optimizeShiftsConservative: optimizeShiftsConservative,
  optimizeShiftsAggressive: optimizeShiftsAggressive,
  hoistMultiples: hoistMultiples,
  loopOptimizer: loopOptimizer,
  registerize: registerize,
  eliminate: eliminate,
  eliminateMemSafe: eliminateMemSafe,
  minifyGlobals: minifyGlobals,
  relocate: relocate,
  outline: outline,
  minifyWhitespace: function() { minifyWhitespace = true },
  noPrintMetadata: function() { printMetadata = false },
  asm: function() { asm = true },
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
var ast = srcToAst(src);
//printErr(JSON.stringify(ast)); throw 1;
generatedFunctions = src.lastIndexOf(GENERATED_FUNCTIONS_MARKER) >= 0;
var extraInfoStart = src.lastIndexOf('// EXTRA_INFO:')
if (extraInfoStart > 0) extraInfo = JSON.parse(src.substr(extraInfoStart + 14));
//printErr(JSON.stringify(extraInfo));


arguments_.slice(1).forEach(function(arg) {
  passes[arg](ast);
});
if (asm && last) {
  asmLastOpts(ast); // TODO: move out of last, to make last faster when done later (as in side modules)
  prepDotZero(ast);
}
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

