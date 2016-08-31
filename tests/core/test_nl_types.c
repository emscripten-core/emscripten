#include <nl_types.h>
#include <stdio.h>

int main(int argc, char ** argv) {
 nl_catd c = catopen("none", 0);
 printf("Hello, %s.\n", catgets(c, 0, 0, "world"));
 return catclose(c);
}
