/* ----------------------------------------------------------------------------
Copyright (c) 2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#pragma once
#ifndef MI_PRIM_MTE_H
#define MI_PRIM_MTE_H

#include "../decl.h"
#include "../arch.h"

/* ----------------------------------------------------------------------------
Memory tagging
Currently supports ARM MTE but we should be able to support future memory tagging
extensions as well (like x86 ChkTag)

Every 16-byte unit of tagged memory has a 4-bit tag, and each access must use a
pointer with a matching tag in the top bits (56-59 on ARM, see `MI_PTR_MEMTAG_SHIFT`).
Blocks in tagged pages get a random tag (from 1 to 15) that always differs from the tags of
the adjacent blocks: when a block is (re)tagged we read the current tags of its neighbours and
exclude those (see `mi_memtag_exclude_adjacent` and `mi_memtag_random_adjacent`). (A neighbour may be retagged concurrently
by another thread in which case adjacent blocks can rarely end up with the same tag.)
At the edges of a page we cannot read the adjacent memory and there we use tag classes
instead: the first block of a page uses the even class (tags 1 to 7) and the last block the
odd class (tags 8 to 15), or there is a tag 0 gap (see `mi_page_memtag_parity`).
When building with `MI_MTE_ODD_EVEN=1`, all blocks use tag classes instead where blocks
at even and odd indices use disjoint tag sets. This does not need to read the neighbouring tags
but has less entropy (on free, a block gets one of 6 or 7 possible tags instead of 12 to 14).
Tag 0 is never used for blocks and is reserved for free arena memory and untagged meta data.
Most meta data (like the page meta data, the page map, and the process meta data) gets a
random tag as well that differs from adjacent meta data (see `mi_memtag_meta_init_adjacent` and
`mi_memtag_meta_init_separate`).

Memory tagging is only used when built with `MI_MTE=1` and when it is enabled
at runtime (see `mi_memtag_is_enabled`); otherwise all operations are no-ops and
the hardware instructions are never executed (as these trap on CPU's without MTE).
-----------------------------------------------------------------------------*/

#define MI_MEMTAG_UNIT_MAX        (128)       // ranges up to this size are always tagged per unit
#define MI_MEMTAG_META            (0)         // tag used for meta data (and free arena memory)
#define MI_MEMTAG_LINE_MIN        (32)        // supported range of the cache line size tagged by `dc gva` (checked at initialization)
#define MI_MEMTAG_LINE_MAX        (2048)
#define MI_ZERO_ON_FREE_MAX       (1024)      // blocks are zeroed on free up to this size with MTE (larger ones are only retagged), or with `MI_SECURE>=3` (only the first bytes of larger ones)

// Tag exclusion masks (bit `i` set excludes tag `i`); blocks never use tag 0.
#define MI_MEMTAG_EXCLUDE         (0x0001)
#define MI_MEMTAG_EXCLUDE_EVEN    (MI_MEMTAG_EXCLUDE | 0xFF00)   // even blocks use tags 1 to 7
#define MI_MEMTAG_EXCLUDE_ODD     (MI_MEMTAG_EXCLUDE | 0x00FE)   // odd blocks use tags 8 to 15
#define MI_MEMTAG_ODD_BIT         (8)   // logical tag bit that is set for odd blocks

// Use only tag classes (even and odd) to ensure adjacent blocks and meta data have different tags (instead of reading the tags of neighbours)
#ifndef MI_MTE_ODD_EVEN
#define MI_MTE_ODD_EVEN           (0)
#endif

#if (MI_CACHE_LINE_SIZE % MI_MEMTAG_UNIT_SIZE) != 0
#error "`mi_decl_cache_align` must align on a memory tag unit"
#endif

// Set once at process initialization in the sealed process data (see `prim/os.c:_mi_os_init`)
bool   _mi_os_has_memtag(void);         // is memory tagging used?
size_t _mi_os_memtag_line_size(void);   // cache line size tagged by `dc gva`

// Values of the `mte` option (`MIMALLOC_MTE=off|on|sync|async|preferred`). The tag check mode can only be selected on Linux
// (on macOS it is determined by the OS). With `preferred` the kernel selects the preferred mode of the CPU which can
// also be `asymm` (see `/sys/devices/system/cpu/cpu<N>/mte_tcf_preferred`).
#define MI_MEMTAG_MODE_OFF        (0)
#define MI_MEMTAG_MODE_DEFAULT    (1)   // keep the current tag check mode of the process if set, and use `sync` otherwise
#define MI_MEMTAG_MODE_SYNC       (2)
#define MI_MEMTAG_MODE_ASYNC      (3)
#define MI_MEMTAG_MODE_PREFERRED  (4)

