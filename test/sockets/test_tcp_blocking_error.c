/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * A blocking recv() on a connected TCP socket is woken by a connection error
 * that lands after it has blocked, and returns that error. Node drains the
 * kernel buffer eagerly so a real peer cannot reliably produce an RST here; the
 * error is injected on the socket object from the JS side instead (on the
 * main thread, where the socket lives: from a second thread under
 * PROXY_TO_PTHREAD, or a timer under JSPI).
 */

#include <arpa/inet.h>
#include <assert.h>
#include <emscripten.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif

int listen_fd = -1, client_fd = -1, peer_fd = -1;
struct sockaddr_in addr;

void inject_error(void* arg) {
  MAIN_THREAD_EM_ASM({
    var sock = SOCKFS.getSocket($0);
    sock.error = $1;
    SOCKFS.emit('error', [$0, $1, 'injected']);
  }, peer_fd, ECONNRESET);
}

#ifdef __EMSCRIPTEN_PTHREADS__
void* delayed(void* arg) {
  usleep(100000); // let recv() block first
  inject_error(NULL);
  return NULL;
}
#endif

int main(void) {
  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd >= 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  assert(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  assert(listen(listen_fd, 4) == 0);
  socklen_t l = sizeof(addr);
  assert(getsockname(listen_fd, (struct sockaddr*)&addr, &l) == 0);

  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(client_fd >= 0);
  assert(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0);
  peer_fd = accept(listen_fd, NULL, NULL); // blocks until the connection lands
  assert(peer_fd >= 0);

#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_t t;
  assert(pthread_create(&t, NULL, delayed, NULL) == 0);
#else
  emscripten_async_call(inject_error, NULL, 100);
#endif

  char buf[8];
  assert(recv(peer_fd, buf, sizeof(buf), 0) == -1); // blocks; only the error can wake it
  assert(errno == ECONNRESET);
  // The error was consumed: SO_ERROR is clear, and the socket is back to
  // would-block (still readable-wait, no data).
  int err = -1;
  socklen_t el = sizeof(err);
  assert(getsockopt(peer_fd, SOL_SOCKET, SO_ERROR, &err, &el) == 0 && err == 0);
  assert(recv(peer_fd, buf, sizeof(buf), MSG_DONTWAIT) == -1 && errno == EAGAIN);

#ifdef __EMSCRIPTEN_PTHREADS__
  assert(pthread_join(t, NULL) == 0);
#endif
  close(peer_fd);
  close(client_fd);
  close(listen_fd);
  printf("done\n");
  return 0;
}
