/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <pthread.h>
#include <emscripten.h>

int globalData = 1;

void *thread_main(void *arg)
{
  EM_ASM(out('hello from pthread 1: ' + $0), globalData);
  assert(globalData == 10);

  globalData = 20;
  EM_ASM(out('hello from pthread 2: ' + $0), globalData);
  assert(globalData == 20);
	return 0;
}

int main()
{
  EM_ASM(out('hello from main 1: ' + $0), globalData);
  assert(globalData == 1);

  globalData = 10;
  EM_ASM(out('hello from main 2: ' + $0), globalData);
  assert(globalData == 10);

	pthread_t thread;
	pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, 0);

  EM_ASM(out('hello from main 3: ' + $0), globalData);
  assert(globalData == 20);
#ifdef REPORT_RESULT
  REPORT_RESULT(globalData);
#endif
}
