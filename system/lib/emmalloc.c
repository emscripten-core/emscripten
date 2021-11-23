/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Simple minimalistic but efficient sbrk()-based malloc/free that works in
 * singlethreaded and multithreaded builds.
 *
 * Assumptions:
 *
 *  - Pointers are 32-bit.
 *  - sbrk() is used to claim new memory (sbrk handles geometric/linear
 *  - overallocation growth)
 *  - sbrk() can be used by other code outside emmalloc.
 *  - sbrk() is very fast in most cases (internal wasm call).
 *
 * Invariants:
 *
 *  - Per-allocation header overhead is 8 bytes, smallest allocated payload
 *    amount is 8 bytes, and a multiple of 4 bytes.
 *  - Acquired memory blocks are subdivided into disjoint regions that lie
 *    next to each other.
 *  - A region is either in used or free.
 *    Used regions may be adjacent, and a used and unused region
 *    may be adjacent, but not two unused ones - they would be
 *    merged.
 *  - Memory allocation takes constant time, unless the alloc needs to sbrk()
 *    or memory is very close to being exhausted.
 *
 * Debugging:
 *
 *  - If not NDEBUG, runtime assert()s are in use.
 *  - If EMMALLOC_MEMVALIDATE is defined, a large amount of extra checks are done.
 *  - If EMMALLOC_VERBOSE is defined, a lot of operations are logged
 *    out, in addition to EMMALLOC_MEMVALIDATE.
 *  - Debugging and logging directly uses console.log via uses EM_ASM, not
 *    printf etc., to minimize any risk of debugging or logging depending on
 *    malloc.
 */

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <assert.h>
#include <malloc.h>
#include <emscripten.h>
#include <emscripten/heap.h>
#include <emscripten/threading.h>

#ifdef __EMSCRIPTEN_TRACING__
#include <emscripten/trace.h>
#endif

// Behavior of right shifting a signed integer is compiler implementation defined.
static_assert((((int32_t)0x80000000U) >> 31) == -1, "This malloc implementation requires that right-shifting a signed integer produces a sign-extending (arithmetic) shift!");

// Configuration: specifies the minimum alignment that malloc()ed memory outputs. Allocation requests with smaller alignment
// than this will yield an allocation with this much alignment.
#define MALLOC_ALIGNMENT alignof(max_align_t)
static_assert(alignof(max_align_t) == 8, "max_align_t must be correct");

#define EMMALLOC_EXPORT __attribute__((weak, __visibility__("default")))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define NUM_FREE_BUCKETS 64
#define BUCKET_BITMASK_T uint64_t

// Dynamic memory is subdivided into regions, in the format

// <size:uint32_t> ..... <size:uint32_t> | <size:uint32_t> ..... <size:uint32_t> | <size:uint32_t> ..... <size:uint32_t> | .....

// That is, at the bottom and top end of each memory region, the size of that region is stored. That allows traversing the
// memory regions backwards and forwards. Because each allocation must be at least a multiple of 4 bytes, the lowest two bits of
// each size field is unused. Free regions are distinguished by used regions by having the FREE_REGION_FLAG bit present
// in the size field. I.e. for free regions, the size field is odd, and for used regions, the size field reads even.
#define FREE_REGION_FLAG 0x1u

// Attempts to malloc() more than this many bytes would cause an overflow when calculating the size of a region,
// therefore allocations larger than this are short-circuited immediately on entry.
#define MAX_ALLOC_SIZE 0xFFFFFFC7u

// A free region has the following structure:
// <size:uint32_t> <prevptr> <nextptr> ... <size:uint32_t>

typedef struct Region
{
  uint32_t size;
  // Use a circular doubly linked list to represent free region data.
  struct Region *prev, *next;
  // ... N bytes of free data
  uint32_t _at_the_end_of_this_struct_size; // do not dereference, this is present for convenient struct sizeof() computation only
} Region;

#if defined(__EMSCRIPTEN_PTHREADS__)
// In multithreaded builds, use a simple global spinlock strategy to acquire/release access to the memory allocator.
static volatile uint8_t multithreadingLock = 0;
#define MALLOC_ACQUIRE() while(__sync_lock_test_and_set(&multithreadingLock, 1)) { while(multithreadingLock) { /*nop*/ } }
#define MALLOC_RELEASE() __sync_lock_release(&multithreadingLock)
// Test code to ensure we have tight malloc acquire/release guards in place.
#define ASSERT_MALLOC_IS_ACQUIRED() assert(multithreadingLock == 1)
#else
// In singlethreaded builds, no need for locking.
#define MALLOC_ACQUIRE() ((void)0)
#define MALLOC_RELEASE() ((void)0)
#define ASSERT_MALLOC_IS_ACQUIRED() ((void)0)
#endif

#define IS_POWER_OF_2(val) (((val) & ((val)-1)) == 0)
#define ALIGN_UP(ptr, alignment) ((uint8_t*)((((uintptr_t)(ptr)) + ((alignment)-1)) & ~((alignment)-1)))
#define HAS_ALIGNMENT(ptr, alignment) ((((uintptr_t)(ptr)) & ((alignment)-1)) == 0)

static_assert(IS_POWER_OF_2(MALLOC_ALIGNMENT), "MALLOC_ALIGNMENT must be a power of two value!");
static_assert(MALLOC_ALIGNMENT >= 4, "Smallest possible MALLOC_ALIGNMENT if 4!");

// A region that contains as payload a single forward linked list of pointers to head regions of each disjoint region blocks.
static Region *listOfAllRegions = 0;

// For each of the buckets, maintain a linked list head node. The head node for each
// free region is a sentinel node that does not actually represent any free space, but
// the sentinel is used to avoid awkward testing against (if node == freeRegionHeadNode)
// when adding and removing elements from the linked list, i.e. we are guaranteed that
// the sentinel node is always fixed and there, and the actual free region list elements
// start at freeRegionBuckets[i].next each.
static Region freeRegionBuckets[NUM_FREE_BUCKETS];

// A bitmask that tracks the population status for each of the 32 distinct memory regions:
// a zero at bit position i means that the free list bucket i is empty. This bitmask is
// used to avoid redundant scanning of the 32 different free region buckets: instead by
// looking at the bitmask we can find in constant time an index to a free region bucket
// that contains free memory of desired size.
static BUCKET_BITMASK_T freeRegionBucketsUsed = 0;

// Amount of bytes taken up by allocation header data
#define REGION_HEADER_SIZE (2*sizeof(uint32_t))

// Smallest allocation size that is possible is 2*pointer size, since payload of each region must at least contain space
// to store the free region linked list prev and next pointers. An allocation size smaller than this will be rounded up
// to this size.
#define SMALLEST_ALLOCATION_SIZE (2*sizeof(void*))

