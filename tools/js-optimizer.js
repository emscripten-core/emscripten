//==============================================================================
//  Optimizer tool. This is meant to be run after the emscripten compiler has
//  finished generating code. These optimizations are done on the generated
//  code to further improve it. Some of the modifications also work in
//  conjunction with closure compiler.
//==============================================================================

var uglify = require('../tools/eliminator/node_modules/uglify-js');
var fs = require('fs');

// Make node environment compatible with JS shells

function print(text) {
  process.stdout.write(text + '\n');
}
function printErr(text) {
  process.stderr.write(text + '\n');
}
function read(filename) {
  if (filename[0] != '/') filename = __dirname.split('/').slice(0, -1).join('/') + '/src/' + filename;
  return fs.readFileSync(filename).toString();
}
var arguments = process.argv.slice(2);

// Load some modules

eval(read('utility.js'));

// Utilities

var FUNCTION = set('defun', 'function');
var LOOP = set('do', 'while', 'for');
var LOOP_FLOW = set('break', 'continue');

var NULL_NODE = ['name', 'null'];
var UNDEFINED_NODE = ['unary-prefix', 'void', ['num', 0]];
var TRUE_NODE = ['unary-prefix', '!', ['num', 0]];
var FALSE_NODE = ['unary-prefix', '!', ['num', 1]];

var GENERATED_FUNCTIONS_MARKER = '// EMSCRIPTEN_GENERATED_FUNCTIONS:';
var generatedFunctions = null;
function setGeneratedFunctions(metadata) {
  generatedFunctions = set(eval(metadata.replace(GENERATED_FUNCTIONS_MARKER, '')));
}
function isGenerated(ident) {
  return ident in generatedFunctions;
}

function srcToAst(src) {
  return uglify.parser.parse(src);
}

function astToSrc(ast) {
    return uglify.uglify.gen_code(ast, {
    ascii_only: true,
    beautify: true,
    indent_level: 2
  });
}

