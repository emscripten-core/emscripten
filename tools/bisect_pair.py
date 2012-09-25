'''
Given two similar files, for example one with an additional optimization pass,
and with different results, will bisect between them to find the smallest
diff that makes the outputs different.
'''

import os, sys, shutil
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

file1 = open(sys.argv[1]).read()
file2 = open(sys.argv[2]).read()

leftf = open('left', 'w')
leftf.write(file1)
leftf.close()

rightf = open('right', 'w')
rightf.write(file2)
rightf.close()

print 'running files'
left_result = run_js('left', stderr=PIPE)
right_result = run_js('right', stderr=PIPE) # right as in left-right, not as in correct
assert left_result != right_result

# Calculate diff chunks
print 'diffing'
diff = Popen(['diff', '-U', '5', 'left', 'right'], stdin=PIPE, stdout=PIPE, stderr=PIPE).communicate()[0].split('\n')
pre_diff = diff[:2]
diff = diff[2:]

chunks = []
curr = []
for i in range(len(diff)):
  if diff[i].startswith('@'):
    if len(curr) > 0:
      chunks.append(curr)
    curr = [diff[i]]
  else:
    curr.append(diff[i])
if len(curr) > 0:
  chunks.append(curr)

# Bisect both sides of the span, until we have a single chunk
high = len(chunks)

print 'beginning bisection, %d chunks' % high

for mid in range(high):
  print '  current: %d' % mid
  # Take chunks from the middle and on. This is important because the eliminator removes variables, so starting from the beginning will add errors
  curr_diff = '\n'.join(map(lambda parts: '\n'.join(parts), chunks[mid:])) + '\n'
  difff = open('diff.diff', 'w')
  difff.write(curr_diff)
  difff.close()
  shutil.copy('left', 'middle')
  Popen(['patch', 'middle', 'diff.diff'], stdin=PIPE, stdout=PIPE, stderr=PIPE).communicate()
  result = run_js('middle', stderr=PIPE)
  if result == left_result:
    print 'found where it starts to work: %d' % mid
    found = mid
    break

critical = '\n'.join(chunks[found-1]) + '\n'

c = open('critical.diff', 'w')
c.write(critical)
c.close()
print 'sanity check'
shutil.copy('middle', 'middle2')
Popen(['patch', 'middle2', 'critical.diff'], stdin=PIPE, stdout=PIPE, stderr=PIPE).communicate()
assert run_js('middle', stderr=PIPE) == left_result, 'middle was expected %s' % left_result
assert run_js('middle2', stderr=PIPE) != left_result, 'middle2 was expected NOT %s' % left_result

print 'middle is like left, middle2 is like right, critical.diff is the difference that matters,'
print critical

