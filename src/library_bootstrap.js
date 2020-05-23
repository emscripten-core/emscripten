/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// When bootstrapping struct info, we can't use the full libray because
// it itself depends on the struct info information.

#if !BOOTSTRAPPING_STRUCT_INFO
assert(false)
#endif

assert(!LibraryManager.library);
LibraryManager.library = {};
