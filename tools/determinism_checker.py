# Copyright 2018 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

'''
Runs a build command many times to search for any nondeterminism.
'''

import os
import random
import shutil
import subprocess
import time

def run():
  subprocess.check_call(['emcc', 'src.cpp', '-O2', '-s', 'WASM=1'])
  ret = {}
  for relevant_file in os.listdir('.'):
    ret[relevant_file] = open(relevant_file).read()
  return ret

def write(data, subdir):
  if not os.path.exists(subdir):
    os.mkdir(subdir)
  for relevant_file in data.keys():
    open(os.path.join(subdir, relevant_file), 'w').write(data[relevant_file])

old = os.getcwd()
try:
  os.chdir('/tmp/emscripten_temp')
  assert len(os.listdir('.')) == 0, 'temp dir should start out empty, after that, everything there looks important to us'
  open('src.cpp', 'w').write('''
    #include <iostream>

    int main()
    {
      std::cout << "hello world" << std::endl << 77 << "." << std::endl;
      return 0;
    }
  ''')

  os.environ['EMCC_DEBUG'] = '1'
  #os.environ['BINARYEN_PASS_DEBUG'] = '3'

  first = run()

  i = 0
  while 1:
    print(i)
    i += 1
    time.sleep(random.random()/(10*5)) # add some timing nondeterminism here, not that we need it, but whatever
    curr = run()
    if first != curr:
      print('NONDETERMINISM!!!1')
      write(first, 'first')
      write(curr, 'second')
      break

finally:
  os.chdir(old)

