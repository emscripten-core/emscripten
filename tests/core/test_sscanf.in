#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
#define CHECK(str)                     \
  {                                    \
    char name[1000];                   \
    memset(name, 0, 1000);             \
    int prio = 99;                     \
    sscanf(str, "%s %d", name, &prio); \
    printf("%s : %d\n", name, prio);   \
  }
  CHECK("en-us 2");
  CHECK("en-r");
  CHECK("en 3");

  printf("%f, %f\n", atof("1.234567"), atof("cheez"));

  char float_formats[] = "fegE";
  char format[] = "%_";
  for (int i = 0; i < 4; ++i) {
    format[1] = float_formats[i];

    float n = -1;
    sscanf(" 2.8208", format, &n);
    printf("%.4f\n", n);

    float a = -1;
    sscanf("-3.03", format, &a);
    printf("%.4f\n", a);
  }

  char buffy[100];
  sscanf("cheez some thing moar 123\nyet more\n", "cheez %s", buffy);
  printf("|%s|\n", buffy);
  sscanf("cheez something\nmoar 123\nyet more\n", "cheez %s", buffy);
  printf("|%s|\n", buffy);
  sscanf("cheez somethingmoar\tyet more\n", "cheez %s", buffy);
  printf("|%s|\n", buffy);

  int numverts = -1;
  printf("%d\n",
         sscanf("	numverts 1499\n", " numverts %d",
                &numverts));  // white space is the same, even if tab vs space
  printf("%d\n", numverts);

  int index;
  float u, v;
  short start, count;
  printf("%d\n",
         sscanf("	vert 87 ( 0.481565 0.059481 ) 0 1\n",
                " vert %d ( %f %f ) %hu %hu", &index, &u, &v, &start, &count));
  printf("%d,%.6f,%.6f,%hu,%hu\n", index, u, v, start, count);

  int neg, neg2, neg3 = 0;
  printf("%d\n", sscanf("-123 -765 -34-6", "%d %u %d", &neg, &neg2, &neg3));
  printf("%d,%u,%d\n", neg, neg2, neg3);

  {
    int a = 0;
    sscanf("1", "%i", &a);
    printf("%i\n", a);
  }

  char buf1[100], buf2[100], buf3[100], buf4[100];
  memset(buf4, 0, 100);


  int numItems = sscanf("level=4:ref=3", "%255[^:=]=%255[^:]:%255[^=]=%c",
                        buf1, buf2, buf3, buf4);
  printf("%d, %s, %s, %s, %s\n", numItems, buf1, buf2, buf3, buf4);

  numItems = sscanf("def|456", "%[a-z]|%[0-9]", buf1, buf2);
  printf("%d, %s, %s\n", numItems, buf1, buf2);

  numItems = sscanf("3-4,-ab", "%[-0-9],%[ab-z-]", buf1, buf2);
  printf("%d, %s, %s\n", numItems, buf1, buf2);

  numItems = sscanf("Hello,World", "%[A-Za-z],%[^0-9]", buf1, buf2);
  printf("%d, %s, %s\n", numItems, buf1, buf2);

  numItems = sscanf("Hello4711", "%[^0-9],%[^0-9]", buf1, buf2);
  printf("%d, %s\n", numItems, buf1);

  numItems = sscanf("JavaScript", "%4[A-Za-z]", buf1);
  printf("%d, %s\n", numItems, buf1);

  numItems = sscanf("[]", "%1[[]%1[]]", buf1, buf2);
  printf("%d, %s, %s\n", numItems, buf1, buf2);

  return 0;
}
