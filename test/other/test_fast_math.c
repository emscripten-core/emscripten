#include <math.h>
#include <stdio.h>

static double mandelbrot_iter(double cx, double cy, int max_iter) {
  double x = 0.0, y = 0.0;
  for (int i = 0; i < max_iter; i++) {
    if (x*x + y*y > 4.0) return (double)i;
    double tx = x*x - y*y + cx;
    y = 2.0*x*y + cy;
    x = tx;
  }
  return (double)max_iter;
}

static double newton_raphson(double x, int iterations) {
  for (int i = 0; i < iterations; i++) {
    double fx = x*x*x - x - 1.0;
    double fpx = 3.0*x*x - 1.0;
    if (fabs(fpx) < 1e-10) break;
    x = x - fx / fpx;
  }
  return x;
}

int main() {
  double result = 0.0;
  
  for (int i = 0; i < 100; i++) {
    double x = (i - 50) * 0.02;
    for (int j = 0; j < 100; j++) {
      double y = (j - 50) * 0.02;
      result += mandelbrot_iter(x, y, 50);
    }
  }
  
  for (int i = 0; i < 50; i++) {
    result += newton_raphson(1.5 + i * 0.1, 20);
  }
  
  for (int i = 0; i < 1000; i++) {
    double angle = i * 0.01;
    result += sin(angle) * cos(angle) + tan(angle);
  }
  
  printf("Result: %f\n", result);
  return 0;
}
