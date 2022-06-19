# Copyright 2011 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""Utilties for manipulating WebAssembly binaries from python.
"""

from collections import namedtuple
from enum import IntEnum
from functools import wraps
import logging
import os
import sys

from . import utils

sys.path.append(utils.path_from_root('third_party'))

import leb128

logger = logging.getLogger('webassembly')

WASM_PAGE_SIZE = 65536

MAGIC = b'\0asm'

VERSION = b'\x01\0\0\0'

HEADER_SIZE = 8

LIMITS_HAS_MAX = 0x1

SEG_PASSIVE = 0x1

PREFIX_MATH = 0xfc
PREFIX_THREADS = 0xfe
PREFIX_SIMD = 0xfd

SYMBOL_BINDING_MASK = 0x3
SYMBOL_BINDING_GLOBAL = 0x0
SYMBOL_BINDING_WEAK = 0x1
SYMBOL_BINDING_LOCAL = 0x2


def to_leb(num):
  return leb128.u.encode(num)


def read_uleb(iobuf):
  return leb128.u.decode_reader(iobuf)[0]


def read_sleb(iobuf):
  return leb128.i.decode_reader(iobuf)[0]


def cache(method):

  @wraps(method)
  def wrapper(self, *args, **kwargs):
    assert not kwargs
    key = method
    if key not in self._cache:
      self._cache[key] = method(self, *args, **kwargs)
    return self._cache[key]

  return wrapper


def once(method):

  @wraps(method)
  def helper(self, *args, **kwargs):
    key = method
    if key not in self._cache:
      self._cache[key] = method(self, *args, **kwargs)

  return helper


class Type(IntEnum):
  I32 = 0x7f # -0x1
  I64 = 0x7e # -0x2
  F32 = 0x7d # -0x3
  F64 = 0x7c # -0x4
  V128 = 0x7b # -0x5
  FUNCREF = 0x70 # -0x10
  EXTERNREF = 0x6f # -0x11


class OpCode(IntEnum):
  NOP = 0x01
  BLOCK = 0x02
  CALL = 0x10
  END = 0x0b
  LOCAL_GET = 0x20
  LOCAL_SET = 0x21
  GLOBAL_GET = 0x23
  GLOBAL_SET = 0x24
  RETURN = 0x0f
  I32_CONST = 0x41
  I64_CONST = 0x42
  F32_CONST = 0x43
  F64_CONST = 0x44
  REF_NULL = 0xd0


class SecType(IntEnum):
  CUSTOM = 0
  TYPE = 1
  IMPORT = 2
  FUNCTION = 3
  TABLE = 4
  MEMORY = 5
  TAG = 13
  GLOBAL = 6
  EXPORT = 7
  START = 8
  ELEM = 9
  DATACOUNT = 12
  CODE = 10
  DATA = 11


class ExternType(IntEnum):
  FUNC = 0
  TABLE = 1
  MEMORY = 2
  GLOBAL = 3
  TAG = 4


class DylinkType(IntEnum):
  MEM_INFO = 1
  NEEDED = 2
  EXPORT_INFO = 3
  IMPORT_INFO = 4


class InvalidWasmError(BaseException):
  pass


Section = namedtuple('Section', ['type', 'size', 'offset', 'name'])
Limits = namedtuple('Limits', ['flags', 'initial', 'maximum'])
Import = namedtuple('Import', ['kind', 'module', 'field', 'type'])
Export = namedtuple('Export', ['name', 'kind', 'index'])
Global = namedtuple('Global', ['type', 'mutable', 'init'])
Dylink = namedtuple('Dylink', ['mem_size', 'mem_align', 'table_size', 'table_align', 'needed', 'export_info', 'import_info'])
Table = namedtuple('Table', ['elem_type', 'limits'])
FunctionBody = namedtuple('FunctionBody', ['offset', 'size'])
DataSegment = namedtuple('DataSegment', ['flags', 'init', 'offset', 'size'])
FuncType = namedtuple('FuncType', ['params', 'returns'])


class Module:
  """Extremely minimal wasm module reader.  Currently only used
  for parsing the dylink section."""
  def __init__(self, filename):
    self.buf = None # Set this before FS calls below in case they throw.
    self.filename = filename
    self.size = os.path.getsize(filename)
    self.buf = open(filename, 'rb')
    magic = self.buf.read(4)
    version = self.buf.read(4)
    if magic != MAGIC or version != VERSION:
      raise InvalidWasmError(f'{filename} is not a valid wasm file')
    self._cache = {}

  def __del__(self):
    if self.buf:
      self.buf.close()

  def read_at(self, offset, count):
    self.buf.seek(offset)
    return self.buf.read(count)

  def read_byte(self):
    return self.buf.read(1)[0]

  def read_uleb(self):
    return read_uleb(self.buf)

  def read_sleb(self):
    return read_sleb(self.buf)

  def read_string(self):
    size = self.read_uleb()
    return self.buf.read(size).decode('utf-8')

  def read_limits(self):
    flags = self.read_byte()
    initial = self.read_uleb()
    maximum = 0
    if flags & LIMITS_HAS_MAX:
      maximum = self.read_uleb()
    return Limits(flags, initial, maximum)

  def read_type(self):
    return Type(self.read_uleb())

  def read_init(self):
    code = []
    while 1:
      opcode = OpCode(self.read_byte())
      args = []
      if opcode in (OpCode.GLOBAL_GET, OpCode.I32_CONST, OpCode.I64_CONST):
        args.append(self.read_uleb())
      elif opcode in (OpCode.REF_NULL,):
        args.append(self.read_type())
      elif opcode in (OpCode.END,):
        pass
      else:
        raise Exception('unexpected opcode %s' % opcode)
      code.append((opcode, args))
      if opcode == OpCode.END:
        break
    return code

  def seek(self, offset):
    return self.buf.seek(offset)

  def tell(self):
    return self.buf.tell()

  def skip(self, count):
    self.buf.seek(count, os.SEEK_CUR)

  def sections(self):
    """Generator that lazily returns sections from the wasm file."""
    offset = HEADER_SIZE
    while offset < self.size:
      self.seek(offset)
      section_type = SecType(self.read_byte())
      section_size = self.read_uleb()
      section_offset = self.buf.tell()
      name = None
      if section_type == SecType.CUSTOM:
        name = self.read_string()

      yield Section(section_type, section_size, section_offset, name)
      offset = section_offset + section_size

  def get_types(self):
    type_section = self.get_section(SecType.TYPE)
    if not type_section:
      return []
    self.seek(type_section.offset)
    num_types = self.read_uleb()
    types = []
    for _ in range(num_types):
      params = []
      returns = []
      type_form = self.read_byte()
      assert type_form == 0x60
      num_params = self.read_uleb()
      for _ in range(num_params):
        params.append(self.read_type())
      num_returns = self.read_uleb()
      for _ in range(num_returns):
        returns.append(self.read_type())
      types.append(FuncType(params, returns))
    return types

  def parse_features_section(self):
    features = []
    sec = self.get_custom_section('target_features')
    if sec:
      self.seek(sec.offset)
      self.read_string()  # name
      feature_count = self.read_uleb()
      while feature_count:
        prefix = self.read_byte()
        features.append((chr(prefix), self.read_string()))
        feature_count -= 1
    return features

  @cache
  def parse_dylink_section(self):
    dylink_section = next(self.sections())
    assert dylink_section.type == SecType.CUSTOM
    self.seek(dylink_section.offset)
    # section name
    needed = []
    export_info = {}
    import_info = {}
    self.read_string()  # name

    if dylink_section.name == 'dylink':
      mem_size = self.read_uleb()
      mem_align = self.read_uleb()
      table_size = self.read_uleb()
      table_align = self.read_uleb()

      needed_count = self.read_uleb()
      while needed_count:
        libname = self.read_string()
        needed.append(libname)
        needed_count -= 1
    elif dylink_section.name == 'dylink.0':
      section_end = dylink_section.offset + dylink_section.size
      while self.tell() < section_end:
        subsection_type = self.read_uleb()
        subsection_size = self.read_uleb()
        end = self.tell() + subsection_size
        if subsection_type == DylinkType.MEM_INFO:
          mem_size = self.read_uleb()
          mem_align = self.read_uleb()
          table_size = self.read_uleb()
          table_align = self.read_uleb()
        elif subsection_type == DylinkType.NEEDED:
          needed_count = self.read_uleb()
          while needed_count:
            libname = self.read_string()
            needed.append(libname)
            needed_count -= 1
        elif subsection_type == DylinkType.EXPORT_INFO:
          count = self.read_uleb()
          while count:
            sym = self.read_string()
            flags = self.read_uleb()
            export_info[sym] = flags
            count -= 1
        elif subsection_type == DylinkType.IMPORT_INFO:
          count = self.read_uleb()
          while count:
            module = self.read_string()
            field = self.read_string()
            flags = self.read_uleb()
            import_info.setdefault(module, {})
            import_info[module][field] = flags
            count -= 1
        else:
          print(f'unknown subsection: {subsection_type}')
          # ignore unknown subsections
          self.skip(subsection_size)
        assert(self.tell() == end)
    else:
      utils.exit_with_error('error parsing shared library')

    return Dylink(mem_size, mem_align, table_size, table_align, needed, export_info, import_info)

  @cache
  def get_exports(self):
    export_section = self.get_section(SecType.EXPORT)
    if not export_section:
      return []

    self.seek(export_section.offset)
    num_exports = self.read_uleb()
    exports = []
    for _ in range(num_exports):
      name = self.read_string()
      kind = ExternType(self.read_byte())
      index = self.read_uleb()
      exports.append(Export(name, kind, index))

    return exports

  @cache
  def get_imports(self):
    import_section = self.get_section(SecType.IMPORT)
    if not import_section:
      return []

    self.seek(import_section.offset)
    num_imports = self.read_uleb()
    imports = []
    for _ in range(num_imports):
      mod = self.read_string()
      field = self.read_string()
      kind = ExternType(self.read_byte())
      type_ = None
      if kind == ExternType.FUNC:
        type_ = self.read_uleb()
      elif kind == ExternType.GLOBAL:
        type_ = self.read_sleb()
        self.read_byte()  # mutable
      elif kind == ExternType.MEMORY:
        self.read_limits()  # limits
      elif kind == ExternType.TABLE:
        type_ = self.read_sleb()
        self.read_limits()  # limits
      elif kind == ExternType.TAG:
        self.read_byte()  # attribute
        type_ = self.read_uleb()
      else:
        raise AssertionError()
      imports.append(Import(kind, mod, field, type_))

    return imports

  @cache
  def get_globals(self):
    global_section = self.get_section(SecType.GLOBAL)
    if not global_section:
      return []
    globls = []
    self.seek(global_section.offset)
    num_globals = self.read_uleb()
    for _ in range(num_globals):
      global_type = self.read_type()
      mutable = self.read_byte()
      init = self.read_init()
      globls.append(Global(global_type, mutable, init))
    return globls

  @cache
  def get_functions(self):
    code_section = self.get_section(SecType.CODE)
    if not code_section:
      return []
    functions = []
    self.seek(code_section.offset)
    num_functions = self.read_uleb()
    for _ in range(num_functions):
      body_size = self.read_uleb()
      start = self.tell()
      functions.append(FunctionBody(start, body_size))
      self.seek(start + body_size)
    return functions

  def get_section(self, section_code):
    return next((s for s in self.sections() if s.type == section_code), None)

  @cache
  def get_custom_section(self, name):
    for section in self.sections():
      if section.type == SecType.CUSTOM and section.name == name:
        return section
    return None

  @cache
  def get_segments(self):
    segments = []
    data_section = self.get_section(SecType.DATA)
    self.seek(data_section.offset)
    num_segments = self.read_uleb()
    for _ in range(num_segments):
      flags = self.read_uleb()
      if (flags & SEG_PASSIVE):
        init = None
      else:
        init = self.read_init()
      size = self.read_uleb()
      offset = self.tell()
      segments.append(DataSegment(flags, init, offset, size))
      self.seek(offset + size)
    return segments

  @cache
  def get_tables(self):
    table_section = self.get_section(SecType.TABLE)
    if not table_section:
      return []

    self.seek(table_section.offset)
    num_tables = self.read_uleb()
    tables = []
    for _ in range(num_tables):
      elem_type = self.read_type()
      limits = self.read_limits()
      tables.append(Table(elem_type, limits))

    return tables

  def has_name_section(self):
    return self.get_custom_section('name') is not None

  @once
  def _calc_indexes(self):
    self.num_imported_funcs = 0
    self.num_imported_globals = 0
    self.num_imported_memories = 0
    self.num_imported_tables = 0
    self.num_imported_tags = 0
    for i in self.get_imports():
      if i.kind == ExternType.FUNC:
        self.num_imported_funcs += 1
      elif i.kind == ExternType.GLOBAL:
        self.num_imported_globals += 1
      elif i.kind == ExternType.MEMORY:
        self.num_imported_memories += 1
      elif i.kind == ExternType.TABLE:
        self.num_imported_tables += 1
      elif i.kind == ExternType.TAG:
        self.num_imported_tags += 1
      else:
        assert False, 'unhandled export type: %s' % i.kind

  def get_function(self, idx):
    self._calc_indexes()
    assert idx >= self.num_imported_funcs
    return self.get_functions()[idx - self.num_imported_funcs]

  def get_global(self, idx):
    self._calc_indexes()
    assert idx >= self.num_imported_globals
    return self.get_globals()[idx - self.num_imported_globals]


def parse_dylink_section(wasm_file):
  module = Module(wasm_file)
  return module.parse_dylink_section()


def get_exports(wasm_file):
  module = Module(wasm_file)
  return module.get_exports()


def get_imports(wasm_file):
  module = Module(wasm_file)
  return module.get_imports()
