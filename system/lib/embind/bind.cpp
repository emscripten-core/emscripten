#include <emscripten/bind.h>
#ifdef USE_CXA_DEMANGLE
#include <../lib/libcxxabi/include/cxxabi.h>
#endif
#include <list>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <emscripten/emscripten.h>
#include <climits>
#include <limits>

using namespace emscripten;

extern "C" {
    const char* __attribute__((used)) __getTypeName(const std::type_info* ti) {
        if (has_unbound_type_names) {
#ifdef USE_CXA_DEMANGLE
            int stat;
            char* demangled = abi::__cxa_demangle(ti->name(), NULL, NULL, &stat);
            if (stat == 0 && demangled) {
                return demangled;
            }

            switch (stat) {
                case -1:
                    return strdup("<allocation failure>");
                case -2:
                    return strdup("<invalid C++ symbol>");
                case -3:
                    return strdup("<invalid argument>");
                default:
                    return strdup("<unknown error>");
            }
#else
            return strdup(ti->name());
#endif
        } else {
            char str[80];
            sprintf(str, "%p", ti);
            return strdup(str);
        }
    }
}

namespace {
    template<typename T>
    static void register_integer(const char* name) {
        using namespace internal;
        _embind_register_integer(TypeID<T>::get(), name, sizeof(T), std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    template<typename T>
    static void register_float(const char* name) {
        using namespace internal;
        _embind_register_float(TypeID<T>::get(), name, sizeof(T));
    }
}

EMSCRIPTEN_BINDINGS(native_and_builtin_types) {
    using namespace emscripten::internal;

    _embind_register_void(TypeID<void>::get(), "void");
    
    _embind_register_bool(TypeID<bool>::get(), "bool", sizeof(bool), true, false);

    register_integer<char>("char");
    register_integer<signed char>("signed char");
    register_integer<unsigned char>("unsigned char");
    register_integer<signed short>("short");
    register_integer<unsigned short>("unsigned short");
    register_integer<signed int>("int");
    register_integer<unsigned int>("unsigned int");
    register_integer<signed long>("long");
    register_integer<unsigned long>("unsigned long");
    
    register_float<float>("float");
    register_float<double>("double");
    
    _embind_register_std_string(TypeID<std::string>::get(), "std::string");
    _embind_register_std_string(TypeID<std::basic_string<unsigned char> >::get(), "std::basic_string<unsigned char>");
    _embind_register_std_wstring(TypeID<std::wstring>::get(), sizeof(wchar_t), "std::wstring");
    _embind_register_emval(TypeID<val>::get(), "emscripten::val");
    _embind_register_memory_view(TypeID<memory_view>::get(), "emscripten::memory_view");
}
