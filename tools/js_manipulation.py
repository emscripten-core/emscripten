# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import re

from .settings import settings
from . import utils, shared

emscripten_license = '''\
/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */
'''

# handle the above form, and also what closure can emit which is stuff like
#  /*
#
#   Copyright 2019 The Emscripten Authors
#   SPDX-License-Identifier: MIT
#
#   Copyright 2017 The Emscripten Authors
#   SPDX-License-Identifier: MIT
#  */
emscripten_license_regex = r'\/\*\*?(\s*\*?\s*@license)?(\s*\*?\s*Copyright \d+ The Emscripten Authors\s*\*?\s*SPDX-License-Identifier: MIT)+\s*\*\/\s*'


def add_files_pre_js(pre_js_list, files_pre_js):
  # the normal thing is to just combine the pre-js content
  filename = shared.get_temp_files().get('.js').name
  utils.write_file(filename, files_pre_js)
  pre_js_list.insert(0, filename)
  if not settings.ASSERTIONS:
    return

  # if a user pre-js tramples the file code's changes to Module.preRun
  # that could be confusing. show a clear error at runtime if assertions are
  # enabled
  pre = shared.get_temp_files().get('.js').name
  post = shared.get_temp_files().get('.js').name
  utils.write_file(pre, '''
    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['$ww'] || (typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD)) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  ''')
  utils.write_file(post, '''
    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach((task) => {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  ''')

  pre_js_list.insert(1, pre)
  pre_js_list.append(post)


def handle_license(js_target):
  # ensure we emit the license if and only if we need to, and exactly once
  js = utils.read_file(js_target)
  # first, remove the license as there may be more than once
  processed_js = re.sub(emscripten_license_regex, '', js)
  if settings.EMIT_EMSCRIPTEN_LICENSE:
    processed_js = emscripten_license + processed_js
  if processed_js != js:
    utils.write_file(js_target, processed_js)


# Returns the given string with escapes added so that it can safely be placed inside a string in JS code.
def escape_for_js_string(s):
  s = s.replace('\\', '/').replace("'", "\\'").replace('"', '\\"')
  return s


def legalize_sig(sig):
  # with BigInt support all sigs are legal since we can use i64s.
  if settings.WASM_BIGINT:
    return sig
  legal = [sig[0]]
  # a return of i64 is legalized into an i32 (and the high bits are
  # accessible on the side through getTempRet0).
  if legal[0] == 'j':
    legal[0] = 'i'
  # a parameter of i64 is legalized into i32, i32
  for s in sig[1:]:
    if s != 'j':
      legal.append(s)
    else:
      legal.append('i')
      legal.append('i')
  return ''.join(legal)


def is_legal_sig(sig):
  # with BigInt support all sigs are legal since we can use i64s.
  if settings.WASM_BIGINT:
    return True
  return sig == legalize_sig(sig)


def isidentifier(name):
  # https://stackoverflow.com/questions/43244604/check-that-a-string-is-a-valid-javascript-identifier-name-using-python-3
  return name.replace('$', '_').isidentifier()


def make_dynCall(sig, args):
  # wasm2c and asyncify are not yet compatible with direct wasm table calls
  if settings.MEMORY64:
    args = list(args)
    args[0] = f'Number({args[0]})'
  if settings.DYNCALLS or not is_legal_sig(sig):
    args = ','.join(args)
    if not settings.MAIN_MODULE and not settings.SIDE_MODULE:
      # Optimize dynCall accesses in the case when not building with dynamic
      # linking enabled.
      return 'dynCall_%s(%s)' % (sig, args)
    else:
      return 'Module["dynCall_%s"](%s)' % (sig, args)
  else:
    call_args = ",".join(args[1:])
    return f'getWasmTableEntry({args[0]})({call_args})'


def make_invoke(sig):
  legal_sig = legalize_sig(sig) # TODO: do this in extcall, jscall?
  args = ['index'] + ['a' + str(i) for i in range(1, len(legal_sig))]
  ret = 'return ' if sig[0] != 'v' else ''
  # For function that needs to return a genuine i64 (i.e. if legal_sig[0] is 'j')
  # we need to return an actual BigInt, even in the exceptional case because
  # wasm won't implicitly convert undefined to 0 in this case.
  exceptional_ret = '\n    return 0n;' if legal_sig[0] == 'j' else ''
  body = '%s%s;' % (ret, make_dynCall(sig, args))
  # Create a try-catch guard that rethrows the Emscripten EH exception.
  if settings.EXCEPTION_STACK_TRACES:
    # Exceptions thrown from C++ and longjmps will be an instance of
    # EmscriptenEH.
    maybe_rethrow = 'if (!(e instanceof EmscriptenEH)) throw e;'
  else:
    # Exceptions thrown from C++ will be a pointer (number) and longjmp will
    # throw the number Infinity. Use the compact and fast "e !== e+0" test to
    # check if e was not a Number.
    maybe_rethrow = 'if (e !== e+0) throw e;'

  ret = '''\
function invoke_%s(%s) {
  var sp = stackSave();
  try {
    %s
  } catch(e) {
    stackRestore(sp);
    %s
    _setThrew(1, 0);%s
  }
}''' % (sig, ','.join(args), body, maybe_rethrow, exceptional_ret)

  return ret


def make_wasm64_wrapper(sig):
  assert 'p' in sig.lower()
  n_args = len(sig) - 1
  args = ['a%d' % i for i in range(n_args)]
  args_converted = args.copy()
  for i, arg_type in enumerate(sig[1:]):
    if arg_type == 'p':
      args_converted[i] = f'BigInt({args_converted[i]})'
    elif arg_type == 'P':
      args_converted[i] = f'BigInt({args_converted[i]} ? {args_converted[i]} : 0)'
    else:
      assert arg_type == '_'

  args_in = ', '.join(args)
  args_out = ', '.join(args_converted)
  result = f'f({args_out})'
  if sig[0] == 'p':
    result = f'Number({result})'

  # We can't use an arrow function for the inner wrapper here since there
  # are certain places we need to avoid strict mode still.
  # e.g. emscripten_get_callstack (getCallstack) which uses the `arguments`
  # global.
  return f'  var makeWrapper_{sig} = (f) => ({args_in}) => {result};\n'


def make_unsign_pointer_wrapper(sig):
  assert sig[0] == 'p'
  n_args = len(sig) - 1
  args = ','.join('a%d' % i for i in range(n_args))
  return f'  var makeWrapper_{sig} = (f) => ({args}) => f({args}) >>> 0;\n'
