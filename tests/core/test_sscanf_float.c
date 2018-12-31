#include "stdio.h"

int main() {
  float f1, f2, f3, f4, f5, f6, f7, f8, f9;
  sscanf("0.512 0.250x5.129_-9.98 1.12*+54.32E3 +54.32E3^87.5E-3 87.5E-3$",
         "%f %fx%f_%f %f*%f %f^%f %f$", &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8,
         &f9);
  printf("\n%f, %f, %f, %f, %f, %f, %f, %f, %f\n", f1, f2, f3, f4, f5, f6, f7,
         f8, f9);
}
