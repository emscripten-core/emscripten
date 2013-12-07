#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

static void destr_function(void *arg)
{
    // Not implemented yet in Emscripten
}

int main(void)
{
    pthread_key_t key = 0;
    int rv;
    int data = 123;
    int *data2;

    assert(pthread_key_delete(key) != 0);
    assert(pthread_getspecific(key) == NULL);

    rv = pthread_key_create(&key, &destr_function);
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

    rv = pthread_key_create(&key, &destr_function);
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

    rv = pthread_key_create(&key, &destr_function);
    assert(rv == 0);
    rv = pthread_key_delete(key);
    printf("pthread_key_delete just after created = %d\n", rv);
    assert(rv == 0);

    return 0;
}
