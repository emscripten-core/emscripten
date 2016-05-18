#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=0
'''

import os, sys, re, json
import asm_module, shared, shutil

# params

INNERTERPRETER_LAST_OPCODE = 0 # 'CONDD'

EMT_STACK_MAX = 1024*1024

LOG_CODE = os.environ.get('EMCC_LOG_EMTERPRETER_CODE')

ZERO = False
ASYNC = False
ASSERTIONS = False
PROFILING = False
SWAPPABLE = False
FROUND = False
ADVISE = False
MEMORY_SAFE = False
OUTPUT_FILE = None

def handle_arg(arg):
  global ZERO, ASYNC, ASSERTIONS, PROFILING, FROUND, ADVISE, MEMORY_SAFE, OUTPUT_FILE
  if '=' in arg:
    l, r = arg.split('=')
    if l == 'ZERO': ZERO = int(r)
    elif l == 'ASYNC': ASYNC = int(r)
    elif l == 'ASSERTIONS': ASSERTIONS = int(r)
    elif l == 'PROFILING': PROFILING = int(r)
    elif l == 'FROUND': FROUND = int(r)
    elif l == 'ADVISE': ADVISE = int(r)
    elif l == 'MEMORY_SAFE': MEMORY_SAFE = int(r)
    elif l == 'FILE': OUTPUT_FILE = r[1:-1]
    return False
  return True

DEBUG = os.environ.get('EMCC_DEBUG')

config = shared.Configuration()
temp_files = config.get_temp_files()

if DEBUG:
  print >> sys.stderr, 'running emterpretify on', sys.argv

if FROUND:
  shared.Settings.PRECISE_F32 = 1

sys.argv = filter(handle_arg, sys.argv)

# consts

BLACKLIST = set(['_malloc', '_free', '_memcpy', '_memmove', '_memset', '_strlen', 'stackAlloc', 'setThrew', 'stackRestore', 'setTempRet0', 'getTempRet0', 'stackSave', '_emscripten_autodebug_double', '_emscripten_autodebug_float', '_emscripten_autodebug_i8', '_emscripten_autodebug_i16', '_emscripten_autodebug_i32', '_emscripten_autodebug_i64', '_strncpy', '_strcpy', '_strcat', '_saveSetjmp', '_testSetjmp', '_emscripten_replace_memory', '_bitshift64Shl', '_bitshift64Ashr', '_bitshift64Lshr', 'setAsyncState', 'emtStackSave', 'emtStackRestore'])
WHITELIST = []

SYNC_FUNCS = set(['_emscripten_sleep', '_emscripten_sleep_with_yield', '_emscripten_wget_data', '_emscripten_idb_load', '_emscripten_idb_store', '_emscripten_idb_delete'])

OPCODES = [ # l, lx, ly etc - one of 256 locals
  'SET',     # [lx, ly, 0]          lx = ly (int or float, not double)
  'SETVI',   # [l, vl, vh]          l = v (16-bit signed int)
  'SETVIB',  # [l, 0, 0] [..v..]    l = 32-bit int in next 32-bit instruction

  'ADD',     # [lx, ly, lz]         lx = ly + lz (32-bit int)
  'SUB',     # [lx, ly, lz]         lx = ly - lz (32-bit int)
  'MUL',     # [lx, ly, lz]         lx = ly * lz (32-bit int)
  'SDIV',    # [lx, ly, lz]         lx = ly / lz (32-bit signed int)
  'UDIV',    # [lx, ly, lz]         lx = ly / lz (32-bit unsigned int)
  'SMOD',    # [lx, ly, lz]         lx = ly % lz (32-bit signed int)
  'UMOD',    # [lx, ly, lz]         lx = ly % lz (32-bit unsigned int)
  'NEG',     # [lx, ly, 0]          lx = -ly (int)
  'BNOT',    # [lx, ly, 0]          ly = ~ly (int)

  'LNOT',    # [lx, ly, 0]          ly = !ly (int)
  'EQ',      # [lx, ly, lz]         lx = ly == lz (32-bit int)
  'NE',      # [lx, ly, lz]         lx = ly != lz (32-bit int)
  'SLT',     # [lx, ly, lz]         lx = ly < lz (32-bit signed)
  'ULT',     # [lx, ly, lz]         lx = ly < lz (32-bit unsigned)
  'SLE',     # [lx, ly, lz]         lx = ly <= lz (32-bit signed)
  'ULE',     # [lx, ly, lz]         lx = ly <= lz (32-bit unsigned)

  'AND',     # [lx, ly, lz]         lx = ly & lz
  'OR',      # [lx, ly, lz]         lx = ly | lz
  'XOR',     # [lx, ly, lz]         lx = ly ^ lz
  'SHL',     # [lx, ly, lz]         lx = ly << lz
  'ASHR',    # [lx, ly, lz]         lx = ly >> lz
  'LSHR',    # [lx, ly, lz]         lx = ly >>> lz

  'ADDV',    # [lx, ly, v]          lx = ly + v (32-bit int, v is 8-bit signed)
  'SUBV',
  'MULV',
  'SDIVV',
  'UDIVV',    #                                              (v is 8-bit unsigned)
  'SMODV',
  'UMODV',    #                                              (v is 8-bit unsigned)
  'EQV',
  'NEV',
  'SLTV',
  'ULTV',    #                                               (v is 8-bit unsigned)
  'SLEV',
  'ULEV',    #                                               (v is 8-bit unsigned)
  'ANDV',
  'ORV',
  'XORV',
  'SHLV',    #                                               (v is 8-bit unsigned)
  'ASHRV',   #                                               (v is 8-bit unsigned)
  'LSHRV',   #                                               (v is 8-bit unsigned)

  'LNOTBRF',    # [cond] [absolute-target]      cond+branch
  'EQBRF',
  'NEBRF',
  'SLTBRF',
  'ULTBRF',
  'SLEBRF',
  'ULEBRF',
  'LNOTBRT',
  'EQBRT',
  'NEBRT',
  'SLTBRT',
  'ULTBRT',
  'SLEBRT',
  'ULEBRT',

  'SETD',    # [lx, ly, 0]          lx = ly (double)
  'SETVD',   # [lx, vl, vh]         lx = ly (16 bit signed int, converted into double)
  'SETVDI',  # [lx, 0, 0] [..v..]   lx = v (32 bit signed int, converted into double)
  'SETVDF',  # [lx, 0, 0] [..v..]   lx = v (32 bit float, converted into double)
  'SETVDD',  # [lx, 0, 0][.v.][.v.] lx = v (64 bit double)
  'ADDD',    # [lx, ly, lz]         lx = ly + lz (double)
  'SUBD',    # [lx, ly, lz]         lx = ly - lz (double)
  'MULD',    # [lx, ly, lz]         lx = ly * lz (double)
  'DIVD',    # [lx, ly, lz]         lx = ly / lz (double)
  'MODD',    # [lx, ly, lz]         lx = ly % lz (double)
  'NEGD',    # [lx, ly, 0]          lx = -ly (double)
  'EQD',     # [lx, ly, lz]         lx = ly == lz (double)
  'NED',     # [lx, ly, lz]         lx = ly != lz (double)
  'LTD',     # [lx, ly, lz]         lx = ly < lz (signed)
  'LED',     # [lx, ly, lz]         lx = ly < lz (double)
  'GTD',     # [lx, ly, lz]         lx = ly <= lz (double)
  'GED',     # [lx, ly, lz]         lx = ly <= lz (double)
  'D2I',     # [lx, ly, 0]          lx = ~~ly (double-to-int)
  'SI2D',    # [lx, ly, 0]          lx = +ly (signed int-to-double)
  'UI2D',    # [lx, ly, 0]          lx = +ly (unsigned int-to-double)

  'LOAD8',   # [lx, ly, 0]          lx = HEAP8[ly >> 0]
  'LOADU8',  # [lx, ly, 0]          lx = HEAPU8[ly >> 0]
  'LOAD16',  # [lx, ly, 0]          lx = HEAP16[ly >> 1]
  'LOADU16', # [lx, ly, 0]          lx = HEAPU16[ly >> 1]
  'LOAD32',  # [lx, ly, 0]          lx = HEAP32[ly >> 2]   - no need for unsigned version, this is set to a register anyhow
  'STORE8',  # [lx, ly, 0]          HEAP8[lx >> 2] = ly
  'STORE16', # [lx, ly, 0]          HEAP16[lx >> 2] = ly
  'STORE32', # [lx, ly, 0]          HEAP32[lx >> 2] = ly
  'LOADF64',  # [lx, ly, 0]         lx = HEAPF64[ly >> 3]
  'STOREF64', # [lx, ly, 0]         HEAPF64[lx >> 3] = ly
  'LOADF32',  # [lx, ly, 0]         lx = HEAPF32[ly >> 3]
  'STOREF32', # [lx, ly, 0]         HEAPF32[lx >> 3] = ly

  'LOAD8A',   # [lx, ly, lz]        load-add and store-add instructions, whose pointer input is a signed addition: lx = load(ly + lz), store(lx + ly) = lz
  'LOADU8A',
  'LOAD16A',
  'LOADU16A',
  'LOAD32A',
  'STORE8A',
  'STORE16A',
  'STORE32A',
  'LOADF64A',
  'STOREF64A',
  'LOADF32A',
  'STOREF32A',

  'LOAD8AV',   # [lx, ly, lz]        load-add and store-add instructions, whose pointer input is a signed addition: lx = load(ly + lz), store(lx + ly) = lz, where the second add op is 8-bit signed
  'LOADU8AV',
  'LOAD16AV',
  'LOADU16AV',
  'LOAD32AV',
  'STORE8AV',
  'STORE16AV',
  'STORE32AV',
  'LOADF64AV',
  'STOREF64AV',
  'LOADF32AV',
  'STOREF32AV',

  'STORE8C',
  'STORE16C',
  'STORE32C',
  'STOREF64C',
  'STOREF32C',

  'BR',      # [0, tl, th]          jump t instructions (multiple of 4)
  'BRT',     # [cond, tl, th]       if cond, jump t instructions (multiple of 4)
  'BRF',     # [cond, tl, th]       if !cond, jump t instructions (multiple of 4)
  'BRA',      # [0, 0, 0] [addr]     jump to addr
  'BRTA',     # [cond, 0, 0] [addr]  if cond, jump to addr
  'BRFA',     # [cond, 0, 0] [addr]  if !cond, jump to addr

  'COND',    # [out, cond, x] [y]   out = cond ? x : y, int
  'CONDD',   # [out, cond, x] [y]   out = cond ? x : y, double

  'GETTDP',  # [l, 0, 0]            l = tempDoublePtr
  'GETTR0',  # [l, 0, 0]            l = tempRet0
  'SETTR0',  # [l, 0, 0]            tempRet0 = l
  'GETGLBI', # [l, vl, vh]          get global value, int, indexed by v
  'GETGLBD', # [l, vl, vh]          get global value, double, indexed by v
  'SETGLBI', # [vl, vh, l]          set global value, int, indexed by v (v = l)
  'SETGLBD', # [vl, vh, l]          set global value, double, indexed by v (v = l)

  'INTCALL', # [lx, 0, 0] [target] [params]         (lx = ) target(params..)
                    #                               Internal, emterpreter-to-emterpreter call.
  'EXTCALL', # [lx, targetl, targeth] [params...]   (lx = ) target(params..) lx's existence and type depend on the target's actual callsig;
                    #                                 this instruction can take multiple 32-bit instruction chunks
                    #                                 if target is a function table, then the first param is the index of the register holding the function pointer

  'GETST',   # [l, 0, 0]            l = STACKTOP
  'SETST',   # [l, 0, 0]            STACKTOP = l

  'SWITCH',  # [lx, ly, lz]         switch (lx) { .. }. followed by a jump table for values in range [ly..ly+lz), after which is the default (which might be empty)
  'RET',     # [l, 0, 0]            return l (depending on which emterpreter_x we are in, has the right type)
  'FUNC',    # [num params, total locals (low 8 bits), total locals (high 8 bits)] [which emterpreter (0 = normal, 1 = zero), 0, 0, 0]           function with n locals (each taking 64 bits), of which the first are params
             # this is read in the emterpreter prelude, and also in intcalls

  # slow locals support - copying from/to slow locals
  'FSLOW',     # [lx, lyl, lyh]       lx = ly (int or float, not double; ly = lyl,lyh
  'FSLOWD',    # [lx, lyl, lyh]       lx = ly (double)
  'TSLOW',     # [lxl, lxh, ly]       lx = ly (int or float, not double; lx = lxl,lxh
  'TSLOWD',    # [lxl, lxh, ly]       lx = ly (double; lx = lxl,lxh)
]

if FROUND:
  OPCODES.append(
    'FROUND',    # [lx, ly]         lx = Math.fround(ly), rounds doubles to floats
  )

def randomize_opcodes():
  global OPCODES
  import random
  random.shuffle(opcodes)
  print OPCODES
#randomize_opcodes()

assert len(OPCODES) == len(set(OPCODES)) # no dupe names
assert len(OPCODES) < 256

ROPCODES = {}
for i in range(len(OPCODES)):
  ROPCODES[OPCODES[i]] = i

# utils

settings = { 'PRECISE_F32': 0 } # TODO

def bytify(x):
  assert x >= 0 and x < (1 << 32)
  return [x & 255, (x >> 8) & 255, (x >> 16) & 255, (x >> 24) & 255]

def next_power_of_two(x):
  if x == 0: return 0
  ret = 1
  while ret < x: ret <<= 1
  return ret

def get_access(l, s='i', base='sp', offset=None):
  if offset is not None:
    offset = '+ ' + str(offset) + ' '
  else:
    offset = ''
  if s == 'i':
    return 'HEAP32[' + str(base) + ' + (' + l + ' << 3) ' + offset + '>> 2]'
  elif s == 'd' or s == 'f':
    return 'HEAPF64[' + str(base) + ' + (' + l + ' << 3) ' + offset + '>> 3]'
  else:
    assert 0

def get_coerced_access(l, s='i', unsigned=False, base='sp', offset=None):
  if s == 'i':
    if not unsigned:
      return get_access(l, s, base, offset) + '|0'
    else:
      return get_access(l, s, base, offset) + '>>>0'
  elif s == 'd' or s == 'f':
    return '+' + get_access(l, s, base, offset)
  else:
    assert 0

def make_assign(left, right, temp): # safely assign, taking into account memory safety
  if not MEMORY_SAFE:
    return left + ' = ' + right + ';'
  return temp + ' = ' + right + '; ' + left + ' = ' + temp + ';'

CASES = {}
CASES[ROPCODES['SET']] = get_access('lx') + ' = ' + get_coerced_access('ly') + ';'
CASES[ROPCODES['GETST']] = get_access('lx') + ' = STACKTOP;'
CASES[ROPCODES['SETST']] = 'STACKTOP = ' + get_coerced_access('lx') + ';'
CASES[ROPCODES['SETVI']] = get_access('lx') + ' = inst >> 16;'
CASES[ROPCODES['SETVIB']] = 'pc = pc + 4 | 0; ' + get_access('lx') + ' = HEAP32[pc >> 2] | 0;'

CASES[ROPCODES['ADD']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['SUB']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') - (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['MUL']] = make_assign(get_access('lx'), 'Math_imul(' + get_coerced_access('ly') + ', ' + get_coerced_access('lz') + ') | 0', 'ly')
CASES[ROPCODES['SDIV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') / (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['UDIV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') / (' + get_coerced_access('lz', unsigned=True) + ') >>> 0;'
CASES[ROPCODES['SMOD']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') % (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['UMOD']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') % (' + get_coerced_access('lz', unsigned=True) + ') >>> 0;'
CASES[ROPCODES['NEG']] = get_access('lx') + ' = -(' + get_coerced_access('ly') + ');'
CASES[ROPCODES['BNOT']] = get_access('lx') + ' = ~(' + get_coerced_access('ly') + ');'

CASES[ROPCODES['LNOT']] = get_access('lx') + ' = !(' + get_coerced_access('ly') + ');'
CASES[ROPCODES['EQ']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') == (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['NE']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') != (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['SLT']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') < (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['ULT']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') < (' + get_coerced_access('lz', unsigned=True) + ') | 0;'
CASES[ROPCODES['SLE']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') <= (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['ULE']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') <= (' + get_coerced_access('lz', unsigned=True) + ') | 0;'

CASES[ROPCODES['AND']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') & (' + get_coerced_access('lz') + ');'
CASES[ROPCODES['OR']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') | (' + get_coerced_access('lz') + ');'
CASES[ROPCODES['XOR']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') ^ (' + get_coerced_access('lz') + ');'
CASES[ROPCODES['SHL']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') << (' + get_coerced_access('lz') + ');'
CASES[ROPCODES['ASHR']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') >> (' + get_coerced_access('lz') + ');'
CASES[ROPCODES['LSHR']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') >>> (' + get_coerced_access('lz') + ');'

CASES[ROPCODES['ADDV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') + (inst >> 24) | 0;'
CASES[ROPCODES['SUBV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') - (inst >> 24) | 0;'
CASES[ROPCODES['MULV']] = make_assign(get_access('lx'), 'Math_imul(' + get_coerced_access('ly') + ', inst >> 24) | 0', 'ly')
CASES[ROPCODES['SDIVV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') / (inst >> 24) | 0;'
CASES[ROPCODES['UDIVV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') / (lz >>> 0) >>> 0;'
CASES[ROPCODES['SMODV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') % (inst >> 24) | 0;'
CASES[ROPCODES['UMODV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') % (lz >>> 0) >>> 0;'
CASES[ROPCODES['EQV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') == (inst >> 24) | 0;'
CASES[ROPCODES['NEV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') != (inst >> 24) | 0;'
CASES[ROPCODES['SLTV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') < (inst >> 24) | 0;'
CASES[ROPCODES['ULTV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') < (lz >>> 0) | 0;'
CASES[ROPCODES['SLEV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') <= (inst >> 24) | 0;'
CASES[ROPCODES['ULEV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') <= (lz >>> 0) | 0;'
CASES[ROPCODES['ANDV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') & (inst >> 24);'
CASES[ROPCODES['ORV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') | (inst >> 24);'
CASES[ROPCODES['XORV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') ^ (inst >> 24);'
CASES[ROPCODES['SHLV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') << lz;'
CASES[ROPCODES['ASHRV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') >> lz;'
CASES[ROPCODES['LSHRV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') >>> lz;'

CASES[ROPCODES['LNOTBRF']] = 'if (' + get_coerced_access('ly') + ') { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['EQBRF']] = 'if ((' + get_coerced_access('ly') + ') == (' + get_coerced_access('lz') + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['NEBRF']] = 'if ((' + get_coerced_access('ly') + ') != (' + get_coerced_access('lz') + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['SLTBRF']] = 'if ((' + get_coerced_access('ly') + ') < (' + get_coerced_access('lz') + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['ULTBRF']] = 'if ((' + get_coerced_access('ly', unsigned=True) + ') < (' + get_coerced_access('lz', unsigned=True) + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['SLEBRF']] = 'if ((' + get_coerced_access('ly') + ') <= (' + get_coerced_access('lz') + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['ULEBRF']] = 'if ((' + get_coerced_access('ly', unsigned=True) + ') <= (' + get_coerced_access('lz', unsigned=True) + ')) { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'

CASES[ROPCODES['LNOTBRT']] = 'if (' + get_coerced_access('ly') + ') { pc = pc + 4 | 0; } else { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['EQBRT']] = 'if ((' + get_coerced_access('ly') + ') == (' + get_coerced_access('lz') + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['NEBRT']] = 'if ((' + get_coerced_access('ly') + ') != (' + get_coerced_access('lz') + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['SLTBRT']] = 'if ((' + get_coerced_access('ly') + ') < (' + get_coerced_access('lz') + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['ULTBRT']] = 'if ((' + get_coerced_access('ly', unsigned=True) + ') < (' + get_coerced_access('lz', unsigned=True) + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['SLEBRT']] = 'if ((' + get_coerced_access('ly') + ') <= (' + get_coerced_access('lz') + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'
CASES[ROPCODES['ULEBRT']] = 'if ((' + get_coerced_access('ly', unsigned=True) + ') <= (' + get_coerced_access('lz', unsigned=True) + ')) { pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; } else { pc = pc + 4 | 0; }'

CASES[ROPCODES['SETD']] = get_access('lx', s='d') + ' = ' + get_coerced_access('ly', s='d') + ';'
CASES[ROPCODES['SETVD']] = get_access('lx', s='d') + ' = +(inst >> 16);'
CASES[ROPCODES['SETVDI']] = 'pc = pc + 4 | 0; ' + get_access('lx', s='d') + ' = +(HEAP32[pc >> 2] | 0);'
CASES[ROPCODES['SETVDF']] = 'pc = pc + 4 | 0; ' + get_access('lx', s='d') + ' = +HEAPF32[pc >> 2];'
CASES[ROPCODES['SETVDD']] = 'HEAP32[tempDoublePtr >> 2] = HEAP32[pc + 4 >> 2]; HEAP32[tempDoublePtr + 4 >> 2] = HEAP32[pc + 8 >> 2]; pc = pc + 8 | 0; ' + get_access('lx', s='d') + ' = +HEAPF64[tempDoublePtr >> 3];'
CASES[ROPCODES['ADDD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') + (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['SUBD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') - (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['MULD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') * (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['DIVD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') / (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['MODD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') % (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['NEGD']] = get_access('lx', s='d') + ' = -(' + get_coerced_access('ly', s='d') + ');'
CASES[ROPCODES['EQD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') == (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['NED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') != (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['LTD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') < (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['LED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') <= (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['GTD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') > (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['GED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') >= (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['D2I']] = get_access('lx') + ' = ~~(' + get_coerced_access('ly', s='d') + ');'
CASES[ROPCODES['SI2D']] = get_access('lx', s='d') + ' = +(' + get_coerced_access('ly') + ');'
CASES[ROPCODES['UI2D']] = get_access('lx', s='d') + ' = +(' + get_coerced_access('ly', unsigned=True) + ');'

CASES[ROPCODES['LOAD8']] = get_access('lx') + ' = ' + 'HEAP8[' + get_access('ly') + ' >> 0];'
CASES[ROPCODES['LOADU8']] = get_access('lx') + ' = ' + 'HEAPU8[' + get_access('ly') + ' >> 0];'
CASES[ROPCODES['LOAD16']] = get_access('lx') + ' = ' + 'HEAP16[' + get_access('ly') + ' >> 1];'
CASES[ROPCODES['LOADU16']] = get_access('lx') + ' = ' + 'HEAPU16[' + get_access('ly') + ' >> 1];'
CASES[ROPCODES['LOAD32']] = get_access('lx') + ' = ' + 'HEAP32[' + get_access('ly') + ' >> 2];'
CASES[ROPCODES['STORE8']] = 'HEAP8[' + get_access('lx') + ' >> 0] = ' + get_coerced_access('ly') + ';'
CASES[ROPCODES['STORE16']] = 'HEAP16[' + get_access('lx') + ' >> 1] = ' + get_coerced_access('ly') + ';'
CASES[ROPCODES['STORE32']] = 'HEAP32[' + get_access('lx') + ' >> 2] = ' + get_coerced_access('ly') + ';'
CASES[ROPCODES['LOADF64']] = get_access('lx', s='d') + ' = ' + '+HEAPF64[' + get_access('ly') + ' >> 3];'
CASES[ROPCODES['STOREF64']] = 'HEAPF64[' + get_access('lx') + ' >> 3] = ' + get_coerced_access('ly', s='d') + ';'
CASES[ROPCODES['LOADF32']] = get_access('lx', s='d') + ' = ' + '+HEAPF32[' + get_access('ly') + ' >> 2];'
CASES[ROPCODES['STOREF32']] = 'HEAPF32[' + get_access('lx') + ' >> 2] = ' + get_coerced_access('ly', s='d') + ';'

CASES[ROPCODES['LOAD8A']] = get_access('lx') + ' = ' + 'HEAP8[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 0];'
CASES[ROPCODES['LOADU8A']] = get_access('lx') + ' = ' + 'HEAPU8[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 0];'
CASES[ROPCODES['LOAD16A']] = get_access('lx') + ' = ' + 'HEAP16[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 1];'
CASES[ROPCODES['LOADU16A']] = get_access('lx') + ' = ' + 'HEAPU16[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 1];'
CASES[ROPCODES['LOAD32A']] = get_access('lx') + ' = ' + 'HEAP32[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 2];'
CASES[ROPCODES['STORE8A']] = 'HEAP8[(' + get_coerced_access('lx') + ') + (' + get_coerced_access('ly') + ') >> 0] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['STORE16A']] = 'HEAP16[(' + get_coerced_access('lx') + ') + (' + get_coerced_access('ly') + ') >> 1] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['STORE32A']] = 'HEAP32[(' + get_coerced_access('lx') + ') + (' + get_coerced_access('ly') + ') >> 2] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['LOADF64A']] = get_access('lx', s='d') + ' = ' + '+HEAPF64[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 3];'
CASES[ROPCODES['STOREF64A']] = 'HEAPF64[(' + get_coerced_access('lx') + ') + (' + get_coerced_access('ly') + ') >> 3] = ' + get_coerced_access('lz', s='d') + ';'
CASES[ROPCODES['LOADF32A']] = get_access('lx', s='d') + ' = ' + '+HEAPF32[(' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') >> 2];'
CASES[ROPCODES['STOREF32A']] = 'HEAPF32[(' + get_coerced_access('lx') + ') + (' + get_coerced_access('ly') + ') >> 2] = ' + get_coerced_access('lz', s='d') + ';'

CASES[ROPCODES['LOAD8AV']] = get_access('lx') + ' = ' + 'HEAP8[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 0];'
CASES[ROPCODES['LOADU8AV']] = get_access('lx') + ' = ' + 'HEAPU8[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 0];'
CASES[ROPCODES['LOAD16AV']] = get_access('lx') + ' = ' + 'HEAP16[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 1];'
CASES[ROPCODES['LOADU16AV']] = get_access('lx') + ' = ' + 'HEAPU16[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 1];'
CASES[ROPCODES['LOAD32AV']] = get_access('lx') + ' = ' + 'HEAP32[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 2];'
CASES[ROPCODES['STORE8AV']] = 'HEAP8[(' + get_coerced_access('lx') + ') + (ly << 24 >> 24) >> 0] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['STORE16AV']] = 'HEAP16[(' + get_coerced_access('lx') + ') + (ly << 24 >> 24) >> 1] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['STORE32AV']] = 'HEAP32[(' + get_coerced_access('lx') + ') + (ly << 24 >> 24) >> 2] = ' + get_coerced_access('lz') + ';'
CASES[ROPCODES['LOADF64AV']] = get_access('lx', s='d') + ' = ' + '+HEAPF64[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 3];'
CASES[ROPCODES['STOREF64AV']] = 'HEAPF64[(' + get_coerced_access('lx') + ') + (ly << 24 >> 24) >> 3] = ' + get_coerced_access('lz', s='d') + ';'
CASES[ROPCODES['LOADF32AV']] = get_access('lx', s='d') + ' = ' + '+HEAPF32[(' + get_coerced_access('ly') + ') + (inst >> 24) >> 2];'
CASES[ROPCODES['STOREF32AV']] = 'HEAPF32[(' + get_coerced_access('lx') + ') + (ly << 24 >> 24) >> 2] = ' + get_coerced_access('lz', s='d') + ';'

CASES[ROPCODES['STORE8C']] = 'HEAP8[' + get_access('lx') + ' >> 0] = HEAP8[' + get_access('ly') + ' >> 0] | 0;'
CASES[ROPCODES['STORE16C']] = 'HEAP16[' + get_access('lx') + ' >> 1] = HEAP16[' + get_access('ly') + ' >> 1] | 0;'
CASES[ROPCODES['STORE32C']] = 'HEAP32[' + get_access('lx') + ' >> 2] = HEAP32[' + get_access('ly') + ' >> 2] | 0;'
CASES[ROPCODES['STOREF32C']] = 'HEAPF32[' + get_access('lx') + ' >> 2] = +HEAPF32[' + get_access('ly') + ' >> 2];'
CASES[ROPCODES['STOREF64C']] = 'HEAPF64[' + get_access('lx') + ' >> 3] = +HEAPF64[' + get_access('ly') + ' >> 3];'

CASES[ROPCODES['BR']] = 'pc = pc + ((inst >> 16) << 2) | 0; PROCEED_WITHOUT_PC_BUMP;'
CASES[ROPCODES['BRT']] = 'if (' + get_coerced_access('lx') + ') { pc = pc + ((inst >> 16) << 2) | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['BRF']] = 'if (!(' + get_coerced_access('lx') + ')) { pc = pc + ((inst >> 16) << 2) | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['BRA']] = 'pc = HEAP32[pc + 4 >> 2] | 0; PROCEED_WITHOUT_PC_BUMP;'
CASES[ROPCODES['BRTA']] = 'pc = pc + 4 | 0; if (' + get_coerced_access('lx') + ') { pc = HEAP32[pc >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'
CASES[ROPCODES['BRFA']] = 'pc = pc + 4 | 0; if (!(' + get_coerced_access('lx') + ')) { pc = HEAP32[pc >> 2] | 0; PROCEED_WITHOUT_PC_BUMP; }'

CASES[ROPCODES['COND']] = 'pc = pc + 4 | 0; ' + get_access('lx') + ' = (' + get_coerced_access('ly') + ') ? (' + get_coerced_access('lz') + ') : (' + get_coerced_access('(HEAPU8[pc >> 0] | 0)') + ');'
CASES[ROPCODES['CONDD']] = 'pc = pc + 4 | 0; ' + get_access('lx', s='d') + ' = (' + get_coerced_access('ly') + ') ? (' + get_coerced_access('lz', s='d') + ') : (' + get_coerced_access('(HEAPU8[pc >> 0] | 0)', s='d') + ');'

CASES[ROPCODES['GETTDP']] = get_access('lx') + ' = tempDoublePtr;'
#CASES[ROPCODES['GETPC']] = get_access('lx') + ' = pc;'
CASES[ROPCODES['GETTR0']] = get_access('lx') + ' = tempRet0;'
CASES[ROPCODES['SETTR0']] = 'tempRet0 = ' + get_coerced_access('lx') + ';'

if FROUND:
  CASES[ROPCODES['FROUND']] = get_access('lx', s='d') + ' = Math_fround(' + get_coerced_access('ly', s='d') + ');'

# stacktop handling: if allowing async, the very bottom will contain the function being executed,
#                    for stack trace reconstruction. We store [pc of function, curr pc]
#                    where curr pc is the current position in that function, when asyncing
#                    The effective sp, where locals reside, is 8 above that.

def push_stacktop(zero):
  return (' sp = EMTSTACKTOP;' if not ASYNC else ' sp = EMTSTACKTOP + 8 | 0;') if not zero else ''

def pop_stacktop(zero):
  return '//Module.print("exit");\n' + ((' EMTSTACKTOP = sp; ' if not ASYNC else 'EMTSTACKTOP = sp - 8 | 0; ') if not zero else '')

def handle_async_pre_call():
  return 'HEAP32[sp - 4 >> 2] = pc;' if ASYNC else ''

def handle_async_post_call():
  assert not ZERO
  return 'if ((asyncState|0) == 1) { ' + pop_stacktop(zero=False) + ' return }\n' if ASYNC else '' # save pc and exit immediately if currently saving state

CASES[ROPCODES['INTCALL']] = '''
    lz = HEAPU8[(HEAP32[pc + 4 >> 2] | 0) + 1 | 0] | 0; // FUNC inst, see definition above; we read params here
    ly = 0;
    assert(((EMTSTACKTOP + 8|0) <= (EMT_STACK_MAX|0))|0); // for return value
    %s
     %s
      while ((ly|0) < (lz|0)) {
        %s = %s;
        %s = %s;
        ly = ly + 1 | 0;
      }
      %s
      %s
      emterpret(HEAP32[pc + 4 >> 2] | 0);
      %s
    %s
    %s = HEAP32[EMTSTACKTOP >> 2] | 0;
    %s = HEAP32[EMTSTACKTOP + 4 >> 2] | 0;
    pc = pc + (((4 + lz + 3) >> 2) << 2) | 0;
