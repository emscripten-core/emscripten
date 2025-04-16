#include <math.h>
#include <emscripten.h>
#include <stdio.h>

int main()
{
	float f = fmodf(emscripten_get_now() >= 0 ? 5.f : -5.f, 3.14f);
	printf("Mod: %f\n", f);
}
