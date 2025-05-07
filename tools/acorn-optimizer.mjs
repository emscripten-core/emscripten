#!/usr/bin/env node

import {readFileSync, writeFileSync} from 'node:fs';
import {parseArgs} from 'node:util';
import {parse} from '@babel/parser';
import {generate} from '@babel/generator';
import * as t from '@babel/types';
import { codeFrameColumns } from "@babel/code-frame";
import * as assert from 'node:assert/strict';

const EXTRA_INFO_PREFIX = '// EXTRA_INFO:';

/**
 * @param {boolean} condition
 * @param {t.Node} node
 * @param {string} message
 */
function assertAt(condition, node, message) {
  if (condition) return;
  if (!('EMCC_DEBUG_SAVE' in process.env)) {
    message += ' (use EMCC_DEBUG_SAVE=1 to preserve temporary inputs)';
  }
  const err = new Error(message);
  Error.captureStackTrace(err, assertAt);
  err['loc'] = node.loc;
  throw err;
}

// Visits and walks
// (We don't use acorn-walk because it ignores x in 'x = y'.)

/**
 * @param {t.Node} node
 * @param {(node: t.Node) => void} c
 */
function visitChildren(node, c) {
  // emptyOut() and temporary ignoring may mark nodes as empty,
  // while they have properties with children we should ignore.
  if (t.isEmptyStatement(node)) {
    return;
  }
  /**
   * @param {t.Node} child
   */
  function maybeChild(child) {
    if (t.isNode(child)) {
      c(child);
      return true;
    }
    return false;
  }
  for (const child of Object.values(node)) {
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
/**
 * @param {t.Node} node
 * @param {{ [K in t.Node['type']]?: (node: t.Node & { type: K }) => void }} cs
 */
function simpleWalk(node, cs) {
  visitChildren(node, (child) => simpleWalk(child, cs));
  cs[node.type]?.(node);
}

// Full post-order walk, calling a single function for all types. If |pre| is
// provided, it is called in pre-order (before children).
/**
 * @param {t.Node} node
 * @param {(node: t.Node) => void} c
 * @param {(node: t.Node) => void} [pre]
 */
function fullWalk(node, c, pre) {
  pre?.(node);
  visitChildren(node, (child) => fullWalk(child, c, pre));
  c(node);
}

// Recursive post-order walk, calling properties on an object by node type,
// if the type exists, and if so leaving recursion to that function.
/**
 * @param {t.Node} node
 * @param {{ [K in t.Node['type']]?: (node: t.Node & { type: K }, c: (child: t.Node) => void) => void }} cs
 */
function recursiveWalk(node, cs) {
  return (function c(node) {
    const handler = cs[node.type];
    if (!handler) {
      visitChildren(node, (child) => recursiveWalk(child, cs));
    } else {
      handler(node, c);
    }
  })(node);
}

// AST Utilities

/**
 * @param {t.Node} node
 * @returns {t.EmptyStatement}
 */
function emptyOut(node) {
  return Object.assign(node, t.emptyStatement());
}

/**
 * @param {t.StringLiteral} item
 * @param {string} value
 */
function setLiteralValue(item, value) {
  item.value = value;
  item.extra = null;
}

/**
 * @param {t.Node} node
 */
function dump(node) {
  console.log(JSON.stringify(node, null, ' '));
}

// Mark inner scopes temporarily as empty statements. Returns
// a special object that must be used to restore them.
/**
 * @param {t.Node} node
 */
function ignoreInnerScopes(node) {
  /** @type {WeakMap<t.Node, t.Node['type']>} */
  const map = new WeakMap();
  /**
   * @param {t.Node} node
   */
  function ignore(node) {
    map.set(node, node.type);
    emptyOut(node);
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
/**
 * @param {t.Node} node
 * @param {WeakMap<t.Node, t.Node['type']>} map
 */
function restoreInnerScopes(node, map) {
  fullWalk(node, (node) => {
    if (map.has(node)) {
      node.type = map.get(node);
      map.delete(node);
      restoreInnerScopes(node, map);
    }
  });
}

/**
 * @param {t.Node} node
 */
function hasSideEffects(node) {
  // Conservative analysis.
  const map = ignoreInnerScopes(node);
  const hasSideEffects = Symbol('hasSideEffects');
  try {
    fullWalk(node, (node) => {
      if (
        t.isPureish(node) ||
        t.isBinary(node) ||
        t.isUnaryLike(node) ||
        t.isProperty(node) ||
        t.isDeclaration(node) ||
        t.isPatternLike(node) ||
        t.isBlock(node)
      ) {
        return; // safe
      }
      switch (node.type) {
        // TODO: go through all the ESTree spec
        case 'ParenthesizedExpression':
        case 'ExpressionStatement':
        case 'ConditionalExpression':
        case 'VariableDeclarator':
        case 'ObjectExpression':
        case 'ArrayExpression':
        case 'EmptyStatement': {
          return; // safe
        }
        case 'MemberExpression':
          // safe if on Math (or other familiar objects, TODO)
          if (t.isIdentifier(node.object, {name: 'Math'})) {
            return;
          }
          break;
        case 'NewExpression':
          // default to unsafe, but can be safe on some familiar objects
          if (t.isIdentifier(node.callee)) {
            const name = node.callee.name;
            if (
              name === 'TextDecoder' ||
              name === 'ArrayBuffer' ||
              name === 'Int8Array' ||
              name === 'Uint8Array' ||
              name === 'Int16Array' ||
              name === 'Uint16Array' ||
              name === 'Int32Array' ||
              name === 'Uint32Array' ||
              name === 'Float32Array' ||
              name === 'Float64Array'
            ) {
              // no side effects, but the arguments might (we walk them in
              // full walk as well)
              return;
            }
          }
          break;
      }
      // not one of the safe cases
      throw hasSideEffects;
    });
    return false;
  } catch (err) {
    if (err !== hasSideEffects) {
      throw err;
    }
    // we hit a side effect, return true
    return true;
  } finally {
    restoreInnerScopes(node, map);
  }
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

/**
 * @param {t.File} ast
 * @param {boolean} aggressive
 */
function JSDCE(ast, aggressive = false) {
  function iteration() {
    let removed = 0;
    const scopes = [{}]; // begin with empty toplevel scope
    /** @type {WeakMap<t.VariableDeclaration, t.For>} */
    let forLhs = new WeakMap();

    /**
     * @param {string} name
     */
    function ensureData(scope, name) {
      if (Object.prototype.hasOwnProperty.call(scope, name)) return scope[name];
      scope[name] = {
        def: 0,
        use: 0,
        param: 0, // true for function params, which cannot be eliminated
      };
      return scope[name];
    }
    /**
     * @param {t.Node} ast
     * @param {Set<string>} names
     */
    function cleanUp(ast, names) {
      recursiveWalk(ast, {
        ForStatement(node, c) {
          if (t.isVariableDeclaration(node.init)) {
            forLhs.set(node.init, node);
          }
          visitChildren(node, c);
        },
        ForInStatement(node, c) {
          if (t.isVariableDeclaration(node.left)) {
            forLhs.set(node.left, node);
          }
          visitChildren(node, c);
        },
        ForOfStatement(node, c) {
          if (t.isVariableDeclaration(node.left)) {
            forLhs.set(node.left, node);
          }
          visitChildren(node, c);
        },
        VariableDeclaration(node, _c) {
          // If we're in the LHS of a for-in or for-of loop, we need to keep at least one variable for the loop to remain valid.
          const parentForLoop = forLhs.get(node);
          const needsAtLeastOneVar = t.isForXStatement(parentForLoop);
          let removedHere = 0;
          node.declarations = node.declarations.filter((node, i) => {
            t.assertVariableDeclarator(node);
            const id = node.id;
            if (t.isObjectPattern(id) || t.isArrayPattern(id)) {
              // TODO: DCE into object patterns, that is, things like
              //         let { a, b } = ..
              //         let [ a, b ] = ..
              return true;
            }
            t.assertIdentifier(id);
            const curr = id.name;
            const value = node.init;
            const keep =
              !names.has(curr) ||
              (value && hasSideEffects(value)) ||
              (needsAtLeastOneVar && i === 0);
            if (!keep) removedHere = 1;
            return keep;
          });
          removed += removedHere;
          if (node.declarations.length === 0) {
            if (t.isForStatement(parentForLoop)) {
              // for (var i = 0; ...) -> for (; ...)
              parentForLoop.init = null;
            } else {
              emptyOut(node);
            }
          }
        },
        ExpressionStatement(node, _c) {
          if (aggressive && !hasSideEffects(node)) {
            emptyOut(node);
            removed++;
          }
        },
        FunctionDeclaration(node, _c) {
          if (names.has(node.id.name)) {
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
    }

    /**
     * @param {t.Function} node
     * @param {(node: t.Node) => void} visitExpr
     */
    function handleFunction(node, visitExpr) {
      // defun names matter - function names (the y in var x = function y() {..}) are just for stack traces.
      if (t.isFunctionDeclaration(node)) {
        ensureData(scopes[scopes.length - 1], node.id.name).def = 1;
      }
      const scope = {};
      scopes.push(scope);
      for (const param of node.params) {
        recursiveWalk(param, {
          AssignmentPattern(node, visitPat) {
            visitPat(node.left);
            visitExpr(node.right);
          },
          ObjectProperty(node, visitPat) {
            if (node.computed) {
              visitExpr(node.key);
            }
            visitPat(node.value);
          },
          Identifier({name}) {
            ensureData(scope, name).def = 1;
            scope[name].param = 1;
          },
        });
      }
      visitExpr(node.body);
      // we can ignore self-references, i.e., references to ourselves inside
      // ourselves, for named defined (defun) functions
      const ownName = node['id']?.name;
      const names = new Set();
      for (const name in scopes.pop()) {
        if (name === ownName) continue;
        const data = scope[name];
        if (data.use && !data.def) {
          // this is used from a higher scope, propagate the use down
          ensureData(scopes[scopes.length - 1], name).use = 1;
          continue;
        }
        if (data.def && !data.use && !data.param) {
          // this is eliminateable!
          names.add(name);
        }
      }
      cleanUp(node.body, names);
    }

    recursiveWalk(ast, {
      VariableDeclarator(node, c) {
        recursiveWalk(node.id, {
          AssignmentPattern(node, visitPat) {
            visitPat(node.left);
            c(node.right);
          },
          ObjectProperty(node, visitPat) {
            if (node.computed) {
              c(node.key);
            }
            visitPat(node.value);
          },
          Identifier(id) {
            const name = id.name;
            ensureData(scopes[scopes.length - 1], name).def = 1;
          },
        });

        if (node.init) c(node.init);
      },
      ObjectMethod(node, c) {
        if (node.computed) {
          c(node.key);
        }
        handleFunction(node, c);
      },
      ObjectProperty(node, c) {
        if (node.computed) {
          c(node.key);
        }
        c(node.value);
      },
      MemberExpression(node, c) {
        c(node.object);
        // Ignore a property identifier (a.X), but notice a[X] (computed props).
        if (node.computed) {
          c(node.property);
        }
      },
      FunctionDeclaration(node, c) {
        handleFunction(node, c);
      },
      FunctionExpression(node, c) {
        handleFunction(node, c);
      },
      ArrowFunctionExpression(node, c) {
        handleFunction(node, c);
      },
      Identifier(node, _c) {
        ensureData(scopes[scopes.length - 1], node.name).use = 1;
      },
      ExportDefaultDeclaration(node, c) {
        t.assertFunctionDeclaration(node.declaration);
        const name = node.declaration.id.name;
        ensureData(scopes[scopes.length - 1], name).use = 1;
        c(node.declaration);
      },
      ExportNamedDeclaration(node, c) {
        if (node.declaration) {
          if (t.isFunctionDeclaration(node.declaration)) {
            const name = node.declaration.id.name;
            ensureData(scopes[scopes.length - 1], name).use = 1;
          } else {
            t.assertVariableDeclaration(node.declaration);
            for (const decl of node.declaration.declarations) {
              t.assertIdentifier(decl.id);
              const name = decl.id.name;
              ensureData(scopes[scopes.length - 1], name).use = 1;
            }
          }
          c(node.declaration);
        } else {
          for (const specifier of node.specifiers) {
            t.assertExportSpecifier(specifier);
            const name = specifier.local.name;
            ensureData(scopes[scopes.length - 1], name).use = 1;
          }
        }
      },
    });

    // toplevel
    const scope = scopes.pop();
    assert.equal(scopes.length, 0);

    const names = new Set();
    for (const [name, data] of Object.entries(scope)) {
      if (data.def && !data.use) {
        assert.ok(!data.param); // can't be
        // this is eliminateable!
        names.add(name);
      }
    }
    cleanUp(ast, names);
    return removed;
  }
  while (iteration() && aggressive) {} // eslint-disable-line no-empty
}

// Aggressive JSDCE - multiple iterations
/**
 * @param {t.File} ast
 */
function AJSDCE(ast) {
  JSDCE(ast, /* aggressive= */ true);
}

/**
 * @param {t.Node} node
 * @returns {node is t.AssignmentExpression & { left: t.Identifier, right: t.ObjectExpression } | t.VariableDeclaration & { declarations: [t.VariableDeclarator & { id: t.Identifier, init: t.ObjectExpression }] }}
 */
function isWasmImportsAssign(node) {
  // var wasmImports = ..
  //   or
  // wasmImports = ..
  if (
    t.isAssignmentExpression(node) &&
    t.isIdentifier(node.left, {name: 'wasmImports'}) &&
    t.isObjectExpression(node.right)
  ) {
    return true;
  }
  return (
    t.isVariableDeclaration(node) &&
    node.declarations.length === 1 &&
    t.isIdentifier(node.declarations[0].id, {name: 'wasmImports'}) &&
    t.isObjectExpression(node.declarations[0].init)
  );
}

/**
 * @param {t.AssignmentExpression & { left: t.Identifier, right: t.ObjectExpression } | t.VariableDeclaration & { declarations: [t.VariableDeclarator & { id: t.Identifier, init: t.ObjectExpression }] }} node
 */
function getWasmImportsValue(node) {
  if (t.isVariableDeclaration(node)) {
    return node.declarations[0].init;
  } else {
    return node.right;
  }
}

/**
 * @param {t.Node} node
 * @returns {node is t.MemberExpression & { object: t.Identifier, property: t.StringLiteral }}
 */
function isExportUse(node) {
  // Match usages of symbols on the `wasmExports` object. e.g:
  //   wasmExports['X']
  return (
    t.isMemberExpression(node) &&
    t.isIdentifier(node.object) &&
    t.isStringLiteral(node.property) &&
    node.object.name === 'wasmExports'
  );
}

/**
 * @param {t.MemberExpression & { property: t.StringLiteral }} node
 */
function getExportOrModuleUseName(node) {
  return node.property.value;
}

/**
 * @param {t.Node} node
 * @returns {node is t.MemberExpression & { object: t.Identifier, property: t.StringLiteral }}
 */
function isModuleUse(node) {
  return (
    t.isMemberExpression(node) && // Module['X']
    t.isIdentifier(node.object, {name: 'Module'}) &&
    t.isStringLiteral(node.property)
  );
}

// Apply import/export name changes (after minifying them)
/**
 * @param {t.Node} ast
 */
function applyImportAndExportNameChanges(ast) {
  const mapping = extraInfo.mapping;
  fullWalk(ast, (node) => {
    if (isWasmImportsAssign(node)) {
      const assignedObject = getWasmImportsValue(node);
      assignedObject.properties.forEach((item) => {
        t.assertObjectProperty(item, {computed: false});
        t.assertIdentifier(item.key);
        if (mapping[item.key.name]) {
          item.key.name = mapping[item.key.name];
        }
      });
    } else if (t.isAssignmentExpression(node)) {
      const value = node.right;
      if (isExportUse(value)) {
        const name = value.property.value;
        if (mapping[name]) {
          setLiteralValue(value.property, mapping[name]);
        }
      }
    } else if (t.isCallExpression(node) && isExportUse(node.callee)) {
      // wasmExports["___wasm_call_ctors"](); -> wasmExports["M"]();
      const callee = node.callee;
      const name = callee.property.value;
      if (mapping[name]) {
        setLiteralValue(callee.property, mapping[name]);
      }
    } else if (isExportUse(node)) {
      const prop = node.property;
      const name = prop.value;
      if (mapping[name]) {
        setLiteralValue(prop, mapping[name]);
      }
    }
  });
}

// A static dyncall is dynCall('vii', ..), which is actually static even
// though we call dynCall() - we see the string signature statically.
/**
 * @param {t.Node} node
 * @returns {node is t.CallExpression & { arguments: [t.StringLiteral] }}
 */
function isStaticDynCall(node) {
  return (
    t.isCallExpression(node) &&
    t.isIdentifier(node.callee, {name: 'dynCall'}) &&
    t.isStringLiteral(node.arguments[0])
  );
}

/**
 * @param {t.CallExpression & { arguments: [t.StringLiteral] }} node
 */
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
/**
 * @param {t.Node} node
 * @returns {node is (t.CallExpression & { callee: t.Identifier }) | t.StringLiteral}
 */
function isDynamicDynCall(node) {
  return (
    (t.isCallExpression(node) &&
      t.isIdentifier(node.callee, {name: 'dynCall'}) &&
      !t.isStringLiteral(node.arguments[0])) ||
    t.isStringLiteral(node, {value: 'dynCall_'})
  );
}

//
// Matches the wasm export wrappers generated by emcc (see make_export_wrappers
// in emscripten.py). For example, the right hand side of these assignments:
//
//   var _foo = (a0, a1) => (_foo = wasmExports['foo'])(a0, a1):
//
// or
//
//   var _foo = (a0, a1) => (_foo = Module['_foo'] = wasmExports['foo'])(a0, a1):
//
/**
 * @param {t.ArrowFunctionExpression} f
 */
function isExportWrapperFunction(f) {
  if (!t.isCallExpression(f.body)) return null;
  let callee = f.body.callee;
  if (t.isParenthesizedExpression(callee)) {
    callee = callee.expression;
  }
  if (!t.isAssignmentExpression(callee)) return null;
  var rhs = callee.right;
  if (t.isAssignmentExpression(rhs)) {
    rhs = rhs.right;
  }
  if (!t.isMemberExpression(rhs) || !isExportUse(rhs)) return null;
  return getExportOrModuleUseName(rhs);
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
/**
 * @param {t.Node} ast
 */
function emitDCEGraph(ast) {
  // First pass: find the wasm imports and exports, and the toplevel
  // defuns, and save them on the side, removing them from the AST,
  // which makes the second pass simpler.
  //
  // The imports that wasm receives look like this:
  //
  //  var wasmImports = { "abort": abort, "assert": assert, [..] };
  //
  // The exports are trickier, as they have a different form whether or not
  // async compilation is enabled. It can be either:
  //
  //  var _malloc = Module['_malloc'] = wasmExports['_malloc'];
  //
  // or
  //
  //  var _malloc = wasmExports['_malloc'];
  //
  // or
  //
  //  var _malloc = Module['_malloc'] = (x) => wasmExports['_malloc'](x);
  //
  // or, in the minimal runtime, it looks like
  //
  //  function assignWasmExports(wasmExports)
  //   ..
  //   _malloc = wasmExports["malloc"];
  //   ..
  //  });
  const imports = [];
  const defuns = [];
  const dynCallNames = [];
  const nameToGraphName = {};
  const modulePropertyToGraphName = {};
  const exportNameToGraphName = {}; // identical to wasmExports['..'] nameToGraphName
  const graph = [];
  let foundWasmImportsAssign = false;
  let foundMinimalRuntimeExports = false;

  /**
   * @param {string} name
   * @param {string} asmName
   */
  function saveAsmExport(name, asmName) {
    // the asmName is what the wasm provides directly; the outside JS
    // name may be slightly different (extra "_" in wasm backend)
    const graphName = getGraphName(name, 'export');
    nameToGraphName[name] = graphName;
    modulePropertyToGraphName[name] = graphName;
    exportNameToGraphName[asmName] = graphName;
    if (/^dynCall_/.test(name)) {
      dynCallNames.push(graphName);
    }
  }

  // We track defined functions very carefully, so that we can remove them and
  // the things they call, but other function scopes (like arrow functions and
  // object methods) are trickier to track (object methods require knowing what
  // object a function name is called on), so we do not track those. We consider
  // all content inside them as top-level, which means it is used.
  var specialScopes = 0;

  fullWalk(
    ast,
    (node) => {
      if (isWasmImportsAssign(node)) {
        const assignedObject = getWasmImportsValue(node);
        assignedObject.properties.forEach((item) => {
          t.assertObjectProperty(item);
          let value = item.value;
          if (t.isLiteral(value) || t.isFunctionExpression(value)) {
            return; // if it's a numeric or function literal, nothing to do here
          }
          if (t.isLogicalExpression(value)) {
            // We may have something like  wasmMemory || Module.wasmMemory  in pthreads code;
            // use the left hand identifier.
            value = value.left;
          }
          t.assertIdentifier(value);
          const nativeName = item.computed
            ? (t.assertStringLiteral(item.key), item.key.value)
            : (t.assertIdentifier(item.key), item.key.name);
          assert.ok(nativeName);
          imports.push([value.name, nativeName]);
        });
        foundWasmImportsAssign = true;
        emptyOut(node); // ignore this in the second pass; this does not root
      } else if (t.isAssignmentExpression(node)) {
        const target = node.left;
        // Ignore assignment to the wasmExports object (as happens in
        // applySignatureConversions).
        if (isExportUse(target)) {
          emptyOut(node);
        }
      } else if (t.isVariableDeclaration(node)) {
        if (node.declarations.length === 1) {
          const item = node.declarations[0];
          if (t.isIdentifier(item.id)) {
            const name = item.id.name;
            const value = item.init;
            if (value && isExportUse(value)) {
              const asmName = getExportOrModuleUseName(value);
              // this is:
              //  var _x = wasmExports['x'];
              saveAsmExport(name, asmName);
              emptyOut(node);
            } else if (value && t.isArrowFunctionExpression(value)) {
              // this is
              //  () => (x = wasmExports['x'])(..)
              // or
              //  () => (x = Module['_x'] = wasmExports['x'])(..)
              let asmName = isExportWrapperFunction(value);
              if (asmName) {
                saveAsmExport(name, asmName);
                emptyOut(node);
              }
            } else if (value && t.isAssignmentExpression(value)) {
              const assigned = value.left;
              if (isModuleUse(assigned) && getExportOrModuleUseName(assigned) === name) {
                // this is
                //  var x = Module['x'] = ?
                // which looks like a wasm export being received. confirm with the asm use
                let found = 0;
                let asmName;
                fullWalk(value.right, (node) => {
                  if (isExportUse(node)) {
                    found++;
                    asmName = getExportOrModuleUseName(node);
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
                if (t.isNumericLiteral(value.right)) {
                  // this is
                  //  var x = Module['x'] = 1234;
                  // this form occurs when global addresses are exported from the
                  // module.  It doesn't constitute a usage.
                  emptyOut(node);
                }
              }
            }
          }
        }
        // A variable declaration that has no initial values can be ignored in
        // the second pass, these are just declarations, not roots - an actual
        // use must be found in order to root.
        if (!node.declarations.reduce((hasInit, decl) => hasInit || !!decl.init, false)) {
          emptyOut(node);
        }
      } else if (t.isFunctionDeclaration(node)) {
        const name = node.id.name;
        // Check if this is the minimal runtime exports function, which looks like
        //   function assignWasmExports(wasmExports)
        if (
          name === 'assignWasmExports' &&
          node.params.length === 1 &&
          t.isIdentifier(node.params[0], {name: 'wasmExports'})
        ) {
          // This looks very much like what we are looking for.
          const body = node.body.body;
          assert.ok(!foundMinimalRuntimeExports);
          foundMinimalRuntimeExports = true;
          for (const item of body) {
            if (
              t.isExpressionStatement(item) &&
              t.isAssignmentExpression(item.expression, {operator: '='}) &&
              t.isIdentifier(item.expression.left) &&
              t.isMemberExpression(item.expression.right) &&
              t.isIdentifier(item.expression.right.object, {name: 'wasmExports'}) &&
              t.isStringLiteral(item.expression.right.property)
            ) {
              const name = item.expression.left.name;
              const asmName = item.expression.right.property.value;
              saveAsmExport(name, asmName);
              emptyOut(item); // ignore all this in the second pass; this does not root
            }
          }
        } else if (!specialScopes) {
          defuns.push(node);
          nameToGraphName[name] = getGraphName(name, 'defun');
          emptyOut(node); // ignore this in the second pass; we scan defuns separately
        }
      } else if (t.isArrowFunctionExpression(node) || t.isObjectMethod(node)) {
        assert.ok(specialScopes > 0);
        specialScopes--;
      }
    },
    (node) => {
      // Pre-walking logic. We note special scopes (see above).
      if (t.isArrowFunctionExpression(node) || t.isObjectMethod(node)) {
        specialScopes++;
      }
    },
  );
  // Scoping must balance out.
  assert.ok(specialScopes === 0);
  // We must have found the info we need.
  assert.ok(
    foundWasmImportsAssign,
    'could not find the assignment to "wasmImports". perhaps --pre-js or --post-js code moved it out of the global scope? (things like that should be done after emcc runs, as they do not need to be run through the optimizer which is the special thing about --pre-js/--post-js code)',
  );
  // Read exports that were declared in extraInfo
  if (extraInfo) {
    for (const exp of extraInfo.exports) {
      saveAsmExport(exp[0], exp[1]);
    }
  }

  // Second pass: everything used in the toplevel scope is rooted;
  // things used in defun scopes create links
  /**
   * @param {string} name
   * @param {string} what
   */
  function getGraphName(name, what) {
    return 'emcc$' + what + '$' + name;
  }
  const infos = {}; // the graph name of the item => info for it
  for (const [jsName, nativeName] of imports) {
    const name = getGraphName(jsName, 'import');
    const info = (infos[name] = {
      name: name,
      import: ['env', nativeName],
      reaches: {},
    });
    if (nameToGraphName.hasOwnProperty(jsName)) {
      info.reaches[nameToGraphName[jsName]] = 1;
    } // otherwise, it's a number, ignore
  }
  for (const [e, _] of Object.entries(exportNameToGraphName)) {
    const name = exportNameToGraphName[e];
    infos[name] = {
      name: name,
      export: e,
      reaches: {},
    };
  }
  // a function that handles a node we visit, in either a defun or
  // the toplevel scope (in which case the second param is not provided)
  /**
   * @param {t.Node} node
   * @param {{ name?: string; reaches: Record<string, 1>; }} defunInfo
   */
  function visitNode(node, defunInfo) {
    // TODO: scope awareness here. for now we just assume all uses are
    //       from the top scope, which might create more uses than needed
    let reached;
    if (t.isIdentifier(node)) {
      const name = node.name;
      if (nameToGraphName.hasOwnProperty(name)) {
        reached = nameToGraphName[name];
      }
    } else if (isModuleUse(node)) {
      const name = getExportOrModuleUseName(node);
      if (modulePropertyToGraphName.hasOwnProperty(name)) {
        reached = modulePropertyToGraphName[name];
      }
    } else if (isStaticDynCall(node)) {
      reached = getGraphName(getStaticDynCallName(node), 'export');
    } else if (isDynamicDynCall(node)) {
      // this can reach *all* dynCall_* targets, we can't narrow it down
      reached = dynCallNames;
    } else if (isExportUse(node)) {
      // any remaining asm uses are always rooted in any case
      const name = getExportOrModuleUseName(node);
      if (exportNameToGraphName.hasOwnProperty(name)) {
        infos[exportNameToGraphName[name]].root = true;
      }
      return;
    }
    if (reached) {
      /**
       * @param {string} reached
       */
      function addReach(reached) {
        if (defunInfo) {
          defunInfo.reaches[reached] = 1; // defun reaches it
        } else {
          if (infos[reached]) {
            infos[reached].root = true; // in global scope, root it
          } else {
            // An info might not exist for the identifier if it is missing, for
            // example, we might call Module.dynCall_vi in library code, but it
            // won't exist in a standalone (non-JS) build anyhow. We can ignore
            // it in that case as the JS won't be used, but warn to be safe.
            trace('metadce: missing declaration for ' + reached);
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
  defuns.forEach((defun) => {
    const name = getGraphName(defun.id.name, 'defun');
    const info = (infos[name] = {
      name: name,
      reaches: {},
    });
    fullWalk(defun.body, (node) => visitNode(node, info));
  });
  fullWalk(ast, (node) => visitNode(node, null));
  // Final work: print out the graph
  // sort for determinism
  /**
   * @param {{}} map
   */
  function sortedNamesFromMap(map) {
    const names = [];
    for (const name of Object.keys(map)) {
      names.push(name);
    }
    names.sort();
    return names;
  }
  sortedNamesFromMap(infos).forEach((name) => {
    const info = infos[name];
    info.reaches = sortedNamesFromMap(info.reaches);
    graph.push(info);
  });
  console.log(JSON.stringify(graph, null, ' '));
}

// Apply graph removals from running wasm-metadce. This only removes imports and
// exports from JS side, effectively disentangling the wasm and JS sides that
// way (and we leave further DCE on the JS and wasm sides to their respective
// optimizers, closure compiler and binaryen).
/**
 * @param {t.Node} ast
 */
function applyDCEGraphRemovals(ast) {
  const unusedExports = new Set(extraInfo.unusedExports);
  const unusedImports = new Set(extraInfo.unusedImports);
  const foundUnusedImports = new Set();
  const foundUnusedExports = new Set();
  trace('unusedExports:', unusedExports);
  trace('unusedImports:', unusedImports);

  fullWalk(ast, (node) => {
    if (isWasmImportsAssign(node)) {
      const assignedObject = getWasmImportsValue(node);
      assignedObject.properties = assignedObject.properties.filter((item) => {
        t.assertObjectProperty(item, {computed: false});
        t.assertIdentifier(item.key);
        const name = item.key.name;
        const value = item.value;
        if (unusedImports.has(name)) {
          foundUnusedImports.add(name);
          return hasSideEffects(value);
        }
        return true;
      });
    } else if (t.isVariableDeclaration(node)) {
      // Handle the various ways in which we extract wasmExports:
      //   1. var _x = wasmExports['x'];
      // or
      //   2. var _x = Module['_x'] = wasmExports['x'];
      //
      // Or for delayed instantiation:
      //   3. var _x = () => (_x = wasmExports['x'])(...);
      // or
      //   4. var _x = Module['_x] = () => (_x = Module['_x'] = wasmExports['x'])(...);
      const init = node.declarations[0].init;
      if (!init) {
        return;
      }

      // Look through the optional `Module['_x']`
      let realInit = init;
      if (t.isAssignmentExpression(init) && isModuleUse(init.left)) {
        realInit = init.right;
      }

      if (isExportUse(realInit)) {
        const export_name = getExportOrModuleUseName(realInit);
        if (unusedExports.has(export_name)) {
          // Case (1) and (2)
          trace('found unused export:', export_name);
          emptyOut(node);
          foundUnusedExports.add(export_name);
        }
      } else if (t.isArrowFunctionExpression(realInit)) {
        const export_name = isExportWrapperFunction(realInit);
        if (unusedExports.has(export_name)) {
          // Case (3) and (4)
          trace('found unused export:', export_name);
          emptyOut(node);
          foundUnusedExports.add(export_name);
        }
      }
    } else if (t.isExpressionStatement(node)) {
      const expr = node.expression;
      // In the MINIMAL_RUNTIME code pattern we have just
      //   _x = wasmExports['x']
      // and never in a var.
      if (
        t.isAssignmentExpression(expr, {operator: '='}) &&
        t.isIdentifier(expr.left) &&
        isExportUse(expr.right)
      ) {
        const export_name = getExportOrModuleUseName(expr.right);
        if (unusedExports.has(export_name)) {
          emptyOut(node);
          foundUnusedExports.add(export_name);
        }
      }
    }
  });

  for (const i of unusedImports) {
    assert.ok(foundUnusedImports.has(i), 'unused import not found: ' + i);
  }
  for (const e of unusedExports) {
    assert.ok(foundUnusedExports.has(e), 'unused export not found: ' + e);
  }
}

/**
 * @param {t.Node} node
 * @param {string} name
 * @param {(t.Expression | t.SpreadElement | t.ArgumentPlaceholder)[]} args
 */
function makeCallExpression(node, name, args) {
  Object.assign(node, t.callExpression(t.identifier(name), args));
}

/**
 * @param {string} name
 */
function parseEmscriptenHeapName(name) {
  switch (name) {
    case 'HEAP8':
      return {type: 'I', bits: 8};
    case 'HEAPU8':
      return {type: 'U', bits: 8};
    case 'HEAP16':
      return {type: 'I', bits: 16};
    case 'HEAPU16':
      return {type: 'U', bits: 16};
    case 'HEAP32':
      return {type: 'I', bits: 32};
    case 'HEAPU32':
      return {type: 'U', bits: 32};
    case 'HEAPF32':
      return {type: 'F', bits: 32};
    case 'HEAPF64':
      return {type: 'F', bits: 64};
  }
}

// Replaces each HEAP access with function call that uses DataView to enforce
// LE byte order for HEAP buffer
/**
 * @param {t.Node} ast
 */
function littleEndianHeap(ast) {
  recursiveWalk(ast, {
    FunctionDeclaration: (node, c) => {
      // do not recurse into LE_HEAP_STORE, LE_HEAP_LOAD functions
      if (
        !(
          t.isIdentifier(node.id) &&
          (node.id.name.startsWith('LE_HEAP') || node.id.name.startsWith('LE_ATOMICS_'))
        )
      ) {
        c(node.body);
      }
    },
    VariableDeclarator: (node, c) => {
      if (!(t.isIdentifier(node.id) && node.id.name.startsWith('LE_ATOMICS_'))) {
        c(node.id);
        if (node.init) c(node.init);
      }
    },
    AssignmentExpression: (node, c) => {
      const target = node.left;
      const value = node.right;
      c(value);
      const parsed = parseEmscriptenHeapAccess(target);
      if (!parsed || parsed.bits === 8) {
        // not accessing the HEAP or loading only 1 byte
        return c(target);
      }
      // replace the heap access with LE_HEAP_STORE
      makeCallExpression(node, `LE_HEAP_STORE_${parsed.type}${parsed.bits}`, [
        multiply(parsed.property, parsed.bits / 8),
        value,
      ]);
    },
    CallExpression: (node, c) => {
      if (node.arguments) {
        for (var a of node.arguments) c(a);
      }
      if (
        // Atomics.X(args) -> LE_ATOMICS_X(args)
        t.isMemberExpression(node.callee, {computed: false}) &&
        t.isIdentifier(node.callee.object, {name: 'Atomics'})
      ) {
        t.assertIdentifier(node.callee.property);
        makeCallExpression(
          node,
          'LE_ATOMICS_' + node.callee.property.name.toUpperCase(),
          node.arguments,
        );
      } else {
        c(node.callee);
      }
    },
    MemberExpression: (node, c) => {
      c(node.property);
      const parsed = parseEmscriptenHeapAccess(node);
      if (!parsed || parsed.bits === 8) {
        // not accessing the HEAP or loading only 1 byte
        return c(node.object);
      }
      // replace the heap access with LE_HEAP_LOAD
      makeCallExpression(node, `LE_HEAP_LOAD_${parsed.type}${parsed.bits}`, [
        multiply(parsed.property, parsed.bits / 8),
      ]);
    },
  });
}

// Instrument heap accesses to call GROWABLE_HEAP_* helper functions instead, which allows
// pthreads + memory growth to work (we check if the memory was grown on another thread
// in each access), see #8365.
/**
 * @param {t.Node} ast
 */
function growableHeap(ast) {
  recursiveWalk(ast, {
    FunctionDeclaration(node, c) {
      // Do not recurse into to GROWABLE_HEAP_ helper functions themselves.
      if (
        !(
          t.isIdentifier(node.id) &&
          (node.id.name.startsWith('GROWABLE_HEAP_') || node.id.name === 'LE_HEAP_UPDATE')
        )
      ) {
        c(node.body);
      }
    },
    AssignmentExpression: (node) => {
      if (t.isIdentifier(node.left) && parseEmscriptenHeapName(node.left.name)) {
        // Don't transform initial setup of the arrays.
        return;
      }
      growableHeap(node.left);
      growableHeap(node.right);
    },
    VariableDeclaration: (node) => {
      // Don't transform the var declarations for HEAP8 etc
      node.declarations.forEach((decl) => {
        // but do transform anything that sets a var to
        // something from HEAP8 etc
        if (decl.init) {
          growableHeap(decl.init);
        }
      });
    },
    Identifier: (node) => {
      const parsed = parseEmscriptenHeapName(node.name);
      if (parsed) {
        // Turn HEAP8 into GROWABLE_HEAP_I8() etc
        makeCallExpression(node, `GROWABLE_HEAP_${parsed.type}${parsed.bits}`, []);
      }
    },
  });
}

// Make all JS pointers unsigned. We do this by modifying things like
// HEAP32[X >> 2] to HEAP32[X >>> 2]. We also need to handle the case of
// HEAP32[X] and make that HEAP32[X >>> 0], things like subarray(), etc.
/**
 * @param {t.Node} ast
 */
function unsignPointers(ast) {
  // Aside from the standard emscripten HEAP*s, also identify just "HEAP"/"heap"
  // as representing a heap. This can be used in JS library code in order
  // to get this pass to fix it up.
  /**
   * @param {t.Node} node
   */
  function isHeap(node) {
    if (!t.isIdentifier(node)) return false;
    const {name} = node;
    return parseEmscriptenHeapName(name) || name === 'heap' || name === 'HEAP';
  }

  /**
   * @param {t.Expression | t.PrivateName | t.ArgumentPlaceholder | t.SpreadElement} node
   */
  function unsign(node) {
    t.assertExpression(node);
    // The pointer is often a >> shift, which we can just turn into >>>
    if (t.isBinaryExpression(node)) {
      if (node.operator === '>>') {
        node.operator = '>>>';
        return node;
      }
    }
    // If nothing else worked out, add a new shift.
    return t.binaryExpression('>>>', node, t.numericLiteral(0));
  }

  fullWalk(ast, (node) => {
    if (t.isMemberExpression(node, {computed: true})) {
      // Check if this is HEAP*[?]
      if (isHeap(node.object)) {
        node.property = unsign(node.property);
      }
    } else if (t.isCallExpression(node)) {
      if (
        t.isMemberExpression(node.callee, {computed: false}) &&
        isHeap(node.callee.object)
      ) {
        t.assertIdentifier(node.callee.property);
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

/**
 * @param {t.Node} node
 */
function parseEmscriptenHeapAccess(node) {
  if (
    t.isMemberExpression(node) &&
    t.isIdentifier(node.object) &&
    node.computed // notice a[X] but not a.X
  ) {
    const parsed = parseEmscriptenHeapName(node.object.name);
    if (parsed) {
      t.assertExpression(node.property);
      return Object.assign(parsed, {
        property: node.property,
      });
    }
  }
}

// Replace direct HEAP* loads/stores with calls into C, in which ASan checks
// are applied. That lets ASan cover JS too.
/**
 * @param {t.Node} ast
 */
function asanify(ast) {
  /**
   * @param {t.Node} target
   */
  function toSuffixAndPtr(target) {
    const parsed = parseEmscriptenHeapAccess(target);
    if (!parsed) {
      return;
    }
    let suffix = '';
    switch (parsed.type) {
      case 'U':
        suffix = 'u';
      // fallthrough
      case 'I':
        suffix = `${parsed.bits / 8}${suffix}`;
        break;
      case 'F':
        suffix = parsed.bits === 64 ? 'd' : 'f';
        break;
    }
    return {suffix, ptr: parsed.property};
  }

  recursiveWalk(ast, {
    FunctionDeclaration(node, c) {
      if (t.isIdentifier(node.id) && node.id.name.startsWith('_asan_js_')) {
        // do not recurse into this js impl function, which we use during
        // startup before the wasm is ready
      } else {
        c(node.body);
      }
    },
    AssignmentExpression(node, c) {
      const target = node.left;
      const value = node.right;
      c(value);
      const parsed = toSuffixAndPtr(target);
      if (!parsed) {
        return c(target);
      }
      // Instrument a store.
      makeCallExpression(node, `_asan_js_store_${parsed.suffix}`, [parsed.ptr, value]);
    },
    MemberExpression(node, c) {
      c(node.property);
      const parsed = toSuffixAndPtr(node);
      if (!parsed) {
        c(node.object);
        if (node.computed) {
          c(node.property);
        }
        return;
      }
      // Instrument a load.
      makeCallExpression(node, `_asan_js_load_${parsed.suffix}`, [parsed.ptr]);
    },
  });
}

/**
 * @param {t.Expression} value
 * @param {number} by
 */
function multiply(value, by) {
  return t.binaryExpression('*', value, t.numericLiteral(by));
}

// Replace direct heap access with SAFE_HEAP* calls.
/**
 * @param {t.Node} ast
 */
function safeHeap(ast) {
  recursiveWalk(ast, {
    FunctionDeclaration(node, c) {
      if (
        t.isIdentifier(node.id) &&
        (node.id.name.startsWith('SAFE_HEAP') ||
          node.id.name === 'setValue_safe' ||
          node.id.name === 'getValue_safe')
      ) {
        // do not recurse into this js impl function, which we use during
        // startup before the wasm is ready
      } else {
        c(node.body);
      }
    },
    AssignmentExpression(node, c) {
      const target = node.left;
      const value = node.right;
      c(value);
      const parsed = parseEmscriptenHeapAccess(target);
      if (!parsed) {
        return c(target);
      }
      // Instrument a store.
      let ptr = parsed.property;
      const byteSize = parsed.bits / 8;
      if (byteSize !== 1) {
        ptr = multiply(ptr, byteSize);
      }
      makeCallExpression(node, `SAFE_HEAP_STORE${parsed.type === 'F' ? '_D' : ''}`, [
        ptr,
        value,
        t.numericLiteral(byteSize),
      ]);
    },
    MemberExpression(node, c) {
      c(node.property);
      const parsed = parseEmscriptenHeapAccess(node);
      if (!parsed) {
        return c(node.object);
      }
      // Instrument a load.
      let ptr = parsed.property;
      const byteSize = parsed.bits / 8;
      if (byteSize !== 1) {
        ptr = multiply(ptr, byteSize);
      }
      let args = [ptr, t.numericLiteral(byteSize)];
      switch (parsed.type) {
        case 'U':
          args.push(t.numericLiteral(1));
          break;
        case 'F':
          args.push(t.numericLiteral(2));
          break;
      }
      makeCallExpression(node, 'SAFE_HEAP_LOAD', args);
    },
  });
}

// Name minification

const RESERVED = new Set([
  'do',
  'if',
  'in',
  'for',
  'new',
  'try',
  'var',
  'env',
  'let',
  'case',
  'else',
  'enum',
  'void',
  'this',
  'void',
  'with',
]);
const VALID_MIN_INITS = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$';
const VALID_MIN_LATERS = VALID_MIN_INITS + '0123456789';

const minifiedNames = [];
const minifiedState = [0];

// Make sure the nth index in minifiedNames exists. Done 100% deterministically.
/**
 * @param {number} n
 */
function ensureMinifiedNames(n) {
  while (minifiedNames.length < n + 1) {
    // generate the current name
    let name = VALID_MIN_INITS[minifiedState[0]];
    for (let i = 1; i < minifiedState.length; i++) {
      name += VALID_MIN_LATERS[minifiedState[i]];
    }
    if (!RESERVED.has(name)) minifiedNames.push(name);
    // increment the state
    let i = 0;
    while (true) {
      minifiedState[i]++;
      if (minifiedState[i] < (i === 0 ? VALID_MIN_INITS : VALID_MIN_LATERS).length) break;
      // overflow
      minifiedState[i] = 0;
      i++;
      // will become 0 after increment in next loop head
      if (i === minifiedState.length) minifiedState.push(-1);
    }
  }
}

/**
 * @param {t.File} ast
 */
function minifyLocals(ast) {
  // We are given a mapping of global names to their minified forms.
  assert.ok(extraInfo?.globals);

  for (const fun of ast.program.body) {
    if (!t.isFunctionDeclaration(fun)) {
      continue;
    }
    // Find the list of local names, including params.
    const localNames = new Set();
    for (const param of fun.params) {
      t.assertIdentifier(param);
      localNames.add(param.name);
    }
    simpleWalk(fun, {
      VariableDeclaration(node) {
        for (const dec of node.declarations) {
          t.assertIdentifier(dec.id);
          localNames.add(dec.id.name);
        }
      },
    });

    /**
     * @param {string} name
     */
    function isLocalName(name) {
      return localNames.has(name);
    }

    // Names old to new names.
    const newNames = new Map();

    // The names in use, that must not be collided with.
    const usedNames = new Set();

    // Put the function name aside. We don't want to traverse it as it is not
    // in the scope of itself.
    const funId = fun.id;
    fun.id = null;

    // Find all the globals that we need to minify using pre-assigned names.
    // Don't actually minify them yet as that might interfere with local
    // variable names; just mark them as used, and what their new name will be.
    simpleWalk(fun, {
      Identifier(node) {
        const name = node.name;
        if (!isLocalName(name)) {
          const minified = extraInfo.globals[name];
          if (minified) {
            newNames.set(name, minified);
            usedNames.add(minified);
          }
        }
      },
      CallExpression(node) {
        // We should never call a local name, as in asm.js-style code our
        // locals are just numbers, not functions; functions are all declared
        // in the outer scope. If a local is called, that is a bug.
        if (t.isIdentifier(node.callee)) {
          assertAt(!isLocalName(node.callee.name), node.callee, 'cannot call a local');
        }
      },
    });

    // The first time we encounter a local name, we assign it a/ minified name
    // that's not currently in use. Allocating on demand means they're processed
    // in a predictable order, which is very handy for testing/debugging
    // purposes.
    let nextMinifiedName = 0;

    function getNextMinifiedName() {
      while (true) {
        ensureMinifiedNames(nextMinifiedName);
        const minified = minifiedNames[nextMinifiedName++];
        // TODO: we can probably remove !isLocalName here
        if (!usedNames.has(minified) && !isLocalName(minified)) {
          return minified;
        }
      }
    }

    // Traverse and minify all names. First the function parameters.
    for (const param of fun.params) {
      t.assertIdentifier(param);
      const minified = getNextMinifiedName();
      newNames.set(param.name, minified);
      param.name = minified;
    }

    // Label minification is done in a separate namespace.
    const labelNames = new Map();
    let nextMinifiedLabel = 0;
    function getNextMinifiedLabel() {
      ensureMinifiedNames(nextMinifiedLabel);
      return minifiedNames[nextMinifiedLabel++];
    }

    // Finally, the function body.
    recursiveWalk(fun, {
      Identifier(node) {
        const name = node.name;
        if (newNames.has(name)) {
          node.name = newNames.get(name);
        } else if (isLocalName(name)) {
          const minified = getNextMinifiedName();
          newNames.set(name, minified);
          node.name = minified;
        }
      },
      LabeledStatement(node, c) {
        if (!labelNames.has(node.label.name)) {
          labelNames.set(node.label.name, getNextMinifiedLabel());
        }
        node.label.name = labelNames.get(node.label.name);
        c(node.body);
      },
      BreakStatement(node, _c) {
        if (node.label) {
          node.label.name = labelNames.get(node.label.name);
        }
      },
      ContinueStatement(node, _c) {
        if (node.label) {
          node.label.name = labelNames.get(node.label.name);
        }
      },
    });

    // Finally, the function name, after restoring it.
    fun.id = funId;
    assert.ok(extraInfo.globals.hasOwnProperty(fun.id.name));
    fun.id.name = extraInfo.globals[fun.id.name];
  }
}

/**
 * @param {t.File} fileAst
 */
function minifyGlobals({program: ast}) {
  // The input is in form
  //
  //   function instantiate(wasmImports, wasmMemory, wasmTable) {
  //      var helper..
  //      function asmFunc(global, env, buffer) {
  //        var memory = env.memory;
  //        var HEAP8 = new global.Int8Array(buffer);
  //
  // We want to minify the interior of instantiate, basically everything but
  // the name instantiate itself, which is used externally to call it.
  //
  // This is *not* a complete minification algorithm. It does not have a full
  // understanding of nested scopes. Instead it assumes the code is fairly
  // simple - as wasm2js output is - and looks at all the minifiable names as
  // a whole. A possible bug here is something like
  //
  //   function instantiate(wasmImports, wasmMemory, wasmTable) {
  //      var x = foo;
  //      function asmFunc(global, env, buffer) {
  //        var foo = 10;
  //
  // Here foo is declared in an inner scope, and the outer use of foo looks
  // to the global scope. The analysis here only thinks something is from the
  // global scope if it is not in any var or function declaration. In practice,
  // the globals used from wasm2js output are things like Int8Array that we
  // don't declare as locals, but we should probably have a fully scope-aware
  // analysis here. FIXME

  // We must run on a singleton instantiate() function as described above.
  t.assertProgram(ast);
  assert.equal(ast.body.length, 1, 'expected a single function body');
  const fun = ast.body[0];
  t.assertFunctionDeclaration(fun);
  assertAt(fun.id.name === 'instantiate', fun.id, 'expected instantiate() function');

  // Swap the function's name away so that we can then minify everything else.
  const funId = fun.id;
  fun.id = null;

  // Find all the declarations.
  const declared = new Set();

  // Some identifiers must be left as they are and not minified.
  const ignore = new Set();

  simpleWalk(fun, {
    FunctionDeclaration(node) {
      if (node.id) {
        declared.add(node.id.name);
      }
      for (const param of node.params) {
        t.assertIdentifier(param);
        declared.add(param.name);
      }
    },
    FunctionExpression(node) {
      for (const param of node.params) {
        t.assertIdentifier(param);
        declared.add(param.name);
      }
    },
    VariableDeclaration(node) {
      for (const decl of node.declarations) {
        t.assertIdentifier(decl.id);
        declared.add(decl.id.name);
      }
    },
    MemberExpression(node) {
      // In  x.a  we must not minify a. However, for  x[a]  we must.
      if (!node.computed) {
        ignore.add(node.property);
      }
    },
  });

  // TODO: find names to avoid, that are not declared (should not happen in
  // wasm2js output)

  // Minify the names.
  let nextMinifiedName = 0;

  function getNewMinifiedName() {
    ensureMinifiedNames(nextMinifiedName);
    return minifiedNames[nextMinifiedName++];
  }

  const minified = new Map();

  /**
   * @param {string} name
   */
  function minify(name) {
    let minifiedName = minified.get(name);
    if (!minifiedName) {
      minifiedName = getNewMinifiedName();
      minified.set(name, minifiedName);
    }
    return minifiedName;
  }

  // Start with the declared things in the lowest indices. Things like HEAP8
  // can have very high use counts.
  for (const name of declared) {
    minify(name);
  }

  // Minify all globals in function chunks, i.e. not seen here, but will be in
  // the minifyLocals work on functions.
  for (const name of extraInfo.globals) {
    declared.add(name);
    minify(name);
  }

  // Replace the names with their minified versions.
  simpleWalk(fun, {
    Identifier(node) {
      if (declared.has(node.name) && !ignore.has(node)) {
        node.name = minify(node.name);
      }
    },
  });

  // Restore the name
  fun.id = funId;

  // Emit the metadata
  const json = {};
  for (const x of minified.entries()) json[x[0]] = x[1];

  suffix = EXTRA_INFO_PREFIX + JSON.stringify(json);
}

// Main

let suffix = '';

// If enabled, output retains parentheses and comments so that the
// output can further be passed out to Closure.
const {
  positionals: [infile, ...passes],
  values: {
    'closure-friendly': closureFriendly,
    'export-es6': exportES6,
    verbose: verbose,
    'no-print': noPrint,
    'minify-whitespace': minifyWhitespace,
    'out-file': outfile,
  },
} = parseArgs({
  allowPositionals: true,
  options: {
    'closure-friendly': {type: 'boolean'},
    'export-es6': {type: 'boolean'},
    verbose: {type: 'boolean'},
    'no-print': {type: 'boolean'},
    'minify-whitespace': {type: 'boolean'},
    'out-file': {type: 'string', short: 'o'},
  },
});

const trace = verbose
  ? console.warn
  : () => {
      /* no-op */
    };

let input = readFileSync(infile, 'utf-8');
const extraInfoStart = input.lastIndexOf(EXTRA_INFO_PREFIX);
let extraInfo = null;
if (extraInfoStart > 0) {
  extraInfo = JSON.parse(input.slice(extraInfoStart + EXTRA_INFO_PREFIX.length));
  input = input.slice(0, extraInfoStart);
}

try {
const ast = parse(input, {
  sourceFilename: infile,
  sourceType: exportES6 ? 'module' : 'script',
  allowAwaitOutsideFunction: true,
  createParenthesizedExpressions: closureFriendly,
  attachComment: closureFriendly,
});

/** @type {Record<string, (file: t.File) => void>} */
const registry = {
  JSDCE,
  AJSDCE,
  applyImportAndExportNameChanges,
  emitDCEGraph,
  applyDCEGraphRemovals,
  dump,
  littleEndianHeap,
  growableHeap,
  unsignPointers,
  minifyLocals,
  asanify,
  safeHeap,
  minifyGlobals,
};

for (const pass of passes) {
  trace(`running AST pass: ${pass}`);
  const resolvedPass = registry[pass];
  assert.ok(resolvedPass, `unknown optimizer pass: ${pass}`);
  resolvedPass(ast);
}

if (!noPrint) {
  let output = generate(ast, {
    filename: outfile,
    minified: minifyWhitespace,
    comments: closureFriendly,
  }).code;

  output += '\n';
  if (suffix) {
    output += suffix + '\n';
  }

  if (outfile) {
    writeFileSync(outfile, output);
  } else {
    // Simply using `fs.writeFileSync` on `process.stdout` has issues with
    // large amount of data. It can cause:
    //   Error: EAGAIN: resource temporarily unavailable, write
    process.stdout.write(output);
  }
}
} catch (err) {
  let loc = err['loc'];
  if (loc) {
    if ('line' in loc) {
      loc = { start: loc };
    }
    console.error(codeFrameColumns(input, loc, {
      highlightCode: true,
      message: err.message,
    }));
    process.exit(1);
  } else {
    throw err;
  }
}
