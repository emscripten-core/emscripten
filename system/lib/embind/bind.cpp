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

    int __getBaseOffset(const __class_type_info* ctiDv, const __class_type_info* ctiBs) {
        int offset = 0;

        const __vmi_class_type_info* vcti = dynamic_cast<const __vmi_class_type_info*>(ctiDv);
        if (vcti) {
            for (int i = 0; i < vcti->__base_count; i++) {
                if (vcti->__base_info[i].__base_type == ctiBs) {
                    offset = vcti->__base_info[i].__offset_flags >> __base_class_type_info::__offset_shift;
                    break;
                }
            }
        }
        return offset;
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

    int __pathOffset(std::vector<const __cxxabiv1::__class_type_info*> path) {
        int offset = 0;
        for (int i = 0; i < path.size()-1; i++) {
            offset += __getBaseOffset(path[i], path[i+1]);
        }
        return offset;
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

            void* EMSCRIPTEN_KEEPALIVE __staticPointerCast(void* p, int from, int to) {
                std::vector<std::vector<const __cxxabiv1::__class_type_info*>> paths;
                int direction = 1;

                const std::type_info* from1 = (std::type_info*)from;
                const std::type_info* to1 = (std::type_info*)to;
                const __cxxabiv1::__class_type_info* from2 = dynamic_cast<const __cxxabiv1::__class_type_info*>(from1);
                const __cxxabiv1::__class_type_info* to2 = dynamic_cast<const __cxxabiv1::__class_type_info*>(to1);

                if (from2 && to2) {
                    __cxxabiv1::__getDerivationPaths(from2, to2, std::vector<const __cxxabiv1::__class_type_info*>(), paths);
                    if (paths.size() == 0) {
                        __cxxabiv1::__getDerivationPaths(to2, from2, std::vector<const __cxxabiv1::__class_type_info*>(), paths);
                        direction = -1;
                    }
                }

                int offset = -1;
                for (int i = 0; i < paths.size(); i++) {
                    if (offset < 0) {
                        offset = __cxxabiv1::__pathOffset(paths[i]);
                    } else {
                        if (offset != __cxxabiv1::__pathOffset(paths[i])) {
                            return (void *)-2;
                        }
                    }
                }
                if (offset < 0) {
                    return (void *)-1;
                }
                if (p == 0) {
                    return (void *)0;
                }
                return (void *)((int)p + offset * direction);
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
            int EMSCRIPTEN_KEEPALIVE __dynamicPointerCast(int p, int to) {
                int ret = (int)__staticPointerCast((void *)p, __getDynamicPointerType(p), to);
                if (ret < 0) {
                    return 0;
                }
                return ret;
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

            int EMSCRIPTEN_KEEPALIVE __peek32(int p) {
                return *(int *)p;
            }

            EMSCRIPTEN_BINDINGS(([]() {
                // We bind __getDerivationPath in order to take advantage of the std::vector to Javascript array
                // conversion for the return value. This has the unfortunate side-effect of exposing it to third party
                // developers, but perhaps the double underscore will scare them away from calling it.
                function("__getDerivationPath", &__getDerivationPath);
                function("__peek32", &__peek32);
            }));
        }

        JSInterface* create_js_interface(EM_VAL e) {
            return new JSInterface(e);
        }
    }
}



