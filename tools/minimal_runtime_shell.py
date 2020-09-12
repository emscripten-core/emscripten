import re
import sys
import os
import logging

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools import shared
from tools import line_endings

logger = logging.getLogger('minimal_runtime_shell')


def generate_minimal_runtime_load_statement(target_basename):
  prefix_statements = [] # Extra code to appear before the loader
  then_statements = [] # Statements to appear inside a Promise .then() block after loading has finished
  modularize_imports = [] # Import parameters to call the main JS runtime function with

  # Depending on whether streaming Wasm compilation is enabled or not, the minimal sized code to download Wasm looks a bit different.
  # Expand {{{ DOWNLOAD_WASM }}} block from here (if we added #define support, this could be done in the template directly)
  if shared.Settings.MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION:
    if shared.Settings.MIN_SAFARI_VERSION != shared.Settings.TARGET_NOT_SUPPORTED or shared.Settings.ENVIRONMENT_MAY_BE_NODE or shared.Settings.MIN_FIREFOX_VERSION < 58 or shared.Settings.MIN_CHROME_VERSION < 61:
      # Firefox 52 added Wasm support, but only Firefox 58 added compileStreaming.
      # Chrome 57 added Wasm support, but only Chrome 61 added compileStreaming.
      # https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/compileStreaming
      # In Safari and Node.js, WebAssembly.compileStreaming() is not supported, in which case fall back to regular download.
      download_wasm = "WebAssembly.compileStreaming ? WebAssembly.compileStreaming(fetch('%s')) : binary('%s')" % (target_basename + '.wasm', target_basename + '.wasm')
    else:
      # WebAssembly.compileStreaming() is unconditionally supported:
      download_wasm = "WebAssembly.compileStreaming(fetch('%s'))" % (target_basename + '.wasm')
  elif shared.Settings.MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION:
    # Same compatibility story as above for https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
    if shared.Settings.MIN_SAFARI_VERSION != shared.Settings.TARGET_NOT_SUPPORTED or shared.Settings.ENVIRONMENT_MAY_BE_NODE or shared.Settings.MIN_FIREFOX_VERSION < 58 or shared.Settings.MIN_CHROME_VERSION < 61:
      download_wasm = "!WebAssembly.instantiateStreaming && binary('%s')" % (target_basename + '.wasm')
    else:
      # WebAssembly.instantiateStreaming() is unconditionally supported, so we do not download wasm in the .html file,
      # but leave it to the .js file to download
      download_wasm = None
  else:
    download_wasm = "binary('%s')" % (target_basename + '.wasm')

  files_to_load = ["script('%s')" % (target_basename + '.js')] # Main JS file always in first entry

  # Download separate memory initializer file .mem
  if shared.Settings.MEM_INIT_METHOD == 1 and not shared.Settings.MEM_INIT_IN_WASM:
    if shared.Settings.MODULARIZE:
      modularize_imports += ['mem: r[%d]' % len(files_to_load)]
    else:
      then_statements += ["Module.mem = r[%d];" % len(files_to_load)]
    files_to_load += ["binary('%s')" % (target_basename + '.mem')]

  # Download .wasm file
  if shared.Settings.WASM == 1 or not download_wasm:
    if shared.Settings.MODULARIZE:
      modularize_imports += ['wasm: r[%d]' % len(files_to_load)]
    else:
      then_statements += ["Module.wasm = r[%d];" % len(files_to_load)]
    if download_wasm:
      files_to_load += [download_wasm]

  if shared.Settings.MODULARIZE and shared.Settings.USE_PTHREADS:
    modularize_imports += ["worker: '{{{ PTHREAD_WORKER_FILE }}}'"]

  # Download Wasm2JS code if target browser does not support WebAssembly
  if shared.Settings.WASM == 2:
    if shared.Settings.MODULARIZE:
      modularize_imports += ['wasm: supportsWasm ? r[%d] : 0' % len(files_to_load)]
    else:
      then_statements += ["if (supportsWasm) Module.wasm = r[%d];" % len(files_to_load)]
    files_to_load += ["supportsWasm ? %s : script('%s')" % (download_wasm, target_basename + '.wasm.js')]

  # Execute compiled output when building with MODULARIZE
  if shared.Settings.MODULARIZE:
    then_statements += ['''// Detour the JS code to a separate variable to avoid instantiating with 'r' array as "this" directly to avoid strict ECMAScript/Firefox GC problems that cause a leak, see https://bugzilla.mozilla.org/show_bug.cgi?id=1540101
  var js = r[0];\n  js({ %s });''' % ',\n  '.join(modularize_imports)]

  binary_xhr = '''  function binary(url) { // Downloads a binary file and outputs it in the specified callback
      return new Promise((ok, err) => {
        var x = new XMLHttpRequest();
        x.open('GET', url, true);
        x.responseType = 'arraybuffer';
        x.onload = () => { ok(x.response); }
        x.send(null);
      });
    }
  '''

  script_xhr = '''  function script(url) { // Downloads a script file and adds it to DOM
    return new Promise((ok, err) => {
      var s = document.createElement('script');
      s.src = url;
      s.onload = () => {
#if MODULARIZE
#if WASM == 2
        // In MODULARIZEd WASM==2 builds, we use this same function to download
        // both .js and .asm.js that are structured with {{{ EXPORT_NAME }}}
        // at the top level, but also use this function to download the Wasm2JS
        // file that does not have an {{{ EXPORT_NAME }}} function, hence the
        // variable typeof check:
        if (typeof {{{ EXPORT_NAME }}} !== 'undefined') {
          var c = {{{ EXPORT_NAME }}};
          delete {{{ EXPORT_NAME }}};
          ok(c);
        } else {
          ok();
        }
#else
        var c = {{{ EXPORT_NAME }}};
        delete {{{ EXPORT_NAME }}};
        ok(c);
#endif
#else
        ok();
#endif
      };
      document.body.appendChild(s);
    });
  }
  '''

  # Only one file to download - no need to use Promise.all()
  if len(files_to_load) == 1:
    if shared.Settings.MODULARIZE:
      return script_xhr + files_to_load[0] + ".then((js) => {\n  js();\n});"
    else:
      return script_xhr + files_to_load[0] + ";"

  if not shared.Settings.MODULARIZE:
    # If downloading multiple files like .wasm or .mem, those need to be loaded in
    # before we can add the main runtime script to the DOM, so convert the main .js
    # script load from direct script() load to a binary() load so we can still
    # immediately start the download, but can control when we add the script to the
    # DOM.
    if shared.Settings.USE_PTHREADS:
      script_load = "script(url)"
    else:
      script_load = "script(url).then(() => { URL.revokeObjectURL(url) });"

    files_to_load[0] = "binary('%s')" % (target_basename + '.js')
    then_statements += ["var url = URL.createObjectURL(new Blob([r[0]], { type: 'application/javascript' }));",
                        script_load]

  # Add in binary() XHR loader if used:
  if any("binary(" in s for s in files_to_load + then_statements):
    prefix_statements += [binary_xhr]
  if any("script(" in s for s in files_to_load + then_statements):
    prefix_statements += [script_xhr]

  # Several files to download, go via Promise.all()
  load = '\n'.join(prefix_statements)
  load += "Promise.all([" + ', '.join(files_to_load) + "])"
  if len(then_statements) > 0:
    load += '.then((r) => {\n  %s\n});' % '\n  '.join(then_statements)
  return load


