/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

// Emscripten fetch attributes:
// If passed, the body of the request will be present in full in the onsuccess()
// handler.
#define EMSCRIPTEN_FETCH_LOAD_TO_MEMORY  1

// If passed, the intermediate streamed bytes will be passed in to the
// onprogress() handler. If not specified, the onprogress() handler will still
// be called, but without data bytes.  Note: Firefox only as it depends on
// 'moz-chunked-arraybuffer'.
#define EMSCRIPTEN_FETCH_STREAM_DATA 2

// If passed, the final download will be stored in IndexedDB. If not specified,
// the file will only reside in browser memory.
#define EMSCRIPTEN_FETCH_PERSIST_FILE 4

// Looks up if the file already exists in IndexedDB, and if so, it is returned
// without redownload. If a partial transfer exists in IndexedDB, the download
// will resume from where it left off and run to completion.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and
// EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.  If none of these three
// flags is specified, the fetch operation is implicitly treated as if
// EMSCRIPTEN_FETCH_APPEND had been passed.
#define EMSCRIPTEN_FETCH_APPEND 8

// If the file already exists in IndexedDB, the old file will be deleted and a
// new download is started.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and
// EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.  If you would like to
// perform an XHR that neither reads or writes to IndexedDB, pass this flag
// EMSCRIPTEN_FETCH_REPLACE, and do not pass the flag
// EMSCRIPTEN_FETCH_PERSIST_FILE.
#define EMSCRIPTEN_FETCH_REPLACE 16

// If specified, the file will only be looked up in IndexedDB, but if it does
// not exist, it is not attempted to be downloaded over the network but an error
// is raised.
// EMSCRIPTEN_FETCH_APPEND, EMSCRIPTEN_FETCH_REPLACE and
// EMSCRIPTEN_FETCH_NO_DOWNLOAD are mutually exclusive.
#define EMSCRIPTEN_FETCH_NO_DOWNLOAD 32

// If specified, emscripten_fetch() will synchronously run to completion before
// returning.  The callback handlers will be called from within
// emscripten_fetch() while the operation is in progress.
#define EMSCRIPTEN_FETCH_SYNCHRONOUS 64

// If specified, it will be possible to call emscripten_fetch_wait() on the
// fetch to test or wait for its completion.
#define EMSCRIPTEN_FETCH_WAITABLE 128

struct emscripten_fetch_t;

// Specifies the parameters for a newly initiated fetch operation.
typedef struct emscripten_fetch_attr_t {
  // 'POST', 'GET', etc.
  char requestMethod[32];

  // Custom data that can be tagged along the process.
  void *userData;

  void (*onsuccess)(struct emscripten_fetch_t *fetch);
  void (*onerror)(struct emscripten_fetch_t *fetch);
  void (*onprogress)(struct emscripten_fetch_t *fetch);
  void (*onreadystatechange)(struct emscripten_fetch_t *fetch);

  // EMSCRIPTEN_FETCH_* attributes
  uint32_t attributes;

  // Specifies the amount of time the request can take before failing due to a
  // timeout.
  unsigned long timeoutMSecs;

  // Indicates whether cross-site access control requests should be made using
  // credentials.
  EM_BOOL withCredentials;

  // Specifies the destination path in IndexedDB where to store the downloaded
  // content body. If this is empty, the transfer is not stored to IndexedDB at
  // all.  Note that this struct does not contain space to hold this string, it
  // only carries a pointer.
  // Calling emscripten_fetch() will make an internal copy of this string.
  const char *destinationPath;

  // Specifies the authentication username to use for the request, if necessary.
  // Note that this struct does not contain space to hold this string, it only
  // carries a pointer.
  // Calling emscripten_fetch() will make an internal copy of this string.
  const char *userName;

  // Specifies the authentication username to use for the request, if necessary.
  // Note that this struct does not contain space to hold this string, it only
  // carries a pointer.
  // Calling emscripten_fetch() will make an internal copy of this string.
  const char *password;

  // Points to an array of strings to pass custom headers to the request. This
  // array takes the form
  // {"key1", "value1", "key2", "value2", "key3", "value3", ..., 0 }; Note
  // especially that the array needs to be terminated with a null pointer.
  const char * const *requestHeaders;

  // Pass a custom MIME type here to force the browser to treat the received
  // data with the given type.
  const char *overriddenMimeType;

  // If non-zero, specifies a pointer to the data that is to be passed as the
  // body (payload) of the request that is being performed. Leave as zero if no
  // request body needs to be sent.  The memory pointed to by this field is
  // provided by the user, and needs to be valid throughout the duration of the
  // fetch operation. If passing a non-zero pointer into this field, make sure
  // to implement *both* the onsuccess and onerror handlers to be notified when
  // the fetch finishes to know when this memory block can be freed. Do not pass
  // a pointer to memory on the stack or other temporary area here.
  const char *requestData;

  // Specifies the length of the buffer pointed by 'requestData'. Leave as 0 if
  // no request body needs to be sent.
  size_t requestDataSize;
} emscripten_fetch_attr_t;

