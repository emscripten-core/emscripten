//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "abort_message.h"

#ifdef __BIONIC__
#   include <android/api-level.h>
#   if __ANDROID_API__ >= 21
#       include <syslog.h>
        extern "C" void android_set_abort_message(const char* msg);
#   else
#       include <assert.h>
#   endif // __ANDROID_API__ >= 21
#endif // __BIONIC__

#if defined(__APPLE__) && __has_include(<CrashReporterClient.h>)
#   include <CrashReporterClient.h>
#   define _LIBCXXABI_USE_CRASHREPORTER_CLIENT
#endif

void abort_message(const char* format, ...)
{
    // Write message to stderr. We do this before formatting into a
    // variable-size buffer so that we still get some information if
    // formatting into the variable-sized buffer fails.
#if !defined(NDEBUG) || !defined(LIBCXXABI_BAREMETAL)
    {
#if defined(__EMSCRIPTEN__) && defined(NDEBUG)
        // Just trap in a non-debug build. These internal libcxxabi assertions are
        // very rare, and it's not worth linking in vfprintf stdio support or
        // even minimal logging for them, as we'll have a proper call stack, which
        // will show a call into "abort_message", and can help debugging. (In a
        // debug build that won't be inlined.)
        abort();
#else
        fprintf(stderr, "libc++abi: ");
        va_list list;
        va_start(list, format);
        vfprintf(stderr, format, list);
        va_end(list);
        fprintf(stderr, "\n");
#endif
    }
#endif

    // Format the arguments into an allocated buffer. We leak the buffer on
    // purpose, since we're about to abort() anyway.
#if defined(_LIBCXXABI_USE_CRASHREPORTER_CLIENT)
    char* buffer;
    va_list list;
    va_start(list, format);
    vasprintf(&buffer, format, list);
    va_end(list);

    CRSetCrashLogMessage(buffer);
#elif defined(__BIONIC__)
    char* buffer;
    va_list list;
    va_start(list, format);
    vasprintf(&buffer, format, list);
    va_end(list);

#   if __ANDROID_API__ >= 21
    // Show error in tombstone.
    android_set_abort_message(buffer);

    // Show error in logcat.
    openlog("libc++abi", 0, 0);
    syslog(LOG_CRIT, "%s", buffer);
    closelog();
#   else
    // The good error reporting wasn't available in Android until L. Since we're
    // about to abort anyway, just call __assert2, which will log _somewhere_
    // (tombstone and/or logcat) in older releases.
    __assert2(__FILE__, __LINE__, __func__, buffer);
#   endif // __ANDROID_API__ >= 21
#endif // __BIONIC__

    abort();
}
