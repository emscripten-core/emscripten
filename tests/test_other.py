import multiprocessing, os, re, shutil, subprocess, sys
import tools.shared
from tools.shared import *
from runner import RunnerCore, path_from_root, get_bullet_library

class other(RunnerCore):
  def get_zlib_library(self):
    if WINDOWS:
      return self.get_library('zlib', os.path.join('libz.a'), configure=['emconfigure.bat'], configure_args=['cmake', '.', '-DBUILD_SHARED_LIBS=OFF'], make=['mingw32-make'], make_args=[])
    else:
      return self.get_library('zlib', os.path.join('libz.a'), make_args=['libz.a'])
    
  def test_emcc(self):
    for compiler in [EMCC, EMXX]:
      shortcompiler = os.path.basename(compiler)
      suffix = '.c' if compiler == EMCC else '.cpp'

      # --version
      output = Popen([PYTHON, compiler, '--version'], stdout=PIPE, stderr=PIPE).communicate()
      output = output[0].replace('\r', '')
      self.assertContained('''emcc (Emscripten GCC-like replacement)''', output)
      self.assertContained('''Copyright (C) 2013 the Emscripten authors (see AUTHORS.txt)
This is free and open source software under the MIT license.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
''', output)

      # -v, without input files
      output = Popen([PYTHON, compiler, '-v'], stdout=PIPE, stderr=PIPE).communicate()
      self.assertContained('''clang version''', output[1].replace('\r', ''), output[1].replace('\r', ''))

      # --help
      output = Popen([PYTHON, compiler, '--help'], stdout=PIPE, stderr=PIPE).communicate()
      self.assertContained('''%s [options] file...

Most normal gcc/g++ options will work, for example:
  --help                   Display this information
  --version                Display compiler version information

Options that are modified or new in %s include:

  -O0                      No optimizations (default)''' % (shortcompiler, shortcompiler), output[0].replace('\r', ''), output[1].replace('\r', ''))

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
      for args in [['-c'], ['-o', 'src.o'], ['-o', 'src.bc'], ['-o', 'src.so'], ['-o', 'js']]:
        target = args[1] if len(args) == 2 else 'hello_world.o'
        self.clear()
        Popen([PYTHON, compiler, path_from_root('tests', 'hello_world' + suffix)] + args, stdout=PIPE, stderr=PIPE).communicate()
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

      # dlmalloc. dlmalloc is special in that it is the only part of libc that is (1) hard to write well, and
      # very speed-sensitive. So we do not implement it in JS in library.js, instead we compile it from source
      for source, has_malloc in [('hello_world' + suffix, False), ('hello_malloc.cpp', True)]:
        print source, has_malloc
        self.clear()
        output = Popen([PYTHON, compiler, path_from_root('tests', source)], stdout=PIPE, stderr=PIPE).communicate()
        assert os.path.exists('a.out.js'), '\n'.join(output)
        self.assertContained('hello, world!', run_js('a.out.js'))
        generated = open('a.out.js').read()
        assert ('function _malloc(bytes) {' in generated) == (not has_malloc), 'If malloc is needed, it should be there, if not not'

      # Optimization: emcc src.cpp -o something.js [-Ox]. -O0 is the same as not specifying any optimization setting
      for params, opt_level, bc_params, closure, has_malloc in [ # bc params are used after compiling to bitcode
        (['-o', 'something.js'],                          0, None, 0, 1),
        (['-o', 'something.js', '-O0'],                   0, None, 0, 0),
        (['-o', 'something.js', '-O1'],                   1, None, 0, 0),
        (['-o', 'something.js', '-O1', '-g'],             1, None, 0, 0), # no closure since debug
        (['-o', 'something.js', '-O2'],                   2, None, 0, 1),
        (['-o', 'something.js', '-O2', '-g'],             2, None, 0, 0),
        (['-o', 'something.js', '-Os'],                   2, None, 0, 1),
        (['-o', 'something.js', '-O3', '-s', 'ASM_JS=0'], 3, None, 1, 1),
        # and, test compiling to bitcode first
        (['-o', 'something.bc'], 0, [],      0, 0),
        (['-o', 'something.bc', '-O0'], 0, [], 0, 0),
        (['-o', 'something.bc', '-O1'], 1, ['-O1'], 0, 0),
        (['-o', 'something.bc', '-O2'], 2, ['-O2'], 0, 0),
        (['-o', 'something.bc', '-O3'], 3, ['-O3', '-s', 'ASM_JS=0'], 1, 0),
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
        assert ('Applying some potentially unsafe optimizations!' in output[1]) == (opt_level >= 3), 'unsafe warning should appear in opt >= 3'
        self.assertContained('hello, world!', run_js('something.js'))

        # Verify optimization level etc. in the generated code
        # XXX these are quite sensitive, and will need updating when code generation changes
        generated = open('something.js').read() # TODO: parse out the _main function itself, not support code, if the tests below need that some day
        assert 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 should be used by default'
        assert 'SAFE_HEAP' not in generated, 'safe heap should not be used by default'
        assert ': while(' not in generated, 'when relooping we also js-optimize, so there should be no labelled whiles'
        if closure:
          if opt_level == 0: assert '._main =' in generated, 'closure compiler should have been run'
          elif opt_level >= 1: assert '._main=' in generated, 'closure compiler should have been run (and output should be minified)'
        else:
          # closure has not been run, we can do some additional checks. TODO: figure out how to do these even with closure
          assert '._main = ' not in generated, 'closure compiler should not have been run'
          if keep_debug:
            assert ('(label)' in generated or '(label | 0)' in generated) == (opt_level <= 0), 'relooping should be in opt >= 1'
            assert ('assert(STACKTOP < STACK_MAX' in generated) == (opt_level == 0), 'assertions should be in opt == 0'
            assert '$i' in generated or '$storemerge' in generated or '$original' in generated, 'micro opts should always be on'
          if opt_level >= 2 and '-g' in params:
            assert re.search('HEAP8\[\$?\w+ ?\+ ?\(+\$?\w+ ?', generated) or re.search('HEAP8\[HEAP32\[', generated), 'eliminator should create compound expressions, and fewer one-time vars' # also in -O1, but easier to test in -O2
          assert ('_puts(' in generated) == (opt_level >= 1), 'with opt >= 1, llvm opts are run and they should optimize printf to puts'
          if opt_level == 0 or '-g' in params: assert 'function _main() {' in generated or 'function _main(){' in generated, 'Should be unminified'
          elif opt_level >= 2: assert ('function _main(){' in generated or '"use asm";var a=' in generated), 'Should be whitespace-minified'

      # emcc -s RELOOP=1 src.cpp ==> should pass -s to emscripten.py. --typed-arrays is a convenient alias for -s USE_TYPED_ARRAYS
      for params, test, text in [
        (['-O2'], lambda generated: 'function intArrayToString' in generated, 'shell has unminified utilities'),
        (['-O2', '--closure', '1'], lambda generated: 'function intArrayToString' not in generated, 'closure minifies the shell'),
        (['-O2'], lambda generated: 'var b=0' in generated and not 'function _main' in generated, 'registerize/minify is run by default in -O2'),
        (['-O2', '--minify', '0'], lambda generated: 'var b = 0' in generated and not 'function _main' in generated, 'minify is cancelled, but not registerize'),
        (['-O2', '--js-opts', '0'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'js opts are cancelled'),
        (['-O2', '-g'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'registerize/minify is cancelled by -g'),
        (['-O2', '-g0'], lambda generated: 'var b=0'   in generated and not 'function _main' in generated, 'registerize/minify is run by default in -O2 -g0'),
        (['-O2', '-g1'], lambda generated: 'var b = 0' in generated and not 'function _main' in generated, 'compress is cancelled by -g1'),
        (['-O2', '-g2'], lambda generated: ('var b = 0' in generated or 'var i1 = 0' in generated) and 'function _main' in generated, 'minify is cancelled by -g2'),
        (['-O2', '-g3'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'registerize is cancelled by -g3'),
        #(['-O2', '-g4'], lambda generated: 'var b=0' not in generated and 'var b = 0' not in generated and 'function _main' in generated, 'same as -g3 for now'),
        (['-s', 'INLINING_LIMIT=0'], lambda generated: 'function _dump' in generated, 'no inlining without opts'),
        (['-O3', '-s', 'INLINING_LIMIT=0', '--closure', '0'], lambda generated: 'function _dump' not in generated, 'lto/inlining'),
        (['-Os', '--llvm-lto', '1', '-s', 'ASM_JS=0'], lambda generated: 'function _dump' in generated, '-Os disables inlining'),
        (['-s', 'USE_TYPED_ARRAYS=0'], lambda generated: 'new Int32Array' not in generated, 'disable typed arrays'),
        (['-s', 'USE_TYPED_ARRAYS=1'], lambda generated: 'IHEAPU = ' in generated, 'typed arrays 1 selected'),
        ([], lambda generated: 'Module["_dump"]' not in generated, 'dump is not exported by default'),
        (['-s', 'EXPORTED_FUNCTIONS=["_main", "_dump"]'], lambda generated: 'Module["_dump"]' in generated, 'dump is now exported'),
        (['--typed-arrays', '0'], lambda generated: 'new Int32Array' not in generated, 'disable typed arrays'),
        (['--typed-arrays', '1'], lambda generated: 'IHEAPU = ' in generated, 'typed arrays 1 selected'),
        (['--typed-arrays', '2'], lambda generated: 'new Uint16Array' in generated and 'new Uint32Array' in generated, 'typed arrays 2 selected'),
        (['--llvm-opts', '1'], lambda generated: '_puts(' in generated, 'llvm opts requested'),
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
        self.assertContained('missing function', run_js(target, stderr=STDOUT))
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

    # TODO: Add in files test a clear example of using disablePermissions, and link to it from the wiki
    # TODO: test normal project linking, static and dynamic: get_library should not need to be told what to link!
    # TODO: deprecate llvm optimizations, dlmalloc, etc. in emscripten.py.

  def test_cmake(self):
    # Test all supported generators.
    if WINDOWS:
      generators = ['MinGW Makefiles', 'NMake Makefiles']
    else:
      generators = ['Unix Makefiles']

    make_commands = { 'MinGW Makefiles': ['mingw32-make'], 'NMake Makefiles': ['nmake', '/NOLOGO'], 'Unix Makefiles': ['make'] }

    if os.name == 'nt':
      emconfigure = path_from_root('emconfigure.bat')
    else:
      emconfigure = path_from_root('emconfigure')

    for generator in generators:
      if generator == 'NMake Makefiles' and not Building.which('nmake'):
        print >> sys.stderr, 'Skipping NMake test for CMake support, since nmake was not found in PATH. Run this test in Visual Studio command prompt to easily access nmake.'
        continue

      make = make_commands[generator]
      cmake_cases = ['target_js', 'target_html']
      cmake_outputs = ['test_cmake.js', 'hello_world_gles.html']
      for i in range(0, 2):
        for configuration in ['Debug', 'Release']:
          # CMake can be invoked in two ways, using 'emconfigure cmake', or by directly running 'cmake'.
          # Test both methods.
          for invoke_method in ['cmake', 'emconfigure']:

            # Create a temp workspace folder
            cmakelistsdir = path_from_root('tests', 'cmake', cmake_cases[i])
            tempdirname = tempfile.mkdtemp(prefix='emscripten_test_' + self.__class__.__name__ + '_', dir=TEMP_DIR)
            try:
              os.chdir(tempdirname)

              verbose_level = int(os.getenv('EM_BUILD_VERBOSE')) if os.getenv('EM_BUILD_VERBOSE') != None else 0
              
              # Run Cmake
              if invoke_method == 'cmake':
                # Test invoking cmake directly.
                cmd = ['cmake', '-DCMAKE_TOOLCHAIN_FILE='+path_from_root('cmake', 'Platform', 'Emscripten.cmake'),
                                '-DCMAKE_BUILD_TYPE=' + configuration, '-G', generator, cmakelistsdir]
              else:
                # Test invoking via 'emconfigure cmake'
                cmd = [emconfigure, 'cmake', '-DCMAKE_BUILD_TYPE=' + configuration, '-G', generator, cmakelistsdir]

              ret = Popen(cmd, stdout=None if verbose_level >= 2 else PIPE, stderr=None if verbose_level >= 1 else PIPE).communicate()
              if len(ret) > 1 and ret[1] != None and len(ret[1].strip()) > 0:
                logging.error(ret[1]) # If there were any errors, print them directly to console for diagnostics.
              if len(ret) > 1 and ret[1] != None and 'error' in ret[1].lower():
                logging.error('Failed command: ' + ' '.join(cmd))
                logging.error('Result:\n' + ret[1])
                raise Exception('cmake call failed!')
              assert os.path.exists(tempdirname + '/Makefile'), 'CMake call did not produce a Makefile!'

              # Build
              cmd = make + (['VERBOSE=1'] if verbose_level >= 3 else [])
              ret = Popen(cmd, stdout=None if verbose_level >= 2 else PIPE).communicate()
              if len(ret) > 1 and ret[1] != None and len(ret[1].strip()) > 0:
                logging.error(ret[1]) # If there were any errors, print them directly to console for diagnostics.
              if len(ret) > 0 and ret[0] != None and 'error' in ret[0].lower() and not '0 error(s)' in ret[0].lower():
                logging.error('Failed command: ' + ' '.join(cmd))
                logging.error('Result:\n' + ret[0])
                raise Exception('make failed!')
              assert os.path.exists(tempdirname + '/' + cmake_outputs[i]), 'Building a cmake-generated Makefile failed to produce an output file %s!' % tempdirname + '/' + cmake_outputs[i]

              # Run through node, if CMake produced a .js file.
              if cmake_outputs[i].endswith('.js'):
                ret = Popen(listify(NODE_JS) + [tempdirname + '/' + cmake_outputs[i]], stdout=PIPE).communicate()[0]
                self.assertTextDataIdentical(open(cmakelistsdir + '/out.txt', 'r').read().strip(), ret.strip())
            finally:
              os.chdir(path_from_root('tests')) # Move away from the directory we are about to remove.
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
    for compiler in [EMCC, EMXX]:
      process = Popen([PYTHON, compiler, '-std=c++11', path_from_root('tests', 'hello_cxx11.cpp')], stdout=PIPE, stderr=PIPE)
      process.communicate()
      assert process.returncode is 0, 'User should be able to specify custom -std= on the command line!'

  def test_cap_suffixes(self):
    shutil.copyfile(path_from_root('tests', 'hello_world.cpp'), 'test.CPP')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'test.CPP')]).communicate()
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

  def test_unaligned_memory(self):
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

  def test_unaligned_memory_2(self):
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
    def test(args, expected, err_expected=None):
      out, err = Popen([PYTHON, EMCC, 'src.c'] + args, stderr=PIPE).communicate()
      if err_expected: self.assertContained(err_expected, err)
      self.assertContained(expected, run_js(self.in_dir('a.out.js'), stderr=PIPE, full_output=True))
      return open(self.in_dir('a.out.js')).read()

    test([], 'my func') # no asm, so casting func works
    test(['-O2'], 'abort', ['Casting potentially incompatible function pointer i32 ()* to void (...)*, for my_func',
                            'Incompatible function pointer casts are very dangerous with ASM_JS=1, you should investigate and correct these']) # asm, so failure
    test(['-O2', '-s', 'ASSERTIONS=1'],
         'Invalid function pointer called. Perhaps a miscast function pointer (check compilation warnings) or bad vtable lookup (maybe due to derefing a bad pointer, like NULL)?',
         ['Casting potentially incompatible function pointer i32 ()* to void (...)*, for my_func',
         'Incompatible function pointer casts are very dangerous with ASM_JS=1, you should investigate and correct these']) # asm, so failure

  def test_l_link(self):
    # Linking with -lLIBNAME and -L/DIRNAME should work

    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      extern void printey();
      int main() {
        printey();
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
        printf("hello from lib\\n");
      }
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), '-c']).communicate()
    shutil.move(os.path.join(self.get_dir(), 'libfile.o'), os.path.join(self.get_dir(), 'libdir', 'libfile.so'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile']).communicate()
    self.assertContained('hello from lib', run_js(os.path.join(self.get_dir(), 'a.out.js')))
    assert not os.path.exists('a.out') and not os.path.exists('a.exe'), 'Must not leave unneeded linker stubs'

  def test_static_link(self):
    def test(name, header, main, side, expected, args=[], suffix='cpp', first=True):
      print name
      #t = main ; main = side ; side = t
      original_main = main
      original_side = side
      if header: open(os.path.join(self.get_dir(), 'header.h'), 'w').write(header)
      if type(main) == str:
        open(os.path.join(self.get_dir(), 'main.' + suffix), 'w').write(main)
        main = ['main.' + suffix]
      if type(side) == str:
        open(os.path.join(self.get_dir(), 'side.' + suffix), 'w').write(side)
        side = ['side.' + suffix]
      Popen([PYTHON, EMCC] + side + ['-o', 'side.js', '-s', 'SIDE_MODULE=1', '-O2'] + args).communicate()
      # TODO: test with and without DISABLE_GL_EMULATION, check that file sizes change
      Popen([PYTHON, EMCC] + main + ['-o', 'main.js', '-s', 'MAIN_MODULE=1', '-O2', '-s', 'DISABLE_GL_EMULATION=1'] + args).communicate()
      Popen([PYTHON, EMLINK, 'main.js', 'side.js', 'together.js'], stdout=PIPE).communicate()
      assert os.path.exists('together.js')
      for engine in JS_ENGINES:
        out = run_js('together.js', engine=engine, stderr=PIPE, full_output=True)
        self.assertContained(expected, out)
        if engine == SPIDERMONKEY_ENGINE: self.validate_asmjs(out)
      if first:
        shutil.copyfile('together.js', 'first.js')
        test(name + ' (reverse)', header, original_side, original_main, expected, args, suffix, False) # test reverse order

    # test a simple call from one module to another. only one has a string (and constant memory initialization for it)
    test('basics', '', '''
      #include <stdio.h>
      extern int sidey();
      int main() {
        printf("other says %d.", sidey());
        return 0;
      }
    ''', '''
      int sidey() { return 11; }
    ''', 'other says 11.')

    # finalization of float variables should pass asm.js validation
    test('floats', '', '''
      #include <stdio.h>
      extern float sidey();
      int main() {
        printf("other says %.2f.", sidey()+1);
        return 0;
      }
    ''', '''
      float sidey() { return 11.5; }
    ''', 'other says 12.50')

    # memory initialization in both
    test('multiple memory inits', '', r'''
      #include <stdio.h>
      extern void sidey();
      int main() {
        printf("hello from main\n");
        sidey();
        return 0;
      }
    ''', r'''
      #include <stdio.h>
      void sidey() { printf("hello from side\n"); }
    ''', 'hello from main\nhello from side\n')

    # function pointers
    test('fp1', 'typedef void (*voidfunc)();', r'''
      #include <stdio.h>
      #include "header.h"
      voidfunc sidey(voidfunc f);
      void a() { printf("hello from funcptr\n"); }
      int main() {
        sidey(a)();
        return 0;
      }
    ''', '''
      #include "header.h"
      voidfunc sidey(voidfunc f) { return f; }
    ''', 'hello from funcptr\n')

    # function pointers with 'return' in the name
    test('fp2', 'typedef void (*voidfunc)();', r'''
      #include <stdio.h>
      #include "header.h"
      int sidey(voidfunc f);
      void areturn0() { printf("hello 0\n"); }
      void areturn1() { printf("hello 1\n"); }
      void areturn2() { printf("hello 2\n"); }
      int main(int argc, char **argv) {
        voidfunc table[3] = { areturn0, areturn1, areturn2 };
        table[sidey(NULL)]();
        return 0;
      }
    ''', '''
      #include "header.h"
      int sidey(voidfunc f) { if (f) f(); return 1; }
    ''', 'hello 1\n')

    # Global initializer
    test('global init', '', r'''
      #include <stdio.h>
      struct Class {
        Class() { printf("a new Class\n"); }
      };
      static Class c;
      int main() {
        return 0;
      }
    ''', r'''
      void nothing() {}
    ''', 'a new Class\n')

    # Multiple global initializers (LLVM generates overlapping names for them)
    test('global inits', r'''
      #include <stdio.h>
      struct Class {
        Class(const char *name) { printf("new %s\n", name); }
      };
    ''', r'''
      #include "header.h"
      static Class c("main");
      int main() {
        return 0;
      }
    ''', r'''
      #include "header.h"
      static Class c("side");
    ''', ['new main\nnew side\n', 'new side\nnew main\n'])

    # Class code used across modules
    test('codecall', r'''
      #include <stdio.h>
      struct Class {
        Class(const char *name);
      };
    ''', r'''
      #include "header.h"
      int main() {
        Class c("main");
        return 0;
      }
    ''', r'''
      #include "header.h"
      Class::Class(const char *name) { printf("new %s\n", name); }
    ''', ['new main\n'])

    # malloc usage in both modules
    test('malloc', r'''
      #include <stdlib.h>
      #include <string.h>
      char *side(const char *data);
    ''', r'''
      #include <stdio.h>
      #include "header.h"
      int main() {
        char *temp = side("hello through side\n");
        char *ret = (char*)malloc(strlen(temp)+1);
        strcpy(ret, temp);
        temp[1] = 'x';
        puts(ret);
        return 0;
      }
    ''', r'''
      #include "header.h"
      char *side(const char *data) {
        char *ret = (char*)malloc(strlen(data)+1);
        strcpy(ret, data);
        return ret;
      }
    ''', ['hello through side\n'])

    # js library call
    open('lib.js', 'w').write(r'''
      mergeInto(LibraryManager.library, {
        test_lib_func: function(x) {
          return x + 17.2;
        }
      });
    ''')
    test('js-lib', 'extern "C" { extern double test_lib_func(int input); }', r'''
      #include <stdio.h>
      #include "header.h"
      extern double sidey();
      int main2() { return 11; }
      int main() {
        int input = sidey();
        double temp = test_lib_func(input);
        printf("other says %.2f\n", temp);
        printf("more: %.5f, %d\n", temp, input);
        return 0;
      }
    ''', r'''
      #include <stdio.h>
      #include "header.h"
      extern int main2();
      double sidey() {
        int temp = main2();
        printf("main2 sed: %d\n", temp);
        printf("main2 sed: %u, %c\n", temp, temp/2);
        return test_lib_func(temp);
      }
    ''', 'other says 45.2', ['--js-library', 'lib.js'])

    # libc usage in one modules. must force libc inclusion in the main module if that isn't the one using mallinfo()
    try:
      os.environ['EMCC_FORCE_STDLIBS'] = 'libc'
      test('malloc-1', r'''
        #include <string.h>
        int side();
      ''', r'''
        #include <stdio.h>
        #include "header.h"
        int main() {
          printf("|%d|\n", side());
          return 0;
        }
      ''', r'''
        #include <stdlib.h>
        #include <malloc.h>
        #include "header.h"
        int side() {
          struct mallinfo m = mallinfo();
          return m.arena > 1;
        }
      ''', ['|1|\n'])
    finally:
      del os.environ['EMCC_FORCE_STDLIBS']

    # iostream usage in one and std::string in both
    test('iostream', r'''
      #include <iostream>
      #include <string>
      std::string side();
    ''', r'''
      #include "header.h"
      int main() {
        std::cout << "hello from main " << side() << std::endl;
        return 0;
      }
    ''', r'''
      #include "header.h"
      std::string side() { return "and hello from side"; }
    ''', ['hello from main and hello from side\n'])

    # followup to iostream test: a second linking
    print 'second linking of a linking output'
    open('moar.cpp', 'w').write(r'''
      #include <iostream>
      struct Moar {
        Moar() { std::cout << "moar!" << std::endl; }
      };
      Moar m;
    ''')
    Popen([PYTHON, EMCC, 'moar.cpp', '-o', 'moar.js', '-s', 'SIDE_MODULE=1', '-O2']).communicate()
    Popen([PYTHON, EMLINK, 'together.js', 'moar.js', 'triple.js'], stdout=PIPE).communicate()
    assert os.path.exists('triple.js')
    for engine in JS_ENGINES:
      out = run_js('triple.js', engine=engine, stderr=PIPE, full_output=True)
      self.assertContained('moar!\nhello from main and hello from side\n', out)
      if engine == SPIDERMONKEY_ENGINE: self.validate_asmjs(out)

    # zlib compression library. tests function pointers in initializers and many other things
    test('zlib', '', open(path_from_root('tests', 'zlib', 'example.c'), 'r').read(), 
                     self.get_zlib_library(),
                     open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                     args=['-I' + path_from_root('tests', 'zlib')], suffix='c')

    use_cmake = WINDOWS
    bullet_library = get_bullet_library(self, use_cmake)

    # bullet physics engine. tests all the things
    test('bullet', '', open(path_from_root('tests', 'bullet', 'Demos', 'HelloWorld', 'HelloWorld.cpp'), 'r').read(), 
         bullet_library,
         [open(path_from_root('tests', 'bullet', 'output.txt'), 'r').read(), # different roundings
          open(path_from_root('tests', 'bullet', 'output2.txt'), 'r').read(),
          open(path_from_root('tests', 'bullet', 'output3.txt'), 'r').read()],
         args=['-I' + path_from_root('tests', 'bullet', 'src')])

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
         100: (190, 275),
         250: (200, 500),
         500: (250, 500),
        1000: (230, 1000),
        2000: (380, 2000),
        5000: (800, 5000),
           0: (1500, 5000)
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
                   self.get_zlib_library(),
                   open(path_from_root('tests', 'zlib', 'ref.txt'), 'r').read(),
                   expected_ranges,
                   args=['-I' + path_from_root('tests', 'zlib')], suffix='c')

  def test_symlink(self):
    if os.name == 'nt':
      return self.skip('Windows FS does not need to be tested for symlinks support, since it does not have them.')
    open(os.path.join(self.get_dir(), 'foobar.xxx'), 'w').write('int main(){ return 0; }')
    os.symlink(os.path.join(self.get_dir(), 'foobar.xxx'), os.path.join(self.get_dir(), 'foobar.c'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foobar.c'), '-o', os.path.join(self.get_dir(), 'foobar')], stdout=PIPE, stderr=PIPE).communicate()
    assert os.path.exists(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar'))
    try_delete(os.path.join(self.get_dir(), 'foobar.xxx'))
    try_delete(os.path.join(self.get_dir(), 'foobar.c'))

    open(os.path.join(self.get_dir(), 'foobar.c'), 'w').write('int main(){ return 0; }')
    os.symlink(os.path.join(self.get_dir(), 'foobar.c'), os.path.join(self.get_dir(), 'foobar.xxx'))
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foobar.xxx'), '-o', os.path.join(self.get_dir(), 'foobar')], stdout=PIPE, stderr=PIPE).communicate()
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
    Building.emcc(main_name, ['-L.', '-lLIB', a_name+'.o', c_name + '.o'], output_filename='a.out.js')

    self.assertContained('result: 62', run_js(os.path.join(self.get_dir(), 'a.out.js')))

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

    Building.emcc(lib_name, ['-s', 'EXPORT_ALL=1', '--post-js', 'main.js'], output_filename='a.out.js')

    self.assertContained('libf1\nlibf2\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

  def test_stdin(self):
    Building.emcc(path_from_root('tests', 'module', 'test_stdin.c'), output_filename='a.out.js')
    open('in.txt', 'w').write('abcdef\nghijkl')

    for engine in JS_ENGINES:
      print >> sys.stderr, engine
      if engine == NODE_JS: continue # FIXME
      if engine == V8_ENGINE: continue # no stdin support in v8 shell
      self.assertContained('abcdef\nghijkl\neof', run_js(os.path.join(self.get_dir(), 'a.out.js'), engine=engine, stdin=open('in.txt')))

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
    self.assertContained('zyx', Popen(listify(JS_ENGINES[0]) + ['a.out.js'], stdout=PIPE, stderr=PIPE).communicate()[0])

  def test_abspaths(self):
    # Includes with absolute paths are generally dangerous, things like -I/usr/.. will get to system local headers, not our portable ones.

    shutil.copyfile(path_from_root('tests', 'hello_world.c'), 'main.c')

    for args, expected in [(['-I/usr/something'], True),
                           (['-L/usr/something'], True),
                           (['-I/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-L/usr/something', '-Wno-warn-absolute-paths'], False),
                           (['-Isubdir/something'], False),
                           (['-Lsubdir/something'], False),
                           ([], False)]:
      err = Popen([PYTHON, EMCC, 'main.c'] + args, stderr=PIPE).communicate()[1]
      assert ('encountered. If this is to a local system header/library, it may cause problems (local system files make sense for compiling natively on your system, but not necessarily to JavaScript)' in err) == expected, err

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

    Popen([PYTHON, EMCC, 'testa.cpp', '-o', 'liba.js', '-s', 'BUILD_AS_SHARED_LIB=2', '-s', 'LINKABLE=1', '-s', 'NAMED_GLOBALS=1', '-I.']).communicate()
    Popen([PYTHON, EMCC, 'testb.cpp', '-o', 'libb.js', '-s', 'BUILD_AS_SHARED_LIB=2', '-s', 'LINKABLE=1', '-s', 'NAMED_GLOBALS=1', '-I.']).communicate()
    Popen([PYTHON, EMCC, 'main.cpp', '-o', 'main.js', '-s', 'RUNTIME_LINKED_LIBS=["liba.js", "libb.js"]', '-s', 'NAMED_GLOBALS=1', '-I.', '-s', 'LINKABLE=1']).communicate()

    Popen([PYTHON, EMCC, 'main.cpp', 'testa.cpp', 'testb.cpp', '-o', 'full.js', '-I.']).communicate()

    self.assertContained('TestA\nTestB\nTestA\n', run_js('main.js', engine=SPIDERMONKEY_ENGINE))

  def test_js_libraries(self):
    open(os.path.join(self.get_dir(), 'main.cpp'), 'w').write('''
      #include <stdio.h>
      extern "C" {
        extern void printey();
        extern int calcey(int x, int y);
      }
      int main() {
        printey();
        printf("*%d*\\n", calcey(10, 22));
        return 0;
      }
    ''')
    open(os.path.join(self.get_dir(), 'mylib1.js'), 'w').write('''
      mergeInto(LibraryManager.library, {
        printey: function() {
          Module.print('hello from lib!');
        }
      });
    ''')
    open(os.path.join(self.get_dir(), 'mylib2.js'), 'w').write('''
      mergeInto(LibraryManager.library, {
        calcey: function(x, y) {
          return x + y;
        }
      });
    ''')

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--js-library', os.path.join(self.get_dir(), 'mylib1.js'),
                                                                     '--js-library', os.path.join(self.get_dir(), 'mylib2.js')]).communicate()
    self.assertContained('hello from lib!\n*32*\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

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

  def test_multidynamic_link(self):
    # Linking the same dynamic library in will error, normally, since we statically link it, causing dupe symbols
    # A workaround is to use --ignore-dynamic-linking, see emcc --help for details

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

    # This lets us link the same dynamic lib twice. We will need to link it in manually at the end.
    compiler = [PYTHON, EMCC, '--ignore-dynamic-linking']

    # Build libfile normally into an .so
    Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libfile.cpp'), '-o', os.path.join(self.get_dir(), 'libdir', 'libfile.so')]).communicate()
    # Build libother and dynamically link it to libfile - but add --ignore-dynamic-linking
    Popen(compiler + [os.path.join(self.get_dir(), 'libdir', 'libother.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-o', os.path.join(self.get_dir(), 'libdir', 'libother.so')]).communicate()
    # Build the main file, linking in both the libs
    Popen(compiler + [os.path.join(self.get_dir(), 'main.cpp'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-lother', '-c']).communicate()

    # The normal build system is over. We need to do an additional step to link in the dynamic libraries, since we ignored them before
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.o'), '-L' + os.path.join(self.get_dir(), 'libdir'), '-lfile', '-lother']).communicate()

    self.assertContained('*hello from lib\n|hello from lib|\n*', run_js(os.path.join(self.get_dir(), 'a.out.js')))

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

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'before.js', '--post-js', 'after.js']).communicate()
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

      extern "C" {
        void something();
        void elsey();
      }

      int main() {
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

    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js']).communicate()
    self.assertContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # never run, so no preRun or postRun
    src = open(os.path.join(self.get_dir(), 'a.out.js')).read().replace('// {{PRE_RUN_ADDITIONS}}', 'addRunDependency()')
    open(os.path.join(self.get_dir(), 'a.out.js'), 'w').write(src)
    self.assertNotContained('pre-run\nhello from main\npost-run\n', run_js(os.path.join(self.get_dir(), 'a.out.js')))

    # noInitialRun prevents run
    for no_initial_run, run_dep in [(0, 0), (1, 0), (0, 1)]:
      print no_initial_run, run_dep
      Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp')]).communicate()
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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js']).communicate()
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
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'main.cpp'), '--pre-js', 'pre.js', '--pre-js', 'pre2.js']).communicate()
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
      (path_from_root('tools', 'test-js-optimizer.js'), open(path_from_root('tools', 'test-js-optimizer-output.js')).read(),
       ['hoistMultiples', 'loopOptimizer', 'removeAssignsToUndefined', 'simplifyExpressions']),
      (path_from_root('tools', 'test-js-optimizer-t2c.js'), open(path_from_root('tools', 'test-js-optimizer-t2c-output.js')).read(),
       ['simplifyExpressions', 'optimizeShiftsConservative']),
      (path_from_root('tools', 'test-js-optimizer-t2.js'), open(path_from_root('tools', 'test-js-optimizer-t2-output.js')).read(),
       ['simplifyExpressions', 'optimizeShiftsAggressive']),
      # Make sure that optimizeShifts handles functions with shift statements.
      (path_from_root('tools', 'test-js-optimizer-t3.js'), open(path_from_root('tools', 'test-js-optimizer-t3-output.js')).read(),
       ['optimizeShiftsAggressive']),
      (path_from_root('tools', 'test-js-optimizer-regs.js'), open(path_from_root('tools', 'test-js-optimizer-regs-output.js')).read(),
       ['registerize']),
      (path_from_root('tools', 'eliminator', 'eliminator-test.js'), open(path_from_root('tools', 'eliminator', 'eliminator-test-output.js')).read(),
       ['eliminate']),
      (path_from_root('tools', 'eliminator', 'safe-eliminator-test.js'), open(path_from_root('tools', 'eliminator', 'safe-eliminator-test-output.js')).read(),
       ['eliminateMemSafe']),
      (path_from_root('tools', 'eliminator', 'asm-eliminator-test.js'), open(path_from_root('tools', 'eliminator', 'asm-eliminator-test-output.js')).read(),
       ['asm', 'eliminate']),
      (path_from_root('tools', 'test-js-optimizer-asm-regs.js'), open(path_from_root('tools', 'test-js-optimizer-asm-regs-output.js')).read(),
       ['asm', 'registerize']),
      (path_from_root('tools', 'test-js-optimizer-asm-regs-min.js'), open(path_from_root('tools', 'test-js-optimizer-asm-regs-min-output.js')).read(),
       ['asm', 'registerize']),
      (path_from_root('tools', 'test-js-optimizer-asm-pre.js'), open(path_from_root('tools', 'test-js-optimizer-asm-pre-output.js')).read(),
       ['asm', 'simplifyExpressions']),
      (path_from_root('tools', 'test-js-optimizer-asm-last.js'), open(path_from_root('tools', 'test-js-optimizer-asm-last-output.js')).read(),
       ['asm', 'last']),
      (path_from_root('tools', 'test-js-optimizer-asm-relocate.js'), open(path_from_root('tools', 'test-js-optimizer-asm-relocate-output.js')).read(),
       ['asm', 'relocate']),
      (path_from_root('tools', 'test-js-optimizer-asm-outline1.js'), open(path_from_root('tools', 'test-js-optimizer-asm-outline1-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tools', 'test-js-optimizer-asm-outline2.js'), open(path_from_root('tools', 'test-js-optimizer-asm-outline2-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tools', 'test-js-optimizer-asm-outline3.js'), open(path_from_root('tools', 'test-js-optimizer-asm-outline3-output.js')).read(),
       ['asm', 'outline']),
      (path_from_root('tools', 'test-js-optimizer-asm-minlast.js'), open(path_from_root('tools', 'test-js-optimizer-asm-minlast-output.js')).read(),
       ['asm', 'minifyWhitespace', 'last']),
    ]:
      print input
      output = Popen(listify(NODE_JS) + [path_from_root('tools', 'js-optimizer.js'), input] + passes, stdin=PIPE, stdout=PIPE).communicate()[0]
      self.assertIdentical(expected, output.replace('\r\n', '\n').replace('\n\n', '\n'))

  def test_m_mm(self):
    open(os.path.join(self.get_dir(), 'foo.c'), 'w').write('''#include <emscripten.h>''')
    for opt in ['M', 'MM']:
      output, err = Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'foo.c'), '-' + opt], stdout=PIPE, stderr=PIPE).communicate()
      assert 'foo.o: ' in output, '-%s failed to produce the right output: %s' % (opt, output)
      assert 'error' not in err, 'Unexpected stderr: ' + err

  def test_chunking(self):
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    if os.environ.get('EMCC_CORES'): return self.skip('cannot run if cores are altered')
    if multiprocessing.cpu_count() < 2: return self.skip('need multiple cores')
    try:
      os.environ['EMCC_DEBUG'] = '1'
      os.environ['EMCC_CORES'] = '2' # standardize over machines
      for asm, linkable, chunks in [
          (0, 0, 2), (0, 1, 2),
          (1, 0, 2), (1, 1, 2)
        ]:
        print asm, linkable, chunks
        output, err = Popen([PYTHON, EMCC, path_from_root('tests', 'hello_libcxx.cpp'), '-O1', '-s', 'LINKABLE=%d' % linkable, '-s', 'ASM_JS=%d' % asm] + (['-O2'] if asm else []), stdout=PIPE, stderr=PIPE).communicate()
        ok = False
        for c in range(chunks, chunks+2):
          ok = ok or ('phase 2 working on %d chunks' % c in err)
        assert ok, err
    finally:
      del os.environ['EMCC_DEBUG']
      del os.environ['EMCC_CORES']

  def test_debuginfo(self):
    if os.environ.get('EMCC_DEBUG'): return self.skip('cannot run in debug mode')
    try:
      os.environ['EMCC_DEBUG'] = '1'
      # llvm debug info is kept only when we can see it, which is without the js optimize, -O0. js debug info is lost by registerize in -O2, so - g disables it
      for args, expect_llvm, expect_js in [
          (['-O0'], True, True),
          (['-O0', '-g'], True, True),
          (['-O1'], False, True),
          (['-O1', '-g'], False, True),
          (['-O2'], False, False),
          (['-O2', '-g'], False, True),
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
    output = run_js('scons_integration.js')
    assert 'If you see this - the world is all right!' in output

  def test_embind(self):
    for args, fail in [
      ([], True), # without --bind, we fail
      (['--bind'], False),
      (['--bind', '-O1'], False),
      (['--bind', '-O2'], False),
      (['--bind', '-O1', '-s', 'ASM_JS=0'], False),
      (['--bind', '-O2', '-s', 'ASM_JS=0'], False)
    ]:
      print args, fail
      self.clear()
      try_delete(self.in_dir('a.out.js'))
      Popen([PYTHON, EMCC, path_from_root('tests', 'embind', 'embind_test.cpp'), '--post-js', path_from_root('tests', 'embind', 'underscore-1.4.2.js'), '--post-js', path_from_root('tests', 'embind', 'imvu_test_adapter.js'), '--post-js', path_from_root('tests', 'embind', 'embind.test.js')] + args, stderr=PIPE if fail else None).communicate()
      assert os.path.exists(self.in_dir('a.out.js')) == (not fail)
      if not fail:
        output = run_js(self.in_dir('a.out.js'), stdout=PIPE, stderr=PIPE, full_output=True)
        assert "FAIL" not in output, output

  def test_llvm_nativizer(self):
    try:
      Popen(['as', '--version'], stdout=PIPE, stderr=PIPE).communicate()
    except:
      return self.skip('no gnu as, cannot run nativizer')

    # avoid impure_ptr problems etc.
    shutil.copyfile(path_from_root('tests', 'files.cpp'), os.path.join(self.get_dir(), 'files.cpp'))
    open(os.path.join(self.get_dir(), 'somefile.binary'), 'w').write('''waka waka############################''')
    open(os.path.join(self.get_dir(), 'test.file'), 'w').write('''ay file..............,,,,,,,,,,,,,,''')
    open(os.path.join(self.get_dir(), 'stdin'), 'w').write('''inter-active''')
    Popen([PYTHON, EMCC, os.path.join(self.get_dir(), 'files.cpp'), '-c']).communicate()
    Popen([PYTHON, path_from_root('tools', 'nativize_llvm.py'), os.path.join(self.get_dir(), 'files.o')], stdout=PIPE, stderr=PIPE).communicate(input)
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
    self.assertContained('treating -s as linker option', output[1])
    output = run_js('a.out.js')
    assert 'yello' in output, 'code works'
    code = open('a.out.js').read()
    assert 'SAFE_HEAP' in code, 'valid -s option had an effect'

  def test_jcache_printf(self):
    open(self.in_dir('src.cpp'), 'w').write(r'''
      #include <stdio.h>
      #include <stdint.h>
      #include <emscripten.h>
      int main() {
        emscripten_jcache_printf("hello world\n");
        emscripten_jcache_printf("hello %d world\n", 5);
        emscripten_jcache_printf("hello %.3f world\n", 123.456789123);
        emscripten_jcache_printf("hello %llx world\n", 0x1234567811223344ULL);
        return 0;
      }
    ''')
    Popen([PYTHON, EMCC, self.in_dir('src.cpp')]).communicate()
    output = run_js('a.out.js')
    self.assertIdentical('hello world\nhello 5 world\nhello 123.457 world\nhello 1234567811223300 world\n', output)

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
      return filter(lambda line: 'PACKAGE_UUID' not in line, txt.split('\n'))
    out = clean(out)
    out2 = clean(out2)
    assert out == out2
    # sanity check that we do generate different code for different inputs
    out3, err3 = Popen([PYTHON, FILE_PACKAGER, 'test.data', '--preload', 'data2.txt', 'data2.txt@waka.txt'], stdout=PIPE, stderr=PIPE).communicate()
    out3 = clean(out3)
    assert out != out3

  def test_crunch(self):
    # crunch should not be run if a .crn exists that is more recent than the .dds
    shutil.copyfile(path_from_root('tests', 'ship.dds'), 'ship.dds')
    time.sleep(0.1)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
    assert os.stat('test.data').st_size < 0.25*os.stat('ship.dds').st_size, 'Compressed should be much smaller than dds'
    crunch_time = os.stat('ship.crn').st_mtime
    dds_time = os.stat('ship.dds').st_mtime
    assert crunch_time > dds_time, 'Crunch is more recent'
    # run again, should not recrunch!
    time.sleep(0.1)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
    assert crunch_time == os.stat('ship.crn').st_mtime, 'Crunch is unchanged'
    # update dds, so should recrunch
    time.sleep(0.1)
    os.utime('ship.dds', None)
    Popen([PYTHON, FILE_PACKAGER, 'test.data', '--pre-run', '--crunch=32', '--preload', 'ship.dds'], stdout=open('pre.js', 'w')).communicate()
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
        one(17);
        return 0;
      }
    ''')

    Popen([PYTHON, EMCC, 'src.cpp', '-s', 'LINKABLE=1']).communicate()
    output = run_js('a.out.js')
    self.assertContained('''operator new()
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
__cxxabiv1::__si_class_type_info::search_below_dst(__cxxabiv1::__dynamic_cast_info*, void*, int, bool)
parseword(char*&, int, int&)
multi(wchar_t, signed char, unsigned char, unsigned short, unsigned int, unsigned long, long long, unsigned long long, ...)
a(int [32], char [5]*)
FWakaGLXFleeflsMarfoo::FWakaGLXFleeflsMarfoo(unsigned int, unsigned int, unsigned int, void*, bool, unsigned int, unsigned int)
''', output)
    # test for multiple functions in one stack trace
    assert 'one(int)' in output
    assert 'two(char)' in output

  def test_module_exports_with_closure(self):
    # This test checks that module.export is retained when JavaScript is minified by compiling with --closure 1
    # This is important as if module.export is not present the Module object will not be visible to node.js
    # Run with ./runner.py other.test_module_exports_with_closure

    # First make sure test.js isn't present.
    try_delete(path_from_root('tests', 'Module-exports', 'test.js'))
    assert not os.path.exists(path_from_root('tests', 'Module-exports', 'test.js'))

    # compile with -O2 --closure 0
    Popen([PYTHON, EMCC, path_from_root('tests', 'Module-exports', 'test.c'), '-o', path_from_root('tests', 'Module-exports', 'test.js'), '-O2', '--closure', '0', '--pre-js', path_from_root('tests', 'Module-exports', 'setup.js'), '-s', 'EXPORTED_FUNCTIONS=["_bufferTest"]'], stdout=PIPE, stderr=PIPE).communicate()

    # Check that compilation was successful
    assert os.path.exists(path_from_root('tests', 'Module-exports', 'test.js'))
    test_js_closure_0 = open(path_from_root('tests', 'Module-exports', 'test.js')).read()

    # Check that test.js compiled with --closure 0 contains "module['exports'] = Module;"
    assert "module['exports'] = Module;" in test_js_closure_0

    # Check that main.js (which requires test.js) completes successfully when run in node.js
    # in order to check that the exports are indeed functioning correctly.
    if NODE_JS in JS_ENGINES:
      self.assertContained('bufferTest finished', run_js(path_from_root('tests', 'Module-exports', 'main.js'), engine=NODE_JS))

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
      self.assertContained('bufferTest finished', run_js(path_from_root('tests', 'Module-exports', 'main.js'), engine=NODE_JS))

    # Tidy up files that might have been created by this test.
    try_delete(path_from_root('tests', 'Module-exports', 'test.js'))
    try_delete(path_from_root('tests', 'Module-exports', 'test.js.map'))

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
    int file_size = 0;
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
        printf("File size: %d\n", file_size);
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
      self.assertContained('File size: 722', out)

  def test_simd(self):
    self.clear()
    Popen([PYTHON, EMCC, path_from_root('tests', 'linpack.c'), '-O2', '-DSP', '--llvm-opts', '''['-O3', '-vectorize', '-vectorize-loops', '-bb-vectorize-vector-bits=128', '-force-vector-width=4']''']).communicate()
    self.assertContained('Unrolled Single  Precision', run_js('a.out.js'))

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

