#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

// This file tests the old GCC built-in atomic operations of the form __sync_fetch_and_op().
// See https://gcc.gnu.org/onlinedocs/gcc-4.6.4/gcc/Atomic-Builtins.html

#define NUM_THREADS 8

#define T uint64_t

#if 0
// TEMP to make this test pass:
// Our Clang backend doesn't define this builtin function, so implement it ourselves.
// The current Atomics spec doesn't have the nand atomic op either, so must use a cas loop.
// TODO: Move this to Clang backend?
T __sync_nand_and_fetch(T *ptr, T x)
{
	for(;;)
	{
		T old = emscripten_atomic_load_u32(ptr);
		T newVal = ~(old & x);
		T old2 = emscripten_atomic_cas_u32(ptr, old, newVal);
		if (old2 == old) return old;
	}
}
#endif

void *thread_add_and_fetch(void *arg)
{
	for(int i = 0; i < 10000; ++i)
		__sync_add_and_fetch((T*)arg, 0x0000000100000001ULL);
	pthread_exit(0);
}

void *thread_sub_and_fetch(void *arg)
{
	for(int i = 0; i < 10000; ++i)
		__sync_sub_and_fetch((T*)arg, 0x0000000100000001ULL);
	pthread_exit(0);
}
volatile T or_and_fetch_data = 0;
void *thread_or_and_fetch(void *arg)
{
	for(int i = 0; i < 10000; ++i)
		__sync_or_and_fetch((T*)&or_and_fetch_data, *(T*)arg);
	pthread_exit(0);
}

volatile T and_and_fetch_data = 0;
void *thread_and_and_fetch(void *arg)
{
	for(int i = 0; i < 10000; ++i)
		__sync_and_and_fetch((T*)&and_and_fetch_data, *(T*)arg);
	pthread_exit(0);
}

volatile T xor_and_fetch_data = 0;
void *thread_xor_and_fetch(void *arg)
{
	for(int i = 0; i < 9999; ++i) // Odd number of times so that the operation doesn't cancel itself out.
		__sync_xor_and_fetch((T*)&xor_and_fetch_data, *(T*)arg);
	pthread_exit(0);
}
#if 0

// XXX NAND support does not exist in Atomics API.
#if 0
volatile int nand_and_fetch_data = 0;
void *thread_nand_and_fetch(void *arg)
{
	for(int i = 0; i < 9999; ++i) // Odd number of times so that the operation doesn't cancel itself out.
		__sync_nand_and_fetch((int*)&nand_and_fetch_data, (int)arg);
	pthread_exit(0);
}
#endif
#endif
T threadArg[NUM_THREADS];
pthread_t thread[NUM_THREADS];

#define HILO(hi, lo) ((((uint64_t)(hi)) << 32) | ((uint64_t)(lo)))
#define DUP(x) HILO((x), (x))

int main()
{
	{
		T x = HILO(5, 3);
		T y = __sync_add_and_fetch(&x, DUP(1));
		assert(y == HILO(6, 4));
		assert(x == HILO(6, 4));
		volatile T n = HILO(2, 1);
		if (emscripten_has_threading_support())
		{
			for(int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_add_and_fetch, (void*)&n);
			for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
			printf("n: %llx\n", n);
			assert(n == HILO(NUM_THREADS*10000ULL+2ULL, NUM_THREADS*10000ULL+1ULL));
		}
	}
	{
		T x = HILO(15, 13);
		T y = __sync_sub_and_fetch(&x, HILO(10, 10));
		assert(y == HILO(5, 3));
		assert(x == HILO(5, 3));
		volatile T n = HILO(NUM_THREADS*10000ULL+5ULL, NUM_THREADS*10000ULL+3ULL);
		if (emscripten_has_threading_support())
		{
			for(int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_sub_and_fetch, (void*)&n);
			for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
			printf("n: %llx\n", n);
			assert(n == HILO(5,3));
		}
	}
	{
		T x = HILO(32768 + 5, 5);
		T y = __sync_or_and_fetch(&x, HILO(65536 + 9, 9));
		assert(y == HILO(32768 + 65536 + 13, 13));
		assert(x == HILO(32768 + 65536 + 13, 13));
		for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
		{
			or_and_fetch_data = HILO(65536 + (1<<NUM_THREADS), 1<<NUM_THREADS);
			if (emscripten_has_threading_support())
			{
				for(int i = 0; i < NUM_THREADS; ++i)
				{
					threadArg[i] = DUP(1 << i);
					pthread_create(&thread[i], NULL, thread_or_and_fetch, (void*)&threadArg[i]);
				}
				for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
				assert(or_and_fetch_data == HILO(65536 + (1<<(NUM_THREADS+1))-1, (1<<(NUM_THREADS+1))-1));
			}
		}
	}
	{
		T x = HILO(32768 + 5, 5);
		T y = __sync_and_and_fetch(&x, HILO(32768 + 9, 9));
		assert(y == HILO(32768 + 1, 1));
		assert(x == HILO(32768 + 1, 1));
		if (emscripten_has_threading_support())
		{
			for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
			{
				and_and_fetch_data = HILO(65536 + (1<<(NUM_THREADS+1))-1, (1<<(NUM_THREADS+1))-1);
				for(int i = 0; i < NUM_THREADS; ++i)
				{
					threadArg[i] = DUP(~(1UL<<i));
					pthread_create(&thread[i], NULL, thread_and_and_fetch, (void*)&threadArg[i]);
				}
				for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
				assert(and_and_fetch_data == HILO(65536 + (1<<NUM_THREADS), 1<<NUM_THREADS));
			}
		}
	}
	{
		T x = HILO(32768 + 5, 5);
		T y = __sync_xor_and_fetch(&x, HILO(16384 + 9, 9));
		assert(y == HILO(32768 + 16384 + 12, 12));
		assert(x == HILO(32768 + 16384 + 12, 12));
		if (emscripten_has_threading_support())
		{
			for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
			{
				xor_and_fetch_data = HILO(32768 + (1<<NUM_THREADS), 1<<NUM_THREADS);
				for(int i = 0; i < NUM_THREADS; ++i)
				{
					threadArg[i] = DUP(~(1UL<<i));
					pthread_create(&thread[i], NULL, thread_xor_and_fetch, (void*)&threadArg[i]);
				}
				for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
				assert(xor_and_fetch_data == HILO(32768 + ((1<<(NUM_THREADS+1))-1), (1<<(NUM_THREADS+1))-1));
			}
		}
	}
// XXX NAND support does not exist in Atomics API.
#if 0
	{
		T x = 5;
		T y = __sync_nand_and_fetch(&x, 9);
		assert(y == 5);
		assert(x == -2);
		const int oddNThreads = NUM_THREADS-1;
		for(int x = 0; x < 100; ++x) // Test a few times for robustness, since this test is so short-lived.
		{
			nand_and_fetch_data = 0;
			for(int i = 0; i < oddNThreads; ++i) pthread_create(&thread[i], NULL, thread_nand_and_fetch, (void*)-1);
			for(int i = 0; i < oddNThreads; ++i) pthread_join(thread[i], NULL);
			assert(nand_and_fetch_data == -1);
		}
	}
#endif

#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
