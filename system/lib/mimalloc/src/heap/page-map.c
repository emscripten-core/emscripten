/*----------------------------------------------------------------------------
Copyright (c) 2023-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#include "mimalloc.h"
#include "mimalloc/util/memid.h"
#include "mimalloc/prim/prim-mte.h"
#include "mimalloc/alloc/page.h"
#include "mimalloc/arena/arena.h"
#include "mimalloc/heap/subproc.h"
#include "mimalloc/heap/page-map.h"
#include "../arena/bitmap.h"

static void mi_page_map_cannot_commit(void) {
  _mi_warning_message("unable to commit the allocation page-map on-demand\n" );
}

// A 2-level page map
#define MI_PAGE_MAP_SUB_SIZE          (sizeof(mi_page_submap_t))

// Set a (signed) sub map pointer in the page map
static void mi_page_map_set_at(mi_page_map_t* pmap, size_t idx, mi_page_submap_t* sub) {
  mi_atomic_ptr_sign_release(&pmap->submaps[idx], sub);
}

static size_t mi_page_map_count_of_size(size_t size) {
  return (size < sizeof(mi_page_map_t) ? 0 : 1 + (size - sizeof(mi_page_map_t))/sizeof(mi_ptr_signed_t));
}

static size_t mi_page_map_size_of_count(size_t count) {
  return (count == 0 ? 0 : sizeof(mi_page_map_t) + (count - 1)*sizeof(mi_ptr_signed_t));
}

// With memory tagging, the page map and its sub maps get a random tag (and are only accessed through the tagged pointers).
// The page map is tagged incrementally as it is committed on-demand.
static bool mi_page_map_use_memtag(const mi_memid_t* memid) {
  return mi_memtag_use_for_meta(!memid->is_pinned);
}

// static void* mi_page_map_addr_of_index(size_t idx) {
//   return (void*)((uintptr_t)idx * MI_PAGE_MAP_SUB_COUNT * MI_ARENA_SLICE_SIZE);
// }

// Store the committed count in the sealed process data
static void mi_page_map_store_committed(size_t committed_count) {
  mi_process_sealed_t* const sealed = _mi_process_unseal();
  mi_atomic_store_release(&sealed->page_map_committed, committed_count);
  _mi_process_seal();
}

// The page map with the authority to commit or free it (on CHERI), and without a memory tag
static mi_page_map_t* mi_page_map_vmem(mi_page_map_t* pmap) {
  return (mi_page_map_t*)mi_ptr_memtag_meta(mi_ptr_rederive(pmap->memid.mem.os.base, pmap));
}

mi_decl_nodiscard static mi_decl_noinline bool mi_page_map_commit_entries(mi_page_map_t* pmap, size_t required_idx) {
  const size_t reserved_count = mi_page_map_count_of_size(pmap->reserved_size);
  if mi_unlikely(required_idx >= reserved_count) {
    mi_page_map_cannot_commit();
    return false;
  }
  size_t commit_size = mi_align_up( sizeof(mi_page_map_t) + (required_idx * sizeof(mi_ptr_signed_t)), MI_ARENA_SLICE_SIZE );
  if (pmap->reserved_size < commit_size) { commit_size = pmap->reserved_size; }
  const size_t commit_count = mi_page_map_count_of_size(commit_size);
  mi_assert_internal(commit_count > required_idx);
  mi_assert_internal(commit_count <= reserved_count);
  const size_t prev_count = mi_page_map_committed();
  // note: we rely on uncommitted memory to be zero initialized on the first commit (and further concurrent commits leave the memory as is).
  if (!pmap->memid.initially_committed) {  // can be initially committed with memory tagging (see `mi_page_map_init_once`)
    bool is_zero;
    if mi_unlikely(!_mi_os_commit(_mi_subproc_main(), mi_page_map_vmem(pmap), commit_size, &is_zero)) {
      mi_page_map_cannot_commit();
      return false;
    }
    mi_assert_internal(is_zero || pmap->memid.initially_zero);
  }
  // tag the new entries with the tag of `pmap` before publishing the new count
  // (concurrent commits may tag the same entries again but that leaves the memory as is)
  if (mi_page_map_use_memtag(&pmap->memid)) {
    const size_t tag_start = mi_align_down(mi_page_map_size_of_count(prev_count), MI_MEMTAG_UNIT_SIZE);
    mi_memtag_store((uint8_t*)pmap + tag_start, commit_size - tag_start);
  }
  mi_page_map_store_committed(commit_count);
  return true;
}

mi_decl_nodiscard static bool mi_page_map_ensure_committed(mi_page_map_t* pmap, size_t idx, mi_page_submap_t** submap) {
  mi_assert_internal(submap!=NULL && *submap==NULL);
  if mi_unlikely(idx >= mi_page_map_committed()) {
    // commit under the lock so the committed count only increases
    bool ok = true;
    mi_lock(&pmap->lock) {
      if (idx >= mi_atomic_load_acquire(&mi_process_get_sealed()->page_map_committed)) {
        ok = mi_page_map_commit_entries(pmap,idx);
      }
    }
    if (!ok) return false;
    mi_assert_internal(idx < mi_page_map_committed());
  }
  *submap = mi_page_map_at(pmap, idx);
  return true;
}

// initialize the page map
static bool mi_page_map_init_once(void) {
  size_t vbits = (size_t)mi_option_get_clamp(mi_option_max_vabits, 0, MI_MAX_VABITS);
  if (vbits == 0) {
    vbits = _mi_os_virtual_address_bits();
    #if MI_ARCH_X64  // canonical address is limited to the first 128 TiB
    if (vbits >= 48) { vbits = 47; }
    #endif
  }
  if (vbits < MI_PAGE_MAP_SUB_SHIFT + MI_ARENA_SLICE_SHIFT) {
    vbits = MI_PAGE_MAP_SUB_SHIFT + MI_ARENA_SLICE_SHIFT;
  }
  if (vbits < MI_MIN_VABITS) {    // cover at least this much for a faster _mi_checked_ptr
    vbits = MI_MIN_VABITS;
  }
  if (vbits > MI_MAX_VABITS) {    // limit page map size even if more virtual addresses are available
    vbits = MI_MAX_VABITS;
  }

  // Allocate the page map and commit bits
  mi_assert(MI_MAX_VABITS >= vbits);
  mi_assert(MI_MIN_VABITS <= vbits);
  const size_t reserve_count    = (MI_ZU(1) << (vbits - MI_PAGE_MAP_SUB_SHIFT - MI_ARENA_SLICE_SHIFT));
  const size_t os_page_size  = _mi_os_page_size();
  const size_t reserve_size  = mi_align_up( sizeof(mi_page_map_t) + ((reserve_count - 1) * sizeof(mi_ptr_signed_t)), os_page_size);
  const size_t submap_size   = MI_PAGE_MAP_SUB_SIZE;
  const size_t extra_reserve_size  = reserve_size + submap_size;
  const bool commit = (vbits == MI_MIN_VABITS) || (reserve_size <= 64*MI_KiB) || // 42 virtual address bits
                      mi_option_is_enabled(mi_option_pagemap_commit) || _mi_os_has_overcommit();
  mi_subproc_t* const subproc = _mi_subproc_main();
  mi_memid_t memid;
  mi_page_map_t* pmap = (mi_page_map_t*)_mi_os_alloc_with_guards(subproc, extra_reserve_size, commit, true /* allow large */, &memid);
  if mi_unlikely(pmap==NULL) {
    _mi_error_message(ENOMEM, "unable to reserve virtual memory for the page map (%zu KiB)\n", extra_reserve_size / MI_KiB);
    return false;
  }

  // commit first entries up to MI_MIN_VABITS entries
  const bool use_memtag = mi_page_map_use_memtag(&memid);
  const size_t min_commit_count = (MI_ZU(1) << (MI_MIN_VABITS - MI_PAGE_MAP_SUB_SHIFT - MI_ARENA_SLICE_SHIFT));
  const size_t min_commit_size = mi_align_up( mi_page_map_size_of_count(min_commit_count), os_page_size);
  mi_assert_internal(min_commit_size <= reserve_size);
  size_t commit_count;
  if (memid.initially_committed) {
    if (!memid.initially_zero) {
      _mi_warning_message("internal: the page map was committed but not zero initialized!\n");
      mi_memzero_aligned(pmap, extra_reserve_size);
      memid.initially_zero = true;
    }
    // with memory tagging, we still "commit" on-demand so we only tag the used part of the page map
    commit_count = mi_page_map_count_of_size(use_memtag ? min_commit_size : reserve_size);
  }
  else {
    bool is_zero;
    if (!_mi_os_commit(subproc,pmap,min_commit_size,&is_zero)) {
      mi_page_map_cannot_commit();
      _mi_os_free_with_guards(subproc,memid);
      return false;
    };
    mi_assert_internal(is_zero || memid.initially_zero);
    commit_count = mi_page_map_count_of_size(min_commit_size);
  }
  mi_assert_internal(commit_count >= min_commit_count);

  // ensure there is a submap for the NULL address
  mi_page_submap_t* sub0 = (mi_page_submap_t*)((uint8_t*)pmap + reserve_size);  // we reserved a submap part at the end already
  if (!memid.initially_committed) {
    if (!_mi_os_commit(subproc, sub0, submap_size, NULL)) {  // commit full submap (issue #1087)
      mi_page_map_cannot_commit();
      _mi_os_free_with_guards(subproc,memid);
      return false;
    }
  }
  if (!memid.initially_zero) {     // initialize low addresses with NULL
    mi_memzero_aligned(sub0, submap_size);
  }

  // tag the committed page map entries and the first sub map (with a different tag as it is adjacent once the page map is fully committed)
  if (use_memtag) {
    pmap = (mi_page_map_t*)mi_memtag_meta_init_separate(pmap, mi_align_up(mi_page_map_size_of_count(commit_count), MI_MEMTAG_UNIT_SIZE), 0, NULL);
    sub0 = (mi_page_submap_t*)mi_memtag_meta_init_separate(sub0, submap_size, 1, pmap);
  }

  // initialize the fields
  pmap->memid = memid;
  pmap->reserved_size  = reserve_size;
  mi_lock_init(&pmap->lock);
  mi_page_map_set_at(pmap, 0, (mi_page_submap_t*)mi_ptr_bounded_data(sub0, submap_size));
  // set the page map (once) before the committed count as `mi_checked_ptr_page` only reads the page map if the count is non-zero
  mi_process_sealed_once_t* const once = _mi_process_unseal_once();
  mi_page_map_t* const pmap_bounded = (mi_page_map_t*)mi_ptr_bounded_data(pmap, reserve_size);  // see `mi_page_map_vmem`
  mi_atomic_store_ptr_release(mi_page_map_t, &once->page_map, (mi_page_map_t*)mi_ptr_memtag_meta(pmap_bounded));  // store without memory tag (see `mi_page_map_untagged`)
  _mi_process_seal_once();
  mi_page_map_store_committed(commit_count);
  mi_assert_internal(mi_ptr_page(NULL)==NULL);
  return true;
}

