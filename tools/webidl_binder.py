# Copyright 2014 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""WebIDL binder

https://emscripten.org/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html
"""

import argparse
import os
import sys
from typing import List

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import utils

sys.path.append(utils.path_from_root('third_party'))
sys.path.append(utils.path_from_root('third_party/ply'))

import WebIDL

# CHECKS='FAST' will skip most argument type checks in the wrapper methods for
#                  performance (~3x faster than default).
# CHECKS='ALL' will do extensive argument type checking (~5x slower than default).
#                 This will catch invalid numbers, invalid pointers, invalid strings, etc.
# Anything else defaults to legacy mode for backward compatibility.
CHECKS = os.environ.get('IDL_CHECKS', 'DEFAULT')
# DEBUG=1 will print debug info in render_function
DEBUG = os.environ.get('IDL_VERBOSE') == '1'


def dbg(*args):
  if DEBUG:
    print(*args, file=sys.stderr)


dbg(f'Debug print ON, CHECKS=${CHECKS}')

# We need to avoid some closure errors on the constructors we define here.
CONSTRUCTOR_CLOSURE_SUPPRESSIONS = '/** @suppress {undefinedVars, duplicate} @this{Object} */'


class Dummy:
  def __init__(self, type):
    self.type = type

  def __repr__(self):
    return f'<Dummy type:{self.type}>'

  def getExtendedAttribute(self, _name):
    return None


parser = argparse.ArgumentParser()
parser.add_argument('--wasm64', action='store_true', default=False,
                    help='Build for wasm64')
parser.add_argument('infile')
parser.add_argument('outfile')
options = parser.parse_args()

input_file = options.infile
output_base = options.outfile
cpp_output = output_base + '.cpp'
js_output = output_base + '.js'

utils.delete_file(cpp_output)
utils.delete_file(js_output)

p = WebIDL.Parser()
p.parse('''
interface VoidPtr {
};
''' + utils.read_file(input_file))
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

# print interfaces
# print implements

pre_c = ['''
#include <emscripten.h>
#include <stdlib.h>

EM_JS_DEPS(webidl_binder, "$intArrayFromString,$UTF8ToString,$alignMemory");
''']

mid_c = ['''
extern "C" {

// Define custom allocator functions that we can force export using
// EMSCRIPTEN_KEEPALIVE.  This avoids all webidl users having to add
// malloc/free to -sEXPORTED_FUNCTIONS.
EMSCRIPTEN_KEEPALIVE void webidl_free(void* p) { free(p); }
EMSCRIPTEN_KEEPALIVE void* webidl_malloc(size_t len) { return malloc(len); }

''']


def build_constructor(name):
  implementing_name = implements[name][0] if implements.get(name) else 'WrapperObject'
  return [r'''{name}.prototype = Object.create({implementing}.prototype);
{name}.prototype.constructor = {name};
{name}.prototype.__class__ = {name};
{name}.__cache__ = {{}};
Module['{name}'] = {name};
'''.format(name=name, implementing=implementing_name)]


mid_js = ['''
// Bindings utilities

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function WrapperObject() {
}
''']

mid_js += build_constructor('WrapperObject')

mid_js += ['''
/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant)
    @param {*=} __class__ */
function getCache(__class__) {
  return (__class__ || WrapperObject).__cache__;
}
Module['getCache'] = getCache;

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant)
    @param {*=} __class__ */
function wrapPointer(ptr, __class__) {
  var cache = getCache(__class__);
  var ret = cache[ptr];
  if (ret) return ret;
  ret = Object.create((__class__ || WrapperObject).prototype);
  ret.ptr = ptr;
  return cache[ptr] = ret;
}
Module['wrapPointer'] = wrapPointer;

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function castObject(obj, __class__) {
  return wrapPointer(obj.ptr, __class__);
}
Module['castObject'] = castObject;

Module['NULL'] = wrapPointer(0);

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function destroy(obj) {
  if (!obj['__destroy__']) throw 'Error: Cannot destroy object. (Did you create it yourself?)';
  obj['__destroy__']();
  // Remove from cache, so the object can be GC'd and refs added onto it released
  delete getCache(obj.__class__)[obj.ptr];
}
Module['destroy'] = destroy;

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function compare(obj1, obj2) {
  return obj1.ptr === obj2.ptr;
}
Module['compare'] = compare;

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function getPointer(obj) {
  return obj.ptr;
}
Module['getPointer'] = getPointer;

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function getClass(obj) {
  return obj.__class__;
}
Module['getClass'] = getClass;

// Converts big (string or array) values into a C-style storage, in temporary space

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
var ensureCache = {
  buffer: 0,  // the main buffer of temporary storage
  size: 0,   // the size of buffer
  pos: 0,    // the next free offset in buffer
  temps: [], // extra allocations
  needed: 0, // the total size we need next time

  prepare() {
    if (ensureCache.needed) {
      // clear the temps
      for (var i = 0; i < ensureCache.temps.length; i++) {
        Module['_webidl_free'](ensureCache.temps[i]);
      }
      ensureCache.temps.length = 0;
      // prepare to allocate a bigger buffer
      Module['_webidl_free'](ensureCache.buffer);
      ensureCache.buffer = 0;
      ensureCache.size += ensureCache.needed;
      // clean up
      ensureCache.needed = 0;
    }
    if (!ensureCache.buffer) { // happens first time, or when we need to grow
      ensureCache.size += 128; // heuristic, avoid many small grow events
      ensureCache.buffer = Module['_webidl_malloc'](ensureCache.size);
      assert(ensureCache.buffer);
    }
    ensureCache.pos = 0;
  },
  alloc(array, view) {
    assert(ensureCache.buffer);
    var bytes = view.BYTES_PER_ELEMENT;
    var len = array.length * bytes;
    len = alignMemory(len, 8); // keep things aligned to 8 byte boundaries
    var ret;
    if (ensureCache.pos + len >= ensureCache.size) {
      // we failed to allocate in the buffer, ensureCache time around :(
      assert(len > 0); // null terminator, at least
      ensureCache.needed += len;
      ret = Module['_webidl_malloc'](len);
      ensureCache.temps.push(ret);
    } else {
      // we can allocate in the buffer
      ret = ensureCache.buffer + ensureCache.pos;
      ensureCache.pos += len;
    }
    return ret;
  },
  copy(array, view, offset) {
    offset /= view.BYTES_PER_ELEMENT;
    for (var i = 0; i < array.length; i++) {
      view[offset + i] = array[i];
    }
  },
};

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureString(value) {
  if (typeof value === 'string') {
    var intArray = intArrayFromString(value);
    var offset = ensureCache.alloc(intArray, HEAP8);
    ensureCache.copy(intArray, HEAP8, offset);
    return offset;
  }
  return value;
}

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureInt8(value) {
  if (typeof value === 'object') {
    var offset = ensureCache.alloc(value, HEAP8);
    ensureCache.copy(value, HEAP8, offset);
    return offset;
  }
  return value;
}

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureInt16(value) {
  if (typeof value === 'object') {
    var offset = ensureCache.alloc(value, HEAP16);
    ensureCache.copy(value, HEAP16, offset);
    return offset;
  }
  return value;
}

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureInt32(value) {
  if (typeof value === 'object') {
    var offset = ensureCache.alloc(value, HEAP32);
    ensureCache.copy(value, HEAP32, offset);
    return offset;
  }
  return value;
}

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureFloat32(value) {
  if (typeof value === 'object') {
    var offset = ensureCache.alloc(value, HEAPF32);
    ensureCache.copy(value, HEAPF32, offset);
    return offset;
  }
  return value;
}

/** @suppress {duplicate} (TODO: avoid emitting this multiple times, it is redundant) */
function ensureFloat64(value) {
  if (typeof value === 'object') {
    var offset = ensureCache.alloc(value, HEAPF64);
    ensureCache.copy(value, HEAPF64, offset);
    return offset;
  }
  return value;
}
''']

C_FLOATS = ['float', 'double']


def full_typename(arg):
  return ('const ' if arg.getExtendedAttribute('Const') else '') + arg.type.name + ('[]' if arg.type.isArray() else '')


def type_to_c(t, non_pointing=False):
  # print 'to c ', t
  def base_type_to_c(t):
    if t == 'Long':
      return 'int'
    elif t == 'UnsignedLong':
      return 'unsigned int'
    elif t == 'LongLong':
      return 'long long'
    elif t == 'UnsignedLongLong':
      return 'unsigned long long'
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
    elif t in ('Any', 'VoidPtr'):
      return 'void*'
    elif t in interfaces:
      return (interfaces[t].getExtendedAttribute('Prefix') or [''])[0] + t + ('' if non_pointing else '*')
    else:
      return t

  t = t.replace(' (Wrapper)', '')

  prefix = ''
  suffix = ''
  if '[]' in t:
    t = t.replace('[]', '')
    suffix = '*'
  if 'const ' in t:
    t = t.replace('const ', '')
    prefix = 'const '
  return prefix + base_type_to_c(t) + suffix


def take_addr_if_nonpointer(m):
  if m.getExtendedAttribute('Ref') or m.getExtendedAttribute('Value'):
    return '&'
  return ''


def deref_if_nonpointer(m):
  if m.getExtendedAttribute('Ref') or m.getExtendedAttribute('Value'):
    return '*'
  return ''


def type_to_cdec(raw):
  ret = type_to_c(full_typename(raw), non_pointing=True)
  if raw.type.name not in interfaces:
    return ret
  if raw.getExtendedAttribute('Ref'):
    return ret + '&'
  if raw.getExtendedAttribute('Value'):
    return ret
  return ret + '*'


def render_function(class_name, func_name, sigs, return_type, non_pointer,
                    copy, operator, constructor, is_static, func_scope,
                    call_content=None, const=False, array_attribute=False):
  legacy_mode = CHECKS not in ['ALL', 'FAST']
  all_checks = CHECKS == 'ALL'

  bindings_name = class_name + '_' + func_name
  min_args = min(sigs.keys())
  max_args = max(sigs.keys())

  all_args = sigs.get(max_args)

  if DEBUG:
    dbg('renderfunc', class_name, func_name, list(sigs.keys()), return_type, constructor)
    for i, a in enumerate(all_args):
      if isinstance(a, WebIDL.IDLArgument):
        dbg('  ', a.identifier.name, a.identifier, a.type, a.optional)
      else:
        dbg('  arg%d (%s)' % (i, a))

  # JS

  cache = ('getCache(%s)[this.ptr] = this;' % class_name) if constructor else ''
  call_prefix = ''
  if constructor:
    call_prefix += 'this.ptr = '
  call_postfix = ''
  if return_type != 'Void' and not constructor:
    call_prefix = 'return '

  ptr_rtn = constructor or return_type in interfaces or return_type == 'String'
  if options.wasm64 and ptr_rtn:
    call_postfix += ')'

  if not constructor:
    if return_type in interfaces:
      call_prefix += 'wrapPointer('
      call_postfix += ', ' + return_type + ')'
    elif return_type == 'String':
      call_prefix += 'UTF8ToString('
      call_postfix += ')'
    elif return_type == 'Boolean':
      call_prefix += '!!('
      call_postfix += ')'

  if options.wasm64 and ptr_rtn:
    call_prefix += 'Number('

  args = [(all_args[i].identifier.name if isinstance(all_args[i], WebIDL.IDLArgument) else ('arg%d' % i)) for i in range(max_args)]
  if not constructor and not is_static:
    body = '  var self = this.ptr;\n'
    if options.wasm64:
      pre_arg = ['BigInt(self)']
    else:
      pre_arg = ['self']
  else:
    body = ''
    pre_arg = []

  if any(arg.type.isString() or arg.type.isArray() for arg in all_args):
    body += '  ensureCache.prepare();\n'

  def is_ptr_arg(i):
    t = all_args[i].type
    return (t.isArray() or t.isAny() or t.isString() or t.isObject() or t.isInterface())

  for i, (js_arg, arg) in enumerate(zip(args, all_args)):
    if i >= min_args:
      optional = True
    else:
      optional = False
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
      check_msg = "[CHECK FAILED] %s::%s(%s:%s): " % (class_name, func_name, js_arg, arg_name)
      if isinstance(arg.type, WebIDL.IDLWrapperType):
        inner = arg.type.inner
      else:
        inner = ""

      # Print type info in comments.
      body += "  /* %s <%s> [%s] */\n" % (js_arg, arg.type.name, inner)

      # Wrap asserts with existence check when argument is optional.
      if all_checks and optional:
        body += "if(typeof {0} !== 'undefined' && {0} !== null) {{\n".format(js_arg)
      # Special case argument types.
      if arg.type.isNumeric():
        if arg.type.isInteger():
          if all_checks:
            body += "  assert(typeof {0} === 'number' && !isNaN({0}), '{1}Expecting <integer>');\n".format(js_arg, check_msg)
        else:
          if all_checks:
            body += "  assert(typeof {0} === 'number', '{1}Expecting <number>');\n".format(js_arg, check_msg)
        # No transform needed for numbers
      elif arg.type.isBoolean():
        if all_checks:
          body += "  assert(typeof {0} === 'boolean' || (typeof {0} === 'number' && !isNaN({0})), '{1}Expecting <boolean>');\n".format(js_arg, check_msg)
        # No transform needed for booleans
      elif arg.type.isString():
        # Strings can be DOM strings or pointers.
        if all_checks:
          body += "  assert(typeof {0} === 'string' || ({0} && typeof {0} === 'object' && typeof {0}.ptr === 'number'), '{1}Expecting <string>');\n".format(js_arg, check_msg)
        do_default = True # legacy path is fast enough for strings.
      elif arg.type.isInterface():
        if all_checks:
          body += "  assert(typeof {0} === 'object' && typeof {0}.ptr === 'number', '{1}Expecting <pointer>');\n".format(js_arg, check_msg)
        if optional:
          body += "  if(typeof {0} !== 'undefined' && {0} !== null) {{ {0} = {0}.ptr }};\n".format(js_arg)
        else:
          # No checks in fast mode when the arg is required
          body += "  {0} = {0}.ptr;\n".format(js_arg)
      else:
        do_default = True

      if all_checks and optional:
        body += "}\n"
    else:
      do_default = True

    if do_default:
      if not (arg.type.isArray() and not array_attribute):
        body += f"  if ({js_arg} && typeof {js_arg} === 'object') {js_arg} = {js_arg}.ptr;\n"
        if arg.type.isString():
          body += "  else {0} = ensureString({0});\n".format(js_arg)
        if options.wasm64 and is_ptr_arg(i):
          body += f'  if ({args[i]} === null) {args[i]} = 0;\n'
      else:
        # an array can be received here
        arg_type = arg.type.name
        if arg_type in ['Byte', 'Octet']:
          body += "  if (typeof {0} == 'object') {{ {0} = ensureInt8({0}); }}\n".format(js_arg)
        elif arg_type in ['Short', 'UnsignedShort']:
          body += "  if (typeof {0} == 'object') {{ {0} = ensureInt16({0}); }}\n".format(js_arg)
        elif arg_type in ['Long', 'UnsignedLong']:
          body += "  if (typeof {0} == 'object') {{ {0} = ensureInt32({0}); }}\n".format(js_arg)
        elif arg_type == 'Float':
          body += "  if (typeof {0} == 'object') {{ {0} = ensureFloat32({0}); }}\n".format(js_arg)
        elif arg_type == 'Double':
          body += "  if (typeof {0} == 'object') {{ {0} = ensureFloat64({0}); }}\n".format(js_arg)

  call_args = pre_arg.copy()

  for i, arg in enumerate(args):
    if options.wasm64 and is_ptr_arg(i):
      arg = f'BigInt({arg})'
    call_args.append(arg)

  c_names = {}

  def make_call_args(i):
    if pre_arg:
      i += 1
    return ', '.join(call_args[:i])

  for i in range(min_args, max_args):
    c_names[i] = f'emscripten_bind_{bindings_name}_{i}'
    if 'return ' in call_prefix:
      after_call = ''
    else:
      after_call = '; ' + cache + 'return'
    args_for_call = make_call_args(i)
    body += '  if (%s === undefined) { %s_%s(%s)%s%s }\n' % (args[i], call_prefix, c_names[i],
                                                             args_for_call,
                                                             call_postfix, after_call)
  dbg(call_prefix)
  c_names[max_args] = f'emscripten_bind_{bindings_name}_{max_args}'
  args_for_call = make_call_args(len(args))
  body += '  %s_%s(%s)%s;\n' % (call_prefix, c_names[max_args], args_for_call, call_postfix)
  if cache:
    body += f'  {cache}\n'

  if constructor:
    declare_name = ' ' + func_name
  else:
    declare_name = ''
  mid_js.append(r'''function%s(%s) {
%s
};
''' % (declare_name, ', '.join(args), body[:-1]))

  # C

  for i in range(min_args, max_args + 1):
    raw = sigs.get(i)
    if raw is None:
      continue
    sig = list(map(full_typename, raw))
    if array_attribute:
      # for arrays, ignore that this is an array - our get/set methods operate on the elements
      sig = [x.replace('[]', '') for x in sig]

    c_arg_types = list(map(type_to_c, sig))
    c_class_name = type_to_c(class_name, non_pointing=True)

    normal_args = ', '.join(['%s %s' % (c_arg_types[j], args[j]) for j in range(i)])
    if constructor or is_static:
      full_args = normal_args
    else:
      full_args = c_class_name + '* self'
      if normal_args:
        full_args += ', ' + normal_args
    call_args = ', '.join(['%s%s' % ('*' if raw[j].getExtendedAttribute('Ref') else '', args[j]) for j in range(i)])
    if constructor:
      call = 'new ' + c_class_name + '(' + call_args + ')'
    elif call_content is not None:
      call = call_content
    else:
      call = func_name + '(' + call_args + ')'
      if is_static:

        call = c_class_name + '::' + call
      else:
        call = 'self->' + call

    if operator:
      cast_self = 'self'
      if class_name != func_scope:
        # this function comes from an ancestor class; for operators, we must cast it
        cast_self = 'dynamic_cast<' + type_to_c(func_scope) + '>(' + cast_self + ')'
      maybe_deref = deref_if_nonpointer(raw[0])
      if '=' in operator:
        call = '(*%s %s %s%s)' % (cast_self, operator, maybe_deref, args[0])
      elif operator == '[]':
        call = '((*%s)[%s%s])' % (cast_self, maybe_deref, args[0])
      else:
        raise Exception('unfamiliar operator ' + operator)

    pre = ''

    basic_return = 'return ' if constructor or return_type != 'Void' else ''
    return_prefix = basic_return
    return_postfix = ''
    if non_pointer:
      return_prefix += '&'
    if copy:
      pre += '  static %s temp;\n' % type_to_c(return_type, non_pointing=True)
      return_prefix += '(temp = '
      return_postfix += ', &temp)'

    c_return_type = type_to_c(return_type)
    maybe_const = 'const ' if const else ''
    mid_c.append(r'''
%s%s EMSCRIPTEN_KEEPALIVE %s(%s) {
%s  %s%s%s;
}
''' % (maybe_const, type_to_c(class_name) if constructor else c_return_type, c_names[i], full_args, pre, return_prefix, call, return_postfix))

    if not constructor:
      if i == max_args:
        dec_args = ', '.join([type_to_cdec(raw[j]) + ' ' + args[j] for j in range(i)])
        js_call_args = ', '.join(['%s%s' % (('(ptrdiff_t)' if sig[j] in interfaces else '') + take_addr_if_nonpointer(raw[j]), args[j]) for j in range(i)])
        em_asm_macro = 'EM_ASM_%s' % ('PTR' if c_return_type[-1] == '*' else 'INT' if c_return_type not in C_FLOATS else 'DOUBLE')

        js_impl_methods.append(r'''  %s %s(%s) %s {
    %s (%s) %s({
      var self = Module['getCache'](Module['%s'])[$0];
      if (!self.hasOwnProperty('%s')) throw 'a JSImplementation must implement all functions, you forgot %s::%s.';
      %sself['%s'](%s)%s;
    }, (ptrdiff_t)this%s);
  }''' % (c_return_type, func_name, dec_args, maybe_const,
          basic_return, c_return_type, em_asm_macro,
          class_name,
          func_name, class_name, func_name,
          return_prefix,
          func_name,
          ','.join(['$%d' % i for i in range(1, max_args + 1)]),
          return_postfix,
          (', ' if js_call_args else '') + js_call_args))


def add_bounds_check_impl():
  if hasattr(add_bounds_check_impl, 'done'):
    return
  add_bounds_check_impl.done = True
  mid_c.append('''
EM_JS(void, array_bounds_check_error, (size_t idx, size_t size), {
  throw 'Array index ' + idx + ' out of bounds: [0,' + size + ')';
});

static void array_bounds_check(size_t array_size, size_t array_idx) {
  if (array_idx < 0 || array_idx >= array_size) {
    array_bounds_check_error(array_idx, array_size);
  }
}
''')


for name, interface in interfaces.items():
  js_impl = interface.getExtendedAttribute('JSImplementation')
  if not js_impl:
    continue
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
for child, parent in implements.items():
  parent = parent[0]
  while parent:
    nodeHeight[parent] = max(nodeHeight.get(parent, 0), nodeHeight.get(child, 0) + 1)
    grandParent = implements.get(parent)
    if grandParent:
      child = parent
      parent = grandParent[0]
    else:
      parent = None

names = sorted(interfaces.keys(), key=lambda x: nodeHeight.get(x, 0), reverse=True)

for name in names:
  interface = interfaces[name]

  mid_js += ['\n// Interface: ' + name + '\n\n']
  mid_c += ['\n// Interface: ' + name + '\n\n']

  js_impl_methods: List[str] = []

  cons = interface.getExtendedAttribute('Constructor')
  if type(cons) is list:
    raise Exception('do not use "Constructor", instead create methods with the name of the interface')

  js_impl = interface.getExtendedAttribute('JSImplementation')
  if js_impl:
    js_impl = js_impl[0]

  # Methods

  # Ensure a constructor even if one is not specified.
  if not any(m.identifier.name == name for m in interface.members):
    mid_js += ['%s\nfunction %s() { throw "cannot construct a %s, no constructor in IDL" }\n' % (CONSTRUCTOR_CLOSURE_SUPPRESSIONS, name, name)]
    mid_js += build_constructor(name)

  for m in interface.members:
    if not m.isMethod():
      continue
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
    mid_js += [CONSTRUCTOR_CLOSURE_SUPPRESSIONS, '\n']
    if not constructor:
      mid_js += ["%s.prototype['%s'] = %s.prototype.%s = " % (name, m.identifier.name, name, m.identifier.name)]
    sigs = {}
    return_type = None
    for ret, args in m.signatures():
      if return_type is None:
        return_type = ret.name
      else:
        assert return_type == ret.name, 'overloads must have the same return type'
      for i in range(len(args) + 1):
        if i == len(args) or args[i].optional:
          assert i not in sigs, 'overloading must differentiate by # of arguments (cannot have two signatures that differ by types but not by length)'
          sigs[i] = args[:i]
    render_function(name,
                    m.identifier.name, sigs, return_type,
                    m.getExtendedAttribute('Ref'),
                    m.getExtendedAttribute('Value'),
                    (m.getExtendedAttribute('Operator') or [None])[0],
                    constructor,
                    is_static=m.isStatic(),
                    func_scope=m.parentScope.identifier.name,
                    const=m.getExtendedAttribute('Const'))
    mid_js += ['\n']
    if constructor:
      mid_js += build_constructor(name)

  for m in interface.members:
    if not m.isAttr():
      continue
    attr = m.identifier.name

    if m.type.isArray():
      get_sigs = {1: [Dummy(type=WebIDL.BuiltinTypes[WebIDL.IDLBuiltinType.Types.long])]}
      set_sigs = {2: [Dummy(type=WebIDL.BuiltinTypes[WebIDL.IDLBuiltinType.Types.long]),
                      Dummy(type=m.type.inner)]}
      get_call_content = take_addr_if_nonpointer(m) + 'self->' + attr + '[arg0]'
      set_call_content = 'self->' + attr + '[arg0] = ' + deref_if_nonpointer(m) + 'arg1'
      if m.getExtendedAttribute('BoundsChecked'):

        bounds_check = "array_bounds_check(sizeof(self->%s) / sizeof(self->%s[0]), arg0)" % (attr, attr)
        add_bounds_check_impl()

        get_call_content = "(%s, %s)" % (bounds_check, get_call_content)
        set_call_content = "(%s, %s)" % (bounds_check, set_call_content)
    else:
      get_sigs = {0: []}
      set_sigs = {1: [Dummy(type=m.type)]}
      get_call_content = take_addr_if_nonpointer(m) + 'self->' + attr
      set_call_content = 'self->' + attr + ' = ' + deref_if_nonpointer(m) + 'arg0'

    get_name = 'get_' + attr
    mid_js += [r'''%s
%s.prototype['%s'] = %s.prototype.%s = ''' % (CONSTRUCTOR_CLOSURE_SUPPRESSIONS, name, get_name, name, get_name)]
    render_function(name,
                    get_name, get_sigs, m.type.name,
                    None,
                    None,
                    None,
                    False,
                    False,
                    func_scope=interface,
                    call_content=get_call_content,
                    const=m.getExtendedAttribute('Const'),
                    array_attribute=m.type.isArray())

    if m.readonly:
      mid_js += [r'''
/** @suppress {checkTypes} */
Object.defineProperty(%s.prototype, '%s', { get: %s.prototype.%s });
''' % (name, attr, name, get_name)]
    else:
      set_name = 'set_' + attr
      mid_js += [r'''
%s
%s.prototype['%s'] = %s.prototype.%s = ''' % (CONSTRUCTOR_CLOSURE_SUPPRESSIONS, name, set_name, name, set_name)]
      render_function(name,
                      set_name, set_sigs, 'Void',
                      None,
                      None,
                      None,
                      False,
                      False,
                      func_scope=interface,
                      call_content=set_call_content,
                      const=m.getExtendedAttribute('Const'),
                      array_attribute=m.type.isArray())
      mid_js += [r'''
/** @suppress {checkTypes} */
Object.defineProperty(%s.prototype, '%s', { get: %s.prototype.%s, set: %s.prototype.%s });
''' % (name, attr, name, get_name, name, set_name)]

  if not interface.getExtendedAttribute('NoDelete'):
    mid_js += [r'''
%s
%s.prototype['__destroy__'] = %s.prototype.__destroy__ = ''' % (CONSTRUCTOR_CLOSURE_SUPPRESSIONS, name, name)]
    render_function(name,
                    '__destroy__', {0: []}, 'Void',
                    None,
                    None,
                    None,
                    False,
                    False,
                    func_scope=interface,
                    call_content='delete self')

  # Emit C++ class implementation that calls into JS implementation

  if js_impl:
    pre_c += ['''
class %s : public %s {
public:
%s
};
''' % (name, type_to_c(js_impl, non_pointing=True), '\n'.join(js_impl_methods))]

deferred_js = []

for name, enum in enums.items():
  mid_c += [f'\n// ${name}\n']
  deferred_js += [f'\n// ${name}\n']
  for value in enum.values():
    function_id = '%s_%s' % (name, value.split('::')[-1])
    function_id = 'emscripten_enum_%s' % function_id
    mid_c += ['''%s EMSCRIPTEN_KEEPALIVE %s() {
  return %s;
}
''' % (name, function_id, value)]
    symbols = value.split('::')
    if len(symbols) == 1:
      identifier = symbols[0]
      deferred_js += ["Module['%s'] = _%s();\n" % (identifier, function_id)]
    elif len(symbols) == 2:
      [namespace, identifier] = symbols
      if namespace in interfaces:
        # namespace is a class
        deferred_js += ["Module['%s']['%s'] = _%s();\n" % (namespace, identifier, function_id)]
      else:
        # namespace is a namespace, so the enums get collapsed into the top level namespace.
        deferred_js += ["Module['%s'] = _%s();\n" % (identifier, function_id)]
    else:
      raise Exception(f'Illegal enum value ${value}')

mid_c += ['\n}\n\n']
if len(deferred_js):
  mid_js += ['''
(function() {
  function setupEnums() {
    %s
  }
  if (runtimeInitialized) setupEnums();
  else addOnInit(setupEnums);
})();
''' % '\n    '.join(deferred_js)]

# Write

with open(cpp_output, 'w') as c:
  for x in pre_c:
    c.write(x)
  for x in mid_c:
    c.write(x)

with open(js_output, 'w') as js:
  for x in mid_js:
    js.write(x)