''' % (
  'if ((HEAPU8[(HEAP32[pc + 4 >> 2] | 0) + 4 | 0] | 0) == 0) {' if ZERO else '',
  'if ((asyncState|0) != 2) {' if ASYNC else '',
  get_access('ly', base='EMTSTACKTOP', offset=8 if ASYNC else 0),  get_coerced_access('HEAPU8[pc + 8 + ly >> 0]'),
  get_access('ly', base='EMTSTACKTOP', offset=12 if ASYNC else 4), get_coerced_access('HEAPU8[pc + 8 + ly >> 0]', offset=4),
  '}' if ASYNC else '',
  handle_async_pre_call(),
  handle_async_post_call(),
  ('''} else {
      while ((ly|0) < (lz|0)) {
        %s = %s;
        %s = %s;
        ly = ly + 1 | 0;
      }
      emterpret_z(HEAP32[pc + 4 >> 2] | 0);
    }''' % (
      get_access('ly', base=0),     get_coerced_access('HEAPU8[pc + 8 + ly >> 0]'),
      get_access('ly', base=0, offset=4), get_coerced_access('HEAPU8[pc + 8 + ly >> 0]', offset=4),
  )) if ZERO else '',
  get_access('lx'), get_access('lx', offset=4),
)

CASES[ROPCODES['SWITCH']] = '''
    lz = ''' + get_coerced_access('lz') + ''';
    lx = ((''' + get_coerced_access('lx') + ''') - (''' + get_coerced_access('ly') + ''')) >>> 0; // lx is now relative to the base
    if ((lx >>> 0) >= (lz >>> 0)) { // is the adjusted value too big?
      pc = (pc + (lz << 2)) | 0; // jump to right after the table, where the default is
      PROCEED_WITH_PC_BUMP; // also increment the pc normally, to skip the switch itself
    }
    pc = HEAP32[pc + 4 + (lx << 2) >> 2] | 0; // load from the jump table which is right after this instruction, and set pc
    PROCEED_WITHOUT_PC_BUMP;'''

CASES[ROPCODES['FSLOW']] = get_access('lx') + ' = ' + get_coerced_access('inst >>> 16') + ';'
CASES[ROPCODES['FSLOWD']] = get_access('lx', s='d') + ' = ' + get_coerced_access('inst >>> 16', s='d') + ';'
CASES[ROPCODES['TSLOW']] = get_access('inst >>> 16') + ' = ' + get_coerced_access('lx') + ';'
CASES[ROPCODES['TSLOWD']] = get_access('inst >>> 16', s='d') + ' = ' + get_coerced_access('lx', s='d') + ';'

opcode_used = {}
for opcode in OPCODES:
  opcode_used[opcode] = False

def make_emterpreter(zero=False):
  # return is specialized per interpreter
  CASES[ROPCODES['RET']] = pop_stacktop(zero)
  CASES[ROPCODES['RET']] += 'HEAP32[EMTSTACKTOP >> 2] = ' + get_coerced_access('lx') + '; HEAP32[EMTSTACKTOP + 4 >> 2] = ' + get_coerced_access('lx', offset=4) + '; return;'

  # call is custom generated using information of actual call patterns, and which emterpreter this is
  def make_target_call(i):
    name = global_func_names[i]
    sig = global_func_sigs[i]

    function_pointer_call = shared.JS.is_function_table(name)

    # our local registers are never true floats, and we just do fround calls to ensure correctness, not caring
    # about performance. but when coercing to outside of the emterpreter, we need to know the true sig,
    # and must use frounds
    true_sig = sig
    if function_pointer_call:
      true_sig = name.split('_')[-1]
    elif name in actual_sigs:
      true_sig = actual_sigs[name]

    def fix_coercion(value, s):
      if s == 'f':
        value = 'Math_fround(' + value + ')'
      return value

    ret = name
    if function_pointer_call:
      ret += '[' + get_access('HEAPU8[pc+4>>0]') + ' & %d]' % (next_power_of_two(asm.tables[name].count(',')+1)-1)
    ret += '(' + ', '.join([fix_coercion(get_coerced_access('HEAPU8[pc+%d>>0]' % (i+4+int(function_pointer_call)), s=sig[i+1]), true_sig[i+1]) for i in range(len(sig)-1)]) + ')'
    if sig[0] != 'v':
      ret = shared.JS.make_coercion(fix_coercion(ret, true_sig[0]), sig[0])
      if not ASYNC:
        ret = make_assign(get_access('lx', sig[0]), ret, 'ly' if sig[0] == 'i' else 'ld')
      else:
        # we cannot save the return value immediately! if we are saving the stack, it is meaningless, and would corrupt a local stack variable
        if sig[0] == 'i':
          ret = 'lz = ' + ret
        else:
          assert sig[0] == 'd'
          ret = 'ld = ' + ret
    elif name in actual_sigs and actual_sigs[name][0] != 'v':
      ret = shared.JS.make_coercion(ret, actual_sigs[name][0]) # return value ignored, but need a coercion
    if ASYNC:
      # check if we are asyncing, and if not, it is ok to save the return value
      ret = handle_async_pre_call() + ret + '; ' +  handle_async_post_call()
      if sig[0] != 'v':
        ret += ' else ' + get_access('lx', sig[0]) + ' = ';
        if sig[0] == 'i':
          ret += 'lz'
        else:
          assert sig[0] == 'd'
          ret += 'ld '
        ret += ';'
    extra = len(sig) - 1 + int(function_pointer_call) # [opcode, lx, target, sig], take the usual 4. params are extra
    if extra > 0:
      ret += '; pc = pc + %d | 0' % (4*((extra+3)>>2))
    return '     ' + ret + '; PROCEED_WITH_PC_BUMP;'

  CASES[ROPCODES['EXTCALL']] = 'switch ((inst>>>16)|0) {\n' + \
    '\n'.join(['    case %d: {\n%s\n    }' % (i, make_target_call(i)) for i in range(global_func_id)]) + \
    '\n    default: assert(0);' + \
    '\n   }'

  if ROPCODES['GETGLBI'] not in CASES:
    def make_load(i, t):
      name = rglobal_vars[i]
      return '     ' + get_access('lx', t) + ' = ' + name + '; PROCEED_WITH_PC_BUMP;'

    def make_getglb(suffix, t):
      CASES[ROPCODES['GETGLB' + suffix]] = 'switch (ly|0) {\n' + \
        '\n'.join(['    case %d: {\n%s\n    }' % (i, make_load(i, t)) for i in range(global_var_id) if global_var_types[rglobal_vars[i]] == t]) + \
        '\n    default: assert(0);' + \
        '\n   }'

    make_getglb('I', 'i')
    make_getglb('D', 'd')

    def make_store(i, t):
      name = rglobal_vars[i]
      return '     ' + name + ' = ' + get_coerced_access('lz', t) + '; PROCEED_WITH_PC_BUMP;'

    def make_setglb(suffix, t):
      CASES[ROPCODES['SETGLB' + suffix]] = 'switch ((inst >> 8)&255) {\n' + \
        '\n'.join(['    case %d: {\n%s\n    }' % (i, make_store(i, t)) for i in range(global_var_id) if global_var_types[rglobal_vars[i]] == t]) + \
        '\n    default: assert(0);' + \
        '\n   }'

    make_setglb('I', 'i')
    make_setglb('D', 'd')

  def fix_case(case):
    # we increment pc at the top of the loop. to avoid a pc bump, we decrement it first; this is rare, most opcodes just continue; this avoids any code at the end of the loop
    return case.replace('PROCEED_WITH_PC_BUMP', 'continue').replace('PROCEED_WITHOUT_PC_BUMP', 'pc = pc - 4 | 0; continue').replace('continue; continue;', 'continue;')

  def process(code):
    if not ASSERTIONS: code = code.replace(' assert(', ' //assert(')
    if zero: code = code.replace('sp + ', '')
    return code

  main_loop_prefix = r'''  //if (first) first = false; else print('last lx (' + lx + '): ' + [''' + get_coerced_access('lx') + ',' + get_coerced_access('lx', s='d') + ''']);
  pc = pc + 4 | 0;
  inst = HEAP32[pc>>2]|0;
  lx = (inst >> 8) & 255;
  ly = (inst >> 16) & 255;
  lz = inst >>> 24;
  //Module.print([pc, inst&255, ''' + json.dumps(OPCODES) + '''[inst&255], lx, ly, lz, HEAPU8[pc + 4],HEAPU8[pc + 5],HEAPU8[pc + 6],HEAPU8[pc + 7]]);
