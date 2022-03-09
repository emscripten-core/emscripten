const acorn = require('acorn');
const fs = require('fs');
const assert = require('assert');
const path = require('path');
const terser = require('../third_party/terser');

// Finds the list of child AST nodes of the given node, that can contain other code (and are relevant for size computation)
function getChildNodes(node) {
  assert(node && !Array.isArray(node));
  var children = [];
  function addChild(child) {
    assert(child && !Array.isArray(child));
    children.push(child);
  }
  function maybeChild(child) {
    if (child) {
      assert(!Array.isArray(child));
      children.push(child);
    }
  }
  function addChildArray(childArray) {
    assert(Array.isArray(childArray));
    children = children.concat(childArray);
  }

  if (['BlockStatement', 'Program'].indexOf(node.type) != -1) { addChildArray(node.body); }
  else if (['IfStatement'].indexOf(node.type) != -1) { addChild(node.test); addChild(node.consequent); maybeChild(node.alternate); }
  else if (['BinaryStatement', 'BinaryExpression', 'LogicalExpression', 'AssignmentExpression'].indexOf(node.type) != -1) { addChild(node.left); addChild(node.right); }
  else if (['MemberExpression'].indexOf(node.type) != -1) { addChild(node.object); addChild(node.property); }
  else if (['Property'].indexOf(node.type) != -1) { addChild(node.key); addChild(node.value); }
  else if (['TryStatement'].indexOf(node.type) != -1) { addChild(node.block); maybeChild(node.handler); }
  else if (['CatchClause'].indexOf(node.type) != -1) { /*addChild(node.param);*/ addChild(node.body); }
  else if (['FunctionDeclaration'].indexOf(node.type) != -1) { addChild(node.body); }
  else if (['FunctionExpression', 'ArrowFunctionExpression'].indexOf(node.type) != -1) { addChild(node.body); }
  else if (['ThrowStatement', 'ReturnStatement'].indexOf(node.type) != -1) { maybeChild(node.argument); }
  else if (['UnaryExpression', 'UpdateExpression'].indexOf(node.type) != -1) { addChild(node.argument); }
  else if (['CallExpression', 'NewExpression'].indexOf(node.type) != -1) { addChild(node.callee); addChildArray(node.arguments); }
  else if (['VariableDeclaration'].indexOf(node.type) != -1) { addChildArray(node.declarations); }
  else if (['ArrayExpression'].indexOf(node.type) != -1) { addChildArray(node.elements); }
  else if (['VariableDeclarator'].indexOf(node.type) != -1) { maybeChild(node.init); }
  else if (['ObjectExpression'].indexOf(node.type) != -1) { addChildArray(node.properties); }
  else if (['ExpressionStatement'].indexOf(node.type) != -1) { addChild(node.expression); }
  else if (['BreakStatement'].indexOf(node.type) != -1) { /*maybeChild(node.label);*/ }
  else if (['LabeledStatement'].indexOf(node.type) != -1) { addChild(node.body); /*addChild(node.label);*/ }
  else if (['SwitchStatement'].indexOf(node.type) != -1) { addChild(node.discriminant); addChildArray(node.cases); }
  else if (['SwitchCase'].indexOf(node.type) != -1) { addChildArray(node.consequent); maybeChild(node.test); }
  else if (['SequenceExpression'].indexOf(node.type) != -1) { addChildArray(node.expressions); }
  else if (['ConditionalExpression'].indexOf(node.type) != -1) { addChild(node.test); addChild(node.consequent); addChild(node.alternate); }
  else if (['ForStatement'].indexOf(node.type) != -1) { maybeChild(node.init); maybeChild(node.test); maybeChild(node.update); addChild(node.body); }
  else if (['WhileStatement', 'DoWhileStatement'].indexOf(node.type) != -1) { addChild(node.test); addChild(node.body); }
  else if (['ForInStatement'].indexOf(node.type) != -1) { addChild(node.left); addChild(node.right); addChild(node.body); }
  else if (['Identifier', 'Literal', 'ThisExpression', 'EmptyStatement', 'DebuggerStatement', 'ContinueStatement', 'SpreadElement'].indexOf(node.type) != -1) {
    // no children
  } else {
    console.error('----NODE----');
    console.error(node);
    console.error('----ENDNODE----');
    assert(false);
  }
  return children;
}

