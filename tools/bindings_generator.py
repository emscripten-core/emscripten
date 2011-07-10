#!/usr/bin/env python

'''
Use CppHeaderParser to parse some C++ headers, and generate binding code for them.

Usage:
        bindings_generator.py BASENAME HEADER1 HEADER2 ... [-- "LAMBDA"]

  BASENAME is the name used for output files (with added suffixes).
  HEADER1 etc. are the C++ headers to parse

We generate the following:

  * BASENAME.c: C bindings file, with generated C wrapper functions. You will
                need to build this with your project, and make sure it compiles
                properly by adding the proper #includes etc. You can also just
                #include this file itself in one of your existing project files.

  * BASENAME.js: JavaScript bindings file, with generated JavaScript wrapper
                 objects. This is a high-level wrapping, using native JS classes.

  * LAMBDA: Optionally, provide the text of a lambda function here that will be
            used to process the header files. This lets you manually tweak them.

The C bindings file is basically a tiny C wrapper around the C++ code.
It's only purpose is to make it easy to access the C++ code in the JS
bindings, and to prevent DFE from removing the code we care about. The
JS bindings do more serious work, creating class structures in JS and
linking them to the C bindings.
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

processor = lambda line: line
if '--' in sys.argv:
  index = sys.argv.index('--')
  processor = eval(sys.argv[index+1])
  sys.argv = sys.argv[:index]

# First pass - read everything

classes = {}

all_h_name = basename + '.all.h'
all_h = open(all_h_name, 'w')

for header in sys.argv[2:]:
  all_h.write('//// ' + header + '\n')
  all_h.write(processor(open(header, 'r').read()))

all_h.close()

parsed = CppHeaderParser.CppHeader(all_h_name)
for cname, clazz in parsed.classes.iteritems():
  if len(clazz['methods']['public']) > 0: # Do not notice stub classes 
    classes[cname] = clazz

# Second pass - generate bindings
# TODO: Bind virtual functions using dynamic binding in the C binding code

funcs = {} # name -> # of copies in the original, and originalname in a copy

gen_c = open(basename + '.c', 'w')
gen_js = open(basename + '.js', 'w')

gen_c.write('extern "C" {\n')

def generate_class(generating_cname, cname, clazz):
  inherited = generating_cname != cname

  for method in clazz['methods']['public']:
    print '   ', method['name'], method

    mname = method['name']
    args = method['parameters']
    constructor = mname == cname

    if constructor and inherited: continue

    # C

    ret = (cname + ' *') if constructor else method['rtnType']
    callprefix = 'new ' if constructor else 'self->'
    typedargs = ', '.join( ([] if constructor else [cname + ' * self']) + map(lambda arg: arg['type'] + ' ' + arg['name'], args) )
    justargs = ', '.join(map(lambda arg: arg['name'], args))
    fullname = 'emscripten_bind_' + generating_cname + '__' + mname
    generating_cname_suffixed = generating_cname
    mname_suffixed = mname
    count = funcs.setdefault(fullname, 0)
    funcs[fullname] += 1

    # handle overloading
    if count > 0:
      suffix = '_' + str(count+1)
      funcs[fullname + suffix] = fullname # this should never change
      fullname += suffix
      mname_suffixed += suffix
      if constructor:
        generating_cname_suffixed += suffix

    gen_c.write('''
%s %s(%s) {
  return %s%s(%s);
}
''' % (ret, fullname, typedargs, callprefix, mname, justargs))

    # JS

    if constructor:
      dupe = type(funcs[fullname]) is str
      if not dupe:
        gen_js.write('''
function %s(%s) {
  this.ptr = _%s(%s);
}
''' % (generating_cname_suffixed, justargs, fullname, justargs))
      else:
        gen_js.write('''
function %s(%s) {
  this.ptr = _%s(%s);
}
%s.prototype = %s.prototype;
''' % (generating_cname_suffixed, justargs, fullname, justargs, generating_cname_suffixed, cname))
    else:
      gen_js.write('''
%s.prototype.%s = function(%s) {
  %s_%s(this.ptr%s);
}
''' % (generating_cname, mname_suffixed, justargs, 'return ' if ret != 'void' else '', fullname, (', ' if len(justargs) > 0 else '') + justargs))

for cname, clazz in classes.iteritems():
  generate_class(cname, cname, clazz)

  # In addition, generate all methods of parent classes. We do not inherit in JS (how would we do multiple inheritance etc.?)
  for parent in clazz['inherits']:
    generate_class(cname, parent['class'], classes[parent['class']])

# Finish up

funcs = funcs.keys()

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