// Traverses a JavaScript syntax tree rooted at the given node calling the given
// callback for each node.
//   @arg node: The root of the AST.
//   @arg pre: The pre to call for each node. This will be called with
//     the node as the first argument and its type as the second. If true is
//     returned, the traversal is stopped. If an object is returned,
//     it replaces the passed node in the tree.
//   @arg post: A callback to call after traversing all children.
//   @arg stack: If true, a stack will be implemented: If pre does not push on
//               the stack, we push a 0. We pop when we leave the node. The
//               stack is passed as a third parameter to the callbacks.
//   @returns: If the root node was replaced, the new root node. If the traversal
//     was stopped, true. Otherwise undefined.
function traverse(node, pre, post, stack) {
  var type = node[0], result, len;
  var relevant = typeof type == 'string';
  if (relevant) {
    if (stack) len = stack.length;
    var result = pre(node, type, stack);
    if (result == true) return true;
    if (typeof result == 'object') node = result; // Continue processing on this node
    if (stack && len == stack.length) stack.push(0);
  }
  for (var i = 0; i < node.length; i++) {
    var subnode = node[i];
    if (typeof subnode == 'object' && subnode && subnode.length) {
      var subresult = traverse(subnode, pre, post, stack);
      if (subresult == true) return true;
      if (typeof subresult == 'object') node[i] = subresult;
    }
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
  ast[1].forEach(function(node, i) {
    if (node[0] == 'defun' && isGenerated(node[1])) {
      traverse(node, pre, post, stack);
    }
  });
}

function traverseGeneratedFunctions(ast, callback) {
  ast[1].forEach(function(node, i) {
    if (node[0] == 'defun' && isGenerated(node[1])) {
      callback(node);
    }
  });
}

// Walk the ast in a simple way, with an understanding of which JS variables are defined)
function traverseWithVariables(ast, callback) {
  traverse(ast, function(node, type, stack) {
    if (type in FUNCTION) {
      stack.push({ type: 'function', vars: node[2] });
    } else if (type == 'var') {
      // Find our function, add our vars
      var func = stack[stack.length-1];
      if (func) {
        func.vars = func.vars.concat(node[1].map(function(varItem) { return varItem[0] }));
      }
    }
  }, function(node, type, stack) {
    if (type == 'toplevel' || type in FUNCTION) {
      // We know all of the variables that are seen here, proceed to do relevant replacements
      var allVars = stack.map(function(item) { return item ? item.vars : [] }).reduce(concatenator, []); // FIXME dictionary for speed?
      traverse(node, function(node2, type2, stack2) {
        // Be careful not to look into our inner functions. They have already been processed.
        if (sum(stack2) > 1 || (type == 'toplevel' && sum(stack2) == 1)) return;
        if (type2 in FUNCTION) stack2.push(1);
        return callback(node2, type2, allVars);
      }, null, []);
    }
  }, []);
}

function emptyNode() {
  return ['toplevel', []]
}

// Passes

// Dump the AST. Useful for debugging. For example,
//  echo "HEAP[(a+b+c)>>2]" | node tools/js-optimizer.js dump
function dumpAst(ast) {
  printErr(JSON.stringify(ast));
}

function dumpSrc(ast) {
  printErr(astToSrc(ast));
}

// Undos closure's creation of global variables with values true, false,
// undefined, null. These cut down on size, but do not affect gzip size
// and make JS engine's lives slightly harder (?)
function unGlobalize(ast) {
  assert(ast[0] == 'toplevel');
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
        if (type == 'assign' && node[2][0] == 'name' && node[2][1] == ident) assigned = true;
      });
      ast[1][i][1][j] = [ident, value];
      if (!assigned) {
        values[ident] = value;
        return false;
      }
      return true;
    });

    if (node[1].length == 0) {
      ast[1][i] = emptyNode();
    }
  });
  traverseWithVariables(ast, function(node, type, allVars) {
    if (type == 'name') {
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
    if (type == 'assign' && jsonCompare(node[3], ['unary-prefix', 'void', ['num', 0]])) {
      return emptyNode();
    } else if (type == 'var') {
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
      if (type == 'assign' && jsonCompare(node[3], emptyNode())) {
        modified = true;
        return emptyNode();
      } else if (type == 'var') {
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
// We sometimes leave some settings to __label__ that are not needed, if later in
// the relooper we realize that we have a single entry, so no checks on __label__
// are actually necessary. It's easy to clean those up now.
function removeUnneededLabelSettings(ast) {
  traverse(ast, function(node, type) {
    if (type == 'defun') { // all of our compiled code is in defun nodes
      // Find all checks
      var checked = {};
      traverse(node, function(node, type) {
        if (type == 'binary' && node[1] == '==' && node[2][0] == 'name' && node[2][1] == '__label__') {
          assert(node[3][0] == 'num');
          checked[node[3][1]] = 1;
        }
      });
      // Remove unneeded sets
      traverse(node, function(node, type) {
        if (type == 'assign' && node[2][0] == 'name' && node[2][1] == '__label__') {
          assert(node[3][0] == 'num');
          if (!(node[3][1] in checked)) return emptyNode();
        }
      });
    }
  });
}

// Various expression simplifications. Pre run before closure (where we still have metadata), Post run after.

function simplifyExpressionsPre(ast) {
  // When there is a bunch of math like (((8+5)|0)+12)|0, only the external |0 is needed, one correction is enough.
  // At each node, ((X|0)+Y)|0 can be transformed into (X+Y): The inner corrections are not needed
  // TODO: Is the same is true for 0xff, 0xffff?
  // Likewise, if we have |0 inside a block that will be >>'d, then the |0 is unnecessary because some
  // 'useful' mathops already |0 anyhow.

  function simplifyBitops(ast) {
    var USEFUL_BINARY_OPS = set('<<', '>>', '|', '&', '^');
    var SAFE_BINARY_OPS = set('+', '-', '*', '/', '%');
    var ZERO = ['num', 0];
    var rerun = true;
    while (rerun) {
      rerun = false;
      traverseGenerated(ast, function(node, type, stack) {
        if (type == 'binary' && node[1] == '|' && (jsonCompare(node[2], ZERO) || jsonCompare(node[3], ZERO))) {
          stack.push(1); // From here on up, no need for this kind of correction, it's done at the top

          // We might be able to remove this correction
          for (var i = stack.length-2; i >= 0; i--) {
            if (stack[i] == 1) {
              // Great, we can eliminate
              rerun = true;
              return jsonCompare(node[2], ZERO) ? node[3] : node[2];
            } else if (stack[i] == -1) {
              break; // Too bad, we can't
            }
          }
        } else if (type == 'binary' && node[1] in USEFUL_BINARY_OPS) {
          stack.push(1);
        } else if ((type == 'binary' && node[1] in SAFE_BINARY_OPS) || type == 'num' || type == 'name') {
          stack.push(0); // This node is safe in that it does not interfere with this optimization
        } else {
          stack.push(-1); // This node is dangerous! Give up if you see this before you see '1'
        }
      }, null, []);
    }
  }

  // The most common mathop is addition, e.g. in getelementptr done repeatedly. We can join all of those,
  // by doing (num+num) ==> newnum, and (name+num)+num = name+newnum
  function joinAdditions(ast) {
    var rerun = true;
    while (rerun) {
      rerun = false;
      traverseGenerated(ast, function(node, type) {
        if (type == 'binary' && node[1] == '+') {
          if (node[2][0] == 'num' && node[3][0] == 'num') {
            rerun = true;
            return ['num', node[2][1] + node[3][1]];
          }
          for (var i = 2; i <= 3; i++) {
            var ii = 5-i;
            for (var j = 2; j <= 3; j++) {
              if (node[i][0] == 'num' && node[ii][0] == 'binary' && node[ii][1] == '+' && node[ii][j][0] == 'num') {
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

  simplifyBitops(ast);
  joinAdditions(ast);
}

// In typed arrays mode 2, we can have
//  HEAP[x >> 2]
// very often. We can in some cases do the shift on the variable itself when it is set,
// to greatly reduce the number of shift operations.
function optimizeShifts(ast) {
  traverseGeneratedFunctions(ast, function(fun) {
    var funMore = true;
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
      traverse(fun, function(node, type) {
        if (type == 'var') {
          node[1].forEach(function(arg) {
            newVar(arg[0], false, arg[1]);
          });
        }
      });
      // uses and defs TODO: weight uses by being inside a loop (powers). without that, we
      // optimize for code size, not speed.
      traverse(fun, function(node, type, stack) {
        stack.push(node);
        if (type == 'name' && vars[node[1]] && stack[stack.length-2][0] != 'assign') {
          vars[node[1]].uses++;
        } else if (type == 'assign' && node[2][0] == 'name' && vars[node[2][1]]) {
          vars[node[2][1]].defs++;
        }
      }, null, []);
      // First, break up elements inside a shift. This lets us see clearly what to do next.
      traverse(fun, function(node, type) {
        if (type == 'binary' && node[1] == '>>' && node[3][0] == 'num') {
          var shifts = node[3][1];
          // Push the >> inside the value elements, doing some simplification while we are there
          function addShift(subNode) {
            if (subNode[0] == 'binary' && subNode[1] == '+') {
              subNode[2] = addShift(subNode[2]);
              subNode[3] = addShift(subNode[3]);
              return subNode;
            }
            if (subNode[0] == 'binary' && subNode[1] == '<<' && subNode[3][0] == 'num') {
              if (subNode[3][1] > shifts) {
                subNode[3][1] -= shifts;
                return subNode;
              }
              // fall through to >> case
              subNode[1] = '>>';
              subNode[3][1] *= -1;
            }
            if (subNode[0] == 'binary' && subNode[1] == '>>') {
              if (subNode[3][0] == 'num') {
                subNode[3][1] += shifts;
                // XXX this may be wrong, if we removed a |0 that assumed we had this >> which |0's anyhow!
                if (subNode[3][1] == 0) return subNode[2];
              } else {
                subNode[3] = ['binary', '+', subNode[3], ['num', shifts]];
              }
              return subNode;
            }
            if (subNode[0] == 'num') {
              assert(subNode[1] % Math.pow(2, shifts) == 0, subNode);
              subNode[1] /= Math.pow(2, shifts); // bake the shift in
              return subNode;
            }
            if (subNode[0] == 'name' && !subNode[2]) { // names are returned with a shift, but we also note their being shifted
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
      });
      traverse(fun, function(node, type) {
        if (node[0] == 'name' && node[2]) {
          return node.slice(0, 2); // clean up our notes
        }
      });
      // At this point, shifted expressions are split up, and we know who the vars are and their info, so we can decide
      // TODO: vars that depend on other vars
      for (var name in vars) {
        var data = vars[name];
        var totalTimesShifted = sum(data.timesShifted);
        if (totalTimesShifted == 0) {
          continue;
        }
        if (totalTimesShifted != Math.max.apply(null, data.timesShifted)) {
          // TODO: Handle multiple different shifts
          continue;
        }
        // We have one shift size (and possible unshifted uses). Consider replacing this variable with a shifted clone. If
        // the estimated benefit is >0, we will replace
        data.benefit = totalTimesShifted - data.defs*2 - (data.uses-totalTimesShifted);
        if (data.benefit > 0) {
          funMore = true; // We will reprocess this function
          for (var i = 0; i < 4; i++) {
            if (data.timesShifted[i]) {
              data.primaryShift = i;
            }
          }
        }
      }
      function cleanNotes() { // We need to mark 'name' nodes as 'processed' in some passes here; this cleans the notes up
        traverse(fun, function(node, type) {
          if (node[0] == 'name' && node[2]) {
            return node.slice(0, 2);
          }
        });
      }
      cleanNotes();
      // Apply changes
      for (var name in vars) { // add shifts for params
        var data = vars[name];
        if (data.primaryShift >= 0 && data.param) {
          fun[3].unshift(['stat', ['assign', '>>', ['name', name], ['num', data.primaryShift]]]);
        }
      }
      function needsShift(name) {
        return vars[name] && vars[name].primaryShift >= 0;
      }
      traverse(fun, function(node, type) { // add shift to assignments
        if (node[0] == 'assign' && node[1] === true && node[2][0] == 'name' && needsShift(node[2][1])) {
          node[3] = ['binary', '>>', node[3], ['num', vars[node[2][1]].primaryShift]];
          return node;
        } else if (node[0] == 'var') {
          node[1].forEach(function(arg) {
            if (arg[1] && needsShift(arg[0])) {
              arg[1] = ['binary', '>>', arg[1], ['num', vars[arg[0]].primaryShift]];
            }
          });
          return node;
        }
      });
      traverse(fun, function(node, type, stack) { // replace name with unshifted name, making everything valid again by balancing the assign changes
        stack.push(node);
        if (node[0] == 'name' && !node[2] && vars[node[1]] && vars[node[1]].primaryShift >= 0 && stack[stack.length-2][0] != 'assign') {
          node[2] = true;
          return ['binary', '<<', node, ['num', vars[node[1]].primaryShift]];
        }
      }, null, []);
      cleanNotes();
      var more = true;
      var SIMPLE_SHIFTS = set('<<', '>>');
      while (more) { // collapse shifts and unshifts, completing the optimization
        more = false;
        traverse(fun, function(node, type) {
          if (node[0] == 'binary' && node[1] in SIMPLE_SHIFTS && node[2][0] == 'binary' && node[2][1] in SIMPLE_SHIFTS &&
              node[3][0] == 'num' && node[2][3][0] == 'num') { // do not turn a << b << c into a << b + c; while logically identical, it is slower
            more = true;
            var combinedShift = '>>';
            var sign1 = node[1] == '>>' ? 1 : -1;
            var sign2 = node[2][1] == '>>' ? 1 : -1;
            var total = 0;
            total = ['num', sign1*node[3][1] + sign2*node[2][3][1]];
            if (total[1] < 0) {
              combinedShift = '<<';
              total[1] *= -1;
            }
            if (total[1] == 0) {
              // XXX this may be wrong, if we removed a |0 that assumed we had this >> which |0's anyhow!
              return node[2][2];
            }
            return ['binary', combinedShift, node[2][2], total];
          }
        });
      }
      // Before recombining, do some additional optimizations
      traverse(fun, function(node, type) {
        // Optimize the case of ($a*80)>>2
        if (type == 'binary' && node[1] in SIMPLE_SHIFTS &&
            node[2][0] == 'binary' && node[2][1] == '*') {
          var mulNode = node[2];
          if (mulNode[2][0] == 'num') {
            var temp = mulNode[2];
            mulNode[2] = mulNode[3];
            mulNode[3] = temp;
          }
          if (mulNode[3][0] == 'num') {
            if (node[1] == '<<') {
              mulNode[3][1] *= Math.pow(2, node[3][1]);
              return mulNode;
            } else {
              if (mulNode[3][1] % Math.pow(2, node[3][1]) == 0) {
                mulNode[3][1] /= Math.pow(2, node[3][1]);
                return mulNode;
              }
            }
          }
        }
      });
      // Re-combine remaining shifts, to undo the breaking up we did before. may require reordering inside +'s
      traverse(fun, function(node, type, stack) {
        stack.push(node);
        if (type == 'binary' && node[1] == '+') {
          // 'Flatten' added items
          var addedItems = [];
          function flatten(node) {
            if (node[0] == 'binary' && node[1] == '+') {
              flatten(node[2]);
              flatten(node[3]);
            } else {
              addedItems.push(node);
            }
          }
          flatten(node);
          function key(node) { // a unique value for all relevant shifts for recombining, non-unique for stuff we don't need to bother with
            if (node[0] == 'binary' && node[1] in SIMPLE_SHIFTS) {
              if (node[3][0] == 'num' && node[3][1] >= 0 && node[3][1] <= 3) return 2*node[3][1] + (node[1] == '>>' ? 100 : 0); // 0-106
              return node[1] == '>>' ? 2000 : 1000;
            }
            if (node[0] == 'num') return -1000;
            return -1;
          }
          addedItems.sort(function(node1, node2) {
            return key(node1) - key(node2);
          });
          // Regenerate items, now sorted
          var i = 0;
          while (i < addedItems.length-1) { // re-combine inside addedItems
            var k = key(addedItems[i]), k1 = key(addedItems[i+1]);
            if (k == k1 && k >= 0 && k1 <= 106) {
              addedItems[i] = ['binary', addedItems[i][1], ['binary', '+', addedItems[i][2], addedItems[i+1][2]], addedItems[i][3]];
              addedItems.splice(i+1, 1);
            } else {
              i++;
            }
          }
          var ret = addedItems.pop();
          while (addedItems.length > 0) { // re-create AST from addedItems
            ret = ['binary', '+', ret, addedItems.pop()];
          }
          return ret;
        }
      }, null, []);
    }
  });
}

function simplifyExpressionsPost(ast) {
  // We often have branchings that are simplified so one end vanishes, and
  // we then get
  //   if (!(x < 5))
  // or such. Simplifying these saves space and time.
  function simplifyNotComps(ast) {
    traverse(ast, function(node, type) {
      if (type == 'unary-prefix' && node[1] == '!' && node[2][0] == 'binary') {
        if (node[2][1] == '<') {
          return ['binary', '>=', node[2][2], node[2][3]];
        } else if (node[2][1] == '>') {
          return ['binary', '<=', node[2][2], node[2][3]];
        } else if (node[2][1] == '==') {
          return ['binary', '!=', node[2][2], node[2][3]];
        } else if (node[2][1] == '!=') {
          return ['binary', '==', node[2][2], node[2][3]];
        } else if (node[2][1] == '===') {
          return ['binary', '!==', node[2][2], node[2][3]];
        } else if (node[2][1] == '!==') {
          return ['binary', '===', node[2][2], node[2][3]];
        }
      }
    });
  }

  // Go

  simplifyNotComps(ast);
}

// Clear out empty ifs and blocks, and redundant blocks/stats and so forth
function vacuum(ast) {
  function isEmpty(node) {
    if (!node) return true;
    if (jsonCompare(node, emptyNode())) return true;
    if (node[0] == 'block' && (!node[1] || (typeof node[1] != 'object') || node[1].length == 0 || (node[1].length == 1 && isEmpty(node[1])))) return true;
    return false;
  }
  var ret;
  var more = true;
  while (more) {
    more = false;
    ast[1].forEach(function(node, i) {
      function simplifyList(node, i) {
        var changed = false;
        var pre = node[i].length;
        node[i] = node[i].filter(function(node) { return !isEmpty(node) });
        if (node[i].length < pre) changed = true;
        // Also, seek blocks with single items we can simplify
        node[i] = node[i].map(function(subNode) {
          if (subNode[0] == 'block' && typeof subNode[1] == 'object' && subNode[1].length == 1 && subNode[1][0][0] == 'if') {
            return subNode[1][0];
          }
          return subNode;
        });
        if (changed) {
          more = true;
          return node;
        }
      }
      var type = node[0];
      if (type == 'defun' && isGenerated(node[1])) {
        traverse(node, function(node, type) {
          if (type == 'if') {
            if (((node[2][0] == 'block' && (!node[2][1] || node[2][1].length == 0)) ||
                  jsonCompare(node[2], emptyNode())) && !node[3]) {
              more = true;
              return emptyNode();
            } else if (node[3] && isEmpty(node[3])) {
              more = true;
              node[3] = null;
              return node;
            }
          } else if (type == 'block' && node[1] && node[1].length == 1 && node[1][0][0] == 'block') {
            more = true;
            return node[1][0];
          } else if (type == 'stat' && node[1][0] == 'block') {
            more = true;
            return node[1];
          } else if (type == 'block' && typeof node[1] == 'object') {
            ret = simplifyList(node, 1);
            if (ret) return ret;
          } else if (type == 'defun' && node[3].length == 1 && node[3][0][0] == 'block') {
            more = true;
            node[3] = node[3][0][1];
            return node;
          } else if (type == 'defun') {
            ret = simplifyList(node, 3);
            if (ret) return ret;
          } else if (type == 'do' && node[1][0] == 'num' && jsonCompare(node[2], emptyNode())) {
            more = true;
            return emptyNode();
          } else if (type == 'label' && jsonCompare(node[2], emptyNode())) {
            more = true;
            return emptyNode();
          } else if (type == 'if' && isEmpty(node[3])) { // empty else clauses
            node[3] = null;
            return node;
          }
        });
      }
    });
  }
}

// Multiple blocks from the relooper are, in general, implemented by
//   if (__label__ == x) { } else if ..
// and branching into them by
//   if (condition) { __label__ == x } else ..
// We can hoist the multiple block into the condition, thus removing code and one 'if' check
function hoistMultiples(ast) {
  ast[1].forEach(function(node, i) {
    if (!(node[0] == 'defun' && isGenerated(node[1]))) return;
    traverse(node, function(node, type) {
      var statements = null;
      if (type == 'defun') {
        statements = node[3];
      } else if (type == 'block') {
        statements = node[1];
      }
      if (!statements) return;
      var modified = false;
      for (var i = 0; i < statements.length-1; i++) {
        var modifiedI = false;
        var pre = statements[i];
        if (pre[0] != 'if') continue;
        var post = statements[i+1];
        // Look into some block types. shell() will then recreate the shell that we looked into
        var postInner = post;
        var shell = function(x) { return x };
        while (true) {
          /*if (postInner[0] == 'block') {
            postInner = postInner[1][0];
          } else */if (postInner[0] == 'label') {
            shell = (function(oldShell, oldPostInner) {
              return function(x) {
                return oldShell(['label', oldPostInner[1], x]);
              };
            })(shell, postInner);
            postInner = postInner[2];
          } else if (postInner[0] == 'do') {
            shell = (function(oldShell, oldPostInner) {
              return function(x) {
                return oldShell(['do', copy(oldPostInner[1]), ['block', [x]]]);
              }
            })(shell, postInner);;
            postInner = postInner[2][1][0];
          } else {
            break; // give up
          }
        }
        if (postInner[0] != 'if') continue;
        // Look into this if, and its elseifs
        while (postInner && postInner[0] == 'if') {
          var cond = postInner[1];
          if (cond[0] == 'binary' && cond[1] == '==' && cond[2][0] == 'name' && cond[2][1] == '__label__') {
            assert(cond[3][0] == 'num');
            // We have a valid Multiple check here. Try to hoist it, look for the source in |pre| and its else's
            var labelNum = cond[3][1];
            var labelBlock = postInner[2];
            assert(labelBlock[0] == 'block');
            var found = false;
            traverse(pre, function(preNode, preType) {
              if (!found && preType == 'assign' && preNode[2][0] == 'name' && preNode[2][1] == '__label__') {
                assert(preNode[3][0] == 'num');
                if (preNode[3][1] == labelNum) {
                  // That's it! Hoist away
                  found = true;
                  modifiedI = true;
                  postInner[2] = ['block', []];
                  return ['block', [preNode].concat(labelBlock[1])];
                }
              }
            });
          }
          postInner = postInner[3]; // Proceed to look in the else clause
        }
        if (modifiedI) {
          statements[i] = shell(pre);
        }
      }
      if (modified) return node;
    });
  });

  vacuum(ast);
}

// Simplifies loops
// WARNING: This assumes all loops and breaks/continues are labelled
function loopOptimizer(ast) {
  // Remove unneeded labels and one-time (do while(0)) loops. It is convenient to do these both at once.
  function passTwo(ast) {
    var neededDos = [];
    // Find unneeded labels
    traverseGenerated(ast, function(node, type, stack) {
      if (type == 'label' && node[2][0] in LOOP) {
        // this is a labelled loop. we don't know if it's needed yet. Mark its label for removal for now now.
        stack.push(node);
        node[1] = '+' + node[1];
      } else if (type in LOOP) {
        stack.push(node);
      } else if (type in LOOP_FLOW && node[1]) { // only care about break/continue with an explicit label
        // Find topmost loop, and its label if there is one
        var lastLabel = null, lastLoop = null, i = stack.length-1;
        while (i >= 0 && !lastLoop) {
          if (stack[i][0] in LOOP) lastLoop = stack[i];
          i--;
        }
        assert(lastLoop, 'Cannot break/continue without a Label');
        while (i >= 0 && !lastLabel) {
          if (stack[i][0] in LOOP) break; // another loop in the middle - no label for lastLoop
          if (stack[i][0] == 'label') lastLabel = stack[i];
          i--;
        }
        var ident = node[1];
        var plus = '+' + ident;
        if (lastLabel && (ident == lastLabel[1] || plus == lastLabel[1])) {
          // If this is a 'do' loop, this break means we actually need it.
          neededDos.push(lastLoop);
          // We don't need the control flow command to have a label - it's referring to the current loop
          return [node[0]];
        } else {
          // Find the label node that needs to stay alive
          stack.forEach(function(label) {
            if (!label) return;
            if (label[1] == plus) label[1] = label[1].substr(1); // Remove '+', marking it as needed
          });
        }
      }
    }, null, []);
    // Remove unneeded labels
    traverseGenerated(ast, function(node, type) {
      if (type == 'label' && node[1][0] == '+') {
        var ident = node[1].substr(1);
        // Remove label from loop flow commands
        traverse(node[2], function(node2, type) {
          if (type in LOOP_FLOW && node2[1] == ident) {
            return [node2[0]];
          }
        });
        return node[2]; // Remove the label itself on the loop
      }
    });
    // Remove unneeded one-time loops. We need such loops if (1) they have a label, or (2) they have a direct break so they are in neededDos.
    // First, add all labeled loops of this nature to neededDos
    traverseGenerated(ast, function(node, type) {
      if (type == 'label' && node[2][0] == 'do') {
        neededDos.push(node[2]);
      }
    });
    // Remove unneeded dos, we know who they are now
    traverseGenerated(ast, function(node, type) {
      if (type == 'do' && neededDos.indexOf(node) < 0) {
        assert(jsonCompare(node[1], ['num', 0]), 'Trying to remove a one-time do loop that is not one of our generated ones.;');
        return node[2];
      }
    });
  }

  // Go

  // TODO: pass 1: Removal of unneeded continues, breaks if they get us to where we are already going. That will
  //               help the next pass.
  passTwo(ast);

  vacuum(ast);
}

// Passes table

var passes = {
  dumpAst: dumpAst,
  dumpSrc: dumpSrc,
  unGlobalize: unGlobalize,
  removeAssignsToUndefined: removeAssignsToUndefined,
  //removeUnneededLabelSettings: removeUnneededLabelSettings,
  simplifyExpressionsPre: simplifyExpressionsPre,
  optimizeShifts: optimizeShifts,
  simplifyExpressionsPost: simplifyExpressionsPost,
  hoistMultiples: hoistMultiples,
  loopOptimizer: loopOptimizer
};

// Main

var src = fs.readFileSync('/dev/stdin').toString();
var ast = srcToAst(src);
//printErr(JSON.stringify(ast)); throw 1;
var metadata = src.split('\n').filter(function(line) { return line.indexOf('EMSCRIPTEN_GENERATED_FUNCTIONS') >= 0 })[0];
//assert(metadata, 'Must have EMSCRIPTEN_GENERATED_FUNCTIONS metadata');
if (metadata) setGeneratedFunctions(metadata);

arguments.forEach(function(arg) {
  passes[arg](ast);
});
//printErr('output: ' + dump(ast));
//printErr('output: ' + astToSrc(ast));
ast = srcToAst(astToSrc(ast)); // re-parse, to simplify a little
print(astToSrc(ast));
if (metadata) print(metadata + '\n');

