/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define FNAME_RO "data_ro.dat"
#define FNAME_RW "data_rw.dat"

/*
 * Each test will return 0 as success (no error) and error code in case of failure:
 * - > 0 it is errno value
 * - < 0 test specific failure (not related to any syscall)
 */

#define ASSERT(cond, msg) do {                          \
    if (!(cond)) {                                      \
        printf("%s:%03d FAILED '%s' - %s errno: %d\n",  \
                __func__, __LINE__, #cond, msg, errno); \
        return (errno != 0 ? errno : -1);               \
    }                                                   \
} while (0)

#define TEST_START() printf("%s - START\n", __func__)
#define TEST_PASS() do {               \
    printf("%s - PASSED\n", __func__); \
    return 0;                          \
} while (0)

const char file_data[] =
    "Copyright 2019 The Emscripten Authors.  All rights reserved.\n"
    "Emscripten is available under two separate licenses, the MIT license and the\n"
    "University of Illinois/NCSA Open Source License.  Both these licenses can be\n"
    "found in the LICENSE file.\n";

FILE *f_ro = NULL;
FILE *f_rw = NULL;

// Length of the file data excluding trailing '\0'. Equivalent to strlen(file_data) .
size_t file_len() {
    return sizeof(file_data) - 1;
}

int test_mmap_anon() {
    TEST_START();
    errno = 0;

    void *m = (char *)mmap(NULL, file_len(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(m != MAP_FAILED, "Failed to mmap pages");
    ASSERT(munmap(m, file_len()) == 0, "Failed to unmmap allocated pages");
    TEST_PASS();
}

typedef int (*test_fn)();

int main() {
    int failures = 0;
    test_fn tests[] = {
        test_mmap_anon,
        NULL
    };

    int tests_run = 0;
    while (tests[tests_run] != NULL) {
        failures += (tests[tests_run]() != 0 ? 1 : 0);
        ++tests_run;
    }

    printf("tests_run: %d\n", tests_run);
    printf("failures: %d\n", failures);
    return failures;
}

