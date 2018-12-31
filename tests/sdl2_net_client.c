/*
 * Compile with:
 *
 * gcc -Wall `sdl-config --cflags` sdl2_net_client.c -o sdl2_net_client `sdl-config --libs` -lSDL_net
 *
 * or
 *
 * emcc -Wall sdl2_net_client.c -s USE_SDL_NET=2 -s USE_SDL=2 -o sdl2_net_client.js
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "SDL_net.h"

typedef enum {
  MSG_READ,
  MSG_WRITE
} msg_state_t;

typedef struct {
    TCPsocket sd; /* Socket descriptor */
    msg_state_t msg_state;
    int msg_i;
} state_t;

state_t state;

void finish(int result) {
  if (state.sd) {
    SDLNet_TCP_Close(state.sd);
    SDLNet_Quit();
  }
#ifdef __EMSCRIPTEN__
  REPORT_RESULT();
  emscripten_force_exit(result);
#else
  exit(result);
#endif
}

char *msgs[] = {
  "testmsg1",
  "anothertestmsg",
  "exit",
};

void main_loop()
{
  char *sendbuf = msgs[state.msg_i];
  char recvbuf[256] = {0};
  int actual = 0, len = strlen(sendbuf) + 1;
  printf("main loop with string %s and len %d\n", sendbuf, len);

  if (state.msg_state == MSG_WRITE) {
    printf("trying to send %s\n", sendbuf);
    if ((actual = SDLNet_TCP_Send(state.sd, (void *)sendbuf, len)) != len)
    {
      fprintf(stderr, "SDLNet_TCP_Send: count:%d/%d errno:%d msg:%s\n",
        actual, len, errno, SDLNet_GetError());
      if (errno == EAGAIN) {
        if (actual > 0) {
          assert(0);
        }
        return;
      }
      finish(EXIT_FAILURE);
    }
    printf("send success\n");
    state.msg_state = MSG_READ;
  }
  if (state.msg_state == MSG_READ) {
    printf("trying to receive %s\n", sendbuf);
    if ((actual = SDLNet_TCP_Recv(state.sd, (void *)recvbuf, len)) != len)
    {
      fprintf(stderr, "SDLNet_TCP_Recv: count:%d/%d errno:%d msg:%s\n",
        actual, len, errno, SDLNet_GetError());
      if (errno == EAGAIN) {
        if (actual > 0) {
          assert(0);
        }
        return;
      }
      finish(EXIT_FAILURE);
    }
    printf("receive success\n");
    assert(strcmp(sendbuf, recvbuf) == 0);
    if (!strcmp(recvbuf, "exit")) {
      finish(EXIT_SUCCESS);
    }
    state.msg_i++;
    state.msg_state = MSG_WRITE;
  }
}

int main(int argc, char **argv)
{
  IPaddress ip;   /* Server address */
  memset(&state, 0, sizeof(state_t));
  state.msg_state = MSG_WRITE;

  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    finish(EXIT_FAILURE);
  }

  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, "localhost", SOCKK) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    finish(EXIT_FAILURE);
  }

  /* Open a connection with the IP provided */
  if (!(state.sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    finish(EXIT_FAILURE);
  }

  /* Send messages */
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 60, 0);
#else
  while (1) main_loop();
#endif

  return EXIT_SUCCESS;
}
