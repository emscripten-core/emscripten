// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>

int main()
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	void *val = pthread_getspecific(key);
	assert(val == 0);
	pthread_setspecific(key, (void*)1);
	val = pthread_getspecific(key);
	assert(val == (void*)1);

#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
