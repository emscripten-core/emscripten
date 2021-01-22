#include <stdint.h>
#include <assert.h>
#include <stdio.h>

extern void test_wbind_iijdf(int (*test_func)(int, uint64_t, double, float));
extern void test_wbind_ji(uint64_t (*test_func)(int));

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

int main()
{
	test_wbind_iijdf(test_func_iijdf);
	test_wbind_ji(test_func_ji);
}
