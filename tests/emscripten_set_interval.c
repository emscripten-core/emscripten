#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>

int funcExecuted = 0;

void testDone(void *userData)
{
	assert((int)userData == 2);
	assert(funcExecuted == 10);
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}

long intervalId = 0;

void tick(void *userData)
{
	assert((int)userData == 1);
	++funcExecuted;
	if (funcExecuted == 10)
	{
		emscripten_set_timeout(testDone, 300, (void*)2);
	}
	if (funcExecuted >= 10)
	{
		emscripten_clear_interval(intervalId);
	}
}

int main()
{
	intervalId = emscripten_set_interval(tick, 100, (void*)1);
	EM_ASM(noExitRuntime = 1);
}