def generate_minimal_runtime_html(target, options, js_target, target_basename):
  logger.debug('generating HTML for minimal runtime')
  shell = open(options.shell_path, 'r').read()
  if shared.Settings.SINGLE_FILE:
    # No extra files needed to download in a SINGLE_FILE build.
    shell = shell.replace('{{{ DOWNLOAD_JS_AND_WASM_FILES }}}', '')
  else:
    shell = shell.replace('{{{ DOWNLOAD_JS_AND_WASM_FILES }}}', generate_minimal_runtime_load_statement(target_basename))

  temp_files = shared.configuration.get_temp_files()
  with temp_files.get_file(suffix='.js') as shell_temp:
    open(shell_temp, 'w').write(shell)
    shell = shared.read_and_preprocess(shell_temp)

  if re.search(r'{{{\s*SCRIPT\s*}}}', shell):
    shared.exit_with_error('--shell-file "' + options.shell_path + '": MINIMAL_RUNTIME uses a different kind of HTML page shell file than the traditional runtime! Please see $EMSCRIPTEN/src/shell_minimal_runtime.html for a template to use as a basis.')

  shell = shell.replace('{{{ TARGET_BASENAME }}}', target_basename)
  shell = shell.replace('{{{ EXPORT_NAME }}}', shared.Settings.EXPORT_NAME)
  shell = shell.replace('{{{ PTHREAD_WORKER_FILE }}}', shared.Settings.PTHREAD_WORKER_FILE)

  # In SINGLE_FILE build, embed the main .js file into the .html output
  if shared.Settings.SINGLE_FILE:
    js_contents = open(js_target).read()
    shared.try_delete(js_target)
  else:
    js_contents = ''
  shell = shell.replace('{{{ JS_CONTENTS_IN_SINGLE_FILE_BUILD }}}', js_contents)
  shell = line_endings.convert_line_endings(shell, '\n', options.output_eol)
  with open(target, 'wb') as f:
    f.write(shared.asbytes(shell))
