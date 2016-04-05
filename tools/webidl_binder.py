
'''
WebIDL binder

http://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html
'''

import os, sys

import shared

sys.path.append(shared.path_from_root('third_party'))
sys.path.append(shared.path_from_root('third_party', 'ply'))

import WebIDL

# CHECKS='FAST' will skip most argument type checks in the wrapper methods for
#                  performance (~3x faster than default).
# CHECKS='ALL' will do extensive argument type checking (~5x slower than default).
#                 This will catch invalid numbers, invalid pointers, invalid strings, etc.
# Anything else defaults to legacy mode for backward compatibility.
CHECKS = os.environ.get('IDL_CHECKS') or 'DEFAULT'
# DEBUG=1 will print debug info in render_function
DEBUG = os.environ.get('IDL_VERBOSE') is '1'

if DEBUG: print "Debug print ON, CHECKS=%s" % CHECKS

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

// Converts big (string or array) values into a C-style storage, in temporary space

var ensureCache = {
  buffer: 0,  // the main buffer of temporary storage
  size: 0,   // the size of buffer
  pos: 0,    // the next free offset in buffer
  temps: [], // extra allocations
  needed: 0, // the total size we need next time

  prepare: function() {
    if (this.needed) {
      // clear the temps
      for (var i = 0; i < this.temps.length; i++) {
        Module['_free'](this.temps[i]);
      }
      this.temps.length = 0;
      // prepare to allocate a bigger buffer
      Module['_free'](this.buffer);
      this.buffer = 0;
      this.size += this.needed;
      // clean up
      this.needed = 0;
    }
    if (!this.buffer) { // happens first time, or when we need to grow
      this.size += 128; // heuristic, avoid many small grow events
      this.buffer = Module['_malloc'](this.size);
      assert(this.buffer);
    }
    this.pos = 0;
  },
  alloc: function(array, view) {
    assert(this.buffer);
    var bytes = view.BYTES_PER_ELEMENT;
    var len = array.length * bytes;
    len = (len + 7) & -8; // keep things aligned to 8 byte boundaries
    var ret;
    if (this.pos + len >= this.size) {
      // we failed to allocate in the buffer, this time around :(
      assert(len > 0); // null terminator, at least
      this.needed += len;
      ret = Module['_malloc'](len);
      this.temps.push(ret);
    } else {
      // we can allocate in the buffer
      ret = this.buffer + this.pos;
      this.pos += len;
    }
    var retShifted = ret;
    switch (bytes) {
      case 2: retShifted >>= 1; break;
      case 4: retShifted >>= 2; break;
      case 8: retShifted >>= 3; break;
    }
    for (var i = 0; i < array.length; i++) {
      view[retShifted + i] = array[i];
    }
    return ret;
  },
};

function ensureString(value) {
  if (typeof value === 'string') return ensureCache.alloc(intArrayFromString(value), HEAP8);
  return value;
}
function ensureInt8(value) {
  if (typeof value === 'object') return ensureCache.alloc(value, HEAP8);
  return value;
}
function ensureInt16(value) {
  if (typeof value === 'object') return ensureCache.alloc(value, HEAP16);
  return value;
}
function ensureInt32(value) {
  if (typeof value === 'object') return ensureCache.alloc(value, HEAP32);
  return value;
}
function ensureFloat32(value) {
  if (typeof value === 'object') return ensureCache.alloc(value, HEAPF32);
  return value;
}
function ensureFloat64(value) {
  if (typeof value === 'object') return ensureCache.alloc(value, HEAPF64);
  return value;
}

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

def full_typename(arg):
  return arg.type.name + ('[]' if arg.type.isArray() else '')

def type_to_c(t, non_pointing=False):
  #print 'to c ', t
  t = t.replace(' (Wrapper)', '')
  suffix = ''
  if '[]' in t:
    suffix = '*'
    t = t.replace('[]', '')
  if t == 'Long':
    ret = 'int'
  elif t == 'UnsignedLong':
    ret = 'unsigned int'
  elif t == 'Short':
    ret = 'short'
  elif t == 'UnsignedShort':
    ret = 'unsigned short'
  elif t == 'Byte':
    ret = 'char'
  elif t == 'Octet':
    ret = 'unsigned char'
  elif t == 'Void':
    ret = 'void'
  elif t == 'String':
    ret = 'char*'
  elif t == 'Float':
    ret = 'float'
  elif t == 'Double':
    ret = 'double'
  elif t == 'Boolean':
    ret = 'bool'
  elif t == 'Any' or t == 'VoidPtr':
    ret = 'void*'
  elif t in interfaces:
    ret = (interfaces[t].getExtendedAttribute('Prefix') or [''])[0] + t + ('' if non_pointing else '*')
  else:
    ret = t
  return ret + suffix

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

