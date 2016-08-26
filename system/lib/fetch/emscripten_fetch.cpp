#include <memory.h>
#include <stdlib.h>
#include <stdio.h> //todoremove
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <emscripten/threading.h>

struct __emscripten_fetch_queue
{
	emscripten_fetch_t **queuedOperations;
	int numQueuedItems;
	int queueSize;
};

extern "C" {
	void emscripten_start_fetch(emscripten_fetch_t *fetch);
	__emscripten_fetch_queue *_emscripten_get_fetch_work_queue();

	__emscripten_fetch_queue *_emscripten_get_fetch_queue()
	{
		__emscripten_fetch_queue *queue = _emscripten_get_fetch_work_queue();
		if (!queue->queuedOperations)
		{
			queue->queueSize = 64;
			queue->numQueuedItems = 0;
			queue->queuedOperations = (emscripten_fetch_t**)malloc(sizeof(emscripten_fetch_t*) * queue->queueSize);
		}
		return queue;
	}
}

void emscripten_proxy_fetch(emscripten_fetch_t *fetch)
{
	// TODO: mutex lock
	__emscripten_fetch_queue *queue = _emscripten_get_fetch_queue();
//	TODO handle case when queue->numQueuedItems >= queue->queueSize
	queue->queuedOperations[queue->numQueuedItems++] = fetch;
	printf("Queued fetch to fetch-worker to process. There are now %d operations in the queue\n", queue->numQueuedItems);
	// TODO: mutex unlock
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

	if ((fetch->__attributes.attributes & EMSCRIPTEN_FETCH_WAITABLE) != 0)
	{
		emscripten_atomic_store_u32(&fetch->__proxyState, 1); // sent to proxy worker.
		emscripten_proxy_fetch(fetch);
	}
	else
		emscripten_start_fetch(fetch);
	return fetch;
}

EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMsecs)
{
	if (!fetch) return EMSCRIPTEN_RESULT_INVALID_PARAM;
	uint32_t proxyState = emscripten_atomic_load_u32(&fetch->__proxyState);
	if (proxyState == 2) return EMSCRIPTEN_RESULT_SUCCESS; // already finished.
	if (proxyState != 1) return EMSCRIPTEN_RESULT_INVALID_PARAM; // the fetch should be ongoing?
// #ifdef FETCH_DEBUG
	printf("fetch: emscripten_fetch_wait..\n");
// #endif
	while(proxyState == 1/*sent to proxy worker*/)
	{
		emscripten_futex_wait(&fetch->__proxyState, proxyState, 100 /*TODO HACK:Sleep sometimes doesn't wake up?*/);//timeoutMsecs);
		proxyState = emscripten_atomic_load_u32(&fetch->__proxyState);
	}
// #ifdef FETCH_DEBUG
	printf("fetch: emscripten_fetch_wait done..\n");
// #endif

	if (proxyState == 2) return EMSCRIPTEN_RESULT_SUCCESS;
	else return EMSCRIPTEN_RESULT_FAILED;
}

EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch)
{
	emscripten_atomic_store_u32(&fetch->__proxyState, 0);
	free(fetch); // TODO: thread-safety before freeing (what if freeing an operation in progress? explicit emscripten_fetch_abort()?)
	return EMSCRIPTEN_RESULT_SUCCESS;
}
