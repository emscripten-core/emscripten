
'''
WebIDL binder

https://github.com/kripken/emscripten/wiki/WebIDL-Binder
'''

import os, sys

import shared

sys.path.append(shared.path_from_root('third_party'))
sys.path.append(shared.path_from_root('third_party', 'ply'))

import WebIDL

class Dummy:
  def __init__(self, init):
    for k, v in init.iteritems():
      self.__dict__[k] = v

  def getExtendedAttribute(self, name):
    return None

input_file = sys.argv[1]
output_base = sys.argv[2]

shared.try_delete(output_base + '.cpp')
shared.try_delete(output_base + '.js')

p = WebIDL.Parser()
p.parse(r'''
interface VoidPtr {
};
''' + open(input_file).read())
data = p.finish()

interfaces = {}
implements = {}
enums = {}

for thing in data:
  if isinstance(thing, WebIDL.IDLInterface):
    interfaces[thing.identifier.name] = thing
  elif isinstance(thing, WebIDL.IDLImplementsStatement):
    implements.setdefault(thing.implementor.identifier.name, []).append(thing.implementee.identifier.name)
  elif isinstance(thing, WebIDL.IDLEnum):
    enums[thing.identifier.name] = thing

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

def emit_constructor(name):
  global mid_js
  mid_js += [r'''%s.prototype = %s;
%s.prototype.constructor = %s;
%s.prototype.__class__ = %s;
%s.__cache__ = {};
Module['%s'] = %s;
''' % (name, 'Object.create(%s.prototype)' % (implements[name][0] if implements.get(name) else 'WrapperObject'), name, name, name, name, name, name, name)]


mid_js += ['''
// Bindings utilities

function WrapperObject() {
}
''']

emit_constructor('WrapperObject')

mid_js += ['''
function getCache(__class__) {
  return (__class__ || WrapperObject).__cache__;
}
Module['getCache'] = getCache;

function wrapPointer(ptr, __class__) {
  var cache = getCache(__class__);
  var ret = cache[ptr];
  if (ret) return ret;
  ret = Object.create((__class__ || WrapperObject).prototype);
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
  delete getCache(obj.__class__)[obj.ptr];
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
var ensureString = (function() {
  var stringCache = {};
  function ensureString(value) {
    if (typeof value == 'string') {
      var cachedVal = stringCache[value];
      if (cachedVal) return cachedVal;
      var ret = allocate(intArrayFromString(value), 'i8', ALLOC_STACK);
      stringCache[value] = ret;
      return ret;
    }
    return value;
  }
  return ensureString;
})();

''']

mid_c += ['''
// Not using size_t for array indices as the values used by the javascript code are signed.
void array_bounds_check(const int array_size, const int array_idx) {
  if (array_idx < 0 || array_idx >= array_size) {
    EM_ASM_INT({
      throw 'Array index ' + $0 + ' out of bounds: [0,' + $1 + ')';
    }, array_idx, array_size);
  }
}
''']

C_FLOATS = ['float', 'double']

def type_to_c(t, non_pointing=False):
  #print 'to c ', t
  t = t.replace(' (Wrapper)', '')
  if t == 'Long':
    return 'int'
  elif t == 'UnsignedLong':
    return 'unsigned int'
  elif t == 'Short':
    return 'short'
  elif t == 'UnsignedShort':
    return 'unsigned short'
  elif t == 'Byte':
    return 'char'
  elif t == 'Octet':
    return 'unsigned char'
  elif t == 'Void':
    return 'void'
  elif t == 'String':
    return 'char*'
  elif t == 'Float':
    return 'float'
  elif t == 'Double':
    return 'double'
  elif t == 'Boolean':
    return 'bool'
  elif t == 'Any' or t == 'VoidPtr':
    return 'void*'
  elif t in interfaces:
    return (interfaces[t].getExtendedAttribute('Prefix') or [''])[0] + t + ('' if non_pointing else '*')
  else:
    return t

def take_addr_if_nonpointer(m):
  if m.getExtendedAttribute('Ref') or m.getExtendedAttribute('Value'):
    return '&'
  return ''

def deref_if_nonpointer(m):
  if m.getExtendedAttribute('Ref') or m.getExtendedAttribute('Value'):
    return '*'
  return ''

def type_to_cdec(raw):
  name = ret = type_to_c(raw.type.name, non_pointing=True)
  if raw.getExtendedAttribute('Const'): ret = 'const ' + ret
  if name not in interfaces: return ret
  if raw.getExtendedAttribute('Ref'):
    return ret + '&'
  if raw.getExtendedAttribute('Value'):
    return ret
  return ret + '*'

