/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026 Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_CONFIG_H
#define MI_CONFIG_H

// Minimal alignment necessary. On most platforms 16 bytes are needed
// due to SSE registers for example. This must be at least `sizeof(void*)`
#ifndef MI_MAX_ALIGN_SIZE
#define MI_MAX_ALIGN_SIZE  16   // sizeof(max_align_t)
#endif

#if MI_MAX_ALIGN_SIZE < MI_INTPTR_SIZE
#error MI_MAX_ALIGN_SIZE must be at least MI_INTPTR_SIZE
#endif

// ------------------------------------------------------
// Build Variants
// ------------------------------------------------------

// Define NDEBUG in the release version to disable assertions.
// #define NDEBUG

// Define MI_TRACK_<tool> to enable tracking support
// #define MI_TRACK_VALGRIND 1
// #define MI_TRACK_ASAN     1
// #define MI_TRACK_ETW      1
// #define MI_TRACK_CUSTOM   1

// Define MI_STATS as 1 to maintain statistics; set it to 2 to have detailed statistics (but costs some performance).
// #define MI_STATS 1

// Define MI_SECURE to enable security mitigations
// #define MI_SECURE 0  // no security mitigations
// #define MI_SECURE 1  // + guard pages around metadata (and separated metadata) and after singleton blocks, randomized arena/OS allocation addresses,
                        //   abort on EFAULT (corrupted meta-data), encode free lists with a fast MAC (64-bit multiply), clear free list `next` on allocation
                        //   (so double frees are detected on allocation, at an immediate double free, or when the page is freed) (`-DMI_SECURE=BASIC`)
// #define MI_SECURE 2  // + invalid-pointer check on free (checked lookup in page-map.h), encode free lists with a stronger MAC (128-bit multiply+xor),
                        //   randomized free list (4-way) initialization and allocation addresses within mimalloc pages (`-DMI_SECURE=CHECKED`)
// #define MI_SECURE 3  // + zero blocks on free (up to the first 1KiB), check double free (slow, and only for thread-local frees;
                        //   only at level 3 as padding is used at level 4)
// #define MI_SECURE 4  // + add padding to each block, check buffer overflow (using the padding), immediate double free check (using the padding),
                        //   more randomized free lists (64-way), decommit with mprotect, reuse of freed blocks is delayed (until 1/8th of a page is free)
                        //   and a random number of the most recently freed blocks is held back (`-DMI_SECURE=ON`)
// #define MI_SECURE 5  // + guard page at the end of each mimalloc page (expensive!), and byte-precise buffer overflow checks (`-DMI_SECURE=FULL`)

// The default is 2 with MTE or PAC, and 3 on CHERI (where capabilities already provide spatial safety)
#if !defined(MI_SECURE)
#if MI_MTE || MI_PAC
#define MI_SECURE 2
#elif MI_CHERI
#define MI_SECURE 3
#else
#define MI_SECURE 0
#endif
#endif

// Restrict environment options by default in secure, PAC, and MTE builds.
#if !defined(MI_SECURE_ENV)
#if MI_SECURE >= 1 || MI_PAC || MI_MTE
#define MI_SECURE_ENV 1
#else
#define MI_SECURE_ENV 0
#endif
#endif

// MTE, PAC, or secure level 2 and higher always check for invalid pointers in `mi_free`
#if (MI_MTE || MI_PAC || MI_SECURE>=2) && !MI_FREE_IS_CHECKED
#undef  MI_FREE_IS_CHECKED
#define MI_FREE_IS_CHECKED 1
#endif

// Statistics (0=only essential, 1=detailed (fast, can be enabled always), 2=slightly more expensive tracking of precise requested bytes)
#ifndef MI_STATS
#if (MI_DEBUG>0)
#define MI_STATS 2
#else
#define MI_STATS 1
#endif
#endif

// Enable profiling support (0=off, 1=fast, can be enabled always, 2=allow fine-grained sample rates(<64 KiB), a tad more expensive)
#ifndef MI_PROFILE
#define MI_PROFILE  1
#endif

// Enable guard pages behind objects of a certain size (set by the MIMALLOC_GUARDED_MIN/MAX/SAMPLE_RATE options)
#if !defined(MI_GUARDED) && MI_DEBUG && !defined(NDEBUG) && !MI_OPT_FREE_SMALL
#define MI_GUARDED  1
#endif

// For profiling or guarded pages we need to sample every once in a while. (0=no sampling, 1=fast, 2=allow fine grained sample rates(<64 KiB), a tad more expensive)
#ifndef MI_SAMPLE
#if MI_PROFILE>1 || MI_GUARDED>1
  #define MI_SAMPLE 2