bool _mi_page_map_init(void) {
  mi_atomic_do_once {
    mi_page_map_init_once();
  }
  return (mi_page_map() != NULL);
}

// Sub maps are allocated with guard pages around them in secure mode
static mi_page_submap_t* mi_page_map_alloc_submap(mi_subproc_t* subproc, mi_memid_t* memid) {
  mi_page_submap_t* sub = (mi_page_submap_t*)_mi_os_alloc_with_guards(subproc, MI_PAGE_MAP_SUB_SIZE, true /* commit */, false /* allow large */, memid);
  if (sub != NULL && !memid->initially_zero) {
    mi_memzero_aligned(sub, MI_PAGE_MAP_SUB_SIZE);
  }
  if (sub != NULL && mi_page_map_use_memtag(memid)) {
    sub = (mi_page_submap_t*)mi_memtag_meta_init_separate(sub, MI_PAGE_MAP_SUB_SIZE, 0, NULL);
  }
  return sub;
}

// We do not store the memid of a sub map but can recreate it (see `_mi_os_alloc_with_guards`)
static mi_memid_t mi_page_map_submap_memid(mi_page_submap_t* sub) {
  const size_t guard_size = _mi_os_secure_guard_page_size();
  const size_t size = _mi_os_good_alloc_size(MI_PAGE_MAP_SUB_SIZE + 2*guard_size);
  return _mi_memid_create_os((uint8_t*)mi_ptr_memtag_meta(sub) - guard_size, size, size, false, false);
}

