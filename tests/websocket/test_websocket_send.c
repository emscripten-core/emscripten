#include <stdio.h>
#include <stdlib.h>
#include <emscripten/websocket.h>
#include <assert.h>

EM_BOOL WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
{
	printf("open(eventType=%d, userData=%ld)\n", eventType, (long)userData);

	emscripten_websocket_send_utf8_text(e->socket, "hello on the other side");

	char data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	emscripten_websocket_send_binary(e->socket, data, sizeof(data));

	return 0;
}

EM_BOOL WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
{
	printf("close(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%ld)\n", eventType, e->wasClean, e->code, e->reason, (long)userData);
	return 0;
}

EM_BOOL WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
{
	printf("error(eventType=%d, userData=%ld)\n", eventType, (long)userData);
	return 0;
}

static int passed = 0;

EM_BOOL WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
{
	printf("message(eventType=%d, userData=%ld, data=%p, numBytes=%d, isText=%d)\n", eventType, (long)userData, e->data, e->numBytes, e->isText);
	if (e->isText)
	{
		printf("text data: \"%s\"\n", e->data);
#ifdef REPORT_RESULT
		if (!!strcmp((const char*)e->data, "hello on the other side")) REPORT_RESULT(-1);
		passed += 1;
#endif
	}
	else
	{
		printf("binary data:");
		for(int i = 0; i < e->numBytes; ++i)
		{
			printf(" %02X", e->data[i]);
#ifdef REPORT_RESULT
			if (e->data[i] != i) REPORT_RESULT(-2);
#endif
		}
		printf("\n");
		passed += 100;

		emscripten_websocket_close(e->socket, 0, 0);
		emscripten_websocket_delete(e->socket);
#ifdef REPORT_RESULT
		printf("%d\n", passed);
		REPORT_RESULT(passed);
#endif
	}
	return 0;
}

int main()
{
	if (!emscripten_websocket_is_supported())
	{
		printf("WebSockets are not supported, cannot continue!\n");
		exit(1);
	}

	EmscriptenWebSocketCreateAttributes attr;
	emscripten_websocket_init_create_attributes(&attr);

	const char *url = "ws://localhost:8088/";
	attr.url = url;
	attr.protocols = "binary,base64"; // We don't really use a special protocol on the server backend in this test, but check that it can be passed.

	EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
	if (socket <= 0)
	{
		printf("WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
		exit(1);
	}

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
	assert(!strcmp(protocol, "")); // We don't really use a special protocol on the server backend in this test, but test that it comes out as an empty string at least.

	// Extensions:
	int extensionsLength = 0;
	res = emscripten_websocket_get_extensions_length(socket, &extensionsLength);
	assert(res == EMSCRIPTEN_RESULT_SUCCESS);
	assert(extensionsLength == 1); // Null byte

	char *extensions = malloc(extensionsLength);
	res = emscripten_websocket_get_extensions(socket, extensions, extensionsLength);
	assert(res == EMSCRIPTEN_RESULT_SUCCESS);
	assert(!strcmp(extensions, "")); // We don't really use any extensions on the server backend in this test, but test that it comes out as an empty string at least.

	emscripten_websocket_set_onopen_callback(socket, (void*)42, WebSocketOpen);
	emscripten_websocket_set_onclose_callback(socket, (void*)43, WebSocketClose);
	emscripten_websocket_set_onerror_callback(socket, (void*)44, WebSocketError);
	emscripten_websocket_set_onmessage_callback(socket, (void*)45, WebSocketMessage);
}
