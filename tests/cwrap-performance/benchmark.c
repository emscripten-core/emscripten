#include "add.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>

int main(void) {
	char dest[20];
	int i;
	clock_t t = clock();
	for (i=0; i<10000; i++) {
		concat("hello", "world", dest);
	}
	printf("Executed concat 10000 times in %f ms\n", (float)((clock()-t)*1000)/CLOCKS_PER_SEC);
	assert(strcmp("helloworld",dest) == 0);
	return 0;
}