typedef struct emscripten_fetch_t {
  // Unique identifier for this fetch in progress.
  unsigned int id;

  // Custom data that can be tagged along the process.
  void *userData;

  // The remote URL that is being downloaded.
  const char *url;

  // In onsuccess() handler:
  //   - If the EMSCRIPTEN_FETCH_LOAD_TO_MEMORY attribute was specified for the
  //     transfer, this points to the body of the downloaded data. Otherwise
  //     this will be null.
  // In onprogress() handler:
  //   - If the EMSCRIPTEN_FETCH_STREAM_DATA attribute was specified for the
  //     transfer, this points to a partial chunk of bytes related to the
  //     transfer. Otherwise this will be null.
  // The data buffer provided here has identical lifetime with the
  // emscripten_fetch_t object itself, and is freed by calling
  // emscripten_fetch_close() on the emscripten_fetch_t pointer.
  const char *data;

  // Specifies the length of the above data block in bytes. When the download
  // finishes, this field will be valid even if EMSCRIPTEN_FETCH_LOAD_TO_MEMORY
  // was not specified.
  uint64_t numBytes;

  // If EMSCRIPTEN_FETCH_STREAM_DATA is being performed, this indicates the byte
  // offset from the start of the stream that the data block specifies. (for
  // onprogress() streaming XHR transfer, the number of bytes downloaded so far
  // before this chunk)
  uint64_t dataOffset;

  // Specifies the total number of bytes that the response body will be.
  // Note: This field may be zero, if the server does not report the
  // Content-Length field.
  uint64_t totalBytes;

  // Specifies the readyState of the XHR request:
  // 0: UNSENT: request not sent yet
  // 1: OPENED: emscripten_fetch has been called.
  // 2: HEADERS_RECEIVED: emscripten_fetch has been called, and headers and
  //    status are available.
  // 3: LOADING: download in progress.
  // 4: DONE: download finished.
  // See https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/readyState
  unsigned short readyState;

  // Specifies the status code of the response.
  unsigned short status;

  // Specifies a human-readable form of the status code.
  char statusText[64];

  _Atomic uint32_t __proxyState;

  // For internal use only.
  emscripten_fetch_attr_t __attributes;
} emscripten_fetch_t;

// Clears the fields of an emscripten_fetch_attr_t structure to their default
// values in a future-compatible manner.
void emscripten_fetch_attr_init(emscripten_fetch_attr_t *fetch_attr);

// Initiates a new Emscripten fetch operation, which downloads data from the
// given URL or from IndexedDB database.
emscripten_fetch_t *emscripten_fetch(emscripten_fetch_attr_t *fetch_attr, const char *url);

// Synchronously blocks to wait for the given fetch operation to complete. This
// operation is not allowed in the main browser thread, in which case it will
// return EMSCRIPTEN_RESULT_NOT_SUPPORTED. Pass timeoutMSecs=infinite to wait
// indefinitely. If the wait times out, the return value will be
// EMSCRIPTEN_RESULT_TIMEOUT.
// The onsuccess()/onerror()/onprogress() handlers will be called in the calling
// thread from within this function before this function returns.
EMSCRIPTEN_RESULT emscripten_fetch_wait(emscripten_fetch_t *fetch, double timeoutMSecs);