/* Subdivide regions of free space into distinct circular doubly linked lists, where each linked list
represents a range of free space blocks. The following function compute_free_list_bucket() converts
an allocation size to the bucket index that should be looked at. The buckets are grouped as follows:

  Bucket 0: [8, 15], range size=8
  Bucket 1: [16, 23], range size=8
  Bucket 2: [24, 31], range size=8
  Bucket 3: [32, 39], range size=8
  Bucket 4: [40, 47], range size=8
  Bucket 5: [48, 55], range size=8
  Bucket 6: [56, 63], range size=8
  Bucket 7: [64, 71], range size=8
  Bucket 8: [72, 79], range size=8
  Bucket 9: [80, 87], range size=8
  Bucket 10: [88, 95], range size=8
  Bucket 11: [96, 103], range size=8
  Bucket 12: [104, 111], range size=8
  Bucket 13: [112, 119], range size=8
  Bucket 14: [120, 159], range size=40
  Bucket 15: [160, 191], range size=32
  Bucket 16: [192, 223], range size=32
  Bucket 17: [224, 255], range size=32
  Bucket 18: [256, 319], range size=64
  Bucket 19: [320, 383], range size=64
  Bucket 20: [384, 447], range size=64
  Bucket 21: [448, 511], range size=64
  Bucket 22: [512, 639], range size=128
  Bucket 23: [640, 767], range size=128
  Bucket 24: [768, 895], range size=128
  Bucket 25: [896, 1023], range size=128
  Bucket 26: [1024, 1279], range size=256
  Bucket 27: [1280, 1535], range size=256
  Bucket 28: [1536, 1791], range size=256
  Bucket 29: [1792, 2047], range size=256
  Bucket 30: [2048, 2559], range size=512
  Bucket 31: [2560, 3071], range size=512
  Bucket 32: [3072, 3583], range size=512
  Bucket 33: [3584, 6143], range size=2560
  Bucket 34: [6144, 8191], range size=2048
  Bucket 35: [8192, 12287], range size=4096
  Bucket 36: [12288, 16383], range size=4096
  Bucket 37: [16384, 24575], range size=8192
  Bucket 38: [24576, 32767], range size=8192
  Bucket 39: [32768, 49151], range size=16384
  Bucket 40: [49152, 65535], range size=16384
  Bucket 41: [65536, 98303], range size=32768
  Bucket 42: [98304, 131071], range size=32768
  Bucket 43: [131072, 196607], range size=65536
  Bucket 44: [196608, 262143], range size=65536
  Bucket 45: [262144, 393215], range size=131072
  Bucket 46: [393216, 524287], range size=131072
  Bucket 47: [524288, 786431], range size=262144
  Bucket 48: [786432, 1048575], range size=262144
  Bucket 49: [1048576, 1572863], range size=524288
  Bucket 50: [1572864, 2097151], range size=524288
  Bucket 51: [2097152, 3145727], range size=1048576
  Bucket 52: [3145728, 4194303], range size=1048576
  Bucket 53: [4194304, 6291455], range size=2097152
  Bucket 54: [6291456, 8388607], range size=2097152
  Bucket 55: [8388608, 12582911], range size=4194304
  Bucket 56: [12582912, 16777215], range size=4194304
  Bucket 57: [16777216, 25165823], range size=8388608
  Bucket 58: [25165824, 33554431], range size=8388608
  Bucket 59: [33554432, 50331647], range size=16777216
  Bucket 60: [50331648, 67108863], range size=16777216
  Bucket 61: [67108864, 100663295], range size=33554432
  Bucket 62: [100663296, 134217727], range size=33554432
  Bucket 63: 134217728 bytes and larger. */
static_assert(NUM_FREE_BUCKETS == 64, "Following function is tailored specifically for NUM_FREE_BUCKETS == 64 case");
static int compute_free_list_bucket(uint32_t allocSize)
{
  if (allocSize < 128) return (allocSize >> 3) - 1;
  int clz = __builtin_clz(allocSize);
  int bucketIndex = (clz > 19) ? 110 - (clz<<2) + ((allocSize >> (29-clz)) ^ 4) : MIN(71 - (clz<<1) + ((allocSize >> (30-clz)) ^ 2), NUM_FREE_BUCKETS-1);
  assert(bucketIndex >= 0);
  assert(bucketIndex < NUM_FREE_BUCKETS);
  return bucketIndex;
}

#define DECODE_CEILING_SIZE(size) ((uint32_t)((size) & ~FREE_REGION_FLAG))

static Region *prev_region(Region *region)
{
  uint32_t prevRegionSize = ((uint32_t*)region)[-1];
  prevRegionSize = DECODE_CEILING_SIZE(prevRegionSize);
  return (Region*)((uint8_t*)region - prevRegionSize);
}

static Region *next_region(Region *region)
{
  return (Region*)((uint8_t*)region + region->size);
}

static uint32_t region_ceiling_size(Region *region)
{
  return ((uint32_t*)((uint8_t*)region + region->size))[-1];
}

static bool region_is_free(Region *r)
{
  return region_ceiling_size(r) & FREE_REGION_FLAG;
}

static bool region_is_in_use(Region *r)
{
  return r->size == region_ceiling_size(r);
}

static uint32_t size_of_region_from_ceiling(Region *r)
{
  uint32_t size = region_ceiling_size(r);
  return DECODE_CEILING_SIZE(size);
}

static bool debug_region_is_consistent(Region *r)
{
  assert(r);
  uint32_t sizeAtBottom = r->size;
  uint32_t sizeAtCeiling = size_of_region_from_ceiling(r);
  return sizeAtBottom == sizeAtCeiling;
}

static uint8_t *region_payload_start_ptr(Region *region)
{
  return (uint8_t*)region + sizeof(uint32_t);
}

static uint8_t *region_payload_end_ptr(Region *region)
{
  return (uint8_t*)region + region->size - sizeof(uint32_t);
}

static void create_used_region(void *ptr, uint32_t size)
{
  assert(ptr);
  assert(HAS_ALIGNMENT(ptr, sizeof(uint32_t)));
  assert(HAS_ALIGNMENT(size, sizeof(uint32_t)));
  assert(size >= sizeof(Region));
  *(uint32_t*)ptr = size;
  ((uint32_t*)ptr)[(size>>2)-1] = size;
}

static void create_free_region(void *ptr, uint32_t size)
{
  assert(ptr);
  assert(HAS_ALIGNMENT(ptr, sizeof(uint32_t)));
  assert(HAS_ALIGNMENT(size, sizeof(uint32_t)));
  assert(size >= sizeof(Region));
  Region *freeRegion = (Region*)ptr;
  freeRegion->size = size;
  ((uint32_t*)ptr)[(size>>2)-1] = size | FREE_REGION_FLAG;
}

static void prepend_to_free_list(Region *region, Region *prependTo)
{
  assert(region);
  assert(prependTo);
  // N.b. the region we are prepending to is always the sentinel node,
  // which represents a dummy node that is technically not a free node, so
  // region_is_free(prependTo) does not hold.
  assert(region_is_free((Region*)region));
  region->next = prependTo;
  region->prev = prependTo->prev;
  assert(region->prev);
  prependTo->prev = region;
  region->prev->next = region;
}

static void unlink_from_free_list(Region *region)
{
  assert(region);
  assert(region_is_free((Region*)region));
  assert(region->prev);
  assert(region->next);
  region->prev->next = region->next;
  region->next->prev = region->prev;
}

static void link_to_free_list(Region *freeRegion)
{
  assert(freeRegion);
  assert(freeRegion->size >= sizeof(Region));
  int bucketIndex = compute_free_list_bucket(freeRegion->size-REGION_HEADER_SIZE);
  Region *freeListHead = freeRegionBuckets + bucketIndex;
  freeRegion->prev = freeListHead;
  freeRegion->next = freeListHead->next;
  assert(freeRegion->next);
  freeListHead->next = freeRegion;
  freeRegion->next->prev = freeRegion;
  freeRegionBucketsUsed |= ((BUCKET_BITMASK_T)1) << bucketIndex;
}

