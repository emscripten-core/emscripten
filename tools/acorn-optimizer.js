var acorn = require('acorn');
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
  if (!condition) throw text + ' : ' + new Error().stack;
}

function set(args) {
  var ret = {};
  for (var i = 0; i < args.length; i++) {
    ret[args[i]] = 0;
  }
  return ret;
}

// Visits and walks
// (We don't use acorn-walk because it ignores x in 'x = y'.)

function visitChildren(node, c) {
  // emptyOut() and temporary ignoring may mark nodes as empty,
  // while they have properties with children we should ignore.
  if (node.type === 'EmptyStatement') {
    return;
  }
//print('vcs ' + JSON.stringify(node));
  function maybeChild(child) {
//print('mc ' + JSON.stringify(child));
    if (child && typeof child === 'object' && typeof child.type === 'string') {

//print('MC!!!!!!!');
      c(child);
      return true;
    }
    return false;
  }
  for (var key in node) {
    var child = node[key];
//print('a1 ' + key + ' : ' + JSON.stringify(child));
    // Check for a child.
    if (!maybeChild(child)) {
//print('a2');
      // Check for an array of children.
      if (Array.isArray(child)) {
//print('a3');
        child.forEach(maybeChild);
      } else {
//print('a4');
        /*
        // Check for an object of children.
        if (child && typeof child === 'object') {
//print('a5');
          for (var grandChild in child) {
//print('a6');
            maybeChild(grandChild);
          }
        }
        */
      }
    }
  }
}

// Simple post-order walk, calling properties on an object by node type,
// if the type exists.
function simpleWalk(node, cs) {
  visitChildren(node, function(child) {
    simpleWalk(child, cs);
  });
  if (node.type in cs) {
    cs[node.type](node);
  }
}

// Full post-order walk, calling a single function for all types.
function fullWalk(node, c) {
  visitChildren(node, function(child) {
    fullWalk(child, c);
  });
  c(node);
}

// Recursive post-order walk, calling properties on an object by node type,
// if the type exists, and if so leaving recursion to that function.
function recursiveWalk(node, cs) {
//print('recw1 ' + JSON.stringify(node));
  (function c(node) {
    if (!(node.type in cs)) {
      visitChildren(node, function(child) {
        recursiveWalk(child, cs);
      });
    } else {
      cs[node.type](node, c);
    }
  })(node);
}

// AST Utilities

function emptyOut(node) {
  node.type = 'EmptyStatement';
}

function nullify(node) {
  node.type = 'Literal';
  node.value = null;
  node.raw = 'null';
}

function undefinedify(node) {
  node.type = 'Literal';
  node.value = undefined;
  node.raw = 'undefined';
}

function setLiteralValue(item, value) {
  item.value = value;
  item.raw = "'" + value + "'";
}

function dump(node, text) {
  if (text) print(text);
  print(JSON.stringify(node, null, ' '));
}

// Mark inner scopes temporarily as empty statements. Returns
// a special object that must be used to restore them.
function ignoreInnerScopes(node) {
  var map = new WeakMap();
  function ignore(node) {
    map.set(node, node.type);
    node.type = 'EmptyStatement';
  }
  simpleWalk(node, {
    FunctionExpression(node) {
      ignore(node);
    },
    FunctionDeclaration(node) {
      ignore(node);
    },
    // TODO: arrow etc.
  });
  return map;
}

// Mark inner scopes temporarily as empty statements.
function restoreInnerScopes(node, map) {
  fullWalk(node, function(node) {
    if (map.has(node)) {
      node.type = map.get(node);
      map.delete(node);
      restoreInnerScopes(node, map);
    }
  });
}

