#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <vector>
#include <sys/types.h>
#include <inttypes.h>

#include "posix_sockets.h"
#include "threads.h"
#include "sha1.h"
#include "websocket_to_posix_proxy.h"
#include "socket_registry.h"

// #define PROXY_DEBUG

// #define PROXY_DEEP_DEBUG

static const unsigned char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static void base64_encode(void *dst, const void *src, size_t len) { // thread-safe, re-entrant
  assert(dst != src);
  unsigned int *d = (unsigned int *)dst;
  const unsigned char *s = (const unsigned char*)src;
  const unsigned char *end = s + len;
  while (s < end) {
    uint32_t e = *s++ << 16;
    if (s < end) e |= *s++ << 8;
    if (s < end) e |= *s++;
    *d++ = b64[e >> 18] | (b64[(e >> 12) & 0x3F] << 8) | (b64[(e >> 6) & 0x3F] << 16) | (b64[e & 0x3F] << 24);
  }
  for (size_t i = 0; i < (3 - (len % 3)) % 3; i++) ((char *)d)[-1-i] = '=';
}

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

// Given a multiline string of HTTP headers, returns a pointer to the beginning
// of the value of given header inside the string that was passed in.
static int GetHttpHeader(const char *headers, const char *header, char *out, int maxBytesOut) { // thread-safe, re-entrant
  const char *pos = strstr(headers, header);
  if (!pos) return 0;
  pos += strlen(header);
  const char *end = pos;
  while (*end != '\r' && *end != '\n' && *end != '\0') ++end;
  int numBytesToWrite = MIN((int)(end-pos), maxBytesOut-1);
  memcpy(out, pos, numBytesToWrite);
  out[numBytesToWrite] = '\0';
  return (int)(end-pos);
}

// Sends WebSocket handshake back to the given WebSocket connection.
void SendHandshake(int fd, const char *request) {
  const char webSocketGlobalGuid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; // 36 characters long
  char key[128+sizeof(webSocketGlobalGuid)];
  GetHttpHeader(request, "Sec-WebSocket-Key: ", key, sizeof(key)/2);
  strcat(key, webSocketGlobalGuid);

  char sha1[21];
  printf("hashing key: \"%s\"\n", key);
  SHA1(sha1, key, (int)strlen(key));

  char handshakeMsg[] = 
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: 0000000000000000000000000000\r\n"
    "\r\n";

  base64_encode(strstr(handshakeMsg, "Sec-WebSocket-Accept: ") + strlen("Sec-WebSocket-Accept: "), sha1, 20);

  int err = send(fd, handshakeMsg, (int)strlen(handshakeMsg), 0);
  if (err < 0) on_error("Client write failed\n");
  printf("Sent handshake:\n%s\n", handshakeMsg);
}

// Validates if the given, possibly partially received WebSocket message has
// enough bytes to contain a full WebSocket header.
static bool WebSocketHasFullHeader(uint8_t *data, uint64_t obtainedNumBytes) {
  if (obtainedNumBytes < 2) return false;
  uint64_t expectedNumBytes = 2;
  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  if (header->mask) expectedNumBytes += 4;
  switch (header->payloadLength) {
    case 127: return expectedNumBytes += 8; break;
    case 126: return expectedNumBytes += 2; break;
    default: break;
  }
  return obtainedNumBytes >= expectedNumBytes;
}

// Computes the total number of bytes that the given WebSocket message will take
// up.
uint64_t WebSocketFullMessageSize(uint8_t *data, uint64_t obtainedNumBytes) {
  assert(WebSocketHasFullHeader(data, obtainedNumBytes));

  uint64_t expectedNumBytes = 2;
  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  if (header->mask) expectedNumBytes += 4;
  switch (header->payloadLength) {
    case 127: return expectedNumBytes += 8 + ntoh64(*(uint64_t*)(data+2)); break;
    case 126: return expectedNumBytes += 2 + ntohs(*(uint16_t*)(data+2)); break;
    default: expectedNumBytes += header->payloadLength; break;
  }
  return expectedNumBytes;
}

// Tests the structure integrity of the websocket message length.
bool WebSocketValidateMessageSize(uint8_t *data, uint64_t obtainedNumBytes) {
  uint64_t expectedNumBytes = WebSocketFullMessageSize(data, obtainedNumBytes);

  if (expectedNumBytes != obtainedNumBytes) {
    printf("Corrupt WebSocket message size! (got %" PRIu64 " bytes, expected %" PRIu64 " bytes)\n", obtainedNumBytes, expectedNumBytes);
    printf("Received data:");
    for (size_t i = 0; i < obtainedNumBytes; ++i)
      printf(" %02X", data[i]);
    printf("\n");
  }
  return expectedNumBytes == obtainedNumBytes;
}