static void dump_memory_regions()
{
  ASSERT_MALLOC_IS_ACQUIRED();
  Region *root = listOfAllRegions;
  MAIN_THREAD_ASYNC_EM_ASM(console.log('All memory regions:'));
  while(root)
  {
    Region *r = root;
    assert(debug_region_is_consistent(r));
    uint8_t *lastRegionEnd = (uint8_t*)(((uint32_t*)root)[2]);
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Region block 0x'+($0>>>0).toString(16)+' - 0x'+($1>>>0).toString(16)+ ' ('+($2>>>0)+' bytes):'),
      r, lastRegionEnd, lastRegionEnd-(uint8_t*)r);
    while((uint8_t*)r < lastRegionEnd)
    {
      MAIN_THREAD_ASYNC_EM_ASM(console.log('Region 0x'+($0>>>0).toString(16)+', size: '+($1>>>0)+' ('+($2?"used":"--FREE--")+')'),
        r, r->size, region_ceiling_size(r) == r->size);

      assert(debug_region_is_consistent(r));
      uint32_t sizeFromCeiling = size_of_region_from_ceiling(r);
      if (sizeFromCeiling != r->size)
        MAIN_THREAD_ASYNC_EM_ASM(console.log('Corrupt region! Size marker at the end of the region does not match: '+($0>>>0)), sizeFromCeiling);
      if (r->size == 0)
        break;
      r = next_region(r);
    }
    root = ((Region*)((uint32_t*)root)[1]);
    MAIN_THREAD_ASYNC_EM_ASM(console.log(""));
  }
  MAIN_THREAD_ASYNC_EM_ASM(console.log('Free regions:'));
  for(int i = 0; i < NUM_FREE_BUCKETS; ++i)
  {
    Region *prev = &freeRegionBuckets[i];
    Region *fr = freeRegionBuckets[i].next;
    while(fr != &freeRegionBuckets[i])
    {
      MAIN_THREAD_ASYNC_EM_ASM(console.log('In bucket '+$0+', free region 0x'+($1>>>0).toString(16)+', size: ' + ($2>>>0) + ' (size at ceiling: '+($3>>>0)+'), prev: 0x' + ($4>>>0).toString(16) + ', next: 0x' + ($5>>>0).toString(16)),
        i, fr, fr->size, size_of_region_from_ceiling(fr), fr->prev, fr->next);
      assert(debug_region_is_consistent(fr));
      assert(region_is_free(fr));
      assert(fr->prev == prev);
      prev = fr;
      assert(fr->next != fr);
      assert(fr->prev != fr);
      fr = fr->next;
    }
  }
  MAIN_THREAD_ASYNC_EM_ASM(console.log('Free bucket index map: ' + ($0>>>0).toString(2) + ' ' + ($1>>>0).toString(2)), (uint32_t)(freeRegionBucketsUsed >> 32), (uint32_t)freeRegionBucketsUsed);
  MAIN_THREAD_ASYNC_EM_ASM(console.log(""));
}

void emmalloc_dump_memory_regions()
{
  MALLOC_ACQUIRE();
  dump_memory_regions();
  MALLOC_RELEASE();
}

static int validate_memory_regions()
{
  ASSERT_MALLOC_IS_ACQUIRED();
  Region *root = listOfAllRegions;
  while(root)
  {
    Region *r = root;
    if (!debug_region_is_consistent(r))
    {
      MAIN_THREAD_ASYNC_EM_ASM(console.error('Used region 0x'+($0>>>0).toString(16)+', size: '+($1>>>0)+' ('+($2?"used":"--FREE--")+') is corrupt (size markers in the beginning and at the end of the region do not match!)'),
        r, r->size, region_ceiling_size(r) == r->size);
      return 1;
    }
    uint8_t *lastRegionEnd = (uint8_t*)(((uint32_t*)root)[2]);
    while((uint8_t*)r < lastRegionEnd)
    {
      if (!debug_region_is_consistent(r))
      {
        MAIN_THREAD_ASYNC_EM_ASM(console.error('Used region 0x'+($0>>>0).toString(16)+', size: '+($1>>>0)+' ('+($2?"used":"--FREE--")+') is corrupt (size markers in the beginning and at the end of the region do not match!)'),
          r, r->size, region_ceiling_size(r) == r->size);
        return 1;
      }
      if (r->size == 0)
        break;
      r = next_region(r);
    }
    root = ((Region*)((uint32_t*)root)[1]);
  }
  for(int i = 0; i < NUM_FREE_BUCKETS; ++i)
  {
    Region *prev = &freeRegionBuckets[i];
    Region *fr = freeRegionBuckets[i].next;
    while(fr != &freeRegionBuckets[i])
    {
      if (!debug_region_is_consistent(fr) || !region_is_free(fr) || fr->prev != prev || fr->next == fr || fr->prev == fr)
      {
        MAIN_THREAD_ASYNC_EM_ASM(console.log('In bucket '+$0+', free region 0x'+($1>>>0).toString(16)+', size: ' + ($2>>>0) + ' (size at ceiling: '+($3>>>0)+'), prev: 0x' + ($4>>>0).toString(16) + ', next: 0x' + ($5>>>0).toString(16) + ' is corrupt!'),
          i, fr, fr->size, size_of_region_from_ceiling((Region*)fr), fr->prev, fr->next);
        return 1;
      }
      prev = fr;
      fr = fr->next;
    }
  }
  return 0;
}

int emmalloc_validate_memory_regions()
{
  MALLOC_ACQUIRE();
  int memoryError = validate_memory_regions();
  MALLOC_RELEASE();
  return memoryError;
}

static bool claim_more_memory(size_t numBytes)
{
#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('claim_more_memory(numBytes='+($0>>>0)+ ')'), numBytes);
#endif

#ifdef EMMALLOC_MEMVALIDATE
  validate_memory_regions();
#endif

  // Claim memory via sbrk
  uint8_t *startPtr = (uint8_t*)sbrk(numBytes);
  if ((intptr_t)startPtr == -1)
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.error('claim_more_memory: sbrk failed!'));
#endif
    return false;
  }
