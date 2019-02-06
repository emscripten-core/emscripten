var acorn = require('acorn');
var terser = require("terser");
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
  function maybeChild(child) {
    if (child && typeof child === 'object' && typeof child.type === 'string') {
      c(child);
      return true;
    }
    return false;
  }
  for (var key in node) {
    var child = node[key];
    // Check for a child.
    if (!maybeChild(child)) {
      // Check for an array of children.
      if (Array.isArray(child)) {
        child.forEach(maybeChild);
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

function isLiteralString(node) {
  return node.type === 'Literal' &&
         (node.raw[0] === '"' || node.raw[0] === "'");
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
    FunctionDeclaration(node) {
      ignore(node);
    },
    FunctionExpression(node) {
      ignore(node);
    },
    ArrowFunctionExpression(node) {
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

// If we empty out a var from
//   for (var i in x) {}
//   for (var j = 0;;) {}
// then it will be invalid. We saved it on the side;
// restore it here.
function restoreForVars(node) {
  var restored = 0;
  function fix(init) {
    if (init && init.type === 'EmptyStatement') {
      assert(init.oldDeclarations);
      init.type = 'VariableDeclaration';
      init.declarations = init.oldDeclarations;
      restored++;
    }
  }
  simpleWalk(node, {
    ForStatement(node) { fix(node.init) },
    ForInStatement(node) { fix(node.left) },
  });
  return restored;
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
      case 'FunctionDeclaration':
      case 'FunctionExpression':
      case 'ArrowFunctionExpression':
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
            node.object.name !== 'Math') {
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
    var removed = 0;
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
          var old = node.declarations;
          var removedHere = 0;
          node.declarations = node.declarations.filter(function(node) {
            var curr = node.id.name
            var value = node.init;
            var keep = !(curr in names) || (value && hasSideEffects(value));
            if (!keep) removedHere = 1;
            return keep;
          });
          removed += removedHere;
          if (node.declarations.length === 0) {
            emptyOut(node);
            // If this is in a for, we may need to restore it.
            node.oldDeclarations = old;
          }
        },
        FunctionDeclaration(node, c) {
          if (Object.prototype.hasOwnProperty.call(names, node.id.name)) {
            removed++;
            emptyOut(node);
            return;
          }
          // do not recurse into other scopes
        },
        // do not recurse into other scopes
        FunctionExpression() {},
        ArrowFunctionExpression() {},
      });
      removed -= restoreForVars(ast);
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
      ArrowFunctionExpression(node, c) {
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

function isAsmLibraryArgAssign(node) { // var asmLibraryArg = ..
  return node.type === 'VariableDeclaration' &&
         node.declarations.length === 1 &&
         node.declarations[0].id.name === 'asmLibraryArg' &&
         node.declarations[0].init &&
         node.declarations[0].init.type === 'ObjectExpression';
}

function getAsmLibraryArgValue(node) {
  return node.declarations[0].init;
}

function isAsmUse(node) {
  return node.type === 'MemberExpression' &&
         ((node.object.type === 'Identifier' && // asm['X']
           node.object.name === 'asm' &&
           node.property.type === 'Literal') ||
          (node.object.type === 'MemberExpression' && // Module['asm']['X']
           node.object.object.type === 'Identifier' &&
           node.object.object.name === 'Module' &&
           node.object.property.type === 'Literal' &&
           node.object.property.value === 'asm' &&
           isLiteralString(node.property)));
}

function getAsmOrModuleUseName(node) {
  return node.property.value;
}

function isModuleUse(node) {
  return node.type === 'MemberExpression' && // Module['X']
         node.object.type === 'Identifier' &&
         node.object.name === 'Module' &&
         isLiteralString(node.property);
}

function isModuleAsmUse(node) { // Module['asm'][..string..]
  return node.type === 'MemberExpression' &&
         node.object.type === 'MemberExpression' &&
         node.object.object.type === 'Identifier' &&
         node.object.object.name === 'Module' &&
         node.object.property.type === 'Literal' && 
         node.object.property.value === 'asm' &&
         isLiteralString(node.property);
}

// Apply import/export name changes (after minifying them)
function applyImportAndExportNameChanges(ast) {
  var mapping = extraInfo.mapping;
  fullWalk(ast, function(node) {
    if (isAsmLibraryArgAssign(node)) {
      var assignedObject = getAsmLibraryArgValue(node);
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
    } else if (isAsmUse(node)) {
      var prop = node.property;
      var name = prop.value;
      if (mapping[name]) {
        setLiteralValue(prop, mapping[name]);
      }
    }
  });
}

// A static dyncall is dynCall('vii', ..), which is actually static even
// though we call dynCall() - we see the string signature statically.
function isStaticDynCall(node) {
  return node.type === 'CallExpression' &&
         node.callee.type === 'Identifier' &&
         node.callee.name === 'dynCall' &&
         isLiteralString(node.arguments[0])
}

function getStaticDynCallName(node) {
  return 'dynCall_' + node.arguments[0].value;
}

// a dynamic dyncall is one in which all we know is *some* dynCall may
// be called, but not who. This can be either
//   dynCall(*not a string*, ..)
// or, to be conservative,
//   "dynCall_"
// as that prefix means we may be constructing a dynamic dyncall name
// (dynCall and embind's requireFunction do this internally).
function isDynamicDynCall(node) {
  return (node.type === 'CallExpression' &&
          node.callee.type === 'Identifier' &&
          node.callee.name === 'dynCall' &&
          !isLiteralString(node.arguments[0])) ||
         (isLiteralString(node) &&
          node.value === 'dynCall_');
}

//
// Emit the DCE graph, to help optimize the combined JS+wasm.
// This finds where JS depends on wasm, and where wasm depends
// on JS, and prints that out.
//
// The analysis here is simplified, and not completely general. It
// is enough to optimize the common case of JS library and runtime
// functions involved in loops with wasm, but not more complicated
// things like JS objects and sub-functions. Specifically we
// analyze as follows:
//
//  * We consider (1) the toplevel scope, and (2) the scopes of toplevel defined
//    functions (defun, not function; i.e., function X() {} where
//    X can be called later, and not y = function Z() {} where Z is
//    just a name for stack traces). We also consider the wasm, which
//    we can see things going to and arriving from.
//  * Anything used in a defun creates a link in the DCE graph, either
//    to another defun, or the wasm.
//  * Anything used in the toplevel scope is rooted, as it is code
//    we assume will execute. The exceptions are
//     * when we receive something from wasm; those are "free" and
//       do not cause rooting. (They will become roots if they are
//       exported, the metadce logic will handle that.)
//     * when we send something to wasm; sending a defun causes a
//       link in the DCE graph.
//  * Anything not in the toplevel or not in a toplevel defun is
//    considering rooted. We don't optimize those cases.
//
// Special handling:
//
//  * dynCall('vii', ..) are dynamic dynCalls, but we analyze them
//    statically, to preserve the dynCall_vii etc. method they depend on.
//    Truly dynamic dynCalls (not to a string constant) will not work,
//    and require the user to export them.
//  * Truly dynamic dynCalls are assumed to reach any dynCall_*.
//
// XXX this modifies the input AST. if you want to keep using it,
//     that should be fixed. Currently the main use case here does
//     not require that. TODO FIXME
//
function emitDCEGraph(ast) {
  // First pass: find the wasm imports and exports, and the toplevel
  // defuns, and save them on the side, removing them from the AST,
  // which makes the second pass simpler.
  //
  // The imports that wasm receives look like this:
  //
  //  Module.asmLibraryArg = { "abort": abort, "assert": assert, [..] };
  //
  // The exports are trickier, as they have a different form whether or not
  // async compilation is enabled. It can be either:
  //
  //  var _malloc = Module["_malloc"] = asm["_malloc"];
  //
  // or
  //
  //  var _malloc = Module["_malloc"] = (function() {
  //   return Module["asm"]["_malloc"].apply(null, arguments);
  //  });
  //
  var imports = [];
  var defuns = [];
  var dynCallNames = [];
  var nameToGraphName = {};
  var modulePropertyToGraphName = {};
  var exportNameToGraphName = {}; // identical to asm['..'] nameToGraphName
  var foundAsmLibraryArgAssign = false;
  var graph = [];

  function saveAsmExport(name, asmName) {
    // the asmName is what the wasm provides directly; the outside JS
    // name may be slightly different (extra "_" in wasm backend)
    var graphName = getGraphName(name, 'export');
    nameToGraphName[name] = graphName;
    modulePropertyToGraphName[name] = graphName;
    exportNameToGraphName[asmName] = graphName;
    if (/^dynCall_/.test(name)) {
      dynCallNames.push(graphName);
    }
  }

  fullWalk(ast, function(node) {
    if (isAsmLibraryArgAssign(node)) {
      var assignedObject = getAsmLibraryArgValue(node);
      assignedObject.properties.forEach(function(item) {
        var value = item.value;
        assert(value.type === 'Identifier');
        imports.push(value.name); // the name doesn't matter, only the value which is that actual thing we are importing
      });
      foundAsmLibraryArgAssign = true;
      emptyOut(node); // ignore this in the second pass; this does not root
    } else if (node.type === 'VariableDeclaration') {
      if (node.declarations.length === 1) {
        var item = node.declarations[0];
        var name = item.id.name;
        var value = item.init;
        if (value && value.type === 'AssignmentExpression') {
          var assigned = value.left;
          if (isModuleUse(assigned) && getAsmOrModuleUseName(assigned) === name) {
            // this is
            //  var x = Module['x'] = ?
            // which looks like a wasm export being received. confirm with the asm use
            var found = 0;
            var asmName;
            fullWalk(value.right, function(node) {
              if (isAsmUse(node)) {
                found++;
                asmName = getAsmOrModuleUseName(node);
              }
            });
            // in the wasm backend, the asm name may have one fewer "_" prefixed
            if (found === 1) {
              // this is indeed an export
              // the asmName is what the wasm provides directly; the outside JS
              // name may be slightly different (extra "_" in wasm backend)
              saveAsmExport(name, asmName);
              emptyOut(node); // ignore this in the second pass; this does not root
            }
          }
        }
      }
    } else if (node.type === 'FunctionDeclaration') {
      defuns.push(node);
      var name = node.id.name;
      nameToGraphName[name] = getGraphName(name, 'defun');
      emptyOut(node); // ignore this in the second pass; we scan defuns separately
    }
  });
  // must find the info we need
  assert(foundAsmLibraryArgAssign, 'could not find the assigment to "asmLibraryArg". perhaps --pre-js or --post-js code moved it out of the global scope? (things like that should be done after emcc runs, as they do not need to be run through the optimizer which is the special thing about --pre-js/--post-js code)');
  // Read exports that were declared in extraInfo
  if (extraInfo) {
    for (var e in extraInfo.exports) {
      var exp = extraInfo.exports[e];
      saveAsmExport(exp[0], exp[1]);
    }
  }
  // Second pass: everything used in the toplevel scope is rooted;
  // things used in defun scopes create links
  function getGraphName(name, what) {
    return 'emcc$' + what + '$' + name;
  }
  var infos = {}; // the graph name of the item => info for it
  imports.forEach(function(import_) {
    var name = getGraphName(import_, 'import');
    var info = infos[name] = {
      name: name,
      import: ['env', import_],
      reaches: {}
    };
    if (nameToGraphName.hasOwnProperty(import_)) {
      info.reaches[nameToGraphName[import_]] = 1;
    } // otherwise, it's a number, ignore
  });
  for (var e in exportNameToGraphName) {
    var name = exportNameToGraphName[e];
    infos[name] = {
      name: name,
      export: e,
      reaches: {}
    };
  }
  // a function that handles a node we visit, in either a defun or
  // the toplevel scope (in which case the second param is not provided)
  function visitNode(node, defunInfo) {
    // TODO: scope awareness here. for now we just assume all uses are
    //       from the top scope, which might create more uses than needed
    var reached;
    if (node.type === 'Identifier') {
      var name = node.name;
      if (nameToGraphName.hasOwnProperty(name)) {
        reached = nameToGraphName[name];
      }
    } else if (isModuleUse(node)) {
      var name = getAsmOrModuleUseName(node);
      if (modulePropertyToGraphName.hasOwnProperty(name)) {
        reached = modulePropertyToGraphName[name];
      }
    } else if (isStaticDynCall(node)) {
      reached = getGraphName(getStaticDynCallName(node), 'export');
    } else if (isDynamicDynCall(node)) {
      // this can reach *all* dynCall_* targets, we can't narrow it down
      reached = dynCallNames;
    } else if (isAsmUse(node)) {
      // any remaining asm uses are always rooted in any case
      var name = getAsmOrModuleUseName(node);
      if (exportNameToGraphName.hasOwnProperty(name)) {
        infos[exportNameToGraphName[name]].root = true;
      }
      return;
    }
    if (reached) {
      function addReach(reached) {
        if (defunInfo) {
          defunInfo.reaches[reached] = 1; // defun reaches it
        } else {
          infos[reached].root = true; // in global scope, root it
        }
      }
      if (typeof reached === 'string') {
        addReach(reached);
      } else {
        reached.forEach(addReach);
      }
    }
  }
  defuns.forEach(function(defun) {
    var name = getGraphName(defun.id.name, 'defun');
    var info = infos[name] = {
      name: name,
      reaches: {}
    };
    fullWalk(defun.body, function(node) {
      visitNode(node, info);
    });
  });
  fullWalk(ast, function(node) {
    visitNode(node, null);
  });
  // Final work: print out the graph
  // sort for determinism
  function sortedNamesFromMap(map) {
    var names = [];
    for (var name in map) {
      names.push(name);
    }
    names.sort();
    return names;
  }
  sortedNamesFromMap(infos).forEach(function(name) {
    var info = infos[name];
    info.reaches = sortedNamesFromMap(info.reaches);
    graph.push(info);
  });
  print(JSON.stringify(graph, null, ' '));
}

// Apply graph removals from running wasm-metadce
function applyDCEGraphRemovals(ast) {
  var unused = set(extraInfo.unused);

  fullWalk(ast, function(node) {
    if (isAsmLibraryArgAssign(node)) {
      var assignedObject = getAsmLibraryArgValue(node);
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
        if ((full in unused) &&
            (isAsmUse(value) || !hasSideEffects(value))) {
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
var noPrint = false;

var registry = {
  JSDCE: JSDCE,
  AJSDCE: AJSDCE,
  applyImportAndExportNameChanges: applyImportAndExportNameChanges,
  emitDCEGraph: emitDCEGraph,
  applyDCEGraphRemovals: applyDCEGraphRemovals,
  minifyWhitespace: function() { minifyWhitespace = true },
  noPrint: function() { noPrint = true },
  dump: function() { dump(ast) },
};

passes.forEach(function(pass) {
  registry[pass](ast);
});

if (!noPrint) {
  var terserAst = terser.AST_Node.from_mozilla_ast(ast);
  var output = terserAst.print_to_string({
    beautify: !minifyWhitespace,
    indent_level: minifyWhitespace ? 0 : 1,
    keep_quoted_props: true, // for closure
  });
  print(output);
}

