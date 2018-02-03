#include <unistd.h> // for sbrk()

// Math utilities

static bool isPowerOf2(size_t x) {
  return __builtin_popcount(x) == 1;
}

static size_t upperBoundByPowerOf2(size_t x) {
  if (x == 0) return 1;
  if (isPowerOf2(x)) return x;
  // e.g. 5 is 0..0101, so clz is 29, and we
  // want 8 which is 1 << 3
  return 1 << (32 - __builtin_clz(x));
}

static size_t lowerBoundByPowerOf2(size_t x) {
  if (x == 0) return 1;
  if (isPowerOf2(x)) return x;
  // e.g. 5 is 0..0101, so clz is 29, and we
  // want 4 which is 1 << 2
  return 1 << (31 - __builtin_clz(x));
}

// Constants

// All allocations are aligned to this value. This is
// also the minimum allocation size.
static const size_t ALIGNMENT = 16;
static const size_t MIN_ALLOC = ALIGNMENT;

// How big the metadata is in each region. It is convenient
// that this is identical to the above values.
static const size_t METADATA_SIZE = MIN_ALLOC;

// How big a minimal region is.
static const size_t MIN_REGION_SIZE = METADATA_SIZE + MIN_ALLOC;

// Constant utilities

// Align a pointer, increasing it upwards as necessary
static size_t alignUp(size_t ptr) {
  return (size_t(ptr) + ALIGNMENT - 1) & -ALIGNMENT;
}

static void* alignUpPointer(void* ptr) {
  return (void*)alignUp(size_t(ptr));
}

//
// Data structures
//

struct Region;

// Information memory that is a free list, i.e., may
// be reused.
struct FreeInfo {
  // The region of the memory this is associated with.
  Region* region;

  // free lists are doubly-linked lists
  FreeInfo* prev = NULL,
            next = NULL;
};

// The core metadata for each section of memory
struct Metadata {
  // The total size of the section of memory this is associated
  // with and contained in.
  // That includes the metadata itself and the payload memory after.
  size_t totalSize;

  // Each memory area knows its neighbors, as we hope to merge them.
  // If there is no neighbor, NULL.
  Region* prev = NULL,
          next = NULL;

  // Whether the payload is being used for allocation, that is, whether
  // it was malloc'd.
  size_t inUse;
};

// A contiguous region of memory. Metadata at the beginning describes it,
// after which is the "payload", the sections that user code calling
// malloc can use.
struct Region {
  // The core metadata for this memory.
  Metadata metadata;

  // If in use, we have the payload here. If not in use, the memory
  // contains a FreeInfo
  union {
    char payload[];
    FreeInfo freeInfo;
  };
};

// Region utilities

static void initRegion(Region* region, size_t totalSize) {
  region->metadata.totalSize = totalSize;
  region->metadata.prev = NULL;
  region->metadata.next = NULL;
  region->metadata.inUse = 0;
}

static void* getPayload(Region* region) {
  assert(sizeof(Metadata) == METADATA_SIZE);
  assert(&region->freeInfo - region == METADATA_SIZE);
  assert(region->inUse);
  return &region->payload;
}

static Region* fromPayload(void* payload) {
  assert(sizeof(Metadata) == METADATA_SIZE);
  assert(&region->freeInfo - region == METADATA_SIZE);
  assert(region->inUse);
  return (region*)(payload - sizeof(Metadata));
}

static void* getPayloadSize(Region* region) {
  return region->metadata.totalSize - METADATA_SIZE;
}

static FreeInfo* getFreeInfo(Region* region) {
  assert(!region->inUse);
  return &region->freeInfo;
}

static void* getAfter(Region* region) {
  return ((void*)region) + region->metadata.totalSize;
}

// Globals

// TODO: For now we have a single global space for all allocations,
//       but for multithreading etc. we may want to generalize that.

// a freelist (a list of Regions ready for re-use) for all
// power of 2 payload sizes (only the ones from ALIGNMENT
// size and above are relevant, though). The freelist at index
// K contains regions of memory big enough to contain 2^K bytes.
static const size_t NUM_FREELISTS = 32;
static FreeInfo* freeLists[NUM_FREELISTS] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

