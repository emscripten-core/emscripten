/*
 * Simple minimalistic but efficient malloc/free.
 *
 * Assumptions:
 *
 *  - 32-bit system.
 *  - Single-threaded.
 *  - sbrk() is used, and nothing else.
 *  - sbrk() will not be accessed by anyone else.
 *
 * Invariants:
 *
 *  - Metadata is 16 bytes, allocation payload is a
 *    multiple of 16 bytes.
 *  - All regions of memory are adjacent.
 *  - Due to the above, after initial alignment fixing, all
 *    regions are aligned.
 *  - A region is either in use (used payload > 0) or not.
 *    Used regions may be adjacent, and a used and unused region
 *    may be adjacent, but not two unused ones - they would be
 *    merged.
 *  - A used region always has minimal space at the end - we
 *    split off extra space when possible immediately.
 *
 * Debugging:
 *
 *  - If not NDEBUG, runtime assert()s are in use.
 *  - If EMMALLOC_DEBUG is defined, a large amount of extra checks are done.
 *  - If EMMALLOC_DEBUG_LOG is defined, a lot of operations are logged
 *    out, in addition to EMMALLOC_DEBUG.
 *  - Debugging and logging uses EM_ASM, not printf etc., to minimize any
 *    risk of debugging or logging depending on malloc.
 *
 * TODO
 *
 *  - posix_memalign() (test_aligned_alloc)
 *  - memalign() (test_mmap, test_openjpeg)
 */

#include <assert.h>
#include <malloc.h> // mallinfo
#include <string.h> // for memcpy, memset
#include <unistd.h> // for sbrk()
#include <emscripten.h>

#define EMMALLOC_EXPORT __attribute__((__weak__, __visibility__("default")))

// Debugging

#ifdef EMMALLOC_DEBUG_LOG
#ifndef EMMALLOC_DEBUG
#define EMMALLOC_DEBUG
#endif
#endif

#ifdef EMMALLOC_DEBUG
// Forward declaration for convenience.
static void emmalloc_validate_all();
#endif
#ifdef EMMALLOC_DEBUG
// Forward declaration for convenience.
static void emmalloc_dump_all();
#endif

// Math utilities

static bool isPowerOf2(size_t x) {
  return __builtin_popcount(x) == 1;
}

static size_t lowerBoundPowerOf2(size_t x) {
  if (x == 0) return 1;
  // e.g. 5 is 0..0101, so clz is 29, and we want
  // 4 which is 1 << 2, so the result should be 2
  return 31 - __builtin_clz(x);
}

// Constants

// The absolute minimum alignment of all things.
static const size_t MIN_ALIGNMENT = 4;

// Even allocating 1 byte incurs this much actual payload
// allocation. This is big enough to allow freelist info to
// be kept in a region's payload when it is not in use.
// (A downside is that we need a little more room for tiny
// allocations of 1-7 bytes).
// This is also the alignment of things of this size and
// above.
// TODO: should 16-byte and above values have 16-byte align?
static const size_t ALLOC_UNIT = 8;

// How big the metadata is in each region. It is convenient
// that this is identical to the above values.
static const size_t METADATA_SIZE = 8;

// How big a minimal region is.
static const size_t MIN_REGION_SIZE = METADATA_SIZE + ALLOC_UNIT;

// Constant utilities

// Align a size for allocation, increasing it upwards as necessary.
// How much alignment depends on the size we are aligning for.
// We also enforce the minimum allocation size rule
static size_t alignUp(size_t size) {
  if (size >= ALLOC_UNIT) {
    return (size + MIN_ALIGNMENT - 1) & -MIN_ALIGNMENT;
  } else {
    // We have a minimum size of ALLOC_UNIT.
    return ALLOC_UNIT;
  }
}

static void* alignUpForSize(void* ptr, size_t forSize) {
  if (forSize >= ALLOC_UNIT) {
    return (void*)((size_t(ptr) + ALLOC_UNIT - 1) & -ALLOC_UNIT);
  } else {
    return (void*)((size_t(ptr) + MIN_ALIGNMENT - 1) & -MIN_ALIGNMENT);
  }
}

static int isAlignedForSize(void* ptr, size_t size) {
  return ptr == alignUpForSize(ptr, size);
}

//
// Data structures
//

struct Region;

// Information memory that is a free list, i.e., may
// be reused.
// Note how this can fit instead of the payload (as
// the payload is a multiple of MIN_ALLOC).
struct FreeInfo {
  // free lists are doubly-linked lists
  FreeInfo* _prev;
  FreeInfo* _next;