void _mi_page_map_unsafe_destroy(void) {
  mi_page_map_t* const pmap = mi_page_map();
  mi_assert_internal(pmap != NULL);
  if (pmap == NULL) return;
  mi_subproc_t* const subproc = _mi_subproc_main();
  mi_lock_done(&pmap->lock);
  const size_t committed_count = mi_page_map_committed();
  mi_page_map_store_committed(0);  // before freeing so `mi_checked_ptr_page` returns NULL (the page map itself is sealed and not reset)
  for (size_t idx = 1; idx < committed_count; idx++) {  // skip entry 0 (as we allocate that submap at the end of the page_map)
    // free all sub-maps
    mi_page_submap_t* sub = mi_page_map_at(pmap,idx);
    if (sub != NULL) {
      _mi_os_free_with_guards(subproc, mi_page_map_submap_memid(sub));
      mi_page_map_set_at(pmap, idx, NULL);
    }
  }
  _mi_os_free_with_guards(subproc, pmap->memid);
}

mi_decl_nodiscard static mi_decl_noinline mi_page_submap_t* mi_page_map_alloc_submap_at(mi_page_map_t* pmap, size_t idx) {
  // sub map not yet allocated, alloc now
  mi_page_submap_t* sub = NULL;
  mi_lock(&pmap->lock)
  {
    sub = mi_page_map_at(pmap, idx); // reload
    if (sub==NULL) // not yet allocated by another thread?
    {
      mi_subproc_t* const subproc = _mi_subproc_main();
      mi_memid_t memid;
      sub = mi_page_map_alloc_submap(subproc, &memid);
      if (sub==NULL) {
        _mi_warning_message("internal error: unable to extend the page map\n");
      }
      else {
        mi_page_map_set_at(pmap, idx, sub);  // ok, since we hold the lock
      }
    }
  }
  return sub;
}