'''

  if not INNERTERPRETER_LAST_OPCODE:
    main_loop = main_loop_prefix + r'''
  switch (inst&255) {
%s
   default: assert(0);
  }
''' % ('\n'.join([fix_case('   case %d: %s break;' % (k, CASES[k])) for k in sorted(CASES.keys()) if opcode_used[OPCODES[k]]]))
  else:
    # emit an inner interpreter (innerterpreter) loop, of trivial opcodes that hopefully the JS engine will implement with no spills
    assert OPCODES[-1] == 'FUNC' # we don't need to emit that one
    main_loop = r'''  innerterpreter: while (1) {
%s
   switch (inst&255) {
%s
%s
    default: break innerterpreter;
   }
  }
  switch (inst&255) {
%s
   default: assert(0);
  }
''' % (
  ' ' + '\n '.join(main_loop_prefix.split('\n')),
  '\n'.join([fix_case('    case %d: %s break;' % (ROPCODES[k], CASES[ROPCODES[k]])) for k in OPCODES[:-1][:ROPCODES[INNERTERPRETER_LAST_OPCODE]+1]]),
  '\n'.join([fix_case('    case %d:' % (ROPCODES[k])) for k in OPCODES[:-1][ROPCODES[INNERTERPRETER_LAST_OPCODE]+1:]]),
  '\n'.join([fix_case('   case %d: %s break;' % (ROPCODES[k], CASES[ROPCODES[k]])) for k in OPCODES[:-1][ROPCODES[INNERTERPRETER_LAST_OPCODE]+1:]])
)

  return process(r'''