  FreeInfo*& prev() { return _prev; }
  FreeInfo*& next() { return _next; }
};

// The first region of memory.
static Region* firstRegion = NULL;

// The last region of memory. It's important to know the end
// since we may append to it.
static Region* lastRegion = NULL;

// A contiguous region of memory. Metadata at the beginning describes it,
// after which is the "payload", the sections that user code calling
// malloc can use.
struct Region {
  // The total size of the section of memory this is associated
  // with and contained in.
  // That includes the metadata itself and the payload memory after,
  // which includes the used and unused portions of it.
  // FIXME: Shift by 1, as our size is even anyhow?
  //        Or, disallow allocation of half the total space or above.
  //        Browsers barely allow allocating 2^31 anyhow, so inside that
  //        space we can just allocate something smaller than it.
  size_t _totalSize : 31;

  // Whether this region is in use or not.
  size_t _used : 1;

  // Each memory area knows its previous neighbor, as we hope to merge them.
  // To compute the next neighbor we can use the total size, and to know
  // if a neighbor exists we can compare the region to lastRegion
  Region* _prev;

  // Up to here was the fixed metadata, of size 16. The rest is either
  // the payload, or freelist info.
  union {
    FreeInfo _freeInfo;
    char _payload[];
  };

  size_t getTotalSize() { return _totalSize; }
  void setTotalSize(size_t x) { _totalSize = x; }
  void incTotalSize(size_t x) { _totalSize += x; }
  void decTotalSize(size_t x) { _totalSize -= x; }

  size_t getUsed() { return _used; }
  void setUsed(size_t x) { _used = x; }

  Region*& prev() { return _prev; }
  // The next region is not, as we compute it on the fly
  Region* next() {
    if (this != lastRegion) {
      return (Region*)((char*)this + getTotalSize());
    } else {
      return NULL;
    }
  }
  FreeInfo& freeInfo() { return _freeInfo; }
  // The payload is special, we just return its address, as we
  // never want to modify it ourselves.
  char* payload() { return &_payload[0]; }
};

// Region utilities

// TODO: move into struct
static void* getPayload(Region* region) {
  assert(((char*)&region->freeInfo()) - ((char*)region) == METADATA_SIZE);
  assert(sizeof(FreeInfo) == ALLOC_UNIT);
  return region->payload();
}

static Region* fromPayload(void* payload) {
  return (Region*)((char*)payload - METADATA_SIZE);
}

static Region* fromFreeInfo(FreeInfo* freeInfo) {
  return (Region*)((char*)freeInfo - METADATA_SIZE);
}

static size_t getMaxPayload(Region* region) {
  return region->getTotalSize() - METADATA_SIZE;
}

// TODO: move into class, make more similar to next()
static void* getAfter(Region* region) {
  return ((char*)region) + region->getTotalSize();
}

// Globals

// TODO: For now we have a single global space for all allocations,
//       but for multithreading etc. we may want to generalize that.

// A freelist (a list of Regions ready for re-use) for all
// power of 2 payload sizes (only the ones from ALIGNMENT
// size and above are relevant, though). The freelist at index
// K contains regions of memory big enough to contain at least
// 2^K bytes.
//
// Note that there is no freelist for 2^32, as that amount can
// never be allocated.
//
// The freelist indexes normally go from 3 (2^3 == 8, ALLOC_UNIT),
// but if an item of size 8 is not aligned it will go into the
// lower index of 2. Speculative freelist tries can get it from
// there.

static const size_t MIN_FREELIST_INDEX = 2;  // 8 == ALLOC_UNIT
static const size_t MAX_FREELIST_INDEX = 32; // uint32_t

static FreeInfo* freeLists[MAX_FREELIST_INDEX] = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

// Global utilities

// Items in the freelist at this index must be at least this large.
static size_t getMinSizeForFreeListIndex(size_t index) {
  return 1 << index;
}

// The freelist index is where we would appear in a freelist if
// we were one. It is a list of items of size at least the power
// of 2 that lower bounds us, and with proper alignment.
static size_t getFreeListIndex(size_t size, void* payload) {
  assert((1 << MIN_FREELIST_INDEX) == (ALLOC_UNIT >> 1));
  assert(size > 0);
  if (size < ALLOC_UNIT) size = ALLOC_UNIT;
  // We need a lower bound here, as the list contains things
  // that can contain at least a power of 2.
  size_t index = lowerBoundPowerOf2(size);
  // TODO: this loop could be a few ifs.
  while (!isAlignedForSize(payload, getMinSizeForFreeListIndex(index))) {
    index--;
  }
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.getFreeListIndex " + [$0, $1, $2]) }, size, payload, index);
#endif
  assert(MIN_FREELIST_INDEX <= index && index < MAX_FREELIST_INDEX);
  return index;
}

