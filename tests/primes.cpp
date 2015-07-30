#include<stdio.h>
#include<math.h>
int main(int argc, char **argv) {
  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: arg = 33000; break;
    case 2: arg = 130000; break;
    case 3: arg = 220000; break;
    case 4: arg = 610000; break;
    case 5: arg = 1010000; break;
    default: printf("error: %d\\n", arg); return -1;
  }

  int primes = 0, curri = 2;
  while (primes < arg) {
    int ok = true;
    for (int j = 2; j < sqrtf(curri); j++) {
      if (curri % j == 0) {
        ok = false;
        break;
      }
    }
    if (ok) {
      primes++;
    }
    curri++;
  }
  printf("lastprime: %d.\n", curri-1);
  return 0;
}

