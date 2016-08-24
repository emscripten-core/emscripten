#include <stdio.h>

static const double grid[4][2] = {{-3 / 3., -1 / 3.},
                                  {+1 / 3., -3 / 3.},
                                  {-1 / 3., +3 / 3.},
                                  {+3 / 3., +1 / 3.}};

int main() {
  for (int i = 0; i < 4; i++)
    printf("%d:%.2f,%.2f ", i, grid[i][0], grid[i][1]);
  printf("\n");
  return 0;
}
