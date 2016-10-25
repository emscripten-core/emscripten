#include <stdio.h>

struct DATA {
  int value;

  DATA() { value = 0; }
};

DATA& GetData() {
  static DATA data;

  return data;
}

int main() {
  GetData().value = 10;
  printf("value:%i", GetData().value);
}
