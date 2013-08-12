#include <stdio.h>

#ifndef START
#define START 0
#endif

int pc;
int pc2;
int pc3;

int f(int k) {

  for (;;) {
    switch (pc) {

      case  0:
      case 10:
      case 20:
      case 30:
      case 40:
      case 50:
      case 60:
      case 70:
      case 80:
      case 90: pc = pc2; pc2 += 10; continue;

      case  1: pc =  1+k; continue;
      case  2: pc =  2+k; continue;
      case  3: pc =  3+k; continue;
      case  4: pc =  4+k; continue;

      case  5:
      case 15:
      case 25:
      case 35:
      case 45:
      case 55:
      case 65:
      case 75:
      case 85:
      case 95: pc = pc3; pc3 += 10; continue;

      case  6: pc =  6+k; continue;
      case  7: pc =  7+k; continue;
      case  8: pc =  8+k; continue;
      case  9: pc =  9+k; continue;

      case 11: pc = 11+k; continue;
      case 12: pc = 12+k; continue;
      case 13: pc = 13+k; continue;
      case 14: pc = 14+k; continue;

      case 16: pc = 16+k; continue;
      case 17: pc = 17+k; continue;
      case 18: pc = 18+k; continue;
      case 19: pc = 19+k; continue;

      case 21: pc = 21+k; continue;
      case 22: pc = 22+k; continue;
      case 23: pc = 23+k; continue;
      case 24: pc = 24+k; continue;

      case 26: pc = 26+k; continue;
      case 27: pc = 27+k; continue;
      case 28: pc = 28+k; continue;
      case 29: pc = 29+k; continue;

      case 31: pc = 31+k; continue;
      case 32: pc = 32+k; continue;
      case 33: pc = 33+k; continue;
      case 34: pc = 34+k; continue;

      case 36: pc = 36+k; continue;
      case 37: pc = 37+k; continue;
      case 38: pc = 38+k; continue;
      case 39: pc = 39+k; continue;

      case 41: pc = 41+k; continue;
      case 42: pc = 42+k; continue;
      case 43: pc = 43+k; continue;
      case 44: pc = 44+k; continue;

      case 46: pc = 46+k; continue;
      case 47: pc = 47+k; continue;
      case 48: pc = 48+k; continue;
      case 49: pc = 49+k; continue;

      case 51: pc = 51+k; continue;
      case 52: pc = 52+k; continue;
      case 53: pc = 53+k; continue;
      case 54: pc = 54+k; continue;

      case 56: pc = 56+k; continue;
      case 57: pc = 57+k; continue;
      case 58: pc = 58+k; continue;
      case 59: pc = 59+k; continue;

      case 61: pc = 61+k; continue;
      case 62: pc = 62+k; continue;
      case 63: pc = 63+k; continue;
      case 64: pc = 64+k; continue;

      case 66: pc = 66+k; continue;
      case 67: pc = 67+k; continue;
      case 68: pc = 68+k; continue;
      case 69: pc = 69+k; continue;

      case 71: pc = 71+k; continue;
      case 72: pc = 72+k; continue;
      case 73: pc = 73+k; continue;
      case 74: pc = 74+k; continue;

      case 76: pc = 76+k; continue;
      case 77: pc = 77+k; continue;
      case 78: pc = 78+k; continue;
      case 79: pc = 79+k; continue;

      case 81: pc = 81+k; continue;
      case 82: pc = 82+k; continue;
      case 83: pc = 83+k; continue;
      case 84: pc = 84+k; continue;

      case 86: pc = 86+k; continue;
      case 87: pc = 87+k; continue;
      case 88: pc = 88+k; continue;
      case 89: pc = 89+k; continue;

      case 91: pc = 91+k; continue;
      case 92: pc = 92+k; continue;
      case 93: pc = 93+k; continue;
      case 94: pc = 94+k; continue;

      case 96: pc = 96+k; continue;
      case 97: pc = 97+k; continue;
      case 98: pc = 98+k; continue;
      case 99: pc = 99+k; continue;
    }
    break;
  }

  return pc;
}

int main() {

  int n;
  int sum = 0;

  printf("start = %d\n", START);

  for (n=0; n<2000000; n++) {
    pc = START;
    pc2 = START+1;
    pc3 = START+6;
    sum += f(1);
  }

  printf("end (sum=%d)\n", sum);

  return 0;
}
