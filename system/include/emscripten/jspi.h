/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>

// Lifecycle hooks for JSPI (-sJSPI) stack-switching fibers (-sJSPI_HOOKS).
//
// A fiber is the wasm activation started by a call to a promising export. It
// is entered once, may be suspended and resumed any number of times while its
// suspending imports await, and exits once, normally or with an exception.
// A suspension is a leave of the fiber: while it is suspended other fibers
// (or the top level) run on the same thread and see the same static storage
// and shadow stack, so libraries with fiber-affine state stash it on
// JSPI_SUSPEND and restore it on JSPI_RESUME, with the lifetime bounded by
// JSPI_ENTER/JSPI_EXIT. Fibers never move between threads.

#ifdef __cplusplus
extern "C" {
#endif

// Hooks run synchronously inside the boundary call, on whatever stack the
// fiber's code runs on: ENTER before the export body (the fiber is already
// current), SUSPEND before the import is called with the fiber's frames still
// live, RESUME after the import returned with the frames live again, and EXIT
// after the export body returned, with the fiber still current. A trap inside
// a fiber bypasses the hooks and leaves the JSPI state (and, with
// REENTRANT_JSPI, the stack pointer) undefined.
//
// Events are bit flags so that they combine into the mask for jspi_register.
typedef enum {
  // A promising export was called.
  JSPI_ENTER = 1,
  // The promising export is returning (or throwing).
  JSPI_EXIT = 2,
  // A suspending import is about to be called (it may or may not suspend).
  JSPI_SUSPEND = 4,
  // The suspending import has returned (or thrown); the fiber is running again.
  JSPI_RESUME = 8,
} jspi_event;

#define JSPI_ALL (JSPI_ENTER | JSPI_EXIT | JSPI_SUSPEND | JSPI_RESUME)

// Called for each event of every fiber, inside the fiber's own frames
// immediately before/after the boundary call. `token` is the hook's own
// per-fiber value: NULL at the first event the hook sees for a fiber, then
// whatever the hook returned at the previous event of that fiber; the return
// value at JSPI_EXIT is ignored. `error` is nonzero when the export or import
// completed with an exception (a JS exception, a rejected promise or a wasm
// exception such as a C++ throw), which is rethrown unchanged after the hooks
// run; hooks cannot inspect it. Hooks must not throw, suspend, or call
// promising exports.
typedef void* (*jspi_hook)(jspi_event event, void* token, int error);

// Registers `fn` for the events in `mask` on the calling thread. Hooks run in
// registration order. Returns 0, -1 if the program was linked without
// -sJSPI_HOOKS (no events are ever delivered), or -2 if the table is full
// (JSPI_MAX_HOOKS registrations per thread).
int jspi_register(jspi_hook fn, uint32_t mask);

#define JSPI_MAX_HOOKS 64

#ifdef __cplusplus
}
#endif
