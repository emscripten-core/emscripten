//===-- sanitizer_errno_codes.h ---------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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

#define errno_ENOMEM       __WASI_ERRNO_NOMEM
#define errno_EBUSY        __WASI_ERRNO_BUSY
#define errno_EINVAL       __WASI_ERRNO_INVAL
#define errno_ENAMETOOLONG __WASI_ERRNO_NAMETOOLONG
#define errno_ENOSYS       __WASI_ERRNO_NOSYS

// Those might not present or their value differ on different platforms.
extern const int errno_EOWNERDEAD;

}  // namespace __sanitizer

#endif  // SANITIZER_ERRNO_CODES_H
