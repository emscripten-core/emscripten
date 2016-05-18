# coding=utf-8

import multiprocessing, os, pipes, re, shutil, subprocess, sys
import glob
import tools.shared
from tools.shared import *
from runner import RunnerCore, path_from_root, get_zlib_library, get_bullet_library

class other(RunnerCore):
  def test_emcc(self):
    for compiler in [EMCC, EMXX]:
      shortcompiler = os.path.basename(compiler)
      suffix = '.c' if compiler == EMCC else '.cpp'

      # --version
      output = Popen([PYTHON, compiler, '--version'], stdout=PIPE, stderr=PIPE).communicate()
      output = output[0].replace('\r', '')
      self.assertContained('''emcc (Emscripten gcc/clang-like replacement)''', output)
      self.assertContained('''Copyright (C) 2014 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''', output)

      # -v, without input files
      output = Popen([PYTHON, compiler, '-v'], stdout=PIPE, stderr=PIPE).communicate()
      self.assertContained('''clang version %s.0 ''' % '.'.join(map(str, EXPECTED_LLVM_VERSION)), output[1].replace('\r', ''), output[1].replace('\r', ''))
      self.assertContained('''GNU''', output[0])
      self.assertNotContained('this is dangerous', output[0])
      self.assertNotContained('this is dangerous', output[1])

      # --help
      output = Popen([PYTHON, compiler, '--help'], stdout=PIPE, stderr=PIPE).communicate()
      self.assertContained('Display this information', output[0])
      self.assertContained('Most clang options will work', output[0])

      # -dumpmachine
      output = Popen([PYTHON, compiler, '-dumpmachine'], stdout=PIPE, stderr=PIPE).communicate()
      self.assertContained(get_llvm_target(), output[0])

      # emcc src.cpp ==> writes a.out.js
      self.clear()
      output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world' + suffix)], stdout=PIPE, stderr=PIPE).communicate()
      assert len(output[0]) == 0, output[0]
      assert os.path.exists('a.out.js'), '\n'.join(output)
      self.assertContained('hello, world!', run_js('a.out.js'))

      # properly report source code errors, and stop there
      self.clear()
      assert not os.path.exists('a.out.js')
      process = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world_error' + suffix)], stdout=PIPE, stderr=PIPE)
      output = process.communicate()
      assert not os.path.exists('a.out.js'), 'compilation failed, so no output file is expected'
      assert len(output[0]) == 0, output[0]
      assert process.returncode is not 0, 'Failed compilation must return a nonzero error code!'
      self.assertNotContained('IOError', output[1]) # no python stack
      self.assertNotContained('Traceback', output[1]) # no python stack
      self.assertContained('error: invalid preprocessing directive', output[1])
      self.assertContained(["error: use of undeclared identifier 'cheez", "error: unknown type name 'cheez'"], output[1])
      self.assertContained('errors generated', output[1])
      assert 'compiler frontend failed to generate LLVM bitcode, halting' in output[1].split('errors generated.')[1]

      # emcc src.cpp -c    and   emcc src.cpp -o src.[o|bc] ==> should give a .bc file
      #      regression check: -o js should create "js", with bitcode content
      for args in [['-c'], ['-o', 'src.o'], ['-o', 'src.bc'], ['-o', 'src.so'], ['-o', 'js'], ['-O1', '-c', '-o', '/dev/null'], ['-O1', '-o', '/dev/null']]:
        print '-c stuff', args
        if '/dev/null' in args and WINDOWS:
          print 'skip because windows'
          continue
        target = args[1] if len(args) == 2 else 'hello_world.o'
        self.clear()
        proc = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world' + suffix)] + args, stdout=PIPE, stderr=PIPE)
        proc.communicate()
        assert proc.returncode == 0, [proc.returncode, args]
        if args[-1] == '/dev/null':
          print '(no output)'
          continue
        syms = Building.llvm_nm(target)
        assert len(syms.defs) == 1 and 'main' in syms.defs, 'Failed to generate valid bitcode'
        if target == 'js': # make sure emcc can recognize the target as a bitcode file
          shutil.move(target, target + '.bc')
          target += '.bc'
        output = Popen([PYTHON, compiler, target, '-o', target + '.js'], stdout = PIPE, stderr = PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists(target + '.js'), 'Expected %s to exist since args are %s : %s' % (target + '.js', str(args), '\n'.join(output))
        self.assertContained('hello, world!', run_js(target + '.js'))

      # handle singleton archives
      self.clear()
      Popen([PYTHON, compiler, path_from_root('tests', 'hello_world' + suffix), '-o', 'a.bc'], stdout=PIPE, stderr=PIPE).communicate()
      Popen([LLVM_AR, 'r', 'a.a', 'a.bc'], stdout=PIPE, stderr=PIPE).communicate()
      assert os.path.exists('a.a')
      output = Popen([PYTHON, compiler, 'a.a']).communicate()
      assert os.path.exists('a.out.js'), output
      self.assertContained('hello, world!', run_js('a.out.js'))

      # emcc src.ll ==> generates .js
      self.clear()
      output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world.ll')], stdout=PIPE, stderr=PIPE).communicate()
      assert len(output[0]) == 0, output[0]
      assert os.path.exists('a.out.js'), '\n'.join(output)
      self.assertContained('hello, world!', run_js('a.out.js'))

      # emcc [..] -o [path] ==> should work with absolute paths
      try:
        for path in [os.path.abspath(os.path.join('..', 'file1.js')), os.path.join('b_dir', 'file2.js')]:
          print path
          self.clear(in_curr=True)
          os.chdir(self.get_dir())
          if not os.path.exists('a_dir'): os.mkdir('a_dir')
          os.chdir('a_dir')
          if not os.path.exists('b_dir'): os.mkdir('b_dir')
          output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world.ll'), '-o', path], stdout=PIPE, stderr=PIPE).communicate()
          print output
          assert os.path.exists(path), path + ' does not exist; ' + '\n'.join(output)
          last = os.getcwd()
          os.chdir(os.path.dirname(path))
          self.assertContained('hello, world!', run_js(os.path.basename(path)))
          os.chdir(last)
      finally:
        os.chdir(self.get_dir())
      self.clear()

      # Optimization: emcc src.cpp -o something.js [-Ox]. -O0 is the same as not specifying any optimization setting
      for params, opt_level, bc_params, closure, has_malloc in [ # bc params are used after compiling to bitcode
        (['-o', 'something.js'],                          0, None, 0, 1),
        (['-o', 'something.js', '-O0'],                   0, None, 0, 0),
        (['-o', 'something.js', '-O1'],                   1, None, 0, 0),
        (['-o', 'something.js', '-O1', '-g'],             1, None, 0, 0), # no closure since debug
        (['-o', 'something.js', '-O2'],                   2, None, 0, 1),
        (['-o', 'something.js', '-O2', '-g'],             2, None, 0, 0),
        (['-o', 'something.js', '-Os'],                   2, None, 0, 1),
        (['-o', 'something.js', '-O3'],                   3, None, 0, 1),
        # and, test compiling to bitcode first
        (['-o', 'something.bc'], 0, [],      0, 0),
        (['-o', 'something.bc', '-O0'], 0, [], 0, 0),
        (['-o', 'something.bc', '-O1'], 1, ['-O1'], 0, 0),
        (['-o', 'something.bc', '-O2'], 2, ['-O2'], 0, 0),
        (['-o', 'something.bc', '-O3'], 3, ['-O3'], 0, 0),
        (['-O1', '-o', 'something.bc'], 1, [], 0, 0),
      ]:
        print params, opt_level, bc_params, closure, has_malloc
        self.clear()
        keep_debug = '-g' in params
        args = [PYTHON, compiler, path_from_root('tests', 'hello_world_loop' + ('_malloc' if has_malloc else '') + '.cpp')] + params
        print '..', args
        output = Popen(args,
                       stdout=PIPE, stderr=PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        if bc_params is not None:
          assert os.path.exists('something.bc'), output[1]
          bc_args = [PYTHON, compiler, 'something.bc', '-o', 'something.js'] + bc_params
          print '....', bc_args
          output = Popen(bc_args, stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists('something.js'), output[1]
        self.assertContained('hello, world!', run_js('something.js'))

        # Verify optimization level etc. in the generated code
        # XXX these are quite sensitive, and will need updating when code generation changes
        generated = open('something.js').read()
        main = self.get_func(generated, '_main') if 'function _main' in generated else generated
        assert 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 should be used by default'
        assert 'SAFE_HEAP' not in generated, 'safe heap should not be used by default'
        assert ': while(' not in main, 'when relooping we also js-optimize, so there should be no labelled whiles'
        if closure:
          if opt_level == 0: assert '._main =' in generated, 'closure compiler should have been run'
          elif opt_level >= 1: assert '._main=' in generated, 'closure compiler should have been run (and output should be minified)'
        else:
          # closure has not been run, we can do some additional checks. TODO: figure out how to do these even with closure
          assert '._main = ' not in generated, 'closure compiler should not have been run'
          if keep_debug:
            assert ('switch (label)' in generated or 'switch (label | 0)' in generated) == (opt_level <= 0), 'relooping should be in opt >= 1'
            assert ('assert(STACKTOP < STACK_MAX' in generated) == (opt_level == 0), 'assertions should be in opt == 0'
            assert '$i' in generated or '$storemerge' in generated or '$original' in generated, 'micro opts should always be on'
          if opt_level >= 2 and '-g' in params:
            assert re.search('HEAP8\[\$?\w+ ?\+ ?\(+\$?\w+ ?', generated) or re.search('HEAP8\[HEAP32\[', generated) or re.search('[i$]\d+ & ~\(1 << [i$]\d+\)', generated), 'eliminator should create compound expressions, and fewer one-time vars' # also in -O1, but easier to test in -O2
          if opt_level == 0 or '-g' in params: assert 'function _main() {' in generated or 'function _main(){' in generated, 'Should be unminified'
          elif opt_level >= 2: assert ('function _main(){' in generated or '"use asm";var a=' in generated), 'Should be whitespace-minified'

      # emcc -s INLINING_LIMIT=0 src.cpp ==> should pass -s to emscripten.py.
      for params, test, text in [
        (['-O2'], lambda generated: 'function intArrayToString' in generated, 'shell has unminified utilities'),
        (['-O2', '--closure', '1'], lambda generated: 'function intArrayToString' not in generated and ';function' in generated, 'closure minifies the shell, removes whitespace'),
        (['-O2', '--closure', '1', '-g1'], lambda generated: 'function intArrayToString' not in generated and ';function' not in generated, 'closure minifies the shell, -g1 makes it keep whitespace'),
        (['-O2'], lambda generated: 'var b=0' in generated and not 'function _main' in generated, 'registerize/minify is run by default in -O2'),
        (['-O2', '--minify', '0'], lambda generated: 'var b = 0' in generated and not 'function _main' in generated, 'minify is cancelled, but not registerize'),
        (['-O2', '--js-opts', '0'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'js opts are cancelled'),
        (['-O2', '-g'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'registerize/minify is cancelled by -g'),
        (['-O2', '-g0'], lambda generated: 'var b=0'   in generated and not 'function _main' in generated, 'registerize/minify is run by default in -O2 -g0'),
        (['-O2', '-g1'], lambda generated: 'var b = 0' in generated and not 'function _main' in generated, 'compress is cancelled by -g1'),
        (['-O2', '-g2'], lambda generated: ('var b = 0' in generated or 'var i1 = 0' in generated) and 'function _main' in generated, 'minify is cancelled by -g2'),
        (['-O2', '-g3'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'registerize is cancelled by -g3'),
        (['-O2', '--profiling'], lambda generated: ('var b = 0' in generated or 'var i1 = 0' in generated) and 'function _main' in generated, 'similar to -g2'),
        (['-O2', '-profiling'],  lambda generated: ('var b = 0' in generated or 'var i1 = 0' in generated) and 'function _main' in generated, 'similar to -g2'),
        (['-O2', '--profiling-funcs'], lambda generated: 'var b=0' in generated and '"use asm";var a=' in generated and 'function _main' in generated, 'very minified, but retain function names'),
        (['-O2', '-profiling-funcs'],  lambda generated: 'var b=0' in generated and '"use asm";var a=' in generated and 'function _main' in generated, 'very minified, but retain function names'),
        (['-O2'],                      lambda generated: 'var b=0' in generated and '"use asm";var a=' in generated and 'function _main' not in generated, 'very minified, no function names'),
        #(['-O2', '-g4'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'same as -g3 for now'),
        (['-s', 'INLINING_LIMIT=0'], lambda generated: 'function _dump' in generated, 'no inlining without opts'),
        ([], lambda generated: 'Module["_dump"]' not in generated, 'dump is not exported by default'),
        (['-s', 'EXPORTED_FUNCTIONS=["_main", "_dump"]'], lambda generated: 'Module["_dump"]' in generated, 'dump is now exported'),
        (['--llvm-opts', '1'], lambda generated: '_puts(' in generated, 'llvm opts requested'),
        ([], lambda generated: '// The Module object' in generated, 'without opts, comments in shell code'),
        (['-O2'], lambda generated: '// The Module object' not in generated, 'with opts, no comments in shell code'),
        (['-O2', '-g2'], lambda generated: '// The Module object' not in generated, 'with -g2, no comments in shell code'),
        (['-O2', '-g3'], lambda generated: '// The Module object' in generated, 'with -g3, yes comments in shell code'),
      ]:
        print params, text
        self.clear()
        output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world_loop.cpp'), '-o', 'a.out.js'] + params, stdout=PIPE, stderr=PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists('a.out.js'), '\n'.join(output)
        self.assertContained('hello, world!', run_js('a.out.js'))
        assert test(open('a.out.js').read()), text

      # Compiling two source files into a final JS.
      for args, target in [([], 'a.out.js'), (['-o', 'combined.js'], 'combined.js')]:
        self.clear()
        output = Popen([PYTHON, compiler, path_from_root('tests', 'twopart_main.cpp'), path_from_root('tests', 'twopart_side.cpp')] + args,
                       stdout=PIPE, stderr=PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists(target), '\n'.join(output)
        self.assertContained('side got: hello from main, over', run_js(target))

        # Compiling two files with -c will generate separate .bc files
        self.clear()
        output = Popen([PYTHON, compiler, path_from_root('tests', 'twopart_main.cpp'), path_from_root('tests', 'twopart_side.cpp'), '-c'] + args,
                       stdout=PIPE, stderr=PIPE).communicate()
        if '-o' in args:
          # specifying -o and -c is an error
          assert 'fatal error' in output[1], output[1]
          continue

        assert os.path.exists('twopart_main.o'), '\n'.join(output)
        assert os.path.exists('twopart_side.o'), '\n'.join(output)
        assert not os.path.exists(target), 'We should only have created bitcode here: ' + '\n'.join(output)

        # Compiling one of them alone is expected to fail
        output = Popen([PYTHON, compiler, 'twopart_main.o', '-O1', '-g'] + args, stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists(target), '\n'.join(output)
        #print '\n'.join(output)
        self.assertContained('missing function', run_js(target, stderr=STDOUT, assert_returncode=None))
        try_delete(target)

        # Combining those bc files into js should work
        output = Popen([PYTHON, compiler, 'twopart_main.o', 'twopart_side.o'] + args, stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists(target), '\n'.join(output)
        self.assertContained('side got: hello from main, over', run_js(target))

        # Combining bc files into another bc should also work
        try_delete(target)
        assert not os.path.exists(target)
        output = Popen([PYTHON, compiler, 'twopart_main.o', 'twopart_side.o', '-o', 'combined.bc'] + args, stdout=PIPE, stderr=PIPE).communicate()
        syms = Building.llvm_nm('combined.bc')
        assert len(syms.defs) == 2 and 'main' in syms.defs, 'Failed to generate valid bitcode'
        output = Popen([PYTHON, compiler, 'combined.bc', '-o', 'combined.bc.js'], stdout = PIPE, stderr = PIPE).communicate()
        assert len(output[0]) == 0, output[0]
        assert os.path.exists('combined.bc.js'), 'Expected %s to exist' % ('combined.bc.js')
        self.assertContained('side got: hello from main, over', run_js('combined.bc.js'))

      # --js-transform <transform>
      self.clear()
      trans = os.path.join(self.get_dir(), 't.py')
      trans_file = open(trans, 'w')
      trans_file.write('''
import sys
f = open(sys.argv[1], 'w')
f.write('transformed!')
f.close()
''')
      trans_file.close()
      output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world' + suffix), '--js-transform', '%s t.py' % (PYTHON)], stdout=PIPE, stderr=PIPE).communicate()
      assert open('a.out.js').read() == 'transformed!', 'Transformed output must be as expected'

      for opts in [0, 1, 2, 3]:
        print 'mem init in', opts
        self.clear()
        output = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world.c'), '-O' + str(opts)], stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists('a.out.js.mem') == (opts >= 2), 'mem file should exist in -O2+'

    # TODO: Add in files test a clear example of using disablePermissions, and link to it from the wiki
    # TODO: test normal project linking, static and dynamic: get_library should not need to be told what to link!
    # TODO: deprecate llvm optimizations, dlmalloc, etc. in emscripten.py.

  def test_emcc_nonfastcomp_fails(self):
    open(os.path.join(self.get_dir(), 'test.c'), 'w').write(r'''
      int main() {
        return 0;
      }
    ''')
    def check_errors(command):
      process = Popen(command, stdout=PIPE, stderr=PIPE)
      stdout, stderr = process.communicate()
      self.assertIn('Non-fastcomp compiler is no longer available', stderr)
      self.assertEqual(process.returncode, 1)
    def check_success(command):
      process = Popen(command, stdout=PIPE, stderr=PIPE)
      stdout, stderr = process.communicate()
      self.assertEqual(stderr, '')
      self.assertEqual(process.returncode, 0)
    try:
      old_fastcomp = os.environ.get('EMCC_FAST_COMPILER')
      os.environ['EMCC_FAST_COMPILER'] = '0'
      check_success([PYTHON, EMCC, '--version'])
      check_success([PYTHON, EMCC, '--help'])
      check_errors([PYTHON, EMCC, '-v'])
      check_errors([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.c')])
    finally:
      if old_fastcomp is None:
        del os.environ['EMCC_FAST_COMPILER']
      else:
        os.environ['EMCC_FAST_COMPILER'] = old_fastcomp
    self.assertFalse(os.path.exists('a.out.js'))

  def test_emcc_cache_flag(self):
    tempdirname = tempfile.mkdtemp(prefix='emscripten_test_emcache_', dir=TEMP_DIR)
    try:
      os.chdir(tempdirname)
      c_file = os.path.join(tempdirname, 'test.c')
      cache_dir_name = os.path.join(tempdirname, 'emscripten_cache')
      assert os.path.exists(cache_dir_name) == False, 'The cache directory %s must not already exist' % cache_dir_name
      open(c_file, 'w').write(r'''
        #include <stdio.h>
        int main() {
          printf("hello, world!\n");
          return 0;
        }
        ''')
      subprocess.check_call([PYTHON, EMCC, c_file, '--cache', cache_dir_name])
      assert os.path.exists(cache_dir_name), 'The cache directory %s must exist after the build' % cache_dir_name
      assert os.path.exists(os.path.join(cache_dir_name, 'asmjs', 'libc.bc')), 'The cache directory must contain a built libc'
    finally:
      os.chdir(path_from_root('tests')) # Move away from the directory we are about to remove.
      shutil.rmtree(tempdirname)

  def test_emar_em_config_flag(self):
    # We expand this in case the EM_CONFIG is ~/.emscripten (default)
    config = os.path.expanduser(EM_CONFIG)
    # We pass -version twice to work around the newargs > 2 check in emar
    (out, err) = Popen([PYTHON, EMAR, '--em-config', config, '-version', '-version'], stdout=PIPE, stderr=PIPE).communicate()
    assert out
    assert not err
    self.assertContained('LLVM', out)

  def test_cmake(self):
    # Test all supported generators.
    if WINDOWS:
      generators = ['MinGW Makefiles', 'NMake Makefiles']
    else:
      generators = ['Unix Makefiles', 'Ninja', 'Eclipse CDT4 - Ninja']

    def nmake_detect_error(configuration):
      if Building.which(configuration['build'][0]):
        return None
      else:
        return 'Skipping NMake test for CMake support, since nmake was not found in PATH. Run this test in Visual Studio command prompt to easily access nmake.'

    def check_makefile(configuration, dirname):
      assert os.path.exists(dirname + '/Makefile'), 'CMake call did not produce a Makefile!'

    configurations = { 'MinGW Makefiles'     : { 'prebuild': check_makefile,
                                                 'build'   : ['mingw32-make'],

                       },
                       'NMake Makefiles'     : { 'detect'  : nmake_detect_error,
                                                 'prebuild': check_makefile,
                                                 'build'   : ['nmake', '/NOLOGO'],
                       },
                       'Unix Makefiles'      : { 'prebuild': check_makefile,
                                                 'build'   : ['make'],
                       },
                       'Ninja'               : { 'build'   : ['ninja'],
                       },
                       'Eclipse CDT4 - Ninja': { 'build'   : ['ninja'],
                       }
    }

    if os.name == 'nt':
      emconfigure = path_from_root('emconfigure.bat')
    else:
      emconfigure = path_from_root('emconfigure')

    for generator in generators:
      conf = configurations[generator]

      make = conf['build']

      try:
        detector = conf['detect']
      except KeyError:
        detector = None

      if detector:
        error = detector(conf)
      elif len(make) == 1 and not Building.which(make[0]):
        # Use simple test if applicable
        error = 'Skipping %s test for CMake support, since it could not be detected.' % generator
      else:
        error = None

      if error:
        logging.warning(error)
        continue

      try:
        prebuild = conf['prebuild']
      except KeyError:
        prebuild = None

      try:
        postbuild = conf['postbuild']
      except KeyError:
        postbuild = None

      cmake_cases = ['target_js', 'target_html', 'target_library', 'target_library']
      cmake_outputs = ['test_cmake.js', 'hello_world_gles.html', 'libtest_cmake.a', 'libtest_cmake.so']
      cmake_arguments = ['', '', '-DBUILD_SHARED_LIBS=OFF', '-DBUILD_SHARED_LIBS=ON']
      for i in range(0, len(cmake_cases)):
        for configuration in ['Debug', 'Release']:
          # CMake can be invoked in two ways, using 'emconfigure cmake', or by directly running 'cmake'.
          # Test both methods.
          for invoke_method in ['cmake', 'emconfigure']:
            for cpp_lib_type in ['STATIC', 'SHARED']:
              # Create a temp workspace folder
              cmakelistsdir = path_from_root('tests', 'cmake', cmake_cases[i])
              tempdirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
              try:
                os.chdir(tempdirname)

                # Run Cmake
                if invoke_method == 'cmake':
                  # Test invoking cmake directly.
                  cmd = ['cmake', '-DCMAKE_TOOLCHAIN_FILE='+path_from_root('cmake', 'Modules', 'Platform', 'Emscripten.cmake'),
                                  '-DCPP_LIBRARY_TYPE='+cpp_lib_type,
                                  '-DCMAKE_CROSSCOMPILING_EMULATOR="' + Building.which(NODE_JS[0] if type(NODE_JS) is list else NODE_JS) + '"',
                                  '-DCMAKE_BUILD_TYPE=' + configuration, cmake_arguments[i], '-G', generator, cmakelistsdir]
                  env = tools.shared.Building.remove_sh_exe_from_path(os.environ)
                else:
                  # Test invoking via 'emconfigure cmake'
                  cmd = [emconfigure, 'cmake', '-DCMAKE_BUILD_TYPE=' + configuration, cmake_arguments[i], '-G', generator, cmakelistsdir]
                  env = os.environ.copy()

                print str(cmd)
                ret = Popen(cmd, stdout=None if EM_BUILD_VERBOSE_LEVEL >= 2 else PIPE, stderr=None if EM_BUILD_VERBOSE_LEVEL >= 1 else PIPE, env=env).communicate()
                if len(ret) > 1 and ret[1] != None and len(ret[1].strip()) > 0:
                  logging.error(ret[1]) # If there were any errors, print them directly to console for diagnostics.
                if len(ret) > 1 and ret[1] != None and 'error' in ret[1].lower():
                  logging.error('Failed command: ' + ' '.join(cmd))
                  logging.error('Result:\n' + ret[1])
                  raise Exception('cmake call failed!')

                if prebuild:
                  prebuild(configuration, tempdirname)

                # Build
                cmd = make
                if EM_BUILD_VERBOSE_LEVEL >= 3 and 'Ninja' not in generator:
                  cmd += ['VERBOSE=1']
                ret = Popen(cmd, stdout=None if EM_BUILD_VERBOSE_LEVEL >= 2 else PIPE).communicate()
                if len(ret) > 1 and ret[1] != None and len(ret[1].strip()) > 0:
                  logging.error(ret[1]) # If there were any errors, print them directly to console for diagnostics.
                if len(ret) > 0 and ret[0] != None and 'error' in ret[0].lower() and not '0 error(s)' in ret[0].lower():
                  logging.error('Failed command: ' + ' '.join(cmd))
                  logging.error('Result:\n' + ret[0])
                  raise Exception('make failed!')
                assert os.path.exists(tempdirname + '/' + cmake_outputs[i]), 'Building a cmake-generated Makefile failed to produce an output file %s!' % tempdirname + '/' + cmake_outputs[i]

                if postbuild:
                  postbuild(configuration, tempdirname)

                # Run through node, if CMake produced a .js file.
                if cmake_outputs[i].endswith('.js'):
                  ret = Popen(NODE_JS + [tempdirname + '/' + cmake_outputs[i]], stdout=PIPE).communicate()[0]
                  self.assertTextDataIdentical(open(cmakelistsdir + '/out.txt', 'r').read().strip(), ret.strip())
              finally:
                os.chdir(path_from_root('tests')) # Move away from the directory we are about to remove.
                #there is a race condition under windows here causing an exception in shutil.rmtree because the directory is not empty yet
                try:
                  shutil.rmtree(tempdirname)
                except:
                  time.sleep(0.1)
                  shutil.rmtree(tempdirname)

  def test_failure_error_code(self):
    for compiler in [EMCC, EMXX]:
      # Test that if one file is missing from the build, then emcc shouldn't succeed, and shouldn't try to produce an output file.
      process = Popen([PYTHON, compiler, path_from_root('tests', 'hello_world.c'), 'this_file_is_missing.c', '-o', 'this_output_file_should_never_exist.js'], stdout=PIPE, stderr=PIPE)
      process.communicate()
      assert process.returncode is not 0, 'Trying to compile a nonexisting file should return with a nonzero error code!'
      assert os.path.exists('this_output_file_should_never_exist.js') == False, 'Emcc should not produce an output file when build fails!'

  def test_cxx03(self):
    for compiler in [EMCC, EMXX]:
      process = Popen([PYTHON, compiler, path_from_root('tests', 'hello_cxx03.cpp')], stdout=PIPE, stderr=PIPE)
      process.communicate()
      assert process.returncode is 0, 'By default, emscripten should build using -std=c++03!'

  def test_cxx11(self):
    for std in ['-std=c++11', '--std=c++11']:
      for compiler in [EMCC, EMXX]:
        process = Popen([PYTHON, compiler, std, path_from_root('tests', 'hello_cxx11.cpp')], stdout=PIPE, stderr=PIPE)
        process.communicate()
        assert process.returncode is 0, 'User should be able to specify custom -std= on the command line!'

  def test_odd_suffixes(self):
    for suffix in ['CPP', 'c++', 'C++', 'cxx', 'CXX', 'cc', 'CC', 'i', 'ii']:
      self.clear()
      print suffix
      shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'test.' + suffix)
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.' + suffix)]).communicate()
      self.assertContained('hello, world!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    for suffix in ['lo']:
      self.clear()
      print suffix
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-o', 'binary.' + suffix]).communicate()
      Popen([PYTHON, EMCC, 'binary.' + suffix]).communicate()
      self.assertContained('hello, world!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_catch_undef(self):
    open(os.path.join(self.get_dir(), 'test.cpp'), 'w').write(r'''
      #include <vector>
      #include <stdio.h>

      class Test {
      public:
        std::vector<int> vector;
      };

      Test globalInstance;

      int main() {
        printf("hello, world!\n");
        return 0;
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.cpp'), '-fsanitize=undefined']).communicate()
    self.assertContained('hello, world!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  # TODO: test only worked in non-fastcomp
  def test_unaligned_memory(self):
    return self.skip('non-fastcomp is deprecated and fails in 3.5')
    open(os.path.join(self.get_dir(), 'test.cpp'), 'w').write(r'''
      #include <stdio.h>
      #include <stdarg.h>

      typedef unsigned char   Bit8u;
      typedef unsigned short  Bit16u;
      typedef unsigned int    Bit32u;

      int main()
      {
        va_list argp;
        va_arg(argp, char *); // check for compilation error, #1705

        Bit8u data[4] = {0x01,0x23,0x45,0x67};

        printf("data: %x\n", *(Bit32u*)data);
        printf("data[0,1] 16bit: %x\n", *(Bit16u*)data);
        printf("data[1,2] 16bit: %x\n", *(Bit16u*)(data+1));
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.cpp'), '-s', 'UNALIGNED_MEMORY=1']).communicate()
    self.assertContained('data: 67452301\ndata[0,1] 16bit: 2301\ndata[1,2] 16bit: 4523', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  # TODO: test only worked in non-fastcomp
  def test_unaligned_memory_2(self):
    return self.skip('non-fastcomp is deprecated and fails in 3.5')
    open(os.path.join(self.get_dir(), 'test.cpp'), 'w').write(r'''
      #include <string>
      #include <stdio.h>

      int main( int argc, char ** argv )
      {
          std::string testString( "Hello, World!" );

          printf( "testString = %s\n", testString.c_str() );
          return 0;
      }
      ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.cpp'), '-s', 'UNALIGNED_MEMORY=1']).communicate()
    self.assertContained('testString = Hello, World!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_asm_minify(self):
    def test(args):
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_loop_malloc.cpp')] + args).communicate()
      self.assertContained('hello, world!', run_js(self.in_dir('a.out.js')))
      return open(self.in_dir('a.out.js')).read()

    src = test([])
    assert 'function _malloc' in src

    src = test(['-O2', '-s', 'ASM_JS=1'])
    normal_size = len(src)
    print 'normal', normal_size
    assert 'function _malloc' not in src

    src = test(['-O2', '-s', 'ASM_JS=1', '--minify', '0'])
    unminified_size = len(src)
    print 'unminified', unminified_size
    assert unminified_size > normal_size
    assert 'function _malloc' not in src

    src = test(['-O2', '-s', 'ASM_JS=1', '-g'])
    debug_size = len(src)
    print 'debug', debug_size
    assert debug_size > unminified_size
    assert 'function _malloc' in src

  def test_dangerous_func_cast(self):
    src = r'''
      #include <stdio.h>
      typedef void (*voidfunc)();
      int my_func() {
        printf("my func\n");
        return 10;
      }
      int main(int argc, char **argv) {
        voidfunc fps[10];
        for (int i = 0; i < 10; i++) fps[i] = (i == argc) ? (void (*)())my_func : NULL;
        fps[2*(argc-1) + 1]();
        return 0;
      }
    '''
    open('src.c', 'w').write(src)
    def test(args, expected, moar_expected=None):
      print args, expected, moar_expected
      out, err = Popen([PYTHON, EMCC, 'src.c'] + args, stderr=PIPE).communicate()
      self.assertContained(expected, run_js(self.in_dir('a.out.js'), stderr=PIPE, full_output=True, assert_returncode=None))
      print 'with emulated function pointers'
      Popen([PYTHON, EMCC, 'src.c'] + args + ['-s', 'EMULATED_FUNCTION_POINTERS=1'], stderr=PIPE).communicate()
      out = run_js(self.in_dir('a.out.js'), stderr=PIPE, full_output=True, assert_returncode=None)
      self.assertContained(expected, out)
      if moar_expected: self.assertContained(moar_expected, out)

    # fastcomp. all asm, so it can't just work with wrong sigs. but, ASSERTIONS=2 gives much better info to debug
    test(['-O1'], 'If this abort() is unexpected, build with -s ASSERTIONS=1 which can give more information.') # no useful info, but does mention ASSERTIONS
    test(['-O1', '-s', 'ASSERTIONS=1'], '''Invalid function pointer called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)
Build with ASSERTIONS=2 for more info.
''') # some useful text
    test(['-O1', '-s', 'ASSERTIONS=2'], ('''Invalid function pointer '0' called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)
This pointer might make sense in another type signature:''', '''Invalid function pointer '1' called with signature 'v'. Perhaps this is an invalid value (e.g. caused by calling a virtual method on a NULL pointer)? Or calling a function with an incorrect type, which will fail? (it is worth building your source files with -Werror (warnings are errors), as warnings can indicate undefined behavior which can cause this)'''), "i: asm['_my_func']") # actually useful identity of the bad pointer, with comparisons to what it would be in other types/tables
    test(['-O1', '-s', 'EMULATE_FUNCTION_POINTER_CASTS=1'], '''my func\n''') # emulate so it works

  def test_l_link(self):
    # Linking with -lLIBNAME and -L/DIRNAME should work, also should work with spaces

    def build(path, args):
        check_execute([PYTHON, EMCC, self.in_dir(*path)] + args)

    open(self.in_dir('main.cpp'), 'w').write('''
      extern void printey();
      int main() {
        printey();
        return 0;
      }
    ''')

    try:
      os.makedirs(self.in_dir('libdir'))
    except:
      pass

    open(self.in_dir('libdir', 'libfile.cpp'), 'w').write('''
      #include <stdio.h>
      void printey() {
        printf("hello from lib\\n");
      }
    ''')

    libfile = self.in_dir('libdir', 'libfile.so')
    aout = self.in_dir('a.out.js')

    # Test linking the library built here by emcc
    build(['libdir', 'libfile.cpp'], ['-c'])
    shutil.move(self.in_dir('libfile.o'), libfile)
    build(['main.cpp'], ['-L' + self.in_dir('libdir'), '-lfile'])

    self.assertContained('hello from lib', run_js(aout))

    # Also test execution with `-l c` and space-separated library linking syntax
    os.remove(aout)
    build(['libdir', 'libfile.cpp'], ['-c', '-l', 'c'])
    shutil.move(self.in_dir('libfile.o'), libfile)
    build(['main.cpp'], ['-L', self.in_dir('libdir'), '-l', 'file'])

    self.assertContained('hello from lib', run_js(aout))

    assert not os.path.exists('a.out') and not os.path.exists('a.exe'), 'Must not leave unneeded linker stubs'

  def test_outline(self):
    def test(name, src, libs, expected, expected_ranges, args=[], suffix='cpp'):
      print name

      def measure_funcs(filename):
        i = 0
        start = -1
        curr = None
        ret = {}
        for line in open(filename):
          i += 1
          if line.startswith('function '):
            start = i
            curr = line
          elif line.startswith('}') and curr:
            size = i - start
            ret[curr] = size
            curr = None
        return ret

      for debug, outlining_limits in [
        ([], (1000,)),
        (['-g1'], (1000,)),
        (['-g2'], (1000,)),
        (['-g'], (100, 250, 500, 1000, 2000, 5000, 0))
      ]:
        for outlining_limit in outlining_limits:
          print '\n', Building.COMPILER_TEST_OPTS, debug, outlining_limit, '\n'
          # TODO: test without -g3, tell all sorts
          Popen([PYTHON, EMCC, src] + libs + ['-o', 'test.js', '-O2'] + debug + ['-s', 'OUTLINING_LIMIT=%d' % outlining_limit] + args).communicate()
          assert os.path.exists('test.js')
          shutil.copyfile('test.js', '%d_test.js' % outlining_limit)
          for engine in JS_ENGINES:
            if engine == V8_ENGINE: continue # ban v8, weird failures
            out = run_js('test.js', engine=engine, stderr=PIPE, full_output=True)
            self.assertContained(expected, out)
            if engine == SPIDERMONKEY_ENGINE: self.validate_asmjs(out)
          if debug == ['-g']:
            low = expected_ranges[outlining_limit][0]
            seen = max(measure_funcs('test.js').values())
            high = expected_ranges[outlining_limit][1]
            print Building.COMPILER_TEST_OPTS, outlining_limit, '   ', low, '<=', seen, '<=', high
            assert low <= seen <= high

    for test_opts, expected_ranges in [
      ([], {
         100: (150, 500),
         250: (150, 600),
         500: (150, 700),
        1000: (200, 1000),
        2000: (250, 2000),
        5000: (500, 5000),
           0: (1000, 5000)
      }),
      (['-O2'], {
         100: (0, 1600),
         250: (0, 1600),
         500: (0, 1600),
        1000: (0, 1600),
        2000: (0, 2000),
        5000: (0, 5000),
           0: (0, 5000)
      }),
    ]:
      Building.COMPILER_TEST_OPTS = test_opts
      test('zlib', path_from_root('tests', 'zlib', 'example.c'),
                   get_zlib_library(self),
                   open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                   expected_ranges,
                   args=['-I' + path_from_root('tests', 'zlib')], suffix='c')

  def test_symlink(self):
    self.clear()
    if os.name == 'nt':
      return self.skip('Windows FS does not need to be tested for symlinks support, since it does not have them.')
    open(os.path.join(self.get_dir(), 'foobar.xxx'), 'w').write('int main(){ return 0; }')
    os.symlink(os.path.join(self.get_dir(), 'foobar.xxx'), os.path.join(self.get_dir(), 'foobar.c'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foobar.c'), '-o', os.path.join(self.get_dir(), 'foobar')]).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar.xxx'))
    try_delete(os.path.join(self.get_dir(), 'foobar.c'))

    open(os.path.join(self.get_dir(), 'foobar.c'), 'w').write('int main(){ return 0; }')
    os.symlink(os.path.join(self.get_dir(), 'foobar.c'), os.path.join(self.get_dir(), 'foobar.xxx'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foobar.xxx'), '-o', os.path.join(self.get_dir(), 'foobar')]).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar.xxx'))
    try_delete(os.path.join(self.get_dir(), 'foobar.c'))

  def test_multiply_defined_libsymbols(self):
    lib = "int mult() { return 1; }"
    lib_name = os.path.join(self.get_dir(), 'libA.c')
    open(lib_name, 'w').write(lib)
    a2 = "void x() {}"
    a2_name = os.path.join(self.get_dir(), 'a2.c')
    open(a2_name, 'w').write(a2)
    b2 = "void y() {}"
    b2_name = os.path.join(self.get_dir(), 'b2.c')
    open(b2_name, 'w').write(b2)
    main = r'''
      #include <stdio.h>
      int mult();
      int main() {
        printf("result: %d\n", mult());
        return 0;
      }
    '''
    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(main)

    Building.emcc(lib_name, output_filename='libA.so')

    Building.emcc(a2_name, ['-L.', '-lA'])
    Building.emcc(b2_name, ['-L.', '-lA'])

    Building.emcc(main_name, ['-L.', '-lA', a2_name+'.o', b2_name+'.o'], output_filename='a.out.js')

    self.assertContained('result: 1', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_multiply_defined_libsymbols_2(self):
    a = "int x() { return 55; }"
    a_name = os.path.join(self.get_dir(), 'a.c')
    open(a_name, 'w').write(a)
    b = "int y() { return 2; }"
    b_name = os.path.join(self.get_dir(), 'b.c')
    open(b_name, 'w').write(b)
    c = "int z() { return 5; }"
    c_name = os.path.join(self.get_dir(), 'c.c')
    open(c_name, 'w').write(c)
    main = r'''
      #include <stdio.h>
      int x();
      int y();
      int z();
      int main() {
        printf("result: %d\n", x() + y() + z());
        return 0;
      }
    '''
    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(main)

    Building.emcc(a_name) # a.c.o
    Building.emcc(b_name) # b.c.o
    Building.emcc(c_name) # c.c.o
    lib_name = os.path.join(self.get_dir(), 'libLIB.a')
    Building.emar('cr', lib_name, [a_name + '.o', b_name + '.o']) # libLIB.a with a and b

    # a is in the lib AND in an .o, so should be ignored in the lib. We do still need b from the lib though
    Building.emcc(main_name, [a_name+'.o', c_name + '.o', '-L.', '-lLIB'], output_filename='a.out.js')

    self.assertContained('result: 62', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_link_group(self):
    lib_src_name = os.path.join(self.get_dir(), 'lib.c')
    open(lib_src_name, 'w').write('int x() { return 42; }')

    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(r'''
      #include <stdio.h>
      int x();
      int main() {
        printf("result: %d\n", x());
        return 0;
      }
    ''')

    Building.emcc(lib_src_name) # lib.c.o
    lib_name = os.path.join(self.get_dir(), 'libLIB.a')
    Building.emar('cr', lib_name, [lib_src_name + '.o']) # libLIB.a with lib.c.o

    def test(lib_args, err_expected):
      print err_expected
      output = Popen([PYTHON, EMCC, main_name, '-o', 'a.out.js'] + lib_args, stdout=PIPE, stderr=PIPE).communicate()
      #print output[1]
      if err_expected:
        self.assertContained(err_expected, output[1])
      else:
        self.assertNotContained('unresolved symbol', output[1])
        out_js = os.path.join(self.get_dir(), 'a.out.js')
        assert os.path.exists(out_js), '\n'.join(output)
        self.assertContained('result: 42', run_js(out_js))

    test(['-Wl,--start-group', lib_name], '--start-group without matching --end-group')
    test(['-Wl,--start-group', lib_name, '-Wl,--start-group'], 'Nested --start-group, missing --end-group?')
    test(['-Wl,--end-group', lib_name, '-Wl,--start-group'], '--end-group without --start-group')
    test(['-Wl,--start-group', lib_name, '-Wl,--end-group'], None)

    print 'embind test with groups'

    main_name = os.path.join(self.get_dir(), 'main.cpp')
    open(main_name, 'w').write(r'''
      #include <stdio.h>
      #include<emscripten/val.h>
      using namespace emscripten;
      extern "C" int x();
      int main() {
        int y = -x();
        y = val::global("Math").call<int>("abs", y);
        printf("result: %d\n", y);
        return 0;
      }
    ''')
    test(['-Wl,--start-group', lib_name, '-Wl,--end-group', '--bind'], None)

  def test_link_group_bitcode(self):
    one = open('1.c', 'w').write(r'''
int f(void);
int main() {
  f();
  return 0;
}
    ''')
    two = open('2.c', 'w').write(r'''
#include <stdio.h>
int f() {
  printf("Hello\n");
  return 0;
}
    ''')

    Popen([PYTHON, EMCC, '-o', '1.o', '1.c']).communicate()
    Popen([PYTHON, EMCC, '-o', '2.o', '2.c']).communicate()
    Popen([PYTHON, EMAR, 'crs', '2.a', '2.o']).communicate()
    Popen([PYTHON, EMCC, '-o', 'out.bc', '-Wl,--start-group', '2.a', '1.o', '-Wl,--end-group']).communicate()
    Popen([PYTHON, EMCC, 'out.bc']).communicate()
    self.assertContained('Hello', run_js('a.out.js'))

  def test_circular_libs(self):
    def tmp_source(name, code):
      file_name = os.path.join(self.get_dir(), name)
      open(file_name, 'w').write(code)
      return file_name

    a = tmp_source('a.c', 'int z(); int x() { return z(); }')
    b = tmp_source('b.c', 'int x(); int y() { return x(); } int z() { return 42; }')
    c = tmp_source('c.c', 'int q() { return 0; }')
    main = tmp_source('main.c', r'''
      #include <stdio.h>
      int y();
      int main() {
        printf("result: %d\n", y());
        return 0;
      }
    ''')

    Building.emcc(a) # a.c.o
    Building.emcc(b) # b.c.o
    Building.emcc(c) # c.c.o
    lib_a = os.path.join(self.get_dir(), 'libA.a')
    Building.emar('cr', lib_a, [a + '.o', c + '.o']) # libA.a with a.c.o,c.c.o
    lib_b = os.path.join(self.get_dir(), 'libB.a')
    Building.emar('cr', lib_b, [b + '.o', c + '.o']) # libB.a with b.c.o,c.c.o

    args = ['-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1', main, '-o', 'a.out.js']
    libs_list = [lib_a, lib_b]

    # lib_a does not satisfy any symbols from main, so it will not be included,
    # and there will be an unresolved symbol.
    output = Popen([PYTHON, EMCC] + args + libs_list, stdout=PIPE, stderr=PIPE).communicate()
    self.assertContained('error: unresolved symbol: x', output[1])

    # -Wl,--start-group and -Wl,--end-group around the libs will cause a rescan
    # of lib_a after lib_b adds undefined symbol "x", so a.c.o will now be
    # included (and the link will succeed).
    libs = ['-Wl,--start-group'] + libs_list + ['-Wl,--end-group']
    output = Popen([PYTHON, EMCC] + args + libs, stdout=PIPE, stderr=PIPE).communicate()
    out_js = os.path.join(self.get_dir(), 'a.out.js')
    assert os.path.exists(out_js), '\n'.join(output)
    self.assertContained('result: 42', run_js(out_js))

    # -( and -) should also work.
    args = ['-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1', main, '-o', 'a2.out.js']
    libs = ['-Wl,-('] + libs_list + ['-Wl,-)']
    output = Popen([PYTHON, EMCC] + args + libs, stdout=PIPE, stderr=PIPE).communicate()
    out_js = os.path.join(self.get_dir(), 'a2.out.js')
    assert os.path.exists(out_js), '\n'.join(output)
    self.assertContained('result: 42', run_js(out_js))

  def test_redundant_link(self):
    lib = "int mult() { return 1; }"
    lib_name = os.path.join(self.get_dir(), 'libA.c')
    open(lib_name, 'w').write(lib)
    main = r'''
      #include <stdio.h>
      int mult();
      int main() {
        printf("result: %d\n", mult());
        return 0;
      }
    '''
    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(main)

    Building.emcc(lib_name, output_filename='libA.so')

    Building.emcc(main_name, ['libA.so']*2, output_filename='a.out.js')

    self.assertContained('result: 1', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_export_all(self):
    lib = r'''
      #include <stdio.h>
      void libf1() { printf("libf1\n"); }
      void libf2() { printf("libf2\n"); }
    '''
    lib_name = os.path.join(self.get_dir(), 'lib.c')
    open(lib_name, 'w').write(lib)

    open('main.js', 'w').write('''
      _libf1();
      _libf2();
    ''')

    Building.emcc(lib_name, ['-s', 'EXPORT_ALL=1', '-s', 'LINKABLE=1', '--post-js', 'main.js'], output_filename='a.out.js')

    self.assertContained('libf1\nlibf2\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_stdin(self):
    def _test():
      for engine in JS_ENGINES:
        if engine == V8_ENGINE: continue # no stdin support in v8 shell
        engine[0] = os.path.normpath(engine[0])
        print >> sys.stderr, engine
        # work around a bug in python's subprocess module
        # (we'd use run_js() normally)
        try_delete('out.txt')
        if os.name == 'nt': # windows
          os.system('type "in.txt" | {} >out.txt'.format(' '.join(make_js_command(os.path.normpath(exe), engine))))
        else: # posix
          os.system('cat in.txt | {} > out.txt'.format(' '.join(make_js_command(exe, engine))))
        self.assertContained('abcdef\nghijkl\neof', open('out.txt').read())

    Building.emcc(path_from_root('tests', 'module', 'test_stdin.c'), output_filename='a.out.js')
    open('in.txt', 'w').write('abcdef\nghijkl')
    exe = os.path.join(self.get_dir(), 'a.out.js')
    _test()
    Building.emcc(path_from_root('tests', 'module', 'test_stdin.c'),
                  ['-O2', '--closure', '1'],
                  output_filename='a.out.js')
    _test()

  def test_ungetc_fscanf(self):
    open('main.cpp', 'w').write(r'''
      #include <stdio.h>
      int main(int argc, char const *argv[])
      {
          char str[4] = {0};
          FILE* f = fopen("my_test.input", "r");
          if (f == NULL) {
              printf("cannot open file\n");
              return -1;
          }
          ungetc('x', f);
          ungetc('y', f);
          ungetc('z', f);
          fscanf(f, "%3s", str);
          printf("%s\n", str);
          return 0;
      }
    ''')
    open('my_test.input', 'w').write('abc')
    Building.emcc('main.cpp', ['--embed-file', 'my_test.input'], output_filename='a.out.js')
    self.assertContained('zyx', Popen(JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_abspaths(self):
    # Includes with absolute paths are generally dangerous, things like -I/usr/.. will get to system local headers, not our portable ones.

    shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'main.c')

    for args, expected in [(['-I/usr/something', '-Wwarn-absolute-paths'], True),
                           (['-L/usr/something', '-Wwarn-absolute-paths'], True),
                           (['-I/usr/something'], False),
                           (['-L/usr/something'], False),
                           (['-I/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-L/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-Isubdir/something', '-Wwarn-absolute-paths'], False),
                           (['-Lsubdir/something', '-Wwarn-absolute-paths'], False),
                           ([], False)]:
      print args, expected
      proc = Popen([PYTHON, EMCC, 'main.c'] + args, stderr=PIPE)
      err = proc.communicate()[1]
      assert proc.returncode is 0
      assert ('encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript)' in err) == expected, err
      if not expected:
        assert err == '', err

  def test_local_link(self):
    # Linking a local library directly, like /usr/lib/libsomething.so, cannot work of course since it
    # doesn't contain bitcode. However, when we see that we should look for a bitcode file for that
    # library in the -L paths and system/lib
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      extern void printey();
      int main() {
        printey();
        return 0;
      }
    ''')

    try:
      os.makedirs(os.path.join(self.get_dir(), 'subdir'));
    except:
      pass
    open(os.path.join(self.get_dir(), 'subdir', 'libfile.so'), 'w').write('this is not llvm bitcode!')

    open(os.path.join(self.get_dir(), 'libfile.cpp'), 'w').write('''
      #include <stdio.h>
      void printey() {
        printf("hello from lib\\n");
      }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'libfile.cpp'), '-o', 'libfile.so']).communicate()
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), os.path.join(self.get_dir(), 'subdir', 'libfile.so'), '-L.'], stderr=PIPE).communicate()
    self.assertContained('hello from lib', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_runtimelink_multi(self):
    return self.skip('BUILD_AS_SHARED_LIB=2 is deprecated')
    if Settings.ASM_JS: return self.skip('asm does not support runtime linking yet')

    if SPIDERMONKEY_ENGINE not in JS_ENGINES: return self.skip('cannot run without spidermonkey due to node limitations')

    open('testa.h', 'w').write(r'''
      #ifndef _TESTA_H_
      #define _TESTA_H_

      class TestA {
        public:
          TestA();
      };

      #endif
    ''')
    open('testb.h', 'w').write(r'''
      #ifndef _TESTB_H_
      #define _TESTB_H_

      class TestB {
        public:
          TestB();
      };

      #endif
    ''')
    open('testa.cpp', 'w').write(r'''
      #include <stdio.h>
      #include <testa.h>

      TestA::TestA() {
        printf("TestA\n");
      }
    ''')
    open('testb.cpp', 'w').write(r'''
      #include <stdio.h>
      #include <testb.h>
      #include <testa.h>
      /*
      */
      TestB::TestB() {
        printf("TestB\n");
        TestA* testa = new TestA();
      }
    ''')
    open('main.cpp', 'w').write(r'''
      #include <stdio.h>
      #include <testa.h>
      #include <testb.h>

      /*
      */
      int main(int argc, char** argv) {
        printf("Main\n");
        TestA* testa = new TestA();
        TestB* testb = new TestB();
      }
    ''')

    Popen([PYTHON, EMCC, 'testa.cpp', '-o', 'liba.js', '-s', 'BUILD_AS_SHARED_LIB=2', '-s', 'LINKABLE=1', '-I.']).communicate()
    Popen([PYTHON, EMCC, 'testb.cpp', '-o', 'libb.js', '-s', 'BUILD_AS_SHARED_LIB=2', '-s', 'LINKABLE=1', '-I.']).communicate()
    Popen([PYTHON, EMCC, 'main.cpp', '-o', 'main.js', '-s', 'RUNTIME_LINKED_LIBS=["liba.js", "libb.js"]', '-I.', '-s', 'LINKABLE=1']).communicate()

    Popen([PYTHON, EMCC, 'main.cpp', 'testa.cpp', 'testb.cpp', '-o', 'full.js', '-I.']).communicate()

    self.assertContained('TestA\nTestB\nTestA\n', run_js('main.js', engine=SPIDERMONKEY_ENGINE))

  def test_identical_basenames(self):
    # Issue 287: files in different dirs but with the same basename get confused as the same,
    # causing multiply defined symbol errors
    try:
      os.makedirs(os.path.join(self.get_dir(), 'foo'));
    except:
      pass
    try:
      os.makedirs(os.path.join(self.get_dir(), 'bar'));
    except:
      pass
    open(os.path.join(self.get_dir(), 'foo', 'main.cpp'), 'w').write('''
      extern void printey();
      int main() {
        printey();
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'bar', 'main.cpp'), 'w').write('''
      #include<stdio.h>
      void printey() { printf("hello there\\n"); }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foo', 'main.cpp'), os.path.join(self.get_dir(), 'bar', 'main.cpp')]).communicate()
    self.assertContained('hello there', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # ditto with first creating .o files
    try_delete(os.path.join(self.get_dir(), 'a.out.js'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foo', 'main.cpp'), '-o', os.path.join(self.get_dir(), 'foo', 'main.o')]).communicate()
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'bar', 'main.cpp'), '-o', os.path.join(self.get_dir(), 'bar', 'main.o')]).communicate()
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foo', 'main.o'), os.path.join(self.get_dir(), 'bar', 'main.o')]).communicate()
    self.assertContained('hello there', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_main_a(self):
    # if main() is in a .a, we need to pull in that .a

    main_name = os.path.join(self.get_dir(), 'main.c')
    open(main_name, 'w').write(r'''
      #include <stdio.h>
      extern int f();
      int main() {
        printf("result: %d.\n", f());
        return 0;
      }
    ''')

    other_name = os.path.join(self.get_dir(), 'other.c')
    open(other_name, 'w').write(r'''
      #include <stdio.h>
      int f() { return 12346; }
    ''')

    Popen([PYTHON, EMCC, main_name, '-c', '-o', main_name+'.bc']).communicate()
    Popen([PYTHON, EMCC, other_name, '-c', '-o', other_name+'.bc']).communicate()

    Popen([PYTHON, EMAR, 'cr', main_name+'.a', main_name+'.bc']).communicate()

    Popen([PYTHON, EMCC, other_name+'.bc', main_name+'.a']).communicate()

    self.assertContained('result: 12346.', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_dup_o_in_a(self):
    open('common.c', 'w').write(r'''
      #include <stdio.h>
      void a(void) {
        printf("a\n");
      }
    ''')
    Popen([PYTHON, EMCC, 'common.c', '-c', '-o', 'common.o']).communicate()
    Popen([PYTHON, EMAR, 'rc', 'liba.a', 'common.o']).communicate()

    open('common.c', 'w').write(r'''
      #include <stdio.h>
      void b(void) {
        printf("b\n");
      }
    ''')
    Popen([PYTHON, EMCC, 'common.c', '-c', '-o', 'common.o']).communicate()
    Popen([PYTHON, EMAR, 'rc', 'libb.a', 'common.o']).communicate()

    open('main.c', 'w').write(r'''
      void a(void);
      void b(void);
      int main() {
        a();
        b();
      }
    ''')
    Popen([PYTHON, EMCC, 'main.c', '-L.', '-la', '-lb']).communicate()

    self.assertContained('a\nb\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_dup_o_in_one_a(self):
    open('common.c', 'w').write(r'''
      #include <stdio.h>
      void a(void) {
        printf("a\n");
      }
    ''')
    Popen([PYTHON, EMCC, 'common.c', '-c', '-o', 'common.o']).communicate()

    try:
      os.makedirs(os.path.join(self.get_dir(), 'libdir'));
    except:
      pass
    open(os.path.join('libdir', 'common.c'), 'w').write(r'''
      #include <stdio.h>
      void b(void) {
        printf("b...\n");
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join('libdir', 'common.c'), '-c', '-o', os.path.join('libdir', 'common.o')]).communicate()

    Popen([PYTHON, EMAR, 'rc', 'liba.a', 'common.o', os.path.join('libdir', 'common.o')]).communicate()

    open('main.c', 'w').write(r'''
      void a(void);
      void b(void);
      int main() {
        a();
        b();
      }
    ''')
    out, err = Popen([PYTHON, EMCC, 'main.c', '-L.', '-la'], stderr=PIPE).communicate()
    assert 'loading from archive' not in err, err
    assert 'which has duplicate entries' not in err, err
    assert 'duplicate: common.o' not in err, err
    self.assertContained('a\nb...\n', run_js('a.out.js'))

    text = Popen([PYTHON, EMAR, 't', 'liba.a'], stdout=PIPE).communicate()[0]
    assert 'common.o' not in text, text
    assert text.count('common_') == 2, text
    for line in text.split('\n'):
      assert len(line) < 20, line # should not have huge hash names

    # make the hashing fail: 'q' is just a quick append, no replacement, so hashing is not done, and dupes are easy
    Popen([PYTHON, EMAR, 'q', 'liba.a', 'common.o', os.path.join('libdir', 'common.o')]).communicate()
    out, err = Popen([PYTHON, EMCC, 'main.c', '-L.', '-la'], stderr=PIPE).communicate()
    assert 'loading from archive' in err, err
    assert 'which has duplicate entries' in err, err
    assert 'duplicate: common.o' in err, err
    assert err.count('duplicate: ') == 1, err # others are not duplicates - the hashing keeps them separate

  def test_export_in_a(self):
    export_name = 'this_is_an_entry_point'

    open('export.c', 'w').write(r'''
      #include <stdio.h>
      void %s(void) {
        printf("Hello, world!\n");
      }
    ''' % export_name)
    Popen([PYTHON, EMCC, 'export.c', '-c', '-o', 'export.o']).communicate()
    Popen([PYTHON, EMAR, 'rc', 'libexport.a', 'export.o']).communicate()

    open('main.c', 'w').write(r'''
      int main() {
        return 0;
      }
    ''')

    definition = 'function _%s(' % export_name

    # Sanity check: the symbol should not be linked in if not requested.
    Popen([PYTHON, EMCC, 'main.c', '-L.', '-lexport']).communicate()
    self.assertNotContained(definition, open(os.path.join(self.get_dir(), 'a.out.js')).read())

    # Sanity check: exporting without a definition does not cause it to appear.
    # Note: exporting main prevents emcc from warning that it generated no code.
    Popen([PYTHON, EMCC, 'main.c', '-s', '''EXPORTED_FUNCTIONS=['_main', '_%s']''' % export_name]).communicate()
    self.assertNotContained(definition, open(os.path.join(self.get_dir(), 'a.out.js')).read())

    # Actual test: defining symbol in library and exporting it causes it to appear in the output.
    Popen([PYTHON, EMCC, 'main.c', '-L.', '-lexport', '-s', '''EXPORTED_FUNCTIONS=['_%s']''' % export_name]).communicate()
    self.assertContained(definition, open(os.path.join(self.get_dir(), 'a.out.js')).read())

  def test_embed_file(self):
    open(os.path.join(self.get_dir(), 'somefile.txt'), 'w').write('''hello from a file with lots of data and stuff in it thank you very much''')
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>
      int main() {
        FILE *f = fopen("somefile.txt", "r");
        char buf[100];
        fread(buf, 1, 20, f);
        buf[20] = 0;
        fclose(f);
        printf("|%s|\n", buf);
        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'somefile.txt']).communicate()
    self.assertContained('|hello from a file wi|', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # preload twice, should not err
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'somefile.txt', '--embed-file', 'somefile.txt']).communicate()
    self.assertContained('|hello from a file wi|', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_embed_file_dup(self):
    try_delete(os.path.join(self.get_dir(), 'tst'))
    os.mkdir(os.path.join(self.get_dir(), 'tst'))
    os.mkdir(os.path.join(self.get_dir(), 'tst', 'test1'))
    os.mkdir(os.path.join(self.get_dir(), 'tst', 'test2'))

    open(os.path.join(self.get_dir(), 'tst', 'aa.txt'), 'w').write('''frist''')
    open(os.path.join(self.get_dir(), 'tst', 'test1', 'aa.txt'), 'w').write('''sacond''')
    open(os.path.join(self.get_dir(), 'tst', 'test2', 'aa.txt'), 'w').write('''thard''')
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>
      #include <string.h>
      void print_file(const char *name) {
        FILE *f = fopen(name, "r");
        char buf[100];
        memset(buf, 0, 100);
        fread(buf, 1, 20, f);
        buf[20] = 0;
        fclose(f);
        printf("|%s|\n", buf);
      }
      int main() {
        print_file("tst/aa.txt");
        print_file("tst/test1/aa.txt");
        print_file("tst/test2/aa.txt");
        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'tst']).communicate()
    self.assertContained('|frist|\n|sacond|\n|thard|\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_exclude_file(self):
    try_delete(os.path.join(self.get_dir(), 'tst'))
    os.mkdir(os.path.join(self.get_dir(), 'tst'))
    os.mkdir(os.path.join(self.get_dir(), 'tst', 'abc.exe'))
    os.mkdir(os.path.join(self.get_dir(), 'tst', 'abc.txt'))

    open(os.path.join(self.get_dir(), 'tst', 'hello.exe'), 'w').write('''hello''')
    open(os.path.join(self.get_dir(), 'tst', 'hello.txt'), 'w').write('''world''')
    open(os.path.join(self.get_dir(), 'tst', 'abc.exe', 'foo'), 'w').write('''emscripten''')
    open(os.path.join(self.get_dir(), 'tst', 'abc.txt', 'bar'), 'w').write('''!!!''')
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>
      int main() {
        if(fopen("tst/hello.exe", "rb")) printf("Failed\n");
        if(!fopen("tst/hello.txt", "rb")) printf("Failed\n");
        if(fopen("tst/abc.exe/foo", "rb")) printf("Failed\n");
        if(!fopen("tst/abc.txt/bar", "rb")) printf("Failed\n");

        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--embed-file', 'tst', '--exclude-file', '*.exe']).communicate()
    output = run_js(os.path.join(self.get_dir(), 'a.out.js'))
    assert output == '' or output == ' \n'

  def test_multidynamic_link(self):
    # Linking the same dynamic library in statically will error, normally, since we statically link it, causing dupe symbols

    def test(link_cmd, lib_suffix=''):
      print link_cmd, lib_suffix

      self.clear()

      open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
        #include <stdio.h>
        extern void printey();
        extern void printother();
        int main() {
          printf("*");
          printey();
          printf("\n");
          printother();
          printf("\n");
          printf("*");
          return 0;
        }
      ''')

      try:
        os.makedirs(os.path.join(self.get_dir(), 'libdir'));
      except:
        pass

      open(os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), 'w').write('''
        #include <stdio.h>
        void printey() {
          printf("hello from lib");
        }
      ''')

      open(os.path.join(self.get_dir(), 'libdir', 'libother.cpp'), 'w').write('''
        #include <stdio.h>
        extern void printey();
        void printother() {
          printf("|");
          printey();
          printf("|");
        }
      ''')

      compiler = [PYTHON, EMCC]

      # Build libfile normally into an .so
      Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), '-o', os.path.join(self.get_dir(), 'libdir', 'libfile.so' + lib_suffix)]).communicate()
      # Build libother and dynamically link it to libfile
      Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libother.cpp')] + link_cmd + ['-o', os.path.join(self.get_dir(), 'libdir', 'libother.so')]).communicate()
      # Build the main file, linking in both the libs
      Popen(compiler + [os.path.join(self.get_dir(), 'main.cpp')] + link_cmd + ['-lother', '-c']).communicate()
      print '...'
      # The normal build system is over. We need to do an additional step to link in the dynamic libraries, since we ignored them before
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.o')] + link_cmd + ['-lother']).communicate()

      self.assertContained('*hello from lib\n|hello from lib|\n*', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    test(['-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile']) # -l, auto detection from library path
    test(['-L' + os.path.join(self.get_dir(), 'libdir'), os.path.join(self.get_dir(), 'libdir', 'libfile.so.3.1.4.1.5.9')], '.3.1.4.1.5.9') # handle libX.so.1.2.3 as well

  def test_js_link(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from main\\n");
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'before.js'), 'w').write('''
      var MESSAGE = 'hello from js';
      if (typeof Module != 'undefined') throw 'This code should run before anything else!';
    ''')
    open(os.path.join(self.get_dir(), 'after.js'), 'w').write('''
      Module.print(MESSAGE);
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'before.js', '--post-js', 'after.js', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
    self.assertContained('hello from main\nhello from js\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_sdl_endianness(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>
      #include <SDL/SDL.h>

      int main() {
        printf("%d, %d, %d\n", SDL_BYTEORDER, SDL_LIL_ENDIAN, SDL_BIG_ENDIAN);
        return 0;
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp')]).communicate()
    self.assertContained('1234, 1234, 4321\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_libpng(self):
    shutil.copyfile(path_from_root('tests', 'pngtest.png'), 'pngtest.png')
    Building.emcc(path_from_root('tests','pngtest.c'), ['--embed-file', 'pngtest.png', '-s', 'USE_ZLIB=1', '-s', 'USE_LIBPNG=1'], output_filename='a.out.js')
    self.assertContained('TESTS PASSED', Popen(JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_bullet(self):
    Building.emcc(path_from_root('tests','bullet_hello_world.cpp'), ['-s', 'USE_BULLET=1'], output_filename='a.out.js')
    self.assertContained('BULLET RUNNING', Popen(JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_vorbis(self):
    #This will also test if ogg compiles, because vorbis depends on ogg
    Building.emcc(path_from_root('tests','vorbis_test.c'), ['-s', 'USE_VORBIS=1'], output_filename='a.out.js')
    self.assertContained('ALL OK', Popen(JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_freetype(self):
    # copy the Liberation Sans Bold truetype file located in the <emscripten_root>/tests/freetype to the compilation folder
    shutil.copy2(path_from_root('tests/freetype','LiberationSansBold.ttf'), os.getcwd())
    # build test program with the font file embed in it
    Building.emcc(path_from_root('tests','freetype_test.c'), ['-s', 'USE_FREETYPE=1', '--embed-file', 'LiberationSansBold.ttf'], output_filename='a.out.js')
    # the test program will print an ascii representation of a bitmap where the 'w' character has been rendered using the Liberation Sans Bold font
    expectedOutput = '***   +***+   **\n' + \
                     '***+  +***+  +**\n' + \
                     '***+  *****  +**\n' + \
                     '+**+ +**+**+ +**\n' + \
                     '+*** +**+**+ ***\n' + \
                     ' *** +** **+ ***\n' + \
                     ' ***+**+ +**+**+\n' + \
                     ' +**+**+ +**+**+\n' + \
                     ' +*****  +*****+\n' + \
                     '  *****   ***** \n' + \
                     '  ****+   +***+ \n' + \
                     '  +***+   +***+ \n'
    self.assertContained(expectedOutput, Popen(JS_ENGINES[0] + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_link_memcpy(self):
    # memcpy can show up *after* optimizations, so after our opportunity to link in libc, so it must be special-cased
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>

      int main(int argc, char **argv) {
        int num = argc + 10;
        char buf[num], buf2[num];
        for (int i = 0; i < num; i++) {
          buf[i] = i*i+i/3;
        }
        for (int i = 1; i < num; i++) {
          buf[i] += buf[i-1];
        }
        for (int i = 0; i < num; i++) {
          buf2[i] = buf[i];
        }
        for (int i = 1; i < num; i++) {
          buf2[i] += buf2[i-1];
        }
        for (int i = 0; i < num; i++) {
          printf("%d:%d\n", i, buf2[i]);
        }
        return 0;
      }
    ''')
    Popen([PYTHON, EMCC, '-O2', os.path.join(self.get_dir(), 'main.cpp')]).communicate()
    output = run_js(os.path.join(self.get_dir(), 'a.out.js'), full_output=True, stderr=PIPE)
    self.assertContained('''0:0
1:1
2:6
3:21
4:53
5:111
6:-49
7:98
8:55
9:96
10:-16
''', output)
    self.assertNotContained('warning: library.js memcpy should not be running, it is only for testing!', output)

  def test_warn_undefined(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>
      #include <SDL.h>
      #include "SDL/SDL_opengl.h"

      extern "C" {
        void something();
        void elsey();
      }

      int main() {
        printf("%p", SDL_GL_GetProcAddress("glGenTextures")); // pull in gl proc stuff, avoid warnings on emulation funcs
        something();
        elsey();
        return 0;
      }
    ''')

    def clear(): try_delete('a.out.js')

    for args in [[], ['-O2']]:
      for action in ['WARN', 'ERROR', None]:
        for value in ([0, 1] if action else [0]):
          clear()
          print 'warn', args, action, value
          extra = ['-s', action + '_ON_UNDEFINED_SYMBOLS=%d' % value] if action else []
          output = Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp')] + extra + args, stderr=PIPE).communicate()
          if action == None or (action == 'WARN' and value):
            self.assertContained('unresolved symbol: something', output[1])
            self.assertContained('unresolved symbol: elsey', output[1])
            assert os.path.exists('a.out.js')
            self.assertNotContained('unresolved symbol: emscripten_', output[1])
          elif action == 'ERROR' and value:
            self.assertContained('unresolved symbol: something', output[1])
            self.assertContained('unresolved symbol: elsey', output[1])
            self.assertNotContained('warning', output[1])
            assert not os.path.exists('a.out.js')
          elif action == 'WARN' and not value:
            self.assertNotContained('unresolved symbol', output[1])
            assert os.path.exists('a.out.js')

  def test_toobig(self):
    # very large [N x i8], we should not oom in the compiler
    self.clear()
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      #include <stdio.h>

      #define BYTES 100*1024*1024

      int main(int argc, char **argv) {
        if (argc == 100) {
          static char buf[BYTES];
          static char buf2[BYTES];
          for (int i = 0; i < BYTES; i++) {
            buf[i] = i*i;
            buf2[i] = i/3;
          }
          for (int i = 0; i < BYTES; i++) {
            buf[i] = buf2[i/2];
            buf2[i] = buf[i/3];
          }
          printf("%d\n", buf[10] + buf2[20]);
        }
        return 0;
      }
    ''')
    output = Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp')], stderr=PIPE).communicate()[1]
    print output
    assert os.path.exists('a.out.js')

  def test_prepost(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from main\\n");
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      var Module = {
        preRun: function() { Module.print('pre-run') },
        postRun: function() { Module.print('post-run') }
      };
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
    self.assertContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # never run, so no preRun or postRun
    src = open(os.path.join(self.get_dir(), 'a.out.js')).read().replace('// {{PRE_RUN_ADDITIONS}}', 'addRunDependency()')
    open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
    self.assertNotContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # noInitialRun prevents run
    for no_initial_run, run_dep in [(0, 0), (1, 0), (0, 1)]:
      print no_initial_run, run_dep
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-s', 'NO_EXIT_RUNTIME=1']).communicate()
      src = 'var Module = { noInitialRun: %d };\n' % no_initial_run + open(os.path.join(self.get_dir(), 'a.out.js')).read()
      if run_dep:
        src = src.replace('// {{PRE_RUN_ADDITIONS}}', '// {{PRE_RUN_ADDITIONS}}\naddRunDependency("test");') \
                 .replace('// {{POST_RUN_ADDITIONS}}', '// {{POST_RUN_ADDITIONS}}\nremoveRunDependency("test");')
      open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
      assert ('hello from main' in run_js(os.path.join(self.get_dir(), 'a.out.js'))) != no_initial_run, 'only run if no noInitialRun'

      if no_initial_run:
        # Calling main later should still work, filesystem etc. must be set up.
        print 'call main later'
        src = open(os.path.join(self.get_dir(), 'a.out.js')).read() + '\nModule.callMain();\n';
        open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
        assert 'hello from main' in run_js(os.path.join(self.get_dir(), 'a.out.js')), 'main should print when called manually'

    # Use postInit
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      var Module = {
        preRun: function() { Module.print('pre-run') },
        postRun: function() { Module.print('post-run') },
        preInit: function() { Module.print('pre-init') }
      };
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
    self.assertContained('pre-init\npre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_prepost2(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from main\\n");
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      var Module = {
        preRun: function() { Module.print('pre-run') },
      };
    ''')
    open(os.path.join(self.get_dir(), 'pre2.js'), 'w').write('''
      Module.postRun = function() { Module.print('post-run') };
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '--pre-js', 'pre2.js', '-s', 'NO_EXIT_RUNTIME=1']).communicate()
    self.assertContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_prepre(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      #include <stdio.h>
      int main() {
        printf("hello from main\\n");
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'pre.js'), 'w').write('''
      var Module = {
        preRun: [function() { Module.print('pre-run') }],
      };
    ''')
    open(os.path.join(self.get_dir(), 'pre2.js'), 'w').write('''
      Module.preRun.push(function() { Module.print('prepre') });
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '--pre-js', 'pre2.js']).communicate()
    self.assertContained('prepre\npre-run\nhello from main\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_save_bc(self):
    for save in [0, 1]:
      self.clear()
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_loop_malloc.cpp')] + ([] if not save else ['--save-bc', self.in_dir('my_bitcode.bc')])).communicate()
      assert 'hello, world!' in run_js(self.in_dir('a.out.js'))
      assert os.path.exists(self.in_dir('my_bitcode.bc')) == save
      if save:
        try_delete('a.out.js')
        Building.llvm_dis(self.in_dir('my_bitcode.bc'), self.in_dir('my_ll.ll'))
        try:
          os.environ['EMCC_LEAVE_INPUTS_RAW'] = '1'
          Popen([PYTHON, EMCC, 'my_ll.ll', '-o', 'two.js']).communicate()
          assert 'hello, world!' in run_js(self.in_dir('two.js'))
        finally:
          del os.environ['EMCC_LEAVE_INPUTS_RAW']

  def test_fix_closure(self):
    input = path_from_root('tests', 'test-fix-closure.js')
    expected = path_from_root('tests', 'test-fix-closure.out.js')
    Popen([PYTHON, path_from_root('tools', 'fix_closure.py'), input, 'out.js']).communicate(input)
    output = open('out.js').read()
    assert '0,zzz_Q_39fa,0' in output
    assert 'function(a,c)' not in output # should be uninlined, so it gets a name
    assert run_js(input) == run_js('out.js')

  def test_js_optimizer(self):
    for input, expected, passes in [
      (path_from_root('tests', 'optimizer', 'eliminateDeadGlobals.js'), open(path_from_root('tests', 'optimizer', 'eliminateDeadGlobals-output.js')).read(),
       ['eliminateDeadGlobals']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-output.js')).read(),
       ['hoistMultiples', 'removeAssignsToUndefined', 'simplifyExpressions']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-output.js')).read(),
       ['asm', 'simplifyExpressions']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-si.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-si-output.js')).read(),
       ['simplifyIfs']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-regs.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-regs-output.js')).read(),
       ['registerize']),
      (path_from_root('tests', 'optimizer', 'eliminator-test.js'), open(path_from_root('tests', 'optimizer', 'eliminator-test-output.js')).read(),
       ['eliminate']),
      (path_from_root('tests', 'optimizer', 'safe-eliminator-test.js'), open(path_from_root('tests', 'optimizer', 'safe-eliminator-test-output.js')).read(),
       ['eliminateMemSafe']),
      (path_from_root('tests', 'optimizer', 'asm-eliminator-test.js'), open(path_from_root('tests', 'optimizer', 'asm-eliminator-test-output.js')).read(),
       ['asm', 'eliminate']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-output.js')).read(),
       ['asm', 'registerize']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-harder.js'), [open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-harder-output.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-harder-output2.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-harder-output3.js')).read()],
       ['asm', 'registerizeHarder']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-min.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-regs-min-output.js')).read(),
       ['asm', 'registerize', 'minifyLocals']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre.js'), [open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-output.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-output2.js')).read()],
       ['asm', 'simplifyExpressions']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-f32.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-output-f32.js')).read(),
       ['asm', 'asmPreciseF32', 'simplifyExpressions', 'optimizeFrounds']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-f32.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-pre-output-f32-nosimp.js')).read(),
       ['asm', 'asmPreciseF32', 'optimizeFrounds']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-last.js'), [open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-lastOpts-output.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-lastOpts-output2.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-lastOpts-output3.js')).read()],
       ['asm', 'asmLastOpts']),
      (path_from_root('tests', 'optimizer', 'asmLastOpts.js'), open(path_from_root('tests', 'optimizer', 'asmLastOpts-output.js')).read(),
       ['asm', 'asmLastOpts']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-last.js'), [open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-last-output.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-last-output2.js')).read(), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-last-output3.js')).read()],
       ['asm', 'asmLastOpts', 'last']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-relocate.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-relocate-output.js')).read(),
       ['asm', 'relocate']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline1.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline1-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline2.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline2-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline3.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline3-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline4.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-outline4-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-minlast.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-asm-minlast-output.js')).read(),
       ['asm', 'minifyWhitespace', 'asmLastOpts', 'last']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-shiftsAggressive.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-shiftsAggressive-output.js')).read(),
       ['asm', 'aggressiveVariableElimination']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-localCSE.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-localCSE-output.js')).read(),
       ['asm', 'localCSE']),
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-ensureLabelSet.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-ensureLabelSet-output.js')).read(),
       ['asm', 'ensureLabelSet']),
      (path_from_root('tests', 'optimizer', '3154.js'), open(path_from_root('tests', 'optimizer', '3154-output.js')).read(),
       ['asm', 'eliminate', 'registerize', 'asmLastOpts', 'last']),
      (path_from_root('tests', 'optimizer', 'simd.js'), open(path_from_root('tests', 'optimizer', 'simd-output.js')).read(),
       ['asm', 'eliminate']), # eliminate, just enough to trigger asm normalization/denormalization
      (path_from_root('tests', 'optimizer', 'simd.js'), open(path_from_root('tests', 'optimizer', 'simd-output-memSafe.js')).read(),
       ['asm', 'eliminateMemSafe']),
      (path_from_root('tests', 'optimizer', 'safeLabelSetting.js'), open(path_from_root('tests', 'optimizer', 'safeLabelSetting-output.js')).read(),
       ['asm', 'safeLabelSetting']), # eliminate, just enough to trigger asm normalization/denormalization
      (path_from_root('tests', 'optimizer', 'null_if.js'), [open(path_from_root('tests', 'optimizer', 'null_if-output.js')).read(), open(path_from_root('tests', 'optimizer', 'null_if-output2.js')).read()],
       ['asm', 'registerizeHarder', 'asmLastOpts', 'minifyWhitespace']), # issue 3520
      (path_from_root('tests', 'optimizer', 'null_else.js'), [open(path_from_root('tests', 'optimizer', 'null_else-output.js')).read(), open(path_from_root('tests', 'optimizer', 'null_else-output2.js')).read()],
       ['asm', 'registerizeHarder', 'asmLastOpts', 'minifyWhitespace']), # issue 3549
      (path_from_root('tests', 'optimizer', 'test-js-optimizer-splitMemory.js'), open(path_from_root('tests', 'optimizer', 'test-js-optimizer-splitMemory-output.js')).read(),
       ['splitMemory']),
      (path_from_root('tests', 'optimizer', 'JSDCE.js'), open(path_from_root('tests', 'optimizer', 'JSDCE-output.js')).read(),
       ['JSDCE']),
    ]:
      print input, passes

      if type(expected) == str: expected = [expected]
      expected = map(lambda out: out.replace('\n\n', '\n').replace('\n\n', '\n'), expected)

      # test calling js optimizer
      print '  js'
      output = Popen(NODE_JS + [path_from_root('tools', 'js-optimizer.js'), input] + passes, stdin=PIPE, stdout=PIPE).communicate()[0]

      def check_js(js, expected):
        #print >> sys.stderr, 'chak\n==========================\n', js, '\n===========================\n'
        if 'registerizeHarder' in passes:
          # registerizeHarder is hard to test, as names vary by chance, nondeterminstically FIXME
          def fix(src):
            if type(src) is list:
              return map(fix, src)
            src = '\n'.join(filter(lambda line: 'var ' not in line, src.split('\n'))) # ignore vars
            def reorder(func):
              def swap(func, stuff):
                # emit EYE_ONE always before EYE_TWO, replacing i1,i2 or i2,i1 etc
                for i in stuff:
                  if i not in func: return func
                indexes = map(lambda i: [i, func.index(i)], stuff)
                indexes.sort(lambda x, y: x[1] - y[1])
                for j in range(len(indexes)):
                  func = func.replace(indexes[j][0], 'STD_' + str(j))
                return func
              func = swap(func, ['i1', 'i2', 'i3'])
              func = swap(func, ['i1', 'i2'])
              func = swap(func, ['i4', 'i5'])
              return func
            src = 'function '.join(map(reorder, src.split('function ')))
            return src
          js = fix(js)
          expected = fix(expected)
        self.assertIdentical(expected, js.replace('\r\n', '\n').replace('\n\n', '\n').replace('\n\n', '\n'))

      if input not in [ # blacklist of tests that are native-optimizer only
        path_from_root('tests', 'optimizer', 'asmLastOpts.js'),
        path_from_root('tests', 'optimizer', '3154.js')
      ]:
        check_js(output, expected)
      else:
        print '(skip non-native)'

      if js_optimizer.use_native(passes) and js_optimizer.get_native_optimizer():
        # test calling native
        def check_json():
          Popen(listify(NODE_JS) + [path_from_root('tools', 'js-optimizer.js'), output_temp, 'receiveJSON'], stdin=PIPE, stdout=open(output_temp + '.js', 'w')).communicate()
          output = open(output_temp + '.js').read()
          check_js(output, expected)

        self.clear()
        input_temp = 'temp.js'
        output_temp = 'output.js'
        shutil.copyfile(input, input_temp)
        Popen(listify(NODE_JS) + [path_from_root('tools', 'js-optimizer.js'), input_temp, 'emitJSON'], stdin=PIPE, stdout=open(input_temp + '.js', 'w')).communicate()
        original = open(input).read()
        if '// EXTRA_INFO:' in original:
          json = open(input_temp + '.js').read()
          json += '\n' + original[original.find('// EXTRA_INFO:'):]
          open(input_temp + '.js', 'w').write(json)

        # last is only relevant when we emit JS
        if 'last' not in passes and \
           'null_if' not in input and 'null_else' not in input:  # null-* tests are js optimizer or native, not a mixture (they mix badly)
          print '  native (receiveJSON)'
          output = Popen([js_optimizer.get_native_optimizer(), input_temp + '.js'] + passes + ['receiveJSON', 'emitJSON'], stdin=PIPE, stdout=open(output_temp, 'w')).communicate()[0]
          check_json()

          print '  native (parsing JS)'
          output = Popen([js_optimizer.get_native_optimizer(), input] + passes + ['emitJSON'], stdin=PIPE, stdout=open(output_temp, 'w')).communicate()[0]
          check_json()

        print '  native (emitting JS)'
        output = Popen([js_optimizer.get_native_optimizer(), input] + passes, stdin=PIPE, stdout=PIPE).communicate()[0]
        check_js(output, expected)

  def test_m_mm(self):
    open(os.path.join(self.get_dir(), 'foo.c'), 'w').write('''#include <emscripten.h>''')
    for opt in ['M', 'MM']:
      output, err = Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foo.c'), '-' + opt], stdout=PIPE, stderr=PIPE).communicate()
      assert 'foo.o: ' in output, '-%s failed to produce the right output: %s' % (opt, output)
      assert 'error' not in err, 'Unexpected stderr: ' + err

  def test_emcc_debug_files(self):
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    if not os.path.exists(CANONICAL_TEMP_DIR):
      os.makedirs(CANONICAL_TEMP_DIR)
    for opts in [0, 1, 2, 3]:
      for debug in [None, '1', '2']:
        print opts, debug
        try:
          if debug: os.environ['EMCC_DEBUG'] = debug
          for x in os.listdir(CANONICAL_TEMP_DIR):
            if x.startswith('emcc-'):
              os.unlink(os.path.join(CANONICAL_TEMP_DIR, x))
          check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '-O'+ str(opts)], stderr=PIPE)
          if debug is None:
            for x in os.listdir(CANONICAL_TEMP_DIR):
              if x.startswith('emcc-'):
                assert 0
          elif debug == '1':
            assert os.path.exists(os.path.join(CANONICAL_TEMP_DIR, 'emcc-0-linktime.bc'))
            assert os.path.exists(os.path.join(CANONICAL_TEMP_DIR, 'emcc-1-original.js'))
          elif debug == '2':
            assert os.path.exists(os.path.join(CANONICAL_TEMP_DIR, 'emcc-0-basebc.bc'))
            assert os.path.exists(os.path.join(CANONICAL_TEMP_DIR, 'emcc-1-linktime.bc'))
            assert os.path.exists(os.path.join(CANONICAL_TEMP_DIR, 'emcc-2-original.js'))
        finally:
          if debug: del os.environ['EMCC_DEBUG']

  def test_debuginfo(self):
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    try:
      os.environ['EMCC_DEBUG'] = '1'
      for args, expect_llvm, expect_js in [
          (['-O0'], False, True),
          (['-O0', '-g'], True, True),
          (['-O0', '-g4'], True, True),
          (['-O1'], False, True),
          (['-O1', '-g'], True, True),
          (['-O2'], False, False),
          (['-O2', '-g'], False, True), # drop llvm debug info as js opts kill it anyway
          (['-O2', '-g4'], True, True), # drop llvm debug info as js opts kill it anyway
        ]:
        print args, expect_llvm, expect_js
        output, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp')] + args, stdout=PIPE, stderr=PIPE).communicate()
        assert expect_llvm == ('strip-debug' not in err)
        assert expect_js == ('registerize' not in err)
    finally:
      del os.environ['EMCC_DEBUG']

  def test_scons(self): # also incidentally tests c++11 integration in llvm 3.1
    try_delete(os.path.join(self.get_dir(), 'test'))
    shutil.copytree(path_from_root('tests', 'scons'), os.path.join(self.get_dir(), 'test'))
    shutil.copytree(path_from_root('tools', 'scons', 'site_scons'), os.path.join(self.get_dir(), 'test', 'site_scons'))
    os.chdir(os.path.join(self.get_dir(), 'test'))
    Popen(['scons']).communicate()
    output = run_js('scons_integration.js', assert_returncode=5)
    assert 'If you see this - the world is all right!' in output

  def test_embind(self):
    environ = os.environ.copy()
    environ['EMCC_CLOSURE_ARGS'] = environ.get('EMCC_CLOSURE_ARGS', '') + " --externs " + pipes.quote(path_from_root('tests', 'embind', 'underscore-externs.js'))
    for args, fail in [
      ([], True), # without --bind, we fail
      (['--bind'], False),
      (['--bind', '-O1'], False),
      (['--bind', '-O2'], False),
      (['--bind', '-O2', '--closure', '1'], False),
      (['--bind', '-O2', '-s', 'ALLOW_MEMORY_GROWTH=1', path_from_root('tests', 'embind', 'isMemoryGrowthEnabled=true.cpp')], False),
    ]:
      print args, fail
      self.clear()
      try_delete(self.in_dir('a.out.js'))

      testFiles = [
        path_from_root('tests', 'embind', 'underscore-1.4.2.js'),
        path_from_root('tests', 'embind', 'imvu_test_adapter.js'),
        path_from_root('tests', 'embind', 'embind.test.js'),
      ]

      Popen(
        [ PYTHON,
          EMCC,
          path_from_root('tests', 'embind', 'embind_test.cpp'),
          '--pre-js', path_from_root('tests', 'embind', 'test.pre.js'),
          '--post-js', path_from_root('tests', 'embind', 'test.post.js'),
        ] + args,
        stderr=PIPE if fail else None,
        env=environ).communicate()

      assert os.path.exists(self.in_dir('a.out.js')) == (not fail)
      if not fail:
        with open(self.in_dir('a.out.js'), 'ab') as f:
          for tf in testFiles:
            f.write(open(tf, 'rb').read())

        output = run_js(self.in_dir('a.out.js'), stdout=PIPE, stderr=PIPE, full_output=True, assert_returncode=0)
        assert "FAIL" not in output, output

  def test_llvm_nativizer(self):
    if WINDOWS: return self.skip('test_llvm_nativizer does not work on Windows: https://github.com/kripken/emscripten/issues/702')
    if OSX: return self.skip('test_llvm_nativizer does not work on OS X: https://github.com/kripken/emscripten/issues/709')
    try:
      Popen(['as', '--version'], stdout=PIPE, stderr=PIPE).communicate()
    except:
      return self.skip('no gnu as, cannot run nativizer')

    # avoid impure_ptr problems etc.
    shutil.copyfile(path_from_root('tests', 'files.cpp'), os.path.join(self.get_dir(), 'files.cpp'))
    open(os.path.join(self.get_dir(), 'somefile.binary'), 'w').write('''waka waka############################''')
    open(os.path.join(self.get_dir(), 'test.file'), 'w').write('''ay file..............,,,,,,,,,,,,,,''')
    open(os.path.join(self.get_dir(), 'stdin'), 'w').write('''inter-active''')
    subprocess.check_call([PYTHON, EMCC, os.path.join(self.get_dir(), 'files.cpp'), '-c'])
    nativize_llvm = Popen([PYTHON, path_from_root('tools', 'nativize_llvm.py'), os.path.join(self.get_dir(), 'files.o')], stdout=PIPE, stderr=PIPE)
    nativize_llvm.communicate(input)
    assert nativize_llvm.returncode == 0
    output = Popen([os.path.join(self.get_dir(), 'files.o.run')], stdin=open(os.path.join(self.get_dir(), 'stdin')), stdout=PIPE, stderr=PIPE).communicate()
    self.assertContained('''size: 37
data: 119,97,107,97,32,119,97,107,97,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35,35
loop: 119 97 107 97 32 119 97 107 97 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 35 
input:inter-active
texto
$
5 : 10,30,20,11,88
other=ay file...
seeked= file.
''', output[0])
    self.assertContained('texte\n', output[1])

  def test_emconfig(self):
    output = Popen([PYTHON, EMCONFIG, 'LLVM_ROOT'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
    try:
      assert output == LLVM_ROOT
    except:
      print >> sys.stderr, 'Assertion failed: python %s LLVM_ROOT returned "%s" instead of expected "%s"!' % (EMCONFIG, output, LLVM_ROOT)
      raise
    invalid = 'Usage: em-config VAR_NAME'
    # Don't accept variables that do not exist
    output = Popen([PYTHON, EMCONFIG, 'VAR_WHICH_DOES_NOT_EXIST'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
    assert output == invalid
    # Don't accept no arguments
    output = Popen([PYTHON, EMCONFIG], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
    assert output == invalid
    # Don't accept more than one variable
    output = Popen([PYTHON, EMCONFIG, 'LLVM_ROOT', 'EMCC'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
    assert output == invalid
    # Don't accept arbitrary python code
    output = Popen([PYTHON, EMCONFIG, 'sys.argv[1]'], stdout=PIPE, stderr=PIPE).communicate()[0].strip()
    assert output == invalid

  def test_link_s(self):
    # -s OPT=VALUE can conflict with -s as a linker option. We warn and ignore
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
      extern "C" {
        void something();
      }

      int main() {
        something();
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'supp.cpp'), 'w').write(r'''
      #include <stdio.h>

      extern "C" {
        void something() {
          printf("yello\n");
        }
      }
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-o', 'main.o']).communicate()
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'supp.cpp'), '-o', 'supp.o']).communicate()

    output = Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.o'), '-s', os.path.join(self.get_dir(), 'supp.o'), '-s', 'SAFE_HEAP=1'], stderr=PIPE).communicate()
    output = run_js('a.out.js')
    assert 'yello' in output, 'code works'
    code = open('a.out.js').read()
    assert 'SAFE_HEAP' in code, 'valid -s option had an effect'

  def test_conftest_s_flag_passing(self):
    open(os.path.join(self.get_dir(), 'conftest.c'), 'w').write(r'''
      int main() {
        return 0;
      }
    ''')
    os.environ["EMMAKEN_JUST_CONFIGURE"] = "1"
    cmd = [PYTHON, EMCC, '-s', 'ASSERTIONS=1', os.path.join(self.get_dir(), 'conftest.c'), '-o', 'conftest']
    output = Popen(cmd, stderr=PIPE).communicate()
    del os.environ["EMMAKEN_JUST_CONFIGURE"]
    self.assertNotContained('emcc: warning: treating -s as linker option', output[1])
    assert os.path.exists('conftest')

  def test_file_packager(self):
    try:
      os.mkdir('subdir')
    except:
      pass
    open('data1.txt', 'w').write('data1')
    os.chdir('subdir')
    open('data2.txt', 'w').write('data2')
    # relative path to below the current dir is invalid
    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', '../data1.txt'], stdout=PIPE, stderr=PIPE).communicate()
    assert len(out) == 0
    assert 'below the current directory' in err
    # relative path that ends up under us is cool
    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', '../subdir/data2.txt'], stdout=PIPE, stderr=PIPE).communicate()
    assert len(out) > 0
    assert 'below the current directory' not in err
    # direct path leads to the same code being generated - relative path does not make us do anything different
    out2, err2 = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data2.txt'], stdout=PIPE, stderr=PIPE).communicate()
    assert len(out2) > 0
    assert 'below the current directory' not in err2
    def clean(txt):
      return filter(lambda line: 'PACKAGE_UUID' not in line and 'loadPackage({' not in line, txt.split('\n'))
    out = clean(out)
    out2 = clean(out2)
    assert out == out2
    # verify '--separate-metadata' option produces separate metadata file
    os.chdir('..')
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data1.txt', '--preload', 'subdir/data2.txt', '--js-output=immutable.js', '--separate-metadata']).communicate()
    assert os.path.isfile('immutable.js.metadata')
    # verify js output file is immutable when metadata is separated
    shutil.copy2('immutable.js', 'immutable.js.copy') # copy with timestamp preserved
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data1.txt', '--preload', 'subdir/data2.txt', '--js-output=immutable.js', '--separate-metadata']).communicate()
    import filecmp
    assert filecmp.cmp('immutable.js.copy', 'immutable.js')
    assert str(os.path.getmtime('immutable.js.copy')) == str(os.path.getmtime('immutable.js')) # assert both file content and timestamp are the same as reference copy
    # verify the content of metadata file is correct
    f = open('immutable.js.metadata', 'r')
    import json
    metadata = json.load(f)
    f.close
    assert len(metadata['files']) == 2
    assert metadata['files'][0]['start'] == 0 and metadata['files'][0]['end'] == len('data1') and metadata['files'][0]['filename'] == '/data1.txt'
    assert metadata['files'][1]['start'] == len('data1') and metadata['files'][1]['end'] == len('data1') + len('data2') and metadata['files'][1]['filename'] == '/subdir/data2.txt'
    assert metadata['remote_package_size'] == len('data1') + len('data2')
    import uuid
    try:
      uuid = uuid.UUID(metadata['package_uuid'], version = 4) # can only assert the uuid format is correct, the uuid's value is expected to differ in between invocation
    except ValueError:
      assert False

  def test_file_packager_unicode(self):
    unicode_name = 'unicode…☃'
    if not os.path.exists(unicode_name):
      try:
        os.mkdir(unicode_name)
      except:
        print "we failed to even create a unicode dir, so on this OS, we can't test this"
        return
    full = os.path.join(unicode_name, 'data.txt')
    open(full, 'w').write('data')
    proc = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', full], stdout=PIPE, stderr=PIPE)
    out, err = proc.communicate()
    assert proc.returncode == 0, err
    assert len(out) > 0, err
    assert len(err) == 0, err
    assert unicode_name in out, out
    print len(err)

  def test_crunch(self):
    try:
      print 'Crunch is located at ' + CRUNCH
    except:
      return self.skip('Skipped: Crunch is not present on the current system. Please install it (manually or via emsdk) and make sure it is activated in the Emscripten configuration file.')
    # crunch should not be run if a .crn exists that is more recent than the .dds
    shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
    time.sleep(0.1)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
    assert os.stat('test.data').st_size < 0.25*os.stat('ship.dds').st_size, 'Compressed should be much smaller than dds'
    crunch_time = os.stat('ship.crn').st_mtime
    dds_time = os.stat('ship.dds').st_mtime
    assert crunch_time >= dds_time, 'Crunch is more recent'
    # run again, should not recrunch!
    time.sleep(0.1)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
    if 'linux' in sys.platform: # OS time reporting in other OSes (OS X) seems flaky here
      assert crunch_time == os.stat('ship.crn').st_mtime, 'Crunch is unchanged ' + str([crunch_time, os.stat('ship.crn').st_mtime])
    # update dds, so should recrunch
    time.sleep(0.1)
    os.utime('ship.dds', None)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
    assert crunch_time < os.stat('ship.crn').st_mtime, 'Crunch was changed'

  def test_headless(self):
    if SPIDERMONKEY_ENGINE not in JS_ENGINES: return self.skip('cannot run without spidermonkey due to node limitations (Uint8ClampedArray etc.)')

    shutil.copyfile(path_from_root('tests', 'screenshot.png'), os.path.join(self.get_dir(), 'example.png'))
    Popen([PYTHON, EMCC, path_from_root('tests', 'sdl_headless.c'), '-s', 'HEADLESS=1']).communicate()
    output = run_js('a.out.js', engine=SPIDERMONKEY_ENGINE, stderr=PIPE)
    assert '''Init: 0
Font: 0x1
Sum: 0
you should see two lines of text in different colors and a blue rectangle
SDL_Quit called (and ignored)
done.
''' in output, output

  def test_preprocess(self):
    self.clear()

    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-E'], stdout=PIPE).communicate()
    assert not os.path.exists('a.out.js')
    # Test explicitly that the output contains a line typically written by the preprocessor.
    # Clang outputs on Windows lines like "#line 1", on Unix '# 1 '.
    # TODO: This is one more of those platform-specific discrepancies, investigate more if this ever becomes an issue,
    # ideally we would have emcc output identical data on all platforms.
    assert '''#line 1 ''' in out or '''# 1 ''' in out
    assert '''hello_world.c"''' in out
    assert '''printf("hello, world!''' in out

  def test_demangle(self):
    open('src.cpp', 'w').write('''
      #include <stdio.h>
      #include <emscripten.h>
      void two(char c) {
        EM_ASM(Module.print(stackTrace()));
      }
      void one(int x) {
        two(x % 17);
      }
      int main() {
        EM_ASM(Module.print(demangle('__Znwj'))); // check for no aborts
        EM_ASM(Module.print(demangle('_main')));
        EM_ASM(Module.print(demangle('__Z2f2v')));
        EM_ASM(Module.print(demangle('__Z12abcdabcdabcdi')));
        EM_ASM(Module.print(demangle('__ZL12abcdabcdabcdi')));
        EM_ASM(Module.print(demangle('__Z4testcsifdPvPiPc')));
        EM_ASM(Module.print(demangle('__ZN4test5moarrEcslfdPvPiPc')));
        EM_ASM(Module.print(demangle('__ZN4Waka1f12a234123412345pointEv')));
        EM_ASM(Module.print(demangle('__Z3FooIiEvv')));
        EM_ASM(Module.print(demangle('__Z3FooIidEvi')));
        EM_ASM(Module.print(demangle('__ZN3Foo3BarILi5EEEvv')));
        EM_ASM(Module.print(demangle('__ZNK10__cxxabiv120__si_class_type_info16search_below_dstEPNS_19__dynamic_cast_infoEPKvib')));
        EM_ASM(Module.print(demangle('__Z9parsewordRPKciRi')));
        EM_ASM(Module.print(demangle('__Z5multiwahtjmxyz')));
        EM_ASM(Module.print(demangle('__Z1aA32_iPA5_c')));
        EM_ASM(Module.print(demangle('__ZN21FWakaGLXFleeflsMarfooC2EjjjPKvbjj')));
        EM_ASM(Module.print(demangle('__ZN5wakaw2Cm10RasterBaseINS_6watwat9PolocatorEE8merbine1INS4_2OREEEvPKjj'))); // we get this wrong, but at least emit a '?'
        one(17);
        return 0;
      }
    ''')

    # full demangle support

    Popen([PYTHON, EMCC, 'src.cpp', '-s', 'DEMANGLE_SUPPORT=1']).communicate()
    output = run_js('a.out.js')
    self.assertContained('''operator new(unsigned int)
_main
f2()
abcdabcdabcd(int)
abcdabcdabcd(int)
test(char, short, int, float, double, void*, int*, char*)
test::moarr(char, short, long, float, double, void*, int*, char*)
Waka::f::a23412341234::point()
void Foo<int>()
void Foo<int, double>(int)
void Foo::Bar<5>()
__cxxabiv1::__si_class_type_info::search_below_dst(__cxxabiv1::__dynamic_cast_info*, void const*, int, bool) const
parseword(char const*&, int, int&)
multi(wchar_t, signed char, unsigned char, unsigned short, unsigned int, unsigned long, long long, unsigned long long, ...)
a(int [32], char (*) [5])
FWakaGLXFleeflsMarfoo::FWakaGLXFleeflsMarfoo(unsigned int, unsigned int, unsigned int, void const*, bool, unsigned int, unsigned int)
void wakaw::Cm::RasterBase<wakaw::watwat::Polocator>::merbine1<wakaw::Cm::RasterBase<wakaw::watwat::Polocator>::OR>(unsigned int const*, unsigned int)
''', output)
    # test for multiple functions in one stack trace
    assert 'one(int)' in output
    assert 'two(char)' in output

  def test_demangle_cpp(self):
    open('src.cpp', 'w').write('''
      #include <stdio.h>
      #include <emscripten.h>
      #include <cxxabi.h>
      #include <assert.h>

      int main() {
        char out[256];
        int status = 1;
        size_t length = 255;
        abi::__cxa_demangle("_ZN4Waka1f12a234123412345pointEv", out, &length, &status);
        assert(status == 0);
        printf("%s\\n", out);
        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    output = run_js('a.out.js')
    self.assertContained('Waka::f::a23412341234::point()', output)

  def test_module_exports_with_closure(self):
    # This test checks that module.export is retained when JavaScript is minified by compiling with --closure 1
    # This is important as if module.export is not present the Module object will not be visible to node.js
    # Run with ./runner.py other.test_module_exports_with_closure

    # First make sure test.js isn't present.
    self.clear()

    # compile with -O2 --closure 0
    Popen([PYTHON, EMCC, path_from_root('tests', 'Module-exports', 'test.c'), '-o', 'test.js', '-O2', '--closure', '0', '--pre-js', path_from_root('tests', 'Module-exports', 'setup.js'), '-s', 'EXPORTED_FUNCTIONS=["_bufferTest"]'], stdout=PIPE, stderr=PIPE).communicate()

    # Check that compilation was successful
    assert os.path.exists('test.js')
    test_js_closure_0 = open('test.js').read()

    # Check that test.js compiled with --closure 0 contains "module['exports'] = Module;"
    assert ("module['exports'] = Module;" in test_js_closure_0) or ('module["exports"]=Module' in test_js_closure_0)

    # Check that main.js (which requires test.js) completes successfully when run in node.js
    # in order to check that the exports are indeed functioning correctly.
    shutil.copyfile(path_from_root('tests', 'Module-exports', 'main.js'), 'main.js')
    if NODE_JS in JS_ENGINES:
      self.assertContained('bufferTest finished', run_js('main.js', engine=NODE_JS))

    # Delete test.js again and check it's gone.
    try_delete(path_from_root('tests', 'Module-exports', 'test.js'))
    assert not os.path.exists(path_from_root('tests', 'Module-exports', 'test.js'))

    # compile with -O2 --closure 1
    Popen([PYTHON, EMCC, path_from_root('tests', 'Module-exports', 'test.c'), '-o', path_from_root('tests', 'Module-exports', 'test.js'), '-O2', '--closure', '1', '--pre-js', path_from_root('tests', 'Module-exports', 'setup.js'), '-s', 'EXPORTED_FUNCTIONS=["_bufferTest"]'], stdout=PIPE, stderr=PIPE).communicate()

    # Check that compilation was successful
    assert os.path.exists(path_from_root('tests', 'Module-exports', 'test.js'))
    test_js_closure_1 = open(path_from_root('tests', 'Module-exports', 'test.js')).read()

    # Check that test.js compiled with --closure 1 contains "module.exports", we want to verify that
    # "module['exports']" got minified to "module.exports" when compiling with --closure 1
    assert "module.exports" in test_js_closure_1

    # Check that main.js (which requires test.js) completes successfully when run in node.js
    # in order to check that the exports are indeed functioning correctly.
    if NODE_JS in JS_ENGINES:
      self.assertContained('bufferTest finished', run_js('main.js', engine=NODE_JS))

    # Tidy up files that might have been created by this test.
    try_delete(path_from_root('tests', 'Module-exports', 'test.js'))
    try_delete(path_from_root('tests', 'Module-exports', 'test.js.map'))
    try_delete(path_from_root('tests', 'Module-exports', 'test.js.mem'))

  def test_fs_stream_proto(self):
    open('src.cpp', 'wb').write(r'''
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main()
{
    long file_size = 0;
    int h = open("src.cpp", O_RDONLY, 0666);
    if (0 != h)
    {
        FILE* file = fdopen(h, "rb");
        if (0 != file)
        {
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
        }
        else
        {
            printf("fdopen() failed: %s\n", strerror(errno));
            return 10;
        }
        close(h);
        printf("File size: %ld\n", file_size);
    }
    else
    {
        printf("open() failed: %s\n", strerror(errno));
        return 10;
    }
    return 0;
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp', '--embed-file', 'src.cpp']).communicate()
    for engine in JS_ENGINES:
      out = run_js('a.out.js', engine=engine, stderr=PIPE, full_output=True)
      self.assertContained('File size: 724', out)

  def check_simd(self, expected_simds, expected_out):
    if SPIDERMONKEY_ENGINE in JS_ENGINES:
      out = run_js('a.out.js', engine=SPIDERMONKEY_ENGINE, stderr=PIPE, full_output=True)
      self.validate_asmjs(out)
    else:
      out = run_js('a.out.js')
    self.assertContained(expected_out, out)

    src = open('a.out.js').read()
    asm = src[src.find('// EMSCRIPTEN_START_FUNCS'):src.find('// EMSCRIPTEN_END_FUNCS')]
    simds = asm.count('SIMD_')
    assert simds >= expected_simds, 'expecting to see at least %d SIMD* uses, but seeing %d' % (expected_simds, simds)

  def test_autovectorize_linpack(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'linpack.c'), '-O2', '-s', 'SIMD=1', '-DSP', '-s', 'PRECISE_F32=1', '--profiling']).communicate()
    self.check_simd(30, 'Unrolled Single  Precision')

  def test_autovectorize_bullet(self):
    Building.emcc(path_from_root('tests','bullet_hello_world.cpp'), ['-O2', '-s', 'SIMD=1', '-s', 'INLINING_LIMIT=1', '--llvm-lto', '2', '-s', 'USE_BULLET=1', '-profiling'], output_filename='a.out.js')
    self.check_simd(100, 'BULLET RUNNING')

  def test_dependency_file(self):
    # Issue 1732: -MMD (and friends) create dependency files that need to be
    # copied from the temporary directory.

    open(os.path.join(self.get_dir(), 'test.cpp'), 'w').write(r'''
      #include "test.hpp"

      void my_function()
      {
      }
    ''')
    open(os.path.join(self.get_dir(), 'test.hpp'), 'w').write(r'''
      void my_function();
    ''')

    Popen([PYTHON, EMCC, '-MMD', '-c', os.path.join(self.get_dir(), 'test.cpp'), '-o',
      os.path.join(self.get_dir(), 'test.o')]).communicate()

    assert os.path.exists(os.path.join(self.get_dir(), 'test.d')), 'No dependency file generated'
    deps = open(os.path.join(self.get_dir(), 'test.d')).read()
    # Look for ': ' instead of just ':' to not confuse C:\path\ notation with make "target: deps" rule. Not perfect, but good enough for this test.
    head, tail = deps.split(': ', 2)
    assert 'test.o' in head, 'Invalid dependency target'
    assert 'test.cpp' in tail and 'test.hpp' in tail, 'Invalid dependencies generated'

  def test_dependency_file_2(self):
    self.clear()
    shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'a.c')
    Popen([PYTHON, EMCC, 'a.c', '-MMD', '-MF', 'test.d', '-c']).communicate()
    self.assertContained(open('test.d').read(), 'a.o: a.c\n')

    self.clear()
    shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'a.c')
    Popen([PYTHON, EMCC, 'a.c', '-MMD', '-MF', 'test.d', '-c', '-o', 'test.o']).communicate()
    self.assertContained(open('test.d').read(), 'test.o: a.c\n')

    self.clear()
    shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'a.c')
    os.mkdir('obj')
    Popen([PYTHON, EMCC, 'a.c', '-MMD', '-MF', 'test.d', '-c', '-o', 'obj/test.o']).communicate()
    self.assertContained(open('test.d').read(), 'obj/test.o: a.c\n')

  def test_quoted_js_lib_key(self):
    open('lib.js', 'w').write(r'''
mergeInto(LibraryManager.library, {
   __internal_data:{
    '<' : 0,
    'white space' : 1
  },
  printf__deps: ['__internal_data', 'fprintf']
});
''')

    Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '--js-library', 'lib.js']).communicate()
    self.assertContained('hello, world!', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_EMCC_BUILD_DIR(self):
    # EMCC_BUILD_DIR env var contains the dir we were building in, when running the js compiler (e.g. when
    # running a js library). We force the cwd to be src/ for technical reasons, so this lets you find out
    # where you were.
    open('lib.js', 'w').write(r'''
printErr('dir was ' + process.env.EMCC_BUILD_DIR);
''')
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '--js-library', 'lib.js'], stderr=PIPE).communicate()
    self.assertContained('dir was ' + os.path.realpath(os.path.normpath(self.get_dir())), err)

  def test_float_h(self):
    process = Popen([PYTHON, EMCC, path_from_root('tests', 'float+.c')], stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    assert process.returncode is 0, 'float.h should agree with our system: ' + out + '\n\n\n' + err

  def test_default_obj_ext(self):
    outdir = os.path.join(self.get_dir(), 'out_dir') + '/'

    self.clear()
    os.mkdir(outdir)
    process = Popen([PYTHON, EMCC, '-c', path_from_root('tests', 'hello_world.c'), '-o', outdir], stderr=PIPE)
    out, err = process.communicate()
    assert not err, err
    assert os.path.isfile(outdir + 'hello_world.o')

    self.clear()
    os.mkdir(outdir)
    process = Popen([PYTHON, EMCC, '-c', path_from_root('tests', 'hello_world.c'), '-o', outdir, '--default-obj-ext', 'obj'], stderr=PIPE)
    out, err = process.communicate()
    assert not err, err
    assert os.path.isfile(outdir + 'hello_world.obj')

  def test_doublestart_bug(self):
    open('code.cpp', 'w').write(r'''
#include <stdio.h>
#include <emscripten.h>

void main_loop(void) {
    static int cnt = 0;
    if (++cnt >= 10) emscripten_cancel_main_loop();
}

int main(void) {
    printf("This should only appear once.\n");
    emscripten_set_main_loop(main_loop, 10, 0);
    return 0;
}
''')

    open('pre.js', 'w').write(r'''
if (typeof Module === 'undefined') Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()');
if (!Module['preRun']) Module['preRun'] = [];
Module["preRun"].push(function () {
    Module['addRunDependency']('test_run_dependency');
    Module['removeRunDependency']('test_run_dependency');
});
''')

    Popen([PYTHON, EMCC, 'code.cpp', '--pre-js', 'pre.js']).communicate()
    output = run_js(os.path.join(self.get_dir(), 'a.out.js'), engine=NODE_JS)

    assert output.count('This should only appear once.') == 1, '\n'+output

  def test_module_print(self):
    open('code.cpp', 'w').write(r'''
#include <stdio.h>
int main(void) {
  printf("123456789\n");
  return 0;
}
''')

    open('pre.js', 'w').write(r'''
var Module = { print: function(x) { throw '<{(' + x + ')}>' } };
''')

    Popen([PYTHON, EMCC, 'code.cpp', '--pre-js', 'pre.js']).communicate()
    output = run_js(os.path.join(self.get_dir(), 'a.out.js'), stderr=PIPE, full_output=True, engine=NODE_JS, assert_returncode=None)
    assert r'<{(123456789)}>' in output, output

  def test_precompiled_headers(self):
    for suffix in ['gch', 'pch']:
      print suffix
      self.clear()

      open('header.h', 'w').write('#define X 5\n')
      Popen([PYTHON, EMCC, '-xc++-header', 'header.h', '-c']).communicate()
      assert os.path.exists('header.h.gch') # default output is gch
      if suffix != 'gch':
        Popen([PYTHON, EMCC, '-xc++-header', 'header.h', '-o', 'header.h.' + suffix]).communicate()
        assert open('header.h.gch').read() == open('header.h.' + suffix).read()

      open('src.cpp', 'w').write(r'''
#include <stdio.h>
int main() {
  printf("|%d|\n", X);
  return 0;
}
''')
      Popen([PYTHON, EMCC, 'src.cpp', '-include', 'header.h']).communicate()

      output = run_js(self.in_dir('a.out.js'), stderr=PIPE, full_output=True, engine=NODE_JS)
      assert '|5|' in output, output

      # also verify that the gch is actually used
      err = Popen([PYTHON, EMCC, 'src.cpp', '-include', 'header.h', '-Xclang', '-print-stats'], stderr=PIPE).communicate()
      self.assertTextDataContained('*** PCH/Modules Loaded:\nModule: header.h.' + suffix, err[1])
      # and sanity check it is not mentioned when not
      try_delete('header.h.' + suffix)
      err = Popen([PYTHON, EMCC, 'src.cpp', '-include', 'header.h', '-Xclang', '-print-stats'], stderr=PIPE).communicate()
      assert '*** PCH/Modules Loaded:\nModule: header.h.' + suffix not in err[1].replace('\r\n', '\n'), err[1]

      # with specified target via -o
      try_delete('header.h.' + suffix)
      Popen([PYTHON, EMCC, '-xc++-header', 'header.h', '-o', 'my.' + suffix]).communicate()
      assert os.path.exists('my.' + suffix)

      # -include-pch flag
      Popen([PYTHON, EMCC, '-xc++-header', 'header.h', '-o', 'header.h.' + suffix]).communicate()
      check_execute([PYTHON, EMCC, 'src.cpp', '-include-pch', 'header.h.' + suffix])
      output = run_js('a.out.js')
      assert '|5|' in output, output

  def test_warn_unaligned(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>
struct packey {
  char x;
  int y;
  double z;
} __attribute__((__packed__));
int main() {
  volatile packey p;
  p.x = 0;
  p.y = 1;
  p.z = 2;
  return 0;
}
''')
    output = Popen([PYTHON, EMCC, 'src.cpp', '-s', 'WARN_UNALIGNED=1'], stderr=PIPE).communicate()
    assert 'emcc: warning: unaligned store' in output[1], output[1]
    output = Popen([PYTHON, EMCC, 'src.cpp', '-s', 'WARN_UNALIGNED=1', '-g'], stderr=PIPE).communicate()
    assert 'emcc: warning: unaligned store' in output[1], output[1]
    assert '@line 11 "src.cpp"' in output[1], output[1]

  def test_no_exit_runtime(self):
    open('code.cpp', 'w').write(r'''
#include <stdio.h>

template<int x>
struct Waste {
  Waste() {
    printf("coming around %d\n", x);
  }
  ~Waste() {
    printf("going away %d\n", x);
  }
};

Waste<1> w1;
Waste<2> w2;
Waste<3> w3;
Waste<4> w4;
Waste<5> w5;

int main(int argc, char **argv) {
  return 0;
}
''')

    for no_exit in [0, 1]:
      for opts in [[], ['-O1'], ['-O2', '-g2'], ['-O2', '-g2', '--llvm-lto', '1']]:
        print no_exit, opts
        Popen([PYTHON, EMCC] + opts + ['code.cpp', '-s', 'NO_EXIT_RUNTIME=' + str(no_exit)]).communicate()
        output = run_js(os.path.join(self.get_dir(), 'a.out.js'), stderr=PIPE, full_output=True, engine=NODE_JS)
        src = open('a.out.js').read()
        exit = 1-no_exit
        assert 'coming around' in output
        assert ('going away' in output) == exit, 'destructors should not run if no exit'
        assert ('_ZN5WasteILi2EED' in src) == exit, 'destructors should not appear if no exit'
        assert ('atexit(' in src) == exit, 'atexit should not appear or be called'

  def test_os_oz(self):
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    try:
      os.environ['EMCC_DEBUG'] = '1'
      for args, expect in [
          (['-O1'], 'LLVM opts: -O1'),
          (['-O2'], 'LLVM opts: -O3'),
          (['-Os'], 'LLVM opts: -Os'),
          (['-Oz'], 'LLVM opts: -Oz'),
          (['-O3'], 'LLVM opts: -O3'),
        ]:
        print args, expect
        output, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp')] + args, stdout=PIPE, stderr=PIPE).communicate()
        self.assertContained(expect, err)
        if '-O3' in args or '-Oz' in args or '-Os' in args:
          self.assertContained('registerizeHarder', err)
        else:
          self.assertNotContained('registerizeHarder', err)
        self.assertContained('hello, world!', run_js('a.out.js'))
    finally:
      del os.environ['EMCC_DEBUG']

  def test_oz_size(self):
    sizes = {}
    for name, args in [
        ('0', ['-o', 'dlmalloc.o']),
        ('1', ['-o', 'dlmalloc.o', '-O1']),
        ('2', ['-o', 'dlmalloc.o', '-O2']),
        ('s', ['-o', 'dlmalloc.o', '-Os']),
        ('z', ['-o', 'dlmalloc.o', '-Oz']),
        ('3', ['-o', 'dlmalloc.o', '-O3']),
        ('0c', ['-c']),
        ('1c', ['-c', '-O1']),
        ('2c', ['-c', '-O2']),
        ('sc', ['-c', '-Os']),
        ('zc', ['-c', '-Oz']),
        ('3c', ['-c', '-O3']),
      ]:
      print name, args
      self.clear()
      Popen([PYTHON, EMCC, path_from_root('system', 'lib', 'dlmalloc.c')] + args, stdout=PIPE, stderr=PIPE).communicate()
      sizes[name] = os.stat('dlmalloc.o').st_size
    print sizes
    # -c should not affect code size
    for name in ['0', '1', '2', '3', 's', 'z']:
      assert sizes[name] == sizes[name + 'c']
    opt_min = min(sizes['1'], sizes['2'], sizes['3'], sizes['s'], sizes['z'])
    opt_max = max(sizes['1'], sizes['2'], sizes['3'], sizes['s'], sizes['z'])
    assert opt_min - opt_max <= opt_max*0.1, 'opt builds are all fairly close'
    assert sizes['0'] > 1.20*opt_max, 'unopt build is quite larger'

  def test_global_inits(self):
    open('inc.h', 'w').write(r'''
#include <stdio.h>

template<int x>
struct Waste {
  int state;
  Waste() : state(10) {}
  void test(int a) {
    printf("%d\n", a + state);
  }
  ~Waste() {
    printf("going away %d\n", x);
  }
};

Waste<3> *getMore();

''')
    open('main.cpp', 'w').write(r'''
#include "inc.h"

Waste<1> mw1;
Waste<2> mw2;

int main(int argc, char **argv) {
  printf("argc: %d\n", argc);
  mw1.state += argc;
  mw2.state += argc;
  mw1.test(5);
  mw2.test(6);
  getMore()->test(0);
  return 0;
}
''')

    open('side.cpp', 'w').write(r'''
#include "inc.h"

Waste<3> sw3;

Waste<3> *getMore() {
  return &sw3;
}
''')

    for opts, has_global in [
      (['-O2', '-g'], True),
      (['-O2', '-g', '-s', 'NO_EXIT_RUNTIME=1'], False), # no-exit-runtime removes the atexits, and then globalgce can work it's magic to remove the global initializer entirely
      (['-Os', '-g'], True),
      (['-Os', '-g', '-s', 'NO_EXIT_RUNTIME=1'], False),
      (['-O2', '-g', '--llvm-lto', '1'], True),
      (['-O2', '-g', '-s', 'NO_EXIT_RUNTIME=1', '--llvm-lto', '1'], False),
    ]:
      print opts, has_global
      Popen([PYTHON, EMCC, 'main.cpp', '-c'] + opts).communicate()
      Popen([PYTHON, EMCC, 'side.cpp', '-c'] + opts).communicate()
      Popen([PYTHON, EMCC, 'main.o', 'side.o'] + opts).communicate()
      output = run_js(os.path.join(self.get_dir(), 'a.out.js'), stderr=PIPE, full_output=True, engine=NODE_JS)
      src = open('a.out.js').read()
      self.assertContained('argc: 1\n16\n17\n10\n', run_js('a.out.js'))
      assert ('_GLOBAL_' in src) == has_global

  def test_implicit_func(self):
    open('src.c', 'w').write(r'''
#include <stdio.h>
int main()
{
    printf("hello %d\n", strnlen("waka", 2)); // Implicit declaration, no header, for strnlen
    int (*my_strnlen)(char*, ...) = strnlen;
    printf("hello %d\n", my_strnlen("shaka", 2));
    return 0;
}
''')

    IMPLICIT_WARNING = '''warning: implicit declaration of function 'strnlen' is invalid in C99'''
    IMPLICIT_ERROR = '''error: implicit declaration of function 'strnlen' is invalid in C99'''

    for opts, expected, compile_expected in [
      ([], None, [IMPLICIT_ERROR]),
      (['-Wno-error=implicit-function-declaration'], ['hello '], [IMPLICIT_WARNING]), # turn error into warning
      (['-Wno-implicit-function-declaration'], ['hello '], []), # turn error into nothing at all (runtime output is incorrect)
    ]:
      print opts, expected
      try_delete('a.out.js')
      stdout, stderr = Popen([PYTHON, EMCC, 'src.c'] + opts, stderr=PIPE).communicate()
      for ce in compile_expected + ['''warning: incompatible pointer types''']:
        self.assertContained(ce, stderr)
      if expected is None:
        assert not os.path.exists('a.out.js')
      else:
        output = run_js(os.path.join(self.get_dir(), 'a.out.js'), stderr=PIPE, full_output=True)
        for e in expected:
          self.assertContained(e, output)

  def test_incorrect_static_call(self):
    for opts in [0, 1]:
      for asserts in [0, 1]:
        extra = []
        if opts != 1-asserts: extra = ['-s', 'ASSERTIONS=' + str(asserts)]
        cmd = [PYTHON, EMCC, path_from_root('tests', 'sillyfuncast2_noasm.ll'), '-O' + str(opts)] + extra
        print cmd
        stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
        assert ('''unexpected number of arguments 3 in call to 'doit', should be 2''' in stderr) == asserts, stderr
        assert ('''unexpected return type i32 in call to 'doit', should be void''' in stderr) == asserts, stderr
        assert ('''unexpected argument type float at index 1 in call to 'doit', should be i32''' in stderr) == asserts, stderr

  def test_llvm_lit(self):
    llvm_src = get_fastcomp_src_dir()
    LLVM_LIT = os.path.join(LLVM_ROOT, 'llvm-lit.py')
    if not os.path.exists(LLVM_LIT):
      LLVM_LIT = os.path.join(LLVM_ROOT, 'llvm-lit')
      if not os.path.exists(LLVM_LIT):
        raise Exception('cannot find llvm-lit tool')
    cmd = [PYTHON, LLVM_LIT, '-v', os.path.join(llvm_src, 'test', 'CodeGen', 'JS')]
    print cmd
    p = Popen(cmd)
    p.communicate()
    assert p.returncode == 0, 'LLVM tests must pass with exit code 0'

  def test_odin_validation(self):
    if not SPIDERMONKEY_ENGINE or SPIDERMONKEY_ENGINE not in JS_ENGINES: return self.skip('this test tests asm.js validation in SpiderMonkey')
    Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O1'], stdout=PIPE, stderr=PIPE).communicate()
    output = run_js('a.out.js', stderr=PIPE, full_output=True, engine=SPIDERMONKEY_ENGINE)
    assert 'asm.js' in output, 'spidermonkey should mention asm.js compilation: ' + output

  def test_bad_triple(self):
    Popen([CLANG, path_from_root('tests', 'hello_world.c'), '-c', '-emit-llvm', '-o', 'a.bc'] + get_clang_native_args(), env=get_clang_native_env(), stdout=PIPE, stderr=PIPE).communicate()
    out, err = Popen([PYTHON, EMCC, 'a.bc'], stdout=PIPE, stderr=PIPE).communicate()
    assert 'warning' in err or 'WARNING' in err, err
    assert 'incorrect target triple' in err or 'different target triples' in err, err

  def test_valid_abspath(self):
    # Test whether abspath warning appears
    abs_include_path = os.path.abspath(self.get_dir())
    process = Popen([PYTHON, EMCC, '-I%s' % abs_include_path, '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    warning = '-I or -L of an absolute path "-I%s" encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript).' % abs_include_path
    assert(warning in err)

    # Passing an absolute path to a directory inside the emscripten tree is always ok and should not issue a warning.
    abs_include_path = path_from_root('tests')
    process = Popen([PYTHON, EMCC, '-I%s' % abs_include_path, '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    warning = '-I or -L of an absolute path "-I%s" encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript).' % abs_include_path
    assert(warning not in err)

    # Hide warning for this include path
    process = Popen([PYTHON, EMCC, '--valid-abspath', abs_include_path,'-I%s' % abs_include_path, '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.c')], stdout=PIPE, stderr=PIPE)
    out, err = process.communicate()
    assert(warning not in err)

  def test_warn_dylibs(self):
    shared_suffixes = ['.so', '.dylib', '.dll']

    for suffix in ['.o', '.a', '.bc', '.so', '.lib', '.dylib', '.js', '.html']:
      process = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-o', 'out' + suffix], stdout=PIPE, stderr=PIPE)
      out, err = process.communicate()
      warning = 'Dynamic libraries (.so, .dylib, .dll) are currently not supported by Emscripten'
      if suffix in shared_suffixes:
        assert(warning in err)
      else:
        assert(warning not in err)

  def test_simplify_ifs(self):
    def test(src, nums):
      open('src.c', 'w').write(src)
      for opts, ifs in [
        [['-g2'], nums[0]],
        [['-profiling'], nums[1]], # Test that for compatibility support, both forms '-profiling' and '--profiling' are accepted. (https://github.com/kripken/emscripten/issues/2679)
        [['--profiling'], nums[1]],
        [['--profiling', '-g2'], nums[2]]
      ]:
        print opts, ifs
        if type(ifs) == int: ifs = [ifs]
        try_delete('a.out.js')
        Popen([PYTHON, EMCC, 'src.c', '-O2'] + opts, stdout=PIPE).communicate()
        src = open('a.out.js').read()
        main = src[src.find('function _main'):src.find('\n}', src.find('function _main'))]
        actual_ifs = main.count('if (')
        assert actual_ifs in ifs, main + ' : ' + str([ifs, actual_ifs])
        #print main

    test(r'''
      #include <stdio.h>
      #include <string.h>
      int main(int argc, char **argv) {
        if (argc > 5 && strlen(argv[0]) > 1 && strlen(argv[1]) > 2) printf("halp");
        return 0;
      }
    ''', [3, 1, 1])

    test(r'''
      #include <stdio.h>
      #include <string.h>
      int main(int argc, char **argv) {
        while (argc % 3 == 0) {
          if (argc > 5 && strlen(argv[0]) > 1 && strlen(argv[1]) > 2) {
            printf("halp");
            argc++;
          } else {
            while (argc > 0) {
              printf("%d\n", argc--);
            }
          }
        }
        return 0;
      }
    ''', [8, [5,7], [5,7]])

    test(r'''
      #include <stdio.h>
      #include <string.h>
      int main(int argc, char **argv) {
        while (argc % 17 == 0) argc *= 2;
        if (argc > 5 && strlen(argv[0]) > 10 && strlen(argv[1]) > 20) {
          printf("halp");
          argc++;
        } else {
          printf("%d\n", argc--);
        }
        while (argc % 17 == 0) argc *= 2;
        return argc;
      }
    ''', [6, 3, 3])

    test(r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main(int argc, char *argv[]) {
        if (getenv("A") && getenv("B")) {
            printf("hello world\n");
        } else {
            printf("goodnight moon\n");
        }
        printf("and that's that\n");
        return 0;
      }
    ''', [[3,2], 1, 1])

    test(r'''
      #include <stdio.h>
      #include <stdlib.h>

      int main(int argc, char *argv[]) {
        if (getenv("A") || getenv("B")) {
            printf("hello world\n");
        }
        printf("and that's that\n");
        return 0;
      }
    ''', [[3,2], 1, 1])

  def test_symbol_map(self):
    for m in [0, 1]:
      self.clear()
      cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O2']
      if m: cmd += ['--emit-symbol-map']
      print cmd
      stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
      assert ('''wrote symbol map file''' in stderr) == m, stderr
      assert (os.path.exists('a.out.js.symbols') == m), stderr
      if m:
        symbols = open('a.out.js.symbols').read()
        assert ':_main' in symbols

  def test_bc_to_bc(self):
    # emcc should 'process' bitcode to bitcode. build systems can request this if
    # e.g. they assume our 'executable' extension is bc, and compile an .o to a .bc
    # (the user would then need to build bc to js of course, but we need to actually
    # emit the bc)
    cmd = Popen([PYTHON, EMCC, '-c', path_from_root('tests', 'hello_world.c')]).communicate()
    assert os.path.exists('hello_world.o')
    cmd = Popen([PYTHON, EMCC, 'hello_world.o', '-o', 'hello_world.bc']).communicate()
    assert os.path.exists('hello_world.o')
    assert os.path.exists('hello_world.bc')

  def test_bad_function_pointer_cast(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>

typedef int (*callback) (int, ...);

int impl(int foo) {
  printf("Hello, world.\n");
  return 0;
}

int main() {
  volatile callback f = (callback) impl;
  f(0); /* This fails with or without additional arguments. */
  return 0;
}
''')

    for opts in [0, 1, 2]:
      for safe in [0, 1]:
        for emulate_casts in [0, 1]:
          for emulate_fps in [0, 1]:
            for relocate in [0, 1]:
              cmd = [PYTHON, EMCC, 'src.cpp', '-O' + str(opts), '-s', 'SAFE_HEAP=' + str(safe)]
              if emulate_casts:
                cmd += ['-s', 'EMULATE_FUNCTION_POINTER_CASTS=1']
              if emulate_fps:
                cmd += ['-s', 'EMULATED_FUNCTION_POINTERS=1']
              if relocate:
                cmd += ['-s', 'RELOCATABLE=1'] # disables asm-optimized safe heap
              print cmd
              Popen(cmd).communicate()
              output = run_js('a.out.js', stderr=PIPE, full_output=True, assert_returncode=None)
              if emulate_casts:
                assert 'Hello, world.' in output, output
              elif safe:
                assert 'Function table mask error' in output, output
              else:
                if opts == 0:
                  assert 'Invalid function pointer called' in output, output
                else:
                  assert 'abort()' in output, output

  def test_aliased_func_pointers(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>

int impl1(int foo) { return foo; }
float impla(float foo) { return foo; }
int impl2(int foo) { return foo+1; }
float implb(float foo) { return foo+1; }
int impl3(int foo) { return foo+2; }
float implc(float foo) { return foo+2; }

int main(int argc, char **argv) {
  volatile void *f = (void*)impl1;
  if (argc == 50) f = (void*)impla;
  if (argc == 51) f = (void*)impl2;
  if (argc == 52) f = (void*)implb;
  if (argc == 53) f = (void*)impl3;
  if (argc == 54) f = (void*)implc;
  return (int)f;
}
''')

    print 'aliasing'

    sizes_ii = {}
    sizes_dd = {}

    for alias in [None, 0, 1]:
      cmd = [PYTHON, EMCC, 'src.cpp', '-O1']
      if alias is not None:
        cmd += ['-s', 'ALIASING_FUNCTION_POINTERS=' + str(alias)]
      else:
        alias = -1
      print cmd
      Popen(cmd).communicate()
      src = open('a.out.js').read().split('\n')
      for line in src:
        if line.strip().startswith('var FUNCTION_TABLE_ii = '):
          sizes_ii[alias] = line.count(',')
        if line.strip().startswith('var FUNCTION_TABLE_dd = '):
          sizes_dd[alias] = line.count(',')

    print 'ii', sizes_ii
    print 'dd', sizes_dd

    for sizes in [sizes_ii, sizes_dd]:
      assert sizes[-1] == sizes[1] # default is to alias
      assert sizes[1] < sizes[0] # without aliasing, we have more unique values and fat tables

  def test_bad_export(self):
    for m in ['', ' ']:
      self.clear()
      cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'EXPORTED_FUNCTIONS=["' + m + '_main"]']
      print cmd
      stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
      if m:
        assert 'function requested to be exported, but not implemented: " _main"' in stderr, stderr
      else:
        self.assertContained('hello, world!', run_js('a.out.js'))

  def test_no_dynamic_execution(self):
    cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O1', '-s', 'NO_DYNAMIC_EXECUTION=1']
    stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
    self.assertContained('hello, world!', run_js('a.out.js'))
    src = open('a.out.js').read()
    assert 'eval(' not in src
    assert 'eval.' not in src
    assert 'new Function' not in src
    try_delete('a.out.js')

    # Test that --preload-file doesn't add an use of eval().
    cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O1', '-s', 'NO_DYNAMIC_EXECUTION=1', '--preload-file', 'temp.txt']
    stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
    self.assertContained('hello, world!', run_js('a.out.js'))
    src = open('a.out.js').read()
    assert 'eval(' not in src
    assert 'eval.' not in src
    assert 'new Function' not in src
    try_delete('a.out.js')

    # Test that -s NO_DYNAMIC_EXECUTION=1 and --closure 1 are not allowed together.
    cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O1', '-s', 'NO_DYNAMIC_EXECUTION=1', '--closure', '1']
    proc = Popen(cmd, stderr=PIPE)
    proc.communicate()
    assert proc.returncode != 0
    try_delete('a.out.js')

    # Test that -s NO_DYNAMIC_EXECUTION=1 and -s RELOCATABLE=1 are not allowed together.
    cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O1', '-s', 'NO_DYNAMIC_EXECUTION=1', '-s', 'RELOCATABLE=1']
    proc = Popen(cmd, stderr=PIPE)
    proc.communicate()
    assert proc.returncode != 0
    try_delete('a.out.js')

    open('test.c', 'w').write(r'''
      #include <emscripten/emscripten.h>
      int main() {
        emscripten_run_script("console.log('hello from script');");
        return 0;
      }
      ''')

    # Test that emscripten_run_script() aborts when -s NO_DYNAMIC_EXECUTION=1
    Popen([PYTHON, EMCC, 'test.c', '-O1', '-s', 'NO_DYNAMIC_EXECUTION=1']).communicate()
    self.assertContained('NO_DYNAMIC_EXECUTION=1 was set, cannot eval', run_js(os.path.join(self.get_dir(), 'a.out.js'), assert_returncode=None, full_output=True, stderr=PIPE))
    try_delete('a.out.js')

    # Test that emscripten_run_script() posts a warning when -s NO_DYNAMIC_EXECUTION=2
    Popen([PYTHON, EMCC, 'test.c', '-O1', '-s', 'NO_DYNAMIC_EXECUTION=2']).communicate()
    self.assertContained('Warning: NO_DYNAMIC_EXECUTION=2 was set, but calling eval in the following location:', run_js(os.path.join(self.get_dir(), 'a.out.js'), assert_returncode=None, full_output=True, stderr=PIPE))
    self.assertContained('hello from script', run_js(os.path.join(self.get_dir(), 'a.out.js'), assert_returncode=None, full_output=True, stderr=PIPE))
    try_delete('a.out.js')

  def test_init_file_at_offset(self):
    open('src.cpp', 'w').write(r'''
      #include <stdio.h>
      int main() {
        int data = 0x12345678;
        FILE *f = fopen("test.dat", "wb");
        fseek(f, 100, SEEK_CUR);
        fwrite(&data, 4, 1, f);
        fclose(f);

        int data2;
        f = fopen("test.dat", "rb");
        fread(&data2, 4, 1, f); // should read 0s, not that int we wrote at an offset
        printf("read: %d\n", data2);
        fseek(f, 0, SEEK_END);
        long size = ftell(f); // should be 104, not 4
        fclose(f);
        printf("file size is %d\n", size);
      }
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('read: 0\nfile size is 104\n', run_js('a.out.js'))

  def test_argv0_node(self):
    open('code.cpp', 'w').write(r'''
#include <stdio.h>
int main(int argc, char **argv) {
  printf("I am %s.\n", argv[0]);
  return 0;
}
''')

    Popen([PYTHON, EMCC, 'code.cpp']).communicate()
    self.assertContained('I am ' + os.path.realpath(self.get_dir()).replace('\\', '/') + '/a.out.js', run_js('a.out.js', engine=NODE_JS).replace('\\', '/'))

  def test_returncode(self):
    open('src.cpp', 'w').write(r'''
      #include <stdio.h>
      int main() {
        return 123;
      }
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    for engine in JS_ENGINES:
      print engine
      process = Popen(engine + ['a.out.js'], stdout=PIPE, stderr=PIPE)
      output = process.communicate()
      assert process.returncode == 123, process.returncode

  def test_mkdir_silly(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("\n");
  for (int i = 1; i < argc; i++) {
    printf("%d:\n", i);
    int ok = mkdir(argv[i], S_IRWXU|S_IRWXG|S_IRWXO);
    printf("  make %s: %d\n", argv[i], ok);
    DIR *dir = opendir(argv[i]);
    printf("  open %s: %d\n", argv[i], dir != NULL);
    if (dir) {
      struct dirent *entry;
      while ((entry = readdir(dir))) {
        printf("  %s, %d\n", entry->d_name, entry->d_type);
      }
    }
  }
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    # cannot create /, can open
    self.assertContained(r'''
1:
  make /: -1
  open /: 1
  proc, 4
  dev, 4
  home, 4
  tmp, 4
  .., 4
  ., 4
''', run_js('a.out.js', args=['/']))
    # cannot create empty name, cannot open
    self.assertContained(r'''
1:
  make : -1
  open : 0
''', run_js('a.out.js', args=['']))
    # can create unnormalized path, can open
    self.assertContained(r'''
1:
  make /a//: 0
  open /a//: 1
  .., 4
  ., 4
''', run_js('a.out.js', args=['/a//']))
    # can create child unnormalized
    self.assertContained(r'''
1:
  make /a: 0
  open /a: 1
  .., 4
  ., 4
2:
  make /a//b//: 0
  open /a//b//: 1
  .., 4
  ., 4
''', run_js('a.out.js', args=['/a', '/a//b//']))

  def test_stat_silly(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    const char *path = argv[i];
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
      printf("Failed to stat path: %s; errno=%d\n", path, errno);
    } else {
      printf("ok on %s\n", path);
    }
  }
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    # cannot stat ""
    self.assertContained(r'''Failed to stat path: /a; errno=2
Failed to stat path: ; errno=2
''', run_js('a.out.js', args=['/a', '']))

  def test_symlink_silly(self):
    open('src.cpp', 'w').write(r'''
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
  if (symlink(argv[1], argv[2]) != 0) {
    printf("Failed to symlink paths: %s, %s; errno=%d\n", argv[1], argv[2], errno);
  } else {
    printf("ok\n");
  }
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    # cannot symlink nonexistents
    self.assertContained(r'''Failed to symlink paths: , abc; errno=2''', run_js('a.out.js', args=['', 'abc']))
    self.assertContained(r'''Failed to symlink paths: , ; errno=2''', run_js('a.out.js', args=['', '']))
    self.assertContained(r'''ok''', run_js('a.out.js', args=['123', 'abc']))
    self.assertContained(r'''Failed to symlink paths: abc, ; errno=2''', run_js('a.out.js', args=['abc', '']))

  def test_rename_silly(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
  if (rename(argv[1], argv[2]) != 0) {
    printf("Failed to rename paths: %s, %s; errno=%d\n", argv[1], argv[2], errno);
  } else {
    printf("ok\n");
  }
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    # cannot symlink nonexistents
    self.assertContained(r'''Failed to rename paths: , abc; errno=2''', run_js('a.out.js', args=['', 'abc']))
    self.assertContained(r'''Failed to rename paths: , ; errno=2''', run_js('a.out.js', args=['', '']))
    self.assertContained(r'''Failed to rename paths: 123, abc; errno=2''', run_js('a.out.js', args=['123', 'abc']))
    self.assertContained(r'''Failed to rename paths: abc, ; errno=2''', run_js('a.out.js', args=['abc', '']))

  def test_readdir_r_silly(self):
    open('src.cpp', 'w').write(r'''
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
using std::endl;
namespace
{
  void check(const bool result)
  {
    if(not result) {
      std::cout << "Check failed!" << endl;
      throw "bad";
    }
  }
  // Do a recursive directory listing of the directory whose path is specified
  // by \a name.
  void ls(const std::string& name, std::size_t indent = 0)
  {
    ::DIR *dir;
    struct ::dirent *entry;
    if(indent == 0) {
      std::cout << name << endl;
      ++indent;
    }
    // Make sure we can open the directory.  This should also catch cases where
    // the empty string is passed in.
    if (not (dir = ::opendir(name.c_str()))) {
      const int error = errno;
      std::cout
        << "Failed to open directory: " << name << "; " << error << endl;
      return;
    }
    // Just checking the sanity.
    if (name.empty()) {
      std::cout
        << "Managed to open a directory whose name was the empty string.."
        << endl;
      check(::closedir(dir) != -1);
      return;
    }
    // Iterate over the entries in the directory.
    while ((entry = ::readdir(dir))) {
      const std::string entryName(entry->d_name);
      if (entryName == "." || entryName == "..") {
        // Skip the dot entries.
        continue;
      }
      const std::string indentStr(indent * 2, ' ');
      if (entryName.empty()) {
        std::cout
          << indentStr << "\"\": Found empty string as a "
          << (entry->d_type == DT_DIR ? "directory" : "file")
          << " entry!" << endl;
        continue;
      } else {
        std::cout << indentStr << entryName
                  << (entry->d_type == DT_DIR ? "/" : "") << endl;
      }
      if (entry->d_type == DT_DIR) {
        // We found a subdirectory; recurse.
        ls(std::string(name + (name == "/" ? "" : "/" ) + entryName),
           indent + 1);
      }
    }
    // Close our handle.
    check(::closedir(dir) != -1);
  }
  void touch(const std::string &path)
  {
    const int fd = ::open(path.c_str(), O_CREAT | O_TRUNC, 0644);
    check(fd != -1);
    check(::close(fd) != -1);
  }
}
int main()
{
  check(::mkdir("dir", 0755) == 0);
  touch("dir/a");
  touch("dir/b");
  touch("dir/c");
  touch("dir/d");
  touch("dir/e");
  std::cout << "Before:" << endl;
  ls("dir");
  std::cout << endl;
  // Attempt to delete entries as we walk the (single) directory.
  ::DIR * const dir = ::opendir("dir");
  check(dir != NULL);
  struct ::dirent *entry;
  while((entry = ::readdir(dir)) != NULL) {
    const std::string name(entry->d_name);
    // Skip "." and "..".
    if(name == "." || name == "..") {
      continue;
    }
    // Unlink it.
    std::cout << "Unlinking " << name << endl;
    check(::unlink(("dir/" + name).c_str()) != -1);
  }
  check(::closedir(dir) != -1);
  std::cout << "After:" << endl;
  ls("dir");
  std::cout << endl;
  return 0;
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    # cannot symlink nonexistents
    self.assertContained(r'''Before:
dir
  e
  d
  c
  b
  a

Unlinking e
Unlinking d
Unlinking c
Unlinking b
Unlinking a
After:
dir
''', run_js('a.out.js', args=['', 'abc']))

  def test_emversion(self):
    open('src.cpp', 'w').write(r'''
      #include <stdio.h>
      int main() {
        printf("major: %d\n", __EMSCRIPTEN_major__);
        printf("minor: %d\n", __EMSCRIPTEN_minor__);
        printf("tiny: %d\n", __EMSCRIPTEN_tiny__);
      }
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained(r'''major: %d
minor: %d
tiny: %d
''' % (EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY), run_js('a.out.js'))

  def test_dashE(self):
    open('src.cpp', 'w').write(r'''#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE __EMSCRIPTEN_major__ __EMSCRIPTEN_minor__ __EMSCRIPTEN_tiny__ EMSCRIPTEN_KEEPALIVE
''')
    def test(args=[]):
      print args
      out = Popen([PYTHON, EMCC, 'src.cpp', '-E'] + args, stdout=PIPE).communicate()[0]
      self.assertContained(r'''__attribute__((used)) %d %d %d __attribute__((used))''' % (EMSCRIPTEN_VERSION_MAJOR, EMSCRIPTEN_VERSION_MINOR, EMSCRIPTEN_VERSION_TINY), out)
    test()
    test(['--bind'])

  def test_dashE_consistent(self): # issue #3365
    normal = Popen([PYTHON, EMXX, '-v', '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.cpp'), '-c'], stdout=PIPE, stderr=PIPE).communicate()[1]
    dash_e = Popen([PYTHON, EMXX, '-v', '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.cpp'), '-E'], stdout=PIPE, stderr=PIPE).communicate()[1]

    import difflib
    diff = [a.rstrip()+'\n' for a in difflib.unified_diff(normal.split('\n'), dash_e.split('\n'), fromfile='normal', tofile='dash_e')]
    left_std = filter(lambda x: x.startswith('-') and '-std=' in x, diff)
    right_std = filter(lambda x: x.startswith('+') and '-std=' in x, diff)
    assert len(left_std) == len(right_std) == 1, '\n\n'.join(diff)
    bad = filter(lambda x: '-Wwarn-absolute-paths' in x, diff)
    assert len(bad) == 0, '\n\n'.join(diff)

  def test_dashE_respect_dashO(self): # issue #3365
    with_dash_o = Popen([PYTHON, EMXX, path_from_root('tests', 'hello_world.cpp'), '-E', '-o', '/dev/null'], stdout=PIPE, stderr=PIPE).communicate()[0]
    without_dash_o = Popen([PYTHON, EMXX, path_from_root('tests', 'hello_world.cpp'), '-E'], stdout=PIPE, stderr=PIPE).communicate()[0]
    assert len(with_dash_o) == 0
    assert len(without_dash_o) != 0

  def test_dashM(self):
    out = Popen([PYTHON, EMXX, path_from_root('tests', 'hello_world.cpp'), '-M'], stdout=PIPE).communicate()[0]
    self.assertContained('hello_world.o:', out) # Verify output is just a dependency rule instead of bitcode or js

  def test_dashM_consistent(self):
    normal = Popen([PYTHON, EMXX, '-v', '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.cpp'), '-c'], stdout=PIPE, stderr=PIPE).communicate()[1]
    dash_m = Popen([PYTHON, EMXX, '-v', '-Wwarn-absolute-paths', path_from_root('tests', 'hello_world.cpp'), '-M'], stdout=PIPE, stderr=PIPE).communicate()[1]

    import difflib
    diff = [a.rstrip()+'\n' for a in difflib.unified_diff(normal.split('\n'), dash_m.split('\n'), fromfile='normal', tofile='dash_m')]
    left_std = filter(lambda x: x.startswith('-') and '-std=' in x, diff)
    right_std = filter(lambda x: x.startswith('+') and '-std=' in x, diff)
    assert len(left_std) == len(right_std) == 1, '\n\n'.join(diff)
    bad = filter(lambda x: '-Wwarn-absolute-paths' in x, diff)
    assert len(bad) == 0, '\n\n'.join(diff)

  def test_dashM_respect_dashO(self):
    with_dash_o = Popen([PYTHON, EMXX, path_from_root('tests', 'hello_world.cpp'), '-M', '-o', '/dev/null'], stdout=PIPE, stderr=PIPE).communicate()[0]
    without_dash_o = Popen([PYTHON, EMXX, path_from_root('tests', 'hello_world.cpp'), '-M'], stdout=PIPE, stderr=PIPE).communicate()[0]
    assert len(with_dash_o) == 0
    assert len(without_dash_o) != 0

  def test_malloc_implicit(self):
    open('src.cpp', 'w').write(r'''
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
int main() {
  const char *home = getenv("HOME");
  for(unsigned int i = 0; i < 5; ++i) {
    const char *curr = getenv("HOME");
    assert(curr == home);
  }
  printf("ok\n");
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('ok', run_js('a.out.js'))

  def test_switch64phi(self):
    # issue 2539, fastcomp segfault on phi-i64 interaction
    self.clear()
    open('src.cpp', 'w').write(r'''
#include <cstdint>
#include <limits>
#include <cstdio>

//============================================================================

namespace
{
  class int_adapter {
  public:
    typedef ::int64_t int_type;

    int_adapter(int_type v = 0)
      : value_(v)
    {}
    static const int_adapter pos_infinity()
    {
      return (::std::numeric_limits<int_type>::max)();
    }
    static const int_adapter neg_infinity()
    {
      return (::std::numeric_limits<int_type>::min)();
    }
    static const int_adapter not_a_number()
    {
      return (::std::numeric_limits<int_type>::max)()-1;
    }
    static bool is_neg_inf(int_type v)
    {
      return (v == neg_infinity().as_number());
    }
    static bool is_pos_inf(int_type v)
    {
      return (v == pos_infinity().as_number());
    }
    static bool is_not_a_number(int_type v)
    {
      return (v == not_a_number().as_number());
    }

    bool is_infinity() const
    {
      return (value_ == neg_infinity().as_number() ||
              value_ == pos_infinity().as_number());
    }
    bool is_special() const
    {
      return(is_infinity() || value_ == not_a_number().as_number());
    }
    bool operator<(const int_adapter& rhs) const
    {
      if(value_ == not_a_number().as_number()
         || rhs.value_ == not_a_number().as_number()) {
        return false;
      }
      if(value_ < rhs.value_) return true;
      return false;
    }
    int_type as_number() const
    {
      return value_;
    }

    int_adapter operator-(const int_adapter& rhs)const
    {
      if(is_special() || rhs.is_special())
      {
        if (rhs.is_pos_inf(rhs.as_number()))
        {
          return int_adapter(1);
        }
        if (rhs.is_neg_inf(rhs.as_number()))
        {
          return int_adapter();
        }
      }
      return int_adapter();
    }


  private:
    int_type value_;
  };

  class time_iterator {
  public:
    time_iterator(int_adapter t, int_adapter d)
      : current_(t),
        offset_(d)
    {}

    time_iterator& operator--()
    {
      current_ = int_adapter(current_ - offset_);
      return *this;
    }

    bool operator>=(const int_adapter& t)
    {
      return not (current_ < t);
    }

  private:
    int_adapter current_;
    int_adapter offset_;
  };

  void iterate_backward(const int_adapter *answers, const int_adapter& td)
  {
    int_adapter end = answers[0];
    time_iterator titr(end, td);

    std::puts("");
    for (; titr >= answers[0]; --titr) {
    }
  }
}

int
main()
{
  const int_adapter answer1[] = {};
  iterate_backward(NULL, int_adapter());
  iterate_backward(answer1, int_adapter());
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp', '-O2', '-s', 'SAFE_HEAP=1']).communicate()
    assert os.path.exists('a.out.js') # build should succeed
    self.assertContained(('trap!', 'segmentation fault loading 4 bytes from address 0'), run_js('a.out.js', assert_returncode=None, stderr=PIPE)) # program should segfault

  def test_only_force_stdlibs(self):
    def test(name):
      print name
      Popen([PYTHON, EMXX, path_from_root('tests', 'hello_libcxx.cpp')], stderr=PIPE).communicate()
      self.assertContained('hello, world!', run_js('a.out.js', stderr=PIPE))

    test('normal') # normally is ok

    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc,libcxxabi,libcxx'
      test('forced libs is ok, they were there anyhow')
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc'
      test('partial list, but ok since we grab them as needed')
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc'
      os.environ['EMCC_ONLY_FORCED_STDLIBS'] = '1'
      ok = False
      test('fail! not enough stdlibs')
    except:
      ok = True
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']
      del os.environ['EMCC_ONLY_FORCED_STDLIBS']
    assert ok

    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc,libcxxabi,libcxx'
      os.environ['EMCC_ONLY_FORCED_STDLIBS'] = '1'
      test('force all the needed stdlibs, so this works even though we ignore the input file')
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']
      del os.environ['EMCC_ONLY_FORCED_STDLIBS']

  def test_only_force_stdlibs_2(self):
    open('src.cpp', 'w').write(r'''
#include <iostream>
#include <stdexcept>

int main()
{
  try {
    throw std::exception();
    std::cout << "got here" << std::endl;
  }
  catch (const std::exception& ex) {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  }
}
''')
    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc,libcxxabi,libcxx'
      os.environ['EMCC_ONLY_FORCED_STDLIBS'] = '1'
      Popen([PYTHON, EMXX, 'src.cpp']).communicate()
      self.assertContained('Caught exception: std::exception', run_js('a.out.js', stderr=PIPE))
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']
      del os.environ['EMCC_ONLY_FORCED_STDLIBS']

  def test_strftime_zZ(self):
    open('src.cpp', 'w').write(r'''
#include <cerrno>
#include <cstring>
#include <ctime>
#include <iostream>

int main()
{
  // Buffer to hold the current hour of the day.  Format is HH + nul
  // character.
  char hour[3];

  // Buffer to hold our ISO 8601 formatted UTC offset for the current
  // timezone.  Format is [+-]hhmm + nul character.
  char utcOffset[6];

  // Buffer to hold the timezone name or abbreviation.  Just make it
  // sufficiently large to hold most timezone names.
  char timezone[128];

  std::tm tm;

  // Get the current timestamp.
  const std::time_t now = std::time(NULL);

  // What time is that here?
  if (::localtime_r(&now, &tm) == NULL) {
    const int error = errno;
    std::cout
      << "Failed to get localtime for timestamp=" << now << "; errno=" << error
      << "; " << std::strerror(error) << std::endl;
    return 1;
  }

  size_t result = 0;

  // Get the formatted hour of the day.
  if ((result = std::strftime(hour, 3, "%H", &tm)) != 2) {
    const int error = errno;
    std::cout
      << "Failed to format hour for timestamp=" << now << "; result="
      << result << "; errno=" << error << "; " << std::strerror(error)
      << std::endl;
    return 1;
  }
  std::cout << "The current hour of the day is: " << hour << std::endl;

  // Get the formatted UTC offset in ISO 8601 format.
  if ((result = std::strftime(utcOffset, 6, "%z", &tm)) != 5) {
    const int error = errno;
    std::cout
      << "Failed to format UTC offset for timestamp=" << now << "; result="
      << result << "; errno=" << error << "; " << std::strerror(error)
      << std::endl;
    return 1;
  }
  std::cout << "The current timezone offset is: " << utcOffset << std::endl;

  // Get the formatted timezone name or abbreviation.  We don't know how long
  // this will be, so just expect some data to be written to the buffer.
  if ((result = std::strftime(timezone, 128, "%Z", &tm)) == 0) {
    const int error = errno;
    std::cout
      << "Failed to format timezone for timestamp=" << now << "; result="
      << result << "; errno=" << error << "; " << std::strerror(error)
      << std::endl;
    return 1;
  }
  std::cout << "The current timezone is: " << timezone << std::endl;

  std::cout << "ok!\n";
}
''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('ok!', run_js('a.out.js'))

  def test_truncate_from_0(self):
    open('src.cpp', 'w').write(r'''
#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::endl;

//============================================================================
// :: Helpers

namespace
{
  // Returns the size of the regular file specified as 'path'.
  ::off_t getSize(const char* const path)
  {
    // Stat the file and make sure that it's the expected size.
    struct ::stat path_stat;
    if (::stat(path, &path_stat) != 0) {
      const int error = errno;
      std::cout
        << "Failed to lstat path: " << path << "; errno=" << error << "; "
        << std::strerror(error) << endl;
      return -1;
    }

    std::cout
      << "Size of file is: " << path_stat.st_size << endl;
    return path_stat.st_size;
  }

  // Causes the regular file specified in 'path' to have a size of 'length'
  // bytes.
  void resize(const char* const path,
              const ::off_t length)
  {
    std::cout
      << "Truncating file=" << path << " to length=" << length << endl;
    if (::truncate(path, length) == -1)
    {
      const int error = errno;
      std::cout
        << "Failed to truncate file=" << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
    }

    const ::off_t size = getSize(path);
    if (size != length) {
      std::cout
        << "Failed to truncate file=" << path << " to length=" << length
        << "; got size=" << size << endl;
    }
  }

  // Helper to create a file with the given content.
  void createFile(const std::string& path, const std::string& content)
  {
    std::cout
      << "Creating file: " << path << " with content=" << content << endl;

    const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
      const int error = errno;
      std::cout
        << "Failed to open file for writing: " << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      return;
    }

    if (::write(fd, content.c_str(), content.size()) != content.size()) {
      const int error = errno;
      std::cout
        << "Failed to write content=" << content << " to file=" << path
        << "; errno=" << error << "; " << std::strerror(error) << endl;

      // Fall through to close FD.
    }

    ::close(fd);
  }
}

//============================================================================
// :: Entry Point
int main()
{
  const char* const file = "/tmp/file";
  createFile(file, "This is some content");
  getSize(file);
  resize(file, 32);
  resize(file, 17);
  resize(file, 0);

  // This throws a JS exception.
  resize(file, 32);
  return 0;
}
''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained(r'''Creating file: /tmp/file with content=This is some content
Size of file is: 20
Truncating file=/tmp/file to length=32
Size of file is: 32
Truncating file=/tmp/file to length=17
Size of file is: 17
Truncating file=/tmp/file to length=0
Size of file is: 0
Truncating file=/tmp/file to length=32
Size of file is: 32
''', run_js('a.out.js'))

  def test_emcc_s_typo(self):
    # with suggestions
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'DISABLE_EXCEPTION_CATCH=1'], stderr=PIPE).communicate()
    self.assertContained(r'''Assigning a non-existent settings attribute "DISABLE_EXCEPTION_CATCH"''', err)
    self.assertContained(r'''did you mean one of DISABLE_EXCEPTION_CATCHING?''', err)
    # no suggestions
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'CHEEZ=1'], stderr=PIPE).communicate()
    self.assertContained(r'''perhaps a typo in emcc's  -s X=Y  notation?''', err)
    self.assertContained(r'''(see src/settings.js for valid values)''', err)

  def test_create_readonly(self):
    open('src.cpp', 'w').write(r'''
#include <cerrno>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

using std::endl;

//============================================================================
// :: Helpers

namespace
{
  // Helper to create a read-only file with content.
  void readOnlyFile(const std::string& path, const std::string& content)
  {
    std::cout
      << "Creating file: " << path << " with content of size="
      << content.size() << endl;

    const int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0400);
    if (fd == -1) {
      const int error = errno;
      std::cout
        << "Failed to open file for writing: " << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      return;
    }

    // Write the content to the file.
    ssize_t result = 0;
    if ((result = ::write(fd, content.data(), content.size()))
        != ssize_t(content.size()))
    {
      const int error = errno;
      std::cout
        << "Failed to write to file=" << path << "; errno=" << error
        << "; " << std::strerror(error) << endl;
      // Fall through to close the file.
    }
    else {
      std::cout
        << "Data written to file=" << path << "; successfully wrote "
        << result << " bytes" << endl;
    }

    ::close(fd);
  }
}

//============================================================================
// :: Entry Point

int main()
{
  const char* const file = "/tmp/file";
  unlink(file);
  readOnlyFile(file, "This content should get written because the file "
                     "does not yet exist and so, only the mode of the "
                     "containing directory will influence my ability to "
                     "create and open the file. The mode of the file only "
                     "applies to opening of the stream, not subsequent stream "
                     "operations after stream has opened.\n\n");
  readOnlyFile(file, "This should not get written because the file already "
                     "exists and is read-only.\n\n");
}
''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained(r'''Creating file: /tmp/file with content of size=292
Data written to file=/tmp/file; successfully wrote 292 bytes
Creating file: /tmp/file with content of size=79
Failed to open file for writing: /tmp/file; errno=13; Permission denied
''', run_js('a.out.js'))

  def test_embed_file_large(self):
    # If such long files are encoded on one line,
    # they overflow the interpreter's limit
    large_size = int(1500000)
    open('large.txt', 'w').write('x' * large_size)
    open('src.cpp', 'w').write(r'''
      #include <stdio.h>
      #include <unistd.h>
      int main()
      {
          FILE* fp = fopen("large.txt", "r");
          if (fp) {
              printf("ok\n");
              fseek(fp, 0L, SEEK_END);
              printf("%ld\n", ftell(fp));
          } else {
              printf("failed to open large file.txt\n");
          }
          return 0;
      }
    ''')
    Popen([PYTHON, EMCC, 'src.cpp', '--embed-file', 'large.txt']).communicate()
    for engine in JS_ENGINES:
      if engine == V8_ENGINE: continue # ooms
      print engine
      self.assertContained('ok\n' + str(large_size) + '\n', run_js('a.out.js', engine=engine))

  def test_force_exit(self):
    open('src.cpp', 'w').write(r'''
#include <emscripten/emscripten.h>

namespace
{
  extern "C"
  EMSCRIPTEN_KEEPALIVE
  void callback()
  {
    EM_ASM({ Module.print('callback pre()') });
    ::emscripten_force_exit(42);
    EM_ASM({ Module.print('callback post()') });
    }
}

int
main()
{
  EM_ASM({ setTimeout(function() { Module.print("calling callback()"); _callback() }, 100) });
  ::emscripten_exit_with_live_runtime();
  return 123;
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    output = run_js('a.out.js', engine=NODE_JS, assert_returncode=42)
    assert 'callback pre()' in output
    assert 'callback post()' not in output

  def test_bad_locale(self):
    logging.warning('TODO: check with upstream musl if this is correct behavior or not')
    open('src.cpp', 'w').write(r'''

#include <locale.h>
#include <stdio.h>
#include <wctype.h>

int
main(const int argc, const char * const * const argv)
{
  const char * const locale = (argc > 1 ? argv[1] : "C");
  const char * const actual = setlocale(LC_ALL, locale);
  if(actual == NULL) {
    printf("%s locale not supported\n",
           locale);
    return 0;
  }
  printf("locale set to %s: %s\n", locale, actual);
}

    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()

    self.assertContained('locale set to C: C', run_js('a.out.js', args=['C']))
    self.assertContained('locale set to waka: C.UTF-8', run_js('a.out.js', args=['waka'])) # the call still succeeds in musl, even if the locale requested was not selected

  def test_js_malloc(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>
#include <emscripten.h>

int main() {
  EM_ASM({
    for (var i = 0; i < 1000; i++) {
      var ptr = Module._malloc(1024*1024); // only done in JS, but still must not leak
      Module._free(ptr);
    }
  });
  printf("ok.\n");
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('ok.', run_js('a.out.js', args=['C']))

  def test_locale_wrong(self):
    open('src.cpp', 'w').write(r'''
#include <locale>
#include <iostream>
#include <stdexcept>

int
main(const int argc, const char * const * const argv)
{
  const char * const name = argc > 1 ? argv[1] : "C";

  try {
    const std::locale locale(name);
    std::cout
      << "Constructed locale \"" << name << "\"\n"
      << "This locale is "
      << (locale == std::locale::global(locale) ? "" : "not ")
      << "the global locale.\n"
      << "This locale is " << (locale == std::locale::classic() ? "" : "not ")
      << "the C locale." << std::endl;

  } catch(const std::runtime_error &ex) {
    std::cout
      << "Can't construct locale \"" << name << "\": " << ex.what()
      << std::endl;
    return 1;

  } catch(...) {
    std::cout
      << "FAIL: Unexpected exception constructing locale \"" << name << '\"'
      << std::endl;
    return 127;
  }
}
    ''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('Constructed locale "C"\nThis locale is the global locale.\nThis locale is the C locale.', run_js('a.out.js', args=['C']))
    self.assertContained('''Can't construct locale "waka": collate_byname<char>::collate_byname failed to construct for waka''', run_js('a.out.js', args=['waka'], assert_returncode=1))

  def test_cleanup_os(self):
    # issue 2644
    def test(args, be_clean):
      print args
      self.clear()
      shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'a.c')
      open('b.c', 'w').write(' ')
      Popen([PYTHON, EMCC, 'a.c', 'b.c'] + args).communicate()
      clutter = glob.glob('*.o')
      if be_clean: assert len(clutter) == 0, 'should not leave clutter ' + str(clutter)
      else: assert len(clutter) == 2, 'should leave .o files'
    test(['-o', 'c.bc'], True)
    test(['-o', 'c.js'], True)
    test(['-o', 'c.html'], True)
    test(['-c'], False)

  def test_dash_g(self):
    open('src.c', 'w').write('''
      #include <stdio.h>
      #include <assert.h>

      void checker(int x) {
        x += 20;
        assert(x < 15); // this is line 7!
      }

      int main() {
        checker(10);
        return 0;
      }
    ''')

    def check(has):
      print has
      lines = open('a.out.js', 'r').readlines()
      lines = filter(lambda line: '___assert_fail(' in line or '___assert_func(' in line, lines)
      found_line_num = any(('//@line 7 "' in line) for line in lines)
      found_filename = any(('src.c"\n' in line) for line in lines)
      assert found_line_num == has, 'Must have debug info with the line number'
      assert found_filename == has, 'Must have debug info with the filename'

    Popen([PYTHON, EMCC, 'src.c', '-g']).communicate()
    check(True)
    Popen([PYTHON, EMCC, 'src.c']).communicate()
    check(False)
    Popen([PYTHON, EMCC, 'src.c', '-g0']).communicate()
    check(False)
    Popen([PYTHON, EMCC, 'src.c', '-g0', '-g']).communicate() # later one overrides
    check(True)
    Popen([PYTHON, EMCC, 'src.c', '-g', '-g0']).communicate() # later one overrides
    check(False)

  def test_dash_g_bc(self):
    def test(opts):
      print opts
      def get_size(name):
        return os.stat(name).st_size
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-o', 'a_.bc'] + opts).communicate()
      sizes = { '_': get_size('a_.bc') }
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-g', '-o', 'ag.bc'] + opts).communicate()
      sizes['g'] = get_size('ag.bc')
      for i in range(0, 5):
        Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-g' + str(i), '-o', 'a' + str(i) + '.bc'] + opts).communicate()
        sizes[i] = get_size('a' + str(i) + '.bc')
      print '  ', sizes
      assert sizes['_'] == sizes[0] == sizes[1] == sizes[2] == sizes[3], 'no debug or <4 debug, means no llvm debug info ' + str(sizes)
      assert sizes['g'] == sizes[4], '-g or -g4 means llvm debug info ' + str(sizes)
      assert sizes['_'] < sizes['g'], 'llvm debug info has positive size ' + str(sizes)
    test([])
    test(['-O1'])

  def test_no_filesystem(self):
    FS_MARKER = 'var FS'
    # fopen forces full filesystem support
    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world_fopen.c'), '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1'])
    yes_size = os.stat('a.out.js').st_size
    self.assertContained('hello, world!', run_js('a.out.js'))
    assert FS_MARKER in open('a.out.js').read()
    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1'])
    no_size = os.stat('a.out.js').st_size
    self.assertContained('hello, world!', run_js('a.out.js'))
    assert FS_MARKER not in open('a.out.js').read()
    print 'yes fs, no fs:', yes_size, no_size
    assert yes_size - no_size > 100000 # 100K of FS code is removed
    assert no_size < 360000

  def test_no_nuthin(self):
    print 'part one: check NO_FILESYSTEM is automatically set, and effective'
    def test(opts, ratio, absolute):
      print 'opts, ratio, absolute:', opts, ratio, absolute
      def get_size(name):
        return os.stat(name).st_size
      sizes = {}
      def do(name, source, moar_opts):
        self.clear()
        Popen([PYTHON, EMCC, path_from_root('tests', source), '-o', name + '.js'] + opts + moar_opts).communicate()
        sizes[name] = get_size(name + '.js')
        self.assertContained('hello, world!', run_js(name + '.js'))
      do('normal', 'hello_world_fopen.c', [])
      do('no_fs', 'hello_world.c', []) # without fopen, we should auto-detect we do not need full fs support and can do NO_FILESYSTEM
      do('no_fs_manual', 'hello_world.c', ['-s', 'NO_FILESYSTEM=1'])
      do('no_nuthin', 'hello_world.c', ['-s', 'EXPORTED_RUNTIME_METHODS=[]'])
      print '  ', sizes
      assert sizes['no_fs'] < sizes['normal']
      assert sizes['no_nuthin'] < sizes['no_fs']
      assert sizes['no_nuthin'] < ratio*sizes['normal']
      assert sizes['no_nuthin'] < absolute
      if '--closure' in opts: # no EXPORTED_RUNTIME_METHODS makes closure much more effective
        assert sizes['no_nuthin'] < 0.975*sizes['no_fs']
      assert sizes['no_fs_manual'] < sizes['no_fs'] # manual can remove a tiny bit more
    test([], 0.75, 360000)
    test(['-O1'], 0.66, 210000)
    test(['-O2'], 0.50, 70000)
    test(['-O3', '--closure', '1'], 0.60, 50000)
    test(['-O3', '--closure', '2'], 0.60, 41000) # might change now and then

    print 'part two: focus on EXPORTED_RUNTIME_METHODS effects, on hello_world_em_asm'
    def test(opts, ratio, absolute):
      print 'opts, ratio, absolute:', opts, ratio, absolute
      def get_size(name):
        return os.stat(name).st_size
      sizes = {}
      def do(name, moar_opts):
        self.clear()
        Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world_em_asm.c'), '-o', name + '.js'] + opts + moar_opts).communicate()
        sizes[name] = get_size(name + '.js')
        self.assertContained('hello, world!', run_js(name + '.js'))
      do('normal', [])
      do('no_nuthin', ['-s', 'EXPORTED_RUNTIME_METHODS=[]'])
      print '  ', sizes
      assert sizes['no_nuthin'] < sizes['normal']
      assert sizes['no_nuthin'] < ratio*sizes['normal']
      assert sizes['no_nuthin'] < absolute
      if '--closure' in opts: # no EXPORTED_RUNTIME_METHODS makes closure much more effective
        assert sizes['no_nuthin'] < 0.975*sizes['normal']
    test([], 1, 220000)
    test(['-O1'], 1, 215000)
    test(['-O2'], 0.99, 75000)
    test(['-O3', '--closure', '1'], 0.975, 50000)
    test(['-O3', '--closure', '2'], 0.975, 41000) # might change now and then

  def test_no_browser(self):
    BROWSER_INIT = 'var Browser'

    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')])
    assert BROWSER_INIT not in open('a.out.js').read()

    check_execute([PYTHON, EMCC, path_from_root('tests', 'browser_main_loop.c')]) # uses emscripten_set_main_loop, which needs Browser
    assert BROWSER_INIT in open('a.out.js').read()

  def test_EXPORTED_RUNTIME_METHODS(self):
    def test(opts, has, not_has):
      print opts, has, not_has
      self.clear()
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')] + opts).communicate()
      self.assertContained('hello, world!', run_js('a.out.js'))
      src = open('a.out.js').read()
      self.assertContained(has, src)
      self.assertNotContained(not_has, src)

    test([], 'Module["intArray', 'Module["waka')
    test(['-s', 'EXPORTED_RUNTIME_METHODS=[]'], 'Module["print', 'Module["intArray')
    test(['-s', 'EXPORTED_RUNTIME_METHODS=["intArrayToString"]'], 'Module["intArray', 'Module["waka')
    test(['-s', 'EXPORTED_RUNTIME_METHODS=[]', '-s', 'EXTRA_EXPORTED_RUNTIME_METHODS=["intArrayToString"]'], 'Module["intArray', 'Module["waka')

  def test_stat_fail_alongtheway(self):
    open('src.cpp', 'w').write(r'''
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define CHECK(expression) \
  if(!(expression)) {                            \
    error = errno;                               \
    printf("FAIL: %s\n", #expression); fail = 1; \
  } else {                                       \
    error = errno;                               \
    printf("pass: %s\n", #expression);           \
  }                                              \

int
main()
{
  int error;
  int fail = 0;
  CHECK(mkdir("path", 0777) == 0);
  CHECK(close(open("path/file", O_CREAT | O_WRONLY, 0644)) == 0);
  {
    struct stat st;
    CHECK(stat("path", &st) == 0);
    CHECK(st.st_mode = 0777);
  }
  {
    struct stat st;
    CHECK(stat("path/nosuchfile", &st) == -1);
    printf("info: errno=%d %s\n", error, strerror(error));
    CHECK(error == ENOENT);
  }
  {
    struct stat st;
    CHECK(stat("path/file", &st) == 0);
    CHECK(st.st_mode = 0666);
  }
  {
    struct stat st;
    CHECK(stat("path/file/impossible", &st) == -1);
    printf("info: errno=%d %s\n", error, strerror(error));
    CHECK(error == ENOTDIR);
  }
  {
    struct stat st;
    CHECK(lstat("path/file/impossible", &st) == -1);
    printf("info: errno=%d %s\n", error, strerror(error));
    CHECK(error == ENOTDIR);
  }
  return fail;
}
''')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained(r'''pass: mkdir("path", 0777) == 0
pass: close(open("path/file", O_CREAT | O_WRONLY, 0644)) == 0
pass: stat("path", &st) == 0
pass: st.st_mode = 0777
pass: stat("path/nosuchfile", &st) == -1
info: errno=2 No such file or directory
pass: error == ENOENT
pass: stat("path/file", &st) == 0
pass: st.st_mode = 0666
pass: stat("path/file/impossible", &st) == -1
info: errno=20 Not a directory
pass: error == ENOTDIR
pass: lstat("path/file/impossible", &st) == -1
info: errno=20 Not a directory
pass: error == ENOTDIR
''', run_js('a.out.js'))


  def test_emterpreter(self):
    if SPIDERMONKEY_ENGINE not in JS_ENGINES: return self.skip('test_emterpreter requires SpiderMonkey to run.')

    def do_emcc_test(source, args, output, emcc_args=[]):
      print
      print 'emcc', source[:40], '\n' in source
      try_delete('a.out.js')
      if '\n' in source:
        open('src.cpp', 'w').write(source)
        source = 'src.cpp'
      else:
        source = path_from_root('tests', source)
      Popen([PYTHON, EMCC, source, '-O2', '-s', 'EMTERPRETIFY=1', '-g2'] + emcc_args).communicate()
      self.assertTextDataContained(output, run_js('a.out.js', args=args))
      out = run_js('a.out.js', engine=SPIDERMONKEY_ENGINE, args=args, stderr=PIPE, full_output=True)
      self.assertTextDataContained(output, out)
      self.validate_asmjs(out)
      # -g2 enables these
      src = open('a.out.js').read()
      assert 'function emterpret' in src, 'emterpreter should exist'
      # and removing calls to the emterpreter break, so it was being used
      out1 = run_js('a.out.js', args=args)
      assert output in out1
      open('a.out.js', 'w').write(src.replace('function emterpret', 'function do_not_find_me'))
      out2 = run_js('a.out.js', args=args, stderr=PIPE, assert_returncode=None)
      assert output not in out2, out2
      assert out1 != out2

    def do_test(source, args, output):
      print
      print 'emcc', source.replace('\n', '.')[:40], '\n' in source
      self.clear()
      if '\n' in source:
        open('src.cpp', 'w').write(source)
        source = 'src.cpp'
      else:
        source = path_from_root('tests', source)
      Popen([PYTHON, EMCC, source, '-O2', '--profiling', '-s', 'FINALIZE_ASM_JS=0', '-s', 'GLOBAL_BASE=2048', '-s', 'ALLOW_MEMORY_GROWTH=0']).communicate()
      Popen([PYTHON, path_from_root('tools', 'emterpretify.py'), 'a.out.js', 'em.out.js', 'ASYNC=0']).communicate()
      self.assertTextDataContained(output, run_js('a.out.js', args=args))
      self.assertTextDataContained(output, run_js('em.out.js', args=args))
      out = run_js('em.out.js', engine=SPIDERMONKEY_ENGINE, args=args, stderr=PIPE, full_output=True)
      self.assertTextDataContained(output, out)
      self.validate_asmjs(out)

    # generate default shell for js test
    def make_default(args=[]):
      Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O2', '--profiling', '-s', 'FINALIZE_ASM_JS=0', '-s', 'GLOBAL_BASE=2048'] + args).communicate()
      default = open('a.out.js').read()
      start = default.index('function _main(')
      end = default.index('}', start)
      default = default[:start] + '{{{MAIN}}}' + default[end+1:]
      default_mem = open('a.out.js.mem', 'rb').read()
      return default, default_mem
    default, default_mem = make_default()
    default_float, default_float_mem = make_default(['-s', 'PRECISE_F32=1'])

    def do_js_test(name, source, args, output, floaty=False):
      print
      print 'js', name
      self.clear()
      if '\n' not in source:
        source = open(source).read()
      the_default = default if not floaty else default_float
      the_default_mem = default_mem if not floaty else default_float_mem
      source = the_default.replace('{{{MAIN}}}', source)
      open('a.out.js', 'w').write(source)
      open('a.out.js.mem', 'wb').write(the_default_mem)
      Popen([PYTHON, path_from_root('tools', 'emterpretify.py'), 'a.out.js', 'em.out.js', 'ASYNC=0']).communicate()
      sm_no_warn = filter(lambda x: x != '-w', SPIDERMONKEY_ENGINE)
      self.assertTextDataContained(output, run_js('a.out.js', engine=sm_no_warn, args=args)) # run in spidermonkey for print()
      self.assertTextDataContained(output, run_js('em.out.js', engine=sm_no_warn, args=args))

    do_emcc_test('hello_world.c', [], 'hello, world!')

    do_test('hello_world.c', [], 'hello, world!')
    do_test('hello_world_loop.cpp', [], 'hello, world!')
    do_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.')

    print 'profiling'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-g2'])
    normal = open('a.out.js').read()
    shutil.copyfile('a.out.js', 'last.js')
    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-g2', '--profiling'])
    profiling = open('a.out.js').read()
    assert len(profiling) > len(normal) + 250, [len(profiling), len(normal)] # should be much larger

    print 'blacklisting'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', [])
    src = open('a.out.js').read()
    assert 'emterpret' in self.get_func(src, '_main'), 'main is emterpreted'
    assert 'function _atoi(' not in src, 'atoi is emterpreted and does not even have a trampoline, since only other emterpreted can reach it'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_BLACKLIST=["_main"]']) # blacklist main
    src = open('a.out.js').read()
    assert 'emterpret' not in self.get_func(src, '_main'), 'main is NOT emterpreted, it was  blacklisted'
    assert 'emterpret' in self.get_func(src, '_atoi'), 'atoi is emterpreted'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_BLACKLIST=["_main", "_atoi"]']) # blacklist main and atoi
    src = open('a.out.js').read()
    assert 'emterpret' not in self.get_func(src, '_main'), 'main is NOT emterpreted, it was  blacklisted'
    assert 'emterpret' not in self.get_func(src, '_atoi'), 'atoi is NOT emterpreted either'

    open('blacklist.txt', 'w').write('["_main", "_atoi"]')
    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_BLACKLIST=@blacklist.txt']) # blacklist main and atoi with a @response file
    src = open('a.out.js').read()
    assert 'emterpret' not in self.get_func(src, '_main'), 'main is NOT emterpreted, it was  blacklisted'
    assert 'emterpret' not in self.get_func(src, '_atoi'), 'atoi is NOT emterpreted either'

    print 'whitelisting'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_WHITELIST=[]'])
    src = open('a.out.js').read()
    assert 'emterpret' in self.get_func(src, '_main'), 'main is emterpreted'
    assert 'function _atoi(' not in src, 'atoi is emterpreted and does not even have a trampoline, since only other emterpreted can reach it'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_WHITELIST=["_main"]'])
    src = open('a.out.js').read()
    assert 'emterpret' in self.get_func(src, '_main')
    assert 'emterpret' not in self.get_func(src, '_atoi'), 'atoi is not in whitelist, so it is not emterpreted'

    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_WHITELIST=["_main", "_atoi"]'])
    src = open('a.out.js').read()
    assert 'emterpret' in self.get_func(src, '_main')
    assert 'function _atoi(' not in src, 'atoi is emterpreted and does not even have a trampoline, since only other emterpreted can reach it'

    open('whitelist.txt', 'w').write('["_main"]')
    do_emcc_test('fannkuch.cpp', ['5'], 'Pfannkuchen(5) = 7.', ['-s', 'EMTERPRETIFY_WHITELIST=@whitelist.txt'])
    src = open('a.out.js').read()
    assert 'emterpret' in self.get_func(src, '_main')
    assert 'emterpret' not in self.get_func(src, '_atoi'), 'atoi is not in whitelist, so it is not emterpreted'

    do_test(r'''
#include<stdio.h>

int main() {
  volatile float f;
  volatile float *ff = &f;
  *ff = -10;
  printf("hello, world! %d\n", (int)f);
  return 0;
}
''', [], 'hello, world! -10')

    do_test(r'''
#include<stdio.h>

int main() {
  volatile float f;
  volatile float *ff = &f;
  *ff = -10;
  printf("hello, world! %.2f\n", f);
  return 0;
}
''', [], 'hello, world! -10.00')

    do_js_test('float', r'''
function _main() {
  var f = f0;
  f = f0 + f0;
  print(f);
}
''', [], '0\n', floaty=True)

    do_js_test('conditionals', r'''
function _main() {
 var i8 = 0;
 var d10 = +d10, d11 = +d11, d7 = +d7, d5 = +d5, d6 = +d6, d9 = +d9;
 d11 = +1;
 d7 = +2;
 d5 = +3;
 d6 = +4;
 d10 = d11 < d7 ? d11 : d7;
 print(d10);
 d9 = d5 < d6 ? d5 : d6;
 print(d9);
 HEAPF64[tempDoublePtr >> 3] = d10;
 i8 = STACKTOP;
 HEAP32[i8 >> 2] = HEAP32[tempDoublePtr >> 2];
 HEAP32[i8 + 4 >> 2] = HEAP32[tempDoublePtr + 4 >> 2];
 print(HEAP32[i8 >> 2]);
 print(HEAP32[i8 + 4 >> 2]);
}
''', [], '1\n3\n0\n1072693248\n')

    do_js_test('bigswitch', r'''
function _main() {
 var i2 = 0, i3 = 0, i4 = 0, i6 = 0, i8 = 0, i9 = 0, i10 = 0, i11 = 0, i12 = 0, i13 = 0, i14 = 0, i15 = 0, i16 = 0, i5 = 0, i7 = 0, i1 = 0;
 print(4278);
 i6 = 0;
 L1 : while (1) {
  i11 = -1;
  switch ((i11 | 0)) {
  case 0:
   {
    i6 = 67;
    break;
   }
  default:
   {}
  }
  print(i6);
  break;
 }
 print(i6);
}
''', [], '4278\n0\n0\n')

    do_js_test('big int compare', r'''
function _main() {
  print ((0 > 4294963001) | 0);
}
''', [], '0\n')

    do_js_test('effectless expressions, with a subeffect', r'''
function _main() {
  (print (123) | 0) != 0;
  print (456) | 0;
  0 != (print (789) | 0);
  0 | (print (159) | 0);
}
''', [], '123\n456\n789\n159\n')

    do_js_test('effectless unary', r'''
function _main() {
  !(0 != 0);
  !(print (123) | 0);
}
''', [], '123\n')

    do_js_test('flexible mod', r'''
function _main() {
  print(1 % 16);
}
''', [], '1\n')

    # codegen log tests

    def do_log_test(source, expected, func):
      print 'log test', source, expected
      try:
        os.environ['EMCC_LOG_EMTERPRETER_CODE'] = '1'
        out, err = Popen([PYTHON, EMCC, source, '-O3', '-s', 'EMTERPRETIFY=1'], stderr=PIPE).communicate()
      finally:
        del os.environ['EMCC_LOG_EMTERPRETER_CODE']
      lines = err.split('\n')
      lines = filter(lambda line: 'raw bytecode for ' + func in line, lines)
      assert len(lines) == 1, '\n\n'.join(lines)
      err = lines[0]
      parts = err.split('insts: ')
      pre, post = parts[:2]
      assert func in pre, pre
      post = post.split('\n')[0]
      seen = int(post)
      print '  seen', seen, ', expected ', expected, type(seen), type(expected)
      assert expected == seen or (seen in expected if type(expected) in [list, tuple] else False), ['expect', expected, 'but see', seen]

    do_log_test(path_from_root('tests', 'primes.cpp'), range(88, 94), '_main')
    do_log_test(path_from_root('tests', 'fannkuch.cpp'), range(226, 235), '__Z15fannkuch_workerPv')

    # test non-native as well, registerizeHarder can be a little more efficient here
    old_native = os.environ.get('EMCC_NATIVE_OPTIMIZER')
    try:
      os.environ['EMCC_NATIVE_OPTIMIZER'] = '0'
      do_log_test(path_from_root('tests', 'fannkuch.cpp'), range(226, 235), '__Z15fannkuch_workerPv')
    finally:
      if old_native: os.environ['EMCC_NATIVE_OPTIMIZER'] = old_native
      else: del os.environ['EMCC_NATIVE_OPTIMIZER']

  def test_emterpreter_advise(self):
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'emterpreter_advise.cpp'), '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-s', 'EMTERPRETIFY_ADVISE=1'], stdout=PIPE).communicate()
    self.assertContained('-s EMTERPRETIFY_WHITELIST=\'["__Z6middlev", "__Z7sleeperv", "__Z8recurserv", "_main"]\'', out)

    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'emterpreter_advise_funcptr.cpp'), '-s', 'EMTERPRETIFY=1', '-s', 'EMTERPRETIFY_ASYNC=1', '-s', 'EMTERPRETIFY_ADVISE=1'], stdout=PIPE).communicate()
    self.assertContained('-s EMTERPRETIFY_WHITELIST=\'["__Z4posti", "__Z5post2i", "__Z6middlev", "__Z7sleeperv", "__Z8recurserv", "_main"]\'', out)

  def test_link_with_a_static(self):
    for args in [[], ['-O2']]:
      print args
      self.clear()
      open('x.c', 'w').write(r'''
int init_weakref(int a, int b) {
    return a + b;
}
''')
      open('y.c', 'w').write(r'''
static int init_weakref(void) { // inlined in -O2, not in -O0 where it shows up in llvm-nm as 't'
    return 150;
}

int testy(void) {
    return init_weakref();
}
''')
      open('z.c', 'w').write(r'''
extern int init_weakref(int, int);
extern int testy(void);

int main(void) {
    return testy() + init_weakref(5, 6);
}
''')
      Popen([PYTHON, EMCC, 'x.c', '-o', 'x.o']).communicate()
      Popen([PYTHON, EMCC, 'y.c', '-o', 'y.o']).communicate()
      Popen([PYTHON, EMCC, 'z.c', '-o', 'z.o']).communicate()
      Popen([PYTHON, EMAR, 'rc', 'libtest.a', 'y.o']).communicate()
      Popen([PYTHON, EMAR, 'rc', 'libtest.a', 'x.o']).communicate()
      Popen([PYTHON, EMRANLIB, 'libtest.a']).communicate()
      Popen([PYTHON, EMCC, 'z.o', 'libtest.a'] + args).communicate()
      out = run_js('a.out.js', assert_returncode=161)

  def test_require(self):
    inname = path_from_root('tests', 'hello_world.c')
    Building.emcc(inname, output_filename='a.out.js')
    output = Popen(NODE_JS + ['-e', 'require("./a.out.js")'], stdout=PIPE, stderr=PIPE).communicate()
    assert output == ('hello, world!\n', ''), 'expected no output, got\n===\nSTDOUT\n%s\n===\nSTDERR\n%s\n===\n' % output

  def test_native_optimizer(self):
    def test(args, expected):
      print args, expected
      old_debug = os.environ.get('EMCC_DEBUG')
      old_native = os.environ.get('EMCC_NATIVE_OPTIMIZER')
      try:
        os.environ['EMCC_DEBUG'] = '1'
        os.environ['EMCC_NATIVE_OPTIMIZER'] = '1'
        out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O2',] + args, stderr=PIPE).communicate()
      finally:
        if old_debug: os.environ['EMCC_DEBUG'] = old_debug
        else: del os.environ['EMCC_DEBUG']
        if old_native: os.environ['EMCC_NATIVE_OPTIMIZER'] = old_native
        else: del os.environ['EMCC_NATIVE_OPTIMIZER']
      assert err.count('js optimizer using native') == expected, [err, expected]
      assert os.path.exists('a.out.js'), err
      self.assertContained('hello, world!', run_js('a.out.js'))

    test([], 1)
    test(['-s', 'OUTLINING_LIMIT=100000'], 2) # 2, because we run them before and after outline, which is non-native

  def test_emconfigure_js_o(self):
    # issue 2994
    for i in [0, 1, 2]:
      for f in ['hello_world.c', 'files.cpp']:
        print i, f
        try:
          os.environ['EMCONFIGURE_JS'] = str(i)
          self.clear()
          Popen([PYTHON, path_from_root('emconfigure'), PYTHON, EMCC, '-c', '-o', 'a.o', path_from_root('tests', f)]).communicate()
          Popen([PYTHON, EMCC, 'a.o']).communicate()
          if f == 'hello_world.c':
            if i == 0:
              assert not os.path.exists('a.out.js') # native .o, not bitcode!
            else:
              assert 'hello, world!' in run_js(self.in_dir('a.out.js'))
          else:
            # file access, need 2 to force js
            if i == 0 or i == 1:
              assert not os.path.exists('a.out.js') # native .o, not bitcode!
            else:
              assert os.path.exists('a.out.js')
        finally:
          del os.environ['EMCONFIGURE_JS']

  def test_emcc_c_multi(self):
    def test(args, llvm_opts=None):
      print args
      lib = r'''
        int mult() { return 1; }
      '''

      lib_name = 'libA.c'
      open(lib_name, 'w').write(lib)
      main = r'''
        #include <stdio.h>
        int mult();
        int main() {
          printf("result: %d\n", mult());
          return 0;
        }
      '''
      main_name = 'main.c'
      open(main_name, 'w').write(main)

      if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
      try:
        os.environ['EMCC_DEBUG'] = '1'
        out, err = Popen([PYTHON, EMCC, '-c', main_name, lib_name] + args, stderr=PIPE).communicate()
      finally:
        del os.environ['EMCC_DEBUG']

      VECTORIZE = '-disable-loop-vectorization'

      if args:
        assert err.count(VECTORIZE) == 2, err # specified twice, once per file

        assert err.count('emcc: LLVM opts: ' + llvm_opts + ' ' + VECTORIZE) == 2, err # exactly once per invocation of optimizer
      else:
        assert err.count(VECTORIZE) == 0, err # no optimizations

      Popen([PYTHON, EMCC, main_name.replace('.c', '.o'), lib_name.replace('.c', '.o')]).communicate()

      self.assertContained('result: 1', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    test([])
    test(['-O2'], '-O3')
    test(['-Oz'], '-Oz')
    test(['-Os'], '-Os')

  def test_export_all_3142(self):
    open('src.cpp', 'w').write(r'''
typedef unsigned int Bit32u;

struct S_Descriptor {
    Bit32u limit_0_15   :16;
    Bit32u base_0_15    :16;
    Bit32u base_16_23   :8;
};

class Descriptor
{
public:
    Descriptor() { saved.fill[0]=saved.fill[1]=0; }
    union {
        S_Descriptor seg;
        Bit32u fill[2];
    } saved;
};

Descriptor desc;
    ''')
    try_delete('a.out.js')
    Popen([PYTHON, EMCC, 'src.cpp', '-O2', '-s', 'EXPORT_ALL=1']).communicate()
    assert os.path.exists('a.out.js')

  def test_f0(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'fasta.cpp'), '-O2', '-s', 'PRECISE_F32=1', '-profiling']).communicate()
    src = open('a.out.js').read()
    assert ' = f0;' in src or ' = f0,' in src

  def test_merge_pair(self):
    def test(filename, full):
      print '----', filename, full
      Popen([PYTHON, EMCC, path_from_root('tests', filename), '-O1', '-profiling', '-o', 'left.js']).communicate()
      src = open('left.js').read()
      open('right.js', 'w').write(src.replace('function _main() {', 'function _main() { Module.print("replaced"); '))

      self.assertContained('hello, world!', run_js('left.js'))
      self.assertContained('hello, world!', run_js('right.js'))
      self.assertNotContained('replaced', run_js('left.js'))
      self.assertContained('replaced', run_js('right.js'))

      n = src.count('function _')

      def has(i):
        Popen([PYTHON, path_from_root('tools', 'merge_pair.py'), 'left.js', 'right.js', str(i), 'out.js']).communicate()
        return 'replaced' in run_js('out.js')

      assert not has(0), 'same as left'
      assert has(n), 'same as right'
      assert has(n+5), 'same as right, big number is still ok'

      if full:
        change = -1
        for i in range(n):
          if has(i):
            change = i
            break
        assert change > 0 and change <= n

    test('hello_world.cpp', True)
    test('hello_libcxx.cpp', False)

  def test_emmake_emconfigure(self):
    def check(what, args, fail=True, expect=''):
      args = [PYTHON, path_from_root(what)] + args
      print what, args, fail, expect
      out, err = Popen(args, stdout=PIPE, stderr=PIPE).communicate()
      assert ('is a helper for' in err) == fail
      assert ('Typical usage' in err) == fail
      self.assertContained(expect, out)
    check('emmake', [])
    check('emconfigure', [])
    check('emmake', ['--version'])
    check('emconfigure', ['--version'])
    check('emmake', ['make'], fail=False)
    check('emconfigure', ['configure'], fail=False)
    check('emconfigure', ['./configure'], fail=False)
    check('emconfigure', ['cmake'], fail=False)

    open('test.py', 'w').write('''
import os
print os.environ.get('CROSS_COMPILE')
''')
    check('emconfigure', [PYTHON, 'test.py'], expect=path_from_root('em'))
    check('emmake', [PYTHON, 'test.py'], expect=path_from_root('em'))

    open('test.py', 'w').write('''
import os
print os.environ.get('NM')
''')
    check('emconfigure', [PYTHON, 'test.py'], expect=tools.shared.LLVM_NM)

  def test_sdl2_config(self):
    for args, expected in [
      [['--version'], '2.0.0'],
      [['--cflags'], '-s USE_SDL=2'],
      [['--libs'], '-s USE_SDL=2'],
      [['--cflags', '--libs'], '-s USE_SDL=2'],
    ]:
      print args, expected
      out, err = Popen([PYTHON, path_from_root('system', 'bin', 'sdl2-config')] + args, stdout=PIPE, stderr=PIPE).communicate()
      assert expected in out, out
      print 'via emmake'
      out, err = Popen([PYTHON, path_from_root('emmake'), 'sdl2-config'] + args, stdout=PIPE, stderr=PIPE).communicate()
      assert expected in out, out

  def test_warn_toomany_vars(self):
    for source, warn in [
      (path_from_root('tests', 'hello_world.c'), False),
      (path_from_root('tests', 'hello_libcxx.cpp'), False),
      (path_from_root('tests', 'printf', 'test.c'), True)
    ]:
      for opts in [0, 1, 2, 3, 's', 'z']:
        print source, opts
        self.clear()
        out, err = Popen([PYTHON, EMCC, source, '-O' + str(opts)], stderr=PIPE).communicate()
        assert os.path.exists('a.out.js')
        assert ('emitted code will contain very large numbers of local variables' in err) == (warn and (opts in [0, 1]))

  def test_module_onexit(self):
    open('src.cpp', 'w').write(r'''
#include <emscripten.h>
int main() {
  EM_ASM({
    Module.onExit = function(status) { Module.print('exiting now, status ' + status) };
  });
  return 14;
}
    ''')
    try_delete('a.out.js')
    Popen([PYTHON, EMCC, 'src.cpp']).communicate()
    self.assertContained('exiting now, status 14', run_js('a.out.js', assert_returncode=14))

  def test_file_packager_huge(self):
    open('huge.dat', 'w').write('a'*(1024*1024*257))
    open('tiny.dat', 'w').write('a')
    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'tiny.dat'], stdout=PIPE, stderr=PIPE).communicate()
    assert err == '', err
    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'huge.dat'], stdout=PIPE, stderr=PIPE).communicate()
    assert 'warning: file packager is creating an asset bundle of 257 MB. this is very large, and browsers might have trouble loading it' in err, err
    self.clear()

  def test_nosplit(self): # relooper shouldn't split nodes if -Os or -Oz
    open('src.cpp', 'w').write(r'''
      #include <stdio.h>
      int main(int argc, char **argv) {
        printf("extra code\n");
        printf("to make the function big enough to justify splitting\n");
        if (argc == 1) {
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
        } else if (argc/2 == 2) {
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("2\n");
        } else if (argc/3 == 3) {
          printf("1\n");
          printf("3\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
        } else if (argc/4 == 4) {
          printf("4\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
        } else if (argc/5 == 5) {
          printf("five\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
          printf("1\n");
        }
        printf("hai\n");
        return 5;
      }
    ''')
    def test(opts, expected):
      print opts
      Popen([PYTHON, EMCC, 'src.cpp', '--profiling'] + opts).communicate()
      src = open('a.out.js').read()
      main = self.get_func(src, '_main')
      rets = main.count('return ')
      print '    ', rets
      assert rets == expected, [rets, '!=', expected]
    test(['-O1'], 6)
    test(['-O2'], 6)
    test(['-Os'], 1)
    test(['-Oz'], 1)

  def test_massive_alloc(self):
    if SPIDERMONKEY_ENGINE not in JS_ENGINES: return self.skip('cannot run without spidermonkey, node cannnot alloc huge arrays')

    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
#include <stdio.h>
#include <stdlib.h>

int main() {
  return (int)malloc(1024*1024*1400);
}
    ''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-s', 'ALLOW_MEMORY_GROWTH=1']).communicate()[1]
    assert os.path.exists('a.out.js')
    output = run_js('a.out.js', stderr=PIPE, full_output=True, engine=SPIDERMONKEY_ENGINE, assert_returncode=None)
    # just care about message regarding allocating over 1GB of memory
    self.assertContained('''Warning: Enlarging memory arrays, this is not fast! 16777216,1543503872\n''', output)

  def test_failing_alloc(self):
    for pre_fail, post_fail, opts in [
      ('', '', []),
      ('EM_ASM( Module.temp = DYNAMICTOP );', 'EM_ASM( assert(Module.temp === DYNAMICTOP, "must not adjust DYNAMICTOP when an alloc fails!") );', []),
      ('', '', ['-s', 'SPLIT_MEMORY=' + str(16*1024*1024)]),
    ]:
      for growth in [0, 1]:
        for aborting in [0, 1]:
          open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write(r'''
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <emscripten.h>

#define CHUNK_SIZE (10*1024*1024)

int main() {
  std::vector<void*> allocs;
  bool has = false;
  while (1) {
    printf("trying an allocation\n");
    %s
    void* curr = malloc(CHUNK_SIZE);
    if (!curr) {
      %s
      break;
    }
    has = true;
    printf("allocated another chunk, %%d so far\n", allocs.size());
    allocs.push_back(curr);
  }
  assert(has);
  printf("an allocation failed!\n");
  while (1) {
    assert(allocs.size() > 0);
    void *curr = allocs.back();
    allocs.pop_back();
    free(curr);
    printf("freed one\n");
    if (malloc(CHUNK_SIZE)) break;
  }
  printf("managed another malloc!\n");
}
''' % (pre_fail, post_fail))
          args = [PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp')] + opts
          if growth: args += ['-s', 'ALLOW_MEMORY_GROWTH=1']
          if not aborting: args += ['-s', 'ABORTING_MALLOC=0']
          print args, pre_fail
          check_execute(args)
          output = run_js('a.out.js', stderr=PIPE, full_output=True, assert_returncode=None)
          if (not aborting) or growth: # growth also disables aborting
            # we should fail eventually, then free, then succeed
            self.assertContained('''managed another malloc!\n''', output)
          else:
            # we should see an abort
            self.assertContained('''abort("Cannot enlarge memory arrays''', output)
            self.assertContained(('''higher than the current value 16777216,''', '''higher than the current value 33554432,'''), output)
            self.assertContained('''compile with  -s ALLOW_MEMORY_GROWTH=1 ''', output)
            self.assertContained('''compile with  -s ABORTING_MALLOC=0 ''', output)

  def test_libcxx_minimal(self):
    open('vector.cpp', 'w').write(r'''
#include <vector>
int main(int argc, char** argv) {
  std::vector<void*> v;
  for (int i = 0 ; i < argc; i++) {
    v.push_back(nullptr);
  }
  return v.size();
}
''')

    Popen([PYTHON, EMCC, '-O2', 'vector.cpp', '-o', 'vector.js']).communicate()[1]
    Popen([PYTHON, EMCC, '-O2', path_from_root('tests', 'hello_libcxx.cpp'), '-o', 'iostream.js']).communicate()[1]

    vector = os.stat('vector.js').st_size
    iostream = os.stat('iostream.js').st_size
    print vector, iostream

    assert vector > 1000
    assert 2.5*vector < iostream # we can strip out almost all of libcxx when just using vector

  def test_emulated_function_pointers(self):
    src = r'''
      #include <emscripten.h>
      typedef void (*fp)();
      int main(int argc, char **argv) {
        volatile fp f = 0;
        EM_ASM({
          if (typeof FUNCTION_TABLE_v !== 'undefined') {
            Module.print('function table: ' + FUNCTION_TABLE_v);
          } else {
            Module.print('no visible function tables');
          }
        });
        if (f) f();
        return 0;
      }
    '''
    open('src.c', 'w').write(src)
    def test(args, expected):
      print args, expected
      out, err = Popen([PYTHON, EMCC, 'src.c'] + args, stderr=PIPE).communicate()
      self.assertContained(expected, run_js(self.in_dir('a.out.js')))

    for opts in [0, 1, 2, 3]:
      test(['-O' + str(opts)], 'no visible function tables')
      test(['-O' + str(opts), '-s', 'EMULATED_FUNCTION_POINTERS=1'], 'function table: ')

  def test_emulated_function_pointers_2(self):
    src = r'''
      #include <emscripten.h>
      typedef void (*fp)();
      void one() { EM_ASM( Module.print('one') ); }
      void two() { EM_ASM( Module.print('two') ); }
      void test() {
        volatile fp f = one;
        f();
        f = two;
        f();
      }
      int main(int argc, char **argv) {
        test();
        // swap them!
        EM_ASM_INT({
          var one = $0;
          var two = $1;
          if (typeof FUNCTION_TABLE_v === 'undefined') {
            Module.print('no');
            return;
          }
          var temp = FUNCTION_TABLE_v[one];
          FUNCTION_TABLE_v[one] = FUNCTION_TABLE_v[two];
          FUNCTION_TABLE_v[two] = temp;
        }, (int)&one, (int)&two);
        test();
        return 0;
      }
    '''
    open('src.c', 'w').write(src)

    flipped = 'one\ntwo\ntwo\none\n'
    unchanged = 'one\ntwo\none\ntwo\n'
    no_table = 'one\ntwo\nno\none\ntwo\n'

    def test(args, expected):
      print args, expected.replace('\n', ' ')
      Popen([PYTHON, EMCC, 'src.c'] + args).communicate()
      self.assertContained(expected, run_js(self.in_dir('a.out.js')))

    for opts in [0, 1, 2]:
      test(['-O' + str(opts)], no_table)
      test(['-O' + str(opts), '-s', 'EMULATED_FUNCTION_POINTERS=1'], flipped)
      test(['-O' + str(opts), '-s', 'EMULATED_FUNCTION_POINTERS=2'], flipped)
      test(['-O' + str(opts), '-s', 'EMULATED_FUNCTION_POINTERS=1', '-s', 'RELOCATABLE=1'], flipped)
      test(['-O' + str(opts), '-s', 'EMULATED_FUNCTION_POINTERS=2', '-s', 'RELOCATABLE=1'], unchanged) # with both of those, we optimize and you cannot flip them
      test(['-O' + str(opts), '-s', 'MAIN_MODULE=1'], unchanged) # default for modules is optimized
      test(['-O' + str(opts), '-s', 'MAIN_MODULE=1', '-s', 'EMULATED_FUNCTION_POINTERS=2'], unchanged)
      test(['-O' + str(opts), '-s', 'MAIN_MODULE=1', '-s', 'EMULATED_FUNCTION_POINTERS=1'], flipped) # but you can disable that

  def test_minimal_dynamic(self):
    def test(main_args=[], library_args=[], expected='hello from main\nhello from library'):
      print 'testing', main_args, library_args
      self.clear()
      open('library.c', 'w').write(r'''
        #include <stdio.h>
        void library_func() {
        #ifdef USE_PRINTF
          printf("hello from library: %p", (int)&library_func);
        #else
          puts("hello from library");
        #endif
        }
      ''')
      check_execute([PYTHON, EMCC, 'library.c', '-s', 'SIDE_MODULE=1', '-O2', '-o', 'library.js'] + library_args)
      open('main.c', 'w').write(r'''
        #include <dlfcn.h>
        #include <stdio.h>
        int main() {
          puts("hello from main");
          void *lib_handle = dlopen("library.js", 0);
          typedef void (*voidfunc)();
          voidfunc x = (voidfunc)dlsym(lib_handle, "library_func");
          x();
        }
      ''')
      check_execute([PYTHON, EMCC, 'main.c', '-s', 'MAIN_MODULE=1', '--embed-file', 'library.js', '-O2'] + main_args)
      self.assertContained(expected, run_js('a.out.js', assert_returncode=None, stderr=subprocess.STDOUT))
      size = os.stat('a.out.js').st_size
      print '  size:', size
      return size
    full     = test()
    printf   = test(                                   library_args=['-DUSE_PRINTF'])                       # printf is not used in main, but libc was linked in, so it's there
    dce      = test(main_args=['-s', 'MAIN_MODULE=2'])                                                      # dce in main, and side happens to be ok since it uses puts as well
    dce_fail = test(main_args=['-s', 'MAIN_MODULE=2'], library_args=['-DUSE_PRINTF'], expected='undefined') # printf is not used in main, and we dce, so we failz
    dce_save = test(main_args=['-s', 'MAIN_MODULE=2', '-s', 'EXPORTED_FUNCTIONS=["_main", "_printf"]'],
                                                       library_args=['-DUSE_PRINTF'])                       # exporting printf in main keeps it alive for the library

    def percent_diff(x, y):
      small = min(x, y)
      large = max(x, y)
      return float(100*large)/small - 100

    assert percent_diff(full, printf) < 4
    assert percent_diff(dce, dce_fail) < 4
    assert dce < 0.2*full # big effect, 80%+ is gone
    assert dce_save > 1.1*dce # save exported all of printf

  def test_file_packager_eval(self):
    BAD = 'Module = eval('
    src = path_from_root('tests', 'hello_world.c')
    open('temp.txt', 'w').write('temp')

    # should emit eval only when emcc uses closure

    Popen([PYTHON, EMCC, src, '--preload-file', 'temp.txt', '-O1']).communicate()
    out = open('a.out.js').read()
    assert BAD not in out, out[max(out.index(BAD)-80, 0) : min(out.index(BAD)+80, len(out)-1)]

    Popen([PYTHON, EMCC, src, '--preload-file', 'temp.txt', '-O1', '--closure', '1']).communicate()
    out = open('a.out.js').read()
    assert BAD in out

    # file packager defauls to the safe closure case

    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'temp.txt'], stdout=PIPE, stderr=PIPE).communicate()
    assert BAD in out

    out, err = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'temp.txt', '--no-closure'], stdout=PIPE, stderr=PIPE).communicate()
    assert BAD not in out, out[max(out.index(BAD)-80, 0) : min(out.index(BAD)+80, len(out)-1)]

  def test_debug_asmLastOpts(self):
    open('src.c', 'w').write(r'''
#include <stdio.h>
struct Dtlink_t
{   struct Dtlink_t*   right;  /* right child      */
        union
        { unsigned int  _hash;  /* hash value       */
          struct Dtlink_t* _left;  /* left child       */
        } hl;
};
int treecount(register struct Dtlink_t* e)
{
  return e ? treecount(e->hl._left) + treecount(e->right) + 1 : 0;
}
int main() {
  printf("hello, world!\n");
}
''')
    out, err = Popen([PYTHON, EMCC, 'src.c', '-s', 'EXPORTED_FUNCTIONS=["_main", "_treecount"]', '--minify', '0', '-g4', '-Oz']).communicate()
    self.assertContained('hello, world!', run_js('a.out.js'))

  def test_meminit_crc(self):
    with open('src.c', 'w') as f:
      f.write(r'''
#include<stdio.h>
int main() { printf("Mary had a little lamb.\n"); }
''')
    out, err = Popen([PYTHON, EMCC, 'src.c', '-O2', '--memory-init-file', '0', '-s', 'MEM_INIT_METHOD=2', '-s', 'ASSERTIONS=1']).communicate()
    with open('a.out.js', 'r') as f:
      d = f.read()
    d = d.replace('Mary had', 'Paul had')
    with open('a.out.js', 'w') as f:
      f.write(d)
    out = run_js('a.out.js', assert_returncode=None, stderr=subprocess.STDOUT)
    self.assertContained('Assertion failed: memory initializer checksum', out)

  def test_emscripten_print_double(self):
    with open('src.c', 'w') as f:
      f.write(r'''
#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

void test(double d) {
  char buffer[100], buffer2[100];
  unsigned len, len2, len3;
  len = emscripten_print_double(d, NULL, -1);
  len2 = emscripten_print_double(d, buffer, len+1);
  assert(len == len2);
  buffer[len] = 0;
  len3 = snprintf(buffer2, 100, "%g", d);
  printf("|%g : %u : %s : %s : %d|\n", d, len, buffer, buffer2, len3);
}
int main() {
  printf("\n");
  test(0);
  test(1);
  test(-1);
  test(1.234);
  test(-1.234);
  test(1.1234E20);
  test(-1.1234E20);
  test(1.1234E-20);
  test(-1.1234E-20);
  test(1.0/0.0);
  test(-1.0/0.0);
}
''')
    Popen([PYTHON, EMCC, 'src.c']).communicate()
    out = run_js('a.out.js')
    self.assertContained('''
|0 : 1 : 0 : 0 : 1|
|1 : 1 : 1 : 1 : 1|
|-1 : 2 : -1 : -1 : 2|
|1.234 : 5 : 1.234 : 1.234 : 5|
|-1.234 : 6 : -1.234 : -1.234 : 6|
|1.1234e+20 : 21 : 112340000000000000000 : 1.1234e+20 : 10|
|-1.1234e+20 : 22 : -112340000000000000000 : -1.1234e+20 : 11|
|1.1234e-20 : 10 : 1.1234e-20 : 1.1234e-20 : 10|
|-1.1234e-20 : 11 : -1.1234e-20 : -1.1234e-20 : 11|
|inf : 8 : Infinity : inf : 3|
|-inf : 9 : -Infinity : -inf : 4|
''', out)

  def test_no_warn_exported_jslibfunc(self):
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=["alGetError"]', '-s', 'EXPORTED_FUNCTIONS=["_main", "_alGetError"]'], stdout=PIPE, stderr=PIPE).communicate()
    self.assertNotContained('''function requested to be exported, but not implemented: "_alGetError"''', err)

  def test_static_syscalls(self):
    Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')]).communicate()
    src = open('a.out.js').read()
    matches = re.findall('''function ___syscall(\d+)\(''', src)
    print 'seen syscalls:', matches
    assert set(matches) == set(['6', '54', '140', '146']) # close, ioctl, llseek, writev

  def test_emcc_dev_null(self):
    if WINDOWS: return self.skip('posix-only')
    proc = Popen([PYTHON, EMCC, '-dM', '-E', '-x', 'c', '/dev/null'], stdout=PIPE)
    out, err = proc.communicate()
    assert proc.returncode == 0
    self.assertContained('#define __EMSCRIPTEN__ 1', out) # all our defines should show up

  def test_umask_0(self):
    open('src.c', 'w').write(r'''
#include <sys/stat.h>
#include <stdio.h>
int main() {
  umask(0);
  printf("hello, world!\n");
}''')
    Popen([PYTHON, EMCC, 'src.c']).communicate()
    self.assertContained('hello, world!', run_js('a.out.js'))

  def test_no_missing_symbols(self): # simple hello world should not show any missing symbols
    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1'])

    # main() is implemented in C, and even if requested from JS, we should not warn
    open('library_foo.js', 'w').write('''
mergeInto(LibraryManager.library, {
  my_js__deps: ['main'],
  my_js: (function() {
      return function() {
        console.log("hello " + _nonexistingvariable);
      };
  }()),
});
''')
    open('test.cpp', 'w').write('''
#include<stdio.h>
#include<stdlib.h>

extern "C" {
  extern void my_js();
}

int main() {
  my_js();
  return EXIT_SUCCESS;
}
''')
    check_execute([PYTHON, EMCC, 'test.cpp', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1', '--js-library', 'library_foo.js'])

    # but we do error on a missing js var
    open('library_foo.js', 'w').write('''
mergeInto(LibraryManager.library, {
  my_js__deps: ['main', 'nonexistingvariable'],
  my_js: (function() {
      return function() {
        console.log("hello " + _nonexistingvariable);
      };
  }()),
});
''')
    proc = Popen([PYTHON, EMCC, 'test.cpp', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1', '--js-library', 'library_foo.js'], stderr=PIPE)
    out, err = proc.communicate()
    assert proc.returncode != 0
    assert 'unresolved symbol' in err

    # and also for missing C code, of course (without the --js-library, it's just a missing C method)
    proc = Popen([PYTHON, EMCC, 'test.cpp', '-s', 'ERROR_ON_UNDEFINED_SYMBOLS=1'], stderr=PIPE)
    out, err = proc.communicate()
    assert proc.returncode != 0
    assert 'unresolved symbol' in err

  def test_realpath(self):
    open('src.c', 'w').write(r'''
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define TEST_PATH "/boot/README.txt"

int
main(int argc, char **argv)
{
  errno = 0;
  char *t_realpath_buf = realpath(TEST_PATH, NULL);
  if (NULL == t_realpath_buf) {
    perror("Resolve failed");
    return 1;
  } else {
    printf("Resolved: %s", t_realpath_buf);
    free(t_realpath_buf);
    return 0;
  }
}
''')
    if not os.path.exists('boot'):
      os.mkdir('boot')
    open(os.path.join('boot', 'README.txt'), 'w').write(' ')
    Popen([PYTHON, EMCC, 'src.c', '--embed-file', 'boot']).communicate()
    self.assertContained('Resolved: /boot/README.txt', run_js('a.out.js'))

  def test_realpath_2(self):
    open('src.c', 'w').write(r'''
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int testrealpath(const char* path)    {
  errno = 0;
  char *t_realpath_buf = realpath(path, NULL);
  if (NULL == t_realpath_buf) {
    printf("Resolve failed: \"%s\"\n",path);fflush(stdout);
    return 1;
  } else {
    printf("Resolved: \"%s\" => \"%s\"\n", path, t_realpath_buf);fflush(stdout);
    free(t_realpath_buf);
    return 0;
  }
}

int main(int argc, char **argv)
{
    // files:
    testrealpath("testfile.txt");
    testrealpath("Folder/testfile.txt");
    testrealpath("testnonexistentfile.txt");
    // folders
    testrealpath("Folder");
    testrealpath("/Folder");
    testrealpath("./");
    testrealpath("");
    testrealpath("/");
    return 0;
}
''')
    open('testfile.txt', 'w').write('')
    if not os.path.exists('Folder'): os.mkdir('Folder')
    open(os.path.join('Folder', 'testfile.txt'), 'w').write('')
    check_execute([PYTHON, EMCC, 'src.c', '--embed-file', 'testfile.txt', '--embed-file', 'Folder'])
    self.assertContained('''Resolved: "testfile.txt" => "/testfile.txt"
Resolved: "Folder/testfile.txt" => "/Folder/testfile.txt"
Resolve failed: "testnonexistentfile.txt"
Resolved: "Folder" => "/Folder"
Resolved: "/Folder" => "/Folder"
Resolved: "./" => "/"
Resolve failed: ""
Resolved: "/" => "/"
''', run_js('a.out.js'))

  def test_no_warnings(self):
    out, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_libcxx.cpp')], stderr=PIPE).communicate()
    assert err == '', err

  def test_emterpreter_file_suggestion(self):
    for linkable in [0, 1]:
      for to_file in [0, 1]:
        self.clear()
        cmd = [PYTHON, EMCC, '-s', 'EMTERPRETIFY=1', path_from_root('tests', 'hello_libcxx.cpp'), '-s', 'LINKABLE=' + str(linkable), '-O1', '-s', 'USE_ZLIB=1']
        if to_file:
          cmd += ['-s', 'EMTERPRETIFY_FILE="code.dat"']
        print cmd
        stdout, stderr = Popen(cmd, stderr=PIPE).communicate()
        need_warning = linkable and not to_file
        assert ('''warning: emterpreter bytecode is fairly large''' in stderr) == need_warning, stderr
        assert ('''It is recommended to use  -s EMTERPRETIFY_FILE=..''' in stderr) == need_warning, stderr

  def test_llvm_lto(self):
    sizes = {}
    for lto in [0, 1, 2, 3]:
      cmd = [PYTHON, EMCC, path_from_root('tests', 'hello_libcxx.cpp'), '-O2', '--llvm-lto', str(lto)]
      print cmd
      check_execute(cmd)
      self.assertContained('hello, world!', run_js('a.out.js'))
      sizes[lto] = os.stat('a.out.js').st_size
    print sizes
    assert sizes[1] < sizes[0] # lto reduces size
    assert sizes[2] > sizes[0] # fake lto is aggressive at increasing code size
    assert sizes[3] not in set([sizes[0], sizes[1], sizes[2]]) # mode 3 is different (deterministic builds means this tests an actual change)

  def test_split_memory(self): # make sure multiple split memory chunks get used
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
int main() {
  int x = 5;
  EM_ASM_({
    var allocs = [];
    allocs.push([]);
    allocs.push([]);
    allocs.push([]);
    allocs.push([]);
    var ptr = $0;
    assert(ptr >= STACK_BASE && ptr < STACK_MAX, 'ptr should be on stack, but ' + [STACK_BASE, STACK_MAX, ptr]);
    function getIndex(x) {
      return x >> SPLIT_MEMORY_BITS;
    }
    assert(ptr < SPLIT_MEMORY, 'in first chunk');
    assert(getIndex(ptr) === 0, 'definitely in first chunk');
    // allocate into other chunks
    do {
      var t = Module._malloc(1024*1024);
      allocs[getIndex(t)].push(t);
      Module.print('allocating, got in ' + getIndex(t));
    } while (getIndex(t) === 0);
    assert(getIndex(t) === 1, 'allocated into second chunk');
    do {
      var t = Module._malloc(1024*1024);
      allocs[getIndex(t)].push(t);
      Module.print('more allocating, got in ' + getIndex(t));
    } while (getIndex(t) === 1);
    assert(getIndex(t) === 2, 'into third chunk');
    do {
      var t = Module._malloc(1024*1024);
      allocs[getIndex(t)].push(t);
      Module.print('more allocating, got in ' + getIndex(t));
    } while (getIndex(t) === 2);
    assert(getIndex(t) === 3, 'into third chunk');
    // write values
    assert(allocs[1].length > 5 && allocs[2].length > 5);
    for (var i = 0; i < allocs[1].length; i++) {
      HEAPU8[allocs[1][i]] = i & 255
    }
    for (var i = 0; i < allocs[2].length; i++) {
      HEAPU8[allocs[2][i]] = (i*i) & 255;
    }
    for (var i = 0; i < allocs[1].length; i++) {
      assert(HEAPU8[allocs[1][i]] === (i & 255));
    }
    for (var i = 0; i < allocs[2].length; i++) {
      assert(HEAPU8[allocs[2][i]] === ((i*i) & 255));
    }
    Module.print('success.');
  }, &x);
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts)])
      self.assertContained('success.', run_js('a.out.js'))

  def test_split_memory_2(self): # make allocation starts in the first chunk, and moves forward properly
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
#include <assert.h>
int split_memory = 0;
int alloc_where_is_it() {
  static void *last;
  void *ptr = malloc(1024);
  static int counter = 0;
  if (last && (counter++ % 2 == 1)) ptr = realloc(last, 512*1024); // throw in some reallocs of a previous allocation
  last = ptr;
  unsigned x = (unsigned)ptr;
  return x / split_memory;
}
int main() {
  split_memory = EM_ASM_INT_V({
    return SPLIT_MEMORY;
  });
  int counter = 0;
  while (alloc_where_is_it() == 0) {
    counter++;
  }
  printf("allocations in first chunk: %d\n", counter);
  assert(counter > 10); // less in first chunk
  while (alloc_where_is_it() == 1) {
    counter++;
  }
  printf("allocations in first chunk: %d\n", counter);
  assert(counter > 20);
  counter = 0;
  while (alloc_where_is_it() == 2) {
    counter++;
  }
  printf("allocations in second chunk: %d\n", counter);
  assert(counter > 20);
  EM_ASM( Module.print('success.') );
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts)])
      self.assertContained('success.', run_js('a.out.js'))

  def test_split_memory_sbrk(self):
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
int split_memory;
int where(int x) {
  return x / split_memory;
}
int main() {
  split_memory = EM_ASM_INT_V({
    return SPLIT_MEMORY;
  });
  int sbrk_0 = (int)sbrk(0);
  printf("sbrk(0): %d\n", sbrk_0);
  assert(sbrk_0 > 0 && sbrk_0 != -1);
  int sbrk_index = where(sbrk_0);
  assert(sbrk_index > 0 && sbrk_index < 10);
  assert(sbrk(0) == (void*)sbrk_0);
  int one = (int)sbrk(10);
  printf("one: %d\n", one);
  assert(sbrk_0 + 10 == sbrk(0));
  int two = (int)sbrk(20);
  printf("two: %d\n", two);
  assert(sbrk_0 + 10 == two);
  assert(sbrk(-20) == (void*)(two + 20));
  assert(sbrk(-10) == (void*)two);
  int bad = sbrk(split_memory * 2);
  assert(bad == -1);
  EM_ASM( Module.print('success.') );
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts)])
      self.assertContained('success.', run_js('a.out.js'))

  def test_split_memory_faking(self): # fake HEAP8 etc. objects have some faked fake method. they are fake
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
int main() {
  EM_ASM({
    var x = Module._malloc(1024);
    // set
    HEAPU8.set([1,2,3,4], x);
    assert(get8(x+0) === 1);
    assert(get8(x+1) === 2);
    assert(get8(x+2) === 3);
    assert(get8(x+3) === 4);
    // subarray
    var s1 = HEAPU8.subarray(x+2, x+4);
    assert(s1 instanceof Uint8Array);
    assert(s1.length === 2);
    assert(s1[0] === 3);
    assert(s1[1] === 4);
    assert(get8(x+2) === 3);
    s1[0] = 57;
    assert(get8(x+2) === 57);
    // subarray without second param
    var s2 = HEAPU8.subarray(x+2);
    assert(s2 instanceof Uint8Array);
    assert(s2.length > 2);
    assert(s2[0] === 57);
    assert(s2[1] === 4);
    assert(get8(x+2) === 57);
    s2[0] = 98;
    assert(get8(x+2) === 98);
    // buffer.slice
    var b = HEAPU8.buffer.slice(x, x+4);
    assert(b instanceof ArrayBuffer);
    assert(b.byteLength === 4);
    var s = new Uint8Array(b);
    assert(s[0] === 1);
    assert(s[1] === 2);
    assert(s[2] === 98);
    assert(s[3] === 4);
    // check for a bananabread-discovered bug
    var s32 = HEAPU32.subarray(x >> 2, x + 4 >> 2);
    assert(s32 instanceof Uint32Array);
    assert(s32.length === 1);
    assert(s32[0] === 0x04620201, s32[0]);
    // misc subarrays, check assertions only
    SPLIT_MEMORY = 256;
    SPLIT_MEMORY_BITS = 8;
    function getChunk(x) {
      return x >> SPLIT_MEMORY_BITS;
    }
    assert(TOTAL_MEMORY >= SPLIT_MEMORY*3);
    var p = Module.print;
    var e = Module.printErr;
    Module.printErr = Module.print = function(){};
    var fail = false;
    if (!buffers[1]) allocateSplitChunk(1); // we will slice into this
    if (!buffers[2]) allocateSplitChunk(2); // we will slice into this
    TOP:
    for (var i = 0; i < SPLIT_MEMORY*3; i++) {
      HEAPU8.subarray(i);
      if ((i&3) === 0) HEAPU32.subarray(i >> 2);
      for (var j = 1; j < SPLIT_MEMORY*3; j++) {
        //printErr([i, j]);
        if (getChunk(i) == getChunk(j-1) || j <= i) {
          HEAPU8.subarray(i, j);
          if ((i&3) === 0) HEAPU32.subarray(i >> 2, j >> 2);
        } else {
          // expect failures
          try {
            HEAPU8.subarray(i, j);
            fail = ['U8', i, j];
            break TOP;
          } catch (e) {}
          if ((i&3) === 0 && (j&3) === 0) {
            try {
              HEAPU32.subarray(i >> 2, j >> 2);
              fail = ['U32', i, j];
              break TOP;
            } catch (e) {}
          }
          break; // stop inner loop, once we saw different chunks, go to a new i
        }
      }
    }
    Module.print = p;
    Module.printErr = e;
    if (fail) Module.print('FAIL. ' + fail);
    else Module.print('success.');
  });
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts), '-s', 'ASSERTIONS=1'])
      self.assertContained('success.', run_js('a.out.js', stderr=PIPE, assert_returncode=None))

  def test_split_memory_release(self):
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
int main() {
  EM_ASM({
    assert(buffers[0]); // always here
    assert(!buffers[1]);
    assert(!buffers[2]);
    function getIndex(x) {
      return x >> SPLIT_MEMORY_BITS;
    }
    do {
      var t = Module._malloc(1024*1024);
      Module.print('allocating, got in ' + getIndex(t));
    } while (getIndex(t) === 0);
    assert(getIndex(t) === 1, 'allocated into first chunk');
    assert(buffers[1]); // has been allocated now
    do {
      var t = Module._malloc(1024*1024);
      Module.print('allocating, got in ' + getIndex(t));
    } while (getIndex(t) === 1);
    assert(getIndex(t) === 2, 'allocated into second chunk');
    assert(buffers[2]); // has been allocated now
    Module._free(t);
    assert(!buffers[2]); // has been freed now
    var more = [];
    for (var i = 0 ; i < 1024; i++) {
      more.push(Module._malloc(10));
    }
    assert(buffers[2]); // has been allocated again
    for (var i = 0 ; i < 1024; i++) {
      Module._free(more[i]);
    }
    assert(!buffers[2]); // has been freed again
    Module.print('success.');
  });
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts)])
      self.assertContained('success.', run_js('a.out.js'))

  def test_split_memory_use_existing(self):
    open('src.c', 'w').write(r'''
#include <emscripten.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
int main() {
  EM_ASM({
    function getIndex(x) {
      return x >> SPLIT_MEMORY_BITS;
    }
    var t;
    do {
      t = Module._malloc(1024*1024);
    } while (getIndex(t) === 0);
    assert(getIndex(t) === 1, 'allocated into first chunk');
    assert(!buffers[2]);
    var existing = new Uint8Array(1024); // ok to be smaller
    allocateSplitChunk(2, existing.buffer);
    assert(buffers[2]);
    existing[0] = 12;
    existing[50] = 98;
    var p = SPLIT_MEMORY*2;
    assert(HEAPU8[p+0] === 12 && HEAPU8[p+50] === 98); // mapped into the normal memory space!
    HEAPU8[p+33] = 201;
    assert(existing[33] === 201); // works both ways
    do {
      t = Module._malloc(1024*1024);
    } while (getIndex(t) === 1);
    assert(getIndex(t) === 3, 'should skip chunk 2, since it is used by us, but seeing ' + getIndex(t));
    assert(HEAPU8[p+0] === 12 && HEAPU8[p+50] === 98);
    assert(existing[33] === 201);
    Module.print('success.');
  });
}
''')
    for opts in [0, 1, 2]:
      print opts
      check_execute([PYTHON, EMCC, 'src.c', '-s', 'SPLIT_MEMORY=8388608', '-s', 'TOTAL_MEMORY=50000000', '-O' + str(opts)])
      self.assertContained('success.', run_js('a.out.js'))

  def test_sixtyfour_bit_return_value(self):
    # This test checks that the most significant 32 bits of a 64 bit long are correctly made available
    # to native JavaScript applications that wish to interact with compiled code returning 64 bit longs.
    # The MS 32 bits should be available in Runtime.getTempRet0() even when compiled with -O2 --closure 1

    # Compile test.c and wrap it in a native JavaScript binding so we can call our compiled function from JS.
    check_execute([PYTHON, EMCC, path_from_root('tests', 'return64bit', 'test.c'), '--pre-js', path_from_root('tests', 'return64bit', 'testbindstart.js'), '--pre-js', path_from_root('tests', 'return64bit', 'testbind.js'), '--post-js', path_from_root('tests', 'return64bit', 'testbindend.js'), '-s', 'EXPORTED_FUNCTIONS=["_test"]', '-o', 'test.js', '-O2', '--closure', '1'])

    # Simple test program to load the test.js binding library and call the binding to the
    # C function returning the 64 bit long.
    open(os.path.join(self.get_dir(), 'testrun.js'), 'w').write('''
      var test = require("./test.js");
      test.runtest();
    ''')

    # Run the test and confirm the output is as expected.
    out = run_js('testrun.js', full_output=True)
    assert "low = 5678" in out
    assert "high = 1234" in out

  def test_lib_include_flags(self):
    process = Popen([PYTHON, EMCC] + '-l m -l c -I'.split() + [path_from_root('tests', 'include_test'), path_from_root('tests', 'lib_include_flags.c')], stdout=PIPE, stderr=PIPE)
    process.communicate()
    assert process.returncode is 0, 'Empty -l/-L/-I flags should read the next arg as a param'

  def test_dash_s(self):
    print check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '-s', '-std=c++03'])
    self.assertContained('hello, world!', run_js('a.out.js'))

  def test_python_2_3(self): # check emcc/em++ can be called by any python
    print
    for python in ['python', 'python2', 'python3']:
      try:
        check_execute([python, '--version'])
        has = True
      except:
        has = False
      print python, has
      if has:
        print '  checking emcc...'
        check_execute([python, EMCC, '--version'])
        print '  checking em++...'
        check_execute([python, EMXX, '--version'])
        if python == 'python2':
          print '  checking emcc.py...'
          check_execute([python, EMCC + '.py', '--version'])

  def test_zeroinit(self):
    open('src.c', 'w').write(r'''
#include <stdio.h>
int buf[1048576];
int main() {
  printf("hello, world! %d\n", buf[123456]);
  return 0;
}
''')
    check_execute([PYTHON, EMCC, 'src.c', '-O2', '-g'])
    size = os.stat('a.out.js.mem').st_size
    assert size < 4096, size

  def test_separate_asm_warning(self):
    # Test that -s PRECISE_F32=2 raises a warning that --separate-asm is implied.
    stdout, stderr = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'PRECISE_F32=2', '-o', 'a.html'], stderr=PIPE).communicate()
    self.assertContained('forcing separate asm output', stderr)

    # Test that -s PRECISE_F32=2 --separate-asm should not post a warning.
    stdout, stderr = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'PRECISE_F32=2', '-o', 'a.html', '--separate-asm'], stderr=PIPE).communicate()
    self.assertNotContained('forcing separate asm output', stderr)

    # Test that -s PRECISE_F32=1 should not post a warning.
    stdout, stderr = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-s', 'PRECISE_F32=1', '-o', 'a.html'], stderr=PIPE).communicate()
    self.assertNotContained('forcing separate asm output', stderr)

    # Manually doing separate asm should show a warning, if not targeting html
    warning = '--separate-asm works best when compiling to HTML'
    stdout, stderr = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '--separate-asm'], stderr=PIPE).communicate()
    self.assertContained(warning, stderr)
    stdout, stderr = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '--separate-asm', '-o', 'a.html'], stderr=PIPE).communicate()
    self.assertNotContained(warning, stderr)

  def test_canonicalize_nan_warning(self):
    open('src.cpp', 'w').write(r'''
#include <stdio.h>

union U {
  int x;
  float y;
} a;


int main() {
  a.x = 0x7FC01234;
  printf("%f\n", a.y);
}
''')

    stdout, stderr = Popen([PYTHON, EMCC, 'src.cpp', '-O1'], stderr=PIPE).communicate()
    self.assertContained("emcc: warning: cannot represent a NaN literal", stderr)

    stdout, stderr = Popen([PYTHON, EMCC, 'src.cpp', '-O1', '-g'], stderr=PIPE).communicate()
    self.assertContained("emcc: warning: cannot represent a NaN literal", stderr)
    self.assertContained('//@line 12 "src.cpp"', stderr)

  def test_only_my_code(self):
    check_execute([PYTHON, EMCC, '-O1', path_from_root('tests', 'hello_world.c'), '--separate-asm'])
    count = open('a.out.asm.js').read().count('function ')
    assert count > 30, count # libc brings in a bunch of stuff

    def test(filename, opts, expected_funcs, expected_vars):
      print filename, opts
      check_execute([PYTHON, EMCC, path_from_root('tests', filename), '--separate-asm', '-s', 'ONLY_MY_CODE=1'] + opts)
      full = 'var Module = {};\n' + open('a.out.asm.js').read()
      open('asm.js', 'w').write(full)
      funcs = open('a.out.asm.js').read().count('function ')
      vars_ = open('a.out.asm.js').read().count('var ')
      assert funcs == expected_funcs, funcs
      assert vars_ == expected_vars, vars_
      if SPIDERMONKEY_ENGINE in JS_ENGINES:
        out = run_js('asm.js', engine=SPIDERMONKEY_ENGINE, stderr=STDOUT)
        self.validate_asmjs(out)
      else:
        print '(skipping asm.js validation check)'

    test('hello_123.c', ['-O1'], 1, 2)
    test('fasta.cpp', ['-O3', '-g2'], 2, 13)

  def test_link_response_file_does_not_force_absolute_paths(self):
    with_space = 'with space'
    directory_with_space_name = os.path.join(self.get_dir(), with_space)
    if not os.path.exists(directory_with_space_name):
      os.makedirs(directory_with_space_name)

    main = '''
      int main() {
        return 0;
      }
    '''
    main_file_name = 'main.cpp'
    main_path_name = os.path.join(directory_with_space_name, main_file_name)
    open(main_path_name, 'w').write(main)
    main_object_file_name = 'main.cpp.o'

    Building.emcc(main_path_name, ['-g'])

    current_directory = os.getcwd()
    os.chdir(os.path.join(current_directory, with_space))

    link_args = Building.link([main_object_file_name], os.path.join(self.get_dir(), 'all.bc'), just_calculate=True)

    os.chdir(current_directory)

    # We want only the relative path to be in the linker args, it should not be converted to an absolute path.
    self.assertItemsEqual(link_args, [main_object_file_name])

  def test_memory_growth_noasm(self):
    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.c'), '-O2', '-s', 'ALLOW_MEMORY_GROWTH=1'])
    src = open('a.out.js').read()
    assert 'use asm' not in src

  def test_eval_ctors(self):
    print 'non-terminating ctor'
    src = r'''
      struct C {
        C() {
          volatile int y = 0;
          while (y == 0) {}
        }
      };
      C always;
      int main() {}
    '''
    open('src.cpp', 'w').write(src)
    check_execute([PYTHON, EMCC, 'src.cpp', '-O2', '-s', 'EVAL_CTORS=1', '-profiling-funcs'])
    print 'check no ctors is ok'
    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.cpp'), '-Oz'])
    self.assertContained('hello, world!', run_js('a.out.js'))
    # on by default in -Oz, but user-overridable
    def get_size(args):
      print 'get_size', args
      check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_libcxx.cpp')] + args)
      self.assertContained('hello, world!', run_js('a.out.js'))
      return (os.stat('a.out.js').st_size, os.stat('a.out.js.mem').st_size)
    def check_size(left, right):
      if left[0] == right[0] and left[1] == right[1]: return 0
      if left[0] < right[0] and left[1] > right[1]: return -1 # smaller js, bigger mem
      if left[0] > right[0] and left[1] < right[1]: return 1
      assert 0, [left, right]
    o2_size = get_size(['-O2'])
    assert check_size(get_size(['-O2']), o2_size) == 0, 'deterministic'
    assert check_size(get_size(['-O2', '-s', 'EVAL_CTORS=1']), o2_size) < 0, 'eval_ctors works if user asks for it'
    oz_size = get_size(['-Oz'])
    assert check_size(get_size(['-Oz']), oz_size) == 0, 'deterministic'
    assert check_size(get_size(['-Oz', '-s', 'EVAL_CTORS=1']), oz_size) == 0, 'eval_ctors is on by default in oz'
    assert check_size(get_size(['-Oz', '-s', 'EVAL_CTORS=0']), oz_size) == 1, 'eval_ctors can be turned off'

    linkable_size =   get_size(['-Oz', '-s', 'EVAL_CTORS=1', '-s', 'LINKABLE=1'])
    assert check_size(get_size(['-Oz', '-s', 'EVAL_CTORS=0', '-s', 'LINKABLE=1']), linkable_size) == 1, 'noticeable difference in linkable too'

    # ensure order of execution remains correct, even with a bad ctor
    def test(p1, p2, p3, last, expected):
      src = r'''
        #include <stdio.h>
        #include <stdlib.h>
        volatile int total = 0;
        struct C {
          C(int x) {
            volatile int y = x;
            y++;
            y--;
            if (y == 0xf) {
              printf("you can't eval me ahead of time\n"); // bad ctor
            }
            total <<= 4;
            total += int(y);
          }
        };
        C __attribute__((init_priority(%d))) c1(0x5);
        C __attribute__((init_priority(%d))) c2(0x8);
        C __attribute__((init_priority(%d))) c3(%d);
        int main() {
          printf("total is 0x%%x.\n", total);
        }
      ''' % (p1, p2, p3, last)
      open('src.cpp', 'w').write(src)
      check_execute([PYTHON, EMCC, 'src.cpp', '-O2', '-s', 'EVAL_CTORS=1', '-profiling-funcs'])
      self.assertContained('total is %s.' % hex(expected), run_js('a.out.js'))
      shutil.copyfile('a.out.js', 'x' + hex(expected) + '.js')
      return open('a.out.js').read().count('function _')
    print 'no bad ctor'
    first  = test(1000, 2000, 3000, 0xe, 0x58e)
    second = test(3000, 1000, 2000, 0xe, 0x8e5)
    third  = test(2000, 3000, 1000, 0xe, 0xe58)
    print first, second, third
    assert first == second and second == third
    print 'with bad ctor'
    first  = test(1000, 2000, 3000, 0xf, 0x58f) # 2 will succeed
    second = test(3000, 1000, 2000, 0xf, 0x8f5) # 1 will succedd
    third  = test(2000, 3000, 1000, 0xf, 0xf58) # 0 will succeed
    print first, second, third
    assert first < second and second < third

    print 'helpful output'
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    try:
      os.environ['EMCC_DEBUG'] = '1'
      open('src.cpp', 'w').write(r'''
#include <stdio.h>
struct C {
  C() { printf("constructing!\n"); } // don't remove this!
};
C c;
int main() {}
      ''')
      out, err = Popen([PYTHON, EMCC, 'src.cpp', '-Oz'], stderr=PIPE).communicate()
      self.assertContained('___syscall54', err) # the failing call should be mentioned
      self.assertContained('ctorEval.js', err) # with a stack trace
      self.assertContained('ctor_evaller: not successful', err) # with logging
    finally:
      del os.environ['EMCC_DEBUG']

  def test_override_environment(self):
    open('main.cpp', 'w').write(r'''
      #include <emscripten.h>
      int main() {
        EM_ASM({
          Module.print('environment is WEB? ' + ENVIRONMENT_IS_WEB);
          Module.print('environment is WORKER? ' + ENVIRONMENT_IS_WORKER);
          Module.print('environment is NODE? ' + ENVIRONMENT_IS_NODE);
          Module.print('environment is SHELL? ' + ENVIRONMENT_IS_SHELL);
        });
      }
''')
    check_execute([PYTHON, EMCC, 'main.cpp'])
    src = open('a.out.js').read()
    envs = ['WEB', 'WORKER', 'NODE', 'SHELL']
    for env in envs:
      curr = 'var Module = { ENVIRONMENT: "%s" };\n' % env
      open('test.js', 'w').write(curr + src)
      for engine in JS_ENGINES:
        if engine == V8_ENGINE: continue # ban v8, weird failures
        actual = 'NODE' if engine == NODE_JS else 'SHELL'
        print env, actual, engine
        fail = False
        try:
          seen = run_js('test.js', engine=engine, stderr=PIPE)
        except:
          fail = True
        if fail:
          print '-- acceptable fail'
          assert actual != env, 'ok to fail if in the wrong environment'
        else:
          for other in envs:
            if env == other:
              assert ('environment is %s? true' % other) in seen, seen
            else:
              assert ('environment is %s? false' % other) in seen, seen
          print '-- verified proper env is shown'

  def test_warn_no_filesystem(self):
    WARNING = 'Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1'

    check_execute([PYTHON, EMCC, path_from_root('tests', 'hello_world.c')])
    seen = run_js('a.out.js', stderr=PIPE)
    assert WARNING not in seen

    def test(contents):
      open('src.cpp', 'w').write(r'''
  #include<stdio.h>
  #include<emscripten.h>
  int main() {
    EM_ASM({ %s });
    printf("hello, world!\n");
    return 0;
  }
  ''' % contents)
      check_execute([PYTHON, EMCC, 'src.cpp'])
      self.assertContained(WARNING, run_js('a.out.js', stderr=PIPE, assert_returncode=None))

    # might appear in handwritten code
    test("FS.init()")
    test("FS.createPreloadedFile('waka waka, just warning check')");
    test("FS.createDataFile('waka waka, just warning check')");
    test("FS.analyzePath('waka waka, just warning check')");
    test("FS.loadFilesFromDB('waka waka, just warning check')");
    # might appear in filesystem code from a separate script tag
    test("Module['FS_createDataFile']('waka waka, just warning check')");
    test("Module['FS_createPreloadedFile']('waka waka, just warning check')");

    # text is in the source when needed, but when forcing FS, it isn't there
    check_execute([PYTHON, EMCC, 'src.cpp'])
    self.assertContained(WARNING, open('a.out.js').read())
    check_execute([PYTHON, EMCC, 'src.cpp', '-s', 'FORCE_FILESYSTEM=1']) # forcing FS means no need
    self.assertNotContained(WARNING, open('a.out.js').read())
    check_execute([PYTHON, EMCC, 'src.cpp', '-s', 'ASSERTIONS=0']) # no assertions, no need
    self.assertNotContained(WARNING, open('a.out.js').read())
    check_execute([PYTHON, EMCC, 'src.cpp', '-O2']) # optimized, so no assertions
    self.assertNotContained(WARNING, open('a.out.js').read())

  ############################################################
  # Function eliminator tests
  ############################################################
  def normalize_line_endings(self, input):
    return input.replace('\r\n', '\n').replace('\n\n', '\n').replace('\n\n', '\n')

  def get_file_contents(self, file):
    file_contents = ""
    with open(file) as fout:
      file_contents = "".join(fout.readlines())

    file_contents = self.normalize_line_endings(file_contents)

    return file_contents

  def function_eliminator_test_helper(self, input_file, expected_output_file, use_hash_info=False):
    input_file = path_from_root('tests', 'optimizer', input_file)
    expected_output_file = path_from_root('tests', 'optimizer', expected_output_file)
    command = [path_from_root('tools', 'eliminate-duplicate-functions.js'), input_file, '--no-minimize-whitespace', '--use-asm-ast']

    if use_hash_info:
      command.append('--use-hash-info')

    output, err = Popen(NODE_JS + command, stdin=PIPE, stderr=PIPE, stdout=PIPE).communicate()
    assert err == '', err
    expected_output = self.get_file_contents(expected_output_file)
    output = self.normalize_line_endings(output)

    self.assertIdentical(expected_output, output)

  def test_function_eliminator_simple(self):
    self.function_eliminator_test_helper('test-function-eliminator-simple.js',
                                         'test-function-eliminator-simple-output.js')

  def test_function_eliminator_replace_function_call(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-function-call.js',
                                        'test-function-eliminator-replace-function-call-output.js')

  def test_function_eliminator_replace_function_call_two_passes(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-function-call-output.js',
                                         'test-function-eliminator-replace-function-call-two-passes-output.js')

  def test_function_eliminator_replace_array_value(self):
    import tools.duplicate_function_eliminator
    output_file = 'output.js'

    try:
      tools.shared.safe_copy(path_from_root('tests', 'optimizer', 'test-function-eliminator-replace-array-value.js'), output_file)

      tools.duplicate_function_eliminator.run(output_file)

      output_file_contents = self.get_file_contents(output_file)

      expected_file_contents = self.get_file_contents(path_from_root('tests', 'optimizer', 'test-function-eliminator-replace-array-value-output.js'))

      self.assertIdentical(output_file_contents, expected_file_contents)
    finally:
      tools.tempfiles.try_delete(output_file)

  def test_function_eliminator_replace_object_value_assignment(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-object-value-assignment.js',
                                         'test-function-eliminator-replace-object-value-assignment-output.js')

  def test_function_eliminator_variable_clash(self):
    self.function_eliminator_test_helper('test-function-eliminator-variable-clash.js',
                                         'test-function-eliminator-variable-clash-output.js')

  def test_function_eliminator_replace_variable_value(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-variable-value.js',
                                         'test-function-eliminator-replace-variable-value-output.js')

  def test_function_eliminator_double_parsed_correctly(self):
    # This is a test that makes sure that when we perform final optimization on
    # the JS file, doubles are preserved (and not converted to ints).
    import tools.tempfiles
    import tools.duplicate_function_eliminator
    import tools.js_optimizer

    output_file = 'output.js'

    try:
      tools.shared.safe_copy(path_from_root('tests', 'optimizer', 'test-function-eliminator-double-parsed-correctly.js'), output_file)

      # Run duplicate function elimination
      tools.duplicate_function_eliminator.run(output_file)

      # Run last opts
      tools.shared.safe_copy(tools.js_optimizer.run(output_file, ['last', 'asm']), output_file)
      output_file_contents = self.get_file_contents(output_file)

      # Compare
      expected_file_contents = self.get_file_contents(path_from_root('tests', 'optimizer', 'test-function-eliminator-double-parsed-correctly-output.js'))
      self.assertIdentical(output_file_contents, expected_file_contents)
    finally:
      tools.tempfiles.try_delete(output_file)

  # Now do the same, but using a pre-generated equivalent function hash info that
  # comes in handy for parallel processing
  def test_function_eliminator_simple_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-simple-with-hash-info.js',
                                         'test-function-eliminator-simple-output.js',
                                         use_hash_info=True)

  def test_function_eliminator_replace_function_call_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-function-call-with-hash-info.js',
                                         'test-function-eliminator-replace-function-call-output.js',
                                         use_hash_info=True)

  def test_function_eliminator_replace_function_call_two_passes_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-function-call-output-with-hash-info.js',
                                         'test-function-eliminator-replace-function-call-two-passes-output.js',
                                         use_hash_info=True)

  def test_function_eliminator_replace_object_value_assignment_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-object-value-assignment-with-hash-info.js',
                                         'test-function-eliminator-replace-object-value-assignment-output.js',
                                         use_hash_info=True)

  def test_function_eliminator_variable_clash_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-variable-clash-with-hash-info.js',
                                         'test-function-eliminator-variable-clash-output.js',
                                         use_hash_info=True)

  def test_function_eliminator_replace_variable_value_with_hash_info(self):
    self.function_eliminator_test_helper('test-function-eliminator-replace-variable-value-with-hash-info.js',
                                         'test-function-eliminator-replace-variable-value-output.js',
                                         use_hash_info=True)

  def test_cyberdwarf_pointers(self):
    check_execute([PYTHON, EMCC, path_from_root('tests', 'debugger', 'test_pointers.cpp'), '-Oz', '-s', 'CYBERDWARF=1',
    '-std=c++11', '--pre-js', path_from_root('tests', 'debugger', 'test_preamble.js'), '-o', 'test_pointers.js' ], stderr=PIPE)
    run_js('test_pointers.js', engine=NODE_JS)

  def test_cyberdwarf_union(self):
    check_execute([PYTHON, EMCC, path_from_root('tests', 'debugger', 'test_union.cpp'), '-Oz', '-s', 'CYBERDWARF=1',
    '-std=c++11', '--pre-js', path_from_root('tests', 'debugger', 'test_preamble.js'), '-o', 'test_union.js' ], stderr=PIPE)
    run_js('test_union.js', engine=NODE_JS)

  def test_source_file_with_fixed_language_mode(self):
    open('src_tmp_fixed_lang', 'w').write('''
#include <string>
#include <iostream>

int main() {
  std::cout << "Test_source_fixed_lang_hello" << std::endl;
  return 0;
}
    ''')
    stdout, stderr = Popen([PYTHON, EMCC, '-Wall', '-std=c++14', '-x', 'c++', 'src_tmp_fixed_lang'], stderr=PIPE).communicate()
    self.assertNotContained("Input file has an unknown suffix, don't know what to do with it!", stderr)
    self.assertNotContained("Unknown file suffix when compiling to LLVM bitcode", stderr)
    self.assertContained("Test_source_fixed_lang_hello", run_js('a.out.js'))
    
    stdout, stderr = Popen([PYTHON, EMCC, '-Wall', '-std=c++14', 'src_tmp_fixed_lang'], stderr=PIPE).communicate()
    self.assertContained("Input file has an unknown suffix, don't know what to do with it!", stderr)
