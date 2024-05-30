// Fetch polyfill from https://github.com/developit/unfetch
// License:
//==============================================================================
// Copyright (c) 2017 Jason Miller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//==============================================================================

#if !POLYFILL
#error "this file should never be included unless POLYFILL is set"
#endif

if (typeof globalThis.fetch == 'undefined') {
  globalThis.fetch = function (url, options) {
    options = options || {};
    return new Promise((resolve, reject) => {
      const request = new XMLHttpRequest();
      const keys = [];
      const headers = {};

      request.responseType = 'arraybuffer';

      const response = () => ({
        ok: ((request.status / 100) | 0) == 2, // 200-299
        statusText: request.statusText,
        status: request.status,
        url: request.responseURL,
        text: () => Promise.resolve(request.responseText),
        json: () => Promise.resolve(request.responseText).then(JSON.parse),
        blob: () => Promise.resolve(new Blob([request.response])),
        arrayBuffer: () => Promise.resolve(request.response),
        clone: response,
        headers: {
          keys: () => keys,
          entries: () => keys.map((n) => [n, request.getResponseHeader(n)]),
          get: (n) => request.getResponseHeader(n),
          has: (n) => request.getResponseHeader(n) != null,
        },
      });

      request.open(options.method || "get", url, true);

      request.onload = () => {
        request
          .getAllResponseHeaders()
          .toLowerCase()
          .replace(/^(.+?):/gm, (m, key) => {
            headers[key] || keys.push((headers[key] = key));
          });
        resolve(response());
      };

      request.onerror = reject;

      request.withCredentials = options.credentials == "include";

      for (const i in options.headers) {
        request.setRequestHeader(i, options.headers[i]);
      }

      request.send(options.body || null);
    });
  }
}