#elif MI_PROFILE || MI_GUARDED
  #define MI_SAMPLE 1
#else
  #define MI_SAMPLE 0
#endif
#endif

// Reserve extra padding at the end of each block to be more resilient against theap block overflows.
// The padding can detect heap-block overflow on free, and provides byte-precise `mi_usable_size`.
#if !defined(MI_PADDING)
#if (MI_SECURE>=4 || MI_DEBUG>=1 || (MI_TRACK_VALGRIND || MI_TRACK_ASAN || MI_TRACK_ETW))
#define MI_PADDING  1
#else
#define MI_PADDING  0
#endif
#endif

// Check for byte-precise buffer overflow?
#if !defined(MI_PADDING_CHECK_BYTES) && MI_PADDING && (MI_SECURE>=5 || MI_DEBUG>=1)
#define MI_PADDING_CHECK_BYTES      1
#endif


// Encoded free lists allow detection of corrupted free lists
// and can detect buffer overflows, modify after free, and double `free`s.
// Level 1 uses a faster but weaker keyed MAC on 64-bit (the low product only), while level 2 uses the full `mulxor64` MAC.
#ifndef MI_ENCODE_FREELIST
#if (MI_SECURE>=2 || MI_DEBUG>=1)
#define MI_ENCODE_FREELIST          2
#elif (MI_SECURE>=1)
#define MI_ENCODE_FREELIST          1
#endif
#endif


// Use large pages for objects between 80KiB and 512KiB ?
// This is disabled by default as for many workloads the block sizes above 80 KiB
// are quite random which can lead to too many partially used large pages (but see issue #1104).
#ifndef MI_ENABLE_LARGE_PAGES
#define MI_ENABLE_LARGE_PAGES       0
#endif

// Place page meta info at the start of the page area or keep it separate?
// Separate keeps the page info at the arena start (default) which is more secure
// and reduces wasted space due to alignment and block sizes.
#if !defined(MI_PAGE_META_IS_SEPARATED)
#define MI_PAGE_META_IS_SEPARATED   1
#endif

// We can choose to page meta info aligned at the start of every MI_PAGE_META_ALIGNED_CHUNKS
// This can be used to have a faster `mi_free(_small)` as we can avoid a page_map lookup.
// This only works if valid pointers are passed to `mi_free` though. However, checked
// free `mi_cfree` can still use the page map to validate pointers.
#if !MI_FREE_IS_CHECKED && !MI_FREE_USE_PAGEMAP && !MI_CHERI
#if MI_PAGE_META_IS_SEPARATED
#define MI_PAGE_META_IS_ALIGNED         1
#define MI_PAGE_META_ALIGNED_CHUNKS     MI_SIZE_SIZE
#else
#warning "cannot optimize free with alignment since the page meta data is not separated"
#endif
#endif

// We can choose to only put page info of small pages at the start of the page area.
// This can be used to have a slightly faster `mi_free_small` function for specialized
// cases (like language runtime systems).
#if !defined(MI_PAGE_META_SMALL_IS_ALIGNED)
// guarded can put small allocations into big blocks in a medium page (which would make `mi_free_small` fail)
// on CHERI we cannot derive the page from a (bounded) user pointer
#if (defined(MI_OPT_FREE_SMALL) && MI_OPT_FREE_SMALL==0) || MI_SECURE || MI_GUARDED || MI_CHERI
#define MI_PAGE_META_SMALL_IS_ALIGNED   0
#elif (MI_OPT_FREE_SMALL || MI_PAGE_META_IS_ALIGNED)
#define MI_PAGE_META_SMALL_IS_ALIGNED   1
#else
#define MI_PAGE_META_SMALL_IS_ALIGNED   0
#endif
#endif

// --------------------------------------------------------------
// Build configuration checks
// --------------------------------------------------------------

#if !MI_PAGE_META_IS_SEPARATED && MI_SECURE
#error "Secure mode should use separated page infos"
#endif
#if MI_PAGE_META_SMALL_IS_ALIGNED && (MI_SECURE || MI_GUARDED || MI_MTE)
#error "MTE, secure or guarded mode cannot use MI_OPT_FREE_SMALL (MI_PAGE_META_SMALL_IS_ALIGNED)"
#endif
#if (MI_PAGE_META_IS_ALIGNED || MI_PAGE_META_SMALL_IS_ALIGNED) && MI_CHERI
#error "Cannot have aligned page meta with CHERI"
#endif
#if MI_QSBR && MI_CHERI
#error "QSBR cannot be used with CHERI (as delayed concurrent reads would have the wrong capability)"
#endif


