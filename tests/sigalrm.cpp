// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

void alarm_handler(int dummy)
{
	printf("Received alarm!\n");
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
	exit(0);
}

int main()
{
	if (signal(SIGALRM, alarm_handler) == SIG_ERR)
	{
		printf("Error in signal()!\n");
#ifdef REPORT_RESULT
		REPORT_RESULT(1);
#endif
		exit(1);
	}
	alarm(5);
}
