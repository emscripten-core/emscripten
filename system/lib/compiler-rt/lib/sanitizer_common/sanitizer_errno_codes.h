//===-- sanitizer_errno_codes.h ---------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is shared between sanitizers run-time libraries.
//
// Defines errno codes to avoid including errno.h and its dependencies into
// sensitive files (e.g. interceptors are not supposed to include any system
// headers).
// It's ok to use errno.h directly when your file already depend on other system
// includes though.
//
//===----------------------------------------------------------------------===//

#ifndef SANITIZER_ERRNO_CODES_H
#define SANITIZER_ERRNO_CODES_H

// XXX EMSCRIPTEN: use wasi errno codes, which is what our musl port now uses
#include <wasi/api.h>

namespace __sanitizer {

#define errno_ENOMEM __WASI_ERRNO_NOMEM
#define errno_EBUSY  __WASI_ERRNO_BUSY
#define errno_EINVAL __WASI_ERRNO_INVAL

// Those might not present or their value differ on different platforms.
extern const int errno_EOWNERDEAD;

}  // namespace __sanitizer

#endif  // SANITIZER_ERRNO_CODES_H
