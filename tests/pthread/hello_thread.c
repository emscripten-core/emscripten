#include <pthread.h>
#include <emscripten.h>

void *thread_main(void *arg)
{
	EM_ASM(Module.print('hello from thread!'));
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
	return 0;
}

int main()
{
	pthread_t thread;
	pthread_create(&thread, NULL, thread_main, NULL);
	EM_ASM(Module['noExitRuntime']=true);
}