def render_function(class_name, func_name, sigs, return_type, non_pointer, copy, operator, constructor, func_scope, call_content=None, const=False):
  global mid_c, mid_js, js_impl_methods

  #print 'renderfunc', class_name, func_name, sigs, return_type, constructor

  bindings_name = class_name + '_' + func_name
  min_args = min(sigs.keys())
  max_args = max(sigs.keys())

  c_names = {}

  # JS

  cache = ('getCache(%s)[this.ptr] = this;' % class_name) if constructor else ''
  call_prefix = '' if not constructor else 'this.ptr = '
  call_postfix = ''
  if return_type != 'Void' and not constructor: call_prefix = 'return '
  if not constructor:
    if return_type in interfaces:
      call_prefix += 'wrapPointer('
      call_postfix += ', ' + return_type + ')'
    elif return_type == 'String':
      call_prefix += 'Pointer_stringify('
      call_postfix += ')'

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
    body += '  if (arg%d === undefined) { %s%s(%s)%s%s }\n' % (i, call_prefix, '_' + c_names[i], ', '.join(pre_arg + args[:i]), call_postfix, '' if 'return ' in call_prefix else '; ' + (cache or ' ') + 'return')
  c_names[max_args] = 'emscripten_bind_%s_%d' % (bindings_name, max_args)
  body += '  %s%s(%s)%s;\n' % (call_prefix, '_' + c_names[max_args], ', '.join(pre_arg + args), call_postfix)
  if cache:
    body += '  ' + cache + '\n'
  mid_js += [r'''function%s(%s) {
%s
};''' % ((' ' + func_name) if constructor else '', ', '.join(args), body[:-1])]

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
      full_args = type_to_c(class_name, non_pointing=True) + '* self' + ('' if not normal_args else ', ' + normal_args)
    call_args = ', '.join(['%sarg%d' % ('*' if raw[j].getExtendedAttribute('Ref') else '', j) for j in range(i)])
    if constructor:
      call = 'new ' + type_to_c(class_name, non_pointing=True)
      call += '(' + call_args + ')'
    elif call_content is not None:
      call = call_content
    else:
      call = 'self->' + func_name
      call += '(' + call_args + ')'

    if operator:
      assert '=' in operator, 'can only do += *= etc. for now, all with "="'
      cast_self = 'self'
      if class_name != func_scope:
        # this function comes from an ancestor class; for operators, we must cast it
        cast_self = 'dynamic_cast<' + type_to_c(func_scope) + '>(' + cast_self + ')'
      call = '(*%s %s %sarg0)' % (cast_self, operator, '*' if sig[0] in interfaces else '')

    pre = ''

    basic_return = 'return ' if constructor or return_type is not 'Void' else ''
    return_prefix = basic_return
    return_postfix = ''
    if non_pointer:
      return_prefix += '&';
    if copy:
      pre += '  static %s temp;\n' % type_to_c(return_type, non_pointing=True)
      return_prefix += '(temp = '
      return_postfix += ', &temp)'

    c_return_type = type_to_c(return_type)
    mid_c += [r'''
%s%s EMSCRIPTEN_KEEPALIVE %s(%s) {
%s  %s%s%s;
}
''' % ('const ' if const else '', type_to_c(class_name) if constructor else c_return_type, c_names[i], full_args, pre, return_prefix, call, return_postfix)]

    if not constructor:
      if i == max_args:
        dec_args = ', '.join(map(lambda j: type_to_cdec(raw[j]) + ' arg' + str(j), range(i)))
        js_call_args = ', '.join(['%sarg%d' % (('(int)' if sig[j] in interfaces else '') + ('&' if raw[j].getExtendedAttribute('Ref') or raw[j].getExtendedAttribute('Value') else ''), j) for j in range(i)])

        js_impl_methods += [r'''  %s %s(%s) {
    %sEM_ASM_%s({
      var self = Module['getCache'](Module['%s'])[$0];
      if (!self.hasOwnProperty('%s')) throw 'a JSImplementation must implement all functions, you forgot %s::%s.';
      %sself.%s(%s)%s;
    }, (int)this%s);
  }''' % (c_return_type, func_name, dec_args,
          basic_return, 'INT' if c_return_type not in C_FLOATS else 'DOUBLE',
          class_name,
          func_name, class_name, func_name,
          return_prefix,
          func_name,
          ','.join(['$%d' % i for i in range(1, max_args + 1)]),
          return_postfix,
          (', ' if js_call_args else '') + js_call_args)]


for name, interface in interfaces.iteritems():
  js_impl = interface.getExtendedAttribute('JSImplementation')
  if not js_impl: continue
  implements[name] = [js_impl[0]]

names = interfaces.keys()
names.sort(lambda x, y: 1 if implements.get(x) and implements[x][0] == y else (-1 if implements.get(y) and implements[y][0] == x else 0))