#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('claim_more_memory: claimed 0x' + ($0>>>0).toString(16) + ' - 0x' + ($1>>>0).toString(16) + ' (' + ($2>>>0) + ' bytes) via sbrk()'), startPtr, startPtr + numBytes, numBytes);
#endif
  assert(HAS_ALIGNMENT(startPtr, 4));
  uint8_t *endPtr = startPtr + numBytes;

  // Create a sentinel region at the end of the new heap block
  Region *endSentinelRegion = (Region*)(endPtr - sizeof(Region));
  create_used_region(endSentinelRegion, sizeof(Region));

  // If we are the sole user of sbrk(), it will feed us continuous/consecutive memory addresses - take advantage
  // of that if so: instead of creating two disjoint memory regions blocks, expand the previous one to a larger size.
  uint8_t *previousSbrkEndAddress = listOfAllRegions ? (uint8_t*)((uint32_t*)listOfAllRegions)[2] : 0;
  if (startPtr == previousSbrkEndAddress)
  {
    Region *prevEndSentinel = prev_region((Region*)startPtr);
    assert(debug_region_is_consistent(prevEndSentinel));
    assert(region_is_in_use(prevEndSentinel));
    Region *prevRegion = prev_region(prevEndSentinel);
    assert(debug_region_is_consistent(prevRegion));

    ((uint32_t*)listOfAllRegions)[2] = (uint32_t)endPtr;

    // Two scenarios, either the last region of the previous block was in use, in which case we need to create
    // a new free region in the newly allocated space; or it was free, in which case we can extend that region
    // to cover a larger size.
    if (region_is_free(prevRegion))
    {
      size_t newFreeRegionSize = (uint8_t*)endSentinelRegion - (uint8_t*)prevRegion;
      unlink_from_free_list(prevRegion);
      create_free_region(prevRegion, newFreeRegionSize);
      link_to_free_list(prevRegion);
      return true;
    }
    // else: last region of the previous block was in use. Since we are joining two consecutive sbrk() blocks,
    // we can swallow the end sentinel of the previous block away.
    startPtr -= sizeof(Region);
  }
  else
  {
    // Create a sentinel region at the start of the heap block
    create_used_region(startPtr, sizeof(Region));

    // Dynamic heap start region:
    Region *newRegionBlock = (Region*)startPtr;
    ((uint32_t*)newRegionBlock)[1] = (uint32_t)listOfAllRegions; // Pointer to next region block head
    ((uint32_t*)newRegionBlock)[2] = (uint32_t)endPtr; // Pointer to the end address of this region block
    listOfAllRegions = newRegionBlock;
    startPtr += sizeof(Region);
  }

  // Create a new memory region for the new claimed free space.
  create_free_region(startPtr, (uint8_t*)endSentinelRegion - startPtr);
  link_to_free_list((Region*)startPtr);
  return true;
}

// Initialize emmalloc during static initialization.
// See system/lib/README.md for static constructor ordering.
__attribute__((constructor(47)))
static void initialize_emmalloc_heap()
{
  // Initialize circular doubly linked lists representing free space
  // Never useful to unroll this for loop, just takes up code size.
#pragma clang loop unroll(disable)
  for(int i = 0; i < NUM_FREE_BUCKETS; ++i)
    freeRegionBuckets[i].prev = freeRegionBuckets[i].next = &freeRegionBuckets[i];

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('initialize_emmalloc_heap()'));
#endif

  // Start with a tiny dynamic region.
  claim_more_memory(3*sizeof(Region));
}

void emmalloc_blank_slate_from_orbit()
{
  MALLOC_ACQUIRE();
  listOfAllRegions = 0;
  freeRegionBucketsUsed = 0;
  initialize_emmalloc_heap();
  MALLOC_RELEASE();
}

static void *attempt_allocate(Region *freeRegion, size_t alignment, size_t size)
{
  ASSERT_MALLOC_IS_ACQUIRED();
  assert(freeRegion);
  // Look at the next potential free region to allocate into.
  // First, we should check if the free region has enough of payload bytes contained
  // in it to accommodate the new allocation. This check needs to take account the
  // requested allocation alignment, so the payload memory area needs to be rounded
  // upwards to the desired alignment.
  uint8_t *payloadStartPtr = region_payload_start_ptr(freeRegion);
  uint8_t *payloadStartPtrAligned = ALIGN_UP(payloadStartPtr, alignment);
  uint8_t *payloadEndPtr = region_payload_end_ptr(freeRegion);

  // Do we have enough free space, taking into account alignment?
  if (payloadStartPtrAligned + size > payloadEndPtr)
    return 0;

  // We have enough free space, so the memory allocation will be made into this region. Remove this free region
  // from the list of free regions: whatever slop remains will be later added back to the free region pool.
  unlink_from_free_list(freeRegion);

  // Before we proceed further, fix up the boundary of this region and the region that precedes this one,
  // so that the boundary between the two regions happens at a right spot for the payload to be aligned.
  if (payloadStartPtr != payloadStartPtrAligned)
  {
    Region *prevRegion = prev_region((Region*)freeRegion);
    // We never have two free regions adjacent to each other, so the region before this free
    // region should be in use.
    assert(region_is_in_use(prevRegion));
    size_t regionBoundaryBumpAmount = payloadStartPtrAligned - payloadStartPtr;
    size_t newThisRegionSize = freeRegion->size - regionBoundaryBumpAmount;
    create_used_region(prevRegion, prevRegion->size + regionBoundaryBumpAmount);
    freeRegion = (Region *)((uint8_t*)freeRegion + regionBoundaryBumpAmount);
    freeRegion->size = newThisRegionSize;
  }
  // Next, we need to decide whether this region is so large that it should be split into two regions,
  // one representing the newly used memory area, and at the high end a remaining leftover free area.
  // This splitting to two is done always if there is enough space for the high end to fit a region.
  // Carve 'size' bytes of payload off this region. So,
  // [sz prev next sz]
  // becomes
  // [sz payload sz] [sz prev next sz]
  if (sizeof(Region) + REGION_HEADER_SIZE + size <= freeRegion->size)
  {
    // There is enough space to keep a free region at the end of the carved out block
    // -> construct the new block
    Region *newFreeRegion = (Region *)((uint8_t*)freeRegion + REGION_HEADER_SIZE + size);
    create_free_region(newFreeRegion, freeRegion->size - size - REGION_HEADER_SIZE);
    link_to_free_list(newFreeRegion);

    // Recreate the resized Region under its new size.
    create_used_region(freeRegion, size + REGION_HEADER_SIZE);
  }
  else
  {
    // There is not enough space to split the free memory region into used+free parts, so consume the whole
    // region as used memory, not leaving a free memory region behind.
    // Initialize the free region as used by resetting the ceiling size to the same value as the size at bottom.
    ((uint32_t*)((uint8_t*)freeRegion + freeRegion->size))[-1] = freeRegion->size;
  }

#ifdef __EMSCRIPTEN_TRACING__
  emscripten_trace_record_allocation(freeRegion, freeRegion->size);
#endif

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('attempt_allocate - succeeded allocating memory, region ptr=0x' + ($0>>>0).toString(16) + ', align=' + $1 + ', payload size=' + ($2>>>0) + ' bytes)'), freeRegion, alignment, size);
#endif

  return (uint8_t*)freeRegion + sizeof(uint32_t);
}

static size_t validate_alloc_alignment(size_t alignment)
{
  // Cannot perform allocations that are less than 4 byte aligned, because the Region
  // control structures need to be aligned. Also round up to minimum outputted alignment.
  alignment = MAX(alignment, MALLOC_ALIGNMENT);
  // Arbitrary upper limit on alignment - very likely a programming bug if alignment is higher than this.
  assert(alignment <= 1024*1024);
  return alignment;
}

static size_t validate_alloc_size(size_t size)
{
  assert(size + REGION_HEADER_SIZE > size);

  // Allocation sizes must be a multiple of pointer sizes, and at least 2*sizeof(pointer).
  size_t validatedSize = size > SMALLEST_ALLOCATION_SIZE ? (size_t)ALIGN_UP(size, sizeof(Region*)) : SMALLEST_ALLOCATION_SIZE;
  assert(validatedSize >= size); // 32-bit wraparound should not occur, too large sizes should be stopped before

  return validatedSize;
}

static void *allocate_memory(size_t alignment, size_t size)
{
  ASSERT_MALLOC_IS_ACQUIRED();

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('allocate_memory(align=' + $0 + ', size=' + ($1>>>0) + ' bytes)'), alignment, size);
#endif

#ifdef EMMALLOC_MEMVALIDATE
  validate_memory_regions();
