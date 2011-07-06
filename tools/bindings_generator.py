'''
Use CppHeaderParser to parse some C++ headers, and generate binding code for them.

Usage:
        bindings_generator.py BASENAME HEADER1 HEADER2 ...

  BASENAME is the name used for output files (with added suffixes).
  HEADER1 etc. are the C++ headers to parse

We generate the following:

  * BASENAME.c: C bindings file, with generated C wrapper functions. You will
                need to build this with your project, and make sure it compiles
                properly by adding the proper #includes etc. You can also just
                #include this file itself in one of your existing project files.

  * BASENAME.js: JavaScript bindings file, with generated JavaScript wrapper
                 objects. This is a high-level wrapping, using native JS classes.
'''

import os, sys, glob

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + list(pathelems)))
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

# Find ply and CppHeaderParser
sys.path = [path_from_root('third_party', 'ply'), path_from_root('third_party', 'CppHeaderParser')] + sys.path
import CppHeaderParser

#print glob.glob(path_from_root('tests', 'bullet', 'src', 'BulletCollision', 'CollisionDispatch', '*.h'))

basename = sys.argv[1]

# First pass - read everything

classes = {}

for header in sys.argv[2:]:
  #[('tests', 'bullet', 'src', 'BulletCollision/CollisionDispatch/btCollisionWorld.h')]:
  parsed = CppHeaderParser.CppHeader(header)
  #header = CppHeaderParser.CppHeader(path_from_root('tests', 'bullet', 'src', 'btBulletDynamicsCommon.h'))
  #print header.classes.keys()
  #print dir(header.classes['btCollisionShape'])
  #print header.classes['btCollisionWorld']['methods']['public']
  for cname, clazz in parsed.classes.iteritems():
    if len(clazz['methods']['public']) > 0: # Do not notice stub classes 
      print 'Seen', cname
      classes[cname] = clazz

# Second pass - generate bindings

funcs = []

gen_c = open(basename + '.c', 'w')
gen_js = open(basename + '.js', 'w')

gen_c.write('extern "C" {\n')

for cname, clazz in classes.iteritems():
  print 'Generating', cname
  # TODO: Generate all parent class (recursively) data too

  constructor_counter = 0

  for method in clazz['methods']['public']:
    #print '   ', method['name'], method

    mname = method['name']
    args = method['parameters']
    constructor = mname == cname

    # C

    ret = (cname + ' *') if constructor else method['rtnType']
    callprefix = 'new ' if constructor else 'self->'
    typedargs = ', '.join( ([] if constructor else [cname + ' * self']) + map(lambda arg: arg['type'] + ' ' + arg['name'], args) )
    justargs = ', '.join(map(lambda arg: arg['name'], args))
    fullname = cname + '__' + mname

    gen_c.write('''
%s emscripten_bind_%s(%s) {
  return %s%s(%s);
}
''' % (ret, fullname, typedargs, callprefix, mname, justargs))

    funcs.append('emscripten_bind_' + fullname)

    # JS

    if constructor:
      gen_js.write('''
function %s(%s) {
  this.ptr = _emscripten_bind_%s(%s);
}
''' % (cname + (str(constructor_counter) if constructor_counter > 0 else ''), justargs, fullname, justargs))
      constructor_counter += 1
    else: # TODO: handle case of multiple constructors
      gen_js.write('''
%s.prototype.%s = function(%s) {
  %s_emscripten_bind_%s(this.ptr%s);
}
''' % (cname, mname, justargs, 'return ' if ret != 'void' else '', fullname, (', ' if len(justargs) > 0 else '') + justargs))


# Finish up

gen_c.write('''
}

#include <stdio.h>

struct EmscriptenEnsurer
{
  EmscriptenEnsurer() {
    // Actually use the binding functions, so DFE will not eliminate them
    int sum = 0;
    void *seen = (void*)%s;
''' % funcs[0])

for func in funcs[1:]:
  gen_c.write('''    sum += (void*)%s == seen;
''' % func)

gen_c.write('''    printf("(%d)\\n", sum);
  }
};

EmscriptenEnsurer emscriptenEnsurer;
''')

gen_c.close()
gen_js.close()

