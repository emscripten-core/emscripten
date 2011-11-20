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
function read(filename) {
  if (filename[0] != '/') filename = __dirname.split('/').slice(0, -1).join('/') + '/src/' + filename;
  return fs.readFileSync(filename).toString();
}

// Load some modules

eval(read('utility.js'));

// Utilities

var FUNCTION = set('defun', 'function');

var NULL_NODE = ['name', 'null'];
var UNDEFINED_NODE = ['unary-prefix', 'void', ['num', 0]];
var TRUE_NODE = ['unary-prefix', '!', ['num', 0]];
var FALSE_NODE = ['unary-prefix', '!', ['num', 1]];

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
      }
    });
  }
}

// Main

var src = fs.readFileSync('/dev/stdin').toString();

var ast = uglify.parser.parse(src);

unGlobalize(ast);
removeAssignsToUndefined(ast);

print(uglify.uglify.gen_code(ast, {
  ascii_only: true,
  beautify: true,
  indent_level: 2
}));