uint64_t WebSocketMessagePayloadLength(uint8_t *data, uint64_t numBytes) {
  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  switch (header->payloadLength) {
    case 127: return ntoh64(*(uint64_t*)(data+2));
    case 126: return ntohs(*(uint16_t*)(data+2));
    default: return header->payloadLength;
  }
}

uint32_t WebSocketMessageMaskingKey(uint8_t *data, uint64_t numBytes) {
  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  if (!header->mask) return 0;
  switch (header->payloadLength) {
    case 127: return *(uint32_t*)(data+10);
    case 126: return *(uint32_t*)(data+4);
    default: return *(uint32_t*)(data+2);
  }
}

uint8_t *WebSocketMessageData(uint8_t *data, uint64_t numBytes) {
  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  data += 2; // Two bytes of fixed size header
  if (header->mask) data += 4; // If there is a masking key present in the header, that takes up 4 bytes
  switch (header->payloadLength) {
    case 127: return data + 8; // 64-bit length
    case 126: return data + 2; // 16-bit length
    default: return data; // 7-bit length that was embedded in fixed size header.
  }
}

void CloseWebSocket(int client_fd) {
  printf("Closing WebSocket connection %d\n", client_fd);
  CloseAllSocketsByConnection(client_fd);
  shutdown(client_fd, SHUTDOWN_BIDIRECTIONAL);
  CLOSE_SOCKET(client_fd);
}

const char *WebSocketOpcodeToString(int opcode) {
  static const char *opcodes[] = {
    "continuation frame (0x0)",
    "text frame (0x1)",
    "binary frame (0x2)",
    "reserved(0x3)",
    "reserved(0x4)",
    "reserved(0x5)",
    "reserved(0x6)",
    "reserved(0x7)",
    "connection close (0x8)",
    "ping (0x9)",
    "pong (0xA)",
    "reserved(0xB)",
    "reserved(0xC)",
    "reserved(0xD)",
    "reserved(0xE)",
    "reserved(0xF)" 
  };
  return opcodes[opcode];
}

void DumpWebSocketMessage(uint8_t *data, uint64_t numBytes) {
  bool goodMessageSize = WebSocketValidateMessageSize(data, numBytes);
  if (!goodMessageSize)
    return;

  WebSocketMessageHeader *header = (WebSocketMessageHeader *)data;
  uint64_t payloadLength = WebSocketMessagePayloadLength(data, numBytes);
  uint8_t *payload = WebSocketMessageData(data, numBytes);

  printf("Received: FIN: %d, opcode: %s, mask: 0x%08X, payload length: %" PRIu64 " bytes, unmasked payload:", header->fin, WebSocketOpcodeToString(header->opcode),
    WebSocketMessageMaskingKey(data, numBytes), payloadLength);
  for (uint64_t i = 0; i < payloadLength; ++i) {
    if (i%16 == 0) printf("\n");
    if (i%8==0) printf(" ");
    printf(" %02X", payload[i]);
    if (i >= 63 && payloadLength > 64) {
      printf("\n   ... (%" PRIu64 " more bytes)", payloadLength-i);
      break;
    }
  }
  printf("\n");
}

