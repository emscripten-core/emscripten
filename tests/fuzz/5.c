#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  printf("argc %d\n", argc);
  char buffer[100];
  for (int i = 0; i < 100; i++) buffer[i] = argc*(argc > 10 ? (argc*i) % 3 : (i*i));
  memset(&buffer[10], -114, argc+25);
  for(int i = 0; i < 100; i++) printf("%d:%d\n", i, buffer[i]);
  //memset 5243040,-114,6,false,6
  return buffer[15];
}

