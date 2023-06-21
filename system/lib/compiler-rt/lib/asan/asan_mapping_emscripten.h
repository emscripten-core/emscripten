//===-- asan_mapping_emscripten.h -------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// Emscripten-specific definitions for ASan memory mapping.
//===----------------------------------------------------------------------===//
#ifndef ASAN_MAPPING_EMSCRIPTEN_H
#define ASAN_MAPPING_EMSCRIPTEN_H

extern char __global_base;

#define kLowMemBeg     ((uptr) &__global_base)
#define kLowMemEnd     ((kLowShadowBeg << ASAN_SHADOW_SCALE) - 1)

#define kLowShadowBeg  0
#define kLowShadowEnd  ((uptr) &__global_base - 1)

#define kHighMemBeg    0

#define kHighShadowBeg 0
#define kHighShadowEnd 0

#define kMidShadowBeg  0
#define kMidShadowEnd  0

#define kShadowGapBeg  (kLowMemEnd + 1)
#define kShadowGapEnd  0xFFFFFFFF

#define kShadowGap2Beg 0
#define kShadowGap2End 0

#define kShadowGap3Beg 0
#define kShadowGap3End 0

// The first 1/8 of the shadow memory space is shadowing itself.
// This allows attempted accesses into the shadow memory, as well as null
// pointer dereferences, to be detected properly.
// The shadow memory of the shadow memory is poisoned.
#define MEM_TO_SHADOW(mem) ((mem) >> ASAN_SHADOW_SCALE)
#define SHADOW_TO_MEM(mem) ((mem) << ASAN_SHADOW_SCALE)

namespace __asan {

static inline bool AddrIsInLowMem(uptr a) {
  PROFILE_ASAN_MAPPING();
  return a >= kLowMemBeg && a <= kLowMemEnd;
}

static inline bool AddrIsInLowShadow(uptr a) {
  PROFILE_ASAN_MAPPING();
  return a >= kLowShadowBeg && a <= kLowShadowEnd;
}

static inline bool AddrIsInMidMem(uptr a) {
  PROFILE_ASAN_MAPPING();
  return false;
}

static inline bool AddrIsInMidShadow(uptr a) {
  PROFILE_ASAN_MAPPING();
  return false;
}

static inline bool AddrIsInHighMem(uptr a) {
  PROFILE_ASAN_MAPPING();
  return false;
}

static inline bool AddrIsInHighShadow(uptr a) {
  PROFILE_ASAN_MAPPING();
  return false;
}

static inline bool AddrIsInShadowGap(uptr a) {
  PROFILE_ASAN_MAPPING();
  return a >= kShadowGapBeg;
}

}  // namespace __asan

#endif  // ASAN_MAPPING_EMSCRIPTEN_H
