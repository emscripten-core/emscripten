#include <emscripten/bind.h>
#include <../lib/libcxxabi/include/cxxabi.h>
#include <list>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace emscripten;

static std::string _embind_getTypeName(intptr_t ti_raw) {
    auto ti = reinterpret_cast<const std::type_info*>(ti_raw);
    int stat;
    char* demangled = abi::__cxa_demangle(ti->name(), NULL, NULL, &stat);
    if (stat == 0) {
        std::string rv(demangled);
        free(demangled);
        return rv;
    }

    switch (stat) {
        case -1:
            return "<allocation failure>";
        case -2:
            return "<invalid C++ symbol>";
        case -3:
            return "<invalid argument>";
        default:
            return "<unknown error>";
    }
}

namespace emscripten {
    namespace internal {
        JSInterface* create_js_interface(EM_VAL e) {
            return new JSInterface(e);
        }
    }
}

EMSCRIPTEN_BINDINGS(native_and_builtin_types) {
    using namespace emscripten::internal;

    _embind_register_void(TypeID<void>::get(), "void");
    
    _embind_register_bool(TypeID<bool>::get(), "bool", true, false);

    _embind_register_integer(TypeID<char>::get(), "char");
    _embind_register_integer(TypeID<signed char>::get(), "signed char");
    _embind_register_integer(TypeID<unsigned char>::get(), "unsigned char");
    _embind_register_integer(TypeID<signed short>::get(), "short");
    _embind_register_integer(TypeID<unsigned short>::get(), "unsigned short");
    _embind_register_integer(TypeID<signed int>::get(), "int");
    _embind_register_integer(TypeID<unsigned int>::get(), "unsigned int");
    _embind_register_integer(TypeID<signed long>::get(), "long");
    _embind_register_integer(TypeID<unsigned long>::get(), "unsigned long");
    
    _embind_register_float(TypeID<float>::get(), "float");
    _embind_register_float(TypeID<double>::get(), "double");
    
    _embind_register_cstring(TypeID<std::string>::get(), "std::string");
    _embind_register_emval(TypeID<val>::get(), "emscripten::val");

    function("_embind_getTypeName", &_embind_getTypeName);
}
