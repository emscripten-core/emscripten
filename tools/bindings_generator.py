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

processor = lambda text: text
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

text = ''
for header in sys.argv[2:]:
  text += '//// ' + header + '\n'
  text += open(header, 'r').read()
all_h_name = basename + '.all.h'
all_h = open(all_h_name, 'w')

all_h.write(processor(text))
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

    default_param = len(args)+1
    for i in range(len(args)):
      if args[i].get('default'):
        default_param = i+1
        break

    method['num_args'] = set(range(default_param-1, len(args)+1))
    print 'zz ', classname, 'has num_args of', method['num_args']

    if method['static']:
      method['rtnType'] = method['rtnType'].replace('static', '')

# Explore all functions we need to generate, including parent classes, handling of overloading, etc.

for classname, clazz in parsed.classes.iteritems():
  clazz['final_methods'] = {}

  def explore(subclass):
    # Do our functions first, and do not let later classes override
    for method in subclass['methods']['public']:
      if method['constructor']:
        if clazz != subclass: continue # Subclasses cannot directly use their parent's constructors
      if method['destructor']: continue # Nothing to do there

      if method['name'] not in clazz['final_methods']:
        clazz['final_methods'][method['name']] = {}
        for key in ['name', 'constructor', 'static', 'rtnType', 'destructor', 'pure_virtual']:
          clazz['final_methods'][method['name']][key] = method[key]
        clazz['final_methods'][method['name']]['num_args'] = method['num_args'].copy()
        clazz['final_methods'][method['name']]['parameters'] = method['parameters'][:]
      else:
        # Merge the new function in the best way we can. Shared arguments must match!

        curr = clazz['final_methods'][method['name']]
        if any([curr['parameters'][i]['type'] != method['parameters'][i]['type'] for i in range(min(len(curr['parameters']), len(method['parameters'])))]):
          print 'Warning: Cannot mix in overloaded functions', method['name'], 'in class', classname, ', skipping'
          continue
        # TODO: Other compatibility checks, if any?

        if len(method['parameters']) > len(curr['parameters']):
          curr['parameters'] = method['parameters']

        curr['num_args'] = curr['num_args'].union(method['num_args'])
        print 'zz ', classname, 'has an updated num_args of ', curr['num_args']

    # Recurse
    for parent in subclass['inherits']:
      if parent['class'] not in classes:
        print 'Warning: parent class', parent, 'not a known class. Ignoring.'
        return
      explore(classes[parent['class']])

  explore(clazz)

  for method in clazz['final_methods'].itervalues():
    method['num_args'] = list(method['num_args'])
    method['num_args'].sort()

# Second pass - generate bindings
# TODO: Bind virtual functions using dynamic binding in the C binding code

funcs = {} # name -> # of copies in the original, and originalname in a copy
c_funcs = []

gen_c = open(basename + '.c', 'w')
gen_js = open(basename + '.js', 'w')

gen_c.write('extern "C" {\n')

def generate_class(generating_classname, classname, clazz): # TODO: deprecate generating?
  inherited = generating_classname != classname

  for method in clazz['final_methods'].itervalues():
    mname = method['name']
    if classname + '::' + mname in ignored: continue

    args = method['parameters']
    constructor = method['constructor'] # we fixed this before
    destructor = method['destructor']
    static = method['static']

    print "zz generating: ", generating_classname, classname, mname, constructor, method['rtnType']

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
    callprefix = 'new ' if constructor else ('self->' if not static else (classname + '::'))

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

    need_self = not constructor and not static
    typedargs = ([] if not need_self else [classname + ' * self']) + map(lambda arg: arg['type'] + ' ' + arg['name'], args)
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

    argfixes = '\n'.join(map(lambda arg: '''  %s = (%s && %s.ptr) ? %s.ptr : %s;''' % (arg['name'], arg['name'], arg['name'], arg['name'], arg['name']), args))

    for i in method['num_args']:
      # C

      gen_c.write('''
%s %s_p%d(%s) {
  %s%s%s(%s);
}
''' % (ret, fullname, i, ', '.join(typedargs[:i + (0 if not need_self else 1)]), 'return ' if ret.replace(' ', '') != 'void' else '', callprefix, actualmname, ', '.join(justargs[:i])))

      c_funcs.append(fullname + '_p' + str(i))

    # JS
    calls = ''
    print 'js loopin', method['num_args'], '|', len(args), args
    for i in method['num_args']:
      print '    ', i, type(i)
      if i != method['num_args'][0]:
        calls += '  else '
      if i != method['num_args'][-1]:
        calls += '  if (' + justargs[i] + ' === undefined)'
      calls += '\n  ' + ('  ' if len(method['num_args']) > 0 else '')
      if constructor:
        if not dupe:
          calls += '''this.ptr = _%s_p%d(%s);
''' % (fullname, i, ', '.join(justargs[:i]))
        else:
          calls += '''this.ptr = _%s_p%d(%s);
''' % (fullname, i, ', '.join(justargs[:i]))
      else:
        calls += '''%s_%s_p%d(%s);
''' % ('return ' if ret != 'void' else '', fullname, i, ', '.join((['this.ptr'] if need_self else []) + justargs[:i]))

    print 'Maekin:', classname, generating_classname, mname, mname_suffixed
    if constructor:
      if not dupe:
        js_text = '''
function %s(%s) {
%s
%s
}
''' % (mname_suffixed, ', '.join(justargs), argfixes, calls)
      else:
        js_text = '''
function %s(%s) {
%s
%s
}
%s.prototype = %s.prototype;
''' % (mname_suffixed, ', '.join(justargs), argfixes, calls, mname_suffixed, classname)
    else:
      js_text = '''
%s.prototype.%s = function(%s) {
%s
%s
}
''' % (generating_classname, mname_suffixed, ', '.join(justargs), argfixes, calls)

    js_text = js_text.replace('\n\n', '\n').replace('\n\n', '\n')
    gen_js.write(js_text)

# Main loop

for classname, clazz in classes.iteritems():
  if classname in ignored: continue

  # Nothing to generate for pure virtual classes

  def check_pure_virtual(clazz, progeny):
    print 'Checking pure virtual for', clazz['name'], clazz['inherits']
    # If we do not recognize any of the parent classes, assume this is pure virtual - ignore it
    if any([((not parent['class'] in classes) or check_pure_virtual(classes[parent['class']], [clazz] + progeny)) for parent in clazz['inherits']]): return True

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

