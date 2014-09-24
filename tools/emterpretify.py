#!/usr/bin/env python2

'''
Processes asm.js code to make it run in an emterpreter.

Currently this requires the asm.js code to have been built with -s FINALIZE_ASM_JS=0
'''

import os, sys, re, json
import asm_module, shared

# params

EMT_STACK_MAX = 1024*1024

# consts

BLACKLIST = set(['_malloc', '_free', '_memcpy', '_memset', 'copyTempDouble', 'copyTempFloat', '_strlen', 'stackAlloc', 'setThrew', 'stackRestore', 'setTempRet0', 'getTempRet0', 'stackSave', 'runPostSets', '_emscripten_autodebug_double', '_emscripten_autodebug_float', '_emscripten_autodebug_i8', '_emscripten_autodebug_i16', '_emscripten_autodebug_i32', '_emscripten_autodebug_i64'])

OPCODES = { # l, lx, ly etc - one of 256 locals
  '0':   'SET',     # [lx, ly, 0]          lx = ly (int or float, not double)
  '1':   'GETST',   # [l, 0, 0]            l = STACKTOP
  '2':   'SETST',   # [l, 0, 0]            STACKTOP = l
  '3':   'SETVI',   # [l, vl, vh]          l = v (16-bit signed int)
  '4':   'SETVIB',  # [l, 0, 0] [..v..]    l = 32-bit int in next 32-bit instruction
  '5':   'ADD',     # [lx, ly, lz]         lx = ly + lz (32-bit int)
  '6':   'SUB',     # [lx, ly, lz]         lx = ly - lz (32-bit int)
  '7':   'MUL',     # [lx, ly, lz]         lx = ly * lz (32-bit int)
  '8':   'SDIV',    # [lx, ly, lz]         lx = ly / lz (32-bit signed int)
  '9':   'UDIV',    # [lx, ly, lz]         lx = ly / lz (32-bit unsigned int)
  '10':  'SMOD',    # [lx, ly, lz]         lx = ly % lz (32-bit signed int)
  '11':  'UMOD',    # [lx, ly, lz]         lx = ly % lz (32-bit unsigned int)
  '12':  'NEG',     # [lx, ly, 0]          lx = -ly (int)
  '13':  'LNOT',    # [lx, ly, 0]          ly = !ly (int)
  '14':  'BNOT',    # [lx, ly, 0]          ly = ~ly (int)
  '18':  'EQ',      # [lx, ly, lz]         lx = ly == lz (32-bit int)
  '19':  'NE',      # [lx, ly, lz]         lx = ly != lz (32-bit int)
  '20':  'SLT',     # [lx, ly, lz]         lx = ly < lz (32-bit signed)
  '21':  'ULT',     # [lx, ly, lz]         lx = ly < lz (32-bit unsigned)
  '22':  'SLE',     # [lx, ly, lz]         lx = ly <= lz (32-bit signed)
  '23':  'ULE',     # [lx, ly, lz]         lx = ly <= lz (32-bit unsigned)
  '30':  'AND',     # [lx, ly, lz]         lx = ly & lz
  '31':  'OR',      # [lx, ly, lz]         lx = ly | lz
  '32':  'XOR',     # [lx, ly, lz]         lx = ly ^ lz
  '40':  'SHL',     # [lx, ly, lz]         lx = ly << lz
  '41':  'ASHR',    # [lx, ly, lz]         lx = ly >> lz
  '42':  'LSHR',    # [lx, ly, lz]         lx = ly >>> lz

  '60':  'SETD',    # [lx, ly, lz]         lx = ly (double)
  '61':  'SETVD',   # [lx, vl, vh]         lx = ly (16 bit signed int, converted into double)
  '65':  'ADDD',    # [lx, ly, lz]         lx = ly + lz (double)
  '66':  'SUBD',    # [lx, ly, lz]         lx = ly - lz (double)
  '67':  'MULD',    # [lx, ly, lz]         lx = ly * lz (double)
  '69':  'DIVD',    # [lx, ly, lz]         lx = ly / lz (double)
  '70':  'MODD',    # [lx, ly, lz]         lx = ly % lz (double)
  #'72':  'NEGD',    # [lx, ly, 0]          lx = -ly (double)
  '78':  'EQD',     # [lx, ly, lz]         lx = ly == lz (double)
  '79':  'NED',     # [lx, ly, lz]         lx = ly != lz (double)
  '80':  'LTD',     # [lx, ly, lz]         lx = ly < lz (signed)
  '81':  'LED',     # [lx, ly, lz]         lx = ly < lz (double)
  '82':  'GTD',     # [lx, ly, lz]         lx = ly <= lz (double)
  '83':  'GED',     # [lx, ly, lz]         lx = ly <= lz (double)
  '90':  'D2I',     # [lx, ly, 0]          lx = ~~ly (double-to-int)
  '91':  'I2D',     # [lx, ly, 0]          lx = +ly (int-to-double)

  '100': 'LOAD8',   # [lx, ly, 0]          lx = HEAP8[ly >> 0]
  '110': 'LOAD16',  # [lx, ly, 0]          lx = HEAP16[ly >> 1]
  '120': 'LOAD32',  # [lx, ly, 0]          lx = HEAP32[ly >> 2]
  '130': 'STORE8',  # [lx, ly, 0]          HEAP8[lx >> 2] = ly
  '140': 'STORE16', # [lx, ly, 0]          HEAP16[lx >> 2] = ly
  '150': 'STORE32', # [lx, ly, 0]          HEAP32[lx >> 2] = ly

  '151': 'LOADF64',  # [lx, ly, 0]         lx = HEAPF64[ly >> 3]
  '152': 'STOREF64', # [lx, ly, 0]         HEAPF64[lx >> 3] = ly
  '153': 'LOADF32',  # [lx, ly, 0]         lx = HEAPF32[ly >> 3]
  '154': 'STOREF32', # [lx, ly, 0]         HEAPF32[lx >> 3] = ly

  '159': 'BR',      # [0, tl, th]          jump t instructions (multiple of 4)
  '160': 'BRT',     # [cond, tl, th]       if cond, jump t instructions (multiple of 4)
  '161': 'BRF',     # [cond, tl, th]       if !cond, jump t instructions (multiple of 4)
  #'170': 'ABR',     # [lx, 0, 0, 0]        absolute branch to address lx (assumed divisible by 4)
  '200': 'GETTDP',  # [l, 0, 0]            l = tempDoublePtr
  #'201': 'GETPC',   # [l, 0, 0]            l = pc
  '250': 'CALL',    # [lx, target, sig] [params...]   (lx = ) target(params..) lx's existence and type depend on the target's actual callsig;
                    #                                 this instruction can take multiple 32-bit instruction chunks
                    #                                 if target is a function table, then the first param is the index of the register holding the function pointer
  '251': 'SWITCH',  # [lx, ly, lz]         switch (lx) { .. }. followed by a jump table for values in range [ly..ly+lz), after which is the default (which might be empty)
  '254': 'RET',     # [l, 0, 0]            return l (depending on which emterpreter_x we are in, has the right type)
  '255': 'FUNC',    # [n, 0, 0]            function with n locals (each taking 64 bits)
}

