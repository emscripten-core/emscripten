import random, sys
num_cases = int(sys.argv[1])
cases = ''
i = 1
incr = 1
for x in range(0, num_cases):
	cases += '	case ' + str(i) + ': return "' + str(i) + str(i) + str(i) + '";\n'
	i += incr
	incr = (incr % 5) + 1

print '''#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

const char *foo(int x)
{
	switch(x)
	{
''' + cases + '''
	default: return "default";
	}
}

int main()
{
	for(int i = 0; i < ''' + str((num_cases+99)/100) + '''; ++i)
		printf("%d: %s\\n", i*301, foo(i*301));
	printf("Success!\\n");
}'''
