#include <emscripten/websocket.h>
#include <assert.h>

int main() {
  EMSCRIPTEN_WEBSOCKET_T s =
    emscripten_websocket_new(&(EmscriptenWebSocketCreateAttributes){
      .url = "ws://localhost:9000",
      .protocols = "text",
    });

  assert(s > 0);

  return 0;
}