mi_decl_nodiscard static bool mi_page_map_ensure_submap_at(mi_page_map_t* pmap, size_t idx, mi_page_submap_t** submap) {
  mi_assert_internal(submap!=NULL && *submap==NULL);
  mi_page_submap_t* sub = NULL;
  if (!mi_page_map_ensure_committed(pmap, idx, &sub)) {
    return false;
  }
  if mi_unlikely(sub==NULL) {
    sub = mi_page_map_alloc_submap_at(pmap, idx);
    if (sub==NULL) return false; // unable to allocate the submap..
  }
  mi_assert_internal(sub!=NULL);
  *submap = sub;
  return true;
}

static bool mi_page_map_set_range_prim(mi_page_map_t* pmap, mi_page_t* page, size_t idx, size_t sub_idx, size_t slice_count) {
  // is the page map area that contains the page address committed?
  while (slice_count > 0) {
    mi_page_submap_t* sub = NULL;
    if (!mi_page_map_ensure_submap_at(pmap, idx, &sub)) {
      return false;
    };
    mi_assert_internal(sub!=NULL);
    // set the offsets for the page; use atomic stores as the entries of a freed OS page can be set by another
    // thread that reallocates the same address range (and the OS unmap and map do not synchronize for TSAN)
    while (slice_count > 0 && sub_idx < MI_PAGE_MAP_SUB_COUNT) {
      mi_atomic_ptr_sign_relaxed(&sub->page_metas[sub_idx], page);
      slice_count--;
      sub_idx++;
    }
    idx++; // potentially wrap around to the next idx
    sub_idx = 0;
  }
  return true;
}

