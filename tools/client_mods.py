# Copyright 2015 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

class PreciseF32(object):
  name = 'PRECISE_F32 == 2'

  @staticmethod
  def get(settings, minified):
    if settings.PRECISE_F32 == 2:
      # Potentially-modifiable code, load as text, modify, then execute. This lets you
      # patch the code on the client machine right before it is executed, perhaps based
      # on information about the client.
      mod = '''
console.log('optimizing out Math.fround calls');
code = code.replace("'use asm'", "'almost asm'").replace('"use asm"', '"almost asm"');
'''
      if not minified:
        # simple dumb replace
        mod += "code = code.replace(/Math_fround\(/g, '(')\n";
      else:
        # minified, not quite so simple
        mod += '''
try {
  console.log('optimizing out Math.fround calls');
  var m = /var ([^=]+)=global\.Math\.fround;/.exec(code);
  var minified = m[1];
  if (!minified) throw 'fail';

  // The minified JS variable for Math.fround might contain the '$' sign, so this must be escaped to \$ to be used as a search pattern.
  minified = minified.replace(/\$/g, "\\\\$$");

  do {
    var moar = false; // we need to re-do, as x(x( will not be fixed
    code = code.replace(new RegExp('[^a-zA-Z0-9\\\\$\\\\_]' + minified + '\\\\(', 'g'), function(s) { moar = true; return s[0] + '(' });
  } while (moar);
} catch(e) { console.log('failed to optimize out Math.fround calls ' + e) }
'''
      return ['if (!Math.fround) { ' + mod + ' }']
    return []

# Handlers

handlers = [PreciseF32]

# client-side asm code modification

def get_mods(settings, minified, separate_asm):
  ret = []
  for handler in handlers:
    curr = handler.get(settings, minified)
    if curr:
      assert separate_asm, 'options that modify code on the client, like ' + handler.name + ', require --separate-asm'
      ret = ret + curr
  return ret
