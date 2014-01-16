#include <stdio.h>
#include <math.h>

static float XXXf = -0.0f;
static double XXXd = -0.0;

struct x {
  float f;
  double d;
};

static struct x xx[] = {
  -0x0p+0,
  -0x0p+0,
};

int main(int argc, char ** argv) {
  float YYYf = -0.0f;
  float YYYd = -0.0;

  printf("%.2f\n", XXXf);
  printf("%.2f\n", XXXd);
  printf("%.2f\n", YYYf);
  printf("%.2f\n", YYYd);
  printf("%.2f\n", xx->f);
  printf("%.2f\n", xx->d);
}