static inline bool mi_memtag_is_enabled(void) {
  #if MI_MTE >= 2
  return true;   // checked at startup in `_mi_os_init`
  #elif MI_MTE
  return _mi_os_has_memtag();
  #else
  return false;
  #endif
}

// Should meta data in memory that supports tagging (`memid.is_memtag`) be tagged?
static inline bool mi_memtag_use_for_meta(bool is_memtag) {
  return (mi_memtag_is_enabled() && is_memtag);
}

// Should blocks of size `bsize` in memory that supports tagging (`memid.is_memtag`) be tagged?
static inline bool mi_memtag_use_for(bool is_memtag, size_t bsize) {
  return (mi_memtag_is_enabled() && is_memtag && (bsize % MI_MEMTAG_UNIT_SIZE) == 0);
}


/* ----------------------------------------------------------------------------
  Pointer memory tags
-----------------------------------------------------------------------------*/

static inline size_t mi_ptr_memtag(const void* p) {
  #if MI_MTE
  return (size_t)(((uintptr_t)p & MI_PTR_MEMTAG_MASK) >> MI_PTR_MEMTAG_SHIFT);
  #else
  MI_UNUSED(p);
  return 0;
  #endif
}

static inline void* mi_ptr_with_memtag(const void* p, size_t memtag) {
  #if MI_MTE
  return (void*)(((uintptr_t)p & MI_PTR_ADDR_MASK) | (((uintptr_t)memtag << MI_PTR_MEMTAG_SHIFT) & MI_PTR_MEMTAG_MASK));
  #else
  MI_UNUSED(memtag);
  return (void*)p;
  #endif
}

// Pointer to meta data (or free arena memory) derived from a (possibly tagged) pointer `p`
static inline void* mi_ptr_memtag_meta(const void* p) {
  return mi_ptr_with_memtag(p, MI_MEMTAG_META);
}

// Primitive memory tag operations
static inline size_t mi_memtag_cpu_line_size(void);
static inline void  mi_memtag_check(const void* p);
static inline void* mi_memtag_random(void* p, size_t exclude);
static inline void* mi_memtag_load(void* p);
static inline void  mi_memtag_store(void* p, size_t size);
static inline void  mi_memtag_store_zero(void* p, size_t size);
static inline size_t mi_memtag_exclude_tag(const void* p, size_t exclude);
static inline size_t mi_memtag_exclude_adjacent(size_t idx, bool edge, const void* prev, const void* next);
static inline void  mi_memtag_blocks_init(void* start, size_t bsize, size_t i, size_t n, size_t parity, size_t count);
static inline void* mi_memtag_random_adjacent(void* p, size_t exclude, const void* prev, const void* next);
static inline void* mi_memtag_block_free(void* block, size_t bsize, size_t exclude, const void* prev, const void* next);
static inline void* mi_memtag_block_uncanonical(void* block, size_t bsize, size_t exclude, const void* prev, const void* next);
static inline void  mi_memtag_clear(void* p, size_t size);
static inline void* mi_memtag_meta_init_adjacent(void* p, size_t size, size_t idx, bool has_prev, bool has_next);
static inline void* mi_memtag_meta_init_class(void* p, size_t size, size_t idx);
static inline void* mi_memtag_meta_init_separate(void* p, size_t size, size_t idx, const void* other);

// The tag exclusion mask for the tag class of index `idx` (alternating between even and odd so adjacent indices never share a tag)
static inline size_t mi_memtag_exclude_class(size_t idx) {
  return ((idx&1)==0 ? MI_MEMTAG_EXCLUDE_EVEN : MI_MEMTAG_EXCLUDE_ODD);
}

// The tag class index of a tagged pointer `p` (only meaningful if it was tagged using its tag class)
static inline size_t mi_memtag_class_of(const void* p) {
  return ((mi_ptr_memtag(p) & MI_MEMTAG_ODD_BIT) != 0 ? 1 : 0);
}

