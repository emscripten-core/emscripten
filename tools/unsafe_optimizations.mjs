#!/usr/bin/env node

/** Implements a set of potentially unsafe JavaScript AST optimizations for aggressive code size optimizations.
    Enabled when building with -sMINIMAL_RUNTIME=2 linker flag. */

import * as fs from 'node:fs';
import * as acorn from 'acorn';
import * as terser from '../third_party/terser/terser.js';

// Starting at the AST node 'root', calls the given callback function 'func' on all children and grandchildren of 'root'
// that are of any of the type contained in array 'types'.
function visitNodes(root, types, func) {
  // Visit the given node if it is of desired type.
  if (types.includes(root.type)) {
    const continueTraversal = func(root);
    if (continueTraversal === false) return false;
  }

  // Traverse all children of this node to find nodes of desired type.
  for (const member in root) {
    if (Array.isArray(root[member])) {
      for (const elem of root[member]) {
        if (elem?.type) {
          const continueTraversal = visitNodes(elem, types, func);
          if (continueTraversal === false) return false;
        }
      }
    } else if (root[member]?.type) {
      const continueTraversal = visitNodes(root[member], types, func);
      if (continueTraversal === false) return false;
    }
  }
}

function optPassSimplifyModularizeFunction(ast) {
  visitNodes(ast, ['FunctionExpression'], (node) => {
    if (node.params.length == 1 && node.params[0].name == 'Module') {
      const body = node.body.body;
      // Nuke 'Module = Module || {};'
      if (
        body[0].type == 'ExpressionStatement' &&
        body[0].expression.type == 'AssignmentExpression' &&
        body[0].expression.left.name == 'Module'
      ) {
        body.splice(0, 1);
      }
      // Replace 'function(Module) {var f = Module;' -> 'function(f) {'
      if (
        body[0].type == 'VariableDeclaration' &&
        body[0].declarations[0]?.init?.name == 'Module'
      ) {
        node.params[0].name = body[0].declarations[0].id.name;
        body[0].declarations.splice(0, 1);
        if (body[0].declarations.length == 0) {
          body.splice(0, 1);
        }
      }
      return false;
    }
  });
}

// Finds redundant operator new statements that are not assigned anywhere.
// (we aren't interested in side effects of the calls if no assignment)
function optPassRemoveRedundantOperatorNews(ast) {
  // Remove standalone operator new statements that don't have any meaning.
  visitNodes(ast, ['BlockStatement', 'Program'], (node) => {
    const nodeArray = node.body;
    for (let i = 0; i < nodeArray.length; ++i) {
      const n = nodeArray[i];
      if (n.type == 'ExpressionStatement' && n.expression.type == 'NewExpression') {
        // Make an exception for new `new Promise` which is sometimes used
        // in emscripten with real side effects.  For example, see
        // loadWasmModuleToWorker which returns a `new Promise` that is never
        // referenced (a least in some builds).
        //
        // Another exception is made for `new WebAssembly.*` since we create and
        // unused `WebAssembly.Memory` when probing for wasm64 fatures.
        if (
          n.expression.callee.name !== 'Promise' &&
          n.expression.callee.object?.name !== 'WebAssembly'
        ) {
          nodeArray.splice(i--, 1);
        }
      }
    }
  });

  // Remove comma sequence chained operator news ('new foo(), new foo();')
  visitNodes(ast, ['SequenceExpression'], (node) => {
    const nodeArray = node.expressions;
    // Delete operator news that don't have any meaning.
    for (let i = 0; i < nodeArray.length; ++i) {
      const n = nodeArray[i];
      if (n.type == 'NewExpression') {
        nodeArray.splice(i--, 1);
      }
    }
  });
}

// Merges empty VariableDeclarators to previous VariableDeclarations.
// 'var a,b; ...; var c,d;'' -> 'var a,b,c,d; ...;'
function optPassMergeEmptyVarDeclarators(ast) {
  let progress = false;

  visitNodes(ast, ['BlockStatement', 'Program'], (node) => {
    const nodeArray = node.body;
    for (let i = 0; i < nodeArray.length; ++i) {
      const n = nodeArray[i];
      if (n.type != 'VariableDeclaration') continue;
      // Look back to find a preceding VariableDeclaration that empty declarators from this declaration could be fused to.
      for (let j = i - 1; j >= 0; --j) {
        const p = nodeArray[j];
        if (p.type == 'VariableDeclaration') {
          for (let k = 0; k < n.declarations.length; ++k) {
            if (!n.declarations[k].init) {
              p.declarations.push(n.declarations[k]);
              n.declarations.splice(k--, 1);
              progress = true;
            }
          }

          if (n.declarations.length == 0) nodeArray.splice(i--, 1);
          break;
        }
      }
    }
  });
  return progress;
}

// Finds multiple consecutive VariableDeclaration nodes, and fuses them together.
// 'var a = 1; var b = 2;' -> 'var a = 1, b = 2;'
function optPassMergeVarDeclarations(ast) {
  let progress = false;

  visitNodes(ast, ['BlockStatement', 'Program'], (node) => {
    const nodeArray = node.body;
    for (let i = 0; i < nodeArray.length; ++i) {
      const n = nodeArray[i];
      if (n.type != 'VariableDeclaration') continue;
      // Look back to find if there is a preceding VariableDeclaration that this declaration could be fused to.
      for (let j = i - 1; j >= 0; --j) {
        const p = nodeArray[j];
        if (p.type == 'VariableDeclaration') {
          p.declarations = p.declarations.concat(n.declarations);
          nodeArray.splice(i--, 1);
          progress = true;
          break;
        } else if (!['FunctionDeclaration'].includes(p.type)) {
          break;
        }
      }
    }
  });
  return progress;
}

