# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import re

from .settings import settings
from . import utils

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
emscripten_license_regex = r'\/\*\*?(\s*\*?\s*@license)?(\s*\*?\s*Copyright \d+ The Emscripten Authors\s*\*?\s*SPDX-License-Identifier: MIT)+\s*\*\/'


def add_files_pre_js(user_pre_js, files_pre_js):
  # the normal thing is to just combine the pre-js content
  if not settings.ASSERTIONS:
    return files_pre_js + user_pre_js

  # if a user pre-js tramples the file code's changes to Module.preRun
  # that could be confusing. show a clear error at runtime if assertions are
  # enabled
  return files_pre_js + '''
    // All the pre-js content up to here must remain later on, we need to run
    // it.
    if (Module['ENVIRONMENT_IS_PTHREAD']) Module['preRun'] = [];
    var necessaryPreJSTasks = Module['preRun'].slice();
  ''' + user_pre_js + '''
    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach(function(task) {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  '''


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
  if settings.DYNCALLS or not is_legal_sig(sig):
    args = ','.join(args)
    if not settings.MAIN_MODULE and not settings.SIDE_MODULE:
      # Optimize dynCall accesses in the case when not building with dynamic
      # linking enabled.
      return 'dynCall_%s(%s)' % (sig, args)
    else:
      return 'Module["dynCall_%s"](%s)' % (sig, args)
  else:
    return 'getWasmTableEntry(%s)(%s)' % (args[0], ','.join(args[1:]))


def make_invoke(sig, named=True):
  legal_sig = legalize_sig(sig) # TODO: do this in extcall, jscall?
  args = ['index'] + ['a' + str(i) for i in range(1, len(legal_sig))]
  ret = 'return ' if sig[0] != 'v' else ''
  body = '%s%s;' % (ret, make_dynCall(sig, args))
  # C++ exceptions are numbers, and longjmp is a string 'longjmp'
  if settings.SUPPORT_LONGJMP:
    rethrow = "if (e !== e+0 && e !== 'longjmp') throw e;"
  else:
    rethrow = "if (e !== e+0) throw e;"

  name = (' invoke_' + sig) if named else ''
  ret = '''\
function%s(%s) {
var sp = stackSave();
try {
  %s
} catch(e) {
  stackRestore(sp);
  %s
  _setThrew(1, 0);
}
}''' % (name, ','.join(args), body, rethrow)

  return ret