// --------------------------------------------------------------
// Sizes of internal data-structures
// (comments specify sizes on 64-bit, usually 32-bit is halved)
// --------------------------------------------------------------

// Main size parameter; determines max arena sizes and max arena object sizes etc.
#ifndef MI_ARENA_SLICE_SHIFT
  #ifdef  MI_SMALL_PAGE_SHIFT   // backward compatibility
  #define MI_ARENA_SLICE_SHIFT              MI_SMALL_PAGE_SHIFT
  #elif MI_SECURE && MI_OS_PROBABLE_PAGE_SIZE >= 64*MI_KiB
  #define MI_ARENA_SLICE_SHIFT              (18)                        // 256 KiB as 64 KiB guard pages can be at most 1/4 of a slice (issue #1166)
  #elif MI_SECURE>=5 && MI_OS_PROBABLE_PAGE_SIZE >= 16*MI_KiB
  #define MI_ARENA_SLICE_SHIFT              (17)                        // 128 KiB to not waste too much due to 16 KiB guard pages
  #elif MI_OS_PROBABLE_PAGE_SIZE >= 64*MI_KiB
  #define MI_ARENA_SLICE_SHIFT              (16)                        // 64 KiB: at least the OS page size (e.g. on wasm32)
  #else
  #define MI_ARENA_SLICE_SHIFT              (13 + MI_SIZE_SHIFT)        // 64 KiB (32 KiB on 32-bit)
  #endif
#endif
#if MI_ARENA_SLICE_SHIFT < 13
#error Arena slices should be at least 8KiB
#endif

#ifndef MI_BCHUNK_BITS_SHIFT
  #if MI_ARENA_SLICE_SHIFT <= 13    // <= 8KiB
  #define MI_BCHUNK_BITS_SHIFT              (7)   // 128 bits
  #elif MI_ARENA_SLICE_SHIFT < 16   // <= 32KiB
  #define MI_BCHUNK_BITS_SHIFT              (8)   // 256 bits
  #else
  #define MI_BCHUNK_BITS_SHIFT              (6 + MI_SIZE_SHIFT)       // 512 bits (or 256 on 32-bit)
  #endif
#endif

#define MI_BCHUNK_BITS                      (1 << MI_BCHUNK_BITS_SHIFT)         // sub-bitmaps in arena's are "bchunks" of 512 bits
#define MI_ARENA_SLICE_SIZE                 (MI_ZU(1) << MI_ARENA_SLICE_SHIFT)  // arena's allocate in slices of 64 KiB
#define MI_ARENA_SLICE_ALIGN                (MI_ARENA_SLICE_SIZE)
#define MI_ARENA_CHUNK_SIZE                 (MI_BCHUNK_BITS * MI_ARENA_SLICE_SIZE)

#define MI_ARENA_MIN_OBJ_SLICES             (1)
#define MI_ARENA_MAX_CHUNK_OBJ_SLICES       (MI_BCHUNK_BITS)                    // 32 MiB (or 8 MiB on 32-bit)

#define MI_ARENA_MIN_OBJ_SIZE               (MI_ARENA_MIN_OBJ_SLICES * MI_ARENA_SLICE_SIZE)
#define MI_ARENA_MAX_CHUNK_OBJ_SIZE         (MI_ARENA_MAX_CHUNK_OBJ_SLICES * MI_ARENA_SLICE_SIZE)

#if MI_ARENA_MAX_CHUNK_OBJ_SIZE < MI_SIZE_SIZE*1024
#error maximum object size may be too small to hold local thread data
#endif

#define MI_SMALL_PAGE_SIZE                  MI_ARENA_MIN_OBJ_SIZE                    // 64 KiB
#define MI_MEDIUM_PAGE_SIZE                 (8*MI_SMALL_PAGE_SIZE)                   // 512 KiB  (=byte in the bchunk bitmap)
#define MI_LARGE_PAGE_SIZE                  (MI_SIZE_SIZE*MI_MEDIUM_PAGE_SIZE)       // 4 MiB    (=word in the bchunk bitmap)


// Maximum number of size classes. (spaced exponentially in 12.5% increments)
#if MI_BIN_HUGE != 73U
#error "mimalloc internal: expecting 73 bins"
#endif
#define MI_BIN_FULL  (MI_BIN_HUGE+1)
#define MI_BIN_COUNT (MI_BIN_FULL+1)

// We never allocate more than PTRDIFF_MAX (see also <https://sourceware.org/ml/libc-announce/2019/msg00001.html>)
#define MI_MAX_ALLOC_SIZE             PTRDIFF_MAX