#endif

  if (!IS_POWER_OF_2(alignment))
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Allocation failed: alignment not power of 2!'));
#endif
    return 0;
  }

  if (size > MAX_ALLOC_SIZE)
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Allocation failed: attempted allocation size is too large: ' + ($0 >>> 0) + 'bytes! (negative integer wraparound?)'), size);
#endif
    return 0;
  }

  alignment = validate_alloc_alignment(alignment);
  size = validate_alloc_size(size);

  // Attempt to allocate memory starting from smallest bucket that can contain the required amount of memory.
  // Under normal alignment conditions this should always be the first or second bucket we look at, but if
  // performing an allocation with complex alignment, we may need to look at multiple buckets.
  int bucketIndex = compute_free_list_bucket(size);
  BUCKET_BITMASK_T bucketMask = freeRegionBucketsUsed >> bucketIndex;

  // Loop through each bucket that has free regions in it, based on bits set in freeRegionBucketsUsed bitmap.
  while(bucketMask)
  {
    BUCKET_BITMASK_T indexAdd = __builtin_ctzll(bucketMask);
    bucketIndex += indexAdd;
    bucketMask >>= indexAdd;
    assert(bucketIndex >= 0);
    assert(bucketIndex <= NUM_FREE_BUCKETS-1);
    assert(freeRegionBucketsUsed & (((BUCKET_BITMASK_T)1) << bucketIndex));

    Region *freeRegion = freeRegionBuckets[bucketIndex].next;
    assert(freeRegion);
    if (freeRegion != &freeRegionBuckets[bucketIndex])
    {
      void *ptr = attempt_allocate(freeRegion, alignment, size);
      if (ptr)
        return ptr;

      // We were not able to allocate from the first region found in this bucket, so penalize
      // the region by cycling it to the end of the doubly circular linked list. (constant time)
      // This provides a randomized guarantee that when performing allocations of size k to a
      // bucket of [k-something, k+something] range, we will not always attempt to satisfy the
      // allocation from the same available region at the front of the list, but we try each
      // region in turn.
      unlink_from_free_list(freeRegion);
      prepend_to_free_list(freeRegion, &freeRegionBuckets[bucketIndex]);
      // But do not stick around to attempt to look at other regions in this bucket - move
      // to search the next populated bucket index if this did not fit. This gives a practical
      // "allocation in constant time" guarantee, since the next higher bucket will only have
      // regions that are all of strictly larger size than the requested allocation. Only if
      // there is a difficult alignment requirement we may fail to perform the allocation from
      // a region in the next bucket, and if so, we keep trying higher buckets until one of them
      // works.
      ++bucketIndex;
      bucketMask >>= 1;
    }
    else
    {
      // This bucket was not populated after all with any regions,
      // but we just had a stale bit set to mark a populated bucket.
      // Reset the bit to update latest status so that we do not
      // redundantly look at this bucket again.
      freeRegionBucketsUsed &= ~(((BUCKET_BITMASK_T)1) << bucketIndex);
      bucketMask ^= 1;
    }
    // Instead of recomputing bucketMask from scratch at the end of each loop, it is updated as we go,
    // to avoid undefined behavior with (x >> 32)/(x >> 64) when bucketIndex reaches 32/64, (the shift would comes out as a no-op instead of 0).

    assert((bucketIndex == NUM_FREE_BUCKETS && bucketMask == 0) || (bucketMask == freeRegionBucketsUsed >> bucketIndex));
  }

  // None of the buckets were able to accommodate an allocation. If this happens we are almost out of memory.
  // The largest bucket might contain some suitable regions, but we only looked at one region in that bucket, so
  // as a last resort, loop through more free regions in the bucket that represents the largest allocations available.
  // But only if the bucket representing largest allocations available is not any of the first thirty buckets,
  // these represent allocatable areas less than <1024 bytes - which could be a lot of scrap.
  // In such case, prefer to sbrk() in more memory right away.
  int largestBucketIndex = NUM_FREE_BUCKETS - 1 - __builtin_clzll(freeRegionBucketsUsed);
  // freeRegion will be null if there is absolutely no memory left. (all buckets are 100% used)
  Region *freeRegion = freeRegionBucketsUsed ? freeRegionBuckets[largestBucketIndex].next : 0;
  if (freeRegionBucketsUsed >> 30)
  {
    // Look only at a constant number of regions in this bucket max, to avoid bad worst case behavior.
    // If this many regions cannot find free space, we give up and prefer to sbrk() more instead.
    const int maxRegionsToTryBeforeGivingUp = 99;
    int numTriesLeft = maxRegionsToTryBeforeGivingUp;
    while(freeRegion != &freeRegionBuckets[largestBucketIndex] && numTriesLeft-- > 0)
    {
      void *ptr = attempt_allocate(freeRegion, alignment, size);
      if (ptr)
        return ptr;
      freeRegion = freeRegion->next;
    }
  }

  // We were unable to find a free memory region. Must sbrk() in more memory!
  size_t numBytesToClaim = size+sizeof(Region)*3;
  assert(numBytesToClaim > size); // 32-bit wraparound should not happen here, allocation size has been validated above!
  bool success = claim_more_memory(numBytesToClaim);
  if (success)
    return allocate_memory(alignment, size); // Recurse back to itself to try again

  // also sbrk() failed, we are really really constrained :( As a last resort, go back to looking at the
  // bucket we already looked at above, continuing where the above search left off - perhaps there are
  // regions we overlooked the first time that might be able to satisfy the allocation.
  if (freeRegion)
  {
    while(freeRegion != &freeRegionBuckets[largestBucketIndex])
    {
      void *ptr = attempt_allocate(freeRegion, alignment, size);
      if (ptr)
        return ptr;
      freeRegion = freeRegion->next;
    }
  }

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('Could not find a free memory block!'));
#endif

  return 0;
}

void *emmalloc_memalign(size_t alignment, size_t size)
{
  MALLOC_ACQUIRE();
  void *ptr = allocate_memory(alignment, size);
  MALLOC_RELEASE();
  return ptr;
}
extern __typeof(emmalloc_memalign) emscripten_builtin_memalign __attribute__((alias("emmalloc_memalign")));

void * EMMALLOC_EXPORT memalign(size_t alignment, size_t size)
{
  return emmalloc_memalign(alignment, size);
}

void * EMMALLOC_EXPORT aligned_alloc(size_t alignment, size_t size)
{
  if ((alignment % sizeof(void *) != 0) || (size % alignment) != 0)
    return 0;
  return emmalloc_memalign(alignment, size);
}

void *emmalloc_malloc(size_t size)
{
  return emmalloc_memalign(MALLOC_ALIGNMENT, size);
}
extern __typeof(emmalloc_malloc) emscripten_builtin_malloc __attribute__((alias("emmalloc_malloc")));
extern __typeof(emmalloc_malloc) __libc_malloc __attribute__((alias("emmalloc_malloc")));

void * EMMALLOC_EXPORT malloc(size_t size)
{
  return emmalloc_malloc(size);
}

size_t emmalloc_usable_size(void *ptr)
{
  if (!ptr)
    return 0;

  uint8_t *regionStartPtr = (uint8_t*)ptr - sizeof(uint32_t);
  Region *region = (Region*)(regionStartPtr);
  assert(HAS_ALIGNMENT(region, sizeof(uint32_t)));

  MALLOC_ACQUIRE();

  uint32_t size = region->size;
  assert(size >= sizeof(Region));
  assert(region_is_in_use(region));

  MALLOC_RELEASE();

  return size - REGION_HEADER_SIZE;
}

