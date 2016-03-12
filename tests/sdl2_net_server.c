/*
 * Compile with:
 *
 * gcc -Wall `sdl-config --cflags` sdl2_net_server.c -o sdl2_net_server `sdl-config --libs` -lSDL_net
 *
 * or
 *
 * emcc -Wall sdl2_net_server.c -s USE_SDL_NET=2 -s USE_SDL=2 -o sdl2_net_server.js
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "SDL_net.h"

#define MAX_SOCKETS 128
#define MAX_CLIENTS MAX_SOCKETS - 1

typedef struct {
  TCPsocket clientSocket[MAX_CLIENTS];
  SDLNet_SocketSet socketSet;
  TCPsocket sd; /* Socket descriptor, Client socket descriptor */
} state_t;

state_t state;

void finish() { // untested
  printf("Shutting down...\n");
  SDLNet_TCP_Close(state.sd);
  for (int loop = 0; loop < MAX_CLIENTS; loop++)
  {
    if (state.clientSocket[loop] != NULL)
    {
      SDLNet_TCP_Close(state.clientSocket[loop]);
      state.clientSocket[loop] = NULL;
    }
  }
  SDLNet_Quit();

#ifdef __EMSCRIPTEN__
  emscripten_force_exit(0);
#else
  exit(0);
#endif
}

void main_loop() {
  char buffer[512];
  IPaddress *remoteIP;

  /* Check the sd if there is a pending connection.
   * If there is one, accept that, and open a new socket for communicating */
  SDLNet_CheckSockets(state.socketSet, 20);
  int serverSocketActivity = SDLNet_SocketReady(state.sd);

  if (serverSocketActivity)
  {
    printf("new server socket activity!\n");
    TCPsocket csd = SDLNet_TCP_Accept(state.sd);
    /* Now we can communicate with the client using csd socket
     * sd will remain opened waiting other connections */

    /* Get the remote address */
    if ((remoteIP = SDLNet_TCP_GetPeerAddress(csd)))
    {
      /* Print the address, converting in the host format */
      printf("Host connected: %x %d\n", SDLNet_Read32(&remoteIP->host), SDLNet_Read16(&remoteIP->port));
    } else {
      fprintf(stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError());
    }

    for (int loop = 0; loop < MAX_CLIENTS; loop++)
    {
        if (state.clientSocket[loop] == NULL)
        {
          state.clientSocket[loop] = csd;
          SDLNet_TCP_AddSocket(state.socketSet, state.clientSocket[loop]);
          break;
        }
    }
  }
  for (int clientNumber = 0; clientNumber < MAX_CLIENTS; clientNumber++)
  {
    int clientSocketActivity = SDLNet_SocketReady(state.clientSocket[clientNumber]);

    if (clientSocketActivity != 0)
    {
      int recvLen = SDLNet_TCP_Recv(state.clientSocket[clientNumber], buffer, 512);
      if (recvLen > 0)
      {
        assert(buffer[recvLen-1] == '\0');

        printf("Client %d says: %s\n", clientNumber, buffer);
        if (SDLNet_TCP_Send(state.clientSocket[clientNumber], buffer, recvLen) < recvLen) {
          printf("Failed to echo message %s\n", buffer);
        } else {
          printf("Echoed back %d bytes\n", recvLen);
        }

        if(strcmp(buffer, "exit") == 0) /* Terminate this connection */
        {
          printf("Terminate connection\n");
          SDLNet_TCP_Close(state.clientSocket[clientNumber]);
          state.clientSocket[clientNumber] = NULL;
        }
        if(strcmp(buffer, "quit") == 0) /* Quit the program */
        {
          printf("Quit program\n");
          finish();
        }
      } else {
        printf("Closing client socket\n");
        SDLNet_TCP_Close(state.clientSocket[clientNumber]);
        state.clientSocket[clientNumber] = NULL;
      }
    }
  }
}

int main(int argc, char **argv)
{
  IPaddress ip;

  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, INADDR_ANY, SOCKK) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

  state.socketSet = SDLNet_AllocSocketSet(MAX_SOCKETS);

  for (int loop = 0; loop < MAX_CLIENTS; loop++)
  {
      state.clientSocket[loop] = NULL;
  }
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(state.sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  SDLNet_TCP_AddSocket(state.socketSet, state.sd);

  /* Wait for a connection, send data and term */
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 60, 0);
#else
  while (1) main_loop();
#endif

  return EXIT_SUCCESS;
}

