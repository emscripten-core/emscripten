'''
Usage:   left.js right.js n out.js

Given two files with the same function names in them, merges in n functions from the latter from the start,
order taken as from the first file. This is useful for bisection: 0 means the left file is the output,
and a big enough n means the right file is the output. if left is ok and right shows a bug, then the n
where a change occurs shows which function is the culprit.
'''

import os, sys, shutil

import asm_module, shared, shutil

left = sys.argv[1]
left_asm = asm_module.AsmModule(left)

right = sys.argv[2]
right_asm = asm_module.AsmModule(right)

n = int(sys.argv[3])

out = sys.argv[4]

funcs = list(left_asm.funcs)
print 'total funcs:', len(funcs)
left_map = left_asm.get_funcs_map()
right_map = right_asm.get_funcs_map()

n = min(n, len(funcs))

for i in range(n):
  f = funcs[i]
  if f in right_map: # TODO: fix
    left_map[f] = right_map[f]
print 'merging in the first %d from the second file' % n

left_asm.apply_funcs_map(left_map)

left_asm.write(out)

