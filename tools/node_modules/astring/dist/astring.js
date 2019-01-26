(function (global, factory) {
  if (typeof define === "function" && define.amd) {
    define(['exports'], factory);
  } else if (typeof exports !== "undefined") {
    factory(exports);
  } else {
    var mod = {
      exports: {}
    };
    factory(mod.exports);
    global.astring = mod.exports;
  }
})(this, function (exports) {
  'use strict';

  exports.__esModule = true;
  exports.generate = generate;

  function _classCallCheck(instance, Constructor) {
    if (!(instance instanceof Constructor)) {
      throw new TypeError("Cannot call a class as a function");
    }
  }

  var stringify = JSON.stringify;


  /* istanbul ignore if */
  if (!String.prototype.repeat) {
    /* istanbul ignore next */
    throw new Error('String.prototype.repeat is undefined, see https://github.com/davidbonnet/astring#installation');
  }

  /* istanbul ignore if */
  if (!String.prototype.endsWith) {
    /* istanbul ignore next */
    throw new Error('String.prototype.endsWith is undefined, see https://github.com/davidbonnet/astring#installation');
  }

  var OPERATOR_PRECEDENCE = {
    '||': 3,
    '&&': 4,
    '|': 5,
    '^': 6,
    '&': 7,
    '==': 8,
    '!=': 8,
    '===': 8,
    '!==': 8,
    '<': 9,
    '>': 9,
    '<=': 9,
    '>=': 9,
    in: 9,
    instanceof: 9,
    '<<': 10,
    '>>': 10,
    '>>>': 10,
    '+': 11,
    '-': 11,
    '*': 12,
    '%': 12,
    '/': 12,
    '**': 13

    // Enables parenthesis regardless of precedence
  };var NEEDS_PARENTHESES = 17;

  var EXPRESSIONS_PRECEDENCE = {
    // Definitions
    ArrayExpression: 20,
    TaggedTemplateExpression: 20,
    ThisExpression: 20,
    Identifier: 20,
    Literal: 18,
    TemplateLiteral: 20,
    Super: 20,
    SequenceExpression: 20,
    // Operations
    MemberExpression: 19,
    CallExpression: 19,
    NewExpression: 19,
    // Other definitions
    ArrowFunctionExpression: NEEDS_PARENTHESES,
    ClassExpression: NEEDS_PARENTHESES,
    FunctionExpression: NEEDS_PARENTHESES,
    ObjectExpression: NEEDS_PARENTHESES,
    // Other operations
    UpdateExpression: 16,
    UnaryExpression: 15,
    BinaryExpression: 14,
    LogicalExpression: 13,
    ConditionalExpression: 4,
    AssignmentExpression: 3,
    AwaitExpression: 2,
    YieldExpression: 2,
    RestElement: 1
  };

  function formatSequence(state, nodes) {
    var generator = state.generator;

    state.write('(');
    if (nodes != null && nodes.length > 0) {
      generator[nodes[0].type](nodes[0], state);
      var length = nodes.length;

      for (var i = 1; i < length; i++) {
        var param = nodes[i];
        state.write(', ');
        generator[param.type](param, state);
      }
    }
    state.write(')');
  }

  function expressionNeedsParenthesis(node, parentNode, isRightHand) {
    var nodePrecedence = EXPRESSIONS_PRECEDENCE[node.type];
    if (nodePrecedence === NEEDS_PARENTHESES) {
      return true;
    }
    var parentNodePrecedence = EXPRESSIONS_PRECEDENCE[parentNode.type];
    if (nodePrecedence !== parentNodePrecedence) {
      // Different node types
      return nodePrecedence < parentNodePrecedence;
    }
    if (nodePrecedence !== 13 && nodePrecedence !== 14) {
      // Not a `LogicalExpression` or `BinaryExpression`
      return false;
    }
    if (node.operator === '**' && parentNode.operator === '**') {
      // Exponentiation operator has right-to-left associativity
      return !isRightHand;
    }
    if (isRightHand) {
      // Parenthesis are used if both operators have the same precedence
      return OPERATOR_PRECEDENCE[node.operator] <= OPERATOR_PRECEDENCE[parentNode.operator];
    }
    return OPERATOR_PRECEDENCE[node.operator] < OPERATOR_PRECEDENCE[parentNode.operator];
  }

  function formatBinaryExpressionPart(state, node, parentNode, isRightHand) {
    var generator = state.generator;

    if (expressionNeedsParenthesis(node, parentNode, isRightHand)) {
      state.write('(');
      generator[node.type](node, state);
      state.write(')');
    } else {
      generator[node.type](node, state);
    }
  }

  function reindent(state, text, indent, lineEnd) {
    /*
    Writes into `state` the `text` string reindented with the provided `indent`.
    */
    var lines = text.split('\n');
    var end = lines.length - 1;
    state.write(lines[0].trim());
    if (end > 0) {
      state.write(lineEnd);
      for (var i = 1; i < end; i++) {
        state.write(indent + lines[i].trim() + lineEnd);
      }
      state.write(indent + lines[end].trim());
    }
  }

  function formatComments(state, comments, indent, lineEnd) {
    var length = comments.length;

    for (var i = 0; i < length; i++) {
      var comment = comments[i];
      state.write(indent);
      if (comment.type[0] === 'L') {
        // Line comment
        state.write('// ' + comment.value.trim() + '\n');
      } else {
        // Block comment
        state.write('/*');
        reindent(state, comment.value, indent, lineEnd);
        state.write('*/' + lineEnd);
      }
    }
  }

  function hasCallExpression(node) {
    /*
    Returns `true` if the provided `node` contains a call expression and `false` otherwise.
    */
    var currentNode = node;
    while (currentNode != null) {
      var _currentNode = currentNode,
          type = _currentNode.type;

      if (type[0] === 'C' && type[1] === 'a') {
        // Is CallExpression
        return true;
      } else if (type[0] === 'M' && type[1] === 'e' && type[2] === 'm') {
        // Is MemberExpression
        currentNode = currentNode.object;
      } else {
        return false;
      }
    }
  }

  function formatVariableDeclaration(state, node) {
    var generator = state.generator;
    var declarations = node.declarations;

    state.write(node.kind + ' ');
    var length = declarations.length;

    if (length > 0) {
      generator.VariableDeclarator(declarations[0], state);
      for (var i = 1; i < length; i++) {
        state.write(', ');
        generator.VariableDeclarator(declarations[i], state);
      }
    }
  }

  var ForInStatement = void 0,
      FunctionDeclaration = void 0,
      RestElement = void 0,
      BinaryExpression = void 0,
      ArrayExpression = void 0,
      BlockStatement = void 0;

  var baseGenerator = exports.baseGenerator = {
    Program: function Program(node, state) {
      var indent = state.indent.repeat(state.indentLevel);
      var lineEnd = state.lineEnd,
          writeComments = state.writeComments;

      if (writeComments && node.comments != null) {
        formatComments(state, node.comments, indent, lineEnd);
      }
      var statements = node.body;
      var length = statements.length;

      for (var i = 0; i < length; i++) {
        var statement = statements[i];
        if (writeComments && statement.comments != null) {
          formatComments(state, statement.comments, indent, lineEnd);
        }
        state.write(indent);
        this[statement.type](statement, state);
        state.write(lineEnd);
      }
      if (writeComments && node.trailingComments != null) {
        formatComments(state, node.trailingComments, indent, lineEnd);
      }
    },

    BlockStatement: BlockStatement = function BlockStatement(node, state) {
      var indent = state.indent.repeat(state.indentLevel++);
      var lineEnd = state.lineEnd,
          writeComments = state.writeComments;

      var statementIndent = indent + state.indent;
      state.write('{');
      var statements = node.body;
      if (statements != null && statements.length > 0) {
        state.write(lineEnd);
        if (writeComments && node.comments != null) {
          formatComments(state, node.comments, statementIndent, lineEnd);
        }
        var length = statements.length;

        for (var i = 0; i < length; i++) {
          var statement = statements[i];
          if (writeComments && statement.comments != null) {
            formatComments(state, statement.comments, statementIndent, lineEnd);
          }
          state.write(statementIndent);
          this[statement.type](statement, state);
          state.write(lineEnd);
        }
        state.write(indent);
      } else {
        if (writeComments && node.comments != null) {
          state.write(lineEnd);
          formatComments(state, node.comments, statementIndent, lineEnd);
          state.write(indent);
        }
      }
      if (writeComments && node.trailingComments != null) {
        formatComments(state, node.trailingComments, statementIndent, lineEnd);
      }
      state.write('}');
      state.indentLevel--;
    },
    ClassBody: BlockStatement,
    EmptyStatement: function EmptyStatement(node, state) {
      state.write(';');
    },
    ExpressionStatement: function ExpressionStatement(node, state) {
      var precedence = EXPRESSIONS_PRECEDENCE[node.expression.type];
      if (precedence === NEEDS_PARENTHESES || precedence === 3 && node.expression.left.type[0] === 'O') {
        // Should always have parentheses or is an AssignmentExpression to an ObjectPattern
        state.write('(');
        this[node.expression.type](node.expression, state);
        state.write(')');
      } else {
        this[node.expression.type](node.expression, state);
      }
      state.write(';');
    },
    IfStatement: function IfStatement(node, state) {
      state.write('if (');
      this[node.test.type](node.test, state);
      state.write(') ');
      this[node.consequent.type](node.consequent, state);
      if (node.alternate != null) {
        state.write(' else ');
        this[node.alternate.type](node.alternate, state);
      }
    },
    LabeledStatement: function LabeledStatement(node, state) {
      this[node.label.type](node.label, state);
      state.write(': ');
      this[node.body.type](node.body, state);
    },
    BreakStatement: function BreakStatement(node, state) {
      state.write('break');
      if (node.label != null) {
        state.write(' ');
        this[node.label.type](node.label, state);
      }
      state.write(';');
    },
    ContinueStatement: function ContinueStatement(node, state) {
      state.write('continue');
      if (node.label != null) {
        state.write(' ');
        this[node.label.type](node.label, state);
      }
      state.write(';');
    },
    WithStatement: function WithStatement(node, state) {
      state.write('with (');
      this[node.object.type](node.object, state);
      state.write(') ');
      this[node.body.type](node.body, state);
    },
    SwitchStatement: function SwitchStatement(node, state) {
      var indent = state.indent.repeat(state.indentLevel++);
      var lineEnd = state.lineEnd,
          writeComments = state.writeComments;

      state.indentLevel++;
      var caseIndent = indent + state.indent;
      var statementIndent = caseIndent + state.indent;
      state.write('switch (');
      this[node.discriminant.type](node.discriminant, state);
      state.write(') {' + lineEnd);
      var occurences = node.cases;
      var occurencesCount = occurences.length;

      for (var i = 0; i < occurencesCount; i++) {
        var occurence = occurences[i];
        if (writeComments && occurence.comments != null) {
          formatComments(state, occurence.comments, caseIndent, lineEnd);
        }
        if (occurence.test) {
          state.write(caseIndent + 'case ');
          this[occurence.test.type](occurence.test, state);
          state.write(':' + lineEnd);
        } else {
          state.write(caseIndent + 'default:' + lineEnd);
        }
        var consequent = occurence.consequent;
        var consequentCount = consequent.length;

        for (var _i = 0; _i < consequentCount; _i++) {
          var statement = consequent[_i];
          if (writeComments && statement.comments != null) {
            formatComments(state, statement.comments, statementIndent, lineEnd);
          }
          state.write(statementIndent);
          this[statement.type](statement, state);
          state.write(lineEnd);
        }
      }
      state.indentLevel -= 2;
      state.write(indent + '}');
    },
    ReturnStatement: function ReturnStatement(node, state) {
      state.write('return');
      if (node.argument) {
        state.write(' ');
        this[node.argument.type](node.argument, state);
      }
      state.write(';');
    },
    ThrowStatement: function ThrowStatement(node, state) {
      state.write('throw ');
      this[node.argument.type](node.argument, state);
      state.write(';');
    },
    TryStatement: function TryStatement(node, state) {
      state.write('try ');
      this[node.block.type](node.block, state);
      if (node.handler) {
        var handler = node.handler;

        state.write(' catch (');
        this[handler.param.type](handler.param, state);
        state.write(') ');
        this[handler.body.type](handler.body, state);
      }
      if (node.finalizer) {
        state.write(' finally ');
        this[node.finalizer.type](node.finalizer, state);
      }
    },
    WhileStatement: function WhileStatement(node, state) {
      state.write('while (');
      this[node.test.type](node.test, state);
      state.write(') ');
      this[node.body.type](node.body, state);
    },
    DoWhileStatement: function DoWhileStatement(node, state) {
      state.write('do ');
      this[node.body.type](node.body, state);
      state.write(' while (');
      this[node.test.type](node.test, state);
      state.write(');');
    },
    ForStatement: function ForStatement(node, state) {
      state.write('for (');
      if (node.init != null) {
        var init = node.init;

        if (init.type[0] === 'V') {
          formatVariableDeclaration(state, init);
        } else {
          this[init.type](init, state);
        }
      }
      state.write('; ');
      if (node.test) {
        this[node.test.type](node.test, state);
      }
      state.write('; ');
      if (node.update) {
        this[node.update.type](node.update, state);
      }
      state.write(') ');
      this[node.body.type](node.body, state);
    },

    ForInStatement: ForInStatement = function ForInStatement(node, state) {
      state.write('for (');
      var left = node.left;

      if (left.type[0] === 'V') {
        formatVariableDeclaration(state, left);
      } else {
        this[left.type](left, state);
      }
      // Identifying whether node.type is `ForInStatement` or `ForOfStatement`
      state.write(node.type[3] === 'I' ? ' in ' : ' of ');
      this[node.right.type](node.right, state);
      state.write(') ');
      this[node.body.type](node.body, state);
    },
    ForOfStatement: ForInStatement,
    DebuggerStatement: function DebuggerStatement(node, state) {
      state.write('debugger;' + state.lineEnd);
    },

    FunctionDeclaration: FunctionDeclaration = function FunctionDeclaration(node, state) {
      state.write((node.async ? 'async ' : '') + (node.generator ? 'function* ' : 'function ') + (node.id ? node.id.name : ''), node);
      formatSequence(state, node.params);
      state.write(' ');
      this[node.body.type](node.body, state);
    },
    FunctionExpression: FunctionDeclaration,
    VariableDeclaration: function VariableDeclaration(node, state) {
      formatVariableDeclaration(state, node);
      state.write(';');
    },
    VariableDeclarator: function VariableDeclarator(node, state) {
      this[node.id.type](node.id, state);
      if (node.init != null) {
        state.write(' = ');
        this[node.init.type](node.init, state);
      }
    },
    ClassDeclaration: function ClassDeclaration(node, state) {
      state.write('class ' + (node.id ? node.id.name + ' ' : ''), node);
      if (node.superClass) {
        state.write('extends ');
        this[node.superClass.type](node.superClass, state);
        state.write(' ');
      }
      this.ClassBody(node.body, state);
    },
    ImportDeclaration: function ImportDeclaration(node, state) {
      state.write('import ');
      var specifiers = node.specifiers;
      var length = specifiers.length;
      // NOTE: Once babili is fixed, put this after condition
      // https://github.com/babel/babili/issues/430

      var i = 0;
      if (length > 0) {
        for (; i < length;) {
          if (i > 0) {
            state.write(', ');
          }
          var specifier = specifiers[i];
          var type = specifier.type[6];
          if (type === 'D') {
            // ImportDefaultSpecifier
            state.write(specifier.local.name, specifier);
            i++;
          } else if (type === 'N') {
            // ImportNamespaceSpecifier
            state.write('* as ' + specifier.local.name, specifier);
            i++;
          } else {
            // ImportSpecifier
            break;
          }
        }
        if (i < length) {
          state.write('{');
          for (;;) {
            var _specifier = specifiers[i];
            var name = _specifier.imported.name;

            state.write(name, _specifier);
            if (name !== _specifier.local.name) {
              state.write(' as ' + _specifier.local.name);
            }
            if (++i < length) {
              state.write(', ');
            } else {
              break;
            }
          }
          state.write('}');
        }
        state.write(' from ');
      }
      this.Literal(node.source, state);
      state.write(';');
    },
    ExportDefaultDeclaration: function ExportDefaultDeclaration(node, state) {
      state.write('export default ');
      this[node.declaration.type](node.declaration, state);
      if (EXPRESSIONS_PRECEDENCE[node.declaration.type] && node.declaration.type[0] !== 'F') {
        // All expression nodes except `FunctionExpression`
        state.write(';');
      }
    },
    ExportNamedDeclaration: function ExportNamedDeclaration(node, state) {
      state.write('export ');
      if (node.declaration) {
        this[node.declaration.type](node.declaration, state);
      } else {
        state.write('{');
        var specifiers = node.specifiers,
            length = specifiers.length;

        if (length > 0) {
          for (var i = 0;;) {
            var specifier = specifiers[i];
            var name = specifier.local.name;

            state.write(name, specifier);
            if (name !== specifier.exported.name) {
              state.write(' as ' + specifier.exported.name);
            }
            if (++i < length) {
              state.write(', ');
            } else {
              break;
            }
          }
        }
        state.write('}');
        if (node.source) {
          state.write(' from ');
          this.Literal(node.source, state);
        }
        state.write(';');
      }
    },
    ExportAllDeclaration: function ExportAllDeclaration(node, state) {
      state.write('export * from ');
      this.Literal(node.source, state);
      state.write(';');
    },
    MethodDefinition: function MethodDefinition(node, state) {
      if (node.static) {
        state.write('static ');
      }
      var kind = node.kind[0];
      if (kind === 'g' || kind === 's') {
        // Getter or setter
        state.write(node.kind + ' ');
      }
      if (node.value.async) {
        state.write('async ');
      }
      if (node.value.generator) {
        state.write('*');
      }
      if (node.computed) {
        state.write('[');
        this[node.key.type](node.key, state);
        state.write(']');
      } else {
        this[node.key.type](node.key, state);
      }
      formatSequence(state, node.value.params);
      state.write(' ');
      this[node.value.body.type](node.value.body, state);
    },
    ClassExpression: function ClassExpression(node, state) {
      this.ClassDeclaration(node, state);
    },
    ArrowFunctionExpression: function ArrowFunctionExpression(node, state) {
      state.write(node.async ? 'async ' : '', node);
      var params = node.params;

      if (params != null) {
        // Omit parenthesis if only one named parameter
        if (params.length === 1 && params[0].type[0] === 'I') {
          // If params[0].type[0] starts with 'I', it can't be `ImportDeclaration` nor `IfStatement` and thus is `Identifier`
          state.write(params[0].name, params[0]);
        } else {
          formatSequence(state, node.params);
        }
      }
      state.write(' => ');
      if (node.body.type[0] === 'O') {
        // Body is an object expression
        state.write('(');
        this.ObjectExpression(node.body, state);
        state.write(')');
      } else {
        this[node.body.type](node.body, state);
      }
    },
    ThisExpression: function ThisExpression(node, state) {
      state.write('this', node);
    },
    Super: function Super(node, state) {
      state.write('super', node);
    },

    RestElement: RestElement = function RestElement(node, state) {
      state.write('...');
      this[node.argument.type](node.argument, state);
    },
    SpreadElement: RestElement,
    YieldExpression: function YieldExpression(node, state) {
      state.write(node.delegate ? 'yield*' : 'yield');
      if (node.argument) {
        state.write(' ');
        this[node.argument.type](node.argument, state);
      }
    },
    AwaitExpression: function AwaitExpression(node, state) {
      state.write('await ');
      if (node.argument) {
        this[node.argument.type](node.argument, state);
      }
    },
    TemplateLiteral: function TemplateLiteral(node, state) {
      var quasis = node.quasis,
          expressions = node.expressions;

      state.write('`');
      var length = expressions.length;

      for (var i = 0; i < length; i++) {
        var expression = expressions[i];
        state.write(quasis[i].value.raw);
        state.write('${');
        this[expression.type](expression, state);
        state.write('}');
      }
      state.write(quasis[quasis.length - 1].value.raw);
      state.write('`');
    },
    TaggedTemplateExpression: function TaggedTemplateExpression(node, state) {
      this[node.tag.type](node.tag, state);
      this[node.quasi.type](node.quasi, state);
    },

    ArrayExpression: ArrayExpression = function ArrayExpression(node, state) {
      state.write('[');
      if (node.elements.length > 0) {
        var elements = node.elements,
            length = elements.length;

        for (var i = 0;;) {
          var element = elements[i];
          if (element != null) {
            this[element.type](element, state);
          }
          if (++i < length) {
            state.write(', ');
          } else {
            if (element == null) {
              state.write(', ');
            }
            break;
          }
        }
      }
      state.write(']');
    },
    ArrayPattern: ArrayExpression,
    ObjectExpression: function ObjectExpression(node, state) {
      var indent = state.indent.repeat(state.indentLevel++);
      var lineEnd = state.lineEnd,
          writeComments = state.writeComments;

      var propertyIndent = indent + state.indent;
      state.write('{');
      if (node.properties.length > 0) {
        state.write(lineEnd);
        if (writeComments && node.comments != null) {
          formatComments(state, node.comments, propertyIndent, lineEnd);
        }
        var comma = ',' + lineEnd;
        var properties = node.properties,
            length = properties.length;

        for (var i = 0;;) {
          var property = properties[i];
          if (writeComments && property.comments != null) {
            formatComments(state, property.comments, propertyIndent, lineEnd);
          }
          state.write(propertyIndent);
          this.Property(property, state);
          if (++i < length) {
            state.write(comma);
          } else {
            break;
          }
        }
        state.write(lineEnd);
        if (writeComments && node.trailingComments != null) {
          formatComments(state, node.trailingComments, propertyIndent, lineEnd);
        }
        state.write(indent + '}');
      } else if (writeComments) {
        if (node.comments != null) {
          state.write(lineEnd);
          formatComments(state, node.comments, propertyIndent, lineEnd);
          if (node.trailingComments != null) {
            formatComments(state, node.trailingComments, propertyIndent, lineEnd);
          }
          state.write(indent + '}');
        } else if (node.trailingComments != null) {
          state.write(lineEnd);
          formatComments(state, node.trailingComments, propertyIndent, lineEnd);
          state.write(indent + '}');
        } else {
          state.write('}');
        }
      } else {
        state.write('}');
      }
      state.indentLevel--;
    },
    Property: function Property(node, state) {
      if (node.method || node.kind[0] !== 'i') {
        // Either a method or of kind `set` or `get` (not `init`)
        this.MethodDefinition(node, state);
      } else {
        if (!node.shorthand) {
          if (node.computed) {
            state.write('[');
            this[node.key.type](node.key, state);
            state.write(']');
          } else {
            this[node.key.type](node.key, state);
          }
          state.write(': ');
        }
        this[node.value.type](node.value, state);
      }
    },
    ObjectPattern: function ObjectPattern(node, state) {
      state.write('{');
      if (node.properties.length > 0) {
        var properties = node.properties,
            length = properties.length;

        for (var i = 0;;) {
          this[properties[i].type](properties[i], state);
          if (++i < length) {
            state.write(', ');
          } else {
            break;
          }
        }
      }
      state.write('}');
    },
    SequenceExpression: function SequenceExpression(node, state) {
      formatSequence(state, node.expressions);
    },
    UnaryExpression: function UnaryExpression(node, state) {
      if (node.prefix) {
        state.write(node.operator);
        if (node.operator.length > 1) {
          state.write(' ');
        }
        if (EXPRESSIONS_PRECEDENCE[node.argument.type] < EXPRESSIONS_PRECEDENCE.UnaryExpression) {
          state.write('(');
          this[node.argument.type](node.argument, state);
          state.write(')');
        } else {
          this[node.argument.type](node.argument, state);
        }
      } else {
        // FIXME: This case never occurs
        this[node.argument.type](node.argument, state);
        state.write(node.operator);
      }
    },
    UpdateExpression: function UpdateExpression(node, state) {
      // Always applied to identifiers or members, no parenthesis check needed
      if (node.prefix) {
        state.write(node.operator);
        this[node.argument.type](node.argument, state);
      } else {
        this[node.argument.type](node.argument, state);
        state.write(node.operator);
      }
    },
    AssignmentExpression: function AssignmentExpression(node, state) {
      this[node.left.type](node.left, state);
      state.write(' ' + node.operator + ' ');
      this[node.right.type](node.right, state);
    },
    AssignmentPattern: function AssignmentPattern(node, state) {
      this[node.left.type](node.left, state);
      state.write(' = ');
      this[node.right.type](node.right, state);
    },

    BinaryExpression: BinaryExpression = function BinaryExpression(node, state) {
      if (node.operator === 'in') {
        // Avoids confusion in `for` loops initializers
        state.write('(');
        formatBinaryExpressionPart(state, node.left, node, false);
        state.write(' ' + node.operator + ' ');
        formatBinaryExpressionPart(state, node.right, node, true);
        state.write(')');
      } else {
        formatBinaryExpressionPart(state, node.left, node, false);
        state.write(' ' + node.operator + ' ');
        formatBinaryExpressionPart(state, node.right, node, true);
      }
    },
    LogicalExpression: BinaryExpression,
    ConditionalExpression: function ConditionalExpression(node, state) {
      if (EXPRESSIONS_PRECEDENCE[node.test.type] > EXPRESSIONS_PRECEDENCE.ConditionalExpression) {
        this[node.test.type](node.test, state);
      } else {
        state.write('(');
        this[node.test.type](node.test, state);
        state.write(')');
      }
      state.write(' ? ');
      this[node.consequent.type](node.consequent, state);
      state.write(' : ');
      this[node.alternate.type](node.alternate, state);
    },
    NewExpression: function NewExpression(node, state) {
      state.write('new ');
      if (EXPRESSIONS_PRECEDENCE[node.callee.type] < EXPRESSIONS_PRECEDENCE.CallExpression || hasCallExpression(node.callee)) {
        state.write('(');
        this[node.callee.type](node.callee, state);
        state.write(')');
      } else {
        this[node.callee.type](node.callee, state);
      }
      formatSequence(state, node['arguments']);
    },
    CallExpression: function CallExpression(node, state) {
      if (EXPRESSIONS_PRECEDENCE[node.callee.type] < EXPRESSIONS_PRECEDENCE.CallExpression) {
        state.write('(');
        this[node.callee.type](node.callee, state);
        state.write(')');
      } else {
        this[node.callee.type](node.callee, state);
      }
      formatSequence(state, node['arguments']);
    },
    MemberExpression: function MemberExpression(node, state) {
      if (EXPRESSIONS_PRECEDENCE[node.object.type] < EXPRESSIONS_PRECEDENCE.MemberExpression) {
        state.write('(');
        this[node.object.type](node.object, state);
        state.write(')');
      } else {
        this[node.object.type](node.object, state);
      }
      if (node.computed) {
        state.write('[');
        this[node.property.type](node.property, state);
        state.write(']');
      } else {
        state.write('.');
        this[node.property.type](node.property, state);
      }
    },
    MetaProperty: function MetaProperty(node, state) {
      state.write(node.meta.name + '.' + node.property.name, node);
    },
    Identifier: function Identifier(node, state) {
      state.write(node.name, node);
    },
    Literal: function Literal(node, state) {
      if (node.raw != null) {
        state.write(node.raw, node);
      } else if (node.regex != null) {
        this.RegExpLiteral(node, state);
      } else {
        state.write(stringify(node.value), node);
      }
    },
    RegExpLiteral: function RegExpLiteral(node, state) {
      var regex = node.regex;

      state.write('/' + regex.pattern + '/' + regex.flags, node);
    }
  };

  var EMPTY_OBJECT = {};

  var State = function () {
    function State(options) {
      _classCallCheck(this, State);

      var setup = options == null ? EMPTY_OBJECT : options;
      this.output = '';
      // Functional options
      if (setup.output != null) {
        this.output = setup.output;
        this.write = this.writeToStream;
      } else {
        this.output = '';
      }
      this.generator = setup.generator != null ? setup.generator : baseGenerator;
      // Formating setup
      this.indent = setup.indent != null ? setup.indent : '  ';
      this.lineEnd = setup.lineEnd != null ? setup.lineEnd : '\n';
      this.indentLevel = setup.startingIndentLevel != null ? setup.startingIndentLevel : 0;
      this.writeComments = setup.comments ? setup.comments : false;
      // Source map
      if (setup.sourceMap != null) {
        this.write = setup.output == null ? this.writeAndMap : this.writeToStreamAndMap;
        this.sourceMap = setup.sourceMap;
        this.line = 1;
        this.column = 0;
        this.lineEndSize = this.lineEnd.split('\n').length - 1;
        this.mapping = {
          original: null,
          generated: this,
          name: undefined,
          source: setup.sourceMap.file || setup.sourceMap._file
        };
      }
    }

    State.prototype.write = function write(code) {
      this.output += code;
    };

    State.prototype.writeToStream = function writeToStream(code) {
      this.output.write(code);
    };

    State.prototype.writeAndMap = function writeAndMap(code, node) {
      this.output += code;
      this.map(code, node);
    };

    State.prototype.writeToStreamAndMap = function writeToStreamAndMap(code, node) {
      this.output.write(code);
      this.map(code, node);
    };

    State.prototype.map = function map(code, node) {
      if (node != null && node.loc != null) {
        var mapping = this.mapping;

        mapping.original = node.loc.start;
        mapping.name = node.name;
        this.sourceMap.addMapping(mapping);
      }
      if (code.length > 0) {
        if (this.lineEndSize > 0) {
          if (code.endsWith(this.lineEnd)) {
            this.line += this.lineEndSize;
            this.column = 0;
          } else if (code[code.length - 1] === '\n') {
            // Case of inline comment
            this.line++;
            this.column = 0;
          } else {
            this.column += code.length;
          }
        } else {
          if (code[code.length - 1] === '\n') {
            // Case of inline comment
            this.line++;
            this.column = 0;
          } else {
            this.column += code.length;
          }
        }
      }
    };

    State.prototype.toString = function toString() {
      return this.output;
    };

    return State;
  }();

  function generate(node, options) {
    /*
    Returns a string representing the rendered code of the provided AST `node`.
    The `options` are:
     - `indent`: string to use for indentation (defaults to `␣␣`)
    - `lineEnd`: string to use for line endings (defaults to `\n`)
    - `startingIndentLevel`: indent level to start from (defaults to `0`)
    - `comments`: generate comments if `true` (defaults to `false`)
    - `output`: output stream to write the rendered code to (defaults to `null`)
    - `generator`: custom code generator (defaults to `baseGenerator`)
    */
    var state = new State(options);
    // Travel through the AST node and generate the code
    state.generator[node.type](node, state);
    return state.output;
  }
});

//# sourceMappingURL=astring.js.map