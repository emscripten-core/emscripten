# Copyright 2022 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import logging
from typing import List, Dict

from . import webassembly, utils
from .webassembly import OpCode, AtomicOpCode, MemoryOpCode
from .shared import exit_with_error
from .settings import settings


logger = logging.getLogger('extract_metadata')


def skip_function_header(module):
  num_local_decls = module.read_uleb()
  while num_local_decls:
    local_count = module.read_uleb()  # noqa
    local_type = module.read_type()  # noqa
    num_local_decls -= 1


def is_orig_main_wrapper(module, function):
  module.get_types()
  module.get_function_types()
  module.seek(function.offset)
  skip_function_header(module)
  end = function.offset + function.size
  while module.tell() != end:
    opcode = module.read_byte()
    try:
      opcode = OpCode(opcode)
    except ValueError:
      return False
    if opcode == OpCode.CALL:
      callee = module.read_uleb()
      callee_type = module.get_function_type(callee)
      if len(callee_type.params) != 0:
        return False
    elif opcode in (OpCode.LOCAL_GET, OpCode.LOCAL_SET):
      module.read_uleb()  # local index
    elif opcode in (OpCode.END, OpCode.RETURN):
      pass
    else:
      # Any other opcodes and we assume this not a simple wrapper
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
    if opcode in (OpCode.END, OpCode.NOP, OpCode.DROP, OpCode.I32_ADD, OpCode.I64_ADD):
      pass
    elif opcode in (OpCode.BLOCK,):
      module.read_type()
    elif opcode in (OpCode.I32_CONST, OpCode.I64_CONST):
      const_values.append(module.read_sleb())
    elif opcode in (OpCode.GLOBAL_SET, OpCode.BR, OpCode.GLOBAL_GET, OpCode.LOCAL_SET, OpCode.LOCAL_GET, OpCode.LOCAL_TEE):
      module.read_uleb()
    elif opcode == OpCode.CALL:
      call_targets.append(module.read_uleb())
    elif opcode == OpCode.MEMORY_PREFIX:
      opcode = MemoryOpCode(module.read_byte())
      if opcode == MemoryOpCode.MEMORY_INIT:
        segment_idx = module.read_uleb()
        segment = segments[segment_idx]
        offset = to_unsigned(const_values[-3])
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


def to_unsigned(val):
  if val < 0:
    return val & ((2 ** 32) - 1)
  else:
    return val


def find_segment_with_address(module, address):
  segments = module.get_segments()
  active = [s for s in segments if s.init]

  for seg in active:
    offset = to_unsigned(get_const_expr_value(seg.init))
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
  # We have at least one test (test/core/test_utf8.c) that uses a double
  # backslash in the C++ source code, in order to represent a single backslash.
  # This is because these strings historically were written and read back via
  # JSON and a single slash is interpreted as an escape char there.
  # Technically this escaping is no longer needed and could be removed
  # but in order to maintain compatibility we strip out the double
  # slashes here.
  data = data.replace('\\\\', '\\')
  return data


def get_section_strings(module, export_map, section_name):
  start_name = f'__start_{section_name}'
  stop_name = f'__stop_{section_name}'
  if start_name not in export_map or stop_name not in export_map:
    logger.debug(f'no start/stop symbols found for section: {section_name}')
    return {}

  start = export_map[start_name]
  end = export_map[stop_name]
  start_global = module.get_global(start.index)
  end_global = module.get_global(end.index)
  start_addr = to_unsigned(get_global_value(start_global))
  end_addr = to_unsigned(get_global_value(end_global))

  seg = find_segment_with_address(module, start_addr)
  if not seg:
    exit_with_error(f'unable to find segment starting at __start_{section_name}: {start_addr}')
  seg, seg_offset = seg

  asm_strings = {}
  str_start = seg_offset
  data = module.read_at(seg.offset, seg.size)
  size = end_addr - start_addr
  end = seg_offset + size
  while str_start < end:
    str_end = data.find(b'\0', str_start)
    asm_strings[start_addr - seg_offset + str_start] = data_to_string(data[str_start:str_end])
    str_start = str_end + 1
  return asm_strings


