#include <stdio.h>
#include <emscripten.h>

#include <enet/enet.h>

ENetHost * host;

void main_loop() {
  static int counter = 0;
#if EMSCRIPTEN
  counter++;
#endif
  if (counter == 100) {
    printf("stop!\n");
    emscripten_cancel_main_loop();
    return;
  }

  ENetEvent event;
  if (enet_host_service (host, & event, 0) == 0) return;
  switch (event.type)
  {
    case ENET_EVENT_TYPE_CONNECT:
      printf ("Connection succeeded!\n");

      break;
    case ENET_EVENT_TYPE_RECEIVE:
      printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
              event.packet -> dataLength,
              event.packet -> data,
              event.peer -> data,
              event.channelID);

      int result = strcmp("packetfoo", event.packet->data);
      REPORT_RESULT();

      /* Clean up the packet now that we're done using it. */
      enet_packet_destroy (event.packet);
      break;
    case ENET_EVENT_TYPE_DISCONNECT:
      printf ("%s disconected.\n", event.peer -> data);
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

  printf("creating host\n");

  host = enet_host_create (NULL /* create a client host */,
                              1 /* only allow 1 outgoing connection */,
                              2 /* allow up 2 channels to be used, 0 and 1 */,
                              57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
                              14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
  if (host == NULL)
  {
    fprintf (stderr,
              "An error occurred while trying to create an ENet client host.\n");
    exit (EXIT_FAILURE);
  }

  ENetAddress address;
  enet_address_set_host (& address, "localhost");
#if EMSCRIPTEN
  address.port = 1237;
#else
  address.port = 1235;
#endif

  printf("connecting to server...\n");

  ENetPeer *peer = enet_host_connect (host, & address, 2, 0);

  if (peer == NULL)
  {
    fprintf (stderr,
    "No available peers for initiating an ENet connection.\n");
    exit (EXIT_FAILURE);
  }

#if EMSCRIPTEN
  emscripten_run_script("console.log('adding iframe');"
                        "var iframe = document.createElement('iframe');"
                        "iframe.src = 'server.html';"
                        "iframe.width = '100%';"
                        "iframe.height = '33%';"
                        "document.body.appendChild(iframe);"
                        "console.log('added.');");
#endif

  emscripten_set_main_loop(main_loop, 3, 1);

  return 1;
}

