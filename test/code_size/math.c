#include <math.h>

int main(int argc, char **argv) {
  double num = (double)argc / (double)(long)argv[0];
  double num2 = (double)argc / (double)(long)argv[1];
  return pow(num, num2) + log(num);
}