// Minimal commit for a page on-demand commit
#define MI_PAGE_MIN_COMMIT_SIZE       (16*MI_KiB) /* MI_ARENA_SLICE_SIZE */

// In secure mode, a guard slice is reserved in front of the arena info and around the separated page meta data
// of aligned areas and direct OS allocations (but not on WebAssembly as it cannot protect memory)
#if MI_SECURE && !defined(__wasi__) && !defined(__EMSCRIPTEN__)
#define MI_ARENA_GUARD_SLICES         1
#else
#define MI_ARENA_GUARD_SLICES         0
#endif

// Is the page meta data of direct OS allocated pages separated in front of the page area? (between guard slices in secure mode)
// Otherwise it is placed at the start of the page area itself.
#if MI_PAGE_META_IS_ALIGNED || (MI_PAGE_META_IS_SEPARATED && MI_ARENA_GUARD_SLICES > 0)
#define MI_PAGE_META_OS_IS_SEPARATED  1
#else
#define MI_PAGE_META_OS_IS_SEPARATED  0
#endif

#if MI_PAGE_META_IS_ALIGNED
#define MI_PAGE_META_ALIGNED_COUNT    (MI_PAGE_META_ALIGNED_CHUNKS * MI_BCHUNK_BITS)
#define MI_PAGE_META_ALIGNMENT        (MI_PAGE_META_ALIGNED_COUNT * MI_ARENA_SLICE_SIZE)  // 256 MiB (32 MiB on 32-bit)
#define MI_PAGE_META_ALIGNED_OFFSET   (MI_ARENA_GUARD_SLICES * MI_ARENA_SLICE_SIZE)       // offset of the page meta data in an aligned area
#define MI_ARENA_ALIGNMENT            MI_PAGE_META_ALIGNMENT
#else
#define MI_ARENA_ALIGNMENT            MI_ARENA_SLICE_ALIGN
#endif

// spread the page meta entries of adjacent slices over the cache lines (within aligned areas or arena bitmap chunks)
#if !defined(MI_PAGE_META_IDX_MUL)
#define MI_PAGE_META_IDX_MUL          5    // must be odd; use 1 for a direct mapping; 9 or 33 work also well.
#endif
#if (MI_PAGE_META_IDX_MUL % 2) == 0        // (gcd(MI_PAGE_META_ALIGNED_COUNT, MI_PAGE_META_IDX_MUL)==1) => one-to-one mapping
#error "MI_PAGE_META_IDX_MUL must be odd to map slices one-to-one onto the page meta entries"
#endif


// ------------------------------------------------------
// Object sizes
// ------------------------------------------------------

#define MI_PAGE_ALIGN                     MI_ARENA_SLICE_ALIGN      // page area's must be aligned on this for the page map.
#define MI_PAGE_MIN_START_BLOCK_ALIGN     MI_MAX_ALIGN_SIZE         // minimal block alignment for the first block in a page (16b)
#define MI_PAGE_MAX_START_BLOCK_ALIGN2    (4*MI_KiB)                // maximal block alignment for "power of 2"-sized blocks (such that we guarantee natural alignment, i.e. aligned to their own size)
#define MI_PAGE_OSPAGE_BLOCK_ALIGN2       (4*MI_KiB)                // alignment of blocks that are a multiple of this size.
#define MI_PAGE_MAX_OVERALLOC_ALIGN       MI_ARENA_SLICE_SIZE       // (64 KiB) limit for which we overallocate in arena pages, beyond this use OS allocation

// Offset the first block in pages with larger blocks (using the unused space at the end of the page) spread across cache sets (issue #1121).
#if !defined(MI_PAGE_LARGE_CACHE_OFFSET)
#define MI_PAGE_LARGE_CACHE_OFFSET        1
#endif
#define MI_PAGE_LARGE_CACHE_OFFSET_UNIT   (128)                         // offset granularity (power of 2 between 16 and 4KiB)
#define MI_PAGE_LARGE_CACHE_OFFSET_MAX    (MI_PAGE_OSPAGE_BLOCK_ALIGN2 - MI_PAGE_LARGE_CACHE_OFFSET_UNIT) // maximal offset (32 possible offsets)
// Offset the first block in pages with small blocks
#define MI_PAGE_SMALL_CACHE_OFFSET_UNIT   (64)
#define MI_PAGE_SMALL_CACHE_OFFSET_MAX    (7*MI_PAGE_SMALL_CACHE_OFFSET_UNIT)  // up to 512B (7+1 offsets) (using 15+1 offsets upto 1KiB works well too)

