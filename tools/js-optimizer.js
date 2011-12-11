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

// Various expression simplifications
function simplifyExpressions(ast) {
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
}

// Passes table

var passes = {
  unGlobalize: unGlobalize,
  removeAssignsToUndefined: removeAssignsToUndefined,
  //removeUnneededLabelSettings: removeUnneededLabelSettings,
  simplifyExpressions: simplifyExpressions,
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
/* TODO: run only on generated functions (but, some passes look at globals...)
arguments.forEach(function(arg) {
  ast[1].forEach(function(node, i) {
    if (node[0] == 'defun' && isGenerated(node[1])) {
      passes[arg](node);
    }
  });
});
*/

print(astToSrc(ast));
if (metadata) print(metadata + '\n');

