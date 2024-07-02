.. _fetch-api:

=========
Fetch API
=========

The Emscripten Fetch API allows native code to transfer files via XHR (HTTP GET,
PUT, POST) from remote servers, and to persist the downloaded files locally in
browser's IndexedDB storage, so that they can be reaccessed locally on
subsequent page visits. The Fetch API is callable from multiple threads, and the
network requests can be run either synchronously or asynchronously as desired.

.. note::

  In order to use the Fetch API, you would need to compile your code with
  ``-sFETCH``.

Introduction
============

The use of the Fetch API is quick to illustrate via an example. The following
application downloads a file from a web server asynchronously to memory inside
the application heap.

.. code-block:: cpp

  #include <stdio.h>
  #include <string.h>
  #include <emscripten/fetch.h>

  void downloadSucceeded(emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
  }

  void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
  }

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    emscripten_fetch(&attr, "myfile.dat");
  }

If a relative pathname is specified to a call to emscripten_fetch, like in the
above example, the XHR is performed relative to the href (URL) of the current
page. Passing a fully qualified absolute URL allows downloading files across
domains, however these are subject to `HTTP access control (CORS) rules
<https://developer.mozilla.org/en-US/docs/Web/HTTP/Access_control_CORS>`_.

By default the Fetch API runs asynchronously, which means that the
emscripten_fetch() function call returns immediately and the operation will
continue to occur on the background. When the operation finishes, either the
success or the failure callback will be invoked.

Persisting data
===============

The XHR requests issued by the Fetch API are subject to the usual browser
caching behavior. These caches are transient (temporary) so there is no
guarantee that the data will persist in the cache for a given period of time.
Additionally, if the files are somewhat large (multiple megabytes), browsers
typically don't cache the downloads at all.

To enable a more explicit control for persisting the downloaded files, the Fetch
API interacts with the browser's IndexedDB API, which can load and store large
data files that are available on subsequent visits to the page. To enable
IndexedDB storage, pass the EMSCRIPTEN_FETCH_PERSIST_FILE flag in the fetch
attributes:

.. code-block:: cpp

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    ...
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_PERSIST_FILE;
    ...
    emscripten_fetch(&attr, "myfile.dat");
  }

For a full example, see the file
``test/fetch/test_fetch_persist.c``.

Persisting data bytes from memory
---------------------------------

Sometimes it is useful to persist a range of bytes from application memory to
IndexedDB (without having to perform any XHRs). This is possible with the
Emscripten Fetch API by passing the special HTTP action verb "EM_IDB_STORE" to
the Emscripten Fetch operation.

.. code-block:: cpp

  void success(emscripten_fetch_t *fetch) {
    printf("IDB store succeeded.\n");
    emscripten_fetch_close(fetch);
  }

  void failure(emscripten_fetch_t *fetch) {
    printf("IDB store failed.\n");
    emscripten_fetch_close(fetch);
  }

  void persistFileToIndexedDB(const char *outputFilename, uint8_t *data, size_t numBytes) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "EM_IDB_STORE");
    attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;
    attr.requestData = (char *)data;
    attr.requestDataSize = numBytes;
    attr.onsuccess = success;
    attr.onerror = failure;
    emscripten_fetch(&attr, outputFilename);
  }

  int main() {
    // Create data
    uint8_t *data = (uint8_t*)malloc(10240);
    srand(time(NULL));
    for(int i = 0; i < 10240; ++i) data[i] = (uint8_t)rand();

    persistFileToIndexedDB("outputfile.dat", data, 10240);
  }

Deleting a file from IndexedDB
------------------------------

Files can be cleaned up from IndexedDB by using the HTTP action verb "EM_IDB_DELETE":

.. code-block:: cpp

  void success(emscripten_fetch_t *fetch) {
    printf("Deleting file from IDB succeeded.\n");
    emscripten_fetch_close(fetch);
  }

  void failure(emscripten_fetch_t *fetch) {
    printf("Deleting file from IDB failed.\n");
    emscripten_fetch_close(fetch);
  }

  int main() {
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "EM_IDB_DELETE");
    emscripten_fetch(&attr, "filename_to_delete.dat");
  }

Synchronous Fetches
===================

In some scenarios, it would be nice to be able to perform an XHR request or an
IndexedDB file operation synchronously in the calling thread. This can make
porting applications easier and simplify code flow by avoiding the need to pass
a callback.

All types of Emscripten Fetch API operations (XHRs, IndexedDB accesses) can be
performed synchronously by passing the EMSCRIPTEN_FETCH_SYNCHRONOUS flag. When
this flag is passed, the calling thread will block to sleep until the fetch
operation finishes. See the following example.

.. code-block:: cpp

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, "file.dat"); // Blocks here until the operation is complete.
    if (fetch->status == 200) {
      printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
      // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    } else {
      printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    }
    emscripten_fetch_close(fetch);
  }

In the above code sample, the success and failure callback functions are not
used. However, if specified, they will be synchronously called before
emscripten_fetch() returns.

.. note::

  Synchronous Emscripten Fetch operations are subject to a number of
  restrictions, depending on which Emscripten build mode (linker flags) is used:

  - **No flags**: Only asynchronous Fetch operations are available.
  - ``--proxy-to-worker``: Synchronous Fetch operations are allowed for fetches
    that only do an XHR but do not interact with IndexedDB.
  - ``-pthread``: Synchronous Fetch operations are available on
    pthreads, but not on the main thread.
  - ``--proxy-to-worker`` + ``-pthread``: Synchronous Synchronous Fetch operations
    are available both on the main thread and pthreads.

Tracking Progress
=================

For robust fetch management, there are several fields available to track the
status of an XHR.

