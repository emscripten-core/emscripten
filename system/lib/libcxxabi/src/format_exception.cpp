#include "cxa_exception.h"
#include "cxxabi.h"
#include <stdio.h>
#include <typeinfo>

#ifdef __USING_EMSCRIPTEN_EXCEPTIONS__

extern "C" {

int __cxa_can_catch(const std::type_info* catchType,
                    const std::type_info* excpType,
                    void** thrown);

char* emscripten_format_exception(void* exc_ptr) {
  __cxxabiv1::__cxa_exception* exc_info =
    (__cxxabiv1::__cxa_exception*)exc_ptr - 1;
  std::type_info* exc_type = exc_info->exceptionType;
  const char* exc_name = exc_type->name();

  int status = 0;
  char* demangled_buf = __cxxabiv1::__cxa_demangle(exc_name, 0, 0, &status);
  if (status == 0 && demangled_buf) {
    exc_name = demangled_buf;
  }

  int can_catch = __cxa_can_catch(&typeid(std::exception), exc_type, &exc_ptr);
  char* result = NULL;
  if (can_catch) {
    const char* exc_what = ((std::exception*)exc_ptr)->what();
    asprintf(&result, "Cpp Exception %s: %s", exc_name, exc_what);
  } else {
    asprintf(&result,
             "Cpp Exception: The exception is an object of type '%s' at "
             "address %p which does not inherit from std::exception",
             exc_name,
             exc_ptr);
  }

  if (demangled_buf) {
    free(demangled_buf);
  }
  return result;
}
}

#endif // __USING_EMSCRIPTEN_EXCEPTIONS__