function emterpret%s(pc) {
 //Module.print('emterpret: ' + pc + ',' + EMTSTACKTOP);
 pc = pc | 0;
 var %sinst = 0, lx = 0, ly = 0, lz = 0;
%s
%s
%s
 assert(((HEAPU8[pc>>0]>>>0) == %d)|0);
 lx = HEAPU16[pc + 2 >> 1] | 0; // num locals
%s
%s
 //print('enter func ' + [pc, HEAPU8[pc + 0],HEAPU8[pc + 1],HEAPU8[pc + 2],HEAPU8[pc + 3],HEAPU8[pc + 4],HEAPU8[pc + 5],HEAPU8[pc + 6],HEAPU8[pc + 7]].join(', '));
 //var first = true;
 pc = pc + 4 | 0;
 while (1) {
%s
 }
 assert(0);
}''' % (
  '' if not zero else '_z',
  'sp = 0, ' if not zero else '',
  '' if not ASYNC and not MEMORY_SAFE else 'var ld = +0;',
  '' if not ASYNC else 'HEAP32[EMTSTACKTOP>>2] = pc;\n',
  push_stacktop(zero),
  ROPCODES['FUNC'],
  (''' EMTSTACKTOP = EMTSTACKTOP + (lx ''' + (' + 1 ' if ASYNC else '') + '''<< 3) | 0;
 assert(((EMTSTACKTOP|0) <= (EMT_STACK_MAX|0))|0);\n''' + (' if ((asyncState|0) != 2) {' if ASYNC else '')) if not zero else '',
  ' } else { pc = (HEAP32[sp - 4 >> 2] | 0) - 8 | 0; }' if ASYNC else '',
  main_loop,
))

# main
if __name__ == '__main__':
  infile = sys.argv[1]
  outfile = sys.argv[2]

  extra_blacklist = []
  if len(sys.argv) >= 4:
    temp = sys.argv[3]
    if temp[0] == '"':
      # response file
      assert temp[1] == '@'
      temp = open(temp[2:-1]).read()
    extra_blacklist = json.loads(temp)

    if len(sys.argv) >= 5:
      temp = sys.argv[4]
      if temp[0] == '"':
        # response file
        assert temp[1] == '@'
        temp = open(temp[2:-1]).read()
      WHITELIST = json.loads(temp)

      if len(sys.argv) >= 6:
        if len(sys.argv) >= 7:
          SWAPPABLE = int(sys.argv[6])

  if ADVISE:
    # Advise the user on which functions should likely be emterpreted
    data = shared.Building.calculate_reachable_functions(infile, list(SYNC_FUNCS))
    advised = data['reachable']
    total_funcs = data['total_funcs']
    print "Suggested list of functions to run in the emterpreter:"
    print "  -s EMTERPRETIFY_WHITELIST='" + str(sorted(advised)).replace("'", '"') + "'"
    print "(%d%% out of %d functions)" % (int((100.0*len(advised))/total_funcs), total_funcs)
    sys.exit(0)

  # final global functions

  asm = asm_module.AsmModule(infile)

  # process blacklist

  for func in extra_blacklist:
    assert func in asm.funcs, 'requested blacklist of %s but it does not exist' % func

  # blacklist all runPostSet* methods

  for func in asm.funcs:
    if func.startswith('runPostSet'):
      extra_blacklist.append(func)

  # finalize blacklist

  BLACKLIST = set(list(BLACKLIST) + extra_blacklist)

  if DEBUG or SWAPPABLE:
    orig = infile + '.orig.js'
    shared.logging.debug('saving original (non-emterpreted) code to ' + orig)
    shutil.copyfile(infile, orig)

  if len(WHITELIST) > 0:
    # we are using a whitelist: fill the blacklist with everything not whitelisted
    BLACKLIST = set([func for func in asm.funcs if func not in WHITELIST])

  # decide which functions will be emterpreted, and find which are externally reachable (from outside other emterpreted code; those will need trampolines)

  emterpreted_funcs = set([func for func in asm.funcs if func not in BLACKLIST and not shared.JS.is_dyn_call(func)])

  tabled_funcs = asm.get_table_funcs()
  exported_funcs = [func.split(':')[0] for func in asm.exports]

  temp = temp_files.get('.js').name # infile + '.tmp.js'

  # find emterpreted functions reachable by non-emterpreted ones, we will force a trampoline for them later

  shared.Building.js_optimizer(infile, ['findReachable'], extra_info={ 'blacklist': list(emterpreted_funcs) }, output_filename=temp, just_concat=True)
  asm = asm_module.AsmModule(temp)
  lines = asm.funcs_js.split('\n')

  reachable_funcs = set([])
  for i in range(len(lines)):
    line = lines[i]
    if line.startswith('// REACHABLE '):
      curr = json.loads(line[len('// REACHABLE '):])
      reachable_funcs = set(list(reachable_funcs) + curr)

  external_emterpreted_funcs = filter(lambda func: func in tabled_funcs or func in exported_funcs or func in reachable_funcs, emterpreted_funcs)

  # process functions, generating bytecode
  shared.Building.js_optimizer(infile, ['emterpretify'], extra_info={ 'emterpretedFuncs': list(emterpreted_funcs), 'externalEmterpretedFuncs': list(external_emterpreted_funcs), 'opcodes': OPCODES, 'ropcodes': ROPCODES, 'ASYNC': ASYNC, 'PROFILING': PROFILING, 'ASSERTIONS': ASSERTIONS }, output_filename=temp, just_concat=True)

  # load the module and modify it
  asm = asm_module.AsmModule(temp)

  relocations = [] # list of places that need to contain absolute offsets, we will add eb to them at runtime to relocate them

  # parse out bytecode and add to mem init file
  all_code = []
  funcs = {}
  lines = asm.funcs_js.split('\n')
  asm.funcs_js = None
  func = None

  # first pass, collect and process bytecode

  global_funcs = {} # 'name|sig' -> id
  global_func_names = {} # id -> name
  global_func_sigs = {} # id -> sig, one name can have multiple sigs
  global_func_id = 0

  global_vars = {}
  rglobal_vars = {}
  global_var_types = {}
  global_var_id = 0

  def note_global(target, j, code):
    global global_var_id
    imp = asm.imports[target]
    ty = asm.get_import_type(imp)
    if target == 'f0':
      assert imp == 'Math_fround(0)'
      # fake it
      ty = 'd'
      imp = '+0'
    assert ty in ['i', 'd'], target
    if code[j] == 'GETGLBI' and ty == 'd':
      # the js optimizer doesn't know all types, we must fix it up here
      assert '.0' in imp or '+' in imp, imp
      code[j] = 'GETGLBD'
      ty = 'd'
    if target not in global_vars:
      global_vars[target] = global_var_id
      rglobal_vars[global_var_id] = target
      global_var_id += 1
      global_var_types[target] = ty
    else:
      assert global_var_types[target] == ty

  call_sigs = {} # signatures appearing for each call target
  def process_code(func, code, absolute_targets):
    global global_func_id
    absolute_start = len(all_code) # true absolute starting point of this function (except for eb)
    #print 'processing code', func, absolute_start
    for i in range(len(code)/4):
      j = i*4
      if code[j] == 'EXTCALL':
        # fix CALL instructions' targets and signatures
        target = code[j+2]
        sig = code[j+3]
        if target not in call_sigs: call_sigs[target] = []
        sigs = call_sigs[target]
        if sig not in sigs: sigs.append(sig)
        fullname = target + '|' + sig
        if fullname not in global_funcs:
          global_funcs[fullname] = global_func_id
          global_func_names[global_func_id] = target
          global_func_sigs[global_func_id] = sig
          global_func_id += 1
        code[j+2] = global_funcs[fullname] & 255
        code[j+3] = global_funcs[fullname] >> 8
        if sig[0] == 'v':
          if code[j+1] == -1: # dummy value for assignment XXX we should not have assignments on void calls
            code[j+1] = 0 # clear it
        else:
          assert code[j+1] >= 0 # there should be a real target here
      elif code[j] in ['GETGLBI', 'GETGLBD']:
        # fix global-accessing instructions' targets
        target = code[j+2]
        note_global(target, j, code)
        code[j+2] = global_vars[target]
      elif code[j] in ['SETGLBI', 'SETGLBD']:
        # fix global-accessing instructions' targets
        target = code[j+1]
        note_global(target, j, code)
        code[j+1] = global_vars[target]
      elif code[j] == 'absolute-value':
        # put the 32-bit absolute value of an abolute target here (correct except for adding eb to relocate at runtime)
        absolute_value = absolute_start + absolute_targets[unicode(code[j+1])]
        #print '  fixing absolute value', code[j+1], absolute_targets[unicode(code[j+1])], absolute_value
        assert absolute_value < (1 << 31)
        assert absolute_value % 4 == 0
        value = bytify(absolute_value)
        for k in range(4):
          code[j + k] = value[k]
        relocations.append(absolute_start + j)

  actual_sigs = {}

  for i in range(len(lines)):
    line = lines[i]
    if line.startswith('function ') and '}' not in line:
      assert not func
    elif line.startswith('// EMTERPRET_INFO '):
      try:
        func, curr, absolute_targets = json.loads(line[len('// EMTERPRET_INFO '):])
      except Exception, e:
        print >> sys.stderr, 'failed to parse code from', line
        raise e
      assert len(curr) % 4 == 0, len(curr)
      funcs[func] = len(all_code) # no operation here should change the length
      if LOG_CODE: print >> sys.stderr, 'raw bytecode for %s:' % func, curr, 'insts:', len(curr)/4
      process_code(func, curr, absolute_targets)
      #print >> sys.stderr, 'processed bytecode for %s:' % func, curr
      all_code += curr
      func = None
      lines[i] = ''
    elif line.startswith('// return type: ['):
      name, sig = line.split('[')[1].split(']')[0].split(',')
      actual_sigs[name] = sig
      lines[i] = ''

  if global_func_id >= 65536:
    msg = 'Too many extcall-able global functions (%d) for emterpreter bytecode' % global_func_id
    if PROFILING:
      msg += '\nDue to --profiling or --profiling-funcs being on, all emterpreter calls are extcalls. Building without those flags might avoid this problem.'
    raise Exception(msg)

  assert global_var_id < 256, [global_vars, global_var_id]

  def post_process_code(code):
    for i in range(len(code)/4):
      j = i*4
      if code[j] == 'absolute-funcaddr':
        # put the 32-bit absolute value of an abolute function here
        absolute_value = funcs[code[j+1]]
        #print '  fixing absolute value', code[j+1], absolute_targets[unicode(code[j+1])], absolute_value
        assert absolute_value < (1 << 31)
        assert absolute_value % 4 == 0
        value = bytify(absolute_value)
        for k in range(4):
          code[j + k] = value[k]
        relocations.append(j)

    # finalize instruction string names to opcodes
    for i in range(len(code)/4):
      j = i*4
      if type(code[j]) in (str, unicode):
        opcode_used[code[j]] = True
        code[j] = ROPCODES[code[j]]

    # sanity checks
    for i in range(len(code)):
      v = code[i]
      assert type(v) == int and v >= 0 and v < 256, [i, v, 'in', code[i-5:i+5], ROPCODES]

  post_process_code(all_code)

  # finalize our mem init
  while len(all_code) % 8 != 0:
    all_code.append(0)

  # second pass, finalize trampolines
  for i in range(len(lines)):
    line = lines[i]
    if line.startswith('function ') and '}' not in line:
      assert not func
      func = line.split(' ')[1].split('(')[0]
    elif line.startswith('}'):
      assert func
      func = None
    elif func and func in funcs:
      call = '(EMTERPRETER_' + func + ')'
      if call in line:
        lines[i] = lines[i].replace(call, '(eb + %s | 0)' % (funcs[func]))

  # finalize funcs JS (first line has the marker, add emterpreters right after that)
  asm.funcs_js = '\n'.join([lines[0], make_emterpreter(), make_emterpreter(zero=True) if ZERO else '', '\n'.join(filter(lambda line: len(line) > 0, lines[1:]))]) + '\n'
  lines = None

  # set up emterpreter stack top (note we must use malloc if in a shared lib, or other enviroment where static memory is sealed)
  js = ['''