static bool mi_page_map_set_range(mi_page_map_t* pmap, mi_page_t* page, size_t idx, size_t sub_idx, size_t slice_count) {
  if mi_unlikely(!mi_page_map_set_range_prim(pmap, page,idx,sub_idx,slice_count)) {
    // failed to commit, call again to reset the page pointer if needed
    if (page!=NULL) {
      mi_page_map_set_range_prim(pmap,NULL,idx,sub_idx,slice_count);
    }
    return false;
  }
  return true;
}

static size_t mi_page_map_get_idx(mi_page_t* page, size_t* sub_idx, size_t* slice_count) {
  size_t page_size;
  uint8_t* page_start = mi_page_area(page, &page_size);
  if (page_size > MI_LARGE_PAGE_SIZE) { page_size = MI_LARGE_PAGE_SIZE - MI_ARENA_SLICE_SIZE; }  // furthest interior pointer
  mi_assert_internal(page_size > 0);
  // count the slices spanned by `[page_start, page_start + page_size)` exactly
  const mi_addr_t start = mi_addr(page_start);
  *slice_count = ((start + page_size - 1) / MI_ARENA_SLICE_SIZE) - (start / MI_ARENA_SLICE_SIZE) + 1;
  return mi_page_map_index(page_start, sub_idx);
}

bool _mi_page_map_register(mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_page_map_t* pmap = mi_page_map();
  mi_assert_internal(pmap != NULL);  // should be initialized before multi-thread access!
  if mi_unlikely(pmap == NULL) {
    if (!_mi_page_map_init()) return false;
    pmap = mi_page_map();
  }
  mi_assert(pmap!=NULL);
  size_t   slice_count;
  size_t   sub_idx;
  const size_t idx = mi_page_map_get_idx(page, &sub_idx, &slice_count);
  return mi_page_map_set_range(pmap, page, idx, sub_idx, slice_count);
}

void _mi_page_map_unregister(mi_page_t* page) {
  mi_assert_internal(page != NULL);
  mi_assert_internal(mi_is_aligned(mi_page_slice_start(page), MI_PAGE_ALIGN));
  mi_page_map_t* const pmap = mi_page_map();
  // note: should proceed even if the page was not registered yet (for failure paths in page allocation in `arena/arena-page-alloc.c`)
  if mi_unlikely(pmap == NULL) return;
  // get index and count
  size_t slice_count;
  size_t sub_idx;
  const size_t idx = mi_page_map_get_idx(page, &sub_idx, &slice_count);
  // unset the offsets
  mi_page_map_set_range(pmap, NULL, idx, sub_idx, slice_count);
}

void _mi_page_map_unregister_range(void* start, size_t size) {
  mi_page_map_t* const pmap = mi_page_map();
  if mi_unlikely(pmap == NULL) return;
  const size_t slice_count = mi_divide_up(size, MI_ARENA_SLICE_SIZE);
  size_t sub_idx;
  const size_t idx = mi_page_map_index(start, &sub_idx);
  mi_page_map_set_range(pmap, NULL, idx, sub_idx, slice_count);  // todo: avoid committing if not already committed?
}

// Return NULL for invalid pointers
mi_page_t* _mi_safe_ptr_page(const void* p) {
  return mi_checked_ptr_page(p);
}

mi_decl_nodiscard mi_decl_export bool mi_is_in_heap_region(const void* p) mi_attr_noexcept {
  return (_mi_safe_ptr_page(p) != NULL);
}
