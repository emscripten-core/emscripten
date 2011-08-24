(function() {
  var ACCESSOR, CoffeeScript, Module, REPL_PROMPT, REPL_PROMPT_CONTINUATION, SIMPLEVAR, Script, autocomplete, backlog, completeAttribute, completeVariable, enableColours, error, g, getCompletions, inspect, nonContextGlobals, readline, repl, run, sandbox, stdin, stdout, _i, _len;
  CoffeeScript = require('./coffee-script');
  readline = require('readline');
  inspect = require('util').inspect;
  Script = require('vm').Script;
  Module = require('module');
  REPL_PROMPT = 'coffee> ';
  REPL_PROMPT_CONTINUATION = '......> ';
  enableColours = false;
  if (process.platform !== 'win32') {
    enableColours = !process.env.NODE_DISABLE_COLORS;
  }
  stdin = process.openStdin();
  stdout = process.stdout;
  error = function(err) {
    return stdout.write((err.stack || err.toString()) + '\n\n');
  };
  backlog = '';
  sandbox = Script.createContext();
  nonContextGlobals = ['Buffer', 'console', 'process', 'setInterval', 'clearInterval', 'setTimeout', 'clearTimeout'];
  for (_i = 0, _len = nonContextGlobals.length; _i < _len; _i++) {
    g = nonContextGlobals[_i];
    sandbox[g] = global[g];
  }
  sandbox.global = sandbox.root = sandbox.GLOBAL = sandbox;
  run = function(buffer) {
    var code, returnValue, _;
    if (!buffer.toString().trim() && !backlog) {
      repl.prompt();
      return;
    }
    code = backlog += buffer;
    if (code[code.length - 1] === '\\') {
      backlog = "" + backlog.slice(0, -1) + "\n";
      repl.setPrompt(REPL_PROMPT_CONTINUATION);
      repl.prompt();
      return;
    }
    repl.setPrompt(REPL_PROMPT);
    backlog = '';
    try {
      _ = sandbox._;
      returnValue = CoffeeScript.eval("_=(" + code + "\n)", {
        sandbox: sandbox,
        filename: 'repl',
        modulename: 'repl'
      });
      if (returnValue === void 0) {
        sandbox._ = _;
      } else {
        process.stdout.write(inspect(returnValue, false, 2, enableColours) + '\n');
      }
    } catch (err) {
      error(err);
    }
    return repl.prompt();
  };
  ACCESSOR = /\s*([\w\.]+)(?:\.(\w*))$/;
  SIMPLEVAR = /\s*(\w*)$/i;
  autocomplete = function(text) {
    return completeAttribute(text) || completeVariable(text) || [[], text];
  };
  completeAttribute = function(text) {
    var all, completions, match, obj, prefix, val;
    if (match = text.match(ACCESSOR)) {
      all = match[0], obj = match[1], prefix = match[2];
      try {
        val = Script.runInContext(obj, sandbox);
      } catch (error) {
        return;
      }
      completions = getCompletions(prefix, Object.getOwnPropertyNames(val));
      return [completions, prefix];
    }
  };
  completeVariable = function(text) {
    var completions, free, possibilities, vars, _ref;
    if (free = (_ref = text.match(SIMPLEVAR)) != null ? _ref[1] : void 0) {
      vars = Script.runInContext('Object.getOwnPropertyNames(this)', sandbox);
      possibilities = vars.concat(CoffeeScript.RESERVED);
      completions = getCompletions(free, possibilities);
      return [completions, free];
    }
  };
  getCompletions = function(prefix, candidates) {
    var el, _j, _len2, _results;
    _results = [];
    for (_j = 0, _len2 = candidates.length; _j < _len2; _j++) {
      el = candidates[_j];
      if (el.indexOf(prefix) === 0) {
        _results.push(el);
      }
    }
    return _results;
  };
  process.on('uncaughtException', error);
  if (readline.createInterface.length < 3) {
    repl = readline.createInterface(stdin, autocomplete);
    stdin.on('data', function(buffer) {
      return repl.write(buffer);
    });
  } else {
    repl = readline.createInterface(stdin, stdout, autocomplete);
  }
  repl.on('attemptClose', function() {
    if (backlog) {
      backlog = '';
      process.stdout.write('\n');
      repl.setPrompt(REPL_PROMPT);
      return repl.prompt();
    } else {
      return repl.close();
    }
  });
  repl.on('close', function() {
    process.stdout.write('\n');
    return stdin.destroy();
  });
  repl.on('line', run);
  repl.setPrompt(REPL_PROMPT);
  repl.prompt();
}).call(this);
