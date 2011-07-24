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
for classname, clazz in parsed.classes.iteritems():
  print 'zz see', classname
  classes[classname] = clazz
  for sname, struct in clazz._public_structs.iteritems():
    struct_parents[sname] = classname
    #print 'zz seen struct %s in %s' % (sname, classname)

  # Various precalculations
  for method in clazz['methods']['public'][:]:
    constructor = method['name'] == classname
    method['constructor'] = constructor # work around cppheaderparser issue
    args = method['parameters']
    method['first_default_param'] = len(args)
    for i in range(len(args)):
      if args[i].get('default'):
        method['first_default_param'] = i

# Second pass - generate bindings
# TODO: Bind virtual functions using dynamic binding in the C binding code

funcs = {} # name -> # of copies in the original, and originalname in a copy
c_funcs = []

gen_c = open(basename + '.c', 'w')
gen_js = open(basename + '.js', 'w')

gen_c.write('extern "C" {\n')

def generate_class(generating_classname, classname, clazz):
  inherited = generating_classname != classname

  for method in clazz['methods']['public']:
    mname = method['name']
    if classname + '::' + mname in ignored: continue

    args = method['parameters']
    constructor = method['constructor'] # we fixed this before
    destructor = method['destructor']

    print "zz generating: ", generating_classname, classname, mname, constructor

    if destructor: continue
    if constructor and inherited: continue

    skip = False
    for i in range(len(args)):
      #print 'zz   arggggggg', classname, 'x', mname, 'x', args[i]['name'], 'x', args[i]['type'], 'x', dir(args[i]), 'y', args[i].get('default'), 'z', args[i].get('defaltValue'), args[i].keys()

      if args[i]['name'].replace(' ', '') == '':
        args[i]['name'] = 'arg' + str(i+1)
      elif args[i]['name'] == '&':
        args[i]['name'] = 'arg' + str(i+1)
        args[i]['type'] += '&'

      if '>' in args[i]['name']:
        print 'WARNING: odd ">" in %s, skipping' % classname
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
      #print 'POST arggggggg', classname, 'x', mname, 'x', args[i]['name'], 'x', args[i]['type']
    if skip:
      continue

    ret = ((classname + ' *') if constructor else method['rtnType']).replace('virtual ', '')
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

    typedargs = ([] if constructor else [classname + ' * self']) + map(lambda arg: arg['type'] + ' ' + arg['name'], args)
    justargs = map(lambda arg: arg['name'], args)
    fullname = 'emscripten_bind_' + generating_classname + '__' + mname
    generating_classname_suffixed = generating_classname
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
        generating_classname_suffixed += suffix

    argfixes = map(lambda arg: '''  %s = (%s && %s.ptr) ? %s.ptr : %s;''' % (arg['name'], arg['name'], arg['name'], arg['name'], arg['name']), args)

    for i in range(method['first_default_param'], len(args)+1):
      # C

      gen_c.write('''
%s %s_p%d(%s) {
  %s%s%s(%s);
}
''' % (ret, fullname, i, ', '.join(typedargs[:i+1]), 'return ' if ret.replace(' ', '') != 'void' else '', callprefix, actualmname, ', '.join(justargs[:i])))

      c_funcs.append(fullname + '_p' + str(i))

    # JS
    calls = ''
    print 'js loopin', method['first_default_param'], len(args), args
    for i in range(method['first_default_param'], len(args)+1):
      print '    ', i, type(i)
      if i > method['first_default_param']:
        calls += '  else '
      if i != len(args):
        calls += '  if (' + justargs[i] + ' === undefined)'
      calls += '\n  ' + ('  ' if method['first_default_param'] != len(args) else '')
      if constructor:
        if not dupe:
          calls += '''this.ptr = _%s_p%d(%s);
''' % (fullname, i, ', '.join(justargs[:i]))
        else:
          calls += '''this.ptr = _%s_p%d(%s);
''' % (fullname, i, ', '.join(justargs[:i]))
      else:
        calls += '''%s_%s_p%d(this.ptr%s);
''' % ('return ' if ret != 'void' else '', fullname, i, (', ' if i > 0 else '') + ', '.join(justargs[:i]))

    print 'Maekin:', classname, generating_classname, mname, mname_suffixed
    if constructor:
      if not dupe:
        js_text = '''
function %s(%s) {
%s
%s
}
''' % (mname_suffixed, ', '.join(justargs), ', '.join(argfixes), calls)
      else:
        js_text = '''
function %s(%s) {
%s
%s
}
%s.prototype = %s.prototype;
''' % (mname_suffixed, ', '.join(justargs), ', '.join(argfixes), calls, mname_suffixed, classname)
    else:
      js_text = '''
%s.prototype.%s = function(%s) {
%s
%s
}
''' % (generating_classname, mname_suffixed, ', '.join(justargs), ', '.join(argfixes), calls)

    js_text = js_text.replace('\n\n', '\n').replace('\n\n', '\n')
    gen_js.write(js_text)

# Main loop

for classname, clazz in classes.iteritems():
  if classname in ignored: continue

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
        print 'zz ignoring pure virtual class', classname, 'due to', method['name']
        return True

  if check_pure_virtual(clazz, []):
    continue

  # Add a constructor if none exist
  has_constructor = False
  for method in clazz['methods']['public']:
    mname = method['name']
    has_constructor = has_constructor or (classname == mname and not method['destructor'])

  print 'zz ', classname, 'has constructor?', has_constructor

  if not has_constructor:
    print 'zz no constructor for', classname, 'so ignoring'
    continue

    clazz['methods']['public'] = [{
      'name': classname,
      'parameters': [],
      'pure_virtual': False,
      'destructor': False,
    }] + clazz['methods']['public']

  generate_class(classname, classname, clazz)

  # In addition, generate all methods of parent classes. We do not inherit in JS (how would we do multiple inheritance etc.?)
  for parent in clazz['inherits']:
    generate_class(classname, parent['class'], classes[parent['class']])

  # TODO: Add a destructor

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
''' % c_funcs[0])

for func in c_funcs[1:]:
  gen_c.write('''    sum += (void*)%s == seen;
''' % func)

gen_c.write('''    printf("(%d)\\n", sum);
  }
};

EmscriptenEnsurer emscriptenEnsurer;
''')

gen_c.close()
gen_js.close()

