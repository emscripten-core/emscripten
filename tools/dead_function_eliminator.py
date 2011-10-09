#!/usr/bin/env python

'''
LLVM doesn't appear to have a way to remove unused functions. This little
script will do that. It requires annotations to be in the .ll file it parses
(run llvm-dis with -show-annotations).

Closure compiler can remove unused functions, however it is much faster
to remove them before Emscripten runs.
'''

import os, sys, re

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

infile = sys.argv[1]
outfile = sys.argv[2]

lines = open(infile, 'r').read().split('\n')

class Dummy: pass

# Discover functions

functions = {}

func_header = re.compile('^define[^@]* (?P<ident>@\w+)\(.* {$')
func_footer = '}'
func_annot = re.compile('^; \[#uses=(?P<uses>\d+)\]$')

print '\nDiscovery pass 1\n'

for i in range(len(lines)):
  line = lines[i]
  m_header = func_header.match(line)
  if m_header:
    m_annot = func_annot.match(lines[i-1])
    assert m_annot
    ident = m_header.group('ident')
    func = functions[ident] = Dummy()
    func.uses = int(m_annot.group('uses')) # XXX This info from LLVM is very inaccurate
    func.callers = set()
    func.callees = set()

for ident in functions.iterkeys():
  func = functions[ident]
  #print ident

if '@main' not in functions:
  print 'No @main found, not running DFE'
  import shutil
  shutil.copy(infile, outfile)
  sys.exit(1)

print '\nDiscovery pass 2\n'

ident_frag = re.compile('[, ](?P<ident>@\w+)[, ()}\]]')
metadata = re.compile('!(?P<index>\d+) = metadata !{.*')

inside = None

for i in range(len(lines)):
  line = lines[i]
  if line == func_footer:
    inside = None
    continue
  m_header = func_header.match(line)
  if m_header:
    inside = m_header.group('ident')
    continue
  meta = metadata.match(line)
  for m in re.finditer(ident_frag, line):
    ident = m.groups('ident')[0]
    if ident not in functions: continue
    if inside != ident:
      functions[ident].callers.add(inside if inside else ('GLOBAL' if not meta else 'METADATA_'+str(i)+'_'+meta.groups('index')[0]))
      if inside:
        functions[inside].callees.add(ident)

functions['@main'].callers.add('GLOBAL')

for ident in functions.iterkeys():
  func = functions[ident]
  #print ident, func.uses, func.callers#, 'WARNING!' if func.uses != len(func.callers) else ''

# Garbage collect

print '\nGC pass 1\n'

for ident in functions.iterkeys():
  func = functions[ident]
  func.root = func.marked = False
  for caller in func.callers:
    if caller == 'GLOBAL':
      func.root = True
      #print 'ROOT:', ident
      break

def mark_and_recurse(func):
  if func.marked: return
  func.marked = True
  for callee in func.callees:
    if callee == 'GLOBAL': continue
    mark_and_recurse(functions[callee])

for ident in functions.iterkeys():
  func = functions[ident]
  if func.root:
    mark_and_recurse(func)

marked = unmarked = 0
for ident in functions.iterkeys():
  func = functions[ident]
  if func.root: assert func.marked
  #print ident, func.marked
  marked += func.marked
  unmarked += 1-func.marked

dead_metadatas = set() # metadata pruning pass
for ident in functions.iterkeys():
  func = functions[ident]
  if func.marked: continue
  for caller in func.callers:
    if caller.startswith('METADATA_'):
      dummy, i, index = caller.split('_')
      lines[int(i)] = ';'
      dead_metadatas.add(int(index))
inner_metadata = re.compile('metadata !(?P<index>\d+)')
for i in range(len(lines)):
  line = lines[i]
  if metadata.match(line):
    lines[i] = re.sub(inner_metadata, lambda m: 'i32 0' if int(m.groups('index')[0]) in dead_metadatas else m.string[m.start():m.end()], line)

print 'Marked: ', marked, ', unmarked: ', unmarked

# Write

print '\nWriting\n'

inside = None
marked = False

target = open(outfile, 'w')

for line in lines:
  if line == func_footer:
    inside = None
    if marked: target.write(line + '\n')
    continue
  m_header = func_header.match(line)
  if m_header:
    inside = m_header.group('ident')
    marked = functions[inside].marked
#########  if metadata.match(line): continue # metadata is not enough to keep things alive
  if line.startswith('!llvm.dbg.sp = '): continue
  if not inside or marked:
    if len(line) > 0:
      target.write(line + '\n')

target.close()

