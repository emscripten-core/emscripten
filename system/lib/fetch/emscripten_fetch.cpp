#include <memory.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

void emscripten_fetch_attr_init(emscripten_fetch_attr_t *fetch_attr)
{
	memset(fetch_attr, 0, sizeof(emscripten_fetch_attr_t));
}

EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMsecs)
{
	return EMSCRIPTEN_RESULT_SUCCESS;
}

EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch)
{
	return EMSCRIPTEN_RESULT_SUCCESS;
}