// The big-enough freelist index is the index of the freelist of
// items that are all big enough for us. This is computed using
// an upper bound power of 2.
static size_t getBigEnoughFreeListIndex(size_t size) {
  assert((1 << MIN_FREELIST_INDEX) == (ALLOC_UNIT >> 1));
  assert(size > 0);
  if (size < ALLOC_UNIT) size = ALLOC_UNIT;
  // Start with a lower bound.
  size_t index = lowerBoundPowerOf2(size);
  // If we weren't exact in the lower bound, add one.
  if (!isPowerOf2(size)) index++;
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.getBigEnoughFreeListIndex " + [$0, $1]) }, size, index);
#endif
  assert(MIN_FREELIST_INDEX <= index && index < MAX_FREELIST_INDEX);
  return index;
}

// Items in the freelist at this index must be smaller than this.
static size_t getMaxSizeForFreeListIndex(size_t index) {
  return 1 << (index + 1);
}

static void removeFromFreeList(Region* region) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.removeFromFreeList " + $0) },region);
#endif
  assert(!region->getUsed());
  size_t index = getFreeListIndex(getMaxPayload(region), getPayload(region));
  FreeInfo* freeInfo = &region->freeInfo();
  if (freeLists[index] == freeInfo) {
    freeLists[index] = freeInfo->next();
  }
  if (freeInfo->prev()) {
    freeInfo->prev()->next() = freeInfo->next();
  }
  if (freeInfo->next()) {
    freeInfo->next()->prev() = freeInfo->prev();
  }
}

static void addToFreeList(Region* region) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.addToFreeList " + $0) }, region);
#endif
  assert(!region->getUsed());
  size_t index = getFreeListIndex(getMaxPayload(region), getPayload(region));
  FreeInfo* freeInfo = &region->freeInfo();
  FreeInfo* last = freeLists[index];
  freeLists[index] = freeInfo;
  freeInfo->prev() = NULL;
  freeInfo->next() = last;
  if (last) {
    last->prev() = freeInfo;
  }
}

// Receives a region that has just become free (and is not yet in a freelist).
// Tries to merge it into a region before or after it to which it is adjacent.
static int mergeIntoExistingFreeRegion(Region* region) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.mergeIntoExistingFreeRegion " + $0) }, region);
#endif
  int merged = 0;
  Region* prev = region->prev();
  Region* next = region->next();
  if (prev && !prev->getUsed() && isAlignedForSize(getPayload(prev), getMaxPayload(prev) + region->getTotalSize())) {
    // Merge them.
#ifdef EMMALLOC_DEBUG_LOG
    EM_ASM({ Module.print("  emmalloc.mergeIntoExistingFreeRegion merge into prev " + $0) }, prev);
#endif
    removeFromFreeList(prev);
    prev->incTotalSize(region->getTotalSize());
    if (next) {
      next->prev() = prev; // was: region
    } else {
      assert(region == lastRegion);
      lastRegion = prev;
    }
    if (next) {
      // We may also be able to merge with the next, keep trying.
      if (!next->getUsed() && isAlignedForSize(getPayload(prev), getMaxPayload(prev) + next->getTotalSize())) {
#ifdef EMMALLOC_DEBUG_LOG
        EM_ASM({ Module.print("  emmalloc.mergeIntoExistingFreeRegion also merge into next " + $0) }, next);
#endif
        removeFromFreeList(next);
        prev->incTotalSize(next->getTotalSize());
        if (next != lastRegion) {
          next->next()->prev() = prev;
        } else {
          lastRegion = prev;
        }
      }
    }
    addToFreeList(prev);
    return 1;
  }
  if (next && !next->getUsed() && isAlignedForSize(getPayload(region), getMaxPayload(region) + next->getTotalSize())) {
#ifdef EMMALLOC_DEBUG_LOG
    EM_ASM({ Module.print("  emmalloc.mergeIntoExistingFreeRegion merge into next " + $0) }, next);
#endif
    // Merge them.
    removeFromFreeList(next);
    region->incTotalSize(next->getTotalSize());
    if (next != lastRegion) {
      next->next()->prev() = region;
    } else {
      lastRegion = region;
    }
    addToFreeList(region);
    return 1;
  }
  return 0;
}

