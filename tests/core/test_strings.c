#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int x = 5, y = 9, magic = 7;  // fool compiler with magic
  memmove(&x, &y, magic - 7);   // 0 should not crash us

  int xx, yy, zz;
  char s[32];
  int cc = sscanf("abc_10.b1_xyz9_543_defg", "abc_%d.%2x_xyz9_%3d_%3s", &xx,
                  &yy, &zz, s);
  printf("%d:%d,%d,%d,%s\n", cc, xx, yy, zz, s);

  printf("%d\n", argc);
  puts(argv[1]);
  puts(argv[2]);
  printf("%d\n", atoi(argv[3]) + 2);
  const char *foolingthecompiler = "\rabcd";
  printf("%d\n", strlen(foolingthecompiler));  // Tests parsing /0D in llvm -
                                               // should not be a 0 (end string)
                                               // then a D!
  printf("%s\n", NULL);  // Should print '(null)', not the string at address 0,
                         // which is a real address for us!
  printf("/* a comment */\n");  // Should not break the generated code!
  printf("// another\n");       // Should not break the generated code!

  char *strdup_val = strdup("test");
  printf("%s\n", strdup_val);
  free(strdup_val);

  {
    char *one = "one 1 ONE !";
    char *two = "two 2 TWO ?";
    char three[1024];
    memset(three, '.', 1024);
    three[50] = 0;
    strncpy(three + argc, one + (argc / 2), argc + 1);
    strncpy(three + argc * 3, two + (argc / 3), argc + 2);
    printf("waka %s\n", three);
  }

  {
    char *one = "string number one top notch";
    char *two = "fa la sa ho fi FI FO FUM WHEN WHERE WHY HOW WHO";
    char three[1000];
    strcpy(three, &one[argc * 2]);
    char *four = strcat(three, &two[argc * 3]);
    printf("cat |%s|\n", three);
    printf("returned |%s|\n", four);
  }

  return 0;
}
