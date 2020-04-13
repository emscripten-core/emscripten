#include <SDL.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int test_thread(void *data)
{
	return 2 + 2;
}

int main()
{
	SDL_Thread *thread;
	int result;

	thread = SDL_CreateThread(test_thread, "Test Thread", (void *)NULL);

	if (NULL == thread) {
		return 1;
	} else {
		SDL_WaitThread(thread, &result);
	}

#ifdef REPORT_RESULT
	REPORT_RESULT(result);
#endif
	return 0;
}
