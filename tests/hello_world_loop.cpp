#include<stdio.h>
#include<string.h>
#include<stdlib.h>

extern "C" {
  void dump(char *s) {
    printf("%s\n", s);
  }
}

int main() {
  char *original = (char*)"h e l l o ,   w o r l d ! ";
  char copy[strlen(original)];
  for (int i = 0; i < strlen(original); i += 2) {
    copy[i/2] = original[i];
  }
  copy[strlen(copy)+1] = (int)&original; // force original to be on the stack
  dump(copy);
  return 0;
}

