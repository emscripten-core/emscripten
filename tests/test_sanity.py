# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import platform
import shutil
import stat
import time
import re
import tempfile
import zipfile
from subprocess import PIPE, STDOUT

from runner import RunnerCore, path_from_root, env_modify, chdir
from runner import create_test_file, no_wasm_backend, ensure_dir
from tools.shared import NODE_JS, PYTHON, EMCC, SPIDERMONKEY_ENGINE, V8_ENGINE
from tools.shared import CONFIG_FILE, EM_CONFIG, LLVM_ROOT, CANONICAL_TEMP_DIR
from tools.shared import try_delete
from tools.shared import EXPECTED_LLVM_VERSION, Cache
from tools import shared, system_libs

SANITY_FILE = shared.Cache.get_path('sanity.txt', root=True)
commands = [[EMCC], [PYTHON, path_from_root('tests', 'runner.py'), 'blahblah']]


def restore():
  shutil.copyfile(CONFIG_FILE + '_backup', CONFIG_FILE)


# restore the config file and set it up for our uses
def restore_and_set_up():
  restore()
  with open(CONFIG_FILE, 'a') as f:
    # make LLVM_ROOT sensitive to the LLVM env var, as we test that
    f.write('LLVM_ROOT = "%s"\n' % LLVM_ROOT)
    # unfreeze the cache, so we can test that
    f.write('FROZEN_CACHE = False\n')


# wipe the config and sanity files, creating a blank slate
def wipe():
  try_delete(CONFIG_FILE)
  try_delete(SANITY_FILE)


def add_to_config(content):
  with open(CONFIG_FILE, 'a') as f:
    f.write('\n' + content + '\n')


def get_basic_config():
  return '''\
LLVM_ROOT = "%s"
BINARYEN_ROOT = "%s"
NODE_JS = %s
''' % (LLVM_ROOT, shared.BINARYEN_ROOT, NODE_JS)


