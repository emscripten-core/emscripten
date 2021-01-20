#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>

void vijdf(int i, int64_t j, double d, float f)
{
	EM_ASM(console.log('vijdf: i='+$0+',jlo='+$1+',jhi='+$2+',d='+$3+',f='+$4), i, (uint32_t)j, (uint32_t)(((uint64_t)j) >> 32), d, f);
}

int iii(int i, int j)
{
	EM_ASM(console.log('iii: i='+$0+',j='+$1), i, j);
	return 42;
}


void test_dyncalls_vijdf(void(*)(int, int64_t, double, float));
void test_dyncalls_iii(int(*)(int, int));

int main()
{
	test_dyncalls_vijdf(&vijdf);
	test_dyncalls_iii(&iii);
}