for name in names:
  interface = interfaces[name]

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

  seen_constructor = False # ensure a constructor, even for abstract base classes
  for m in interface.members:
    if m.identifier.name == name:
      seen_constructor = True
      break
  if not seen_constructor:
    mid_js += ['function %s() { throw "cannot construct a %s, no constructor in IDL" }\n' % (name, name)]
    emit_constructor(name)

  for m in interface.members:
    if not m.isMethod(): continue
    constructor = m.identifier.name == name
    if not constructor:
      parent_constructor = False
      temp = m.parentScope
      while temp.parentScope:
        if temp.identifier.name == m.identifier.name:
          parent_constructor = True
        temp = temp.parentScope
      if parent_constructor:
        continue
    if not constructor:
      mid_js += [r'''
%s.prototype['%s'] = ''' % (name, m.identifier.name)]
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
    render_function(name,
                    m.identifier.name, sigs, return_type,
                    m.getExtendedAttribute('Ref'),
                    m.getExtendedAttribute('Value'),
                    (m.getExtendedAttribute('Operator') or [None])[0],
                    constructor,
                    func_scope=m.parentScope.identifier.name,
                    const=m.getExtendedAttribute('Const'))
    mid_js += [';\n']
    if constructor:
      emit_constructor(name)

  for m in interface.members:
    if not m.isAttr(): continue
    attr = m.identifier.name

    if m.type.isArray():
      get_sigs = { 1: [Dummy({ 'type': WebIDL.BuiltinTypes[WebIDL.IDLBuiltinType.Types.long] })] }
      set_sigs = { 2: [Dummy({ 'type': WebIDL.BuiltinTypes[WebIDL.IDLBuiltinType.Types.long] }),
                       Dummy({ 'type': m.type })] }
      get_call_content = take_addr_if_nonpointer(m) + 'self->' + attr + '[arg0]'
      set_call_content = 'self->' + attr + '[arg0] = ' + deref_if_nonpointer(m) + 'arg1'
      if m.getExtendedAttribute('BoundsChecked'):
        bounds_check = "array_bounds_check(sizeof(self->%s) / sizeof(self->%s[0]), arg0)" % (attr, attr)
        get_call_content = "(%s, %s)" % (bounds_check, get_call_content)
        set_call_content = "(%s, %s)" % (bounds_check, set_call_content)
    else:
      get_sigs = { 0: [] }
      set_sigs = { 1: [Dummy({ 'type': m.type })] }
      get_call_content = take_addr_if_nonpointer(m) + 'self->' + attr
      set_call_content = 'self->' + attr + ' = ' + deref_if_nonpointer(m) + 'arg0'

    get_name = 'get_' + attr
    mid_js += [r'''
  %s.prototype['%s']= ''' % (name, get_name)]
    render_function(name,
                    get_name, get_sigs, m.type.name,
                    None,
                    None,
                    None,
                    False,
                    func_scope=interface,
                    call_content=get_call_content,
                    const=m.getExtendedAttribute('Const'))

    if not m.readonly:
      set_name = 'set_' + attr
      mid_js += [r'''
    %s.prototype['%s']= ''' % (name, set_name)]
      render_function(name,
                      set_name, set_sigs, 'Void',
                      None,
                      None,
                      None,
                      False,
                      func_scope=interface,
                      call_content=set_call_content,
                      const=m.getExtendedAttribute('Const'))

  if not interface.getExtendedAttribute('NoDelete'):
    mid_js += [r'''
  %s.prototype['__destroy__'] = ''' % name]
    render_function(name,
                    '__destroy__', { 0: [] }, 'Void',
                    None,
                    None,
                    None,
                    False,
                    func_scope=interface,
                    call_content='delete self')

  # Emit C++ class implementation that calls into JS implementation

  if js_impl:
    pre_c += [r'''
class %s : public %s {
public:
%s
};
''' % (name, type_to_c(js_impl, non_pointing=True), '\n'.join(js_impl_methods))]

deferred_js = []

for name, enum in enums.iteritems():
  mid_c += ['\n// ' + name + '\n']
  deferred_js += ['\n', '// ' + name + '\n']
  for value in enum.values():
    function_id = "%s_%s" % (name, value.split('::')[-1])
    mid_c += [r'''%s EMSCRIPTEN_KEEPALIVE emscripten_enum_%s() {
  return %s;
}
''' % (name, function_id, value)]
    symbols = value.split('::')
    if len(symbols) == 1:
      identifier = symbols[0]
      deferred_js += ["Module['%s'] = _emscripten_enum_%s();\n" % (identifier, function_id)]
    elif len(symbols) == 2:
      [namespace, identifier] = symbols
      if namespace in interfaces:
        # namespace is a class
        deferred_js += ["Module['%s']['%s'] = _emscripten_enum_%s();\n" % \
                  (namespace, identifier, function_id)]
      else:
        # namespace is a namespace, so the enums get collapsed into the top level namespace.
        deferred_js += ["Module['%s'] = _emscripten_enum_%s();\n" % (identifier, function_id)]
    else:
      throw ("Illegal enum value %s" % value)

mid_c += ['\n}\n\n']
mid_js += ['''
(function() {
  function setupEnums() {
    %s
  }
  if (Module['calledRun']) setupEnums();
  else addOnPreMain(setupEnums);
})();
''' % '\n    '.join(deferred_js)]

# Write

c = open(output_base + '.cpp', 'w')
for x in pre_c: c.write(x)
for x in mid_c: c.write(x)
c.close()

js = open(output_base + '.js', 'w')
for x in mid_js: js.write(x)
js.close()

