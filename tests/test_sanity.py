import os, shutil, stat, subprocess
from runner import RunnerCore, path_from_root
from tools.shared import *

SANITY_FILE = CONFIG_FILE + '_sanity'
commands = [[EMCC], [PYTHON, path_from_root('tests', 'runner.py'), 'blahblah']]

def restore():
  shutil.copyfile(CONFIG_FILE + '_backup', CONFIG_FILE)

def wipe():
  try_delete(CONFIG_FILE)
  try_delete(SANITY_FILE)

def mtime(filename):
  return os.stat(filename).st_mtime

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
      if command[0] == EMCC:
        expected = 'no input files'
      else:
        expected = "No tests found for ['blahblah']"

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
      self.assertContained('PYTHON', output)
      if platform.system() is not 'Windows':
        # os.chmod can't make files executable on Windows
        self.assertIdentical(temp_bin, re.search("^ *LLVM_ROOT *= (.*)$", output, re.M).group(1))
        self.assertIdentical(os.path.join(temp_bin, 'node'), re.search("^ *NODE_JS *= (.*)$", output, re.M).group(1))
        self.assertIdentical(os.path.join(temp_bin, 'python2'), re.search("^ *PYTHON *= (.*)$", output, re.M).group(1))
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
    CLOSURE_FATAL = 'fatal: Closure compiler'
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
    output = self.check_working([EMCC, '-O2', '-s', 'ASM_JS=0', '--closure', '1', 'tests/hello_world.cpp'], CLOSURE_FATAL)

    # With a working path, all is well
    restore()
    try_delete('a.out.js')
    output = self.check_working([EMCC, '-O2', '-s', 'ASM_JS=0', '--closure', '1', 'tests/hello_world.cpp'], '')
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
    SANITY_MESSAGE = 'Emscripten: Running sanity checks'
    SANITY_FAIL_MESSAGE = 'sanity check failed to run'

    # emcc should check sanity if no ${EM_CONFIG}_sanity
    restore()
    time.sleep(0.1)
    assert not os.path.exists(SANITY_FILE) # restore is just the settings, not the sanity
    output = self.check_working(EMCC)
    self.assertContained(SANITY_MESSAGE, output)
    assert os.path.exists(SANITY_FILE) # EMCC should have checked sanity successfully
    assert mtime(SANITY_FILE) >= mtime(CONFIG_FILE)
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

        # Building a file that doesn't need cached stuff should not trigger cache generation
        output = self.do([compiler, path_from_root('tests', 'hello_world.cpp')])
        assert INCLUDING_MESSAGE.replace('X', 'libc') not in output
        assert BUILDING_MESSAGE.replace('X', 'libc') not in output
        self.assertContained('hello, world!', run_js('a.out.js'))
        try_delete('a.out.js')

        basebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-0-basebc.bc')
        dcebc_name = os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-1-linktime.bc')
        ll_names = [os.path.join(TEMP_DIR, 'emscripten_temp', 'emcc-X-ll.ll').replace('X', str(x)) for x in range(2,5)]

        # Building a file that *does* need dlmalloc *should* trigger cache generation, but only the first time
        for filename, libname in [('hello_malloc.cpp', 'libc'), ('hello_libcxx.cpp', 'libcxx')]:
          for i in range(3):
            print filename, libname, i
            self.clear()
            try_delete(basebc_name) # we might need to check this file later
            try_delete(dcebc_name) # we might need to check this file later
            for ll_name in ll_names: try_delete(ll_name)
            output = self.do([compiler, '-O' + str(i), '-s', 'RELOOP=0', '--llvm-lto', '0', path_from_root('tests', filename), '--save-bc', 'a.bc'])
            #print output
            assert INCLUDING_MESSAGE.replace('X', libname) in output
            if libname == 'libc':
              assert INCLUDING_MESSAGE.replace('X', 'libcxx') not in output # we don't need libcxx in this code
            else:
              assert INCLUDING_MESSAGE.replace('X', 'libc') in output # libcxx always forces inclusion of libc
            assert (BUILDING_MESSAGE.replace('X', libname) in output) == (i == 0), 'Must only build the first time'
            self.assertContained('hello, world!', run_js('a.out.js'))
            assert os.path.exists(EMCC_CACHE)
            assert os.path.exists(os.path.join(EMCC_CACHE, libname + '.bc'))
            if libname == 'libcxx':
              print os.stat(os.path.join(EMCC_CACHE, libname + '.bc')).st_size, os.stat(basebc_name).st_size, os.stat(dcebc_name).st_size
              assert os.stat(os.path.join(EMCC_CACHE, libname + '.bc')).st_size > 1000000, 'libc++ is big'
              assert os.stat(basebc_name).st_size > 1000000, 'libc++ is indeed big'
              assert os.stat(dcebc_name).st_size < os.stat(basebc_name).st_size/2, 'Dead code elimination must remove most of libc++'
            # should only have metadata in -O0, not 1 and 2
            if i > 0:
              for ll_name in ll_names:
                ll = None
                try:
                  ll = open(ll_name).read()
                  break
                except:
                  pass
              assert ll
              assert ll.count('\n!') < 10 # a few lines are left even in -O1 and -O2
      finally:
        del os.environ['EMCC_DEBUG']

    restore()

    def ensure_cache():
      self.do([EMCC, '-O2', path_from_root('tests', 'hello_world.c')])

    # Manual cache clearing
    ensure_cache()
    assert os.path.exists(EMCC_CACHE)
    output = self.do([EMCC, '--clear-cache'])
    assert ERASING_MESSAGE in output
    assert not os.path.exists(EMCC_CACHE)

    # Changing LLVM_ROOT, even without altering .emscripten, clears the cache
    ensure_cache()
    old = os.environ.get('LLVM')
    try:
      os.environ['LLVM'] = 'waka'
      assert os.path.exists(EMCC_CACHE)
      output = self.do([EMCC])
      assert ERASING_MESSAGE in output
      assert not os.path.exists(EMCC_CACHE)
    finally:
      if old: os.environ['LLVM'] = old
      else: del os.environ['LLVM']

    try_delete(CANONICAL_TEMP_DIR)

  def test_relooper(self):
    RELOOPER = Cache.get_path('relooper.js')

    restore()
    for phase in range(2): # 0: we wipe the relooper dir. 1: we have it, so should just update
      if phase == 0: Cache.erase()
      try_delete(RELOOPER)

      for i in range(4):
        print >> sys.stderr, phase, i
        opt = min(i, 2)
        try_delete('a.out.js')
        output = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_loop.cpp'), '-O' + str(opt), '-g'],
                       stdout=PIPE, stderr=PIPE).communicate()
        self.assertContained('hello, world!', run_js('a.out.js'))
        output = '\n'.join(output)
        assert ('bootstrapping relooper succeeded' in output) == (i == 1), 'only bootstrap on first O2: ' + output
        assert os.path.exists(RELOOPER) == (i >= 1), 'have relooper on O2: ' + output
        src = open('a.out.js').read()
        main = src.split('function _main()')[1].split('\n}\n')[0]
        assert ('while (1) {' in main or 'while(1){' in main or 'while(1) {' in main or '} while ($' in main or '}while($' in main) == (i >= 1), 'reloop code on O2: ' + main
        assert ('switch' not in main) == (i >= 1), 'reloop code on O2: ' + main

  def test_jcache(self):
    PRE_LOAD_MSG = 'loading pre from jcache'
    PRE_SAVE_MSG = 'saving pre to jcache'
    FUNC_CHUNKS_LOAD_MSG = ' funcchunks from jcache'
    FUNC_CHUNKS_SAVE_MSG = ' funcchunks to jcache'
    JSFUNC_CHUNKS_LOAD_MSG = 'jsfuncchunks from jcache'
    JSFUNC_CHUNKS_SAVE_MSG = 'jsfuncchunks to jcache'

    restore()
    Cache.erase()

    try:
      os.environ['EMCC_DEBUG'] = '1'
      os.environ['EMCC_JSOPT_MIN_CHUNK_SIZE'] = str(1024*512)

      self.working_dir = os.path.join(TEMP_DIR, 'emscripten_temp')
      if not os.path.exists(self.working_dir): os.makedirs(self.working_dir)

      assert not os.path.exists(JCache.get_cachename('emscript_files'))

      srcs = {}
      used_jcache = False

      for args, input_file, expect_pre_save, expect_pre_load, expect_funcs_save, expect_funcs_load, expect_jsfuncs_save, expect_jsfuncs_load, expected in [
        ([], 'hello_world_loop.cpp', False, False, False, False, False, False, []),
        (['--jcache'], 'hello_world_loop.cpp', True, False, True, False, True, False, []),
        (['--jcache'], 'hello_world_loop.cpp', False, True, False, True, False, True, []),
        ([], 'hello_world_loop.cpp', False, False, False, False, False, False, []),
        # new
        ([], 'hello_world.cpp', False, False, False, False, False, False, []),
        (['--jcache'], 'hello_world.cpp', True, False, True, False, True, False, []),
        (['--jcache'], 'hello_world.cpp', False, True, False, True, False, True, []),
        ([], 'hello_world.cpp', False, False, False, False, False, False, []),
        # go back to old file, experience caching
        (['--jcache'], 'hello_world_loop.cpp', False, True, False, True, False, True, []),
        # new, large file
        ([], 'hello_malloc.cpp', False, False, False, False, False, False, []),
        (['--jcache'], 'hello_malloc.cpp', True, False, True, False, True, False, []),
        (['--jcache'], 'hello_malloc.cpp', False, True, False, True, False, True, []),
        ([], 'hello_malloc.cpp', False, False, False, False, False, False, []),
        # new, huge file
        ([], 'hello_libcxx.cpp', False, False, False, False, False, False, ('4 chunks',)),
        (['--jcache'], 'hello_libcxx.cpp', True, False, True, False, True, False, []),
        (['--jcache'], 'hello_libcxx.cpp', False, True, False, True, False, True, []),
        ([], 'hello_libcxx.cpp', False, False, False, False, False, False, []),
        # finally, build a file close to the previous, to see that some chunks are found in the cache and some not
        (['--jcache'], 'hello_libcxx_mod1.cpp', False, True, True, True, True, True, []), # win on pre, mix on funcs, mix on jsfuncs
        (['--jcache'], 'hello_libcxx_mod1.cpp', False, True, False, True, False, True, []),
        (None, None, None, None, None, None, None, None, None), # clear
        (['--jcache'], 'hello_libcxx_mod2.cpp', True, False, True, False, True, False, []), # load into cache
        (['--jcache'], 'hello_libcxx_mod2a.cpp', False, True, True, True, True, True, []) # add a printf, do not lose everything
      ]:
        self.clear()
        if args is None:
          Cache.erase()
          continue

        print >> sys.stderr, args, input_file, expect_pre_save, expect_pre_load, expect_funcs_save, expect_funcs_load, expect_jsfuncs_save, expect_jsfuncs_load, expected

        out, err = Popen([PYTHON, EMCC, '-O2', '-g', path_from_root('tests', input_file)] + args, stdout=PIPE, stderr=PIPE).communicate()
        errtail = err.split('emcc invocation')[-1]
        self.assertContained('hello, world!', run_js('a.out.js'), errtail)
        assert (PRE_SAVE_MSG in err) == expect_pre_save, errtail
        assert (PRE_LOAD_MSG in err) == expect_pre_load, errtail
        assert (FUNC_CHUNKS_SAVE_MSG in err) == expect_funcs_save, errtail
        assert (FUNC_CHUNKS_LOAD_MSG in err) == expect_funcs_load, errtail
        assert (JSFUNC_CHUNKS_SAVE_MSG in err) == expect_jsfuncs_save, errtail
        assert (JSFUNC_CHUNKS_LOAD_MSG in err) == expect_jsfuncs_load, errtail
        for expect in expected: assert expect in err, expect + ' ? ' + errtail
        curr = open('a.out.js').read()
        if input_file not in srcs:
          srcs[input_file] = curr
        else:
          #open('/home/alon/Dev/emscripten/a', 'w').write(srcs[input_file])
          #open('/home/alon/Dev/emscripten/b', 'w').write(curr)
          assert abs(len(curr)/float(len(srcs[input_file]))-1)<0.01, 'contents may shift in order, but must remain the same size  %d vs %d' % (len(curr), len(srcs[input_file])) + '\n' + errtail
        used_jcache = used_jcache or ('--jcache' in args)
        assert used_jcache == os.path.exists(JCache.get_cachename('emscript_files'))
        #print >> sys.stderr, errtail

    finally:
      del os.environ['EMCC_DEBUG']
      del os.environ['EMCC_JSOPT_MIN_CHUNK_SIZE']

  def test_nostdincxx(self):
    restore()
    Cache.erase()

    try:
      old = os.environ.get('EMCC_LLVM_TARGET') or ''
      for compiler in [EMCC, EMXX]:
        for target in ['i386-pc-linux-gnu', 'le32-unknown-nacl']:
          print compiler, target
          os.environ['EMCC_LLVM_TARGET'] = target
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
    finally:
      if old:
        os.environ['EMCC_LLVM_TARGET'] = old

