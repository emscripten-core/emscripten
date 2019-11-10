# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Performs a search-replace in all of js/
'''

import os

def all_children(subdir):
  return [os.path.join(dp, f) for dp, dn, fn in os.walk(subdir) for f in fn]

for x in all_children('src') + all_children('tests') + all_children('tools') + all_children('system/lib') + ['emscripten.py', 'emcc.py']:
  if 'update_js.py' in x:
    continue
  if not (x.endswith('.py') or x.endswith('.c') or x.endswith('.cpp') or x.endswith('.h') or x.endswith('.js') or x.endswith('.ll')):
    continue
  print x
  orig = open(x).read()
  fixed = orig[:]
  fixed = fixed.replace('Module["print"](', 'out(')
  fixed = fixed.replace('Module[\'print\'](', 'out(')
  fixed = fixed.replace('Module.print(', 'out(')

  fixed = fixed.replace('Module["printErr"](', 'err(')
  fixed = fixed.replace('Module[\'printErr\'](', 'err(')
  fixed = fixed.replace('Module.printErr(', 'err(')

  fixed = fixed.replace(' = Module["print"]', ' = out')
  fixed = fixed.replace(' = Module[\'print\']', ' = out')
  fixed = fixed.replace('Module["print"] = ', 'out = ')
  fixed = fixed.replace('Module[\'print\'] = ', 'out = ')
  fixed = fixed.replace(' = Module.print', ' = out')
  fixed = fixed.replace('Module.print = ', 'out = ')

  fixed = fixed.replace(' = Module["printErr"]', ' = err')
  fixed = fixed.replace(' = Module[\'printErr\']', ' = err')
  fixed = fixed.replace('Module["printErr"] = ', 'err = ')
  fixed = fixed.replace('Module[\'printErr\'] = ', 'err = ')
  fixed = fixed.replace(' = Module.printErr', ' = err')
  fixed = fixed.replace('Module.printErr = ', 'err = ')

  if fixed != orig:
    open(x, 'w').write(fixed)

