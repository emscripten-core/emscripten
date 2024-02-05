// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>
#include <cstring>

#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

int failed = 0; // If 1, this test failed.

// A custom assert macro to test varargs routing to emscripten_log().
#define MYASSERT(condition, msg, ...) \
  do { \
    if (!(condition)) { \
      emscripten_log(EM_LOG_ERROR, "%s", "Condition '" #condition "' failed in file " __FILE__ ":" STRINGIZE(__LINE__) "!"); \
      emscripten_log(EM_LOG_ERROR, msg, ##__VA_ARGS__); \
      failed = 1; \
    } \
  } while(0)

void __attribute__((noinline)) kitten() {
  // Log to Emscripten Module.
  emscripten_log(EM_LOG_NO_PATHS, "Print a log message: int: %d, string: %s.", 42, "hello");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_WARN, "Print a warning message");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_ERROR, "This is an error!");

  // Log directly to Browser web inspector/console.
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE, "Info log to console: int: %d, string: %s", 42, "hello");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_WARN, "Warning message to console.");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_ERROR, "Error message to console! This should appear in red!");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_INFO, "Info message to console.");
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_DEBUG, "Debug message to console.");

  // Log to with full callstack information (both original C source and JS callstacks):
  emscripten_log(EM_LOG_C_STACK | EM_LOG_JS_STACK, "A message with as full call stack information as possible:");

  // Log with just mangled JS callstacks:
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_JS_STACK, "This is a message with a mangled JS callstack:");

  // Log only clean C callstack:
  emscripten_log(EM_LOG_NO_PATHS | EM_LOG_C_STACK, "This message should have a clean C callstack:");
}

  // Arbitrary function signature to add some content to callstack.
void __attribute__((noinline)) bar(int = 0, char * = 0, double = 0) {
  if (1 == 2)
    MYASSERT(2 == 1, "World falls apart!");
  else
    MYASSERT(1 == 1, "");

  int flags = EM_LOG_NO_PATHS | EM_LOG_JS_STACK | EM_LOG_FUNC_PARAMS;
#ifndef RUN_FROM_JS_SHELL
  flags |= EM_LOG_C_STACK;
#endif

  // We can programmatically get the callstack.
  // 1. Ask for callstack length:
  int nbytes = emscripten_get_callstack(flags, 0, 0);
  // 2. Allocate temp memory to hold the callstack.
  char *callstack = new char[nbytes];
  // 3. Obtain it.
  // 4. Do something with the callstack string.

  emscripten_get_callstack(flags, callstack, nbytes);

  /* The callstack should be something like
    at bar(int, char*, double) (src.cpp.o.js:5383:12)
    at void Foo<int>() (src.cpp.o.js:5417:4)
    at main() (src.cpp.o.js:5404:2)
    at Object.callMain (src.cpp.o.js:71344:30)
    at doRun (src.cpp.o.js:71383:25)
    at run (src.cpp.o.js:71396:5)
    at Object.<anonymous> (src.cpp.o.js:71439:1)
    at Module._compile (module.js:456:26)

    but the line numbers will greatly vary depending on the mode we are compiling in, so cannot test with direct string comparison. */

  if ((flags & EM_LOG_C_STACK) != 0) {
    // TODO(https://github.com/emscripten-core/emscripten/issues/13089)
    // We should be able to check for emscripten_log.cpp here but sadly
    // source maps seems to be broken under wasm.
#if 0
    MYASSERT(!!strstr(callstack, ".cpp:"), "Callstack was %s!", callstack);
#endif
  } else {
    MYASSERT(!!strstr(callstack, ".js:"), "Callstack was %s!", callstack);
  }
  MYASSERT(!!strstr(callstack, "bar(int, char*, double)"), "Callstack was %s!", callstack);
  MYASSERT(!!strstr(callstack, "void Foo<int>()"), "Callstack was %s!", callstack);

  // 5. Clean up.
  delete[] callstack;

  // Test that obtaining a truncated callstack works. (https://github.com/emscripten-core/emscripten/issues/2171)
  const size_t callstack_buf_len = 50;
  char *buffer = new char[callstack_buf_len+1];
  buffer[callstack_buf_len] = 0x01; // Magic sentinel that should not change its value.
  emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_NO_PATHS | EM_LOG_FUNC_PARAMS, buffer, callstack_buf_len);
  MYASSERT(!!strstr(buffer, "bar(int,"), "Truncated callstack was %s!", buffer);
  MYASSERT(buffer[callstack_buf_len] == 0x01, "");
  delete[] buffer;

  // Or alternatively use a fixed-size buffer for the callstack (and get a truncated output if it was too small).
  char str[1024];
  emscripten_get_callstack(EM_LOG_NO_PATHS | EM_LOG_JS_STACK, str, 1024);

  // TODO(sbc): should we try to revive these checks? The callstacks don't look quite like
  // this in the wasm world and we already have coverage above I think?
#if 0
  /* With EM_LOG_JS_STACK, the callstack will be
    at __Z3bariPcd (src.cpp.o.js:5394:12)
    at __Z3FooIiEvv (src.cpp.o.js:5417:4)
    at Object._main (src.cpp.o.js:5404:2)
    at Object.callMain (src.cpp.o.js:71344:30)
    at doRun (src.cpp.o.js:71383:25)
    at run (src.cpp.o.js:71396:5)
    at Object.<anonymous> (src.cpp.o.js:71439:1)
    at Module._compile (module.js:456:26) */
#ifdef RUN_FROM_JS_SHELL
  MYASSERT(!!strstr(str, "at __Z3bariPcd (src.cpp"), "Callstack was %s!", str);
  MYASSERT(!!strstr(str, "at __Z3FooIiEvv (src.cpp"), "Callstack was %s!", str);
#else
  MYASSERT(!!strstr(str, "at __Z3bariPcd (page.js"), "Callstack was %s!", str);
  MYASSERT(!!strstr(str, "at __Z3FooIiEvv (page.js"), "Callstack was %s!", str);
#endif
#endif
}

// Arbitrary function signature to add some content to callstack.
template<typename T>
void __attribute__((noinline)) Foo() {
  bar();
}

#define TestLog(args...)        emscripten_log(EM_LOG_CONSOLE, args)

void PrintDoubleStuff(double first, double second) {
  double divided = first / second;

  TestLog("%f %f %f\n", first, second, divided);
  TestLog("%d %d %d\n", (int)(first * 1000000), (int)(second * 1000000), (int)(divided * 1000000));
  TestLog("%f %d %d\n", first, (int)(second * 1000000), (int)(divided * 1000000));
  TestLog("%d %f %d\n", (int)(first * 1000000), second, (int)(divided * 1000000));
  TestLog("%d %d %f\n", (int)(first * 1000000), (int)(second * 1000000), divided);
  TestLog("%d %f %f\n", (int)(first * 1000000), second, divided);
  TestLog("%f %d %f\n", first, (int)(second * 1000000), divided);
  TestLog("%f %f %d\n", first, second, (int)(divided * 1000000));
}

void DoubleTest() {
  PrintDoubleStuff(12.3456789, 9.12345678);
}

int main() {
  int test = 123;
  emscripten_log(EM_LOG_FUNC_PARAMS | EM_LOG_CONSOLE, "test print %d\n", test);

  Foo<int>();

  DoubleTest();

  if (failed) {
    printf("Failed!\n");
    return 1;
  }

  printf("Success!\n");
  return 0;
}