function dump(nodeArray) {
  for(let node of nodeArray) {
    console.dir(node);
  }
}

// Closure integration of the Module object generates an awkward "var b; b || (b = Module);" code.
// 'b || (b = Module)' -> 'b = Module'.
function optPassSimplifyModuleInitialization(nodeArray) {
  for(let n of nodeArray) {
    if (n.type == 'ExpressionStatement' && n.expression.type == 'LogicalExpression' && n.expression.operator == '||'
      && n.expression.left.name == n.expression.right.left.name && n.expression.right.right.name == 'Module') {

      n.expression = n.expression.right;
      // There is only one Module assignment, so can finish the pass here.
      return;
    }
  }
}

// Merges empty VariableDeclarators to previous VariableDeclarations.
// 'var a,b; ...; var c,d;'' -> 'var a,b,c,d; ...;'
function optPassMergeEmptyVarDeclarators(nodeArray) {
  for(let i = 0; i < nodeArray.length; ++i) {
    let n = nodeArray[i];
    if (n.type != 'VariableDeclaration') continue;
    // Look back to find a preceding VariableDeclaration that empty declarators from this declaration could be fused to.
    for(let j = i-1; j >= 0; --j) {
      let p = nodeArray[j];
      if (p.type == 'VariableDeclaration') {
        for(let k = 0; k < n.declarations.length; ++k) {
          if (!n.declarations[k].init) {
            p.declarations.push(n.declarations[k]);
            n.declarations.splice(k--, 1);
          }
        }

        if (n.declarations.length == 0) nodeArray.splice(i--, 1);
        break;
      }
    }
  }
}

// Finds multiple consecutive VariableDeclaration nodes, and fuses them together.
// 'var a = 1; var b = 2;' -> 'var a = 1, b = 2;'
function optPassMergeVarDeclarations(nodeArray) {
  let progress = false;
  for(let i = 0; i < nodeArray.length; ++i) {
    let n = nodeArray[i];
    if (n.type != 'VariableDeclaration') continue;
    // Look back to find if there is a preceding VariableDeclaration that this declaration could be fused to.
    for(let j = i-1; j >= 0; --j) {
      let p = nodeArray[j];
      if (p.type == 'VariableDeclaration') {
        p.declarations = p.declarations.concat(n.declarations);
        nodeArray.splice(i--, 1);
        progress = true;
        break;
      }
      else if (!['FunctionDeclaration'].includes(p.type)) {
        break;
      }
    }
  }
  return progress;
}

// Finds redundant operator new statements that are not assigned anywhere.
// (we aren't interested in side effects of the calls if no assignment)
function optPassRemoveRedundantOperatorNews(nodeArray) {
  let progress = false;
  // Delete operator news that don't have any meaning.
  for(let i = 0; i < nodeArray.length; ++i) {
    let n = nodeArray[i];
    if (n.type == 'ExpressionStatement' && n.expression.type == 'NewExpression') {
      nodeArray.splice(i--, 1);
      progress = true;
    } else {
      for(let c of getChildNodes(n)) {
        if (c.type == 'BlockStatement') optPassRemoveRedundantOperatorNews(c.body);
        else optPassRemoveRedundantOperatorNews([c]);
      }
    }
  }
  return progress;
}

