#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>

void Sleep(double msecs)
{
	double t1 = emscripten_get_now();
	emscripten_thread_sleep(msecs);
	double t2 = emscripten_get_now();
	printf("emscripten_thread_sleep() slept for %f msecs.\n", t2 - t1);

	assert(t2 - t1 >= 0.9 * msecs); // Should have slept ~ the requested time.
}

void *thread_main(void *arg)
{
	EM_ASM(Module.print('hello from thread!'));

	Sleep(1);
	Sleep(10);
	Sleep(100);
	Sleep(1000);
	Sleep(5000);

#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
	return 0;
}

int main()
{
	// Bad bad bad to sleep on the main thread, but test that it works.
	Sleep(1);
	Sleep(10);
	Sleep(100);
	Sleep(1000);
	Sleep(5000);
	pthread_t thread;
	pthread_create(&thread, NULL, thread_main, NULL);
	EM_ASM(noExitRuntime=true);
}