static void stopUsing(Region* region) {
  region->setUsed(0);
  if (!mergeIntoExistingFreeRegion(region)) {
    addToFreeList(region);
  }
}

// Extends the last region's payload to a certain size. Returns 1 if successful,
// 0 if an error occurred in sbrk().
static int extendLastRegion(size_t size) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.extendLastRegionToSize " + $0) }, size);
#endif
  size_t reusable = getMaxPayload(lastRegion);
  size_t sbrkSize = alignUp(size) - reusable;
  void* ptr = sbrk(sbrkSize);
  if (ptr == (void*)-1) {
    // sbrk() failed, we failed.
#ifdef EMMALLOC_DEBUG_LOG
   EM_ASM({ Module.print("  emmalloc.extendLastRegion sbrk failure") });
#endif
    return 0;
  }
  // sbrk() should give us new space right after the last region.
  assert(ptr == getAfter(lastRegion));
  lastRegion->incTotalSize(sbrkSize);
  lastRegion->setUsed(1);
  return 1;
}

static void possiblySplitRemainder(Region* region, size_t size) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.possiblySplitRemainder " + [$0, $1]) }, region, size);
#endif
  size = alignUp(size);
  size_t payloadSize = getMaxPayload(region);
  assert(payloadSize >= size);
  size_t splittableSize = payloadSize - size;
  // If we don't have the bare minimum, give up now.
  if (splittableSize < ALLOC_UNIT) return;
  // Compute where the split-off region would be.
  char* payload = (char*)getPayload(region);
  Region* split = (Region*)alignUpForSize(payload + size, splittableSize);
  // Alignment might change things (moving the region up to an aligned
  // address means less bytes to split off).
  splittableSize = size_t(payload) + payloadSize - (size_t)split;
  // Room for a minimal region is definitely worth splitting. Otherwise,
  // if we don't have room for a full region, but we do have an allocation
  // unit's worth, and we are the last region, it's worth allocating some
  // more memory to create a region here. The next allocation can reuse it,
  // which is better than leaving it as unused and unreusable space at the
  // end of this region.
  if (region == lastRegion && splittableSize >= ALLOC_UNIT && splittableSize < MIN_REGION_SIZE) {
    // Yes, this is a small-but-useful amount of memory in the final region,
    // extend it.
#ifdef EMMALLOC_DEBUG_LOG
    EM_ASM({ Module.print("    emmalloc.possiblySplitRemainder pre-extending") });
#endif
    // We'll need at least this much.
    size_t extra = ALLOC_UNIT;
    splittableSize += extra;
    // The change should not make us unaligned.
    assert(split == alignUpForSize(payload + size, splittableSize));
    if (extendLastRegion(METADATA_SIZE + payloadSize + extra)) {
      // Success.
      splittableSize += extra;
      assert(splittableSize >= MIN_REGION_SIZE);
    } else {
      return;
    }
  } else {
    // If it's not worth it, stop.
    if (splittableSize < MIN_REGION_SIZE) {
      return;
    }
  }
  // Worth it, split the region
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("    emmalloc.possiblySplitRemainder is splitting") });
#endif
  assert(splittableSize >= MIN_REGION_SIZE);
  region->setTotalSize((char*)split - (char*)region);
  split->setTotalSize(splittableSize);
  split->prev() = region;
  if (region != lastRegion) {
    split->next()->prev() = split;
  } else {
    lastRegion = split;
  }
  stopUsing(split);
}

// Sets the used payload of a region, and does other necessary work when
// starting to use a region, such as splitting off a remainder if there is
// any.
static void useRegion(Region* region, size_t size) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.useRegion " + [$0, $1]) }, region, size);
#endif
  assert(size > 0);
  region->setUsed(1);
  // We may not be using all of it, split out a smaller
  // region into a free list if it's large enough.
  possiblySplitRemainder(region, size);
}

static Region* useFreeInfo(FreeInfo* freeInfo, size_t size) {
  Region* region = fromFreeInfo(freeInfo);
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.useFreeInfo " + [$0, $1]) }, region, size);
#endif
  assert(isAlignedForSize(getPayload(region), size));
  // This region is no longer free
  removeFromFreeList(region);
  // This region is now in use
  useRegion(region, size);
  return region;
}

// Debugging

