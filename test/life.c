// From http://rosettacode.org/wiki/Conway%27s_Game_of_Life#C

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#define for_x for (int x = 0; x < w; x++)
#define for_y for (int y = 0; y < h; y++)
#define for_xy for_x for_y

void show(void *u, int w, int h)
{
  int (*univ)[w] = u;
  for_x printf("-"); printf("\n");
  for_y {
    for_x printf(univ[y][x] ? "[]" : "  ");
    printf("\n");
  }
  for_x printf("-"); printf("\n");
  fflush(stdout);
}
 
void evolve(void *u, int w, int h)
{
  unsigned (*univ)[w] = u;
  unsigned new[h][w];
 
  for_y for_x {
    int n = 0;
    for (int y1 = y - 1; y1 <= y + 1; y1++)
      for (int x1 = x - 1; x1 <= x + 1; x1++)
        if (univ[(y1 + h) % h][(x1 + w) % w])
          n++;
 
    if (univ[y][x]) n--;
    new[y][x] = (n == 3 || (n == 2 && univ[y][x]));
  }
  for_y for_x univ[y][x] = new[y][x];
}

void nudge(void *u, int w, int h)
{
  unsigned (*univ)[w] = u;
  int sum = 0;
  for_xy sum += univ[y][x];
  while (sum < (w*h)/8) {
    int x = sum & (w-1);
    int y = (sum*sum) & (h-1);
    univ[y][x] = 1;
    sum++;
  }
}

void game(int w, int h, int i)
{
  unsigned univ[h][w];
  //for_xy univ[y][x] = rand() < RAND_MAX / 10 ? 1 : 0;
  int acc = 0; // nonrandom generation, for benchmarking
  for_xy {
    acc += (x*17) % (y*3 + 1);
    univ[y][x] = acc & 1;
  }
  while (i != 0) {
    //show(univ, w, h);
    evolve(univ, w, h);
    if (i > 0) {
      i--;
      nudge(univ, w, h); // keep it interesting for benchmark
    } else {
#if !__EMSCRIPTEN__
      usleep(20000);
#endif
      show(univ, w, h);
    }
  }
  show(univ, w, h);
}
 
int main(int argc, char **argv)
{
  int w, h, i;
  int arg = argc > 1 ? argv[1][0] - '0' : 3;
  switch(arg) {
    case 0: return 0; break;
    case 1: w = h = 32; i = 2500; break;
    case 2: w = h = 32; i = 13000; break;
    case 3: w = h = 32; i = 24000; break;
    case 4: w = h = 32; i = 5*24000; break;
    case 5: w = h = 32; i = 10*24000; break;
    default: printf("error: %d\\n", arg); return -1;
  }

  printf("life: %d,%d,%d,%d\n", arg, w, h, i);
  game(w, h, i);

  return 0;
}

