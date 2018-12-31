#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  uint16_t length;
  struct {
    int32_t int32;
  } value[];
} Tuple;

int main() {
  Tuple T[10];
  Tuple *t = &T[0];

  t->length = 4;
  t->value->int32 = 100;

  printf("(%d, %d)\n", t->length, t->value->int32);
  return 0;
}
