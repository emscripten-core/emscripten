import random, sys
num_cases = int(sys.argv[1])
cases = ''
i = 1
for x in range(0, num_cases):
	cases += '	case ' + str(i) + ': return "' + str(i) + str(i) + str(i) + '";\n'
	i += random.randint(1, 5)

print '''#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

const char *foo(int x)
{
	switch(x)
	{
''' + cases + '''
	default: return "";
	}
}

int main()
{
	for(int i = 0; i < 100; ++i)
		printf("%s\\n", foo((int)(emscripten_get_now() * 1000) % ''' + str(i) + '''));
	printf("Success!\\n");
}'''
