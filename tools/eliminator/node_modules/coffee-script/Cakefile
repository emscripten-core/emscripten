fs            = require 'fs'
path          = require 'path'
{extend}      = require './lib/coffee-script/helpers'
CoffeeScript  = require './lib/coffee-script'
{spawn, exec} = require 'child_process'

# ANSI Terminal Colors.
enableColors = no
unless process.platform is 'win32'
  enableColors = not process.env.NODE_DISABLE_COLORS

bold = red = green = reset = ''
if enableColors
  bold  = '\x1B[0;1m'
  red   = '\x1B[0;31m'
  green = '\x1B[0;32m'
  reset = '\x1B[0m'

# Built file header.
header = """
  /**
   * CoffeeScript Compiler v#{CoffeeScript.VERSION}
   * http://coffeescript.org
   *
   * Copyright 2011, Jeremy Ashkenas
   * Released under the MIT License
   */
"""

sources = [
  'coffee-script', 'grammar', 'helpers'
  'lexer', 'nodes', 'rewriter', 'scope'
].map (filename) -> "src/#{filename}.coffee"

# Run a CoffeeScript through our node/coffee interpreter.
run = (args, cb) ->
  proc =         spawn 'node', ['bin/coffee'].concat(args)
  proc.stderr.on 'data', (buffer) -> console.log buffer.toString()
  proc.on        'exit', (status) ->
    process.exit(1) if status != 0
    cb() if typeof cb is 'function'

# Log a message with a color.
log = (message, color, explanation) ->
  console.log color + message + reset + ' ' + (explanation or '')

option '-p', '--prefix [DIR]', 'set the installation prefix for `cake install`'

task 'install', 'install CoffeeScript into /usr/local (or --prefix)', (options) ->
  base = options.prefix or '/usr/local'
  lib  = "#{base}/lib/coffee-script"
  bin  = "#{base}/bin"
  node = "~/.node_libraries/coffee-script"
  console.log   "Installing CoffeeScript to #{lib}"
  console.log   "Linking to #{node}"
  console.log   "Linking 'coffee' to #{bin}/coffee"
  exec([
    "mkdir -p #{lib} #{bin}"
    "cp -rf bin lib LICENSE README package.json src #{lib}"
    "ln -sfn #{lib}/bin/coffee #{bin}/coffee"
    "ln -sfn #{lib}/bin/cake #{bin}/cake"
    "mkdir -p ~/.node_libraries"
    "ln -sfn #{lib}/lib/coffee-script #{node}"
  ].join(' && '), (err, stdout, stderr) ->
    if err then console.log stderr.trim() else log 'done', green
  )


task 'build', 'build the CoffeeScript language from source', build = (cb) ->
  files = fs.readdirSync 'src'
  files = ('src/' + file for file in files when file.match(/\.coffee$/))
  run ['-c', '-o', 'lib/coffee-script'].concat(files), cb


task 'build:full', 'rebuild the source twice, and run the tests', ->
  build ->
    build ->
      csPath = './lib/coffee-script'
      delete require.cache[require.resolve csPath]
      unless runTests require csPath
        process.exit 1


task 'build:parser', 'rebuild the Jison parser (run build first)', ->
  extend global, require('util')
  require 'jison'
  parser = require('./lib/coffee-script/grammar').parser
  fs.writeFile 'lib/coffee-script/parser.js', parser.generate()


task 'build:ultraviolet', 'build and install the Ultraviolet syntax highlighter', ->
  exec 'plist2syntax ../coffee-script-tmbundle/Syntaxes/CoffeeScript.tmLanguage', (err) ->
    throw err if err
    exec 'sudo mv coffeescript.yaml /usr/local/lib/ruby/gems/1.8/gems/ultraviolet-0.10.2/syntax/coffeescript.syntax'


task 'build:browser', 'rebuild the merged script for inclusion in the browser', ->
  code = ''
  for name in ['helpers', 'rewriter', 'lexer', 'parser', 'scope', 'nodes', 'coffee-script', 'browser']
    code += """
      require['./#{name}'] = new function() {
        var exports = this;
        #{fs.readFileSync "lib/coffee-script/#{name}.js"}
      };
    """
  code = """
    (function(root) {
      var CoffeeScript = function() {
        function require(path){ return require[path]; }
        #{code}
        return require['./coffee-script'];
      }();

      if (typeof define === 'function' && define.amd) {
        define(function() { return CoffeeScript; });
      } else { 
        root.CoffeeScript = CoffeeScript; 
      }
    }(this));
  """
  unless process.env.MINIFY is 'false'
    {parser, uglify} = require 'uglify-js'
    code = uglify.gen_code uglify.ast_squeeze uglify.ast_mangle parser.parse code
  fs.writeFileSync 'extras/coffee-script.js', header + '\n' + code
  console.log "built ... running browser tests:"
  invoke 'test:browser'


