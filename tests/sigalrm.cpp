#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

void alarm_handler(int dummy)
{
	printf("Received alarm!\n");
#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
	exit(0);
}

int main()
{
	if (signal(SIGALRM, alarm_handler) == SIG_ERR)
	{
		printf("Error in signal()!\n");
#ifdef REPORT_RESULT
		int result = 1;
		REPORT_RESULT();
#endif
		exit(1);
	}
	alarm(5);
}
