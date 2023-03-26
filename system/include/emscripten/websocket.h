/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <memory.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EMSCRIPTEN_WEBSOCKET_T int

// Returns the WebSocket.readyState field into readyState. readyState must not be a null pointer.
EMSCRIPTEN_RESULT emscripten_websocket_get_ready_state(EMSCRIPTEN_WEBSOCKET_T socket, unsigned short *readyState __attribute__((nonnull)));

// Returns the WebSocket.bufferedAmount field into bufferedAmount. bufferedAmount must not be a null pointer.
EMSCRIPTEN_RESULT emscripten_websocket_get_buffered_amount(EMSCRIPTEN_WEBSOCKET_T socket, size_t *bufferedAmount __attribute__((nonnull)));

// Writes the WebSocket.url field as a UTF-8 string to the memory area pointed by url. The memory area must contain at least urlLength bytes of free space. If this memory area cannot
// fit the url string, it will be truncated. Call emscripten_websocket_get_url_length() to determine how large memory area will be required to store the url.
// url must not be a null pointer.
EMSCRIPTEN_RESULT emscripten_websocket_get_url(EMSCRIPTEN_WEBSOCKET_T socket, char *url __attribute__((nonnull)), int urlLength);
// Returns the byte length needed to store WebSocket.url string in Wasm heap. This length can be passed to emscripten_websocket_get_url as it includes the null byte in the count.
// urlLength must not be a null pointer.
EMSCRIPTEN_RESULT emscripten_websocket_get_url_length(EMSCRIPTEN_WEBSOCKET_T socket, int *urlLength __attribute__((nonnull)));

// Similar to emscripten_websocket_get_url(), but returns WebSocket.extensions field instead.
EMSCRIPTEN_RESULT emscripten_websocket_get_extensions(EMSCRIPTEN_WEBSOCKET_T socket, char *extensions __attribute__((nonnull)), int extensionsLength);
EMSCRIPTEN_RESULT emscripten_websocket_get_extensions_length(EMSCRIPTEN_WEBSOCKET_T socket, int *extensionsLength __attribute__((nonnull)));

// Similar to emscripten_websocket_get_url(), but returns WebSocket.protocol field instead.
EMSCRIPTEN_RESULT emscripten_websocket_get_protocol(EMSCRIPTEN_WEBSOCKET_T socket, char *protocol __attribute__((nonnull)), int protocolLength);
EMSCRIPTEN_RESULT emscripten_websocket_get_protocol_length(EMSCRIPTEN_WEBSOCKET_T socket, int *protocolLength __attribute__((nonnull)));

typedef struct EmscriptenWebSocketOpenEvent {
  EMSCRIPTEN_WEBSOCKET_T socket;
} EmscriptenWebSocketOpenEvent;

typedef EM_BOOL (*em_websocket_open_callback_func)(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent __attribute__((nonnull)), void *userData);
EMSCRIPTEN_RESULT emscripten_websocket_set_onopen_callback_on_thread(EMSCRIPTEN_WEBSOCKET_T socket, void *userData, em_websocket_open_callback_func callback, pthread_t targetThread);

typedef struct EmscriptenWebSocketMessageEvent {
  EMSCRIPTEN_WEBSOCKET_T socket;
  uint8_t *data;
  uint32_t numBytes;
  EM_BOOL isText;
} EmscriptenWebSocketMessageEvent;

typedef EM_BOOL (*em_websocket_message_callback_func)(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent __attribute__((nonnull)), void *userData);
EMSCRIPTEN_RESULT emscripten_websocket_set_onmessage_callback_on_thread(EMSCRIPTEN_WEBSOCKET_T socket, void *userData, em_websocket_message_callback_func callback, pthread_t targetThread);

typedef struct EmscriptenWebSocketErrorEvent {
  EMSCRIPTEN_WEBSOCKET_T socket;
} EmscriptenWebSocketErrorEvent;

typedef EM_BOOL (*em_websocket_error_callback_func)(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent __attribute__((nonnull)), void *userData);
EMSCRIPTEN_RESULT emscripten_websocket_set_onerror_callback_on_thread(EMSCRIPTEN_WEBSOCKET_T socket, void *userData, em_websocket_error_callback_func callback, pthread_t targetThread);

typedef struct EmscriptenWebSocketCloseEvent {
  EMSCRIPTEN_WEBSOCKET_T socket;
  EM_BOOL wasClean;
  unsigned short code;
  char reason[512]; // WebSockets spec enforces this can be max 123 characters, so as UTF-8 at most 123*4 bytes < 512.
} EmscriptenWebSocketCloseEvent;

typedef EM_BOOL (*em_websocket_close_callback_func)(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent __attribute__((nonnull)), void *userData);
EMSCRIPTEN_RESULT emscripten_websocket_set_onclose_callback_on_thread(EMSCRIPTEN_WEBSOCKET_T socket, void *userData, em_websocket_close_callback_func callback, pthread_t targetThread);