size_t EMMALLOC_EXPORT malloc_usable_size(void *ptr)
{
  return emmalloc_usable_size(ptr);
}

void emmalloc_free(void *ptr)
{
#ifdef EMMALLOC_MEMVALIDATE
  emmalloc_validate_memory_regions();
#endif

  if (!ptr)
    return;

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('free(ptr=0x'+($0>>>0).toString(16)+')'), ptr);
#endif

  uint8_t *regionStartPtr = (uint8_t*)ptr - sizeof(uint32_t);
  Region *region = (Region*)(regionStartPtr);
  assert(HAS_ALIGNMENT(region, sizeof(uint32_t)));

  MALLOC_ACQUIRE();

  uint32_t size = region->size;
#ifdef EMMALLOC_VERBOSE
  if (size < sizeof(Region) || !region_is_in_use(region))
  {
    if (debug_region_is_consistent(region))
      // LLVM wasm backend bug: cannot use MAIN_THREAD_ASYNC_EM_ASM() here, that generates internal compiler error
      // Reproducible by running e.g. other.test_alloc_3GB
      EM_ASM(console.error('Double free at region ptr 0x' + ($0>>>0).toString(16) + ', region->size: 0x' + ($1>>>0).toString(16) + ', region->sizeAtCeiling: 0x' + ($2>>>0).toString(16) + ')'), region, size, region_ceiling_size(region));
    else
      MAIN_THREAD_ASYNC_EM_ASM(console.error('Corrupt region at region ptr 0x' + ($0>>>0).toString(16) + ' region->size: 0x' + ($1>>>0).toString(16) + ', region->sizeAtCeiling: 0x' + ($2>>>0).toString(16) + ')'), region, size, region_ceiling_size(region));
  }
#endif
  assert(size >= sizeof(Region));
  assert(region_is_in_use(region));

#ifdef __EMSCRIPTEN_TRACING__
  emscripten_trace_record_free(region);
#endif

  // Check merging with left side
  uint32_t prevRegionSizeField = ((uint32_t*)region)[-1];
  uint32_t prevRegionSize = prevRegionSizeField & ~FREE_REGION_FLAG;
  if (prevRegionSizeField != prevRegionSize) // Previous region is free?
  {
    Region *prevRegion = (Region*)((uint8_t*)region - prevRegionSize);
    assert(debug_region_is_consistent(prevRegion));
    unlink_from_free_list(prevRegion);
    regionStartPtr = (uint8_t*)prevRegion;
    size += prevRegionSize;
  }

  // Check merging with right side
  Region *nextRegion = next_region(region);
  assert(debug_region_is_consistent(nextRegion));
  uint32_t sizeAtEnd = *(uint32_t*)region_payload_end_ptr(nextRegion);
  if (nextRegion->size != sizeAtEnd)
  {
    unlink_from_free_list(nextRegion);
    size += nextRegion->size;
  }

  create_free_region(regionStartPtr, size);
  link_to_free_list((Region*)regionStartPtr);

  MALLOC_RELEASE();

#ifdef EMMALLOC_MEMVALIDATE
  emmalloc_validate_memory_regions();
#endif
}
extern __typeof(emmalloc_free) emscripten_builtin_free __attribute__((alias("emmalloc_free")));
extern __typeof(emmalloc_free) __libc_free __attribute__((alias("emmalloc_free")));

void EMMALLOC_EXPORT free(void *ptr)
{
  return emmalloc_free(ptr);
}

// Can be called to attempt to increase or decrease the size of the given region
// to a new size (in-place). Returns 1 if resize succeeds, and 0 on failure.
static int attempt_region_resize(Region *region, size_t size)
{
  ASSERT_MALLOC_IS_ACQUIRED();
  assert(size > 0);
  assert(HAS_ALIGNMENT(size, sizeof(uint32_t)));

#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('attempt_region_resize(region=0x' + ($0>>>0).toString(16) + ', size=' + ($1>>>0) + ' bytes)'), region, size);
#endif

  // First attempt to resize this region, if the next region that follows this one
  // is a free region.
  Region *nextRegion = next_region(region);
  uint8_t *nextRegionEndPtr = (uint8_t*)nextRegion + nextRegion->size;
  size_t sizeAtCeiling = ((uint32_t*)nextRegionEndPtr)[-1];
  if (nextRegion->size != sizeAtCeiling) // Next region is free?
  {
    assert(region_is_free(nextRegion));
    uint8_t *newNextRegionStartPtr = (uint8_t*)region + size;
    assert(HAS_ALIGNMENT(newNextRegionStartPtr, sizeof(uint32_t)));
    // Next region does not shrink to too small size?
    if (newNextRegionStartPtr + sizeof(Region) <= nextRegionEndPtr)
    {
      unlink_from_free_list(nextRegion);
      create_free_region(newNextRegionStartPtr, nextRegionEndPtr - newNextRegionStartPtr);
      link_to_free_list((Region*)newNextRegionStartPtr);
      create_used_region(region, newNextRegionStartPtr - (uint8_t*)region);
      return 1;
    }
    // If we remove the next region altogether, allocation is satisfied?
    if (newNextRegionStartPtr <= nextRegionEndPtr)
    {
      unlink_from_free_list(nextRegion);
      create_used_region(region, region->size + nextRegion->size);
      return 1;
    }
  }
  else
  {
    // Next region is an used region - we cannot change its starting address. However if we are shrinking the
    // size of this region, we can create a new free region between this and the next used region.
    if (size + sizeof(Region) <= region->size)
    {
      size_t freeRegionSize = region->size - size;
      create_used_region(region, size);
      Region *freeRegion = (Region *)((uint8_t*)region + size);
      create_free_region(freeRegion, freeRegionSize);
      link_to_free_list(freeRegion);
      return 1;
    }
    else if (size <= region->size)
    {
      // Caller was asking to shrink the size, but due to not being able to fit a full Region in the shrunk
      // area, we cannot actually do anything. This occurs if the shrink amount is really small. In such case,
      // just call it success without doing any work.
      return 1;
    }
  }
#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('attempt_region_resize failed.'));
#endif
  return 0;
}

static int acquire_and_attempt_region_resize(Region *region, size_t size)
{
  MALLOC_ACQUIRE();
  int success = attempt_region_resize(region, size);
  MALLOC_RELEASE();
  return success;
}

void *emmalloc_aligned_realloc(void *ptr, size_t alignment, size_t size)
{
#ifdef EMMALLOC_VERBOSE
  MAIN_THREAD_ASYNC_EM_ASM(console.log('aligned_realloc(ptr=0x' + ($0>>>0).toString(16) + ', alignment=' + $1 + ', size=' + ($2>>>0)), ptr, alignment, size);
#endif

  if (!ptr)
    return emmalloc_memalign(alignment, size);

  if (size == 0)
  {
    free(ptr);
    return 0;
  }

  if (size > MAX_ALLOC_SIZE)
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Allocation failed: attempted allocation size is too large: ' + ($0 >>> 0) + 'bytes! (negative integer wraparound?)'), size);
#endif
    return 0;
  }

  assert(IS_POWER_OF_2(alignment));
  // aligned_realloc() cannot be used to ask to change the alignment of a pointer.
  assert(HAS_ALIGNMENT(ptr, alignment));
  size = validate_alloc_size(size);

  // Calculate the region start address of the original allocation
  Region *region = (Region*)((uint8_t*)ptr - sizeof(uint32_t));

  // First attempt to resize the given region to avoid having to copy memory around
  if (acquire_and_attempt_region_resize(region, size + REGION_HEADER_SIZE))
  {
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_record_reallocation(ptr, ptr, size);
#endif
    return ptr;
  }

  // If resize failed, we must allocate a new region, copy the data over, and then
  // free the old region.
  void *newptr = emmalloc_memalign(alignment, size);
  if (newptr)
  {
    memcpy(newptr, ptr, MIN(size, region->size - REGION_HEADER_SIZE));
    free(ptr);
  }
  // N.B. If there is not enough memory, the old memory block should not be freed and
  // null pointer is returned.
  return newptr;
}

