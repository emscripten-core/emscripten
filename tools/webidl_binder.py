
import os, sys

import shared

sys.path.append(shared.path_from_root('third_party'))
sys.path.append(shared.path_from_root('third_party', 'ply'))

import WebIDL

input_file = sys.argv[1]
output_base = sys.argv[2]

shared.try_delete(output_base + '.cpp')
shared.try_delete(output_base + '.js')

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

pre_c = []
mid_c = []
mid_js = []

pre_c += [r'''
#include <emscripten.h>
''']

mid_c += [r'''
extern "C" {
''']

mid_js += ['''
// Bindings utilities

var Object__cache = Module['Object__cache'] = {}; // we do it this way so we do not modify |Object|
function wrapPointer(ptr, __class__) {
  var cache = Object__cache;
  var ret = cache[ptr];
  if (ret) return ret;
  __class__ = __class__ || Object;
  ret = Object.create(__class__.prototype);
  ret.ptr = ptr;
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
  if (typeof value == 'string') return allocate(intArrayFromString(value), 'i8', ALLOC_STACK);
  return value;
}

''']

C_FLOATS = ['float', 'double']

def type_to_c(t):
  #print 'to c ', t
  t = t.replace(' (Wrapper)', '')
  if t == 'Long':
    return 'int'
  elif t == 'Short':
    return 'short'
  elif t == 'Void':
    return 'void'
  elif t == 'String':
    return 'char*'
  elif t == 'Float':
    return 'float'
  elif t == 'Double':
    return 'double'
  elif t in interfaces:
    return t + '*'
  else:
    return t

def render_function(class_name, func_name, sigs, return_type, constructor):
  global mid_c, mid_js, js_impl_methods

  #print 'renderfunc', class_name, func_name, sigs, return_type, constructor

  bindings_name = class_name + '_' + func_name
  min_args = min(sigs.keys())
  max_args = max(sigs.keys())

  c_names = {}

  # JS
  cache = 'Object__cache[this.ptr] = this'
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
    body += "  else arg%d = ensureString(arg%d);\n" % (i, i)

  for i in range(min_args, max_args):
    c_names[i] = 'emscripten_bind_%s_%d' % (bindings_name, i)
    body += '  if (arg%d === undefined) { %s%s(%s)%s }\n' % (i, call_prefix, '_' + c_names[i], ', '.join(pre_arg + args[:i]), '' if 'return ' in call_prefix else '; ' + cache + '; return')
  c_names[max_args] = 'emscripten_bind_%s_%d' % (bindings_name, max_args)
  body += '  %s%s(%s);\n' % (call_prefix, '_' + c_names[max_args], ', '.join(pre_arg + args))
  body += '  ' + cache + ';\n'
  mid_js += [r'''function%s(%s) {
%s
}''' % ((' ' + func_name) if constructor else '', ', '.join(args), body[:-1])]

  # C

  for i in range(min_args, max_args+1):
    raw = sigs.get(i)
    if raw is None: continue
    sig = [arg.type.name for arg in raw]

    c_arg_types = map(type_to_c, sig)
 
    normal_args = ', '.join(['%s arg%d' % (c_arg_types[j], j) for j in range(i)])
    if constructor:
      full_args = normal_args
    else:
      full_args = class_name + '* self' + ('' if not normal_args else ', ' + normal_args)
    call_args = ', '.join(['%sarg%d' % ('*' if raw[j].getExtendedAttribute('ByValue') else '', j) for j in range(i)])
    if constructor:
      call = 'new '
    else:
      call = 'self->'
    call += func_name + '(' + call_args + ')'
    return_statement = 'return ' if return_type is not 'Void' or constructor else ''
    c_return_type = type_to_c(return_type)
    mid_c += [r'''
%s EMSCRIPTEN_KEEPALIVE %s(%s) {
  %s%s;
}
''' % ((class_name + '*') if constructor else c_return_type, c_names[i], full_args, return_statement, call)]

    if not constructor:
      if i == max_args:
        js_impl_methods += [r'''  %s %s(%s) {
    %sEM_ASM_%s({
      var self = Module['Object__cache'][$0];
      if (!self.hasOwnProperty('%s')) throw 'a JSImplementation must implement all functions, you forgot %s::%s.';
      %sself.%s(%s);
    }, (int)this%s);
  }''' % (c_return_type, func_name, normal_args,
          return_statement, 'INT' if c_return_type not in C_FLOATS else 'DOUBLE',
          func_name, class_name, func_name,
          return_statement,
          func_name,
          ','.join(['$%d' % i for i in range(1, max_args)]),
          (', ' if call_args else '') + call_args)]

for name, interface in interfaces.iteritems():
  mid_js += ['\n// ' + name + '\n']
  mid_c += ['\n// ' + name + '\n']

  global js_impl_methods
  js_impl_methods = []

  cons = interface.getExtendedAttribute('Constructor')
  if type(cons) == list: raise Exception('do not use "Constructor", instead create methods with the name of the interface')

  js_impl = interface.getExtendedAttribute('JSImplementation')
  if js_impl:
    js_impl = js_impl[0]

  # Methods

  for m in interface.members:
    if m.identifier.name in (implements.get(name) or []): continue
    constructor = m.identifier.name == name
    if not constructor:
      mid_js += [r'''
%s.prototype.%s = ''' % (name, m.identifier.name)]
    sigs = {}
    return_type = None
    for ret, args in m.signatures():
      if return_type is None:
        return_type = ret.name
      else:
        assert return_type == ret.name, 'overloads must have the same return type'
      for i in range(len(args)+1):
        if i == len(args) or args[i].optional:
          assert i not in sigs, 'overloading must differentiate by # of arguments (cannot have two signatures that differ by types but not by length)'
          sigs[i] = args[:i]
    render_function(name, m.identifier.name, sigs, return_type, constructor)
    mid_js += [';\n']
    if constructor:
      mid_js += [r'''Module['%s'] = %s;
''' % (name, name)]

  # Emit C++ class implementation that calls into JS implementation

  if js_impl:
    pre_c += [r'''
class %s : public %s {
public:
%s
};
''' % (name, js_impl, '\n'.join(js_impl_methods))]

mid_c += ['\n}\n\n']
mid_js += ['\n']

# Write

c = open(output_base + '.cpp', 'w')
for x in pre_c: c.write(x)
for x in mid_c: c.write(x)
c.close()

js = open(output_base + '.js', 'w')
for x in mid_js: js.write(x)
js.close()

