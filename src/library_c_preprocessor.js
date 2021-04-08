mergeInto(LibraryManager.library, {
  // Removes all C++ '//' and '/* */' comments from the given source string.
  // N.b. will also eat comments inside strings.
  $remove_cpp_comments_in_shaders: function(code) {
    var i = 0, out = '', ch, next, len = code.length;
    for(; i < len; ++i) {
      ch = code[i];
      if (ch == '/') {
        next = code[i+1];
        if (next == '/') {
          while(i < len && code[i+1] != '\n') ++i;
        } else if (next == '*') {
          while(i < len && (code[i-1] != '*' || code[i] != '/')) ++i;
        } else {
          out += ch;
        }
      } else {
        out += ch;
      }
    }
    return out;
  },

  // Runs C preprocessor algorithm on the given code string.
  // Supported preprocessor directives: #if, #ifdef, #ifndef, #else, #elif, #endif, #define and #undef.
  $preprocess_c_code: function(code) {
    var i = 0, // iterator over the input string
      len = code.length, // cache input length
      out = '', // generates the preprocessed output string
      stack = [1], // preprocessing stack (state of active/inactive #ifdef/#else blocks we are currently inside)
      defs = { // a mapping 'symbolname' -> function(args) which evaluates the given cpp macro, e.g. #define FOO(x) x+10.
      'defined': function(args) { // built-in "#if defined(x)"" macro.
#if ASSERTIONS
        assert(args.length == 1);
#endif
        return defs[args[0]] ? 1 : 0;
      }
    };

    // Returns true if str[i] is whitespace.
    function isWhitespace(str, i) {
      return !(str.charCodeAt(i) > 32); // Compare as negation to treat end-of-string undefined as whitespace
    }

    // Returns index to the next whitespace character starting at str[i].
    function nextWhitespace(str, i) {
      while(!isWhitespace(str, i)) ++i;
      return i;
    }

    // Finds the index of closing parens ')' from the starting parens '(' at arr[i].
    // Used polymorphically for strings ("foo") and token arrays (['(', 'foo', ')']) as input.
    function findClosingParensIndex(arr, i) {
      for(var nesting = 0; i < arr.length; ++i) {
        if (arr[i] == '(') ++nesting;
        if (arr[i] == ')' && --nesting == 0) {
          return i;
        }
      }
    }

    // Returns an integer ID classification of the character at str[idx], used for tokenization purposes.
    function classifyChar(str, idx) {
      var cc = str.charCodeAt(idx);
  #if ASSERTIONS
      assert(!(cc > 127), "Only 7-bit ASCII can be used in preprocessor #if/#ifdef/#define statements!");
  #endif
      if (cc > 32) {
        if (cc < 48) return 1; // an operator symbol, any of !"#$%&'()*+,-./
        if (cc < 58) return 2; // a number 0123456789
        if (cc < 65) return 1; // an operator symbol, any of :;<=>?@
        if (cc < 91 || cc == 95/*_*/) return 3; // a character, any of A-Z or _
        if (cc < 97) return 1; // an operator symbol, any of [\]^`
        if (cc < 123) return 3; // a character, any of a-z
        return 1; // an operator symbol, any of {|}~
      }
      return cc < 33 ? 0 : 4; // 0=whitespace, 4=end-of-string
    }

    // Returns a tokenized array of the given string expression, i.e. "FOO > BAR && BAZ" -> ["FOO", ">", "BAR", "&&", "BAZ"]
    // Optionally keeps whitespace as tokens to be able to reconstruct the original input string.
    function tokenize(exprString, keepWhitespace) {
      var out = [], len = exprString.length;
      for(var i = 0; i <= len; ++i) {
        var kind = classifyChar(exprString, i);
        if (kind == 2 || kind == 3) { // a character or a number
          for(var j = i+1; j <= len; ++j) {
            var kind2 = classifyChar(exprString, j);
            if (kind2 != kind && (kind2 != 2 || kind != 3)) { // parse number sequence "423410", and identifier sequence "FOO32BAR"
              out.push(exprString.substring(i, j));
              i = j-1;
              break;
            }
          }
        } else if (kind == 1) {
          // Lookahead for two-character operators.
          var op2 = exprString.substr(i, 2);
          if (['<=', '>=', '==', '!=', '&&', '||'].indexOf(op2) >= 0) {
            out.push(op2);
            ++i;
          } else {
            out.push(exprString[i]);
          }
        }
      }
      return out;
    }

    // Expands preprocessing macros on substring str[lineStart...lineEnd]
    function expandMacros(str, lineStart, lineEnd) {
      if (!lineEnd) lineEnd = str.length;
      var len = str.length;
      var out = '';
      for(var i = lineStart; i < lineEnd; ++i) {
        var kind = classifyChar(str, i);
        if (kind == 3) {
          for(var j = i + 1; j <= lineEnd; ++j) {
            var kind2 = classifyChar(str, j);
            if (kind2 != 2 && kind2 != 3) {
              var symbol = str.substring(i, j);
              var pp = defs[symbol];
              if (pp) {
                var expanded = str.substring(lineStart, i);
                if (str[j] == '(') {
                  var closeParens = findClosingParensIndex(str, j);
#if ASSERTIONS
                  assert(str[closeParens] == ')');
#endif
                  expanded += pp(str.substring(j+1, closeParens).split(',')) + str.substring(closeParens+1, lineEnd);
                } else {
                  expanded += pp() + str.substring(j, lineEnd);
                }
                return expandMacros(expanded, 0);
              } else {
                out += symbol;
                i = j-1;
                break;
              }
            }
          }
        } else {
          out += str[i];
        }
      }
      return out;
    }

    // Given a token list e.g. ['2', '>', '1'], returns a function that evaluates that token list.
    function buildExprTree(tokens) {
      // Consume tokens array into a function tree until the tokens array is exhausted
      // to a single root node that evaluates it.
      while(tokens.length > 1 || typeof(tokens[0]) != 'function') {
        tokens = (function(tokens) {
          // Find the index 'i' of the operator we should evaluate next:
          var i, j, p, operatorAndPriority = -2;
          for(j = 0; j < tokens.length; ++j) {
            if ((p = ['*', '/', '+', '-', '!', '<', '<=', '>', '>=', '==', '!=', '&&', '||', '('].indexOf(tokens[j])) > operatorAndPriority) {
              i = j;
              operatorAndPriority = p;
            }
          }

          if (operatorAndPriority == 13 /* parens '(' */) {
            // Find the closing parens position
            var j = findClosingParensIndex(tokens, i);
            if (j) {
              tokens.splice(i, j+1-i, buildExprTree(tokens.slice(i+1, j)));
              return tokens;
            }
          }

          if (operatorAndPriority == 4 /* unary ! */) {
            // Special case: the unary operator ! needs to evaluate right-to-left.
            i = tokens.lastIndexOf('!');
            var innerExpr = buildExprTree(tokens.slice(i+1, i+2));
            tokens.splice(i, 2, function() { return !innerExpr(); })
            return tokens;
          }

          // A binary operator:
          if (operatorAndPriority >= 0) {
            var left = buildExprTree(tokens.slice(0, i));
            var right = buildExprTree(tokens.slice(i+1));
            switch(tokens[i]) {
              case '&&': return [function() { return left() && right(); }];
              case '||': return [function() { return left() || right(); }];
              case '==': return [function() { return left() == right(); }];
              case '!=': return [function() { return left() != right(); }];
              case '<' : return [function() { return left() <  right(); }];
              case '<=': return [function() { return left() <= right(); }];
              case '>' : return [function() { return left() >  right(); }];
              case '>=': return [function() { return left() >= right(); }];
              case  '+': return [function() { return left()  + right(); }];
              case  '-': return [function() { return left()  - right(); }];
              case  '*': return [function() { return left()  * right(); }];
              case  '/': return [function() { return Math.floor(left() / right()); }];
            }
          }
          // else a number:
#if ASSERTIONS
          if (tokens[i] == ')') throw 'Parsing failure, mismatched parentheses in parsing!' + tokens.toString();
          assert(operatorAndPriority == -1);
#endif
          var num = parseInt(tokens[i]);
          return [function() { return num; }]
        })(tokens);
      }
      return tokens[0];
    }

    // Preprocess the input one line at a time.
    for(; i < len; ++i) {
      // Find the start of the current line.
      var lineStart = i;

      // Seek iterator to end of current line.
      i = code.indexOf('\n', i);
      if (i < 0) i = len;

      // Find the first non-whitespace character on the line.
      for(var j = lineStart; j < i && isWhitespace(code, j); ++j);

      // Is this a non-preprocessor directive line?
      if (code[j] != '#') { // non-preprocessor line?
        if (stack[stack.length-1]) {
          out += expandMacros(code, lineStart, i) + '\n';
        }
        continue;
      }
      // This is a preprocessor directive line, e.g. #ifdef or #define.

      // Parse the line as #<directive> <expression>
      var space = nextWhitespace(code, j);
      var directive = code.substring(j+1, space);
      var expression = code.substring(space, i).trim();

      switch(directive) {
      case 'if':
        var tokens = tokenize(expandMacros(expression, 0));
        var exprTree = buildExprTree(tokens);
        var evaluated = exprTree();
        stack.push(!!evaluated * stack[stack.length-1]);
        break;
      case 'ifdef': stack.push(!!defs[expression] * stack[stack.length-1]); break;
      case 'ifndef': stack.push(!defs[expression] * stack[stack.length-1]); break;
      case 'else': stack[stack.length-1] = 1-stack[stack.length-1]; break;
      case 'endif': stack.pop(); break;
      case 'define':
        // This could either be a macro with input args (#define MACRO(x,y) x+y), or a direct expansion #define FOO 2,
        // figure out which.
        var macroStart = expression.indexOf('(');
        var firstWs = nextWhitespace(expression, 0);
        if (firstWs < macroStart) macroStart = 0;
        if (macroStart > 0) { // #define MACRO( x , y , z ) <statement of x,y and z>
          var macroEnd = expression.indexOf(')', macroStart);
          let params = expression.substring(macroStart+1, macroEnd).split(',').map(x => x.trim());
          let value = tokenize(expression.substring(macroEnd+1).trim())
          defs[expression.substring(0, macroStart)] = function(args) {
            var ret = '';
            value.forEach((x) => {
              var argIndex = params.indexOf(x);
              ret += (argIndex >= 0) ? args[argIndex] : x;
            });
            return ret;
          };
        } else { // #define FOO (x + y + z)
          let value = expandMacros(expression.substring(firstWs+1).trim(), 0);
          defs[expression.substring(0, firstWs)] = function() {
            return value;
          };
        }
        break;
      case 'undef': delete defs[expression]; break;
      default:
        if (directive == 'version' || directive == 'pragma') { // GLSL shader compiler specific #directives.
          out += expandMacros(code, lineStart, i) + '\n';
        } else {
#if ASSERTIONS
          console.error('Unrecognized preprocessor directive #' + directive + '!');
#endif
        }
      }
    }
    return out;
  }
});
