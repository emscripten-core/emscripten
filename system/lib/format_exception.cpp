#define __USING_EMSCRIPTEN_EXCEPTIONS__
#include "cxa_exception.h"
#include "cxxabi.h"
#include <stdio.h>
#include <typeinfo>

extern "C" {

#define DEMANGLED_BUF_SIZE 100

int __cxa_can_catch(const std::type_info* catchType,
                    const std::type_info* excpType,
                    void** thrown);

int format_exception(char** result, void* exc_ptr) {
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
  int ret;
  if (can_catch) {
    const char* exc_what = ((std::exception*)exc_ptr)->what();
    ret = asprintf(result, "Cpp Exception %s: %s", exc_name, exc_what);
  } else {
    ret = asprintf(result,
                   "Cpp Exception: The exception is an object of type '%s' at "
                   "address %p which does not inherit from std::exception",
                   exc_name,
                   exc_ptr);
  }

  if (demangled_buf) {
    free(demangled_buf);
  }
  return ret;
}
}