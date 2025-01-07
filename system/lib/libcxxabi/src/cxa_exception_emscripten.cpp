//===-------------------- cxa_exception_emscripten.cpp --------------------===//
//
//  Most code in the file is directly copied from cxa_exception.cpp.
//  TODO(sbc): consider merging them
//
//  Notable changes:
 //   __cxa_allocate_exception doesn't add get_cxa_exception_offset
//    __cxa_decrement_exception_refcount dosn't call the destructor if rethrown
//  Both of these changes are mirrored from the historical JS implemenation of
//  thse functions.
//
//===----------------------------------------------------------------------===//

#ifdef __EMSCRIPTEN__

#include "cxxabi.h"
#include "cxa_exception.h"
#include "include/atomic_support.h"
#include "fallback_malloc.h"
#include "private_typeinfo.h"
#include "stdio.h"
#include "assert.h"

#ifdef __WASM_EXCEPTIONS__
#error "This file should only be included when building with emscripten exceptions"
#endif

// Define to enable extra debugging on stderr.
#if EXCEPTIONS_DEBUG
#include "emscripten/console.h"
#define DEBUG emscripten_errf
#else
#define DEBUG(...)
#endif

// Until recently, Rust's `rust_eh_personality` for emscripten referred to this
// symbol. If Emscripten doesn't provide it, there will be errors when linking
// rust. The rust personality function is never called so we can just abort.
// We need this to support old versions of Rust.
// https://github.com/rust-lang/rust/pull/97888
// TODO: Remove this when Rust doesn't need it anymore.
extern "C" _LIBCXXABI_FUNC_VIS _Unwind_Reason_Code
__gxx_personality_v0(int version,
                     _Unwind_Action actions,
                     uint64_t exceptionClass,
                     _Unwind_Exception* unwind_exception,
                     _Unwind_Context* context) {
    abort();
}

namespace __cxxabiv1 {

//  Utility routines
static
inline
__cxa_exception*
cxa_exception_from_thrown_object(void* thrown_object)
{
    DEBUG("cxa_exception_from_thrown_object %p -> %p",
          thrown_object, static_cast<__cxa_exception*>(thrown_object) - 1);
    return static_cast<__cxa_exception*>(thrown_object) - 1;
}

// Note:  This is never called when exception_header is masquerading as a
//        __cxa_dependent_exception.
static
inline
void*
thrown_object_from_cxa_exception(__cxa_exception* exception_header)
{
    DEBUG("thrown_object_from_cxa_exception %p -> %p",
          exception_header, static_cast<void*>(exception_header + 1));
    return static_cast<void*>(exception_header + 1);
}

// Round s up to next multiple of a.
static inline
size_t aligned_allocation_size(size_t s, size_t a) {
    return (s + a - 1) & ~(a - 1);
}

static inline
size_t cxa_exception_size_from_exception_thrown_size(size_t size) {
    return aligned_allocation_size(size + sizeof (__cxa_exception),
                                   alignof(__cxa_exception));
}

extern "C" {

//  Allocate a __cxa_exception object, and zero-fill it.
//  Reserve "thrown_size" bytes on the end for the user's exception
//  object. Zero-fill the object. If memory can't be allocated, call
//  std::terminate. Return a pointer to the memory to be used for the
//  user's exception object.
void *__cxa_allocate_exception(size_t thrown_size) throw() {
    size_t actual_size = cxa_exception_size_from_exception_thrown_size(thrown_size);

    char *raw_buffer =
        (char *)__aligned_malloc_with_fallback(actual_size);
    if (NULL == raw_buffer)
        std::terminate();
    __cxa_exception *exception_header =
        static_cast<__cxa_exception *>((void *)(raw_buffer));
    ::memset(exception_header, 0, actual_size);
    return thrown_object_from_cxa_exception(exception_header);
}


//  Free a __cxa_exception object allocated with __cxa_allocate_exception.
void __cxa_free_exception(void *thrown_object) throw() {
    // Compute the size of the padding before the header.
    char *raw_buffer =
        ((char *)cxa_exception_from_thrown_object(thrown_object));
    __aligned_free_with_fallback((void *)raw_buffer);
}

/*
    If thrown_object is not null, atomically increment the referenceCount field
    of the __cxa_exception header associated with the thrown object referred to
    by thrown_object.

    Requires:  If thrown_object is not NULL, it is a native exception.
*/
void
__cxa_increment_exception_refcount(void *thrown_object) throw() {
    if (thrown_object != NULL )
    {
        __cxa_exception* exception_header = cxa_exception_from_thrown_object(thrown_object);
        DEBUG("INC: %p refcnt=%zu", thrown_object, exception_header->referenceCount);
        std::__libcpp_atomic_add(&exception_header->referenceCount, size_t(1));
    }
}

/*
    If thrown_object is not null, atomically decrement the referenceCount field
    of the __cxa_exception header associated with the thrown object referred to
    by thrown_object.  If the referenceCount drops to zero, destroy and
    deallocate the exception.

    Requires:  If thrown_object is not NULL, it is a native exception.
*/
_LIBCXXABI_NO_CFI
void __cxa_decrement_exception_refcount(void *thrown_object) throw() {
    if (thrown_object != NULL )
    {
        __cxa_exception* exception_header = cxa_exception_from_thrown_object(thrown_object);
        DEBUG("DEC: %p refcnt=%zu rethrown=%d", thrown_object,
              exception_header->referenceCount, exception_header->rethrown);
        assert(exception_header->referenceCount > 0);
        if (std::__libcpp_atomic_add(&exception_header->referenceCount, size_t(-1)) == 0 && !exception_header->rethrown)
        {
            DEBUG("DEL: %p (dtor=%p)", thrown_object, exception_header->exceptionDestructor);
            if (NULL != exception_header->exceptionDestructor)
                exception_header->exceptionDestructor(thrown_object);
            __cxa_free_exception(thrown_object);
        }
    }
}

__cxa_exception* __cxa_init_primary_exception(void* object, std::type_info* tinfo,
                                              void *(_LIBCXXABI_DTOR_FUNC* dest)(void*)) throw() {
  __cxa_exception* exception_header = cxa_exception_from_thrown_object(object);
  exception_header->referenceCount = 0;
  exception_header->exceptionType = tinfo;
  exception_header->exceptionDestructor = dest;
  return exception_header;
}

}  // extern "C"

}  // abi

#endif // __EMSCRIPTEN__
