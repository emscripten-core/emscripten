// llvm => internal intermediate representation

function intertyper(data) {
  // Substrate

  substrate = new Substrate('Intertyper');

  // Line splitter.
  substrate.addZyme('LineSplitter', {
    processItem: function(item) {
      var lines = item.llvmText.split('\n');
      var ret = [];
      var inContinual = false;
      for (var i = 0; i < lines.length; i++) {
        var line = lines[i];
        if (inContinual || new RegExp(/^\ +to.*/g).test(line)) {
          // to after invoke
          ret.slice(-1)[0].lineText += line;
          if (new RegExp(/^\ +\]/g).test(line)) { // end of llvm switch
            inContinual = false;
          }
        } else {
          ret.push({
            lineText: line,
            lineNum: i + 1,
          });
          if (new RegExp(/^\ +switch\ .*/g).test(line)) {
            // beginning of llvm switch
            inContinual = true;
          }
        }
      }
      this.forwardItems(ret.filter(function(item) { return item.lineText; }), 'Tokenizer');
    },
  });

  // Line tokenizer
  substrate.addZyme('Tokenizer', {
    processItem: function(item, inner) {
      var lineText = item.lineText + " ";
      var tokens = [];
      var tokenStart = -1;
      var indent = -1;
      var quotes = 0;
      var lastToken = null;
      var i = 0;
      // Note: '{' is not an encloser, as its use in functions is split over many lines
      var enclosers = {
        '[': 0,
        ']': '[',
        '(': 0,
        ')': '(',
        '<': 0,
        '>': '<',
      };
      function notEnclosed() {
        if (enclosers['['] > 0 || enclosers['('] > 0 || enclosers['<'] > 0)
          return false;
        return true;
      }
      var that = this;
      function tryStartToken() {
        if (tokenStart == -1 && notEnclosed() && quotes == 0) {
          //print("try START " + tokenStart + ',' + JSON.stringify(enclosers));
          tokenStart = i;
        }
      }
      function tryFinishToken(includeThis) {
        if (tokenStart >= 0 && notEnclosed() && quotes == 0) {
          //print("try finish " + tokenStart + ',' + JSON.stringify(enclosers));
          var token = {
            text: lineText.substr(tokenStart, i-tokenStart + (includeThis ? 1 : 0)),
          };
          if (token.text[0] in enclosers) {
            token.item = that.processItem({
              lineText: token.text.substr(1, token.text.length-2)
            }, true);
            token.type = token.text[0];
          }
          if (indent == -1) {
            indent = tokenStart;
          }
          // merge certain tokens
          if ( (lastToken && lastToken.text == '%' && token.text[0] == '"' ) ||
               (lastToken && token.text.replace(/\*/g, '') == '') ) {
            lastToken.text += token.text;
          } else if (lastToken && isType(lastToken.text) && isFunctionDef(token)) {
            lastToken.text += ' ' + token.text;
          } else if (lastToken && token.text[token.text.length-1] == '}') {
            var openBrace = tokens.length-1;
            while (tokens[openBrace].text != '{') openBrace --;
            token = combineTokens(tokens.slice(openBrace+1));
            tokens.splice(openBrace, tokens.length-openBrace+1);
            tokens.push(token);
            token.type = '{';
            lastToken = token;
          } else {
            tokens.push(token);
            lastToken = token;
          }
          // print("new token: " + dump(lastToken));
          tokenStart = -1;
        }
      }
      for (; i < lineText.length; i++) {
        var letter = lineText[i];
        //print("letter: " + letter);
        switch (letter) {
          case ' ':
            tryFinishToken();
            break;
          case '"':
            tryFinishToken();
            tryStartToken();
            quotes = 1-quotes;
            break;
          case ',':
            tryFinishToken();
            if (notEnclosed() && quotes == 0) {
              tokens.push({ text: ',' });
            }
            break;
          default:
            if (letter in enclosers && quotes == 0) {
              if (typeof enclosers[letter] === 'number') {
                tryFinishToken();
                tryStartToken();
                enclosers[letter]++;
              } else {
                enclosers[enclosers[letter]]--;
                tryFinishToken(true);
              }
              //print('     post-enclosers: ' + JSON.stringify(enclosers));
            } else {
              tryStartToken();
            }
        }
      }
      var item = {
        tokens: tokens,
        indent: indent,
        lineNum: item.lineNum,
      };
      if (inner) {
        return [item];
      } else {
        this.forwardItem(item, 'Triager');
      }
    },
  });

  substrate.addZyme('Triager', {
    processItem: function(item) {
      function triage() {
        if (!item.intertype) {
          if (item.tokens[0].text in searchable(';', 'target'))
            return '/dev/null';
          if (item.tokens.length >= 3 && item.indent === 0 && item.tokens[1].text == '=')
            return 'Global';
          if (item.tokens.length >= 4 && item.indent === 0 && item.tokens[0].text == 'define' &&
             item.tokens.slice(-1)[0].text == '{')
            return 'FuncHeader';
          if (item.tokens.length >= 1 && item.indent === 0 && item.tokens[0].text.substr(-1) == ':')
            return 'Label';
          if (item.indent === 2 && item.tokens && item.tokens.length >= 3 && findTokenText(item, '=') >= 0 &&
              !item.intertype)
            return 'Assign';
          if (!item.intertype && item.indent === -1 && item.tokens && item.tokens.length >= 3 &&
              (item.tokens[0].text == 'load' || item.tokens[1].text == 'load'))
            return 'Load';
          if (!item.intertype && item.indent === -1 && item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'bitcast')
            return 'Bitcast';
          if (!item.intertype && item.indent === -1 && item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'getelementptr')
            return 'GEP';
          if (item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'call' && !item.intertype)
            return 'Call';
          if (item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'invoke' && !item.intertype)
            return 'Invoke';
          if (!item.intertype && item.indent === -1 && item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'alloca')
            return 'Alloca';
          if (!item.intertype && item.indent === -1 && item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'phi')
            return 'Phi';
          if (item.indent === -1 && item.tokens && item.tokens.length >= 3 &&
              ['add', 'sub', 'sdiv', 'udiv', 'mul', 'icmp', 'zext', 'urem', 'srem', 'fadd', 'fsub', 'fmul', 'fdiv', 'fcmp', 'uitofp', 'sitofp', 'fpext', 'fptrunc', 'fptoui', 'fptosi', 'trunc', 'sext', 'select', 'shl', 'shr', 'ashl', 'ashr', 'lshr', 'lshl', 'xor', 'or', 'and', 'ptrtoint', 'inttoptr'].indexOf(item.tokens[0].text) != -1 && !item.intertype)
            return 'Mathops';
          if (item.indent === 2 && item.tokens && item.tokens.length >= 5 &&
              (item.tokens[0].text == 'store' || item.tokens[1].text == 'store'))
            return 'Store';
          if (item.indent === 2 && item.tokens && item.tokens.length >= 3 && item.tokens[0].text == 'br' &&
                                     !item.intertype)
            return 'Branch';
          if (item.indent === 2 && item.tokens && item.tokens.length >= 2 && item.tokens[0].text == 'ret' &&
                                     !item.intertype)
            return 'Return';
          if (item.indent === 2 && item.tokens && item.tokens.length >= 2 && item.tokens[0].text == 'switch' &&
                                     !item.intertype)
            return 'Switch';
          if (item.indent === 0 && item.tokens && item.tokens.length >= 1 && item.tokens[0].text == '}' && !item.intertype)
            return 'FuncEnd';
          if (item.indent === 0 && item.tokens && item.tokens.length >= 4 && item.tokens[0].text == 'declare' &&
                                     !item.intertype)
            return 'External';
          if (item.indent === 2 && item.tokens && item.tokens[0].text == 'unreachable' &&
                                     !item.intertype)
            return 'Unreachable';
        } else {
          // Already intertyped
          if (item.parentSlot)
            return 'Reintegrator';
        }
        throw 'Invalid token, cannot triage: ' + dump(item);
      }
      this.forwardItem(item, triage(item));
    },
  });

  // Line parsers to intermediate form

  // globals: type or variable
  substrate.addZyme('Global', {
    processItem: function(item) {
      if (item.tokens[2].text == 'alias') {
        return; // TODO: handle this. See raytrace.cpp
      }
      if (item.tokens[2].text == 'type') {
        var fields = [];
        if (isNumberType(item.tokens[3].text)) {
          // Clang sometimes has |= i32| instead of |= { i32 }|
          fields = [item.tokens[3].text];
        } else if (item.tokens[3].text != 'opaque') {
          if (item.tokens[3].type == '<') { // type <{ i8 }> XXX - check spec
            item.tokens[3] = item.tokens[3].item[0];
          }
          var subTokens = item.tokens[3].tokens;
          subTokens.push({text:','});
          while (subTokens[0]) {
            var stop = 1;
            while ([','].indexOf(subTokens[stop].text) == -1) stop ++;
            fields.push(combineTokens(subTokens.slice(0, stop)).text);
            subTokens.splice(0, stop+1);
          }
        }
        return [{
        __result__: true, // XXX can remove these
          intertype: 'type',
          name_: item.tokens[0].text,
          fields: fields,
          lineNum: item.lineNum,
        }]
      } else {
        // variable
        var ident = item.tokens[0].text;
        while (item.tokens[2].text in { 'private': 0, 'constant': 0, 'appending': 0, 'global': 0, 'weak_odr': 0, 'internal': 0 })
          item.tokens.splice(2, 1);
        var ret = {
          __result__: true,
          intertype: 'globalVariable',
          ident: ident,
          type: item.tokens[2],
          lineNum: item.lineNum,
        };
        if (ident == '@llvm.global_ctors') {
          ret.ctors = [];
          var subTokens = item.tokens[3].item[0].tokens;
          splitTokenList(subTokens).forEach(function(segment) {
            ret.ctors.push(segment[1].tokens.slice(-1)[0].text);
          });
        } else {
          if (item.tokens[3].type == '<') { // type <{ i8 }> XXX - check spec
            item.tokens[3] = item.tokens[3].item[0].tokens;
          }

          if (item.tokens[3].text == 'c')
            item.tokens.splice(3, 1);
          ret.value = item.tokens[3];
        }
        return [ret];
      }
    },
  });
  // function header
  substrate.addZyme('FuncHeader', {
    processItem: function(item) {
      item.tokens = item.tokens.filter(function(token) {
        return ['noalias', 'available_externally', 'weak', 'internal', 'signext', 'zeroext', 'nounwind', 'define', 'linkonce_odr', 'inlinehint', '{'].indexOf(token.text) == -1;
      });
      return [{
        __result__: true,
        intertype: 'function',
        ident: item.tokens[1].text,
        returnType: item.tokens[0],
        params: item.tokens[2],
        lineNum: item.lineNum,
      }];
    },
  });
  // label
  substrate.addZyme('Label', {
    processItem: function(item) {
      return [{
        __result__: true,
        intertype: 'label',
        ident: '%' + item.tokens[0].text.substr(0, item.tokens[0].text.length-1),
        lineNum: item.lineNum,
      }];
    },
  });

  // assignment
  substrate.addZyme('Assign', {
    processItem: function(item) {
      var opIndex = findTokenText(item, '=');
      var pair = splitItem({
        intertype: 'assign',
        ident: combineTokens(item.tokens.slice(0, opIndex)).text,
        lineNum: item.lineNum,
      }, 'value');
      this.forwardItem(pair.parent, 'Reintegrator');
      this.forwardItem(mergeInto(pair.child, { // Additional token, to be triaged and later re-integrated
        indent: -1,
        tokens: item.tokens.slice(opIndex+1),
      }), 'Triager');
    },
  });
  // reintegration - find intermediate representation-parsed items and
  // place back in parents TODO: Optimize this code to optimal O(..)
  substrate.addZyme('Reintegrator', makeReintegrator(function(parent, child) {
    // Special re-integration behaviors
    if (child.intertype == 'fastgetelementptrload') {
      parent.intertype = 'fastgetelementptrload';
    }
    this.forwardItem(parent, '/dev/stdout');
  }));

  // 'load'
  substrate.addZyme('Load', {
    processItem: function(item) {
      if (item.tokens[0].text == 'volatile') item.tokens.shift(0);
      item.pointerType = item.tokens[1];
      item.type = { text: removePointing(item.pointerType.text) };
      if (item.tokens[2].text == 'getelementptr') {
        var last = getTokenIndexByText(item.tokens, ';');
        var data = parseLLVMFunctionCall(item.tokens.slice(1, last));
        item.intertype = 'fastgetelementptrload';
        item.type = data.type;
        item.params = data.params;
        item.pointer = { text: data.ident };
        item.value = data.value;
      } else {
        item.intertype = 'load';
        if (item.tokens[2].text == 'bitcast') {
          item.pointer = item.tokens[3].item[0].tokens[1];
          item.originalType = item.tokens[3].item[0].tokens[0];
        } else {
          item.pointer = item.tokens[2];
        }
      }
      item.ident = item.pointer.text;
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'bitcast'
  substrate.addZyme('Bitcast', {
    processItem: function(item) {
      item.intertype = 'bitcast';
      item.type = item.tokens[1];
      item.ident = item.tokens[2].text;
      item.type2 = item.tokens[4];
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'getelementptr'
  substrate.addZyme('GEP', {
    processItem: function(item) {
      var first = 0;
      while (!isType(item.tokens[first].text)) first++;
      var last = getTokenIndexByText(item.tokens, ';');
      var segment = [ item.tokens[first], { text: 'getelementptr' }, null, { item: [ {
        tokens: item.tokens.slice(first, last)
      } ] } ];
      var data = parseLLVMFunctionCall(segment);
      item.intertype = 'getelementptr';
      item.type = data.type;
      item.params = data.params;
      item.ident = data.ident;
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'call'
  substrate.addZyme('Call', {
    processItem: function(item) {
      item.intertype = 'call';
      if (['signext', 'zeroext'].indexOf(item.tokens[1].text) != -1) {
        item.tokens.splice(1, 1);
      }
      item.type = item.tokens[1];
      item.functionType = '';
      while (['@', '%'].indexOf(item.tokens[2].text[0]) == -1) {
        item.functionType += item.tokens[2].text;
        item.tokens.splice(2, 1);
      }
      item.ident = item.tokens[2].text;
      if (item.ident.substr(-2) == '()') {
        // See comment in isStructType()
        item.ident = item.ident.substr(0, item.ident.length-2);
        // Also, we remove some spaces which might occur.
        while (item.ident[item.ident.length-1] == ' ') {
          item.ident = item.ident.substr(0, item.ident.length-1);
        }
        item.params = [];
      } else {
        item.params = parseParamTokens(item.tokens[3].item[0].tokens);
      }
      if (item.indent == 2) {
        // standalone call - not in assign
        item.standalone = true;
        item.__result__ = true;
        return [item];
      }
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'invoke'
  substrate.addZyme('Invoke', {
    processItem: function(item) {
      item.intertype = 'invoke';
      item.type = item.tokens[1];
      item.functionType = '';
      while (['@', '%'].indexOf(item.tokens[2].text[0]) == -1) {
        item.functionType += item.tokens[2].text;
        item.tokens.splice(2, 1);
      }
      cleanOutTokens(['alignstack', 'alwaysinline', 'inlinehint', 'naked', 'noimplicitfloat', 'noinline', 'alwaysinline attribute.', 'noredzone', 'noreturn', 'nounwind', 'optsize', 'readnone', 'readonly', 'ssp', 'sspreq'], item.tokens, 4);
      item.ident = item.tokens[2].text;
      item.params = parseParamTokens(item.tokens[3].item[0].tokens);
      item.toLabel = toNiceIdent(item.tokens[6].text);
      item.unwindLabel = toNiceIdent(item.tokens[9].text);
      if (item.indent == 2) {
        // standalone call - not in assign
        item.standalone = true;
        item.__result__ = true;
        return [item];
      }
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'alloca'
  substrate.addZyme('Alloca', {
    processItem: function(item) {
      item.intertype = 'alloca';
      item.allocatedType = item.tokens[1];
      item.type = { text: addPointing(item.tokens[1].text) }; // type of pointer we will get
      item.type2 = { text: item.tokens[1].text }; // value we will create, and get a pointer to
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'phi'
  substrate.addZyme('Phi', {
    processItem: function(item) {
      item.intertype = 'phi';
      item.type = { text: item.tokens[1].text }
      item.label1 = item.tokens[2].item[0].tokens[2].text;
      item.value1 = item.tokens[2].item[0].tokens[0].text;
      item.label2 = item.tokens[4].item[0].tokens[2].text;
      item.value2 = item.tokens[4].item[0].tokens[0].text;
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // mathops
  substrate.addZyme('Mathops', {
    processItem: function(item) {
      item.intertype = 'mathop';
      item.op = item.tokens[0].text;
      item.variant = null;
      if (item.tokens[1].text == 'nsw') item.tokens.splice(1, 1);
      if (['icmp', 'fcmp'].indexOf(item.op) != -1) {
        item.variant = item.tokens[1].text;
        item.tokens.splice(1, 1);
      }
      var segments = splitTokenList(item.tokens.slice(1));
      for (var i = 1; i <= 4; i++) {
        if (segments[i-1]) {
          item['param'+i] = parseLLVMSegment(segments[i-1]);
        }
      }
      item.type = { text: item.param1.type }; // TODO: unobject this
      this.forwardItem(item, 'Reintegrator');
    },
  });
  // 'store'
  substrate.addZyme('Store', {
    processItem: function(item) {
      if (item.tokens[0].text == 'volatile') item.tokens.shift(0);
      var segments = splitTokenList(item.tokens.slice(1));
      var ret = {
        __result__: true,
        intertype: 'store',
        valueType: item.tokens[1],
        value: parseLLVMSegment(segments[0]), // TODO: Make everything use this method, with finalizeLLVMParameter too
        pointer: parseLLVMSegment(segments[1]),
        lineNum: item.lineNum,
      };
      ret.ident = ret.pointer.ident;
      ret.pointerType = { text: ret.pointer.type }; // TODO: unobject this
      return [ret];
    },
  });
  // 'br'
  substrate.addZyme('Branch', {
    processItem: function(item) {
      if (item.tokens[1].text == 'label') {
        return [{
          __result__: true,
          intertype: 'branch',
          label: toNiceIdent(item.tokens[2].text),
          lineNum: item.lineNum,
        }];
      } else {
        return [{
          __result__: true,
          intertype: 'branch',
          ident: item.tokens[2].text,
          labelTrue: toNiceIdent(item.tokens[5].text),
          labelFalse: toNiceIdent(item.tokens[8].text),
          lineNum: item.lineNum,
        }];
      }
    },
  });
  // 'ret'
  substrate.addZyme('Return', {
    processItem: function(item) {
      return [{
        __result__: true,
        intertype: 'return',
        type: item.tokens[1].text,
        value: item.tokens[2] ? item.tokens[2].text : null,
        lineNum: item.lineNum,
      }];
    },
  });
  // 'switch'
  substrate.addZyme('Switch', {
    processItem: function(item) {
      function parseSwitchLabels(item) {
        var ret = [];
        var tokens = item.item[0].tokens;
        while (tokens.length > 0) {
          ret.push({
            value: tokens[1].text,
            label: toNiceIdent(tokens[4].text),
          });
          tokens = tokens.slice(5);
        }
        return ret;
      }
      return [{
        __result__: true,
        intertype: 'switch',
        type: item.tokens[1].text,
        ident: item.tokens[2].text,
        defaultLabel: item.tokens[5].text,
        switchLabels: parseSwitchLabels(item.tokens[6]),
        lineNum: item.lineNum,
      }];
    },
  });
  // function end
  substrate.addZyme('FuncEnd', {
    processItem: function(item) {
      return [{
        __result__: true,
        intertype: 'functionEnd',
        lineNum: item.lineNum,
      }];
    },
  });
  // external function stub
  substrate.addZyme('External', {
    processItem: function(item) {
      return [{
        __result__: true,
        intertype: 'functionStub',
        ident: item.tokens[2].text,
        returnType: item.tokens[1],
        params: item.tokens[3],
        lineNum: item.lineNum,
      }];
    },
  });
  // 'unreachable'
  substrate.addZyme('Unreachable', {
    processItem: function(item) {
      return [{
        __result__: true,
        intertype: 'unreachable',
        lineNum: item.lineNum,
      }];
    },
  });

  // Input

  substrate.addItem({
    llvmText: data,
  }, 'LineSplitter');

  return substrate.solve();
}