// Closes a finished or an executing fetch operation and frees up all memory. If
// the fetch operation was still executing, the onerror() handler will be called
// in the calling thread before this function returns.
EMSCRIPTEN_RESULT emscripten_fetch_close(emscripten_fetch_t *fetch);

// Gets the size (in bytes) of the response headers as plain text.
// This must be called on the same thread as the fetch originated on.
// Note that this will return 0 if readyState < HEADERS_RECEIVED.
size_t emscripten_fetch_get_response_headers_length(emscripten_fetch_t *fetch);

// Gets the response headers as plain text. dstSizeBytes should be
// headers_length + 1 (for the null terminator).
// This must be called on the same thread as the fetch originated on.
size_t emscripten_fetch_get_response_headers(emscripten_fetch_t *fetch, char *dst, size_t dstSizeBytes);

// Converts the plain text headers into an array of strings. This array takes
// the form {"key1", "value1", "key2", "value2", "key3", "value3", ..., 0 };
// Note especially that the array is terminated with a null pointer.
char **emscripten_fetch_unpack_response_headers(const char *headersString);

// This frees the memory used by the array of headers. Call this when finished
// with the data returned by emscripten_fetch_unpack_response_headers.
void emscripten_fetch_free_unpacked_response_headers(char **unpackedHeaders);

#define emscripten_asmfs_open_t int

// The following flags specify how opening files for reading works (from
// strictest behavior to most flexible)

// When a file is opened for reading, the file data must already fully reside in
// memory. (most similar to MEMFS behavior)
#define EMSCRIPTEN_ASMFS_OPEN_MEMORY    0

// The file data does not need to be already in memory, but can reside in
// IndexedDB.
#define EMSCRIPTEN_ASMFS_OPEN_INDEXEDDB 1

// The file will be downloaded from remote server, as long as it has an index
// entry in local filesystem.
#define EMSCRIPTEN_ASMFS_OPEN_REMOTE    2

// A file entry does not need to exist on the local filesystem, but discovery
// will be attempted from remote server via an XHR first.
#define EMSCRIPTEN_ASMFS_OPEN_REMOTE_DISCOVER 3

// Specifies how calls to non-truncating open(), fopen(), std::ifstream etc.
// behave on the calling thread.
void emscripten_asmfs_set_file_open_behavior(emscripten_asmfs_open_t behavior);

// Returns the current file open behavior modein the calling thread.
emscripten_asmfs_open_t emscripten_asmfs_get_file_open_behavior();

// Records the URL from where the given file on the ASMFS filesystem can be
// obtained from.
void emscripten_asmfs_set_remote_url(const char *filename, const char *remoteUrl);

// Given a filename, outputs the remote URL address that file can be located in.
void emscripten_asmfs_remote_url(const char *filename, char *outRemoteUrl, int maxBytesToWrite);

// Unloads the given file from the ASMFS filesystem. Call this function to save
// memory from files that have been already loaded in to memory and will no
// longer be needed.
void emscripten_asmfs_unload_data(const char *pathname);

// Starts an asynchronous preload of a file from the given URL to the local
// filesystem to destination path 'pathname' for synchronous access on the main
// thread. Specify a onsuccess callback in options structure to be notified of
// when the transfer finishes.
// The resulting download will always be performed with the flag
// EMSCRIPTEN_FETCH_LOAD_TO_MEMORY and without the flags
// EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_WAITABLE |
// EMSCRIPTEN_FETCH_STREAM_DATA. The remaining flags
// EMSCRIPTEN_FETCH_NO_DOWNLOAD, EMSCRIPTEN_FETCH_PERSIST_FILE,
// EMSCRIPTEN_FETCH_APPEND/REPLACE are customizable in the options field. In
// particular, if EMSCRIPTEN_FETCH_NO_DOWNLOAD is passed, then the file is
// loaded to memory for synchronous access by looking at IndexedDB only.
EMSCRIPTEN_RESULT emscripten_asmfs_preload_file(const char *url, const char *pathname, int mode, emscripten_fetch_attr_t *options);

// Computes the total amount of bytes in memory utilized by the filesystem at
// the moment.
// Note: This function can be slow since it walks through the whole filesystem.
uint64_t emscripten_asmfs_compute_memory_usage();

#ifdef __cplusplus
}
#endif