// Mostly for testing purposes, wipes everything.
EMMALLOC_EXPORT
void emmalloc_blank_slate_from_orbit() {
  for (int i = 0; i < MAX_FREELIST_INDEX; i++) {
    freeLists[i] = NULL;
  }
  firstRegion = NULL;
  lastRegion = NULL;
}

#ifdef EMMALLOC_DEBUG
// For testing purposes, validate a region.
static void emmalloc_validate_region(Region* region) {
  assert(getAfter(region) <= sbrk(0));
  assert(getMaxPayload(region) < region->getTotalSize());
  if (region->prev()) {
    assert(getAfter(region->prev()) == region);
    assert(region->prev()->next() == region);
  }
  if (region->next()) {
    assert(getAfter(region) == region->next());
    assert(region->next()->prev() == region);
  }
}

// For testing purposes, check that everything is valid.
static void emmalloc_validate_all() {
  void* end = sbrk(0);
  // Validate regions.
  Region* curr = firstRegion;
  Region* prev = NULL;
  EM_ASM({
    Module.emmallocDebug = {
      regions: {}
    };
  });
  while (curr) {
    // Note all region, so we can see freelist items are in the main list.
    EM_ASM({
      var region = $0;
      assert(!Module.emmallocDebug.regions[region], "dupe region");
      Module.emmallocDebug.regions[region] = 1;
    }, curr);
    assert(curr->prev() == prev);
    size_t maxPayload = getMaxPayload(curr);
    assert(maxPayload >= ALLOC_UNIT);
    if (prev) {
      assert(getAfter(prev) == curr);
      // Adjacent free regions must be merged.
      assert(!(!prev->getUsed() && !curr->getUsed()));
    }
    assert(getAfter(curr) <= end);
    prev = curr;
    curr = curr->next();
  }
  if (prev) {
    assert(prev == lastRegion);
  } else {
    assert(!lastRegion);
  }
  if (lastRegion) {
    assert(getAfter(lastRegion) == end);
  }
  // Validate freelists.
  for (int i = 0; i < MAX_FREELIST_INDEX; i++) {
    FreeInfo* curr = freeLists[i];
    if (!curr) continue;
    FreeInfo* prev = NULL;
    while (curr) {
      assert(curr->prev() == prev);
      Region* region = fromFreeInfo(curr);
      // Regions must be in the main list.
      EM_ASM({
        var region = $0;
        assert(Module.emmallocDebug.regions[region], "free region not in list");
      }, region);
      assert(getAfter(region) <= end);
      assert(getMaxPayload(region) >= getMinSizeForFreeListIndex(i));
      if (getMaxPayload(region) >= getMaxSizeForFreeListIndex(i)) {
        // We may put a larger item in a lower index depending on alignment.
        assert(!isAlignedForSize(getPayload(region), getMinSizeForFreeListIndex(i + 1)));
      }
      assert(isAlignedForSize(getPayload(region), getMinSizeForFreeListIndex(i)));
      prev = curr;
      curr = curr->next();
    }
  }
  // Validate lastRegion.
  if (lastRegion) {
    assert(lastRegion->next() == NULL);
    assert(getAfter(lastRegion) <= end);
    assert(firstRegion);
  } else {
    assert(!firstRegion);
  }
}

#ifdef EMMALLOC_DEBUG_LOG
// For testing purposes, dump out a region.
static void emmalloc_dump_region(Region* region) {
  EM_ASM({ Module.print("      [" + $0 + " - " + $1 + " (" + $2 + " bytes" + ($3 ? ", used" : "") + ")]") },
         region, getAfter(region), getMaxPayload(region), region->getUsed());
}

// For testing purposes, dumps out the entire global state.
static void emmalloc_dump_all() {
  EM_ASM({ Module.print("  emmalloc_dump_all:\n    sbrk(0) = " + $0) }, sbrk(0));
  Region* curr = firstRegion;
  EM_ASM({ Module.print("    all regions:") });
  while (curr) {
    emmalloc_dump_region(curr);
    curr = curr->next();
  }
  for (int i = 0; i < MAX_FREELIST_INDEX; i++) {
    FreeInfo* curr = freeLists[i];
    if (!curr) continue;
    EM_ASM({ Module.print("    freeList[" + $0 + "] sizes: [" + $1 + ", " + $2 + ")") }, i, getMinSizeForFreeListIndex(i), getMaxSizeForFreeListIndex(i));
    FreeInfo* prev = NULL;
    while (curr) {
      Region* region = fromFreeInfo(curr);
      emmalloc_dump_region(region);
      prev = curr;
      curr = curr->next();
    }
  }
}
#endif // EMMALLOC_DEBUG_LOG
#endif // EMMALLOC_DEBUG

