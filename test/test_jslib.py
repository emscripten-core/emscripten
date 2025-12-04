# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import shutil
from subprocess import PIPE

from common import RunnerCore, create_file, read_file, test_file
from decorators import also_with_wasm64, also_without_bigint, parameterized

from tools.shared import EMCC
from tools.utils import delete_file


class jslib(RunnerCore):
  def test_jslib_no_override(self):
    create_file('duplicated_func.c', '''
      #include <stdio.h>
      extern int duplicatedFunc();

      int main() {
        int res = duplicatedFunc();
        printf("*%d*\\n", res);
        return 0;
      }
    ''')
    create_file('duplicated_func_1.js', '''
      addToLibrary(
        { duplicatedFunc: () => 1 },
        { noOverride: true }
      );
    ''')
    create_file('duplicated_func_2.js', '''
      addToLibrary(
        { duplicatedFunc: () => 2 },
        { noOverride: true }
      );
    ''')

    self.cflags += ['--js-library', 'duplicated_func_1.js', '--js-library', 'duplicated_func_2.js']
    err = self.expect_fail([EMCC, 'duplicated_func.c'] + self.get_cflags())
    self.assertContained('duplicated_func_2.js: Symbol re-definition in JavaScript library: duplicatedFunc. Do not use noOverride if this is intended', err)

  def test_jslib_missing_sig(self):
    create_file('some_func.c', '''
      #include <stdio.h>
      extern int someFunc();

      int main() {
        int res = someFunc();
        printf("*%d*\\n", res);
        return 0;
      }
    ''')
    create_file('some_func.js', '''
      addToLibrary(
        { someFunc: () => 1 },
        { checkSig: true }
      );
    ''')

    self.cflags += ['--js-library', 'some_func.js']
    err = self.expect_fail([EMCC, 'some_func.c'] + self.get_cflags())
    self.assertContained('some_func.js: __sig is missing for function: someFunc. Do not use checkSig if this is intended', err)

  def test_jslib_extra_args(self):
    # Verify that extra arguments in addition to those listed in `__sig` are still present
    # in the generated JS library function.
    # See https://github.com/emscripten-core/emscripten/issues/21056
    create_file('some_func.js', '''
      addToLibrary({
        someFunc: (arg1, arg2) => {
          err('arg1:' + arg1);
          err('arg2:' + arg2);
        },
        someFunc__sig: 'pp',
      });
    ''')
    create_file('test.c', '''
    void someFunc(long p);
    int main() {
      someFunc(42);
    }
    ''')
    self.cflags += ['--js-library', 'some_func.js', '-sALLOW_MEMORY_GROWTH', '-sMAXIMUM_MEMORY=4Gb']
    self.do_runf('test.c', 'arg1:42\narg2:undefined\n')

  def test_jslib_quoted_key(self):
    create_file('lib.js', r'''
addToLibrary({
  __internal_data:{
    '<' : 0,
    'white space' : 1
  },
  foo__deps: ['__internal_data'],
  foo: () => {
    return 0;
  }
});
''')

    self.do_run_in_out_file_test('hello_world.c', cflags=['--js-library', 'lib.js'])

  def test_jslib_proxying(self):
    # Regression test for a bug we had where jsifier would find and use
    # the inner function in a library function consisting of a single
    # line arrow function.
    # See https://github.com/emscripten-core/emscripten/issues/20264
    create_file('lib.js', r'''
addToLibrary({
  $doNotCall: (x) => {},
  foo__deps: ['$doNotCall'],
  foo__proxy: 'sync',
  foo: () => doNotCall(() => {
    out('should never see this');
  }),
});
''')
    create_file('src.c', r'''
    #include <stdio.h>
    void foo();
    int main() {
      printf("main\n");
      foo();
      printf("done\n");
    }
    ''')
    self.do_runf('src.c', 'main\ndone\n', cflags=['-sEXIT_RUNTIME', '-pthread', '-sPROXY_TO_PTHREAD', '--js-library', 'lib.js'])

  def test_jslib_method_syntax(self):
    create_file('lib.js', r'''
addToLibrary({
  foo() {
    out('foo');
  },
});
''')
    create_file('src.c', r'''
    #include <stdio.h>
    void foo();
    int main() {
      foo();
    }
    ''')
    self.do_runf('src.c', 'foo', cflags=['--js-library', 'lib.js'])

  def test_jslib_exported(self):
    create_file('lib.js', r'''
addToLibrary({
 jslibfunc: (x) => 2 * x
});
''')
    create_file('src.c', r'''
#include <emscripten.h>
#include <stdio.h>
int jslibfunc(int x);
int main() {
  printf("c calling: %d\n", jslibfunc(6));
  EM_ASM({
    out('js calling: ' + Module['_jslibfunc'](5) + '.');
  });
}
''')
    self.do_runf('src.c', 'c calling: 12\njs calling: 10.',
                 cflags=['--js-library', 'lib.js', '-sEXPORTED_FUNCTIONS=_main,_jslibfunc'])

  def test_jslib_using_asm_lib(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: ['asmlibfunc'],
  jslibfunc: (x) => 2 * _asmlibfunc(x),

  asmlibfunc__asm: true,
  asmlibfunc__sig: 'ii',
  asmlibfunc: (x) => {
    x = x | 0;
    return x + 1 | 0;
  }
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc(int x);
int main() {
  printf("c calling: %d\n", jslibfunc(6));
}
''')
    self.do_runf('src.c', 'c calling: 14\n', cflags=['--js-library', 'lib.js'])

  def test_jslib_preprocessor_errors(self):
    create_file('lib.js', '''\
// This is a library file
#endif // line 2
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js:2: #endif without matching #if', err)

    create_file('lib.js', '''\
// This is a library file

#else // line 3
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js:3: #else without matching #if', err)

  def test_jslib_internal_deps(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: ['$callRuntimeCallbacks'],
  jslibfunc: (x) => {
    callRuntimeCallbacks();
  },
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("c calling: %d\n", jslibfunc());
}
''')
    err = self.run_process([EMCC, 'src.c', '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained("warning: user library symbol 'jslibfunc' depends on internal symbol '$callRuntimeCallbacks'", err)

  def test_jslib_sig_redefinition(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => {},
});

addToLibrary({
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => {},
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("c calling: %d\n", jslibfunc());
}
''')
    err = self.run_process([EMCC, 'src.c', '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained('lib.js: signature redefinition for: jslibfunc__sig', err)

    # Add another redefinition, this time not matching
    create_file('lib2.js', r'''
addToLibrary({
  jslibfunc__sig: 'pp',
  jslibfunc: (x) => {},
});
''')
    err = self.expect_fail([EMCC, 'src.c', '--js-library', 'lib.js', '--js-library', 'lib2.js'])
    self.assertContained('lib2.js: signature redefinition for: jslibfunc__sig. (old=ii vs new=pp)', err)

  def test_jslib_invalid_deps(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__deps: 'hello',
  jslibfunc: (x) => {},
});
''')

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained('lib.js: JS library directive jslibfunc__deps=hello is of type \'string\', but it should be an array', err)

    create_file('lib2.js', r'''
addToLibrary({
  jslibfunc__deps: [1,2,3],
  jslibfunc: (x) => {},
});
''')

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib2.js'])
    self.assertContained("lib2.js: __deps entries must be of type 'string' or 'function' not 'number': jslibfunc__deps", err)

  def test_jslib_invalid_decorator(self):
    create_file('lib.js', r'''
addToLibrary({
  jslibfunc__async: 'hello',
  jslibfunc: (x) => {},
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'])
    self.assertContained("lib.js: Decorator (jslibfunc__async} has wrong type. Expected 'boolean' not 'string'", err)

  @also_with_wasm64
  @also_without_bigint
  def test_jslib_i53abi(self):
    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc__i53abi: true,
  jslibfunc__sig: 'j',
  jslibfunc: (x) => { return 42 },
});
''')
    create_file('test.c', r'''
#include <stdio.h>
int64_t jslibfunc();
int main() {
  printf("main: %lld\n", jslibfunc());
}
''')
    self.do_runf('test.c', 'main: 42\n', cflags=['--js-library', 'lib.js'])

  def test_jslib_i53abi_errors(self):
    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc__i53abi: true,
  jslibfunc: (x) => { return 42 },
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=jslibfunc', '--js-library', 'lib.js'])
    self.assertContained("error: JS library error: '__i53abi' decorator requires '__sig' decorator: 'jslibfunc'", err)

    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc__i53abi: true,
  jslibfunc__sig: 'ii',
  jslibfunc: (x) => { return 42 },
});
''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=jslibfunc', '--js-library', 'lib.js'])
    self.assertContained("error: JS library error: '__i53abi' only makes sense when '__sig' includes 'j' (int64): 'jslibfunc'", err)

  def test_jslib_legacy(self):
    create_file('lib.js', r'''
mergeInto(LibraryManager.library, {
  jslibfunc: (x) => { return 42 },
});
''')
    create_file('src.c', r'''
#include <stdio.h>
int jslibfunc();
int main() {
  printf("main: %d\n", jslibfunc());
}
''')
    self.do_runf('src.c', 'main: 42\n', cflags=['--js-library', 'lib.js'])

  # Tests that users can pass custom JS options from command line using
  # the -jsDfoo=val syntax:
  # See https://github.com/emscripten-core/emscripten/issues/10580.
  def test_jslib_custom_settings(self):
    self.cflags += ['--js-library', test_file('jslib/test_jslib_custom_settings.js'), '-jsDCUSTOM_JS_OPTION=1']
    self.do_run_in_out_file_test('jslib/test_jslib_custom_settings.c')

    self.assertContained('cannot change built-in settings values with a -jsD directive', self.expect_fail([EMCC, '-jsDWASM=0']))

  def test_jslib_native_deps(self):
    # Verify that memset (which lives in compiled code), can be specified as a JS library
    # dependency.
    create_file('lib.js', r'''
addToLibrary({
  depper__deps: ['memset'],
  depper: (ptr) => {
    _memset(ptr, 'd'.charCodeAt(0), 10);
  },
});
''')
    create_file('test.c', r'''
#include <stdio.h>

void depper(char*);

int main(int argc, char** argv) {
  char buffer[11] = { 0 };
  depper(buffer);
  puts(buffer);
}
''')

    self.do_runf('test.c', 'dddddddddd\n', cflags=['--js-library', 'lib.js'])

  def test_jslib_native_deps_extra(self):
    # Similar to above but the JS symbol is not used by the native code.
    # Instead is it explicitly injected using `extraLibraryFuncs`.
    create_file('lib.js', r'''
addToLibrary({
  jsfunc__deps: ['raise'],
  jsfunc: (ptr) => {
    _raise(1);
  },
});
extraLibraryFuncs.push('jsfunc');
''')
    self.do_runf('hello_world.c', cflags=['--js-library', 'lib.js'])

  def test_jslib_clobber_i(self):
    # Regression check for an issue we have where a library clobbering the global `i` variable could
    # prevent processing of further libraries.
    create_file('lib1.js', 'for (var i = 0; i < 100; i++) {}')
    create_file('lib2.js', '''
      addToLibrary({
        foo: () => {}
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'),
                      '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=foo',
                      '--js-library=lib1.js',
                      '--js-library=lib2.js'])

  def test_jslib_bad_config(self):
    create_file('lib.js', '''
      addToLibrary({
        foo__sig: 'ii',
      });
      ''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js'])
    self.assertContained("lib.js: Missing library element 'foo' for library config 'foo__sig'", err)

  def test_jslib_ifdef(self):
    create_file('lib.js', '''
      #ifdef ASSERTIONS
      var foo;
      #endif
      ''')
    proc = self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js'], stderr=PIPE)
    self.assertContained('lib.js: use of #ifdef in js library.  Use #if instead.', proc.stderr)

  def test_jslib_mangling(self):
    create_file('lib.js', '''
      addToLibrary({
        $__foo: () => 43,
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$__foo'])

  def test_jslib_exported_functions(self):
    create_file('lib.js', '''
      addToLibrary({
        $Foo: () => 43,
      });
      ''')
    create_file('post.js', 'console.log("Foo:", Module.Foo())')
    self.do_runf(test_file('hello_world.c'), cflags=['--post-js=post.js', '--js-library=lib.js', '-sEXPORTED_FUNCTIONS=Foo,_main'])

  def test_jslib_search_path(self):
    create_file('libfoo.js', '''
      addToLibrary({
        foo: () => 42,
      });
      ''')
    create_file('main.c', r'''
      #include <stdio.h>
      int foo();
      int main() {
        printf("%d\n", foo());
        return 0;
      }''')
    self.do_runf('main.c', '42\n', cflags=['-L.', '-lfoo.js'])

    # If the library path is not included with `-L` we expect the command to fail
    err = self.expect_fail([EMCC, 'main.c', '-lfoo.js'])
    self.assertContained('emcc: error: unable to find library -lfoo.js', err)

  # Tests using the #warning directive in JS library files
  def test_jslib_warnings(self):
    shutil.copy(test_file('warning_in_js_libraries.js'), '.')
    proc = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'warning_in_js_libraries.js'], stdout=PIPE, stderr=PIPE)
    self.assertNotContained('This warning should not be present!', proc.stderr)
    self.assertContained('warning: warning_in_js_libraries.js:5: #warning This is a warning string!', proc.stderr)
    self.assertContained('warning: warning_in_js_libraries.js:7: #warning This is a second warning string!', proc.stderr)
    self.assertContained('emcc: warning: warnings in JS library compilation [-Wjs-compiler]', proc.stderr)

    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'warning_in_js_libraries.js', '-Werror'])
    self.assertNotContained('This warning should not be present!', err)
    self.assertContained('warning: warning_in_js_libraries.js:5: #warning This is a warning string!', err)
    self.assertContained('warning: warning_in_js_libraries.js:7: #warning This is a second warning string!', err)
    self.assertContained('emcc: error: warnings in JS library compilation [-Wjs-compiler] [-Werror]', err)

  # Tests using the #error directive in JS library files
  def test_jslib_errors(self):
    shutil.copy(test_file('error_in_js_libraries.js'), '.')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'error_in_js_libraries.js'])
    self.assertNotContained('This error should not be present!', err)
    self.assertContained('error: error_in_js_libraries.js:5: #error This is an error string!', err)
    self.assertContained('error: error_in_js_libraries.js:7: #error This is a second error string!', err)

  def test_jslib_include(self):
    create_file('inc.js', '''
    let MY_VAR = 10;
    ''')
    create_file('foo.js', '''
    // Include a file from system directory
    #include "IDBStore.js"
    // Include a local file.
    #include "inc.js"
    ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'foo.js'])

    delete_file('inc.js')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library', 'foo.js'])
    self.assertContained('foo.js:5: file not found: inc.js', err)

  @also_with_wasm64
  @also_without_bigint
  @parameterized({
    '': ([],),
    'closure': (['--closure=1'],),
  })
  def test_jslib_aliases(self, args):
    create_file('foo.js', '''
      addToLibrary({
        foo: () => 42,
        foo__sig: 'i',

        foo_alias: 'foo',

        foo_alias_i64: 'foo',
        foo_alias_i64__sig: 'j',
        foo_alias_i64__i53abi: true,

        foo_alias_ptr: 'foo',
        foo_alias_ptr__sig: 'p',

        // Normal JS function that calls a native function
        call_native__deps: ['native_func'],
        call_native: () => _native_func(),

        // JS function that calls nativeAlias
        call_native_alias__deps: ['$nativeAlias'],
        call_native_alias: () => nativeAlias(),

        // This is a JS-only symbol (i.e. no leading underscore) that
        // aliases a native symbol.
        $nativeAlias__deps: ['native_func2'],
        $nativeAlias: 'native_func2',
      });
    ''')
    create_file('main.c', r'''
      #include <stdio.h>
      #include <emscripten.h>
      int foo();
      int foo_alias();
      void* foo_alias_ptr();
      int64_t foo_alias_i64();
      int call_native();
      int call_native_alias();

      int native_func() {
        return 43;
      }

      int native_func2() {
        return 44;
      }

      int main() {
        printf("foo: %d\n", foo());
        printf("foo_alias: %d\n", foo_alias());
        printf("foo_alias_i64: %lld\n", foo_alias_i64());
        printf("foo_alias_ptr: %p\n", foo_alias_ptr());
        printf("call_native: %d\n", call_native());
        printf("call_native_alias: %d\n", call_native_alias());
        return 0;
      }
    ''')
    expected = '''\
foo: 42
foo_alias: 42
foo_alias_i64: 42
foo_alias_ptr: 0x2a
call_native: 43
call_native_alias: 44
'''
    self.do_runf('main.c', expected, cflags=['--js-library', 'foo.js'] + args)

  @parameterized({
    '': ([],),
    'closure': (['--closure=1'],),
  })
  def test_jslib_export_alias(self, args):
    create_file('lib.js', '''
      addToLibrary({
        $foo: 'main',
        $bar: '__indirect_function_table',
        $baz: 'memory',
      });
    ''')
    create_file('extern_pre.js', r'''
      Module = {
        onRuntimeInitialized: () => {
          const assert = require('assert');
          console.log("onRuntimeInitialized");
          console.log('foo:', typeof Module.foo)
          console.log('bar:', typeof Module.bar)
          console.log('baz:', typeof Module.baz)
          assert(Module.foo instanceof Function);
          assert(Module.bar instanceof WebAssembly.Table);
          assert(Module.baz instanceof WebAssembly.Memory);
          console.log('done');
        }
      };
    ''')
    self.do_runf('hello_world.c', 'done\n', cflags=['--js-library=lib.js', '--extern-pre-js=extern_pre.js', '-sEXPORTED_RUNTIME_METHODS=foo,bar,baz'] + args)

  def test_postjs_errors(self):
    create_file('post.js', '#preprocess\n#error This is an error')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--post-js', 'post.js'])
    self.assertContained('post.js:2: #error This is an error', err)

  def test_jslib_has_library(self):
    create_file('libfoo.js', '''
    // libwasi.js should be included.
    #if !LibraryManager.has('libwasi.js')
    #error "oops 1"
    #endif

    // Checking for the legacy name should also work
    #if !LibraryManager.has('library_wasi.js')
    #error "oops 2"
    #endif

    #if LibraryManager.has('libbar.js')
    #error "oops 3"
    #endif
    ''')
    self.do_runf('hello_world.c', cflags=['-L', '-lfoo.js'])

  def test_jslib_new_objects_basic(self):
    create_file('lib.js', '''
      addToLibrary({
        $obj: {
          a: new Map(),
          b: new Set(),
          c: new WeakMap(),
          d: new WeakSet()
        }
      });
      ''')
    self.run_process([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sEXPORTED_FUNCTIONS=obj,_main'])
    self.assertContained("a:new Map,", read_file('a.out.js'))
    self.assertContained("b:new Set,", read_file('a.out.js'))
    self.assertContained("c:new WeakMap,", read_file('a.out.js'))
    self.assertContained("d:new WeakSet,", read_file('a.out.js'))

  def test_jslib_new_objects_non_empty(self):
    create_file('lib.js', '''
      addToLibrary({
        $obj: {
          bad: new Map([[1,2],[3,4]])
        }
      });
      ''')
    err = self.expect_fail([EMCC, test_file('hello_world.c'), '--js-library=lib.js', '-sEXPORTED_FUNCTIONS=obj,_main'])
    self.assertContained('cannot stringify Map with data', err)

  def test_jslib_system_lib_name(self):
    create_file('libcore.js', r'''
addToLibrary({
 jslibfunc: (x) => 2 * x
});
''')
    create_file('src.c', r'''
#include <emscripten.h>
#include <stdio.h>
int jslibfunc(int x);
int main() {
  printf("jslibfunc: %d\n", jslibfunc(6));
  return 0;
}
''')
    self.do_runf('src.c', 'jslibfunc: 12', cflags=['--js-library', 'libcore.js'])

  def test_jslib_preprocess(self):
    # Use stderr rather than stdout here because stdout is redirected to the output JS file itself.
    create_file('lib.js', '''
#if MAIN_MODULE == 1
console.error('JSLIB: MAIN_MODULE=1');
#elif MAIN_MODULE == 2
console.error('JSLIB: MAIN_MODULE=2');
#elif EXIT_RUNTIME
console.error('JSLIB: EXIT_RUNTIME');
#else
console.error('JSLIB: none of the above');
#endif
''')

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js'], stderr=PIPE).stderr
    self.assertContained('JSLIB: none of the above', err)
    self.assertNotContained('JSLIB: MAIN_MODULE', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sMAIN_MODULE'], stderr=PIPE).stderr
    self.assertContained('JSLIB: MAIN_MODULE=1', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sMAIN_MODULE=2'], stderr=PIPE).stderr
    self.assertContained('JSLIB: MAIN_MODULE=2', err)
    self.assertNotContained('JSLIB: EXIT_RUNTIME', err)

    err = self.run_process([EMCC, test_file('hello_world.c'), '--js-library', 'lib.js', '-sEXIT_RUNTIME'], stderr=PIPE).stderr
    self.assertContained('JSLIB: EXIT_RUNTIME', err)
    self.assertNotContained('JSLIB: MAIN_MODULE', err)

  # Tests that it is possible to hook into/override a symbol defined in a system library.
  def test_jslib_override_system_symbol(self):
    # This test verifies it is possible to override a symbol from WebGL library.

    # When WebGL is implicitly linked in, the implicit linking should happen before any user
    # --js-libraries, so that they can adjust the behavior afterwards.
    self.do_run_in_out_file_test('test_jslib_override_system_symbol.c', cflags=['--js-library', test_file('test_jslib_override_system_symbol.js'), '-sMAX_WEBGL_VERSION=2'])

    # When WebGL is explicitly linked to in strict mode, the linking order on command line should enable overriding.
    self.cflags += ['-sAUTO_JS_LIBRARIES=0', '-sMAX_WEBGL_VERSION=2', '-lwebgl.js', '--js-library', test_file('test_jslib_override_system_symbol.js')]
    self.do_run_in_out_file_test('test_jslib_override_system_symbol.c')

  def test_jslib_version_check(self):
    create_file('libfoo.js', '''
      #if parseInt(EMSCRIPTEN_VERSION.split('.')[0]) > 3
      #error "library does not support emscripten > 3.0.0"
      #endif
    ''')
    self.assert_fail([EMCC, '--js-library=libfoo.js'], 'error: libfoo.js:3: #error "library does not support emscripten > 3.0.0"')
