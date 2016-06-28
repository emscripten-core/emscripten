import os, shutil, stat, subprocess
from runner import RunnerCore, path_from_root
from tools.shared import *

SANITY_FILE = CONFIG_FILE + '_sanity'
commands = [[PYTHON, EMCC], [PYTHON, path_from_root('tests', 'runner.py'), 'blahblah']]

def restore():
  shutil.copyfile(CONFIG_FILE + '_backup', CONFIG_FILE)

def wipe():
  try_delete(CONFIG_FILE)
  try_delete(SANITY_FILE)

def mtime(filename):
  return os.stat(filename).st_mtime

SANITY_MESSAGE = 'Emscripten: Running sanity checks'

class sanity(RunnerCore):
  @classmethod
  def setUpClass(self):
    super(RunnerCore, self).setUpClass()
    shutil.copyfile(CONFIG_FILE, CONFIG_FILE + '_backup')

    print
    print 'Running sanity checks.'
    print 'WARNING: This will modify %s, and in theory can break it although it should be restored properly. A backup will be saved in %s_backup' % (EM_CONFIG, EM_CONFIG)
    print

    assert os.path.exists(CONFIG_FILE), 'To run these tests, we need a (working!) %s file to already exist' % EM_CONFIG
    assert not os.environ.get('EMCC_DEBUG'), 'do not run sanity checks in debug mode!'
    assert not os.environ.get('EMCC_WASM_BACKEND'), 'do not force wasm backend either way in sanity checks!'

  @classmethod
  def tearDownClass(self):
    super(RunnerCore, self).tearDownClass()

  def setUp(self):
    wipe()

  def tearDown(self):
    restore()

  def do(self, command):
    if type(command) is not list:
      command = [command]
    if command[0] == EMCC:
      command = [PYTHON] + command

    return Popen(command, stdout=PIPE, stderr=STDOUT).communicate()[0]

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

  def test_aaa_normal(self): # this should be the very first thing that runs. if this fails, everything else is irrelevant!
    for command in commands:
      # Your existing EM_CONFIG should work!
      restore()
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

      # The guessed config should be ok XXX This depends on your local system! it is possible `which` guesses wrong
      #try_delete('a.out.js')
      #output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE).communicate()
      #self.assertContained('hello, world!', run_js('a.out.js'), output)

      # Second run, with bad EM_CONFIG
      for settings in ['blah', 'LLVM_ROOT="blarg"; JS_ENGINES=[]; COMPILER_ENGINE=NODE_JS=SPIDERMONKEY_ENGINE=[]']:
        f = open(CONFIG_FILE, 'w')
        f.write(settings)
        f.close()
        output = self.do(command)

        if 'LLVM_ROOT' not in settings:
          self.assertContained('Error in evaluating %s' % EM_CONFIG, output)
        elif 'runner.py' not in ' '.join(command):
          self.assertContained('CRITICAL', output) # sanity check should fail

  def test_closure_compiler(self):
    CLOSURE_FATAL = 'fatal: closure compiler'
    CLOSURE_WARNING = 'does not exist'

    # Sanity check should find closure
    restore()
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
    output = self.check_working([EMCC, '-O2', '-s', '--closure', '1', 'tests/hello_world.cpp'], CLOSURE_FATAL)

    # With a working path, all is well
    restore()
    try_delete('a.out.js')
    output = self.check_working([EMCC, '-O2', '-s', '--closure', '1', 'tests/hello_world.cpp'], '')
    assert os.path.exists('a.out.js'), output

  def test_llvm(self):
    LLVM_WARNING = 'LLVM version appears incorrect'

    restore()

    # Clang should report the version number we expect, and emcc should not warn
    assert check_clang_version()
    output = self.check_working(EMCC)
    assert LLVM_WARNING not in output, output

    # Fake a different llvm version
    restore()
    f = open(CONFIG_FILE, 'a')
    f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake') + '"')
    f.close()

    if not os.path.exists(path_from_root('tests', 'fake')):
      os.makedirs(path_from_root('tests', 'fake'))

    try:
      os.environ['EM_IGNORE_SANITY'] = '1'
      for x in range(-2, 3):
        for y in range(-2, 3):
          f = open(path_from_root('tests', 'fake', 'clang'), 'w')
          f.write('#!/bin/sh\n')
          f.write('echo "clang version %d.%d" 1>&2\n' % (EXPECTED_LLVM_VERSION[0] + x, EXPECTED_LLVM_VERSION[1] + y))
          f.close()
          shutil.copyfile(path_from_root('tests', 'fake', 'clang'), path_from_root('tests', 'fake', 'clang++'))
          os.chmod(path_from_root('tests', 'fake', 'clang'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
          os.chmod(path_from_root('tests', 'fake', 'clang++'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
          if x != 0 or y != 0:
            output = self.check_working(EMCC, LLVM_WARNING)
          else:
            output = self.check_working(EMCC)
            assert LLVM_WARNING not in output, output
    finally:
      del os.environ['EM_IGNORE_SANITY']

  def test_llvm_fastcomp(self):
    WARNING = 'fastcomp in use, but LLVM has not been built with the JavaScript backend as a target'
    WARNING2 = 'you can fall back to the older (pre-fastcomp) compiler core, although that is not recommended, see http://kripken.github.io/emscripten-site/docs/building_from_source/LLVM-Backend.html'

    restore()

    # Should see js backend during sanity check
    assert check_fastcomp()
    output = self.check_working(EMCC)
    assert WARNING not in output, output
    assert WARNING2 not in output, output

    # Fake incorrect llc output, no mention of js backend
    restore()
    f = open(CONFIG_FILE, 'a')
    f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake', 'bin') + '"')
    f.close()
    #print '1', open(CONFIG_FILE).read()

    try_delete(path_from_root('tests', 'fake'))
    os.makedirs(path_from_root('tests', 'fake', 'bin'))

    f = open(path_from_root('tests', 'fake', 'bin', 'llc'), 'w')
    f.write('#!/bin/sh\n')
    f.write('echo "llc fake output\nRegistered Targets:\nno j-s backend for you!"')
    f.close()
    os.chmod(path_from_root('tests', 'fake', 'bin', 'llc'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
    output = self.check_working(EMCC, WARNING)
    output = self.check_working(EMCC, WARNING2)

    # fake some more
    for fake in ['llvm-link', 'clang', 'clang++', 'llvm-ar', 'opt', 'llvm-as', 'llvm-dis', 'llvm-nm', 'lli']:
      open(path_from_root('tests', 'fake', 'bin', fake), 'w').write('.')
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, WARNING)
    # make sure sanity checks notice there is no source dir with version #
    open(path_from_root('tests', 'fake', 'bin', 'llc'), 'w').write('#!/bin/sh\necho "Registered Targets: there IZ a js backend: JavaScript (asm.js, emscripten) backend"')
    open(path_from_root('tests', 'fake', 'bin', 'clang++'), 'w').write('#!/bin/sh\necho "clang version %s (blah blah)" >&2\necho "..." >&2\n' % '.'.join(map(str, EXPECTED_LLVM_VERSION)))
    os.chmod(path_from_root('tests', 'fake', 'bin', 'llc'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
    os.chmod(path_from_root('tests', 'fake', 'bin', 'clang++'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, 'did not see a source tree above or next to the LLVM root directory')

    VERSION_WARNING = 'Emscripten, llvm and clang repo versions do not match, this is dangerous'
    BUILD_VERSION_WARNING = 'Emscripten, llvm and clang build versions do not match, this is dangerous'

    # add version number
    open(path_from_root('tests', 'fake', 'emscripten-version.txt'), 'w').write('waka')
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, VERSION_WARNING)

    os.makedirs(path_from_root('tests', 'fake', 'tools', 'clang'))

    open(path_from_root('tests', 'fake', 'tools', 'clang', 'emscripten-version.txt'), 'w').write(EMSCRIPTEN_VERSION)
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, VERSION_WARNING)

    open(path_from_root('tests', 'fake', 'emscripten-version.txt'), 'w').write(EMSCRIPTEN_VERSION)
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC)
    assert VERSION_WARNING not in output

    open(path_from_root('tests', 'fake', 'tools', 'clang', 'emscripten-version.txt'), 'w').write('waka')
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, VERSION_WARNING)

    # restore clang version to ok, and fake the *build* versions
    open(path_from_root('tests', 'fake', 'tools', 'clang', 'emscripten-version.txt'), 'w').write(EMSCRIPTEN_VERSION)
    output = self.check_working(EMCC)
    assert VERSION_WARNING not in output
    fake = '#!/bin/sh\necho "clang version %s (blah blah) (emscripten waka : waka)"\necho "..."\n' % '.'.join(map(str, EXPECTED_LLVM_VERSION))
    open(path_from_root('tests', 'fake', 'bin', 'clang'), 'w').write(fake)
    open(path_from_root('tests', 'fake', 'bin', 'clang++'), 'w').write(fake)
    os.chmod(path_from_root('tests', 'fake', 'bin', 'clang'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
    os.chmod(path_from_root('tests', 'fake', 'bin', 'clang++'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)
    try_delete(SANITY_FILE)
    output = self.check_working(EMCC, BUILD_VERSION_WARNING)
    assert VERSION_WARNING not in output
    # break clang repo version again, see it hides the build warning
    open(path_from_root('tests', 'fake', 'tools', 'clang', 'emscripten-version.txt'), 'w').write('waka')
    output = self.check_working(EMCC, VERSION_WARNING)
    assert BUILD_VERSION_WARNING not in output

    restore()

    self.check_working([EMCC, 'tests/hello_world.cpp', '-s', 'ASM_JS=0'], '''Compiler settings are incompatible with fastcomp. You can fall back to the older compiler core, although that is not recommended''')

  def test_node(self):
    NODE_WARNING = 'node version appears too old'
    NODE_WARNING_2 = 'cannot check node version'

    restore()

    # Clang should report the version number we expect, and emcc should not warn
    assert check_node_version()
    output = self.check_working(EMCC)
    assert NODE_WARNING not in output, output

    # Fake a different node version
    restore()
    f = open(CONFIG_FILE, 'a')
    f.write('NODE_JS = "' + path_from_root('tests', 'fake', 'nodejs') + '"')
    f.close()

    if not os.path.exists(path_from_root('tests', 'fake')):
      os.makedirs(path_from_root('tests', 'fake'))

    try:
      os.environ['EM_IGNORE_SANITY'] = '1'
      for version, succeed in [('v0.7.9', False),
                               ('v0.8.0', True),
                               ('v0.8.1', True),
                               ('v0.10.21-pre', True),
                               ('cheez', False)]:
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
    finally:
      del os.environ['EM_IGNORE_SANITY']

  def test_emcc(self):
    SANITY_FAIL_MESSAGE = 'sanity check failed to run'

    # emcc should check sanity if no ${EM_CONFIG}_sanity
    restore()
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
    try:
      os.environ['EMCC_DEBUG'] = '1'
      output = self.check_working(EMCC)
    finally:
      del os.environ['EMCC_DEBUG']
    self.assertContained(SANITY_MESSAGE, output)
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)

    # also with -v, with or without inputs
    output = self.check_working([EMCC, '-v'], SANITY_MESSAGE)
    output = self.check_working([EMCC, '-v', path_from_root('tests', 'hello_world.c')], SANITY_MESSAGE)

    # Make sure the test runner didn't do anything to the setup
    output = self.check_working(EMCC)
    self.assertNotContained(SANITY_MESSAGE, output)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc should also check sanity if the file is outdated
    time.sleep(0.1)
    restore()
    assert mtime(SANITY_FILE) < mtime(CONFIG_FILE)
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    assert mtime(SANITY_FILE) >= mtime(CONFIG_FILE)
    self.assertNotContained(SANITY_FAIL_MESSAGE, output)

    # emcc should be configurable directly from EM_CONFIG without any config file
    restore()
    config = open(CONFIG_FILE, 'r').read()
    os.environ['EM_CONFIG'] = config
    wipe()
    dirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
    open(os.path.join(dirname, 'main.cpp'), 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from emcc with no config file\\n");
        return 0;
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join(dirname, 'main.cpp'), '-o', os.path.join(dirname, 'a.out.js')]).communicate()
    del os.environ['EM_CONFIG']
    old_dir = os.getcwd()
    try:
      os.chdir(dirname)
      self.assertContained('hello from emcc with no config file', run_js('a.out.js'))
    finally:
      os.chdir(old_dir)
    shutil.rmtree(dirname)

    try_delete(CANONICAL_TEMP_DIR)

  def test_emcc_caching(self):
    INCLUDING_MESSAGE = 'including X'
    BUILDING_MESSAGE = 'building X for cache'
    ERASING_MESSAGE = 'clearing cache'

    EMCC_CACHE = Cache.dirname

    for compiler in [EMCC, EMXX]:
      print compiler

      restore()

      Cache.erase()
      assert not os.path.exists(EMCC_CACHE)

      try:
        os.environ['EMCC_DEBUG'] ='1'
        self.working_dir = os.path.join(TEMP_DIR, 'emscripten_temp')
        if not os.path.exists(self.working_dir):
          os.mkdir(self.working_dir)

        basebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-0-basebc.bc')
        dcebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-1-linktime.bc')
        ll_names = [os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-X-ll.ll').replace('X', str(x)) for x in range(2,5)]

        # Building a file that *does* need something *should* trigger cache generation, but only the first time
        for filename, libname in [('hello_libcxx.cpp', 'libcxx')]:
          for i in range(3):
            print filename, libname, i
            self.clear()
            try_delete(basebc_name) # we might need to check this file later
            try_delete(dcebc_name) # we might need to check this file later
            for ll_name in ll_names: try_delete(ll_name)
            output = self.do([compiler, '-O' + str(i), '-s', '--llvm-lto', '0', path_from_root('tests', filename), '--save-bc', 'a.bc', '-s', 'DISABLE_EXCEPTION_CATCHING=0'])
            #print '\n\n\n', output
            assert INCLUDING_MESSAGE.replace('X', libname) in output
            if libname == 'libc':
              assert INCLUDING_MESSAGE.replace('X', 'libcxx') not in output # we don't need libcxx in this code
            else:
              assert INCLUDING_MESSAGE.replace('X', 'libc') in output # libcxx always forces inclusion of libc
            assert (BUILDING_MESSAGE.replace('X', libname) in output) == (i == 0), 'Must only build the first time'
            self.assertContained('hello, world!', run_js('a.out.js'))
            assert os.path.exists(EMCC_CACHE)
            full_libname = libname + '.bc' if libname != 'libcxx' else libname + '.a'
            assert os.path.exists(os.path.join(EMCC_CACHE, full_libname))
      finally:
        del os.environ['EMCC_DEBUG']

    restore()

    def ensure_cache():
      self.do([PYTHON, EMCC, '-O2', path_from_root('tests', 'hello_world.c')])

    # Manual cache clearing
    ensure_cache()
    assert os.path.exists(EMCC_CACHE)
    output = self.do([PYTHON, EMCC, '--clear-cache'])
    assert ERASING_MESSAGE in output
    assert not os.path.exists(EMCC_CACHE)
    assert SANITY_MESSAGE in output

    # Changing LLVM_ROOT, even without altering .emscripten, clears the cache
    ensure_cache()
    old = os.environ.get('LLVM')
    try:
      os.environ['LLVM'] = 'waka'
      assert os.path.exists(EMCC_CACHE)
      output = self.do([PYTHON, EMCC])
      assert ERASING_MESSAGE in output
      assert not os.path.exists(EMCC_CACHE)
    finally:
      if old: os.environ['LLVM'] = old
      else: del os.environ['LLVM']

    try_delete(CANONICAL_TEMP_DIR)

  def test_nostdincxx(self):
    restore()
    Cache.erase()

    for compiler in [EMCC, EMXX]:
      print compiler
      out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '-v'], stdout=PIPE, stderr=PIPE).communicate()
      out2, err2 = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '-v', '-nostdinc++'], stdout=PIPE, stderr=PIPE).communicate()
      assert out == out2
      def focus(e):
        assert 'search starts here:' in e, e
        assert e.count('End of search list.') == 1, e
        return e[e.index('search starts here:'):e.index('End of search list.')+20]
      err = focus(err)
      err2 = focus(err2)
      assert err == err2, err + '\n\n\n\n' + err2

  def test_emconfig(self):
    restore()
    
    (fd, custom_config_filename) = tempfile.mkstemp(prefix='.emscripten_config_')

    orig_config = open(CONFIG_FILE, 'r').read()
 
    # Move the ~/.emscripten to a custom location.
    tfile = os.fdopen(fd, "w")
    tfile.write(orig_config)
    tfile.close()

    # Make a syntax error in the original config file so that attempting to access it would fail.
    open(CONFIG_FILE, 'w').write('asdfasdfasdfasdf\n\'\'\'' + orig_config)

    temp_dir = tempfile.mkdtemp(prefix='emscripten_temp_')

    os.chdir(temp_dir)
    self.do([PYTHON, EMCC, '-O2', '--em-config', custom_config_filename, path_from_root('tests', 'hello_world.c')])
    result = run_js('a.out.js')
    
    # Clean up created temp files.
    os.remove(custom_config_filename)
    os.chdir(path_from_root())
    shutil.rmtree(temp_dir)

    self.assertContained('hello, world!', result)

  def test_emcc_ports(self):
    restore()

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

    for compiler in [EMCC, EMXX]:
      print compiler

      for i in [0, 1]:
        print i
        if i == 0:
          try_delete(PORTS_DIR)
        else:
          self.do([PYTHON, compiler, '--clear-ports'])
        assert not os.path.exists(PORTS_DIR)

        # Building a file that doesn't need ports should not trigger anything
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp')])
        assert RETRIEVING_MESSAGE not in output
        assert BUILDING_MESSAGE not in output
        assert not os.path.exists(PORTS_DIR)

        # Building a file that need a port does trigger stuff
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'USE_SDL=2'])
        assert RETRIEVING_MESSAGE in output, output
        assert BUILDING_MESSAGE in output, output
        assert os.path.exists(PORTS_DIR)

        def second_use():
          # Using it again avoids retrieve and build
          output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'USE_SDL=2'])
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
        output = self.do([compiler, path_from_root('tests', 'hello_world_sdl.cpp'), '-s', 'USE_SDL=2'])
        assert RETRIEVING_MESSAGE in output, output
        assert BUILDING_MESSAGE in output, output
        assert os.path.exists(PORTS_DIR)

        second_use()

  def test_native_optimizer(self):
    restore()

    def build():
      return self.check_working([EMCC, '-O2', 'tests/hello_world.c'], 'running js post-opts')

    def test():
      self.assertContained('hello, world!', run_js('a.out.js'))

    try:
      os.environ['EMCC_DEBUG'] = '1'

      # basic usage or lack of usage
      for native in [None, 0, 1]:
        print 'phase 1, part', native
        Cache.erase()
        try:
          if native is not None: os.environ['EMCC_NATIVE_OPTIMIZER'] = str(native)
          output = build()
          assert ('js optimizer using native' in output) == (not not (native or native is None)), output
          test()
          if native or native is None: # None means use the default, which is to use the native optimizer
            assert 'building native optimizer' in output
            # compile again, no rebuild of optimizer
            output = build()
            assert 'building native optimizer' not in output
            assert 'js optimizer using native' in output
            test()
        finally:
          if native is not None: del os.environ['EMCC_NATIVE_OPTIMIZER']

      # force a build failure, see we fall back to non-native

      try:
        for native in [1, 'g']:
          print 'phase 2, part', native
          Cache.erase()
          os.environ['EMCC_NATIVE_OPTIMIZER'] = str(native)

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

      finally:
        del os.environ['EMCC_NATIVE_OPTIMIZER']

    finally:
      del os.environ['EMCC_DEBUG']

  def test_embuilder(self):
    restore()

    tests = []
    if get_llvm_target() == WASM_TARGET:
      tests.append(([PYTHON, 'embuilder.py', 'build', 'wasm_compiler_rt'], ['building and verifying wasm_compiler_rt', 'success'], True, ['wasm_compiler_rt.a']),)
    for command, expected, success, result_libs in tests + [
      ([PYTHON, 'embuilder.py'], ['Emscripten System Builder Tool', 'build libc', 'native_optimizer'], True, []),
      ([PYTHON, 'embuilder.py', 'build', 'waka'], 'ERROR', False, []),
      ([PYTHON, 'embuilder.py', 'build', 'libc'], ['building and verifying libc', 'success'], True, ['libc.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'libc-mt'], ['building and verifying libc-mt', 'success'], True, ['libc-mt.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'dlmalloc'], ['building and verifying dlmalloc', 'success'], True, ['dlmalloc.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'dlmalloc_threadsafe'], ['building and verifying dlmalloc_threadsafe', 'success'], True, ['dlmalloc_threadsafe.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'pthreads'], ['building and verifying pthreads', 'success'], True, ['pthreads.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'libcxx'], ['success'], True, ['libcxx.a']),
      ([PYTHON, 'embuilder.py', 'build', 'libcxx_noexcept'], ['success'], True, ['libcxx_noexcept.a']),
      ([PYTHON, 'embuilder.py', 'build', 'libcxxabi'], ['success'], True, ['libcxxabi.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'gl'], ['success'], True, ['gl.bc']),
      ([PYTHON, 'embuilder.py', 'build', 'native_optimizer'], ['success'], True, ['optimizer.2.exe']),
      ([PYTHON, 'embuilder.py', 'build', 'zlib'], ['building and verifying zlib', 'success'], True, [os.path.join('ports-builds', 'zlib', 'libz.a')]),
      ([PYTHON, 'embuilder.py', 'build', 'libpng'], ['building and verifying libpng', 'success'], True, [os.path.join('ports-builds', 'libpng', 'libpng.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'bullet'], ['building and verifying bullet', 'success'], True, [os.path.join('ports-builds', 'bullet', 'libbullet.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'vorbis'], ['building and verifying vorbis', 'success'], True, [os.path.join('ports-builds', 'vorbis', 'libvorbis.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'ogg'], ['building and verifying ogg', 'success'], True, [os.path.join('ports-builds', 'ogg', 'libogg.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'sdl2'], ['success'], True, [os.path.join('ports-builds', 'sdl2', 'libsdl2.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'sdl2-image'], ['success'], True, [os.path.join('ports-builds', 'sdl2-image', 'libsdl2_image.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'freetype'], ['building and verifying freetype', 'success'], True, [os.path.join('ports-builds', 'freetype', 'libfreetype.a')]),
      ([PYTHON, 'embuilder.py', 'build', 'sdl2-ttf'], ['building and verifying sdl2-ttf', 'success'], True, [os.path.join('ports-builds', 'sdl2-ttf', 'libsdl2_ttf.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'sdl2-net'], ['building and verifying sdl2-net', 'success'], True, [os.path.join('ports-builds', 'sdl2-net', 'libsdl2_net.bc')]),
      ([PYTHON, 'embuilder.py', 'build', 'binaryen'], ['building and verifying binaryen', 'success'], True, []),
    ]:
      print command
      Cache.erase()

      proc = Popen(command, stdout=PIPE, stderr=STDOUT)
      out, err = proc.communicate()
      assert (proc.returncode == 0) == success, out
      if type(expected) == str: expected = [expected]
      for ex in expected:
        print '    seek', ex
        assert ex in out, out
      for lib in result_libs:
        print '    verify', lib
        assert os.path.exists(Cache.get_path(lib))

  def test_d8_path(self):
    """ Test that running JS commands works for node, d8, and jsc and is not path dependent """
    # Fake some JS engines
    restore()

    sample_script = path_from_root('tests', 'print_args.js')

    # Note that the path contains 'd8'.
    test_path = path_from_root('tests', 'fake', 'abcd8765')
    if not os.path.exists(test_path):
      os.makedirs(test_path)

    try:
      os.environ['EM_IGNORE_SANITY'] = '1'
      jsengines = [('d8',     V8_ENGINE),
                   ('d8_g',   V8_ENGINE),
                   ('js',     SPIDERMONKEY_ENGINE),
                   ('node',   NODE_JS),
                   ('nodejs', NODE_JS)]
      for filename, engine in jsengines:
        if type(engine) is list:
          engine = engine[0]
        if engine == '':
            print 'WARNING: Not testing engine %s, not configured.' % (filename)
            continue

        print filename, engine

        test_engine_path = os.path.join(test_path, filename)
        f = open(test_engine_path, 'w')
        f.write('#!/bin/sh\n')
        f.write('%s $@\n' % (engine))
        f.close()
        os.chmod(test_engine_path, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

        try:
          out = jsrun.run_js(sample_script, engine=test_engine_path, args=['--foo'], full_output=True, assert_returncode=0)
        except Exception as e:
          if 'd8' in filename:
            assert False, 'Your d8 version does not correctly parse command-line arguments, please upgrade or delete from ~/.emscripten config file: %s' % (e)
          else:
            assert False, 'Error running script command: %s' % (e)

        self.assertEqual('0: --foo', out.strip())

    finally:
      del os.environ['EM_IGNORE_SANITY']

  def test_wacky_env(self):
    restore()

    def build():
      return self.check_working([EMCC, 'tests/hello_world.c'], '')

    def test():
      self.assertContained('hello, world!', run_js('a.out.js'))

    assert 'EMCC_FORCE_STDLIBS' not in os.environ

    print 'normal build'
    Cache.erase()
    build()
    test()

    try:
      print 'wacky env vars, these should not mess our bootstrapping'
      os.environ['EMCC_FORCE_STDLIBS'] = '1'
      Cache.erase()
      build()
      test()
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

  def test_struct_info(self):
    restore()

    struct_info_file = path_from_root('src', 'struct_info.compiled.json')
    before = open(struct_info_file).read()
    os.remove(struct_info_file)
    self.check_working([EMCC, 'tests/hello_world.c'], '')
    self.assertContained('hello, world!', run_js('a.out.js'))
    assert os.path.exists(struct_info_file), 'removing the struct info file forces a rebuild'
    after = open(struct_info_file).read()
    assert len(after) == len(before), 'struct info must be already valid, recreating it should not alter anything (checking size, since order might change)'

  def test_vanilla(self):
    restore()
    Cache.erase()

    try:
      os.environ['EMCC_DEBUG'] = '1'
      # see that we test vanilla status, and just once
      TESTING = 'testing for asm.js target'
      self.check_working(EMCC, TESTING)
      for i in range(3):
        output = self.check_working(EMCC, 'check tells us to use')
        assert TESTING not in output
      # if env var tells us, do what it says
      os.environ['EMCC_WASM_BACKEND'] = '1'
      self.check_working(EMCC, 'EMCC_WASM_BACKEND tells us to use wasm backend')
      os.environ['EMCC_WASM_BACKEND'] = '0'
      self.check_working(EMCC, 'EMCC_WASM_BACKEND tells us to use asm.js backend')
    finally:
      del os.environ['EMCC_DEBUG']
      if 'EMCC_WASM_BACKEND' in os.environ:
        del os.environ['EMCC_WASM_BACKEND']

    def make_fake(report):
      f = open(CONFIG_FILE, 'a')
      f.write('LLVM_ROOT = "' + path_from_root('tests', 'fake', 'bin') + '"\n')
      f.close()

      f = open(path_from_root('tests', 'fake', 'bin', 'llc'), 'w')
      f.write('#!/bin/sh\n')
      f.write('echo "llc fake output\nRegistered Targets:\n%s"' % report)
      f.close()
      os.chmod(path_from_root('tests', 'fake', 'bin', 'llc'), stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

    try:
      os.environ['EMCC_DEBUG'] = '1'
      os.environ['EMCC_WASM_BACKEND'] = '1'
      make_fake('wasm32-unknown-unknown')
      # see that we request the right backend from llvm
      self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'wasm32-unknown-unknown')
      os.environ['EMCC_WASM_BACKEND'] = '0'
      make_fake('asmjs-unknown-emscripten')
      self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'asmjs-unknown-emscripten')
      del os.environ['EMCC_WASM_BACKEND']
      # check the current installed one is ok
      restore()
      self.check_working(EMCC)
      output = self.check_working(EMCC, 'check tells us to use')
      if 'wasm backend' in output:
        self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'wasm32-unknown-unknown')
      else:
        assert 'asm.js backend' in output
        self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'asmjs-unknown-emscripten')      
    finally:
      del os.environ['EMCC_DEBUG']
      if 'EMCC_WASM_BACKEND' in os.environ:
        del os.environ['EMCC_WASM_BACKEND']

    # fake llc output

    try_delete(path_from_root('tests', 'fake'))
    os.makedirs(path_from_root('tests', 'fake', 'bin'))

    def test_with_fake(report, expected):
      make_fake(report)
      try:
        os.environ['EMCC_DEBUG'] = '1'
        output = self.check_working([EMCC, 'tests/hello_world.c', '-c'], expected)
        self.assertContained('config file changed since we checked vanilla', output)
      finally:
        del os.environ['EMCC_DEBUG']

    test_with_fake('got js backend! JavaScript (asm.js, emscripten) backend', 'check tells us to use asm.js backend')
    test_with_fake('got wasm32 backend! WebAssembly 32-bit',                  'check tells us to use wasm backend')

    # use LLVM env var to modify LLVM between vanilla checks

    assert not os.environ.get('LLVM'), 'we need to modify LLVM env var for this'

    f = open(CONFIG_FILE, 'a')
    f.write('LLVM_ROOT = os.getenv("LLVM") or "' + path_from_root('tests', 'fake1', 'bin') + '"\n')
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

    try:
      os.environ['EMCC_DEBUG'] = '1'
      self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'use asm.js backend')
      os.environ['LLVM'] = path_from_root('tests', 'fake2', 'bin')
      self.check_working([EMCC, 'tests/hello_world.c', '-c'], 'regenerating vanilla check since other llvm')
    finally:
      del os.environ['EMCC_DEBUG']
      if os.environ.get('LLVM'):
        del os.environ['LLVM']

    return # TODO: the rest of this

    # check separate cache dirs are used

    restore()
    self.check_working([EMCC], '')

    root_cache = os.path.expanduser('~/.emscripten_cache')
    if os.path.exists(os.path.join(root_cache, 'asmjs')):
      shutil.rmtree(os.path.join(root_cache, 'asmjs'))
    if os.path.exists(os.path.join(root_cache, 'wasm')):
      shutil.rmtree(os.path.join(root_cache, 'wasm'))

    try:
      os.environ['EMCC_WASM_BACKEND'] = '1'
      self.check_working([EMCC, 'tests/hello_world.c'], '')
      assert os.path.exists(os.path.join(root_cache, 'wasm'))
      os.environ['EMCC_WASM_BACKEND'] = '0'
      self.check_working([EMCC, 'tests/hello_world.c'], '')
      assert os.path.exists(os.path.join(root_cache, 'asmjs'))
      shutil.rmtree(os.path.join(root_cache, 'asmjs'))
      del os.environ['EMCC_WASM_BACKEND']
      self.check_working([EMCC, 'tests/hello_world.c'], '')
      assert os.path.exists(os.path.join(root_cache, 'asmjs'))
    finally:
      del os.environ['EMCC_WASM_BACKEND']

  def test_binaryen(self):
    import tools.ports.binaryen as binaryen
    tag_file = Cache.get_path('binaryen_tag_' + binaryen.TAG + '.txt')
    try_delete(tag_file)
    # if BINARYEN_ROOT is set, we don't build the port. Check we do built it if not
    restore()
    config = open(CONFIG_FILE).read()
    config = config.replace('BINARYEN_ROOT', '#')
    open(CONFIG_FILE, 'w').write(config)
    subprocess.check_call([PYTHON, 'embuilder.py', 'build', 'binaryen'])
    assert os.path.exists(tag_file)
    subprocess.check_call([PYTHON, 'emcc.py', 'tests/hello_world.c', '-s', 'BINARYEN=1'])
    self.assertContained('hello, world!', run_js('a.out.js'))
