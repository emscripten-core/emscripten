#include <stdio.h>
#include <stdlib.h>

FILE *file;
bool getline(char *str, int len) { return fgets(str, len, file)!=NULL; }

int main() {
  file = fopen("cube2md5.txt", "r");
  char buf[1024];
  int tmp;
  getline(buf, sizeof(buf));
  if(sscanf(buf, " frame %d", &tmp)==1)
  {
    printf("frame %d\n", tmp);
    for(int numdata = 0; getline(buf, sizeof(buf)) && buf[0]!='}';)
    {
      printf("frameline\n");
      for(char *src = buf, *next = src; numdata < 198; numdata++, src = next)
      {
        double x = strtod(src, &next);
        printf("animdata[%d] = %.8f\n", numdata, x);
        if(next <= src) break;
      }
    }
  }
  return 1;
}

