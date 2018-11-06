/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __emscripten_fetch_h__
#define __emscripten_fetch_h__

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

// Emscripten fetch attributes:
// If passed, the body of the request will be present in full in the onsuccess() handler.
#define EMSCRIPTEN_FETCH_LOAD_TO_MEMORY  1

// If passed, the intermediate streamed bytes will be passed in to the onprogress() handler. If not specified, the
// onprogress() handler will still be called, but without data bytes.
#define EMSCRIPTEN_FETCH_STREAM_DATA 2

// If passed, the final download will be stored in IndexedDB. If not specified, the file will only reside in browser memory.
#define EMSCRIPTEN_FETCH_PERSIST_FILE 4

// If the file already exists in IndexedDB, it is returned without redownload. If a partial transfer exists in IndexedDB,
// the download will resume from where it left off and run to completion.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.
#define EMSCRIPTEN_FETCH_APPEND 8

// If the file already exists in IndexedDB, the old file will be deleted and a new download is started.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.
#define EMSCRIPTEN_FETCH_REPLACE 16

// If specified, the file will only be looked up in IndexedDB, but if it does not exist, it is not attempted to be downloaded
// over the network but an error is raised.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.
#define EMSCRIPTEN_FETCH_NO_DOWNLOAD 32

// If specified, emscripten_fetch() will synchronously run to completion before returning.
// The callback handlers will be called from within emscripten_fetch() while the operation is in progress.
#define EMSCRIPTEN_FETCH_SYNCHRONOUS 64

// If specified, it will be possible to call emscripten_fetch_wait() on the fetch
// to test or wair for its completion.
#define EMSCRIPTEN_FETCH_WAITABLE 128

struct emscripten_fetch_t;

// Specifies the parameters for a newly initiated fetch operation.
typedef struct emscripten_fetch_attr_t
{
	// 'POST', 'GET', etc.
	char requestMethod[32];

	// Custom data that can be tagged along the process.
	void *userData;

	void (*onsuccess)(struct emscripten_fetch_t *fetch);
	void (*onerror)(struct emscripten_fetch_t *fetch);
	void (*onprogress)(struct emscripten_fetch_t *fetch);

	// EMSCRIPTEN_FETCH_* attributes
	uint32_t attributes;

	// Specifies the amount of time the request can take before failing due to a timeout.
	unsigned long timeoutMSecs;

	// Indicates whether cross-site access control requests should be made using credentials.
	EM_BOOL withCredentials;

	// Specifies the destination path in IndexedDB where to store the downloaded content body. If this is empty, the transfer
	// is not stored to IndexedDB at all.
	// Note that this struct does not contain space to hold this string, it only carries a pointer.
	// Calling emscripten_fetch() will make an internal copy of this string.
	const char *destinationPath;

	// Specifies the authentication username to use for the request, if necessary.
	// Note that this struct does not contain space to hold this string, it only carries a pointer.
	// Calling emscripten_fetch() will make an internal copy of this string.
	const char *userName;

	// Specifies the authentication username to use for the request, if necessary.
	// Note that this struct does not contain space to hold this string, it only carries a pointer.
	// Calling emscripten_fetch() will make an internal copy of this string.
	const char *password;

	// Points to an array of strings to pass custom headers to the request. This array takes the form
	// {"key1", "value1", "key2", "value2", "key3", "value3", ..., 0 }; Note especially that the array
	// needs to be terminated with a null pointer.
	const char * const *requestHeaders;

	// Pass a custom MIME type here to force the browser to treat the received data with the given type.
	const char *overriddenMimeType;

	// If non-zero, specifies a pointer to the data that is to be passed as the body (payload) of the request
	// that is being performed. Leave as zero if no request body needs to be sent.
	// The memory pointed to by this field is provided by the user, and needs to be valid throughout the
	// duration of the fetch operation. If passing a non-zero pointer into this field, make sure to implement
	// *both* the onsuccess and onerror handlers to be notified when the fetch finishes to know when this memory
	// block can be freed. Do not pass a pointer to memory on the stack or other temporary area here.
	const char *requestData;

	// Specifies the length of the buffer pointed by 'requestData'. Leave as 0 if no request body needs to be sent.
	size_t requestDataSize;
} emscripten_fetch_attr_t;

typedef struct emscripten_fetch_t
{
	// Unique identifier for this fetch in progress.
	unsigned int id;

	// Custom data that can be tagged along the process.
	void *userData;

	// The remote URL that is being downloaded.
	const char *url;

	// In onsuccess() handler:
	//   - If the EMSCRIPTEN_FETCH_LOAD_TO_MEMORY attribute was specified for the transfer, this points to the
	//     body of the downloaded data. Otherwise this will be null.
	// In onprogress() handler:
	//   - If the EMSCRIPTEN_FETCH_STREAM_DATA attribute was specified for the transfer, this points to a partial
	//     chunk of bytes related to the transfer. Otherwise this will be null.
	// The data buffer provided here has identical lifetime with the emscripten_fetch_t object itself, and is freed by
	// calling emscripten_fetch_close() on the emscripten_fetch_t pointer.
	const char *data;

	// Specifies the length of the above data block in bytes. When the download finishes, this field will be valid even if
	// EMSCRIPTEN_FETCH_LOAD_TO_MEMORY was not specified.
	uint64_t numBytes;

	// If EMSCRIPTEN_FETCH_STREAM_DATA is being performed, this indicates the byte offset from the start of the stream
	// that the data block specifies. (for onprogress() streaming XHR transfer, the number of bytes downloaded so far before this chunk)
	uint64_t dataOffset;

	// Specifies the total number of bytes that the response body will be.
	// Note: This field may be zero, if the server does not report the Content-Length field.
	uint64_t totalBytes;

	// Specifies the readyState of the XHR request:
	// 0: UNSENT: request not sent yet
	// 1: OPENED: emscripten_fetch has been called.
	// 2: HEADERS_RECEIVED: emscripten_fetch has been called, and headers and status are available.
	// 3: LOADING: download in progress.
	// 4: DONE: download finished.
	// See https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState
	unsigned short readyState;

	// Specifies the status code of the response.
	unsigned short status;

	// Specifies a human-readable form of the status code.
	char statusText[64];

	uint32_t __proxyState;

	// For internal use only.
	emscripten_fetch_attr_t __attributes;
} emscripten_fetch_t;

// Clears the fields of an emscripten_fetch_attr_t structure to their default values in a future-compatible manner.
void emscripten_fetch_attr_init(emscripten_fetch_attr_t *fetch_attr);

// Initiates a new Emscripten fetch operation, which downloads data from the given URL or from IndexedDB database.
emscripten_fetch_t *emscripten_fetch(emscripten_fetch_attr_t *fetch_attr, const char *url);

// Synchronously blocks to wait for the given fetch operation to complete. This operation is not allowed in the main browser
// thread, in which case it will return EMSCRIPTEN_RESULT_NOT_SUPPORTED. Pass timeoutMSecs=infinite to wait indefinitely. If
// the wait times out, the return value will be EMSCRIPTEN_RESULT_TIMEOUT.
// The onsuccess()/onerror()/onprogress() handlers will be called in the calling thread from within this function before
// this function returns.
EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMSecs);

// Closes a finished or an executing fetch operation and frees up all memory. If the fetch operation was still executing, the
// onerror() handler will be called in the calling thread before this function returns.
EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch);

#ifdef __cplusplus
}
#endif

// ~__emscripten_fetch_h__
#endif
