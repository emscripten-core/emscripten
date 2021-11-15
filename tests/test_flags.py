# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import shutil
import time
from subprocess import PIPE, STDOUT

from common import RunnerCore
from common import create_file, read_file
from tools.config import EM_CONFIG
from tools.shared import EMCC
from tools.shared import try_delete

# sharad: I dont have V8_ENGINE, using node 16 that also has wasm eh support.
NODE16_PATH = os.path.abspath(os.path.expanduser('~/.nvm/versions/node/v16.13.0/bin/node'))


def restore():
  shutil.copyfile(EM_CONFIG + '_backup', EM_CONFIG)


# restore the config file and set it up for our uses
def restore_and_set_up(self):
  restore()
  with open(EM_CONFIG, 'a') as f:
    # unfreeze the cache, so we can test that
    f.write('FROZEN_CACHE = False\n')


# wipe the config and sanity files, creating a blank slate
def wipe():
  try_delete(EM_CONFIG)


class flags(RunnerCore):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()

    assert os.path.exists(NODE16_PATH), 'To run these tests, we need %s to exist' % NODE16_PATH
    assert os.path.exists(EM_CONFIG), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG
    shutil.copyfile(EM_CONFIG, EM_CONFIG + '_backup')

    print()
    print('Running flags test.')
    print('WARNING: This will modify %s, and in theory can break it although it should be restored properly. A backup will be saved in %s_backup' % (EM_CONFIG, EM_CONFIG))
    print()
    print('>>> the original settings file is:')
    print(open(EM_CONFIG).read().strip())
    print('<<<')
    print()

    assert 'EMCC_DEBUG' not in os.environ, 'do not run sanity checks in debug mode!'

  @classmethod
  def tearDownClass(cls):
    super().tearDownClass()
    restore()

  def setUp(self):
    super().setUp()
    wipe()
    self.start_time = time.time()

  def tearDown(self):
    super().tearDown()
    print('time:', time.time() - self.start_time)

  def do(self, command, env=None):
    print('Running: ' + ' '.join(command))
    if type(command) is not list:
      command = [command]

    return self.run_process(command, stdout=PIPE, stderr=STDOUT, check=False, env=env).stdout

  def check_working(self, command, expected=None):
    if type(command) is not list:
      command = [command]
    if expected is None:
      if command[0] == EMCC or (len(command) >= 2 and command[1] == EMCC):
        expected = 'no input files'
      else:
        expected = "could not find the following tests: blahblah"

    output = self.do(command)
    self.assertContained(expected, output)
    return output

  # @require_v8
  def test_dylink_wasm_eh(self):
    restore_and_set_up(self)
    create_file('main.cpp', r'''
#include <stdio.h>
#include <dlfcn.h>
int main() {
  puts("hello from main");
  void *lib_handle = dlopen("./side.wasm", RTLD_NOW);
  if (!lib_handle) {
    puts("cannot load side module");
    puts(dlerror());
    return 1;
  }
  try {
    typedef int (*func)();
    func fn = (func)dlsym(lib_handle, "sidey");
    if (!fn)
      puts("cannot find side function");
    else
      throw fn();
  }
  catch(int i) {
    printf("i = %d\n", i);
  }
  return 0;
}
    ''')
    create_file('side.cpp', r'''
#include <stdio.h>
#include <setjmp.h>

extern "C" {
int sidey() {
  jmp_buf env;

  puts("hello from side 1");
  int i = setjmp(env);
  puts("hello from side 2");
  if (i!=0) {
    puts("hello from side 3");
    return i;
  }
  puts("hello from side 4");
  longjmp(env, 2);
  puts("hello from side 5");
  return -1;
}
}
    ''')
    create_file('expected.txt', r'''hello from main
hello from side 1
hello from side 2
hello from side 4
hello from side 2
hello from side 3
i = 2''')
    self.set_setting('SUPPORT_LONGJMP', 'wasm')
    self.emcc_args += ['-fwasm-exceptions']

    build_side_cmd = [EMCC, 'side.cpp', '-o', 'side.wasm', '-sSIDE_MODULE=2' ] + self.get_emcc_args()
    print(build_side_cmd)
    self.run_process(build_side_cmd, True)
    build_main_cmd = [EMCC, 'main.cpp', '-sMAIN_MODULE=2', 'side.wasm', '-s', 'EXIT_RUNTIME=0'] + self.get_emcc_args()
    print(build_main_cmd)
    self.run_process(build_main_cmd, True)

    engine_with_args = [NODE16_PATH, '--experimental-wasm-eh']
    out = self.run_js('a.out.js', engine=engine_with_args)
    create_file('actual.txt', out)
    expected_output = read_file('expected.txt')
    self.assertTextDataContained(expected_output, out)
