#include <emscripten.h>
#include <stdio.h>
#include <cstring>

#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

#ifndef REPORT_RESULT
#define REPORT_RESULT int dummy
#endif

int result = 1; // If 1, this test succeeded.

// A custom assert macro to test varargs routing to emscripten_log().
#define MYASSERT(condition, ...) \
	do { \
		if (!(condition)) { \
			emscripten_log(EM_LOG_ERROR, "%s", "Condition '" #condition "' failed in file " __FILE__ ":" STRINGIZE(__LINE__) "!"); \
			emscripten_log(EM_LOG_ERROR, ##__VA_ARGS__); \
			result = 0; \
		} \
	} while(0)

void __attribute__((noinline)) kitten()
{
	// Log to Emscripten Module.
	emscripten_log(EM_LOG_NO_PATHS, "Print a log message: int: %d, string: %s.", 42, "hello");
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_WARN, "Print a warning message");
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_ERROR, "This is an error!");

	// Log directly to Browser web inspector/console.
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE, "Info log to console: int: %d, string: %s", 42, "hello");
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_WARN, "Warning message to console.");
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_CONSOLE | EM_LOG_ERROR, "Error message to console! This should appear in red!");

	// Log to with full callstack information (both original C source and JS callstacks):
	emscripten_log(EM_LOG_C_STACK | EM_LOG_JS_STACK | EM_LOG_DEMANGLE, "A message with as full call stack information as possible:");

	// Log with just mangled JS callstacks:
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_JS_STACK, "This is a message with a mangled JS callstack:");

	// Log only clean C callstack:
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_C_STACK | EM_LOG_DEMANGLE, "This message should have a clean C callstack:");

	// We can leave out the message to just print out the callstack:
	printf("The following line should show just the callstack without a message:\n");
	emscripten_log(EM_LOG_NO_PATHS | EM_LOG_ERROR | EM_LOG_C_STACK | EM_LOG_JS_STACK | EM_LOG_DEMANGLE);
}

void __attribute__((noinline)) bar(int = 0, char * = 0, double = 0) // Arbitrary function signature to add some content to callstack.
{
	if (1 == 2)
		MYASSERT(2 == 1, "World falls apart!");
	else
		MYASSERT(1 == 1);

	int flags = EM_LOG_NO_PATHS | EM_LOG_JS_STACK | EM_LOG_DEMANGLE | EM_LOG_FUNC_PARAMS;
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

	if ((flags & EM_LOG_C_STACK) != 0)
	{
		MYASSERT(!!strstr(callstack, "at bar(int, char*, double) (src.cpp:"), "Callstack was %s!", callstack);
		MYASSERT(!!strstr(callstack, "at void Foo<int>() (src.cpp:"), "Callstack was %s!", callstack);
	}
	else
	{
		MYASSERT(!!strstr(callstack, "at bar(int, char*, double) (src.cpp.o.js:"), "Callstack was %s!", callstack);
		MYASSERT(!!strstr(callstack, "at void Foo<int>() (src.cpp.o.js:"), "Callstack was %s!", callstack);
	}

	// 5. Clean up.
	delete[] callstack;

	// Or alternatively use a fixed-size buffer for the callstack (and get a truncated output if it was too small).
	char str[1024];
	emscripten_get_callstack(EM_LOG_NO_PATHS | EM_LOG_JS_STACK, str, 1024);

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
}

template<typename T>
void __attribute__((noinline)) Foo() // Arbitrary function signature to add some content to callstack.
{
	bar();
}

int main()
{
	Foo<int>();
#ifndef RUN_FROM_JS_SHELL
	REPORT_RESULT();
	return 0;
#else
	if (result)
		printf("Success!\n");
#endif
}
