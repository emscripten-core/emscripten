# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import platform
import shutil
import time
import re
import tempfile
from pathlib import Path
from subprocess import PIPE, STDOUT

from common import RunnerCore, path_from_root, env_modify, test_file
from common import create_file, ensure_dir, make_executable, with_env_modify
from common import parameterized, EMBUILDER
from tools.config import EM_CONFIG
from tools.shared import EMCC
from tools.shared import config
from tools.shared import EXPECTED_LLVM_VERSION, Cache
from tools.utils import delete_file, delete_dir
from tools import shared, utils
from tools import response_file
from tools import ports

SANITY_FILE = shared.Cache.get_path('sanity.txt')
commands = [[EMCC], [path_from_root('test/runner'), 'blahblah']]


def restore():
  shutil.copyfile(EM_CONFIG + '_backup', EM_CONFIG)


# restore the config file and set it up for our uses
def restore_and_set_up():
  restore()
  with open(EM_CONFIG, 'a') as f:
    # make LLVM_ROOT sensitive to the LLVM env var, as we test that
    f.write('LLVM_ROOT = "%s"\n' % config.LLVM_ROOT)
    # unfreeze the cache, so we can test that
    f.write('FROZEN_CACHE = False\n')


# wipe the config and sanity files, creating a blank slate
def wipe():
  delete_file(EM_CONFIG)
  delete_file(SANITY_FILE)


def add_to_config(content):
  with open(EM_CONFIG, 'a') as f:
    f.write('\n' + content + '\n')


def get_basic_config():
  return '''\
LLVM_ROOT = "%s"
BINARYEN_ROOT = "%s"
NODE_JS = %s
''' % (config.LLVM_ROOT, config.BINARYEN_ROOT, config.NODE_JS)


