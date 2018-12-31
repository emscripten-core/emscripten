'''
From llvm migration helper scripts, commits like

commit 32b845d223393d9f5e7317b9e754a52b79543de2
Author: David Blaikie <dblaikie@gmail.com>
Date:   Thu Apr 16 23:24:18 2015 +0000

    [opaque pointer type] Add textual IR support for explicit type parameter to the call instruction
    
    See r230786 and r230794 for similar changes to gep and load
    respectively.

'''

import fileinput
import sys
import re

def loads():
  pat = re.compile(r"((?:=|:|^)\s*load (?:atomic )?(?:volatile )?(.*?))(| addrspace\(\d+\) *)\*($| *(?:%|@|null|undef|blockaddress|getelementptr|addrspacecast|bitcast|inttoptr|\[\[[a-zA-Z]|\{\{).*$)")

  for line in sys.stdin:
    sys.stdout.write(re.sub(pat, r"\1, \2\3*\4", line))

def geps():
  rep = re.compile(r"(getelementptr(?:\s+inbounds)?\s*\()((<\d*\s+x\s+)?([^@]*?)(|\s*addrspace\(\d+\))\s*\*(?(3)>)\s*)(?=$|%|@|null|undef|blockaddress|getelementptr|addrspacecast|bitcast|inttoptr|zeroinitializer|<|\[\[[a-zA-Z]|\{\{)", re.MULTILINE | re.DOTALL)
  
  def conv(match):
    line = match.group(1)
    line += match.group(4)
    line += ", "
    line += match.group(2)
    return line
  
  line = sys.stdin.read()
  off = 0
  for match in re.finditer(rep, line):
    sys.stdout.write(line[off:match.start()])
    sys.stdout.write(conv(match))
    off = match.end()
  sys.stdout.write(line[off:])

def calls():
  pat = re.compile(r'((?:=|:|^|\s)call\s(?:[^@]*?))(\s*$|\s*(?:(?:\[\[[a-zA-Z0-9_]+\]\]|[@%](?:(")?[\\\?@a-zA-Z0-9_.]*?(?(3)"|)|{{.*}}))(?:\(|$)|undef|inttoptr|bitcast|null|asm).*$)')
  addrspace_end = re.compile(r"addrspace\(\d+\)\s*\*$")
  func_end = re.compile("(?:void.*|\)\s*)\*$")
  
  def conv(match, line):
    if not match or re.search(addrspace_end, match.group(1)) or not re.search(func_end, match.group(1)):
      return line
    return line[:match.start()] + match.group(1)[:match.group(1).rfind('*')].rstrip() + match.group(2) + line[match.end():]
  
  for line in sys.stdin:
    sys.stdout.write(conv(re.search(pat, line), line))

def ours():
  for line in sys.stdin:
    #line = line.replace('getelementptr inbounds i8** ', 'getelementptr inbounds i8*, i8** ') 
    #if line.startswith('!'): line = ''
    #if ', !dbg !' in line:
    #  line = line.split(', !dbg !')[0] + '\n'
    if ', !tbaa !' in line:
      line = line.split(', !tbaa !')[0] + '\n'
    if ', !tbaa.struct !' in line:
      line = line.split(', !tbaa.struct !')[0] + '\n'
    if ', !prof !' in line:
      line = line.split(', !prof !')[0] + '\n'
    #line = line.replace('getelementptr [4194304 x i8]* ', 'getelementptr [4194304 x i8], [4194304 x i8]* ')
    line = re.sub(r"getelementptr \[(\w+) x ([%._@\*\w]+)\]\* ", lambda m: "getelementptr [%s x %s], [%s x %s]* " % (m.groups(0)[0], m.groups(0)[1], m.groups(0)[0], m.groups(0)[1]), line)
    line = re.sub(r"getelementptr inbounds \[(\w+) x ([%._@\*\w]+)\]\* ", lambda m: "getelementptr inbounds [%s x %s], [%s x %s]* " % (m.groups(0)[0], m.groups(0)[1], m.groups(0)[0], m.groups(0)[1]), line)
    line = re.sub(r"getelementptr inbounds \[(\w+) x (\w+)\]\* ", lambda m: "getelementptr inbounds [%s x %s], [%s x %s]* " % (m.groups(0)[0], m.groups(0)[1], m.groups(0)[0], m.groups(0)[1]), line)
    line = re.sub(r" getelementptr inbounds ([%._@\w]+)\* ([%._@\w]+),", lambda m: " getelementptr inbounds %s, %s* %s," % (m.groups(0)[0], m.groups(0)[0], m.groups(0)[1]), line)
    line = line.replace('getelementptr inbounds [100 x [100 x i32]]* ', 'getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* ')
    line = line.replace('getelementptr inbounds [10 x [1 x %union.U4]]* ', 'getelementptr inbounds [10 x [1 x %union.U4]], [10 x [1 x %union.U4]]* ')
    line = line.replace('getelementptr inbounds [9 x [3 x [7 x %struct.S1]]]* ', 'getelementptr inbounds [9 x [3 x [7 x %struct.S1]]], [9 x [3 x [7 x %struct.S1]]]* ')
    line = line.replace('getelementptr inbounds [3 x [6 x [4 x i16]]]* ', 'getelementptr inbounds [3 x [6 x [4 x i16]]], [3 x [6 x [4 x i16]]]* ')
    marker = ' getelementptr '
    if marker in line:
      start = line.find(marker) + len(marker)
      if line[start:].startswith('inbounds '):
        start += len('inbounds ')
      if line[start] != '(':
        i = start
        levels = 0
        while i < len(line) and (levels > 0 or line[i] not in [',', ' ']):
          if line[i] in ['(', '{', '[']: levels += 1
          elif line[i] in [')', '}', ']']:
            levels -= 1
            assert levels >= 0, line
          i += 1
        if i < len(line) and line[i] == ' ':
          # needs a fix
          if line[i-1] == '*':
            t = line[start:i-1]
            line = line[:start] + t + ', ' + t + '* ' + line[i+1:]
    sys.stdout.write(line)

# pick one of the above
ours()