// When we free something of size 100, we put it in the
// freelist for items of size 64 and above. Then when something
// needs 64 bytes, we know the things in that list are all
// suitable. However, note that this means that if we then
// try to allocate something of size 100 once more, we will
// look in the freelist for items of size 128 or more (again,
// so we know all items in the list are big enough), which means
// we may not reuse the perfect region we just freed. It's hard
// to do a perfect job on that without a lot more work (memory
// and/or time), so instead, we use a simple heuristic to look
// at the one-lower freelist, which *may* contain something
// big enough for us. We look at just a few elements, but that is
// enough if we are alloating/freeing a lot of such elements
// (since the recent items are there).
// TODO: Consider more optimizations, e.g. slow bubbling of larger
//       items in each freelist towards the root, or even actually
//       keep it sorted by size.
// Consider also what happens to the very largest allocations,
// 2^32 - a little. That goes in the freelist of items of size
// 2^31 or less. >2 tries is enough to go through that entire
// freelist because even 2 can't exist, they'd exhaust memory
// (together with metadata overhead). So we should be able to
// free and allocate such largest allocations (barring fragmentation
// happening in between).
static const size_t SPECULATIVE_FREELIST_TRIES = 32;

static Region* tryFromFreeList(size_t size) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.tryFromFreeList " + $0) }, size);
#endif
  // Look in the freelist of items big enough for us.
  size_t index = getBigEnoughFreeListIndex(size);
  // If we *may* find an item in the index one
  // below us, try that briefly in constant time;
  // see comment on algorithm on the declaration of
  // SPECULATIVE_FREELIST_TRIES.
  if (index > MIN_FREELIST_INDEX &&
      size < getMinSizeForFreeListIndex(index)) {
    FreeInfo* freeInfo = freeLists[index - 1];
    size_t tries = 0;
    while (freeInfo && tries < SPECULATIVE_FREELIST_TRIES) {
      Region* region = fromFreeInfo(freeInfo);
      // See if it's big enough and aligned enough.
      if (getMaxPayload(region) >= size &&
          isAlignedForSize(getPayload(region), size)) {
        // Success, use it
#ifdef EMMALLOC_DEBUG_LOG
        EM_ASM({ Module.print("  emmalloc.tryFromFreeList try succeeded") });
#endif
        return useFreeInfo(freeInfo, size);
      }
      freeInfo = freeInfo->next();
      tries++;
    }
  }
  // Note that index may start out at MAX_FREELIST_INDEX,
  // if it is almost the largest allocation possible,
  // 2^32 minus a little. In that case, looking in the lower
  // freelist is our only hope, and it can contain at most 1
  // element (see discussion above), so we will find it if
  // it's there). If not, and we got here, we'll never enter
  // the loop at all.
  while (index < MAX_FREELIST_INDEX) {
    FreeInfo* freeInfo = freeLists[index];
    if (freeInfo) {
      // We found one, use it.
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("  emmalloc.tryFromFreeList had item to use") });
#endif
      return useFreeInfo(freeInfo, size);
    }
    // Look in a freelist of larger elements.
    // TODO This does increase the risk of fragmentation, though,
    //      and maybe the iteration adds runtime overhead.
    index++;
  }
  // No luck, no free list.
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.tryFromFreeList no luck") });
#endif
  return NULL;
}

