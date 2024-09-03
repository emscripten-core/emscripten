#include <stdio.h>
#include <stdlib.h>
#include <emscripten/websocket.h>
#include <assert.h>

// This test performs that same server communications using two different
// sockets. This verifies that multiple sockets are supported simultaneously.
EMSCRIPTEN_WEBSOCKET_T sock1;
EMSCRIPTEN_WEBSOCKET_T sock2;

bool WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData) {
  printf("open(socket=%d, eventType=%d, userData=%p)\n", e->socket, eventType, userData);

  emscripten_websocket_send_utf8_text(e->socket, "hello on the other side");

  char data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  emscripten_websocket_send_binary(e->socket, data, sizeof(data));

  return 0;
}

bool WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData) {
  printf("close(socket=%d, eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%p)\n", e->socket, eventType, e->wasClean, e->code, e->reason, userData);
  assert(e->wasClean == 1);
  assert(e->code == 1005 /* No Status Rcvd */);
  emscripten_websocket_delete(e->socket);
  if (e->socket == sock1) {
    sock1 = 0;
  } else if (e->socket == sock2) {
    sock2 = 0;
  }
  // Once both sockets have been closed we are done.
  if (!sock1 && !sock2) {
    emscripten_force_exit(0);
  }
  return 0;
}

bool WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData) {
  printf("error(socket=%d, eventType=%d, userData=%p)\n", e->socket, eventType, userData);
  return 0;
}

bool WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData) {
  printf("message(socket=%d, eventType=%d, userData=%p data=%p, numBytes=%d, isText=%d)\n", e->socket, eventType, userData, e->data, e->numBytes, e->isText);
  static int text_received = 0;
  assert(e->socket == sock1 || e->socket == sock2);
  if (e->isText) {
    printf("text data: \"%s\"\n", e->data);
    assert(strcmp((const char*)e->data, "hello on the other side") == 0);
    text_received++;
    return 0;
  }

  // We expect to receive the text message before the binary one
  assert(text_received);
  printf("binary data:");
  for (int i = 0; i < e->numBytes; ++i) {
    printf(" %02X", e->data[i]);
    assert(e->data[i] == i);
  }
  printf("\n");
  emscripten_websocket_close(e->socket, 0, 0);
  return 0;
}

EMSCRIPTEN_WEBSOCKET_T create_socket() {
  EmscriptenWebSocketCreateAttributes attr;
  emscripten_websocket_init_create_attributes(&attr);

  const char *url = "ws://localhost:8089/";
  attr.url = url;
  // We don't really use a special protocol on the server backend in this test,
  // but check that it can be passed.
  attr.protocols = "binary,base64";

  EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
  assert(socket >= 0);

  // URL:
  int urlLength = 0;
  EMSCRIPTEN_RESULT res = emscripten_websocket_get_url_length(socket, &urlLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(urlLength == strlen(url)+1);

  char *url2 = malloc(urlLength);
  res = emscripten_websocket_get_url(socket, url2, urlLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  printf("url: %s, verified: %s, length: %d\n", url, url2, urlLength);
  assert(!strcmp(url, url2));

  // Protocol:
  int protocolLength = 0;
  res = emscripten_websocket_get_protocol_length(socket, &protocolLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(protocolLength == 1); // Null byte

  char *protocol = malloc(protocolLength);
  res = emscripten_websocket_get_protocol(socket, protocol, protocolLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  // We don't really use a special protocol on the server backend in this test,
  // but test that it comes out as an empty string at least.
  assert(!strcmp(protocol, ""));

  // Extensions:
  int extensionsLength = 0;
  res = emscripten_websocket_get_extensions_length(socket, &extensionsLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  assert(extensionsLength == 1); // Null byte

  char *extensions = malloc(extensionsLength);
  res = emscripten_websocket_get_extensions(socket, extensions, extensionsLength);
  assert(res == EMSCRIPTEN_RESULT_SUCCESS);
  // We don't really use any extensions on the server backend in this test, but
  // test that it comes out as an empty string at least.
  assert(!strcmp(extensions, ""));

  emscripten_websocket_set_onopen_callback(socket, (void*)0x42, WebSocketOpen);
  emscripten_websocket_set_onclose_callback(socket, (void*)0x43, WebSocketClose);
  emscripten_websocket_set_onerror_callback(socket, (void*)0x44, WebSocketError);
  emscripten_websocket_set_onmessage_callback(socket, (void*)0x45, WebSocketMessage);
  return socket;
}

int main() {
  assert(emscripten_websocket_is_supported());
  sock1 = create_socket();
  sock2 = create_socket();
  emscripten_exit_with_live_runtime();
  return 0;
}