void * EMMALLOC_EXPORT aligned_realloc(void *ptr, size_t alignment, size_t size)
{
  return emmalloc_aligned_realloc(ptr, alignment, size);
}

// realloc_try() is like realloc(), but only attempts to try to resize the existing memory
// area. If resizing the existing memory area fails, then realloc_try() will return 0
// (the original memory block is not freed or modified). If resizing succeeds, previous
// memory contents will be valid up to min(old length, new length) bytes.
void *emmalloc_realloc_try(void *ptr, size_t size)
{
  if (!ptr)
    return 0;

  if (size == 0)
  {
    free(ptr);
    return 0;
  }

  if (size > MAX_ALLOC_SIZE)
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Allocation failed: attempted allocation size is too large: ' + ($0 >>> 0) + 'bytes! (negative integer wraparound?)'), size);
#endif
    return 0;
  }

  size = validate_alloc_size(size);

  // Calculate the region start address of the original allocation
  Region *region = (Region*)((uint8_t*)ptr - sizeof(uint32_t));

  // Attempt to resize the given region to avoid having to copy memory around
  int success = acquire_and_attempt_region_resize(region, size + REGION_HEADER_SIZE);
#ifdef __EMSCRIPTEN_TRACING__
  if (success)
    emscripten_trace_record_reallocation(ptr, ptr, size);
#endif
  return success ? ptr : 0;
}

// emmalloc_aligned_realloc_uninitialized() is like aligned_realloc(), but old memory contents
// will be undefined after reallocation. (old memory is not preserved in any case)
void *emmalloc_aligned_realloc_uninitialized(void *ptr, size_t alignment, size_t size)
{
  if (!ptr)
    return emmalloc_memalign(alignment, size);

  if (size == 0)
  {
    free(ptr);
    return 0;
  }

  if (size > MAX_ALLOC_SIZE)
  {
#ifdef EMMALLOC_VERBOSE
    MAIN_THREAD_ASYNC_EM_ASM(console.log('Allocation failed: attempted allocation size is too large: ' + ($0 >>> 0) + 'bytes! (negative integer wraparound?)'), size);
#endif
    return 0;
  }

  size = validate_alloc_size(size);

  // Calculate the region start address of the original allocation
  Region *region = (Region*)((uint8_t*)ptr - sizeof(uint32_t));

  // First attempt to resize the given region to avoid having to copy memory around
  if (acquire_and_attempt_region_resize(region, size + REGION_HEADER_SIZE))
  {
#ifdef __EMSCRIPTEN_TRACING__
    emscripten_trace_record_reallocation(ptr, ptr, size);
#endif
    return ptr;
  }

  // If resize failed, drop the old region and allocate a new region. Memory is not
  // copied over
  free(ptr);
  return emmalloc_memalign(alignment, size);
}

void *emmalloc_realloc(void *ptr, size_t size)
{
  return emmalloc_aligned_realloc(ptr, MALLOC_ALIGNMENT, size);
}
extern __typeof(emmalloc_realloc) __libc_realloc __attribute__((alias("emmalloc_realloc")));

void * EMMALLOC_EXPORT realloc(void *ptr, size_t size)
{
  return emmalloc_realloc(ptr, size);
}

// realloc_uninitialized() is like realloc(), but old memory contents
// will be undefined after reallocation. (old memory is not preserved in any case)
void *emmalloc_realloc_uninitialized(void *ptr, size_t size)
{
  return emmalloc_aligned_realloc_uninitialized(ptr, MALLOC_ALIGNMENT, size);
}

int emmalloc_posix_memalign(void **memptr, size_t alignment, size_t size)
{
  assert(memptr);
  if (alignment % sizeof(void *) != 0)
    return 22/* EINVAL*/;
  *memptr = emmalloc_memalign(alignment, size);
  return *memptr ?  0 : 12/*ENOMEM*/;
}

int EMMALLOC_EXPORT posix_memalign(void **memptr, size_t alignment, size_t size)
{
  return emmalloc_posix_memalign(memptr, alignment, size);
}

void *emmalloc_calloc(size_t num, size_t size)
{
  size_t bytes = num*size;
  void *ptr = emmalloc_memalign(MALLOC_ALIGNMENT, bytes);
  if (ptr)
    memset(ptr, 0, bytes);
  return ptr;
}
extern __typeof(emmalloc_calloc) __libc_calloc __attribute__((alias("emmalloc_calloc")));

void * EMMALLOC_EXPORT calloc(size_t num, size_t size)
{
  return emmalloc_calloc(num, size);
}

static int count_linked_list_size(Region *list)
{
  int size = 1;
  for(Region *i = list->next; i != list; list = list->next)
    ++size;
  return size;
}

static size_t count_linked_list_space(Region *list)
{
  size_t space = 0;
  for(Region *i = list->next; i != list; list = list->next)
    space += region_payload_end_ptr(i) - region_payload_start_ptr(i);
  return space;
}

struct mallinfo emmalloc_mallinfo()
{
  MALLOC_ACQUIRE();

  struct mallinfo info;
  // Non-mmapped space allocated (bytes): For emmalloc,
  // let's define this as the difference between heap size and dynamic top end.
  info.arena = emscripten_get_heap_size() - (size_t)sbrk(0);
  // Number of "ordinary" blocks. Let's define this as the number of highest
  // size blocks. (subtract one from each, since there is a sentinel node in each list)
  info.ordblks = count_linked_list_size(&freeRegionBuckets[NUM_FREE_BUCKETS-1])-1;
  // Number of free "fastbin" blocks. For emmalloc, define this as the number
  // of blocks that are not in the largest pristine block.
  info.smblks = 0;
  // The total number of bytes in free "fastbin" blocks.
  info.fsmblks = 0;
  for(int i = 0; i < NUM_FREE_BUCKETS-1; ++i)
  {
    info.smblks += count_linked_list_size(&freeRegionBuckets[i])-1;
    info.fsmblks += count_linked_list_space(&freeRegionBuckets[i]);
  }

  info.hblks = 0; // Number of mmapped regions: always 0. (no mmap support)
  info.hblkhd = 0; // Amount of bytes in mmapped regions: always 0. (no mmap support)

  // Walk through all the heap blocks to report the following data:
  // The "highwater mark" for allocated space—that is, the maximum amount of
  // space that was ever allocated. Emmalloc does not want to pay code to
  // track this, so this is only reported from current allocation data, and
  // may not be accurate.
  info.usmblks = 0;
  info.uordblks = 0; // The total number of bytes used by in-use allocations.
  info.fordblks = 0; // The total number of bytes in free blocks.
  // The total amount of releasable free space at the top of the heap.
  // This is the maximum number of bytes that could ideally be released by malloc_trim(3).
  Region *lastActualRegion = prev_region((Region*)((uint8_t*)((uint32_t*)listOfAllRegions)[2] - sizeof(Region)));
  info.keepcost = region_is_free(lastActualRegion) ? lastActualRegion->size : 0;