static Region* newAllocation(size_t size) {
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("  emmalloc.newAllocation " + $0) }, size);
#endif
  assert(size > 0);
  if (lastRegion) {
    // If the last region is free, we can extend it rather than leave it
    // as fragmented free spce between allocated regions. This is also
    // more efficient and simple as well.
    if (!lastRegion->getUsed()) {
#ifdef EMMALLOC_DEBUG_LOG
     EM_ASM({ Module.print("    emmalloc.newAllocation extending lastRegion at " + $0) }, lastRegion);
#endif
      // Remove it first, before we adjust the size (which affects which list
      // it should be in).
      removeFromFreeList(lastRegion);
      if (extendLastRegion(size)) {
        return lastRegion;
      } else {
        return NULL;
      }
    }
  }
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("    emmalloc.newAllocation getting brand new space") });
#endif
  size_t sbrkSize = METADATA_SIZE + alignUp(size);
  void* ptr = sbrk(sbrkSize);
  if (ptr == (void*)-1) {
    // sbrk() failed, we failed.
#ifdef EMMALLOC_DEBUG_LOG
    EM_ASM({ Module.print("    emmalloc.newAllocation sbrk failure") });
#endif
    return NULL;
  }
  // sbrk() results might not be aligned. We assume single-threaded sbrk()
  // access here in order to fix that up
  void* fixedPtr = alignUpForSize(ptr, size);
  if (ptr != fixedPtr) {
#ifdef EMMALLOC_DEBUG_LOG
    EM_ASM({ Module.print("    emmalloc.newAllocation fixing alignment") });
#endif
    size_t extra = (char*)fixedPtr - (char*)ptr;
    // Use sbrk() to get us more room in which to align.
    // TODO: we could save an sbrk() call, we can know this before.
    void* extraPtr = sbrk(extra);
    if (extraPtr == (void*)-1) {
      // sbrk() failed, we failed.
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("    emmalloc.newAllocation sbrk failure") });;
#endif
      return NULL;
    }
    // Verify the sbrk() assumption, no one else should call it.
    // If this fails, it means we also leak the previous allocation,
    // so we don't even try to handle it.
    assert((char*)extraPtr == (char*)ptr + sbrkSize);
    if (lastRegion) {
      // There is a previous region here. We must also extend it so that
      // the two are contiguous.
      lastRegion->incTotalSize(extra);
      // The only possible difference here is when the previous ends on a
      // 4-aligned address but we need 8.
      assert(extra == 4);
    }
  }
  Region* region = (Region*)fixedPtr;
  // Apply globally
  if (!lastRegion) {
    assert(!firstRegion);
    firstRegion = region;
    lastRegion = region;
  } else {
    assert(firstRegion);
    region->prev() = lastRegion;
    lastRegion = region;
  }
  // Success, we have new memory
  region->setTotalSize(sbrkSize);
  useRegion(region, size);
  return region;
}

// Internal mirror of public API.

static void* emmalloc_malloc(size_t size) {
  // malloc() spec defines malloc(0) => NULL.
  if (size == 0) return NULL;
  // Look in the freelist first.
  Region* region = tryFromFreeList(size);
  if (!region) {
    // Allocate some new memory otherwise.
    region = newAllocation(size);
    if (!region) {
      // We failed to allocate, sadly.
      return NULL;
    }
  }
  assert(getAfter(region) <= sbrk(0));
  return getPayload(region);
}

static void emmalloc_free(void *ptr) {
  if (ptr == NULL) return;
  stopUsing(fromPayload(ptr));
}

static void* emmalloc_calloc(size_t nmemb, size_t size) {
  // TODO If we know no one else is using sbrk(), we can assume that new
  //      memory allocations are zero'd out.
  void* ptr = emmalloc_malloc(nmemb * size);
  if (!ptr) return NULL;
  memset(ptr, 0, nmemb * size);
  return ptr;
}

static void* emmalloc_realloc(void *ptr, size_t size) {
  if (!ptr) return emmalloc_malloc(size);
  if (!size) {
    emmalloc_free(ptr);
    return NULL;
  }
  Region* region = fromPayload(ptr);
  // We may create a new region while trying to efficiently realloc.
  Region* newRegion = NULL;
  // If the current pointer is properly aligned for the new size, we may
  // be able to extend things in place.
  if (isAlignedForSize(ptr, size)) {
    // Grow it. First, maybe we can do simple growth in the current region.
    if (size <= getMaxPayload(region)) {
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("  emmalloc.emmalloc_realloc use existing payload space") });
#endif
      region->setUsed(1);
      // There might be enough left over to split out now.
      possiblySplitRemainder(region, size);
      return ptr;
    }
    // Perhaps right after us is free space we can merge to us.
    Region* next = region->next();
    if (next && !next->getUsed()) {
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("  emmalloc.emmalloc_realloc merge in next") });
#endif
      removeFromFreeList(next);
      region->incTotalSize(next->getTotalSize());
      if (next != lastRegion) {
        next->next()->prev() = region;
      } else {
        lastRegion = region;
      }
    }
    // We may now be big enough.
    if (size <= getMaxPayload(region)) {
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("  emmalloc.emmalloc_realloc use existing payload space after merge") });
#endif
      region->setUsed(1);
      // There might be enough left over to split out now.
      possiblySplitRemainder(region, size);
      return ptr;
    }
    // We still aren't big enough. If we are the last, we can extend ourselves - however, that
    // definitely means increasing the total sbrk(), and there may be free space lower down, so
    // this is a tradeoff between speed (avoid the memcpy) and space. It's not clear what's
    // better here; for now, check for free space first.
    newRegion = tryFromFreeList(size);
    if (!newRegion && region == lastRegion) {
#ifdef EMMALLOC_DEBUG_LOG
      EM_ASM({ Module.print("  emmalloc.emmalloc_realloc extend last region") });
#endif
      if (extendLastRegion(size)) {
        // It worked. We don't need the formerly free region.
        if (newRegion) {
          stopUsing(newRegion);
        }
        return ptr;
      } else {
        // If this failed, we can also try the normal
        // malloc path, which may find space in a freelist;
        // fall through.
      }
    }
  }
  // We need new space, and a copy
  if (!newRegion) {
    newRegion = newAllocation(size);
    if (!newRegion) return NULL;
  }
  memcpy(getPayload(newRegion), getPayload(region), size < getMaxPayload(region) ? size : getMaxPayload(region));
  stopUsing(region);
  return getPayload(newRegion);
}

