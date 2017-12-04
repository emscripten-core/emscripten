#include <emscripten.h>
#include <stdio.h>

int main()
{
	// Test that on main browser thread, MAIN_THREAD_ASYNC_EM_ASM() will get
	// synchronously executed.
	printf("Before MAIN_THREAD_ASYNC_EM_ASM\n");
	MAIN_THREAD_ASYNC_EM_ASM(Module.print('Inside MAIN_THREAD_ASYNC_EM_ASM: ' + $0 + ' ' + $1), 42, 3.5);
	printf("After MAIN_THREAD_ASYNC_EM_ASM\n");
}
