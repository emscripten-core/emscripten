var acorn = require('acorn');
var terser = require('../third_party/terser');
var fs = require('fs');

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

function warnOnce(msg) {
  if (!warnOnce.msgs) warnOnce.msgs = {};
  if (msg in warnOnce.msgs) return;
  warnOnce.msgs[msg] = true;
  printErr('warning: ' + msg);
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

// This converts the node into something that terser will ignore in a var
// declaration, that is, it is a way to get rid of initial values.
function convertToNothingInVarInit(node) {
  node.type = 'Literal';
  node.value = undefined;
  node.raw = 'undefined';
}

function convertToNull(node) {
  node.type = 'Identifier';
  node.name = 'null';
}

function convertToNullStatement(node) {
  node.type = 'ExpressionStatement';
  node.expression = {
    type: "Literal",
    value: null,
    raw: "null",
    start: 0,
    end: 0,
  };
  node.start = 0;
  node.end = 0;
}

function isNull(node) {
  return node.type === 'Literal' && node.raw === 'null';
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
      case 'BinaryExpression':
      case 'LogicalExpression':
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
      case 'NewExpression': {
        // default to unsafe, but can be safe on some familiar objects
        if (node.callee.type === 'Identifier') {
          var name = node.callee.name;
          if (name === 'TextDecoder'  || name === 'ArrayBuffer' ||
              name === 'Int8Array'    || name === 'Uint8Array' ||
              name === 'Int16Array'   || name === 'Uint16Array' ||
              name === 'Int32Array'   || name === 'Uint32Array' ||
              name === 'Float32Array' || name === 'Float64Array') {
            // no side effects, but the arguments might (we walk them in
            // full walk as well)
            break;
          }
        }
        // not one of the safe cases
        has = true;
        break;
      }
      default: {
        has = true;
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

function JSDCE(ast, aggressive) {
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
        ExpressionStatement(node, c) {
          if (aggressive && !hasSideEffects(node)) {
            if (!isNull(node.expression)) {
              convertToNullStatement(node);
              removed++;
            }
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
      MemberExpression(node, c) {
        c(node.object);
        // Ignore a property identifier (a.X), but notice a[X] (computed
        // is true) and a["X"] (it will be a Literal and not Identifier).
        if (node.property.type !== 'Identifier' || node.computed) {
          c(node.property);
        }
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
  while (iteration() && aggressive) { }
}

// Aggressive JSDCE - multiple iterations
function AJSDCE(ast) {
  JSDCE(ast, /* aggressive= */ true);
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
    } else if (node.type === 'CallExpression' && isAsmUse(node.callee)) { // asm["___wasm_call_ctors"](); -> asm["M"]();
      var callee = node.callee;
      var name = callee.property.value;
      if (mapping[name]) {
        setLiteralValue(callee.property, mapping[name]);
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
  //  var asmLibraryArg = { "abort": abort, "assert": assert, [..] };
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
  // or, in the minimal runtime, it looks like
  //
  //  WebAssembly.instantiate(Module["wasm"], imports).then(function(output) {
  //   var asm = output.instance.exports; // may also not have "var", if
  //                                      // declared outside and used elsewhere
  //   ..
  //   _malloc = asm["malloc"];
  //   ..
  //  });
  var imports = [];
  var defuns = [];
  var dynCallNames = [];
  var nameToGraphName = {};
  var modulePropertyToGraphName = {};
  var exportNameToGraphName = {}; // identical to asm['..'] nameToGraphName
  var foundAsmLibraryArgAssign = false;
  var foundMinimalRuntimeExports = false;
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
        if (value.type === 'Literal' || value.type === 'FunctionExpression') {
          return; // if it's a numeric or function literal, nothing to do here
        }
        if (value.type === 'LogicalExpression') {
          // We may have something like  wasmMemory || Module.wasmMemory  in pthreads code;
          // use the left hand identifier.
          value = value.left;
        }
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
              return;
            }
          }
        }
      }
      // A variable declaration that has no initial values can be ignored in
      // the second pass, these are just declarations, not roots - an actual
      // use must be found in order to root.
      if (!node.declarations.reduce(function(hasInit, decl) {
            return hasInit || !!decl.init
          }, false)) {
        emptyOut(node);
      }
    } else if (node.type === 'FunctionDeclaration') {
      defuns.push(node);
      var name = node.id.name;
      nameToGraphName[name] = getGraphName(name, 'defun');
      emptyOut(node); // ignore this in the second pass; we scan defuns separately
    } else if (node.type === 'FunctionExpression') {
      // Check if this is the minimal runtime exports function, which looks like
      //   function(output) { var asm = output.instance.exports;
      if (node.params.length === 1 && node.params[0].type === 'Identifier' &&
          node.params[0].name === 'output' && node.body.type === 'BlockStatement') {
        var body = node.body.body;
        if (body.length >= 1) {
          var first = body[0];
          var target, value; // "(var?) target = value"
          // Look either for  var asm =  or just   asm =
          if (first.type === 'VariableDeclaration' && first.declarations.length === 1) {
            var decl = first.declarations[0];
            target = decl.id;
            value = decl.init;
          } else if (first.type === 'ExpressionStatement' &&
                     first.expression.type === 'AssignmentExpression') {
            var assign = first.expression;
            if (assign.operator === '=') {
              target = assign.left;
              value = assign.right;
            }
          }
          if (target && target.type === 'Identifier' && target.name === 'asm' && value) {
            if (value.type === 'MemberExpression' &&
                value.object.type === 'MemberExpression' &&
                value.object.object.type === 'Identifier' &&
                value.object.object.name === 'output' &&
                value.object.property.type === 'Identifier' &&
                value.object.property.name === 'instance' &&
                value.property.type === 'Identifier' &&
                value.property.name === 'exports') {
              // This looks very much like what we are looking for.
              assert(!foundMinimalRuntimeExports);
              for (var i = 1; i < body.length; i++) {
                var item = body[i];
                if (item.type === 'ExpressionStatement' &&
                    item.expression.type === 'AssignmentExpression' &&
                    item.expression.operator === '=' &&
                    item.expression.left.type === 'Identifier' &&
                    item.expression.right.type === 'MemberExpression' &&
                    item.expression.right.object.type === 'Identifier' &&
                    item.expression.right.object.name === 'asm' &&
                    item.expression.right.property.type === 'Literal') {
                  var name = item.expression.left.name;
                  var asmName = item.expression.right.property.value;
                  saveAsmExport(name, asmName);
                  emptyOut(item); // ignore all this in the second pass; this does not root
                }
              }
              foundMinimalRuntimeExports = true;
            }
          }
        }
      }
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
    if (Object.prototype.hasOwnProperty.call(nameToGraphName, import_)) {
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
      if (Object.prototype.hasOwnProperty.call(nameToGraphName, name)) {
        reached = nameToGraphName[name];
      }
    } else if (isModuleUse(node)) {
      var name = getAsmOrModuleUseName(node);
      if (Object.prototype.hasOwnProperty.call(modulePropertyToGraphName, name)) {
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
      if (Object.prototype.hasOwnProperty.call(exportNameToGraphName, name)) {
        infos[exportNameToGraphName[name]].root = true;
      }
      return;
    }
    if (reached) {
      function addReach(reached) {
        if (defunInfo) {
          defunInfo.reaches[reached] = 1; // defun reaches it
        } else {
          if (infos[reached]) {
            infos[reached].root = true; // in global scope, root it
          } else {
            // An info might not exist for the identifer if it is missing, for
            // example, we might call Module.dynCall_vi in library code, but it
            // won't exist in a standalone (non-JS) build anyhow. We can ignore
            // it in that case as the JS won't be used, but warn to be safe.
            warnOnce('metadce: missing declaration for ' + reached);
          }
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
          // This will be in a var init, and we just remove that value.
          convertToNothingInVarInit(node);
        }
      }
    } else if (node.type === 'ExpressionStatement') {
      var expr = node.expression;
      // In the minimal runtime code pattern we have just
      //   x = asm['x']
      // and never in a var.
      if (expr.operator === '=' &&
          expr.left.type === 'Identifier' &&
          isAsmUse(expr.right)) {
        var name = expr.left.name;
        if (name === getAsmOrModuleUseName(expr.right)) {
          var full = 'emcc$export$' + name;
          if (full in unused) {
            emptyOut(node);
          }
        }
      }
    }
  });
}

// Need a parser to pass to acorn.Node constructor.
// Create it once and reuse it.
var stubParser = new acorn.Parser();

function createNode(props) {
  var node = new acorn.Node(stubParser);
  Object.assign(node, props);
  return node;
}

function makeCallExpression(node, name, args) {
  Object.assign(node, {
    type: 'CallExpression',
    callee: createNode({
      type: 'Identifier',
      name: name,
    }),
    arguments: args
  });
}

function isEmscriptenHEAP(name) {
  switch (name) {
    case 'HEAP8':  case 'HEAPU8':
    case 'HEAP16': case 'HEAPU16':
    case 'HEAP32': case 'HEAPU32':
    case 'HEAPF32': case 'HEAPF64': {
      return true;
    }
    default: {
      return false;
    }
  }
}

// Instrument heap accesses to call GROWABLE_HEAP_* helper functions instead, which allows
// pthreads + memory growth to work (we check if the memory was grown on another thread
// in each access), see #8365.
function growableHeap(ast) {
  recursiveWalk(ast, {
    AssignmentExpression: function(node) {
      if (node.left.type === 'Identifier' &&
          isEmscriptenHEAP(node.left.name)) {
        // Don't transform initial setup of the arrays.
        return;
      }
      growableHeap(node.left);
      growableHeap(node.right);
    },
    VariableDeclaration: function(node) {
      // Don't transform the var declarations for HEAP8 etc
      node.declarations.forEach(function(decl) {
        // but do transform anything that sets a var to
        // something from HEAP8 etc
        if (decl.init) {
          growableHeap(decl.init);
        }
      });
    },
    Identifier: function(node) {
      if (node.name.startsWith('HEAP')) {
        // Turn HEAP8 into GROWABLE_HEAP_I8() etc
        switch (node.name) {
          case 'HEAP8': {
            makeCallExpression(node, 'GROWABLE_HEAP_I8', []);
            break;
          }
          case 'HEAPU8': {
            makeCallExpression(node, 'GROWABLE_HEAP_U8', []);
            break;
          }
          case 'HEAP16': {
            makeCallExpression(node, 'GROWABLE_HEAP_I16', []);
            break;
          }
          case 'HEAPU16': {
            makeCallExpression(node, 'GROWABLE_HEAP_U16', []);
            break;
          }
          case 'HEAP32': {
            makeCallExpression(node, 'GROWABLE_HEAP_I32', []);
            break;
          }
          case 'HEAPU32': {
            makeCallExpression(node, 'GROWABLE_HEAP_U32', []);
            break;
          }
          case 'HEAPF32': {
            makeCallExpression(node, 'GROWABLE_HEAP_F32', []);
            break;
          }
          case 'HEAPF64': {
            makeCallExpression(node, 'GROWABLE_HEAP_F64', []);
            break;
          }
          default: {}
        }
      }
    }
  });
}

// Make all JS pointers unsigned. We do this by modifying things like
// HEAP32[X >> 2] to HEAP32[X >>> 2]. We also need to handle the case of
// HEAP32[X] and make that HEAP32[X >>> 0], things like subarray(), etc.
function unsignPointers(ast) {
  // Aside from the standard emscripten HEAP*s, also identify just "HEAP"/"heap"
  // as representing a heap. This can be used in JS library code in order
  // to get this pass to fix it up.
  function isHeap(name) {
    return isEmscriptenHEAP(name) || name === 'heap' || name === 'HEAP';
  }

  function unsign(node) {
    // The pointer is often a >> shift, which we can just turn into >>>
    if (node.type === 'BinaryExpression') {
      if (node.operator === '>>') {
        node.operator = '>>>';
        return node;
      }
    }
    // If nothing else worked out, add a new shift.
    return {
      type: 'BinaryExpression',
      left: node,
      operator: ">>>",
      right: {
        type: "Literal",
        value: 0,
        raw: "0",
        start: 0,
        end: 0,
      },
      start: 0,
      end: 0,
    };
  }

  fullWalk(ast, function(node) {
    if (node.type === 'MemberExpression') {
      // Check if this is HEAP*[?]
      if (node.object.type === 'Identifier' &&
          isHeap(node.object.name) &&
          node.computed) {
        node.property = unsign(node.property);
      }
    } else if (node.type === 'CallExpression') {
      if (node.callee.type === 'MemberExpression' &&
          node.callee.object.type === 'Identifier' &&
          isHeap(node.callee.object.name) &&
          node.callee.property.type === 'Identifier' &&
          !node.computed) {
        // This is a call on HEAP*.?. Specific things we need to fix up are
        // subarray, set, and copyWithin. TODO more?
        if (node.callee.property.name === 'set') {
          if (node.arguments.length >= 2) {
            node.arguments[1] = unsign(node.arguments[1]);
          }
        } else if (node.callee.property.name === 'subarray') {
          if (node.arguments.length >= 1) {
            node.arguments[0] = unsign(node.arguments[0]);
            if (node.arguments.length >= 2) {
              node.arguments[1] = unsign(node.arguments[1]);
            }
          }
        } else if (node.callee.property.name === 'copyWithin') {
          node.arguments[0] = unsign(node.arguments[0]);
          node.arguments[1] = unsign(node.arguments[1]);
          if (node.arguments.length >= 3) {
            node.arguments[2] = unsign(node.arguments[2]);
          }
        }
      }
    }
  });
}

// Replace direct HEAP* loads/stores with calls into C, in which ASan checks
// are applied. That lets ASan cover JS too.
function asanify(ast) {
  function isHEAPAccess(node) {
    return node.type === 'MemberExpression' &&
           node.object.type === 'Identifier' &&
           node.computed && // notice a[X] but not a.X
           isEmscriptenHEAP(node.object.name);
  }

  recursiveWalk(ast, {
    FunctionDeclaration(node, c) {
      if (node.id.type === 'Identifier' && node.id.name.startsWith('_asan_js_')) {
        // do not recurse into this js impl function, which we use during
        // startup before the wasm is ready
      } else {
        c(node.body);
      }
    },
    AssignmentExpression(node, c) {
      var target = node.left;
      var value = node.right;
      c(value);
      if (isHEAPAccess(target)) {
        // Instrument a store.
        var ptr = target.property;
        switch (target.object.name) {
          case 'HEAP8': {
            makeCallExpression(node, '_asan_js_store_1', [ptr, value]);
            break;
          }
          case 'HEAPU8': {
            makeCallExpression(node, '_asan_js_store_1u', [ptr, value]);
            break;
          }
          case 'HEAP16': {
            makeCallExpression(node, '_asan_js_store_2', [ptr, value]);
            break;
          }
          case 'HEAPU16': {
            makeCallExpression(node, '_asan_js_store_2u', [ptr, value]);
            break;
          }
          case 'HEAP32': {
            makeCallExpression(node, '_asan_js_store_4', [ptr, value]);
            break;
          }
          case 'HEAPU32': {
            makeCallExpression(node, '_asan_js_store_4u', [ptr, value]);
            break;
          }
          case 'HEAPF32': {
            makeCallExpression(node, '_asan_js_store_f', [ptr, value]);
            break;
          }
          case 'HEAPF64': {
            makeCallExpression(node, '_asan_js_store_d', [ptr, value]);
            break;
          }
          default: {}
        }
      } else {
        c(target);
      }
    },
    MemberExpression(node, c) {
      c(node.property);
      if (!isHEAPAccess(node)) {
        c(node.object);
      } else {
        // Instrument a load.
        var ptr = node.property;
        switch (node.object.name) {
          case 'HEAP8': {
            makeCallExpression(node, '_asan_js_load_1', [ptr]);
            break;
          }
          case 'HEAPU8': {
            makeCallExpression(node, '_asan_js_load_1u', [ptr]);
            break;
          }
          case 'HEAP16': {
            makeCallExpression(node, '_asan_js_load_2', [ptr]);
            break;
          }
          case 'HEAPU16': {
            makeCallExpression(node, '_asan_js_load_2u', [ptr]);
            break;
          }
          case 'HEAP32': {
            makeCallExpression(node, '_asan_js_load_4', [ptr]);
            break;
          }
          case 'HEAPU32': {
            makeCallExpression(node, '_asan_js_load_4u', [ptr]);
            break;
          }
          case 'HEAPF32': {
            makeCallExpression(node, '_asan_js_load_f', [ptr]);
            break;
          }
          case 'HEAPF64': {
            makeCallExpression(node, '_asan_js_load_d', [ptr]);
            break;
          }
          default: {}
        }
      }
    }
  });
}

function reattachComments(ast, comments) {
  var symbols = [];

  // Collect all code symbols
  ast.walk(new terser.TreeWalker(function(node) {
    if (node.start && node.start.pos) {
      symbols.push(node);
    }
  }));

  // Sort them by ascending line number
  symbols.sort((a,b) => {
    return a.start.pos - b.start.pos;
  })

  // Walk through all comments in ascending line number, and match each
  // comment to the appropriate code block.
  for(var i = 0, j = 0; i < comments.length; ++i) {
    while(j < symbols.length && symbols[j].start.pos < comments[i].end) {
      ++j;
    }
    if (j >= symbols.length) {
      break;
    }
    if (symbols[j].start.pos - comments[i].end > 20) {
      // This comment is too far away to refer to the given symbol. Drop
      // the comment altogether.
      continue;
    }
    if (!Array.isArray(symbols[j].start.comments_before)) {
      symbols[j].start.comments_before = [];
    }
    symbols[j].start.comments_before.push(new terser.AST_Token({
        end: undefined,
        quote: undefined,
        raw: undefined,
        file: '0',
        comments_after: undefined,
        comments_before: undefined,
        nlb: false,
        endpos: undefined,
        endcol: undefined,
        endline: undefined,
        pos: undefined,
        col: undefined,
        line: undefined,
        value: comments[i].value,
        type: comments[i].type == 'Line' ? 'comment' : 'comment2',
        flags: 0
      }));
  }
}

// Main

var arguments = process['argv'].slice(2);;
// If enabled, output retains parentheses and comments so that the
// output can further be passed out to Closure.
var closureFriendly = arguments.indexOf('--closureFriendly');
if (closureFriendly > -1) {
  arguments.splice(closureFriendly, 1);
  closureFriendly = true;
} else {
  closureFriendly = false;
}

var infile = arguments[0];
var passes = arguments.slice(1);

var input = read(infile);
var extraInfoStart = input.lastIndexOf('// EXTRA_INFO:')
var extraInfo = null;
if (extraInfoStart > 0) {
  extraInfo = JSON.parse(input.substr(extraInfoStart + 14));
}
// Collect all JS code comments to this array so that we can retain them in the outputted code
// if --closureFriendly was requested.
var sourceComments = [];
var ast;
try {
  ast = acorn.parse(input, {
    // Keep in sync with --language_in that we pass to closure in building.py
    ecmaVersion: 2018,
    preserveParens: closureFriendly,
    onComment: closureFriendly ? sourceComments : undefined
  });
} catch (err) {
  err.message += (function() {
    var errorMessage = '\n' + input.split(acorn.lineBreak)[err.loc.line - 1] + '\n';
    var column = err.loc.column;
    while (column--) {
      errorMessage += ' ';
    }
    errorMessage += '^\n';
    return errorMessage;
  })();
  throw err;
}

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
  growableHeap: growableHeap,
  unsignPointers: unsignPointers,
  asanify: asanify
};

passes.forEach(function(pass) {
  registry[pass](ast);
});

if (!noPrint) {
  var terserAst = terser.AST_Node.from_mozilla_ast(ast);

  if (closureFriendly) {
    reattachComments(terserAst, sourceComments);
  }

  var output = terserAst.print_to_string({
    beautify: !minifyWhitespace,
    indent_level: minifyWhitespace ? 0 : 1,
    keep_quoted_props: true, // for closure
    comments: true // for closure as well
  });
  print(output);
}
