#include <emscripten/html5.h>
#include <assert.h>

double previousRafTime = 0;
int funcExecuted = 0;

void testDone(void *userData)
{
	assert((long)userData == 2);
	assert(funcExecuted == 10);
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}

EM_BOOL tick(double time, void *userData)
{
	assert(time > previousRafTime);
	previousRafTime = time;
	assert((long)userData == 1);
	++funcExecuted;
	if (funcExecuted == 10)
	{
		emscripten_set_timeout(testDone, 300, (void*)2);
	}
	return funcExecuted < 10;
}

int main()
{
	emscripten_request_animation_frame_loop(tick, (void*)1);
}
