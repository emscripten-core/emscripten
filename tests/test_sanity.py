# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os
import platform
import shutil
import stat
import time
import re
import tempfile
import zipfile

from runner import RunnerCore, path_from_root, env_modify, chdir
from runner import create_test_file, no_wasm_backend
from tools.shared import NODE_JS, PYTHON, EMCC, SPIDERMONKEY_ENGINE, V8_ENGINE
from tools.shared import CONFIG_FILE, PIPE, STDOUT, EM_CONFIG, LLVM_ROOT, CANONICAL_TEMP_DIR
from tools.shared import run_process, try_delete, run_js, safe_ensure_dirs
from tools.shared import expected_llvm_version, generate_sanity, Cache, Settings
from tools import jsrun, shared, system_libs

SANITY_FILE = CONFIG_FILE + '_sanity'
commands = [[PYTHON, EMCC], [PYTHON, path_from_root('tests', 'runner.py'), 'blahblah']]


def restore():
  shutil.copyfile(CONFIG_FILE + '_backup', CONFIG_FILE)


# restore the config file and set it up for our uses
def restore_and_set_up():
  restore()
  with open(CONFIG_FILE, 'a') as f:
    # don't use the native optimizer from the emsdk - we want to test how it builds
    f.write('\nEMSCRIPTEN_NATIVE_OPTIMIZER = ""\n')
    # make LLVM_ROOT sensitive to the LLVM env var, as we test that
    f.write('\nLLVM_ROOT = os.path.expanduser(os.getenv("LLVM", "%s"))\n' % LLVM_ROOT)
    # unfreeze the cache, so we can test that
    f.write('\nFROZEN_CACHE = False\n')


# wipe the config and sanity files, creating a blank slate
def wipe():
  try_delete(CONFIG_FILE)
  try_delete(SANITY_FILE)


def add_to_config(content):
  with open(CONFIG_FILE, 'a') as f:
    f.write('\n' + content + '\n')


def mtime(filename):
  return os.path.getmtime(filename)


def make_fake_clang(filename, version):
  """Create a fake clang that only handles --version
  --version writes to stdout (unlike -v which writes to stderr)
  """
  if not os.path.exists(os.path.dirname(filename)):
    os.makedirs(os.path.dirname(filename))
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
  if not os.path.exists(os.path.dirname(filename)):
    os.makedirs(os.path.dirname(filename))
  with open(filename, 'w') as f:
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake output\nRegistered Targets:\n%s"' % targets)
  os.chmod(filename, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)


