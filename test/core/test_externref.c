#include <stdio.h>

void log_externref();
void get_externref();

int main() {
  printf("in main\n");
  log_externref();
  get_externref();
  printf("extenref stored\n");
  log_externref();
  printf("extenref logged\n");
  return 0;
}

