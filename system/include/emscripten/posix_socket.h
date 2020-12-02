#pragma once

#include "websocket.h"

#ifdef __cplusplus
extern "C" {
#endif

EMSCRIPTEN_RESULT emscripten_init_websocket_to_posix_socket_bridge(const char *bridgeUrl);

#ifdef __cplusplus
}
#endif
