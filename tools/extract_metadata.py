# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from . import webassembly
from .webassembly import OpCode, AtomicOpCode, MemoryOpCode
from .shared import exit_with_error
from .settings import settings


def skip_function_header(module):
  num_local_decls = module.read_uleb()
  while num_local_decls:
    local_count = module.read_uleb()  # noqa
    local_type = module.read_type()  # noqa
    num_local_decls -= 1


def is_wrapper_function(module, function):
  module.seek(function.offset)
  skip_function_header(module)
  end = function.offset + function.size
  while module.tell() != end:
    opcode = module.read_byte()
    try:
      opcode = OpCode(opcode)
    except ValueError as e:
      print(e)
      return False
    if opcode == OpCode.CALL:
      callee = module.read_uleb()  # noqa
    elif opcode == OpCode.END:
      break
    else:
      return False
  assert opcode == OpCode.END
  return True


def get_const_expr_value(expr):
  assert len(expr) == 2
  assert expr[1][0] == OpCode.END
  opcode, immediates = expr[0]
  if opcode in (OpCode.I32_CONST, OpCode.I64_CONST):
    assert len(immediates) == 1
    return immediates[0]
  elif opcode in (OpCode.GLOBAL_GET,):
    return 0
  else:
    exit_with_error('unexpected opcode in const expr: ' + str(opcode))


def get_global_value(globl):
  return get_const_expr_value(globl.init)


def parse_function_for_memory_inits(module, func_index, offset_map):
  """Very limited function parser that uses `memory.init` instructions
  to derive segment offset.

  When segments are passive they don't have an offset but (at least with
  llvm-generated code) are loaded during the start function
  (`__wasm_init_memory`) using `memory.init` instructions.

  Here we parse the `__wasm_init_memory` function and make many assumptions
  about its layout.  For example, we assume the first argument to `memory.init`
  is either an `i32.const` or the result of an `i32.add`.
  """
  segments = module.get_segments()
  func = module.get_function(func_index)
  module.seek(func.offset)
  skip_function_header(module)
  end = func.offset + func.size
  const_values = []
  call_targets = []
  while module.tell() != end:
    opcode = OpCode(module.read_byte())
    if opcode in (OpCode.END, OpCode.NOP, OpCode.DROP, OpCode.I32_ADD):
      pass
    elif opcode in (OpCode.BLOCK,):
      module.read_type()
    elif opcode in (OpCode.I32_CONST, OpCode.I64_CONST):
      const_values.append(module.read_uleb())
    elif opcode in (OpCode.GLOBAL_SET, OpCode.BR, OpCode.GLOBAL_GET, OpCode.LOCAL_SET, OpCode.LOCAL_GET, OpCode.LOCAL_TEE):
      module.read_uleb()
    elif opcode == OpCode.CALL:
      call_targets.append(module.read_uleb())
    elif opcode == OpCode.MEMORY_PREFIX:
      opcode = MemoryOpCode(module.read_byte())
      if opcode == MemoryOpCode.MEMORY_INIT:
        segment_idx = module.read_uleb()
        segment = segments[segment_idx]
        offset = const_values[-3]
        offset_map[segment] = offset
        memory = module.read_uleb()
        assert memory == 0
      elif opcode == MemoryOpCode.MEMORY_FILL:
        memory = module.read_uleb() # noqa
        assert memory == 0
      elif opcode == MemoryOpCode.MEMORY_DROP:
        segment = module.read_uleb() # noqa
      else:
        assert False, "unknown: %s" % opcode
    elif opcode == OpCode.ATOMIC_PREFIX:
      opcode = AtomicOpCode(module.read_byte())
      if opcode in (AtomicOpCode.ATOMIC_I32_RMW_CMPXCHG, AtomicOpCode.ATOMIC_I32_STORE,
                    AtomicOpCode.ATOMIC_NOTIFY, AtomicOpCode.ATOMIC_WAIT32,
                    AtomicOpCode.ATOMIC_WAIT64):
        module.read_uleb()
        module.read_uleb()
      else:
        assert False, "unknown: %s" % opcode
    elif opcode == OpCode.BR_TABLE:
      count = module.read_uleb()
      for _ in range(count):
        depth = module.read_uleb() # noqa
      default = module.read_uleb() # noqa
    else:
      assert False, "unknown: %s" % opcode

  # Recursion is safe here because the layout of the wasm-ld-generated
  # start function has a specific structure and has at most on level
  # of call stack depth.
  for t in call_targets:
    parse_function_for_memory_inits(module, t, offset_map)


@webassembly.memoize
def get_passive_segment_offsets(module):
  start_func_index = module.get_start()
  assert start_func_index is not None
  offset_map = {}
  parse_function_for_memory_inits(module, start_func_index, offset_map)
  return offset_map


def find_segment_with_address(module, address):
  segments = module.get_segments()
  active = [s for s in segments if s.init]

  for seg in active:
    offset = get_const_expr_value(seg.init)
    if offset is None:
      continue
    if address >= offset and address < offset + seg.size:
      return (seg, address - offset)

  passive = [s for s in segments if not s.init]
  if passive:
    offset_map = get_passive_segment_offsets(module)
    for seg, offset in offset_map.items():
      if address >= offset and address < offset + seg.size:
        return (seg, address - offset)

  raise AssertionError('unable to find segment for address: %s' % address)


