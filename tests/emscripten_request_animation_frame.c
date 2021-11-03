#include <emscripten/html5.h>
#include <assert.h>

int func1Executed = 0;
int func2Executed = 0;

EM_BOOL func1(double time, void *userData);

EM_BOOL func2(double time, void *userData)
{
	assert((long)userData == 2);
	assert(time > 0);
	++func2Executed;

	if (func2Executed == 1)
	{
		// Test canceling an animation frame: register rAF() but then cancel it immediately
		long id = emscripten_request_animation_frame(func1, (void*)2);
		emscripten_cancel_animation_frame(id);

		emscripten_request_animation_frame(func2, (void*)2);
	}
	if (func2Executed == 2)
	{
#ifdef REPORT_RESULT
		assert(func1Executed == 1);
		REPORT_RESULT(0);
#endif
	}
	return 0;
}

EM_BOOL func1(double time, void *userData)
{
	assert((long)userData == 1);
	assert(time > 0);
	++func1Executed;

#ifdef REPORT_RESULT
	assert(func1Executed == 1);
#endif

	emscripten_request_animation_frame(func2, (void*)2);

	return 0;
}

int main()
{
	emscripten_request_animation_frame(func1, (void*)1);
}