The onprogress callback is called whenever new data has been received. This
allows one to measure the download speed and compute an ETA for completion.
Additionally, the emscripten_fetch_t structure passes the XHR object fields
readyState, status and statusText, which give information about the HTTP loading
state of the request.

The emscripten_fetch_attr_t object has a timeoutMSecs field which allows
specifying a timeout duration for the transfer. Additionally,
emscripten_fetch_close() can be called at any time for asynchronous and waitable
fetches to abort the download.
The following example illustrates these fields
and the onprogress handler.

.. code-block:: cpp

  void downloadProgress(emscripten_fetch_t *fetch) {
    if (fetch->totalBytes) {
      printf("Downloading %s.. %.2f%% complete.\n", fetch->url, fetch->dataOffset * 100.0 / fetch->totalBytes);
    } else {
      printf("Downloading %s.. %lld bytes complete.\n", fetch->url, fetch->dataOffset + fetch->numBytes);
    }
  }

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onprogress = downloadProgress;
    attr.onerror = downloadFailed;
    emscripten_fetch(&attr, "myfile.dat");
  }

Managing Large Files
====================

Particular attention should be paid to the memory usage strategy of a fetch.
Previous examples have all passed the EMSCRIPTEN_FETCH_LOAD_TO_MEMORY flag,
which causes emscripten_fetch() to populate the downloaded file in full in
memory in the onsuccess() callback. This is convenient when the whole file is to
be immediately accessed afterwards, but for large files, this can be a wasteful
strategy in terms of memory usage. If the file is very large, it might not even
fit inside the application's heap area.

The following subsections provide ways to manage large fetches in a memory
efficient manner.

Downloading directly to IndexedDB
---------------------------------

If an application wants to download a file for local access, but does not
immediately need to use the file, e.g. when preloading data up front for later
access, it is a good idea to avoid the EMSCRIPTEN_FETCH_LOAD_TO_MEMORY flag
altogether, and only pass the EMSCRIPTEN_FETCH_PERSIST_FILE flag instead. This
causes the fetch to download the file directly to IndexedDB, which avoids
temporarily populating the file in memory after the download finishes. In this
scenario, the onsuccess() handler will only report the total downloaded file
size, but will not contain the data bytes to the file.

Streaming Downloads
-------------------

Note: This currently only works in Firefox as it uses 'moz-chunked-arraybuffer'.

If the application does not need random seek access to the file, but is able to
process the file in a streaming manner, it can use the
EMSCRIPTEN_FETCH_STREAM_DATA flag to stream through the bytes in the file as
they are downloaded. If this flag is passed, the downloaded data chunks are
passed into the onprogress() callback in coherent file sequential order. See the
following snippet for an example.

.. code-block:: cpp

  void downloadProgress(emscripten_fetch_t *fetch) {
    printf("Downloading %s.. %.2f%%s complete. HTTP readyState: %d. HTTP status: %d.\n"
      "HTTP statusText: %s. Received chunk [%llu, %llu[\n",
      fetch->url, fetch->totalBytes > 0 ? (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes : (fetch->dataOffset + fetch->numBytes),
      fetch->totalBytes > 0 ? "%" : " bytes",
      fetch->readyState, fetch->status, fetch->statusText,
      fetch->dataOffset, fetch->dataOffset + fetch->numBytes);

    // Process the partial data stream fetch->data[0] thru fetch->data[fetch->numBytes-1]
    // This buffer represents the file at offset fetch->dataOffset.
    for(size_t i = 0; i < fetch->numBytes; ++i)
      ; // Process fetch->data[i];
  }

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_STREAM_DATA;
    attr.onsuccess = downloadSucceeded;
    attr.onprogress = downloadProgress;
    attr.onerror = downloadFailed;
    attr.timeoutMSecs = 2*60;
    emscripten_fetch(&attr, "myfile.dat");
  }

In this case, the onsuccess() handler will not receive the final file buffer at
all so memory usage will remain at a minimum.

Byte Range Downloads
--------------------

Large files can also be managed in smaller chunks by performing Byte Range
downloads on them. This initiates an XHR or IndexedDB transfer that only fetches
the desired subrange of the whole file. This is useful for example when a large
package file contains multiple smaller ones at certain seek offsets, which can
be dealt with separately.

.. code-block:: cpp

  #include <stdio.h>
  #include <string.h>
  #include <emscripten/fetch.h>

  void downloadSucceeded(emscripten_fetch_t *fetch) {
    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    emscripten_fetch_close(fetch); // Free data associated with the fetch.
  }

  void downloadFailed(emscripten_fetch_t *fetch) {
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
  }

  int main() {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    // Make a Range request to only fetch bytes 10 to 20
    const char* headers[] = {"Range", "bytes=10-20", NULL};
    attr.requestHeaders = headers;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    emscripten_fetch(&attr, "myfile.dat");
  }


TODO To Document
================

Emscripten_fetch() supports the following operations as well, that need
documenting:

 - Emscripten_fetch can be used to upload files to remote servers via HTTP PUT
 - Emscripten_fetch_attr_t allows setting custom HTTP request headers (e.g. for
   cache control)
 - Document HTTP simple auth fields in Emscripten_fetch_attr_t.
 - Document overriddenMimeType attribute in Emscripten_fetch_attr_t.
 - Reference documentation of the individual fields in Emscripten_fetch_attr_t,
   Emscripten_fetch_t and #defines.
 - Example about loading only from IndexedDB without XHRing.
 - Example about overriding an existing file in IndexedDB with a new XHR.
 - Example how to preload a whole filesystem to IndexedDB for easy replacement
   of --preload-file.
 - Example how to persist content as gzipped to IndexedDB and decompress on
   load.
 - Example how to abort and resume partial transfers to IndexedDB.
