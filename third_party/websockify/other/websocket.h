#include <openssl/ssl.h>

#define BUFSIZE 65536
#define DBUFSIZE (BUFSIZE * 3) / 4 - 20

#define SERVER_HANDSHAKE_HIXIE "HTTP/1.1 101 Web Socket Protocol Handshake\r\n\
Upgrade: WebSocket\r\n\
Connection: Upgrade\r\n\
%sWebSocket-Origin: %s\r\n\
%sWebSocket-Location: %s://%s%s\r\n\
%sWebSocket-Protocol: %s\r\n\
\r\n%s"

#define SERVER_HANDSHAKE_HYBI "HTTP/1.1 101 Switching Protocols\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: %s\r\n\
Sec-WebSocket-Protocol: %s\r\n\
\r\n"

#define HYBI_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define HYBI10_ACCEPTHDRLEN 29

#define HIXIE_MD5_DIGEST_LENGTH 16

#define POLICY_RESPONSE "<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>\n"

typedef struct {
    char path[1024+1];
    char host[1024+1];
    char origin[1024+1];
    char version[1024+1];
    char connection[1024+1];
    char protocols[1024+1];
    char key1[1024+1];
    char key2[1024+1];
    char key3[8+1];
} headers_t;

typedef struct {
    int        sockfd;
    SSL_CTX   *ssl_ctx;
    SSL       *ssl;
    int        hixie;
    int        hybi;
    headers_t *headers;
    char      *cin_buf;
    char      *cout_buf;
    char      *tin_buf;
    char      *tout_buf;
} ws_ctx_t;

typedef struct {
    int verbose;
    char listen_host[256];
    int listen_port;
    void (*handler)(ws_ctx_t*);
    int handler_id;
    char *cert;
    char *key;
    int ssl_only;
    int daemon;
    int run_once;
} settings_t;


ssize_t ws_recv(ws_ctx_t *ctx, void *buf, size_t len);

ssize_t ws_send(ws_ctx_t *ctx, const void *buf, size_t len);

/* base64.c declarations */
//int b64_ntop(u_char const *src, size_t srclength, char *target, size_t targsize);
//int b64_pton(char const *src, u_char *target, size_t targsize);

#define gen_handler_msg(stream, ...) \
    if (! settings.daemon) { \
        fprintf(stream, "  %d: ", settings.handler_id); \
        fprintf(stream, __VA_ARGS__); \
    }

#define handler_msg(...) gen_handler_msg(stdout, __VA_ARGS__);
#define handler_emsg(...) gen_handler_msg(stderr, __VA_ARGS__);