// The base tag exclusion mask for memory with tag class index `idx`: at the `edge` of a region (where the adjacent
// memory cannot be read) we use the tag class, and otherwise only exclude tag 0 (or always use the class with `MI_MTE_ODD_EVEN`)
static inline size_t mi_memtag_exclude_base(size_t idx, bool edge) {
  return (MI_MTE_ODD_EVEN || edge ? mi_memtag_exclude_class(idx) : MI_MEMTAG_EXCLUDE);
}


/* ----------------------------------------------------------------------------
  Memory tag instructions (only valid to call if `mi_memtag_is_enabled()`).
  We use inline assembly so we do not need to compile with `-march=armv8.5-a+memtag`;
  this way the code is not changed otherwise and still runs on CPU's without MTE.
-----------------------------------------------------------------------------*/
#if MI_MTE
#if defined(__clang__)
#define MI_MEMTAG_ASM   ".arch_extension memtag\n\t"
#else
// older GNU assemblers (like binutils 2.38) only accept memtag instructions for armv8.5-a or later. Note that
// this stays in effect for the rest of the assembly file which is fine as armv8.5-a is a superset of armv8.1-a.
#define MI_MEMTAG_ASM   ".arch armv8.5-a+memtag\n\t"
#endif

// Return `p` with a random tag that is not in the `exclude` mask (`irg`)
static inline void* mi_memtag_random(void* p, size_t exclude) {
  void* q;
  __asm__ __volatile__ (MI_MEMTAG_ASM "irg %0, %1, %2" : "=r"(q) : "r"(p), "r"(exclude));
  return q;
}

// Return `p` with the tag of the memory unit at `p` (`ldg`)
static inline void* mi_memtag_load(void* p) {
  void* q = p;
  __asm__ __volatile__ (MI_MEMTAG_ASM "ldg %0, [%1]" : "+r"(q) : "r"(p) : "memory");
  return q;
}

// Size of the cache line tagged by `dc gva` (as given by `dczid_el0`), or 0 if not permitted
// (called once at initialization if the OS supports tagging, see `_mi_os_memtag_line_size`)
static inline size_t mi_memtag_cpu_line_size(void) {
  uint64_t dczid;
  __asm__ ("mrs %0, dczid_el0" : "=r"(dczid));
  return ((dczid & 0x10) != 0 ? 0 : (MI_ZU(4) << (dczid & 0x0F)));
}

// Read from `p` such that it faults if the tag of `p` does not match the memory tag
static inline void mi_memtag_check(const void* p) {
  (void)(*(const volatile uint8_t*)p);
}

// Set the tag of the unit at `p` (or two units, or the cache line) to the tag of `p`, and zero it if `zero` is set
static inline void __mi_memtag_store_unit(uint8_t* p, bool zero) {
  if (zero) { __asm__ __volatile__ (MI_MEMTAG_ASM "stzg %0, [%0]" : : "r"(p) : "memory"); }
       else { __asm__ __volatile__ (MI_MEMTAG_ASM "stg %0, [%0]"  : : "r"(p) : "memory"); }
}
static inline void __mi_memtag_store_unit2(uint8_t* p, bool zero) {
  if (zero) { __asm__ __volatile__ (MI_MEMTAG_ASM "stz2g %0, [%0]" : : "r"(p) : "memory"); }
       else { __asm__ __volatile__ (MI_MEMTAG_ASM "st2g %0, [%0]"  : : "r"(p) : "memory"); }
}
static inline void __mi_memtag_store_line(uint8_t* p, bool zero) {
  if (zero) { __asm__ __volatile__ (MI_MEMTAG_ASM "dc gzva, %0" : : "r"(p) : "memory"); }
       else { __asm__ __volatile__ (MI_MEMTAG_ASM "dc gva, %0"  : : "r"(p) : "memory"); }
}

// Tag the units in `[p,end)` (using pairs of units where possible)
static inline void __mi_memtag_store_units(uint8_t* p, uint8_t* end, bool zero) {
  if (((end - p) % (2*MI_MEMTAG_UNIT_SIZE)) != 0) { __mi_memtag_store_unit(p, zero); p += MI_MEMTAG_UNIT_SIZE; }
  for (; p < end; p += 2*MI_MEMTAG_UNIT_SIZE) { __mi_memtag_store_unit2(p, zero); }
}

