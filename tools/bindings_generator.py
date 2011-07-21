#!/usr/bin/env python

'''
Use CppHeaderParser to parse some C++ headers, and generate binding code for them.

Usage:
        bindings_generator.py BASENAME HEADER1 HEADER2 ... [-- "LAMBDA" ["IGNORED"]]

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

  * IGNORED: Optionally, a list of classes and class::methods not to generate code for.
             Comma separated.

The C bindings file is basically a tiny C wrapper around the C++ code.
It's only purpose is to make it easy to access the C++ code in the JS
bindings, and to prevent DFE from removing the code we care about. The
JS bindings do more serious work, creating class structures in JS and
linking them to the C bindings.
'''

import os, sys, glob, re

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
ignored = []

if '--' in sys.argv:
  index = sys.argv.index('--')
  processor = eval(sys.argv[index+1])
  if len(sys.argv) > index+2:
    ignored = sys.argv[index+2].split(',')
  sys.argv = sys.argv[:index]

# First pass - read everything

classes = {}
struct_parents = {}

all_h_name = basename + '.all.h'
all_h = open(all_h_name, 'w')

for header in sys.argv[2:]:
  all_h.write('//// ' + header + '\n')
  all_h.write(processor(open(header, 'r').read()))

all_h.close()

parsed = CppHeaderParser.CppHeader(all_h_name)
for cname, clazz in parsed.classes.iteritems():
  print 'zz see', cname
  if len(clazz['methods']['public']) > 0: # Do not notice stub classes 
    print 'zz for real', cname, clazz, dir(clazz)
    classes[cname] = clazz
    for sname, struct in clazz._public_structs.iteritems():
      struct_parents[sname] = cname
      #print 'zz seen struct %s in %s' % (sname, cname)

    # Add shadow versions of functions with default parameters
    for method in clazz['methods']['public'][:]:
      constructor = method['name'] == cname
      method['constructor'] = constructor # work around cppheaderparser issue
      args = method['parameters']
      for i in range(len(args)):
        if args[i].get('default'):
          method['parameters'][i]['default'] = None
          for j in range(i, len(args)):
            method['parameters'][j]['default'] = None
            print 'zz add version with default params', cname, method['name'], j, constructor
            clazz['methods']['public'].append({
              'name': method['name'] + '__' + str(j+1),
              'truename': method['name'],
              'parameters': args[:j+1],
              'pure_virtual': False,
              'constructor': constructor,
              'destructor': False,
              'rtnType': method['rtnType'],
            })
          method['parameters'] = method['parameters'][:i]

# Second pass - generate bindings
# TODO: Bind virtual functions using dynamic binding in the C binding code

funcs = {} # name -> # of copies in the original, and originalname in a copy

gen_c = open(basename + '.c', 'w')
gen_js = open(basename + '.js', 'w')

gen_c.write('extern "C" {\n')