  Region *root = listOfAllRegions;
  while(root)
  {
    Region *r = root;
    assert(debug_region_is_consistent(r));
    uint8_t *lastRegionEnd = (uint8_t*)(((uint32_t*)root)[2]);
    while((uint8_t*)r < lastRegionEnd)
    {
      assert(debug_region_is_consistent(r));

      if (region_is_free(r))
      {
        // Count only the payload of the free block towards free memory.
        info.fordblks += region_payload_end_ptr(r) - region_payload_start_ptr(r);
        // But the header data of the free block goes towards used memory.
        info.uordblks += REGION_HEADER_SIZE;
      }
      else
      {
        info.uordblks += r->size;
      }
      // Update approximate watermark data
      info.usmblks = MAX(info.usmblks, (int)(r + r->size));

      if (r->size == 0)
        break;
      r = next_region(r);
    }
    root = ((Region*)((uint32_t*)root)[1]);
  }

  MALLOC_RELEASE();
  return info;
}

struct mallinfo EMMALLOC_EXPORT mallinfo()
{
  return emmalloc_mallinfo();
}

// Note! This function is not fully multithreadin safe: while this function is running, other threads should not be
// allowed to call sbrk()!
static int trim_dynamic_heap_reservation(size_t pad)
{
  ASSERT_MALLOC_IS_ACQUIRED();

  if (!listOfAllRegions)
    return 0; // emmalloc is not controlling any dynamic memory at all - cannot release memory.
  uint32_t *previousSbrkEndAddress = (uint32_t*)((uint32_t*)listOfAllRegions)[2];
  assert(sbrk(0) == previousSbrkEndAddress);
  uint32_t lastMemoryRegionSize = previousSbrkEndAddress[-1];
  assert(lastMemoryRegionSize == 16); // // The last memory region should be a sentinel node of exactly 16 bytes in size.
  Region *endSentinelRegion = (Region*)((uint8_t*)previousSbrkEndAddress - sizeof(Region));
  Region *lastActualRegion = prev_region(endSentinelRegion);

  // Round padding up to multiple of 4 bytes to keep sbrk() and memory region alignment intact.
  // Also have at least 8 bytes of payload so that we can form a full free region.
  size_t newRegionSize = (size_t)ALIGN_UP(pad, 4);
  if (pad > 0)
    newRegionSize += sizeof(Region) - (newRegionSize - pad);

  if (!region_is_free(lastActualRegion) || lastActualRegion->size <= newRegionSize)
    return 0; // Last actual region is in use, or caller desired to leave more free memory intact than there is.

  // This many bytes will be shrunk away.
  size_t shrinkAmount = lastActualRegion->size - newRegionSize;
  assert(HAS_ALIGNMENT(shrinkAmount, 4));

  unlink_from_free_list(lastActualRegion);
  // If pad == 0, we should delete the last free region altogether. If pad > 0,
  // shrink the last free region to the desired size.
  if (newRegionSize > 0)
  {
    create_free_region(lastActualRegion, newRegionSize);
    link_to_free_list(lastActualRegion);
  }

  // Recreate the sentinel region at the end of the last free region
  endSentinelRegion = (Region*)((uint8_t*)lastActualRegion + newRegionSize);
  create_used_region(endSentinelRegion, sizeof(Region));

  // And update the size field of the whole region block.
  ((uint32_t*)listOfAllRegions)[2] = (uint32_t)endSentinelRegion + sizeof(Region);

  // Finally call sbrk() to shrink the memory area.
  void *oldSbrk = sbrk(-(intptr_t)shrinkAmount);
  assert((intptr_t)oldSbrk != -1); // Shrinking with sbrk() should never fail.
  assert(oldSbrk == previousSbrkEndAddress); // Another thread should not have raced to increase sbrk() on us!

  // All successful, and we actually trimmed memory!
  return 1;
}

int emmalloc_trim(size_t pad)
{
  MALLOC_ACQUIRE();
  int success = trim_dynamic_heap_reservation(pad);
  MALLOC_RELEASE();
  return success;
}

int EMMALLOC_EXPORT malloc_trim(size_t pad)
{
  return emmalloc_trim(pad);
}

size_t emmalloc_dynamic_heap_size()
{
  size_t dynamicHeapSize = 0;

  MALLOC_ACQUIRE();
  Region *root = listOfAllRegions;
  while(root)
  {
    Region *r = root;
    uintptr_t blockEndPtr = ((uint32_t*)r)[2];
    dynamicHeapSize += blockEndPtr - (uintptr_t)r;
    root = ((Region*)((uint32_t*)root)[1]);
  }
  MALLOC_RELEASE();
  return dynamicHeapSize;
}

size_t emmalloc_free_dynamic_memory()
{
  size_t freeDynamicMemory = 0;

  int bucketIndex = 0;

  MALLOC_ACQUIRE();
  BUCKET_BITMASK_T bucketMask = freeRegionBucketsUsed;

  // Loop through each bucket that has free regions in it, based on bits set in freeRegionBucketsUsed bitmap.
  while(bucketMask)
  {
    BUCKET_BITMASK_T indexAdd = __builtin_ctzll(bucketMask);
    bucketIndex += indexAdd;
    bucketMask >>= indexAdd;
    for(Region *freeRegion = freeRegionBuckets[bucketIndex].next;
      freeRegion != &freeRegionBuckets[bucketIndex];
      freeRegion = freeRegion->next)
    {
      freeDynamicMemory += freeRegion->size - REGION_HEADER_SIZE;
    }
    ++bucketIndex;
    bucketMask >>= 1;
  }
  MALLOC_RELEASE();
  return freeDynamicMemory;
}

size_t emmalloc_compute_free_dynamic_memory_fragmentation_map(size_t freeMemorySizeMap[32])
{
  memset((void*)freeMemorySizeMap, 0, sizeof(freeMemorySizeMap[0])*32);

  size_t numFreeMemoryRegions = 0;
  int bucketIndex = 0;
  MALLOC_ACQUIRE();
  BUCKET_BITMASK_T bucketMask = freeRegionBucketsUsed;

  // Loop through each bucket that has free regions in it, based on bits set in freeRegionBucketsUsed bitmap.
  while(bucketMask)
  {
    BUCKET_BITMASK_T indexAdd = __builtin_ctzll(bucketMask);
    bucketIndex += indexAdd;
    bucketMask >>= indexAdd;
    for(Region *freeRegion = freeRegionBuckets[bucketIndex].next;
      freeRegion != &freeRegionBuckets[bucketIndex];
      freeRegion = freeRegion->next)
    {
      ++numFreeMemoryRegions;
      size_t freeDynamicMemory = freeRegion->size - REGION_HEADER_SIZE;
      if (freeDynamicMemory > 0)
        ++freeMemorySizeMap[31-__builtin_clz(freeDynamicMemory)];
      else
        ++freeMemorySizeMap[0];
    }
    ++bucketIndex;
    bucketMask >>= 1;
  }
  MALLOC_RELEASE();
  return numFreeMemoryRegions;
}

size_t emmalloc_unclaimed_heap_memory(void) {
  return emscripten_get_heap_max() - (size_t)sbrk(0);
}