assert len(OPCODES.values()) == len(set(OPCODES.values())) # no dupe names

ROPCODES = {}
for o in OPCODES:
  ROPCODES[OPCODES[o]] = int(o);

GLOBAL_BASE = 8

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

def get_access(l, s='i'):
  if s == 'i':
    return 'HEAP32[sp + (' + l + ' << 3) >> 2]'
  elif s == 'd':
    return 'HEAPF64[sp + (' + l + ' << 3) >> 3]'
  else:
    assert 0

def get_coerced_access(l, s='i', unsigned=False):
  if s == 'i':
    if not unsigned:
      return get_access(l, s) + '|0'
    else:
      return get_access(l, s) + '>>>0'
  elif s == 'd':
    return '+' + get_access(l, s)
  else:
    assert 0

CASES = {}
CASES[ROPCODES['SET']] = get_access('lx') + ' = ' + get_coerced_access('ly') + ';'
CASES[ROPCODES['GETST']] = 'HEAP32[sp + (lx << 3) >> 2] = STACKTOP;'
CASES[ROPCODES['SETST']] = 'STACKTOP = HEAP32[sp + (lx << 3) >> 2]|0;'
CASES[ROPCODES['SETVI']] = 'HEAP32[sp + (lx << 3) >> 2] = inst >> 16;'
CASES[ROPCODES['SETVIB']] = 'pc = pc + 4 | 0; ' + get_access('lx') + ' = HEAP32[pc >> 2] | 0;'
CASES[ROPCODES['ADD']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') + (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['SUB']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') - (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['MUL']] = get_access('lx') + ' = Math_imul(' + get_coerced_access('ly') + ', ' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['SDIV']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') / (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['UDIV']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') / (' + get_coerced_access('lz', unsigned=True) + ') >>> 0;'
CASES[ROPCODES['SMOD']] = get_access('lx') + ' = (' + get_coerced_access('ly') + ') % (' + get_coerced_access('lz') + ') | 0;'
CASES[ROPCODES['UMOD']] = get_access('lx') + ' = (' + get_coerced_access('ly', unsigned=True) + ') % (' + get_coerced_access('lz', unsigned=True) + ') >>> 0;'
CASES[ROPCODES['NEG']] = get_access('lx') + ' = -(' + get_coerced_access('ly') + ');'
CASES[ROPCODES['LNOT']] = get_access('lx') + ' = !(' + get_coerced_access('ly') + ');'
CASES[ROPCODES['BNOT']] = get_access('lx') + ' = ~(' + get_coerced_access('ly') + ');'
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