def generate_class(generating_cname, cname, clazz):
  inherited = generating_cname != cname

  for method in clazz['methods']['public']:
    mname = method['name']
    if cname + '::' + mname in ignored: continue

    args = method['parameters']
    constructor = method['constructor'] # we fixed this before
    destructor = method['destructor']

    print "zz generating: ", generating_cname, cname, mname, constructor

    if destructor: continue
    if constructor and inherited: continue

    skip = False
    for i in range(len(args)):
      #print 'zz   arggggggg', cname, 'x', mname, 'x', args[i]['name'], 'x', args[i]['type'], 'x', dir(args[i]), 'y', args[i].get('default'), 'z', args[i].get('defaltValue'), args[i].keys()

      if args[i]['name'].replace(' ', '') == '':
        args[i]['name'] = 'arg' + str(i+1)
      elif args[i]['name'] == '&':
        args[i]['name'] = 'arg' + str(i+1)
        args[i]['type'] += '&'

      assert not args[i].get('default')

      if '>' in args[i]['name']:
        print 'WARNING: odd ">" in %s, skipping' % cname
        skip = True
        break
      #print 'c1', struct_parents.keys()
      if args[i]['type'][-1] == '&':
        sname = args[i]['type'][:-1]
        if sname[-1] == ' ': sname = sname[:-1]
        if sname in struct_parents:
          args[i]['type'] = struct_parents[sname] + '::' + sname + '&'
        elif sname.replace('const ', '') in struct_parents:
          sname = sname.replace('const ', '')
          args[i]['type'] = 'const ' + struct_parents[sname] + '::' + sname + '&'
      #print 'POST arggggggg', cname, 'x', mname, 'x', args[i]['name'], 'x', args[i]['type']
    if skip:
      continue

    # C

    ret = ((cname + ' *') if constructor else method['rtnType']).replace('virtual ', '')
    callprefix = 'new ' if constructor else 'self->'

    actualmname = ''
    if mname == '__operator___assignment_':
      callprefix = '*self = '
      continue # TODO
    elif mname == '__operator____mul__':
      callprefix = '*self * '
      continue # TODO
    elif mname == '__operator____div__':
      callprefix = '*self + '
      continue # TODO
    elif mname == '__operator____add__':
      callprefix = '*self + '
      continue # TODO
    elif mname == '__operator____sub__':
      callprefix = '*self - '
      continue # TODO
    elif mname == '__operator____imult__':
      callprefix = '*self * '
      continue # TODO
    elif mname == '__operator____idiv__':
      callprefix = '*self + '
      continue # TODO
    elif mname == '__operator____iadd__':
      callprefix = '*self + '
      continue # TODO
    elif mname == '__operator____isub__':
      callprefix = '*self - '
      continue # TODO
    elif mname == '__operator____eq__':
      callprefix = '*self - '
      continue # TODO
    else:
      actualmname = method.get('truename') or mname

    typedargs = ', '.join( ([] if constructor else [cname + ' * self']) + map(lambda arg: arg['type'] + ' ' + arg['name'], args) )
    justargs = ', '.join(map(lambda arg: arg['name'], args))
    fullname = 'emscripten_bind_' + generating_cname + '__' + mname
    generating_cname_suffixed = generating_cname
    mname_suffixed = mname
    count = funcs.setdefault(fullname, 0)
    funcs[fullname] += 1

    # handle overloading
    dupe = False
    if count > 0:
      dupe = True
      suffix = '_' + str(count+1)
      funcs[fullname + suffix] = 0
      fullname += suffix
      mname_suffixed += suffix
      if constructor:
        generating_cname_suffixed += suffix

    gen_c.write('''
%s %s(%s) {
  %s%s%s(%s);
}
''' % (ret, fullname, typedargs, 'return ' if ret.replace(' ', '') != 'void' else '', callprefix, actualmname, justargs))

    # JS

    if constructor:
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
  if cname in ignored: continue

  # Nothing to generate for pure virtual classes

  def check_pure_virtual(clazz, progeny):
    if any([check_pure_virtual(classes[parent['class']], [clazz] + progeny) for parent in clazz['inherits']]): return True

    def dirtied(mname):
      #print 'zz checking dirtiness for', mname, 'in', progeny
      for progen in progeny:
        for method in progen['methods']['public']:
          if method['name'] == mname and not method['pure_virtual']:
            #print 'zz dirty'
            return True
      #print 'zz not dirtied'
      return False

    for method in clazz['methods']['public']:
      if method['pure_virtual'] and not dirtied(method['name']):
        print 'zz ignoring pure virtual class', cname, 'due to', method['name']
        return True

  if check_pure_virtual(clazz, []):
    continue

  # Add a constructor if none exist
  has_constructor = False
  for method in clazz['methods']['public']:
    mname = method['name']
    has_constructor = has_constructor or (cname == mname and not method['destructor'])

  print 'zz ', cname, 'has constructor?', has_constructor

  if not has_constructor:
    print 'zz no constructor for', cname, 'so ignoring'
    continue

    clazz['methods']['public'] = [{
      'name': cname,
      'parameters': [],
      'pure_virtual': False,
      'destructor': False,
    }] + clazz['methods']['public']

  generate_class(cname, cname, clazz)

  # In addition, generate all methods of parent classes. We do not inherit in JS (how would we do multiple inheritance etc.?)
  for parent in clazz['inherits']:
    generate_class(cname, parent['class'], classes[parent['class']])

  # TODO: Add a destructor

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

