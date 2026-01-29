import logging
import os
import re
import sys

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from . import building, shared, utils
from .settings import settings

logger = logging.getLogger('minimal_runtime_shell')


def generate_minimal_runtime_load_statement(target_basename):
  # Extra code to appear before the loader
  prefix_statements = []
  # Statements to appear inside a Promise .then() block after loading has finished
  then_statements = []
  # Import parameters to call the main JS runtime function with
  modularize_imports = []

  # Depending on whether streaming Wasm compilation is enabled or not, the minimal sized code to
  # download Wasm looks a bit different.
  # Expand {{{ DOWNLOAD_WASM }}} block from here (if we added #define support, this could be done in
  # the template directly)
  if settings.MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION:
    if settings.MIN_SAFARI_VERSION < 150000 or settings.MIN_NODE_VERSION < 180100 or settings.MIN_FIREFOX_VERSION < 58:
      # https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/compileStreaming
      download_wasm = f"WebAssembly.compileStreaming ? WebAssembly.compileStreaming(fetch('{target_basename}.wasm')) : binary('{target_basename}.wasm')"
    else:
      # WebAssembly.compileStreaming() is unconditionally supported:
      download_wasm = f"WebAssembly.compileStreaming(fetch('{target_basename}.wasm'))"
  elif settings.MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION:
    # Same compatibility story as above for
    # https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
    if settings.MIN_SAFARI_VERSION < 150000 or settings.MIN_NODE_VERSION < 180100 or settings.MIN_FIREFOX_VERSION < 58:
      download_wasm = f"!WebAssembly.instantiateStreaming && binary('{target_basename}.wasm')"
    else:
      # WebAssembly.instantiateStreaming() is unconditionally supported, so we do not download wasm
      # in the .html file, but leave it to the .js file to download
      download_wasm = None
  else:
    download_wasm = f"binary('{target_basename}.wasm')"

  # Main JS file always in first entry
  files_to_load = [f"script('{settings.TARGET_JS_NAME}')"]

  # Download .wasm file
  if (settings.WASM == 1 and settings.WASM2JS == 0) or not download_wasm:
    if settings.MODULARIZE:
      modularize_imports += [f'wasm: r[{len(files_to_load)}]']
    else:
      then_statements += [f"{settings.EXPORT_NAME}.wasm = r[{len(files_to_load)}];"]
    if download_wasm:
      files_to_load += [download_wasm]

  # Download wasm_worker file
  if settings.WASM_WORKERS and settings.MODULARIZE:
    modularize_imports += ['js: js']

  # Download Wasm2JS code if target browser does not support WebAssembly
  if settings.WASM == 2:
    if settings.MODULARIZE:
      modularize_imports += [f'wasm: supportsWasm ? r[{len(files_to_load)}] : 0']
    else:
      then_statements += [f"if (supportsWasm) {settings.EXPORT_NAME}.wasm = r[{len(files_to_load)}];"]
    files_to_load += [f"supportsWasm ? {download_wasm} : script('{target_basename}.wasm.js')"]

  # Execute compiled output when building with MODULARIZE
  if settings.MODULARIZE:
    modularize_imports = ',\n  '.join(modularize_imports)
    if settings.WASM_WORKERS:
      then_statements += ['''\
  // Detour the JS code to a separate variable to avoid instantiating with 'r' array as "this"
  // directly to avoid strict ECMAScript/Firefox GC problems that cause a leak, see
  // https://bugzil.la/1540101
  var js = URL.createObjectURL(new Blob([r[0]], { type: \'application/javascript\' }));
  script(js).then((c) => c({
  %s
  }));''' % modularize_imports]
    else:
      then_statements += ['''\
  // Detour the JS code to a separate variable to avoid instantiating with 'r' array as "this"
  // directly to avoid strict ECMAScript/Firefox GC problems that cause a leak, see
  // https://bugzil.la/1540101
  var js = r[0];
  js({
  %s
  });''' % modularize_imports]

  binary_xhr = '  var binary = (url) => fetch(url).then((rsp) => rsp.arrayBuffer());'

  script_xhr = '''\
  function script(url) { // Downloads a script file and adds it to DOM
    return new Promise((ok) => {
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
    if settings.MODULARIZE:
      return script_xhr + files_to_load[0] + ".then(js);"
    else:
      return script_xhr + files_to_load[0] + ";"

  if not settings.MODULARIZE or settings.WASM_WORKERS:
    # If downloading multiple files like .wasm or .mem, those need to be loaded in
    # before we can add the main runtime script to the DOM, so convert the main .js
    # script load from direct script() load to a binary() load so we can still
    # immediately start the download, but can control when we add the script to the
    # DOM.
    if settings.PTHREADS or settings.WASM_WORKERS:
      script_load = "script(url)"
    else:
      script_load = "script(url).then(() => URL.revokeObjectURL(url));"

    if settings.WASM_WORKERS:
      save_js = f'{settings.EXPORT_NAME}.js = '
    else:
      save_js = ''

    files_to_load[0] = f"binary('{settings.TARGET_JS_NAME}')"
    if not settings.MODULARIZE:
      then_statements += ["var url = %sURL.createObjectURL(new Blob([r[0]], { type: 'application/javascript' }));" % save_js,
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
  shell = utils.read_file(options.shell_html)
  if settings.SINGLE_FILE:
    # No extra files needed to download in a SINGLE_FILE build.
    shell = shell.replace('{{{ DOWNLOAD_JS_AND_WASM_FILES }}}', '')
  else:
    shell = shell.replace('{{{ DOWNLOAD_JS_AND_WASM_FILES }}}', generate_minimal_runtime_load_statement(target_basename))

  temp_files = shared.get_temp_files()
  with temp_files.get_file(suffix='.js') as shell_temp:
    utils.write_file(shell_temp, shell)
    shell = building.read_and_preprocess(shell_temp)

  if re.search(r'{{{\s*SCRIPT\s*}}}', shell):
    utils.exit_with_error(f'--shell-file "{options.shell_html}": MINIMAL_RUNTIME uses a different kind of HTML page shell file than the traditional runtime! Please see $EMSCRIPTEN/html/shell_minimal_runtime.html for a template to use as a basis.')

  shell = shell.replace('{{{ TARGET_BASENAME }}}', settings.TARGET_BASENAME)
  shell = shell.replace('{{{ EXPORT_NAME }}}', settings.EXPORT_NAME)
  shell = shell.replace('{{{ TARGET_JS_NAME }}}', settings.TARGET_JS_NAME)

  # In SINGLE_FILE build, embed the main .js file into the .html output
  if settings.SINGLE_FILE:
    js_contents = utils.read_file(js_target)
    utils.delete_file(js_target)
  else:
    js_contents = ''
  shell = shell.replace('{{{ JS_CONTENTS_IN_SINGLE_FILE_BUILD }}}', js_contents)
  utils.write_file(target, shell, options.output_eol)