def render_function(class_name, func_name, sigs, return_type, non_pointer, copy, operator, constructor, func_scope, call_content=None, const=False, array_attribute=False):
  global mid_c, mid_js, js_impl_methods

  legacy_mode = CHECKS not in ['ALL', 'FAST']
  all_checks = CHECKS  == 'ALL'

  bindings_name = class_name + '_' + func_name
  min_args = min(sigs.keys())
  max_args = max(sigs.keys())

  c_names = {}
  all_args = sigs.get(max_args)

  if DEBUG:
    print 'renderfunc', class_name, func_name, sigs.keys(), return_type, constructor
    for i in range(max_args):
      a = all_args[i]
      if isinstance(a, WebIDL.IDLArgument):
        print ("  arg%d" % i), a.identifier, a.type, a.optional
      else:
        print "  arg%d" % i

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
    elif return_type == 'Boolean':
      call_prefix += '!!('
      call_postfix += ')'

  args = ['arg%d' % i for i in range(max_args)]
  if not constructor:
    body = '  var self = this.ptr;\n'
    pre_arg = ['self']
  else:
    body = ''
    pre_arg = []

  for i in range(max_args):
    arg = all_args[i]
    if arg.type.isString() or arg.type.isArray():
      body += '  ensureCache.prepare();\n'
      break

  full_name = "%s::%s" % (class_name, func_name)

  for i in range(max_args):
    if i >= min_args:
      optional = True
    else:
      optional = False
    arg = all_args[i]
    do_default = False
    # Filter out arguments we don't know how to parse. Fast casing only common cases.
    compatible_arg = isinstance(arg, Dummy) or (isinstance(arg, WebIDL.IDLArgument) and arg.optional is False)
    # note: null has typeof object, but is ok to leave as is, since we are calling into asm code where null|0 = 0
    if not legacy_mode and compatible_arg:
      if isinstance(arg, WebIDL.IDLArgument):
        arg_name = arg.identifier.name
      else:
        arg_name = ''
      # Format assert fail message
      check_msg = "[CHECK FAILED] %s(arg%d:%s): " % (full_name, i, arg_name)
      if isinstance(arg.type, WebIDL.IDLWrapperType):
        inner = arg.type.inner
      else:
        inner = ""

      # Print type info in comments.
      body += "  /* arg%d <%s> [%s] */\n" % (i, arg.type.name, inner)

      # Wrap asserts with existance check when argument is optional.
      if all_checks and optional: body += "if(typeof arg%d !== 'undefined' && arg%d !== null) {\n" % (i, i)
      # Special case argument types.
      if arg.type.isNumeric():
        if arg.type.isInteger():
          if all_checks: body += "  assert(typeof arg%d === 'number' && !isNaN(arg%d), '%sExpecting <integer>');\n" % (i, i, check_msg)
        else:
          if all_checks: body += "  assert(typeof arg%d === 'number', '%sExpecting <number>');\n" % (i, check_msg)
        # No transform needed for numbers
      elif arg.type.isBoolean():
        if all_checks: body += "  assert(typeof arg%d === 'boolean' || (typeof arg%d === 'number' && !isNaN(arg%d)), '%sExpecting <boolean>');\n" % (i, i, i, check_msg)
        # No transform needed for booleans
      elif arg.type.isString():
        # Strings can be DOM strings or pointers.
        if all_checks: body += "  assert(typeof arg%d === 'string' || (arg%d && typeof arg%d === 'object' && typeof arg%d.ptr === 'number'), '%sExpecting <string>');\n" % (i, i, i, i, check_msg)
        do_default = True # legacy path is fast enough for strings.
      elif arg.type.isInterface():
        if all_checks: body += "  assert(typeof arg%d === 'object' && typeof arg%d.ptr === 'number', '%sExpecting <pointer>');\n" % (i, i, check_msg)
        if optional:
          body += "  if(typeof arg%d !== 'undefined' && arg%d !== null) { arg%d = arg%d.ptr };\n" % (i, i, i, i)
        else:
          # No checks in fast mode when the arg is required
          body += "  arg%d = arg%d.ptr;\n" % (i, i)
      else:
        do_default = True

      if all_checks and optional: body += "}\n"
    else:
      do_default = True

    if do_default:
      if not (arg.type.isArray() and not array_attribute):
        body += "  if (arg%d && typeof arg%d === 'object') arg%d = arg%d.ptr;\n" % (i, i, i, i)
        if arg.type.isString():
          body += "  else arg%d = ensureString(arg%d);\n" % (i, i)
      else:
        # an array can be received here
        arg_type = arg.type.name
        if arg_type in ['Byte', 'Octet']:
          body += "  if (typeof arg%d == 'object') { arg%d = ensureInt8(arg%d); }\n" % (i, i, i)
        elif arg_type in ['Short', 'UnsignedShort']:
          body += "  if (typeof arg%d == 'object') { arg%d = ensureInt16(arg%d); }\n" % (i, i, i)
        elif arg_type in ['Long', 'UnsignedLong']:
          body += "  if (typeof arg%d == 'object') { arg%d = ensureInt32(arg%d); }\n" % (i, i, i)
        elif arg_type == 'Float':
          body += "  if (typeof arg%d == 'object') { arg%d = ensureFloat32(arg%d); }\n" % (i, i, i)
        elif arg_type == 'Double':
          body += "  if (typeof arg%d == 'object') { arg%d = ensureFloat64(arg%d); }\n" % (i, i, i)

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
    sig = map(full_typename, raw)
    if array_attribute:
      sig = map(lambda x: x.replace('[]', ''), sig) # for arrays, ignore that this is an array - our get/set methods operate on the elements

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
      cast_self = 'self'
      if class_name != func_scope:
        # this function comes from an ancestor class; for operators, we must cast it
        cast_self = 'dynamic_cast<' + type_to_c(func_scope) + '>(' + cast_self + ')'
      if '=' in operator:
        call = '(*%s %s %sarg0)' % (cast_self, operator, '*' if sig[0] in interfaces else '')
      elif operator == '[]':
        call = '((*%s)[%sarg0])' % (cast_self, '*' if sig[0] in interfaces else '')
      else:
        raise Exception('unfamiliar operator ' + operator)

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
      %sself['%s'](%s)%s;
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

