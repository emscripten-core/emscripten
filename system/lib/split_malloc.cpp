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

enum AllocateResult {
  OK = 0,
  NO_MEMORY = 1,
  ALREADY_USED = 2
};

struct Space {
  mspace space;
  bool allocated; // whether storage is allocated for this chunk, both an ArrayBuffer in JS and an mspace here
  size_t count; // how many allocations are in the space
  size_t index; // the index of this space, it then represents memory at SPLIT_MEMORY*index

  void init(int i) {
    space = 0;
    allocated = false;
    count = 0;
    index = i;
  }

  AllocateResult allocate() {
    assert(!allocated);
    assert(count == 0);
    allocated = true;
    int start;
    if (index > 0) {
      if (int(split_memory*(index+1)) < 0) {
        // 32-bit pointer overflow. we could support more than this 2G, up to 4GB, if we made all pointer shifts >>>. likely slower though
        return NO_MEMORY;
      }
      AllocateResult result = (AllocateResult)EM_ASM_INT({
        // can fail due to the browser not have enough memory for the chunk, or if the slice
        // is already used, which can happen if other code allocated it
        try {
          return allocateSplitChunk($0) ? $1 : $3;
        } catch(e) {
          return $2; // failed to allocate
        }
      }, index, OK, NO_MEMORY, ALREADY_USED);
      if (result != OK) return result;
      start = split_memory*index;
    } else {
      // small area in existing chunk 0
      start = EM_ASM_INT_V({ return (HEAP32[DYNAMICTOP_PTR>>2]+3)&-4; });
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
    return OK;
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
    spaces[i].init(i);
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

static void* get_memory(size_t size, bool malloc=true, size_t alignment=-1, bool must_succeed=false) {
  if (!initialized) {
    init();
  }
  if (size >= split_memory) {
    static bool warned = false;
    if (!warned) {
      EM_ASM_({
        Module.print("trying to get " + $0 + ", a size >= than SPLIT_MEMORY (" + $1 + "), increase SPLIT_MEMORY if you want that to work");
      }, size, split_memory);
      warned = true;
    }
    return 0;
  }
  static int next = 0;
  int start = next;
  while (1) { // simple round-robin, while keeping to use the same one as long as it keeps succeeding
    AllocateResult result = OK;
    if (!spaces[next].allocated) {
      result = spaces[next].allocate();
      if (result == NO_MEMORY) return 0; // mallocation failure
    }
    if (result == OK) {
      void *ret;
      if (malloc) {
        ret = mspace_malloc(spaces[next].space, size);
      } else {
        ret = mspace_memalign(spaces[next].space, alignment, size);
      }
      if (ret) {
        spaces[next].count++;
        return ret;
      }
      if (must_succeed) {
        EM_ASM({ Module.printErr("failed to allocate in a new space after memory growth, perhaps increase SPLIT_MEMORY?"); });
        abort();
      }
    } else {
      assert(result == ALREADY_USED); // continue on to the next space
    }
    next++;
    if (next == num_spaces) next = 0;
    if (next == start) break;
  }
  // we cycled, so none of them can allocate
  int returnNull = EM_ASM_INT_V({
    if (!ABORTING_MALLOC && !ALLOW_MEMORY_GROWTH) return 1; // malloc can return 0, and we cannot grow
    if (!ALLOW_MEMORY_GROWTH) {
      abortOnCannotGrowMemory();
    }
    return 0;
  });
  if (returnNull) return 0;
  // memory growth is on, add another chunk
  if (num_spaces + 1 >= MAX_SPACES) abort();
  spaces[num_spaces].init(num_spaces);
  next = num_spaces;
  num_spaces++;
  return get_memory(size, malloc, alignment, true);
}

extern "C" {

void* malloc(size_t size) {
  return get_memory(size);
}

void* memalign(size_t alignment, size_t size) {
  return get_memory(size, false, alignment);
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

