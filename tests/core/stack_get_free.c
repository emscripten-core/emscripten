#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>
#include <emscripten.h>
#include <emscripten/stack.h>
#include <assert.h>
#include <string.h>

void __attribute__((noinline)) DoSomething(void *addr)
{
	memcpy(addr, addr + 42, 13);
}

void TestStackValidity()
{
	unsigned int used, free, free2, total;

	used = abs((int)emscripten_stack_get_base() - (int)emscripten_stack_get_current());
	free = abs((int)emscripten_stack_get_end() - (int)emscripten_stack_get_current());
	free2 = emscripten_stack_get_free();
	total = abs((int)emscripten_stack_get_end() - (int)emscripten_stack_get_base());
	assert(used + free == total);
	assert(free == free2);
}

int main()
{
	printf("Stack free: %u\n", emscripten_stack_get_free());
	TestStackValidity();

	unsigned int prevFree = emscripten_stack_get_free();
	for(int i = 0; i < 10; ++i)
	{
		void *p = alloca(emscripten_random() >= 0 ? 256*1024 : 255 * 1024);
		unsigned int free = emscripten_stack_get_free();
		assert(prevFree - free == 256*1024);
		prevFree = free;
		DoSomething(p);
		printf("Stack free: %u\n", emscripten_stack_get_free());
		TestStackValidity();
	}
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
