/**
 * @license
 * Copyright 2016 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#include Fetch.js

var LibraryFetch = {
  $Fetch__postset: 'Fetch.init();',
  $Fetch__deps: ['$HandleAllocator'],
  $Fetch: Fetch,
  _emscripten_fetch_get_response_headers_length__deps: ['$lengthBytesUTF8'],
  _emscripten_fetch_get_response_headers_length: fetchGetResponseHeadersLength,
  _emscripten_fetch_get_response_headers__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  _emscripten_fetch_get_response_headers: fetchGetResponseHeaders,
  emscripten_fetch_free: fetchFree,

#if FETCH_SUPPORT_INDEXEDDB
  $fetchDeleteCachedData: fetchDeleteCachedData,
  $fetchLoadCachedData: fetchLoadCachedData,
  $fetchCacheData: fetchCacheData,
#endif
  $fetchXHR: fetchXHR,

  emscripten_start_fetch: startFetch,
  emscripten_start_fetch__deps: [
    'malloc',
    'free',
    '$Fetch',
    '$fetchXHR',
    '$callUserCallback',
    '$writeI53ToI64',
    '$stringToUTF8',
    '$stringToNewUTF8',
#if FETCH_SUPPORT_INDEXEDDB
    '$fetchCacheData',
    '$fetchLoadCachedData',
    '$fetchDeleteCachedData',
#endif
#if FETCH_STREAMING
    '$FetchXHR',
#endif
  ],
  /**
   * A class that mimics the XMLHttpRequest API using the modern Fetch API.
   * This implementation is specifically tailored to only handle 'arraybuffer'
   * responses.
   */
  $FetchXHR: class {
    constructor() {
      // --- Public XHR Properties ---

      // Event Handlers
      this.onload = null;
      this.onerror = null;
      this.onprogress = null;
      this.onreadystatechange = null;
      this.ontimeout = null;

      // Request Configuration
      this.responseType = 'arraybuffer';
      this.withCredentials = false;
      this.timeout = 0; // Standard XHR timeout property

      // Response / State Properties
      this.readyState = 0; // 0: UNSENT
      this.response = null;
      this.responseURL = '';
      this.status = 0;
      this.statusText = '';

      // --- Internal Properties ---
      this._method = '';
      this._url = '';
      this._headers = {};
      this._abortController = null;
      this._aborted = false;
      this._responseHeaders = null;
    }

    // --- Private state management ---
    _changeReadyState(state) {
      this.readyState = state;
      this.onreadystatechange?.();
    }

    // --- Public XHR Methods ---

    /**
     * Initializes a request.
     * @param {string} method The HTTP request method (e.g., 'GET', 'POST').
     * @param {string} url The URL to send the request to.
     * @param {boolean} [async=true] This parameter is ignored as Fetch is always async.
     * @param {string|null} [user=null] The username for basic authentication.
     * @param {string|null} [password=null] The password for basic authentication.
     */
    open(method, url, async = true, user = null, password = null) {
      if (this.readyState !== 0 && this.readyState !== 4) {
        console.warn("FetchXHR.open() called while a request is in progress.");
        this.abort();
      }

      // Reset internal state for the new request
      this._method = method;
      this._url = url;
      this._headers = {};
      this._responseHeaders = null;

      // The async parameter is part of the XHR API but is an error here because
      // the Fetch API is inherently asynchronous and does not support synchronous requests.
      if (!async) {
        throw new Error("FetchXHR does not support synchronous requests.");
      }

      // Handle Basic Authentication if user/password are provided.
      // This creates a base64-encoded string and sets the Authorization header.
      if (user) {
        const credentials = btoa(`${user}:${password || ''}`);
        this._headers['Authorization'] = `Basic ${credentials}`;
      }

      this._changeReadyState(1); // 1: OPENED
    }

    /**
     * Sets the value of an HTTP request header.
     * @param {string} header The name of the header.
     * @param {string} value The value of the header.
     */
    setRequestHeader(header, value) {
      if (this.readyState !== 1) {
        throw new Error('setRequestHeader can only be called when state is OPENED.');
      }
      this._headers[header] = value;
    }

    /**
     * This method is not effectively implemented because Fetch API relies on the
     * server's Content-Type header and does not support overriding the MIME type
     * on the client side in the same way as XHR.
     * @param {string} mimetype The MIME type to use.
     */
    overrideMimeType(mimetype) {
      throw new Error("overrideMimeType is not supported by the Fetch API and has no effect.");
    }

    /**
     * Returns a string containing all the response headers, separated by CRLF.
     * @returns {string} The response headers.
     */
    getAllResponseHeaders() {
      if (!this._responseHeaders) {
        return '';
      }

      let headersString = '';
      // The Headers object is iterable.
      for (const [key, value] of this._responseHeaders.entries()) {
        headersString += `${key}: ${value}\r\n`;
      }
      return headersString;
    }

    /**
     * Sends the request.
     * @param body The body of the request.
     */
    async send(body = null) {
      if (this.readyState !== 1) {
        throw new Error('send() can only be called when state is OPENED.');
      }

      this._abortController = new AbortController();
      const signal = this._abortController.signal;

      // Handle timeout
      let timeoutID;
      if (this.timeout > 0) {
        timeoutID = setTimeout(
          () => this._abortController.abort(new DOMException('The user aborted a request.', 'TimeoutError')),
          this.timeout
        );
      }

      const fetchOptions = {
        method: this._method,
        headers: this._headers,
        body: body,
        signal: signal,
        credentials: this.withCredentials ? 'include' : 'same-origin',
      };

      try {
        const response = await fetch(this._url, fetchOptions);

        // Populate response properties once headers are received
        this.status = response.status;
        this.statusText = response.statusText;
        this.responseURL = response.url;
        this._responseHeaders = response.headers;
        this._changeReadyState(2); // 2: HEADERS_RECEIVED

        // Start processing the body
        this._changeReadyState(3); // 3: LOADING

        if (!response.body) {
          throw new Error("Response has no body to read.");
        }

        const reader = response.body.getReader();
        const contentLength = +response.headers.get('Content-Length');

        let receivedLength = 0;
        const chunks = [];

        while (true) {
          const { done, value } = await reader.read();
          if (done) {
            break;
          }

          chunks.push(value);
          receivedLength += value.length;

          if (this.onprogress) {
            // Convert to ArrayBuffer as requested by responseType.
            this.response = value.buffer;
            const progressEvent = {
              lengthComputable: contentLength > 0,
              loaded: receivedLength,
              total: contentLength
            };
            this.onprogress(progressEvent);
          }
        }

        // Combine chunks into a single Uint8Array.
        const allChunks = new Uint8Array(receivedLength);
        let position = 0;
        for (const chunk of chunks) {
          allChunks.set(chunk, position);
          position += chunk.length;
        }

        // Convert to ArrayBuffer as requested by responseType
        this.response = allChunks.buffer;
      } catch (error) {
        this.statusText = error.message;

        if (error.name === 'AbortError') {
          // Do nothing.
        } else if (error.name === 'TimeoutError') {
          this.ontimeout?.();
        } else {
          // This is a network error
          this.onerror?.();
        }
      } finally {
        clearTimeout(timeoutID);
        if (!this._aborted) {
          this._changeReadyState(4); // 4: DONE
          // The XHR 'load' event fires for successful HTTP statuses (2xx) as well as
          // unsuccessful ones (4xx, 5xx). The 'error' event is for network failures.
          this.onload?.();
        }
      }
    }

    /**
     * Aborts the request if it has already been sent.
     */
    abort() {
      this._aborted = true;
      this.status = 0;
      this._changeReadyState(4); // 4: DONE
      this._abortController?.abort();
    }
  }
};

addToLibrary(LibraryFetch);
