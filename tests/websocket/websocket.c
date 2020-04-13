#include <stdio.h>
#include <stdlib.h>
#include <emscripten/websocket.h>

EM_BOOL WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
{
	printf("open(eventType=%d, userData=%d)\n", eventType, (int)userData);

	emscripten_websocket_send_utf8_text(e->socket, "hello on the other side");

	char data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	emscripten_websocket_send_binary(e->socket, data, sizeof(data));

	emscripten_websocket_close(e->socket, 0, 0);
	return 0;
}

EM_BOOL WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
{
	printf("close(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%d)\n", eventType, e->wasClean, e->code, e->reason, (int)userData);
	return 0;
}

EM_BOOL WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
{
	printf("error(eventType=%d, userData=%d)\n", eventType, (int)userData);
	return 0;
}

EM_BOOL WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
{
	printf("message(eventType=%d, userData=%d, data=%p, numBytes=%d, isText=%d)\n", eventType, (int)userData, e->data, e->numBytes, e->isText);
	if (e->isText)
		printf("text data: \"%s\"\n", e->data);
	else
	{
		printf("binary data:");
		for(int i = 0; i < e->numBytes; ++i)
			printf(" %02X", e->data[i]);
		printf("\n");

		emscripten_websocket_delete(e->socket);
		exit(0);
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

	attr.url = "ws://localhost:8080";

	EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
	if (socket <= 0)
	{
		printf("WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
		exit(1);
	}

	emscripten_websocket_set_onopen_callback(socket, (void*)42, WebSocketOpen);
	emscripten_websocket_set_onclose_callback(socket, (void*)43, WebSocketClose);
	emscripten_websocket_set_onerror_callback(socket, (void*)44, WebSocketError);
	emscripten_websocket_set_onmessage_callback(socket, (void*)45, WebSocketMessage);
}
