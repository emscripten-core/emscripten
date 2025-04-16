// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <thread>
#include <math.h>
#include <emscripten.h>
#include <assert.h>
#include <iostream>

extern "C" {
//Create a thread that does some work
void EMSCRIPTEN_KEEPALIVE spawn_a_thread() {
	std::thread( [] {
		double d=0;
		for (int i=0; i<10; i++)			//simulate work
			d += (i%2 ? sqrt((int)(rand())) : (-1)*sqrt((int)(rand())));
	} ).detach();
}


//Check that the number of workers is less than the number of spawned threads.
void EMSCRIPTEN_KEEPALIVE count_threads(int num_threads_spawned, int num_threads_spawned_extra) {
	num_threads_spawned += num_threads_spawned_extra;
	int num_workers = EM_ASM_INT({
		return PThread.runningWorkers.length + PThread.unusedWorkers.length;
	});

	std::cout << 
		"Worker pool size: " << num_workers << 
		", Number of threads spawned: " << num_threads_spawned 
	<< "." << std::endl;
	assert(num_threads_spawned_extra != 0);
	assert(num_workers < num_threads_spawned);
	emscripten_force_exit(0);
}
}

//Spawn a detached thread every 0.1s. After 0.3s Check that the number of workers are less than the number of spawned threads
int main(int argc, char** argv) {
	EM_ASM(
		let thread_check = 0;
		const max_thread_check = 5;		//fail the test if the number of threads doesn't go down after checking this many times
		const threads_to_spawn = 3;
		let threads_to_spawn_extra = 0;
		
		//Spawn some detached threads
		for (let i=0; i<threads_to_spawn; i++) {
			setTimeout(() => { _spawn_a_thread(); }, i*100);
		}
		
		//Check if a worker is free every threads_to_spawn*100 ms, or until max_thread_check is exceeded
		const SpawnMoreThreads = setInterval(() => {
			if (PThread.unusedWorkers.length > 0) {	//Spawn a thread if a worker is available
				_spawn_a_thread();
				threads_to_spawn_extra++;
			}
			if (thread_check++ > max_thread_check || threads_to_spawn_extra > 0) {
				clearInterval(SpawnMoreThreads);
				_count_threads(threads_to_spawn, threads_to_spawn_extra);
			}
		}, threads_to_spawn*100);
	);
	emscripten_exit_with_live_runtime();
}
