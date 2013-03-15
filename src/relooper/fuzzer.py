
import random, subprocess, difflib

while True:
  # Random decisions
  num = random.randint(2, 250)
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
  entry = '''print('entry'); var label; var state; var decisions = %s; var index = 0; function check() { if (index == decisions.length) throw 'HALT'; return decisions[index++] }''' % str(decisions)

  slow = entry + '\n'
  for i in range(len(branches[0])):
    if i > 0: slow += 'else '
    b = branches[0]
    slow += 'if (state %% %d == %d) { label = %d; }\n' % (len(b)+1, i, b[i]) # TODO: split range 1-n into these options
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
  Relooper::SetOutputBuffer(buffer, 10*1024*1024);
'''

  for i in range(1, num):
    slow += '  case %d: print(%d); state = check(); \n' % (i, i)
    b = branches[i]
    for j in range(len(b)):
      slow += '    if (state %% %d == %d) { label = %d; break }\n' % (len(b)+1, j, b[j]) # TODO: split range 1-n into these options
    slow += '    label = %d; break\n' % defaults[i]

  for i in range(num):
    if i == 0:
      fast += '''
  Block *b%d = new Block("%s");
''' % (i, entry)
    else:
      fast += '''  Block *b%d = new Block("print(%d); state = check();%s");
''' % (i, i, '// ' + ('.' * int(random.expovariate(0.5/num))))

  for i in range(num):
    b = branches[i]
    for j in range(len(b)):
      fast += '''  b%d->AddBranchTo(b%d, "state %% %d == %d");
''' % (i, b[j], len(b)+1, j)
    fast += '''  b%d->AddBranchTo(b%d, NULL);
''' % (i, defaults[i])

  fast += '''
  Relooper r;
'''

  for i in range(num):
    fast += '''  r.AddBlock(b%d);
''' % i

  fast += '''
  r.Calculate(b0);
  printf("\\n\\n");
  r.Render();

  puts(buffer);

  return 1;
}
'''

  slow += '}'

  open('fuzz.slow.js', 'w').write(slow)
  open('fuzz.cpp', 'w').write(fast)
  print '_'
  slow_out = subprocess.Popen(['/home/alon/Dev/odinmonkey/js/src/fast/js', '-m', '-n', 'fuzz.slow.js'], stdout=subprocess.PIPE).communicate()[0]

  print '.'
  subprocess.call(['g++', 'fuzz.cpp', 'Relooper.o', '-o', 'fuzz', '-g'])
  print '*'
  subprocess.call(['./fuzz'], stdout=open('fuzz.fast.js', 'w'))
  print '-'
  fast_out = subprocess.Popen(['/home/alon/Dev/odinmonkey/js/src/fast/js', '-m', '-n', 'fuzz.fast.js'], stdout=subprocess.PIPE).communicate()[0]
  print

  if slow_out != fast_out:
    print ''.join([a.rstrip()+'\n' for a in difflib.unified_diff(slow_out.split('\n'), fast_out.split('\n'), fromfile='slow', tofile='fast')])
    assert False

  #break