// Merges "var a,b;a = ...;" to "var b, a = ...;"
function optPassMergeVarInitializationAssignments(nodeArray) {
  // Tests if the assignment expression at nodeArray[i] is the first assignment to the given variable, and it was undefined before that.
  function isUndefinedBeforeThisAssignment(nodeArray, i) {
    let name = nodeArray[i].expression.left.name;
    for(let j = i-1; j >= 0; --j) {
      let n = nodeArray[j];
      if (n.type == 'ExpressionStatement' && n.expression.type == 'AssignmentExpression' && n.expression.left.name == name) {
        return null;
      }
      if (n.type == 'VariableDeclaration') {
        for(let k = n.declarations.length -1; k >= 0; --k) {
          let d = n.declarations[k];
          if (d.id.name == name) {
            if (d.init) return null;
            else return [n, k];
          }
        }
      }
    }
  }

  // Find all assignments that are preceded by a variable declaration.
  let progress = false;
  for(let i = 1; i < nodeArray.length; ++i) {
    let n = nodeArray[i];
    if (n.type != 'ExpressionStatement' || n.expression.type != 'AssignmentExpression') continue;
    if (nodeArray[i-1].type != 'VariableDeclaration') continue;
    let [declaration, declarationIndex] = isUndefinedBeforeThisAssignment(nodeArray, i);
    if (!declaration) continue;
    let declarator = declaration.declarations[declarationIndex];
    declarator.init = n.expression.right;
    declaration.declarations.splice(declarationIndex, 1);
    nodeArray[i-1].declarations.push(declarator);
    nodeArray.splice(i--, 1);
    progress = true;
  }
  return progress;
}

function runOnJsText(js, pretty=false) {
  var ast = acorn.parse(js, { ecmaVersion: 6 });

  optPassSimplifyModuleInitialization(ast.body);
  optPassRemoveRedundantOperatorNews(ast.body);

  let progress = true;
  while(progress) {
    progress = false;
    progress = progress || optPassMergeVarDeclarations(ast.body);
    progress = progress || optPassMergeVarInitializationAssignments(ast.body);
    progress = progress || optPassMergeEmptyVarDeclarators(ast.body);
  }

  const terserAst = terser.AST_Node.from_mozilla_ast(ast);
  const output = terserAst.print_to_string({ beautify: pretty, indent_level: pretty ? 1 : 0 });

  return output;
}

function runOnFile(input, pretty=false, output=null) {
  let js = fs.readFileSync(input).toString();
  js = runOnJsText(js, pretty);
  if (output) fs.writeFileSync(output, js);
  else console.log(js);
}

function test(input, expected) {
  let observed = runOnJsText(input);
  if (observed != expected) {
    console.error(`Input: ${input}\nobserved: ${observed}\nexpected: ${expected}\n`);
  } else {
    console.log(`OK: ${input} -> ${expected}`);
  }
}

function runTests() {
  // optPassSimplifyModuleInitialization:
  test("b || (b = Module);", "b=Module;");

  // optPassRemoveRedundantOperatorNews:
  test("new Uint16Array(a);", "");
  test("new function(a) {new TextDecoder(a);}('utf8');", "");
  test("WebAssembly.instantiate(c.wasm,{}).then(function(a){new Int8Array(b);});", "WebAssembly.instantiate(c.wasm,{}).then(function(a){});");
  test("let x=new Uint16Array(a);", "let x=new Uint16Array(a);");

  // optPassMergeVarDeclarations:
  test("var a; var b;", 'var a,b;');
  test("var a=1; var b=2;", 'var a=1,b=2;');
  test("var a=1; function foo(){} var b=2;", 'var a=1,b=2;function foo(){}');

  // optPassMergeEmptyVarDeclarators:
  test("var a;a=1;", "var a=1;");
  test("var a = 1, b; ++a; var c;", "var a=1,b,c;++a;");

  // Interaction between multiple passes:
  test("var d, f; f = new Uint8Array(16); var h = f.buffer; d = new Uint8Array(h);","var f=new Uint8Array(16),h=f.buffer,d=new Uint8Array(h);");
}

let args = process['argv'].slice(2);

function readBool(arg) {
  let ret = false;
  for(;;) {
    let i = args.indexOf(arg);
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
  for(;;) {
    let i = args.indexOf(arg);
    if (i >= 0) {
      ret = args[i+1];
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
