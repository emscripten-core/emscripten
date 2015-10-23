// This file tests pthread barrier usage.

#include <emscripten/threading.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define N 100
#define THREADS 8

int matrix[N][N] = {};
int intermediate[N] = {};

// Barrier variable
pthread_barrier_t barr;

// Sums a single row of a matrix.
int sum_row(int r)
{
    int sum = 0;
    for(int i = 0; i < N; ++i)
        sum += matrix[r][i];
    return sum;
}

void *thread_main(void *arg)
{
    // Each thread sums individual rows.
    int id = (int)arg;
    for(int i = id; i < N; i += THREADS)
        intermediate[i] = sum_row(i);

    // Synchronization point
    int rc = pthread_barrier_wait(&barr);
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on barrier\n");
        exit(-1);
    }

    // Then each thread sums the one intermediate vector.
    int totalSum = 0;
    for(int i = 0; i < N; ++i)
        totalSum += intermediate[i];

    pthread_exit((void*)totalSum);
}

int main(int argc, char **argv)
{
    pthread_t thr[THREADS];

    // Create the matrix and compute the expected result.
    int expectedTotalSum = 0;
    srand(time(NULL));
    for(int i = 0; i < N; ++i)
        for(int j = 0; j < N; ++j)
        {
            matrix[i][j] = rand();
            expectedTotalSum += matrix[i][j];
        }
    printf("The sum of the matrix is %d.\n", expectedTotalSum);

    // Barrier initialization
    int ret = pthread_barrier_init(&barr, NULL, THREADS);
    assert(ret == 0); 

    for(int i = 0; i < THREADS; ++i) pthread_create(&thr[i], NULL, &thread_main, (void*)i);
    if (emscripten_has_threading_support())
    {
        for(int i = 0; i < THREADS; ++i)
        {
            int totalSum = 0;
            pthread_join(thr[i], (void**)&totalSum);
            assert(totalSum == expectedTotalSum);
        }
    }

#ifdef REPORT_RESULT
    int result = 0;
    REPORT_RESULT();
#endif
}