// Set the memory tags of `[p,p+size)` to the tag of `p`, and zero it if `zero` is set.
// Small ranges are tagged per unit; for larger ranges only the (unaligned) edges are tagged per unit
// while the aligned middle is tagged per cache line (which is much faster).
static inline void __mi_memtag_store_ex(void* p, size_t size, bool zero) {
  mi_assert_internal(((uintptr_t)p % MI_MEMTAG_UNIT_SIZE) == 0 && (size % MI_MEMTAG_UNIT_SIZE) == 0 && size > 0);
  uint8_t* const start = (uint8_t*)p;
  uint8_t* const end = start + size;
  if (size <= MI_MEMTAG_UNIT_MAX) {
    __mi_memtag_store_units(start, end, zero);
  }
  else {
    const uintptr_t line_size = _mi_os_memtag_line_size();
    uint8_t* const line_start = (uint8_t*)(((uintptr_t)start + line_size - 1) & ~(line_size - 1));
    uint8_t* const line_end   = (uint8_t*)((uintptr_t)end & ~(line_size - 1));
    if (line_start >= line_end) {
      __mi_memtag_store_units(start, end, zero);
    }
    else {
      __mi_memtag_store_units(start, line_start, zero);
      for (uint8_t* q = line_start; q < line_end; q += line_size) { __mi_memtag_store_line(q, zero); }
      __mi_memtag_store_units(line_end, end, zero);
    }
  }
}

// Set the memory tags of `[p,p+size)` to the tag of `p`
static inline void mi_memtag_store(void* p, size_t size) {
  __mi_memtag_store_ex(p, size, false);
}

// Zero `[p,p+size)` and set its memory tags to the tag of `p`
static inline void mi_memtag_store_zero(void* p, size_t size) {
  __mi_memtag_store_ex(p, size, true);
}

// Add the tag of pointer `p` to the tag exclusion mask (`gmi`)
static inline size_t mi_memtag_exclude_tag(const void* p, size_t exclude) {
  size_t x;
  __asm__ (MI_MEMTAG_ASM "gmi %0, %1, %2" : "=r"(x) : "r"(p), "r"(exclude));
  return x;
}

// The tag exclusion mask for (re)tagging memory such that its tag differs from the current tags of the adjacent memory
// at `prev` and `next` (which are only read if not NULL and must be committed). At the `edge` of a region the tag class of
// `idx` is used as well (see `mi_memtag_exclude_base`). With `MI_MTE_ODD_EVEN` the neighbours are never read.
static inline size_t mi_memtag_exclude_adjacent(size_t idx, bool edge, const void* prev, const void* next) {
  size_t exclude = mi_memtag_exclude_base(idx, edge);
  #if !MI_MTE_ODD_EVEN
  if (prev != NULL) { exclude = mi_memtag_exclude_tag(mi_memtag_load((void*)prev), exclude); }
  if (next != NULL) { exclude = mi_memtag_exclude_tag(mi_memtag_load((void*)next), exclude); }
  #else
  MI_UNUSED(prev); MI_UNUSED(next);
  #endif
  return exclude;
}

// Return `p` with a random tag that is not in `exclude` and differs from the current tags of the adjacent memory at `prev`
// and `next` (which are only read if not NULL and must be committed). This has the same result as `mi_memtag_random(p, exclude')`
// with `exclude' == mi_memtag_exclude_adjacent(..)` but is faster on the critical free path: we first pick a tag without
// considering the neighbours, and only on a (rare) conflict pick again. This way the new tag does not depend on the neighbour
// loads (as the conflict branch is predicted) which keeps those loads off the critical path to the subsequent tag stores.
static inline void* mi_memtag_random_adjacent(void* p, size_t exclude, const void* prev, const void* next) {
  void* q = mi_memtag_random(p, exclude);
  #if !MI_MTE_ODD_EVEN
  void* const ptag = (prev != NULL ? mi_memtag_load((void*)prev) : NULL);
  void* const ntag = (next != NULL ? mi_memtag_load((void*)next) : NULL);
  const size_t tag = mi_ptr_memtag(q);
  if mi_unlikely((prev != NULL && mi_ptr_memtag(ptag) == tag) || (next != NULL && mi_ptr_memtag(ntag) == tag)) {
    if (prev != NULL) { exclude = mi_memtag_exclude_tag(ptag, exclude); }
    if (next != NULL) { exclude = mi_memtag_exclude_tag(ntag, exclude); }
    q = mi_memtag_random(p, exclude);
  }
  #else
  MI_UNUSED(prev); MI_UNUSED(next);
  #endif
  return q;
}

