#include <stdio.h>
char ch[5];
void write(int *address) { *address = 0x04030201; }
int main()
{
  write((int*)(ch+1));
  printf("%d %d %d %d %d\n", ch[0], ch[1], ch[2], ch[3], ch[4]);
}
