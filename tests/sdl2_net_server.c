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
#include <string.h>
 
#include "SDL_net.h"

#define MAX_SOCKETS 128
#define MAX_CLIENTS MAX_SOCKETS - 1
 
int main(int argc, char **argv)
{
  TCPsocket sd; /* Socket descriptor, Client socket descriptor */
  IPaddress ip, *remoteIP;
  int quit;
  char buffer[512];
  TCPsocket clientSocket[MAX_CLIENTS];
 
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolving the host using NULL make network interface to listen */
  if (SDLNet_ResolveHost(&ip, NULL, 2000) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_SOCKETS);

  for (int loop = 0; loop < MAX_CLIENTS; loop++)
  {
      clientSocket[loop] = NULL;
  }

  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  SDLNet_TCP_AddSocket(socketSet, sd);
 
  /* Wait for a connection, send data and term */
  quit = 0;
  while (!quit)
  {
    /* Check the sd if there is a pending connection.
     * If there is one, accept that, and open a new socket for communicating */
    SDLNet_CheckSockets(socketSet, 20);
    int serverSocketActivity = SDLNet_SocketReady(sd);

    if (serverSocketActivity)
    {
      printf("new server socket activity!\n");
      TCPsocket csd = SDLNet_TCP_Accept(sd);
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
          if (clientSocket[loop] == NULL)
          {
            clientSocket[loop] = csd;
            SDLNet_TCP_AddSocket(socketSet, clientSocket[loop]);
            break;
          }
      }
    }
    for (int clientNumber = 0; clientNumber < MAX_CLIENTS; clientNumber++)
    {
      int clientSocketActivity = SDLNet_SocketReady(clientSocket[clientNumber]);

      if (clientSocketActivity != 0)
      {
        int recvLen = SDLNet_TCP_Recv(clientSocket[clientNumber], buffer, 512); 
        if (recvLen > 0)
        {
          int trim = 0;
          for (int i = recvLen; i > 0; i++) {
            if (buffer[i] == '\r' || buffer[i] == '\n' || buffer[i] == ' ') {
              trim++;
            } else {
              break;
            }
          }
          buffer[recvLen - trim] = 0;
          printf("Client %d says: %s\n", clientNumber, buffer);
 
          if(strcmp(buffer, "exit") == 0) /* Terminate this connection */
          {
            printf("Terminate connection\n");
            SDLNet_TCP_Close(clientSocket[clientNumber]);
            clientSocket[clientNumber] = NULL;
          }
          if(strcmp(buffer, "quit") == 0) /* Quit the program */
          {
            quit = 1;
            printf("Quit program\n");
          }
        } else {
          SDLNet_TCP_Close(clientSocket[clientNumber]);
          clientSocket[clientNumber] = NULL;
        }
      } 
    }
  }
 
  printf("Shutting down...\n");
  SDLNet_TCP_Close(sd);
  for (int loop = 0; loop < MAX_CLIENTS; loop++)
  {
    if (clientSocket[loop] != NULL) 
    {
      SDLNet_TCP_Close(clientSocket[loop]);
      clientSocket[loop] = NULL;
    }
  }
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}