static struct mallinfo emmalloc_mallinfo() {
	struct mallinfo info;
  info.arena = 0;
  info.ordblks = 0;
  info.smblks = 0;
  info.hblks = 0;
  info.hblkhd = 0;
  info.usmblks = 0;
  info.fsmblks = 0;
  info.uordblks = 0;
  info.ordblks = 0;
  info.keepcost = 0;
  if (firstRegion) {
    info.arena = (char*)sbrk(0) - (char*)firstRegion;
    Region* region = firstRegion;
    while (region) {
      if (region->getUsed()) {
        info.uordblks += getMaxPayload(region);
      } else {
        info.fordblks += getMaxPayload(region);
        info.ordblks++;
      }
      region = region->next();
    }
  }
  return info;
}

// Public API. This is a thin wrapper around our mirror of it, adding
// logging and validation when debugging. Otherwise it should inline
// out.

extern "C" {

EMMALLOC_EXPORT
void* malloc(size_t size) {
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.malloc " + $0) }, size);
#endif
  emmalloc_validate_all();
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
#endif
  void* ptr = emmalloc_malloc(size);
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.malloc ==> " + $0) }, ptr);
#endif
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
  emmalloc_validate_all();
#endif
  return ptr;
}

EMMALLOC_EXPORT
void free(void *ptr) {
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.free " + $0) }, ptr);
#endif
  emmalloc_validate_all();
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
#endif
  emmalloc_free(ptr);
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
  emmalloc_validate_all();
#endif
}

EMMALLOC_EXPORT
void* calloc(size_t nmemb, size_t size) {
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.calloc " + $0) }, size);
#endif
  emmalloc_validate_all();
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
#endif
  void* ptr = emmalloc_calloc(nmemb, size);
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.calloc ==> " + $0) }, ptr);
#endif
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
  emmalloc_validate_all();
#endif
  return ptr;
}

EMMALLOC_EXPORT
void* realloc(void *ptr, size_t size) {
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.realloc " + [$0, $1]) }, ptr, size);
#endif
  emmalloc_validate_all();
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
#endif
  void* newPtr = emmalloc_realloc(ptr, size);
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.realloc ==> " + $0) }, newPtr);
#endif
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
  emmalloc_validate_all();
#endif
  return newPtr;
}

EMMALLOC_EXPORT
struct mallinfo mallinfo() {
#ifdef EMMALLOC_DEBUG
#ifdef EMMALLOC_DEBUG_LOG
  EM_ASM({ Module.print("emmalloc.mallinfo") });
#endif
  emmalloc_validate_all();
#ifdef EMMALLOC_DEBUG_LOG
  emmalloc_dump_all();
#endif
#endif
  return emmalloc_mallinfo();
}

// Export malloc and free as duplicate names emscripten_builtin_malloc and
// emscripten_builtin_free so that applications can replace malloc and free
// in their code, and make those replacements refer to the original malloc
// and free from this file.
// This allows an easy mechanism for hooking into memory allocation.
#if defined(__EMSCRIPTEN__) && !ONLY_MSPACES
extern __typeof(malloc) emscripten_builtin_malloc __attribute__((weak, alias("malloc")));
extern __typeof(free) emscripten_builtin_free __attribute__((weak, alias("free")));
#endif

} // extern "C"
