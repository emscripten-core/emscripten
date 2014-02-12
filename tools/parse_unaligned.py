import os, sys
from subprocess import Popen, PIPE, STDOUT

shown = set()

for line in open(sys.argv[1]).readlines():
  try:
    x = line.split(' in ')[1].split(':')[0]
    #output = str([x, Popen(['c++filt', x], stdout=PIPE).communicate()])
    output = Popen(['c++filt', x], stdout=PIPE).communicate()[0]
    if output not in shown:
      shown.add(output)
      print output,
  except:
    pass


