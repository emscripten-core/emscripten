//===------------------- cxa_exception_emscripten.cpp ---------------------===//
//
// This code contains Emscripten specific code for exception handling.
// Emscripten has two modes of exception handling: Emscripten EH, which uses JS
// glue code, and Wasm EH, which uses the new Wasm exception handling proposal
// and meant to be faster. Code for different modes is demarcated with
// '__USING_EMSCRIPTEN_EXCEPTIONS__' and '__USING_WASM_EXCEPTIONS__'.
//
//===----------------------------------------------------------------------===//

#include "cxa_exception.h"
#include "private_typeinfo.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__USING_WASM_EXCEPTIONS__)
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
#endif // !defined(__USING_WASM_EXCEPTIONS__)

#if defined(__USING_EMSCRIPTEN_EXCEPTIONS__) ||                                \
  defined(__USING_WASM_EXCEPTIONS__)

using namespace __cxxabiv1;

//  Some utility routines are copied from cxa_exception.cpp
static inline __cxa_exception*
cxa_exception_from_thrown_object(void* thrown_object) {
  return static_cast<__cxa_exception*>(thrown_object) - 1;
}

// Note:  This is never called when exception_header is masquerading as a
//        __cxa_dependent_exception.
static inline void*
thrown_object_from_cxa_exception(__cxa_exception* exception_header) {
  return static_cast<void*>(exception_header + 1);
}

//  Get the exception object from the unwind pointer.
//  Relies on the structure layout, where the unwind pointer is right in
//  front of the user's exception object
static inline __cxa_exception* cxa_exception_from_unwind_exception(
  _Unwind_Exception* unwind_exception) {
  return cxa_exception_from_thrown_object(unwind_exception + 1);
}

static inline void* thrown_object_from_unwind_exception(
  _Unwind_Exception* unwind_exception) {
  __cxa_exception* exception_header =
    cxa_exception_from_unwind_exception(unwind_exception);
  return thrown_object_from_cxa_exception(exception_header);
}

extern "C" {

void* __thrown_object_from_unwind_exception(
  _Unwind_Exception* unwind_exception) {
  return thrown_object_from_unwind_exception(unwind_exception);
}

// Given a thrown_object, puts the information about its type and message into  
// 'type' and 'message' output parameters. 'type' will contain the string        
// representation of the type of the exception, e.g., 'int'. 'message' will      
// contain the result of 'std::exception::what()' method if the type of the      
// exception is a subclass of std::exception; otherwise it will be NULL. The     
// caller is responsible for freeing 'type' buffer and also 'message' buffer, if 
// it is not NULL.
void __get_exception_message(void* thrown_object, char** type, char** message) {
  __cxa_exception* exception_header =
    cxa_exception_from_thrown_object(thrown_object);
  const __shim_type_info* thrown_type =
    static_cast<const __shim_type_info*>(exception_header->exceptionType);
  const char* type_name = thrown_type->name();

  int status = 0;
  char* demangled_buf = __cxa_demangle(type_name, 0, 0, &status);
  if (status == 0 && demangled_buf) {
    *type = demangled_buf;
  } else {
    if (demangled_buf) {
      free(demangled_buf);
    }
    *type = (char*)malloc(strlen(type_name) + 1);
    strcpy(*type, type_name);
  }

  *message = NULL;
  const __shim_type_info* catch_type =
    static_cast<const __shim_type_info*>(&typeid(std::exception));
  int can_catch = catch_type->can_catch(thrown_type, thrown_object);
  if (can_catch) {
    const char* what =
      static_cast<const std::exception*>(thrown_object)->what();
    *message = (char*)malloc(strlen(what) + 1);
    strcpy(*message, what);
  }
}

// Returns a message saying that execution was terminated due to an exception.
// This message is freshly malloc'd and should be freed.
char* __get_exception_terminate_message(void* thrown_object) {
  char* type;
  char* message;
  __get_exception_message(thrown_object, &type, &message);
  char* result;
  if (message != NULL) {
    asprintf(
      &result, "terminating with uncaught exception %s: %s", type, message);
    free(message);
  } else {
    asprintf(&result, "terminating with uncaught exception of type %s", type);
  }
  free(type);
  return result;
}
}

#endif // __USING_EMSCRIPTEN_EXCEPTIONS__ || __USING_WASM_EXCEPTIONS__