/* ----------------------------------------------------------------------------
  Tagging blocks
-----------------------------------------------------------------------------*/

// Tag `count` fresh blocks of size `bsize` at `start` where the first block has index `i` in a page of `n` blocks (with tag class index `i + parity`).
// The block before the first one (if `i > 0`) is already tagged, while the blocks after the last one are not yet tagged (and exclude the tag
// of the last block when they are tagged later on). The first and last block of the page use their tag class (see `mi_memtag_exclude_base`).
static inline void mi_memtag_blocks_init(void* start, size_t bsize, size_t i, size_t n, size_t parity, size_t count) {
  uint8_t* p = (uint8_t*)start;
  #if !MI_MTE_ODD_EVEN
  void* prev = (i > 0 ? mi_memtag_load(p - bsize) : NULL);
  #endif
  for (const size_t end = i + count; i < end; i++, p += bsize) {
    size_t exclude = mi_memtag_exclude_base(i + parity, i == 0 || i + 1 == n);
    #if !MI_MTE_ODD_EVEN
    if (prev != NULL) { exclude = mi_memtag_exclude_tag(prev, exclude); }
    #endif
    void* const tp = mi_memtag_random(p, exclude);
    mi_memtag_store(tp, bsize);
    #if !MI_MTE_ODD_EVEN
    prev = tp;
    #endif
  }
}

// Retag a block on free with a fresh random tag that differs from its current tag, is not in `exclude`, and differs from the adjacent
// blocks at `prev` and `next` (if not NULL, see `mi_memtag_random_adjacent`),
// and zero it if it is small; returns the block with its new tag. The first read uses the current tag of `block` so a stale pointer (e.g. a double free) faults.
static inline void* mi_memtag_block_free(void* block, size_t bsize, size_t exclude, const void* prev, const void* next) {
  mi_memtag_check(block);
  void* const tblock = mi_memtag_random_adjacent(block, mi_memtag_exclude_tag(block, exclude), prev, next);
  if (bsize <= MI_ZERO_ON_FREE_MAX) { mi_memtag_store_zero(tblock, bsize); }
                              else { mi_memtag_store(tblock, bsize); }
  return tblock;
}

// Retag a canonical block (with the meta tag, see `_mi_memtag_canonical`) with a fresh random tag that is not in `exclude`
// and differs from the adjacent blocks at `prev` and `next` (if not NULL, see `mi_memtag_random_adjacent`); returns the block
// with its new tag. The first read uses the meta tag so a stale pointer faults.
static inline void* mi_memtag_block_uncanonical(void* block, size_t bsize, size_t exclude, const void* prev, const void* next) {
  mi_memtag_check(block);
  void* const tblock = mi_memtag_random_adjacent(block, exclude, prev, next);
  mi_memtag_store(tblock, bsize);
  return tblock;
}

// Reset the tags of `[p,p+size)` to the meta data tag
static inline void mi_memtag_clear(void* p, size_t size) {
  mi_memtag_store(mi_ptr_memtag_meta(p), size);
}

// Tag the meta data `[p,p+size)` with a random tag that is not in `exclude` and differs from its current tag; returns `p` with its new tag.
static inline void* mi_memtag_meta_init_ex(void* p, size_t size, size_t exclude) {
  uint8_t* const q = (uint8_t*)mi_ptr_memtag_meta(p);
  void* const tp = mi_memtag_random(q, mi_memtag_exclude_tag(mi_memtag_load(q), exclude));
  mi_memtag_store(tp, size);
  return tp;
}

// Tag the meta data `[p,p+size)` with a random tag that differs from its current tag; returns `p` with its new tag.
// The meta data is part of a region of adjacent meta data where `idx` is its index (or the index of the first part plus
// its distance). If `has_prev` (or `has_next`) is set, the adjacent meta data before (or after) it is part of the same region
// (and can be read) such that we exclude its tag. The edges of a region use the full tag range as well: in secure mode
// the meta data regions are surrounded by guard pages, and otherwise adjacent regions can rarely have the same tag.
// With `MI_MTE_ODD_EVEN` the tag class of `idx` is used instead (and the neighbours are not read).
static inline void* mi_memtag_meta_init_adjacent(void* p, size_t size, size_t idx, bool has_prev, bool has_next) {
  uint8_t* const q = (uint8_t*)mi_ptr_memtag_meta(p);
  return mi_memtag_meta_init_ex(q, size, mi_memtag_exclude_adjacent(idx, false, (has_prev ? q - MI_MEMTAG_UNIT_SIZE : NULL), (has_next ? q + size : NULL)));
}

