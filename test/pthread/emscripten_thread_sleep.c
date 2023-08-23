#include <pthread.h>
#include <emscripten.h>
#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <stdio.h>

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
	emscripten_out("hello from thread!");

	Sleep(1);
	Sleep(10);
	Sleep(100);
	Sleep(1000);
	Sleep(5000);

	emscripten_force_exit(0);
	return NULL;
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
	emscripten_exit_with_live_runtime();
	__builtin_trap();
}
