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

struct Space {
  mspace space;
  bool allocated; // whether storage is allocated for this chunk, both an ArrayBuffer in JS and an mspace here
  size_t count; // how many allocations are in the space
  size_t index; // the index of this space, it then represents memory at SPLIT_MEMORY*index

  void allocate() {
    assert(!allocated);
    assert(count == 0);
    allocated = true;
    int start;
    if (index > 0) {
      EM_ASM_({ allocateSplitChunk($0) }, index);
      start = split_memory*index;
    } else {
      // small area in existing chunk 0
      start = EM_ASM_INT_V({ return (DYNAMICTOP+3)&-4; });
      assert(start < split_memory);
    }
    int size = (split_memory*(index+1)) - start;
    if (index > 0) assert(size == split_memory);
    space = create_mspace_with_base((void*)start, size, 0);
    if (index == 0) {
      if (!space) {
        EM_ASM({ Module.printErr("failed to create space in the first split memory chunk - SPLIT_MEMORY might need to be larger"); });
      }
    }
    assert(space);
  }

  void free() {
    assert(allocated);
    assert(count == 0);
    allocated = false;
    destroy_mspace((void*)(split_memory*index));
    if (index > 0) {
      EM_ASM_({ freeSplitChunk($0) }, index);
    }
  }
};

static Space spaces[MAX_SPACES];

static void init() {
  total_memory = EM_ASM_INT_V({ return TOTAL_MEMORY; });
  split_memory = EM_ASM_INT_V({ return SPLIT_MEMORY; });
  num_spaces = EM_ASM_INT_V({ return HEAPU8s.length; });
  if (num_spaces >= MAX_SPACES) abort();
  for (int i = 0; i < num_spaces; i++) {
    spaces[i].space = 0;
    spaces[i].allocated = false;
    spaces[i].count = 0;
    spaces[i].index = i;
  }
  initialized = true;
}

// TODO: optimize, these are powers of 2
// TODO: add optional asserts in these
#define space_index(ptr) (((unsigned)ptr) / split_memory)
#define space_relative(ptr) (((unsigned)ptr) % split_memory)

static mspace get_space(void* ptr) { // for a valid pointer, so the space must already exist
  int index = space_index(ptr);
  Space& space = spaces[index];
  assert(space.allocated);
  assert(space.count > 0);
  return space.space;
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
  static int next = 0;
  int start = next;
  while (1) { // simple round-robin, while keeping to use the same one as long as it keeps succeeding
    if (!spaces[next].allocated) spaces[next].allocate();
    void *ret = mspace_malloc(spaces[next].space, size);
    if (ret) {
      spaces[next].count++;
      return ret;
    }
    next++;
    if (next == num_spaces) next = 0;
    if (next == start) break;
  }
  return 0; // we cycled, so none of them can allocate
}

void free(void* ptr) {
  if (ptr == 0) return;
  int index = space_index(ptr);
  Space& space = spaces[index];
  assert(space.count > 0);
  mspace_free(get_space(ptr), ptr);
  space.count--;
  if (space.count == 0) {
    spaces[index].free();
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
  static int next = 0;
  int start = next;
  while (1) { // simple round-robin, while keeping to use the same one as long as it keeps succeeding
    if (!spaces[next].allocated) spaces[next].allocate();
    void *ret = mspace_memalign(spaces[next].space, alignment, size);
    if (ret) {
      spaces[next].count++;
      return ret;
    }
    next++;
    if (next == num_spaces) next = 0;
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

