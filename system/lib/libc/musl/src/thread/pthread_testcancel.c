#include "pthread_impl.h"

void __testcancel(void);

void pthread_testcancel()
{
	__testcancel();
}