function hasSideEffects(node) {
  // Conservative analysis.
  var map = ignoreInnerScopes(node);
  var has = false;
  fullWalk(node, function(node) {
    switch (node.type) {
      // TODO: go through all the ESTree spec
      case 'Literal':
      case 'Identifier':
      case 'UnaryExpression':
      case 'BinaryExpresson':
      case 'ExpressionStatement':
      case 'UpdateOperator':
      case 'ConditionalExpression':
      case 'AssignmentExpression':
      case 'FunctionExpression':
      case 'FunctionDeclaration':
      case 'VariableDeclaration':
      case 'VariableDeclarator':
      case 'ObjectExpression':
      case 'Property':
      case 'BlockStatement':
      case 'ArrayExpression':
      case 'EmptyStatement': {
        break; // safe
      }
      case 'MemberExpression': {
        // safe if on Math (or other familiar objects, TODO)
        if (node.object.type !== 'Identifier' ||
            (node.object.name !== 'Math' &&
             node.object.name !== 'asm' &&
             node.object.name !== 'Module')) {
          //console.error('because member on ' + node.object.name);
          has = true;
        }
        break;
      }
      default: {
        has = true;
        //console.error('because ' + node.type);
      }
    }
  });
  restoreInnerScopes(node, map);
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
      recursiveWalk(ast, {
        VariableDeclaration(node, c) {
          node.declarations = node.declarations.filter(function(node) {
            var curr = node.id.name
            var value = node.init;
            var keep = !(curr in names) || (value && hasSideEffects(value));
            if (!keep) removed = true;
            return keep;
          });
          if (node.declarations.length === 0) {
            emptyOut(node);
          }
        },
        FunctionDeclaration(node, c) {
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

    function handleFunction(node, c, defun) {
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
      c(node.body);
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

    recursiveWalk(ast, {
      VariableDeclarator(node, c) {
        var name = node.id.name;
        ensureData(scopes[scopes.length-1], name).def = 1;
        if (node.init) c(node.init);
      },
      ObjectExpression(node, c) {
        // ignore the property identifiers
        node.properties.forEach(function(node) {
          c(node.value);
        });
      },
      FunctionDeclaration(node, c) {
        handleFunction(node, c, true /* defun */);
      },
      FunctionExpression(node, c) {
        handleFunction(node, c);
      },
      Identifier(node, c) {
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

function isAsmLibraryArgAssign(node) { // Module.asmLibraryArg = ..
  return node.type === 'AssignmentExpression' &&
         node.operator === '=' &&
         node.left.type === 'MemberExpression' &&
         node.left.object.type === 'Identifier' &&
         node.left.object.name === 'Module' &&
         node.left.property.type === 'Identifier' &&
         node.left.property.name === 'asmLibraryArg';
}

function isAsmUse(node) {
  return node.type === 'MemberExpression' &&
         ((node.object.type === 'Identifier' && // asm['X']
           node.object.name === 'asm' &&
           node.property.type === 'Literal') ||
          (node.object.type === 'MemberExpression' && // Module[asm]['X']
           node.object.object.type === 'Identifier' &&
           node.object.object.name === 'Module' &&
           node.object.property.type === 'Identifier' &&
           node.object.property.name === 'asm' &&
           node.property.type === 'Literal'));
}

function getAsmOrModuleUseName(node) {
  return node.property.value;
}

function isModuleUse(node) {
  return node.type === 'MemberExpression' && // Module['X']
         node.object.type === 'Identifier' &&
         node.object.name === 'Module' &&
         node.property.type === 'Literal';
}

function isModuleAsmUse(node) { // Module['asm'][..string..]
  return node.type === 'MemberExpression' &&
         node.object.type === 'MemberExpression' &&
         node.object.object.type === 'Identifier' &&
         node.object.object.name === 'Module' &&
         node.object.property.type === 'Literal' && 
         node.object.property.value === 'asm' &&
         node.property.type === 'Literal';
}

// Apply import/export name changes (after minifying them)
function applyImportAndExportNameChanges(ast) {
  var mapping = extraInfo.mapping;
  fullWalk(ast, function(node) {
    if (isAsmLibraryArgAssign(node)) {
      var assignedObject = node.right;
      assignedObject.properties.forEach(function(item) {
        if (mapping[item.key.value]) {
          setLiteralValue(item.key, mapping[item.key.value]);
        }
      });
    } else if (node.type === 'AssignmentExpression') {
      var target = node.left;
      var value = node.right;
      if (isAsmUse(value)) {
        var name = value.property.value;
        if (mapping[name]) {
          setLiteralValue(value.property, mapping[name]);
        }
      }
    } else if (isModuleAsmUse(node)) {
      var prop = node.property;
      var name = prop.value;
      if (mapping[name]) {
        setLiteralValue(prop, mapping[name]);
      }
    }
  });
}

// Apply graph removals from running wasm-metadce
function applyDCEGraphRemovals(ast) {
  var unused = set(extraInfo.unused);

  fullWalk(ast, function(node) {
    if (isAsmLibraryArgAssign(node)) {
      var assignedObject = node.right;
      assignedObject.properties = assignedObject.properties.filter(function(item) {
        var name = item.key.value;
        var value = item.value;
        var full = 'emcc$import$' + name;
        return !((full in unused) && !hasSideEffects(value));
      });
    } else if (node.type === 'AssignmentExpression') {
      // when we assign to a thing we don't need, we can just remove the assign
      var target = node.left;
      if (isAsmUse(target) || isModuleUse(target)) {
        var name = getAsmOrModuleUseName(target);
        var full = 'emcc$export$' + name;
        var value = node.right;
        if ((full in unused) && !hasSideEffects(value)) {
          undefinedify(node);
        }
      }
    }
  });
}

// Main

var arguments = process['argv'].slice(2);;
var infile = arguments[0];
var passes = arguments.slice(1);

var input = read(infile);
var extraInfoStart = input.lastIndexOf('// EXTRA_INFO:')
var extraInfo = null;
if (extraInfoStart > 0) {
  extraInfo = JSON.parse(input.substr(extraInfoStart + 14));
}
var ast = acorn.parse(input, { ecmaVersion: 6 });

var minifyWhitespace = false;

var registry = {
  JSDCE: JSDCE,
  AJSDCE: AJSDCE,
  applyImportAndExportNameChanges: applyImportAndExportNameChanges,
  applyDCEGraphRemovals: applyDCEGraphRemovals,
  minifyWhitespace: function() { minifyWhitespace = true },
  dump: function() { dump(ast) },
};

passes.forEach(function(pass) {
  registry[pass](ast);
});

//print('\nPOST\n' + JSON.stringify(ast, null, ' '));

var output = astring.generate(ast, {
  indent: minifyWhitespace ? '' : ' ',
  lineEnd: minifyWhitespace ? '' : '\n',
  // may want to use escodegen with compact=true and semicolons=false (but it has many more deps)
});
print(output);

