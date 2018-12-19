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

from runner import RunnerCore, path_from_root, env_modify, chdir
from tools.shared import NODE_JS, PYTHON, EMCC, SPIDERMONKEY_ENGINE, V8_ENGINE, CONFIG_FILE, PIPE, STDOUT, EM_CONFIG, LLVM_ROOT, CANONICAL_TEMP_DIR
from tools.shared import run_process, try_delete, run_js, safe_ensure_dirs, expected_llvm_version, generate_sanity
from tools.shared import Cache, Settings
from tools import jsrun, shared

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


# wipe the config and sanity files, creating a blank slate
def wipe():
  try_delete(CONFIG_FILE)
  try_delete(SANITY_FILE)


def add_to_config(content):
  with open(CONFIG_FILE, 'a') as f:
    f.write(content + '\n')


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
      if platform.system() is not 'Windows':
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
      for content in ['EMSCRIPTEN_ROOT', 'LLVM_ROOT', 'NODE_JS', 'TEMP_DIR', 'COMPILER_ENGINE', 'JS_ENGINES']:
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
    assert os.path.exists('a.out.js'), output

  def test_llvm(self):
    LLVM_WARNING = 'LLVM version appears incorrect'

    restore_and_set_up()

    # Clang should report the version number we expect, and emcc should not warn
    assert shared.check_llvm_version()
    output = self.check_working(EMCC)
    assert LLVM_WARNING not in output, output

    # Fake a different llvm version
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake') + '"')

    with env_modify({'EM_IGNORE_SANITY': '1'}):
      for x in range(-2, 3):
        for y in range(-2, 3):
          expected_x, expected_y = (int(x) for x in expected_llvm_version().split('.'))
          expected_x += x
          expected_y += y
          if expected_x < 0 or expected_y < 0:
            continue # must be a valid llvm version
          print(expected_llvm_version(), x, y, expected_x, expected_y)
          make_fake_clang(path_from_root('tests', 'fake', 'clang'), '%s.%s' % (expected_x, expected_y))
          if x != 0 or y != 0:
            output = self.check_working(EMCC, LLVM_WARNING)
          else:
            output = self.check_working(EMCC)
            assert LLVM_WARNING not in output, output

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
    WARNING2 = 'you can fall back to the older (pre-fastcomp) compiler core, although that is not recommended, see http://kripken.github.io/emscripten-site/docs/building_from_source/LLVM-Backend.html'

    restore_and_set_up()

    # Should see js backend during sanity check
    self.assertTrue(shared.check_llvm())
    output = self.check_working(EMCC)
    self.assertNotIn(WARNING, output)
    self.assertNotIn(WARNING2, output)

    # Fake incorrect llc output, no mention of js backend
    restore_and_set_up()
    with open(CONFIG_FILE, 'a') as f:
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake', 'bin') + '"')
    # print '1', open(CONFIG_FILE).read()

    make_fake_clang(path_from_root('tests', 'fake', 'bin', 'clang'), expected_llvm_version())
    make_fake_llc(path_from_root('tests', 'fake', 'bin', 'llc'), 'no j-s backend for you!')
    self.check_working(EMCC, WARNING)
    self.check_working(EMCC, WARNING2)

    # fake some more
    for fake in ['llvm-link', 'llvm-ar', 'opt', 'llvm-as', 'llvm-dis', 'llvm-nm', 'lli']:
      open(path_from_root('tests', 'fake', 'bin', fake), 'w').write('.')
    try_delete(SANITY_FILE)
    self.check_working(EMCC, WARNING)
    # make sure sanity checks notice there is no source dir with version #
    make_fake_llc(path_from_root('tests', 'fake', 'bin', 'llc'), 'there IZ a js backend: JavaScript (asm.js, emscripten) backend')
    try_delete(SANITY_FILE)
    self.check_working(EMCC, 'clang version does not appear to include fastcomp')

    VERSION_WARNING = 'Emscripten, llvm and clang build versions do not match, this is dangerous'

    # add version number
    make_fake_clang(path_from_root('tests', 'fake', 'bin', 'clang'), '%s (emscripten waka : waka)' % expected_llvm_version())
    try_delete(SANITY_FILE)
    self.check_working(EMCC, VERSION_WARNING)

    restore_and_set_up()

    self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-s', 'ASM_JS=0'], '''Compiler settings are incompatible with fastcomp. You can fall back to the older compiler core, although that is not recommended''')

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
    assert os.path.exists(SANITY_FILE) # EMCC should have checked sanity successfully
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

  def test_emcc_caching(self):
    INCLUDING_MESSAGE = 'including X'
    BUILDING_MESSAGE = 'building X for cache'
    ERASING_MESSAGE = 'clearing cache'

    EMCC_CACHE = Cache.dirname

    restore_and_set_up()

    Cache.erase()
    assert not os.path.exists(EMCC_CACHE)

    with env_modify({'EMCC_DEBUG': '1'}):
      # Building a file that *does* need something *should* trigger cache
      # generation, but only the first time
      for filename, libname in [('hello_libcxx.cpp', 'libc++')]:
        for i in range(3):
          print(filename, libname, i)
          self.clear()
          output = self.do([EMCC, '-O' + str(i), '-s', '--llvm-lto', '0', path_from_root('tests', filename), '--save-bc', 'a.bc', '-s', 'DISABLE_EXCEPTION_CATCHING=0'])
          # print '\n\n\n', output
          assert INCLUDING_MESSAGE.replace('X', libname) in output
          if libname == 'libc':
            assert INCLUDING_MESSAGE.replace('X', 'libc++') not in output # we don't need libc++ in this code
          else:
            assert INCLUDING_MESSAGE.replace('X', 'libc') in output # libc++ always forces inclusion of libc
          assert (BUILDING_MESSAGE.replace('X', libname) in output) == (i == 0), 'Must only build the first time'
          self.assertContained('hello, world!', run_js('a.out.js'))
          assert os.path.exists(EMCC_CACHE)
          full_libname = libname + '.bc' if libname != 'libc++' else libname + '.a'
          assert os.path.exists(os.path.join(EMCC_CACHE, full_libname))

    try_delete(CANONICAL_TEMP_DIR)
    restore_and_set_up()

    def ensure_cache():
      self.do([PYTHON, EMCC, '-O2', path_from_root('tests', 'hello_world.c')])

    # Manual cache clearing
    ensure_cache()
    self.assertTrue(os.path.exists(EMCC_CACHE))
    self.assertTrue(os.path.exists(Cache.root_dirname))
    output = self.do([PYTHON, EMCC, '--clear-cache'])
    self.assertIn(ERASING_MESSAGE, output)
    self.assertFalse(os.path.exists(EMCC_CACHE))
    self.assertFalse(os.path.exists(Cache.root_dirname))
    self.assertIn(SANITY_MESSAGE, output)

    # Changing LLVM_ROOT, even without altering .emscripten, clears the cache
    ensure_cache()
    make_fake_clang(path_from_root('tests', 'fake', 'bin', 'clang'), expected_llvm_version())
    with env_modify({'LLVM': path_from_root('tests', 'fake', 'bin')}):
      self.assertTrue(os.path.exists(EMCC_CACHE))
      output = self.do([PYTHON, EMCC])
      self.assertIn(ERASING_MESSAGE, output)
      self.assertFalse(os.path.exists(EMCC_CACHE))

  def test_nostdincxx(self):
    restore_and_set_up()
    Cache.erase()

    for compiler in [EMCC]:
      print(compiler)
      run_process([PYTHON, EMCC] + MINIMAL_HELLO_WORLD + ['-v']) # run once to ensure binaryen port is all ready
      proc = run_process([PYTHON, EMCC] + MINIMAL_HELLO_WORLD + ['-v'], stdout=PIPE, stderr=PIPE)
      out = proc.stdout
      err = proc.stderr
      proc2 = run_process([PYTHON, EMCC] + MINIMAL_HELLO_WORLD + ['-v', '-nostdinc++'], stdout=PIPE, stderr=PIPE)
      out2 = proc2.stdout
      err2 = proc2.stderr
      self.assertIdentical(out, out2)

      def focus(e):
        assert 'search starts here:' in e, e
        assert e.count('End of search list.') == 1, e
        return e[e.index('search starts here:'):e.index('End of search list.') + 20]

      err = focus(err)
      err2 = focus(err2)
      assert err == err2, err + '\n\n\n\n' + err2

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

    from tools import system_libs
    PORTS_DIR = system_libs.Ports.get_dir()

    for compiler in [EMCC]:
      print(compiler)

      for i in [0, 1]:
        self.do([PYTHON, EMCC, '--clear-cache'])
        print(i)
        if i == 0:
          try_delete(PORTS_DIR)
        else:
          self.do([PYTHON, compiler, '--clear-ports'])
        assert not os.path.exists(PORTS_DIR)

        # Building a file that doesn't need ports should not trigger anything
        # (avoid wasm to avoid the binaryen port)
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0'])
        print('no', output)
        assert RETRIEVING_MESSAGE not in output, output
        assert BUILDING_MESSAGE not in output
        assert not os.path.exists(PORTS_DIR)

        # Building a file that need a port does trigger stuff
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0', '-s', 'USE_SDL=2'])
        print('yes', output)
        assert RETRIEVING_MESSAGE in output, output
        assert BUILDING_MESSAGE in output, output
        assert os.path.exists(PORTS_DIR)

        def second_use():
          # Using it again avoids retrieve and build
          output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0', '-s', 'USE_SDL=2'])
          assert RETRIEVING_MESSAGE not in output, output
          assert BUILDING_MESSAGE not in output, output

        second_use()

        # if the version isn't sufficient, we retrieve and rebuild
        subdir = os.listdir(os.path.join(PORTS_DIR, 'sdl2'))[0]
        os.rename(os.path.join(PORTS_DIR, 'sdl2', subdir), os.path.join(PORTS_DIR, 'sdl2', 'old-subdir'))
        import zipfile
        z = zipfile.ZipFile(os.path.join(PORTS_DIR, 'sdl2' + '.zip'), 'w')
        if not os.path.exists('old-sub'):
          os.mkdir('old-sub')
        open(os.path.join('old-sub', 'a.txt'), 'w').write('waka')
        open(os.path.join('old-sub', 'b.txt'), 'w').write('waka')
        z.write(os.path.join('old-sub', 'a.txt'))
        z.write(os.path.join('old-sub', 'b.txt'))
        z.close()
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'WASM=0', '-s', 'USE_SDL=2'])
        assert RETRIEVING_MESSAGE in output, output
        assert BUILDING_MESSAGE in output, output
        assert os.path.exists(PORTS_DIR)

        second_use()

  def test_native_optimizer(self):
    restore_and_set_up()

    def build():
      return self.check_working([EMCC] + MINIMAL_HELLO_WORLD + ['-O2', '-s', 'WASM=0'], 'running js post-opts')

    def test():
      self.assertContained('hello, world!', run_js('a.out.js'))

    with env_modify({'EMCC_DEBUG': '1'}):
      # basic usage or lack of usage
      for native in [None, 0, 1]:
        print('phase 1, part', native)
        Cache.erase()
        try:
          if native is not None:
            os.environ['EMCC_NATIVE_OPTIMIZER'] = str(native)
          output = build()
          assert ('js optimizer using native' in output) == (not not (native or native is None)), output
          test()
          if native or native is None: # None means use the default, which is to use the native optimizer
            assert 'building native optimizer' in output, output
            # compile again, no rebuild of optimizer
            output = build()
            assert 'building native optimizer' not in output
            assert 'js optimizer using native' in output
            test()
        finally:
          if native is not None:
            del os.environ['EMCC_NATIVE_OPTIMIZER']

      # force a build failure, see we fall back to non-native

      for native in [1, 'g']:
        with env_modify({'EMCC_NATIVE_OPTIMIZER': str(native)}):
          print('phase 2, part', native)
          Cache.erase()

          try:
            # break it
            f = path_from_root('tools', 'optimizer', 'optimizer-main.cpp')
            src = open(f).read()
            bad = src.replace('main', '!waka waka<')
            assert bad != src
            open(f, 'w').write(bad)
            # first try
            output = build()
            assert 'failed to build native optimizer' in output, output
            if native == 1:
              assert 'to see compiler errors, build with EMCC_NATIVE_OPTIMIZER=g' in output
              assert 'waka waka' not in output
            else:
              assert 'output from attempt' in output, output
              assert 'waka waka' in output, output
            assert 'js optimizer using native' not in output
            test() # still works, without native optimizer
            # second try, see previous failure
            output = build()
            assert 'failed to build native optimizer' not in output
            assert 'seeing that optimizer could not be built' in output
            test() # still works, without native optimizer
            # clear cache, try again
            Cache.erase()
            output = build()
            assert 'failed to build native optimizer' in output
            test() # still works, without native optimizer
          finally:
            open(f, 'w').write(src)

          Cache.erase()

          # now it should work again
          output = build()
          assert 'js optimizer using native' in output
          test() # still works

    try_delete(CANONICAL_TEMP_DIR)

  def test_embuilder(self):
    restore_and_set_up()

    tests = [
      ([PYTHON, EMBUILDER], ['Emscripten System Builder Tool', 'build libc', 'native_optimizer'], True, []),
      ([PYTHON, EMBUILDER, 'build', 'waka'], 'ERROR', False, []),
      ([PYTHON, EMBUILDER, 'build', 'struct_info'], ['building and verifying struct_info', 'success'], True, ['generated_struct_info.json']),
      ([PYTHON, EMBUILDER, 'build', 'libc'], ['building and verifying libc', 'success'], True, ['libc.bc']),
      ([PYTHON, EMBUILDER, 'build', 'libc-mt'], ['building and verifying libc-mt', 'success'], True, ['libc-mt.bc']),
      ([PYTHON, EMBUILDER, 'build', 'libc-extras'], ['building and verifying libc-extras', 'success'], True, ['libc-extras.bc']),
      ([PYTHON, EMBUILDER, 'build', 'dlmalloc'], ['building and verifying dlmalloc', 'success'], True, ['libdlmalloc.bc']),
      ([PYTHON, EMBUILDER, 'build', 'dlmalloc_debug'], ['building and verifying dlmalloc', 'success'], True, ['libdlmalloc_debug.bc']),
      ([PYTHON, EMBUILDER, 'build', 'dlmalloc_threadsafe'], ['building and verifying dlmalloc_threadsafe', 'success'], True, ['libdlmalloc_threadsafe.bc']),
      ([PYTHON, EMBUILDER, 'build', 'dlmalloc_threadsafe_debug'], ['building and verifying dlmalloc', 'success'], True, ['libdlmalloc_threadsafe_debug.bc']),
      ([PYTHON, EMBUILDER, 'build', 'emmalloc'], ['building and verifying emmalloc', 'success'], True, ['libemmalloc.bc']),
      ([PYTHON, EMBUILDER, 'build', 'emmalloc_debug'], ['building and verifying emmalloc', 'success'], True, ['libemmalloc_debug.bc']),
      ([PYTHON, EMBUILDER, 'build', 'pthreads'], ['building and verifying pthreads', 'success'], True, ['libpthreads.bc']),
      ([PYTHON, EMBUILDER, 'build', 'libc++'], ['success'], True, ['libc++.a']),
      ([PYTHON, EMBUILDER, 'build', 'libc++_noexcept'], ['success'], True, ['libc++_noexcept.a']),
      ([PYTHON, EMBUILDER, 'build', 'libc++abi'], ['success'], True, ['libc++abi.bc']),
      ([PYTHON, EMBUILDER, 'build', 'gl'], ['success'], True, ['libgl.bc']),
      ([PYTHON, EMBUILDER, 'build', 'native_optimizer'], ['success'], True, ['optimizer.2.exe']),
      ([PYTHON, EMBUILDER, 'build', 'zlib'], ['building and verifying zlib', 'success'], True, ['zlib.bc']),
      ([PYTHON, EMBUILDER, 'build', 'libpng'], ['building and verifying libpng', 'success'], True, ['libpng.bc']),
      ([PYTHON, EMBUILDER, 'build', 'bullet'], ['building and verifying bullet', 'success'], True, ['bullet.bc']),
      ([PYTHON, EMBUILDER, 'build', 'vorbis'], ['building and verifying vorbis', 'success'], True, ['vorbis.bc']),
      ([PYTHON, EMBUILDER, 'build', 'ogg'], ['building and verifying ogg', 'success'], True, ['ogg.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2'], ['success'], True, ['sdl2.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2-gfx'], ['success'], True, ['sdl2-gfx.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2-image'], ['success'], True, ['sdl2-image.bc']),
      ([PYTHON, EMBUILDER, 'build', 'freetype'], ['building and verifying freetype', 'success'], True, ['freetype.bc']),
      ([PYTHON, EMBUILDER, 'build', 'harfbuzz'], ['building and verifying harfbuzz', 'success'], True, ['harfbuzz.bc']),
      ([PYTHON, EMBUILDER, 'build', 'icu'], ['building and verifying icu', 'success'], True, ['icu.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2-ttf'], ['building and verifying sdl2-ttf', 'success'], True, ['sdl2-ttf.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2-net'], ['building and verifying sdl2-net', 'success'], True, ['sdl2-net.bc']),
      ([PYTHON, EMBUILDER, 'build', 'sdl2-mixer'], ['building and verifying sdl2-mixer', 'success'], True, ['sdl2-mixer.bc']),
      ([PYTHON, EMBUILDER, 'build', 'binaryen'], ['building and verifying binaryen', 'success'], True, []),
      ([PYTHON, EMBUILDER, 'build', 'cocos2d'], ['building and verifying cocos2d', 'success'], True, ['libCocos2d.bc']),
      ([PYTHON, EMBUILDER, 'build', 'libc-wasm'], ['building and verifying libc-wasm', 'success'], True, ['libc-wasm.bc']),
    ]
    if Settings.WASM_BACKEND:
      tests.append(([PYTHON, EMBUILDER, 'build', 'libcompiler_rt_wasm'], ['building and verifying libcompiler_rt_wasm', 'success'], True, ['libcompiler_rt_wasm.a']),)

    Cache.erase()

    for command, expected, success, result_libs in tests:
      print(command)
      for lib in result_libs:
        if os.path.sep in lib:
          dirname = os.path.dirname(Cache.get_path(lib))
          print('    erase dir', dirname)
          try_delete(dirname)
        else:
          print('    erase file', lib)
          try_delete(Cache.get_path(lib))
      proc = run_process(command, stdout=PIPE, stderr=STDOUT, check=False)
      out = proc.stdout
      if success and proc.returncode != 0:
        print(out)
      assert (proc.returncode == 0) == success
      if not isinstance(expected, list):
        expected = [expected]
      for ex in expected:
        print('    seek', ex)
        assert ex in out, out
      for lib in result_libs:
        print('    verify', lib)
        assert os.path.exists(Cache.get_path(lib))

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

      with open(path_from_root('tests', 'fake', 'bin', 'wasm-ld'), 'w') as f:
        f.write('#!/bin/sh\n')
        f.write('exit 0\n')
      os.chmod(path_from_root('tests', 'fake', 'bin', 'wasm-ld'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

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
      assert os.path.exists(os.path.join(root_cache, 'wasm'))

    with env_modify({'EMCC_WASM_BACKEND': '0'}):
      self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')
      assert os.path.exists(os.path.join(root_cache, 'asmjs'))
      shutil.rmtree(os.path.join(root_cache, 'asmjs'))

    self.check_working([EMCC] + MINIMAL_HELLO_WORLD, '')
    assert os.path.exists(os.path.join(root_cache, 'asmjs'))

  def test_wasm_backend_builds(self):
    # we can build a program using the wasm backend, rebuilding binaryen etc. as needed
    restore_and_set_up()

    def check():
      print(self.do([PYTHON, EMCC, '--clear-cache']))
      print(self.do([PYTHON, EMCC, '--clear-ports']))
      with env_modify({'EMCC_WASM_BACKEND': '1'}):
        self.check_working([EMCC, path_from_root('tests', 'hello_world.c')], '')

    print('normally')
    check()
    print('with no BINARYEN_ROOT')
    open(CONFIG_FILE, 'a').write('''