def make_fake_tool(filename, version, report_name=None):
  if not report_name:
    report_name = os.path.basename(filename)
  print('make_fake_tool: %s' % filename)
  ensure_dir(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "%s version %s"\n' % (report_name, version))
    f.write('echo "..."\n')
    f.write('exit 0\n')
  make_executable(filename)


def make_fake_clang(filename, version):
  """Create a fake clang that only handles --version
  --version writes to stdout (unlike -v which writes to stderr)
  """
  make_fake_tool(filename, version)
  make_fake_tool(filename + '++', version)


def make_fake_llc(filename, targets):
  """Create a fake llc that only handles --version and writes target
  list to stdout.
  """
  print('make_fake_llc: %s' % filename)
  ensure_dir(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake output\nRegistered Targets:\n%s"' % targets)
  make_executable(filename)


SANITY_MESSAGE = 'Emscripten: Running sanity checks'

# arguments to build a minimal hello world program, without even libc
# (-O1 avoids -O0's default assertions which bring in checking code;
#  FILESYSTEM=0 avoids bringing libc for that)
# (ERROR_ON_UNDEFINED_SYMBOLS=0 is needed because __errno_location is
#  not included on the native side but needed by a lot of JS libraries.)
MINIMAL_HELLO_WORLD = [test_file('hello_world_em_asm.c'), '-O1', '-sFILESYSTEM=0', '-sERROR_ON_UNDEFINED_SYMBOLS=0']


class sanity(RunnerCore):
  @classmethod
  def setUpClass(cls):
    super().setUpClass()
    # Unlike the other test suites we explicitly don't want to be skipping
    # the sanity checks here
    del os.environ['EMCC_SKIP_SANITY_CHECK']

    assert os.path.exists(EM_CONFIG), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG
    shutil.copyfile(EM_CONFIG, EM_CONFIG + '_backup')

    print()
    print('Running sanity checks.')
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

  # this should be the very first thing that runs. if this fails, everything else is irrelevant!
  def test_aaa_normal(self):
    for command in commands:
      # Your existing EM_CONFIG should work!
      restore_and_set_up()
      self.check_working(command)

  @with_env_modify({'EM_CONFIG': None})
  def test_firstrun(self):
    default_config = config.embedded_config
    output = self.do([EMCC, '-v'])
    self.assertContained('emcc: error: config file not found: %s.  Please create one by hand or run `emcc --generate-config`' % default_config, output)

    try:
      temp_bin = tempfile.mkdtemp()

      def make_new_executable(name):
        open(os.path.join(temp_bin, name), 'w').close()
        make_executable(os.path.join(temp_bin, name))

      make_new_executable('llvm-dis')
      make_new_executable('node')

      with env_modify({'PATH': temp_bin + os.pathsep + os.environ['PATH']}):
        output = self.do([EMCC, '--generate-config'])
    finally:
      shutil.rmtree(temp_bin)
      config_data = open(default_config).read()

    self.assertContained('An Emscripten settings file has been generated at:', output)
    self.assertContained(default_config, output)
    self.assertContained('It contains our best guesses for the important paths, which are:', output)
    self.assertContained('LLVM_ROOT', output)
    self.assertContained('NODE_JS', output)
    if platform.system() != 'Windows':
      # os.chmod can't make files executable on Windows
      self.assertIdentical(temp_bin, re.search("^ *LLVM_ROOT *= (.*)$", output, re.M).group(1))
      possible_nodes = [os.path.join(temp_bin, 'node')]
      if os.path.exists('/usr/bin/nodejs'):
        possible_nodes.append('/usr/bin/nodejs')
      self.assertIdentical(possible_nodes, re.search("^ *NODE_JS *= (.*)$", output, re.M).group(1))

    template_data = Path(path_from_root('tools/config_template.py')).read_text()
    self.assertNotContained('{{{', config_data)
    self.assertNotContained('}}}', config_data)
    self.assertContained('{{{', template_data)
    self.assertContained('}}}', template_data)
    for content in ['EMSCRIPTEN_ROOT', 'LLVM_ROOT', 'NODE_JS', 'JS_ENGINES']:
      self.assertContained(content, config_data)

    # The guessed config should be ok
    # XXX This depends on your local system! it is possible `which` guesses wrong
    # delete_file('a.out.js')
    # output = self.run_process([EMCC, test_file('hello_world.c')], stdout=PIPE, stderr=PIPE).output
    # self.assertContained('hello, world!', self.run_js('a.out.js'), output)

    for command in commands:
      # Second run, with bad EM_CONFIG
      for settings in ['blah', 'LLVM_ROOT="blarg"; JS_ENGINES=[]; NODE_JS=[]; SPIDERMONKEY_ENGINE=[]']:
        try:
          with open(default_config, 'w') as f:
            f.write(settings)
          output = self.do(command)

          if 'blah' in settings:
            self.assertContained('Error in evaluating config file (%s)' % default_config, output)
          elif 'runner' not in ' '.join(command):
            self.assertContained('error: NODE_JS is set to empty value', output) # sanity check should fail
        finally:
          delete_file(default_config)

  def test_llvm(self):
    LLVM_WARNING = 'LLVM version for clang executable'

    restore_and_set_up()

    # Clang should report the version number we expect, and emcc should not warn
    assert shared.check_llvm_version()
    output = self.check_working(EMCC)
    self.assertNotContained(LLVM_WARNING, output)

    # Fake a different llvm version
    restore_and_set_up()
    with open(EM_CONFIG, 'a') as f:
      f.write('LLVM_ROOT = "' + self.in_dir('fake') + '"')

    real_version_x, real_version_y = (int(x) for x in EXPECTED_LLVM_VERSION.split('.'))
    make_fake_llc(self.in_dir('fake', 'llc'), 'wasm32 - WebAssembly 32-bit')
    make_fake_tool(self.in_dir('fake', 'wasm-ld'), EXPECTED_LLVM_VERSION)

    for inc_x in range(-2, 3):
      for inc_y in range(-2, 3):
        delete_file(SANITY_FILE)
        expected_x = real_version_x + inc_x
        expected_y = real_version_y + inc_y
        if expected_x < 0 or expected_y < 0:
          continue # must be a valid llvm version
        print("mod LLVM version: %d %d -> %d %d" % (real_version_x, real_version_y, expected_x, expected_y))
        make_fake_clang(self.in_dir('fake', 'clang'), '%s.%s' % (expected_x, expected_y))
        make_fake_tool(self.in_dir('fake', 'llvm-ar'), '%s.%s' % (expected_x, expected_y))
        make_fake_tool(self.in_dir('fake', 'llvm-nm'), '%s.%s' % (expected_x, expected_y))
        did_modify = inc_x != 0 or inc_y != 0
        if did_modify:
          output = self.check_working(EMCC, LLVM_WARNING)
        else:
          output = self.check_working(EMCC)
          self.assertNotContained(LLVM_WARNING, output)

  def test_emscripten_root(self):
    # The correct path
    restore_and_set_up()
    add_to_config("EMSCRIPTEN_ROOT = '%s'" % path_from_root())
    self.check_working(EMCC)

    # The correct path with extra stuff
    restore_and_set_up()
    add_to_config("EMSCRIPTEN_ROOT = '%s'" % (path_from_root() + os.path.sep))
    self.check_working(EMCC)

  def test_node(self):
    NODE_WARNING = 'node version appears too old'
    NODE_WARNING_2 = 'cannot check node version'

    restore_and_set_up()
    cmd = [EMCC, test_file('hello_world.c')]

    # Clang should report the version number we expect, and emcc should not warn
    output = self.do(cmd)
    self.assertNotContained(NODE_WARNING, output)

    # Fake a different node version
    restore_and_set_up()
    with open(EM_CONFIG, 'a') as f:
      f.write('NODE_JS = "' + self.in_dir('fake', 'nodejs') + '"')

    ensure_dir('fake')

    for version, succeed in [('v0.8.0', False),
                             ('v4.1.0', False),
                             ('v4.1.1', True),
                             ('v4.2.3-pre', True),
                             ('cheez', False)]:
      print(version, succeed)
      delete_file(SANITY_FILE)
      f = open(self.in_dir('fake', 'nodejs'), 'w')
      f.write('#!/bin/sh\n')
      f.write('''if [ $1 = "--version" ]; then
echo "%s"
else
%s $@
fi
''' % (version, ' '.join(config.NODE_JS)))
      f.close()
      make_executable(self.in_dir('fake', 'nodejs'))
      if not succeed:
        if version[0] == 'v':
          self.check_working(cmd, NODE_WARNING)
        else:
          self.check_working(cmd, NODE_WARNING_2)
      else:
        output = self.do(cmd)
        self.assertNotContained(NODE_WARNING, output)

  def test_emcc(self):
    SANITY_FAIL_MESSAGE = 'sanity check failed to run'

    # emcc should check sanity if no ${EM_CONFIG}_sanity
    restore_and_set_up()
    time.sleep(1)
    assert not os.path.exists(SANITY_FILE) # restore is just the settings, not the sanity
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    # EMCC should have checked sanity successfully
    old_sanity = open(SANITY_FILE).read()
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc run again should not sanity check, because the sanity file is newer
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # incorrect sanity contents mean we *must* check
    open(SANITY_FILE, 'w').write('wakawaka')
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)

    # correct sanity contents mean we need not check
    open(SANITY_FILE, 'w').write(old_sanity)
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)

    # but with EMCC_DEBUG=1 we should check
    with env_modify({'EMCC_DEBUG': '1'}):
      output = self.check_working(EMCC)
    delete_dir(shared.CANONICAL_TEMP_DIR)

    self.assertContained(SANITY_MESSAGE, output)
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)

    # Make sure the test runner didn't do anything to the setup
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc should also check sanity if the file is outdated
    open(EM_CONFIG, 'a').write('# extra stuff\n')
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

  def test_em_config_env_var(self):
    # emcc should be configurable directly from EM_CONFIG without any config file
    restore_and_set_up()
    create_file('main.cpp', '''
      #include <stdio.h>
      int main() {
        printf("hello from emcc with no config file\\n");
        return 0;
      }
    ''')

    wipe()
    with env_modify({'EM_CONFIG': get_basic_config()}):
      out = self.expect_fail([EMCC, 'main.cpp', '-Wno-deprecated', '-o', 'a.out.js'])

    self.assertContained('error: Inline EM_CONFIG data no longer supported.  Please use a config file.', out)

  def clear_cache(self):
    self.run_process([EMCC, '--clear-cache'])
    self.assertCacheEmpty()

  def assertCacheEmpty(self):
    if os.path.exists(Cache.dirname):
      # The cache is considered empty if it contains no files at all or just the cache.lock
      self.assertIn(os.listdir(Cache.dirname), ([], ['cache.lock']))

  def ensure_cache(self):
    self.do([EMCC, '-O2', test_file('hello_world.c')])

  def test_emcc_caching(self):
    BUILDING_MESSAGE = 'generating system library: %s'

    restore_and_set_up()
    self.clear_cache()

    # Building a file that *does* need something *should* trigger cache
    # generation, but only the first time
    libname = Cache.get_lib_name('libc++.a')
    for i in range(3):
      print(i)
      self.clear()
      output = self.do([EMCC, '-O' + str(i), test_file('hello_libcxx.cpp'), '-sDISABLE_EXCEPTION_CATCHING=0'])
      print('\n\n\n', output)
      self.assertContainedIf(BUILDING_MESSAGE % libname, output, i == 0)
      self.assertContained('hello, world!', self.run_js('a.out.js'))
      self.assertExists(Cache.dirname)
      self.assertExists(os.path.join(Cache.dirname, libname))

  def test_cache_clearing_manual(self):
    # Manual cache clearing
    restore_and_set_up()
    self.ensure_cache()
    self.assertExists(Cache.dirname)
    output = self.do([EMCC, '--clear-cache'])
    self.assertIn('clearing cache', output)
    self.assertIn(SANITY_MESSAGE, output)
    self.assertCacheEmpty()

  def test_cache_clearing_auto(self):
    # Changing LLVM_ROOT, even without altering .emscripten, clears the cache
    restore_and_set_up()
    self.ensure_cache()
    make_fake_clang(self.in_dir('fake', 'bin', 'clang'), EXPECTED_LLVM_VERSION)
    make_fake_llc(self.in_dir('fake', 'bin', 'llc'), 'got wasm32 backend! WebAssembly 32-bit')
    with env_modify({'EM_LLVM_ROOT': self.in_dir('fake', 'bin')}):
      self.assertExists(Cache.dirname)
      output = self.do([EMCC])
      self.assertIn('clearing cache', output)
      self.assertCacheEmpty()

  # FROZEN_CACHE prevents cache clears, and prevents building
  def test_FROZEN_CACHE(self):
    restore_and_set_up()
    self.clear_cache()
    self.ensure_cache()
    self.assertExists(Cache.dirname)
    # changing config file should not clear cache
    add_to_config('FROZEN_CACHE = True')
    self.do([EMCC])
    self.assertExists(Cache.dirname)
    # building libraries is disallowed
    output = self.do([EMBUILDER, 'build', 'libemmalloc'])
    self.assertContained('FROZEN_CACHE is set, but cache file is missing', output)

  # Test that if multiple processes attempt to access or build stuff to the
  # cache on demand, that exactly one of the processes will, and the other
  # processes will block to wait until that process finishes.
  def test_emcc_multiprocess_cache_access(self):
    restore_and_set_up()

    create_file('test.c', r'''
      #include <stdio.h>
      int main() {
        printf("hello, world!\n");
        return 0;
      }
      ''')
    cache_dir_name = self.in_dir('test_cache')
    libname = Cache.get_lib_name('libc.a')
    with env_modify({'EM_CACHE': cache_dir_name}):
      tasks = []
      num_times_libc_was_built = 0
      for i in range(3):
        p = self.run_process([EMCC, 'test.c', '-O2', '-o', '%d.js' % i], stderr=STDOUT, stdout=PIPE)
        tasks += [p]
      for p in tasks:
        print('stdout:\n', p.stdout)
        if 'generating system library: ' + libname in p.stdout:
          num_times_libc_was_built += 1

    # The cache directory must exist after the build
    self.assertExists(cache_dir_name)
    # The cache directory must contain a built libc
    self.assertExists(os.path.join(cache_dir_name, libname))
    # Exactly one child process should have triggered libc build!
    self.assertEqual(num_times_libc_was_built, 1)

  @parameterized({
    '': [False, False],
    'response_files': [True, False],
    'relative': [False, True]
  })
  def test_emcc_cache_flag(self, use_response_files, relative):
    restore_and_set_up()

    if relative:
      cache_dir_name = 'emscripten_cache'
    else:
      cache_dir_name = self.in_dir('emscripten_cache')
    self.assertFalse(os.path.exists(cache_dir_name))
    create_file('test.c', r'''
      #include <stdio.h>
      int main() {
        printf("hello, world!\n");
        return 0;
      }
      ''')
    args = ['--cache', cache_dir_name]
    if use_response_files:
      rsp = response_file.create_response_file(args, shared.TEMP_DIR)
      args = ['@' + rsp]

    self.run_process([EMCC, 'test.c'] + args, stderr=PIPE)
    if use_response_files:
      os.remove(rsp)

    # The cache directory must exist after the build
    self.assertTrue(os.path.exists(cache_dir_name))
    # The cache directory must contain a sysroot
    self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'sysroot')))

  def test_emconfig(self):
    restore_and_set_up()

    fd, custom_config_filename = tempfile.mkstemp(prefix='.emscripten_config_')

    orig_config = open(EM_CONFIG, 'r').read()

    # Move the ~/.emscripten to a custom location.
    with os.fdopen(fd, "w") as f:
      f.write(get_basic_config())

    # Make a syntax error in the original config file so that attempting to access it would fail.
    open(EM_CONFIG, 'w').write('asdfasdfasdfasdf\n\'\'\'' + orig_config)

    temp_dir = tempfile.mkdtemp(prefix='emscripten_temp_')

    with utils.chdir(temp_dir):
      self.run_process([EMCC, '--em-config', custom_config_filename] + MINIMAL_HELLO_WORLD + ['-O2'])
      result = self.run_js('a.out.js')

    self.assertContained('hello, world!', result)

    # Clean up created temp files.
    os.remove(custom_config_filename)
    shutil.rmtree(temp_dir)

  def test_emcc_ports(self):
    restore_and_set_up()

    # listing ports
    out = self.do([EMCC, '--show-ports'])
    self.assertContained('Available ports:', out)
    self.assertContained('SDL2', out)
    self.assertContained('SDL2_image', out)
    self.assertContained('SDL2_net', out)

    # using ports
    RETRIEVING_MESSAGE = 'retrieving port'
    BUILDING_MESSAGE = 'generating port'

    PORTS_DIR = ports.Ports.get_dir()

    for i in [0, 1]:
      self.do([EMCC, '--clear-cache'])
      print(i)
      if i == 0:
        delete_dir(PORTS_DIR)
      else:
        self.do([EMCC, '--clear-ports'])
      self.assertNotExists(PORTS_DIR)

      # Building a file that doesn't need ports should not trigger anything
      output = self.do([EMCC, test_file('hello_world_sdl.cpp')])
      self.assertNotContained(RETRIEVING_MESSAGE, output)
      self.assertNotContained(BUILDING_MESSAGE, output)
      self.assertNotExists(PORTS_DIR)

      def first_use():
        output = self.do([EMCC, test_file('hello_world_sdl.cpp'), '-sUSE_SDL=2'])
        self.assertContained(RETRIEVING_MESSAGE, output)
        self.assertContained(BUILDING_MESSAGE, output)
        self.assertExists(PORTS_DIR)

      def second_use():
        # Using it again avoids retrieve and build
        output = self.do([EMCC, test_file('hello_world_sdl.cpp'), '-sUSE_SDL=2'])
        self.assertNotContained(RETRIEVING_MESSAGE, output)
        self.assertNotContained(BUILDING_MESSAGE, output)

      # Building a file that need a port does trigger stuff
      first_use()
      second_use()

      # if the url doesn't match, we retrieve and rebuild
      with open(os.path.join(PORTS_DIR, 'sdl2', '.emscripten_url'), 'w') as f:
        f.write('foo')

      first_use()
      second_use()

  def test_js_engine_path(self):
    # Test that running JS commands works for node, d8, and jsc and is not path dependent
    restore_and_set_up()

    sample_script = test_file('print_args.js')

    # Fake some JS engines
    # Note that the path contains 'd8'.
    test_path = self.in_dir('fake', 'abcd8765')
    ensure_dir(test_path)

    jsengines = [('d8',     config.V8_ENGINE),
                 ('d8_g',   config.V8_ENGINE),
                 ('js',     config.SPIDERMONKEY_ENGINE),
                 ('node',   config.NODE_JS),
                 ('nodejs', config.NODE_JS)]
    for filename, engine in jsengines:
      delete_file(SANITY_FILE)
      if type(engine) is list:
        engine = engine[0]
      if not engine:
        print('WARNING: Not testing engine %s, not configured.' % (filename))
        continue

      print(filename, engine)

      test_engine_path = os.path.join(test_path, filename)
      with open(test_engine_path, 'w') as f:
        f.write('#!/bin/sh\n')
        f.write('exec %s $@\n' % (engine))
      make_executable(test_engine_path)

      out = self.run_js(sample_script, engine=test_engine_path, args=['--foo'])

      self.assertEqual('0: --foo', out.strip())

  def test_wacky_env(self):
    restore_and_set_up()

    def build():
      return self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')

    def test():
      self.assertContained('hello, world!', self.run_js('a.out.js'))

    print('normal build')
    with env_modify({'EMCC_FORCE_STDLIBS': None}):
      self.clear_cache()
      build()
      test()

    print('wacky env vars, these should not mess our bootstrapping')
    with env_modify({'EMCC_FORCE_STDLIBS': '1'}):
      self.clear_cache()
      build()
      test()

  def test_vanilla(self):
    restore_and_set_up()
    self.clear_cache()

    def make_fake(report):
      with open(EM_CONFIG, 'a') as f:
        f.write('LLVM_ROOT = "' + self.in_dir('fake', 'bin') + '"\n')
        # BINARYEN_ROOT needs to exist in the config, even though this test
        # doesn't actually use it.
        f.write('BINARYEN_ROOT = "%s"\n' % self.in_dir('fake', 'bin'))

      make_fake_clang(self.in_dir('fake', 'bin', 'clang'), EXPECTED_LLVM_VERSION)
      make_fake_llc(self.in_dir('fake', 'bin', 'llc'), report)
      make_fake_tool(self.in_dir('fake', 'bin', 'wasm-ld'), EXPECTED_LLVM_VERSION)

    # fake llc output

    def test_with_fake(report, expected):
      make_fake(report)
      with env_modify({'EMCC_DEBUG': '1'}):
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], expected)

    test_with_fake('got js backend! JavaScript (asm.js, emscripten) backend', 'LLVM has not been built with the WebAssembly backend')
    delete_dir(shared.CANONICAL_TEMP_DIR)

  def test_required_config_settings(self):
    # with no binaryen root, an error is shown
    restore_and_set_up()

    open(EM_CONFIG, 'a').write('\nBINARYEN_ROOT = ""\n')
    self.check_working([EMCC, test_file('hello_world.c')], 'BINARYEN_ROOT is set to empty value in %s' % EM_CONFIG)

    open(EM_CONFIG, 'a').write('\ndel BINARYEN_ROOT\n')
    self.check_working([EMCC, test_file('hello_world.c')], 'BINARYEN_ROOT is not defined in %s' % EM_CONFIG)

  def test_embuilder_force(self):
    restore_and_set_up()
    self.do([EMBUILDER, 'build', 'libemmalloc'])
    # Second time it should not generate anything
    self.assertNotContained('generating system library', self.do([EMBUILDER, 'build', 'libemmalloc']))
    # Unless --force is specified
    self.assertContained('generating system library', self.do([EMBUILDER, 'build', 'libemmalloc', '--force']))

  def test_embuilder_force_port(self):
    restore_and_set_up()
    self.do([EMBUILDER, 'build', 'zlib'])
    # Second time it should not generate anything
    self.assertNotContained('generating port', self.do([EMBUILDER, 'build', 'zlib']))
    # Unless --force is specified
    self.assertContained('generating port', self.do([EMBUILDER, 'build', 'zlib', '--force']))

  def test_embuilder_wasm_backend(self):
    restore_and_set_up()
    # the --lto flag makes us build wasm-bc
    self.clear_cache()
    self.run_process([EMBUILDER, 'build', 'libemmalloc'])
    self.assertExists(os.path.join(config.CACHE, 'sysroot', 'lib', 'wasm32-emscripten'))
    self.clear_cache()
    self.run_process([EMBUILDER, 'build', 'libemmalloc', '--lto'])
    self.assertExists(os.path.join(config.CACHE, 'sysroot', 'lib', 'wasm32-emscripten', 'lto'))

  def test_binaryen_version(self):
    restore_and_set_up()
    with open(EM_CONFIG, 'a') as f:
      f.write('\nBINARYEN_ROOT = "' + self.in_dir('fake') + '"')

    make_fake_tool(self.in_dir('fake', 'bin', 'wasm-opt'), 'foo')
    self.check_working([EMCC, test_file('hello_world.c')], 'error parsing binaryen version (wasm-opt version foo). Please check your binaryen installation')

    make_fake_tool(self.in_dir('fake', 'bin', 'wasm-opt'), '70')
    self.check_working([EMCC, test_file('hello_world.c')], 'unexpected binaryen version: 70 (expected ')