def get_main_reads_params(module, export_map):
  if settings.STANDALONE_WASM:
    return True

  main = export_map.get('main') or export_map.get('__main_argc_argv')
  if not main or main.kind != webassembly.ExternType.FUNC:
    return False

  main_func = module.get_function(main.index)
  if is_orig_main_wrapper(module, main_func):
    # If main is simple wrapper function then we know that __original_main
    # doesn't read arguments.
    return False

  # By default assume params are read
  return True


def get_global_exports(module, exports):
  global_exports = {}
  for export in exports:
    if export.kind == webassembly.ExternType.GLOBAL:
      g = module.get_global(export.index)
      global_exports[export.name] = str(get_global_value(g))
  return global_exports


def get_function_exports(module):
  rtn = {}
  for e in module.get_exports():
    if e.kind == webassembly.ExternType.FUNC:
      rtn[e.name] = module.get_function_type(e.index)
  return rtn


def update_metadata(filename, metadata):
  imports = []
  invoke_funcs = []
  with webassembly.Module(filename) as module:
    for i in module.get_imports():
      if i.kind == webassembly.ExternType.FUNC:
        if i.field.startswith('invoke_'):
          invoke_funcs.append(i.field)
        else:
          imports.append(i.field)
      elif i.kind in (webassembly.ExternType.GLOBAL, webassembly.ExternType.TAG):
        imports.append(i.field)

    metadata.function_exports = get_function_exports(module)
    metadata.all_exports = [utils.removeprefix(e.name, '__em_js__') for e in module.get_exports()]

  metadata.imports = imports
  metadata.invoke_funcs = invoke_funcs


def get_string_at(module, address):
  seg, offset = find_segment_with_address(module, address)
  data = module.read_at(seg.offset, seg.size)
  str_end = data.find(b'\0', offset)
  return data_to_string(data[offset:str_end])


class Metadata:
  imports: List[str]
  export: List[str]
  em_asm_consts: Dict[int, str]
  js_deps: List[str]
  em_js_funcs: Dict[str, str]
  em_js_func_types: Dict[str, str]
  features: List[str]
  invoke_funcs: List[str]
  main_reads_params: bool
  global_exports: List[str]

  def __init__(self):
    pass


def extract_metadata(filename):
  import_names = []
  invoke_funcs = []
  em_js_funcs = {}
  em_js_func_types = {}

  with webassembly.Module(filename) as module:
    exports = module.get_exports()
    imports = module.get_imports()

    export_map = {e.name: e for e in exports}
    for e in exports:
      if e.kind == webassembly.ExternType.GLOBAL and e.name.startswith('__em_js__'):
        name = utils.removeprefix(e.name, '__em_js__')
        globl = module.get_global(e.index)
        string_address = to_unsigned(get_global_value(globl))
        em_js_funcs[name] = get_string_at(module, string_address)

    for i in imports:
      if i.kind == webassembly.ExternType.FUNC:
        if i.field.startswith('invoke_'):
          invoke_funcs.append(i.field)
        else:
          if i.field in em_js_funcs:
            types = module.get_types()
            em_js_func_types[i.field] = types[i.type]
          import_names.append(i.field)
      elif i.kind in (webassembly.ExternType.GLOBAL, webassembly.ExternType.TAG):
        import_names.append(i.field)

    features = module.parse_features_section()
    features = ['--enable-' + f[1] for f in features if f[0] == '+']
    features = [f.replace('--enable-atomics', '--enable-threads') for f in features]
    features = [f.replace('--enable-simd128', '--enable-simd') for f in features]
    features = [f.replace('--enable-nontrapping-fptoint', '--enable-nontrapping-float-to-int') for f in features]

    # If main does not read its parameters, it will just be a stub that
    # calls __original_main (which has no parameters).
    metadata = Metadata()
    metadata.imports = import_names
    metadata.function_exports = get_function_exports(module)
    metadata.all_exports = [utils.removeprefix(e.name, '__em_js__') for e in exports]
    metadata.em_asm_consts = get_section_strings(module, export_map, 'em_asm')
    metadata.js_deps = [d for d in get_section_strings(module, export_map, 'em_lib_deps').values() if d]
    metadata.em_js_funcs = em_js_funcs
    metadata.em_js_func_types = em_js_func_types
    metadata.features = features
    metadata.invoke_funcs = invoke_funcs
    metadata.main_reads_params = get_main_reads_params(module, export_map)
    metadata.global_exports = get_global_exports(module, exports)

    # print("Metadata parsed: " + pprint.pformat(metadata))
    return metadata
