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
#include <malloc.h>
#include <unistd.h>

#define FNAME_RO "data_ro.dat"
#define FNAME_RW "data_rw.dat"

/*
 * Each test will return 0 as success (no error) and error code in case of failure:
 * - > 0 it is errno value
 * - < 0 test specific failure (not related to any syscall)
 */

#define ASSERT(cond, msg) do {                      \
  if (!(cond)) {                                    \
    printf("%s:%03d FAILED '%s' - %s errno: %d\n",  \
            __func__, __LINE__, #cond, msg, errno); \
    return (errno != 0 ? errno : -1);               \
  }                                                 \
} while (0)

#define TEST_START() printf("%s - START\n", __func__)
#define TEST_PASS() do {             \
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

int test_mmap_read() {
  TEST_START();
  errno = 0;

  char *m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f_ro), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  for (size_t i = 0; i < file_len(); ++i) {
    ASSERT(m[i] == file_data[i], "Wrong file data mmaped");
  }
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");
  TEST_PASS();
}

int test_mmap_write() {
  TEST_START();
  errno = 0;

  char *m = (char *)mmap(NULL, file_len(), PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f_rw), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  // Reverse the data in the mapped file in memory, which should be written
  // out.
  for (size_t i = 0; i < file_len(); ++i) {
    size_t k = file_len() - i - 1;
    m[k] = file_data[i];
  }
  // Write to a byte past the end of the file. mmap will allocate a multiple
  // of the page size, which is bigger than our small file, and it will zero
  // that out. So it is ok for us to write there, but those changes should
  // not be saved anywhere.
  ASSERT(m[file_len()] == 0, "No zero past file contents");
  m[file_len()] = 42;
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");

  // mmap it again, where we should see the reversed data that was written.
  m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f_rw), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  for (size_t i = 0; i < file_len(); ++i) {
    size_t k = file_len() - i - 1;
    ASSERT(m[k] == file_data[i], "Wrong file data written or mapped");
  }
  ASSERT(m[file_len()] == 0, "No zero past file contents");
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");

  TEST_PASS();
}

int test_mmap_write_private() {
  TEST_START();
  errno = 0;

  /*
   * POSIX spec (http://pubs.opengroup.org/onlinepubs/9699919799/) for mmap
   * says that it should be possible to set PROT_WRITE flag for file opened
   * in RO mode if we use MAP_PRIVATE flag.
   */
  char *m = (char *)mmap(NULL, file_len(), PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f_ro), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  for (size_t i = 0; i < file_len(); ++i) {
    size_t k = file_len() - i;
    m[k] = file_data[i];
  }
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");

  /*
   * It is undefined however, if subsequent maps in the same process will or won't
   * see data written by such mmap.
   */
  m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f_ro), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  for (size_t i = 0; i < file_len(); ++i) {
    size_t k = file_len() - i;
    ASSERT(m[k] == file_data[i] || m[i] == file_data[i],
         "Wrong file data written or mapped");
  }
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");

  TEST_PASS();
}

int test_mmap_write_to_ro_file() {
  TEST_START();
  errno = 0;

  char *m = (char *)mmap(NULL, file_len(), PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f_ro), 0);
  ASSERT(m == MAP_FAILED && errno == EACCES,
       "Expected EACCES when requesting writing to file opened in read-only mode");
  TEST_PASS();
}

