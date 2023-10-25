#ifdef __EMSCRIPTEN__

#include "cxxabi.h"

#include "cxa_exception.h"
#include "private_typeinfo.h"
#include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
#include <string.h>

namespace __cxxabiv1 {

//  Utility routines
static
inline
__cxa_exception*
cxa_exception_from_thrown_object(void* thrown_object)
{
    return static_cast<__cxa_exception*>(thrown_object) - 1;
}

// Note:  This is never called when exception_header is masquerading as a
//        __cxa_dependent_exception.
static
inline
void*
thrown_object_from_cxa_exception(__cxa_exception* exception_header)
{
    return static_cast<void*>(exception_header + 1);
}

//  Get the exception object from the unwind pointer.
//  Relies on the structure layout, where the unwind pointer is right in
//  front of the user's exception object
static inline __cxa_exception* cxa_exception_from_unwind_exception(
  _Unwind_Exception* unwind_exception) {
  return cxa_exception_from_thrown_object(unwind_exception + 1);
}

extern "C" {

void* __thrown_object_from_unwind_exception(
  _Unwind_Exception* unwind_exception) {
  __cxa_exception* exception_header =
    cxa_exception_from_unwind_exception(unwind_exception);
  return thrown_object_from_cxa_exception(exception_header);
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
} // extern "C"

} // namespace __cxxabiv1

#endif // __EMSCRIPTEN__
