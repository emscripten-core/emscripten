#include <emscripten/html5.h>
#include <assert.h>

int func1Executed = 0;
int func2Executed = 0;

void func1(void *userData);

void func2(void *userData)
{
	assert((int)userData == 2);
	++func2Executed;

	if (func2Executed == 1)
	{
		// Test canceling a setTimeout: register a callback but then cancel it immediately
		long id = emscripten_set_timeout(func1, 10, (void*)2);
		emscripten_clear_timeout(id);

		emscripten_set_timeout(func2, 100, (void*)2);
	}
	if (func2Executed == 2)
	{
#ifdef REPORT_RESULT
		assert(func1Executed == 1);
		REPORT_RESULT(0);
#endif
	}
}

void func1(void *userData)
{
	assert((int)userData == 1);
	++func1Executed;

#ifdef REPORT_RESULT
	assert(func1Executed == 1);
#endif

	emscripten_set_timeout(func2, 100, (void*)2);
}

int main()
{
	emscripten_set_timeout(func1, 100, (void*)1);
	EM_ASM(noExitRuntime = 1);
}
