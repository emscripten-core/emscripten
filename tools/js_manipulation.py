# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from . import shared


def add_files_pre_js(user_pre_js, files_pre_js):
  # the normal thing is to just combine the pre-js content
  if not shared.Settings.ASSERTIONS:
    return files_pre_js + user_pre_js

  # if a user pre-js tramples the file code's changes to Module.preRun
  # that could be confusing. show a clear error at runtime if assertions are
  # enabled
  return files_pre_js + '''
    // All the pre-js content up to here must remain later on, we need to run
    // it.
    var necessaryPreJSTasks = Module['preRun'].slice();
  ''' + user_pre_js + '''
    if (!Module['preRun']) throw 'Module.preRun should exist because file support used it; did a pre-js delete it?';
    necessaryPreJSTasks.forEach(function(task) {
      if (Module['preRun'].indexOf(task) < 0) throw 'All preRun tasks that exist before user pre-js code should remain after; did you replace Module or modify Module.preRun?';
    });
  '''
