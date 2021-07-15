// TCP client that operates TCP connection in two threads: one thread that does blocking recv(),
// and another that does send()s
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/websocket.h>
#include <emscripten/threading.h>

EMSCRIPTEN_WEBSOCKET_T bridgeSocket = 0;

extern "C" {
EMSCRIPTEN_WEBSOCKET_T emscripten_init_websocket_to_posix_socket_bridge(const char *bridgeUrl);
}
#endif

int lookup_host(const char *host)
{
  struct addrinfo hints, *res;
  int errcode;
  char addrstr[100];
  void *ptr;

  memset(&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo(host, NULL, &hints, &res);
  if (errcode != 0)
  {
    printf("getaddrinfo failed!\n");
    return -1;
  }

  printf("Host: %s\n", host);
  while (res)
  {
    inet_ntop(res->ai_family, res->ai_addr->sa_data, addrstr, 100);

    switch (res->ai_family)
    {
    case AF_INET:
      ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
      break;
    case AF_INET6:
      ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
      break;
    }
    inet_ntop(res->ai_family, ptr, addrstr, 100);
    printf("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
    res = res->ai_next;
  }

  return 0;
}

_Atomic uint32_t pendingMessages = 0;

void *send_thread(void *arg)
{
  int sock = (int)(intptr_t)arg;

  for(int i = 0; i < 10; ++i)
  {
    char message[] = "hella";
    message[4] += i;

    while(pendingMessages != 0)
      emscripten_thread_sleep(10);

    printf("Send()ing\n");
    if (send(sock, message, strlen(message), 0) < 0)
    {
      puts("Send failed");
      pthread_exit((void*)1);
    }
    printf("Send() done\n");
    pendingMessages++;
  }
  pthread_exit(0);
}

void *recv_thread(void *arg)
{
  int sock = (int)(intptr_t)arg;

  for(int i = 0; i < 10; ++i)
  {
    char server_reply[256] = {};
    printf("Recv()ing\n");
    if (recv(sock, server_reply, 256, 0) < 0)
    {
      puts("recv failed");
      pthread_exit((void*)1);
    }
    printf("Recv() done\n");

    puts("Server reply: ");
    puts(server_reply);
    pendingMessages--;
  }
  pthread_exit(0);
}

int main(int argc , char *argv[])
{
#ifdef __EMSCRIPTEN__
  bridgeSocket = emscripten_init_websocket_to_posix_socket_bridge("ws://localhost:8080");
  // Synchronously wait until connection has been established.
  uint16_t readyState = 0;
  do {
    emscripten_websocket_get_ready_state(bridgeSocket, &readyState);
    emscripten_thread_sleep(100);
  } while(readyState == 0);
#endif

  lookup_host("google.com");

  // Create socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    printf("Could not create socket");
    exit(1);
  }
  printf("Socket created: %d\n", sock);

  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8888);

  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("connect failed. Error");
    return 1;
  }

  pthread_t sendThread;
  pthread_t recvThread;
  pthread_create(&recvThread, 0, &recv_thread, (void*)sock);
  emscripten_thread_sleep(1000); // To get a better guarantee that the first recv() precedes the first send(), sleep for a moment between recv+send thread starts
  pthread_create(&sendThread, 0, &send_thread, (void*)sock);
  void *sendRet;
  void *recvRet;
  printf("Waiting for send thread to be finished\n");
  pthread_join(sendThread, &sendRet);
  printf("Send thread finished, waiting for recv thread to be finished\n");
  pthread_join(recvThread, &recvRet);
  printf("Send thread and recv thread finished\n");
  close(sock);
  if ((long)sendRet != 0) fprintf(stderr, "pthread send failed!\n");
  if ((long)recvRet != 0) fprintf(stderr, "pthread recv failed!\n");
  return 0;
}
