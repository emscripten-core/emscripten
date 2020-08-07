/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// g++ -fpermissive ../enet_server.c -I/home/alon/Dev/emscripten/system/include/emscripten/ -Iinclude/ -fpermissive .libs/libenet.a -o enet_server ; g++ ../enet_client.c -I/home/alon/Dev/emscripten/system/include/emscripten/ -Iinclude/ -fpermissive .libs/libenet.a -o enet_client

#include <stdio.h>
#include <string.h>
#include <enet/enet.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

ENetHost *host;

void send_msg(ENetPeer *peer) {
   /* Create a reliable packet of size 7 containing "packet\0" */
  ENetPacket * packet = enet_packet_create ("packet",
                                            strlen ("packet") + 1,
                                            ENET_PACKET_FLAG_RELIABLE);
  /* Extend the packet so and append the string "foo", so it now */
  /* contains "packetfoo\0" */
  enet_packet_resize (packet, strlen ("packetfoo") + 1);
  strcpy ((char*)& packet -> data [strlen ("packet")], "foo");
  /* Send the packet to the peer over channel id 0. */
  /* One could also broadcast the packet by */
  /* enet_host_broadcast (host, 0, packet); */
  enet_peer_send (peer, 0, packet);
  /* One could just use enet_host_service() instead. */
  enet_host_flush (host);
}

void main_loop() {
  static int counter = 0;
#ifdef __EMSCRIPTEN__
  counter++;
#endif
  if (counter == 100) {
    printf("stop!\n");
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
    return;
  }

  ENetEvent event;
//printf("enet host?\n");
  if (enet_host_service (host, & event, 0) == 0) return;
printf("enet host, got event of type %d\n", event.type);
  switch (event.type)
  {
    case ENET_EVENT_TYPE_CONNECT:
      printf ("A new client connected from %x:%u.\n",
              event.peer -> address.host,
              event.peer -> address.port);
      /* Store any relevant client information here. */
      event.peer -> data = "Client information";

      send_msg(event.peer);

      break;
    case ENET_EVENT_TYPE_RECEIVE:
      printf ("A packet of length %zu containing %s was received from %s on channel %u.\n",
              event.packet -> dataLength,
              (char*)event.packet -> data,
              (char*)event.peer -> data,
              event.channelID);
      /* Clean up the packet now that we're done using it. */
      enet_packet_destroy (event.packet);
      break;
    case ENET_EVENT_TYPE_DISCONNECT:
      printf ("%s disconected.\n", (char*)event.peer -> data);
      /* Reset the peer's client information. */
      event.peer -> data = NULL;
      enet_host_destroy(host);
      break;
    default:
      printf("whaaa? %d\n", event.type);
  }
}

int main (int argc, char ** argv)
{
  if (enet_initialize () != 0)
  {
    fprintf (stderr, "An error occurred while initializing ENet.\n");
    return EXIT_FAILURE;
  }
  atexit (enet_deinitialize);

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = SOCKK;
  printf("create!\n");
  host = enet_host_create (& address /* the address to bind the server host to */,
                             32 /* allow up to 32 clients and/or outgoing connections */,
                             2 /* allow up to 2 channels to be used, 0 and 1 */,
                             0 /* assume any amount of incoming bandwidth */,
                             0 /* assume any amount of outgoing bandwidth */);
  if (host == NULL)
  {
    fprintf (stderr,
    "An error occurred while trying to create an ENet server host.\n");
    exit (EXIT_FAILURE);
  }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 3, 1);
#else
  while (1) main_loop();
#endif

  return 1;
}