CASES[ROPCODES['SETD']] = get_access('lx', s='d') + ' = ' + get_coerced_access('ly', s='d') + ';'
CASES[ROPCODES['SETVD']] = get_access('lx', s='d') + ' = +(inst >> 16);'
CASES[ROPCODES['ADDD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') + (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['SUBD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') - (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['MULD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') * (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['DIVD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') / (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['MODD']] = get_access('lx', s='d') + ' = (' + get_coerced_access('ly', s='d') + ') % (' + get_coerced_access('lz', s='d') + ');'
CASES[ROPCODES['EQD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') == (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['NED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') != (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['LTD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') < (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['LED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') <= (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['GTD']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') > (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['GED']] = get_access('lx') + ' = (' + get_coerced_access('ly', s='d') + ') >= (' + get_coerced_access('lz', s='d') + ') | 0;'
CASES[ROPCODES['D2I']] = get_access('lx') + ' = ~~(' + get_coerced_access('ly', s='d') + ');'
CASES[ROPCODES['I2D']] = get_access('lx', s='d') + ' = +(' + get_coerced_access('ly') + ');'

CASES[ROPCODES['LOAD8']] = get_access('lx') + ' = ' + 'HEAP8[' + get_access('ly') + ' >> 0];'
CASES[ROPCODES['LOAD16']] = get_access('lx') + ' = ' + 'HEAP16[' + get_access('ly') + ' >> 1];'
CASES[ROPCODES['LOAD32']] = get_access('lx') + ' = ' + 'HEAP32[' + get_access('ly') + ' >> 2];'
CASES[ROPCODES['STORE8']] = 'HEAP8[' + get_access('lx') + ' >> 0] = ' + get_coerced_access('ly') + ';';
CASES[ROPCODES['STORE16']] = 'HEAP16[' + get_access('lx') + ' >> 1] = ' + get_coerced_access('ly') + ';';
CASES[ROPCODES['STORE32']] = 'HEAP32[' + get_access('lx') + ' >> 2] = ' + get_coerced_access('ly') + ';';

CASES[ROPCODES['LOADF64']] = get_access('lx', s='d') + ' = ' + 'HEAPF64[' + get_access('ly') + ' >> 3];'
CASES[ROPCODES['STOREF64']] = 'HEAPF64[' + get_access('lx') + ' >> 3] = ' + get_coerced_access('ly', s='d') + ';';
CASES[ROPCODES['LOADF32']] = get_access('lx', s='d') + ' = ' + 'HEAPF32[' + get_access('ly') + ' >> 2];'
CASES[ROPCODES['STOREF32']] = 'HEAPF32[' + get_access('lx') + ' >> 2] = ' + get_coerced_access('ly', s='d') + ';';

CASES[ROPCODES['BR']] = 'pc = pc + ((inst >> 16) << 2) | 0; continue;'
CASES[ROPCODES['BRT']] = 'if (' + get_coerced_access('lx') + ') { pc = pc + ((inst >> 16) << 2) | 0; continue; }'
CASES[ROPCODES['BRF']] = 'if (!(' + get_coerced_access('lx') + ')) { pc = pc + ((inst >> 16) << 2) | 0; continue; }'
#CASES[ROPCODES['ABR']] = 'pc = ' + get_coerced_access('lx') + '; continue;'
CASES[ROPCODES['GETTDP']] = 'HEAP32[sp + (lx << 3) >> 2] = tempDoublePtr;'
#CASES[ROPCODES['GETPC']] = 'HEAP32[sp + (lx << 3) >> 2] = pc;'
CASES[ROPCODES['SWITCH']] = '''
    lz = ''' + get_coerced_access('lz') + ''';
    lx = ((''' + get_coerced_access('lx') + ''') - (''' + get_coerced_access('ly') + ''')) >>> 0; // lx is now relative to the base
    if ((lx >>> 0) >= (lz >>> 0)) { // is the adjusted value too big?
      pc = (pc + (lz << 2)) | 0; // jump to right after the table, where the default is
      continue;
    }
    pc = HEAP32[pc + 4 + (lx << 2) >> 2] | 0; // load from the jump table which is right after this instruction, and set pc
    continue;'''

def make_emterpreter(t):
  # return is specialized per interpreter
  CASES[ROPCODES['RET']] = 'EMTSTACKTOP = sp; '
  if t == 'void':
    CASES[ROPCODES['RET']] += 'return;'
  elif t == 'int':
    CASES[ROPCODES['RET']] += 'return HEAP32[sp + (lx << 3) >> 2]|0;'
  elif t == 'double':
    CASES[ROPCODES['RET']] += 'return +HEAPF64[sp + (lx << 3) >> 3];'

  # call is generated using information of actual call patterns
  if ROPCODES['CALL'] not in CASES:
    #print >> sys.stderr, call_sigs
    def make_target_call(i):
      name = rglobal_funcs[i]
      function_pointer_call = name.startswith('FUNCTION_TABLE_')
      if name not in call_sigs: return None
      sigs = call_sigs[name]

      def make_target_call_sig(sig):
        ret = name
        if function_pointer_call:
          ret += '[' + get_access('HEAP8[pc+4>>0]') + ' & %d]' % (next_power_of_two(asm.tables[name].count(',')+1)-1)
        ret += '(' + ', '.join([get_coerced_access('HEAP8[pc+%d>>0]' % (i+4+(1 if function_pointer_call else 0)), s=sig[i+1]) for i in range(len(sig)-1)]) + ')'
        if sig[0] != 'v':
          ret = get_access('lx', sig[0]) + ' = ' + shared.JS.make_coercion(ret, sig[0])
        elif name in actual_return_types and actual_return_types[name] != 'v':
          ret = shared.JS.make_coercion(ret, actual_return_types[name]) # return value ignored, but need a coercion
        extra = len(sig) - 1 # [opcode, lx, target, sig], take the usual 4. params are extra
        if extra > 0:
          ret += '; pc = pc + %d | 0' % (4*((extra+3)>>2))
        return '     ' + ret + '; break;'

      if len(sigs) == 1:
        return make_target_call_sig(sigs[0])
      else:
        assert len(sigs) == 2
        return 'if ((HEAP8[pc+3>>0]|0) == 0) { ' + make_target_call_sig(sigs[0]) + ' } else { ' + make_target_call_sig(sigs[1]) + ' }'

    CASES[ROPCODES['CALL']] = 'switch (ly|0) {\n' + \
      '\n'.join(filter(lambda x: 'None' not in x, ['    case %d: {\n%s\n    }' % (i, make_target_call(i)) for i in range(global_id-1)])) + \
      '\n    default: assert(0);' + \
      '\n   }'

  def fix_case(case):
    # we increment pc at the top of the loop. cases doing 'continue' really need to decrement it
    return case.replace('continue;', 'pc = pc - 4 | 0; continue;')

  return r'''
function emterpret%s%s(pc) {
 pc = pc | 0;
 var sp = 0, inst = 0, lx = 0, ly = 0, lz = 0;
 sp = EMTSTACKTOP;
 assert(((HEAPU8[pc>>0]>>>0) == %d)|0);
 EMTSTACKTOP = EMTSTACKTOP + (HEAP8[pc + 1 >> 0] << 3) | 0;
 assert(((EMTSTACKTOP|0) <= (EMT_STACK_MAX|0))|0);
 while (1) {
  //printErr('last lx: ' + [HEAP32[sp + (lx << 3) >> 2]|0, +HEAPF64[sp + (lx << 3) >> 3]]);
  pc = pc + 4 | 0;
  inst = HEAP32[pc>>2]|0;
  lx = (inst >> 8) & 255;
  ly = (inst >> 16) & 255;
  lz = inst >>> 24;
  //printErr([pc, inst&255, %s[inst&255], lx, ly, lz].join(', '));
  //printErr('  ' + Array.prototype.slice.call(HEAPU8, sp, sp+8));
  switch (inst&255) {
%s
   default: assert(0);
  }
  //printErr('result in ' + lx + ': ' + Array.prototype.slice.call(HEAPU8, sp+8*lx, sp+8*(lx+1)));
 }
 EMTSTACKTOP = sp;
 %s
}''' % (
  '_' if t != 'void' else '',
  '' if t == 'void' else t[0],
  ROPCODES['FUNC'],
  json.dumps(OPCODES),
  '\n'.join([fix_case('   case %d: %s break;' % (k, CASES[k])) for k in sorted(CASES.keys())]),
  '' if t == 'void' else 'return %s;' % shared.JS.make_initializer(t[0], settings)
)

# main

infile = sys.argv[1]
outfile = sys.argv[2]
force_memfile = sys.argv[3] if len(sys.argv) >= 4 else None

print 'emterpretifying %s to %s' % (infile, outfile)

# final global functions

asm = asm_module.AsmModule(infile)
global_funcs = {}
rglobal_funcs = {}
global_id = 0
for k, v in asm.imports.iteritems():
  if '|' not in v and '+' not in v and 'new ' not in v and '.0' not in v and v != '0':
    global_funcs[k] = global_id
    rglobal_funcs[global_id] = k
    global_id += 1

lines = asm.funcs_js.split('\n')
for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function ') and '}' not in line:
    func = line.split(' ')[1].split('(')[0]
    global_funcs[func] = global_id
    rglobal_funcs[global_id] = func
    global_id += 1

for table in asm.tables:
  global_funcs[table] = global_id
  rglobal_funcs[global_id] = table
  global_id += 1

assert global_id < 256

# process functions, generating bytecode
temp = infile + '.tmp.js'
shared.Building.js_optimizer(infile, ['emterpretify'], extra_info={ 'blacklist': list(BLACKLIST), 'globalFuncs': global_funcs, 'opcodes': OPCODES, 'ropcodes': ROPCODES }, output_filename=temp)

# load the module and modify it
asm = asm_module.AsmModule(temp)

in_mem_file = infile + '.mem'
in_mem_file_base = os.path.basename(in_mem_file)
out_mem_file = outfile + '.mem'
out_mem_file_base = os.path.basename(out_mem_file)

assert in_mem_file_base in asm.pre_js, 'we assume a mem init file for now (looked for %s)' % in_mem_file
if not force_memfile:
  asm.pre_js = asm.pre_js.replace(in_mem_file_base, out_mem_file_base)
  assert os.path.exists(in_mem_file), 'need to find mem file at %s' % in_mem_file
else:
  out_mem_file = force_memfile
  out_mem_file_base = os.path.basename(out_mem_file)
mem_init = map(ord, open(in_mem_file, 'rb').read())
zero_space = asm.staticbump - len(mem_init)
assert zero_space >= 0 # can be positive, if we add a bump of zeros

assert 'GLOBAL_BASE: 8,' in asm.pre_js

# calculate where code will start
while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
code_start = len(mem_init) + GLOBAL_BASE

# parse out bytecode and add to mem init file
all_code = []
funcs = {}
lines = asm.funcs_js.split('\n')
asm.funcs_js = None
func = None

# first pass, collect and process bytecode

call_sigs = {} # signatures appearing for each call target
def process_code(func, code, absolute_targets):
  absolute_start = code_start + len(all_code) # true absolute starting point of this function
  #print 'processing code', func, absolute_start
  for i in range(len(code)/4):
    j = i*4
    if code[j] == 'CALL':
      # fix CALL instructions' targets and signatures
      target = code[j+2]
      sig = code[j+3]
      if target not in call_sigs: call_sigs[target] = []
      sigs = call_sigs[target]
      if sig not in sigs: sigs.append(sig)
      code[j+2] = global_funcs[target]
      code[j+3] = sigs.index(sig)
      if sig[0] == 'v':
        assert code[j+1] == -1 # dummy value for assignment
        code[j+1] = 0 # clear it
      else:
        assert code[j+1] >= 0 # there should be a real target here
    elif code[j] == 'absolute-value':
      # put the 32-bit absolute value of an abolute target here
      #print '  fixing absolute value', code[j+1], absolute_targets[unicode(code[j+1])], absolute_start + absolute_targets[unicode(code[j+1])]
      value = bytify(absolute_start + absolute_targets[unicode(code[j+1])])
      for k in range(4):
        code[j + k] = value[k]

  # finalize instruction string names to opcodes
  for i in range(len(code)/4):
    j = i*4
    if type(code[j]) in (str, unicode):
      code[j] = ROPCODES[code[j]]

actual_return_types = {}

for i in range(len(lines)):
  line = lines[i]
  if line.startswith('function ') and '}' not in line:
    assert not func
    func = line.split(' ')[1].split('(')[0]
  elif line.startswith('}'):
    assert func
    try:
      curr, absolute_targets = json.loads(line[4:])
    except:
      if '[' in line: print >> sys.stderr, 'failed to parse code from', line
      curr = None
    if curr is not None:
      assert len(curr) % 4 == 0, curr
      funcs[func] = len(all_code) # no operation here should change the length
      #print >> sys.stderr, 'raw bytecode for %s:' % func, curr
      process_code(func, curr, absolute_targets)
      #print >> sys.stderr, 'processed bytecode for %s:' % func, curr
      all_code += curr
    func = None
    lines[i] = '}'
  elif line.startswith('// return type: ['):
    name, ret = line.split('[')[1].split(']')[0].split(',')
    if ret == 'undefined':
      actual_return_types[name] = 'v'
    elif ret == '0':
      actual_return_types[name] = 'i'
    elif ret == '1':
      actual_return_types[name] = 'd'
    elif ret == '2':
      actual_return_types[name] = 'f'

# create new mem init
mem_init = mem_init + all_code
asm.staticbump += len(all_code)

while len(mem_init) % 8 != 0:
  mem_init.append(0)
  asm.staticbump += 1
stack_start = len(mem_init)
asm.staticbump += EMT_STACK_MAX

open(out_mem_file, 'wb').write(''.join(map(chr, mem_init)))

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
      lines[i] = lines[i].replace(call, '(%s)' % (funcs[func] + code_start))

# finalize funcs JS
asm.funcs_js = '\n'.join(['\n'.join(lines), make_emterpreter('void'), make_emterpreter('int'), make_emterpreter('double')])
lines = None

# set up emterpreter stack top
asm.set_pre_js(js='var EMTSTACKTOP = STATIC_BASE + %s, EMT_STACK_MAX = EMTSTACKTOP + %d;' % (stack_start, EMT_STACK_MAX))

# send EMT vars into asm
brace = asm.post_js.find(', {') + 3
asm.post_js = asm.post_js[:brace] + ' "EMTSTACKTOP": EMTSTACKTOP, "EMT_STACK_MAX": EMT_STACK_MAX, ' + asm.post_js[brace:]
asm.imports_js += 'var EMTSTACKTOP = env.EMTSTACKTOP|0;\nvar EMT_STACK_MAX = env.EMT_STACK_MAX|0;\n'

asm.write(outfile)