def make_fake_wasm_opt(filename, version):
  print('make_fake_wasm_opt: %s' % filename)
  ensure_dir(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "wasm-opt version %s"\n' % version)
    f.write('echo "..."\n')
  shutil.copyfile(filename, filename + '++')
  os.chmod(filename, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
  os.chmod(filename + '++', stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_fake_clang(filename, version):
  """Create a fake clang that only handles --version
  --version writes to stdout (unlike -v which writes to stderr)
  """
  print('make_fake_clang: %s' % filename)
  ensure_dir(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "clang version %s"\n' % version)
    f.write('echo "..."\n')
  shutil.copyfile(filename, filename + '++')
  os.chmod(filename, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
  os.chmod(filename + '++', stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_fake_llc(filename, targets):
  """Create a fake llc that only handles --version and writes target
  list to stdout.
  """
  print('make_fake_llc: %s' % filename)
  ensure_dir(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake output\nRegistered Targets:\n%s"' % targets)
  os.chmod(filename, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_fake_lld(filename):
  print('make_fake_lld: %s' % filename)
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('exit 0\n')
  os.chmod(filename, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


SANITY_MESSAGE = 'Emscripten: Running sanity checks'

EMBUILDER = path_from_root('embuilder.py')

# arguments to build a minimal hello world program, without even libc
# (-O1 avoids -O0's default assertions which bring in checking code;
#  FILESYSTEM=0 avoids bringing libc for that)
# (ERROR_ON_UNDEFINED_SYMBOLS=0 is needed because __errno_location is
#  not included on the native side but needed by a lot of JS libraries.)
MINIMAL_HELLO_WORLD = [path_from_root('tests', 'hello_world_em_asm.c'), '-O1', '-s', 'FILESYSTEM=0', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=0']


class sanity(RunnerCore):
  @classmethod
  def setUpClass(cls):
    super(sanity, cls).setUpClass()
    # Unlike the other test suites we explicitly don't want to be skipping
    # the sanity checks here
    del os.environ['EMCC_SKIP_SANITY_CHECK']

    assert os.path.exists(CONFIG_FILE), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG
    shutil.copyfile(CONFIG_FILE, CONFIG_FILE + '_backup')

    print()
    print('Running sanity checks.')
    print('WARNING: This will modify %s, and in theory can break it although it should be restored properly. A backup will be saved in %s_backup' % (CONFIG_FILE, CONFIG_FILE))
    print()
    print('>>> the original settings file is:')
    print(open(CONFIG_FILE).read().strip())
    print('<<<')
    print()

    assert 'EMCC_DEBUG' not in os.environ, 'do not run sanity checks in debug mode!'

  @classmethod
  def tearDownClass(cls):
    super(sanity, cls).tearDownClass()
    restore()

  def setUp(self):
    super(sanity, self).setUp()
    wipe()
    self.start_time = time.time()

  def tearDown(self):
    super(sanity, self).tearDown()
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

  def test_firstrun(self):
    for command in commands:
      wipe()

      def make_executable(name):
        with open(os.path.join(temp_bin, name), 'w') as f:
          os.fchmod(f.fileno(), stat.S_IRWXU)

      env = os.environ.copy()
      if 'EM_CONFIG' in env:
        del env['EM_CONFIG']

      try:
        temp_bin = tempfile.mkdtemp()
        make_executable('llvm-dis')
        make_executable('node')
        env['PATH'] = temp_bin + os.pathsep + os.environ['PATH']
        output = self.do(command, env=env)
      finally:
        shutil.rmtree(temp_bin)

      default_config = shared.embedded_config
      self.assertContained('Welcome to Emscripten!', output)
      self.assertContained('This is the first time any of the Emscripten tools has been run.', output)
      self.assertContained('A settings file has been copied to %s, at absolute path: %s' % (default_config, default_config), output)
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
      self.assertContained('Please edit the file if any of those are incorrect', output)
      self.assertContained('This command will now exit. When you are done editing those paths, re-run it.', output)
      assert output.split()[-1].endswith('===='), 'We should have stopped: ' + output
      config_file = open(default_config).read()
      template_file = open(path_from_root('tools', 'settings_template.py')).read()
      self.assertNotContained('{{{', config_file)
      self.assertNotContained('}}}', config_file)
      self.assertContained('{{{', template_file)
      self.assertContained('}}}', template_file)
      for content in ['EMSCRIPTEN_ROOT', 'LLVM_ROOT', 'NODE_JS', 'JS_ENGINES']:
        self.assertContained(content, config_file)

      # The guessed config should be ok
      # XXX This depends on your local system! it is possible `which` guesses wrong
      # try_delete('a.out.js')
      # output = self.run_process([EMCC, path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE).output
      # self.assertContained('hello, world!', self.run_js('a.out.js'), output)

      # Second run, with bad EM_CONFIG
      for settings in ['blah', 'LLVM_ROOT="blarg"; JS_ENGINES=[]; NODE_JS=[]; SPIDERMONKEY_ENGINE=[]']:
        f = open(default_config, 'w')
        f.write(settings)
        f.close()
        output = self.do(command, env=env)

        if 'LLVM_ROOT' not in settings:
          self.assertContained('Error in evaluating %s' % default_config, output)
        elif 'runner.py' not in ' '.join(command):
          self.assertContained('error:', output) # sanity check should fail
      try_delete(default_config)

  def test_llvm(self):
    LLVM_WARNING = 'LLVM version appears incorrect'

    restore_and_set_up()

    # Clang should report the version number we expect, and emcc should not warn
    assert shared.check_llvm_version()
    output = self.check_working(EMCC)
    self.assertNotContained(LLVM_WARNING, output)

    # Fake a different llvm version
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('LLVM_ROOT = "' + self.in_dir('fake') + '"')

    real_version_x, real_version_y = (int(x) for x in EXPECTED_LLVM_VERSION.split('.'))
    make_fake_llc(self.in_dir('fake', 'llc'), 'wasm32 - WebAssembly 32-bit')
    make_fake_lld(self.in_dir('fake', 'wasm-ld'))

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      for inc_x in range(-2, 3):
        for inc_y in range(-2, 3):
          expected_x = real_version_x + inc_x
          expected_y = real_version_y + inc_y
          if expected_x < 0 or expected_y < 0:
            continue # must be a valid llvm version
          print("mod LLVM version: %d %d -> %d %d" % (real_version_x, real_version_x, expected_x, expected_y))
          make_fake_clang(self.in_dir('fake', 'clang'), '%s.%s' % (expected_x, expected_y))
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

    # Clang should report the version number we expect, and emcc should not warn
    assert shared.check_node_version()
    output = self.check_working(EMCC)
    self.assertNotIn(NODE_WARNING, output)

    # Fake a different node version
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('NODE_JS = "' + self.in_dir('fake', 'nodejs') + '"')

    ensure_dir('fake')

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      for version, succeed in [('v0.8.0', False),
                               ('v4.1.0', False),
                               ('v4.1.1', True),
                               ('v4.2.3-pre', True),
                               ('cheez', False)]:
        print(version, succeed)
        f = open(self.in_dir('fake', 'nodejs'), 'w')
        f.write('#!/bin/sh\n')
        f.write('''if [ $1 = "--version" ]; then
echo "%s"
else
%s $@
fi
''' % (version, NODE_JS))
        f.close()
        os.chmod(self.in_dir('fake', 'nodejs'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
        if not succeed:
          if version[0] == 'v':
            self.check_working(EMCC, NODE_WARNING)
          else:
            self.check_working(EMCC, NODE_WARNING_2)
        else:
          output = self.check_working(EMCC)
          assert NODE_WARNING not in output, output

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
    try_delete(CANONICAL_TEMP_DIR)

    self.assertContained(SANITY_MESSAGE, output)
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)

    # also with -v, with or without inputs
    output = self.check_working([EMCC, '-v'], SANITY_MESSAGE)
    output = self.check_working([EMCC, '-v'] + MINIMAL_HELLO_WORLD + [], SANITY_MESSAGE)

    # Make sure the test runner didn't do anything to the setup
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc should also check sanity if the file is outdated
    open(CONFIG_FILE, 'a').write('# extra stuff\n')
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

  def test_em_config_env_var(self):
    # emcc should be configurable directly from EM_CONFIG without any config file
    restore_and_set_up()
    create_test_file('main.cpp', '''
      #include <stdio.h>
      int main() {
        printf("hello from emcc with no config file\\n");
        return 0;
      }
    ''')

    wipe()
    with env_modify({'EM_CONFIG': get_basic_config()}):
      self.run_process([EMCC, 'main.cpp', '-Wno-deprecated', '-o', 'a.out.js'])

    self.assertContained('hello from emcc with no config file', self.run_js('a.out.js'))

  def erase_cache(self):
    Cache.erase()
    self.assertCacheEmpty()

  def assertCacheEmpty(self):
    if os.path.exists(Cache.dirname):
      self.assertEqual(os.listdir(Cache.dirname), [])

  def ensure_cache(self):
    self.do([EMCC, '-O2', path_from_root('tests', 'hello_world.c')])

  def test_emcc_caching(self):
    BUILDING_MESSAGE = 'generating system library: X'

    restore_and_set_up()
    self.erase_cache()

    # Building a file that *does* need something *should* trigger cache
    # generation, but only the first time
    libname = 'libc++'
    for i in range(3):
      print(i)
      self.clear()
      output = self.do([EMCC, '-O' + str(i), path_from_root('tests', 'hello_libcxx.cpp'), '-s', 'DISABLE_EXCEPTION_CATCHING=0'])
      print('\n\n\n', output)
      self.assertContainedIf(BUILDING_MESSAGE.replace('X', libname), output, i == 0)
      self.assertContained('hello, world!', self.run_js('a.out.js'))
      self.assertExists(Cache.dirname)
      full_libname = libname + '.bc' if libname != 'libc++' else libname + '.a'
      self.assertExists(os.path.join(Cache.dirname, full_libname))

  def test_cache_clearing_manual(self):
    # Manual cache clearing
    restore_and_set_up()
    self.ensure_cache()
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
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
      self.assertTrue(os.path.exists(Cache.dirname))
      output = self.do([EMCC])
      self.assertIn('clearing cache', output)
      self.assertCacheEmpty()

  # FROZEN_CACHE prevents cache clears, and prevents building
  def test_FROZEN_CACHE(self):
    restore_and_set_up()
    self.erase_cache()
    self.ensure_cache()
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    # changing config file should not clear cache
    add_to_config('FROZEN_CACHE = True')
    self.do([EMCC])
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    # building libraries is disallowed
    output = self.do([EMBUILDER, 'build', 'libemmalloc'])
    self.assertIn('FROZEN_CACHE disallows building system libs', output)

  # Test that if multiple processes attempt to access or build stuff to the
  # cache on demand, that exactly one of the processes will, and the other
  # processes will block to wait until that process finishes.
  def test_emcc_multiprocess_cache_access(self):
    restore_and_set_up()

    create_test_file('test.c', r'''
      #include <stdio.h>
      int main() {
        printf("hello, world!\n");
        return 0;
      }
      ''')
    cache_dir_name = self.in_dir('test_cache')
    with env_modify({'EM_CACHE': cache_dir_name}):
      tasks = []
      num_times_libc_was_built = 0
      for i in range(3):
        p = self.run_process([EMCC, 'test.c', '-o', '%d.js' % i], stderr=STDOUT, stdout=PIPE)
        tasks += [p]
      for p in tasks:
        print('stdout:\n', p.stdout)
        if 'generating system library: libc' in p.stdout:
          num_times_libc_was_built += 1

    # The cache directory must exist after the build
    self.assertTrue(os.path.exists(cache_dir_name))
    # The cache directory must contain a built libc
    self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'wasm', 'libc.a')))
    # Exactly one child process should have triggered libc build!
    self.assertEqual(num_times_libc_was_built, 1)

  def test_emconfig(self):
    restore_and_set_up()

    fd, custom_config_filename = tempfile.mkstemp(prefix='.emscripten_config_')

    orig_config = open(CONFIG_FILE, 'r').read()

    # Move the ~/.emscripten to a custom location.
    with os.fdopen(fd, "w") as f:
      f.write(get_basic_config())

    # Make a syntax error in the original config file so that attempting to access it would fail.
    open(CONFIG_FILE, 'w').write('asdfasdfasdfasdf\n\'\'\'' + orig_config)

    temp_dir = tempfile.mkdtemp(prefix='emscripten_temp_')

    with chdir(temp_dir):
      self.run_process([EMCC, '--em-config', custom_config_filename] + MINIMAL_HELLO_WORLD + ['-O2'])
      result = self.run_js('a.out.js')

    self.assertContained('hello, world!', result)

    # Clean up created temp files.
    os.remove(custom_config_filename)
    shutil.rmtree(temp_dir)

  @no_wasm_backend('depends on WASM=0 working')
  def test_emcc_ports(self):
    restore_and_set_up()

    # listing ports

    out = self.do([EMCC, '--show-ports'])
    assert 'Available ports:' in out, out
    assert 'SDL2' in out, out
    assert 'SDL2_image' in out, out
    assert 'SDL2_net' in out, out

    # using ports
    RETRIEVING_MESSAGE = 'retrieving port'
    BUILDING_MESSAGE = 'generating port'

    PORTS_DIR = system_libs.Ports.get_dir()

    for i in [0, 1]:
      self.do([EMCC, '--clear-cache'])
      print(i)
      if i == 0:
        try_delete(PORTS_DIR)
      else:
        self.do([EMCC, '--clear-ports'])
      assert not os.path.exists(PORTS_DIR)

      # Building a file that doesn't need ports should not trigger anything
      output = self.do([EMCC, path_from_root('tests', 'hello_world_sdl.cpp')])
      assert RETRIEVING_MESSAGE not in output, output
      assert BUILDING_MESSAGE not in output
      print('no', output)
      assert not os.path.exists(PORTS_DIR)

      def first_use():
        output = self.do([EMCC, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0', '-s', 'USE_SDL=2'])
        assert RETRIEVING_MESSAGE in output, output
        assert BUILDING_MESSAGE in output, output
        self.assertExists(PORTS_DIR)
        print('yes', output)

      def second_use():
        # Using it again avoids retrieve and build
        output = self.do([EMCC, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0', '-s', 'USE_SDL=2'])
        assert RETRIEVING_MESSAGE not in output, output
        assert BUILDING_MESSAGE not in output, output

      # Building a file that need a port does trigger stuff
      first_use()
      second_use()

      # if the tag doesn't match, we retrieve and rebuild
      subdir = os.listdir(os.path.join(PORTS_DIR, 'sdl2'))[0]
      os.rename(os.path.join(PORTS_DIR, 'sdl2', subdir), os.path.join(PORTS_DIR, 'sdl2', 'old-subdir'))
      ensure_dir('old-sub')
      open(os.path.join('old-sub', 'a.txt'), 'w').write('waka')
      open(os.path.join('old-sub', 'b.txt'), 'w').write('waka')
      with zipfile.ZipFile(os.path.join(PORTS_DIR, 'sdl2.zip'), 'w') as z:
        z.write(os.path.join('old-sub', 'a.txt'))
        z.write(os.path.join('old-sub', 'b.txt'))

      first_use()
      second_use()

  def test_js_engine_path(self):
    # Test that running JS commands works for node, d8, and jsc and is not path dependent
    restore_and_set_up()

    sample_script = path_from_root('tests', 'print_args.js')

    # Fake some JS engines
    # Note that the path contains 'd8'.
    test_path = self.in_dir('fake', 'abcd8765')
    ensure_dir(test_path)

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      jsengines = [('d8',     V8_ENGINE),
                   ('d8_g',   V8_ENGINE),
                   ('js',     SPIDERMONKEY_ENGINE),
                   ('node',   NODE_JS),
                   ('nodejs', NODE_JS)]
      for filename, engine in jsengines:
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
        os.chmod(test_engine_path, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

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
      Cache.erase()
      build()
      test()

    print('wacky env vars, these should not mess our bootstrapping')
    with env_modify({'EMCC_FORCE_STDLIBS': '1'}):
      Cache.erase()
      build()
      test()

  def test_vanilla(self):
    restore_and_set_up()
    Cache.erase()

    def make_fake(report):
      with open(CONFIG_FILE, 'a') as f:
        f.write('LLVM_ROOT = "' + self.in_dir('fake', 'bin') + '"\n')
        # BINARYEN_ROOT needs to exist in the config, even though this test
        # doesn't actually use it.
        f.write('BINARYEN_ROOT= "%s"\n' % self.in_dir('fake', 'bin'))

      make_fake_clang(self.in_dir('fake', 'bin', 'clang'), EXPECTED_LLVM_VERSION)
      make_fake_llc(self.in_dir('fake', 'bin', 'llc'), report)
      make_fake_lld(self.in_dir('fake', 'bin', 'wasm-ld'))

    # fake llc output

    def test_with_fake(report, expected):
      make_fake(report)
      with env_modify({'EMCC_DEBUG': '1'}):
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], expected)

    test_with_fake('got js backend! JavaScript (asm.js, emscripten) backend', 'LLVM has not been built with the WebAssembly backend')
    try_delete(CANONICAL_TEMP_DIR)

  def test_required_config_settings(self):
    # with no binaryen root, an error is shown
    restore_and_set_up()

    open(CONFIG_FILE, 'a').write('\nBINARYEN_ROOT = ""\n')
    self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], 'BINARYEN_ROOT is set to empty value in %s' % CONFIG_FILE)

    open(CONFIG_FILE, 'a').write('\ndel BINARYEN_ROOT\n')
    self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], 'BINARYEN_ROOT is not defined in %s' % CONFIG_FILE)

  def test_embuilder_force(self):
    restore_and_set_up()
    self.do([EMBUILDER, 'build', 'libemmalloc'])
    # Second time it should not generate anything
    self.assertNotContained('generating system library', self.do([EMBUILDER, 'build', 'libemmalloc']))
    # Unless --force is specified
    self.assertContained('generating system library', self.do([EMBUILDER, 'build', 'libemmalloc', '--force']))

  def test_embuilder_wasm_backend(self):
    restore_and_set_up()
    # the --lto flag makes us build wasm-bc
    self.do([EMCC, '--clear-cache'])
    self.run_process([EMBUILDER, 'build', 'libemmalloc'])
    self.assertExists(os.path.join(shared.CACHE, 'wasm'))
    self.do([EMCC, '--clear-cache'])
    self.run_process([EMBUILDER, 'build', 'libemmalloc', '--lto'])
    self.assertExists(os.path.join(shared.CACHE, 'wasm-lto'))

  def test_binaryen_version(self):
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('\nBINARYEN_ROOT = "' + self.in_dir('fake') + '"')

    make_fake_wasm_opt(self.in_dir('fake', 'bin', 'wasm-opt'), 'foo')
    self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], 'error parsing binaryen version (wasm-opt version foo). Please check your binaryen installation')

    make_fake_wasm_opt(self.in_dir('fake', 'bin', 'wasm-opt'), '70')
    self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], 'unexpected binaryen version: 70 (expected ')
