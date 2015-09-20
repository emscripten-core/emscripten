#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// This file tests the old GCC built-in atomic operations.
// See https://gcc.gnu.org/onlinedocs/gcc-4.6.4/gcc/Atomic-Builtins.html

#define NUM_THREADS 8

#define T int

// TEMP: Fastcomp backend doesn't implement these as atomic, so #define these to library
//       implementations that are properly atomic. TODO: Implement these in fastcomp.
#define __sync_lock_test_and_set(...) emscripten_atomic_fence()
#define __sync_lock_release(...) emscripten_atomic_fence()

#define Bool int

Bool atomic_bool_cas_u32(T *ptr, T oldVal, T newVal)
{
	T old = emscripten_atomic_cas_u32(ptr, oldVal, newVal);
	return old == oldVal;
}

// Test __sync_val_compare_and_swap.
T nand_and_fetch(T *ptr, T x)
{
	for(;;)
	{
		T old = emscripten_atomic_load_u32(ptr);
		T newVal = ~(old & x);
		T old2 = __sync_val_compare_and_swap(ptr, old, newVal);
		if (old2 == old) return newVal;
	}
}

// Test __sync_bool_compare_and_swap.
T nand_and_fetch_bool(T *ptr, T x)
{
	for(;;)
	{
		T old = emscripten_atomic_load_u32(ptr);
		T newVal = ~(old & x);
		Bool success = __sync_bool_compare_and_swap(ptr, old, newVal);
		if (success) return newVal;
	}
}

volatile int nand_and_fetch_data = 0;
void *thread_nand_and_fetch(void *arg)
{
	for(int i = 0; i < 999; ++i) // Odd number of times so that the operation doesn't cancel itself out.
		nand_and_fetch((int*)&nand_and_fetch_data, (int)arg);
	pthread_exit(0);
}

void *thread_nand_and_fetch_bool(void *arg)
{
	for(int i = 0; i < 999; ++i) // Odd number of times so that the operation doesn't cancel itself out.
		nand_and_fetch_bool((int*)&nand_and_fetch_data, (int)arg);
	pthread_exit(0);
}

pthread_t thread[NUM_THREADS];

int main()
{
	{
		T x = 5;
		T y = nand_and_fetch(&x, 9);
		assert(y == -2);
		assert(x == -2);
		const int oddNThreads = NUM_THREADS-1;
		for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
		{
			nand_and_fetch_data = 0;
			__sync_synchronize(); // This has no effect in this code, but called in here just to test that the compiler generates a valid expression for this.
			if (emscripten_has_threading_support())
			{
				for(int i = 0; i < oddNThreads; ++i) pthread_create(&thread[i], NULL, thread_nand_and_fetch, (void*)-1);
				for(int i = 0; i < oddNThreads; ++i) pthread_join(thread[i], NULL);
				assert(nand_and_fetch_data == -1);
			}
		}
	}
	{
		T x = 5;
		T y = nand_and_fetch_bool(&x, 9);
		assert(y == -2);
		assert(x == -2);
		const int oddNThreads = NUM_THREADS-1;
		for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
		{
			nand_and_fetch_data = 0;
			if (emscripten_has_threading_support())
			{
				for(int i = 0; i < oddNThreads; ++i) pthread_create(&thread[i], NULL, thread_nand_and_fetch_bool, (void*)-1);
				for(int i = 0; i < oddNThreads; ++i) pthread_join(thread[i], NULL);
				assert(nand_and_fetch_data == -1);
			}
		}
	}

#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
