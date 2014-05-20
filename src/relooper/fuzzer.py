
import random, subprocess, difflib

while True:
  # Random decisions
  num = random.randint(2, 500)
  density = random.random() * random.random()
  decisions = [random.randint(1, num*20) for x in range(num*3)]
  branches = [0]*num
  defaults = [0]*num
  for i in range(num):
    b = set([])
    bs = random.randint(1, max(1, round(density*random.random()*(num-1))))
    for j in range(bs):
      b.add(random.randint(1, num-1))
    b = list(b)
    defaults[i] = random.choice(b)
    b.remove(defaults[i])
    branches[i] = b
  print num, density

  for temp in ['fuzz', 'fuzz.fast.js', 'fuzz.slow.js', 'fuzz.cpp']:
    try:
      os.unlink(temp)
    except:
      pass

  # parts
  entry = '''print('entry'); var label; var state; var modded; var decisions = %s; var index = 0; function check() { if (index == decisions.length) throw 'HALT'; return decisions[index++] }''' % str(decisions)

  slow = entry + '\n'
  for i in range(len(branches[0])):
    if i > 0: slow += 'else '
    b = branches[0]
    slow += 'if (modded == %d) { label = %d; }\n' % (i, b[i]) # TODO: split range 1-n into these options
  if len(branches[0]): slow += 'else '
  slow += 'label = %d;\n' % defaults[0]

  slow += '''
while(1) switch(label) {
'''

  fast = '''

#include <stdlib.h>
#include "Relooper.h"

int main() {
  char *buffer = (char*)malloc(10*1024*1024);
'''

  if random.randint(0, 1) == 0:
    make = False
    fast += '''
  Relooper::SetOutputBuffer(buffer, 10*1024*1024);
'''
  else:
    make = True
    fast += '''
  Relooper::MakeOutputBuffer(%d);
''' % random.randint(1, 1024*1024*10)

  for i in range(1, num):
    slow += '  case %d: print(%d); state = check(); modded = state %% %d\n' % (i, i, len(branches[i])+1)
    b = branches[i]
    for j in range(len(b)):
      slow += '    if (modded == %d) { label = %d; break }\n' % (j, b[j]) # TODO: split range 1-n into these options
    slow += '    label = %d; break\n' % defaults[i]

  branch_vars = []
  for i in range(num):
    branch_var = '"modded"' if len(branches[i]) > 0 and not (len(branches[i]) == 1 and random.random() < 0.5) else 'NULL'
    branch_vars.append(branch_var)

    if i == 0:
      fast += '''
  Block *b%d = new Block("%s", %s);
''' % (i, entry, branch_var)
    else:
      fast += '''  Block *b%d = new Block("print(%d); state = check(); modded = state %% %d; %s", %s);
''' % (i, i, len(branches[i])+1, '// ' + ('.' * int(random.expovariate(0.5/num))), branch_var)

  for i in range(num):
    branch_var = branch_vars[i]
    b = branches[i]
    for j in range(len(b)):
      if branch_var == 'NULL':
        fast += '''  b%d->AddBranchTo(b%d, "modded == %d");
''' % (i, b[j], j)
      else:
        fast += '''  b%d->AddBranchTo(b%d, "case %d:");
''' % (i, b[j], j)
    fast += '''  b%d->AddBranchTo(b%d, NULL);
''' % (i, defaults[i])

  fast += '''
  Relooper r;
'''

  if random.random() < 0.1:
    print 'emulate'
    fast += '''
  r.SetEmulate(true);
'''

  for i in range(num):
    fast += '''  r.AddBlock(b%d);
''' % i

  fast += '''
  r.Calculate(b0);
  printf("\\n\\n");
  r.Render();

  puts(%s);

  return 1;
}
''' % ('buffer' if not make else 'Relooper::GetOutputBuffer()')

  slow += '}'

  open('fuzz.slow.js', 'w').write(slow)
  open('fuzz.cpp', 'w').write(fast)
  print '_'
  slow_out = subprocess.Popen(['mozjs', 'fuzz.slow.js'], stdout=subprocess.PIPE).communicate()[0]

  print '.'
  subprocess.call(['g++', 'fuzz.cpp', 'Relooper.o', '-o', 'fuzz', '-g'])
  print '*'
  subprocess.call(['./fuzz'], stdout=open('fuzz.fast.js', 'w'))
  print '-'
  fast_out = subprocess.Popen(['mozjs', 'fuzz.fast.js'], stdout=subprocess.PIPE).communicate()[0]
  print

  if slow_out != fast_out:
    print ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(slow_out.split('\n'), fast_out.split('\n'), fromfile='slow', tofile='fast')])
    assert False

  #break

