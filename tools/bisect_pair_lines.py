'''
Given two similar files, for example one with an additional optimization pass,
and with different results, will bisect between them to find the smallest
diff that makes the outputs different.
Unlike bisect_pairs, this uses lines instead of diffs. We replace line by line. This assumes
the programs differ on each line but lines have not been added or removed
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

def run_code(name):
  ret = run_js(name, stderr=PIPE, full_output=True)
  # fix stack traces
  ret = filter(lambda line: not line.startswith('    at ') and not name in line, ret.split('\n'))
  return '\n'.join(ret)

print 'running files'
left_result = run_code('left')
right_result = run_code('right') # right as in left-right, not as in correct
assert left_result != right_result

low = 0
high = file1.count('\n')

print 'beginning bisection, %d lines' % high

left_lines = file1.split('\n')
right_lines = file2.split('\n')

while True:
  mid = int((low + high)/2)
  print low, high, '  current: %d' % mid,
  open('middle', 'w').write('\n'.join(left_lines[:mid] + right_lines[mid:]))
  shutil.copyfile('middle', 'middle' + str(mid))
  result = run_code('middle')
  print result == left_result, result == right_result#, 'XXX', left_result, 'YYY', result, 'ZZZ', right_result
  if mid == low or mid == high: break
  if result == right_result:
    low = mid
  elif result == left_result:
    high = mid
  else:
    raise Exception('new result!?!?')

print 'middle%d is like left, middle%d is like right' % (mid+1, mid)

