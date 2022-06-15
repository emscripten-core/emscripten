#include <stdint.h>
#include <stdlib.h>


#include "cxa_exception.h"
#include "cxa_handlers.h"
#include "emscripten.h"



// TODO: Remove this when Rust doesn't need it anymore.
// https://github.com/rust-lang/rust/pull/97888
extern "C" _LIBCXXABI_FUNC_VIS _Unwind_Reason_Code
__gxx_personality_v0(int version,
                     _Unwind_Action actions,
                     uint64_t exceptionClass,
                     _Unwind_Exception* unwind_exception,
                     _Unwind_Context* context) {
    abort();
}
