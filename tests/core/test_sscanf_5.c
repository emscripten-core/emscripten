#include "stdio.h"

static const char *colors[] = {"  c black", ". c #001100", "X c #111100"};

int main() {
  unsigned char code;
  char color[32];
  int rcode;
  for (int i = 0; i < 3; i++) {
    rcode = sscanf(colors[i], "%c c %s", &code, color);
    printf("%i, %c, %s\n", rcode, code, color);
  }
}