// The last region of memory. It's important to know the end
// since we may append to it.
static Region* lastRegion = NULL;

// Global utilities

static size_t getFreeListIndex(size) {
  assert(size > 0);
  if (size < MIN_ALLOC) size = MIN_ALLOC;
  // We need a lower bound here, as the list contains things
  // that can contain at least a power of 2.
  size_t ret = lowerBoundByPowerOf2(size);
  assert(ret < NUM_FREELISTS);
  return ret;
}

static void removeFromFreeList(Region* region) {
  assert(!region->metadata.inUse);
  size_t index = getFreeListIndex(getPayloadSize(region));
  FreeInfo* freeInfo = getFreeInfo(region);
  if (*freeLists[index] == freeInfo) {
    *freeLists[index] = freeInfo->next;
  }
  if (freeInfo->prev) {
    freeInfo->prev->next = freeInfo->next;
  }
  if (freeInfo->next) {
    freeInfo->next->prev = freeInfo->prev;
  }
}

static void addToFreeList(Region* region) {
  assert(!region->metadata.inUse);
  size_t index = getFreeListIndex(getPayloadSize(region));
  FreeInfo* freeInfo = getFreeInfo(region);
  FreeInfo* last = freeLists[index];
  freeLists[index] = freeInfo;
  freeInfo->prev = NULL;
  freeInfo->next = last;
}

static void possiblySplitRemainder(Region* region, size_t size) {
  size_t payloadSize = getPayloadSize(region);
  assert(payloadSize >= size);
  size_t extra = payloadSize - size;
  // We need room for a minimal region, but also must align it.
  if (extra >= MIN_REGION_SIZE + ALIGNMENT) {
    // Worth it, split the region
    // TODO: Consider not doing it, may affect long-term fragmentation.
    Region* split = (Region*)alignUpPointer(getPayload(region) + size);
    size_t totalSplitSize = (void*)split - (void*)region;
    assert(totalSplitSize >= MIN_REGION_SIZE);
    initRegion(split, totalSplitSize);
    split->prev = region;
    split->next = region->next;
    region->next = split;
    addToFreeList(split);
  }
}

static void useRegion(Region* region, size_t size) {
  assert(!region->metadata.inUse);
  region->metadata.inUse = 1;
  // We may not be using all of it, split out a smaller
  // region into a free list if it's large enough.
  possiblySplitRemainder(region, size);
}

static Region* getFromFreeList(size_t size) {
  size_t index = getFreeListIndex(size);
  FreeInfo* freeInfo = freeLists[index];
  if (!freeInfo) return NULL;
  Region* region = freeInfo->region;
  // This region is no longer free
  removeFromFreeList(region);
  // This region is now in use
  useRegion(region, size);
  return region;
}

static Region* newAllocation(size_t size) {
  assert(size > 0);
  size_t sbrkSize = METADATA_SIZE + alignUp(size);
  Region* region = sbrk(sbrkSize);
  if (region == (void*)-1) {
    // sbrk failed, we failed.
    return NULL;
  }
  // Success, we have new memory
  initRegion(region, sbrkSize);
  useRegion(region, size);
  // Apply globally, connect it to lastRegion
  if (lastRegion) {
    // If this is adjacent to the previous region, link them.
    if (region == getAfter(lastRegion)) {
      assert(lastRegion->next == NULL);
      lastRegion->next = region;
      region->prev = lastRegion;
    }
  }
  lastRegion = region;
  return region;
}

// public API

void* malloc(size_t size) {
  if (size == 0) return NULL;
  // Look in the freelist first.
  Region* region = getFromFreeList(size);
  if (!region) {
    // Allocate some new memory otherwise.
    region = newAllocation(size);
    if (!region) {
      // We failed to allocate, sadly.
      return NULL;
    }
  }
  return getPayload(region);
}

void free(void *ptr) {
}

void* calloc(size_t nmemb, size_t size) {
}

void* realloc(void *ptr, size_t size) {
}

