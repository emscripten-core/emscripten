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
	assert(thread);

	SDL_WaitThread(thread, &result);
	assert(result == 4);
	return 0;
}