BINARYEN_ROOT = ''
    ''')
    print(open(CONFIG_FILE).read())
    check()

  def test_binaryen(self):
    import tools.ports.binaryen as binaryen
    tag_file = Cache.get_path('binaryen_tag_' + binaryen.TAG + '.txt')

    assert not os.environ.get('BINARYEN') # must not have binaryen env var set

    # test in 2 modes - with BINARYEN_ROOT in the config file, set to '', and without it entirely
    for binaryen_root_in_config in [1, 0]:
      print('binaryen_root_in_config:', binaryen_root_in_config)

      def prep():
        restore_and_set_up()
        print('clearing ports...')
        print(self.do([PYTHON, EMCC, '--clear-ports']))
        wipe()
        self.do([PYTHON, EMCC]) # first run stage
        try_delete(tag_file)
        # if BINARYEN_ROOT is set, we don't build the port. Check we do build it if not
        if binaryen_root_in_config:
          config = open(CONFIG_FILE).read()
          assert '''BINARYEN_ROOT = os.path.expanduser(os.getenv('BINARYEN', ''))''' in config, config # setup created it to be ''
          print('created config:')
          print(config)
          restore_and_set_up()
          config = open(CONFIG_FILE).read()
          config = config.replace('BINARYEN_ROOT', '''BINARYEN_ROOT = os.path.expanduser(os.getenv('BINARYEN', '')) # ''')
        else:
          restore_and_set_up()
          config = open(CONFIG_FILE).read()
          config = config.replace('BINARYEN_ROOT', '#')
        print('modified config:')
        print(config)
        open(CONFIG_FILE, 'w').write(config)

      print('build using embuilder')
      prep()
      run_process([PYTHON, EMBUILDER, 'build', 'binaryen'])
      assert os.path.exists(tag_file)
      run_process([PYTHON, EMCC] + MINIMAL_HELLO_WORLD + ['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
      self.assertContained('hello, world!', run_js('a.out.js'))

      print('see we show an error for emmake (we cannot build natively under emmake)')
      prep()
      try_delete('a.out.js')
      out = self.do([PYTHON, path_from_root('emmake.py'), EMCC] + MINIMAL_HELLO_WORLD + ['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"'])
      assert not os.path.exists(tag_file)
      assert not os.path.exists('a.out.js')
      self.assertContained('For example, for binaryen, do "python embuilder.py build binaryen"', out)

      if not binaryen_root_in_config:
        print('build on demand')
        for side_module in (False, True):
          print(side_module)
          prep()
          assert not os.path.exists(tag_file)
          try_delete('a.out.js')
          try_delete('a.out.wasm')
          cmd = [PYTHON, EMCC]
          if not side_module:
            cmd += MINIMAL_HELLO_WORLD
          else:
            # EM_ASM doesn't work in a wasm side module, build a normal program
            cmd += [path_from_root('tests', 'hello_world.c'), '-s', 'SIDE_MODULE=1']
          cmd += ['-s', 'BINARYEN=1', '-s', 'BINARYEN_METHOD="interpret-binary"']
          run_process(cmd)
          assert os.path.exists(tag_file)
          assert os.path.exists('a.out.wasm')
          if not side_module:
            assert os.path.exists('a.out.js')
            self.assertContained('hello, world!', run_js('a.out.js'))
