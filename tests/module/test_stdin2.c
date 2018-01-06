#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
  char name[200], colour[200], answer[200];

  while (1) {
    printf("Enter your name: ");
    fflush(stdout);
    scanf("%s",name);
    printf("Hello %s.\n", name);
    printf("What is your favourite colour? ");
    fflush(stdout);
    scanf("%s",colour);
    printf("%s is a great colour!\n", colour);
    fprintf(stderr, "Do you want another go? ");
    fflush(stderr);
    scanf("%s", answer);
    if (!strcmp(answer, "yes") || !strcmp(answer, "Yes") || !strcmp(answer, "YES")) continue;
    break; 
  }
  fprintf(stderr, "\n");
  printf("Goodbye, %s.\n", name);
  exit(0);
}