// The max object sizes are intended to not waste more than ~ 12.5% internally over the page sizes.
#define MI_SMALL_MAX_OBJ_SIZE             ((MI_SMALL_PAGE_SIZE-MI_PAGE_OSPAGE_BLOCK_ALIGN2)/6)   // = 10 KiB
#define MI_MEDIUM_MAX_OBJ_SIZE            (5*(MI_MEDIUM_PAGE_SIZE/32))  // largest bin with 6 blocks/medium page (but see `alloc/page.h:mi_block_size_is_medium`)
#if MI_ENABLE_LARGE_PAGES
#define MI_LARGE_MAX_OBJ_SIZE             (MI_LARGE_PAGE_SIZE/8)        // <= 512 KiB. note: this must be a bin-size or we get rounding issues with `_mi_bin`
#else
#define MI_LARGE_MAX_OBJ_SIZE             MI_MEDIUM_MAX_OBJ_SIZE        // note: this must be a bin-size or we get rounding issues with `_mi_bin`
#endif
#define MI_LARGE_MAX_OBJ_WSIZE            (MI_LARGE_MAX_OBJ_SIZE/MI_SIZE_SIZE)
#define MI_MAX_RECLAIM_OBJ_SIZE           (MI_LARGE_MAX_OBJ_SIZE/4)     // only reclaim abandoned pages on free up to this block size

#if (MI_LARGE_MAX_OBJ_WSIZE >= 655360)
#error "mimalloc internal: define more bins"
#endif

// static invariant: MI_MAX_SINGLETON_BIN >= _mi_bin(MI_LARGE_MAX_OBJ_SIZE) (See heap/process.c for the size bins)
#if (MI_LARGE_MAX_OBJ_WSIZE <= 8192)     // 64 KiB
#define MI_MAX_SINGLETON_BIN   (48)
#elif (MI_LARGE_MAX_OBJ_WSIZE <= 32768)  // 256KiB
#define MI_MAX_SINGLETON_BIN   (56)
#elif (MI_LARGE_MAX_OBJ_WSIZE <= 65536)  // 512KiB
#define MI_MAX_SINGLETON_BIN   (60)
#else
#define MI_MAX_SINGLETON_BIN   MI_BIN_HUGE
#endif

// The small object max size must be larger than twice the small size max minus one,
// such that any aligned allocation with `alignment <= size <= MI_SMALL_SIZE_MAX` will
// be allocated in a small page (so `mi_free_csize` can delegate correctly to `mi_free_small`.
#if MI_SMALL_MAX_OBJ_SIZE <= (2*(MI_SMALL_WSIZE_MAX * MI_SIZE_SIZE)-1)
#error "mimalloc internal: the small object max size is too small"
#endif


/* -----------------------------------------------------------
  Secure mode defaults
----------------------------------------------------------- */

// In secure mode, initialize the free list of fresh blocks in a page in a randomized order.
#if !defined(MI_SECURE_SHUFFLE_FREELIST)
#define MI_SECURE_SHUFFLE_FREELIST  (MI_SECURE>=2)
#endif

// In secure mode, randomly extend the free list of a page even if it still has free blocks
#if !defined(MI_SECURE_RANDOM_EXTEND)
#define MI_SECURE_RANDOM_EXTEND     (MI_SECURE>=2)
#endif

// In secure mode, delay the reuse of blocks freed in a page until at least `capacity / MI_SECURE_REUSE_DELAY` blocks are free
#if !defined(MI_SECURE_REUSE_DELAY)
#if MI_SECURE>=2
#define MI_SECURE_REUSE_DELAY       8
#else
#define MI_SECURE_REUSE_DELAY       0
#endif
#endif
#if (MI_SECURE_REUSE_DELAY > 0) && !defined(MI_SECURE_REUSE_DELAY_MIN)
#define MI_SECURE_REUSE_DELAY_MIN   8       // Minimum number of blocks to delay reuse for
#define MI_SECURE_REUSE_DELAY_MAX   64      // Maximum number of blocks to delay reuse for
#define MI_SECURE_REUSE_DELAY_PAGES 4       // Number of pages to delay reuse for
#endif

// In secure mode, when freed blocks become available for reuse again, hold back a random number (between 1 and
// `MI_SECURE_HOLD_BACK`) of the most recently freed ones until the next time (if `MI_SECURE_HOLD_BACK > 0`).
#if !defined(MI_SECURE_HOLD_BACK) && MI_SECURE>=2
#define MI_SECURE_HOLD_BACK         16
#endif


#endif // MI_CONFIG_H