task 'doc:site', 'watch and continually rebuild the documentation for the website', ->
  exec 'rake doc', (err) ->
    throw err if err


task 'doc:source', 'rebuild the internal documentation', ->
  exec 'docco src/*.coffee && cp -rf docs documentation && rm -r docs', (err) ->
    throw err if err


task 'doc:underscore', 'rebuild the Underscore.coffee documentation page', ->
  exec 'docco examples/underscore.coffee && cp -rf docs documentation && rm -r docs', (err) ->
    throw err if err

task 'bench', 'quick benchmark of compilation time', ->
  {Rewriter} = require './lib/coffee-script/rewriter'
  co     = sources.map((name) -> fs.readFileSync name).join '\n'
  fmt    = (ms) -> " #{bold}#{ "   #{ms}".slice -4 }#{reset} ms"
  total  = 0
  now    = Date.now()
  time   = -> total += ms = -(now - now = Date.now()); fmt ms
  tokens = CoffeeScript.tokens co, rewrite: false
  console.log "Lex    #{time()} (#{tokens.length} tokens)"
  tokens = new Rewriter().rewrite tokens
  console.log "Rewrite#{time()} (#{tokens.length} tokens)"
  nodes  = CoffeeScript.nodes tokens
  console.log "Parse  #{time()}"
  js     = nodes.compile bare: true
  console.log "Compile#{time()} (#{js.length} chars)"
  console.log "total  #{ fmt total }"

task 'loc', 'count the lines of source code in the CoffeeScript compiler', ->
  exec "cat #{ sources.join(' ') } | grep -v '^\\( *#\\|\\s*$\\)' | wc -l | tr -s ' '", (err, stdout) ->
    console.log stdout.trim()


# Run the CoffeeScript test suite.
runTests = (CoffeeScript) ->
  startTime   = Date.now()
  currentFile = null
  passedTests = 0
  failures    = []

  global[name] = func for name, func of require 'assert'

  # Convenience aliases.
  global.CoffeeScript = CoffeeScript

  # Our test helper function for delimiting different test cases.
  global.test = (description, fn) ->
    try
      fn.test = {description, currentFile}
      fn.call(fn)
      ++passedTests
    catch e
      e.description = description if description?
      e.source      = fn.toString() if fn.toString?
      failures.push filename: currentFile, error: e

  # See http://wiki.ecmascript.org/doku.php?id=harmony:egal
  egal = (a, b) ->
    if a is b
      a isnt 0 or 1/a is 1/b
    else
      a isnt a and b isnt b

  # A recursive functional equivalence helper; uses egal for testing equivalence.
  arrayEgal = (a, b) ->
    if egal a, b then yes
    else if a instanceof Array and b instanceof Array
      return no unless a.length is b.length
      return no for el, idx in a when not arrayEgal el, b[idx]
      yes

  global.eq      = (a, b, msg) -> ok egal(a, b), msg
  global.arrayEq = (a, b, msg) -> ok arrayEgal(a,b), msg

  # When all the tests have run, collect and print errors.
  # If a stacktrace is available, output the compiled function source.
  process.on 'exit', ->
    time = ((Date.now() - startTime) / 1000).toFixed(2)
    message = "passed #{passedTests} tests in #{time} seconds#{reset}"
    return log(message, green) unless failures.length
    log "failed #{failures.length} and #{message}", red
    for fail in failures
      {error, filename}  = fail
      jsFilename         = filename.replace(/\.coffee$/,'.js')
      match              = error.stack?.match(new RegExp(fail.file+":(\\d+):(\\d+)"))
      match              = error.stack?.match(/on line (\d+):/) unless match
      [match, line, col] = match if match
      console.log ''
      log "  #{error.description}", red if error.description
      log "  #{error.stack}", red
      log "  #{jsFilename}: line #{line ? 'unknown'}, column #{col ? 'unknown'}", red
      console.log "  #{error.source}" if error.source
    return

  # Run every test in the `test` folder, recording failures.
  files = fs.readdirSync 'test'
  for file in files when file.match /\.coffee$/i
    currentFile = filename = path.join 'test', file
    code = fs.readFileSync filename
    try
      CoffeeScript.run code.toString(), {filename}
    catch error
      failures.push {filename, error}
  return !failures.length


task 'test', 'run the CoffeeScript language test suite', ->
  runTests CoffeeScript


task 'test:browser', 'run the test suite against the merged browser script', ->
  source = fs.readFileSync 'extras/coffee-script.js', 'utf-8'
  result = {}
  global.testingBrowser = yes
  (-> eval source).call result
  runTests result.CoffeeScript