def data_to_string(data):
  data = data.decode('utf8')
  # We have at least one test (test/utf8.cpp) that uses a double
  # backslash in the C++ source code, in order to represent a single backslash.
  # This is because these strings historically were written and read back via
  # JSON and a single slash is interpreted as an escape char there.
  # Technically this escaping is no longer needed and could be removed
  # but in order to maintain compatibility we strip out the double
  # slashes here.
  data = data.replace('\\\\', '\\')
  return data


def get_asm_strings(module, export_map):
  if '__start_em_asm' not in export_map or '__stop_em_asm' not in export_map:
    return {}

  start = export_map['__start_em_asm']
  end = export_map['__stop_em_asm']
  start_global = module.get_global(start.index)
  end_global = module.get_global(end.index)
  start_addr = get_global_value(start_global)
  end_addr = get_global_value(end_global)

  seg = find_segment_with_address(module, start_addr)
  if not seg:
    exit_with_error('unable to find segment starting at __start_em_asm: %s' % start_addr)
  seg, seg_offset = seg

  asm_strings = {}
  str_start = seg_offset
  data = module.read_at(seg.offset, seg.size)
  size = end_addr - start_addr
  end = seg_offset + size
  while str_start < end:
    str_end = data.find(b'\0', str_start)
    asm_strings[str(start_addr - seg_offset + str_start)] = data_to_string(data[str_start:str_end])
    str_start = str_end + 1
  return asm_strings


def get_main_reads_params(module, export_map):
  if settings.STANDALONE_WASM:
    return 1

  main = export_map.get('main') or export_map.get('__main_argc_argv')
  if not main or main.kind != webassembly.ExternType.FUNC:
    return 0

  main_func = module.get_function(main.index)
  if is_wrapper_function(module, main_func):
    return 0
  else:
    return 1


def get_named_globals(module, exports):
  named_globals = {}
  for export in exports:
    if export.kind == webassembly.ExternType.GLOBAL:
      if export.name in ('__start_em_asm', '__stop_em_asm') or export.name.startswith('__em_js__'):
        continue
      g = module.get_global(export.index)
      named_globals[export.name] = str(get_global_value(g))
  return named_globals


def update_metadata(filename, metadata):
  declares = []
  invoke_funcs = []
  em_js_funcs = set(metadata['emJsFuncs'])
  with webassembly.Module(filename) as module:
    for i in module.get_imports():
      if i.kind == webassembly.ExternType.FUNC:
        if i.field.startswith('invoke_'):
          invoke_funcs.append(i.field)
        elif i.field not in em_js_funcs:
          declares.append(i.field)

    exports = [e.name for e in module.get_exports() if e.kind in [webassembly.ExternType.FUNC, webassembly.ExternType.TAG]]
  metadata['declares'] = declares
  metadata['exports'] = exports
  metadata['invokeFuncs'] = invoke_funcs


def get_string_at(module, address):
  seg, offset = find_segment_with_address(module, address)
  data = module.read_at(seg.offset, seg.size)
  str_end = data.find(b'\0', offset)
  return data_to_string(data[offset:str_end])


def extract_metadata(filename):
  export_names = []
  declares = []
  invoke_funcs = []
  global_imports = []
  em_js_funcs = {}

  with webassembly.Module(filename) as module:
    exports = module.get_exports()
    imports = module.get_imports()

    for i in imports:
      if i.kind == webassembly.ExternType.GLOBAL:
        global_imports.append(i.field)

    export_map = {e.name: e for e in exports}
    for e in exports:
      if e.kind == webassembly.ExternType.GLOBAL and e.name.startswith('__em_js__'):
        name = e.name[len('__em_js__'):]
        globl = module.get_global(e.index)
        string_address = get_global_value(globl)
        em_js_funcs[name] = get_string_at(module, string_address)

    for i in imports:
      if i.kind == webassembly.ExternType.FUNC:
        if i.field.startswith('invoke_'):
          invoke_funcs.append(i.field)
        elif i.field not in em_js_funcs:
          declares.append(i.field)

    export_names = [e.name for e in exports if e.kind in [webassembly.ExternType.FUNC, webassembly.ExternType.TAG]]

    features = module.parse_features_section()
    features = ['--enable-' + f[1] for f in features if f[0] == '+']
    features = [f.replace('--enable-atomics', '--enable-threads') for f in features]
    features = [f.replace('--enable-simd128', '--enable-simd') for f in features]
    features = [f.replace('--enable-nontrapping-fptoint', '--enable-nontrapping-float-to-int') for f in features]

    # If main does not read its parameters, it will just be a stub that
    # calls __original_main (which has no parameters).
    metadata = {}
    metadata['asmConsts'] = get_asm_strings(module, export_map)
    metadata['declares'] = declares
    metadata['emJsFuncs'] = em_js_funcs
    metadata['exports'] = export_names
    metadata['features'] = features
    metadata['globalImports'] = global_imports
    metadata['invokeFuncs'] = invoke_funcs
    metadata['mainReadsParams'] = get_main_reads_params(module, export_map)
    metadata['namedGlobals'] = get_named_globals(module, exports)
    # print("Metadata parsed: " + pprint.pformat(metadata))
    return metadata