// connection thread manages a single active proxy connection.
THREAD_RETURN_T connection_thread(void *arg) {
  int client_fd = (int)(uintptr_t)arg;
  // TODO: print out getpeername()+getsockname() for more info
  printf("Established new proxy connection handler thread for incoming connection, at fd=%d\n", client_fd);

  // Waiting for connection upgrade handshake
  char buf[BUFFER_SIZE];
  int read = recv(client_fd, buf, BUFFER_SIZE, 0);

  if (!read) {
    CloseWebSocket(client_fd);
    EXIT_THREAD(0);
  }

  if (read < 0) {
    fprintf(stderr, "Client read failed\n");
    CloseWebSocket(client_fd);
    EXIT_THREAD(0);
  }

#ifdef PROXY_DEEP_DEBUG
  printf("Received:");
  for (int i = 0; i < read; ++i) {
    printf(" %02X", buf[i]);
  }
  printf("\n");
  //printf("In text:\n%s\n", buf);
#endif
  SendHandshake(client_fd, buf);

#ifdef PROXY_DEEP_DEBUG
  printf("Handshake received, entering message loop:\n");
#endif

  std::vector<uint8_t> fragmentData;

  bool connectionAlive = true;
  while (connectionAlive) {
    int read = recv(client_fd, buf, BUFFER_SIZE, 0);

    if (!read) break; // done reading
    if (read < 0) {
      fprintf(stderr, "Client read failed\n");
      EXIT_THREAD(0);
    }

#ifdef PROXY_DEEP_DEBUG
    printf("Received:");
    for (int i = 0; i < read; ++i) {
      printf(" %02X", ((unsigned char*)buf)[i]);
    }
    printf("\n");
//    printf("In text:\n%s\n", buf);
#endif

#ifdef PROXY_DEEP_DEBUG
    printf("Have %d+%d==%d bytes now in queue\n", (int)fragmentData.size(), (int)read, (int)(fragmentData.size()+read));
#endif
    fragmentData.insert(fragmentData.end(), buf, buf+read);

    // Process received fragments until there is not enough data for a full message
    while (!fragmentData.empty()) {
      bool hasFullHeader = WebSocketHasFullHeader(&fragmentData[0], fragmentData.size());
      if (!hasFullHeader) {
#ifdef PROXY_DEEP_DEBUG
        printf("(not enough for a full WebSocket header)\n");
#endif
        break;
      }
      uint64_t neededBytes = WebSocketFullMessageSize(&fragmentData[0], fragmentData.size());
      if (fragmentData.size() < neededBytes) {
#ifdef PROXY_DEEP_DEBUG
        printf("(not enough for a full WebSocket message, needed %d bytes)\n", (int)neededBytes);
#endif
        break;
      }

      WebSocketMessageHeader *header = (WebSocketMessageHeader *)&fragmentData[0];
      uint64_t payloadLength = WebSocketMessagePayloadLength(&fragmentData[0], neededBytes);
      uint8_t *payload = WebSocketMessageData(&fragmentData[0], neededBytes);

      // Unmask payload
      if (header->mask)
        WebSocketMessageUnmaskPayload(payload, payloadLength, WebSocketMessageMaskingKey(&fragmentData[0], neededBytes));

#ifdef PROXY_DEEP_DEBUG
        DumpWebSocketMessage(&fragmentData[0], neededBytes);
#endif

      switch (header->opcode) {
      case 0x02: /*binary message*/ ProcessWebSocketMessage(client_fd, payload, payloadLength); break;
      case 0x08: connectionAlive = false; break;
      default:
        fprintf(stderr, "Unknown WebSocket opcode received %x!\n", header->opcode);
        connectionAlive = false; // Kill connection
        break;
      }

      fragmentData.erase(fragmentData.begin(), fragmentData.begin() + (ptrdiff_t)neededBytes);
#ifdef PROXY_DEEP_DEBUG
      printf("Cleared used bytes, got %d left in fragment queue.\n", (int)fragmentData.size());
#endif
    }
  }
  printf("Proxy connection closed\n");
  CloseWebSocket(client_fd);
  EXIT_THREAD(0);
}

// Technically only would need one lock per connection, but this is now one lock
// per all connections, which would be slightly inefficient if we were handling
// multiple proxied connections at the same time. (currently that is a rare use
// case, expected to only be proxying one connection at a time - if this proxy
// bridge is expected to be used for hundreds of connections simultaneously,
// this mutex should be refactored to be per-connection)
static MUTEX_T webSocketSendLock;

extern "C" void lock_websocket_send_lock() {
  LOCK_MUTEX(&webSocketSendLock);
}

extern "C" void unlock_websocket_send_lock() {
  UNLOCK_MUTEX(&webSocketSendLock);
}

int main(int argc, char *argv[]) {
  if (argc < 2) on_error("websocket_to_posix_proxy creates a bridge that allows WebSocket connections on a web page to proxy out to perform TCP/UDP connections.\nUsage: %s [port]\n", argv[0]);

#ifdef _WIN32
  WSADATA wsaData;
  int failed = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (failed) {
    printf("WSAStartup failed: %d\n", failed);
    return 1;
  }
#else
  signal(SIGPIPE, SIG_IGN);
#endif

  const int port = atoi(argv[1]);
  SOCKET_T server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) on_error("Could not create socket\n");

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (SETSOCKOPT_PTR_TYPE)&opt_val, sizeof opt_val);

  int err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0) on_error("Could not bind socket\n");

  err = listen(server_fd, 128);
  if (err < 0) on_error("Could not listen on socket\n");

  printf("websocket_to_posix_proxy server is now listening for WebSocket connections to ws://localhost:%d/\n", port);

  CREATE_MUTEX(&webSocketSendLock);
  InitWebSocketRegistry();

  while (1) {
    SOCKET_T client_fd = accept(server_fd, 0, 0);
    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new incoming proxy connection\n");
      continue; // Do not quit here, but keep serving any existing proxy connections.
    }

    THREAD_T connection;
    CREATE_THREAD_RETURN_T ret = CREATE_THREAD(connection, connection_thread, (void*)(uintptr_t)client_fd);
    if (!CREATE_THREAD_SUCCEEDED(ret)) {
      fprintf(stderr, "Failed to create a connection handler thread for incoming proxy connection!\n");
      continue; // Do not quit here, but keep program alive to manage other existing proxy connections.
    }
  }

#ifdef _WIN32
  WSACleanup();
#endif

  return 0;
}
