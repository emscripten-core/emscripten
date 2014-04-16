
import os, sys

import shared

sys.path.append(shared.path_from_root('third_party'))
sys.path.append(shared.path_from_root('third_party', 'ply'))

import WebIDL

input_file = sys.argv[1]
output_base = sys.argv[2]

p = WebIDL.Parser()
p.parse(open(input_file).read())
data = p.finish()

interfaces = {}
implements = {}

for thing in data:
  if isinstance(thing, WebIDL.IDLInterface):
    interfaces[thing.identifier.name] = thing
  elif isinstance(thing, WebIDL.IDLImplementsStatement):
    implements.setdefault(thing.implementor.identifier.name, []).append(thing.implementee.identifier.name)

#print interfaces
#print implements

gen_c = open(output_base + '.cpp', 'w')
gen_js = open(output_base + '.js', 'w')

gen_c.write(r'''
#include <emscripten.h>

extern "C" {
''')

gen_js.write('''
// Bindings utilities

var Object__cache = {}; // we do it this way so we do not modify |Object|
function wrapPointer(ptr, __class__) {
  var cache = Object__cache;
  var ret = cache[ptr];
  if (ret) return ret;
  __class__ = __class__ || Object;
  ret = Object.create(__class__.prototype);
  ret.ptr = ptr;
  ret.__class__ = __class__;
  return cache[ptr] = ret;
}
Module['wrapPointer'] = wrapPointer;

function castObject(obj, __class__) {
  return wrapPointer(obj.ptr, __class__);
}
Module['castObject'] = castObject;

Module['NULL'] = wrapPointer(0);

function destroy(obj) {
  if (!obj['__destroy__']) throw 'Error: Cannot destroy object. (Did you create it yourself?)';
  obj['__destroy__']();
  // Remove from cache, so the object can be GC'd and refs added onto it released
  delete Object__cache[obj.ptr];
}
Module['destroy'] = destroy;

function compare(obj1, obj2) {
  return obj1.ptr === obj2.ptr;
}
Module['compare'] = compare;

function getPointer(obj) {
  return obj.ptr;
}
Module['getPointer'] = getPointer;

function getClass(obj) {
  return obj.__class__;
}
Module['getClass'] = getClass;

// Converts a value into a C-style string.
function ensureString(value) {
  if (typeof value == 'number') return value;
  return allocate(intArrayFromString(value), 'i8', ALLOC_STACK);
}

''')

def type_to_c(t):
  #print 'to c ', t
  if t == 'Long':
    return 'int'
  elif t == 'Short':
    return 'short'
  elif t == 'Void':
    return 'void'
  elif t in interfaces:
    return t + '*'
  else:
    return t

def render_function(self_name, class_name, func_name, min_args, arg_types, return_type, constructor=False):
  #print >> sys.stderr, 'renderfunc', name, min_args, arg_types
  bindings_name = class_name + '_' + func_name
  max_args = len(arg_types)

  c_names = {}

  # JS
  call_prefix = '' if not constructor else 'this.ptr = '
  if return_type != 'Void' and not constructor: call_prefix = 'return '
  args = ['arg%d' % i for i in range(max_args)]
  if not constructor:
    body = '  var self = this.ptr;\n'
    pre_arg = ['self']
  else:
    body = ''
    pre_arg = []

  for i in range(max_args):
    # note: null has typeof object, but is ok to leave as is, since we are calling into asm code where null|0 = 0
    body += "  if (arg%d && typeof arg%d === 'object') arg%d = arg%d.ptr;\n" % (i, i, i, i)
  for i in range(min_args, max_args):
    c_names[i] = 'emscripten_bind_%s_%d' % (bindings_name, i)
    body += '  if (arg%d === undefined) { %s%s(%s)%s }\n' % (i, call_prefix, '_' + c_names[i], ', '.join(pre_arg + args[:i]), '' if 'return ' in call_prefix else '; return')
  c_names[max_args] = 'emscripten_bind_%s_%d' % (bindings_name, max_args)
  body += '  %s%s(%s);\n' % (call_prefix, '_' + c_names[max_args], ', '.join(pre_arg + args))
  gen_js.write(r'''function%s(%s) {
%s
}''' % ((' ' + self_name) if self_name is not None else '', ', '.join(args), body[:-1]))

  # C
  for i in range(min_args, max_args+1):
    full_args = ', '.join(([class_name + '* self'] if not constructor else []) + ['%s arg%d' % (type_to_c(arg_types[j]), j) for j in range(i)])
    call_args = ', '.join(['arg%d' % j for j in range(i)])
    if constructor:
      call = 'new '
    else:
      call = 'self->'
    call += func_name + '(' + call_args + ')'
    gen_c.write(r'''
%s EMSCRIPTEN_KEEPALIVE %s(%s) {
  %s%s;
}
''' % ((class_name + '*') if constructor else type_to_c(return_type), c_names[i], full_args, 'return ' if return_type is not 'Void' or constructor else '', call))

for name, interface in interfaces.iteritems():
  gen_js.write('\n// ' + name + '\n')
  gen_c.write('\n// ' + name + '\n')

  # Constructor
  min_args = 0
  arg_types = []
  cons = interface.getExtendedAttribute('Constructor')
  if type(cons) == list:
    args_list = cons[0]
    for i in range(len(args_list)):
      arg = args_list[i]
      arg_types.append(str(arg.type))
      if arg.optional:
        break
      min_args = i+1
  parent = '{}'
  if name in implements:
    assert len(implements[name]) == 1, 'cannot handle multiple inheritance yet'
    parent = 'Object.create(%s)' % implements[name][0]
  gen_js.write('\n')
  render_function(name, name, name, min_args, arg_types, 'Void', constructor=True)
  gen_js.write(r'''
Module['%s'] = %s;
%s.prototype = %s;
''' % (name, name, name, parent))

  # Methods
  for m in interface.members:
    #print dir(m)
    gen_js.write(r'''
%s.prototype.%s = ''' % (name, m.identifier.name))
    return_type, args = m.signatures()[0]
    arg_types = [arg.type.name for arg in args]
    render_function(None, name, m.identifier.name, min(m.allowedArgCounts), arg_types, return_type.name)
    gen_js.write(';\n')

gen_c.write('\n}\n\n');
gen_js.write('\n');

gen_c.close()
gen_js.close()

