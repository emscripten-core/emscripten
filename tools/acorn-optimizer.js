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

// Passes

// Removes obviously-unused code. Similar to closure compiler in its rules -
// export e.g. by Module['..'] = theThing; , or use it somewhere, otherwise
// it goes away.
function JSDCE(ast, multipleIterations) {

/*
walk.recursive(ast, null, {
  Literal(node) {
    console.log(`Found a literal: ${node.value}`)
  },
  FunctionDeclaration(node, state, c) {
    c(node.body, state);
    console.log(`Found a func: ${node.id.name}`)
  },
});
return;
*/
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
throw 'TODO';
      traverse(ast, function(node, type) {
        if (type === 'defun' && Object.prototype.hasOwnProperty.call(names, node[1])) {
          removed = true;
          return emptyNode();
        }
        if (type === 'defun' || type === 'function') return null; // do not enter other scopes
        if (type === 'var') {
          node[1] = node[1].filter(function(varItem, j) {
            var curr = varItem[0];
            var value = varItem[1];
            var keep = !(curr in names) || (value && hasSideEffects(value));
            if (!keep) removed = true;
            return keep;
          });
          if (node[1].length === 0) return emptyNode();
        }
      });
      return ast;
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
      cleanUp(node[3], names);
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

