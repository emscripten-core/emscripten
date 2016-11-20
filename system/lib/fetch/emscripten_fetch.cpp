#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>
#include <math.h>

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
	EM_ASM_INT( { console.log('Queued fetch to fetch-worker to process. There are now ' + $0 + ' operations in the queue.') }, 
		queue->numQueuedItems);
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

	const bool waitable = (fetch_attr->attributes & EMSCRIPTEN_FETCH_WAITABLE) != 0;
	const bool synchronous = (fetch_attr->attributes & EMSCRIPTEN_FETCH_SYNCHRONOUS) != 0;
	const bool readFromIndexedDB = (fetch_attr->attributes & (EMSCRIPTEN_FETCH_APPEND | EMSCRIPTEN_FETCH_NO_DOWNLOAD)) != 0;
	const bool writeToIndexedDB = (fetch_attr->attributes & EMSCRIPTEN_FETCH_PERSIST_FILE) != 0 || !strncmp(fetch_attr->requestMethod, "EM_IDB_", strlen("EM_IDB_"));
	const bool performXhr = (fetch_attr->attributes & EMSCRIPTEN_FETCH_NO_DOWNLOAD) == 0;
	const bool isMainBrowserThread = emscripten_is_main_browser_thread() != 0;
	if (isMainBrowserThread && synchronous && (performXhr || readFromIndexedDB || writeToIndexedDB))
	{
		EM_ASM_INT( { Module['printErr']('emscripten_fetch("' + Pointer_stringify($0) + '") failed! Synchronous blocking XHRs and IndexedDB operations are not supported on the main browser thread. Try dropping the EMSCRIPTEN_FETCH_SYNCHRONOUS flag, or run with the linker flag --proxy-to-worker to decouple main C runtime thread from the main browser thread.') }, 
			url);
		return 0;
	}

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

#if __EMSCRIPTEN_PTHREADS__
	// Depending on the type of fetch, we can either perform it in the same Worker/thread than the caller, or we might need
	// to run it in a separate Worker. There is a dedicated fetch worker that is available for the fetch, but in some scenarios
	// it might be desirable to run in the same Worker as the caller, so deduce here whether to run the fetch in this thread,
	// or if we need to use the fetch-worker instead.
	if (waitable // Waitable fetches can be synchronously waited on, so must always be proxied
		|| (synchronous && (readFromIndexedDB || writeToIndexedDB))) // Synchronous IndexedDB access needs proxying
	{
		emscripten_atomic_store_u32(&fetch->__proxyState, 1); // sent to proxy worker.
		emscripten_proxy_fetch(fetch);

		if (synchronous) emscripten_fetch_wait(fetch, INFINITY);
	}
	else
#endif
		emscripten_start_fetch(fetch);
	return fetch;
}

EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMsecs)
{
#if __EMSCRIPTEN_PTHREADS__
	if (!fetch) return EMSCRIPTEN_RESULT_INVALID_PARAM;
	uint32_t proxyState = emscripten_atomic_load_u32(&fetch->__proxyState);
	if (proxyState == 2) return EMSCRIPTEN_RESULT_SUCCESS; // already finished.
	if (proxyState != 1) return EMSCRIPTEN_RESULT_INVALID_PARAM; // the fetch should be ongoing?
// #ifdef FETCH_DEBUG
	EM_ASM({ console.log('fetch: emscripten_fetch_wait..') });
// #endif
	// TODO: timeoutMsecs is currently ignored. Return EMSCRIPTEN_RESULT_TIMED_OUT on timeout.
	while(proxyState == 1/*sent to proxy worker*/)
	{
		emscripten_futex_wait(&fetch->__proxyState, proxyState, 100 /*TODO HACK:Sleep sometimes doesn't wake up?*/);//timeoutMsecs);
		proxyState = emscripten_atomic_load_u32(&fetch->__proxyState);
	}
// #ifdef FETCH_DEBUG
	EM_ASM({ console.log('fetch: emscripten_fetch_wait done..') });
// #endif

	if (proxyState == 2) return EMSCRIPTEN_RESULT_SUCCESS;
	else return EMSCRIPTEN_RESULT_FAILED;
#else
	EM_ASM({ console.error('fetch: emscripten_fetch_wait is not available when building without pthreads!') });
	return EMSCRIPTEN_RESULT_FAILED;
#endif
}

EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch)
{
	if (!fetch) return EMSCRIPTEN_RESULT_SUCCESS; // Closing null pointer is ok, same as with free().

#if __EMSCRIPTEN_PTHREADS__
	emscripten_atomic_store_u32(&fetch->__proxyState, 0);
#endif
	// This function frees the fetch pointer so that it is invalid to access it anymore.
	// Use a few key fields as an integrity check that we are being passed a good pointer to a valid fetch structure,
	// which has not been yet closed. (double close is an error)
	if (fetch->id == 0 || fetch->readyState > 4) return EMSCRIPTEN_RESULT_INVALID_PARAM;

	// This fetch is aborted. Call the error handler if the fetch was still in progress and was canceled in flight.
	if (fetch->readyState != 4 /*DONE*/ && fetch->__attributes.onerror)
	{
		fetch->status = (unsigned short)-1;
		strcpy(fetch->statusText, "aborted with emscripten_fetch_close()");
		fetch->__attributes.onerror(fetch);
	}
	fetch->id = 0;
	free((void*)fetch->data);
	free(fetch);
	return EMSCRIPTEN_RESULT_SUCCESS;
}
