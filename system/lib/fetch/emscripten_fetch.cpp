#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

extern "C" {
	void emscripten_start_fetch(emscripten_fetch_t *fetch);
}

void emscripten_fetch_attr_init(emscripten_fetch_attr_t *fetch_attr)
{
	memset(fetch_attr, 0, sizeof(emscripten_fetch_attr_t));
}

static int globalFetchIdCounter = 1;
emscripten_fetch_t *emscripten_fetch(emscripten_fetch_attr_t *fetch_attr, const char *url)
{
	if (!fetch_attr) return 0;
	if (!url) return 0;
	emscripten_fetch_t *fetch = (emscripten_fetch_t *)malloc(sizeof(emscripten_fetch_t));
	memset(fetch, 0, sizeof(emscripten_fetch_t));
	fetch->id = globalFetchIdCounter++; // TODO: make this thread-safe!
	fetch->userData = fetch_attr->userData;
	fetch->url = strdup(url); // TODO: free
	fetch->__attributes = *fetch_attr;
	fetch->__attributes.destinationPath = fetch->__attributes.destinationPath ? strdup(fetch->__attributes.destinationPath) : 0; // TODO: free
	fetch->__attributes.userName = fetch->__attributes.userName ? strdup(fetch->__attributes.userName) : 0; // TODO: free
	fetch->__attributes.password = fetch->__attributes.password ? strdup(fetch->__attributes.password) : 0; // TODO: free
	fetch->__attributes.requestHeaders = 0;// TODO:strdup(fetch->__attributes.requestHeaders);
	fetch->__attributes.overriddenMimeType = fetch->__attributes.overriddenMimeType ? strdup(fetch->__attributes.overriddenMimeType) : 0; // TODO: free

	emscripten_start_fetch(fetch);
	return fetch;
}

EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMsecs)
{
	return EMSCRIPTEN_RESULT_SUCCESS;
}

EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch)
{
	free(fetch);
	return EMSCRIPTEN_RESULT_SUCCESS;
}
