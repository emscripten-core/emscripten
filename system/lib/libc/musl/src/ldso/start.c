#include <stdlib.h>

/* stub for archs that lack dynamic linker support */

void _start()
{
	_Exit(1);
}
