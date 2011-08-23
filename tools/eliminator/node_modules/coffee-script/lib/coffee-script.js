(function() {
  var Lexer, RESERVED, compile, fs, lexer, parser, path, _ref;
  var __hasProp = Object.prototype.hasOwnProperty;
  fs = require('fs');
  path = require('path');
  _ref = require('./lexer'), Lexer = _ref.Lexer, RESERVED = _ref.RESERVED;
  parser = require('./parser').parser;
  if (require.extensions) {
    require.extensions['.coffee'] = function(module, filename) {
      var content;
      content = compile(fs.readFileSync(filename, 'utf8'), {
        filename: filename
      });
      return module._compile(content, filename);
    };
  } else if (require.registerExtension) {
    require.registerExtension('.coffee', function(content) {
      return compile(content);
    });
  }
  exports.VERSION = '1.1.2';
  exports.RESERVED = RESERVED;
  exports.helpers = require('./helpers');
  exports.compile = compile = function(code, options) {
    if (options == null) {
      options = {};
    }
    try {
      return (parser.parse(lexer.tokenize(code))).compile(options);
    } catch (err) {
      if (options.filename) {
        err.message = "In " + options.filename + ", " + err.message;
      }
      throw err;
    }
  };
  exports.tokens = function(code, options) {
    return lexer.tokenize(code, options);
  };
  exports.nodes = function(source, options) {
    if (typeof source === 'string') {
      return parser.parse(lexer.tokenize(source, options));
    } else {
      return parser.parse(source);
    }
  };
  exports.run = function(code, options) {
    var Module, mainModule;
    mainModule = require.main;
    mainModule.filename = process.argv[1] = options.filename ? fs.realpathSync(options.filename) : '.';
    mainModule.moduleCache && (mainModule.moduleCache = {});
    if (process.binding('natives').module) {
      Module = require('module').Module;
      mainModule.paths = Module._nodeModulePaths(path.dirname(options.filename));
    }
    if (path.extname(mainModule.filename) !== '.coffee' || require.extensions) {
      return mainModule._compile(compile(code, options), mainModule.filename);
    } else {
      return mainModule._compile(code, mainModule.filename);
    }
  };
  exports.eval = function(code, options) {
    var Module, Script, js, k, o, r, sandbox, v, _i, _len, _module, _ref2, _ref3, _ref4, _require;
    if (options == null) {
      options = {};
    }
    if (!(code = code.trim())) {
      return;
    }
    if (_ref2 = require('vm'), Script = _ref2.Script, _ref2) {
      sandbox = Script.createContext();
      sandbox.global = sandbox.root = sandbox.GLOBAL = sandbox;
      if (options.sandbox != null) {
        if (options.sandbox instanceof sandbox.constructor) {
          sandbox = options.sandbox;
        } else {
          _ref3 = options.sandbox;
          for (k in _ref3) {
            if (!__hasProp.call(_ref3, k)) continue;
            v = _ref3[k];
            sandbox[k] = v;
          }
        }
      }
      sandbox.__filename = options.filename || 'eval';
      sandbox.__dirname = path.dirname(sandbox.__filename);
      if (!(sandbox.module || sandbox.require)) {
        Module = require('module');
        sandbox.module = _module = new Module(options.modulename || 'eval');
        sandbox.require = _require = function(path) {
          return Module._load(path, _module);
        };
        _module.filename = sandbox.__filename;
        _ref4 = Object.getOwnPropertyNames(require);
        for (_i = 0, _len = _ref4.length; _i < _len; _i++) {
          r = _ref4[_i];
          _require[r] = require[r];
        }
        _require.paths = _module.paths = Module._nodeModulePaths(process.cwd());
        _require.resolve = function(request) {
          return Module._resolveFilename(request, _module);
        };
      }
    }
    o = {};
    for (k in options) {
      if (!__hasProp.call(options, k)) continue;
      v = options[k];
      o[k] = v;
    }
    o.bare = true;
    js = compile(code, o);
    if (Script) {
      return Script.runInContext(js, sandbox);
    } else {
      return eval(js);
    }
  };
  lexer = new Lexer;
  parser.lexer = {
    lex: function() {
      var tag, _ref2;
      _ref2 = this.tokens[this.pos++] || [''], tag = _ref2[0], this.yytext = _ref2[1], this.yylineno = _ref2[2];
      return tag;
    },
    setInput: function(tokens) {
      this.tokens = tokens;
      return this.pos = 0;
    },
    upcomingInput: function() {
      return "";
    }
  };
  parser.yy = require('./nodes');
}).call(this);
