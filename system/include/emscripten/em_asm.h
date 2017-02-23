#ifndef __em_asm_h__
#define __em_asm_h__

// The wasm backend calls vararg functions by passing the variadic arguments as
// an array on the stack. For EM_ASM's underlying functions, s2wasm needs to
// translate them to emscripten_asm_const_[signature], but the expected
// signature and arguments has been lost as part of the vararg buffer.
// Therefore, we declare EM_ASM's implementing functions as non-variadic.

#ifndef __asmjs
#ifndef __cplusplus
// In C, declare these as non-prototype declarations. This is obsolete K&R C
// (that is still supported) that causes the C frontend to consider any calls
// to them as valid, and avoids using the vararg calling convention.
void emscripten_asm_const();
int emscripten_asm_const_int();
double emscripten_asm_const_double();
#else
// C++ interprets an empty parameter list as a function taking no arguments,
// instead of a K&R C function declaration. Variadic templates are lowered as
// non-vararg calls to the instantiated templated function, which we then
// replace in s2wasm.
template <typename... Args> void emscripten_asm_const(const char* code, Args...);
template <typename... Args> int emscripten_asm_const_int(const char* code, Args...);
template <typename... Args> double emscripten_asm_const_double(const char* code, Args...);
#endif // __cplusplus
#else // __asmjs
// asmjs expects these to be vararg, so let them be vararg.
#ifdef __cplusplus
extern "C" {
#endif
void emscripten_asm_const(const char* code);
int emscripten_asm_const_int(const char* code, ...);
double emscripten_asm_const_double(const char* code, ...);
#ifdef __cplusplus
}
#endif
#endif // __asmjs

void emscripten_asm_const(const char* code);

#define EM_ASM(...) emscripten_asm_const(#__VA_ARGS__)
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(#code, __VA_ARGS__)
#define EM_ASM_INT_V(code) emscripten_asm_const_int(#code)
#define EM_ASM_DOUBLE_V(code) emscripten_asm_const_double(#code)

#endif // __em_asm_h__
