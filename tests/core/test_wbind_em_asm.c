#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <emscripten.h>

int test_func_iijdf(int a, uint64_t b, double d, float f)
{
	printf("a: %d\n", a);
	printf("b: %llx\n", b);
	printf("d: %g\n", d);
	printf("f: %f\n", f);
	return 42;
}

uint64_t test_func_ji(int a)
{
	printf("a: %d\n", a);
	return ((uint64_t)a << 32) | (uint64_t)a;
}

void test_wbind_iijdf()
{
	// Test that wbind() works from inside an EM_ASM() block.
	EM_ASM({
		var ret = wbind($0)(1, BigInt(0x300000002), 4.2, 5.3);
		console.log(ret);
	}, test_func_iijdf);
}

// Test that wbind() works from inside an EM_JS() block.
EM_JS(void, test_wbind_ji, (int funcPtr), {
    var ret = wbind(funcPtr)(1);
    console.log(ret);
})

int main()
{
	test_wbind_iijdf();
	test_wbind_ji((int)test_func_ji);
}
