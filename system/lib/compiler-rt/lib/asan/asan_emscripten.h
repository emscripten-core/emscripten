#include "sanitizer_common/sanitizer_platform.h"

#ifndef ASAN_EMSCRIPTEN_H
#define ASAN_EMSCRIPTEN_H
#include <emscripten.h>

#if !SANITIZER_EMSCRIPTEN
#error "Can only be used on Emscripten!"
#endif

namespace __asan {
extern "C" {
void emasan_set_can_poison(bool value);
bool emasan_can_poison();
void emasan_poison(uptr aligned_beg, uptr aligned_size, u8 value);
void emasan_poison_right(uptr aligned_addr, uptr size, uptr redzone_size,
                         u8 value, bool partial);
void emasan_intra_object_red_zone(uptr ptr, uptr end, bool poison);
bool emasan_is_poisoned(uptr a);
bool emasan_check_poison(uptr addr, uptr size);
bool emasan_range_good_aligned(uptr begin, uptr end);
u8 emasan_shadow_read(uptr addr);
void emasan_shadow_write(uptr addr, u8 value);
}
} // namespace __asan

#endif // ASAN_EMSCRIPTEN_H