#define emscripten_websocket_set_onopen_callback(socket, userData, callback)    emscripten_websocket_set_onopen_callback_on_thread(   (socket), (userData), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)
#define emscripten_websocket_set_onerror_callback(socket, userData, callback)   emscripten_websocket_set_onerror_callback_on_thread(  (socket), (userData), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)
#define emscripten_websocket_set_onclose_callback(socket, userData, callback)   emscripten_websocket_set_onclose_callback_on_thread(  (socket), (userData), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)
#define emscripten_websocket_set_onmessage_callback(socket, userData, callback) emscripten_websocket_set_onmessage_callback_on_thread((socket), (userData), (callback), EM_CALLBACK_THREAD_CONTEXT_CALLING_THREAD)

typedef struct EmscriptenWebSocketCreateAttributes {
  // The target URL to connect to. This string can point to a stack local variable, the string is read immediately at a call to emscripten_websocket_new().
  const char *url;
  // A comma-separated list of protocol strings. Set to e.g. "binary,base64" to create a WebSocket connection with two supported protocols "binary" and "base64".
  // Be careful to avoid leading and trailing spaces, e.g. "binary, base64" may not be interpreted properly.
  // This string can point to a stack local variable, the string is read immediately at a call to emscripten_websocket_new().
  const char *protocols;

  // If true, the created socket will reside on the main browser thread. If false, the created socket is bound to the calling thread.
  // If you want to share the created EMSCRIPTEN_WEBSOCKET_T structure across multiple threads, or are running your own main loop in the
  // pthread that you create the socket, set createOnMainThread to true. If the created WebSocket only needs to be accessible on the thread
  // that created it, and the creating thread is an event based thread (meaning it regularly yields back to the browser event loop), then
  // it is more efficient to set this to false.
  EM_BOOL createOnMainThread;
} EmscriptenWebSocketCreateAttributes;

//extern void emscripten_websocket_init_create_attributes(EmscriptenWebSocketCreateAttributes *attributes);
#define emscripten_websocket_init_create_attributes(attributes) do { memset((attributes), 0, sizeof(EmscriptenWebSocketCreateAttributes)); } while(0)

// Returns true if WebSockets are supported by the current browser
EM_BOOL emscripten_websocket_is_supported(void);

// Creates a new WebSocket and connects it to the given remote host.
// If the return value of this function is > 0, the function has succeeded and the return value represents a handle to the WebSocket object.
// If the return value of this function is < 0, then the function has failed, and the return value can be interpreted as a EMSCRIPTEN_RESULT code
// representing the cause of the failure. If the function returns 0, then the call has failed with an unknown reason (build with -sWEBSOCKET_DEBUG for more information)
EMSCRIPTEN_WEBSOCKET_T emscripten_websocket_new(EmscriptenWebSocketCreateAttributes *createAttributes __attribute__((nonnull)));

// Sends the given string of null-delimited UTF8 encoded text data to the connected server.
EMSCRIPTEN_RESULT emscripten_websocket_send_utf8_text(EMSCRIPTEN_WEBSOCKET_T socket, const char *textData __attribute__((nonnull)));

// Sends the given block of raw memory data out to the connected server.
EMSCRIPTEN_RESULT emscripten_websocket_send_binary(EMSCRIPTEN_WEBSOCKET_T socket, void *binaryData __attribute__((nonnull)), uint32_t dataLength);

// Closes the specified WebSocket. N.B.: the meaning of "closing" a WebSocket means "eager read/lazy write"-closing the socket. That is, all still
// pending untransferred outbound bytes will continue to transfer out, but after calling close on the socket, any pending bytes still in the process
// of being received will never be available. See https://html.spec.whatwg.org/multipage/web-sockets.html#dom-websocket-sclose
// After calling close(), it is no longer possible to send() on the WebSocket to send more bytes.
EMSCRIPTEN_RESULT emscripten_websocket_close(EMSCRIPTEN_WEBSOCKET_T socket, unsigned short code, const char *reason);

// Releases the given WebSocket object and all associated allocated memory for garbage collection. This effectively frees the socket handle, after calling
// this function the given handle no longer exists.
EMSCRIPTEN_RESULT emscripten_websocket_delete(EMSCRIPTEN_WEBSOCKET_T socket);

// This function close()s and releases all created WebSocket connections for the current thread. You can call this at application exit time to enforce
// teardown of all active sockets, although it is optional. When a pthread terminates, it will call this function to delete all active connections bound to
// that specific pthread (sockets created with createOnMainThread=false). Any WebSockets created by a pthread with createOnMainThread=true will remain alive
// even after the pthread quits, although be warned that if the target thread that was registered to handle events for a given WebSocket quits, then those
// events will stop from being delivered altogether.
void emscripten_websocket_deinitialize(void);

#ifdef __cplusplus
} // ~extern "C"
#endif
