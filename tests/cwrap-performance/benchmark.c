#include "add.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

int main(void) {
	char *dest, str1[] = "hello", str2[] = "world";
	int i;
	clock_t t = clock();
	for (i=0; i<10000; i++) {
		dest = concat(str1,str2);
	}
	printf("Executed concat 10000 times in %f ms\n", (float)((clock()-t)*1000)/CLOCKS_PER_SEC);
	printf("concat(\"%s\", \"%s\") = \"%s\"\n", str1, str2, dest);
	assert(strcmp("helloworld",dest) == 0);
	return 0;
}