// Tag the meta data `[p,p+size)` with a random tag that differs from its current tag and from the tag of the (tagged) pointer
// `other` (if not NULL); returns `p` with its new tag. This is used for meta data in a separate OS allocation (see
// `mi_memtag_meta_init_adjacent` for the use of `idx`).
static inline void* mi_memtag_meta_init_separate(void* p, size_t size, size_t idx, const void* other) {
  const size_t exclude = mi_memtag_exclude_base(idx, false);
  return mi_memtag_meta_init_ex(p, size, (other != NULL ? mi_memtag_exclude_tag(other, exclude) : exclude));
}

// Tag the meta data `[p,p+size)` with a random tag of the class of index `idx` that differs from its current tag; returns `p` with its new tag.
// This is used for meta data whose tag must differ from adjacent memory that cannot be read, where that memory uses the other tag class.
static inline void* mi_memtag_meta_init_class(void* p, size_t size, size_t idx) {
  return mi_memtag_meta_init_ex(p, size, mi_memtag_exclude_class(idx));
}

#else
static inline size_t mi_memtag_cpu_line_size(void)             { return 0; }
static inline void  mi_memtag_check(const void* p)             { MI_UNUSED(p); }
static inline void* mi_memtag_random(void* p, size_t exclude)  { MI_UNUSED(exclude); return p; }
static inline void* mi_memtag_load(void* p)                    { return p; }
static inline void  mi_memtag_store(void* p, size_t size)      { MI_UNUSED(p); MI_UNUSED(size); }
static inline void  mi_memtag_store_zero(void* p, size_t size) { MI_UNUSED(p); MI_UNUSED(size); }
static inline size_t mi_memtag_exclude_tag(const void* p, size_t exclude) { MI_UNUSED(p); return exclude; }
static inline size_t mi_memtag_exclude_adjacent(size_t idx, bool edge, const void* prev, const void* next) {
  MI_UNUSED(idx); MI_UNUSED(edge); MI_UNUSED(prev); MI_UNUSED(next); return 0;
}
static inline void mi_memtag_blocks_init(void* start, size_t bsize, size_t i, size_t n, size_t parity, size_t count) {
  MI_UNUSED(start); MI_UNUSED(bsize); MI_UNUSED(i); MI_UNUSED(n); MI_UNUSED(parity); MI_UNUSED(count);
}
static inline void* mi_memtag_random_adjacent(void* p, size_t exclude, const void* prev, const void* next) {
  MI_UNUSED(exclude); MI_UNUSED(prev); MI_UNUSED(next); return p;
}
static inline void* mi_memtag_block_free(void* block, size_t bsize, size_t exclude, const void* prev, const void* next) {
  MI_UNUSED(bsize); MI_UNUSED(exclude); MI_UNUSED(prev); MI_UNUSED(next); return block;
}
static inline void* mi_memtag_block_uncanonical(void* block, size_t bsize, size_t exclude, const void* prev, const void* next) {
  MI_UNUSED(bsize); MI_UNUSED(exclude); MI_UNUSED(prev); MI_UNUSED(next); return block;
}
static inline void mi_memtag_clear(void* p, size_t size) {
  MI_UNUSED(p); MI_UNUSED(size);
}
static inline void* mi_memtag_meta_init_adjacent(void* p, size_t size, size_t idx, bool has_prev, bool has_next) {
  MI_UNUSED(size); MI_UNUSED(idx); MI_UNUSED(has_prev); MI_UNUSED(has_next); return p;
}
static inline void* mi_memtag_meta_init_class(void* p, size_t size, size_t idx) {
  MI_UNUSED(size); MI_UNUSED(idx); return p;
}
static inline void* mi_memtag_meta_init_separate(void* p, size_t size, size_t idx, const void* other) {
  MI_UNUSED(size); MI_UNUSED(idx); MI_UNUSED(other); return p;
}
#endif

#endif // MI_PRIM_MTE_H
