// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Trivially constructible/destructible value_object and value_array argument
// temporaries live on the wasm stack instead of round-tripping through
// new/delete.  malloc and free are overridden here to count allocations, so
// the hot loops below can assert the trivial paths allocate nothing while the
// non-trivial and over-aligned fallbacks still use (and balance) the heap.

#include <assert.h>
#include <array>
#include <cstdint>
#include <new>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/heap.h>

static int allocCount = 0;
static int freeCount = 0;

void* malloc(size_t size) {
  ++allocCount;
  return emscripten_builtin_malloc(size);
}

void free(void* ptr) {
  ++freeCount;
  emscripten_builtin_free(ptr);
}

// Over-aligned new does not route through plain malloc, so count it too.
void* operator new(size_t size, std::align_val_t align) {
  ++allocCount;
  return emscripten_builtin_memalign((size_t)align, size);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
  ++freeCount;
  emscripten_builtin_free(ptr);
}

// Trivial value_object: takes the stack path.
struct Id {
  int32_t index;
  uint16_t world;
  uint16_t generation;
};

using Vec3 = std::array<float, 3>;

int sumId(Id id) {
  return id.index + id.world + id.generation;
}

float sumVec(Id id, Vec3 v) {
  return (float)id.index + v[0] + v[1] + v[2];
}

// Only `regged` is registered below; the wire object must still be
// value-initialized, so the unregistered field reads back zero.
struct Partial {
  int32_t regged;
  int32_t unregged;
};

int32_t readUnregistered(Partial p) {
  return p.unregged;
}

// Non-trivial control: must keep the heap path and run its destructor.
struct Tracked {
  int32_t value;
  Tracked() : value(0) { ++liveCount; }
  Tracked(const Tracked& o) : value(o.value) { ++liveCount; }
  ~Tracked() { --liveCount; }
  static int liveCount;
};
int Tracked::liveCount = 0;

int useTracked(Tracked t) {
  return t.value;
}

// Trivial but over-aligned past STACK_ALIGN: must keep the heap path, and the
// wire object must actually satisfy the alignment.
struct alignas(64) BigAligned {
  float x;
  float y;
  float z;
  float w;
};

float useBig(BigAligned b) {
  return b.x + b.y + b.z + b.w;
}

// The low 32 bits are enough for the alignment check and keep the return a
// plain JS number under wasm64.
uint32_t bigAddr(const BigAligned& b) {
  return (uint32_t)(uintptr_t)&b;
}

EMSCRIPTEN_BINDINGS(trivial_value_stack) {
  emscripten::value_object<Id>("Id")
      .field("index", &Id::index)
      .field("world", &Id::world)
      .field("generation", &Id::generation);

  emscripten::value_array<Vec3>("Vec3")
      .element(emscripten::index<0>())
      .element(emscripten::index<1>())
      .element(emscripten::index<2>());

  emscripten::value_object<Partial>("Partial")
      .field("regged", &Partial::regged);

  emscripten::value_object<Tracked>("Tracked")
      .field("value", &Tracked::value);

  emscripten::value_object<BigAligned>("BigAligned")
      .field("x", &BigAligned::x)
      .field("y", &BigAligned::y)
      .field("z", &BigAligned::z)
      .field("w", &BigAligned::w);

  emscripten::function("sumId", &sumId);
  emscripten::function("sumVec", &sumVec);
  emscripten::function("readUnregistered", &readUnregistered);
  emscripten::function("useTracked", &useTracked);
  emscripten::function("useBig", &useBig);
  emscripten::function("bigAddr", &bigAddr);
}

int main() {
  // Warm every call once, then reset the counters so startup noise and any
  // first-call work is ignored.  The printf warms stdio's buffer allocation.
  printf("start\n");
  EM_ASM({
    var id = ({ index: 1, world: 2, generation: 3 });
    if (Module['sumId'](id) !== 6) throw 'sumId warmup';
    if (Module['sumVec'](id, [1, 2, 3]) !== 7) throw 'sumVec warmup';
    if (Module['readUnregistered']({ regged: 9 }) !== 0) throw 'unregistered field not zeroed';
    if (Module['useTracked']({ value: 7 }) !== 7) throw 'useTracked warmup';
    if (Module['useBig']({ x: 1, y: 2, z: 3, w: 4 }) !== 10) throw 'useBig warmup';
  });
  allocCount = 0;
  freeCount = 0;

  // Trivial value_object and value_array arguments must not touch the heap.
  EM_ASM({
    var id = ({ index: 1, world: 2, generation: 3 });
    for (var i = 0; i < 100; i++) {
      Module['sumId'](id);
      Module['sumVec'](id, [4, 5, 6]);
      Module['readUnregistered']({ regged: i });
    }
  });
  printf("trivial allocs: %d frees: %d\n", allocCount, freeCount);
  assert(allocCount == 0 && freeCount == 0);

  // A conversion that throws mid-call must unwind the stack frame; the
  // stackSave comparison below catches any leaked frame.
  EM_ASM({
    var trap = ({ index: 1, world: 2, get generation() { throw new Error('boom'); } });
    var sp = stackSave();
    for (var i = 0; i < 1000; i++) {
      try {
        Module['sumVec'](trap, [1, 2, 3]);
        // No .message on this string, so the catch below rethrows it.
        throw 'expected sumVec to throw';
      } catch (e) {
        if (e.message !== 'boom') throw e;
      }
    }
    if (stackSave() !== sp) throw 'stack leaked across throwing conversions';
    if (Module['sumId']({ index: 1, world: 2, generation: 3 }) !== 6) throw 'sumId broken after throws';
  });
  printf("throwing allocs: %d frees: %d\n", allocCount, freeCount);
  assert(allocCount == 0 && freeCount == 0);

  // The non-trivial type stays on the heap path, balances its allocations,
  // and runs its destructor.
  EM_ASM({
    for (var i = 0; i < 10; i++) {
      if (Module['useTracked']({ value: i }) !== i) throw 'useTracked';
    }
  });
  printf("tracked allocs: %d frees: %d live: %d\n", allocCount, freeCount, Tracked::liveCount);
  assert(allocCount == 10 && freeCount == 10);
  assert(Tracked::liveCount == 0);

  // The over-aligned trivial type stays on the heap path and its wire object
  // honors alignas(64); the modulo runs in JS because C++ would fold it away.
  allocCount = 0;
  freeCount = 0;
  EM_ASM({
    for (var i = 0; i < 10; i++) {
      if (Module['useBig']({ x: 1, y: 2, z: 3, w: i }) !== 6 + i) throw 'useBig';
      if (Module['bigAddr']({ x: 1, y: 2, z: 3, w: 4 }) % 64 !== 0) throw 'BigAligned wire object misaligned';
    }
  });
  printf("aligned allocs: %d frees: %d\n", allocCount, freeCount);
  assert(allocCount == 20 && freeCount == 20);

  printf("done\n");
  return 0;
}