// Merges "var a,b;a = ...;" to "var b, a = ...;"
function optPassMergeVarInitializationAssignments(ast) {
  // Tests if the assignment expression at nodeArray[i] is the first assignment to the given variable, and it was undefined before that.
  function isUndefinedBeforeThisAssignment(nodeArray, i) {
    const name = nodeArray[i].expression.left.name;
    for (let j = i - 1; j >= 0; --j) {
      const n = nodeArray[j];
      if (
        n.type == 'ExpressionStatement' &&
        n.expression.type == 'AssignmentExpression' &&
        n.expression.left.name == name
      ) {
        return [null, null];
      }
      if (n.type == 'VariableDeclaration') {
        for (let k = n.declarations.length - 1; k >= 0; --k) {
          const d = n.declarations[k];
          if (d.id.name == name) {
            if (d.init) return [null, null];
            else return [n, k];
          }
        }
      }
    }
    return [null, null];
  }

  // Find all assignments that are preceded by a variable declaration.
  let progress = false;
  visitNodes(ast, ['BlockStatement', 'Program'], (node) => {
    const nodeArray = node.body;
    for (let i = 1; i < nodeArray.length; ++i) {
      const n = nodeArray[i];
      if (n.type != 'ExpressionStatement' || n.expression.type != 'AssignmentExpression') continue;
      if (nodeArray[i - 1].type != 'VariableDeclaration') continue;
      const [declaration, declarationIndex] = isUndefinedBeforeThisAssignment(nodeArray, i);
      if (!declaration) continue;
      const declarator = declaration.declarations[declarationIndex];
      declarator.init = n.expression.right;
      declaration.declarations.splice(declarationIndex, 1);
      nodeArray[i - 1].declarations.push(declarator);
      nodeArray.splice(i--, 1);
      progress = true;
    }
  });
  return progress;
}

function runOnJsText(js, pretty = false) {
  const ast = acorn.parse(js, {ecmaVersion: 2021});

  optPassRemoveRedundantOperatorNews(ast);

  let progress = true;
  while (progress) {
    progress = optPassMergeVarDeclarations(ast);
    progress = progress || optPassMergeVarInitializationAssignments(ast);
    progress = progress || optPassMergeEmptyVarDeclarators(ast);
  }

  optPassSimplifyModularizeFunction(ast);

  const terserAst = terser.AST_Node.from_mozilla_ast(ast);
  const output = terserAst.print_to_string({
    wrap_func_args: false,
    beautify: pretty,
    indent_level: pretty ? 2 : 0,
  });

  return output;
}

function runOnFile(input, pretty = false, output = null) {
  let js = fs.readFileSync(input).toString();
  js = runOnJsText(js, pretty);
  if (output) fs.writeFileSync(output, js);
  else console.log(js);
}

let numTestFailures = 0;

function test(input, expected) {
  const observed = runOnJsText(input);
  if (observed != expected) {
    console.error(`ERROR: Input: ${input}\nobserved: ${observed}\nexpected: ${expected}\n`);
    ++numTestFailures;
  } else {
    console.log(`OK: ${input} -> ${expected}`);
  }
}

function runTests() {
  // optPassSimplifyModularizeFunction:
  test(
    'var Module = function(Module) {Module = Module || {};var f = Module;}',
    'var Module=function(f){};',
  );

  // optPassRemoveRedundantOperatorNews:
  test('new Uint16Array(a);', '');
  test('new Uint16Array(a),new Uint16Array(a);', ';');
  test("new function(a) {new TextDecoder(a);}('utf8');", '');
  test(
    'WebAssembly.instantiate(c.wasm,{}).then((a) => {new Int8Array(b);});',
    'WebAssembly.instantiate(c.wasm,{}).then(a=>{});',
  );
  test('let x=new Uint16Array(a);', 'let x=new Uint16Array(a);');
  // new Promise should be preserved
  test('new Promise();', 'new Promise;');

  // optPassMergeVarDeclarations:
  test('var a; var b;', 'var a,b;');
  test('var a=1; var b=2;', 'var a=1,b=2;');
  test('var a=1; function foo(){} var b=2;', 'var a=1,b=2;function foo(){}');

  // optPassMergeEmptyVarDeclarators:
  test('var a;a=1;', 'var a=1;');
  test('var a = 1, b; ++a; var c;', 'var a=1,b,c;++a;');

  // Interaction between multiple passes:
  test(
    'var d, f; f = new Uint8Array(16); var h = f.buffer; d = new Uint8Array(h);',
    'var f=new Uint8Array(16),h=f.buffer,d=new Uint8Array(h);',
  );

  // Older versions of terser would produce sub-optimal output for this.
  // We keep this test around to prevent regression.
  test('var i=new Image;i.onload=()=>{}', 'var i=new Image;i.onload=()=>{};');

  // Test that arrays containing nulls don't cause issues
  test('[,];', '[,];');

  // Test optional chaining operator
  test('console?.log("");', 'console?.log("");');

  process.exit(numTestFailures);
}

const args = process.argv.slice(2);

function readBool(arg) {
  let ret = false;
  for (;;) {
    const i = args.indexOf(arg);
    if (i >= 0) {
      args.splice(i, 1);
      ret = true;
    } else {
      return ret;
    }
  }
}

function readArg(arg) {
  let ret = null;
  for (;;) {
    const i = args.indexOf(arg);
    if (i >= 0) {
      ret = args[i + 1];
      args.splice(i, 2);
    } else {
      return ret;
    }
  }
}

const testMode = readBool('--test');
const pretty = readBool('--pretty');
const output = readArg('-o');
const input = args[0];

if (testMode) {
  runTests();
} else {
  runOnFile(input, pretty, output);
}
