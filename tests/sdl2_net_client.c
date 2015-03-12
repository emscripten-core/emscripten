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
 
#include "SDL_net.h"
 
int main(int argc, char **argv)
{
  IPaddress ip;   /* Server address */
  TCPsocket sd;   /* Socket descriptor */
  int quit, len;
  char buffer[512];
 
  /* Simple parameter checking */
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s host port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 
  if (SDLNet_Init() < 0)
  {
    fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Resolve the host we are connecting to */
  if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
  {
    fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Open a connection with the IP provided (listen on the host's port) */
  if (!(sd = SDLNet_TCP_Open(&ip)))
  {
    fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
 
  /* Send messages */
  quit = 0;
  while (!quit)
  {
    printf("Write something:\n>");
    scanf("%s", buffer);
 
    len = strlen(buffer) + 1;
    if (SDLNet_TCP_Send(sd, (void *)buffer, len) < len)
    {
      fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
      exit(EXIT_FAILURE);
    }
 
    if(strcmp(buffer, "exit") == 0)
      quit = 1;
    if(strcmp(buffer, "quit") == 0)
      quit = 1;
  }
 
  SDLNet_TCP_Close(sd);
  SDLNet_Quit();
 
  return EXIT_SUCCESS;
}

