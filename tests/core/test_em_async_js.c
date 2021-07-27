#include <emscripten.h>
#include <stdio.h>

EM_ASYNC_JS(double, foo, (int timeout), {
	await new Promise(resolve => setTimeout(resolve, timeout));
	return 4.2;
});

int main() {
	printf("foo returned: %f\n", foo(10));
	return 0;
}
