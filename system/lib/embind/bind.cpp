#include <emscripten/bind.h>
#include <../lib/libcxxabi/src/private_typeinfo.h>
#include <../lib/libcxxabi/include/cxxabi.h>
#include <list>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <emscripten/emscripten.h>

using namespace emscripten;

namespace __cxxabiv1 {
    std::vector<const __class_type_info*> __getBaseClasses(const __class_type_info* cti) {
        std::vector<const __class_type_info*> bases;

        const __si_class_type_info* scti = dynamic_cast<const __si_class_type_info*>(cti);
        if (scti) {
            bases.emplace_back(scti->__base_type);
        } else {
            const __vmi_class_type_info* vcti = dynamic_cast<const __vmi_class_type_info*>(cti);
            if (vcti) {
                for (int i = 0; i < vcti->__base_count; i++) {
                    bases.emplace_back(vcti->__base_info[i].__base_type);
                }
            }
        }
        return bases;
    }

    void __getDerivationPaths(const __class_type_info* dv, const __class_type_info* bs, std::vector<const __class_type_info*>path, std::vector<std::vector<const __class_type_info*>>& paths) {
        std::vector<const __class_type_info*> newPath(path);
        newPath.emplace_back(dv);
        if (dv == bs) {
            paths.emplace_back(newPath);
        } else {
            std::vector<const __class_type_info*> bases = __getBaseClasses(dv);
            for (int i = 0; i < bases.size(); i++) {
                __getDerivationPaths(bases[i], bs, newPath, paths);
            }
        }
    }
}

namespace emscripten {
    namespace internal {
        void registerStandardTypes() {
            static bool first = true;
            if (first) {
                first = false;

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
            }
        }

        extern "C" {
            // These three routines constitute an extension to the compiler's support for dynamic type conversion.
            // They are used by embind.js to implement automatic downcasting of return values which are pointers to
            // polymorphic objects.

            // __getDerivationPath returns an array of type_info pointers describing the derivation chain starting with
            // the derived type and proceeding toward (and ending with) the base type. Types are only included if they
            // appear on all possible derivation paths.
            std::vector<int> __getDerivationPath(int dv, const int bs) {
                std::vector<std::vector<const __cxxabiv1::__class_type_info*>> paths;

                const std::type_info* dv1 = (std::type_info*)dv;
                const std::type_info* bs1 = (std::type_info*)bs;
                const __cxxabiv1::__class_type_info* dv2 = dynamic_cast<const __cxxabiv1::__class_type_info*>(dv1);
                const __cxxabiv1::__class_type_info* bs2 = dynamic_cast<const __cxxabiv1::__class_type_info*>(bs1);

                if (dv2 && bs2) {
                    __cxxabiv1::__getDerivationPaths(dv2, bs2, std::vector<const __cxxabiv1::__class_type_info*>(), paths);
                }

                std::vector<int> derivationPath;
                if (paths.size() > 0) {
                    for (int j = 0; j < paths[0].size(); j++) {
                        bool disqualified = false;
                        for (int i = 1; i < paths.size(); i++) {
                            bool found = false;
                            for (int j2 = 0; j2 < paths[i].size(); j2++) {
                                if (paths[i][j2] == paths[0][j]) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                disqualified = true;
                                break;
                            }
                        }
                        if (!disqualified) {
                            derivationPath.emplace_back((int)paths[0][j]);
                        }
                    }
                }

                return derivationPath;
            }

            // __getDynamicPointerType returns (for polymorphic types only!) the type of the instance actually
            // pointed to.
            int EMSCRIPTEN_KEEPALIVE __getDynamicPointerType(int p) {
                void** vtable = *(void***)p;
                return (int)static_cast<const std::type_info*>(vtable[-1]);
            }

            // Calls to __dynamic_cast are generated by the compiler to implement dynamic_cast<>() -- its prototype is
            // not available through any header file. It is called directly here because it allows run-time
            // specification of the target pointer type (which can only be specified at compile time when using
            // dynamic_cast<>().
            void* __dynamic_cast(void*, const std::type_info*, const std::type_info*, int);

            // __dynamicPointerCast performs a C++ dynamic_cast<>() operation, but allowing run-time specification of
            // the from and to pointer types.
            int EMSCRIPTEN_KEEPALIVE __dynamicPointerCast(int p, int from, int to) {
                // The final parameter is a place-holder for a hint, a feature which is not currently implemented
                // in the emscripten runtime. The compiler passes a dummy value of -1, and so do we.
                return (int)__dynamic_cast((void *)p, (const std::type_info*)from, (const std::type_info *)to, -1);
            }

            const char* EMSCRIPTEN_KEEPALIVE __typeName(int p) {
                const std::type_info* ti = (const std::type_info*)p;
                size_t nameLen = std::min(strlen(ti->name()), (unsigned int)1024);
                char* name = (char *)malloc(nameLen+1);
                int stat;

                __cxxabiv1::__cxa_demangle(ti->name(), name, &nameLen, &stat);

                if (stat != 0) {
                    strncpy(name, ti->name(), nameLen);
                    name[nameLen] = '\0';
                }
                return name;
            }

            EMSCRIPTEN_BINDINGS(([]() {
                // We bind __getDerivationPath in order to take advantage of the std::vector to Javascript array
                // conversion for the return value. This has the unfortunate side-effect of exposing it to third party
                // developers, but perhaps the double underscore will scare them away from calling it.
                function("__getDerivationPath", &__getDerivationPath);
            }));


        }
    }
}



