/*
   malloc/free for SPLIT_MEMORY
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <emscripten.h>

extern "C" {

typedef void* mspace;

mspace create_mspace_with_base(void* base, size_t capacity, int locked);
size_t destroy_mspace(mspace msp);
void* mspace_malloc(mspace space, size_t size);
void mspace_free(mspace space, void* ptr);
void* mspace_realloc(mspace msp, void* oldmem, size_t bytes);
void* mspace_memalign(mspace msp, size_t alignment, size_t bytes);

}

#define MAX_SPACES 1000

static bool initialized = false;
static size_t total_memory = 0;
static size_t split_memory = 0;
static size_t num_spaces = 0;
static bool allocated[MAX_SPACES]; // whether storage is allocated for this chunk, both an ArrayBuffer in JS and an mspace here
static mspace spaces[MAX_SPACES]; // 0 is for the stack, static, etc - not used by malloc # TODO: make a small space in there?
static size_t counts[MAX_SPACES]; // how many allocations are in the space

static void init() {
  total_memory = EM_ASM_INT_V({ return TOTAL_MEMORY; });
  split_memory = EM_ASM_INT_V({ return SPLIT_MEMORY; });
  num_spaces = EM_ASM_INT_V({ return HEAPU8s.length; });
  if (num_spaces >= MAX_SPACES) abort();
  allocated[0] = true; // but never used from here
  spaces[0] = 0; // never used
  counts[0] = 0; // never used
  for (int i = 1; i < num_spaces; i++) {
    allocated[i] = false;
    spaces[i] = 0;
    counts[i] = 0;
  }
  initialized = true;
}

static void allocate_space(int i) {
  assert(!allocated[i]);
  assert(counts[i] == 0);
  allocated[i] = true;
  EM_ASM_({ allocateSplitChunk($0) }, i);
  spaces[i] = create_mspace_with_base((void*)(split_memory*i), split_memory, 0);
}

static void free_space(int i) {
  assert(allocated[i]);
  assert(counts[i] == 0);
  allocated[i] = false;
  destroy_mspace((void*)(split_memory*i));
  EM_ASM_({ freeSplitChunk($0) }, i);
}

// TODO: optimize, these are powers of 2
// TODO: add optional asserts in these
#define space_index(ptr) (((unsigned)ptr) / split_memory)
#define space_relative(ptr) (((unsigned)ptr) % split_memory)

static mspace get_space(void* ptr) { // for a valid pointer, so the space must already exist
  int index = space_index(ptr);
  assert(allocated[index]);
  assert(counts[index] > 0);
  return spaces[index];
}

extern "C" {

void* malloc(size_t size) {
  if (!initialized) {
    init();
  }
  if (size >= split_memory) {
    static bool warned = false;
    if (!warned) {
      EM_ASM_({
        Module.print("trying to malloc " + $0 + ", a size >= than SPLIT_MEMORY (" + $1 + "), increase SPLIT_MEMORY if you want that to work");
      }, size, split_memory);
      warned = true;
    }
    return 0;
  }
  static int next = 1;
  int start = next;
  while (1) { // simple round-robin, while keeping to use the same one as long as it keeps succeeding
    if (!allocated[next]) allocate_space(next);
    void *ret = mspace_malloc(spaces[next], size);
    if (ret) {
      counts[next]++;
      return ret;
    }
    next++;
    if (next == num_spaces) next = 1;
    if (next == start) break;
  }
  return 0; // we cycled, so none of them can allocate
}

void free(void* ptr) {
  if (ptr == 0) return;
  int index = space_index(ptr);
  assert(counts[index] > 0);
  mspace_free(get_space(ptr), ptr);
  counts[index]--;
  if (counts[index] == 0) {
    free_space(index);
  }
}

void* realloc(void* ptr, size_t newsize) {
  if (!ptr) return malloc(newsize);
  void* ret = mspace_realloc(get_space(ptr), ptr, newsize);
  if (ret) return ret;
  ret = malloc(newsize);
  if (!ret) return 0;
  size_t copysize = newsize;
  if (((unsigned)ptr) + copysize > total_memory) copysize = total_memory - ((unsigned)ptr);
  memcpy(ret, ptr, copysize);
  free(ptr);
  return ret;
}

void* calloc(size_t num, size_t size) {
  size_t bytes = num * size;
  void* ret = malloc(bytes);
  if (!ret) return 0;
  memset(ret, 0, bytes);
  return ret;
}

void* memalign(size_t alignment, size_t size) {
  if (!initialized) {
    init();
  }
  if (size >= split_memory) {
    static bool warned = false;
    if (!warned) {
      EM_ASM_({
        Module.print("trying to malloc " + $0 + ", a size larger than SPLIT_MEMORY (" + $1 + "), increase SPLIT_MEMORY if you want that to work");
      }, size, split_memory);
      warned = true;
    }
    return 0;
  }
  static int next = 1;
  int start = next;
  while (1) { // simple round-robin, while keeping to use the same one as long as it keeps succeeding
    if (!allocated[next]) allocate_space(next);
    void *ret = mspace_memalign(spaces[next], alignment, size);
    if (ret) {
      counts[next]++;
      return ret;
    }
    next++;
    if (next == num_spaces) next = 1;
    if (next == start) break;
  }
  return 0; // we cycled, so none of them can allocate
}

// very minimal sbrk, within one chunk
void* sbrk(intptr_t increment) {
  const int SBRK_CHUNK = split_memory - 1024;
  static size_t start = -1;
  static size_t curr = 0;
  if (start == -1) {
    start = (size_t)malloc(SBRK_CHUNK);
    if (!start) {
      EM_ASM({ Module.printErr("sbrk() failed to get space"); });
      abort();
    }
    curr = start;
  }
  if (curr - start + increment >= SBRK_CHUNK || curr + increment < start) return (void*)-1;
  size_t ret = curr;
  curr += increment;
  return (void*)ret;
}

}

