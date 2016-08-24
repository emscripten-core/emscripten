#include <stdio.h>

int main() {
  int x;
  int y;

  const char* buffer[] = {
      "\t2\t3\t",                       /* TAB - horizontal tab */
      "\t\t5\t\t7\t\t",   "\n11\n13\n", /* LF - line feed */
      "\n\n17\n\n19\n\n", "\v23\v29\v", /* VT - vertical tab */
      "\v\v31\v\v37\v\v", "\f41\f43\f", /* FF - form feed */
      "\f\f47\f\f53\f\f", "\r59\r61\r", /* CR - carrage return */
      "\r\r67\r\r71\r\r"};

  for (int i = 0; i < 10; ++i) {
    x = 0;
    y = 0;
    sscanf(buffer[i], " %d %d ", &x, &y);
    printf("%d, %d, ", x, y);
  }

  return 0;
}