var EMTSTACKTOP = getMemory(%s);
var EMT_STACK_MAX = EMTSTACKTOP + %d;
''' % (EMT_STACK_MAX, EMT_STACK_MAX)]

  # write out our bytecode, and runtime relocation logic
  js += ['''
var eb = getMemory(%s);
assert(eb %% 8 === 0);
__ATPRERUN__.push(function() {
''' % len(all_code)]

  if OUTPUT_FILE:
    bytecode_file = open(OUTPUT_FILE, 'wb')
    n = len(all_code)
    while n % 4 != 0:
      n += 1
    bytecode_file.write(''.join(map(chr, all_code)))
    for i in range(len(all_code), n):
      bytecode_file.write(chr(0))
    for i in range(len(relocations)):
      bytes = bytify(relocations[i])
      for j in range(4):
        bytecode_file.write(chr(bytes[j]))
    bytecode_file.close()

    js += ['''
  var bytecodeFile = Module['emterpreterFile'];
  assert(bytecodeFile instanceof ArrayBuffer, 'bad emterpreter file');
  var codeSize = %d;
  HEAPU8.set(new Uint8Array(bytecodeFile).subarray(0, codeSize), eb);
  assert(HEAPU8[eb] === %d);
  assert(HEAPU8[eb+1] === %d);
  assert(HEAPU8[eb+2] === %d);
  assert(HEAPU8[eb+3] === %d);
  var relocationsStart = (codeSize+3) >> 2;
  var relocations = (new Uint32Array(bytecodeFile)).subarray(relocationsStart);
  assert(relocations.length === %d);
  if (relocations.length > 0) assert(relocations[0] === %d);
''' % (len(all_code), all_code[0], all_code[1], all_code[2], all_code[3], len(relocations), relocations[0])]

  else:
    if len(all_code) > 1024*1024:
      shared.logging.warning('warning: emterpreter bytecode is fairly large, %.2f MB. It is recommended to use  -s EMTERPRETIFY_FILE=..  so that it is saved as a binary file, instead of the default behavior which is to embed it as text (as text, it can cause very slow compile and startup times)' % (len(all_code) / (1024*1024.)))

    CHUNK_SIZE = 10240

    i = 0
    while i < len(all_code):
      curr = all_code[i:i+CHUNK_SIZE]
      js += ['''  HEAPU8.set([%s], eb + %d);
''' % (','.join(map(str, curr)), i)]
      i += CHUNK_SIZE

    js += ['''
  var relocations = [];
''']

    i = 0
    while i < len(relocations):
      curr = relocations[i:i+CHUNK_SIZE]
      js += ['''  relocations = relocations.concat([%s]);
''' % (','.join(map(str, curr)))]
      i += CHUNK_SIZE

  # same loop to apply relocations for both OUTPUT_FILE and not
  js += ['''
  for (var i = 0; i < relocations.length; i++) {
    assert(relocations[i] %% 4 === 0);
    assert(relocations[i] >= 0 && relocations[i] < eb + %d); // in range
    assert(HEAPU32[eb + relocations[i] >> 2] + eb < (-1 >>> 0), [i, relocations[i]]); // no overflows
    HEAPU32[eb + relocations[i] >> 2] = HEAPU32[eb + relocations[i] >> 2] + eb;
  }
});
''' % len(all_code)]

  js = ''.join(js)
  if not ASSERTIONS:
    js = js.replace('assert(', '//assert(')
  assert '// {{PRE_LIBRARY}}' in asm.pre_js
  asm.pre_js = asm.pre_js.replace('// {{PRE_LIBRARY}}', '// {{PRE_LIBRARY}}\n' + js)

  # send EMT vars into asm
  asm.pre_js += "Module.asmLibraryArg['EMTSTACKTOP'] = EMTSTACKTOP; Module.asmLibraryArg['EMT_STACK_MAX'] = EMT_STACK_MAX; Module.asmLibraryArg['eb'] = eb;\n"
  extra_vars = 'var EMTSTACKTOP = env.EMTSTACKTOP|0;\nvar EMT_STACK_MAX = env.EMT_STACK_MAX|0;\nvar eb = env.eb|0;\n'
  first_func = asm.imports_js.find('function ')
  if first_func < 0:
    asm.imports_js += extra_vars
  else:
    # imports contains a function (not a true asm function, hidden from opt passes) that we must not be before
    asm.imports_js = asm.imports_js[:first_func] + '\n' + extra_vars + '\n' + asm.imports_js[first_func:]

  asm.write(outfile)

temp_files.clean()