def make_fake_lld(filename):
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
    shutil.copyfile(CONFIG_FILE, CONFIG_FILE + '_backup')

    print()
    print('Running sanity checks.')
    print('WARNING: This will modify %s, and in theory can break it although it should be restored properly. A backup will be saved in %s_backup' % (EM_CONFIG, EM_CONFIG))
    print()
    print('>>> the original settings file is:')
    print(open(os.path.expanduser('~/.emscripten')).read())
    print('<<<')
    print()

    assert os.path.exists(CONFIG_FILE), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG
    assert 'EMCC_DEBUG' not in os.environ, 'do not run sanity checks in debug mode!'
    assert 'EMCC_WASM_BACKEND' not in os.environ, 'do not force wasm backend either way in sanity checks!'

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

  def do(self, command):
    print('Running: ' + ' '.join(command))
    if type(command) is not list:
      command = [command]
    if command[0] == EMCC:
      command = [PYTHON] + command

    return run_process(command, stdout=PIPE, stderr=STDOUT, check=False).stdout

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

      try:
        temp_bin = tempfile.mkdtemp()
        old_environ_path = os.environ['PATH']
        os.environ['PATH'] = temp_bin + os.pathsep + old_environ_path
        make_executable('llvm-dis')
        make_executable('node')
        make_executable('python2')
        output = self.do(command)
      finally:
        os.environ['PATH'] = old_environ_path
        shutil.rmtree(temp_bin)

      self.assertContained('Welcome to Emscripten!', output)
      self.assertContained('This is the first time any of the Emscripten tools has been run.', output)
      self.assertContained('A settings file has been copied to %s, at absolute path: %s' % (EM_CONFIG, CONFIG_FILE), output)
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
      config_file = open(CONFIG_FILE).read()
      template_file = open(path_from_root('tools', 'settings_template_readonly.py')).read()
      self.assertNotContained('~/.emscripten', config_file)
      self.assertContained('~/.emscripten', template_file)
      self.assertNotContained('{{{', config_file)
      self.assertNotContained('}}}', config_file)
      self.assertContained('{{{', template_file)
      self.assertContained('}}}', template_file)
      for content in ['EMSCRIPTEN_ROOT', 'LLVM_ROOT', 'NODE_JS', 'COMPILER_ENGINE', 'JS_ENGINES']:
        self.assertContained(content, config_file)

      # The guessed config should be ok
      # XXX This depends on your local system! it is possible `which` guesses wrong
      # try_delete('a.out.js')
      # output = run_process([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE).output
      # self.assertContained('hello, world!', run_js('a.out.js'), output)

      # Second run, with bad EM_CONFIG
      for settings in ['blah', 'LLVM_ROOT="blarg"; JS_ENGINES=[]; COMPILER_ENGINE=NODE_JS=SPIDERMONKEY_ENGINE=[]']:
        f = open(CONFIG_FILE, 'w')
        f.write(settings)
        f.close()
        output = self.do(command)

        if 'LLVM_ROOT' not in settings:
          self.assertContained('Error in evaluating %s' % EM_CONFIG, output)
        elif 'runner.py' not in ' '.join(command):
          self.assertContained('ERROR', output) # sanity check should fail

  def test_closure_compiler(self):
    CLOSURE_FATAL = 'fatal: closure compiler'
    CLOSURE_WARNING = 'does not exist'

    # Sanity check should find closure
    restore_and_set_up()
    output = self.check_working(EMCC)
    self.assertNotContained(CLOSURE_FATAL, output)
    self.assertNotContained(CLOSURE_WARNING, output)

    # Append a bad path for closure, will warn
    f = open(CONFIG_FILE, 'a')
    f.write('CLOSURE_COMPILER = "/tmp/nowhere/nothingtoseehere/kjadsfkjwelkjsdfkqgas/nonexistent.txt"\n')
    f.close()
    output = self.check_working(EMCC, CLOSURE_WARNING)

    # And if you actually try to use the bad path, will be fatal
    f = open(CONFIG_FILE, 'a')
    f.write('CLOSURE_COMPILER = "/tmp/nowhere/nothingtoseehere/kjadsfkjwelkjsdfkqgas/nonexistent.txt"\n')
    f.close()
    output = self.check_working([EMCC, '-s', '--closure', '1'] + MINIMAL_HELLO_WORLD + ['-O2'], CLOSURE_FATAL)

    # With a working path, all is well
    restore_and_set_up()
    try_delete('a.out.js')
    output = self.check_working([EMCC, '-s', '--closure', '1'] + MINIMAL_HELLO_WORLD + ['-O2'], '')
    self.assertExists('a.out.js', output)

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
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake') + '"')

    real_version_x, real_version_y = (int(x) for x in expected_llvm_version().split('.'))
    if shared.get_llvm_target() == shared.WASM_TARGET:
      make_fake_llc(path_from_root('tests', 'fake', 'llc'), 'wasm32 - WebAssembly 32-bit')
      make_fake_lld(path_from_root('tests', 'fake', 'wasm-ld'))
    else:
      make_fake_llc(path_from_root('tests', 'fake', 'llc'), 'js - JavaScript (asm.js, emscripten)')

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      for inc_x in range(-2, 3):
        for inc_y in range(-2, 3):
          expected_x = real_version_x + inc_x
          expected_y = real_version_y + inc_y
          if expected_x < 0 or expected_y < 0:
            continue # must be a valid llvm version
          print("mod LLVM version: %d %d -> %d %d" % (real_version_x, real_version_x, expected_x, expected_y))
          make_fake_clang(path_from_root('tests', 'fake', 'clang'), '%s.%s' % (expected_x, expected_y))
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

  def test_llvm_fastcomp(self):
    WARNING = 'fastcomp in use, but LLVM has not been built with the JavaScript backend as a target'

    restore_and_set_up()

    # Should see js backend during sanity check
    self.assertTrue(shared.check_llvm())
    output = self.check_working(EMCC)
    self.assertNotIn(WARNING, output)

    # Fake incorrect llc output, no mention of js backend
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake', 'bin') + '"')
    # print '1', open(CONFIG_FILE).read()

    make_fake_clang(path_from_root('tests', 'fake', 'bin', 'clang'), expected_llvm_version())
    make_fake_llc(path_from_root('tests', 'fake', 'bin', 'llc'), 'no j-s backend for you!')
    self.check_working(EMCC, WARNING)

    # fake some more
    for fake in ['llvm-link', 'llvm-ar', 'opt', 'llvm-as', 'llvm-dis', 'llvm-nm', 'lli']:
      open(path_from_root('tests', 'fake', 'bin', fake), 'w').write('.')
    try_delete(SANITY_FILE)
    self.check_working(EMCC, WARNING)

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
    f = open(CONFIG_FILE, 'a')
    f.write('NODE_JS = "' + path_from_root('tests', 'fake', 'nodejs') + '"')
    f.close()

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      for version, succeed in [('v0.8.0', False),
                               ('v4.1.0', False),
                               ('v4.1.1', True),
                               ('v4.2.3-pre', True),
                               ('cheez', False)]:
        print(version, succeed)
        f = open(path_from_root('tests', 'fake', 'nodejs'), 'w')
        f.write('#!/bin/sh\n')
        f.write('''if [ $1 = "--version" ]; then
echo "%s"
else
%s $@
fi
''' % (version, NODE_JS))
        f.close()
        os.chmod(path_from_root('tests', 'fake', 'nodejs'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
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
    self.assertExists(SANITY_FILE) # EMCC should have checked sanity successfully
    assert mtime(SANITY_FILE) > mtime(CONFIG_FILE)
    assert generate_sanity() == open(SANITY_FILE).read()
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc run again should not sanity check, because the sanity file is newer
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # correct sanity contents mean we need not check
    open(SANITY_FILE, 'w').write(generate_sanity())
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)

    # incorrect sanity contents mean we *must* check
    open(SANITY_FILE, 'w').write('wakawaka')
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)

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
    time.sleep(0.1)
    restore_and_set_up()
    assert mtime(SANITY_FILE) < mtime(CONFIG_FILE)
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    assert mtime(SANITY_FILE) >= mtime(CONFIG_FILE)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc should be configurable directly from EM_CONFIG without any config file
    restore_and_set_up()
    config = open(CONFIG_FILE, 'r').read()
    open('main.cpp', 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from emcc with no config file\\n");
        return 0;
      }
    ''')

    wipe()
    with env_modify({'EM_CONFIG': config}):
      run_process([PYTHON, EMCC, 'main.cpp', '-o', 'a.out.js'])

    self.assertContained('hello from emcc with no config file', run_js('a.out.js'))

  def erase_cache(self):
    Cache.erase()
    assert not os.path.exists(Cache.dirname)

  def ensure_cache(self):
    self.do([PYTHON, EMCC, '-O2', path_from_root('tests', 'hello_world.c')])

  def test_emcc_caching(self):
    BUILDING_MESSAGE = 'generating system library: X'
    ERASING_MESSAGE = 'clearing cache'

    restore_and_set_up()
    self.erase_cache()

    # Building a file that *does* need something *should* trigger cache
    # generation, but only the first time
    libname = 'libc++'
    for i in range(3):
      print(i)
      self.clear()
      output = self.do([EMCC, '-O' + str(i), '-s', '--llvm-lto', '0', path_from_root('tests', 'hello_libcxx.cpp'), '--save-bc', 'a.bc', '-s', 'DISABLE_EXCEPTION_CATCHING=0'])
      print('\n\n\n', output)
      assert (BUILDING_MESSAGE.replace('X', libname) in output) == (i == 0), 'Must only build the first time'
      self.assertContained('hello, world!', run_js('a.out.js'))
      self.assertExists(Cache.dirname)
      full_libname = libname + '.bc' if libname != 'libc++' else libname + '.a'
      self.assertExists(os.path.join(Cache.dirname, full_libname))

    restore_and_set_up()

    # Manual cache clearing
    self.ensure_cache()
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    output = self.do([PYTHON, EMCC, '--clear-cache'])
    self.assertIn(ERASING_MESSAGE, output)
    self.assertFalse(os.path.exists(Cache.dirname))
    self.assertFalse(os.path.exists(Cache.root_dirname))
    self.assertIn(SANITY_MESSAGE, output)

    # Changing LLVM_ROOT, even without altering .emscripten, clears the cache
    self.ensure_cache()
    make_fake_clang(path_from_root('tests', 'fake', 'bin', 'clang'), expected_llvm_version())
    make_fake_llc(path_from_root('tests', 'fake', 'bin', 'llc'), 'js - JavaScript (asm.js, emscripten)')
    with env_modify({'LLVM': path_from_root('tests', 'fake', 'bin')}):
      self.assertTrue(os.path.exists(Cache.dirname))
      output = self.do([PYTHON, EMCC])
      self.assertIn(ERASING_MESSAGE, output)
      self.assertFalse(os.path.exists(Cache.dirname))

  # FROZEN_CACHE prevents cache clears, and prevents building
  def test_FROZEN_CACHE(self):
    restore_and_set_up()
    self.erase_cache()
    self.ensure_cache()
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    # changing config file should not clear cache
    add_to_config('FROZEN_CACHE = True')
    self.do([PYTHON, EMCC])
    self.assertTrue(os.path.exists(Cache.dirname))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    # building libraries is disallowed
    output = self.do([PYTHON, EMBUILDER, 'build', 'libemmalloc'])
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
    cache_dir_name = self.in_dir('emscripten_cache')
    tasks = []
    num_times_libc_was_built = 0
    for i in range(3):
      p = run_process([PYTHON, EMCC, 'test.c', '--cache', cache_dir_name, '-o', '%d.js' % i], stderr=STDOUT, stdout=PIPE)
      tasks += [p]
    for p in tasks:
      print('stdout:\n', p.stdout)
      if 'generating system library: libc' in p.stdout:
        num_times_libc_was_built += 1
    # The cache directory must exist after the build
    self.assertTrue(os.path.exists(cache_dir_name))
    # The cache directory must contain a built libc
    if self.is_wasm_backend():
      self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'wasm_o', 'libc.a')))
    else:
      self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'asmjs', 'libc.bc')))
    # Exactly one child process should have triggered libc build!
    self.assertEqual(num_times_libc_was_built, 1)

  def test_emcc_cache_flag(self):
    restore_and_set_up()

    cache_dir_name = self.in_dir('emscripten_cache')
    self.assertFalse(os.path.exists(cache_dir_name))
    create_test_file('test.c', r'''
      #include <stdio.h>
      int main() {
        printf("hello, world!\n");
        return 0;
      }
      ''')
    run_process([PYTHON, EMCC, 'test.c', '--cache', cache_dir_name], stderr=PIPE)
    # The cache directory must exist after the build
    self.assertTrue(os.path.exists(cache_dir_name))
    # The cache directory must contain a built libc'
    if self.is_wasm_backend():
      self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'wasm_o', 'libc.a')))
    else:
      self.assertTrue(os.path.exists(os.path.join(cache_dir_name, 'asmjs', 'libc.bc')))

  def test_emconfig(self):
    restore_and_set_up()

    fd, custom_config_filename = tempfile.mkstemp(prefix='.emscripten_config_')

    orig_config = open(CONFIG_FILE, 'r').read()

    # Move the ~/.emscripten to a custom location.
    with os.fdopen(fd, "w") as f:
      f.write(orig_config)

    # Make a syntax error in the original config file so that attempting to access it would fail.
    open(CONFIG_FILE, 'w').write('asdfasdfasdfasdf\n\'\'\'' + orig_config)

    temp_dir = tempfile.mkdtemp(prefix='emscripten_temp_')

    with chdir(temp_dir):
      self.do([PYTHON, EMCC, '--em-config', custom_config_filename] + MINIMAL_HELLO_WORLD + ['-O2'])
      result = run_js('a.out.js')

    self.assertContained('hello, world!', result)

    # Clean up created temp files.
    os.remove(custom_config_filename)
    if Settings.WASM_BACKEND:
      os.remove(custom_config_filename + "_sanity_wasm")
    else:
      os.remove(custom_config_filename + "_sanity")
    shutil.rmtree(temp_dir)

  @no_wasm_backend('depends on WASM=0 working')
  def test_emcc_ports(self):
    restore_and_set_up()

    # listing ports

    out = self.do([PYTHON, EMCC, '--show-ports'])
    assert 'Available ports:' in out, out
    assert 'SDL2' in out, out
    assert 'SDL2_image' in out, out
    assert 'SDL2_net' in out, out

    # using ports
    RETRIEVING_MESSAGE = 'retrieving port'
    BUILDING_MESSAGE = 'generating port'

    PORTS_DIR = system_libs.Ports.get_dir()

    for i in [0, 1]:
      self.do([PYTHON, EMCC, '--clear-cache'])
      print(i)
      if i == 0:
        try_delete(PORTS_DIR)
      else:
        self.do([PYTHON, EMCC, '--clear-ports'])
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
      if not os.path.exists('old-sub'):
        os.mkdir('old-sub')
      open(os.path.join('old-sub', 'a.txt'), 'w').write('waka')
      open(os.path.join('old-sub', 'b.txt'), 'w').write('waka')
      with zipfile.ZipFile(os.path.join(PORTS_DIR, 'sdl2.zip'), 'w') as z:
        z.write(os.path.join('old-sub', 'a.txt'))
        z.write(os.path.join('old-sub', 'b.txt'))

      first_use()
      second_use()

  def test_d8_path(self):
    """ Test that running JS commands works for node, d8, and jsc and is not path dependent """
    # Fake some JS engines
    restore_and_set_up()

    sample_script = path_from_root('tests', 'print_args.js')

    # Note that the path contains 'd8'.
    test_path = path_from_root('tests', 'fake', 'abcd8765')
    if not os.path.exists(test_path):
      os.makedirs(test_path)

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      jsengines = [('d8',     V8_ENGINE),
                   ('d8_g',   V8_ENGINE),
                   ('js',     SPIDERMONKEY_ENGINE),
                   ('node',   NODE_JS),
                   ('nodejs', NODE_JS)]
      for filename, engine in jsengines:
        if type(engine) is list:
          engine = engine[0]
        if engine == '':
            print('WARNING: Not testing engine %s, not configured.' % (filename))
            continue

        print(filename, engine)

        test_engine_path = os.path.join(test_path, filename)
        f = open(test_engine_path, 'w')
        f.write('#!/bin/sh\n')
        f.write('%s $@\n' % (engine))
        f.close()
        os.chmod(test_engine_path, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

        try:
          out = jsrun.run_js(sample_script, engine=test_engine_path, args=['--foo'], full_output=True, assert_returncode=0, skip_check=True)
        except Exception as e:
          if 'd8' in filename:
            assert False, 'Your d8 version does not correctly parse command-line arguments, please upgrade or delete from ~/.emscripten config file: %s' % (e)
          else:
            assert False, 'Error running script command: %s' % (e)

        self.assertEqual('0: --foo', out.strip())

  def test_wacky_env(self):
    restore_and_set_up()

    def build():
      return self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')

    def test():
      self.assertContained('hello, world!', run_js('a.out.js'))

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

    with env_modify({'EMCC_DEBUG': '1'}):
      # see that we test vanilla status, and just once
      TESTING = 'testing for asm.js target'
      self.check_working(EMCC, TESTING)
      for i in range(3):
        output = self.check_working(EMCC, 'check tells us to use')
        assert TESTING not in output
      # if env var tells us, do what it says
      with env_modify({'EMCC_WASM_BACKEND': '1'}):
        self.check_working(EMCC, 'EMCC_WASM_BACKEND tells us to use wasm backend')
      with env_modify({'EMCC_WASM_BACKEND': '0'}):
        self.check_working(EMCC, 'EMCC_WASM_BACKEND tells us to use asm.js backend')

    def make_fake(report):
      with open(CONFIG_FILE, 'a') as f:
        f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake', 'bin') + '"\n')
        # BINARYEN_ROOT needs to exist in the config, even though this test
        # doesn't actually use it.
        f.write('BINARYEN_ROOT= "%s"\n' % path_from_root('tests', 'fake', 'bin'))

      make_fake_llc(path_from_root('tests', 'fake', 'bin', 'llc'), report)
      make_fake_lld(path_from_root('tests', 'fake', 'bin', 'wasm-ld'))

    with env_modify({'EMCC_DEBUG': '1'}):
      make_fake('wasm32-unknown-unknown-elf')
      # see that we request the right backend from llvm
      with env_modify({'EMCC_WASM_BACKEND': '1'}):
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'wasm32-unknown-unknown-elf')
      make_fake('asmjs-unknown-emscripten')
      with env_modify({'EMCC_WASM_BACKEND': '0'}):
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'asmjs-unknown-emscripten')
      # check the current installed one is ok
      restore_and_set_up()
      self.check_working(EMCC)
      output = self.check_working(EMCC, 'check tells us to use')
      if 'wasm backend' in output:
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'wasm32-unknown-unknown-elf')
      else:
        assert 'asm.js backend' in output
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'asmjs-unknown-emscripten')

    # fake llc output

    def test_with_fake(report, expected):
      make_fake(report)
      with env_modify({'EMCC_DEBUG': '1'}):
        output = self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], expected)
        self.assertContained('config file changed since we checked vanilla', output)

    test_with_fake('got js backend! JavaScript (asm.js, emscripten) backend', 'check tells us to use asm.js backend')
    test_with_fake('got wasm32 backend! WebAssembly 32-bit',                  'check tells us to use wasm backend')

    # use LLVM env var to modify LLVM between vanilla checks

    assert not os.environ.get('LLVM'), 'we need to modify LLVM env var for this'

    f = open(CONFIG_FILE, 'a')
    f.write('LLVM_ROOT = os.getenv("LLVM", "' + path_from_root('tests', 'fake1', 'bin') + '")\n')
    f.close()

    safe_ensure_dirs(path_from_root('tests', 'fake1', 'bin'))
    f = open(path_from_root('tests', 'fake1', 'bin', 'llc'), 'w')
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake1 output\nRegistered Targets:\n%s"' % 'got js backend! JavaScript (asm.js, emscripten) backend')
    f.close()
    os.chmod(path_from_root('tests', 'fake1', 'bin', 'llc'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

    safe_ensure_dirs(path_from_root('tests', 'fake2', 'bin'))
    f = open(path_from_root('tests', 'fake2', 'bin', 'llc'), 'w')
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake2 output\nRegistered Targets:\n%s"' % 'got wasm32 backend! WebAssembly 32-bit')
    f.close()
    os.chmod(path_from_root('tests', 'fake2', 'bin', 'llc'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

    with env_modify({'EMCC_DEBUG': '1'}):
      self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'use asm.js backend')
      with env_modify({'LLVM': path_from_root('tests', 'fake2', 'bin')}):
        self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-c'], 'regenerating vanilla check since other llvm')

    try_delete(CANONICAL_TEMP_DIR)
    return # TODO: the rest of this

    # check separate cache dirs are used

    restore_and_set_up()
    self.check_working([EMCC], '')

    root_cache = os.path.expanduser('~/.emscripten_cache')
    if os.path.exists(os.path.join(root_cache, 'asmjs')):
      shutil.rmtree(os.path.join(root_cache, 'asmjs'))
    if os.path.exists(os.path.join(root_cache, 'wasm')):
      shutil.rmtree(os.path.join(root_cache, 'wasm'))

    with env_modify({'EMCC_WASM_BACKEND': '1'}):
      self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')
      self.assertExists(os.path.join(root_cache, 'wasm'))

    with env_modify({'EMCC_WASM_BACKEND': '0'}):
      self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')
      self.assertExists(os.path.join(root_cache, 'asmjs'))
      shutil.rmtree(os.path.join(root_cache, 'asmjs'))

    self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')
    self.assertExists(os.path.join(root_cache, 'asmjs'))

  def test_binaryen_root(self):
    # with no binaryen root, an error is shown
    restore_and_set_up()
    open(CONFIG_FILE, 'a').write('''
BINARYEN_ROOT = ''
    ''')
    self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], 'BINARYEN_ROOT must be set up in .emscripten')

  def test_embuilder_wasm_backend(self):
    if not Settings.WASM_BACKEND:
      self.skipTest('wasm backend only')
    restore_and_set_up()
    root_cache = os.path.expanduser('~/.emscripten_cache')
    # the --lto flag makes us build wasm_bc
    self.do([PYTHON, EMCC, '--clear-cache'])
    run_process([PYTHON, EMBUILDER, 'build', 'emmalloc'])
    self.assertExists(os.path.join(root_cache, 'wasm_o'))
    self.do([PYTHON, EMCC, '--clear-cache'])
    run_process([PYTHON, EMBUILDER, 'build', 'emmalloc', '--lto'])
    self.assertExists(os.path.join(root_cache, 'wasm_bc'))