int test_mmap_anon() {
  TEST_START();
  errno = 0;

  void *m = (char *)mmap(NULL, file_len(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap pages");
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmmap allocated pages");
  TEST_PASS();
}

int test_mmap_fixed() {
  TEST_START();
  errno = 0;
  size_t page_size = sysconf(_SC_PAGE_SIZE);

  char *m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f_ro), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  char *invalid_addr = m;
  if (((uintptr_t)m) % page_size == 0) {
    invalid_addr++;
  }

  char *m2 = (char *)mmap(invalid_addr, file_len(), PROT_READ, MAP_FIXED, fileno(f_ro), 0);
  ASSERT(m2 == MAP_FAILED && errno == EINVAL, "Expected EINVAL for invalid addr");

  size_t invalid_offset = 1;
  char *m3 = (char *)mmap(m, file_len(), PROT_READ, MAP_FIXED, fileno(f_ro), invalid_offset);
  ASSERT(m3 == MAP_FAILED && errno == EINVAL, "Expected EINVAL for invalid offset");

  ASSERT(munmap(m, file_len()) == 0, "Failed to unmmap allocated pages");
  TEST_PASS();
}

int test_mmap_wrong_fd() {
  TEST_START();
  errno = 0;

  char *m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, -1, 0);
  ASSERT(m == MAP_FAILED && errno == EBADF, "Expected EBADF error");
  TEST_PASS();
}

int test_unmap_wrong_addr() {
  TEST_START();
  errno = 0;

  ASSERT(munmap((void*)0xdeadbeef, file_len()) == -1 && errno == EINVAL,
       "Expected EINVAL, as munmap should fail for wrong addr argument");
  TEST_PASS();
}

int test_unmap_zero_len() {
  TEST_START();
  errno = 0;

  char *m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f_ro), 0);
  ASSERT(m != MAP_FAILED, "Failed to mmap file");
  ASSERT(munmap(m, 0) == -1 && errno == EINVAL,
       "Expected EINVAL, as munmap should fail when len argument is 0");
  ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");
  TEST_PASS();
}

static int get_heap_usage() {
  struct mallinfo info = mallinfo();
  return info.uordblks;
}

int test_unmap_after_close() {
  TEST_START();
  errno = 0;

  int heap_start = get_heap_usage();

  for (int i = 0; i < 10; i++) {
    FILE *f = fopen(FNAME_RO, "r");
    char *m = (char *)mmap(NULL, file_len(), PROT_READ, MAP_PRIVATE, fileno(f), 0);
    ASSERT(m != MAP_FAILED, "Failed to mmap file");

    fclose(f);

    // Call munmap after closing the file 
    ASSERT(munmap(m, file_len()) == 0, "Failed to unmap allocated pages");
  }

  int heap_end = get_heap_usage();
  ASSERT(heap_start == heap_end, "Memory leak during mmap/munmap");
  TEST_PASS();
}

typedef int (*test_fn)();

int set_up() {
  FILE *f = fopen(FNAME_RO, "w");
  ASSERT(f != NULL, "Failed to open file " FNAME_RO " for writing");
  fprintf(f, "%s", file_data);
  fclose(f);

  f = fopen(FNAME_RW, "w");
  ASSERT(f != NULL, "Failed to open file " FNAME_RW " for writing");
  fprintf(f, "%s", file_data);
  fclose(f);

  f_ro = fopen(FNAME_RO, "r");
  ASSERT(f_ro != NULL, "Failed to open file " FNAME_RO " for reading");
  f_rw = fopen(FNAME_RW, "r+");
  ASSERT(f_rw != NULL, "Failed to open file " FNAME_RW " for reading and writing");

  return 0;
}

void tear_down() {
  if (f_ro != NULL) {
    fclose(f_ro);
    f_ro = NULL;
  }

  if (f_rw != NULL) {
    fclose(f_rw);
    f_rw = NULL;
  }
}

int main() {
  int failures = 0;
  test_fn tests[] = {
    test_mmap_read,
    test_mmap_write,
    test_mmap_write_private,
    test_mmap_write_to_ro_file,
    test_mmap_anon,
    test_mmap_fixed,
    test_mmap_wrong_fd,
    test_unmap_wrong_addr,
    test_unmap_zero_len,
    test_unmap_after_close,
    NULL
  };

  int tests_run = 0;
  while (tests[tests_run] != NULL) {
    if (set_up() != 0) {
      printf("Failed to set_up environment for TC\n");
      ++failures;
    } else {
      failures += (tests[tests_run]() != 0 ? 1 : 0);
    }

    tear_down();
    ++tests_run;
  }

  printf("tests_run: %d failures: %d\n", tests_run, failures);
  return failures;
}
