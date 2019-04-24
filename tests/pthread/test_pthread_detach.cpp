// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <thread>
#include <math.h>
#include <emscripten.h>
#include <assert.h>

#ifndef REPORT_RESULT
#include <iostream>
#endif

extern "C" {
//Create a thread that does some work
void EMSCRIPTEN_KEEPALIVE spawn_a_thread() {
	std::thread( [] {
		double d=0;
		for (int i=0; i<10000000; i++)			//simulate work
			d += (i%2 ? sqrt((int)(rand())) : (-1)*sqrt((int)(rand())));
#ifndef REPORT_RESULT
		std::cout << d << std::endl;
#endif
	} ).detach();
}


//Check that the number of workers is less than the number of spawned threads.
void EMSCRIPTEN_KEEPALIVE count_threads(int num_threads_spawned) {
	int num_threads = EM_ASM_INT({
		return PThread.runningWorkers.length + PThread.unusedWorkerPool.length;
	});

#ifdef REPORT_RESULT
	if (num_threads < num_threads_spawned)
		REPORT_RESULT(0);
	else
		REPORT_RESULT(num_threads);
#else
	assert(num_threads < num_threads_spawned);
	std::cout << num_threads << std::endl;
#endif
}
}

//Spawn a detached thread every 0.5s. After 10s Check that the number of workers are less than the number of spawned threads
int main(int argc, char** argv) {
	EM_ASM(
		const i_max = 20;
		for (let i=0; i<i_max; i++) {
			setTimeout(() => { _spawn_a_thread(); }, i*500);
		}
		
		setTimeout(() => { _count_threads(i_max); }, i_max*500);
	);
}
