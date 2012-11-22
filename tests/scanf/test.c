#include <stdio.h>
#include <math.h>

int main() {
  {
  	int r, g, b, a, count;
    count = sscanf("#f7f7f7aa", "#%2x%2x%2x%2x", &r, &g, &b, &a);
    printf("%i %d %d %d %d\n", count, r, g, b, a);
  }
  {
  	int r, g, b, a, count;
  	count = sscanf("#f7 f7 f7 aa  ", "#%2x %2x %2x %2x", &r, &g, &b, &a);
  	printf("%i %d %d %d %d\n", count, r, g, b, a);
  }
  {
  	int count;
  	char s1[16], s2[16]; 
  	count = sscanf("Test set", "%s %s", s1, s2);
  	printf("%i %s %s\n", count, s1, s2);
  }
  {
  	int a, b, c, count;
		count = sscanf("12345 6789", "%d %n%d", &a, &b, &c);
		printf("%i %i %i %i\n", count, a, b, c);
  }
  return 0;
}
