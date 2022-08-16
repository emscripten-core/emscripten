#include <emscripten.h>
#include <stdio.h>

double t0 = emscripten_random();

double t1 = emscripten_get_now() + 100.0;

int main()
{
	printf("t1 > t0: %d\n", (int)(t1 > t0));
}
