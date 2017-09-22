#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

int main()
{
	for(int times = 0; times < 16; ++times)
		for(int alignment = sizeof(void*); alignment <= 64; alignment *= 2)
			assert((uintptr_t)aligned_alloc(alignment, 19) % alignment == 0);
	return 0;
}