# Compute the height in the inheritance tree of each node. Note that the order of interation
# of `implements` is irrelevant.
#
# After one iteration of the loop, all ancestors of child are guaranteed to have a a larger
# height number than the child, and this is recursively true for each ancestor. If the height
# of child is later increased, all its ancestors will be readjusted at that time to maintain
# that invariant. Further, the height of a node never decreases. Therefore, when the loop
# finishes, all ancestors of a given node should have a larger height number than that node.
nodeHeight = {}
for child, parent in implements.iteritems():
  parent = parent[0]
  while parent:
    nodeHeight[parent] = max(nodeHeight.get(parent, 0), nodeHeight.get(child, 0) + 1)
    grandParent = implements.get(parent)
    if grandParent:
      child = parent
      parent = grandParent[0]
    else:
      parent = None

names = interfaces.keys()
names.sort(lambda x, y: nodeHeight.get(y, 0) - nodeHeight.get(x, 0))

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
%s.prototype['%s'] = %s.prototype.%s = ''' % (name, m.identifier.name, name, m.identifier.name)]
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
  %s.prototype['%s'] = %s.prototype.%s = ''' % (name, get_name, name, get_name)]
    render_function(name,
                    get_name, get_sigs, m.type.name,
                    None,
                    None,
                    None,
                    False,
                    func_scope=interface,
                    call_content=get_call_content,
                    const=m.getExtendedAttribute('Const'),
                    array_attribute=m.type.isArray())

    if not m.readonly:
      set_name = 'set_' + attr
      mid_js += [r'''
    %s.prototype['%s'] = %s.prototype.%s = ''' % (name, set_name, name, set_name)]
      render_function(name,
                      set_name, set_sigs, 'Void',
                      None,
                      None,
                      None,
                      False,
                      func_scope=interface,
                      call_content=set_call_content,
                      const=m.getExtendedAttribute('Const'),
                      array_attribute=m.type.isArray())

  if not interface.getExtendedAttribute('NoDelete'):
    mid_js += [r'''
  %s.prototype['__destroy__'] = %s.prototype.__destroy__ = ''' % (name, name)]
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
      raise Exception("Illegal enum value %s" % value)

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

