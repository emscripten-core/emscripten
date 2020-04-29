//===------------------------- cxa_handlers.cpp ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//
// This file implements the functionality associated with the terminate_handler,
//   unexpected_handler, and new_handler.
//===----------------------------------------------------------------------===//

#include <stdexcept>
#include <new>
#include <exception>
#include "abort_message.h"
#include "cxxabi.h"
#include "cxa_handlers.h"
#include "cxa_exception.h"
#include "private_typeinfo.h"
#include "include/atomic_support.h"

namespace __cxxabiv1 {

#ifdef __USING_EMSCRIPTEN_EXCEPTIONS__
// XXX EMSCRIPTEN: Copied from cxa_exception.cpp since we don't compile that
// file in Emscripten EH mode. Note that in no-exceptions builds we include
// cxa_noexception.cpp which provides stubs of those anyhow.

//  Is it one of ours?
uint64_t __getExceptionClass(const _Unwind_Exception* unwind_exception) {
//	On x86 and some ARM unwinders, unwind_exception->exception_class is
//		a uint64_t. On other ARM unwinders, it is a char[8]
//	See: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0038b/IHI0038B_ehabi.pdf
//	So we just copy it into a uint64_t to be sure.
	uint64_t exClass;
	::memcpy(&exClass, &unwind_exception->exception_class, sizeof(exClass));
	return exClass;
}

bool __isOurExceptionClass(const _Unwind_Exception* unwind_exception) {
    return (__getExceptionClass(unwind_exception) & get_vendor_and_language) == 
           (kOurExceptionClass                    & get_vendor_and_language);
}
#endif

}

namespace std
{

unexpected_handler
get_unexpected() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_unexpected_handler, _AO_Acquire);
}

void
__unexpected(unexpected_handler func)
{
    func();
    // unexpected handler should not return
    abort_message("unexpected_handler unexpectedly returned");
}

__attribute__((noreturn))
void
unexpected()
{
    __unexpected(get_unexpected());
}

terminate_handler
get_terminate() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_terminate_handler, _AO_Acquire);
}

void
__terminate(terminate_handler func) _NOEXCEPT
{
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    try
    {
#endif  // _LIBCXXABI_NO_EXCEPTIONS
        func();
        // handler should not return
        abort_message("terminate_handler unexpectedly returned");
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    }
    catch (...)
    {
        // handler should not throw exception
        abort_message("terminate_handler unexpectedly threw an exception");
    }
#endif  // _LIBCXXABI_NO_EXCEPTIONS
}

__attribute__((noreturn))
void
terminate() _NOEXCEPT
{
#ifndef _LIBCXXABI_NO_EXCEPTIONS
    // If there might be an uncaught exception
    using namespace __cxxabiv1;
    __cxa_eh_globals* globals = __cxa_get_globals_fast();
    if (globals)
    {
        __cxa_exception* exception_header = globals->caughtExceptions;
        if (exception_header)
        {
            _Unwind_Exception* unwind_exception =
                reinterpret_cast<_Unwind_Exception*>(exception_header + 1) - 1;
            if (__isOurExceptionClass(unwind_exception))
                __terminate(exception_header->terminateHandler);
        }
    }
#endif
    __terminate(get_terminate());
}

extern "C" {
new_handler __cxa_new_handler = 0;
}

new_handler
set_new_handler(new_handler handler) _NOEXCEPT
{
    return __libcpp_atomic_exchange(&__cxa_new_handler, handler, _AO_Acq_Rel);
}

new_handler
get_new_handler() _NOEXCEPT
{
    return __libcpp_atomic_load(&__cxa_new_handler, _AO_Acquire);
}

}  // std
