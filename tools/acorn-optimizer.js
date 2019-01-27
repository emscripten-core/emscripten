var acorn = require('acorn');
var walk = require('acorn-walk');
var astring = require('astring');
var fs = require('fs');
var path = require('path');

// Setup

var print = function(x) {
  process.stdout.write(x + '\n');
};

var printErr = function(x) {
  process.stderr.write(x + '\n');
};

var read = function(x) {
  return fs.readFileSync(x).toString();
}

// Utilities

function assert(condition, text) {
  if (!condition) throw text;
}

function emptyOut(node) {
  node.type = 'EmptyStatement';
}

function hasSideEffects(node) {
  // Conservative analysis.
  var has = false;
  walk.full(node, function(node, state, type) {
    switch (type) {
      case 'Literal':
      case 'Identifier':
      case 'UnaryExpression':
      case 'BinaryExpresson':
      case 'ExpressionStatement':
      case 'UpdateOperator':
      case 'MemberExpression':
      case 'ConditionalExpression':
      case 'FunctionExpression':
      case 'FunctionDeclaration':
      case 'VariableDeclaration':
      case 'VariableDeclarator':
      case 'ObjectExpression':
      case 'ArrayExpression': {
        break; // safe
      }
      default: has = true; console.error('because ' + type);
    }
  });
  return has;
}

// Passes

// Removes obviously-unused code. Similar to closure compiler in its rules -
// export e.g. by Module['..'] = theThing; , or use it somewhere, otherwise
// it goes away.
//
// Note that this is somewhat conservative, since the ESTree AST does not
// have a simple separation between definitions and uses, e.g.
// Identifier is used both for the x in  function foo(x) {
// and for  y = x + 1 . That means we need to consider new ES6+ constructs
// as they appear (like ArrowFunctionExpression). Instead, we do a conservative
// analysis here.

function JSDCE(ast, multipleIterations) {
  function iteration() {
    var removed = false;
    var scopes = [{}]; // begin with empty toplevel scope
    function DUMP() {
      printErr('vvvvvvvvvvvvvv');
      for (var i = 0; i < scopes.length; i++) {
        printErr(i + ' : ' + JSON.stringify(scopes[i]));
      }
      printErr('^^^^^^^^^^^^^^');
    }
    function ensureData(scope, name) {
      if (Object.prototype.hasOwnProperty.call(scope, name)) return scope[name];
      scope[name] = {
        def: 0,
        use: 0,
        param: 0 // true for function params, which cannot be eliminated
      };
      return scope[name];
    }
    function cleanUp(ast, names) {
      walk.recursive(ast, null, {
        VariableDeclaration(node, state, c) {
          node.declarations = node.declarations.filter(function(node) {
            var curr = node.id.name
            var value = node.init;
            var keep = !(curr in names) || hasSideEffects(value);
            if (!keep) removed = true;
            return keep;
          });
          if (node.declarations.length === 0) {
            emptyOut(node);
          }
        },
        FunctionDeclaration(node, state, c) {
          if (Object.prototype.hasOwnProperty.call(names, node.id.name)) {
            removed = true;
            emptyOut(node);
            return;
          }
          // do not recurse into other scopes
        },
        // do not recurse into other scopes
        FunctionExpression() {},
        ArrowFunctionExpression() {},
      });
    }

    function handleFunction(node, state, c, defun) {
      // defun names matter - function names (the y in var x = function y() {..}) are just for stack traces.
      if (defun) {
        ensureData(scopes[scopes.length-1], node.id.name).def = 1;
      }
      var scope = {};
      node.params.forEach(function(param) {
        var name = param.name;
        ensureData(scope, name).def = 1;
        scope[name].param = 1;
      });
      scopes.push(scope);
      c(node.body, state);
      // we can ignore self-references, i.e., references to ourselves inside
      // ourselves, for named defined (defun) functions
      var ownName = defun ? node.id.name : '';
      var scope = scopes.pop();
      var names = {};
      for (var name in scope) {
        if (name === ownName) continue;
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
      cleanUp(node.body, names);
    }

    walk.recursive(ast, null, {
      VariableDeclarator(node, state, c) {
        var name = node.id.name;
        ensureData(scopes[scopes.length-1], name).def = 1;
        c(node.init, state);
      },
      FunctionDeclaration(node, state, c) {
        handleFunction(node, state, c, true /* defun */);
      },
      FunctionExpression(node, state, c) {
        handleFunction(node, state, c);
      },
      Identifier(node, state, c) {
        var name = node.name;
        ensureData(scopes[scopes.length-1], name).use = 1;
      },
    });

    // toplevel
    var scope = scopes.pop();
    assert(scopes.length === 0);

    var names = {};
    for (var name in scope) {
      var data = scope[name];
      if (data.def && !data.use) {
        assert(!data.param); // can't be
        // this is eliminateable!
        names[name] = 0;
      }
    }
    cleanUp(ast, names);
    return removed;
  }
  while (iteration() && multipleIterations) { }
}

// Aggressive JSDCE - multiple iterations
function AJSDCE(ast) {
  JSDCE(ast, /* multipleIterations= */ true);
}

// Registry and global state

var minifyWhitespace = false;

var registry = {
  JSDCE: JSDCE,
  AJSDCE: AJSDCE,
  minifyWhitespace: function() { minifyWhitespace = true },
};

// Main

var arguments = process['argv'].slice(2);;
var infile = arguments[0];
var passes = arguments.slice(1);

var input = read(infile);
var ast = acorn.parse(input, { ecmaVersion: 6 });

print("PRE\n" + JSON.stringify(ast, null, ' '));

passes.forEach(function(pass) {
  registry[pass](ast);
});

//print(JSON.stringify(ast));

var output = astring.generate(ast, {
  indent: minifyWhitespace ? '' : ' ',
  lineEnd: minifyWhitespace ? '' : '\n',
  // may want to use escodegen with compact=true and semicolons=false (but it has many more deps)
});
print(output);

