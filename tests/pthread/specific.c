/*
 * Copyright 2013 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

int main(void)
{
    pthread_key_t key = 0;
    int rv;
    int data = 123;
    int *data2;

    assert(pthread_key_delete(key) != 0);
    assert(pthread_getspecific(key) == NULL);

    rv = pthread_key_create(&key, NULL);
    printf("pthread_key_create = %d\n", rv);
    assert(rv == 0);

    assert(pthread_getspecific(key) == NULL);

    rv = pthread_setspecific(key, (void*) &data);
    printf("pthread_setspecific = %d\n", rv);
    assert(rv == 0);

    data2 = pthread_getspecific(key);
    assert(data2 != NULL);
    printf("pthread_getspecific = %d\n", *data2);
    assert(*data2 == 123);

    rv = pthread_setspecific(key, NULL);
    printf("valid pthread_setspecific for value NULL = %d\n", rv);
    assert(rv == 0);

    data2 = pthread_getspecific(key);
    assert(data2 == NULL);
    printf("pthread_getspecific = %p\n", data2);

    rv = pthread_key_create(&key, NULL);
    data2 = pthread_getspecific(key);
    printf("pthread_getspecific after key recreate = %p\n", data2);
    assert(data2 == NULL);

    rv = pthread_key_delete(key);
    printf("pthread_key_delete = %d\n", rv);
    assert(rv == 0);

    rv = pthread_key_delete(key);
    printf("pthread_key_delete repeated = %d\n", rv);
    assert(rv == EINVAL);

    rv = pthread_setspecific(key, NULL);
    printf("pthread_setspecific for value NULL = %d\n", rv);
    assert(rv == EINVAL);

    rv = pthread_key_create(&key, NULL);
    assert(rv == 0);
    rv = pthread_key_delete(key);
    printf("pthread_key_delete just after created = %d\n", rv);
    assert(rv == 0);

    {
        /* Test creating multiple keys and overflowing the tls_entries array*/
        const int n = 5;
        int i;

        pthread_key_t *keys = malloc(sizeof (pthread_key_t) * n);
        for (i = 0; i < n; ++i) {
            rv = pthread_key_create(&keys[i], NULL);
            assert(rv == 0);
            rv = pthread_setspecific(keys[i], (void*)(intptr_t)(i + 1));
            assert(rv == 0);
        }

        for (i = 0; i < n; ++i) {
            void *d = pthread_getspecific(keys[i]);
            assert (i+1 == (intptr_t)d);
            rv = pthread_key_delete(keys[i]);
            assert(rv == 0);
        }
        free (keys);
    }

    return 0;
}
